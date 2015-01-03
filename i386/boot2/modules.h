/*
 * Module Loading functionality
 * Copyright 2009-2015 Evan Lojewski. All rights reserved.
 *
 */

#include <saio_types.h>
#include "hooks.h"
#include "macho.h"

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
    struct symbolList_t* prev;
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
void            add_symbol(char* symbol, long long addr, char is64);
unsigned int    lookup_all_symbols(const char* name);
long long       remove_symbol(char* name);


/********************************************************************************/
/*    dyld Interface                                                                */
/********************************************************************************/
void dyld_stub_binder();

#endif /* __BOOT_MODULES_H */
