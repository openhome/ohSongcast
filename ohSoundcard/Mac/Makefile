

ifeq ($(release), 1)
build_dir = Release
else
build_dir = Debug
endif


export objdir = $(CURDIR)/Build/Obj/Mac/$(build_dir)/
export objdir_x86_64 = $(objdir)x86_64/
export objdir_i386= $(objdir)i386/
export objdir_ppc= $(objdir)ppc/


default : make_objdir_x86_64 make_objdir_i386 make_objdir_ppc driver


.PHONY : clean
clean :
	rm -rf $(objdir)

make_objdir_x86_64 :
	mkdir -p $(objdir_x86_64)

make_objdir_i386:
	mkdir -p $(objdir_i386)

make_objdir_ppc :
	mkdir -p $(objdir_ppc)

.PHONY : driver
driver :
	$(MAKE) -C Driver


