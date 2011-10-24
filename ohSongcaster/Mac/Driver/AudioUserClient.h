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

    IOReturn DeviceOk();
    IOReturn Open();
    IOReturn Close();
    IOReturn SetActive(uint64_t aActive);
    IOReturn SetEndpoint(uint64_t aIpAddress, uint64_t aPort);
    IOReturn SetTtl(uint64_t aTtl);
    IOReturn SetLatencyMs(uint64_t aLatencyMs);

    friend class AudioUserClientDispatcher;
    AudioDevice* iDevice;
};


#endif // HEADER_AUDIOUSERCLIENT




