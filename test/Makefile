OSNAME = ${shell uname -s}

SUBDIRS = ${wildcard *}

distclean all clean test: ${SUBDIRS}

dist:

ifneq (${OSNAME},Darwin)
${SUBDIRS}:
	@if [ -d $@ ]; 				\
	then					\
		${MAKE} --no-print-directory -C $@ ${MAKECMDGOALS}; 	\
	fi
endif

.PHONY: ${SUBDIRS}
