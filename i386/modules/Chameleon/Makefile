MODULE_NAME = Chameleon
MODULE_AUTHOR = Chameleon
MODULE_DESCRIPTION = Primary xnu bootloader code
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES =

DIR = ${MODULE_NAME}

MODULE_OBJS_INT = module.x86.mach.o boot.x86.mach.o allocate.x86.mach.o bmdecompress.x86.mach.o console.x86.mach.o \
		drivers.x86.mach.o load.x86.mach.o lzss.x86.mach.o options.x86.mach.o prompt.x86.mach.o \
		resume.x86.mach.o stringTable.x86.mach.o sys.x86.mach.o WKdmDecompress.x86.mach.o	\
		ramdisk.x86.mach.o


LIBSAIO_OBJS = platform.x86.mach.o acpi_patcher.x86.mach.o convert.x86.mach.o device_tree.x86.mach.o \
		hda.x86.mach.o pci_setup.x86.mach.o smbios_decode.x86.mach.o aml_generator.x86.mach.o \
		cpu.x86.mach.o disk.x86.mach.o pci.x86.mach.o smbios_getters.x86.mach.o bootstruct.x86.mach.o \
		device_inject.x86.mach.o fake_efi.x86.mach.o pci_root.x86.mach.o smbios.x86.mach.o \
		xml.x86.mach.o base64-decode.x86.mach.o

LIBSA_OBJS = 

DIRS_NEEDED = ${OBJROOT}/libsaio

MODULE_OBJS = ${addprefix libsaio/,${LIBSAIO_OBJS}} ${addprefix libsa/,${LIBSA_OBJS}} ${MODULE_OBJS_INT}

include ../MakeInc.dir
