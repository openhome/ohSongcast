
#include "SongcasterDriver.h"


// Class for the OpenHome Songcaster audio device

class org_openhome_av_songcaster : public AudioDevice
{
    OSDeclareDefaultStructors(org_openhome_av_songcaster);
};

OSDefineMetaClassAndStructors(org_openhome_av_songcaster, AudioDevice);


// Class for the OpenHome Songcaster audio user client

class org_openhome_av_songcaster_userclient : public AudioUserClient
{
    OSDeclareDefaultStructors(org_openhome_av_songcaster_userclient);
};

OSDefineMetaClassAndStructors(org_openhome_av_songcaster_userclient, AudioUserClient);


// Implementation of the audio device info class

const char* AudioDeviceInfo::Name()
{
    return "OpenHome Songcaster";
}

const char* AudioDeviceInfo::ShortName()
{
    return "ohSongcaster";
}

const char* AudioDeviceInfo::ManufacturerName()
{
    return "www.openhome.org";
}


