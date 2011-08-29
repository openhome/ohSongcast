

ifeq ($(release), 1)
debug_specific_flags = -Os
else
debug_specific_flags = -O0
endif


compiler = gcc
compiler_flags = -c -fmessage-length=0 -pipe -nostdinc -Wno-trigraphs -fasm-blocks -force_cpusubtype_ALL $(debug_specific_flags) -finline -fno-keep-inline-functions -Wreturn-type -Wunused-variable -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT -isysroot /Developer/SDKs/MacOSX10.6.sdk -mmacosx-version-min=10.6 -gdwarf-2 -I/System/Library/Frameworks/Kernel.framework/PrivateHeaders -I/Developer/SDKs/MacOSX10.6.sdk/System/Library/Frameworks/Kernel.framework/Headers
kernel_cc_flags = -x c++ $(compiler_flags) -msoft-float -fno-builtin -fno-common -mkernel -fno-exceptions -fno-rtti -fcheck-new -fapple-kext
kernel_c_flags = -x c -std=gnu99 $(compiler_flags) -msoft-float -fno-builtin -fno-common -mkernel
x86_64flags = -arch x86_64
i386flags = -arch i386
ppcflags = -arch ppc -mtune=G5


link = g++
linkflags1 = -isysroot /Developer/SDKs/MacOSX10.6.sdk
linkflags2 = -mmacosx-version-min=10.6 -lcpp_kext -Xlinker -kext -nostdlib -lkmodc++
linkflags3 = -lkmod -lcc_kext
x86_64linkflags = -arch x86_64
i386linkflags = -arch i386
ppclinkflags = -arch ppc


default : $(objdir)ohSoundcard.kext


$(objdir_x86_64)%.o : %.cpp
	$(compiler) $(x86_64flags) $(kernel_cc_flags) $< -o $@

$(objdir_i386)%.o : %.cpp
	$(compiler) $(i386flags) $(kernel_cc_flags) $< -o $@

$(objdir_ppc)%.o : %.cpp
	$(compiler) $(ppcflags) $(kernel_cc_flags) $< -o $@

$(objdir_x86_64)%.o : %.c
	$(compiler) $(x86_64flags) $(kernel_c_flags) $< -o $@

$(objdir_i386)%.o : %.c
	$(compiler) $(i386flags) $(kernel_c_flags) $< -o $@

$(objdir_ppc)%.o : %.c
	$(compiler) $(ppcflags) $(kernel_c_flags) $< -o $@


objects_x86_64 = $(objdir_x86_64)AudioDevice.o $(objdir_x86_64)AudioEngine.o $(objdir_x86_64)AudioUserClient.o $(objdir_x86_64)AudioClip.o

objects_i386 = $(objdir_i386)AudioDevice.o $(objdir_i386)AudioEngine.o $(objdir_i386)AudioUserClient.o $(objdir_i386)AudioClip.o

objects_ppc = $(objdir_ppc)AudioDevice.o $(objdir_ppc)AudioEngine.o $(objdir_ppc)AudioUserClient.o $(objdir_ppc)AudioClip.o


$(objdir_x86_64)ohSoundcard : $(objects_x86_64) $(objdir_x86_64)ohSoundcard_info.o
	$(link) $(x86_64linkflags) $(linkflags1) $(objects_x86_64) $(linkflags2) $(objdir_x86_64)ohSoundcard_info.o $(linkflags3) -o $@

$(objdir_i386)ohSoundcard : $(objects_i386) $(objdir_i386)ohSoundcard_info.o
	$(link) $(i386linkflags) $(linkflags1) $(objects_i386) $(linkflags2) $(objdir_i386)ohSoundcard_info.o $(linkflags3) -o $@

$(objdir_ppc)ohSoundcard : $(objects_ppc) $(objdir_ppc)ohSoundcard_info.o
	$(link) $(ppclinkflags) $(linkflags1) $(objects_ppc) $(linkflags2) $(objdir_ppc)ohSoundcard_info.o $(linkflags3) -o $@


$(objdir)ohSoundcard : $(objdir_x86_64)ohSoundcard $(objdir_i386)ohSoundcard $(objdir_ppc)ohSoundcard
	lipo -create $(objdir_x86_64)ohSoundcard $(objdir_i386)ohSoundcard $(objdir_ppc)ohSoundcard -output $@

$(objdir)ohSoundcard.kext : $(objdir)ohSoundcard Info.plist
	mkdir -p $(objdir)ohSoundcard.kext/Contents/MacOs/
	cp $< $(objdir)ohSoundcard.kext/Contents/MacOs/
	cp Info.plist $(objdir)ohSoundcard.kext/Contents/




