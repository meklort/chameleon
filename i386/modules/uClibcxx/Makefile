MODULE_NAME = uClibcxx
MODULE_DESCRIPTION = This module provides a minimalistic c++ runtime library for use in other modules. This does not provide functionality by itself, instead it is used to allow for the c++ language to be used in other modules. Please note that rtti and exceptions has both been disabled. 
MODULE_AUTHOR = 
MODULE_VERSION = "0.2.2"
MODULE_COMPAT_VERSION = "0.2.2"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES = klibc

DIR = uClibc++
# sstream.x86.mach.o fstream.x86.mach.o
MODULE_OBJS = uClibc++.x86.mach.o abi.x86.mach.o algorithm.x86.mach.o associative_base.x86.mach.o bitset.x86.mach.o \
		complex.x86.mach.o del_op.x86.mach.o del_opnt.x86.mach.o del_opv.x86.mach.o del_opvnt.x86.mach.o \
		deque.x86.mach.o eh_alloc.x86.mach.o eh_globals.x86.mach.o exception.x86.mach.o \
		func_exception.x86.mach.o iomanip.x86.mach.o  char_traits.x86.mach.o \
		iterator.x86.mach.o limits.x86.mach.o list.x86.mach.o locale.x86.mach.o \
		map.x86.mach.o new_handler.x86.mach.o new_op.x86.mach.o new_opnt.x86.mach.o new_opv.x86.mach.o \
		new_opvnt.x86.mach.o numeric.x86.mach.o queue.x86.mach.o set.x86.mach.o \
		stack.x86.mach.o stdexcept.x86.mach.o streambuf.x86.mach.o string.x86.mach.o typeinfo.x86.mach.o \
		utility.x86.mach.o valarray.x86.mach.o vector.x86.mach.o support.x86.mach.o \
		ios.x86.mach.o iostream.x86.mach.o istream.x86.mach.o ostream.x86.mach.o 

include ../MakeInc.dir
