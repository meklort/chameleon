MODULE_NAME = klibc
MODULE_DESCRIPTION = This module provides a standard c library for modules to link to if the library provided by chameleon is insufficient. This is currently only used by the uClibc++ library.
MODULE_AUTHOR = 
MODULE_VERSION = "1.5.20"
MODULE_COMPAT_VERSION = "1.5.20"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = klibc

MODULE_OBJS = 	klibc.x86.mach.o \
		__ashldi3.x86.mach.o __ashrdi3.x86.mach.o __clzsi2.x86.mach.o __divdi3.x86.mach.o __divsi3.x86.mach.o \
		__lshrdi3.x86.mach.o __moddi3.x86.mach.o __modsi3.x86.mach.o __udivdi3.x86.mach.o \
		__udivmoddi4.x86.mach.o __udivmodsi4.x86.mach.o __udivsi3.x86.mach.o \
		__umoddi3.x86.mach.o __umodsi3.x86.mach.o \
		strntoumax.x86.mach.o strntoimax.x86.mach.o atol.x86.mach.o atoll.x86.mach.o \
		strcasecmp.x86.mach.o strncasecmp.x86.mach.o strlcat.x86.mach.o strndup.x86.mach.o strnlen.x86.mach.o \
		strsep.x86.mach.o strtoimax.x86.mach.o strtok_r.x86.mach.o strtok.x86.mach.o strtol.x86.mach.o strtoll.x86.mach.o strtotimespec.x86.mach.o strtotimeval.x86.mach.o \
		strtoul.x86.mach.o strtoull.x86.mach.o strtoumax.x86.mach.o strxspn.x86.mach.o strpbrk.x86.mach.o \
		bsearch.x86.mach.o calloc.x86.mach.o \
		jrand48.x86.mach.o lrand48.x86.mach.o mrand48.x86.mach.o srand48.x86.mach.o nrand48.x86.mach.o seed48.x86.mach.o \
		memccpy.x86.mach.o memchr.x86.mach.o memmem.x86.mach.o memmove.x86.mach.o memrchr.x86.mach.o memswap.x86.mach.o  \
		qsort.x86.mach.o sha1hash.x86.mach.o onexit.x86.mach.o atexit.x86.mach.o exit.x86.mach.o \
		snprintf.x86.mach.o vsnprintf.x86.mach.o sscanf.x86.mach.o vsscanf.x86.mach.o\

include ../MakeInc.dir
