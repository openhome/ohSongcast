#include "AudioEngine.h"
#include <IOKit/audio/IOAudioStream.h>
#include <IOKit/IOLib.h>
#include <libkern/OSByteOrder.h>
#include <sys/kpi_socket.h>


// struct for defining the IPv4 socket address
typedef struct SocketAddress
{
	uint8_t	sa_len;
	sa_family_t	sa_family;
    uint16_t port;
    uint32_t addr;
    char zero[8];

} __attribute__((__packed__)) SocketAddress;



// implementation of the AudioEngine class
OSDefineMetaClassAndStructors(AudioEngine, IOAudioEngine);


bool AudioEngine::init(OSDictionary* aProperties)
{
    IOLog("ohSoundcard AudioEngine[%p]::init(%p) ...\n", this, aProperties);

    uint64_t interval;
    uint64_t max32bit = (uint64_t)2 << 32;

    if (!IOAudioEngine::init(aProperties))
        goto Error;

    iTimer = 0;
    iOutputBuffer = 0;
    iOutputBufferBytes = 0;
    iCurrentBlock = 0;
    iNumBlocks = 16;
    iBlockFrames = 128;
    iSampleRate.whole = 44100;
    iSampleRate.fraction = 0;
    iCurrentFrame = 0;

    iActive = false;
    iTtl = 0;

    // calculate the timer interval making sure no overflows occur
    interval = 1000000000;
    interval *= iBlockFrames;
    interval /= iSampleRate.whole;
    if (interval >= max32bit)
        goto Error;
    iTimerIntervalNs = interval;
    
    IOLog("ohSoundcard AudioEngine[%p]::init(%p) ok\n", this, aProperties);
    return true;

Error:
    IOLog("ohSoundcard AudioEngine[%p]::init(%p) fail\n", this, aProperties);
    return false;
}


bool AudioEngine::initHardware(IOService* aProvider)
{
    IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) ...\n", this, aProvider);

    IOAudioStream* outStream = 0;
    IOWorkLoop* workLoop = 0;
    IOAudioStreamFormat format;

    // base class initialisation
    if (!IOAudioEngine::initHardware(aProvider))
        goto Error;

    setDescription("OpenHome Songcast Driver");
    setSampleRate(&iSampleRate);
    setNumSampleFramesPerBuffer(iNumBlocks * iBlockFrames);

    // create output stream
    outStream = new IOAudioStream;
    if (!outStream)
        goto Error;

    if (!outStream->initWithAudioEngine(this, kIOAudioStreamDirectionOutput, 1))
        goto Error;

    // initialise audio format for the stream
    format.fNumChannels = 2;
    format.fSampleFormat = kIOAudioStreamSampleFormatLinearPCM;
    format.fNumericRepresentation = kIOAudioStreamNumericRepresentationSignedInt;
    format.fBitDepth = 24;
    format.fBitWidth = 24;
    format.fAlignment = kIOAudioStreamAlignmentHighByte;
    format.fByteOrder = kIOAudioStreamByteOrderBigEndian;
    format.fIsMixable = 1;
    format.fDriverTag = 0;
    outStream->addAvailableFormat(&format, &iSampleRate, &iSampleRate);

    // allocate the output buffer
    iOutputBufferBytes = iNumBlocks * iBlockFrames * format.fNumChannels * format.fBitWidth / 8;
    iOutputBuffer = (void*)IOMalloc(iOutputBufferBytes);
    if (!iOutputBuffer)
        goto Error;

    outStream->setSampleBuffer(iOutputBuffer, iOutputBufferBytes);
    
    if (outStream->setFormat(&format) != kIOReturnSuccess)
        goto Error;
    
    if (addAudioStream(outStream) != kIOReturnSuccess)
        goto Error;

    outStream->release();

    
    // create the timer
    workLoop = getWorkLoop();
    if (!workLoop)
        goto Error;
    
    iTimer = IOTimerEventSource::timerEventSource(this, TimerFired);
    if (!iTimer)
        goto Error;
    
    if (workLoop->addEventSource(iTimer) != kIOReturnSuccess)
        goto Error;

    IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) ok\n", this, aProvider);
    return true;

Error:
    if (outStream)
        outStream->release();
    if (iOutputBuffer) {
        IOFree(iOutputBuffer, iOutputBufferBytes);
        iOutputBuffer = 0;
        iOutputBufferBytes = 0;
    }
    
    IOLog("ohSoundcard AudioEngine[%p]::initHardware(%p) fail\n", this, aProvider);
    return false;
}


void AudioEngine::free()
{
    IOLog("ohSoundcard AudioEngine[%p]::free()\n", this);
    if (iOutputBuffer) {
        IOFree(iOutputBuffer, iOutputBufferBytes);
        iOutputBuffer = 0;
        iOutputBufferBytes = 0;
    }
    IOAudioEngine::free();
}


void AudioEngine::stop(IOService* aProvider)
{
    IOLog("ohSoundcard AudioEngine[%p]::stop(%p)\n", this, aProvider);
    IOAudioEngine::stop(aProvider);
    
    // make sure kernel socket resource is freed
    iSocket.Close();
}


IOReturn AudioEngine::performAudioEngineStart()
{
    IOLog("ohSoundcard AudioEngine[%p]::performAudioEngineStart() ...\n", this);

    takeTimeStamp(false);
    iCurrentBlock = 0;
    iCurrentFrame = 0;

    if (iTimer->setTimeout(iTimerIntervalNs) != kIOReturnSuccess)
        goto Error;
    
    IOLog("ohSoundcard AudioEngine[%p]::performAudioEngineStart() ok\n", this);
    return kIOReturnSuccess;

Error:
    IOLog("ohSoundcard AudioEngine[%p]::performAudioEngineStart() fail\n", this);
    return kIOReturnSuccess;
}


IOReturn AudioEngine::performAudioEngineStop()
{
    // stop the sending timer
    iTimer->cancelTimeout();

    // close the kernel socket
    iSocket.Close();

    IOLog("ohSoundcard AudioEngine[%p]::performAudioEngineStop()\n", this);
    return kIOReturnSuccess;
}


UInt32 AudioEngine::getCurrentSampleFrame()
{
    return iCurrentBlock * iBlockFrames;
}


IOReturn AudioEngine::performFormatChange(IOAudioStream* aAudioStream, const IOAudioStreamFormat* aNewFormat, const IOAudioSampleRate* aNewSampleRate)
{
    IOLog("ohSoundcard AudioEngine[%p]::performFormatChange()\n", this);
    return kIOReturnSuccess;
}


IOReturn AudioEngine::clipOutputSamples(const void* aMixBuffer, void* aSampleBuffer, UInt32 aFirstSampleFrame, UInt32 aNumSampleFrames, const IOAudioStreamFormat* aFormat, IOAudioStream* aStream)
{
    // The audio in the input buffer is always in floating point format in range [-1,1]
    uint8_t* inBuffer = ((uint8_t*)aMixBuffer) + (aFirstSampleFrame * aFormat->fNumChannels * sizeof(float));
    uint8_t* outBuffer = ((uint8_t*)aSampleBuffer) + (aFirstSampleFrame * aFormat->fNumChannels * aFormat->fBitWidth / 8);

    // convert the floating point audio into integer PCM
    int numSamples = aNumSampleFrames * aFormat->fNumChannels;

    // calculate the maximum integer value for the sample - the output sample
    // will be in the range [-maxVal, maxVal-1]
    int maxVal = 1 << (aFormat->fBitWidth - 1);
    float maxFloatVal = 1.0f - (1.0f / maxVal);
    int outSampleBytes = aFormat->fBitWidth / 8;

    while (numSamples > 0)
    {
        float inSample = *((float*)inBuffer);

        // clamp the input sample
        if (inSample > maxFloatVal) {
            inSample = maxFloatVal;
        }
        else if (inSample < -1.0f) {
            inSample = -1.0f;
        }

        // convert to integer in range [maxVal-1, -maxVal]
        int32_t outSample = (int32_t)(inSample * maxVal);

        // copy the output sample into the buffer in big endian format
        for (int i=0 ; i<outSampleBytes ; i++)
        {
            outBuffer[i] = (outSample >> (8*(outSampleBytes - i - 1))) & 0xff;
        }

        inBuffer += sizeof(float);
        outBuffer += outSampleBytes;
        numSamples--;
    }

    return kIOReturnSuccess;
}


void AudioEngine::TimerFired(OSObject* aOwner, IOTimerEventSource* aSender)
{
    if (aOwner)
    {
        AudioEngine* engine = OSDynamicCast(AudioEngine, aOwner);
        if (engine)
        {
            if (engine->iActive != 0)
            {
                uint8_t channels = 2;
                uint8_t bitDepth = 24;
                uint32_t sampleRate = 44100;

                uint32_t offset = engine->iCurrentBlock * engine->iBlockFrames * channels * bitDepth / 8;
                void* block = (uint8_t*)engine->iOutputBuffer + offset;

                uint32_t audioBytes = engine->iBlockFrames * channels * bitDepth / 8;
                uint32_t frame = ++engine->iCurrentFrame;

                // set the data for the audio message
                AudioMessage* audioMsg = new AudioMessage(engine->iBlockFrames, channels, bitDepth);                
                audioMsg->SetSampleRate(sampleRate);
                audioMsg->SetFrame(frame);
                audioMsg->SetData(block, audioBytes);

                // send data
                if (engine->iSocket.IsOpen())
                {
                    engine->iSocket.Send(audioMsg->Ptr(), audioMsg->Bytes());
                }

                delete audioMsg;
            }
            
            engine->iCurrentBlock++;
            if (engine->iCurrentBlock >= engine->iNumBlocks) {
                engine->iCurrentBlock = 0;
                engine->takeTimeStamp();
            }
            aSender->setTimeout(engine->iTimerIntervalNs);
        }
    }
}


void AudioEngine::SetActive(uint64_t aActive)
{
    IOLog("ohSoundcard AudioEngine[%p]::SetActive(%llu)\n", this, aActive);
    iActive = aActive;
}


void AudioEngine::SetEndpoint(uint64_t aIpAddress, uint64_t aPort)
{
    iSocket.Close();
    iSocket.Open(aIpAddress, aPort);
}


void AudioEngine::SetTtl(uint64_t aTtl)
{
    IOLog("ohSoundcard AudioEngine[%p]::SetTtl(%llu)\n", this, aTtl);
    iTtl = aTtl;
}



// implementation of AudioSocket class
AudioSocket::AudioSocket()
: iSocket(0)
{
}


AudioSocket::~AudioSocket()
{
}


bool AudioSocket::IsOpen() const
{
    return (iSocket != 0);
}


void AudioSocket::Open(uint32_t aIpAddress, uint16_t aPort)
{
    // ensure socket is closed
    Close();

    // create the new socket
    errno_t err = sock_socket(PF_INET, SOCK_DGRAM, 0, NULL, NULL, &iSocket);
    if (err != 0) {
        IOLog("ohSoundcard AudioSocket[%p]::Open(0x%x, %u) sock_socket failed with %d\n", this, aIpAddress, aPort, err);
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
        IOLog("ohSoundcard AudioSocket[%p]::Open(0x%x, %u) sock_connect failed with %d\n", this, aIpAddress, aPort, err);
        sock_close(iSocket);
        iSocket = 0;
        return;
    }
    
    IOLog("ohSoundcard AudioSocket[%p]::Open(0x%x, %u) ok\n", this, aIpAddress, aPort);
}


void AudioSocket::Close()
{
    if (iSocket != 0) {
        sock_close(iSocket);
        iSocket = 0;
        IOLog("ohSoundcard AudioSocket[%p]::Close()\n", this);
    }
}


void AudioSocket::Send(void* aBuffer, uint32_t aBytes) const
{
    if (iSocket == 0) {
        return;
    }

    struct iovec sockdata;
    sockdata.iov_base = aBuffer;
    sockdata.iov_len = aBytes;

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
}



// implementation of AudioMessage class
AudioMessage::AudioMessage(uint32_t aFrames, uint32_t aChannels, uint32_t aBitDepth)
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
    Header()->iAudioMediaLatency = 1000000;
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
    
    // lossless audio with no timestamps
    Header()->iAudioFlags = 2;    
}


AudioMessage::~AudioMessage()
{
    if (iPtr) {
        IOFree(iPtr, Bytes());
    }
}


void AudioMessage::SetSampleRate(uint32_t aSampleRate)
{
    Header()->iAudioSampleRate = OSSwapHostToBigInt32(aSampleRate);
    Header()->iAudioBitRate = OSSwapHostToBigInt32(aSampleRate * Header()->iAudioChannels * Header()->iAudioBitDepth);
}


void AudioMessage::SetFrame(uint32_t aFrame)
{
    Header()->iAudioFrame = OSSwapHostToBigInt32(aFrame);
}


void AudioMessage::SetData(void* aPtr, uint32_t aBytes)
{
    void* audioPtr = (uint8_t*)iPtr + sizeof(AudioHeader);
    
    if (aBytes == iAudioBytes) {
        memcpy(audioPtr, aPtr, iAudioBytes);
    }
    else {
        memset(audioPtr, 0, iAudioBytes);
    }
}



