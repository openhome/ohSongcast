#ifndef HEADER_AUDIODEVICE
#define HEADER_AUDIODEVICE

#include <IOKit/audio/IOAudioDevice.h>
#include "AudioEngine.h"

class Songcast;


// Implementation of the main class for the audio driver.

#define AudioDevice BRANDING_AUDIODEVICE_CLASSNAME

class AudioDevice : public IOAudioDevice
{
    OSDeclareDefaultStructors(AudioDevice);

public:
    Songcast& GetSongcast() { return *iSongcast; }

private:
    virtual bool initHardware(IOService* aProvider);
    virtual void free();

    Songcast* iSongcast;
    AudioEngine* iEngine;
};


#endif // HEADER_AUDIODEVICE




