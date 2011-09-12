

# Variable to define the name of the product - this will change the name of the
# build kext etc...
product_name=ohSoundcard


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

else ifeq ($(arch), ppc)
arch_flags = -arch ppc -mtune=G5
arch_linkflags = -arch ppc -mtune=G5
outputdir = $(objdir)ppc/Driver/

else ifeq ($(arch), universal)
outputdir = $(objdir)Universal/Driver/

else
$(error Unknown architecture $(arch) allowed values (x86_64, i386, ppc, universal))
endif


compiler = gcc
compiler_flags = -c -fmessage-length=0 -pipe -nostdinc -Wno-trigraphs -fasm-blocks -force_cpusubtype_ALL $(debug_specific_flags) -finline -fno-keep-inline-functions -Wreturn-type -Wunused-variable -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT -isysroot /Developer/SDKs/MacOSX10.6.sdk -mmacosx-version-min=10.6 -gdwarf-2 -I/System/Library/Frameworks/Kernel.framework/PrivateHeaders -I/Developer/SDKs/MacOSX10.6.sdk/System/Library/Frameworks/Kernel.framework/Headers
kernel_cc_flags = -x c++ $(compiler_flags) -msoft-float -fno-builtin -fno-common -mkernel -fno-exceptions -fno-rtti -fcheck-new -fapple-kext
kernel_c_flags = -x c -std=gnu99 $(compiler_flags) -msoft-float -fno-builtin -fno-common -mkernel

link = g++
linkflags1 = -isysroot /Developer/SDKs/MacOSX10.6.sdk
linkflags2 = -mmacosx-version-min=10.6 -lcpp_kext -Xlinker -kext -nostdlib -lkmodc++
linkflags3 = -lkmod -lcc_kext


default : make_outputdir $(outputdir)$(product_name).kext


make_outputdir :
	mkdir -p $(outputdir)


ifeq ($(arch), universal)

# Rule for building the universal binary from the x86_64, i386 and ppc binaries

$(outputdir)$(product_name) : $(objdir)x86_64/Driver/$(product_name) $(objdir)i386/Driver/$(product_name) $(objdir)ppc/Driver/$(product_name)
	lipo -create $^ -output $@

else

# Rules for building the code files for x86_64, i386 and ppc architectures

$(outputdir)%.o : %.cpp
	$(compiler) $(arch_flags) $(kernel_cc_flags) $< -o $@

$(outputdir)%.o : %.c
	$(compiler) $(arch_flags) $(kernel_c_flags) $< -o $@

objects = $(outputdir)AudioDevice.o $(outputdir)AudioEngine.o $(outputdir)AudioUserClient.o $(outputdir)AudioClip.o $(outputdir)Songcast.o

$(outputdir)$(product_name) : $(objects) $(outputdir)KextInfo.o
	$(link) $(arch_linkflags) $(linkflags1) $(objects) $(linkflags2) $(outputdir)KextInfo.o $(linkflags3) -o $@

endif


$(outputdir)$(product_name).kext : $(outputdir)$(product_name) Info.plist
	mkdir -p $(outputdir)$(product_name).kext/Contents/MacOs/
	cp $< $(outputdir)$(product_name).kext/Contents/MacOs/
	cp Info.plist $(outputdir)$(product_name).kext/Contents/



