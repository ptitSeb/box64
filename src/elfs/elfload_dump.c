#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

#include "box64version.h"
#include "elfloader.h"
#include "debug.h"
#include "elfload_dump.h"
#include "elfloader_private.h"

#ifndef SHT_CHECKSUM
#define SHT_CHECKSUM 0x6ffffff8
#endif

const char* DumpSection(Elf64_Shdr *s, char* SST) {
    static char buff[400];
    switch (s->sh_type) {
        case SHT_NULL:
            return "SHT_NULL";
        #define GO(A) \
        case A:     \
            sprintf(buff, #A " Name=\"%s\"(%d) off=0x%lX, size=%ld, attr=0x%04lX, addr=%p(%02lX), link/info=%d/%d", \
                SST+s->sh_name, s->sh_name, s->sh_offset, s->sh_size, s->sh_flags, (void*)s->sh_addr, s->sh_addralign, s->sh_link, s->sh_info); \
            break
        GO(SHT_PROGBITS);
        GO(SHT_SYMTAB);
        GO(SHT_STRTAB);
        GO(SHT_RELA);
        GO(SHT_HASH);
        GO(SHT_DYNAMIC);
        GO(SHT_NOTE);
        GO(SHT_NOBITS);
        GO(SHT_REL);
        GO(SHT_SHLIB);
        GO(SHT_DYNSYM);
        GO(SHT_INIT_ARRAY);
        GO(SHT_FINI_ARRAY);
        GO(SHT_PREINIT_ARRAY);
        GO(SHT_GROUP);
        GO(SHT_SYMTAB_SHNDX);
        GO(SHT_NUM);
        GO(SHT_LOPROC);
        GO(SHT_HIPROC);
        GO(SHT_LOUSER);
        GO(SHT_HIUSER);
        #if defined(SHT_GNU_versym) && defined(SHT_GNU_ATTRIBUTES)
        GO(SHT_GNU_versym);
        GO(SHT_GNU_ATTRIBUTES);
        GO(SHT_GNU_HASH);
        GO(SHT_GNU_LIBLIST);
        GO(SHT_CHECKSUM);
        GO(SHT_LOSUNW);
        //GO(SHT_SUNW_move);
        GO(SHT_SUNW_COMDAT);
        GO(SHT_SUNW_syminfo);
        GO(SHT_GNU_verdef);
        GO(SHT_GNU_verneed);
        #endif
        #undef GO
        default:
            sprintf(buff, "0x%X unknown type", s->sh_type);
    }
    return buff;
}

const char* DumpDynamic(Elf64_Dyn *s) {
    static char buff[200];
    switch (s->d_tag) {
        case DT_NULL:
            return "DT_NULL: End Dynamic Section";
        #define GO(A, Add) \
        case A:     \
            sprintf(buff, #A " %s=0x%lX", (Add)?"Addr":"Val", (Add)?s->d_un.d_ptr:s->d_un.d_val); \
            break
            GO(DT_NEEDED, 0);
            GO(DT_PLTRELSZ, 0);
            GO(DT_PLTGOT, 1);
            GO(DT_HASH, 1);
            GO(DT_STRTAB, 1);
            GO(DT_SYMTAB, 1);
            GO(DT_RELA, 1);
            GO(DT_RELASZ, 0);
            GO(DT_RELAENT, 0);
            GO(DT_STRSZ, 0);
            GO(DT_SYMENT, 0);
            GO(DT_INIT, 1);
            GO(DT_FINI, 1);
            GO(DT_SONAME, 0);
            GO(DT_RPATH, 0);
            GO(DT_SYMBOLIC, 0);
            GO(DT_REL, 1);
            GO(DT_RELSZ, 0);
            GO(DT_RELENT, 0);
            GO(DT_PLTREL, 0);
            GO(DT_DEBUG, 0);
            GO(DT_TEXTREL, 0);
            GO(DT_JMPREL, 1);
            GO(DT_BIND_NOW, 1);
            GO(DT_INIT_ARRAY, 1);
            GO(DT_FINI_ARRAY, 1);
            GO(DT_INIT_ARRAYSZ, 0);
            GO(DT_FINI_ARRAYSZ, 0);
            GO(DT_RUNPATH, 0);
            GO(DT_FLAGS, 0);
            GO(DT_ENCODING, 0);
            #if defined(DT_NUM) && defined(DT_TLSDESC_PLT)
            GO(DT_NUM, 0);
            GO(DT_VALRNGLO, 0);
            GO(DT_GNU_PRELINKED, 0);
            GO(DT_GNU_CONFLICTSZ, 0);
            GO(DT_GNU_LIBLISTSZ, 0);
            GO(DT_CHECKSUM, 0);
            GO(DT_PLTPADSZ, 0);
            GO(DT_MOVEENT, 0);
            GO(DT_MOVESZ, 0);
            GO(DT_FEATURE_1, 0);
            GO(DT_POSFLAG_1, 0);
            GO(DT_SYMINSZ, 0);
            GO(DT_SYMINENT, 0);
            GO(DT_ADDRRNGLO, 0);
            GO(DT_GNU_HASH, 0);
            GO(DT_TLSDESC_PLT, 0);
            GO(DT_TLSDESC_GOT, 0);
            GO(DT_GNU_CONFLICT, 0);
            GO(DT_GNU_LIBLIST, 0);
            GO(DT_CONFIG, 0);
            GO(DT_DEPAUDIT, 0);
            GO(DT_AUDIT, 0);
            GO(DT_PLTPAD, 0);
            GO(DT_MOVETAB, 0);
            GO(DT_SYMINFO, 0);
            GO(DT_VERSYM, 0);
            GO(DT_RELACOUNT, 0);
            GO(DT_RELCOUNT, 0);
            GO(DT_FLAGS_1, 0);
            GO(DT_VERDEF, 0);
            GO(DT_VERDEFNUM, 0);
            GO(DT_VERNEED, 0);
            GO(DT_VERNEEDNUM, 0);
            GO(DT_AUXILIARY, 0);
            GO(DT_FILTER, 0);
            #endif
        #undef GO
        default:
            sprintf(buff, "0x%lX unknown type", s->d_tag);
    }
    return buff;
}

const char* DumpPHEntry(Elf64_Phdr *e)
{
    static char buff[500];
    memset(buff, 0, sizeof(buff));
    switch(e->p_type) {
        case PT_NULL: sprintf(buff, "type: %s", "PT_NULL"); break;
        #define GO(T) case T: sprintf(buff, "type: %s, Off=%lx vaddr=%p paddr=%p filesz=%lu memsz=%lu flags=%x align=%lu", #T, e->p_offset, (void*)e->p_vaddr, (void*)e->p_paddr, e->p_filesz, e->p_memsz, e->p_flags, e->p_align); break
        GO(PT_LOAD);
        GO(PT_DYNAMIC);
        GO(PT_INTERP);
        GO(PT_NOTE);
        GO(PT_SHLIB);
        GO(PT_PHDR);
        GO(PT_TLS);
        #ifdef PT_NUM
        GO(PT_NUM);
        GO(PT_LOOS);
        GO(PT_GNU_EH_FRAME);
        GO(PT_GNU_STACK);
        GO(PT_GNU_RELRO);
        #endif
        #undef GO
        default: sprintf(buff, "type: %x, Off=%lx vaddr=%p paddr=%p filesz=%lu memsz=%lu flags=%x align=%lu", e->p_type, e->p_offset, (void*)e->p_vaddr, (void*)e->p_paddr, e->p_filesz, e->p_memsz, e->p_flags, e->p_align); break;
    }
    return buff;
}

const char* DumpRelType(int t)
{
    static char buff[50];
    memset(buff, 0, sizeof(buff));
    switch(t) {
        #define GO(T) case T: sprintf(buff, "type: %s", #T); break
        GO(R_X86_64_NONE);
        GO(R_X86_64_64);
        GO(R_X86_64_PC32);
        GO(R_X86_64_GOT32);
        GO(R_X86_64_PLT32);
        GO(R_X86_64_COPY);
        GO(R_X86_64_GLOB_DAT);
        GO(R_X86_64_JUMP_SLOT);
        GO(R_X86_64_RELATIVE);
        GO(R_X86_64_GOTPCREL);
        GO(R_X86_64_32);
        GO(R_X86_64_32S);
        GO(R_X86_64_16);
        GO(R_X86_64_PC16);
        GO(R_X86_64_8);
        GO(R_X86_64_PC8);
        GO(R_X86_64_DTPMOD64);
        GO(R_X86_64_DTPOFF64);
        GO(R_X86_64_TPOFF64);
        GO(R_X86_64_TLSGD);
        GO(R_X86_64_TLSLD);
        GO(R_X86_64_DTPOFF32);
        GO(R_X86_64_GOTTPOFF);
        GO(R_X86_64_TPOFF32);
        GO(R_X86_64_PC64);
        GO(R_X86_64_GOTOFF64);
        GO(R_X86_64_GOTPC32);
        GO(R_X86_64_GOT64);
        GO(R_X86_64_GOTPCREL64);
        GO(R_X86_64_GOTPC64);
        GO(R_X86_64_GOTPLT64);
        GO(R_X86_64_PLTOFF64);
        GO(R_X86_64_SIZE32);
        GO(R_X86_64_SIZE64);
        GO(R_X86_64_GOTPC32_TLSDESC);
        GO(R_X86_64_TLSDESC_CALL);
        GO(R_X86_64_TLSDESC);
        GO(R_X86_64_IRELATIVE);
        GO(R_X86_64_RELATIVE64);
        GO(R_X86_64_GOTPCRELX);
        GO(R_X86_64_REX_GOTPCRELX);
        GO(R_X86_64_NUM);
        #undef GO
        default: sprintf(buff, "type: 0x%x (unknown)", t); break;
    }
    return buff;
}

const char* DumpSym(elfheader_t *h, Elf64_Sym* sym, int version)
{
    static char buff[4096];
    const char* vername = (version==-1)?"(none)":((version==0)?"*local*":((version==1)?"*global*":GetSymbolVersion(h, version)));
    int veropt = GetSymbolVersionFlag(h, version)?0:1;
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "\"%s\", value=%p, size=%ld, info/other=%d/%d index=%d (%sver=%d/%s)", 
        h->DynStr+sym->st_name, (void*)sym->st_value, sym->st_size,
        sym->st_info, sym->st_other, sym->st_shndx, veropt?"opt":"", version, vername);
    return buff;
}

const char* SymName(elfheader_t *h, Elf64_Sym* sym)
{
    return h->DynStr+sym->st_name;
}
const char* IdxSymName(elfheader_t *h, int sym)
{
    return h->DynStr+h->DynSym[sym].st_name;
}

void DumpMainHeader(Elf64_Ehdr *header, elfheader_t *h)
{
    if(box64_dump) {
        printf_dump(LOG_NEVER, "ELF Dump main header\n");
        printf_dump(LOG_NEVER, "  Entry point = %p\n", (void*)header->e_entry);
        printf_dump(LOG_NEVER, "  Program Header table offset = %p\n", (void*)header->e_phoff);
        printf_dump(LOG_NEVER, "  Section Header table offset = %p\n", (void*)header->e_shoff);
        printf_dump(LOG_NEVER, "  Flags = 0x%X\n", header->e_flags);
        printf_dump(LOG_NEVER, "  ELF Header size = %d\n", header->e_ehsize);
        printf_dump(LOG_NEVER, "  Program Header Entry num/size = %zu(%d)/%d\n", h->numPHEntries, header->e_phnum, header->e_phentsize);
        printf_dump(LOG_NEVER, "  Section Header Entry num/size = %zu(%d)/%d\n", h->numSHEntries, header->e_shnum, header->e_shentsize);
        printf_dump(LOG_NEVER, "  Section Header index num = %zu(%d)\n", h->SHIdx, header->e_shstrndx);
        printf_dump(LOG_NEVER, "ELF Dump ==========\n");

        printf_dump(LOG_NEVER, "ELF Dump PEntries (%zu)\n", h->numPHEntries);
        for (size_t i=0; i<h->numPHEntries; ++i)
            printf_dump(LOG_NEVER, "  PHEntry %04zu : %s\n", i, DumpPHEntry(h->PHEntries+i));
        printf_dump(LOG_NEVER, "ELF Dump PEntries ====\n");

        printf_dump(LOG_NEVER, "ELF Dump Sections (%zu)\n", h->numSHEntries);
        for (size_t i=0; i<h->numSHEntries; ++i)
            printf_dump(LOG_NEVER, "  Section %04zu : %s\n", i, DumpSection(h->SHEntries+i, h->SHStrTab));
        printf_dump(LOG_NEVER, "ELF Dump Sections ====\n");
    }
}

void DumpSymTab(elfheader_t *h)
{
    if(box64_dump && h->SymTab) {
        const char* name = ElfName(h);
        printf_dump(LOG_NEVER, "ELF Dump SymTab(%zu)=\n", h->numSymTab);
        for (size_t i=0; i<h->numSymTab; ++i)
            printf_dump(LOG_NEVER, "  %s:SymTab[%zu] = \"%s\", value=%p, size=%ld, info/other=%d/%d index=%d\n", name, 
                i, h->StrTab+h->SymTab[i].st_name, (void*)h->SymTab[i].st_value, h->SymTab[i].st_size,
                h->SymTab[i].st_info, h->SymTab[i].st_other, h->SymTab[i].st_shndx);
        printf_dump(LOG_NEVER, "ELF Dump SymTab=====\n");
    }
}

void DumpDynamicSections(elfheader_t *h)
{
    if(box64_dump && h->Dynamic) {
        printf_dump(LOG_NEVER, "ELF Dump Dynamic(%zu)=\n", h->numDynamic);
        for (size_t i=0; i<h->numDynamic; ++i)
            printf_dump(LOG_NEVER, "  Dynamic %04zu : %s\n", i, DumpDynamic(h->Dynamic+i));
        printf_dump(LOG_NEVER, "ELF Dump Dynamic=====\n");
    }
}

void DumpDynSym(elfheader_t *h)
{
    if(box64_dump && h->DynSym) {
        const char* name = ElfName(h);
        printf_dump(LOG_NEVER, "ELF Dump DynSym(%zu)=\n", h->numDynSym);
        for (size_t i=0; i<h->numDynSym; ++i) {
            int version = h->VerSym?((Elf64_Half*)((uintptr_t)h->VerSym+h->delta))[i]:-1;
            printf_dump(LOG_NEVER, "  %s:DynSym[%zu] = %s\n", name, i, DumpSym(h, h->DynSym+i, version));
        }
        printf_dump(LOG_NEVER, "ELF Dump DynSym=====\n");
    }
}

void DumpDynamicNeeded(elfheader_t *h)
{
    if(box64_dump && h->DynStrTab) {
        printf_dump(LOG_NEVER, "ELF Dump DT_NEEDED=====\n");
        for (size_t i=0; i<h->numDynamic; ++i)
            if(h->Dynamic[i].d_tag==DT_NEEDED) {
                printf_dump(LOG_NEVER, "  Needed : %s\n", h->DynStrTab+h->Dynamic[i].d_un.d_val + h->delta);
            }
        printf_dump(LOG_NEVER, "ELF Dump DT_NEEDED=====\n");
    }
}

void DumpDynamicRPath(elfheader_t *h)
{
    if(box64_dump && h->DynStrTab) {
        printf_dump(LOG_NEVER, "ELF Dump DT_RPATH/DT_RUNPATH=====\n");
        for (size_t i=0; i<h->numDynamic; ++i) {
            if(h->Dynamic[i].d_tag==DT_RPATH) {
                printf_dump(LOG_NEVER, "   RPATH : %s\n", h->DynStrTab+h->Dynamic[i].d_un.d_val + h->delta);
            }
            if(h->Dynamic[i].d_tag==DT_RUNPATH) {
                printf_dump(LOG_NEVER, " RUNPATH : %s\n", h->DynStrTab+h->Dynamic[i].d_un.d_val + h->delta);
            }
        }
        printf_dump(LOG_NEVER, "=====ELF Dump DT_RPATH/DT_RUNPATH\n");
    }
}

void DumpRelTable(elfheader_t *h, int cnt, Elf64_Rel *rel, const char* name)
{
    if(box64_dump) {
        const char* elfname = ElfName(h);
        printf_dump(LOG_NEVER, "ELF Dump %s Table(%d) @%p\n", name, cnt, rel);
        for (int i = 0; i<cnt; ++i)
            printf_dump(LOG_NEVER, "  %s:Rel[%d] = %p (0x%lX: %s, sym=0x%0lX/%s)\n", elfname,
                i, (void*)rel[i].r_offset, rel[i].r_info, DumpRelType(ELF64_R_TYPE(rel[i].r_info)), 
                ELF64_R_SYM(rel[i].r_info), IdxSymName(h, ELF64_R_SYM(rel[i].r_info)));
        printf_dump(LOG_NEVER, "ELF Dump Rel Table=====\n");
    }
}

void DumpRelATable(elfheader_t *h, int cnt, Elf64_Rela *rela, const char* name)
{
    if(box64_dump && h->rela) {
        const char* elfname = ElfName(h);
        printf_dump(LOG_NEVER, "ELF Dump %s Table(%d) @%p\n", name, cnt, rela);
        for (int i = 0; i<cnt; ++i)
            printf_dump(LOG_NEVER, "  %s:%s[%d] = %p (0x%lX: %s, sym=0x%lX/%s) Addend=0x%lx\n", elfname, name,
                i, (void*)rela[i].r_offset, rela[i].r_info, DumpRelType(ELF64_R_TYPE(rela[i].r_info)), 
                ELF64_R_SYM(rela[i].r_info), IdxSymName(h, ELF64_R_SYM(rela[i].r_info)), 
                rela[i].r_addend);
        printf_dump(LOG_NEVER, "ELF Dump %s Table=====\n", name);
    }
}

void DumpBinary(char* p, int sz)
{
    // dump p as 
    // PPPPPPPP XX XX XX ... XX | 0123456789ABCDEF
    unsigned char* d = (unsigned char*)p;
    int delta = ((uintptr_t)p)&0xf;
    for (int i = 0; sz; ++i) {
        printf_dump(LOG_NEVER, "%p ", (void*)(((uintptr_t)d)&~0xf));
        int n = 16 - delta;
        if (n>sz) n = sz;
        int fill = 16-sz;
        for (int j = 0; j<delta; ++j)
            printf_dump(LOG_NEVER, "   ");
        for (int j = 0; j<n; ++j)
            printf_dump(LOG_NEVER, "%02X ", d[j]);
        for (int j = 0; j<fill; ++j)
            printf_dump(LOG_NEVER, "   ");
        printf_dump(LOG_NEVER, " | ");
        for (int j = 0; j<delta; ++j)
            printf_dump(LOG_NEVER, " ");
        for (int j = 0; j<n; ++j)
            printf_dump(LOG_NEVER, "%c", (d[j]<32 || d[j]>127)?'.':d[j]);
        for (int j = 0; j<fill; ++j)
            printf_dump(LOG_NEVER, " ");
        printf_dump(LOG_NEVER, "\n");
        d+=n;
        sz-=n;
        delta=0;
    }
}
