#ifndef HEADER_SONGCASTERDRIVER
#define HEADER_SONGCASTERDRIVER

#include <IOKit/audio/IOAudioDevice.h>
#include <IOKit/IOUserClient.h>

class SongcastSocket;

// To rebrand, the Info.plist file is required to change. The following
// fields need to be changed:

// <key>CFBundleExecutable</key> - name of the executable in the kext bundle
// <key>CFBundleName</key>       - same as above
// <key>CFBundleIdentifier</key> - unique ID of the kext, should match BRANDING_AUDIODEVICE_CLASS
//                                 but with "_" replaced by "."
// 
// In <key>IOKitPersonalities</key>:
//   <key> of only item in the dict should be same as CFBundleName
//   <key>IOUserClientClass</key>  - same as BRANDING_AUDIOUSERCLIENT_CLASS
//   <key>IOMatchCategory</key>    - same as BRANDING_AUDIODEVICE_CLASS
//   <key>IOClass</key>            - same as BRANDING_AUDIODEVICE_CLASS
//   <key>CFBundleIdentifier</key> - same as CFBundleIdentifier, above


// Class description for driver info

class AudioDeviceInfo
{
public:
    static const char* Name();
    static const char* ShortName();
    static const char* ManufacturerName();
};


// Implementation of the main class for the audio driver.

class AudioDevice : public IOAudioDevice
{
    OSDeclareDefaultStructors(AudioDevice);

public:
    SongcastSocket& Socket() { return *iSocket; }

private:
    virtual bool initHardware(IOService* aProvider);
    virtual void free();

    SongcastSocket* iSocket;
};


// Implementation of the user client class for the audio driver.

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

    friend class AudioUserClientDispatcher;
    AudioDevice* iDevice;
};


#endif // HEADER_SONGCASTERDRIVER




