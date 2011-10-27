
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
platform_cflags = -DPLATFORM_MACOSX_GNU -arch x86_64 -mmacosx-version-min=10.4
platform_linkflags = -arch x86_64 -framework IOKit -framework CoreFoundation -framework CoreAudio -framework SystemConfiguration
platform_dllflags = -install_name @executable_path/$(@F)
platform_include = -I/System/Library/Frameworks/IOKit.framework/Headers/
osdir = Mac
else
platform_cflags = -Wno-psabi
platform_linkflags = 
platform_dllflags = 
platform_include = 
osdir = Posix
endif


# Macros used by Common.mak

ar = ${CROSS_COMPILE}ar rc $(objdir)
cflags = -fexceptions -Wall -Werror -pipe -D_GNU_SOURCE -D_REENTRANT -DDEFINE_LITTLE_ENDIAN -DDEFINE_TRACE $(debug_specific_flags) -fvisibility=hidden -DDllImport="__attribute__ ((visibility(\"default\")))" -DDllExport="__attribute__ ((visibility(\"default\")))" -DDllExportClass="__attribute__ ((visibility(\"default\")))" $(platform_cflags)
ohnetdir = ../ohnet/Build/Obj/$(osdir)/$(build_dir)/
objdir = Build/Obj/$(osdir)/$(build_dir)/
inc_build = Build/Include/
includes = -I../ohnet/Build/Include/ $(platform_includes)
objext = o
libprefix = lib
libext = a
exeext = elf
compiler = ${CROSS_COMPILE}gcc -fPIC -o $(objdir)
link = ${CROSS_COMPILE}g++ -lpthread $(platform_linkflags)
linkoutput = -o 
dllprefix = lib
dllext = so
link_dll = ${CROSS_COMPILE}g++ -lpthread $(platform_linkflags) $(platform_dllflags) -shared -shared-libgcc --whole-archive
csharp = gmcs /nologo
dirsep = /


default : all

# Include rules to build the shared code
include Common.mak

$(objects_songcast) : | make_obj_dir
make_obj_dir :
	mkdir -p $(objdir)

clean :
	rm -rf $(objdir)

all : all_common $(objdir)$(dllprefix)ohSongcaster.$(dllext) $(objdir)TestSongcaster.$(exeext)


ifeq ($(MACHINE), Darwin)
objects_songcaster_os = $(objdir)SoundcardDriver.$(objext)

$(objdir)SoundcardDriver.$(objext) : ohSongcaster/Mac/SoundcardDriver.cpp
	$(compiler)SoundcardDriver.$(objext) -c $(cflags) $(includes) ohSongcaster/Mac/SoundcardDriver.cpp
else
objects_songcaster_os = $(objdir)SoundcardDriver.$(objext)

$(objdir)SoundcardDriver.$(objext) : ohSongcaster/Posix/SoundcardDriver.cpp
	$(compiler)SoundcardDriver.$(objext) -c $(cflags) $(includes) ohSongcaster/Posix/SoundcardDriver.cpp
endif


$(objdir)$(dllprefix)ohSongcaster.$(dllext) : $(objects_songcast) $(objects_songcaster) $(objects_songcaster_os)
	$(link_dll) $(linkoutput)$(objdir)$(dllprefix)ohSongcaster.$(dllext) $(objects_topology) $(objects_songcast) $(objects_songcaster) $(objects_songcaster_os) $(ohnetdir)$(libprefix)ohNetCore.$(libext)


$(objdir)TestSongcaster.$(exeext) : $(objdir)$(dllprefix)ohSongcaster.$(dllext) ohSongcaster/TestSongcaster.cpp
	$(compiler)TestSongcaster.$(objext) -c $(cflags) $(includes) ohSongcaster/TestSongcaster.cpp
	$(link) $(linkoutput)$(objdir)TestSongcaster.$(exeext) $(objdir)TestSongcaster.$(objext) $(objdir)$(dllprefix)ohSongcaster.$(dllext)


