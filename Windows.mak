# Makefile for Windows
#

!if "$(release)"=="1"
link_flag_debug = 
debug_specific_cflags = /MT /Ox
build_dir = Release
!else
link_flag_debug = /debug
debug_specific_cflags = /MTd /Zi /Od /RTC1
build_dir = Debug
!endif

# Macros used by Common.mak

ar = lib /nologo /out:$(objdir)
cflags = $(debug_specific_cflags) /W4 /WX /EHsc /FR$(objdir) -DDEFINE_LITTLE_ENDIAN -DDEFINE_TRACE -D_CRT_SECURE_NO_WARNINGS 
ohnetdir = ..\ohNet\Build\Obj\Windows\$(build_dir)^\
ohtopologydir = ..\ohTopology\build^\
ohnetmondir = ..\ohNetmon\Build\Obj\Windows\$(build_dir)^\
objdirbare = Build\Obj\Windows\$(build_dir)
objdir = $(objdirbare)^\
inc_build = Build\Include
includes = -I..\ohNet\Build\Include -I..\ohTopology\build\Include
bundle_build = Build\Bundles
osdir = Windows
objext = obj
libprefix = lib
libext = lib
exeext = exe
compiler = cl /nologo /Fo$(objdir)
link = link /nologo $(link_flag_debug) /SUBSYSTEM:CONSOLE /map Ws2_32.lib Iphlpapi.lib Dbghelp.lib /incremental:no
linkoutput = /out:
dllprefix =
dllext = dll
link_dll = link /nologo $(link_flag_debug) /map Ws2_32.lib Iphlpapi.lib Dbghelp.lib /dll
link_dll_service = link /nologo $(link_flag_debug)  /map $(objdir)ohNet.lib Ws2_32.lib Iphlpapi.lib Dbghelp.lib /dll
csplatform = x86
csharp = csc /nologo /platform:$(csplatform) /nostdlib /reference:%SystemRoot%\microsoft.net\framework\v2.0.50727\mscorlib.dll /debug
publiccsdir = Public\Cs^\csharp
dirsep = ^\
installdir = $(PROGRAMFILES)\ohNet
installlibdir = $(installdir)\lib
installincludedir = $(installdir)\include
mkdir = Scripts\mkdir.bat
rmdir = Scripts\rmdir.bat
uset4 = no


all: $(objdir)$(dllprefix)ohSongcast.$(dllext) $(objdir)TestSongcast.$(exeext) all_common


# Include rules to build platform independent code
include Common.mak

$(objects_topology) : make_obj_dir
$(objects_sender) : make_obj_dir
$(objects_songcast) : make_obj_dir

make_obj_dir : $(objdirbare)

$(objdirbare) :
	if not exist $(objdirbare) mkdir $(objdirbare)

clean:
	del /S /Q $(objdirbare)

objects_songcast_dll = $(ohnetdir)$(libprefix)ohNetCore.lib $(objects_topology) $(objects_sender) $(objects_songcast) $(objects_netmon) $(objdir)SoundcardDriver.$(objext) kernel32.lib setupapi.lib shell32.lib ole32.lib

$(objdir)$(dllprefix)ohSongcast.$(dllext) : $(objects_topology) $(objects_sender) $(objects_songcast) $(objects_netmon) ohSongcast\Windows\SoundcardDriver.cpp
	$(compiler)SoundcardDriver.$(objext) -c $(cflags) $(includes) ohSongcast\Windows\SoundcardDriver.cpp
	$(link_dll) $(linkoutput)$(objdir)$(dllprefix)ohSongcast.$(dllext) $(objects_songcast_dll)

$(objdir)TestSongcast.$(exeext) : $(objdir)$(dllprefix)ohSongcast.$(dllext) ohSongcast\TestSongcast.cpp
	$(compiler)TestSongcast.$(objext) -c $(cflags) $(includes) ohSongcast\TestSongcast.cpp
	$(link) $(linkoutput)$(objdir)TestSongcast.$(exeext) $(objdir)TestSongcast.$(objext) $(objects_songcast_dll) $(ohnetdir)$(libprefix)TestFramework.$(libext)

