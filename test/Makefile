SUBDIRS = ${wildcard *}

distclean all clean test: ${SUBDIRS}

dist:

${SUBDIRS}:
	@if [ -d $@ ]; 				\
	then					\
		${MAKE} --no-print-directory -C $@ ${MAKECMDGOALS}; 	\
	fi


.PHONY: ${SUBDIRS}
