/*
 * Copyright 2010-2015 Evan Lojewski. All rights reserved.
 *
 */
 
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include "macho.h"
#include "boot.h"
#include "modules.h"
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


UInt32 gVersion = 0;
UInt32 gCompat = 0;

static inline void        rebase_location(UInt32* location, char* base, int type);
static inline void        bind_location(UInt32* location, char* value, UInt32 addend, int type);

/********************************************************************************/
/*    Macho Parser                                                                */
/********************************************************************************/

/*
 * Parse teh macho module and determine the total ammount of memory needed for it.
 *
 */
UInt32 pre_parse_mach(void* binary)
{
    UInt32 size = 0;
    UInt32 binaryIndex = 0;
    UInt16 cmd = 0;
    struct load_command *loadCommand = NULL;
        struct segment_command *segCommand = NULL;

    // Parse through the load commands
    if(((struct mach_header*)binary)->magic != MH_MAGIC)
    {
        // Invalid
        return 0;
    }
    
    binaryIndex += sizeof(struct mach_header);
    
    while(cmd < ((struct mach_header*)binary)->ncmds)
    {
        cmd++;

        loadCommand = binary + binaryIndex;
        UInt32 cmdSize = loadCommand->cmdsize;


        switch ((loadCommand->cmd & 0x7FFFFFFF))
        {
            case LC_SEGMENT: // 32bit macho
            {
                segCommand = binary + binaryIndex;

                UInt32 sectionIndex;
                sectionIndex = sizeof(struct segment_command);
                
                struct section *sect;
                                
                while(sectionIndex < segCommand->cmdsize)
                {
                    sect = binary + binaryIndex + sectionIndex;
                    sectionIndex += sizeof(struct section);
                }
                if(strcmp(segCommand->segname, "__LINKEDIT") == 0) break;
                if(strcmp(segCommand->segname, "__INFO") == 0) break;

                size += segCommand->vmsize;


            }
            break;
        }
        binaryIndex += cmdSize;

    }
    return size;
}

/*
 * Parse through a macho module. The module will be rebased and binded
 * as specified in the macho header.
 * NOTE; all dependecies will be loaded before this module is started
 * NOTE: If the module is unable to load ot completeion, the modules
 * symbols will still be available.
 */
bool parse_mach(void* binary, void* base,
                 int(*dylib_loader)(char*, UInt32 compat),
                 void (*symbol_handler)(char*, long long, char),
                 void (*section_handler)(char* base, char* new_base, char* section, char* segment, void* cmd, UInt64 offset, UInt64 address)
)
{
    char is64 = false;

    // Module info
    /*char* moduleName = NULL;
     UInt32 moduleVersion = 0;
     UInt32 moduleCompat = 0;
     */
    // TODO convert all of the structs to a union
    struct load_command *loadCommand = NULL;
    struct dylib_command* dylibCommand = NULL;
    struct dyld_info_command* dyldInfoCommand = NULL;

    struct symtab_command* symtabCommand = NULL;
    struct segment_command *segCommand = NULL;
    struct segment_command_64 *segCommand64 = NULL;

    //struct dysymtab_command* dysymtabCommand = NULL;
    UInt32 binaryIndex = 0;
    UInt16 cmd = 0;

    // Parse through the load commands
    if(((struct mach_header*)binary)->magic == MH_MAGIC)
    {
        is64 = false;
        binaryIndex += sizeof(struct mach_header);
    }
    else if(((struct mach_header_64*)binary)->magic == MH_MAGIC_64)
    {
        // NOTE: modules cannot be 64bit. This is used to parse the kernel and kexts
        is64 = true;
        binaryIndex += sizeof(struct mach_header_64);
    }
    else
    {
        verbose("Invalid mach magic 0x%X\n", ((struct mach_header*)binary)->magic);
        return NULL;
    }

    /*if(((struct mach_header*)binary)->filetype != MH_DYLIB)
     {
     printf("Module is not a dylib. Unable to load.\n");
     getchar();
     return NULL; // Module is in the incorrect format
     }*/

    while(cmd < ((struct mach_header*)binary)->ncmds)
    {
        cmd++;

        loadCommand = binary + binaryIndex;
        UInt32 cmdSize = loadCommand->cmdsize;


        switch ((loadCommand->cmd & 0x7FFFFFFF))
        {
            case LC_SYMTAB:
                symtabCommand = binary + binaryIndex;
                break;

            case LC_SEGMENT: // 32bit macho
            {
                segCommand = binary + binaryIndex;

                UInt32 sectionIndex;

                sectionIndex = sizeof(struct segment_command);

                struct section *sect;

                while(sectionIndex < segCommand->cmdsize)
                {
                    sect = binary + binaryIndex + sectionIndex;

                    sectionIndex += sizeof(struct section);

                    if(section_handler) section_handler(binary, base, sect->sectname, segCommand->segname, (void*)sect, sect->offset, sect->addr);
                }
            }
            break;
        
            case LC_SEGMENT_64:    // 64bit macho's
            {
                segCommand64 = binary + binaryIndex;
                UInt32 sectionIndex;

                sectionIndex = sizeof(struct segment_command_64);

                struct section_64 *sect;

                while(sectionIndex < segCommand64->cmdsize)
                {
                    sect = binary + binaryIndex + sectionIndex;

                    sectionIndex += sizeof(struct section_64);

                    if(section_handler) section_handler(binary, base, sect->sectname, segCommand64->segname, (void*)sect, sect->offset, sect->addr);
                }
            }
            break;


            case LC_LOAD_DYLIB:
            case LC_LOAD_WEAK_DYLIB ^ LC_REQ_DYLD:
                // Required modules
                dylibCommand  = binary + binaryIndex;
                char* module  = binary + binaryIndex + ((UInt32)*((UInt32*)&dylibCommand->dylib.name));
                // Possible enhancments: verify version
                UInt32 compat = dylibCommand->dylib.current_version;
                // =    dylibCommand->dylib.compatibility_version;
                if(dylib_loader)
                {
                    char* name = malloc(strlen(module) + strlen(".dylib") + 1);
                    sprintf(name, "%s.dylib", module);

                    if (!dylib_loader(name, compat))
                    {
                        // NOTE: any symbols exported by dep will be replace with the void function
                        free(name);
                    }
                }

                break;

            case LC_ID_DYLIB:
                dylibCommand = binary + binaryIndex;
                //moduleName =    binary + binaryIndex + ((UInt32)*((UInt32*)&dylibCommand->dylib.name));
                gVersion =    dylibCommand->dylib.current_version;
                gCompat =    dylibCommand->dylib.compatibility_version;
                 
                break;

            case LC_DYLD_INFO:
            //case LC_DYLD_INFO_ONLY:    // compressed info, 10.6+ macho files, already handeled
                // Bind and rebase info is stored here
                dyldInfoCommand = binary + binaryIndex;
                break;

            case LC_DYSYMTAB:
            case LC_UUID:
                break;

            case LC_UNIXTHREAD:
                break;

            case LC_VERSION_MIN_MACOSX:
                break;

            case LC_DATA_IN_CODE:
                break;

            case LC_FUNCTION_STARTS:
                break;

            default:
                DBG("Unhandled loadcommand 0x%X\n", loadCommand->cmd & 0x7FFFFFFF);
                break;

        }

        binaryIndex += cmdSize;
    }

    // bind_macho uses the symbols, if the textAdd does not exist (Symbols.dylib, no code), addresses are static and not relative
    handle_symtable((UInt32)binary, (UInt32)base, symtabCommand, symbol_handler, is64);
    
    if(dyldInfoCommand)
    {
        // Rebase the module before binding it.
        if(dyldInfoCommand->rebase_off)       rebase_macho(binary, base, (UInt8*)dyldInfoCommand->rebase_off,       dyldInfoCommand->rebase_size);
        // Bind all symbols.
        if(dyldInfoCommand->bind_off)         bind_macho(  binary, base, (UInt8*)dyldInfoCommand->bind_off,         dyldInfoCommand->bind_size);
        if(dyldInfoCommand->weak_bind_off)    bind_macho(  binary, base, (UInt8*)dyldInfoCommand->weak_bind_off,    dyldInfoCommand->weak_bind_size);
        if(dyldInfoCommand->lazy_bind_off)    bind_macho(  binary, base, (UInt8*)dyldInfoCommand->lazy_bind_off,    dyldInfoCommand->lazy_bind_size);
    }

    return true;

}

/*
 * parse the symbol table
 * Lookup any undefined symbols
 */

bool handle_symtable(UInt32 base, UInt32 new_base, 
                             struct symtab_command* symtabCommand, 
                             void (*symbol_handler)(char*, long long, char), char is64)
{
    UInt32 symbolIndex            = 0;
    char* symbolString            = base + (char*)symtabCommand->stroff;

    if(!is64)
    {
        struct nlist* symbolEntry = (void*)base + symtabCommand->symoff;
        while(symbolIndex < symtabCommand->nsyms)
        {
            // If the symbol is exported by this module
            if(symbolEntry->n_value)
            {
               symbol_handler(symbolString + symbolEntry->n_un.n_strx, (long long)new_base + symbolEntry->n_value, is64);
            }

            symbolEntry++;
            symbolIndex++;    // TODO remove
        }
    }
    else
    {
        struct nlist_64* symbolEntry = (void*)base + symtabCommand->symoff;
        // NOTE First entry is *not* correct, but we can ignore it (i'm getting radar:// right now, verify later)
        while(symbolIndex < symtabCommand->nsyms)
        {
            // If the symbol is exported by this module
            if(symbolEntry->n_value)
            {
               symbol_handler(symbolString + symbolEntry->n_un.n_strx, (long long)new_base + symbolEntry->n_value, is64);
            }

            symbolEntry++;
            symbolIndex++;    // TODO remove
        }
    }
    return true;
}

UInt64 read_uleb(UInt8* bind_stream, unsigned int* i)
{
    UInt64 tmp;
    UInt32 bits;
    tmp = 0;
    bits = 0;
    do {
        tmp += ((UInt64)(bind_stream[++(*i)] & 0x7f)) << bits;
        bits += 7;
    } while(bind_stream[(*i)] & 0x80);
    return tmp;
}


// Based on code from dylibinfo.cpp and ImageLoaderMachOCompressed.cpp
void rebase_macho(void* base, void* new_base, UInt8* rebase_stream, UInt32 size)
{
    rebase_stream += (UInt32)base;

    UInt8 immediate = 0;
    UInt8 opcode = 0;
    UInt8 type = 0;

    UInt32 segmentAddress = 0;


    UInt64 tmp  = 0;
    UInt64 tmp2  = 0;
    UInt8 bits = 0;
    int index = 0;
    unsigned int i = 0;

    while(i < size)
    {
        immediate = rebase_stream[i] & REBASE_IMMEDIATE_MASK;
        opcode = rebase_stream[i] & REBASE_OPCODE_MASK;


        switch(opcode)
        {
            case REBASE_OPCODE_DONE:
                DBG("REBASE_OPCODE_DONE()\n");
                // Rebase complete, reset vars
                immediate = 0;
                opcode = 0;
                type = 0;
                segmentAddress = 0;
                break;


            case REBASE_OPCODE_SET_TYPE_IMM:
		DBG("REBASE_OPCODE_SET_TYPE_IMM(%d)\n", immediate);
                type = immediate;
                break;


            case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:

                // Locate address to begin rebasing
                segmentAddress = 0;
                struct segment_command* segCommand = NULL; // NOTE: 32bit only

                unsigned int binIndex = 0;
                index = 0;
                do {
                    segCommand = base + sizeof(struct mach_header) +  binIndex;

                    binIndex += segCommand->cmdsize;
                    index++;
                } while(index <= immediate);

                segmentAddress = segCommand->fileoff;
                tmp = read_uleb(rebase_stream, &i);

                segmentAddress += tmp;
		DBG("REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB(%d,0x%X)\n", immediate, tmp);
                break;

            case REBASE_OPCODE_ADD_ADDR_ULEB:
                // Add value to rebase address
                tmp = read_uleb(rebase_stream, &i);

                segmentAddress +=    tmp;
		DBG("REBASE_OPCODE_ADD_ADDR_ULEB(0x%X)\n", tmp);
                break;

            case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
                segmentAddress += immediate * sizeof(void*);
		DBG("REBASE_OPCODE_ADD_ADDR_IMM_SCALED(0x%X)\n", immediate * sizeof(void*));
                break;


            case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
                index = 0;
		DBG("REBASE_OPCODE_DO_REBASE_IMM_TIMES(%d)\n", immediate);
                for (index = 0; index < immediate; ++index) {
                    rebase_location(new_base + segmentAddress, (char*)new_base, type);
                    segmentAddress += sizeof(void*);
                }
                break;

            case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
                tmp = read_uleb(rebase_stream, &i);

		DBG("REBASE_OPCODE_DO_REBASE_ULEB_TIMES(%d)\n", tmp);
                index = 0;
                for (index = 0; index < tmp; ++index) {
                    //DBG("\tRebasing 0x%X\n", segmentAddress);
                    rebase_location(new_base + segmentAddress, (char*)new_base, type);
                    segmentAddress += sizeof(void*);
                }
                break;

            case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
                tmp = read_uleb(rebase_stream, &i);
		DBG("REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB(%d)\n", tmp + sizeof(void*));

                rebase_location(new_base + segmentAddress, (char*)new_base, type);

                segmentAddress += tmp + sizeof(void*);
                break;

            case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
                tmp = read_uleb(rebase_stream, &i);
                tmp2 = read_uleb(rebase_stream, &i);
		UInt32 a = tmp;
		DBG("REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB(%d, %d)\n", a, tmp2);

                index = 0;
                for (index = 0; index < tmp; ++index) {

                    rebase_location(new_base + segmentAddress, (char*)new_base, type);

                    segmentAddress += tmp2 + sizeof(void*);
                }
                break;

            default:
                break;
        }
        i++;
	DBGPAUSE();
    }
}


// Based on code from dylibinfo.cpp and ImageLoaderMachOCompressed.cpp
// NOTE: this uses 32bit values, and not 64bit values.
// There is a possibility that this could cause issues,
// however the modules are 32 bits, so it shouldn't matter too much
void bind_macho(void* base, void* new_base, UInt8* bind_stream, UInt32 size)
{
    bind_stream += (UInt32)base;

    UInt8 immediate = 0;
    UInt8 opcode = 0;
    UInt8 type = BIND_TYPE_POINTER;

    UInt32 segmentAddress = 0;

    UInt32 address = 0;

    SInt32 addend = 0;
    SInt32 libraryOrdinal = 0;

    const char* symbolName = NULL;
    UInt8 symboFlags = 0;
    UInt32 symbolAddr = 0xFFFFFFFF;

    // Temperary variables
    UInt32 tmp = 0;
    UInt32 tmp2 = 0;

    UInt32 index = 0;
    unsigned int i = 0;

    while(i < size)
    {
        immediate = bind_stream[i] & BIND_IMMEDIATE_MASK;
        opcode = bind_stream[i] & BIND_OPCODE_MASK;


        switch(opcode)
        {
            case BIND_OPCODE_DONE:
                // reset vars
                type = BIND_TYPE_POINTER;
                segmentAddress = 0;
                address = 0;
                addend = 0;
                libraryOrdinal = 0;
                symbolAddr = 0xFFFFFFFF;
                break;

            case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
                libraryOrdinal = immediate;
                break;

            case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
                libraryOrdinal = read_uleb(bind_stream, &i);
                break;

            case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
                libraryOrdinal = immediate ? (SInt8)(BIND_OPCODE_MASK | immediate) : immediate;
                break;

            case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
                symboFlags = immediate;
                symbolName = (char*)&bind_stream[++i];
                i += strlen((char*)&bind_stream[i]);

                symbolAddr = lookup_all_symbols(symbolName);
                break;

            case BIND_OPCODE_SET_TYPE_IMM:
                type = immediate;
                break;

            case BIND_OPCODE_SET_ADDEND_SLEB:
                addend = read_uleb(bind_stream, &i);
                if(!(bind_stream[i-1] & 0x40)) addend *= -1;
                break;

            case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                segmentAddress = 0;

                // Locate address
                struct segment_command* segCommand = NULL;    // NOTE: 32bit only

                unsigned int binIndex = 0;
                index = 0;
                do
                {
                    segCommand = base + sizeof(struct mach_header) +  binIndex;
                    binIndex += segCommand->cmdsize;
                    index++;
                }
                while(index <= immediate);

                segmentAddress = segCommand->fileoff;

                segmentAddress += read_uleb(bind_stream, &i);
                break;

            case BIND_OPCODE_ADD_ADDR_ULEB:
                segmentAddress += read_uleb(bind_stream, &i);
                break;

            case BIND_OPCODE_DO_BIND:
                if(symbolAddr != 0xFFFFFFFF)
                {
                    address = segmentAddress + (UInt32)new_base;

                    bind_location((UInt32*)address, (char*)symbolAddr, addend, type);
                }
                else
                {
                    printf("Unable to bind symbol %s\n", symbolName);
                    getchar();
                }

                segmentAddress += sizeof(void*);
                break;

            case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
                // Read in offset
                tmp  = read_uleb(bind_stream, &i);

                if(symbolAddr != 0xFFFFFFFF)
                {
                    address = segmentAddress + (UInt32)new_base;

                    bind_location((UInt32*)address, (char*)symbolAddr, addend, type);
                }
                else
                {
                    printf("Unable to bind symbol %s\n", symbolName);
                    getchar();
                }

                segmentAddress += tmp + sizeof(void*);


                break;

            case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
                if(symbolAddr != 0xFFFFFFFF)
                {
                    address = segmentAddress + (UInt32)new_base;

                    bind_location((UInt32*)address, (char*)symbolAddr, addend, type);
                }
                else
                {
                    printf("Unable to bind symbol %s\n", symbolName);
                    getchar();
                }
                segmentAddress += (immediate * sizeof(void*)) + sizeof(void*);


                break;

            case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
                tmp  = read_uleb(bind_stream, &i);

                tmp2  = read_uleb(bind_stream, &i);

                if(symbolAddr != 0xFFFFFFFF)
                {
                    for(index = 0; index < tmp; index++)
                    {

                        address = segmentAddress + (UInt32)new_base;
                        bind_location((UInt32*)address, (char*)symbolAddr, addend, type);
                        segmentAddress += tmp2 + sizeof(void*);
                    }
                }
                else
                {
                    printf("Unable to bind symbol %s\n", symbolName);
                    getchar();
                }
                break;
            default:
                break;

        }
        i++;
    }
}

static inline void rebase_location(UInt32* location, char* base, int type)
{
    DBG("Rebasing locaiton 0x%X with base 0x%X, type = %d\n", (UInt32)location - (UInt32)base, base, type);
    //DBGPAUSE();
    switch(type)
    {
        case REBASE_TYPE_POINTER:
        case REBASE_TYPE_TEXT_ABSOLUTE32:
            *location += (UInt32)base;
            break;

        default:
            break;
    }
}


static inline void bind_location(UInt32* location, char* value, UInt32 addend, int type)
{
    // do actual update
    char* newValue = value + addend;

    switch (type) {
        case BIND_TYPE_POINTER:
        case BIND_TYPE_TEXT_ABSOLUTE32:
            break;

        case BIND_TYPE_TEXT_PCREL32:
            newValue -=  ((UInt32)location + 4);

            break;
        default:
            return;
    }
    //DBG("Binding 0x%X to 0x%X (was 0x%X)\n", location, newValue, *location);
    *location = (UInt32)newValue;
}
