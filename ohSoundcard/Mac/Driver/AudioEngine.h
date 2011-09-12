#ifndef HEADER_AUDIOENGINE
#define HEADER_AUDIOENGINE

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/IOTimerEventSource.h>
#include "Songcast.h"



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
class AudioEngine : public IOAudioEngine
{
    OSDeclareDefaultStructors(AudioEngine);

public:
    virtual bool init(OSDictionary* aProperties);
    virtual void free();
    
    void SetSocket(ISongcastSocket& aSocket);

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
    bool iAudioStopping;
    
    BlockBuffer* iBuffer;
    SongcastAudioMessage* iAudioMsg;

    ISongcastSocket* iSocket;
};


#endif // HEADER_AUDIOENGINE


