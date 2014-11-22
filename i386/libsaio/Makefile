SRCROOT = $(abspath $(CURDIR)/../..)
OBJROOT = $(SRCROOT)/obj/i386/libsaio
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = libsaio
include ${SRCROOT}/Make.rules

UTILDIR  = ../util
LIBSADIR = ../libsa
BOOT2DIR = ../boot2

DEFINES = -DNOTHING
#DEFINES = -DDEBUG_CPU=1 -DDEBUG_MEM=1 -DDEBUG_SPD=1 -DDEBUG_PCI=1 -DDEBUG_SMBIOS=1

CFLAGS	:= $(CFLAGS) $(RC_CFLAGS) $(MORECPP) \
    -D__ARCHITECTURE__=\"i386\" -DSAIO_INTERNAL_USER \
    -DRCZ_COMPRESSED_FILE_SUPPORT -fno-stack-protector \
    -fno-builtin -static $(OMIT_FRAME_POINTER_CFLAG) -fno-stack-protector \
    -march=pentium4 -msse2 -msoft-float -nostdinc -include $(SRCROOT)/autoconf.h

ifeq (${CONFIG_COVERAGE},y)
CFLAGS += -fprofile-arcs -ftest-coverage -Xclang -coverage-cfg-checksum -Xclang -coverage-no-function-names-in-data -Xclang -coverage-version='407*'
endif

CPPFLAGS := $(CPPFLAGS) -nostdinc++

INC = -I. -I$(SRCROOT) -I$(SYMROOT) -I$(LIBSADIR) -I$(BOOT2DIR) -I${SRCROOT}/i386/include


SAIO_OBJS = table.x86.elf.o asm.x86.elf.o bios.x86.elf.o biosfn.x86.elf.o \
	disk.x86.elf.o cache.x86.elf.o \
	misc.x86.elf.o \
	befs.x86.elf.o freebsd.x86.elf.o openbsd.x86.elf.o \
	vbe.x86.elf.o nbp.x86.elf.o hfs.x86.elf.o hfs_compare.x86.elf.o \
	ntfs.x86.elf.o msdos.x86.elf.o md5c.x86.elf.o \
	sys.x86.elf.o \
	\
	ext2fs.x86.elf.o \
	\
	\
	console.x86.elf.o exfat.x86.elf.o base64-decode.x86.elf.o

SAIO_OBJS := $(addprefix $(OBJROOT)/, $(SAIO_OBJS))

LIBS  = libsaio.a
LIBS := $(addprefix $(SYMROOT)/, $(LIBS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)


all: $(DIRS_NEEDED) $(CONFIG_HEADERS) $(LIBS) all-recursive

$(LIBS): $(SAIO_OBJS)
	@echo "	[RM] $@"
	@rm -f $@
	@echo "	[AR] $(@F)"
	@${AR} q $@ $^ &> /dev/null
	@echo "	[RANLIB] $(@F)"
	@${RANLIB} $(SYMROOT)/$(@F)

clean-local:
	@rm -f $(LIBS) $(SAIO_OBJS)

# dependencies
-include $(OBJROOT)/Makedep
