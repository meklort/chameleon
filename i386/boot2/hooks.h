/*
 * Copyright 2010-2015 Evan Lojewski. All rights reserved.
 *
 */
#ifndef HOOKS_H
#define HOOKS_H
#include "libsaio.h"
#include "modules.h"

typedef struct callbackList_t
{
    void(*callback)(void*, void*, void*, void*);
    struct callbackList_t* next;
} callbackList_t;

typedef struct moduleHook_t
{
    const char* name;
    callbackList_t* callbacks;
    struct moduleHook_t* next;
} moduleHook_t;


/********************************************************************************/
/*    Module Interface                                                        */
/********************************************************************************/
int             replace_function(const char* symbol, void* newAddress);
int             execute_hook(const char* name, void*, void*, void*, void*);
void            register_hook_callback(const char* name, void(*callback)(void*, void*, void*, void*));
moduleHook_t*   hook_exists(const char* name);

#if 0
void            print_hook_list();
#endif

#endif /*!HOOKS_H*/
