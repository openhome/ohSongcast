
#ifndef HEADER_BRANDING
#define HEADER_BRANDING


// This file contains information about rebranding the driver.

#define BRANDING_AUDIODEVICE_CLASS org_openhome_ohSoundcard
#define BRANDING_AUDIODEVICE_CLASSNAME "org_openhome_ohSoundcard"
#define BRANDING_AUDIODEVICE_DEVICENAME "OpenHome Songcast Device"
#define BRANDING_AUDIODEVICE_DEVICESHORTNAME "ohSongcastDevice"
#define BRANDING_AUDIODEVICE_MANUFACTURERNAME "OpenHome.org"

#define BRANDING_AUDIOENGINE_DESCRIPTION "OpenHome Songcast Driver"

#define BRANDING_AUDIOUSERCLIENT_CLASS org_openhome_ohSoundcard_userclient

#define BRANDING_KEXTINFO_NAME org.openhome.ohSoundcard
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

// Also, if building with the given Makefile, the "product_name" variable at the top of the
// makefile can be changed to changed the name of kext files that are built


#endif // HEADER_BRANDING



