MODULE_NAME = Chameleon
MODULE_AUTHOR = Chameleon
MODULE_DESCRIPTION = This module enables the HPET is not done so by the bios
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES =

DIR = ${MODULE_NAME}

MODULE_OBJS = module.x86.mach.o boot.x86.mach.o allocate.x86.mach.o bmdecompress.x86.mach.o console.x86.mach.o \
		drivers.x86.mach.o load.x86.mach.o lzss.x86.mach.o options.x86.mach.o prompt.x86.mach.o \
		resume.x86.mach.o stringTable.x86.mach.o sys.x86.mach.o WKdmDecompress.x86.mach.o

include ../MakeInc.dir
