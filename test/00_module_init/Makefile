MODULE_NAME = module_init
MODULE_DESCRIPTION = Module used to test / verify that modile initializaiton is working properly.
MODULE_AUTHOR = 
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES = uClibcxx

DIR = ${MODULE_NAME}

MODULE_OBJS = ${MODULE_NAME}.x86.mach.o

override SRCROOT=${abspath ../../}
override MODULE_DEFINITION=m
override SYMROOT=${abspath sym}
override OBJROOT=${abspath obj}
override BOOT_ELF=${SRCROOT}/sym/i386/boot.elf

include ${SRCROOT}/i386/modules/MakeInc.dir



test: 
	qemu-system-i386 -curses -m 512 -kernel ${SRCROOT}/sym/i386/boot 
