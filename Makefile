
# Makefile for linux and mac


ifeq ($(release), 1)
debug_specific_flags = -O2
build_dir = Release
else
debug_specific_flags = -g -O0
build_dir = Debug
endif


MACHINE = $(shell uname -s)
ifeq ($(MACHINE), Darwin)

ifeq ($(mac-64),1)
    platform_cflags = -DPLATFORM_MACOSX_GNU -arch x86_64 -mmacosx-version-min=10.7
    platform_linkflags = -arch x86_64 -framework IOKit -framework CoreFoundation -framework CoreAudio -framework SystemConfiguration
    osdir = Mac-x64
else
    platform_cflags = -DPLATFORM_MACOSX_GNU -m32 -mmacosx-version-min=10.7
    platform_linkflags = -m32 -framework IOKit -framework CoreFoundation -framework CoreAudio -framework SystemConfiguration
    osdir = Mac-x86
endif

platform_dllflags = -install_name @executable_path/$(@F)
platform_include = -I/System/Library/Frameworks/IOKit.framework/Headers/
else
platform_cflags = -Wno-psabi
platform_linkflags = 
platform_dllflags = 
platform_include = 
osdir = Posix
endif


# Macros used by Common.mak

ar = ${CROSS_COMPILE}ar rc $(objdir)
cflags = -fexceptions -Wall -Werror -pipe -std=c++11 -stdlib=libc++ -D_GNU_SOURCE -D_REENTRANT -DDEFINE_LITTLE_ENDIAN -DDEFINE_TRACE $(debug_specific_flags) -fvisibility=hidden -DDllImport="__attribute__ ((visibility(\"default\")))" -DDllExport="__attribute__ ((visibility(\"default\")))" -DDllExportClass="__attribute__ ((visibility(\"default\")))" $(platform_cflags)
ohnetdir = ../ohNet/Build/Obj/$(osdir)/$(build_dir)/
ohnetgenerateddir = ../ohNetGenerated/Build/Obj/$(osdir)/$(build_dir)/
ohtopologydir = ../ohTopology/build/
ohnetmondir = ../ohNetmon/build/
objdir = Build/Obj/$(osdir)/$(build_dir)/
inc_build = Build/Include/
includes = -I../ohNet/Build/Include/ -I../ohNetGenerated/Build/Include/ -I$(ohtopologydir)Include/ $(platform_includes)
objext = o
libprefix = lib
libext = a
exeext = elf
compiler = ${CROSS_COMPILE}g++ -fPIC -o $(objdir)
link = ${CROSS_COMPILE}g++ -pthread $(platform_linkflags)
linkoutput = -o 
dllprefix = lib
dllext = so
link_dll = ${CROSS_COMPILE}g++ -pthread $(platform_linkflags) $(platform_dllflags) -shared -shared-libgcc
csharp = dmcs /nologo
dirsep = /
copyfile = cp -f


default : all

# Include rules to build the shared code
include Common.mak

$(objects_topology) : | make_obj_dir
$(objects_sender) : | make_obj_dir
$(objects_songcast) : | make_obj_dir
$(objects_driver) : | make_obj_dir

make_obj_dir :
	mkdir -p $(objdir)

clean :
	rm -rf $(objdir)

ifeq ($(nativeonly), yes)
    all_common : all_common_native
else
    all_common : all_common_native all_common_cs
endif
all : all_common $(objdir)$(dllprefix)ohSongcast.$(dllext) $(objdir)TestSongcast.$(exeext)


ifeq ($(MACHINE), Darwin)
objects_driver = $(objdir)SoundcardDriver.$(objext)

$(objdir)SoundcardDriver.$(objext) : ohSongcast/Mac/SoundcardDriver.cpp
	$(compiler)SoundcardDriver.$(objext) -c $(cflags) $(includes) ohSongcast/Mac/SoundcardDriver.cpp
else
objects_driver = $(objdir)SoundcardDriver.$(objext)

$(objdir)SoundcardDriver.$(objext) : ohSongcast/Posix/SoundcardDriver.cpp
	$(compiler)SoundcardDriver.$(objext) -c $(cflags) $(includes) ohSongcast/Posix/SoundcardDriver.cpp
endif

objects_songcast_dll =$(objects_topology) $(objects_sender) $(objects_songcast) $(objects_driver) $(ohnetdir)$(libprefix)ohNetCore.$(libext)

$(objdir)$(dllprefix)ohSongcast.$(dllext) : $(objects_topology) $(objects_sender) $(objects_songcast) $(objects_driver)
	$(link_dll) $(linkoutput)$(objdir)$(dllprefix)ohSongcast.$(dllext) $(objects_songcast_dll)


$(objdir)TestSongcast.$(exeext) : $(objdir)$(dllprefix)ohSongcast.$(dllext) ohSongcast/TestSongcast.cpp
	$(compiler)TestSongcast.$(objext) -c $(cflags) $(includes) ohSongcast/TestSongcast.cpp
	$(link) $(linkoutput)$(objdir)TestSongcast.$(exeext) $(objdir)TestSongcast.$(objext) $(objects_songcast_dll) $(ohnetdir)$(libprefix)TestFramework.$(libext)


