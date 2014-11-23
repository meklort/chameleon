OSNAME = ${shell uname -s}

SUBDIRS = ${wildcard *}

GCOV=gcov
GCOVR=/opt/xZenue/bin/gcovr

COVERAGE_LOG=../sym/test.chamcov

COV_OUTPUT=../sym/i386/coverage/

CHAM2GCOV=../sym/i386/cham2gcov

distclean all clean test: ${SUBDIRS}

dist:

ifneq (${OSNAME},Darwin)
${SUBDIRS}:
	@if [ -d $@ ]; 				\
	then					\
		rm -f ${COVERAGE_LOG};		\
		${MAKE} --no-print-directory 				\
			-C $@ ${MAKECMDGOALS}				\
			 COV_FILE=${abspath ${COVERAGE_LOG}}; 		\
		${CHAM2GCOV} ${COVERAGE_LOG} > /dev/null;					\
		mkdir -p ${COV_OUTPUT} > /dev/null;						\
		${GCOVR} --html --html-details --root . -o ${COV_OUTPUT}/index.html -s > /dev/null; \
	fi
endif

.PHONY: ${SUBDIRS}
