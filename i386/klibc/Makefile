PWD := ${dir $(abspath $(lastword $(MAKEFILE_LIST)))}
SRCROOT = $(abspath $(PWD)/../..)
OBJROOT = $(SRCROOT)/obj/i386/klibc
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = klibc
include ${SRCROOT}/Make.rules

# LIBSAIODIR = ../libsaio

CFLAGS	:= $(CFLAGS) $(RC_CFLAGS) $(MORECPP) \
	      -fno-builtin -static $(OMIT_FRAME_POINTER_CFLAG) \
	      -march=pentium4 -msse2 -msoft-float -fno-stack-protector \
	      -nostdinc -include $(SRCROOT)/autoconf.h

CPPFLAGS := $(CPPFLAGS) -nostdinc++

INC = -I. -I$(SYMROOT) -I${SRCROOT}/i386/include

OBJS = strlcpy.x86.elf.o

OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

LIBS = libklibc.a
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
