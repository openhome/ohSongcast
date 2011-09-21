#ifndef HEADER_AUDIODEVICE
#define HEADER_AUDIODEVICE

#include <IOKit/audio/IOAudioDevice.h>
#include "Branding.h"
#include "Songcast.h"


#define AudioDevice BRANDING_AUDIODEVICE_CLASS


class AudioDevice : public IOAudioDevice
{
    OSDeclareDefaultStructors(AudioDevice);

    virtual bool initHardware(IOService* aProvider);
    virtual void free();

public:
    SongcastSocket& Socket() { return iSocket; }

private:
    SongcastSocket iSocket;
};


#endif // HEADER_AUDIODEVICE



