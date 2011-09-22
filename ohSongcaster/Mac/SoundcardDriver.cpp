#include "../Songcaster.h"
#include "Driver/AudioDeviceInterface.h"

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Net/Core/OhNet.h>
#include <OpenHome/Private/Parser.h>

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
    OhmSenderDriverMac(const Brx& aClassName, const Brx& aDriverName);
    virtual ~OhmSenderDriverMac();

    void SetSongcaster(Songcaster& aSongcaster);

private:
    // IOhmSenderDriver
    virtual void SetEnabled(TBool aValue);
    virtual void SetEndpoint(const Endpoint& aEndpoint);
    virtual void SetActive(TBool aValue);
    virtual void SetTtl(TUint aValue);
    virtual void SetTrackPosition(TUint64 aSampleStart, TUint64 aSamplesTotal);

    static void DriverFound(void* aPtr, io_iterator_t aIterator);
    void DriverFound();

    static OSStatus DefaultDeviceChanged(AudioHardwarePropertyID aId, void* aPtr);
    void DefaultDeviceChanged();

    AudioDeviceID iDeviceSongcaster;
    AudioDeviceID iDevicePrevious;

    Songcaster* iSongcaster;

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
    Brhz iDriverClassName;
    Brhz iDriverName;
};


} // namespace Net
} // namespace OpenHome


EXCEPTION(SongcasterError);

using namespace OpenHome;
using namespace OpenHome::Net;


// static class to wrap some of the messy audio hardware functions
class AudioHardware
{
public:
    static AudioDeviceID Device(const Brhz& aDeviceName)
    {
        // get list of all devices
        AudioDeviceID deviceIds[MAX_AUDIO_DEVICES];
        UInt32 deviceCount = MAX_AUDIO_DEVICES;

        OSStatus err = DeviceList(deviceIds, deviceCount);
        if (err != 0) {
            return kAudioDeviceUnknown;
        }

        // search for device with the given name
        CFStringRef deviceName = CFStringCreateWithCString(NULL, aDeviceName.CString(), kCFStringEncodingMacRoman);

        AudioDeviceID found = kAudioDeviceUnknown;

        for (UInt32 i=0 ; i<deviceCount ; i++)
        {
            CFStringRef name;
            UInt32 propBytes = sizeof(CFStringRef);
            err = AudioDeviceGetProperty(deviceIds[i], 0, false, kAudioObjectPropertyName, &propBytes, &name);

            if (err == 0 && CFStringCompare(name, deviceName, 0) == kCFCompareEqualTo)
            {
                found = deviceIds[i];
            }

            CFRelease(name);

            if (found != kAudioDeviceUnknown)
                break;
        }

        CFRelease(deviceName);

        return found;
    }


    static AudioDeviceID CurrentDevice()
    {
        UInt32 propBytes = sizeof(AudioDeviceID);
        AudioDeviceID device;
        OSStatus err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propBytes, &device);
        return (err == 0) ? device : kAudioDeviceUnknown;
    }


    static void SetCurrentDevice(AudioDeviceID aId)
    {
        UInt32 propBytes = sizeof(AudioDeviceID);
        AudioHardwareSetProperty(kAudioHardwarePropertyDefaultOutputDevice, propBytes, &aId);
    }


    static AudioDeviceID FirstNonSongcasterDevice(AudioDeviceID aSongcaster)
    {
        // get list of all devices
        AudioDeviceID deviceIds[MAX_AUDIO_DEVICES];
        UInt32 deviceCount = MAX_AUDIO_DEVICES;

        OSStatus err = DeviceList(deviceIds, deviceCount);
        if (err != 0) {
            return kAudioDeviceUnknown;
        }

        // look for the first output device that is not the songcaster
        for (UInt32 i=0 ; i<deviceCount ; i++)
        {
            if (deviceIds[i] != aSongcaster)
            {
                UInt32 propBytes = 0;
                OSStatus err = AudioDeviceGetPropertyInfo(deviceIds[i], 0, false, kAudioDevicePropertyStreams, &propBytes, 0);
                if (err == 0 && propBytes > 0)
                {
                    return deviceIds[i];
                }
            }
        }

        return kAudioDeviceUnknown;
    }

private:

    static OSStatus DeviceList(AudioDeviceID* aArray, UInt32& aCount)
    {
        UInt32 propBytes = aCount * sizeof(AudioDeviceID);

        OSStatus ret = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &propBytes, aArray);

        if (ret == 0) {
            aCount = propBytes / sizeof(AudioDeviceID);
        }

        return ret;
    }

    static const int MAX_AUDIO_DEVICES = 32;
};


// OhmSenderDriverMac implementation

OhmSenderDriverMac::OhmSenderDriverMac(const Brx& aClassName, const Brx& aDriverName)
    : iDeviceSongcaster(kAudioDeviceUnknown)
    , iDevicePrevious(kAudioDeviceUnknown)
    , iSongcaster(0)
    , iService(0)
    , iNotification(0)
    , iNotificationPort(0)
    , iEnabled(false)
    , iEndpoint()
    , iActive(false)
    , iTtl(4)
    , iDriver(0)
    , iDriverClassName(aClassName)
    , iDriverName(aDriverName)
{
    // register for notifications of the driver becoming available
    iNotificationPort = IONotificationPortCreate(kIOMasterPortDefault);
    CFRunLoopSourceRef notificationSource = IONotificationPortGetRunLoopSource(iNotificationPort);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), notificationSource, kCFRunLoopDefaultMode);

    IOServiceAddMatchingNotification(iNotificationPort,
                                     kIOFirstMatchNotification,
                                     IOServiceMatching(iDriverClassName.CString()),
                                     DriverFound, this,
                                     &iNotification);

    // need to empty the notification iterator to arm the notification
    io_object_t obj;
    while ((obj = IOIteratorNext(iNotification))) {
        IOObjectRelease(obj);
    }

    // register for notification of default device changes
    AudioHardwareAddPropertyListener(kAudioHardwarePropertyDefaultOutputDevice, DefaultDeviceChanged, this);

    // find the service for the driver
    iService = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching(iDriverClassName.CString()));
    if (iService != 0)
    {
        iDeviceSongcaster = AudioHardware::Device(iDriverName);

        if (iDeviceSongcaster != kAudioDeviceUnknown)
        {
            // notification not required
            IONotificationPortDestroy(iNotificationPort);
            iNotificationPort = 0;
        }
        else
        {
            // device not yet found
            iService = 0;
        }
    }
}

OhmSenderDriverMac::~OhmSenderDriverMac()
{
    // stop notifications
    AudioHardwareRemovePropertyListener(kAudioHardwarePropertyDefaultOutputDevice, DefaultDeviceChanged);

    if (iNotificationPort)
    {
        IONotificationPortDestroy(iNotificationPort);
        iNotificationPort = 0;
    }
}

void OhmSenderDriverMac::SetSongcaster(Songcaster& aSongcaster)
{
    iSongcaster = &aSongcaster;
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
    iService = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching(iDriverClassName.CString()));

    // get the audio device for the songcaster
    iDeviceSongcaster = AudioHardware::Device(iDriverName);

    if (iDeviceSongcaster != kAudioDeviceUnknown)
    {
        // set the state of the driver
        SetEnabled(iEnabled);
    
        // notifications are no longer required
        IONotificationPortDestroy(iNotificationPort);
        iNotificationPort = 0;
    }
    else
    {
        // device not found
        iService = 0;
    }
}

OSStatus OhmSenderDriverMac::DefaultDeviceChanged(AudioHardwarePropertyID aId, void* aPtr)
{
    if (aId == kAudioHardwarePropertyDefaultOutputDevice)
    {
        ((OhmSenderDriverMac*)aPtr)->DefaultDeviceChanged();
    }
    return 0;
}

void OhmSenderDriverMac::DefaultDeviceChanged()
{
    if (iDeviceSongcaster != kAudioDeviceUnknown)
    {
        AudioDeviceID current = AudioHardware::CurrentDevice();

        iSongcaster->SetEnabled(current == iDeviceSongcaster);
    }
}


// IOhmSenderDriver
void OhmSenderDriverMac::SetEnabled(TBool aValue)
{
    iEnabled = aValue;

    // return early if the IOService for the device is not available yet
    if (!iService)
        return;

    if (aValue)
    {
        // initialise the audio driver with the current state
        if (iDriver)
        {
            iDriver->SetActive(false);
            delete iDriver;
            iDriver = 0;
        }

        iDriver = new Driver(iService);

        // set the current state of the driver
        iDriver->SetEndpoint(iEndpoint);
        iDriver->SetTtl(iTtl);
        iDriver->SetActive(iActive);


        // change the current audio output device to be the songcaster device
        AudioDeviceID current = AudioHardware::CurrentDevice();

        // change the current audio device only if it is not already set
        if (current != iDeviceSongcaster)
        {
            iDevicePrevious = current;

            AudioHardware::SetCurrentDevice(iDeviceSongcaster);
        }
    }
    else
    {
        // change the current audio output device to be what it was previously
        AudioDeviceID current = AudioHardware::CurrentDevice();

        if (current == iDeviceSongcaster)
        {
            if (iDevicePrevious != kAudioDeviceUnknown)
            {
                // reset the audio device to the previous value
                AudioHardware::SetCurrentDevice(iDevicePrevious);
            }
            else
            {
                // the previous audio device was not stored
                AudioDeviceID device = AudioHardware::FirstNonSongcasterDevice(iDeviceSongcaster);

                if (device != kAudioDeviceUnknown)
                {
                    AudioHardware::SetCurrentDevice(device);
                }
            }
        }


        // make the driver inactive
        if (iDriver)
        {
            // make sure the driver stops sending data
            iDriver->SetActive(false);

            // delete the internal driver instance
            delete iDriver;
            iDriver = 0;
        }
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
        THROW(SongcasterError);
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

THandle SongcasterCreate(const char* aDomain, uint32_t aSubnet, uint32_t aChannel, uint32_t aTtl, uint32_t aMulticast, uint32_t aEnabled, uint32_t aPreset, ReceiverCallback aReceiverCallback, void* aReceiverPtr, SubnetCallback aSubnetCallback, void* aSubnetPtr, ConfigurationChangedCallback aConfigurationChangedCallback, void* aConfigurationChangedPtr, const char* aManufacturer, const char* aManufacturerUrl, const char* aModelUrl)
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

    // build the audio device driver name
    Bws<64> driverName;
    driverName.Append(aManufacturer);
    driverName.Append(" Songcaster");

    // build the audio device class name for the driver
    Bws<64> className("songcaster");
    Brn domain(aDomain);
    Parser parser(domain);
    while (!parser.Finished())
    {
        Brn part = parser.Next('.');

        Bws<64> temp(className);
        className.Replace(part);
        className.Append("_");
        className.Append(temp);
    }

    // create the driver
    OhmSenderDriverMac* driver;
    try {
        driver = new OhmSenderDriverMac(className, driverName);
    }
    catch (SongcasterError) {
        return 0;
    }

    Songcaster* songcaster = new Songcaster(aSubnet, aChannel, aTtl, aMulticast, aEnabled, aPreset, aReceiverCallback, aReceiverPtr, aSubnetCallback, aSubnetPtr, aConfigurationChangedCallback, aConfigurationChangedPtr, computer, driver, aManufacturer, aManufacturerUrl, aModelUrl);

    driver->SetSongcaster(*songcaster);

	return songcaster;
}





