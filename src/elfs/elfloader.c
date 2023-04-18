#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <link.h>
#include <unistd.h>
#include <errno.h>
#ifndef _DLFCN_H
#include <dlfcn.h>
#endif

#include "custommem.h"
#include "box64version.h"
#include "elfloader.h"
#include "debug.h"
#include "elfload_dump.h"
#include "elfloader_private.h"
#include "librarian.h"
#include "x64run.h"
#include "bridge.h"
#include "wrapper.h"
#include "box64context.h"
#include "library.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "box64stack.h"
#include "wine_tools.h"
#include "dictionnary.h"
#include "symbols.h"
#ifdef DYNAREC
#include "dynablock.h"
#endif
#include "../emu/x64emu_private.h"
#include "x64tls.h"

void* my__IO_2_1_stderr_ = NULL;
void* my__IO_2_1_stdin_  = NULL;
void* my__IO_2_1_stdout_ = NULL;

// return the index of header (-1 if it doesn't exist)
int getElfIndex(box64context_t* ctx, elfheader_t* head) {
    for (int i=0; i<ctx->elfsize; ++i)
        if(ctx->elfs[i]==head)
            return i;
    return -1;
}

elfheader_t* LoadAndCheckElfHeader(FILE* f, const char* name, int exec)
{
    elfheader_t *h = ParseElfHeader(f, name, exec);
    if(!h)
        return NULL;

    if ((h->path = box_realpath(name, NULL)) == NULL) {
        h->path = (char*)box_malloc(1);
        h->path[0] = '\0';
    }
    
    h->mapsymbols = NewMapSymbols();
    h->weaksymbols = NewMapSymbols();
    h->localsymbols = NewMapSymbols();
    h->globaldefver = NewDefaultVersion();
    h->weakdefver = NewDefaultVersion();
    h->refcnt = 1;
    
    return h;
}

void FreeElfHeader(elfheader_t** head)
{
    if(!head || !*head)
        return;
    elfheader_t *h = *head;
    if(my_context)
        RemoveElfHeader(my_context, h);

    box_free(h->PHEntries);
    box_free(h->SHEntries);
    box_free(h->SHStrTab);
    box_free(h->StrTab);
    box_free(h->Dynamic);
    box_free(h->DynStr);
    box_free(h->SymTab);
    box_free(h->DynSym);

    FreeMapSymbols(&h->mapsymbols);
    FreeMapSymbols(&h->weaksymbols);
    FreeMapSymbols(&h->localsymbols);
    FreeDefaultVersion(&h->globaldefver);
    FreeDefaultVersion(&h->weakdefver);
    
    FreeElfMemory(h);

    box_free(h->name);
    box_free(h->path);
    box_free(h);

    *head = NULL;
}

int CalcLoadAddr(elfheader_t* head)
{
    head->memsz = 0;
    head->paddr = head->vaddr = ~(uintptr_t)0;
    head->align = 1;
    for (size_t i=0; i<head->numPHEntries; ++i)
        if(head->PHEntries[i].p_type == PT_LOAD) {
            if(head->paddr > (uintptr_t)head->PHEntries[i].p_paddr)
                head->paddr = (uintptr_t)head->PHEntries[i].p_paddr;
            if(head->vaddr > (uintptr_t)head->PHEntries[i].p_vaddr)
                head->vaddr = (uintptr_t)head->PHEntries[i].p_vaddr;
        }
    
    if(head->vaddr==~(uintptr_t)0 || head->paddr==~(uintptr_t)0) {
        printf_log(LOG_NONE, "Error: v/p Addr for Elf Load not set\n");
        return 1;
    }

    head->stacksz = 1024*1024;          //1M stack size default?
    head->stackalign = 16;   // default align for stack
    for (size_t i=0; i<head->numPHEntries; ++i) {
        if(head->PHEntries[i].p_type == PT_LOAD) {
            uintptr_t phend = head->PHEntries[i].p_vaddr - head->vaddr + head->PHEntries[i].p_memsz;
            if(phend > head->memsz)
                head->memsz = phend;
            if(head->PHEntries[i].p_align > head->align)
                head->align = head->PHEntries[i].p_align;
        }
        if(head->PHEntries[i].p_type == PT_GNU_STACK) {
            if(head->stacksz < head->PHEntries[i].p_memsz)
                head->stacksz = head->PHEntries[i].p_memsz;
            if(head->stackalign < head->PHEntries[i].p_align)
                head->stackalign = head->PHEntries[i].p_align;
        }
        if(head->PHEntries[i].p_type == PT_TLS) {
            head->tlsaddr = head->PHEntries[i].p_vaddr;
            head->tlssize = head->PHEntries[i].p_memsz;
            head->tlsfilesize = head->PHEntries[i].p_filesz;
            head->tlsalign = head->PHEntries[i].p_align;
            // force alignement...
            if(head->tlsalign>1)
                while(head->tlssize&(head->tlsalign-1))
                    head->tlssize++;
        }
    }
    printf_log(LOG_DEBUG, "Elf Addr(v/p)=%p/%p Memsize=0x%zx (align=0x%zx)\n", (void*)head->vaddr, (void*)head->paddr, head->memsz, head->align);
    printf_log(LOG_DEBUG, "Elf Stack Memsize=%zu (align=%zu)\n", head->stacksz, head->stackalign);
    printf_log(LOG_DEBUG, "Elf TLS Memsize=%zu (align=%zu)\n", head->tlssize, head->tlsalign);

    return 0;
}

const char* ElfName(elfheader_t* head)
{
    if(!head)
        return "box64";
    return head->name;
}
const char* ElfPath(elfheader_t* head)
{
    if(!head)
        return NULL;
    return head->path;
}
int AllocElfMemory(box64context_t* context, elfheader_t* head, int mainbin)
{
    uintptr_t offs = 0;
    if(!head->vaddr && box64_load_addr) {
        offs = (uintptr_t)find47bitBlockNearHint((void*)box64_load_addr, head->memsz);
        box64_load_addr += head->memsz;
        box64_load_addr = (box64_load_addr+0x10ffffffLL)&~0xffffffLL;
    }
    int log_level = box64_load_addr?LOG_INFO:LOG_DEBUG;
    if(!offs)
        offs = head->vaddr;
    if(head->vaddr) {
        head->multiblock_n = 0; // count PHEntrie with LOAD
        for (size_t i=0; i<head->numPHEntries; ++i) 
            if(head->PHEntries[i].p_type == PT_LOAD && head->PHEntries[i].p_flags)
                ++head->multiblock_n;
        head->multiblock_size = (uint64_t*)box_calloc(head->multiblock_n, sizeof(uint64_t));
        head->multiblock_offs = (uintptr_t*)box_calloc(head->multiblock_n, sizeof(uintptr_t));
        head->multiblock = (void**)box_calloc(head->multiblock_n, sizeof(void*));
        // and now, create all individual blocks
        head->memory = (char*)0xffffffffffffffff;
        int n = 0;
        for (size_t i=0; i<head->numPHEntries; ++i) 
            if(head->PHEntries[i].p_type == PT_LOAD && head->PHEntries[i].p_flags) {
                Elf64_Phdr * e = &head->PHEntries[i];
                uintptr_t bstart = e->p_vaddr;
                uint64_t bsize = e->p_memsz;
                uintptr_t balign = e->p_align;
                if (balign) balign = balign-1; else balign = 1;
                if(balign<4095) balign = 4095;
                uintptr_t bend = (bstart + bsize + balign)&(~balign);
                bstart &= ~balign;
                int ok = 0;
                for (int j=0; !ok && j<n; ++j) {
                    uintptr_t start = head->multiblock_offs[j];
                    uintptr_t end = head->multiblock_offs[j] + head->multiblock_size[j];
                    start &= ~balign;
                    if( (head->e_type == ET_DYN) ||
                        (((bstart>=start) && (bstart<=end+0x1000)) || ((bend>=start) && (bend<=end)) || ((bstart<start) && (bend>end))))
                    {
                        // merge
                        ok = 1;
                        if(bstart<start)
                            head->multiblock_offs[j] = bstart;
                        head->multiblock_size[j] = ((bend>end)?bend:end) - head->multiblock_offs[j];
                        --head->multiblock_n;
                    }
                }
                if(!ok) {
                    head->multiblock_offs[n] = bstart;
                    head->multiblock_size[n] = bend - head->multiblock_offs[n];
                    ++n;
                }
            }
        head->multiblock_n = n; // might be less in fact
        for (int i=0; i<head->multiblock_n; ++i) {
            printf_log(log_level, "Allocating 0x%lx memory @%p for Elf \"%s\"\n", head->multiblock_size[i], (void*)head->multiblock_offs[i], head->name);
            void* p = mmap((void*)head->multiblock_offs[i], head->multiblock_size[i]
                , PROT_READ | PROT_WRITE | PROT_EXEC
                , MAP_PRIVATE | MAP_ANONYMOUS /*| ((wine_preloaded)?MAP_FIXED:0)*/
                , -1, 0);
            if(p==MAP_FAILED) {
                printf_log(LOG_NONE, "Cannot create memory map (@%p 0x%zx/0x%zx) for elf \"%s\"\n", (void*)head->multiblock_offs[i], head->multiblock_size[i], head->align, head->name);
                return 1;
            }
            if(head->multiblock_offs[i] &&( p!=(void*)head->multiblock_offs[i])) {
                if((head->e_type!=ET_DYN)) {
                    printf_log(LOG_NONE, "Error, memory map (@%p 0x%zx/0x%zx) for elf \"%s\" allocated @%p\n", (void*)head->multiblock_offs[i], head->multiblock_size[i], head->align, head->name, p);
                    return 1;
                } else {
                    printf_log(LOG_INFO, "Allocated memory is not at hinted %p but %p (size %lx) \"%s\"\n", (void*)head->multiblock_offs[i], p, head->multiblock_size[i], head->name);
                    // need to adjust vaddr!
                    for (size_t j=0; j<head->numPHEntries; ++j) 
                        if(head->PHEntries[j].p_type == PT_LOAD) {
                            Elf64_Phdr * e = &head->PHEntries[j];
                            if(e->p_vaddr>=head->multiblock_offs[j] && e->p_vaddr<(head->multiblock_offs[j]+head->multiblock_size[j])) {
                                e->p_vaddr = e->p_vaddr - head->multiblock_offs[j] + (uintptr_t)p;
                                if(!head->delta) head->delta = (intptr_t)p - (intptr_t)head->multiblock_offs[j];
                            }
                        }
                }
            }
            setProtection((uintptr_t)p, head->multiblock_size[i], PROT_READ | PROT_WRITE | PROT_EXEC);
            head->multiblock[i] = p;
            if(p<(void*)head->memory)
                head->memory = (char*)p;
        }
    } else {
        // vaddr is 0, load everything has a One block
        uintptr_t old_offs = offs;
        if(!offs && box64_wine)
            offs = (uintptr_t)find47bitBlock(head->memsz); // limit to 47bits...
        printf_log(log_level, "Allocating 0x%zx memory @%p for Elf \"%s\"\n", head->memsz, (void*)offs, head->name);
        void* p = mmap((void*)offs, head->memsz
            , PROT_READ | PROT_WRITE | PROT_EXEC
            , MAP_PRIVATE | MAP_ANONYMOUS /*| (((offs&&wine_preloaded)?MAP_FIXED:0))*/
            , -1, 0);
        if(offs &&!old_offs && p!=MAP_FAILED && offs!=(uintptr_t)p) {
            // try again
            munmap(p, head->memsz);
            loadProtectionFromMap();
            offs = (uintptr_t)find47bitBlock(head->memsz);
            printf_log(log_level, "New, try. Allocating 0x%zx memory @%p for Elf \"%s\"\n", head->memsz, (void*)offs, head->name);
            p = mmap((void*)offs, head->memsz
                , PROT_READ | PROT_WRITE | PROT_EXEC
                , MAP_PRIVATE | MAP_ANONYMOUS /*| (((offs&&wine_preloaded)?MAP_FIXED:0))*/
                , -1, 0);
        }
        if(p==MAP_FAILED) {
            printf_log(LOG_NONE, "Cannot create memory map (@%p 0x%zx/0x%zx) for elf \"%s\"\n", (void*)offs, head->memsz, head->align, head->name);
            return 1;
        }
        if(offs && (p!=(void*)offs) && (head->e_type!=ET_DYN)) {
            printf_log(LOG_NONE, "Error, memory map (@%p 0x%zx/0x%zx) for elf \"%s\" allocated @%p\n", (void*)offs, head->memsz, head->align, head->name, p);
            return 1;
        }
        setProtection((uintptr_t)p, head->memsz, PROT_READ | PROT_WRITE | PROT_EXEC);
        head->memory = p;
        memset(p, 0, head->memsz);
        head->delta = (intptr_t)p - (intptr_t)head->vaddr;
        printf_log(log_level, "Got %p (delta=%p)\n", p, (void*)head->delta);

        head->multiblock_n = 1;
        head->multiblock_size = (uint64_t*)box_calloc(head->multiblock_n, sizeof(uint64_t));
        head->multiblock_offs = (uintptr_t*)box_calloc(head->multiblock_n, sizeof(uintptr_t));
        head->multiblock = (void**)box_calloc(head->multiblock_n, sizeof(void*));
        head->multiblock_size[0] = head->memsz;
        head->multiblock_offs[0] = (uintptr_t)p;
        head->multiblock[0] = p;
    }

    head->tlsbase = AddTLSPartition(context, head->tlssize);

    return 0;
}

void FreeElfMemory(elfheader_t* head)
{
    if(head->multiblock_n) {
        for(int i=0; i<head->multiblock_n; ++i) {
#ifdef DYNAREC
            dynarec_log(LOG_INFO, "Free DynaBlocks for %s\n", head->path);
            cleanDBFromAddressRange((uintptr_t)head->multiblock[i], head->multiblock_size[i], 1);
#endif
            munmap(head->multiblock[i], head->multiblock_size[i]);
        }
        box_free(head->multiblock);
        box_free(head->multiblock_size);
        box_free(head->multiblock_offs);
    }
}

int LoadElfMemory(FILE* f, box64context_t* context, elfheader_t* head)
{
    for (size_t i=0; i<head->numPHEntries; ++i) {
        if(head->PHEntries[i].p_type == PT_LOAD) {
            Elf64_Phdr * e = &head->PHEntries[i];
            char* dest = (char*)e->p_paddr + head->delta;
            printf_log(LOG_DEBUG, "MMap block #%zu @%p offset=%p (0x%zx/0x%zx)\n", i, dest, (void*)e->p_offset, e->p_filesz, e->p_memsz);
            void* p = (void*)-1;
            if(e->p_memsz==e->p_filesz && !(e->p_align&(box64_pagesize-1))) {
                printf_log(LOG_DEBUG, "MMap block #%zu @%p offset=%p (0x%zx/0x%zx, flags:0x%x)\n", i, dest, (void*)e->p_offset, e->p_filesz, e->p_memsz, e->p_flags);
                p = mmap(dest, e->p_filesz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_FIXED | MAP_PRIVATE, fileno(f), e->p_offset);
            }
            if(p!=dest) {
                printf_log(LOG_DEBUG, "Loading block #%zu %p (0x%zx/0x%zx)\n",i, dest, e->p_filesz, e->p_memsz);
                fseeko64(f, e->p_offset, SEEK_SET);
                if(e->p_filesz) {
                    if(fread(dest, e->p_filesz, 1, f)!=1) {
                        printf_log(LOG_NONE, "Fail to read PT_LOAD part #%zu (size=%zd)\n", i, e->p_filesz);
                        return 1;
                    }
                }
            }
#ifdef DYNAREC
            if(box64_dynarec && (e->p_flags & PF_X)) {
                dynarec_log(LOG_DEBUG, "Add ELF eXecutable Memory %p:%p\n", dest, (void*)e->p_memsz);
                addDBFromAddressRange((uintptr_t)dest, e->p_memsz);
            }
#endif

            // zero'd difference between filesz and memsz
            /*if(e->p_filesz != e->p_memsz)
                memset(dest+e->p_filesz, 0, e->p_memsz - e->p_filesz);*/    //block is already 0'd at creation
        }
        if(head->PHEntries[i].p_type == PT_TLS) {
            Elf64_Phdr * e = &head->PHEntries[i];
            char* dest = (char*)(context->tlsdata+context->tlssize+head->tlsbase);
            printf_log(LOG_DEBUG, "Loading TLS block #%zu @%p (0x%zx/0x%zx)\n", i, dest, e->p_filesz, e->p_memsz);
            if(e->p_filesz) {
                fseeko64(f, e->p_offset, SEEK_SET);
                if(fread(dest, e->p_filesz, 1, f)!=1) {
                    printf_log(LOG_NONE, "Fail to read PT_TLS part #%zu (size=%zd)\n", i, e->p_filesz);
                    return 1;
                }
            }
            // zero'd difference between filesz and memsz
            if(e->p_filesz != e->p_memsz)
                memset(dest+e->p_filesz, 0, e->p_memsz - e->p_filesz);
        }
    }
    return 0;
}

int ReloadElfMemory(FILE* f, box64context_t* context, elfheader_t* head)
{
    (void)context;

    for (size_t i=0; i<head->numPHEntries; ++i) {
        if(head->PHEntries[i].p_type == PT_LOAD) {
            Elf64_Phdr * e = &head->PHEntries[i];
            char* dest = (char*)e->p_paddr + head->delta;
            printf_log(LOG_DEBUG, "Re-loading block #%zu @%p (0x%zx/0x%zx)\n", i, dest, e->p_filesz, e->p_memsz);
            int ret = fseeko64(f, e->p_offset, SEEK_SET);
            if(ret==-1) {printf_log(LOG_NONE, "Fail to (re)seek PT_LOAD part #%zu (offset=%ld, errno=%d/%s)\n", i, e->p_offset, errno, strerror(errno)); return 1;}
            #ifdef DYNAREC
            cleanDBFromAddressRange((uintptr_t)dest, e->p_memsz, 0);
            #endif
            uint32_t page_offset = (uintptr_t)dest & (box64_pagesize - 1);
            mprotect(dest - page_offset, e->p_memsz + page_offset, PROT_READ | PROT_WRITE | PROT_EXEC);
            setProtection((uintptr_t)dest - page_offset, e->p_memsz + page_offset, PROT_READ | PROT_WRITE | PROT_EXEC);
            if(e->p_filesz) {
                ssize_t r = -1;
                if((r=fread(dest, e->p_filesz, 1, f))!=1) {
                    printf_log(LOG_NONE, "Fail to (re)read PT_LOAD part #%zu (dest=%p, size=%zd, return=%zd, feof=%d/ferror=%d/%s)\n", i, dest, e->p_filesz, r, feof(f), ferror(f), strerror(ferror(f)));
                    return 1;
                }
            }
            // zero'd difference between filesz and memsz
            if(e->p_filesz != e->p_memsz)
                memset(dest+e->p_filesz, 0, e->p_memsz - e->p_filesz);
        }
    }
    // TLS data are just a copy, no need to re-load it
    return 0;
}

int FindR64COPYRel(elfheader_t* h, const char* name, uintptr_t *offs, uint64_t** p, size_t size, int version, const char* vername)
{
    if(!h)
        return 0;
    Elf64_Rela * rela = (Elf64_Rela *)(h->rela + h->delta);
    if(!h->rela)
        return 0;
    int cnt = h->relasz / h->relaent;
    for (int i=0; i<cnt; ++i) {
        int t = ELF64_R_TYPE(rela[i].r_info);
        Elf64_Sym *sym = &h->DynSym[ELF64_R_SYM(rela[i].r_info)];
        const char* symname = SymName(h, sym);
        if(t==R_X86_64_COPY && symname && !strcmp(symname, name) && sym->st_size==size) {
            int version2 = h->VerSym?((Elf64_Half*)((uintptr_t)h->VerSym+h->delta))[ELF64_R_SYM(rela[i].r_info)]:-1;
            if(version2!=-1) version2 &= 0x7fff;
            if(version && !version2) version2=-1;   // match a versioned symbol against a global "local" symbol
            const char* vername2 = GetSymbolVersion(h, version2);
            if(SameVersionedSymbol(name, version, vername, symname, version2, vername2)) {
                *offs = sym->st_value + h->delta;
                *p = (uint64_t*)(rela[i].r_offset + h->delta + rela[i].r_addend);
                return 1;
            }
        }
    }
    return 0;
}

int RelocateElfREL(lib_t *maplib, lib_t *local_maplib, int bindnow, elfheader_t* head, int cnt, Elf64_Rel *rel)
{
    int ret_ok = 0;
    for (int i=0; i<cnt; ++i) {
        int t = ELF64_R_TYPE(rel[i].r_info);
        Elf64_Sym *sym = &head->DynSym[ELF64_R_SYM(rel[i].r_info)];
        int bind = ELF64_ST_BIND(sym->st_info);
        const char* symname = SymName(head, sym);
        //uint64_t ndx = sym->st_shndx;
        uint64_t *p = (uint64_t*)(rel[i].r_offset + head->delta);
        uintptr_t offs = 0;
        uintptr_t end = 0;
        size_t size = sym->st_size;
        //uintptr_t tmp = 0;
        int version = head->VerSym?((Elf64_Half*)((uintptr_t)head->VerSym+head->delta))[ELF64_R_SYM(rel[i].r_info)]:-1;
        if(version!=-1) version &=0x7fff;
        const char* vername = GetSymbolVersion(head, version);
        const char* globdefver = (bind==STB_WEAK)?NULL:GetMaplibDefaultVersion(maplib, local_maplib, 0, symname);
        const char* weakdefver = (bind==STB_WEAK)?GetMaplibDefaultVersion(maplib, local_maplib, 1, symname):NULL;
        if(bind==STB_LOCAL) {
            if(!symname || !symname[0]) {
                offs = sym->st_value + head->delta;
                end = offs + sym->st_size;
            } else {
                if(!offs && !end && local_maplib)
                    GetLocalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                if(!offs && !end)
                    GetLocalSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
            }
        } else {
            // this is probably very very wrong. A proprer way to get reloc need to be written, but this hack seems ok for now
            // at least it work for half-life, unreal, ut99, zsnes, Undertale, ColinMcRae Remake, FTL, ShovelKnight...
            /*if(bind==STB_GLOBAL && (ndx==10 || ndx==19) && t!=R_X86_64_GLOB_DAT) {
                offs = sym->st_value + head->delta;
                end = offs + sym->st_size;
            }*/
            // so weak symbol are the one left
            if(!offs && !end) {
                if(!offs && !end && local_maplib)
                    GetGlobalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                if(!offs && !end && local_maplib)
                    GetGlobalSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
            }
        }
        uintptr_t globoffs, globend;
        uint64_t* globp;
        switch(t) {
            case R_X86_64_NONE:
                // can be ignored
                printf_dump(LOG_NEVER, "Ignoring %s @%p (%p)\n", DumpRelType(t), p, (void*)(p?(*p):0));
                break;
            case R_X86_64_PC32:
                    if (!offs) {
                        printf_log(LOG_NONE, "Error: Global Symbol %s not found, cannot apply R_X86_64_PC32 @%p (%p) in %s\n", symname, p, *(void**)p, head->name);
                        ret_ok = 1;
                    }
                    offs = (offs - (uintptr_t)p);
                    if(!offs)
                        printf_dump(LOG_NEVER, "Apply %s R_X86_64_PC32 @%p with sym=%s (%p -> %p)\n", (bind==STB_LOCAL)?"Local":"Global", p, symname, *(void**)p, (void*)(*(uintptr_t*)p+offs));
                    *p += offs;
                break;
            case R_X86_64_GLOB_DAT:
                if(head!=my_context->elfs[0] && !IsGlobalNoWeakSymbolInNative(maplib, symname, version, vername, globdefver) && FindR64COPYRel(my_context->elfs[0], symname, &globoffs, &globp, size, version, vername)) {
                    // set global offs / size for the symbol
                    offs = sym->st_value;
                    end = offs + sym->st_size;
                    if(sym->st_size && offs) {
                        printf_dump(LOG_NEVER, "Apply %s R_X86_64_GLOB_DAT with R_X86_64_COPY @%p/%p (%p/%p -> %p/%p) size=%ld on sym=%s \n", (bind==STB_LOCAL)?"Local":"Global", p, globp, (void*)(p?(*p):0), (void*)(globp?(*globp):0), (void*)(offs + head->delta), (void*)globoffs, sym->st_size, symname);
                        memmove((void*)globoffs, (void*)offs, sym->st_size);   // preapply to copy part from lib to main elf
                        AddUniqueSymbol(GetGlobalData(maplib), symname, offs + head->delta, sym->st_size, version, vername);
                    } else {
                        printf_dump(LOG_NEVER, "Apply %s R_X86_64_GLOB_DAT with R_X86_64_COPY @%p/%p (%p/%p -> %p/%p) null sized on sym=%s \n", (bind==STB_LOCAL)?"Local":"Global", p, globp, (void*)(p?(*p):0), (void*)(globp?(*globp):0), (void*)offs, (void*)globoffs, symname);
                    }
                    *p = globoffs;
                } else {
                    // Look for same symbol already loaded but not in self (so no need for local_maplib here)
                    if (GetGlobalNoWeakSymbolStartEnd(local_maplib?local_maplib:maplib, symname, &globoffs, &globend, version, vername, globdefver)) {
                        offs = globoffs;
                        end = globend;
                    }
                    if (!offs) {
                        if(strcmp(symname, "__gmon_start__") && strcmp(symname, "data_start") && strcmp(symname, "__data_start") && strcmp(symname, "collector_func_load"))
                            printf_log(LOG_NONE, "%s: Global Symbol %s (ver=%d/%s) not found, cannot apply R_X86_64_GLOB_DAT @%p (%p) in %s\n", (bind==STB_WEAK)?"Warning":"Error", symname, version, vername?vername:"(none)", p, *(void**)p, head->name);
                    } else {
                        printf_dump(LOG_NEVER, "Apply %s R_X86_64_GLOB_DAT @%p (%p -> %p) on sym=%s (ver=%d/%s)\n", (bind==STB_LOCAL)?"Local":"Global", p, (void*)(p?(*p):0), (void*)offs, symname, version, vername?vername:"(none)");
                        *p = offs;
                    }
                }
                break;
            case R_X86_64_COPY:
                if(offs) {
                    uintptr_t old_offs = offs;
                    uintptr_t old_end = end;
                    offs = 0;
                    GetSizedSymbolStartEnd(GetGlobalData(maplib), symname, &offs, &end, size, version, vername, 1, globdefver); // try globaldata symbols first
                    if(offs==0) {
                        GetNoSelfSymbolStartEnd(maplib, symname, &offs, &end, head, size, version, vername, globdefver, weakdefver);   // get original copy if any
                        if(!offs && local_maplib)
                            GetNoSelfSymbolStartEnd(local_maplib, symname, &offs, &end, head, size, version, vername, globdefver, weakdefver);
                    }
                    if(!offs) {
                        offs = old_offs;
                        end = old_end;
                    }
                    printf_dump(LOG_NEVER, "Apply %s R_X86_64_COPY @%p with sym=%s, @%p size=%ld (", (bind==STB_LOCAL)?"Local":"Global", p, symname, (void*)offs, sym->st_size);
                    memmove(p, (void*)offs, sym->st_size);
                    if(box64_dump) {
                        uint64_t *k = (uint64_t*)p;
                        for (unsigned j=0; j<((sym->st_size>128u)?128u:sym->st_size); j+=8, ++k)
                            printf_dump(LOG_NEVER, "%s0x%016lX", j?" ":"", *k);
                        printf_dump(LOG_NEVER, "%s)\n", (sym->st_size>128u)?" ...":"");
                    }
                } else {
                    printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_X86_64_COPY @%p (%p) in %s\n", symname, p, *(void**)p, head->name);
                }
                break;
            case R_X86_64_RELATIVE:
                printf_dump(LOG_NEVER, "Apply %s R_X86_64_RELATIVE @%p (%p -> %p)\n", (bind==STB_LOCAL)?"Local":"Global", p, *(void**)p, (void*)((*p)+head->delta));
                *p += head->delta;
                break;
            case R_X86_64_64:
                if (!offs) {
                    printf_log(LOG_NONE, "%s: Symbol %s not found, cannot apply R_X86_64_64 @%p (%p) in %s\n", (bind==STB_GLOBAL)?"Error":"Warning", symname, p, *(void**)p, head->name);
                    if(bind==STB_GLOBAL)
                        ret_ok = 1;
                    // return -1;
                } else {
                    printf_dump(LOG_NEVER, "Apply %s R_X86_64_64 @%p with sym=%s (%p -> %p)\n", (bind==STB_LOCAL)?"Local":"Global", p, symname, *(void**)p, (void*)(offs+*(uint64_t*)p));
                    *p += offs;
                }
                break;
            #if 0
            case R_X86_64_JUMP_SLOT:
                // apply immediatly for gobject closure marshal or for LOCAL binding. Also, apply immediatly if it doesn't jump in the got
                tmp = (uintptr_t)(*p);
                if (bind==STB_LOCAL 
                  || ((symname && strstr(symname, "g_cclosure_marshal_")==symname)) 
                  || !tmp
                  || !((tmp>=head->plt && tmp<head->plt_end) || (tmp>=head->gotplt && tmp<head->gotplt_end))
                  ) {
                    if (!offs) {
                        if(bind==STB_WEAK) {
                            printf_log(LOG_INFO, "Warning: Weak Symbol %s not found, cannot apply R_X86_64_JUMP_SLOT @%p (%p)\n", symname, p, *(void**)p);
                        } else {
                            printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_X86_64_JUMP_SLOT @%p (%p) in %s\n", symname, p, *(void**)p, head->name);
                        }
                        // return -1;
                    } else {
                        if(p) {
                            printf_dump(LOG_NEVER, "Apply %s R_X86_64_JUMP_SLOT @%p with sym=%s (%p -> %p)\n", (bind==STB_LOCAL)?"Local":"Global", p, symname, *(void**)p, (void*)offs);
                            *p = offs;
                        } else {
                            printf_log(LOG_NONE, "Warning, Symbol %s found, but Jump Slot Offset is NULL \n", symname);
                        }
                    }
                } else {
                    printf_dump(LOG_NEVER, "Preparing (if needed) %s R_X86_64_JUMP_SLOT @%p (0x%lx->0x%0lx) with sym=%s to be apply later\n", (bind==STB_LOCAL)?"Local":"Global", p, *p, *p+head->delta, symname);
                    *p += head->delta;
                }
                break;
            #endif
            default:
                printf_log(LOG_INFO, "Warning, don't know how to handle rel #%d %s (%p)\n", i, DumpRelType(ELF64_R_TYPE(rel[i].r_info)), p);
        }
    }
    return bindnow?ret_ok:0;
}

struct tlsdesc
{
  uintptr_t entry;
  uintptr_t arg;
};
uintptr_t tlsdescUndefweak = 0;
uintptr_t GetSegmentBaseEmu(x64emu_t* emu, int seg);
EXPORT uintptr_t _dl_tlsdesc_undefweak(x64emu_t* emu)
{
    struct tlsdesc *td = (struct tlsdesc *)R_RAX;
    return td->arg;
}


int RelocateElfRELA(lib_t *maplib, lib_t *local_maplib, int bindnow, elfheader_t* head, int cnt, Elf64_Rela *rela, int* need_resolv)
{
    int ret_ok = 0;
    const char* old_globdefver = NULL;
    const char* old_weakdefver = NULL;
    int old_bind = -1;
    const char* old_symname = NULL;
    uintptr_t old_offs = 0;
    uintptr_t old_end = 0;
    int old_version = -1;
    for (int i=0; i<cnt; ++i) {
        int t = ELF64_R_TYPE(rela[i].r_info);
        Elf64_Sym *sym = &head->DynSym[ELF64_R_SYM(rela[i].r_info)];
        int bind = ELF64_ST_BIND(sym->st_info);
        //uint64_t ndx = sym->st_shndx;
        const char* symname = SymName(head, sym);
        uint64_t *p = (uint64_t*)(rela[i].r_offset + head->delta);
        uintptr_t offs = 0;
        uintptr_t end = 0;
        size_t size = sym->st_size;
        elfheader_t* h_tls = NULL;//head;
        int version = head->VerSym?((Elf64_Half*)((uintptr_t)head->VerSym+head->delta))[ELF64_R_SYM(rela[i].r_info)]:-1;
        if(version!=-1) version &=0x7fff;
        const char* vername = GetSymbolVersion(head, version);
        const char* globdefver = NULL;
        const char* weakdefver = NULL;
        if(old_bind==bind && old_symname==symname) {
            globdefver = old_globdefver;
            weakdefver = old_weakdefver;
        } else {
            old_globdefver = globdefver = (bind==STB_WEAK)?NULL:GetMaplibDefaultVersion(maplib, local_maplib, 0, symname);
            old_weakdefver = weakdefver = (bind==STB_WEAK)?GetMaplibDefaultVersion(maplib, local_maplib, 1, symname):NULL;
        }
        if(bind==STB_LOCAL) {
            if(!symname || !symname[0]) {
                offs = sym->st_value + head->delta;
                end = offs + sym->st_size;
            } else {
                if(old_version==version && old_bind==bind && old_symname==symname) {
                    offs = old_offs;
                    end = old_end;
                } else {
                    if(local_maplib)
                        GetLocalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                    if(!offs && !end)
                        GetLocalSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                }
            }
        } else {
            // this is probably very very wrong. A proprer way to get reloc need to be written, but this hack seems ok for now
            // at least it work for half-life, unreal, ut99, zsnes, Undertale, ColinMcRae Remake, FTL, ShovelKnight...
            /*if(bind==STB_GLOBAL && (ndx==10 || ndx==19) && t!=R_X86_64_GLOB_DAT) {
                offs = sym->st_value + head->delta;
                end = offs + sym->st_size;
            }*/
            // so weak symbol are the one left
            if(old_version==version && old_bind==bind && old_symname==symname) {
                offs = old_offs;
                end = old_end;
            } else {
                GetGlobalSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
                if(!offs && !end && local_maplib)
                    GetGlobalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, globdefver, weakdefver);
            }
        }
        old_bind = bind;
        old_symname = symname;
        old_offs = offs;
        old_end = end;
        uintptr_t globoffs, globend;
        uint64_t* globp;
        uintptr_t tmp = 0;
        intptr_t delta;
        switch(t) {
            case R_X86_64_NONE:
                break;
            case R_X86_64_PC32:
                // should be "S + A - P" with S=symbol offset, A=addend and P=place of the storage unit, write a word32
                // can be ignored
                break;
            case R_X86_64_RELATIVE:
                printf_dump(LOG_NEVER, "Apply %s R_X86_64_RELATIVE @%p (%p -> %p)\n", (bind==STB_LOCAL)?"Local":"Global", p, *(void**)p, (void*)(head->delta+ rela[i].r_addend));
                *p = head->delta+ rela[i].r_addend;
                break;
            case R_X86_64_IRELATIVE:
                {
                    x64emu_t* emu = thread_get_emu();
                    EmuCall(emu, head->delta+rela[i].r_addend);
                    printf_dump(LOG_NEVER, "Apply %s R_X86_64_IRELATIVE @%p (%p -> %p()=%p)\n", (bind==STB_LOCAL)?"Local":"Global", p, *(void**)p, (void*)(head->delta+ rela[i].r_addend), (void*)(R_RAX));
                    *p = R_RAX;
                }
                break;
            case R_X86_64_COPY:
                globoffs = offs;
                globend = end;
                offs = end = 0;
                GetSizedSymbolStartEnd(GetGlobalData(maplib), symname, &offs, &end, size, version, vername, 1, globdefver); // try globaldata symbols first
                if(!offs && local_maplib)
                    GetNoSelfSymbolStartEnd(local_maplib, symname, &offs, &end, head, size, version, vername, globdefver, weakdefver);
                if(!offs)
                    GetNoSelfSymbolStartEnd(maplib, symname, &offs, &end, head, size, version, vername, globdefver, weakdefver);
                if(!offs) {offs = globoffs; end = globend;}
                if(offs) {
                    // add r_addend to p?
                    printf_dump(LOG_NEVER, "Apply R_X86_64_COPY @%p with sym=%s (ver=%d/%s), @%p+0x%lx size=%ld\n", p, symname, version, vername?vername:"(none)", (void*)offs, rela[i].r_addend, sym->st_size);
                    if(p!=(void*)(offs+rela[i].r_addend))
                        memmove(p, (void*)(offs+rela[i].r_addend), sym->st_size);
                } else {
                    printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply RELA R_X86_64_COPY @%p (%p) in %s\n", symname, p, *(void**)p, head->name);
                }
                break;
            case R_X86_64_GLOB_DAT:
                if(head!=my_context->elfs[0] && !IsGlobalNoWeakSymbolInNative(maplib, symname, version, vername, globdefver) && FindR64COPYRel(my_context->elfs[0], symname, &globoffs, &globp, size, version, vername)) {
                    // set global offs / size for the symbol
                    offs = sym->st_value + head->delta;
                    end = offs + sym->st_size;
                    if(sym->st_size && offs) {
                        printf_dump(LOG_NEVER, "Apply %s R_X86_64_GLOB_DAT with R_X86_64_COPY @%p/%p (%p/%p -> %p/%p) size=%zd on sym=%s (ver=%d/%s) \n", 
                            (bind==STB_LOCAL)?"Local":"Global", p, globp, (void*)(p?(*p):0), 
                            (void*)(globp?(*globp):0), (void*)offs, (void*)globoffs, sym->st_size, symname, version, vername?vername:"(none)");
                        //memmove((void*)globoffs, (void*)offs, sym->st_size);   // preapply to copy part from lib to main elf
                        AddUniqueSymbol(GetGlobalData(maplib), symname, offs, sym->st_size, version, vername);
                    } else {
                        printf_dump(LOG_NEVER, "Apply %s R_X86_64_GLOB_DAT with R_X86_64_COPY @%p/%p (%p/%p -> %p/%p) null sized on sym=%s (ver=%d/%s)\n", 
                            (bind==STB_LOCAL)?"Local":"Global", p, globp, (void*)(p?(*p):0), 
                            (void*)(globp?(*globp):0), (void*)offs, (void*)globoffs, symname, version, vername?vername:"(none)");
                    }
                    *p = globoffs;
                } else {
                    // Look for same symbol already loaded but not in self (so no need for local_maplib here)
                    if (GetGlobalNoWeakSymbolStartEnd(local_maplib?local_maplib:maplib, symname, &globoffs, &globend, version, vername, globdefver)) {
                        offs = globoffs;
                        end = globend;
                    }
                    if (!offs) {
                        if(strcmp(symname, "__gmon_start__") && strcmp(symname, "data_start") && strcmp(symname, "__data_start") && strcmp(symname, "collector_func_load"))
                            printf_log((bind==STB_WEAK)?LOG_INFO:LOG_NONE, "%s: Global Symbol %s not found, cannot apply R_X86_64_GLOB_DAT @%p (%p) in %s\n", (bind==STB_WEAK)?"Warning":"Error", symname, p, *(void**)p, head->name);
                    } else {
                        printf_dump(LOG_NEVER, "Apply %s R_X86_64_GLOB_DAT @%p (%p -> %p) on sym=%s (ver=%d/%s)\n", (bind==STB_LOCAL)?"Local":"Global", p, (void*)(p?(*p):0), (void*)offs, symname, version, vername?vername:"(none)");
                        *p = offs/* + rela[i].r_addend*/;   // not addend it seems
                    }
                }
                break;
            case R_X86_64_JUMP_SLOT:
                // apply immediatly for gobject closure marshal or for LOCAL binding. Also, apply immediatly if it doesn't jump in the got
                tmp = (uintptr_t)(*p);
                if (bind==STB_LOCAL 
                  || ((symname && strstr(symname, "g_cclosure_marshal_")==symname)) 
                  || ((symname && strstr(symname, "__pthread_unwind_next")==symname)) 
                  || !tmp
                  || !((tmp>=head->plt && tmp<head->plt_end) || (tmp>=head->gotplt && tmp<head->gotplt_end))
                  || !need_resolv
                  || bindnow
                  ) {
                    if (!offs) {
                        if(bind==STB_WEAK) {
                            printf_log(LOG_INFO, "Warning: Weak Symbol %s not found, cannot apply R_X86_64_JUMP_SLOT @%p (%p)\n", symname, p, *(void**)p);
                        } else {
                            printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_X86_64_JUMP_SLOT @%p (%p) in %s\n", symname, p, *(void**)p, head->name);
                            ret_ok = 1;
                        }
                        // return -1;
                    } else {
                        if(p) {
                            printf_dump(LOG_NEVER, "Apply %s R_X86_64_JUMP_SLOT @%p with sym=%s (%p -> %p)\n", 
                                (bind==STB_LOCAL)?"Local":"Global", p, symname, *(void**)p, (void*)(offs+rela[i].r_addend));
                            *p = offs + rela[i].r_addend;
                        } else {
                            printf_log(LOG_INFO, "Warning, Symbol %s found, but Jump Slot Offset is NULL \n", symname);
                        }
                    }
                } else {
                    printf_dump(LOG_NEVER, "Preparing (if needed) %s R_X86_64_JUMP_SLOT @%p (0x%lx->0x%0lx) with sym=%s to be apply later (addend=%ld)\n", 
                        (bind==STB_LOCAL)?"Local":"Global", p, *p, *p+head->delta, symname, rela[i].r_addend);
                    *p += head->delta;
                    *need_resolv = 1;
                }
                break;
            case R_X86_64_64:
                if (!offs) {
                    if(symname && !strcmp(symname, "__gxx_personality_v0")) {
                        printf_dump(LOG_NEVER, "Warning: Symbol %s not found, cannot apply R_X86_64_64 @%p (%p) in %s\n", symname, p, *(void**)p, head->name);
                    } else {
                        printf_log(LOG_INFO, "%s: Symbol %s not found, cannot apply R_X86_64_64 @%p (%p) in %s\n", (bind==STB_GLOBAL)?"Error":"Warning", symname, p, *(void**)p, head->name);
                        if(bind==STB_GLOBAL)
                            ret_ok = 1;
                        // return -1;
                    }
                } else {
                    printf_dump(LOG_NEVER, "Apply %s R_X86_64_64 @%p with sym=%s (ver=%d/%s) addend=0x%lx (%p -> %p)\n", 
                        (bind==STB_LOCAL)?"Local":"Global", p, symname, version, vername?vername:"(none)", rela[i].r_addend, *(void**)p, (void*)(offs+rela[i].r_addend/*+*(uint64_t*)p*/));
                    *p /*+*/= offs+rela[i].r_addend;
                }
                break;
            case R_X86_64_TPOFF64:
                // Negated offset in static TLS block
                {
                    if(!symname || !symname[0]) {
                        h_tls = head;
                        offs = sym->st_value;
                    } else {
                        h_tls = NULL;
                        if(local_maplib)
                            h_tls = GetGlobalSymbolElf(local_maplib, symname, version, vername);
                        if(!h_tls)
                            h_tls = GetGlobalSymbolElf(maplib, symname, version, vername);
                    }
                    if(h_tls) {
                        delta = *(int64_t*)p;
                        printf_dump(LOG_NEVER, "Applying %s %s on %s @%p (%ld -> %ld+%ld+%ld, size=%ld)\n", (bind==STB_LOCAL)?"Local":"Global", DumpRelType(t), symname, p, delta, h_tls->tlsbase, (int64_t)offs, rela[i].r_addend, end-offs);
                        *p = (uintptr_t)((int64_t)offs + rela[i].r_addend + h_tls->tlsbase);
                    } else {
                        printf_log(LOG_INFO, "Warning, cannot apply %s %s on %s @%p (%ld), no elf_header found\n", (bind==STB_LOCAL)?"Local":"Global", DumpRelType(t), symname, p, (int64_t)offs);
                    }
                }
                break;
            case R_X86_64_DTPMOD64:
                // ID of module containing symbol
                if(!symname || symname[0]=='\0' || bind==STB_LOCAL)
                    offs = getElfIndex(my_context, head);
                else {
                    if(!h_tls) {
                        if(local_maplib)
                            h_tls = GetGlobalSymbolElf(local_maplib, symname, version, vername);
                        if(!h_tls)
                            h_tls = GetGlobalSymbolElf(maplib, symname, version, vername);
                    }
                    offs = getElfIndex(my_context, h_tls);
                }
                if(p) {
                    printf_dump(LOG_NEVER, "Apply %s %s @%p with sym=%s (%p -> %p)\n", (bind==STB_LOCAL)?"Local":"Global", "R_X86_64_DTPMOD64", p, symname, *(void**)p, (void*)offs);
                    *p = offs;
                } else {
                    printf_log(LOG_INFO, "Warning, Symbol %s or Elf not found, but R_X86_64_DTPMOD64 Slot Offset is NULL \n", symname);
                }
                break;
            case R_X86_64_DTPOFF64:
                // Offset in TLS block
                if (!offs && !end) {
                    if(bind==STB_WEAK) {
                        printf_log(LOG_INFO, "Warning: Weak Symbol %s not found, cannot apply R_X86_64_DTPOFF64 @%p (%p)\n", symname, p, *(void**)p);
                    } else {
                        printf_log(LOG_INFO, "Error: Symbol %s not found, cannot apply R_X86_64_DTPOFF64 @%p (%p) in %s\n", symname, p, *(void**)p, head->name);
                    }
                    // return -1;
                } else {
                    if(!symname || symname[0]=='\0')
                        offs = sym->st_value;
                    if(p) {
                        int64_t tlsoffset = offs;    // it's not an offset in elf memory
                        printf_dump(LOG_NEVER, "Apply %s R_X86_64_DTPOFF64 @%p with sym=%s (%p -> %p)\n", (bind==STB_LOCAL)?"Local":"Global", p, symname, (void*)tlsoffset, (void*)offs);
                        *p = tlsoffset;
                    } else {
                        printf_log(LOG_INFO, "Warning, Symbol %s found, but R_X86_64_DTPOFF64 Slot Offset is NULL \n", symname);
                    }
                }
                break;
            case R_X86_64_TLSDESC:
                if(!symname || !symname[0]) {
                    printf_dump(LOG_NEVER, "Apply %s R_X86_64_TLSDESC @%p with addend=%zu\n", (bind==STB_LOCAL)?"Local":"Global", p, rela[i].r_addend);
                    struct tlsdesc volatile *td = (struct tlsdesc volatile *)p;
                    if(!tlsdescUndefweak)
                        tlsdescUndefweak = AddBridge(my_context->system, pFE, _dl_tlsdesc_undefweak, 0, "_dl_tlsdesc_undefweak");
                    td->entry = tlsdescUndefweak;
                    td->arg = (uintptr_t)(head->tlsbase + rela[i].r_addend);
                } else {
                    printf_log(LOG_INFO, "Warning, R_X86_64_TLSDESC used with Symbol %s(%p) not supported for now \n", symname, sym);
                }
                break;
            default:
                printf_log(LOG_INFO, "Warning, don't know of to handle rela #%d %s on %s\n", i, DumpRelType(ELF64_R_TYPE(rela[i].r_info)), symname);
        }
    }
    return bindnow?ret_ok:0;
}
void checkHookedSymbols(lib_t *maplib, elfheader_t* h); // in mallochook.c
int RelocateElf(lib_t *maplib, lib_t *local_maplib, int bindnow, elfheader_t* head)
{
    if((head->flags&DF_BIND_NOW) && !bindnow) {
        bindnow = 1;
        printf_log(LOG_DEBUG, "Forcing %s to Bind Now\n", head->name);
    }
    if(head->rel) {
        int cnt = head->relsz / head->relent;
        DumpRelTable(head, cnt, (Elf64_Rel *)(head->rel + head->delta), "Rel");
        printf_dump(LOG_DEBUG, "Applying %d Relocation(s) for %s\n", cnt, head->name);
        if(RelocateElfREL(maplib, local_maplib, bindnow, head, cnt, (Elf64_Rel *)(head->rel + head->delta)))
            return -1;
    }
    if(head->rela) {
        int cnt = head->relasz / head->relaent;
        DumpRelATable(head, cnt, (Elf64_Rela *)(head->rela + head->delta), "RelA");
        printf_dump(LOG_DEBUG, "Applying %d Relocation(s) with Addend for %s\n", cnt, head->name);
        if(RelocateElfRELA(maplib, local_maplib, bindnow, head, cnt, (Elf64_Rela *)(head->rela + head->delta), NULL))
            return -1;
    }
    checkHookedSymbols(maplib, head);
    return 0;
}

int RelocateElfPlt(lib_t *maplib, lib_t *local_maplib, int bindnow, elfheader_t* head)
{
    int need_resolver = 0;
    if((head->flags&DF_BIND_NOW) && !bindnow) {
        bindnow = 1;
        printf_log(LOG_DEBUG, "Forcing %s to Bind Now\n", head->name);
    }
    if(head->pltrel) {
        int cnt = head->pltsz / head->pltent;
        if(head->pltrel==DT_REL) {
            DumpRelTable(head, cnt, (Elf64_Rel *)(head->jmprel + head->delta), "PLT");
            printf_dump(LOG_DEBUG, "Applying %d PLT Relocation(s) for %s\n", cnt, head->name);
            if(RelocateElfREL(maplib, local_maplib, bindnow, head, cnt, (Elf64_Rel *)(head->jmprel + head->delta)))
                return -1;
        } else if(head->pltrel==DT_RELA) {
            DumpRelATable(head, cnt, (Elf64_Rela *)(head->jmprel + head->delta), "PLT");
            printf_dump(LOG_DEBUG, "Applying %d PLT Relocation(s) with Addend for %s\n", cnt, head->name);
            if(RelocateElfRELA(maplib, local_maplib, bindnow, head, cnt, (Elf64_Rela *)(head->jmprel + head->delta), &need_resolver))
                return -1;
        }
        if(need_resolver) {
            if(pltResolver==~0LL) {
                pltResolver = AddBridge(my_context->system, vFE, PltResolver, 0, "PltResolver");
            }
            if(head->pltgot) {
                *(uintptr_t*)(head->pltgot+head->delta+16) = pltResolver;
                *(uintptr_t*)(head->pltgot+head->delta+8) = (uintptr_t)head;
                printf_dump(LOG_DEBUG, "PLT Resolver injected in plt.got at %p\n", (void*)(head->pltgot+head->delta+16));
            } else if(head->got) {
                *(uintptr_t*)(head->got+head->delta+16) = pltResolver;
                *(uintptr_t*)(head->got+head->delta+8) = (uintptr_t)head;
                printf_dump(LOG_DEBUG, "PLT Resolver injected in got at %p\n", (void*)(head->got+head->delta+16));
            }
        }
    }
   
    return 0;
}

void CalcStack(elfheader_t* elf, uint64_t* stacksz, size_t* stackalign)
{
    if(*stacksz < elf->stacksz)
        *stacksz = elf->stacksz;
    if(*stackalign < elf->stackalign)
        *stackalign = elf->stackalign;
}

Elf64_Sym* GetFunction(elfheader_t* h, const char* name)
{
    // TODO: create a hash on named to avoid this loop
    for (size_t i=0; i<h->numSymTab; ++i) {
        int type = ELF64_ST_TYPE(h->SymTab[i].st_info);
        if(type==STT_FUNC) {
            const char * symname = h->StrTab+h->SymTab[i].st_name;
            if(strcmp(symname, name)==0) {
                return h->SymTab+i;
            }
        }
    }
    return NULL;
}

Elf64_Sym* GetElfObject(elfheader_t* h, const char* name)
{
    for (size_t i=0; i<h->numSymTab; ++i) {
        int type = ELF64_ST_TYPE(h->SymTab[i].st_info);
        if(type==STT_OBJECT) {
            const char * symname = h->StrTab+h->SymTab[i].st_name;
            if(strcmp(symname, name)==0) {
                return h->SymTab+i;
            }
        }
    }
    return NULL;
}


uintptr_t GetFunctionAddress(elfheader_t* h, const char* name)
{
    Elf64_Sym* sym = GetFunction(h, name);
    if(sym) return sym->st_value;
    return 0;
}

uintptr_t GetEntryPoint(lib_t* maplib, elfheader_t* h)
{
    (void)maplib;
    uintptr_t ep = h->entrypoint + h->delta;
    printf_log(LOG_DEBUG, "Entry Point is %p\n", (void*)ep);
    if(box64_dump) {
        printf_dump(LOG_NEVER, "(short) Dump of Entry point\n");
        int sz = 64;
        uintptr_t lastbyte = GetLastByte(h);
        if (ep + sz >  lastbyte)
            sz = lastbyte - ep;
        DumpBinary((char*)ep, sz);
    }
    return ep;
}

uintptr_t GetLastByte(elfheader_t* h)
{
    return (uintptr_t)h->memory/* + h->delta*/ + h->memsz;
}

void AddSymbols(lib_t *maplib, kh_mapsymbols_t* mapsymbols, kh_mapsymbols_t* weaksymbols, kh_mapsymbols_t* localsymbols, elfheader_t* h)
{
    if(box64_dump && h->DynSym) DumpDynSym(h);
    printf_dump(LOG_NEVER, "Will look for Symbol to add in SymTable(%zu)\n", h->numSymTab);
    for (size_t i=0; i<h->numSymTab; ++i) {
        const char * symname = h->StrTab+h->SymTab[i].st_name;
        int bind = ELF64_ST_BIND(h->SymTab[i].st_info);
        int type = ELF64_ST_TYPE(h->SymTab[i].st_info);
        int vis = h->SymTab[i].st_other&0x3;
        size_t sz = h->SymTab[i].st_size;
        if((type==STT_OBJECT || type==STT_FUNC || type==STT_COMMON || type==STT_TLS  || type==STT_NOTYPE) 
        && (vis==STV_DEFAULT || vis==STV_PROTECTED || (vis==STV_HIDDEN && bind==STB_LOCAL)) && (h->SymTab[i].st_shndx!=0)) {
            if(sz && strstr(symname, "@@")) {
                char symnameversioned[strlen(symname)+1];
                strcpy(symnameversioned, symname);
                // extact symname@@vername
                char* p = strchr(symnameversioned, '@');
                *p=0;
                p+=2;
                symname = AddDictionnary(my_context->versym, symnameversioned);
                const char* vername = AddDictionnary(my_context->versym, p);
                AddDefaultVersion((bind==STB_WEAK)?h->weakdefver:h->globaldefver, symname, vername);
                if((bind==STB_GNU_UNIQUE /*|| (bind==STB_GLOBAL && type==STT_FUNC)*/) && FindGlobalSymbol(maplib, symname, 2, p))
                    continue;
                uintptr_t offs = (type==STT_TLS)?h->SymTab[i].st_value:(h->SymTab[i].st_value + h->delta);
                printf_dump(LOG_NEVER, "Adding Default Versioned Symbol(bind=%s) \"%s@%s\" with offset=%p sz=%zu\n", (bind==STB_LOCAL)?"LOCAL":((bind==STB_WEAK)?"WEAK":"GLOBAL"), symname, vername, (void*)offs, sz);
                if(bind==STB_LOCAL)
                    AddSymbol(localsymbols, symname, offs, sz, 2, vername);
                else    // add in local and global map 
                    if(bind==STB_WEAK) {
                        AddSymbol(weaksymbols, symname, offs, sz, 2, vername);
                    } else {
                        AddSymbol(mapsymbols, symname, offs, sz, 2, vername);
                    }
            } else {
                int to_add = 1;
                if(!to_add || (bind==STB_GNU_UNIQUE && FindGlobalSymbol(maplib, symname, -1, NULL)))
                    continue;
                uintptr_t offs = (type==STT_TLS)?h->SymTab[i].st_value:(h->SymTab[i].st_value + h->delta);
                printf_dump(LOG_NEVER, "Adding Symbol(bind=%s) \"%s\" with offset=%p sz=%zu\n", (bind==STB_LOCAL)?"LOCAL":((bind==STB_WEAK)?"WEAK":"GLOBAL"), symname, (void*)offs, sz);
                if(bind==STB_LOCAL)
                    AddSymbol(localsymbols, symname, offs, sz, 1, NULL);
                else    // add in local and global map 
                    if(bind==STB_WEAK) {
                        AddSymbol(weaksymbols, symname, offs, sz, 1, NULL);
                    } else {
                        AddSymbol(mapsymbols, symname, offs, sz, 1, NULL);
                    }
            }
        }
    }
    
    printf_dump(LOG_NEVER, "Will look for Symbol to add in DynSym (%zu)\n", h->numDynSym);
    for (size_t i=0; i<h->numDynSym; ++i) {
        const char * symname = h->DynStr+h->DynSym[i].st_name;
        int bind = ELF64_ST_BIND(h->DynSym[i].st_info);
        int type = ELF64_ST_TYPE(h->DynSym[i].st_info);
        int vis = h->DynSym[i].st_other&0x3;
        if((type==STT_OBJECT || type==STT_FUNC || type==STT_COMMON || type==STT_TLS  || type==STT_NOTYPE) 
        && (vis==STV_DEFAULT || vis==STV_PROTECTED || (vis==STV_HIDDEN && bind==STB_LOCAL)) && (h->DynSym[i].st_shndx!=0 && h->DynSym[i].st_shndx<=65521)) {
            uintptr_t offs = (type==STT_TLS)?h->DynSym[i].st_value:(h->DynSym[i].st_value + h->delta);
            size_t sz = h->DynSym[i].st_size;
            int version = h->VerSym?((Elf64_Half*)((uintptr_t)h->VerSym+h->delta))[i]:-1;
            int add_default = (version!=-1 && (version&0x7fff)>1 && !(version&0x8000) && !GetMaplibDefaultVersion(my_context->maplib, (maplib==my_context->maplib)?NULL:maplib, (bind==STB_WEAK)?1:0, symname))?1:0;
            if(version!=-1) version &= 0x7fff;
            const char* vername = GetSymbolVersion(h, version);
            if(add_default) {
                AddDefaultVersion((bind==STB_WEAK)?h->weakdefver:h->globaldefver, symname, vername);
                printf_dump(LOG_NEVER, "Adding Default Version \"%s\" for Symbol\"%s\"\n", vername, symname);
            }
            int to_add = 1;
            if(!to_add || (bind==STB_GNU_UNIQUE && FindGlobalSymbol(maplib, symname, version, vername)))
                continue;
            printf_dump(LOG_NEVER, "Adding Versioned Symbol(bind=%s) \"%s\" (ver=%d/%s) with offset=%p sz=%zu\n", (bind==STB_LOCAL)?"LOCAL":((bind==STB_WEAK)?"WEAK":"GLOBAL"), symname, version, vername?vername:"(none)", (void*)offs, sz);
            if(bind==STB_LOCAL)
                AddSymbol(localsymbols, symname, offs, sz, version, vername);
            else // add in local and global map 
                if(bind==STB_WEAK) {
                    AddSymbol(weaksymbols, symname, offs, sz, version, vername);
                } else {
                    AddSymbol(mapsymbols, symname, offs, sz, version?version:1, vername);
                }
        }
    }
}

/*
$ORIGIN – Provides the directory the object was loaded from. This token is typical
used for locating dependencies in unbundled packages. For more details of this
token expansion, see “Locating Associated Dependencies”
$OSNAME – Expands to the name of the operating system (see the uname(1) man
page description of the -s option). For more details of this token expansion, see
“System Specific Shared Objects”
$OSREL – Expands to the operating system release level (see the uname(1) man
page description of the -r option). For more details of this token expansion, see
“System Specific Shared Objects”
$PLATFORM – Expands to the processor type of the current machine (see the
uname(1) man page description of the -i option). For more details of this token
expansion, see “System Specific Shared Objects”
*/
int LoadNeededLibs(elfheader_t* h, lib_t *maplib, int local, int bindnow, box64context_t *box64, x64emu_t* emu)
{
    if(h->needed)   // already done
        return 0;
    DumpDynamicRPath(h);
    // update RPATH first
    for (size_t i=0; i<h->numDynamic; ++i)
        if(h->Dynamic[i].d_tag==DT_RPATH || h->Dynamic[i].d_tag==DT_RUNPATH) {
            char *rpathref = h->DynStrTab+h->delta+h->Dynamic[i].d_un.d_val;
            char* rpath = rpathref;
            while(strstr(rpath, "$ORIGIN")) {
                char* origin = box_strdup(h->path);
                char* p = strrchr(origin, '/');
                if(p) *p = '\0';    // remove file name to have only full path, without last '/'
                char* tmp = (char*)box_calloc(1, strlen(rpath)-strlen("$ORIGIN")+strlen(origin)+1);
                p = strstr(rpath, "$ORIGIN");
                memcpy(tmp, rpath, p-rpath);
                strcat(tmp, origin);
                strcat(tmp, p+strlen("$ORIGIN"));
                if(rpath!=rpathref)
                    box_free(rpath);
                rpath = tmp;
                box_free(origin);
            }
            while(strstr(rpath, "${ORIGIN}")) {
                char* origin = box_strdup(h->path);
                char* p = strrchr(origin, '/');
                if(p) *p = '\0';    // remove file name to have only full path, without last '/'
                char* tmp = (char*)box_calloc(1, strlen(rpath)-strlen("${ORIGIN}")+strlen(origin)+1);
                p = strstr(rpath, "${ORIGIN}");
                memcpy(tmp, rpath, p-rpath);
                strcat(tmp, origin);
                strcat(tmp, p+strlen("${ORIGIN}"));
                if(rpath!=rpathref)
                    box_free(rpath);
                rpath = tmp;
                box_free(origin);
            }
            while(strstr(rpath, "${PLATFORM}")) {
                char* platform = box_strdup("x86_64");
                char* p = strrchr(platform, '/');
                if(p) *p = '\0';    // remove file name to have only full path, without last '/'
                char* tmp = (char*)box_calloc(1, strlen(rpath)-strlen("${PLATFORM}")+strlen(platform)+1);
                p = strstr(rpath, "${PLATFORM}");
                memcpy(tmp, rpath, p-rpath);
                strcat(tmp, platform);
                strcat(tmp, p+strlen("${PLATFORM}"));
                if(rpath!=rpathref)
                    box_free(rpath);
                rpath = tmp;
                box_free(platform);
            }
            if(strchr(rpath, '$')) {
                printf_log(LOG_INFO, "BOX64: Warning, RPATH with $ variable not supported yet (%s)\n", rpath);
            } else {
                printf_log(LOG_DEBUG, "Prepending path \"%s\" to BOX64_LD_LIBRARY_PATH\n", rpath);
                PrependList(&box64->box64_ld_lib, rpath, 1);
            }
            if(rpath!=rpathref)
                box_free(rpath);
        }

    DumpDynamicNeeded(h);
    int cnt = 0;
    for (int i=0; i<h->numDynamic; ++i)
        if(h->Dynamic[i].d_tag==DT_NEEDED)
            ++cnt;
    h->needed = new_neededlib(cnt);
    if(h == my_context->elfs[0])
        my_context->neededlibs = h->needed;
    int j=0;
    for (int i=0; i<h->numDynamic; ++i)
        if(h->Dynamic[i].d_tag==DT_NEEDED)
            h->needed->names[j++] = h->DynStrTab+h->delta+h->Dynamic[i].d_un.d_val;

    // TODO: Add LD_LIBRARY_PATH and RPATH handling
    if(AddNeededLib(maplib, local, bindnow, h->needed, box64, emu)) {
        printf_log(LOG_INFO, "Error loading one of needed lib\n");
        if(!allow_missing_libs)
            return 1;   //error...
    }
    return 0;
}

int ElfCheckIfUseTCMallocMinimal(elfheader_t* h)
{
    if(!h)
        return 0;
    for (size_t i=0; i<h->numDynamic; ++i)
        if(h->Dynamic[i].d_tag==DT_NEEDED) {
            char *needed = h->DynStrTab+h->delta+h->Dynamic[i].d_un.d_val;
            if(!strcmp(needed, "libtcmalloc_minimal.so.4")) // tcmalloc needs to be the 1st lib loaded
                return 1;
            else if(!strcmp(needed, "libtcmalloc_minimal.so.0")) // tcmalloc needs to be the 1st lib loaded
                return 1;
            else
                return 0;
        }
    return 0;
}

void RefreshElfTLS(elfheader_t* h)
{
    if(h->tlsfilesize) {
        char* dest = (char*)(my_context->tlsdata+my_context->tlssize+h->tlsbase);
        printf_dump(LOG_DEBUG, "Refreshing main TLS block @%p from %p:0x%lx\n", dest, (void*)h->tlsaddr, h->tlsfilesize);
        memcpy(dest, (void*)(h->tlsaddr+h->delta), h->tlsfilesize);
        tlsdatasize_t* ptr;
        if ((ptr = (tlsdatasize_t*)pthread_getspecific(my_context->tlskey)) != NULL) {
            // refresh in tlsdata too
            dest = (char*)(ptr->data+h->tlsbase);
            printf_dump(LOG_DEBUG, "Refreshing active TLS block @%p from %p:0x%lx\n", dest, (void*)h->tlsaddr, h->tlssize-h->tlsfilesize);
            memcpy(dest, (void*)(h->tlsaddr+h->delta), h->tlsfilesize);
        }
    }
}
void MarkElfInitDone(elfheader_t* h)
{
    if(h)
        h->init_done = 1;
}
void RunElfInitPltResolver(elfheader_t* h, x64emu_t *emu)
{
    if(!h || h->init_done)
        return;
    uintptr_t p = h->initentry + h->delta;
    h->init_done = 1;
    for(int i=0; i<h->needed->size; ++i) {
        library_t *lib = h->needed->libs[i];
        elfheader_t *lib_elf = GetElf(lib);
        if(lib_elf)
            RunElfInitPltResolver(lib_elf, emu);
    }
    printf_dump(LOG_DEBUG, "Calling Init for %s @%p\n", ElfName(h), (void*)p);
    if(h->initentry)
        RunSafeFunction(my_context, p, 3, my_context->argc, my_context->argv, my_context->envv);
    printf_dump(LOG_DEBUG, "Done Init for %s\n", ElfName(h));
    // and check init array now
    Elf64_Addr *addr = (Elf64_Addr*)(h->initarray + h->delta);
    for (size_t i=0; i<h->initarray_sz; ++i) {
        if(addr[i]) {
            printf_dump(LOG_DEBUG, "Calling Init[%zu] for %s @%p\n", i, ElfName(h), (void*)addr[i]);
            RunSafeFunction(my_context, (uintptr_t)addr[i], 3, my_context->argc, my_context->argv, my_context->envv);
        }
    }

    h->fini_done = 0;   // can be fini'd now (in case it was re-inited)
    printf_dump(LOG_DEBUG, "All Init Done for %s\n", ElfName(h));
    return;
}

void RunElfInit(elfheader_t* h, x64emu_t *emu)
{
    if(!h || h->init_done)
        return;
    // reset Segs Cache
    memset(emu->segs_serial, 0, sizeof(emu->segs_serial));
    uintptr_t p = h->initentry + h->delta;
    box64context_t* context = GetEmuContext(emu);
    // Refresh no-file part of TLS in case default value changed
    RefreshElfTLS(h);
    // check if in deferredInit
    if(context->deferredInit) {
        if(context->deferredInitSz==context->deferredInitCap) {
            context->deferredInitCap += 4;
            context->deferredInitList = (elfheader_t**)box_realloc(context->deferredInitList, context->deferredInitCap*sizeof(elfheader_t*));
        }
        context->deferredInitList[context->deferredInitSz++] = h;
        return;
    }
    h->init_done = 1;
    for(int i=0; i<h->needed->size; ++i) {
        library_t *lib = h->needed->libs[i];
        elfheader_t *lib_elf = GetElf(lib);
        if(lib_elf)
            RunElfInit(lib_elf, emu);
    }
    printf_dump(LOG_DEBUG, "Calling Init for %s @%p\n", ElfName(h), (void*)p);
    if(h->initentry)
        RunFunctionWithEmu(emu, 0, p, 3, context->argc, context->argv, context->envv);
    printf_dump(LOG_DEBUG, "Done Init for %s\n", ElfName(h));
    // and check init array now
    Elf64_Addr *addr = (Elf64_Addr*)(h->initarray + h->delta);
    for (size_t i=0; i<h->initarray_sz; ++i) {
        if(addr[i]) {
            printf_dump(LOG_DEBUG, "Calling Init[%zu] for %s @%p\n", i, ElfName(h), (void*)addr[i]);
            RunFunctionWithEmu(emu, 0, (uintptr_t)addr[i], 3, context->argc, context->argv, context->envv);
        }
    }

    h->fini_done = 0;   // can be fini'd now (in case it was re-inited)
    printf_dump(LOG_DEBUG, "All Init Done for %s\n", ElfName(h));
    return;
}

EXPORTDYN
void RunDeferredElfInit(x64emu_t *emu)
{
    box64context_t* context = GetEmuContext(emu);
    if(!context->deferredInit)
        return;
    context->deferredInit = 0;
    if(!context->deferredInitList)
        return;
    int Sz = context->deferredInitSz;
    elfheader_t** List = context->deferredInitList;
    context->deferredInitList = NULL;
    context->deferredInitCap = context->deferredInitSz = 0;
    for (int i=0; i<Sz; ++i)
        RunElfInit(List[i], emu);
    box_free(List);
}

void RunElfFini(elfheader_t* h, x64emu_t *emu)
{
    if(!h || h->fini_done || !h->init_done)
        return;
    h->fini_done = 1;
    // first check fini array
    Elf64_Addr *addr = (Elf64_Addr*)(h->finiarray + h->delta);
    for (int i=h->finiarray_sz-1; i>=0; --i) {
        printf_dump(LOG_DEBUG, "Calling Fini[%d] for %s @%p\n", i, ElfName(h), (void*)addr[i]);
        RunFunctionWithEmu(emu, 0, (uintptr_t)addr[i], 0);
    }
    // then the "old-style" fini
    if(h->finientry) {
        uintptr_t p = h->finientry + h->delta;
        printf_dump(LOG_DEBUG, "Calling Fini for %s @%p\n", ElfName(h), (void*)p);
        RunFunctionWithEmu(emu, 0, p, 0);
    }
    h->init_done = 0;   // can be re-inited again...
    return;
}

uintptr_t GetElfInit(elfheader_t* h)
{
    return h->initentry + h->delta;
}
uintptr_t GetElfFini(elfheader_t* h)
{
    return h->finientry + h->delta;
}

void* GetBaseAddress(elfheader_t* h)
{
    return h->memory;
}

void* GetElfDelta(elfheader_t* h)
{
    return (void*)h->delta;
}

uint32_t GetBaseSize(elfheader_t* h)
{
    return h->memsz;
}

int IsAddressInElfSpace(const elfheader_t* h, uintptr_t addr)
{
    if(!h)
        return 0;
    for(int i=0; i<h->multiblock_n; ++i) {
        uintptr_t base = h->multiblock_offs[i];
        uintptr_t end = h->multiblock_offs[i] + h->multiblock_size[i] - 1;
        if(addr>=base && addr<=end)
            return 1;
        
    }
    return 0;
}
elfheader_t* FindElfAddress(box64context_t *context, uintptr_t addr)
{
    for (int i=0; i<context->elfsize; ++i)
        if(IsAddressInElfSpace(context->elfs[i], addr))
            return context->elfs[i];
    
    return NULL;
}

const char* FindNearestSymbolName(elfheader_t* h, void* p, uintptr_t* start, uint64_t* sz)
{
    uintptr_t addr = (uintptr_t)p;

    uint32_t distance = 0x7fffffff;
    const char* ret = NULL;
    uintptr_t s = 0;
    uint64_t size = 0;
    #ifdef HAVE_TRACE
    if(!h) {
        if(getProtection((uintptr_t)p)&(PROT_READ)) {
            if(*(uint8_t*)(p)==0xCC && *(uint8_t*)(p+1)=='S' && *(uint8_t*)(p+2)=='C') {
                ret = getBridgeName(*(void**)(p+3+8));
                if(ret) {
                    if(start)
                        *start = (uintptr_t)p;
                    if(sz)
                        *sz = 32;
                }
            }
        }
        return ret;
    }
    #endif
    if(!h || h->fini_done)
        return ret;

    for (size_t i=0; i<h->numSymTab && distance!=0; ++i) {
        const char * symname = h->StrTab+h->SymTab[i].st_name;
        uintptr_t offs = h->SymTab[i].st_value + h->delta;

        if(offs<=addr) {
            if(distance>addr-offs) {
                distance = addr-offs;
                ret = symname;
                s = offs;
                size = h->SymTab[i].st_size;
            }
        }
    }
    for (size_t i=0; i<h->numDynSym && distance!=0; ++i) {
        const char * symname = h->DynStr+h->DynSym[i].st_name;
        uintptr_t offs = h->DynSym[i].st_value + h->delta;

        if(offs<=addr) {
            if(distance>addr-offs) {
                distance = addr-offs;
                ret = symname;
                s = offs;
                size = h->DynSym[i].st_size;
            }
        }
    }

    if(start)
        *start = s;
    if(sz)
        *sz = size;

    return ret;
}

const char* VersionedName(const char* name, int ver, const char* vername)
{
    if(ver==-1)
        return name;
    const char *v=NULL;
    if(ver==0)
        v="";
    if(ver==1)
        v="*";
    if(!v && !vername)
        return name;
    if(ver>1)
        v = vername;
    char buf[strlen(name)+strlen(v)+1+1];
    strcpy(buf, name);
    strcat(buf, "@");
    strcat(buf, v);
    return AddDictionnary(my_context->versym, buf);
}

int SameVersionedSymbol(const char* name1, int ver1, const char* vername1, const char* name2, int ver2, const char* vername2)
{
    if(strcmp(name1, name2))    //name are different, no need to go further
        return 0;
    if(ver1==-1 || ver2==-1)    // don't check version, so ok
        return 1;
    if(ver1==ver2 && ver1<2)    // same ver (local or global), ok
        return 1;
    if(ver1==0 || ver2==0)  // one is local, the other is not, no match
        return 0;
    if(ver1==1 || ver2==1)  // one if global, ok
        return 1;
    if(!strcmp(vername1, vername2))  // same vername
        return 1;
    return 0;
}

void* GetDTatOffset(box64context_t* context, unsigned long int index, unsigned long int offset)
{
    return (void*)((char*)GetTLSPointer(context, context->elfs[index])+offset);
}

int32_t GetTLSBase(elfheader_t* h)
{
    return h?h->tlsbase:0;
}

uint32_t GetTLSSize(elfheader_t* h)
{
    return h?h->tlssize:0;
}

void* GetTLSPointer(box64context_t* context, elfheader_t* h)
{
    if(!h || !h->tlssize)
        return NULL;
    tlsdatasize_t* ptr = getTLSData(context);
    return ptr->data+h->tlsbase;
}

void* GetDynamicSection(elfheader_t* h)
{
    if(!h)
        return NULL;
    return h->Dynamic;
}

#ifdef DYNAREC
dynablock_t* GetDynablocksFromAddress(box64context_t *context, uintptr_t addr)
{
    (void)context;
    // if we are here, the there is not block in standard "space"
    /*dynablocklist_t* ret = getDBFromAddress(addr);
    if(ret) {
        return ret;
    }*/
    if(box64_dynarec_forced) {
        addDBFromAddressRange(addr, 1);
        return getDB(addr);
    }
    //check if address is in an elf... if yes, grant a block (should I warn)
    Dl_info info;
    if(dladdr((void*)addr, &info)) {
        dynarec_log(LOG_INFO, "Address %p is in a native Elf memory space (function \"%s\" in %s)\n", (void*)addr, info.dli_sname, info.dli_fname);
        return NULL;
    }
    dynarec_log(LOG_INFO, "Address %p not found in Elf memory and is not a native call wrapper\n", (void*)addr);
    return NULL;
}
#endif

typedef struct my_dl_phdr_info_s {
    void*           dlpi_addr;
    const char*     dlpi_name;
    Elf64_Phdr*     dlpi_phdr;
    Elf64_Half      dlpi_phnum;
} my_dl_phdr_info_t;

static int dl_iterate_phdr_callback(x64emu_t *emu, void* F, my_dl_phdr_info_t *info, size_t size, void* data)
{
    int ret = RunFunctionWithEmu(emu, 0, (uintptr_t)F, 3, info, size, data);
    return ret;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// dl_iterate_phdr ...
#define GO(A)   \
static uintptr_t my_dl_iterate_phdr_fct_##A = 0;                                    \
static int my_dl_iterate_phdr_##A(struct dl_phdr_info* a, size_t b, void* c)        \
{                                                                                   \
    if(!a->dlpi_name)                                                               \
        return 0;                                                                   \
    if(!a->dlpi_name[0]) /*don't send informations about box64 itself*/             \
        return 0;                                                                   \
    return (int)RunFunction(my_context, my_dl_iterate_phdr_fct_##A, 3, a, b, c);    \
}
SUPER()
#undef GO
static void* find_dl_iterate_phdr_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_dl_iterate_phdr_fct_##A == (uintptr_t)fct) return my_dl_iterate_phdr_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_dl_iterate_phdr_fct_##A == 0) {my_dl_iterate_phdr_fct_##A = (uintptr_t)fct; return my_dl_iterate_phdr_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for elfloader dl_iterate_phdr callback\n");
    return NULL;
}
#undef SUPER

EXPORT int my_dl_iterate_phdr(x64emu_t *emu, void* F, void *data) {
    printf_log(LOG_DEBUG, "Call to partially implemented dl_iterate_phdr(%p, %p)\n", F, data);
    box64context_t *context = GetEmuContext(emu);
    const char* empty = "";
    int ret = 0;
    for (int idx=0; idx<context->elfsize; ++idx) {
        if(context->elfs[idx]) {
            my_dl_phdr_info_t info;
            info.dlpi_addr = GetElfDelta(context->elfs[idx]);
            info.dlpi_name = idx?context->elfs[idx]->name:empty;    //1st elf is program, and this one doesn't get a name
            info.dlpi_phdr = context->elfs[idx]->PHEntries;
            info.dlpi_phnum = context->elfs[idx]->numPHEntries;
            if((ret = dl_iterate_phdr_callback(emu, F, &info, sizeof(info), data))) {
                return ret;
            }
        }
    }
    // and now, go on native version
    ret = dl_iterate_phdr(find_dl_iterate_phdr_Fct(F), data);
    return ret;
}

void ResetSpecialCaseMainElf(elfheader_t* h)
{
    Elf64_Sym *sym = NULL;
    for (size_t i=0; i<h->numDynSym; ++i) {
        if(h->DynSym[i].st_info == 17) {
            sym = h->DynSym+i;
            const char * symname = h->DynStr+sym->st_name;
            if(strcmp(symname, "_IO_2_1_stderr_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stderr, sym->st_size);
                my__IO_2_1_stderr_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "BOX64: Set @_IO_2_1_stderr_ to %p\n", my__IO_2_1_stderr_);
            } else
            if(strcmp(symname, "_IO_2_1_stdin_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stdin, sym->st_size);
                my__IO_2_1_stdin_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "BOX64: Set @_IO_2_1_stdin_ to %p\n", my__IO_2_1_stdin_);
            } else
            if(strcmp(symname, "_IO_2_1_stdout_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stdout, sym->st_size);
                my__IO_2_1_stdout_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "BOX64: Set @_IO_2_1_stdout_ to %p\n", my__IO_2_1_stdout_);
            } else
            if(strcmp(symname, "_IO_stderr_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stderr, sym->st_size);
                my__IO_2_1_stderr_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "BOX64: Set @_IO_stderr_ to %p\n", my__IO_2_1_stderr_);
            } else
            if(strcmp(symname, "_IO_stdin_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stdin, sym->st_size);
                my__IO_2_1_stdin_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "BOX64: Set @_IO_stdin_ to %p\n", my__IO_2_1_stdin_);
            } else
            if(strcmp(symname, "_IO_stdout_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stdout, sym->st_size);
                my__IO_2_1_stdout_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "BOX64: Set @_IO_stdout_ to %p\n", my__IO_2_1_stdout_);
            }
        }
    }
}


void CreateMemorymapFile(box64context_t* context, int fd)
{
    // this will tranform current memory map
    // by anotating anonymous entry that belong to emulated elf
    // also anonymising current stack
    // and setting emulated stack as the current one

    char* line = NULL;
    size_t len = 0;
    char buff[1024];
    int dummy;
    FILE* f = fopen("/proc/self/maps", "r");
    if(!f)
        return;
    while(getline(&line, &len, f)>0) {
        // line is like
        // aaaadd750000-aaaadd759000 r-xp 00000000 103:02 13386730                  /usr/bin/cat
        uintptr_t start, end;
        if (sscanf(line, "%zx-%zx", &start, &end)==2) {
            elfheader_t* h = FindElfAddress(my_context, start);
            int l = strlen(line);
            if(h && l<73) {
                sprintf(buff, "%s%*s\n", line, 74-l, h->name);
                dummy = write(fd, buff, strlen(buff));
            } else if(start==(uintptr_t)my_context->stack) {
                sprintf(buff, "%s%*s\n", line, 74-l, "[stack]");
                dummy = write(fd, buff, strlen(buff));
            } else if (strstr(line, "[stack]")) {
                char* p = strstr(line, "[stack]")-1;
                while (*p==' ' || *p=='\t') --p;
                p[1]='\0';
                strcat(line, "\n");
                dummy = write(fd, line, strlen(line));
            } else {
                dummy = write(fd, line, strlen(line));
            }
        }
    }
    fclose(f);
    (void)dummy;
}

void ElfAttachLib(elfheader_t* head, library_t* lib)
{
    if(!head)
        return;
    head->lib = lib;
}

kh_mapsymbols_t* GetMapSymbols(elfheader_t* h)
{
    if(!h)
        return NULL;
    return h->mapsymbols;
}
kh_mapsymbols_t* GetWeakSymbols(elfheader_t* h)
{
    if(!h)
        return NULL;
    return h->weaksymbols;
}
kh_mapsymbols_t* GetLocalSymbols(elfheader_t* h)
{
    if(!h)
        return NULL;
    return h->localsymbols;
}

typedef struct search_symbol_s{
    const char* name;
    void*       addr;
    void*       lib;
} search_symbol_t;
int dl_iterate_phdr_findsymbol(struct dl_phdr_info* info, size_t size, void* data)
{
    search_symbol_t* s = (search_symbol_t*)data;

    for(int j = 0; j<info->dlpi_phnum; ++j) {
        if (info->dlpi_phdr[j].p_type == PT_DYNAMIC) {
            ElfW(Sym)* sym = NULL;
            ElfW(Word) sym_cnt = 0;
            ElfW(Verdef)* verdef = NULL;
            ElfW(Word) verdef_cnt = 0;
            char *strtab = NULL;
            ElfW(Dyn)* dyn = (ElfW(Dyn)*)(info->dlpi_addr +  info->dlpi_phdr[j].p_vaddr); //Dynamic Section
            // grab the needed info
            while(dyn->d_tag != DT_NULL) {
                switch(dyn->d_tag) {
                    case DT_STRTAB:
                        strtab = (char *)(dyn->d_un.d_ptr);
                        break;
                    case DT_VERDEF:
                        verdef = (ElfW(Verdef)*)(info->dlpi_addr +  dyn->d_un.d_ptr);
                        break;
                    case DT_VERDEFNUM:
                        verdef_cnt = dyn->d_un.d_val;
                        break;
                }
                ++dyn;
            }
            if(strtab && verdef && verdef_cnt) {
                if((uintptr_t)strtab < (uintptr_t)info->dlpi_addr) // this test is need for linux-vdso on PI and some other OS (looks like a bug to me)
                    strtab=(char*)((uintptr_t)strtab + info->dlpi_addr);
                // Look fr all defined versions now
                ElfW(Verdef)* v = verdef;
                while(v) {
                    ElfW(Verdaux)* vda = (ElfW(Verdaux)*)(((uintptr_t)v) + v->vd_aux);
                    if(v->vd_version>0 && !v->vd_flags)
                        for(int i=0; i<v->vd_cnt; ++i) {
                            const char* vername = (strtab+vda->vda_name);
                            if(vername && vername[0] && (s->addr = dlvsym(s->lib, s->name, vername))) {
                                printf_log(/*LOG_DEBUG*/LOG_INFO, "Found symbol with version %s, value = %p\n", vername, s->addr);
                                return 1;   // stop searching
                            }
                            vda = (ElfW(Verdaux)*)(((uintptr_t)vda) + vda->vda_next);
                        }
                    v = v->vd_next?(ElfW(Verdef)*)((uintptr_t)v + v->vd_next):NULL;
                }
            }
        }
    }
    return 0;
}

void* GetNativeSymbolUnversioned(void* lib, const char* name)
{
    // try to find "name" in loaded elf, whithout checking for the symbol version (like dlsym, but no version check)
    search_symbol_t s;
    s.name = name;
    s.addr = NULL;
    if(lib) 
        s.lib = lib;
    else 
        s.lib = my_context->box64lib;
    printf_log(LOG_INFO, "Look for %s in loaded elfs\n", name);
    dl_iterate_phdr(dl_iterate_phdr_findsymbol, &s);
    return s.addr;
}

kh_defaultversion_t* GetGlobalDefaultVersion(elfheader_t* h)
{
    return h?h->globaldefver:NULL;
}
kh_defaultversion_t* GetWeakDefaultVersion(elfheader_t* h)
{
    return h?h->weakdefver:NULL;
}


uintptr_t pltResolver = ~0LL;
EXPORT void PltResolver(x64emu_t* emu)
{
    uintptr_t addr = Pop64(emu);
    int slot = (int)Pop64(emu);
    elfheader_t *h = (elfheader_t*)addr;
    printf_dump(LOG_DEBUG, "PltResolver: Addr=%p, Slot=%d Return=%p: elf is %s (VerSym=%p)\n", (void*)addr, slot, *(void**)(R_RSP), h->name, h->VerSym);

    Elf64_Rela * rel = (Elf64_Rela *)(h->jmprel + h->delta) + slot;

    Elf64_Sym *sym = &h->DynSym[ELF64_R_SYM(rel->r_info)];
    int bind = ELF64_ST_BIND(sym->st_info);
    const char* symname = SymName(h, sym);
    int version = h->VerSym?((Elf64_Half*)((uintptr_t)h->VerSym+h->delta))[ELF64_R_SYM(rel->r_info)]:-1;
    if(version!=-1) version &= 0x7fff;
    const char* vername = GetSymbolVersion(h, version);
    uint64_t *p = (uint64_t*)(rel->r_offset + h->delta);
    uintptr_t offs = 0;
    uintptr_t end = 0;

    library_t* lib = h->lib;
    lib_t* local_maplib = GetMaplib(lib);
    const char* globdefver = (bind==STB_WEAK)?NULL:GetMaplibDefaultVersion(my_context->maplib, (my_context->maplib==local_maplib)?NULL:local_maplib, 0, symname);
    const char* weakdefver = (bind==STB_WEAK)?GetMaplibDefaultVersion(my_context->maplib, (my_context->maplib==local_maplib)?NULL:local_maplib, 1, symname):NULL;
    GetGlobalSymbolStartEnd(my_context->maplib, symname, &offs, &end, h, version, vername, globdefver, weakdefver);
    if(!offs && !end && local_maplib) {
        GetGlobalSymbolStartEnd(local_maplib, symname, &offs, &end, h, version, vername, globdefver, weakdefver);
    }
    if(!offs && !end && !version)
        GetGlobalSymbolStartEnd(my_context->maplib, symname, &offs, &end, h, -1, NULL, globdefver, weakdefver);

    if (!offs) {
        printf_log(LOG_NONE, "Error: PltResolver: Symbol %s(ver %d: %s%s%s) not found, cannot apply R_X86_64_JUMP_SLOT %p (%p) in %s\n", symname, version, symname, vername?"@":"", vername?vername:"", p, *(void**)p, h->name);
        emu->quit = 1;
        return;
    } else {
        elfheader_t* sym_elf = FindElfAddress(my_context, offs);
        if(sym_elf && sym_elf!=my_context->elfs[0] && !sym_elf->init_done) {
            printf_dump(LOG_DEBUG, "symbol %s from %s but elf not initialized yet, run Init now (from %s)\n", symname, ElfName(sym_elf), ElfName(h));
            RunElfInitPltResolver(sym_elf, emu);
        }

        if(p) {
            printf_dump(LOG_DEBUG, "            Apply %s R_X86_64_JUMP_SLOT %p with sym=%s(ver %d: %s%s%s) (%p -> %p / %s)\n", (bind==STB_LOCAL)?"Local":"Global", p, symname, version, symname, vername?"@":"", vername?vername:"",*(void**)p, (void*)offs, ElfName(sym_elf));
            *p = offs;
        } else {
            printf_log(LOG_NONE, "PltResolver: Warning, Symbol %s(ver %d: %s%s%s) found, but Jump Slot Offset is NULL \n", symname, version, symname, vername?"@":"", vername?vername:"");
        }
    }

    // jmp to function
    R_RIP = offs;
}
