MODULE_NAME = hooks_test
MODULE_DESCRIPTION = Module used to test / verify that modile initializaiton is working properly.
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
include ${SRCROOT}/test/MakeInc.dir



test: 
	@echo "*** Running test ${MODULE_NAME}"
	@rm -f ${SYMROOT}/output
	${PRINT}${TIMEOUT_COMMAND} 10 ${QEMU_COMMAND} \
		-initrd ${SYMROOT}/${MODULE_NAME}.dylib > /dev/null || true
	@grep PASS ${SYMROOT}/output > /dev/null && echo "	${MODULE_NAME} test passed." || (echo "	${MODULE_NAME} test FAILED" && false)
