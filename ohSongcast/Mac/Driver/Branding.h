#ifndef HEADER_BRANDING
#define HEADER_BRANDING


#define BRANDING_AUDIODEVICE_CLASSNAME org_openhome_av_songcaster
#define BRANDING_AUDIODEVICE_NAME "OpenHome Songcaster"
#define BRANDING_AUDIODEVICE_SHORTNAME "ohSongcaster"
#define BRANDING_AUDIODEVICE_MANUFACTURERNAME "www.openhome.org"

#define BRANDING_AUDIOENGINE_CLASSNAME org_openhome_av_songcaster_audioengine

#define BRANDING_AUDIOUSERCLIENT_CLASSNAME org_openhome_av_songcaster_userclient

#define BRANDING_KEXTINFO_KMODNAME org.openhome.av.songcaster
#define BRANDING_KEXTINFO_KMODVERSION "1.0"

// To rebrand, the Info.plist file is required to change. The following
// fields need to be changed:

// <key>CFBundleExecutable</key> - name of the executable in the kext bundle
// <key>CFBundleName</key>       - same as above
// <key>CFBundleIdentifier</key> - unique ID of the kext, should match BRANDING_AUDIODEVICE_CLASSNAME
//                                 but with "_" replaced by "."
// 
// In <key>IOKitPersonalities</key>:
//   <key> of only item in the dict should be same as CFBundleName
//   <key>IOUserClientClass</key>  - same as BRANDING_AUDIOUSERCLIENT_CLASSNAME
//   <key>IOMatchCategory</key>    - same as BRANDING_AUDIODEVICE_CLASSNAME
//   <key>IOClass</key>            - same as BRANDING_AUDIODEVICE_CLASSNAME
//   <key>CFBundleIdentifier</key> - same as CFBundleIdentifier, above


#endif // HEADER_BRANDING


