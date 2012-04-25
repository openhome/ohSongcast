#ifndef HEADER_AUDIOUSERCLIENT
#define HEADER_AUDIOUSERCLIENT

#include <IOKit/audio/IOAudioDevice.h>
#include <IOKit/IOUserClient.h>
#include "AudioDevice.h"


// Implementation of the user client class for the audio driver.

#define AudioUserClient BRANDING_AUDIOUSERCLIENT_CLASSNAME

class AudioUserClient : public IOUserClient
{
    OSDeclareDefaultStructors(AudioUserClient);

private:
    virtual bool start(IOService* aProvider);
    virtual void stop(IOService* aProvider);
    virtual IOReturn clientClose();
    virtual IOReturn clientDied();
    virtual IOReturn externalMethod(uint32_t aSelector, IOExternalMethodArguments* aArgs, IOExternalMethodDispatch* aDispatch, OSObject* aTarget, void* aReference);

    typedef IOReturn (*Action)(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3);
    IOReturn DeviceOk();
    IOReturn RunAction(Action aAction, void* aArg1 = 0, void* aArg2 = 0, void* aArg3 = 0);
    static IOReturn ActionCommandGate(OSObject* aOwner, void* aArg1, void* aArg2, void* aArg3, void* aArg4);

    IOReturn Open();
    IOReturn Close();
    IOReturn SetActive(uint64_t aActive);
    IOReturn SetEndpoint(uint64_t aIpAddress, uint64_t aPort, uint64_t aAdapter);
    IOReturn SetTtl(uint64_t aTtl);
    IOReturn SetLatencyMs(uint64_t aLatencyMs);
    IOReturn Resend(uint64_t aFrameCount, const uint32_t* aFrames);

    static IOReturn ActionOpen(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3);
    static IOReturn ActionClose(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3);
    static IOReturn ActionSetActive(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3);
    static IOReturn ActionSetEndpoint(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3);
    static IOReturn ActionSetTtl(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3);
    static IOReturn ActionSetLatencyMs(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3);
    static IOReturn ActionResend(AudioUserClient* aClient, void* aArg1, void* aArg2, void* aArg3);

    IOReturn DoOpen();
    IOReturn DoClose();
    IOReturn DoSetActive(uint64_t aActive);
    IOReturn DoSetEndpoint(uint64_t aIpAddress, uint64_t aPort, uint64_t aAdapter);
    IOReturn DoSetTtl(uint64_t aTtl);
    IOReturn DoSetLatencyMs(uint64_t aLatencyMs);
    IOReturn DoResend(uint64_t aFrameCount, const uint32_t* aFrames);

    friend class AudioUserClientDispatcher;
    AudioDevice* iDevice;
    IOWorkLoop* iWorkLoop;
    IOCommandGate* iCommandGate;
};


#endif // HEADER_AUDIOUSERCLIENT




