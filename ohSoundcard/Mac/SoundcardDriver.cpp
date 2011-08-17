#include "../Soundcard.h"
#include "Driver/AudioDeviceInterface.h"

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Net/Core/OhNet.h>

#include "../../Ohm.h"
#include "../../OhmSender.h"

#include <sys/utsname.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/CoreAudio.h>


namespace OpenHome {
namespace Net {

class OhmSenderDriverMac : public IOhmSenderDriver
{
public:
    OhmSenderDriverMac();

private:
    // IOhmSenderDriver
    virtual void SetEnabled(TBool aValue);
    virtual void SetEndpoint(const Endpoint& aEndpoint);
    virtual void SetActive(TBool aValue);
    virtual void SetTtl(TUint aValue);
    virtual void SetTrackPosition(TUint64 aSampleStart, TUint64 aSamplesTotal);

    AudioDeviceID iDeviceSoundcard;
    AudioDeviceID iDevicePrevious;

    io_service_t iDriver;
    io_connect_t iHandle;
    TBool iHandleOpen;

    Endpoint iEndpoint;
    TBool iActive;
    TUint iTtl;
};


} // namespace Net
} // namespace OpenHome


EXCEPTION(SoundcardError);

using namespace OpenHome;
using namespace OpenHome::Net;


OhmSenderDriverMac::OhmSenderDriverMac()
    : iHandleOpen(false)
    , iEndpoint()
    , iActive(false)
    , iTtl(4)
{
    // get the list of audio devices
    const int MAX_AUDIO_DEVICES = 32;
    AudioDeviceID deviceIds[MAX_AUDIO_DEVICES];
    UInt32 propBytes = MAX_AUDIO_DEVICES * sizeof(AudioDeviceID);
    if (AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &propBytes, &deviceIds) != 0) {
        THROW(SoundcardError);
    }
    UInt32 deviceCount = propBytes / sizeof(AudioDeviceID);


    // look for the ohSoundcard device
    CFStringRef ohDriverName = CFStringCreateWithCString(NULL, "OpenHome Songcast Driver", kCFStringEncodingMacRoman);

    bool found = false;
    for (UInt32 i=0 ; i<deviceCount ; i++)
    {
        // get the name of the device
        CFStringRef name;
        propBytes = sizeof(CFStringRef);
        AudioDeviceGetProperty(deviceIds[i], 0, false, kAudioObjectPropertyName, &propBytes, &name);

        found = (CFStringCompare(name, ohDriverName, 0) == kCFCompareEqualTo);
        CFRelease(name);

        if (found)
        {
            iDeviceSoundcard = deviceIds[i];
            break;
        }
    }

    // clean up
    CFRelease(ohDriverName);

    if (!found) {
        THROW(SoundcardError);
    }


    // find the service for the driver
    iDriver = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching(AudioDeviceName));
    if (iDriver == 0) {
        THROW(SoundcardError);
    }
}

// IOhmSenderDriver
void OhmSenderDriverMac::SetEnabled(TBool aValue)
{
    if ((iHandleOpen && aValue) || (!iHandleOpen && !aValue)) {
        return;
    }

    UInt32 propBytes = sizeof(AudioDeviceID);
    kern_return_t res;

    if (aValue)
    {
        // get a handle to the driver
        res = IOServiceOpen(iDriver, mach_task_self(), 0, &iHandle);
        if (res != KERN_SUCCESS) {
            THROW(SoundcardError);
        }

        // open - is this necessary?
        IOConnectCallScalarMethod(iHandle, eOpen, 0, 0, 0, 0);
        SetEndpoint(iEndpoint);
        SetTtl(iTtl);
        SetActive(iActive);

        // change the current audio output device to be the ohSoundcard driver
        AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propBytes, &iDevicePrevious);
        AudioHardwareSetProperty(kAudioHardwarePropertyDefaultOutputDevice, propBytes, &iDeviceSoundcard);

        iHandleOpen = true;
    }
    else
    {
        // change the current audio output device to be what it was previously
        AudioHardwareSetProperty(kAudioHardwarePropertyDefaultOutputDevice, propBytes, &iDevicePrevious);

        // close the handle to the driver
        IOServiceClose(iHandle);

        iHandleOpen = false;
    }
}

void OhmSenderDriverMac::SetEndpoint(const Endpoint& aEndpoint)
{
    iEndpoint = aEndpoint;

    if (iHandleOpen)
    {
        uint64_t args[2];
        args[0] = aEndpoint.Address();
        args[1] = aEndpoint.Port();
        IOConnectCallScalarMethod(iHandle, eSetEndpoint, args, 2, 0, 0);
    }
}

void OhmSenderDriverMac::SetActive(TBool aValue)
{
    iActive = aValue;

    if (iHandleOpen)
    {
        uint64_t arg = aValue ? 1 : 0;
        IOConnectCallScalarMethod(iHandle, eSetActive, &arg, 1, 0, 0);
    }
}

void OhmSenderDriverMac::SetTtl(TUint aValue)
{
    iTtl = aValue;

    if (iHandleOpen)
    {
    }
}

void OhmSenderDriverMac::SetTrackPosition(TUint64 aSampleStart, TUint64 aSamplesTotal)
{
}


// Soundcard - platform specific implementation of OpenHome::Net::Soundcard

Soundcard* Soundcard::Create(TIpAddress aSubnet, TUint aChannel, TUint aTtl, TBool aMulticast, TBool aEnabled, TUint aPreset, ReceiverCallback aReceiverCallback, void* aReceiverPtr, SubnetCallback aSubnetCallback, void* aSubnetPtr)
{
    // get the computer name
    struct utsname name;
    if (uname(&name) < 0)
        return 0;

    // strip off the ".local" from the end
    Brn computer(name.nodename);
    Brn local(".local");
    if (computer.Bytes() > local.Bytes())
    {
        Brn end = computer.Split(computer.Bytes() - local.Bytes());
        if (Ascii::CaseInsensitiveEquals(end, local))
        {
            computer.Set(computer.Ptr(), computer.Bytes() - local.Bytes());
        }
    }

    // create the driver
    OhmSenderDriverMac* driver;
    try {
        driver = new OhmSenderDriverMac();
    }
    catch (SoundcardError) {
        return 0;
    }

    Soundcard* soundcard = new Soundcard(aSubnet, aChannel, aTtl, aMulticast, aEnabled, aPreset, aReceiverCallback, aReceiverPtr, aSubnetCallback, aSubnetPtr, computer, driver);
    return soundcard;
}




