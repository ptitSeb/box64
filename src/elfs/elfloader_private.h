#ifndef __ELFLOADER_PRIVATE_H_
#define __ELFLOADER_PRIVATE_H_

//#ifdef DYNAREC
//typedef struct dynablocklist_s dynablocklist_t;
//#endif

//typedef struct library_s library_t;
//typedef struct needed_libs_s needed_libs_t;

#include <pthread.h>

struct elfheader_s {
    char*       name;
    char*       path;   // Resolved path to file
    int         numPHEntries;
    Elf64_Phdr  *PHEntries;
    int         numSHEntries;
    Elf64_Shdr  *SHEntries;
    int         SHIdx;
    int         numSST;
    char*       SHStrTab;
    char*       StrTab;
    Elf64_Sym*  SymTab;
    int         numSymTab;
    char*       DynStr;
    Elf64_Sym*  DynSym;
    int         numDynSym;
    Elf64_Dyn*  Dynamic;
    int         numDynamic;
    char*       DynStrTab;
    int         szDynStrTab;
    int         e_type;

    intptr_t    delta;  // should be 0

    uintptr_t   entrypoint;
    uintptr_t   initentry;
    uintptr_t   initarray;
    int         initarray_sz;
    uintptr_t   finientry;
    uintptr_t   finiarray;
    int         finiarray_sz;

    uintptr_t   rel;
    int         relsz;
    int         relent;
    uintptr_t   rela;
    int         relasz;
    int         relaent;
    uintptr_t   jmprel;
    int         pltsz;
    int         pltent;
    uint64_t    pltrel;
    uintptr_t   gotplt;
    uintptr_t   gotplt_end;
    uintptr_t   pltgot;
    uintptr_t   got;
    uintptr_t   got_end;
    uintptr_t   plt;
    uintptr_t   plt_end;
    uintptr_t   text;
    int         textsz;

    uintptr_t   paddr;
    uintptr_t   vaddr;
    int         align;
    uint64_t    memsz;
    uint64_t    stacksz;
    int         stackalign;
    uint32_t    tlssize;
    int         tlsalign;

    int32_t     tlsbase;    // the base of the tlsdata in the global tlsdata (always negative)

    int         init_done;
    int         fini_done;

    char*       memory; // char* and not void* to allow math on memory pointer
    void**      multiblock;
    uintptr_t*  multiblock_offs;
    uint64_t*   multiblock_size;
    int         multiblock_n;

    //library_t   *lib;
    //needed_libs_t *neededlibs;
};

#define R_X86_64_NONE		0	/* No reloc */
#define R_X86_64_64		1	/* Direct 64 bit  */
#define R_X86_64_PC32		2	/* PC relative 32 bit signed */
#define R_X86_64_GOT32		3	/* 32 bit GOT entry */
#define R_X86_64_PLT32		4	/* 32 bit PLT address */
#define R_X86_64_COPY		5	/* Copy symbol at runtime */
#define R_X86_64_GLOB_DAT	6	/* Create GOT entry */
#define R_X86_64_JUMP_SLOT	7	/* Create PLT entry */
#define R_X86_64_RELATIVE	8	/* Adjust by program base */
#define R_X86_64_GOTPCREL	9	/* 32 bit signed pc relative offset to GOT */
#define R_X86_64_32		10	/* Direct 32 bit zero extended */
#define R_X86_64_32S		11	/* Direct 32 bit sign extended */
#define R_X86_64_16		12	/* Direct 16 bit zero extended */
#define R_X86_64_PC16		13	/* 16 bit sign extended pc relative */
#define R_X86_64_8		14	/* Direct 8 bit sign extended  */
#define R_X86_64_PC8		15	/* 8 bit sign extended pc relative */
#define R_X86_64_PC64		24	/* Place relative 64-bit signed */

elfheader_t* ParseElfHeader(FILE* f, const char* name, int exec);

#endif //__ELFLOADER_PRIVATE_H_