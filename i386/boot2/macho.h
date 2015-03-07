/*
 * Module Loading functionality
 * Copyright 2009-2015 Evan Lojewski. All rights reserved.
 *
 */
#include <saio_types.h>

#ifndef __BOOT_MACHO_H
#define __BOOT_MACHO_H

/********************************************************************************/
/*    Macho Parser                                                                */
/********************************************************************************/
UInt32 pre_parse_mach(void* binary);

bool            parse_mach(void* binary, void* base,
                            int(*dylib_loader)(char*, UInt32 compat),
                            void (*symbol_handler)(char*, long long, char),
                            void (*section_handler)(char* base, char* new_base, char* section, char* segment, void* cmd, UInt64 offset, UInt64 address)
                           );
bool            handle_symtable(UInt32 base, UInt32 new_base,
                             struct symtab_command* symtabCommand,
                             void (*symbol_handler)(char*, long long, char),
                             char is64);
void            rebase_macho(void* base, void* new_base, UInt8* rebase_stream, UInt32 size);

void            bind_macho(void* base, void* new_base, UInt8* bind_stream, UInt32 size);

#endif /* __BOOT_MACHO_H */
