#
# Until I can remove the dependency on the appkit,
# we'll just keep the generated files in this directory
# and install them directly, rather than generating them again.
#
SRCROOT = $(abspath $(CURDIR)/../..)
OBJROOT = $(SRCROOT)/obj/i386/config
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = util
include ${SRCROOT}/Make.rules

OBJS = cconfig.o zconf.tab.o \
		  yesno.o  textbox.o  menubox.o \
		  checklist.o inputbox.o

OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

DEFINES = -DKBUILD_NO_NLS -DCURSES_LOC=\<ncurses.h\> -DPATH_MAX=256 -DPACKAGE=\"chameleon\" 
LDFLAGS = -lncurses -lmenu

PROGRAMS = cconfig

SYMPROG = $(addprefix $(SYMROOT)/, $(PROGRAMS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all: $(DIRS_NEEDED) $(SYMPROG)

$(SYMPROG): $(OBJS)
	@echo "	[LD] $(@F)"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -o $(SYMROOT)/$(@F) $(OBJROOT)/*.o

config: $(DIRS_NEEDED) $(SYMPROG)
	@cd ${SRCROOT} && $(SYMPROG) $(SRCROOT)/Cconfig

rebuild_config: $(DIRS_NEEDED) $(SYMPROG)
	@cd ${SRCROOT} && $(SYMPROG) $(SRCROOT)/Cconfig rebuild


.PHONY: config rebuild_config

#dependencies
-include $(OBJROOT)/Makedep

distclean-local:
	@rm -f $(SYMPROG) $(OBJS) $(OBJROOT)/Makedep
