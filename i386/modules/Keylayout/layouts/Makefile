PWD := ${dir $(abspath $(lastword $(MAKEFILE_LIST)))}
SRCROOT = $(abspath $(PWD)/../../../..)
OBJROOT = $(SRCROOT)/obj/i386/util
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
KEYMAPSROOT = $(SRCROOT)/Keymaps

INC = -I$(SRCROOT)/i386/libsaio/ -I. -I..

OBJS     = cham-mklayout.o
OBJS    := $(addprefix $(OBJROOT)/, $(OBJS))

MKLAYOUT = cham-mklayout
MKLAYOUT := $(addprefix $(SYMROOT)/, $(MKLAYOUT))

PROGRAMS = $(MKLAYOUT)

LAYOUTS_SRC_DIR  = layouts-src
KEYBOARD_LAYOUTS = $(patsubst $(LAYOUTS_SRC_DIR)/%.slt, $(KEYMAPSROOT)/%.lyt, \
					$(wildcard $(LAYOUTS_SRC_DIR)/*.slt))

DIRS_NEEDED = $(KEYMAPSROOT)

include ${SRCROOT}/Make.rules

all: $(DIRS_NEEDED) $(OBJROOT) $(SYMROOT) keymaps

$(PROGRAMS): $(OBJS)
	@echo "	[LD] $(@F)"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) $(INCLUDES) \
		-o $(SYMROOT)/$(@F) $(OBJROOT)/$(@F).o

keymaps: $(KEYBOARD_LAYOUTS)

$(KEYMAPSROOT)/%.lyt: $(LAYOUTS_SRC_DIR)/%.slt $(MKLAYOUT)
	@echo "	[LAYOUT] Creating keyboard layout: $*"
	@$(MKLAYOUT) -i $< -o $@

#.PHONY layouts
clean-local:
	@for o in $(OBJS); do if [ -f "$${o}" ];then echo "	[RM] $${o}"; fi; done
	@for p in $(PROGRAMS); do if [ -f "$${p}" ];then echo "	[RM] $${p}"; fi; done
	@for k in $(KEYBOARD_LAYOUTS); do if [ -f "$${k}" ];then echo "	[RM] $${k}"; fi; done
	@$(RM) -f $(PROGRAMS) $(OBJS)

distclean-local:
	@if [ -d "$(KEYMAPSROOT)" ];then echo "	[RMDIR] $(KEYMAPSROOT)"; fi
	@$(RM) -rf $(KEYMAPSROOT)
