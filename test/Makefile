#
# Makefile for tests
#

OSNAME = ${shell uname -s}

SUBDIRS = ${wildcard *}

GCOV=gcov
GCOVR=/opt/xZenue/bin/gcovr

COVERAGE_LOG=../sym/test.chamcov

COV_OUTPUT=../sym/coverage/

CHAM2GCOV=../sym/i386/cham2gcov

QEMU_COMMAND=qemu-system-i386 -m 512 -kernel ${SRCROOT}/sym/i386/boot \
								-device isa-debug-exit,iobase=0xf4,iosize=0x04 \
								-serial file:${COV_FILE} \
								-serial file:${SYMROOT}/output


distclean all clean test: ${sort ${SUBDIRS}}

dist:

ifeq (${MAKECMDGOALS},test)
${SUBDIRS}:
	@if [ -d $@ ]; 				\
	then					\
		rm -f ${COVERAGE_LOG} &&		\
		${MAKE} --no-print-directory 				\
			-C $@ ${MAKECMDGOALS}				\
			 COV_FILE=${abspath ${COVERAGE_LOG}} && 		\
		${CHAM2GCOV} ${COVERAGE_LOG} > /dev/null &&					\
		rm -f ${COVERAGE_LOG} &&							\
		mkdir -p ${COV_OUTPUT} > /dev/null &&						\
		${GCOVR} --html --html-details --root ../ -o ${COV_OUTPUT}/index.html -s; \
	fi
else
${SUBDIRS}:
	@if [ -d $@ ]; 				\
	then					\
		${MAKE} --no-print-directory 				\
			-C $@ ${MAKECMDGOALS}				\
			 COV_FILE=${abspath ${COVERAGE_LOG}};		\
	fi
endif

.PHONY: ${SUBDIRS}
