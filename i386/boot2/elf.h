/*
 * Module Loading functionality
 * Copyright 2017 Evan Lojewski. All rights reserved.
 *
 */
#include <saio_types.h>

#ifndef __BOOT_ELF_H
#define __BOOT_ELF_H

#define ELF_START_SYMBOL    "_start"

typedef UInt32          Elf32_Addr;
typedef UInt16          Elf32_Half;
typedef UInt32          Elf32_Off;
typedef SInt32          Elf32_Sword;
typedef UInt32          Elf32_Word;

#define EI_NIDENT 16
typedef struct {
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} Elf32_Ehdr;

/* Elf32_Ehdr.e_ident */
#define EI_MAG0     (0)
#define EI_MAG1     (1)
#define EI_MAG2     (2)
#define EI_MAG3     (3)
#define EI_CLASS    (4)
#define EI_DATA     (5)
#define EI_VERSION  (6)
#define EI_PAD      (7)

/* Elf32_Ehdr.e_ident[EI_MAG0:EI_MAG3] */
#define ELFMAG0     (0x7F)
#define ELFMAG1     ('E')
#define ELFMAG2     ('L')
#define ELFMAG3     ('F')

/* Elf32_Ehdr.e_ident[EI_CLASS] */
#define ELFCLASSNONE    (0)
#define ELFCLASS32      (1)
#define ELFCLASS64      (2)

/* Elf32_Ehdr.e_ident[EI_DATA] */
#define ELFDATANONE     (0)
#define ELFDATA2LSB     (1)
#define ELFDATA2MSB     (2)

/* Elf32_Ehdr.etype */
#define ET_NONE     (0)
#define ET_REL      (1)
#define ET_EXEC     (2)
#define ET_DYN      (3)
#define ET_CORE     (4)
#define ET_LOPROC   (0xff00)
#define ET_HIPROC   (0xffff)

/* Elf32_Ehdr.e_machine */
#define EM_NONE     (0)
#define EM_M32      (1)
#define EM_SPARC    (2)
#define EM_386      (3)
#define EM_68K      (4)
#define EM_88K      (5)
#define EM_860      (7)
#define EM_MIPS     (8)

/* Elf32_Ehdr.e_version */
#define EV_NONE     (0)
#define EV_CURRENT  (1)


typedef struct {
    Elf32_Word      sh_name;
    Elf32_Word      sh_type;
    Elf32_Word      sh_flags;
    Elf32_Addr      sh_addr;
    Elf32_Off       sh_offset;
    Elf32_Word      sh_size;
    Elf32_Word      sh_link;
    Elf32_Word      sh_info;
    Elf32_Word      sh_addralign;
    Elf32_Word      sh_entsize;
} Elf32_Shdr;

#define SHN_UNDEF       (0)
#define SHN_LORESERVE   (0xff00)
#define SHN_LOPROC      (0xff00)
#define SHN_HIPROC      (0xff1f)
#define SHN_ABS         (0xfff1)
#define SHN_COMMON      (0xfff2)
#define SHN_HIRESERVE   (0xffff)

/* Elf32_Shdr.sh_type */
#define SHT_NULL        (0)
#define SHT_PROGBITS    (1)
#define SHT_SYMTAB      (2)
#define SHT_STRTAB      (3)
#define SHT_RELA        (4)
#define SHT_HASH        (5)
#define SHT_DYNAMIC     (6)
#define SHT_NOTE        (7)
#define SHT_NOBITS      (8)
#define SHT_REL         (9)
#define SHT_SHLIB       (10)
#define SHT_DYNSYM      (11)
#define SHT_LOPROC      (0x70000000)
#define SHT_HIPROC      (0x7fffffff)
#define SHT_LOUSER      (0x80000000)
#define SHT_HIUSER      (0xffffffff)

/* Elf32_Shdr.sh_flags */
#define SHF_WRITE       (0x1)
#define SHF_ALLOC       (0x2)
#define SHF_EXECINSTR   (0x4)
#define SHF_MASKPROC    (0xf0000000)

typedef struct {
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    unsigned char st_info;
    unsigned char st_other;
    Elf32_Half st_shndx;
} Elf32_Sym;

#define ELF32_ST_BIND(i)    ((i)>>4)
#define STB_LOCAL           (0)
#define STB_GLOBAL          (1)
#define STB_WEAK            (2)
#define STB_LOPROC          (13)
#define STB_HIPROC          (15)

#define ELF32_ST_TYPE(i)    ((i)&0xf)
#define STT_NOTYPE          (0)
#define STT_OBJECT          (1)
#define STT_FUNC            (2)
#define STT_SECTION         (3)
#define STT_FILE            (4)
#define STT_LOPROC          (13)
#define STT_HIPROC          (25)

#define ELF32_ST_INFO(b,t)  (((b)<<4)+((t)&0xf))

typedef struct {
    Elf32_Addr r_offset;
    Elf32_Word r_info;
} Elf32_Rel;

typedef struct {
    Elf32_Addr r_offset;
    Elf32_Word r_info;
    Elf32_Sword r_addend;
} Elf32_Rela;

#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

/*
 * A = 
 * B = base address
 * S = value of symbol
 * P = location / PC of item being relocated
 * L = location of procedure linking table
 * GOT = addr of global offset table
 * G = offset into global offset table
 */
#define R_386_NONE          (0)     /* None */
#define R_386_32            (1)     /* S + A */
#define R_386_PC32          (2)     /* S + A - P */
#define R_386_GOT32         (3)     /* G + A - P */
#define R_386_PLT32         (4)     /* L + A - P */
#define R_386_COPY          (5)     /* None */
#define R_386_GLOB_DAT      (6)     /* S */
#define R_386_JMP_SLOT      (7)     /* S */
#define R_386_RELATIVE      (8)     /* B + A */
#define R_386_GOTOFF        (9)     /* S + A - GOT */
#define R_386_GOTPC         (10)    /* GOT + A - P */


typedef struct {
    Elf32_Word p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} Elf32_Phdr;

#define PT_NULL     (0)
#define PT_LOAD     (1)
#define PT_DYNAMIC  (2)
#define PT_INTERP   (3)
#define PT_NOTE     (4)
#define PT_SHLIB    (5)
#define PT_PHDR     (6)
#define PT_LOPROC   (0x70000000)
#define PT_HIPROC   (0x7fffffff)

typedef struct {
    Elf32_Sword     d_tag;
    union {
        Elf32_Word  d_val;
        Elf32_Addr  d_ptr;
    } d_un;
} Elf32_Dyn;

/* Elf32_Dyn.d_tag */
#define DT_NULL         (0)
#define DT_NEEDED       (1)
#define DT_PLTRELSZ     (2)
#define DT_PLTGOT       (3)
#define DT_HASH         (4)
#define DT_STRTAB       (5)
#define DT_SYMTAB       (6)
#define DT_RELA         (7)
#define DT_RELASZ       (8)
#define DT_RELAENT      (9)
#define DT_STRSZ        (10)
#define DT_SYMENT       (11)
#define DT_INIT         (12)
#define DT_FINI         (13)
#define DT_SONAME       (14)
#define DT_RPATH        (15)
#define DT_SYMBOLIC     (16)
#define DT_REL          (17)
#define DT_RELSZ        (18)
#define DT_RELENT       (19)
#define DT_PLTREL       (20)
#define DT_DEBUG        (21)
#define DT_TEXTREL      (22)
#define DT_JMPREL       (23)
#define DT_LOPROC       (0x70000000)
#define DT_HIPROC       (0x7fffffff)


/********************************************************************************/
/*    Macho Parser                                                                */
/********************************************************************************/
UInt32 pre_parse_elf(void* binary);

bool            parse_elf(void* binary, void* base,
                            int(*dylib_loader)(char*, UInt32 compat),
                            void (*symbol_handler)(char*, long long, char),
                            void (*section_handler)(char* base, char* new_base, char* section, char* segment, void* cmd, UInt64 offset, UInt64 address)
                           );
bool            handle_elf_symtable(UInt32 base, UInt32 new_base,
                             struct symtab_command* symtabCommand,
                             void (*symbol_handler)(char*, long long, char),
                             char is64);
void            rebase_elf(void* base, void* new_base, UInt8* rebase_stream, UInt32 size);

void            bind_elf(void* base, void* new_base, UInt8* bind_stream, UInt32 size);

#endif /* __BOOT_ELF_H */
