#
# Until I can remove the dependency on the appkit,
# we'll just keep the generated files in this directory
# and install them directly, rather than generating them again.
#
SRCROOT = $(abspath $(CURDIR)/../..)
OBJROOT = $(SRCROOT)/obj/i386/util
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = util
include ${SRCROOT}/Make.rules

PROGRAMS = bin2h
OBJS = bin2h.o

PROGRAMS += dyldsymboltool
OBJS += dyldsymboltool.o

#PROGRAMS = machOconv dyldsymboltool segsize
#OBJS = dyldsymboltool.o machOconv.o segsize.o
#PROGRAMS = machOconv segsize
#OBJS = machOconv.o segsize.o


ifeq (${CONFIG_COVERAGE}, y)
PROGRAMS += cham2gcov
OBJS += cham2gcov.o
endif

#ifeq (${CONFIG_BDMESG}, y)
#PROGRAMS += bdmesg
#OBJS += bdmesg.o32 bdmesg.o64
#endif

#ifeq (${CONFIG_OPENUP}, y)
#PROGRAMS += openUp
#OBJS += openUp.o64 openUp.o32
#endif

OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

#LDFLAGS += -framework IOKit -framework CoreFoundation -mmacosx-version-min=10.5

ifeq ($(OS),Windows_NT)
CFLAGS += -I${SRCROOT}/i386/util/include/
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
else
CFLAGS += -I${SRCROOT}/i386/util/include/
endif
endif


CFLAGS += 

SYMPROG = $(addprefix $(SYMROOT)/, $(PROGRAMS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

#SUBDIRS = fdisk

all: $(DIRS_NEEDED) $(SYMPROG) all-recursive

$(SYMPROG): $(OBJS)
	@echo "	[LD] $(@F)"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -o $(SYMROOT)/$(@F) $(OBJROOT)/$(@F).o

#dependencies
-include $(OBJROOT)/Makedep

clean-local:
	@for o in $(OBJS); do if [ -f "$${o}" ];then echo "	[RM] $${o}"; fi; done
	@for p in $(SYMPROG); do if [ -f "$${p}" ];then echo "	[RM] $${p}"; fi; done
	@rm -f $(SYMPROG) $(OBJS)
