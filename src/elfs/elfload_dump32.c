#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

#include "box64version.h"
#include "elfloader.h"
#include "debug.h"
#include "elfload_dump.h"
#include "elfloader_private.h"
#include "box32.h"

static const char* DumpSection(Elf32_Shdr *s, char* SST) {
    static char buff[400];
    switch (s->sh_type) {
        case SHT_NULL:
            return "SHT_NULL";
        #define GO(A) \
        case A:     \
            sprintf(buff, #A " Name=\"%s\"(%d) off=0x%X, size=%d, attr=0x%04X, addr=%p(%02X), link/info=%d/%d", \
                SST+s->sh_name, s->sh_name, s->sh_offset, s->sh_size, s->sh_flags, from_ptrv(s->sh_addr), s->sh_addralign, s->sh_link, s->sh_info); \
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

static const char* DumpDynamic(Elf32_Dyn *s) {
    static char buff[200];
    switch (s->d_tag) {
        case DT_NULL:
            return "DT_NULL: End Dynamic Section";
        #define GO(A, Add) \
        case A:     \
            sprintf(buff, #A " %s=0x%X", (Add)?"Addr":"Val", (Add)?s->d_un.d_ptr:s->d_un.d_val); \
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
            sprintf(buff, "0x%X unknown type", s->d_tag);
    }
    return buff;
}

static const char* DumpPHEntry(Elf32_Phdr *e)
{
    static char buff[500];
    memset(buff, 0, sizeof(buff));
    switch(e->p_type) {
        case PT_NULL: sprintf(buff, "type: %s", "PT_NULL"); break;
        #define GO(T) case T: sprintf(buff, "type: %s, Off=%x vaddr=%p paddr=%p filesz=%u memsz=%u flags=%x align=%u", #T, e->p_offset, from_ptrv(e->p_vaddr), from_ptrv(e->p_paddr), e->p_filesz, e->p_memsz, e->p_flags, e->p_align); break
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
        default: sprintf(buff, "type: %x, Off=%x vaddr=%p paddr=%p filesz=%u memsz=%u flags=%x align=%u", e->p_type, e->p_offset, from_ptrv(e->p_vaddr), from_ptrv(e->p_paddr), e->p_filesz, e->p_memsz, e->p_flags, e->p_align); break;
    }
    return buff;
}

const char* DumpRelType32(int t)
{
    static char buff[50];
    memset(buff, 0, sizeof(buff));
    switch(t) {
        #define GO(T) case T: sprintf(buff, "type: %s", #T); break
        GO(R_386_NONE);
        GO(R_386_32);
        GO(R_386_PC32);
        GO(R_386_GOT32);
        GO(R_386_PLT32);
        GO(R_386_COPY);
        GO(R_386_GLOB_DAT);
        GO(R_386_JMP_SLOT);
        GO(R_386_RELATIVE);
        GO(R_386_GOTOFF);
        GO(R_386_GOTPC);
        GO(R_386_PC8);
        GO(R_386_TLS_TPOFF);
        GO(R_386_TLS_GD_32);
        GO(R_386_TLS_DTPMOD32);
        GO(R_386_TLS_DTPOFF32);
        GO(R_386_TLS_TPOFF32);
        #undef GO
        default: sprintf(buff, "type: 0x%x (unknown)", t); break;
    }
    return buff;
}

static const char* DumpSym(elfheader_t *h, Elf32_Sym* sym, int version)
{
    static char buff[4096];
    const char* vername = (version==-1)?"(none)":((version==0)?"*local*":((version==1)?"*global*":GetSymbolVersion(h, version)));
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "\"%s\", value=%p, size=%d, info/other=%d/%d index=%d (ver=%d/%s)", 
        h->DynStr+sym->st_name, from_ptrv(sym->st_value), sym->st_size,
        sym->st_info, sym->st_other, sym->st_shndx, version, vername);
    return buff;
}

const char* SymName32(elfheader_t *h, Elf32_Sym* sym)
{
    return h->DynStr+sym->st_name;
}
static const char* IdxSymName(elfheader_t *h, int sym)
{
    return h->DynStr+h->DynSym._32[sym].st_name;
}

void DumpMainHeader32(Elf32_Ehdr *header, elfheader_t *h)
{
    if (BOX64ENV(dump)) {
        printf_dump(LOG_NEVER, "ELF Dump main header\n");
        printf_dump(LOG_NEVER, "  Entry point = %p\n", from_ptrv(header->e_entry));
        printf_dump(LOG_NEVER, "  Program Header table offset = %p\n", from_ptrv(header->e_phoff));
        printf_dump(LOG_NEVER, "  Section Header table offset = %p\n", from_ptrv(header->e_shoff));
        printf_dump(LOG_NEVER, "  Flags = 0x%X\n", header->e_flags);
        printf_dump(LOG_NEVER, "  ELF Header size = %d\n", header->e_ehsize);
        printf_dump(LOG_NEVER, "  Program Header Entry num/size = %d(%d)/%d\n", h->numPHEntries, header->e_phnum, header->e_phentsize);
        printf_dump(LOG_NEVER, "  Section Header Entry num/size = %d(%d)/%d\n", h->numSHEntries, header->e_shnum, header->e_shentsize);
        printf_dump(LOG_NEVER, "  Section Header index num = %d(%d)\n", h->SHIdx, header->e_shstrndx);
        printf_dump(LOG_NEVER, "ELF Dump ==========\n");

        printf_dump(LOG_NEVER, "ELF Dump PEntries (%d)\n", h->numSHEntries);
        for (int i=0; i<h->numPHEntries; ++i)
            printf_dump(LOG_NEVER, "  PHEntry %04d : %s\n", i, DumpPHEntry(h->PHEntries._32+i));
        printf_dump(LOG_NEVER, "ELF Dump PEntries ====\n");

        printf_dump(LOG_NEVER, "ELF Dump Sections (%d)\n", h->numSHEntries);
        for (int i=0; i<h->numSHEntries; ++i)
            printf_dump(LOG_NEVER, "  Section %04d : %s\n", i, DumpSection(h->SHEntries._32+i, h->SHStrTab));
        printf_dump(LOG_NEVER, "ELF Dump Sections ====\n");
    }
}

void DumpSymTab32(elfheader_t *h)
{
    if (BOX64ENV(dump) && h->SymTab._32) {
        const char* name = ElfName(h);
        printf_dump(LOG_NEVER, "ELF Dump SymTab(%zu)=\n", h->numSymTab);
        for (size_t i=0; i<h->numSymTab; ++i)
            printf_dump(LOG_NEVER, "  %s:SymTab[%zu] = \"%s\", value=%p, size=%d, info/other=%d/%d index=%d\n", name, 
                i, h->StrTab+h->SymTab._32[i].st_name, from_ptrv(h->SymTab._32[i].st_value), h->SymTab._32[i].st_size,
                h->SymTab._32[i].st_info, h->SymTab._32[i].st_other, h->SymTab._32[i].st_shndx);
        printf_dump(LOG_NEVER, "ELF Dump SymTab=====\n");
    }
}

void DumpDynamicSections32(elfheader_t *h)
{
    if (BOX64ENV(dump) && h->Dynamic._32) {
        printf_dump(LOG_NEVER, "ELF Dump Dynamic(%d)=\n", h->numDynamic);
        for (size_t i=0; i<h->numDynamic; ++i)
            printf_dump(LOG_NEVER, "  Dynamic %04d : %s\n", i, DumpDynamic(h->Dynamic._32+i));
        printf_dump(LOG_NEVER, "ELF Dump Dynamic=====\n");
    }
}

void DumpDynSym32(elfheader_t *h)
{
    if (BOX64ENV(dump) && h->DynSym._32) {
        const char* name = ElfName(h);
        printf_dump(LOG_NEVER, "ELF Dump DynSym(%zu)=\n", h->numDynSym);
        for (size_t i=0; i<h->numDynSym; ++i) {
            int version = h->VerSym?((Elf32_Half*)((uintptr_t)h->VerSym+h->delta))[i]:-1;
            printf_dump(LOG_NEVER, "  %s:DynSym[%zu] = %s\n", name, i, DumpSym(h, h->DynSym._32+i, version));
        }
        printf_dump(LOG_NEVER, "ELF Dump DynSym=====\n");
    }
}

void DumpDynamicNeeded32(elfheader_t *h)
{
    if (BOX64ENV(dump) && h->DynStrTab) {
        printf_dump(LOG_NEVER, "ELF Dump DT_NEEDED=====\n");
        for (size_t i=0; i<h->numDynamic; ++i)
            if(h->Dynamic._32[i].d_tag==DT_NEEDED) {
                printf_dump(LOG_NEVER, "  Needed : %s\n", h->DynStrTab+h->Dynamic._32[i].d_un.d_val + h->delta);
            }
        printf_dump(LOG_NEVER, "ELF Dump DT_NEEDED=====\n");
    }
}

void DumpDynamicRPath32(elfheader_t *h)
{
    if (BOX64ENV(dump) && h->DynStrTab) {
        printf_dump(LOG_NEVER, "ELF Dump DT_RPATH/DT_RUNPATH=====\n");
        for (size_t i=0; i<h->numDynamic; ++i) {
            if(h->Dynamic._32[i].d_tag==DT_RPATH) {
                printf_dump(LOG_NEVER, "   RPATH : %s\n", h->DynStrTab+h->Dynamic._32[i].d_un.d_val + h->delta);
            }
            if(h->Dynamic._32[i].d_tag==DT_RUNPATH) {
                printf_dump(LOG_NEVER, " RUNPATH : %s\n", h->DynStrTab+h->Dynamic._32[i].d_un.d_val + h->delta);
            }
        }
        printf_dump(LOG_NEVER, "=====ELF Dump DT_RPATH/DT_RUNPATH\n");
    }
}

void DumpRelTable32(elfheader_t *h, int cnt, Elf32_Rel *rel, const char* name)
{
    if (BOX64ENV(dump)) {
        const char* elfname = ElfName(h);
        printf_dump(LOG_NEVER, "ELF Dump %s Table(%d) @%p\n", name, cnt, rel);
        for (int i = 0; i<cnt; ++i)
            printf_dump(LOG_NEVER, "  %s:Rel[%d] = %p (0x%X: %s, sym=0x%0X/%s)\n", elfname,
                i, from_ptrv(rel[i].r_offset), rel[i].r_info, DumpRelType32(ELF32_R_TYPE(rel[i].r_info)), 
                ELF32_R_SYM(rel[i].r_info), IdxSymName(h, ELF32_R_SYM(rel[i].r_info)));
        printf_dump(LOG_NEVER, "ELF Dump %s Table=====\n", name);
    }
}

void DumpRelATable32(elfheader_t *h, int cnt, Elf32_Rela *rela, const char* name)
{
    if (BOX64ENV(dump) && h->rela) {
        const char* elfname = ElfName(h);
        printf_dump(LOG_NEVER, "ELF Dump %s Table(%d) @%p\n", name, cnt, rela);
        for (int i = 0; i<cnt; ++i)
            printf_dump(LOG_NEVER, "  %s:RelA[%d] = %p (0x%X: %s, sym=0x%X/%s) Addend=%d\n", elfname,
                i, from_ptrv(rela[i].r_offset), rela[i].r_info, DumpRelType32(ELF32_R_TYPE(rela[i].r_info)), 
                ELF32_R_SYM(rela[i].r_info), IdxSymName(h, ELF32_R_SYM(rela[i].r_info)), 
                rela[i].r_addend);
        printf_dump(LOG_NEVER, "ELF Dump %s Table=====\n", name);
    }
}

void DumpRelRTable32(elfheader_t *h, int cnt, Elf32_Relr *relr, const char* name)
{
    if (BOX64ENV(dump) && h->relr) {
        const char* elfname = ElfName(h);
        printf_dump(LOG_NEVER, "ELF Dump %s Table(%d) @%p\n", name, cnt, relr);
        for (int i = 0; i<cnt; ++i)
            printf_dump(LOG_NEVER, "  %s:%s[%d] = %p\n", elfname, name,
                i, from_ptrv(relr[i]));
        printf_dump(LOG_NEVER, "ELF Dump %s Table=====\n", name);
    }
}
