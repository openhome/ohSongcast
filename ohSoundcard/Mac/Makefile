

ifeq ($(release), 1)
build_dir = Release
else
build_dir = Debug
endif


export objdir = $(CURDIR)/Build/Obj/Mac/$(build_dir)/


default : make_objdir_x86_64 make_objdir_i386 make_objdir_ppc make_objdir_universal driver app


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



