
#include "Songcast.h"
#include <IOKit/IOLib.h>
#include <libkern/OSByteOrder.h>
#include <netinet/in.h>



// struct for defining the IPv4 socket address
typedef struct SocketAddress
{
	uint8_t	sa_len;
	sa_family_t	sa_family;
    uint16_t port;
    uint32_t addr;
    char zero[8];
    
} __attribute__((__packed__)) SocketAddress;



// implementation of SongcastSocket class
SongcastSocket::SongcastSocket()
: iSocket(0)
, iTtl(1)
, iState(eSongcastStateInactive)
{
}


SongcastSocket::~SongcastSocket()
{
}


void SongcastSocket::Open(uint32_t aIpAddress, uint16_t aPort, uint32_t aAdapter)
{
    // ensure socket is closed
    Close();
    
    // create the new socket
    errno_t err = sock_socket(PF_INET, SOCK_DGRAM, 0, NULL, NULL, &iSocket);
    if (err != 0) {
        IOLog("Songcast SongcastSocket[%p]::Open(0x%x, %u, 0x%x) sock_socket failed with %d\n", this, aIpAddress, aPort, aAdapter, err);
        iSocket = 0;
        return;
    }
    
    // set the adapter for multicast sending
    struct in_addr adapter;
    adapter.s_addr = aAdapter;
    err = sock_setsockopt(iSocket, IPPROTO_IP, IP_MULTICAST_IF, &adapter, sizeof(adapter));
    if (err != 0) {
        IOLog("Songcast SongcastSocket[%p]::Open(0x%x, %u, 0x%x) sock_setsockopt failed with %d\n", this, aIpAddress, aPort, aAdapter, err);
        sock_close(iSocket);
        iSocket = 0;
        return;
    }

    // connect the socket to the endpoint
    SocketAddress addr;
    memset(&addr, 0, sizeof(SocketAddress));
    addr.sa_len = sizeof(SocketAddress);
    addr.sa_family = AF_INET;
    addr.port = htons(aPort);
    addr.addr = aIpAddress;
    
    err = sock_connect(iSocket, (const sockaddr*)&addr, 0);
    if (err != 0) {
        IOLog("Songcast SongcastSocket[%p]::Open(0x%x, %u, 0x%x) sock_connect failed with %d\n", this, aIpAddress, aPort, aAdapter, err);
        sock_close(iSocket);
        iSocket = 0;
        return;
    }

    // set the ttl
    SetSocketTtl();

    IOLog("Songcast SongcastSocket[%p]::Open(0x%x, %u, 0x%x) ok\n", this, aIpAddress, aPort, aAdapter);
}


void SongcastSocket::Close()
{
    if (iSocket != 0) {
        sock_close(iSocket);
        iSocket = 0;
        IOLog("Songcast SongcastSocket[%p]::Close()\n", this);
    }
}


void SongcastSocket::Send(SongcastAudioMessage& aMsg)
{
    if (iSocket == 0 || iState == eSongcastStateInactive) {
        return;
    }
    
    if (iState == eSongcastStatePendingInactive) {
        aMsg.SetHaltFlag(true);
    }
    
    struct iovec sockdata;
    sockdata.iov_base = aMsg.Ptr();
    sockdata.iov_len = aMsg.Bytes();
    
    struct msghdr msg;
    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = &sockdata;
    msg.msg_iovlen = 1;
    msg.msg_control = 0;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;
    
    size_t bytesSent;
    errno_t ret = sock_send(iSocket, &msg, 0, &bytesSent);
    if (ret != 0) {
        IOLog("Songcast SongcastSocket[%p]::Send() sock_send returned %d\n", this, ret);
    }

    // set state to inactive if pending
    if (iState == eSongcastStatePendingInactive) {
        IOLog("Songcast SongcastSocket[%p]::Send() pending->inactive\n", this);
        iState = eSongcastStateInactive;
    }
}


void SongcastSocket::SetActive(uint64_t aActive)
{
    IOLog("Songcast SongcastSocket[%p]::SetActive(%llu)\n", this, aActive);
    iState = (aActive != 0) ? eSongcastStateActive : eSongcastStatePendingInactive;
}


void SongcastSocket::SetTtl(uint64_t aTtl)
{
    IOLog("Songcast SongcastSocket[%p]::SetTtl(%llu)\n", this, aTtl);
    iTtl = aTtl;
    SetSocketTtl();
}


void SongcastSocket::SetSocketTtl()
{
    if (iSocket != 0)
    {
        u_char ttl = iTtl;
        if (sock_setsockopt(iSocket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) != 0) {
            IOLog("Songcast SongcastSocket[%p]::SetSocketTtl() sock_setsockopt(ttl = %d) failed\n", this, ttl);
        }
    }
}



// implementation of SongcastAudioMessage class
SongcastAudioMessage::SongcastAudioMessage(uint32_t aFrames, uint32_t aChannels, uint32_t aBitDepth)
: iPtr(0)
, iAudioBytes(aFrames * aChannels * aBitDepth / 8)
{
    iPtr = IOMalloc(Bytes());
    
    // set some static header fields
    Header()->iSignature[0] = 'O';
    Header()->iSignature[1] = 'h';
    Header()->iSignature[2] = 'm';
    Header()->iSignature[3] = ' ';
    Header()->iVersion = 1;
    Header()->iType = 3;
    Header()->iAudioHeaderBytes = 50;
    Header()->iAudioNetworkTimestamp = 0;
    Header()->iAudioMediaLatency = OSSwapHostToBigInt32(4410 * 256);    // init to 100ms at 44.1kHz
    Header()->iAudioMediaTimestamp = 0;
    Header()->iAudioStartSample = 0;
    Header()->iAudioTotalSamples = 0;
    Header()->iAudioVolumeOffset = 0;
    Header()->iAudioReserved = 0;
    
    // set size of header + audio data
    Header()->iTotalBytes = OSSwapHostToBigInt16(Bytes());
    
    // codec information
    Header()->iAudioCodecNameBytes = 3;
    Header()->iAudioCodecName[0]= 'P';
    Header()->iAudioCodecName[1]= 'C';
    Header()->iAudioCodecName[2]= 'M';
    
    Header()->iAudioBitDepth = aBitDepth;
    Header()->iAudioChannels = aChannels;
    Header()->iAudioSampleCount = OSSwapHostToBigInt16(aFrames);
    
    // lossless audio with timestamps
    Header()->iAudioFlags = 6;
}


SongcastAudioMessage::~SongcastAudioMessage()
{
    if (iPtr) {
        IOFree(iPtr, Bytes());
    }
}


void SongcastAudioMessage::SetHaltFlag(bool aHalt)
{
    if (aHalt) {
        Header()->iAudioFlags = 7;
    }
    else {
        Header()->iAudioFlags = 6;
    }
}


void SongcastAudioMessage::SetSampleRate(uint32_t aSampleRate)
{
    Header()->iAudioSampleRate = OSSwapHostToBigInt32(aSampleRate);
    Header()->iAudioBitRate = OSSwapHostToBigInt32(aSampleRate * Header()->iAudioChannels * Header()->iAudioBitDepth);
}


void SongcastAudioMessage::SetFrame(uint32_t aFrame)
{
    Header()->iAudioFrame = OSSwapHostToBigInt32(aFrame);
}


void SongcastAudioMessage::SetTimestamp(uint32_t aTimestamp)
{
    Header()->iAudioNetworkTimestamp = OSSwapHostToBigInt32(aTimestamp);
    Header()->iAudioMediaTimestamp = Header()->iAudioNetworkTimestamp;
}


void SongcastAudioMessage::SetMediaLatency(uint32_t aLatency)
{
    Header()->iAudioMediaLatency = OSSwapHostToBigInt32(aLatency);
}


void SongcastAudioMessage::SetData(void* aPtr, uint32_t aBytes)
{
    void* audioPtr = (uint8_t*)iPtr + sizeof(SongcastAudioHeader);
    
    if (aBytes == iAudioBytes) {
        memcpy(audioPtr, aPtr, iAudioBytes);
    }
    else if (aBytes < iAudioBytes) {
        memset(audioPtr, 0, iAudioBytes);
        memcpy(audioPtr, aPtr, aBytes);
    }
    else {
        memset(audioPtr, 0, iAudioBytes);
    }
}


