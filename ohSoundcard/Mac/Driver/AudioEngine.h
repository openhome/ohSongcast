#ifndef HEADER_AUDIOENGINE
#define HEADER_AUDIOENGINE

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/IOTimerEventSource.h>
#include <sys/kpi_socket.h>


// Class to wrap the kernel socket
class AudioSocket
{
public:
    AudioSocket();
    ~AudioSocket();
    
    bool IsOpen() const;
    void Open(uint32_t aIpAddress, uint16_t aPort);
    void Close();
    void Send(void* aBuffer, uint32_t aBytes) const;
    
private:
    socket_t iSocket;
};



// NOTE: This struct is __packed__ - this prevents the compiler from adding
// padding to the struct in order to align data on 4 byte boundaries. This is
// required because, when sending the audio packets a single buffer is allocated
// that holds the header and the audio data. The start of this buffer is then cast
// to an AudioHeader in order to fill in the appropriate data. If the compiler adds
// padding to the struct, this will then screw the whole thing up.
typedef struct AudioHeader
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
    
} __attribute__((__packed__)) AudioHeader;



// Class to wrap the data sent in an audio message
class AudioMessage
{
public:
    AudioMessage(uint32_t aFrames, uint32_t aChannels, uint32_t aBitDepth);
    ~AudioMessage();
    
    void* Ptr() const { return iPtr; }
    uint32_t Bytes() const { return (sizeof(AudioHeader) + iAudioBytes); }
    
    void SetHaltFlag(bool aHalt);
    void SetSampleRate(uint32_t aSampleRate);
    void SetFrame(uint32_t aFrame);
    void SetTimestamp(uint32_t aTimestamp);
    void SetData(void* aPtr, uint32_t aBytes);
    
private:
    AudioHeader* Header() const { return (AudioHeader*)iPtr; }
    
    void* iPtr;
    const uint32_t iAudioBytes;
};



// Class for the audio buffer - audio is sent over the network in blocks of fixed size
class BlockBuffer
{
public:
    BlockBuffer(uint32_t aBlocks, uint32_t aBlockFrames, uint32_t aChannels, uint32_t aBitDepth);
    ~BlockBuffer();

    void* Ptr() const { return iPtr; }
    uint32_t Bytes() const { return iBytes; }
    
    void* BlockPtr(uint32_t aBlockIndex) const { return (uint8_t*)iPtr + (aBlockIndex * iBlockBytes); }
    uint32_t BlockBytes() const { return iBlockBytes; }

    uint32_t Blocks() const { return iBlocks; }
    uint32_t BlockFrames() const { return iBlockFrames; }
    
private:
    void* iPtr;
    const uint32_t iBytes;
    const uint32_t iBlockBytes;
    const uint32_t iBlocks;
    const uint32_t iBlockFrames;
};



// State enum for the audio engine
enum EAudioEngineState
{
    eAudioEngineStateInactive,
    eAudioEngineStateActive,
    eAudioEngineStatePendingInactiveHalt
};


// Main class for the audio engine
class AudioEngine : public IOAudioEngine
{
    OSDeclareDefaultStructors(AudioEngine);

public:
    virtual bool init(OSDictionary* aProperties);
    virtual void free();

    void SetActive(uint64_t aActive);
    void SetInactiveAndHalt();
    void SetEndpoint(uint64_t aIpAddress, uint64_t aPort);
    void SetTtl(uint64_t aTtl);
    
private:
    virtual bool initHardware(IOService* aProvider);
    virtual void stop(IOService* aProvider);

    virtual IOReturn performAudioEngineStart();
    virtual IOReturn performAudioEngineStop();
    virtual UInt32 getCurrentSampleFrame();
    virtual IOReturn performFormatChange(IOAudioStream* aAudioStream, const IOAudioStreamFormat* aNewFormat, const IOAudioSampleRate* aNewSampleRate);
    virtual IOReturn clipOutputSamples(const void* aMixBuffer, void* aSampleBuffer, UInt32 aFirstSampleFrame, UInt32 aNumSampleFrames, const IOAudioStreamFormat* aFormat, IOAudioStream* aStream);

    static void TimerFired(OSObject* aOwner, IOTimerEventSource* aSender);
    void TimerFired();

    uint32_t iCurrentBlock;
    uint32_t iCurrentFrame;
    IOAudioSampleRate iSampleRate;
    uint32_t iTimerIntervalNs;
    uint64_t iTimestamp;

    IOTimerEventSource* iTimer;
    uint64_t iTimeZero;
    uint32_t iTimerFiredCount;
    
    BlockBuffer* iBuffer;
    AudioMessage* iAudioMsg;

    EAudioEngineState iState;
    uint64_t iTtl;
    AudioSocket iSocket;
};


#endif // HEADER_AUDIOENGINE


