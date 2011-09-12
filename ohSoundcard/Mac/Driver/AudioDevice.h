#ifndef HEADER_AUDIODEVICE
#define HEADER_AUDIODEVICE

#include <IOKit/audio/IOAudioDevice.h>
#include "AudioDeviceInterface.h"
#include "Songcast.h"



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



