#include "AudioDevice.h"
#include "AudioEngine.h"
#include <IOKit/IOLib.h>


OSDefineMetaClassAndStructors(AudioDevice, IOAudioDevice);


bool AudioDevice::initHardware(IOService* aProvider)
{
    IOLog("Songcast AudioDevice[%p]::initHardware(%p) ...\n", this, aProvider);

    // initialise base class
    if (!IOAudioDevice::initHardware(aProvider)) {
        IOLog("Songcast AudioDevice[%p]::initHardware(%p) base class initHardware failed\n", this, aProvider);
        return false;
    }

    // set device names
    setDeviceName(BRANDING_AUDIODEVICE_NAME);
    setDeviceShortName(BRANDING_AUDIODEVICE_SHORTNAME); 
    setManufacturerName(BRANDING_AUDIODEVICE_MANUFACTURERNAME);

    // create, initialise and activate the audio engine
    iEngine = new AudioEngine();
    if (!iEngine) {
        IOLog("Songcast AudioDevice[%p]::initHardware(%p) failed to allocated engine\n", this, aProvider);
        return false;
    }

    if (!iEngine->init(0)) {
        IOLog("Songcast AudioDevice[%p]::initHardware(%p) failed to initialise engine\n", this, aProvider);
        iEngine->release();
        iEngine = 0;
        return false;
    }

    // create the songcast socket
    iSocket = new SongcastSocket();
    iEngine->SetSocket(*iSocket);
    iEngine->SetDescription(BRANDING_AUDIODEVICE_NAME);

    if (activateAudioEngine(iEngine) != kIOReturnSuccess) {
        IOLog("Songcast AudioDevice[%p]::initHardware(%p) failed to activate engine\n", this, aProvider);
        iEngine->release();
        iEngine = 0;
        return false;
    }

    IOLog("Songcast AudioDevice[%p]::initHardware(%p) ok\n", this, aProvider);
    return true;
}


void AudioDevice::free()
{
    IOLog("Songcast AudioDevice[%p]::free()\n", this);

    // close the kernel socket
    if (iSocket) {
        iSocket->Close();
        delete iSocket;
        iSocket = 0;
    }

    // release the engine
    if (iEngine) {
        iEngine->release();
        iEngine = 0;
    }
    
    IOAudioDevice::free();
}





