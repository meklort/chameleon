MODULE_NAME = fat12_probe
MODULE_DESCRIPTION = Module used to test / verify that disk / fat12 io code works.
MODULE_AUTHOR = 
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = ${MODULE_NAME}

MODULE_OBJS = ${MODULE_NAME}.x86.mach.o

override SRCROOT=${abspath ../../}
override MODULE_DEFINITION=m
override SYMROOT=${abspath sym}
override OBJROOT=${abspath obj}
override BOOT_ELF=${SRCROOT}/sym/i386/boot.elf

include ${SRCROOT}/i386/modules/MakeInc.dir



test: 
	@echo "*** Running test ${MODULE_NAME}"
	@rm -f ${SYMROOT}/output
	${PRINT}timeout 10 qemu-system-i386 -m 512 \
		-kernel ${SRCROOT}/sym/i386/boot \
		-initrd ${SYMROOT}/${MODULE_NAME}.dylib \
		-hda fat12.qcow2 \
		-serial file:${COV_FILE} \
		-serial file:${SYMROOT}/output > /dev/null
	@grep PASS ${SYMROOT}/output > /dev/null && echo "	${MODULE_NAME} test passed." || (echo "	${MODULE_NAME} test FAILED" && false)
