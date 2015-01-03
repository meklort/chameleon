/*
 * Copyright 2010-2015 Evan Lojewski. All rights reserved.
 *
 */
#include "boot.h"
#include "modules.h"
#include "mboot.h"
#include <vers.h>

#include <string.h>

#ifndef CONFIG_MODULE_DEBUG
#define CONFIG_MODULE_DEBUG 0
#endif


#if CONFIG_MODULE_DEBUG
#define DBG(x...)    printf(x)
#define DBGPAUSE()    getchar()
#else
#define DBG(x...)
#define DBGPAUSE()
#endif

symbolList_t* get_symbol_entry(const char* name);


void module_section_handler(char* base, char* new_base, char* section, char* segment, void* cmd, UInt64 offset, UInt64 address);

// NOTE: Global so that modules can link with this
static UInt64 textAddress = 0;
static UInt64 textSection = 0;
static UInt64 initAddress = 0;
static UInt64 initFunctions = 0;

/** Internal symbols, however there are accessor methods **/
moduleList_t* loadedModules = NULL;
symbolList_t* moduleSymbols = NULL;
unsigned int (*lookup_symbol)(const char*) = NULL;

char* gAuthor = NULL;
char* gDesc = NULL;
extern UInt32 gVersion;
extern UInt32 gCompat;

char *strrchr(const char *s, int c)
{
    const char *found = NULL;

    while (*s) {
        if (*s == (char)c)
            found = s;
        s++;
    }

    return (char *)found;
}

/*
 * Initialize the module system by loading the Symbols.dylib module.
 * Once loaded, locate the _lookup_symbol function so that internal
 * symbols can be resolved.
 */
int init_module_system()
{
    // Start any modules that were compiled in first.
    //start_built_in_modules();


    int retVal = 0;

    extern char*  __data_start;
    char* module_data = (char*)&__data_start;

    // Intialize module system
    if(module_data)
    {
        retVal = (load_module_binary(module_data, SYMBOLS_MODULE) == 0);
    }

    // Look for modules located in the multiboot header.
    if(gMI && (gMI->mi_flags & MULTIBOOT_INFO_HAS_MODS) && gMI->mi_mods_count)
    {
        struct multiboot_module* mod = (struct multiboot_module*)gMI->mi_mods_addr;
        UInt32 count = gMI->mi_mods_count;
        while(count--)
        {
            if(mod->mm_string)
            {
                // Convert string to module name, check for dylib.
                if(strcmp(&mod->mm_string[strlen(mod->mm_string) - sizeof("dylib")], ".dylib") == 0)
                {
                    module_data = (char*)mod->mm_mod_start;

                    char* last = strrchr(mod->mm_string, '/');
                    if(last) last++;
                    else last = mod->mm_string;

                    DBG("Loading multiboot module %s\n", last);

                    load_module_binary(module_data, last);
                }
            }
            mod++;
        }
    }

    if(retVal) execute_hook("ModulesLoaded", NULL, NULL, NULL, NULL);

    return retVal;
}

void start_built_in_module(const char* name,
                           const char* author,
                           const char* description,
                           UInt32 version,
                           UInt32 compat,
                           void(*start_function)(void))
{
    start_function();
    // Notify the module system that this module really exists, specificaly, let other module link with it
    module_loaded(name, (const void*)0, start_function, author, description, version, compat);
}

/*
 * Load all modules in the /Extra/modules/ directory
 * Module depencdies will be loaded first
 * Modules will only be loaded once. When loaded  a module must
 * setup apropriete function calls and hooks as required.
 * NOTE: To ensure a module loads after another you may
 * link one module with the other. For dyld to allow this, you must
 * reference at least one symbol within the module.
 */
void load_all_modules()
{
    char* name;
    long flags;
    long time;
    struct dirstuff* moduleDir = opendir("/Extra/modules/");
    if(!moduleDir)
    {
        verbose("Warning: Unable to open modules folder at '/Extra/modules/'. Ignoring modules.\n");
        return;
    }
    while (readdir(moduleDir, (const char**)&name, &flags, &time) >= 0) {
        if(strcmp(&name[strlen(name) - sizeof("dylib")], ".dylib") == 0) {
            char* tmp = malloc(strlen(name) + 1);
            strcpy(tmp, name);

            if(!load_module(tmp, 0)) {
                // failed to load
                // free(tmp);
            }
        }
        else
        {
            DBG("Ignoring %s\n", name);
        }

    }

    closedir(moduleDir);
}

int load_module_binary(char* binary, char* module)
{
    void (*module_start)(void) = NULL;

    // Module loaded into memory, parse it
    UInt32 base_size = pre_parse_mach((void*)binary);
    char* base = base_size ? malloc(base_size) : binary;

    textSection = 0;
    textAddress = 0;    // reinitialize text location in case it doesn't exist;
    initAddress = 0;
    initFunctions = 0;

    if(!base_size)
    {
        // Code-less module - symbols only. Use Aboslute address of symbols (base = 0).
        base = (void*)0;
    }
    else
    {
        // Standard module - symbols are based off of *base*
    }

    parse_mach(binary, base, &load_module, &add_symbol, &module_section_handler);

    module_start = (void*)remove_symbol(START_SYMBOL);

    if(initAddress && initFunctions)
    {
        void (**ctor)() = (void*)(initAddress + base);
        while(initFunctions--)
        {
            UInt32 data = (UInt32)(ctor);
            UInt32 value = *(UInt32*)(ctor);
            //DBG("Init function at %x = %x\n", data, value); DBGPAUSE();
            ctor[0]();
            ctor++;
        }
    }

    if(module_start && module_start != (void*)0xFFFFFFFF)
    {
        // Notify the system that it was laoded
        module_loaded(module, ((UInt32)textAddress) + base, module_start, gAuthor, gDesc, gVersion, gCompat);
        if(gAuthor) { free(gAuthor); gAuthor = NULL; }
        if(gDesc) { free(gDesc); gDesc = NULL; }
        gVersion = 0;
        gCompat = 0;

        (*module_start)();    // Start the module
        DBG("Module %s Loaded.\n", module); DBGPAUSE();
    }
#if CONFIG_MODULE_DEBUG
    else // The module does not have a valid start function. This may be a library.
    {
        printf("WARNING: Unable to start %s\n", module);
        getchar();
        return -1;
    }
#else
    else 
    {
        verbose("WARNING: Unable to start %s\n", module);
        return -1;
    }
#endif
    return 0;
}

/*
 * Load a module file in /Extra/modules/
 */
int load_module(char* module, UInt32 compat)
{
    int retVal = 1;
    char modString[128];
    int fh = -1;

    // Check to see if the module has already been loaded
    int loaded = is_module_loaded(module, compat);
    if(loaded == MODULE_FOUND)
    {
        return 1;
    }
    if(loaded == MODULE_INVALID_VERSION) return 0;

    snprintf(modString, sizeof(modString), MODULE_PATH "%s", module);
    fh = open(modString, 0);
    if(fh < 0)
    {
        DBG("WARNING: Unable to locate module %s\n", modString); DBGPAUSE();
        return 0;
    }
    unsigned int moduleSize = file_size(fh);

    if(moduleSize == 0)
    {
        DBG("WARNING: The module %s has a file size of %d, the module will not be loaded.\n", modString, moduleSize);
        return 0;
    }

    char* module_base = (char*) malloc(moduleSize);
    if (moduleSize && read(fh, module_base, moduleSize) == moduleSize)
    {
        load_module_binary(module_base, module);
    }
    else
    {
        DBG("Unable to read in module %s\n.", module); DBGPAUSE();
        retVal = 0;
    }
    if(module_base) free(module_base); // Module was copied to new location
    close(fh);
    return retVal;
}


/* module_section_handler
 * This function is used to detect when a module has initilization / construcitons
 * that must run for the module to work properly.
 *
 */
void module_section_handler(char* base, char* new_base, char* section, char* segment, void* cmd, UInt64 offset, UInt64 address)
{
    struct section *sect = cmd;
    if(!cmd) return; // ERROR

    UInt8 flags = sect->flags & SECTION_TYPE;
    if(flags == S_ZEROFILL)
    {
        uint32_t size = sect->size;
        uint32_t addr = sect->addr;
        DBG("Zeroing section    %s,%s: ", section, segment);
        DBG("%x bytes at %x\n", size, addr + new_base);
        memset(addr + new_base, 0, size);
    }
    else if(sect->size && sect->offset)
    {
        uint32_t size = sect->size;
        uint32_t addr = sect->addr;
        uint32_t offset = sect->offset;
        DBG("Relocation section %s,%s: ", section, segment);
        DBG("%x bytes from %x to %x\n", size, base + addr, new_base + addr);
        memcpy(new_base + addr, base + addr, size);
    }

    if((strcmp("__TEXT", segment) == 0) && 
       (strcmp("__text", section) == 0))
    {
        // __TEXT,__text found, save the offset and address for when looking for the calls.
        textSection = sect->offset;
        textAddress = sect->addr;
    }
    else if(    strcmp(section, INIT_SECTION) == 0 &&
                strcmp(segment, INIT_SEGMENT) == 0)
    {
        // TODO: use rebased location
        DBG("Module has initialization data.\n");

        UInt32 num_ctors = sect->size / sizeof(void*);
        DBG("Found %d constructors\n", num_ctors);
        initAddress = address;
        initFunctions = num_ctors;

        DBGPAUSE();
    }
    else if(strcmp(segment, INFO_SEGMENT) == 0)
    {
        uint32_t size = sect->size;
        uint32_t addr = sect->offset; // offset in file

        char* string = malloc(size + 1);
        memcpy(string, base + addr, size);
        string[size] = 0;
        
        if(strcmp(section, AUTHOR_SECTION) == 0) gAuthor = string;
        if(strcmp(section, DESC_SECTION) == 0) gDesc = string;

        //free(string);
        // TODO: look up author and description of module
        
    }
}


/*
 * print out the information about the loaded module
 */
void module_loaded(const char* name, const void* base, void* start, const char* author, const char* description, UInt32 version, UInt32 compat)
{
    moduleList_t* new_entry = malloc(sizeof(moduleList_t));
    new_entry->next = loadedModules;

    loadedModules = new_entry;

    if(!name) name = "Unknown";
    if(!author) author = "Unknown";
    if(!description) description = "";

    new_entry->name = name;
    new_entry->author = author;
    new_entry->description = description;
    new_entry->version = version;
    new_entry->compat = compat;
    new_entry->base = base;

    DBG("Module '%s' by '%s' Loaded.\n", name, author);
    DBG("\tInitialization: 0x%X\n", start);
    DBG("\tDescription: %s\n", description);
    DBG("\tVersion: %d.%d.%d\n", version >> 16, version >> 8 & 0xFF, version & 0xFF);
    DBG("\tCompat:  %d.%d.%d\n", compat >> 16, compat >> 8 & 0xFF, compat & 0xFF);
}


int is_module_loaded(const char* name, UInt32 compat)
{
    // todo sorted search
    moduleList_t* entry = loadedModules;
    while(entry)
    {
        if(strcmp(entry->name, name) == 0)
        {
            if(!compat) return 1; // Ignore compat version
            UInt8 minor = compat >> 8 & 0xFF;
            UInt16 major = compat >> 16;
            UInt8 act_minor = entry->compat >> 8 & 0xFF;
            UInt16 act_major = entry->compat >> 16;

            DBG("Located module %s\n", name); DBGPAUSE();
            if(act_major == major)
            {
                // Found major version compat, check minor version
                if(minor <= act_minor)
                {
                    // Loaded module has a larger minor version, is compat
                    return MODULE_FOUND;
                }
                else
                {
                    // Loaded minor is older tahn teh expected minor revision. ERROR.
                }
            }
            // Major versions do not match.
            printf("Expected module %s minor version is too old.\n"
                "Wanted: %d.%d.%d\n"
                "Found: %d.%d.%d\n", 
                name, 
                major, minor, compat & 0xFF,
                act_major, act_minor, entry->compat & 0xFF);
            getchar();

            return MODULE_INVALID_VERSION;
        }
        else
        {
            entry = entry->next;
        }

    }

    DBG("Module %s not loaded\n", name); DBGPAUSE();
    return 0;
}


/*
 *    lookup symbols in all loaded modules. Returns the symbol entry, or NULL if not found
 *
 */
symbolList_t* get_symbol_entry(const char* name)
{
    symbolList_t* entry = moduleSymbols;
    while(entry)
    {
        if(strcmp(entry->symbol, name) == 0)
        {
            return entry;
        }
        else
        {
            entry = entry->next;
        }
    }
    return NULL;
}


/*
 * remove_symbol
 * This function removes a symbol from  the list of known symbols
 * and returns it's address, if found.
 */
long long remove_symbol(char* name)
{
    symbolList_t* prev = NULL;
    symbolList_t* entry = get_symbol_entry(name);
    if(entry)
    {
        //DBG("External symbol %s located at 0x%X\n", name, entry->addr);
        if(entry->prev)
        {
            entry->prev->next = entry->next;
            entry->next->prev = entry->prev;
        }
        else
        {
            moduleSymbols = entry->next;
            moduleSymbols->prev = NULL;
        }
        long long addr = entry->addr;
        free(entry);

        return entry->addr;
    }

    // No symbol
    return 0xFFFFFFFF;
}


/*
 * add_symbol
 * This function adds a symbol from a module to the list of known symbols
 * possibly change to a pointer and add this to the Symbol module so that it can
 * adjust it's internal symbol list (sort) to optimize locating new symbols
 */
void add_symbol(char* symbol, long long addr, char is64)
{
    // This only can handle 32bit symbols
    symbolList_t* entry;
    DBG("Adding symbol %s at 0x%X\n", symbol, addr);

    entry = malloc(sizeof(symbolList_t));
    moduleSymbols->prev = entry;
    entry->next = moduleSymbols;
    entry->prev = NULL;
    moduleSymbols = entry;

    entry->addr = (UInt32)addr;
    entry->symbol = symbol;
}

/*
 *    lookup symbols in all loaded modules. Thins inludes boot syms due to Symbols.dylib construction
 *
 */
unsigned int lookup_all_symbols(const char* name)
{
    symbolList_t* entry = get_symbol_entry(name);
    if(entry)
    {
        //DBG("External symbol %s located at 0x%X\n", name, entry->addr);
        return entry->addr;
    }

#if CONFIG_MODULE_DEBUG
    printf("Unable to locate symbol %s\n", name);
    getchar();
#endif

    if(strcmp(name, VOID_SYMBOL) == 0) return 0xFFFFFFFF;
    // In the event that a symbol does not exist
    // Return a pointer to a void function.
    else return lookup_all_symbols(VOID_SYMBOL);
}


/********************************************************************************/
/*    dyld / Linker Interface                                                        */
/********************************************************************************/

void dyld_stub_binder()
{
    printf("ERROR: dyld_stub_binder was called, should have been take care of by the linker.\n");
    getchar();
}
