MODULE_NAME = module_init
MODULE_DESCRIPTION = Module used to test / verify that modile initializaiton is working properly.
MODULE_AUTHOR = 
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES = uClibcxx

DIR = HelloWorld

MODULE_OBJS = HelloWorld.x86.mach.o

override SRCROOT=${abspath ../../}
override MODULE_DEFINITION=m
override BUILT_IN=no
override SYMROOT=${abspath sym}
override OBJROOT=${abspath obj}
override BOOT_ELF=${SRCROOT}/sym/i386/boot.elf
include ${SRCROOT}/i386/modules/MakeInc.dir

