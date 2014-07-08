SRCROOT = $(abspath $(CURDIR)/../..)
OBJROOT = $(SRCROOT)/obj/i386/libcompiler-rt
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = compiler-rt
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

CPPFLAGS := $(CPPFLAGS) -nostdinc++

INC = -I. -I$(SRCROOT) -I$(SYMROOT) -I$(LIBSADIR) -I$(BOOT2DIR) -I${SRCROOT}/i386/include

COMPILER_RT_OBJS = udivdi3.x86.elf.o umoddi3.x86.elf.o moddi3.x86.elf.o divdi3.x86.elf.o

COMPILER_RT_OBJS := $(addprefix $(OBJROOT)/, $(COMPILER_RT_OBJS))

LIBS  = libcompiler-rt.a
LIBS := $(addprefix $(SYMROOT)/, $(LIBS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)


all: $(DIRS_NEEDED) $(CONFIG_HEADERS) $(LIBS) all-recursive

$(LIBS): $(COMPILER_RT_OBJS)
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
