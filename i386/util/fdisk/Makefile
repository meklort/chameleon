PWD = ${dir $(abspath $(lastword $(MAKEFILE_LIST)))}
SRCROOT = $(abspath $(PWD)/../../..)
OBJROOT = $(SRCROOT)/obj/i386/util/fdisk
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot


DIR = fdisk

include ${SRCROOT}/Make.rules


LDFLAGS := $(LDFALGS) -mmacosx-version-min=10.5


OBJS =  cmd.o32 disk.o32 fdisk.o32 getrawpartition.o32 mbr.o32 misc.o32 opendev.o32 part.o32 user.o32 auto.o32 \
		cmd.o64 disk.o64 fdisk.o64 getrawpartition.o64 mbr.o64 misc.o64 opendev.o64 part.o64 user.o64 auto.o64

OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

PROGRAM = fdisk440
PROGRAM:= $(addprefix $(SYMROOT)/, $(PROGRAM))

ifeq ($(CONFIG_FDISK440),y)

all: $(SYMROOT) $(OBJROOT) $(PROGRAM)

$(PROGRAM): $(OBJS)
	@echo "[LD] $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -o $@ $(filter %.o,$^)

else

all:

endif


#dependencies
-include $(OBJROOT)/Makedep

clean-local:
	@for p in $(PROGRAMS); do if [ -f "$${p}" ];then echo "	[RM] $${p}"; fi; done
	@for o in $(OBJS); do if [ -f "$${o}" ];then echo "	[RM] $${o}"; fi; done
	@rm -f $(PROGRAM) $(OBJS)
