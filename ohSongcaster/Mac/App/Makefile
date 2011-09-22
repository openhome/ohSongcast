

ifeq ($(release), 1)
debug_specific_flags = -Os -mdynamic-no-pic -fvisibility=hidden
libohSongcaster = ../../../Build/Obj/Mac/Release/libohSongcaster.so
else
debug_specific_flags = -O0 -mfix-and-continue
libohSongcaster = ../../../Build/Obj/Mac/Debug/libohSongcaster.so
endif


arch_flags = -arch x86_64
arch_linkflags = -arch x86_64
outputdir = $(objdir)x86_64/App/
contentsdir = $(outputdir)$(product_name).app/Contents/
macosdir = $(outputdir)$(product_name).app/Contents/MacOS/
resourcedir = $(outputdir)$(product_name).app/Contents/Resources/


compiler = gcc
compiler_flags = -fmessage-length=0 -pipe -Wno-trigraphs -fpascal-strings -fasm-blocks $(debug_specific_flags) -Wreturn-type -Wunused-variable -isysroot /Developer/SDKs/MacOSX10.6.sdk -mmacosx-version-min=10.6 -gdwarf-2 -I../../../../ohnet/Build/Include -I../ -c
objc_flags = -x objective-c -std=gnu99 $(compiler_flags)
objcc_flags = -x objective-c++ -fvisibility-inlines-hidden $(compiler_flags)


link = g++
linkflags1 = -isysroot /Developer/SDKs/MacOSX10.6.sdk
linkflags2 = -mmacosx-version-min=10.6 -framework Cocoa

copystrings = /Developer/Library/Xcode/Plug-ins/CoreBuildTasks.xcplugin/Contents/Resources/copystrings
copystrings_flags = --validate --inputencoding utf-8 --outputencoding UTF-16

ibtool = ibtool
ibtool_flags = --errors --warnings --notices --output-format human-readable-text --sdk /Developer/SDKs/MacOSX10.6.sdk


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

objects = $(outputdir)main.o $(outputdir)SongcasterAppDelegate.o $(outputdir)Model.o $(outputdir)Preferences.o $(outputdir)ReceiverList.o $(outputdir)Receiver.o

$(outputdir)$(product_name) : $(objects)
	$(link) $(arch_linkflags) $(linkflags1) $(objects) $(libohSongcaster) $(linkflags2) -o $@

$(resourcedir)%.strings : %.strings
	$(copystrings) $(copystrings_flags) $< --outdir $(@D)

$(resourcedir)%.nib : %.xib
	$(ibtool) $(ibtool_flags) --compile $@ $<

$(outputdir)$(product_name).app : $(outputdir)$(product_name) $(resourcedir)NonLocalizable.strings $(resourcedir)English.lproj/Localizable.strings $(resourcedir)English.lproj/InfoPlist.strings $(resourcedir)English.lproj/MainMenu.nib Info.plist MenuIconOff.png MenuIconOn.png Icon.icns $(libohSongcaster)
	cp $< $(macosdir)
	cp $(libohSongcaster) $(macosdir)
	cp MenuIconOff.png $(resourcedir)
	cp MenuIconOn.png $(resourcedir)
	cp Icon.icns $(resourcedir)
	cp Info.plist $(contentsdir)


