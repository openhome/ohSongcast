

ifeq ($(release), 1)
debug_specific_flags = -Os -mdynamic-no-pic -fvisibility=hidden
libohSongcast = ../../../Build/Obj/Mac-x86/Release/libohSongcast.so
else
debug_specific_flags = -O0 -mfix-and-continue
libohSongcast = ../../../Build/Obj/Mac-x86/Debug/libohSongcast.so
endif


ifeq ($(mac-64),1)
    arch_flags = -arch x86_64
    arch_linkflags = -arch x86_64
    outputdir = $(objdir)x86_64/App/
else
    arch_flags = -m32
    arch_linkflags = -m32
    outputdir = $(objdir)i386/App/
endif

contentsdir = $(outputdir)$(product_name).app/Contents/
macosdir = $(outputdir)$(product_name).app/Contents/MacOS/
resourcedir = $(outputdir)$(product_name).app/Contents/Resources/


compiler = gcc
compiler_flags = -fmessage-length=0 -pipe -Wno-trigraphs -fpascal-strings -fasm-blocks $(debug_specific_flags) -Wreturn-type -Wunused-variable -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.6.sdk -mmacosx-version-min=10.6 -gdwarf-2 -I../../../../ohnet/Build/Include -I../../../../ohTopology/build/Include -I../ -c
objc_flags = -x objective-c -std=gnu99 $(compiler_flags)
objcc_flags = -x objective-c++ -fvisibility-inlines-hidden $(compiler_flags)


link = g++
linkflags1 = -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.6.sdk
linkflags2 = -mmacosx-version-min=10.6 -framework Cocoa -framework SystemConfiguration

ibtool = ibtool
ibtool_flags = --errors --warnings --notices --output-format human-readable-text --sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.6.sdk


default : make_outputdir $(outputdir)$(product_name).app


make_outputdir :
	mkdir -p $(outputdir)
	mkdir -p $(macosdir)
	mkdir -p $(resourcedir)
	mkdir -p $(resourcedir)English.lproj/


$(outputdir)%.o : %.m
	$(compiler) $(arch_flags) $(objc_flags) $< -o $@

$(outputdir)%.o : %.mm
	$(compiler) $(arch_flags) $(objcc_flags) $< -o $@

$(outputdir)Preferences.o : ../Preferences.m
	$(compiler) $(arch_flags) $(objc_flags) $< -o $@

objects = $(outputdir)main.o $(outputdir)SongcastAppDelegate.o $(outputdir)WindowUpdates.o $(outputdir)Model.o $(outputdir)ModelSongcast.o $(outputdir)Preferences.o $(outputdir)ReceiverList.o $(outputdir)Receiver.o $(outputdir)SubnetList.o $(outputdir)CrashLogging.o $(outputdir)AutoUpdate.o

$(outputdir)$(product_name) : $(objects)
	$(link) $(arch_linkflags) $(linkflags1) $(objects) $(libohSongcast) $(linkflags2) -o $@

$(resourcedir)%.nib : %.xib
	$(ibtool) $(ibtool_flags) --compile $@ $<

$(outputdir)$(product_name).app : $(outputdir)$(product_name) $(resourcedir)English.lproj/MainMenu.nib $(resourcedir)English.lproj/WindowUpdates.nib Info.plist MenuIconOff.png MenuIconOn.png SenderIcon.png ../BundleIcon.icns $(libohSongcast)
	cp $< $(macosdir)
	cp $(libohSongcast) $(macosdir)
	cp MenuIconOff.png $(resourcedir)
	cp MenuIconOn.png $(resourcedir)
	cp SenderIcon.png $(resourcedir)
	cp ../BundleIcon.icns $(resourcedir)Icon.icns
	cp Info.plist $(contentsdir)


