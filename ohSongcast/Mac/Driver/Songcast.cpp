
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



// Implementation of main Songcast class
const SongcastFormat Songcast::SupportedFormats[SupportedFormatCount] = 
{
    // SampleRate, BitDepth, Channels, SampleCount
    SongcastFormat(44100, 24, 2, 220)
};


Songcast::Songcast()
    : iSocket()
    , iState(eSongcastStateInactive)
    , iHistory()
    , iLatencyMs(100)
    , iFrame(0)
{
    // calculate the maximum songcast audio data size
    uint32_t maxBytes = 0;
    for (uint32_t i=0 ; i<SupportedFormatCount ; i++)
    {
        uint32_t bytes = SupportedFormats[i].Bytes();
        if (bytes > maxBytes) {
            maxBytes = bytes;
        }
    }

    // create a full queue of audio messages in the history
    for (uint32_t i=0 ; i<kHistoryCount ; i++)
    {
        // messages contain a buffer to contain the maximum message size and are initialised
        // with the first supported format
        SongcastAudioMessage* msg = new SongcastAudioMessage(maxBytes, SupportedFormats[0]);

        if (msg && msg->Ptr() == 0)
        {
            delete msg;
            msg = 0;
        }

        if (msg == 0) {
            IOLog("Songcast Songcast[%p]::Songcast() failed to create audio msg\n", this);
        }
        else {
            iHistory.Write(msg);
        }
    }
}


Songcast::~Songcast()
{
    TUint count = iHistory.SlotsUsed();

    for (TUint i=0 ; i<count ; i++)
    {
        SongcastAudioMessage* msg = iHistory.Read();
        delete msg;
    }
}


void Songcast::SetActive(uint64_t aActive)
{
    IOLog("Songcast Songcast[%p]::SetActive(%llu)\n", this, aActive);
    iState = (aActive != 0) ? eSongcastStateActive : eSongcastStatePendingInactive;
}


void Songcast::SetEndpoint(uint32_t aIpAddress, uint16_t aPort, uint32_t aAdapter)
{
    iSocket.SetEndpoint(aIpAddress, aPort, aAdapter);
}


void Songcast::SetTtl(uint64_t aTtl)
{
    iSocket.SetTtl(aTtl);
}


void Songcast::SetLatencyMs(uint64_t aLatencyMs)
{
    IOLog("Songcast Songcast[%p]::SetLatencyMs(%llu)\n", this, aLatencyMs);
    iLatencyMs = aLatencyMs;
}


void Songcast::Send(const SongcastFormat& aFormat, uint64_t aTimestampNs, bool aHalt, void* aData, uint32_t aBytes)
{
    // don't send if songcast is inactive or no audio packets were created
    if (iState == eSongcastStateInactive || iHistory.SlotsUsed() == 0) {
        return;
    }

    // extract the oldest message from the history
    SongcastAudioMessage* msg = iHistory.Read();

    // if songcast inactive state is pending, this is the last audio msg - set the halt flag
    if (iState == eSongcastStatePendingInactive) {
        aHalt = true;
    }

    // setup the audio message
    msg->SetHeader(aFormat, aTimestampNs, iLatencyMs, aHalt, ++iFrame);
    msg->SetData(aData, aBytes);

    // send the msg
    iSocket.Send(*msg);

    // set the resend flag before adding the message back to the history as most recent
    msg->SetResent();
    iHistory.Write(msg);

    // set state to inactive if pending
    if (iState == eSongcastStatePendingInactive) {
        IOLog("Songcast Songcast[%p]::Send() pending->inactive\n", this);
        iState = eSongcastStateInactive;
    }
}


void Songcast::Resend(uint64_t aFrameCount, const uint32_t* aFrames)
{
    IOLog("Songcast Songcast[%p]::Resend(%llu, [", this, aFrameCount);

    for (uint64_t i=0 ; i<aFrameCount ; i++)
    {
        uint32_t frame = OSSwapBigToHostInt32(*(aFrames + i));
        IOLog("%u,", frame);
    }

    IOLog("])\n");

    if (aFrameCount == 0) {
        return;
    }

    TUint count = iHistory.SlotsUsed();
    for (TUint i=0 ; i<count ; i++)
    {
        SongcastAudioMessage* msg = iHistory.Read();
        iHistory.Write(msg);

        for (uint64_t i=0 ; i<aFrameCount ; i++)
        {
            uint32_t frame = OSSwapBigToHostInt32(*(aFrames + i));

            if (frame == msg->Frame())
            {
                iSocket.Send(*msg);
                break;
            }
        }
    }
}



// implementation of SongcastSocket class
SongcastSocket::SongcastSocket()
: iSocket(0)
, iTtl(1)
{
}


SongcastSocket::~SongcastSocket()
{
    if (iSocket != 0) {
        sock_close(iSocket);
        iSocket = 0;
    }
}


void SongcastSocket::SetEndpoint(uint32_t aIpAddress, uint16_t aPort, uint32_t aAdapter)
{
    // ensure socket is closed
    if (iSocket != 0) {
        sock_close(iSocket);
        iSocket = 0;
    }
    
    // create the new socket
    errno_t err = sock_socket(PF_INET, SOCK_DGRAM, 0, NULL, NULL, &iSocket);
    if (err != 0) {
        IOLog("Songcast SongcastSocket[%p]::SetEndpoint(0x%x, %u, 0x%x) sock_socket failed with %d\n", this, aIpAddress, aPort, aAdapter, err);
        iSocket = 0;
        return;
    }
    
    // set the adapter for multicast sending
    struct in_addr adapter;
    adapter.s_addr = aAdapter;
    err = sock_setsockopt(iSocket, IPPROTO_IP, IP_MULTICAST_IF, &adapter, sizeof(adapter));
    if (err != 0) {
        IOLog("Songcast SongcastSocket[%p]::SetEndpoint(0x%x, %u, 0x%x) sock_setsockopt failed with %d\n", this, aIpAddress, aPort, aAdapter, err);
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
        IOLog("Songcast SongcastSocket[%p]::SetEndpoint(0x%x, %u, 0x%x) sock_connect failed with %d\n", this, aIpAddress, aPort, aAdapter, err);
        sock_close(iSocket);
        iSocket = 0;
        return;
    }

    // set the ttl
    SetSocketTtl();

    IOLog("Songcast SongcastSocket[%p]::SetEndpoint(0x%x, %u, 0x%x) ok\n", this, aIpAddress, aPort, aAdapter);
}


void SongcastSocket::Send(SongcastAudioMessage& aMsg)
{
    if (iSocket == 0) {
        return;
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
SongcastAudioMessage::SongcastAudioMessage(uint32_t aMaxAudioBytes, const SongcastFormat& aFormat)
    : iPtr(0)
    , iAudioBytes(aFormat.Bytes())
    , iMaxAudioBytes(aMaxAudioBytes)
{
    // allocate a buffer to contain the biggest audio packet
    iPtr = IOMalloc(sizeof(SongcastAudioHeader) + iMaxAudioBytes);
    
    if (iPtr)
    {
        // set header data that does not change
        Header()->iSignature[0] = 'O';
        Header()->iSignature[1] = 'h';
        Header()->iSignature[2] = 'm';
        Header()->iSignature[3] = ' ';
        Header()->iVersion = 1;
        Header()->iType = 3;
        Header()->iAudioHeaderBytes = 50;
        Header()->iAudioStartSample = 0;
        Header()->iAudioTotalSamples = 0;
        Header()->iAudioVolumeOffset = 0;
        Header()->iAudioReserved = 0;
        Header()->iAudioCodecNameBytes = 3;
        Header()->iAudioCodecName[0]= 'P';
        Header()->iAudioCodecName[1]= 'C';
        Header()->iAudioCodecName[2]= 'M';

        // initialise header data to default values
        Header()->iTotalBytes = OSSwapHostToBigInt16(Bytes());
        Header()->iAudioFlags = 6;  // lossless audio with timestamps
        Header()->iAudioSampleCount = OSSwapHostToBigInt16(aFormat.SampleCount);
        Header()->iAudioFrame = 0;
        Header()->iAudioNetworkTimestamp = 0;
        Header()->iAudioMediaLatency = OSSwapHostToBigInt32((aFormat.SampleRate * 256 * 100)/1000);    // init to 100ms at given sample rate
        Header()->iAudioMediaTimestamp = 0;
        Header()->iAudioSampleRate = OSSwapHostToBigInt32(aFormat.SampleRate);
        Header()->iAudioBitRate = OSSwapHostToBigInt32(aFormat.SampleRate * aFormat.Channels * aFormat.BitDepth);
        Header()->iAudioBitDepth = aFormat.BitDepth;
        Header()->iAudioChannels = aFormat.Channels;
    }
}


SongcastAudioMessage::~SongcastAudioMessage()
{
    if (iPtr) {
        IOFree(iPtr, sizeof(SongcastAudioHeader) + iMaxAudioBytes);
    }
}


uint32_t SongcastAudioMessage::Bytes() const
{
    return (sizeof(SongcastAudioHeader) + iAudioBytes);
}


uint32_t SongcastAudioMessage::Frame() const
{
    return OSSwapBigToHostInt32(Header()->iAudioFrame);
}


void SongcastAudioMessage::SetHeader(const SongcastFormat& aFormat, uint64_t aTimestampNs, uint64_t aLatencyMs, bool aHalt, uint32_t aFrame)
{
    // setup message audio data size and format
    iAudioBytes = aFormat.Bytes();
    Header()->iTotalBytes = OSSwapHostToBigInt16(Bytes());
    Header()->iAudioSampleCount = OSSwapHostToBigInt16(aFormat.SampleCount);
    Header()->iAudioSampleRate = OSSwapHostToBigInt32(aFormat.SampleRate);
    Header()->iAudioBitRate = OSSwapHostToBigInt32(aFormat.SampleRate * aFormat.Channels * aFormat.BitDepth);
    Header()->iAudioBitDepth = aFormat.BitDepth;
    Header()->iAudioChannels = aFormat.Channels;

    // convert ns timestamp to correct units and set timestamp fields
    uint64_t timestamp = (aTimestampNs * aFormat.SampleRate * 256) / 1000000000;
    Header()->iAudioNetworkTimestamp = OSSwapHostToBigInt32(timestamp);
    Header()->iAudioMediaTimestamp = Header()->iAudioNetworkTimestamp;

    // convert and set latency
    uint64_t latency = (aLatencyMs * aFormat.SampleRate * 256) / 1000;
    Header()->iAudioMediaLatency = OSSwapHostToBigInt32(latency);

    // set the audio flags to lossless with timestamps and, optionally, a halt
    if (aHalt) {
        Header()->iAudioFlags = 7;
    }
    else {
        Header()->iAudioFlags = 6;
    }

    // set the frame numnber
    Header()->iAudioFrame = OSSwapHostToBigInt32(aFrame);
}


void SongcastAudioMessage::SetResent()
{
    // set the resent flag
    Header()->iAudioFlags |= 0x08;
}


void SongcastAudioMessage::SetData(void* aPtr, uint32_t aBytes)
{
    void* audioPtr = (uint8_t*)iPtr + sizeof(SongcastAudioHeader);

    if (aBytes == iMaxAudioBytes) {
        memcpy(audioPtr, aPtr, aBytes);
    }
    else if (aBytes < iMaxAudioBytes) {
        memset(audioPtr, 0, iMaxAudioBytes);
        memcpy(audioPtr, aPtr, aBytes);
    }
    else {
        memset(audioPtr, 0, iMaxAudioBytes);
    }
}



// Implementation of SongcastFormat
SongcastFormat::SongcastFormat(uint32_t aSampleRate, uint8_t aBitDepth, uint8_t aChannels, uint16_t aSampleCount)
    : SampleRate(aSampleRate)
    , BitDepth(aBitDepth)
    , Channels(aChannels)
    , SampleCount(aSampleCount)
{
}

uint32_t SongcastFormat::Bytes() const
{
    return SampleCount * Channels * BitDepth / 8;
}

uint64_t SongcastFormat::TimeNs() const
{
    // calculate the time of these packets - use 64-bit to avoid overflow
    uint64_t t = 1000000000;
    t *= SampleCount;
    t /= SampleRate;
    return t;
}





