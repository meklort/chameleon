SUBDIRS = ${wildcard *}

distclean all clean test: ${SUBDIRS}

dist:

${SUBDIRS}:
	@if [ -d $@ ]; 				\
	then					\
		make -C $@ ${MAKECMDGOALS}; 	\
	fi


.PHONY: ${SUBDIRS}
