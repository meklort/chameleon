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
#define CONFIG_MODULE_DEBUG 1
#endif


#if CONFIG_MODULE_DEBUG
#define DBG(x...)    printf(x)
#define DBGPAUSE()    getchar()
#else
#define DBG(x...)
#define DBGPAUSE()
#endif


extern UInt32 gVersion;
extern UInt32 gCompat;

static inline void        rebase_location(UInt32* location, char* base, int type);
static inline void        bind_location(UInt32* location, char* value, UInt32 addend, int type);

/********************************************************************************/
/*    Elf32 Parser                                                                */
/********************************************************************************/

/*
 * Parse the e;f module and determine the total ammount of memory needed for it.
 *
 */
UInt32 pre_parse_elf(void* binary)
{
    Elf32_Ehdr* ehdr = binary;
    
    Elf32_Shdr* Shdr = ehdr->e_shoff ? (binary + ehdr->e_shoff) : NULL;
    Elf32_Half  Shdr_size = ehdr->e_shentsize;
    Elf32_Half  Shdr_count = ehdr->e_shnum;
    
    Elf32_Phdr* Phdr = ehdr->e_phoff ? (binary + ehdr->e_phoff) : NULL;
    Elf32_Half  Phdr_size = ehdr->e_phentsize;
    Elf32_Half  Phdr_count = ehdr->e_phnum;

    UInt32 size = 0;

    // Parse through the load commands
    if((ehdr->e_ident[EI_MAG0] != ELFMAG0) ||
       (ehdr->e_ident[EI_MAG1] != ELFMAG1) ||
       (ehdr->e_ident[EI_MAG2] != ELFMAG2) ||
       (ehdr->e_ident[EI_MAG3] != ELFMAG3)
       )
    {
        // Invalid
        return 0;
    }
    
    // Loop through all phdrs and determine to total size needed.
    while(Phdr_count)
    {
        if(PT_LOAD == Phdr->p_type)
        {
            size += Phdr->p_memsz;
        }

        Phdr = (void*)Phdr + Phdr_size;
        Phdr_count--;
    }
    
    
    DBG("Elf found, requires 0x%X bytes\n", size);
    
    return size;
}

/*
 * Parse through a macho module. The module will be rebased and binded
 * as specified in the macho header.
 * NOTE; all dependecies will be loaded before this module is started
 * NOTE: If the module is unable to load ot completeion, the modules
 * symbols will still be available.
 */
bool parse_elf(void* binary, void* base,
                 int(*dylib_loader)(char*, UInt32 compat),
                 void (*symbol_handler)(char*, long long, char),
                 void (*section_handler)(char* base, char* new_base, char* section, char* segment, void* cmd, UInt64 offset, UInt64 address)
)
{
    Elf32_Ehdr* ehdr = binary;
    
    Elf32_Shdr* Shdr = ehdr->e_shoff ? (binary + ehdr->e_shoff) : NULL;
    Elf32_Half  Shdr_size = ehdr->e_shentsize;
    Elf32_Half  Shdr_count = ehdr->e_shnum;
    
    Elf32_Phdr* Phdr = ehdr->e_phoff ? (binary + ehdr->e_phoff) : NULL;
    Elf32_Half  Phdr_size = ehdr->e_phentsize;
    Elf32_Half  Phdr_count = ehdr->e_phnum;
    
    Elf32_Half strsection = ehdr->e_shstrndx;
    
    char is64 = false;

    UInt32 binaryIndex = 0;
    UInt16 cmd = 0;

    // Parse through the load commands
    if((ehdr->e_ident[EI_MAG0] != ELFMAG0) ||
       (ehdr->e_ident[EI_MAG1] != ELFMAG1) ||
       (ehdr->e_ident[EI_MAG2] != ELFMAG2) ||
       (ehdr->e_ident[EI_MAG3] != ELFMAG3)
       )
    {
        // Invalid
        DBG("Invalid elf magic 0x%X\n", ((UInt32*)binary)[0]);
        return NULL;
    }
    else if(ehdr->e_ident[EI_CLASS] == ELFCLASS32)
    {
        is64 = false;
        binaryIndex += sizeof(Elf32_Ehdr);
    }
    else if(ehdr->e_ident[EI_CLASS] == ELFCLASS64)
    {
        is64 = true;
    }
    
    // Loop through all phdrs, copying data to allocated region.
    while(Phdr_count)
    {
        Elf32_Word copysize = Phdr->p_filesz;
        Elf32_Word copyoffset = 0;
        void* dest = base + Phdr->p_vaddr;
        void* source = binary + Phdr->p_offset;

        if(PT_LOAD == Phdr->p_type)
        {
            memset(dest, 0, copysize);
            memcpy(dest, source, copysize);
        }

        Phdr = (void*)Phdr + Phdr_size;
        Phdr_count--;
    }
    
    // Locate the string table.
    char* strings;
    Elf32_Shdr* Stable = ehdr->e_shoff ? (binary + ehdr->e_shoff) : NULL;
    if(Stable && strsection)
    {
        Stable = ((void*)Stable + (strsection) * Shdr_size);
        DBG("Found string table at section %d, 0x%X\n", strsection, Stable->sh_offset);
        strings = (binary + Stable->sh_offset);
    }

    char* dynstr = NULL;
    Elf32_Sym* dynsym = NULL;
    UInt32 symsize = 0;
    // Loop through all shdrs
    while(Shdr_count)
    {
        //DBG("Parsing section %d: %s\n", Shdr->sh_name, &strings[Shdr->sh_name]);

        switch(Shdr->sh_type)
        {
            case SHT_DYNSYM:
            {
                char* symnames;
                Elf32_Shdr* stringtable = ehdr->e_shoff ? (binary + ehdr->e_shoff) : NULL;
                stringtable = (void*)stringtable + (Shdr->sh_link * Shdr_size);
                symnames = (binary + stringtable->sh_offset);

                UInt32 entries = Shdr->sh_size / Shdr->sh_entsize;
                DBG("Parsing %d symbols\n", entries);
                Elf32_Sym* symbols = binary + Shdr->sh_offset;

                dynsym = symbols;
                symsize = Shdr->sh_entsize;
                dynstr = symnames;

                while(entries)
                {
                    if(symbols->st_shndx && // Defined
                       STB_GLOBAL == ELF32_ST_BIND(symbols->st_info)) // And exported
                    {
                        symbol_handler(&symnames[symbols->st_name], (long long)base + symbols->st_value, is64);
                    }
                    
                    symbols = (void*)symbols + Shdr->sh_entsize;
                    entries--;
                }
                
                break;
            }

            case SHT_REL:
            {
                UInt32 entries = Shdr->sh_size / Shdr->sh_entsize;
                Elf32_Rel* dyns = binary + Shdr->sh_offset;
                while(entries)
                {
                    switch(ELF32_R_TYPE(dyns->r_info))
                    {
                        case R_386_RELATIVE:
                        {
                            UInt32 sym = ELF32_R_SYM(dyns->r_info); // must be 0.
                            UInt32 offset = dyns->r_offset;
                            
                            UInt32* data = base;
                            
                            /* A = A + B */
                            data[offset] += (UInt32)base;
                            
                            break;
                        }
                            
                        case R_386_PC32:
                        {
                            UInt32 symbolAddr;
                            UInt32 offset = dyns->r_offset;
                            UInt32* data;

                            UInt32 id = ELF32_R_SYM(dyns->r_info); // must be 0.
                            Elf32_Sym* sym = (void*)dynsym + (symsize * id);
                            char* name = &dynstr[sym->st_name];
                            
                            symbolAddr = lookup_all_symbols(name);
                            if(symbolAddr == lookup_all_symbols(VOID_SYMBOL))
                            {
                                printf("Unable to find symbol '%s', using '%s' instead\n", name, VOID_SYMBOL);
                                pause();
                            }
                            /* A = S + A - P */
                            data = (void*)base + offset;
                            *data += symbolAddr - (UInt32)data;
                            break;
                        }
                          
                        case R_386_32:
                        {
                            UInt32 symbolAddr;
                            UInt32 offset = dyns->r_offset;
                            UInt32* data;
                            
                            UInt32 id = ELF32_R_SYM(dyns->r_info); // must be 0.
                            Elf32_Sym* sym = (void*)dynsym + (symsize * id);
                            char* name = &dynstr[sym->st_name];
                            
                            symbolAddr = lookup_all_symbols(name);
                            if(symbolAddr == lookup_all_symbols(VOID_SYMBOL))
                            {
                                printf("Unable to find symbol '%s', using '%s' instead\n", name, VOID_SYMBOL);
                                pause();
                            }
                            
                            /* A = S + A */
                            data = base + offset;
                            *data += symbolAddr;
                            break;
                        }
                            
                        default:
                        {
                            printf("Unhandeled relocation entry 0x%x\n", ELF32_R_TYPE(dyns->r_info));
                            break;
                        }
                            
                            
                    }
                    dyns = (void*)dyns + Shdr->sh_entsize;
                    entries--;
                }
                DBG("Rel found, unsupporteds\n");
                break;
            }

            case SHT_RELA:
            {
                DBG("Rela found, unsupporteds\n");
                break;
            }
                
            case SHT_DYNAMIC:
            {
                UInt32 entries = Shdr->sh_size / Shdr->sh_entsize;
                Elf32_Dyn* dyns = binary + Shdr->sh_offset;
                
                DBG("Linking information\n");
                break;
            }
        }
        
        Shdr = (void*)Shdr + Shdr_size;
        Shdr_count--;
    }

    return true;

}

/*
 * parse the symbol table
 * Lookup any undefined symbols
 */

bool handle_elf_symtable(UInt32 base, UInt32 new_base,
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

UInt64 elf_read_uleb(UInt8* bind_stream, unsigned int* i)
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
void rebase_elf(void* base, void* new_base, UInt8* rebase_stream, UInt32 size)
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
                tmp = elf_read_uleb(rebase_stream, &i);

                segmentAddress += tmp;
		DBG("REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB(%d,0x%X)\n", immediate, tmp);
                break;

            case REBASE_OPCODE_ADD_ADDR_ULEB:
                // Add value to rebase address
                tmp = elf_read_uleb(rebase_stream, &i);

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
                tmp = elf_read_uleb(rebase_stream, &i);

		DBG("REBASE_OPCODE_DO_REBASE_ULEB_TIMES(%d)\n", tmp);
                index = 0;
                for (index = 0; index < tmp; ++index) {
                    //DBG("\tRebasing 0x%X\n", segmentAddress);
                    rebase_location(new_base + segmentAddress, (char*)new_base, type);
                    segmentAddress += sizeof(void*);
                }
                break;

            case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
                tmp = elf_read_uleb(rebase_stream, &i);
		DBG("REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB(%d)\n", tmp + sizeof(void*));

                rebase_location(new_base + segmentAddress, (char*)new_base, type);

                segmentAddress += tmp + sizeof(void*);
                break;

            case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
                tmp = elf_read_uleb(rebase_stream, &i);
                tmp2 = elf_read_uleb(rebase_stream, &i);
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
void bind_elf(void* base, void* new_base, UInt8* bind_stream, UInt32 size)
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
                libraryOrdinal = elf_read_uleb(bind_stream, &i);
                break;

            case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
                libraryOrdinal = immediate ? (SInt8)(BIND_OPCODE_MASK | immediate) : immediate;
                break;

            case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
                symboFlags = immediate;
                symbolName = (char*)&bind_stream[++i];
                i += strlen((char*)&bind_stream[i]);

                symbolAddr = lookup_all_symbols(symbolName);
		if(symbolAddr == lookup_all_symbols(VOID_SYMBOL))
		{
		    printf("Unable to find symbol '%s', using '%s' instead\n", symbolName, VOID_SYMBOL);
		    pause();
		}
                break;

            case BIND_OPCODE_SET_TYPE_IMM:
                type = immediate;
                break;

            case BIND_OPCODE_SET_ADDEND_SLEB:
                addend = elf_read_uleb(bind_stream, &i);
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

                segmentAddress += elf_read_uleb(bind_stream, &i);
                break;

            case BIND_OPCODE_ADD_ADDR_ULEB:
                segmentAddress += elf_read_uleb(bind_stream, &i);
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
                tmp  = elf_read_uleb(bind_stream, &i);

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
                tmp  = elf_read_uleb(bind_stream, &i);

                tmp2  = elf_read_uleb(bind_stream, &i);

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
