/*
 * Copyright 2010-2015 Evan Lojewski. All rights reserved.
 *
 */
#include "hooks.h"

#ifndef CONFIG_HOOKS_DEBUG
#define CONFIG_HOOKS_DEBUG 0
#endif


#if CONFIG_HOOKS_DEBUG
#define DBG(x...)    printf(x)
#define DBGPAUSE()    getchar()
#else
#define DBG(x...)
#define DBGPAUSE()
#endif

moduleHook_t* gHookCallbacks = NULL;

/********************************************************************************/
/*        Hook Interface                                                        */
/********************************************************************************/

/*
* Locate the symbol for an already loaded function and modify the beginning of
* the function to jump directly to the new one
* example: replace_function("_HelloWorld_start", &replacement_start);
*/
int replace_function(const char* symbol, void* newAddress)
{
#if defined(__i386__)
    UInt32 addr = lookup_all_symbols(symbol);
    if(addr && addr != 0xFFFFFFFF && addr != lookup_all_symbols(VOID_SYMBOL))
    {
        //DBG("Replacing %s to point to 0x%x\n", symbol, newAddress);
        UInt32* jumpPointer = malloc(sizeof(UInt32*));
        char* binary = (char*)addr;
        *binary++ = 0xFF;    // Jump
        *binary++ = 0x25;    // Long Jump
        *((UInt32*)binary) = (UInt32)jumpPointer;

        *jumpPointer = (UInt32)newAddress;
        return 1;
    }
#else
#error Unknown CPU Architechture for replace_function
#endif    
    return 0;
}


/*
 *    execute_hook(  const char* name )
 *        name - Name of the module hook
 *            If any callbacks have been registered for this hook
 *            they will be executed now in the same order that the
 *            hooks were added.
*/
int execute_hook(const char* name, void* arg1, void* arg2, void* arg3, void* arg4)
{
    DBG("Attempting to execute hook '%s'\n", name); DBGPAUSE();
    moduleHook_t* hook = hook_exists(name);

    if(hook)
    {
        // Loop through all callbacks for this module
        callbackList_t* callbacks = hook->callbacks;

        while(callbacks)
        {
            // Execute callback
            callbacks->callback(arg1, arg2, arg3, arg4);
            callbacks = callbacks->next;
        }
        DBG("Hook '%s' executed.\n", name); DBGPAUSE();
        return 1;
    }
    else
    {
        // Callback for this hook doesn't exist;
        DBG("No callbacks for '%s' hook.\n", name);
        return 0;
    }
}



/*
 *    register_hook_callback(  const char* name,  void(*callback)())
 *        name - Name of the module hook to attach to.
 *        callbacks - The funciton pointer that will be called when the
 *            hook is executed. When registering a new callback name, the callback is added sorted.
 *            NOTE: the hooks take four void* arguments.
 */
void register_hook_callback(const char* name, void(*callback)(void*, void*, void*, void*))
{
    DBG("Adding callback for '%s' hook.\n", name); DBGPAUSE();

    moduleHook_t* hook = hook_exists(name);

    if(hook)
    {
        // append
        callbackList_t* newCallback = malloc(sizeof(callbackList_t));
        newCallback->next = hook->callbacks;
        hook->callbacks = newCallback;
        newCallback->callback = callback;
    }
    else
    {
        // create new hook
        moduleHook_t* newHook = malloc(sizeof(moduleHook_t));
        newHook->name = name;
        newHook->callbacks = malloc(sizeof(callbackList_t));
        newHook->callbacks->callback = callback;
        newHook->callbacks->next = NULL;

        newHook->next = gHookCallbacks;
        gHookCallbacks = newHook;

    }

#if CONFIG_HOOKS_DEBUG
    //print_hook_list();
    //getchar();
#endif

}


moduleHook_t* hook_exists(const char* name)
{
    moduleHook_t* hooks = gHookCallbacks;

    // look for a hook. If it exists, return the moduleHook_t*,
    // If not, return NULL.
    while(hooks)
    {
        if(strcmp(name, hooks->name) == 0)
        {
            //DBG("Located hook %s\n", name);
            return hooks;
        }
        hooks = hooks->next;
    }
    //DBG("Hook %s does not exist\n", name);
    return NULL;

}

#if CONFIG_HOOKS_DEBUG
void print_hook_list()
{
    printf("---Hook Table---\n");

    moduleHook_t* hooks = moduleCallbacks;
    while(hooks)
    {
        printf("Hook: %s\n", hooks->name);
        hooks = hooks->next;
    }
}

#endif
