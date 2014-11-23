OSNAME = ${shell uname -s}

SUBDIRS = ${wildcard *}

GCOV=gcov
GCOVR=/opt/xZenue/bin/gcovr

COVERAGE_LOG=../sym/test.chamcov

COV_OUTPUT=../sym/i386/coverage/

CHAM2GCOV=../sym/i386/cham2gcov

distclean all clean test: ${sort ${SUBDIRS}}

dist:

ifneq (${OSNAME},Darwin)
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
		${GCOVR} --html --html-details --root ../ -o ${COV_OUTPUT}/index.html -s > /dev/null; \
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
endif

.PHONY: ${SUBDIRS}
