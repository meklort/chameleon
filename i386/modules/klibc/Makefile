MODULE_NAME = klibc
MODULE_DESCRIPTION = This module provides a standard c library for modules to link to if the library provided by chameleon is insufficient. This is currently only used by the uClibc++ library.
MODULE_AUTHOR = 
MODULE_VERSION = "1.5.20"
MODULE_COMPAT_VERSION = "1.5.20"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = klibc

MODULE_OBJS = 	klibc.x86.elf.o \
		__ashldi3.x86.elf.o __ashrdi3.x86.elf.o __clzsi2.x86.elf.o __divdi3.x86.elf.o __divsi3.x86.elf.o \
		__lshrdi3.x86.elf.o __moddi3.x86.elf.o __modsi3.x86.elf.o __udivdi3.x86.elf.o \
		__udivmoddi4.x86.elf.o __udivmodsi4.x86.elf.o __udivsi3.x86.elf.o \
		__umoddi3.x86.elf.o __umodsi3.x86.elf.o \
		strntoumax.x86.elf.o strntoimax.x86.elf.o atol.x86.elf.o atoll.x86.elf.o \
		strcasecmp.x86.elf.o strncasecmp.x86.elf.o strlcat.x86.elf.o strndup.x86.elf.o strnlen.x86.elf.o \
		strsep.x86.elf.o strtoimax.x86.elf.o strtok_r.x86.elf.o strtok.x86.elf.o strtol.x86.elf.o strtoll.x86.elf.o strtotimespec.x86.elf.o strtotimeval.x86.elf.o \
		strtoul.x86.elf.o strtoull.x86.elf.o strtoumax.x86.elf.o strxspn.x86.elf.o strpbrk.x86.elf.o \
		bsearch.x86.elf.o calloc.x86.elf.o \
		jrand48.x86.elf.o lrand48.x86.elf.o mrand48.x86.elf.o srand48.x86.elf.o nrand48.x86.elf.o seed48.x86.elf.o \
		memccpy.x86.elf.o memchr.x86.elf.o memmem.x86.elf.o memmove.x86.elf.o memrchr.x86.elf.o memswap.x86.elf.o  \
		qsort.x86.elf.o sha1hash.x86.elf.o onexit.x86.elf.o atexit.x86.elf.o exit.x86.elf.o \
		snprintf.x86.elf.o vsnprintf.x86.elf.o sscanf.x86.elf.o vsscanf.x86.elf.o\

include ../MakeInc.dir
