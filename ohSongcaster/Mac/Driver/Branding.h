
#ifndef HEADER_BRANDING
#define HEADER_BRANDING


// This file contains information about rebranding the driver.

#define BRANDING_AUDIODEVICE_CLASS org_openhome_av_songcaster
#define BRANDING_AUDIODEVICE_CLASSNAME "org_openhome_av_songcaster"
#define BRANDING_AUDIODEVICE_DEVICENAME "OpenHome Songcaster"
#define BRANDING_AUDIODEVICE_DEVICESHORTNAME "ohSongcaster"
#define BRANDING_AUDIODEVICE_MANUFACTURERNAME "www.openhome.org"

#define BRANDING_AUDIOENGINE_DESCRIPTION "OpenHome Songcaster"

#define BRANDING_AUDIOUSERCLIENT_CLASS org_openhome_av_songcaster_userclient

#define BRANDING_KEXTINFO_NAME org.openhome.av.songcaster
#define BRANDING_KEXTINFO_VERSION "1.0"


// In addition to the above, the Info.plist file is required to change. The following
// fields need to be changed:

// <key>CFBundleExecutable</key> - name of the executable in the kext bundle
// <key>CFBundleName</key>       - same as above
// <key>CFBundleIdentifier</key> - unique ID of the kext, should match BRANDING_AUDIODEVICE_CLASS
//                                 but with "_" replaced by "."
// 
// In <key>IOKitPersonalities</key>:
//   <key> of only item in the dict should be same as CFBundleName
//   <key>IOUserClientClass</key>  - same as BRANDING_AUDIOUSERCLIENT_CLASS
//   <key>IOMatchCategory</key>    - same as BRANDING_AUDIODEVICE_CLASS
//   <key>IOClass</key>            - same as BRANDING_AUDIODEVICE_CLASS
//   <key>CFBundleIdentifier</key> - same as CFBundleIdentifier, above


#endif // HEADER_BRANDING



