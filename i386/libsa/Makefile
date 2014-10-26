SRCROOT = $(abspath $(CURDIR)/../..)
OBJROOT = $(SRCROOT)/obj/i386/libsa
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = libsa
include ${SRCROOT}/Make.rules

LIBSAIODIR = ../libsaio

CFLAGS	:= $(CFLAGS) $(RC_CFLAGS) $(MORECPP) \
	      -fno-builtin -static $(OMIT_FRAME_POINTER_CFLAG) \
	      -march=pentium4 -msse2 -msoft-float -fno-stack-protector \
	      -nostdinc -include $(SRCROOT)/autoconf.h

ifeq(${CONFIG_COVERAGE},y)
CFLAGS += -fprofile-arcs -ftest-coverage
endif


CPPFLAGS := $(CPPFLAGS) -nostdinc++

INC = -I. -I$(SYMROOT) -I$(LIBSAIODIR) -I${SRCROOT}/i386/include

OBJS = prf.x86.elf.o printf.x86.elf.o zalloc.x86.elf.o \
	string.x86.elf.o strtol.x86.elf.o error.x86.elf.o \
	setjmp.x86.elf.o qsort.x86.elf.o efi_tables.x86.elf.o

OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

LIBS = libsa.a
LIBS := $(addprefix $(SYMROOT)/, $(LIBS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme: $(DIRS_NEEDED) $(LIBS)


$(LIBS): $(OBJS)
	@echo "	[RM] $@"
	@rm -f $@
	@echo "	[AR] $(@F)"
	@${AR} q $@ $^ &> /dev/null
	@echo "	[RANLIB] $(@F)"
	@${RANLIB} $@

# dependencies
-include $(OBJROOT)/Makedep

clean-local:
	@for o in $(OBJS); do if [ -f "$${o}" ];then echo "	[RM] $${o}"; fi; done
	@for l in $(LIBS); do if [ -f "$${l}" ];then echo "	[RM] $${l}"; fi; done
	@rm -f $(LIBS) $(OBJS)
