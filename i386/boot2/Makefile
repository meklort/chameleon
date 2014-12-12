SRCROOT = $(abspath $(CURDIR)/../..)
BINROOT = $(SRCROOT)/bin
OBJROOT = $(SRCROOT)/obj/i386/boot2
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

VPATH = $(SYMROOT)

#	Makefile for i386 boot program
#	define FLOPPY and SMALL using DEFINES macro as necessary

PNGCRUSH = `which pngcrush`
PNGOPTIONS = -rem cHRM -rem gAMA -rem iCCP -rem sRGB

DIR = boot2
include ${SRCROOT}/Make.rules

OPTIM = -Os
CFLAGS	= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -g -Werror \
		-fno-builtin -DSAIO_INTERNAL_USER -static $(OMIT_FRAME_POINTER_CFLAG) -fno-stack-protector \
		-march=pentium4 -msse2 -msoft-float -nostdinc -include $(SRCROOT)/autoconf.h

ifeq (${CONFIG_COVERAGE},y)
CFLAGS += -fprofile-arcs -ftest-coverage -Xclang -coverage-cfg-checksum -Xclang -coverage-no-function-names-in-data -Xclang -coverage-version='407*'
endif

LDFLAGS = -fno-builtin -static -fno-stack-protector $(OMIT_FRAME_POINTER_CFLAG) -DSAIO_INTERNAL_USER \
		-march=pentium4 -msse2  -Wl,-static -nostdlib -g -msoft-float 

LDFLAGS += -Wl,-T,${abspath linker.cfg}

CPPFLAGS := $(CPPFLAGS) -nostdinc++ -include $(SRCROOT)/autoconf.h

DEFINES=
CONFIG = hd
SYMDIR = $(SYMROOT)
LIBSADIR = $(SRCROOT)/i386/libsa
LIBSAIODIR = $(SRCROOT)/i386/libsaio
THEME = $(CONFIG_EMBEDED_THEME)
THEMEDIR = $(SRCROOT)/artwork/themes/$(THEME)
INC = -I. -I$(SRCROOT) -I$(SYMDIR) -I$(LIBSADIR) -I$(LIBSAIODIR) -I${SRCROOT}/i386/include

LIBS= -L$(SYMDIR) -lsaio -lsa -lklibc -lcompiler-rt
LIBDEP= libsaio.a libsa.a libklibc.a  libcompiler-rt.a

OTHER_FILES =

# The ordering is important;
# boot2.o must be first.
OBJS = boot2.x86.elf.o boot.x86.elf.o \
	options.x86.elf.o mboot.x86.elf.o \
	\
	modules.x86.elf.o \
	modules_support.x86.elf.o 

# boot_modules.x86.elf.o
# button.o browser.o scrollbar.o == NOTYET
OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

UTILDIR = $(SRCROOT)/i386/util
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)


#
# Strings used to retrieve the start location for the Symbols.dylib module
#
SYMBOLS_MODULE = Symbols.dylib

all: $(DIRS_NEEDED) ${SYMROOT}/boot.elf all-recursive

BOOT_DEPS = $(CONFIG_HEADERS) $(SYMROOT)/embedded.h $(OBJS) $(addprefix $(SYMROOT)/, $(LIBDEP))

ifeq (${CONFIG_EMBED_THEME}, y)
BOOT_DEPS += $(SYMROOT)/art.h
endif

${SYMROOT}/boot.elf: ${BOOT_DEPS}
	@${MKDIRS} $(OBJROOT)/../boot2_modules/

	@### First pass
	@## Create Dummy linker file.
	@echo "" > $(SYMROOT)/${SYMBOLS_MODULE}.ld
	@echo "	[LD] boot.elf"
	@#$(X86_ELF_CC) $(LDFLAGS) $(INC)  -MM -M -o $@.Makedep ${filter %.o $.a, $^}
	@cd $(SYMROOT) && $(X86_ELF_CC) $(LDFLAGS) $(INC) -o $@ ${filter %.o %.a, $^}

ifeq (${CONFIG_MODULES}, y)
	@# Generate the Symbols.dylib file
	@echo "	[dyldsymboltool] Symbols.dylib"
	@$(SYMROOT)/dyldsymboltool $@ $(SYMROOT)/${SYMBOLS_MODULE}
	@cat $(SYMROOT)/${SYMBOLS_MODULE} | hexdump -v -e '"BYTE(0x" 1/1 "%02X" ")\n"' > $(SYMROOT)/${SYMBOLS_MODULE}.ld

	@### Second pass
	@echo "	[LD] boot.elf"
	@cd $(SYMROOT) && $(X86_ELF_CC) $(LDFLAGS) $(INC) -o $@ ${filter %.o %.a, $^}
	@rm $(SYMROOT)/${SYMBOLS_MODULE}.ld

endif
	@echo "	[OBJCOPY] boot"
	@/opt/xZenue/bin/objcopy i386 $@ -O binary ${SYMROOT}/boot

$(SYMROOT)/art.h:
	@if [ "$(PNGCRUSH)" ]; then	\
		echo "optimizing art files ...\n$(PNGCRUSH) $(PNGOPTIONS) artwork/$(THEME)"; \
		cd $(THEMEDIR) && find . -name '*.png' -exec $(PNGCRUSH) -q $(PNGOPTIONS) -d $(SYMROOT)/embed {} \;; \
		cp theme.plist $(SYMROOT)/embed; \
	else \
		cp -R $(THEMEDIR) $(SYMROOT)/embed; \
	fi;
	@echo "#ifndef __BOOT2_ART_H" >> $(SYMROOT)/art.h
	@echo "#define __BOOT2_ART_H" >> $(SYMROOT)/art.h
	@cd $(SYMROOT)/embed && find . -name 'theme.plist' -exec cat {} | $(SYMROOT)/bin2h {} >> $(SYMROOT)/art.h \;
	@cd $(SYMROOT)/embed && find . -name '*.png'       -exec cat {} | $(SYMROOT)/bin2h {} >> $(SYMROOT)/art.h \;

	@echo "typedef struct {" >> $(SYMROOT)/art.h
	@echo "	char			name[32];" >> $(SYMROOT)/art.h
	@echo "	unsigned char	*pngdata;" >> $(SYMROOT)/art.h
	@echo "	unsigned int	*length;" >> $(SYMROOT)/art.h
	@echo "} embeddedpng_t;" >> $(SYMROOT)/art.h
	@echo "" >> $(SYMROOT)/art.h
	@echo "embeddedpng_t embeddedImages[] = {" >> $(SYMROOT)/art.h
	@cd $(SYMROOT)/embed && find . -name '*.png' | sort | cut -f 2 -d '/' | cut -f 1 -d '.' | \
		awk '{ printf "	{.name = \"%s\", .pngdata = __%s_png, .length = &__%s_png_len},\n", $$1, $$1, $$1 }' >> $(SYMROOT)/art.h
	@echo "};" >> $(SYMROOT)/art.h

	@echo "#endif /* !__BOOT2_ART_H */"  >> $(SYMROOT)/art.h ;\

$(SYMROOT)/embedded.h:
	@cd $(SRCROOT)/doc && cat BootHelp.txt | $(SYMROOT)/bin2h BootHelp_txt > $(SYMROOT)/embedded.h


#dependencies
-include $(OBJROOT)/Makedep

clean-local:
	@for o in $(SYMROOT)/boot $(SYMROOT)/boot.elf $(SYMROOT)/embedded.h $(OBJROOT)/Symbols_LINKER_ONLY.dylib; do \
		if [ -f "$${o}" ];then echo "	[RM] $${o}"; fi; done
	@if [ -d "$(OBJROOT)" ];then echo "	[RM] all objects in $(OBJROOT)"; fi
	@rm -f $(SYMROOT)/boot $(SYMROOT)/boot.elf $(SYMROOT)/embedded.h $(OBJS)
	@rm -rf $(OBJROOT)
