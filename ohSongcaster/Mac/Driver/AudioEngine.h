#ifndef HEADER_AUDIOENGINE
#define HEADER_AUDIOENGINE

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/IOTimerEventSource.h>
#include "Songcast.h"
#include "Branding.h"



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



// Main class for the audio engine

#define AudioEngine BRANDING_AUDIOENGINE_CLASSNAME

class AudioEngine : public IOAudioEngine
{
    OSDeclareDefaultStructors(AudioEngine);

public:
    virtual bool init(OSDictionary* aProperties);
    virtual void free();
    
    void SetSocket(ISongcastSocket& aSocket);
    void SetDescription(const char* aDescription);
    void SetLatencyMs(uint64_t aLatencyMs);

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

    // all timer related integers are 64-bit to avoid conversion errors between 32 and 64 bit
    IOTimerEventSource* iTimer;
    uint64_t iTimeZero;
    uint64_t iTimerFiredCount;
    uint64_t iTimerIntervalNs;
    uint64_t iTimestamp;
    bool iAudioStopping;
    uint64_t iLatencyMs;
    
    BlockBuffer* iBuffer;
    SongcastAudioMessage* iAudioMsg;
    ISongcastSocket* iSocket;
};


#endif // HEADER_AUDIOENGINE


