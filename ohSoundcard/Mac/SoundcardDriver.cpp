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

    static void DriverFound(void* aPtr, io_iterator_t aIterator);
    void DriverFound();
    void FindAudioDevice();

    AudioDeviceID iDeviceSoundcard;
    AudioDeviceID iDevicePrevious;

    io_service_t iService;
    io_iterator_t iNotification;
    IONotificationPortRef iNotificationPort;

    TBool iEnabled;
    Endpoint iEndpoint;
    TBool iActive;
    TUint iTtl;

    class Driver
    {
    public:
        Driver(io_service_t aService);
        ~Driver();

        void SetEndpoint(const Endpoint& aEndpoint);
        void SetActive(TBool aValue);
        void SetTtl(TUint aValue);

    private:
        io_connect_t iHandle;
    };

    Driver* iDriver;
};


} // namespace Net
} // namespace OpenHome


EXCEPTION(SoundcardError);

using namespace OpenHome;
using namespace OpenHome::Net;


OhmSenderDriverMac::OhmSenderDriverMac()
    : iDeviceSoundcard(0)
    , iDevicePrevious(0)
    , iService(0)
    , iNotification(0)
    , iNotificationPort(0)
    , iEnabled(false)
    , iEndpoint()
    , iActive(false)
    , iTtl(4)
    , iDriver(0)
{
    // register for notifications of the driver becoming available
    iNotificationPort = IONotificationPortCreate(kIOMasterPortDefault);
    CFRunLoopSourceRef notificationSource = IONotificationPortGetRunLoopSource(iNotificationPort);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), notificationSource, kCFRunLoopDefaultMode);

    IOServiceAddMatchingNotification(iNotificationPort,
                                     kIOFirstMatchNotification,
                                     IOServiceMatching(AudioDeviceName),
                                     DriverFound, this,
                                     &iNotification);

    // need to empty the notification iterator to arm the notification
    io_object_t obj;
    while ((obj = IOIteratorNext(iNotification))) {
        IOObjectRelease(obj);
    }

    // find the service for the driver
    iService = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching(AudioDeviceName));
    if (iService != 0)
    {
        FindAudioDevice();

        // notification no longer required
        IONotificationPortDestroy(iNotificationPort);
        iNotificationPort = 0;
    }
}

void OhmSenderDriverMac::DriverFound(void* aPtr, io_iterator_t aIterator)
{
    ((OhmSenderDriverMac*)aPtr)->DriverFound();
}

void OhmSenderDriverMac::DriverFound()
{
    if (iService)
        return;

    // get the IOService for the driver
    iService = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching(AudioDeviceName));

    try
    {
        // lookup the AudioDevice for the driver
        FindAudioDevice();
    }
    catch (SoundcardError)
    {
        iService = 0;
        return;
    }

    // set the state of the driver
    SetEnabled(iEnabled);

    // notifications are no longer required
    IONotificationPortDestroy(iNotificationPort);
    iNotificationPort = 0;
}

void OhmSenderDriverMac::FindAudioDevice()
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
}


// IOhmSenderDriver
void OhmSenderDriverMac::SetEnabled(TBool aValue)
{
    iEnabled = aValue;

    // return early if the IOService for the device is not available yet
    if (!iService)
        return;

    UInt32 propBytes = sizeof(AudioDeviceID);

    if (!iDriver && aValue)
    {
        // create the internal driver instance
        iDriver = new Driver(iService);

        // set the current state of the driver
        iDriver->SetEndpoint(iEndpoint);
        iDriver->SetTtl(iTtl);
        iDriver->SetActive(iActive);

        // change the current audio output device to be the ohSoundcard driver
        AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propBytes, &iDevicePrevious);
        AudioHardwareSetProperty(kAudioHardwarePropertyDefaultOutputDevice, propBytes, &iDeviceSoundcard);
    }
    else if (iDriver && !aValue)
    {
        // change the current audio output device to be what it was previously
        AudioHardwareSetProperty(kAudioHardwarePropertyDefaultOutputDevice, propBytes, &iDevicePrevious);

        // make sure the driver stops sending data
        iDriver->SetActive(false);

        // delete the internal driver instance
        delete iDriver;
        iDriver = 0;
    }
}

void OhmSenderDriverMac::SetEndpoint(const Endpoint& aEndpoint)
{
    iEndpoint = aEndpoint;

    if (iDriver) {
        iDriver->SetEndpoint(aEndpoint);
    }
}

void OhmSenderDriverMac::SetActive(TBool aValue)
{
    iActive = aValue;

    if (iDriver) {
        iDriver->SetActive(aValue);
    }
}

void OhmSenderDriverMac::SetTtl(TUint aValue)
{
    iTtl = aValue;

    if (iDriver) {
        iDriver->SetTtl(aValue);
    }
}

void OhmSenderDriverMac::SetTrackPosition(TUint64 aSampleStart, TUint64 aSamplesTotal)
{
}



// Implementation of internal Driver class

OhmSenderDriverMac::Driver::Driver(io_service_t aService)
    : iHandle(0)
{
    // open a connection to communicate with the service
    kern_return_t res = IOServiceOpen(aService, mach_task_self(), 0, &iHandle);
    if (res != KERN_SUCCESS) {
        THROW(SoundcardError);
    }

    // open the "hardware" device
    IOConnectCallScalarMethod(iHandle, eOpen, 0, 0, 0, 0);
}

OhmSenderDriverMac::Driver::~Driver()
{
    // close the connection and the handle to the driver
    IOConnectCallScalarMethod(iHandle, eClose, 0, 0, 0, 0);
    IOServiceClose(iHandle);
}

void OhmSenderDriverMac::Driver::SetEndpoint(const Endpoint& aEndpoint)
{
    uint64_t args[2];
    args[0] = aEndpoint.Address();
    args[1] = aEndpoint.Port();
    IOConnectCallScalarMethod(iHandle, eSetEndpoint, args, 2, 0, 0);
}

void OhmSenderDriverMac::Driver::SetActive(TBool aValue)
{
    uint64_t arg = aValue ? 1 : 0;
    IOConnectCallScalarMethod(iHandle, eSetActive, &arg, 1, 0, 0);
}

void OhmSenderDriverMac::Driver::SetTtl(TUint aValue)
{
    uint64_t arg = aValue;
    IOConnectCallScalarMethod(iHandle, eSetTtl, &arg, 1, 0, 0);
}



// Platform specific parts of the C interface

THandle SoundcardCreateOpenHome(uint32_t aSubnet, uint32_t aChannel, uint32_t aTtl, uint32_t aMulticast, uint32_t aEnabled, uint32_t aPreset, ReceiverCallback aReceiverCallback, void* aReceiverPtr, SubnetCallback aSubnetCallback, void* aSubnetPtr)
{
    const char* ohSoundcardId = NULL;
    return SoundcardCreate(ohSoundcardId, aSubnet, aChannel, aTtl, aMulticast, aEnabled, aPreset, aReceiverCallback, aReceiverPtr, aSubnetCallback, aSubnetPtr, "OpenHome", "http://www.openhome.org", "http://www.openhome.org");
}


THandle SoundcardCreate(const char* aSoundcardId, uint32_t aSubnet, uint32_t aChannel, uint32_t aTtl, uint32_t aMulticast, uint32_t aEnabled, uint32_t aPreset, ReceiverCallback aReceiverCallback, void* aReceiverPtr, SubnetCallback aSubnetCallback, void* aSubnetPtr, const char* aManufacturer, const char* aManufacturerUrl, const char* aModelUrl)
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

    Soundcard* soundcard = new Soundcard(aSubnet, aChannel, aTtl, aMulticast, aEnabled, aPreset, aReceiverCallback, aReceiverPtr, aSubnetCallback, aSubnetPtr, computer, driver, aManufacturer, aManufacturerUrl, aModelUrl);
    return soundcard;
}





