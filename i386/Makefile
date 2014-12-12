#	Makefile for i386 boot program

#
# these paths are only valid in subdirectories of this directory
#
SRCROOT = $(abspath $(CURDIR)/..)
BINROOT = $(SRCROOT)/bin
OBJROOT = $(SRCROOT)/obj/i386
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

include ${SRCROOT}/Make.rules

# The order of building is important.
SUBDIRS_PRE =

SUBDIRS_PRE += util

SUBDIRS = $(SUBDIRS_PRE) compiler-rt klibc libsa libsaio boot0 boot1 boot2 cdboot modules

all: $(CONFIG_HEADERS) all-recursive

