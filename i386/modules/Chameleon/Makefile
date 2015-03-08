MODULE_NAME = Chameleon
MODULE_AUTHOR = Chameleon
MODULE_DESCRIPTION = Primary xnu bootloader code
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = $(MODULE_NAME)_start
#MODULE_DEPENDENCIES = klibc

DIR = ${MODULE_NAME}



MODULE_OBJS_INT = module.x86.mach.o

#asm.x86.mach.o bios.x86.mach.o 
LIBSAIO_OBJS = table.x86.mach.o biosfn.x86.mach.o \
        disk.x86.mach.o sys.x86.mach.o cache.x86.mach.o bootstruct.x86.mach.o \
        stringTable.x86.mach.o load.x86.mach.o pci.x86.mach.o allocate.x86.mach.o misc.x86.mach.o \
        befs.x86.mach.o freebsd.x86.mach.o openbsd.x86.mach.o \
        vbe.x86.mach.o nbp.x86.mach.o hfs.x86.mach.o hfs_compare.x86.mach.o \
        xml.x86.mach.o ntfs.x86.mach.o msdos.x86.mach.o md5c.x86.mach.o device_tree.x86.mach.o \
        cpu.x86.mach.o platform.x86.mach.o acpi_patcher.x86.mach.o state_generator.x86.mach.o \
        smbios.x86.mach.o smbios_getters.x86.mach.o smbios_decode.x86.mach.o \
        fake_efi.x86.mach.o ext2fs.x86.mach.o \
        hpet.x86.mach.o dram_controllers.x86.mach.o spd.x86.mach.o usb.x86.mach.o pci_setup.x86.mach.o \
        device_inject.x86.mach.o nvidia_helper.x86.mach.o nvidia.x86.mach.o ati.x86.mach.o gma.x86.mach.o hda.x86.mach.o pci_root.x86.mach.o \
        convert.x86.mach.o aml_generator.x86.mach.o console.x86.mach.o exfat.x86.mach.o base64-decode.x86.mach.o

#setjmp.x86.mach.o interrupts.x86.mach.o
LIBSA_OBJS = prf.x86.mach.o printf.x86.mach.o \
        string.x86.mach.o strtol.x86.mach.o error.x86.mach.o \
        qsort.x86.mach.o efi_tables.x86.mach.o

BOOT2_OBJS = boot.x86.mach.o graphics.x86.mach.o drivers.x86.mach.o prompt.x86.mach.o options.x86.mach.o lzss.x86.mach.o lzvn.x86.mach.o mboot.x86.mach.o \
           ramdisk.x86.mach.o picopng.x86.mach.o resume.x86.mach.o bmdecompress.x86.mach.o graphic_utils.x86.mach.o gui.x86.mach.o


DIRS_NEEDED = ${OBJROOT}/trunk/i386/libsaio ${OBJROOT}/trunk/i386/libsa ${OBJROOT}/trunk/i386/boot2

MODULE_OBJS = ${addprefix trunk/i386/libsa/,${LIBSA_OBJS}} \
		${addprefix trunk/i386/libsaio/,${LIBSAIO_OBJS}} \
		${addprefix trunk/i386/boot2/,${BOOT2_OBJS}} \
		${MODULE_OBJS_INT}

include ../MakeInc.dir


override INC := ${filter-out -I$(SRCROOT)/i386/include/,${INC}} -Itrunk/i386/include/
override INC = -I$(SRCROOT)/i386/modules/include/ -I${abspath include/} -Itrunk/i386/libsaio/ -Itrunk/i386/libsa/ \
	-Itrunk/i386/include/ -Itrunk/i386/boot2/ $(MODULE_INCLUDES)

