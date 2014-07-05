MODULE_NAME = uClibcxx
MODULE_DESCRIPTION = This module provides a minimalistic c++ runtime library for use in other modules. This does not provide functionality by itself, instead it is used to allow for the c++ language to be used in other modules. Please note that rtti and exceptions has both been disabled. 
MODULE_AUTHOR = 
MODULE_VERSION = "0.2.2"
MODULE_COMPAT_VERSION = "0.2.2"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES = klibc

DIR = uClibc++
# sstream.x86.elf.o fstream.x86.elf.o
MODULE_OBJS = uClibc++.x86.elf.o abi.x86.elf.o algorithm.x86.elf.o associative_base.x86.elf.o bitset.x86.elf.o \
		complex.x86.elf.o del_op.x86.elf.o del_opnt.x86.elf.o del_opv.x86.elf.o del_opvnt.x86.elf.o \
		deque.x86.elf.o eh_alloc.x86.elf.o eh_globals.x86.elf.o exception.x86.elf.o \
		func_exception.x86.elf.o iomanip.x86.elf.o  char_traits.x86.elf.o \
		iterator.x86.elf.o limits.x86.elf.o list.x86.elf.o locale.x86.elf.o \
		map.x86.elf.o new_handler.x86.elf.o new_op.x86.elf.o new_opnt.x86.elf.o new_opv.x86.elf.o \
		new_opvnt.x86.elf.o numeric.x86.elf.o queue.x86.elf.o set.x86.elf.o \
		stack.x86.elf.o stdexcept.x86.elf.o streambuf.x86.elf.o string.x86.elf.o typeinfo.x86.elf.o \
		utility.x86.elf.o valarray.x86.elf.o vector.x86.elf.o support.x86.elf.o \
		ios.x86.elf.o iostream.x86.elf.o istream.x86.elf.o ostream.x86.elf.o 

include ../MakeInc.dir
