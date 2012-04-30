

ifeq ($(release), 1)
debug_specific_flags = -Os
else
debug_specific_flags = -O0
endif


ifeq ($(arch), x86_64)
arch_flags = -arch x86_64
arch_linkflags = -arch x86_64
outputdir = $(objdir)x86_64/Driver/

else ifeq ($(arch), i386)
arch_flags = -arch i386
arch_linkflags = -arch i386
outputdir = $(objdir)i386/Driver/

else ifeq ($(arch), universal)
outputdir = $(objdir)Universal/Driver/

else
$(error Unknown architecture $(arch) allowed values (x86_64, i386, universal))
endif



# To rebrand the driver, update the following defines. The Info.plist for the driver also must change
#
# <key>CFBundleExecutable</key> - name of the executable in the kext bundle
# <key>CFBundleName</key>       - same as above
# <key>CFBundleIdentifier</key> - unique ID of the kext, should match BRANDING_AUDIODEVICE_CLASSNAME below
#                                 but with "_" replaced by "."
# 
# In <key>IOKitPersonalities</key>:
#   <key> of only item in the dict should be same as CFBundleName
#   <key>IOUserClientClass</key>  - same as BRANDING_AUDIOUSERCLIENT_CLASSNAME
#   <key>IOMatchCategory</key>    - same as BRANDING_AUDIODEVICE_CLASSNAME
#   <key>IOClass</key>            - same as BRANDING_AUDIODEVICE_CLASSNAME
#   <key>CFBundleIdentifier</key> - same as CFBundleIdentifier, above
#
branding_flags = -D'BRANDING_AUDIODEVICE_CLASSNAME=org_openhome_av_songcast' \
                 -D'BRANDING_AUDIODEVICE_NAME="OpenHome Songcast"' \
                 -D'BRANDING_AUDIODEVICE_SHORTNAME="ohSongcast"' \
                 -D'BRANDING_AUDIODEVICE_MANUFACTURERNAME="www.openhome.org"' \
                 -D'BRANDING_AUDIOENGINE_CLASSNAME=org_openhome_av_songcast_audioengine' \
                 -D'BRANDING_AUDIOUSERCLIENT_CLASSNAME=org_openhome_av_songcast_userclient' \
                 -D'BRANDING_KEXTINFO_KMODNAME=org.openhome.av.songcast' \
                 -D'BRANDING_KEXTINFO_KMODVERSION="1.0"' \


compiler = gcc
compiler_flags = -c -fmessage-length=0 -pipe -nostdinc -Wno-trigraphs -fasm-blocks -force_cpusubtype_ALL $(debug_specific_flags) -finline -fno-keep-inline-functions -Wreturn-type -Wunused-variable -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT -isysroot /Developer/SDKs/MacOSX10.6.sdk -mmacosx-version-min=10.6 -gdwarf-2 -I/System/Library/Frameworks/Kernel.framework/PrivateHeaders -I/Developer/SDKs/MacOSX10.6.sdk/System/Library/Frameworks/Kernel.framework/Headers -I. $(branding_flags)
kernel_cc_flags = -x c++ $(compiler_flags) -msoft-float -fno-builtin -fno-common -mkernel -fno-exceptions -fno-rtti -fcheck-new -fapple-kext
kernel_c_flags = -x c -std=gnu99 $(compiler_flags) -msoft-float -fno-builtin -fno-common -mkernel


# AudioClip.cpp is the only file containing floating point operations - this is compiled with floating point support as recommended in the apple documentation
audio_clip_flags = -x c++ -c -fmessage-length=0 -pipe -Wno-trigraphs -fasm-blocks -O3 -Wreturn-type -Wunused-variable -DKERNEL=1 -DKERNEL_PRIVATE=1 -gdwarf-2 -isysroot /Developer/SDKs/MacOSX10.6.sdk -mmacosx-version-min=10.6 -I/System/Library/Frameworks/Kernel.framework/PrivateHeaders -I/Developer/SDKs/MacOSX10.6.sdk/System/Library/Frameworks/Kernel.framework/Headers -fno-exceptions -fno-rtti -fpascal-strings -fno-schedule-insns -fno-schedule-insns2 -static -findirect-virtual-calls -mlong-branch $(branding_flags)


ar = ar rc

link = g++
linkflags1 = -isysroot /Developer/SDKs/MacOSX10.6.sdk
linkflags2 = -mmacosx-version-min=10.6 -lcpp_kext -Xlinker -kext -nostdlib -lkmodc++ -lkmod -lcc_kext


default : make_outputdir $(outputdir)$(product_name).kext


make_outputdir :
	mkdir -p $(outputdir)


ifeq ($(arch), universal)

# Rule for building the universal binary from the x86_64 and i386 binaries

$(outputdir)$(product_name) : $(objdir)x86_64/Driver/$(product_name) $(objdir)i386/Driver/$(product_name)
	lipo -create $^ -output $@

else

# Rules for building the code files for x86_64 and i386 architectures

$(outputdir)%.o : %.cpp
	$(compiler) $(arch_flags) $(kernel_cc_flags) $< -o $@

$(outputdir)%.o : %.c
	$(compiler) $(arch_flags) $(kernel_c_flags) $< -o $@

$(outputdir)AudioClip.o : AudioClip.cpp
	$(compiler) $(arch_flags) $(audio_clip_flags) $< -o $@

$(outputdir)%.o : OpenHome/%.cpp
	$(compiler) $(arch_flags) $(kernel_cc_flags) $< -o $@

objects = $(outputdir)AudioDevice.o $(outputdir)AudioEngine.o $(outputdir)AudioUserClient.o $(outputdir)AudioClip.o $(outputdir)Songcast.o $(outputdir)KextInfo.o $(outputdir)Fifo.o

$(outputdir)$(product_name) : $(objects)
	$(link) $(arch_linkflags) $(linkflags1) $(objects) $(linkflags2) -o $@

endif


$(outputdir)$(product_name).kext : $(outputdir)$(product_name) Info.plist
	mkdir -p $(outputdir)$(product_name).kext/Contents/MacOS/
	cp $< $(outputdir)$(product_name).kext/Contents/MacOS/
	cp Info.plist $(outputdir)$(product_name).kext/Contents/



