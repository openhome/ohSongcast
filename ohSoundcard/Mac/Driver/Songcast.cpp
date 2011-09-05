
#include "Songcast.h"
#include <IOKit/IOLib.h>
#include <libkern/OSByteOrder.h>



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
, iTtl(4)
, iState(eSongcastStateInactive)
{
}


SongcastSocket::~SongcastSocket()
{
}


void SongcastSocket::Open(uint32_t aIpAddress, uint16_t aPort)
{
    // ensure socket is closed
    Close();
    
    // create the new socket
    errno_t err = sock_socket(PF_INET, SOCK_DGRAM, 0, NULL, NULL, &iSocket);
    if (err != 0) {
        IOLog("ohSoundcard SongcastSocket[%p]::Open(0x%x, %u) sock_socket failed with %d\n", this, aIpAddress, aPort, err);
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
        IOLog("ohSoundcard SongcastSocket[%p]::Open(0x%x, %u) sock_connect failed with %d\n", this, aIpAddress, aPort, err);
        sock_close(iSocket);
        iSocket = 0;
        return;
    }

    IOLog("ohSoundcard SongcastSocket[%p]::Open(0x%x, %u) ok\n", this, aIpAddress, aPort);
}


void SongcastSocket::Close()
{
    if (iSocket != 0) {
        sock_close(iSocket);
        iSocket = 0;
        IOLog("ohSoundcard SongcastSocket[%p]::Close()\n", this);
    }
}


void SongcastSocket::Send(SongcastAudioMessage& aMsg)
{
    if (iSocket == 0 || iState == eSongcastStateInactive) {
        return;
    }
    
    if (iState == eSongcastStatePendingInactiveHalt) {
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
    sock_send(iSocket, &msg, 0, &bytesSent);

    // set state to inactive if pending
    if (iState == eSongcastStatePendingInactiveHalt) {
        IOLog("ohSoundcard SongcastSocket[%p]::Send() pending->inactive\n", this);
        iState = eSongcastStateInactive;
    }
}


void SongcastSocket::SetActive(uint64_t aActive)
{
    IOLog("ohSoundcard SongcastSocket[%p]::SetActive(%llu)\n", this, aActive);
    iState = (aActive != 0) ? eSongcastStateActive : eSongcastStateInactive;
}


void SongcastSocket::SetInactiveAndHalt()
{
    IOLog("ohSoundcard SongcastSocket[%p]::SetInactiveAndHalt()\n", this);
    iState = eSongcastStatePendingInactiveHalt;
}


void SongcastSocket::SetTtl(uint64_t aTtl)
{
    IOLog("ohSoundcard SongcastSocket[%p]::SetTtl(%llu)\n", this, aTtl);
    iTtl = aTtl;
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
    Header()->iAudioMediaLatency = OSSwapHostToBigInt32(1000000);
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


