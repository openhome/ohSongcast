

ifeq ($(release), 1)
build_dir = Release
else
build_dir = Debug
endif


export objdir = $(CURDIR)/Build/Obj/Mac/$(build_dir)/


default : make_objdir_x86_64 make_objdir_i386 make_objdir_ppc make_objdir_universal $(objdir)Installer/ohSoundcard.pkg


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

$(objdir)Installer/ohSoundcard.pkg : driver app prefs
	mkdir -p $(objdir)Installer/Dst/System/Library/Extensions/
	mkdir -p $(objdir)Installer/Dst/Library/PreferencePanes/
	mkdir -p $(objdir)Installer/Dst/Library/OpenHome/
	mkdir -p $(objdir)Installer/Dst/Library/LaunchAgents/
	cp -R $(objdir)Universal/Driver/ohSoundcard.kext $(objdir)Installer/Dst/System/Library/Extensions/
	cp -R $(objdir)x86_64/Prefs/ohSoundcard.prefPane $(objdir)Installer/Dst/Library/PreferencePanes/
	cp -R $(objdir)x86_64/App/ohSoundcard.app $(objdir)Installer/Dst/Library/OpenHome/
	cp LaunchAgent.plist  $(objdir)Installer/Dst/Library/LaunchAgents/org.openhome.ohSoundcard.plist
	pkgbuild --identifier org.openhome.ohSoundcard --install-location / --root $(objdir)Installer/Dst --component-plist PkgComponent.plist --scripts Scripts $@


