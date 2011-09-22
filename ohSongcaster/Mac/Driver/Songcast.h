#ifndef HEADER_SONGCAST
#define HEADER_SONGCAST

#include <sys/kpi_socket.h>


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



// Class to wrap the data sent in an audio message
class SongcastAudioMessage
{
public:
    SongcastAudioMessage(uint32_t aBufferFrames, uint32_t aFrames, uint32_t aChannels, uint32_t aBitDepth);
    ~SongcastAudioMessage();
    
    void* Ptr() const { return iPtr; }
    uint32_t Bytes() const { return (sizeof(SongcastAudioHeader) + iAudioBytes); }
    
    void SetHaltFlag(bool aHalt);
    void SetSampleRate(uint32_t aSampleRate);
    void SetFrame(uint32_t aFrame);
    void SetTimestamp(uint32_t aTimestamp);
    void SetData(void* aPtr, uint32_t aBytes);
    
private:
    SongcastAudioHeader* Header() const { return (SongcastAudioHeader*)iPtr; }
    
    void* iPtr;
    const uint32_t iAudioBytes;
};



// Class representing the songcast socket
class ISongcastSocket
{
public:
    virtual ~ISongcastSocket() {}
    virtual void Send(SongcastAudioMessage& aMsg) = 0;
};

enum ESongcastState
{
    eSongcastStateInactive,
    eSongcastStateActive,
    eSongcastStatePendingInactive
};

class SongcastSocket : public ISongcastSocket
{
public:
    SongcastSocket();
    ~SongcastSocket();
    
    void Open(uint32_t aIpAddress, uint16_t aPort);
    void Close();
    void Send(SongcastAudioMessage& aMsg);
    
    void SetActive(uint64_t aActive);
    void SetTtl(uint64_t aTtl);
    
private:
    void SetSocketTtl();

    socket_t iSocket;
    uint8_t iTtl;
    ESongcastState iState;
};



#endif // HEADER_SONGCAST


