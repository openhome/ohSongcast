#include "AudioDevice.h"
#include "AudioEngine.h"
#include <IOKit/IOLib.h>


OSDefineMetaClassAndStructors(AudioDevice, IOAudioDevice);


bool AudioDevice::initHardware(IOService* aProvider)
{
    IOLog("ohSoundcard AudioDevice[%p]::initHardware(%p) ...\n", this, aProvider);

    // initialise base class
    if (!IOAudioDevice::initHardware(aProvider)) {
        IOLog("ohSoundcard AudioDevice[%p]::initHardware(%p) base class initHardware failed\n", this, aProvider);
        return false;
    }

    // set device names
    setDeviceName("OpenHome Songcast Device");
    setDeviceShortName("ohSongcastDevice");
    setManufacturerName("OpenHome.org");

    // create, initialise and activate the audio engine
    IOAudioEngine* engine = new AudioEngine();
    if (!engine) {
        IOLog("ohSoundcard AudioDevice[%p]::initHardware(%p) failed to allocated engine\n", this, aProvider);
        return false;
    }

    if (!engine->init(0)) {
        IOLog("ohSoundcard AudioDevice[%p]::initHardware(%p) failed to initialise engine\n", this, aProvider);
        engine->release();
        return false;
    }

    if (activateAudioEngine(engine) != kIOReturnSuccess) {
        IOLog("ohSoundcard AudioDevice[%p]::initHardware(%p) failed to activate engine\n", this, aProvider);
        engine->release();
        return false;
    }

    // the engine must be released as it is retained when passed to activateAudioEngine
    engine->release();

    IOLog("ohSoundcard AudioDevice[%p]::initHardware(%p) ok\n", this, aProvider);
    return true;
}



