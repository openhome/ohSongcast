

ifeq ($(release), 1)
debug_specific_flags = -Os
else
debug_specific_flags = -O0 -mfix-and-continue
endif


arch_flags = -arch x86_64
arch_linkflags = -arch x86_64
outputdir = $(objdir)x86_64/Prefs/
contentsdir = $(outputdir)$(product_name).prefPane/Contents/
macosdir = $(outputdir)$(product_name).prefPane/Contents/MacOS/
resourcedir = $(outputdir)$(product_name).prefPane/Contents/Resources/


compiler = gcc
compiler_flags = -fmessage-length=0 -pipe -Wno-trigraphs -fpascal-strings -fasm-blocks $(debug_specific_flags) -Wreturn-type -Wunused-variable -isysroot /Developer/SDKs/MacOSX10.6.sdk -mmacosx-version-min=10.6 -gdwarf-2 -c -fobjc-gc -I../
objc_flags = -x objective-c -std=gnu99 $(compiler_flags)


link = gcc
linkflags1 = -bundle -isysroot /Developer/SDKs/MacOSX10.6.sdk
linkflags2 = -mmacosx-version-min=10.6 -framework Cocoa -framework PreferencePanes

copystrings = /Developer/Library/Xcode/Plug-ins/CoreBuildTasks.xcplugin/Contents/Resources/copystrings
copystrings_flags = --validate --inputencoding utf-8 --outputencoding UTF-16

ibtool = ibtool
ibtool_flags = --errors --warnings --notices --output-format human-readable-text --sdk /Developer/SDKs/MacOSX10.6.sdk


default : make_outputdir $(outputdir)$(product_name).prefPane


make_outputdir :
	mkdir -p $(outputdir)
	mkdir -p $(macosdir)
	mkdir -p $(resourcedir)
	mkdir -p $(resourcedir)English.lproj/


$(outputdir)%.o : %.m
	$(compiler) $(arch_flags) $(objc_flags) $< -o $@

$(outputdir)Preferences.o : ../Preferences.m
	$(compiler) $(arch_flags) $(objc_flags) $< -o $@

objects = $(outputdir)SongcasterPrefPane.o $(outputdir)Preferences.o

$(outputdir)$(product_name) : $(objects)
	$(link) $(arch_linkflags) $(linkflags1) $(objects) $(linkflags2) -o $@

$(resourcedir)%.strings : %.strings
	$(copystrings) $(copystrings_flags) $< --outdir $(@D)

$(resourcedir)%.nib : %.xib
	$(ibtool) $(ibtool_flags) --compile $@ $<

$(outputdir)$(product_name).prefPane : $(outputdir)$(product_name) $(resourcedir)English.lproj/Localizable.strings $(resourcedir)English.lproj/InfoPlist.strings $(resourcedir)English.lproj/SongcasterPrefPane.nib Info.plist Icon.png IconLarge.png green.tiff red.tiff ../BundleIcon.icns
	cp $< $(macosdir)
	cp Icon.png $(resourcedir)
	cp IconLarge.png $(resourcedir)
	cp green.tiff $(resourcedir)
	cp red.tiff $(resourcedir)
	cp Info.plist $(contentsdir)
	cp ../BundleIcon.icns $(resourcedir)Icon.icns


