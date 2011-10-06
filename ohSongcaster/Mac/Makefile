

ifeq ($(release), 1)
build_dir = Release
else
build_dir = Debug
endif


export objdir = $(CURDIR)/Build/Obj/Mac/$(build_dir)/
export product_name = ohSongcaster

packagemaker=/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker 


default : make_objdir_x86_64 make_objdir_i386 make_objdir_ppc make_objdir_universal $(objdir)Installer/$(product_name).dmg


.PHONY : clean
clean :
	rm -rf $(objdir)

make_objdir_x86_64 :
	mkdir -p $(objdir)x86_64/

make_objdir_i386:
	mkdir -p $(objdir)i386/

make_objdir_ppc :
	mkdir -p $(objdir)ppc/

make_objdir_universal :
	mkdir -p $(objdir)Universal/

.PHONY : driver
driver :
	$(MAKE) -C Driver arch=x86_64
	$(MAKE) -C Driver arch=i386
	$(MAKE) -C Driver arch=ppc
	$(MAKE) -C Driver arch=universal

.PHONY : app
app :
	$(MAKE) -C App

.PHONY : prefs
prefs :
	$(MAKE) -C Prefs

$(objdir)Installer/$(product_name).pkg : driver app prefs
	mkdir -p $(objdir)Installer/Dst/System/Library/Extensions/
	mkdir -p $(objdir)Installer/Dst/Library/PreferencePanes/
	mkdir -p $(objdir)Installer/Dst/Library/OpenHome/
	mkdir -p $(objdir)Installer/Dst/Library/LaunchAgents/
	cp -R $(objdir)Universal/Driver/$(product_name).kext $(objdir)Installer/Dst/System/Library/Extensions/
	cp -R $(objdir)x86_64/Prefs/$(product_name).prefPane $(objdir)Installer/Dst/Library/PreferencePanes/
	cp -R $(objdir)x86_64/App/$(product_name).app $(objdir)Installer/Dst/Library/OpenHome/
	cp LaunchAgent.plist  $(objdir)Installer/Dst/Library/LaunchAgents/org.openhome.av.songcaster.plist
	$(packagemaker) --root $(objdir)Installer/Dst --out $@ --scripts Scripts --info PkgInfo.plist --title ohSongcaster --version 1.1.1 --no-relocate

$(objdir)Installer/$(product_name).dmg : $(objdir)Installer/$(product_name).pkg Scripts/uninstall.tool
	mkdir -p $(objdir)Installer/DmgContents/
	cp -R $(objdir)Installer/$(product_name).pkg $(objdir)Installer/DmgContents/
	cp Scripts/uninstall.tool $(objdir)Installer/DmgContents/
	hdiutil create -ov $@ -volname $(product_name) -fs HFS+ -srcfolder $(objdir)Installer/DmgContents/

