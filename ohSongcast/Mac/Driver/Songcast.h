#ifndef HEADER_SONGCAST
#define HEADER_SONGCAST

#include <sys/kpi_socket.h>
#include <OpenHome/Fifo.h>


// NOTE: This struct is __packed__ - this prevents the compiler from adding
// padding to the struct in order to align data on 4 byte boundaries. This is
// required because, when sending the audio packets a single buffer is allocated
// that holds the header and the audio data. The start of this buffer is then cast
// to an AudioHeader in order to fill in the appropriate data. If the compiler adds
// padding to the struct, this will then screw the whole thing up.
typedef struct SongcastAudioHeader
{
    // common header fields
    uint8_t  iSignature[4];
    uint8_t  iVersion;
    uint8_t  iType;
    uint16_t iTotalBytes;
    
    // audio header fields
    uint8_t  iAudioHeaderBytes;
    uint8_t  iAudioFlags;
    uint16_t iAudioSampleCount;
    uint32_t iAudioFrame;
    uint32_t iAudioNetworkTimestamp;
    uint32_t iAudioMediaLatency;
    uint32_t iAudioMediaTimestamp;
    uint64_t iAudioStartSample;
    uint64_t iAudioTotalSamples;
    uint32_t iAudioSampleRate;
    uint32_t iAudioBitRate;
    uint16_t iAudioVolumeOffset;
    uint8_t  iAudioBitDepth;
    uint8_t  iAudioChannels;
    uint8_t  iAudioReserved;
    uint8_t  iAudioCodecNameBytes;
    uint8_t  iAudioCodecName[3];
    
} __attribute__((__packed__)) SongcastAudioHeader;



// Class defining format of audio sent by songcast
class SongcastFormat
{
public:
    SongcastFormat(uint32_t aSampleRate, uint8_t aBitDepth, uint8_t aChannels, uint16_t aSampleCount);

    uint32_t Bytes() const;
    uint64_t TimeNs() const;

    const uint32_t SampleRate;
    const uint32_t BitDepth;
    const uint32_t Channels;
    const uint32_t SampleCount;
};



// Class to wrap the data sent in an audio message
class SongcastAudioMessage
{
public:
    SongcastAudioMessage(uint32_t aMaxAudioBytes, const SongcastFormat& aFormat);
    ~SongcastAudioMessage();
    
    void* Ptr() const { return iPtr; }
    uint32_t Bytes() const;
    uint32_t Frame() const;
    
    void SetHeader(const SongcastFormat& aFormat, uint64_t aTimestampNs, uint64_t aLatencyMs, bool aHalt, uint32_t aFrame);
    void SetResent();
    void SetData(void* aPtr, uint32_t aBytes);
    
private:
    SongcastAudioHeader* Header() const { return (SongcastAudioHeader*)iPtr; }
    
    void* iPtr;
    uint32_t iAudioBytes;
    const uint32_t iMaxAudioBytes;
};



// Class representing the songcast socket
enum ESongcastState
{
    eSongcastStateInactive,
    eSongcastStateActive,
    eSongcastStatePendingInactive
};

class SongcastSocket
{
public:
    SongcastSocket();
    ~SongcastSocket();
    
    void SetEndpoint(uint32_t aIpAddress, uint16_t aPort, uint32_t aAdapter);
    void Send(SongcastAudioMessage& aMsg);
    void SetTtl(uint64_t aTtl);
    
private:
    void SetSocketTtl();

    socket_t iSocket;
    uint8_t iTtl;
};



// Class defining the interface between the audio driver code and the songcast code
class Songcast
{
public:
    Songcast();
    ~Songcast();

    static const uint32_t SupportedFormatCount = 1;
    static const SongcastFormat SupportedFormats[SupportedFormatCount];

    void SetActive(uint64_t aActive);
    void SetEndpoint(uint32_t aIpAddress, uint16_t aPort, uint32_t aAdapter);
    void SetTtl(uint64_t aTtl);
    void SetLatencyMs(uint64_t aLatencyMs);

    void Send(const SongcastFormat& aFormat, uint32_t aFrameNumber, uint64_t aTimestampNs, bool aHalt, void* aData, uint32_t aBytes);
    void Resend(uint64_t aFrameCount, const uint32_t* aFrames);

private:
    static const uint32_t kHistoryCount = 100;

    SongcastSocket iSocket;
    ESongcastState iState;
    OpenHome::FifoLite<SongcastAudioMessage*, kHistoryCount> iHistory;
    uint64_t iLatencyMs;
};



#endif // HEADER_SONGCAST


