#include "AudioEngine.h"
#include <IOKit/audio/IOAudioStream.h>
#include <IOKit/IOLib.h>



static const uint32_t BLOCKS = 16;
static const uint32_t BLOCK_FRAMES = 220;
static const uint32_t CHANNELS = 2;
static const uint32_t BIT_DEPTH = 24;


// implementation of the AudioEngine class
OSDefineMetaClassAndStructors(AudioEngine, IOAudioEngine);


bool AudioEngine::init(OSDictionary* aProperties)
{
    IOLog("ohSoundcard AudioEngine[%p]::init(%p) ...\n", this, aProperties);

    if (!IOAudioEngine::init(aProperties)) {
        IOLog("ohSoundcard AudioEngine[%p]::init(%p) base class init failed\n", this, aProperties);
        return false;
    }

    iTimer = 0;
    iCurrentBlock = 0;
    iCurrentFrame = 0;

    iSampleRate.whole = 44100;
    iSampleRate.fraction = 0;
    
    iTimeZero = 0;
    iTimerFiredCount = 0;
    iAudioStopping = false;

    // calculate the timer interval making sure no overflows occur
    uint64_t interval = 1000000000;
    interval *= BLOCK_FRAMES;
    interval /= iSampleRate.whole;    
    iTimerIntervalNs = interval;
    iTimestamp = 0;

    // allocate the output buffers
    iBuffer = new BlockBuffer(BLOCKS, BLOCK_FRAMES, CHANNELS, BIT_DEPTH);
    iAudioMsg = new SongcastAudioMessage(BLOCK_FRAMES, CHANNELS, BIT_DEPTH);

    if (!iBuffer || !iBuffer->Ptr() || !iAudioMsg || !iAudioMsg->Ptr()) {
        IOLog("ohSoundcard AudioEngine[%p]::init(%p) buffer alloc failed\n", this, aProperties);
        if (iBuffer) {
            delete iBuffer;
            iBuffer = 0;
        }
        if (iAudioMsg) {
            delete iAudioMsg;
            iAudioMsg = 0;
        }
        return false;
    }

    IOLog("ohSoundcard AudioEngine[%p]::init(%p) ok\n", this, aProperties);
    return true;
}


bool AudioEngine::initHardware(IOService* aProvider)
{
    IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) ...\n", this, aProvider);

    // base class initialisation
    if (!IOAudioEngine::initHardware(aProvider)) {
        IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) base class init failed\n", this, aProvider);
        return false;
    }
    
    setDescription("OpenHome Songcast Driver");
    setNumSampleFramesPerBuffer(BLOCKS * BLOCK_FRAMES);
    setSampleRate(&iSampleRate);


    // create output stream
    IOAudioStream* outStream = new IOAudioStream;
    if (!outStream) {
        IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) failed to alloc stream\n", this, aProvider);
        return false;
    }

    if (!outStream->initWithAudioEngine(this, kIOAudioStreamDirectionOutput, 1)) {
        IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) failed to init stream\n", this, aProvider);
        outStream->release();
        return false;
    }


    // initialise audio format for the stream
    IOAudioStreamFormat format;
    format.fNumChannels = CHANNELS;
    format.fSampleFormat = kIOAudioStreamSampleFormatLinearPCM;
    format.fNumericRepresentation = kIOAudioStreamNumericRepresentationSignedInt;
    format.fBitDepth = BIT_DEPTH;
    format.fBitWidth = BIT_DEPTH;
    format.fAlignment = kIOAudioStreamAlignmentHighByte;
    format.fByteOrder = kIOAudioStreamByteOrderBigEndian;
    format.fIsMixable = 1;
    format.fDriverTag = 0;

    outStream->addAvailableFormat(&format, &iSampleRate, &iSampleRate);
    outStream->setSampleBuffer(iBuffer->Ptr(), iBuffer->Bytes());

    if (outStream->setFormat(&format) != kIOReturnSuccess) {
        IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) failed to set stream format\n", this, aProvider);
        outStream->release();
        return false;
    }

    if (addAudioStream(outStream) != kIOReturnSuccess) {
        IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) failed to add stream\n", this, aProvider);
        outStream->release();
        return false;
    }

    // stream can be released as the addAudioStream will retain it
    outStream->release();


    // create the timer
    IOWorkLoop* workLoop = getWorkLoop();
    if (!workLoop) {
        IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) failed to get work loop\n", this, aProvider);
        return false;
    }

    iTimer = IOTimerEventSource::timerEventSource(this, TimerFired);
    if (!iTimer) {
        IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) failed to create timer\n", this, aProvider);
        return false;
    }

    if (workLoop->addEventSource(iTimer) != kIOReturnSuccess) {
        IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) failed to add timer\n", this, aProvider);
        return false;
    }

    IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) ok\n", this, aProvider);
    return true;
}


void AudioEngine::free()
{
    IOLog("ohSoundcard AudioEngine[%p]::free()\n", this);

    if (iBuffer) {
        delete iBuffer;
        iBuffer = 0;
    }
    if (iAudioMsg) {
        delete iAudioMsg;
        iAudioMsg = 0;
    }

    IOAudioEngine::free();
}


void AudioEngine::SetSocket(ISongcastSocket& aSocket)
{
    iSocket = &aSocket;
}


void AudioEngine::stop(IOService* aProvider)
{
    IOLog("ohSoundcard AudioEngine[%p]::stop(%p)\n", this, aProvider);
    IOAudioEngine::stop(aProvider);
    
    // remove timer
    iTimer->cancelTimeout();
    IOWorkLoop* workLoop = getWorkLoop();
    if (workLoop) {
        workLoop->removeEventSource(iTimer);
    }
    iTimer->release();
    iTimer = 0;
}


IOReturn AudioEngine::performAudioEngineStart()
{
    takeTimeStamp(false);
    iCurrentBlock = 0;
    iCurrentFrame = 0;

    uint64_t currTime;
    clock_get_uptime(&currTime);
    absolutetime_to_nanoseconds(currTime, &iTimestamp);

    iTimeZero = iTimestamp;
    iTimerFiredCount = 0;
    iAudioStopping = false;

    if (iTimer->setTimeout(iTimerIntervalNs) != kIOReturnSuccess) {
        IOLog("ohSoundcard AudioEngine[%p]::performAudioEngineStart() failed to start timer\n", this);
        return kIOReturnError;
    }
    
    IOLog("ohSoundcard AudioEngine[%p]::performAudioEngineStart() ok\n", this);
    return kIOReturnSuccess;
}


IOReturn AudioEngine::performAudioEngineStop()
{
    // set flag that audio is stopping - let the timer method handle cleanup
    iAudioStopping = true;

    IOLog("ohSoundcard AudioEngine[%p]::performAudioEngineStop()\n", this);
    return kIOReturnSuccess;
}


UInt32 AudioEngine::getCurrentSampleFrame()
{
    return iCurrentBlock * iBuffer->BlockFrames();
}


IOReturn AudioEngine::performFormatChange(IOAudioStream* aAudioStream, const IOAudioStreamFormat* aNewFormat, const IOAudioSampleRate* aNewSampleRate)
{
    IOLog("ohSoundcard AudioEngine[%p]::performFormatChange()\n", this);
    return kIOReturnSuccess;
}


extern IOReturn AudioEngineClipOutputSamples(const void* aMixBuffer, void* aSampleBuffer, UInt32 aFirstSampleFrame, UInt32 aNumSampleFrames, UInt32 aNumChannels, UInt32 aBitDepth);


IOReturn AudioEngine::clipOutputSamples(const void* aMixBuffer, void* aSampleBuffer, UInt32 aFirstSampleFrame, UInt32 aNumSampleFrames, const IOAudioStreamFormat* aFormat, IOAudioStream* aStream)
{
    return AudioEngineClipOutputSamples(aMixBuffer, aSampleBuffer, aFirstSampleFrame, aNumSampleFrames, aFormat->fNumChannels, aFormat->fBitWidth);
}


void AudioEngine::TimerFired(OSObject* aOwner, IOTimerEventSource* aSender)
{
    if (aOwner)
    {
        AudioEngine* engine = OSDynamicCast(AudioEngine, aOwner);
        if (engine) {
            engine->TimerFired();
        }
    }
}


void AudioEngine::TimerFired()
{
    // increment the timer fired count - this is used to calculate accurate timer
    // intervals for when the timer is next scheduled
    iTimerFiredCount++;

    if (!iAudioStopping)
    {
        // calculate the absolute time when the next timer should fire - we calculate
        // this based on the following:
        //  - the absolute iTimeZero (which is the origin of time for this session)
        //  - the expected timer interval (based on the sample rate of the audio and the number of samples to send)
        //  - the number of times that the timer has currently fired
        uint64_t timeOfNextFire = iTimeZero + (iTimerIntervalNs * (iTimerFiredCount + 1));

        // calculate the interval for the next timer and schedule it - this is simply based on the
        // difference between the expected time of the next fire and the current time
        uint64_t currTimeAbs, currTimeNs;
        clock_get_uptime(&currTimeAbs);
        absolutetime_to_nanoseconds(currTimeAbs, &currTimeNs);
        iTimer->setTimeout((uint32_t)(timeOfNextFire - currTimeNs));
    }    
    
    // construct the audio message to send
    void* block = iBuffer->BlockPtr(iCurrentBlock);
    uint32_t blockBytes = iBuffer->BlockBytes();

    // convert the timestamp to the correct units
    uint64_t timestamp = (iTimestamp * iSampleRate.whole * 256) / 1000000000;

    // set the data for the audio message
    iAudioMsg->SetHaltFlag(iAudioStopping);
    iAudioMsg->SetSampleRate(iSampleRate.whole);
    iAudioMsg->SetFrame(iCurrentFrame);
    iAudioMsg->SetTimestamp((uint32_t)timestamp);
    iAudioMsg->SetData(block, blockBytes);    

    
    // increment counters and send timestamp to the upper audio layers if the buffer
    // wraps
    iCurrentBlock++;
    iCurrentFrame++;

    if (iCurrentBlock >= iBuffer->Blocks()) {
        iCurrentBlock = 0;
        takeTimeStamp();
    }

    // get the timestamp to use for the next audio packet
    uint64_t currTimeAbs;
    clock_get_uptime(&currTimeAbs);
    absolutetime_to_nanoseconds(currTimeAbs, &iTimestamp);

    // send the data
    iSocket->Send(*iAudioMsg);
}



// implementation of BlockBuffer
BlockBuffer::BlockBuffer(uint32_t aBlocks, uint32_t aBlockFrames, uint32_t aChannels, uint32_t aBitDepth)
: iPtr(0)
, iBytes(aBlocks * aBlockFrames * aChannels * aBitDepth / 8)
, iBlockBytes(aBlockFrames * aChannels * aBitDepth / 8)
, iBlocks(aBlocks)
, iBlockFrames(aBlockFrames)
{
    iPtr = IOMalloc(iBytes);
}


BlockBuffer::~BlockBuffer()
{
    if (iPtr) {
        IOFree(iPtr, iBytes);
    }
}


