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
objdirbare = Build\Obj\Windows\$(build_dir)
objdir = $(objdirbare)^\
inc_build = Build\Include
includes = -I..\ohNet\Build\Include
bundle_build = Build\Bundles
osdir = Windows
objext = obj
libprefix = lib
libext = lib
exeext = exe
compiler = cl /nologo /Fo$(objdir)
link = link /nologo $(link_flag_debug) /SUBSYSTEM:CONSOLE /map Ws2_32.lib Iphlpapi.lib /incremental:no
linkoutput = /out:
dllprefix =
dllext = dll
link_dll = link /nologo $(link_flag_debug) /map Ws2_32.lib Iphlpapi.lib /dll
link_dll_service = link /nologo $(link_flag_debug)  /map $(objdir)ohNet.lib Ws2_32.lib Iphlpapi.lib /dll
csplatform = x86
csharp = csc /nologo /platform:$(csplatform) /nostdlib /reference:%SystemRoot%\microsoft.net\framework\v2.0.50727\mscorlib.dll
publiccsdir = Public\Cs^\csharp
dirsep = ^\
installdir = $(PROGRAMFILES)\ohNet
installlibdir = $(installdir)\lib
installincludedir = $(installdir)\include
mkdir = Scripts\mkdir.bat
rmdir = Scripts\rmdir.bat
uset4 = no


all: $(objdir)$(dllprefix)ohSongcaster.$(dllext) $(objdir)$(dllprefix)ohSongcaster.net.$(dllext) $(objdir)TestSongcaster.$(exeext) $(objdir)TestSongcasterCs.$(exeext) all_common


# Include rules to build platform independent code
include Common.mak

$(objects_songcast) : make_obj_dir

make_obj_dir : $(objdirbare)

$(objdirbare) :
	if not exist $(objdirbare) mkdir $(objdirbare)

clean:
	del /S /Q $(objdirbare)


$(objdir)$(dllprefix)ohSongcaster.$(dllext) : $(objects_songcast) $(objects_songcaster) ohSongcaster\Windows\SoundcardDriver.cpp
	$(compiler)SoundcardDriver.$(objext) -c $(cflags) $(includes) ohSongcaster\Windows\SoundcardDriver.cpp
	$(link_dll) $(linkoutput)$(objdir)$(dllprefix)ohSongcaster.$(dllext) $(ohnetdir)$(libprefix)ohNetCore.lib $(objects_topology) $(objects_songcast) $(objects_songcaster) $(objdir)SoundcardDriver.$(objext) kernel32.lib setupapi.lib shell32.lib ole32.lib

$(objdir)$(dllprefix)ohSongcaster.net.$(dllext) : $(objdir)$(dllprefix)ohSongcaster.$(dllext) ohSongcaster\Windows\Songcaster.cs
	$(csharp) /unsafe /t:library \
		/out:$(objdir)$(dllprefix)ohSongcaster.net.$(dllext) \
		ohSongcaster\Windows\Songcaster.cs

$(objdir)TestSongcaster.$(exeext) : $(objdir)$(dllprefix)ohSongcaster.$(dllext) ohSongcaster\TestSongcaster.cpp
	$(compiler)TestSongcaster.$(objext) -c $(cflags) $(includes) ohSongcaster\TestSongcaster.cpp
	$(link) $(linkoutput)$(objdir)TestSongcaster.$(exeext) $(objdir)ohSongcaster.$(libext) $(objdir)TestSongcaster.$(objext) 

$(objdir)TestSongcasterCs.$(exeext) : $(objdir)$(dllprefix)ohSongcaster.net.$(dllext) ohSongcaster\Windows\TestSongcasterCs.cs
	$(csharp) /target:exe /debug+ \
		/out:$(objdir)TestSongcasterCs.$(exeext) \
		/reference:System.dll \
		/reference:System.Net.dll \
		/reference:$(objdir)$(dllprefix)ohSongcaster.net.$(dllext)  \
		ohSongcaster\Windows\TestSongcasterCs.cs
