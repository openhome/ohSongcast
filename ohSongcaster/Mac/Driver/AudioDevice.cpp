#include "AudioDevice.h"
#include "AudioEngine.h"
#include <IOKit/IOLib.h>


OSDefineMetaClassAndStructors(AudioDevice, IOAudioDevice);


bool AudioDevice::initHardware(IOService* aProvider)
{
    IOLog("Songcaster AudioDevice[%p]::initHardware(%p) ...\n", this, aProvider);

    // initialise base class
    if (!IOAudioDevice::initHardware(aProvider)) {
        IOLog("Songcaster AudioDevice[%p]::initHardware(%p) base class initHardware failed\n", this, aProvider);
        return false;
    }

    // set device names
    setDeviceName(BRANDING_AUDIODEVICE_DEVICENAME);
    setDeviceShortName(BRANDING_AUDIODEVICE_DEVICESHORTNAME);
    setManufacturerName(BRANDING_AUDIODEVICE_MANUFACTURERNAME);

    // create, initialise and activate the audio engine
    AudioEngine* engine = new AudioEngine();
    if (!engine) {
        IOLog("Songcaster AudioDevice[%p]::initHardware(%p) failed to allocated engine\n", this, aProvider);
        return false;
    }

    if (!engine->init(0)) {
        IOLog("Songcaster AudioDevice[%p]::initHardware(%p) failed to initialise engine\n", this, aProvider);
        engine->release();
        return false;
    }

    // create the songcast socket
    iSocket = new SongcastSocket();
    engine->SetSocket(*iSocket);

    if (activateAudioEngine(engine) != kIOReturnSuccess) {
        IOLog("Songcaster AudioDevice[%p]::initHardware(%p) failed to activate engine\n", this, aProvider);
        engine->release();
        return false;
    }

    // the engine must be released as it is retained when passed to activateAudioEngine
    engine->release();

    IOLog("Songcaster AudioDevice[%p]::initHardware(%p) ok\n", this, aProvider);
    return true;
}


void AudioDevice::free()
{
    IOLog("Songcaster AudioDevice[%p]::free()\n", this);

    // close the kernel socket
    iSocket->Close();
    delete iSocket;
    iSocket = 0;
    
    IOAudioDevice::free();
}





