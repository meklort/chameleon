# Makefile for i386 modules
PWD := ${dir $(abspath $(lastword $(MAKEFILE_LIST)))}
SRCROOT = $(abspath $(PWD)/../..)
OBJROOT = $(SRCROOT)/obj/i386/modules/
SYMROOT = $(SRCROOT)/sym/i386/modules/
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

include ${SRCROOT}/Make.rules

# The order of building modules is important.
SUBDIRS =

ifdef CONFIG_KLIBC_MODULE
SUBDIRS += klibc
endif

ifdef CONFIG_UCLIBCXX_MODULE
SUBDIRS += uClibcxx
endif

ifdef CONFIG_CHAMELEON_MODULE
SUBDIRS += Chameleon
endif

ifdef CONFIG_RESOLUTION_MODULE
SUBDIRS += Resolution
endif

ifdef CONFIG_HELLOWORLD_MODULE
SUBDIRS += HelloWorld
endif

ifdef CONFIG_KERNELPATCHER_MODULE
SUBDIRS += KernelPatcher
endif

#ifdef CONFIG_KEXTPATCHER_MODULE
#SUBDIRS += KextPatcher
#endif




CFLAGS= -O3 $(MORECPP) -arch i386 -g -static
DEFINES=
CONFIG = hd
LIBSAIODIR = $(SRCROOT)/i386/libsaio
INC = -I$(LIBSAIODIR)


all: $(SYMROOT) $(OBJROOT) objroot_dirs all-recursive

.PHONY: objroot_dirs

objroot_dirs:

clean-local:
