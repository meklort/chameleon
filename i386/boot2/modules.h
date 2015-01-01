/*
 * Module Loading functionality
 * Copyright 2009 Evan Lojewski. All rights reserved.
 *
 */

#include <saio_types.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include "hooks.h"

#ifndef __BOOT_MODULES_H
#define __BOOT_MODULES_H

#define MODULE_PATH        "/Extra/modules/"

#define SYMBOLS_MODULE "Symbols.dylib"
#define SYMBOLS_AUTHOR "Chameleon"
#define SYMBOLS_DESCRIPTION "Chameleon symbols for linking"
#define SYMBOLS_VERSION     0
#define SYMBOLS_COMPAT      0

#define VOID_SYMBOL        "_dyld_void_start"
#define START_SYMBOL        "start"


#define INIT_SEGMENT        "__DATA"
#define INIT_SECTION        "__mod_init_func"

#define DATA_SEGMENT        "__DATA"
#define BSS_SECTION        "__bss"

#define INFO_SEGMENT        "__INFO"
#define AUTHOR_SECTION        "__author"
#define DESC_SECTION        "__description"


typedef struct symbolList_t
{
    char* symbol;
    UInt64 addr;
    struct symbolList_t* next;
} symbolList_t;

typedef struct modulesList_t
{
    const char*                name;
    const char*             author;
    const char*             description;
    const void*        base;
    UInt32                    version;
    UInt32                    compat;
    struct modulesList_t* next;
} moduleList_t;



int init_module_system();
void load_all_modules();

void start_built_in_module(const char* name, 
                           const char* author, 
                           const char* description,
                           UInt32 version,
                           UInt32 compat,
                           void(*start_function)(void));

int load_module(char* module, UInt32 compat);
int load_module_binary(char* binary, char* module);


#define MODULE_FOUND        1
#define MODULE_INVALID_VERSION    2
int is_module_loaded(const char* name, UInt32 compat);
void module_loaded(const char* name, const void* base, void* start, const char* author, const char* description, UInt32 
version, UInt32 compat);




/********************************************************************************/
/*    Symbol Functions                                                            */
/********************************************************************************/
long long        add_symbol(char* symbol, long long addr, char is64);
unsigned int    lookup_all_symbols(const char* name);
long long remove_symbol(char* name);




/********************************************************************************/
/*    Macho Parser                                                                */
/********************************************************************************/
void*            parse_mach(void* binary, void* base,
                            int(*dylib_loader)(char*, UInt32 compat),
                            long long(*symbol_handler)(char*, long long, char),
                            void (*section_handler)(char* base, char* new_base, char* section, char* segment, void* cmd, UInt64 offset, UInt64 address)
                           );
unsigned int    handle_symtable(UInt32 base, UInt32 new_base,
                             struct symtab_command* symtabCommand,
                             long long(*symbol_handler)(char*, long long, char),
                             char is64);
void            rebase_macho(void* base, void* new_base, char* rebase_stream, UInt32 size);

void            bind_macho(void* base, void* new_base, UInt8* bind_stream, UInt32 size);


/********************************************************************************/
/*    dyld Interface                                                                */
/********************************************************************************/
void dyld_stub_binder();

#endif /* __BOOT_MODULES_H */
