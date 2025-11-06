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
#include "bridge.h"
#include "alternate.h"
#include "wrapper.h"
#include "box64context.h"
#include "library.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "box64stack.h"
#include "box64cpu_util.h"
#include "wine_tools.h"
#include "dictionnary.h"
#include "symbols.h"
#ifdef DYNAREC
#include "dynablock.h"
#endif
#include "../emu/x64emu_private.h"
#include "../emu/x64run_private.h"
#include "../tools/bridge_private.h"
#include "x64tls.h"
#include "box32.h"

ptr_t pltResolver32 = ~(ptr_t)0;

extern void* my__IO_2_1_stderr_;
extern void* my__IO_2_1_stdin_ ;
extern void* my__IO_2_1_stdout_;

// return the index of header (-1 if it doesn't exist)
static int getElfIndex(box64context_t* ctx, elfheader_t* head) {
    for (int i=0; i<ctx->elfsize; ++i)
        if(ctx->elfs[i]==head)
            return i;
    return -1;
}

static elfheader_t* checkElfLib(elfheader_t* h, library_t* lib)
{
    if(h && lib) {
        if(!h->needed)
            h->needed = new_neededlib(1);
        add1libref_neededlib(h->needed, lib);
    }
    return h;
}

static Elf32_Sym* ElfLocateSymbol(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    Elf32_Sym* sym = ElfLookup32(head, symname, *ver, *vername, local, *veropt);
    if(!sym) return NULL;
    if(head->VerSym && !*veropt) {
        int idx = ((uintptr_t)sym - (uintptr_t)head->DynSym._32)/sizeof(Elf32_Sym);
        int version = ((Elf32_Half*)((uintptr_t)head->VerSym+head->delta))[idx];
        if(version!=-1) version &=0x7fff;
        const char* symvername = GetSymbolVersion(head, version);
        Elf32_Half flags = GetSymbolVersionFlag(head, version);
        if(version>1 && *ver<2 && (flags==0)) {
            *ver = version;
            *vername = symvername;
            *veropt = 1;
        } else if(flags==0 && !*veropt && version>1 && *ver>1 && !strcmp(symvername, *vername)) {
            *veropt = 1;
        }
    }
    if(!sym->st_shndx) return NULL;
    int vis = ELF32_ST_VISIBILITY(sym->st_other);
    if(vis==STV_HIDDEN && !local)
        return NULL;
    return sym;
}


static void GrabX32CopyMainElfReloc(elfheader_t* head)
{
    if(head->rel) {
        int cnt = head->relsz / head->relent;
        Elf32_Rel* rel = (Elf32_Rel *)(head->rel + head->delta);
        printf_dump(LOG_DEBUG, "Grabbing R_386_COPY Relocation(s) in advance for %s\n", head->name);
        for (int i=0; i<cnt; ++i) {
            int t = ELF32_R_TYPE(rel[i].r_info);
            if(t == R_386_COPY) {
                Elf32_Sym *sym = &head->DynSym._32[ELF32_R_SYM(rel[i].r_info)];
                const char* symname = SymName32(head, sym);
                int version = head->VerSym?((Elf32_Half*)((uintptr_t)head->VerSym+head->delta))[ELF32_R_SYM(rel[i].r_info)]:-1;
                if(version!=-1) version &=0x7fff;
                const char* vername = GetSymbolVersion(head, version);
                Elf32_Half flags = GetSymbolVersionFlag(head, version);
                int veropt = flags?0:1;
                uintptr_t offs = sym->st_value + head->delta;
                AddUniqueSymbol(my_context->globdata, symname, offs, sym->st_size, version, vername, veropt);
            }
        }
    }
}

void checkHookedSymbols(elfheader_t* h);
void AddSymbols32(lib_t *maplib, elfheader_t* h)
{
    // if(BOX64ENV(dump) && h->hash)   old_elf_hash_dump(h);
    // if(BOX64ENV(dump) && h->gnu_hash)   new_elf_hash_dump(h);
    if (BOX64ENV(dump) && h->DynSym._32) DumpDynSym32(h);
    if(h==my_context->elfs[0]) 
        GrabX32CopyMainElfReloc(h);
    #ifndef STATICBUILD
    checkHookedSymbols(h);
    #endif
}

int AllocLoadElfMemory32(box64context_t* context, elfheader_t* head, int mainbin)
{
    ptr_t offs = 0;
    loadProtectionFromMap();
    int log_level = BOX64ENV(load_addr)?LOG_INFO:LOG_DEBUG;

    head->multiblock_n = 0; // count PHEntrie with LOAD
    uintptr_t max_align = (box64_pagesize-1);
    for (size_t i=0; i<head->numPHEntries; ++i) 
        if(head->PHEntries._32[i].p_type == PT_LOAD && head->PHEntries._32[i].p_flags) {
            ++head->multiblock_n;
        }

    if(!head->vaddr && BOX64ENV(load_addr)) {
        offs = to_ptrv(find31bitBlockNearHint((void*)BOX64ENV(load_addr), head->memsz, max_align));
        BOX64ENV(load_addr) = offs + head->memsz;
        BOX64ENV(load_addr) = (BOX64ENV(load_addr)+0x10ffffff)&~0xffffff;
    }
    if(!offs && !head->vaddr)
        offs = (uintptr_t)find31bitBlockElf(head->memsz, mainbin, max_align);
    // prereserve the whole elf image, without populating
    size_t sz = head->memsz;
    void* raw = NULL;
    void* image = NULL;
    if(!head->vaddr) {
        sz += head->align;
        raw = mmap64(from_ptrv(offs), sz, 0, MAP_ANONYMOUS|MAP_PRIVATE|MAP_NORESERVE, -1, 0);
        image = (void*)(((uintptr_t)raw+max_align)&~max_align);
    } else {
        image = raw = mmap64(from_ptrv(head->vaddr), sz, 0, MAP_ANONYMOUS|MAP_PRIVATE|MAP_NORESERVE, -1, 0);
        if(from_ptr(head->vaddr)&(box64_pagesize-1)) {
            // load address might be lower
            if((uintptr_t)image == (from_ptr(head->vaddr)&~(box64_pagesize-1))) {
                image = from_ptrv(head->vaddr);
                sz += ((uintptr_t)image)-((uintptr_t)raw);
            }
        }
    }
    if(image!=MAP_FAILED && !head->vaddr && image!=from_ptrv(offs)) {
        printf_log(LOG_INFO, "%s: Mmap64 for (@%p 0x%zx) for elf \"%s\" returned %p(%p/0x%zx) instead\n", (((uintptr_t)image)&max_align)?"Error":"Warning", from_ptrv(head->vaddr?head->vaddr:offs), head->memsz, head->name, image, raw, head->align);
        offs = to_ptrv(image);
        if(((uintptr_t)image)&max_align) {
            munmap(raw, sz);
            return 1;   // that's an error, alocated memory is not aligned properly
        }
    }
    if(image==MAP_FAILED || image!=from_ptrv(head->vaddr?head->vaddr:offs)) {
        printf_log(LOG_NONE, "%s cannot create memory map (@%p 0x%zx) for elf \"%s\"", (image==MAP_FAILED)?"Error:":"Warning:", from_ptrv(head->vaddr?head->vaddr:offs), head->memsz, head->name);
        if(image==MAP_FAILED) {
            printf_log(LOG_NONE, " error=%d/%s\n", errno, strerror(errno));
        } else {
            printf_log(LOG_NONE, " got %p\n", image);
        }
        if(image==MAP_FAILED)
            return 1;
        offs = to_ptrv(image)-head->vaddr;
    }
    printf_dump(log_level, "Pre-allocated 0x%zx byte at %p for %s\n", head->memsz, image, head->name);
    head->delta = offs;
    printf_dump(log_level, "Delta of %p (vaddr=%p) for Elf \"%s\"\n", from_ptrv(offs), from_ptrv(head->vaddr), head->name);

    head->image = image;
    head->raw = raw;
    head->raw_size = sz;
    setProtection_elf((uintptr_t)raw, sz, 0);

    head->multiblocks = (multiblock_t*)box_calloc(head->multiblock_n, sizeof(multiblock_t));
    head->tlsbase = AddTLSPartition(context, head->tlssize);
    // and now, create all individual blocks
    head->memory = (char*)0xffffffff;
    int n = 0;
    for (size_t i=0; i<head->numPHEntries; ++i) {
        if(head->PHEntries._32[i].p_type == PT_LOAD && head->PHEntries._32[i].p_flags) {
            Elf32_Phdr * e = &head->PHEntries._32[i];

            head->multiblocks[n].flags = e->p_flags;
            head->multiblocks[n].offs = e->p_offset;
            head->multiblocks[n].paddr = e->p_paddr + offs;
            head->multiblocks[n].size = e->p_filesz;
            head->multiblocks[n].align = e->p_align;
            uint8_t prot = PROT_READ|PROT_WRITE|((e->p_flags & PF_X)?PROT_EXEC:0);
            // check if alignment is correct
            uintptr_t balign = head->multiblocks[n].align-1;
            if(balign<4095) balign = 4095;
            head->multiblocks[n].asize = (e->p_memsz+(e->p_paddr&balign)+4095)&~4095;
            int try_mmap = 1;
            if(e->p_paddr&balign)
                try_mmap = 0;
            if(e->p_offset&(box64_pagesize-1))
                try_mmap = 0;
            if(ALIGN(e->p_memsz)!=ALIGN(e->p_filesz))
                try_mmap = 0;
            if(!e->p_filesz)
                try_mmap = 0;
            if(e->p_align<box64_pagesize)
                try_mmap = 0;
            if(try_mmap) {
                printf_dump(log_level, "Mmaping 0x%lx(0x%lx) bytes @%p for Elf \"%s\"\n", head->multiblocks[n].size, head->multiblocks[n].asize, (void*)head->multiblocks[n].paddr, head->name);
                void* p = mmap64(
                    (void*)head->multiblocks[n].paddr, 
                    head->multiblocks[n].size, 
                    prot,
                    MAP_PRIVATE|MAP_FIXED, //((prot&PROT_WRITE)?MAP_SHARED:MAP_PRIVATE)|MAP_FIXED,
                    head->fileno,
                    e->p_offset
                );
                if(p==MAP_FAILED || p!=(void*)head->multiblocks[n].paddr) {
                    try_mmap = 0;
                    printf_dump(log_level, "Mapping failed, using regular mmap+read");
                } else {
                    if(e->p_memsz>e->p_filesz && (prot&PROT_WRITE))
                        memset((void*)((uintptr_t)p + e->p_filesz), 0, e->p_memsz-e->p_filesz);
                    setProtection_elf((uintptr_t)p, head->multiblocks[n].asize, prot);
                    head->multiblocks[n].p = p;

                }
            }
            if(!try_mmap) {
                uintptr_t paddr = head->multiblocks[n].paddr&~balign;
                size_t asize = head->multiblocks[n].asize;
                void* p = MAP_FAILED;
                if(paddr==(paddr&~(box64_pagesize-1)) && (asize==ALIGN(asize))) {
                    printf_dump(log_level, "Allocating 0x%zx (0x%zx) bytes @%p, will read 0x%zx @%p for Elf \"%s\"\n", asize, e->p_memsz, (void*)paddr, e->p_filesz, (void*)head->multiblocks[n].paddr, head->name);
                    p = mmap64(
                        (void*)paddr,
                        asize,
                        prot|PROT_WRITE,
                        MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,
                        -1,
                        0
                    );
                } else {
                    // difference in pagesize, so need to mmap only what needed to be...
                    //check startint point
                    uintptr_t new_addr = paddr&~(box64_pagesize-1); // new_addr might be smaller than paddr
                    ssize_t new_size = asize + (paddr-new_addr);    // so need new_size to compensate
                    while(getProtection(new_addr) && (new_size>0)) {// but then, there might be some overlap
                        uintptr_t diff = ALIGN(new_addr+1) - new_addr; // next page
                        if(diff<(size_t)new_size)
                            new_size -= diff;
                        else
                            new_size = 0;
                        new_addr = ALIGN(new_addr+1);
                    }
                    if(new_size>0) {
                        printf_dump(log_level, "Allocating 0x%zx (0x%zx/0x%zx) bytes @%p, will read 0x%zx @%p for Elf \"%s\"\n", ALIGN(new_size), paddr, e->p_memsz, (void*)new_addr, e->p_filesz, (void*)head->multiblocks[n].paddr, head->name);
                        p = mmap64(
                            (void*)new_addr,
                            ALIGN(new_size),
                            prot|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,
                            -1,
                            0
                        );
                        if(p==(void*)new_addr)
                            p = (void*)paddr;
                    } else {
                        p = (void*)paddr;
                        printf_dump(log_level, "Will read 0x%zx @%p for Elf \"%s\"\n", e->p_filesz, (void*)head->multiblocks[n].paddr, head->name);    
                    }
                }
                if(p==MAP_FAILED || p!=(void*)paddr) {
                    printf_log(LOG_NONE, "Cannot create memory map (@%p 0x%zx/0x%zx) for elf \"%s\"", (void*)paddr, asize, balign, head->name);
                    if(p==MAP_FAILED) {
                        printf_log(LOG_NONE, " error=%d/%s\n", errno, strerror(errno));
                    } else {
                        printf_log(LOG_NONE, " got %p\n", p);
                    }
                    return 1;
                }
                setProtection_elf((uintptr_t)p, asize, prot);
                head->multiblocks[n].p = p;
                if(e->p_filesz) {
                    fseeko64(head->file, head->multiblocks[n].offs, SEEK_SET);
                    if(fread((void*)head->multiblocks[n].paddr, head->multiblocks[n].size, 1, head->file)!=1) {
                        printf_log(LOG_NONE, "Cannot read elf block (@%p 0x%zx/0x%zx) for elf \"%s\"\n", (void*)head->multiblocks[n].offs, head->multiblocks[n].asize, balign, head->name);
                        return 1;
                    }
                }
                if(!(prot&PROT_WRITE) && (paddr==(paddr&(box64_pagesize-1)) && (asize==ALIGN(asize))))
                    mprotect((void*)paddr, asize, prot);
            }
#ifdef DYNAREC
            if(BOX64ENV(dynarec) && (e->p_flags & PF_X)) {
                dynarec_log(LOG_DEBUG, "Add ELF eXecutable Memory %p:%p\n", head->multiblocks[n].p, (void*)head->multiblocks[n].asize);
                addDBFromAddressRange((uintptr_t)head->multiblocks[n].p, head->multiblocks[n].asize);
            }
#endif
            if((uintptr_t)head->memory>(uintptr_t)head->multiblocks[n].p)
                head->memory = (char*)head->multiblocks[n].p;
            ++n;
        }
        if(head->PHEntries._32[i].p_type == PT_TLS) {
            Elf32_Phdr * e = &head->PHEntries._32[i];
            char* dest = (char*)(context->tlsdata+context->tlssize+head->tlsbase);
            printf_log(LOG_DEBUG, "Loading TLS block #%zu @%p (0x%zx/0x%zx)\n", i, dest, e->p_filesz, e->p_memsz);
            if(e->p_filesz) {
                fseeko64(head->file, e->p_offset, SEEK_SET);
                if(fread(dest, e->p_filesz, 1, head->file)!=1) {
                    printf_log(LOG_NONE, "Fail to read PT_TLS part #%zu (size=%zd)\n", i, e->p_filesz);
                    return 1;
                }
            }
            // zero'd difference between filesz and memsz
            if(e->p_filesz != e->p_memsz)
                memset(dest+e->p_filesz, 0, e->p_memsz - e->p_filesz);
        }
    }
    // record map
    RecordEnvMappings((uintptr_t)head->image, head->memsz, head->fileno);
    // can close the elf file now!
    fclose(head->file);
    head->file = NULL;
    head->fileno = -1;

    return 0;
}

static int IsSymInElfSpace(const elfheader_t* h, Elf32_Sym* sym)
{
    if(!h || !sym)
        return 0;
    uintptr_t addr = (uintptr_t)sym;
    if(h->SymTab._32 && addr>=(uintptr_t)h->SymTab._32 && addr<(uintptr_t)&h->SymTab._32[h->numSymTab])
        return 1;
    if(h->DynSym._32 && addr>=(uintptr_t)h->DynSym._32 && addr<(uintptr_t)&h->DynSym._32[h->numDynSym])
        return 1;
    return 0;
}
static elfheader_t* FindElfSymbol(box64context_t *context, Elf32_Sym* sym)
{
    if(!sym)
        return NULL;
    for (int i=0; i<context->elfsize; ++i)
        if(IsSymInElfSpace(context->elfs[i], sym))
            return context->elfs[i];
    
    return NULL;
}

void GrabR386CopyMainElfReloc(elfheader_t* head)
{
    if(head->rel) {
        int cnt = head->relsz / head->relent;
        Elf32_Rel* rel = (Elf32_Rel *)(head->rel + head->delta);
        printf_dump(LOG_DEBUG, "Grabbing R_386_COPY Relocation(s) in advance for %s\n", head->name);
        for (int i=0; i<cnt; ++i) {
            int t = ELF32_R_TYPE(rel[i].r_info);
            if(t == R_386_COPY) {
                Elf32_Sym *sym = &head->DynSym._32[ELF32_R_SYM(rel[i].r_info)];
                const char* symname = SymName32(head, sym);
                int version = head->VerSym?((Elf32_Half*)((uintptr_t)head->VerSym+head->delta))[ELF32_R_SYM(rel[i].r_info)]:-1;
                if(version!=-1) version &=0x7fff;
                const char* vername = GetSymbolVersion(head, version);
                Elf32_Half flags = GetSymbolVersionFlag(head, version);
                int veropt = flags?0:1;
                uintptr_t offs = sym->st_value + head->delta;
                AddUniqueSymbol(my_context->globdata, symname, offs, sym->st_size, version, vername, veropt);
            }
        }
    }
}


static int FindR386COPYRel(elfheader_t* h, const char* name, ptr_t *offs, uint32_t** p, size_t size, int version, const char* vername, int veropt)
{
    if(!h)
        return 0;
    if(!h->rel)
        return 0;
    if(h->relent) {
        Elf32_Rel * rel = (Elf32_Rel *)(h->rel + h->delta);
        int cnt = h->relsz / h->relent;
        for (int i=0; i<cnt; ++i) {
            int t = ELF32_R_TYPE(rel[i].r_info);
            Elf32_Sym *sym = &h->DynSym._32[ELF32_R_SYM(rel[i].r_info)];
            const char* symname = SymName32(h, sym);
            if((t==R_386_COPY) && symname && !strcmp(symname, name) && (sym->st_size==size)) {
                int version2 = h->VerSym?((Elf32_Half*)((uintptr_t)h->VerSym+h->delta))[ELF32_R_SYM(rel[i].r_info)]:-1;
                if(version2!=-1) version2 &= 0x7fff;
                if(version && !version2) version2=-1;   // match a versioned symbol against a global "local" symbol
                const char* vername2 = GetSymbolVersion(h, version2);
                Elf32_Half flags = GetSymbolVersionFlag(h, version2);
                int veropt2 = flags?0:1;
                if(SameVersionedSymbol(name, version, vername, veropt, symname, version2, vername2, veropt2)) {
                    if(offs) *offs = sym->st_value + h->delta;
                    if(p) *p = (uint32_t*)(rel[i].r_offset + h->delta);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int RelocateElfREL(lib_t *maplib, lib_t *local_maplib, int bindnow, int deepbind, elfheader_t* head, int cnt, Elf32_Rel *rel, int* need_resolv)
{
    int ret_ok = 0;
    for (int i=0; i<cnt; ++i) {
        int t = ELF32_R_TYPE(rel[i].r_info);
        Elf32_Sym *sym = &head->DynSym._32[ELF32_R_SYM(rel[i].r_info)];
        int bind = ELF32_ST_BIND(sym->st_info);
        //uint32_t ndx = sym->st_shndx;
        const char* symname = SymName32(head, sym);
        uint32_t *p = (uint32_t*)(rel[i].r_offset + head->delta);
        uintptr_t offs = 0;
        uintptr_t end = 0;
        size_t size = sym->st_size;
        elfheader_t* sym_elf = NULL;
        elfheader_t* last_elf = NULL;
        int version = head->VerSym?((Elf32_Half*)((uintptr_t)head->VerSym+head->delta))[ELF32_R_SYM(rel[i].r_info)]:-1;
        if(version!=-1) version &=0x7fff;
        const char* vername = GetSymbolVersion(head, version);
        Elf32_Half flags = GetSymbolVersionFlag(head, version);
        int veropt = flags?0:1;
        Elf32_Sym* elfsym = NULL;
        int vis = ELF64_ST_VISIBILITY(sym->st_other);
        if(vis==STV_PROTECTED) {
            elfsym = ElfDynSymLookup32(head, symname);
            if(elfsym) {
                offs = elfsym->st_value + head->delta;
                end = offs + elfsym->st_size;
            }
            printf_log(LOG_DEBUG, "Symbol %s from %s is PROTECTED\n", symname, head->name);
        } else {
            if(bind==STB_GNU_UNIQUE) {
                GetGlobalSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, veropt, (void**)&elfsym);
                if(!offs && !end && local_maplib)
                    GetGlobalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, veropt, (void**)&elfsym);
            } else if(bind==STB_LOCAL) {
                if(!symname || !symname[0]) {
                    offs = sym->st_value + head->delta;
                    end = offs + sym->st_size;
                } else {
                    elfsym = ElfDynSymLookup32(head, symname);
                    if(elfsym && elfsym->st_shndx) {
                        offs = elfsym->st_value + head->delta;
                        end = offs + elfsym->st_size;
                    }
                    if(!offs && !end && local_maplib && deepbind)
                        GetLocalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, veropt, (void**)&elfsym);
                    if(!offs && !end)
                        GetLocalSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, veropt, (void**)&elfsym);
                    if(!offs && !end && local_maplib && !deepbind)
                        GetLocalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, veropt, (void**)&elfsym);
                }
            } else {
                if(!offs && !end && local_maplib && deepbind)
                    GetGlobalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, veropt, (void**)&elfsym);
                if(!offs && !end)
                    GetGlobalSymbolStartEnd(maplib, symname, &offs, &end, head, version, vername, veropt, (void**)&elfsym);
                if(!offs && !end && local_maplib && !deepbind)
                    GetGlobalSymbolStartEnd(local_maplib, symname, &offs, &end, head, version, vername, veropt, (void**)&elfsym);
            }
        }
        sym_elf = FindElfSymbol(my_context, elfsym);
        if(elfsym && (ELF32_ST_TYPE(elfsym->st_info)==STT_TLS))
            offs = elfsym->st_value;
        uintptr_t globoffs, globend;
        uint32_t* globp;
        uintptr_t tmp = 0;
        intptr_t delta;
        switch(t) {
            case R_386_NONE:
                // can be ignored
                printf_dump(LOG_NEVER, "Ignoring [%d] %s %p (%p)\n", i, DumpRelType32(t), p, from_ptrv(p?(*p):0));
                break;
            case R_386_PC32:
                    if (!offs) {
                        printf_log(LOG_NONE, "Error: Global Symbol %s not found, cannot apply R_386_PC32 %p (%p) in %s\n", symname, p, from_ptrv(*p), head->name);
                        ret_ok = 1;
                    }
                    if(offs)
                        printf_dump(LOG_NEVER, "Apply [%d] %s R_386_PC32 %p with sym=%s (ver=%d/%s), (%p -> %p/%p)\n", i, (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, version, vername?vername:"(none)", from_ptrv(*(ptr_t*)p), from_ptrv(*(ptr_t*)p+(offs-to_ptrv(p))), from_ptrv(offs));
                    offs = (offs - to_ptrv(p));
                    *p += offs;
                break;
            case R_386_RELATIVE:
                printf_dump(LOG_NEVER, "Apply [%d] %s R_386_RELATIVE %p (%p -> %p)\n", i, (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, from_ptrv(*p), (void*)((*p)+head->delta));
                *p += head->delta;
                break;
            case R_386_COPY:
                globoffs = offs;
                globend = end;
                offs = end = 0;
                if(!offs && local_maplib && deepbind)
                    GetNoSelfSymbolStartEnd(local_maplib, symname, &offs, &end, head, size, version, vername, veropt, NULL);
                if(!offs)
                    GetNoSelfSymbolStartEnd(maplib, symname, &offs, &end, head, size, version, vername, veropt, NULL);
                if(!offs && local_maplib && !deepbind)
                    GetNoSelfSymbolStartEnd(local_maplib, symname, &offs, &end, head, size, version, vername, veropt, NULL);
                if(!offs) {offs = globoffs; end = globend;}
                if(offs) {
                    // add r_addend to p?
                    printf_dump(LOG_NEVER, "Apply R_386_COPY @%p with sym=%s (%sver=%d/%s), @%p size=%d\n", p, symname, veropt?"opt":"", version, vername?vername:"(none)", from_ptrv(offs), sym->st_size);
                    if(p!=from_ptrv(offs))
                        memmove(p, from_ptrv(offs), sym->st_size);
                    sym_elf = FindElfAddress(my_context, offs);
                    if(sym_elf && sym_elf!=last_elf && sym_elf!=head) last_elf = checkElfLib(head, sym_elf->lib);
                } else {
                    printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply REL R_386_COPY @%p (%p) in %s\n", symname, p, from_ptrv(*p), head->name);
                }
                break;
            case R_386_GLOB_DAT:
                if(GetSymbolStartEnd(my_context->globdata, symname, &globoffs, &globend, version, vername, 1, veropt)) {
                    globp = (uint32_t*)globoffs;
                    printf_dump(LOG_NEVER, "Apply %s R_386_GLOB_DAT with R_386_COPY @%p/%p (%p/%p -> %p/%p) size=%d on sym=%s (%sver=%d/%s) \n", 
                        BindSymFriendly(bind), p, globp, from_ptrv(p?(*p):0), 
                        from_ptrv(globp?(*globp):0), (void*)offs, (void*)globoffs, sym->st_size, symname, veropt?"opt":"", version, vername?vername:"(none)");
                    sym_elf = my_context->elfs[0];
                    *p = globoffs;
                } else {
                    if (!offs) {
                        if(strcmp(symname, "__gmon_start__") && strcmp(symname, "data_start") && strcmp(symname, "__data_start") && strcmp(symname, "collector_func_load"))
                            printf_log((bind==STB_WEAK)?LOG_DEBUG:LOG_NONE, "%s: Global Symbol %s not found, cannot apply R_386_GLOB_DAT @%p (%p) in %s\n", (bind==STB_WEAK)?"Warning":"Error", symname, p, *(void**)p, head->name);
                    } else {
                        printf_dump(LOG_NEVER, "Apply %s R_386_GLOB_DAT @%p (%p -> %p) on sym=%s (%sver=%d/%s, elf=%s)\n", BindSymFriendly(bind), p, from_ptrv(p?(*p):0), from_ptrv(offs), symname, veropt?"opt":"", version, vername?vername:"(none)", sym_elf?sym_elf->name:"(native)");
                        *p = offs;
                        if(sym_elf && sym_elf!=last_elf && sym_elf!=head) last_elf = checkElfLib(head, sym_elf->lib);
                    }
                }
                break;
            case R_386_JMP_SLOT:
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
                            printf_log(LOG_INFO, "Warning: Weak Symbol %s not found, cannot apply R_386_JMP_SLOT %p (%p)\n", symname, p, from_ptrv(*p));
                        } else {
                            printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_386_JMP_SLOT %p (%p) in %s\n", symname, p, from_ptrv(*p), head->name);
                            ret_ok = 1;
                        }
                    } else {
                        if(p) {
                            printf_dump(LOG_NEVER, "Apply %s R_386_JMP_SLOT %p with sym=%s(%s%s%s) (%p -> %p)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, symname, vername?"@":"", vername?vername:"", from_ptrv(*p), from_ptrv(offs));
                            *p = offs;
                        } else {
                            printf_log(LOG_NONE, "Warning, Symbol %s found, but Jump Slot Offset is NULL \n", symname);
                        }
                    }
                } else {
                    printf_dump(LOG_NEVER, "Preparing (if needed) %s R_386_JMP_SLOT %p (0x%x->0x%0x) with sym=%s(%s%s%s/version %d) to be apply later\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, *p, *p+head->delta, symname, symname, vername?"@":"", vername?vername:"", version);
                    *p += head->delta;
                    *need_resolv = 1;
                }
                break;
            case R_386_32:
                if(GetSymbolStartEnd(my_context->globdata, symname, &globoffs, &globend, version, vername, 1, veropt)) {
                    if(offs!=globoffs) {
                        offs = globoffs;
                        sym_elf = my_context->elfs[0];
                        elfsym = ElfDynSymLookup32(sym_elf, symname);
                    }
                }
                if (!offs) {
                        if(strcmp(symname, "__gmon_start__") && strcmp(symname, "data_start") && strcmp(symname, "__data_start")) {
                            printf_log(LOG_NONE, "%s: Symbol sym=%s(%s%s%s/version %d) not found, cannot apply R_386_32 %p (%p) in %s\n", (bind==STB_GLOBAL)?"Error":"Warning", symname, symname, vername?"@":"", vername?vername:"", version, p, from_ptrv(*p), head->name);
                            if(bind==STB_GLOBAL)
                                ret_ok = 1;
                        }
                } else {
                    printf_dump(LOG_NEVER, "Apply %s R_386_32 %p with sym=%s (ver=%d/%s) (%p -> %p)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, version, vername?vername:"(none)", from_ptrv(*p), from_ptrv(offs+*p));
                    *p += offs;
                }
                break;
            case R_386_TLS_TPOFF:
                // Negated offset in static TLS block
                {
                    if(!symname || !symname[0]) {
                        sym_elf = head;
                        offs = sym->st_value;
                    }
                    if(sym_elf) {
                        delta = *(int32_t*)p;
                        printf_dump(LOG_NEVER, "Applying %s %s on %s @%p (%d -> %d+%d, size=%d)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), DumpRelType32(t), symname, p, delta, sym_elf->tlsbase, (int32_t)offs, end-offs);
                        *p = (uintptr_t)((int32_t)offs + sym_elf->tlsbase);
                    } else {
                        printf_log(LOG_INFO, "Warning, cannot apply %s %s on %s @%p (%d), no elf_header found\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), DumpRelType32(t), symname, p, (int32_t)offs);
                    }
                }
                break;
            case R_386_TLS_TPOFF32:
                // Non-negated offset in static TLS block???
                {
                    if(!symname || !symname[0]) {
                        sym_elf = head;
                        offs = sym->st_value;
                    }
                    if(sym_elf) {
                        delta = *(int32_t*)p;
                        printf_dump(LOG_NEVER, "Applying %s %s on %s @%p (%d -> %d+%d, size=%d)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), DumpRelType32(t), symname, p, delta, -sym_elf->tlsbase, (int32_t)offs, end-offs);
                        *p = (uintptr_t)((int32_t)offs - sym_elf->tlsbase);
                    } else {
                        printf_log(LOG_INFO, "Warning, cannot apply %s %s on %s @%p (%d), no elf_header found\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), DumpRelType32(t), symname, p, (int32_t)offs);
                    }
                }
                break;
            case R_386_TLS_DTPMOD32:
                // ID of module containing symbol
                if(!symname || symname[0]=='\0' || bind==STB_LOCAL) {
                    offs = getElfIndex(my_context, head);
                    sym_elf = head;
                } else {
                    offs = getElfIndex(my_context, sym_elf);
                }
                if(p) {
                    printf_dump(LOG_NEVER, "Apply %s %s %p with sym=%s (%p -> %p)\n", "R_386_TLS_DTPMOD32", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, from_ptrv(*p), from_ptrv(offs));
                    *p = offs;
                } else {
                    printf_log(LOG_NONE, "Warning, Symbol %s or Elf not found, but R_386_TLS_DTPMOD32 Slot Offset is NULL \n", symname);
                }
                break;
            case R_386_TLS_DTPOFF32:
                // Offset in TLS block
                if (!offs && !end) {
                    if(bind==STB_WEAK) {
                        printf_log(LOG_INFO, "Warning: Weak Symbol %s not found, cannot apply R_386_TLS_DTPOFF32 %p (%p)\n", symname, p, from_ptrv(*p));
                    } else {
                        printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_386_TLS_DTPOFF32 %p (%p) in %s\n", symname, p, from_ptrv(*p), head->name);
                    }
                } else {
                    if(!symname || !symname[0]) {
                        offs = (uintptr_t)((intptr_t)(head->tlsaddr + head->delta) - (intptr_t)offs);    // negative offset
                    }
                    if(p) {
                        printf_dump(LOG_NEVER, "Apply %s R_386_TLS_DTPOFF32 %p with sym=%s (ver=%d/%s) (%zd -> %zd)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, version, vername?vername:"(none)", (intptr_t)*p, (intptr_t)offs);
                        *p = offs;
                    } else {
                        printf_log(LOG_NONE, "Warning, Symbol %s found, but R_386_TLS_DTPOFF32 Slot Offset is NULL \n", symname);
                    }
                }
                break;
            default:
                printf_log(LOG_INFO, "Warning, don't know of to handle rel #%d %s (%p) for %s\n", i, DumpRelType32(ELF32_R_TYPE(rel[i].r_info)), p, symname?symname:"(nil)");
        }
    }
    return bindnow?ret_ok:0;
}

static int RelocateElfRELA(lib_t *maplib, lib_t *local_maplib, int bindnow, int deepbind, elfheader_t* head, int cnt, Elf32_Rela *rela, int* need_resolv)
{
    printf_log(LOG_NONE, "Error: RELA type of Relocation unsupported (only REL)\n");
    return 1;
}

static int RelocateElfRELR(elfheader_t *head, int cnt, Elf32_Relr *relr)
{
    printf_log(LOG_NONE, "Error: RELR type of Relocation unsupported (only REL)\n");
    return 1;
}

int RelocateElf32(lib_t *maplib, lib_t *local_maplib, int bindnow, int deepbind, elfheader_t* head)
{
    if(0 && (head->flags&DF_BIND_NOW) && !bindnow) { // disable for now, needs more symbol in a fow libs like gtk and nss3
        bindnow = 1;
        printf_log(LOG_DEBUG, "Forcing %s to Bind Now\n", head->name);
    }
    if(head->relr) {
        int cnt = head->relrsz / head->relrent;
        DumpRelRTable32(head, cnt, (Elf32_Relr *)(head->relr + head->delta), "RelR");
        printf_log(LOG_DEBUG, "Applying %d Relocation(s) with Addend for %s\n", cnt, head->name);
        if(RelocateElfRELR(head, cnt, (Elf32_Relr *)(head->relr + head->delta)))
            return -1;
    }
    if(head->rel) {
        int cnt = head->relsz / head->relent;
        DumpRelTable32(head, cnt, (Elf32_Rel *)(head->rel + head->delta), "Rel");
        printf_log(LOG_DEBUG, "Applying %d Relocation(s) for %s\n", cnt, head->name);
        if(RelocateElfREL(maplib, local_maplib, bindnow, deepbind, head, cnt, (Elf32_Rel *)(head->rel + head->delta), NULL))
            return -1;
    }
    if(head->rela) {
        int cnt = head->relasz / head->relaent;
        DumpRelATable32(head, cnt, (Elf32_Rela *)(head->rela + head->delta), "RelA");
        printf_log(LOG_DEBUG, "Applying %d Relocation(s) with Addend for %s\n", cnt, head->name);
        if(RelocateElfRELA(maplib, local_maplib, bindnow, deepbind, head, cnt, (Elf32_Rela *)(head->rela + head->delta), NULL))
            return -1;
    }
    return 0;
}

int RelocateElfPlt32(lib_t *maplib, lib_t *local_maplib, int bindnow, int deepbind, elfheader_t* head)
{
    int need_resolver = 0;
    if(0 && (head->flags&DF_BIND_NOW) && !bindnow) { // disable for now, needs more symbol in a fow libs like gtk and nss3
        bindnow = 1;
        printf_log(LOG_DEBUG, "Forcing %s to Bind Now\n", head->name);
    }
    if(head->pltrel) {
        int cnt = head->pltsz / head->pltent;
        if(head->pltrel==DT_REL) {
            DumpRelTable32(head, cnt, (Elf32_Rel *)(head->jmprel + head->delta), "PLT");
            printf_log(LOG_DEBUG, "Applying %d PLT Relocation(s) for %s\n", cnt, head->name);
            if(RelocateElfREL(maplib, local_maplib, bindnow, deepbind, head, cnt, (Elf32_Rel *)(head->jmprel + head->delta), &need_resolver))
                return -1;
        } else if(head->pltrel==DT_RELA) {
            DumpRelATable32(head, cnt, (Elf32_Rela *)(head->jmprel + head->delta), "PLT");
            printf_log(LOG_DEBUG, "Applying %d PLT Relocation(s) with Addend for %s\n", cnt, head->name);
            if(RelocateElfRELA(maplib, local_maplib, bindnow, deepbind, head, cnt, (Elf32_Rela *)(head->jmprel + head->delta), &need_resolver))
                return -1;
        }
        if(need_resolver) {
            if(pltResolver32==~(ptr_t)0) {
                pltResolver32 = AddBridge(my_context->system, vFEv, PltResolver32, 0, "(PltResolver)");
            }
            if(head->pltgot) {
                *(ptr_t*)from_ptrv(head->pltgot+head->delta+8) = pltResolver32;
                *(ptr_t*)from_ptrv(head->pltgot+head->delta+4) = to_ptrv(head);
                printf_log(LOG_DEBUG, "PLT Resolver injected in plt.got at %p\n", from_ptrv(head->pltgot+head->delta+8));
            } else if(head->got) {
                *(ptr_t*)from_ptrv(head->got+head->delta+8) = pltResolver32;
                *(ptr_t*)from_ptrv(head->got+head->delta+4) = to_ptrv(head);
                printf_log(LOG_DEBUG, "PLT Resolver injected in got at %p\n", from_ptrv(head->got+head->delta+8));
            }
        }
    }
    return 0;
}

void ResetSpecialCaseMainElf32(elfheader_t* h)
{
    Elf32_Sym *sym = NULL;
     for (uint32_t i=0; i<h->numDynSym; ++i) {
        if(h->DynSym._32[i].st_info == 17) {
            sym = h->DynSym._32+i;
            const char * symname = h->DynStr+sym->st_name;
            if(strcmp(symname, "_IO_2_1_stderr_")==0 && (from_ptrv(sym->st_value+h->delta))) {
                memcpy(from_ptrv(sym->st_value+h->delta), stderr, sym->st_size);
                my__IO_2_1_stderr_ = from_ptrv(sym->st_value+h->delta);
                printf_log(LOG_DEBUG, "BOX32: Set @_IO_2_1_stderr_ to %p\n", my__IO_2_1_stderr_);
            } else
            if(strcmp(symname, "_IO_2_1_stdin_")==0 && (from_ptrv(sym->st_value+h->delta))) {
                memcpy(from_ptrv(sym->st_value+h->delta), stdin, sym->st_size);
                my__IO_2_1_stdin_ = from_ptrv(sym->st_value+h->delta);
                printf_log(LOG_DEBUG, "BOX32: Set @_IO_2_1_stdin_ to %p\n", my__IO_2_1_stdin_);
            } else
            if(strcmp(symname, "_IO_2_1_stdout_")==0 && (from_ptrv(sym->st_value+h->delta))) {
                memcpy(from_ptrv(sym->st_value+h->delta), stdout, sym->st_size);
                my__IO_2_1_stdout_ = from_ptrv(sym->st_value+h->delta);
                printf_log(LOG_DEBUG, "BOX32: Set @_IO_2_1_stdout_ to %p\n", my__IO_2_1_stdout_);
            } else
            if(strcmp(symname, "_IO_stderr_")==0 && (from_ptrv(sym->st_value+h->delta))) {
                memcpy(from_ptrv(sym->st_value+h->delta), stderr, sym->st_size);
                my__IO_2_1_stderr_ = from_ptrv(sym->st_value+h->delta);
                printf_log(LOG_DEBUG, "BOX32: Set @_IO_stderr_ to %p\n", my__IO_2_1_stderr_);
            } else
            if(strcmp(symname, "_IO_stdin_")==0 && (from_ptrv(sym->st_value+h->delta))) {
                memcpy(from_ptrv(sym->st_value+h->delta), stdin, sym->st_size);
                my__IO_2_1_stdin_ = from_ptrv(sym->st_value+h->delta);
                printf_log(LOG_DEBUG, "BOX32: Set @_IO_stdin_ to %p\n", my__IO_2_1_stdin_);
            } else
            if(strcmp(symname, "_IO_stdout_")==0 && (from_ptrv(sym->st_value+h->delta))) {
                memcpy(from_ptrv(sym->st_value+h->delta), stdout, sym->st_size);
                my__IO_2_1_stdout_ = from_ptrv(sym->st_value+h->delta);
                printf_log(LOG_DEBUG, "BOX32: Set @_IO_stdout_ to %p\n", my__IO_2_1_stdout_);
            }
        }
    }
}

void* ElfGetLocalSymbolStartEnd32(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    Elf32_Sym* sym = ElfLocateSymbol(head, offs, end, symname, ver, vername, local, veropt);
    if(!sym) return NULL;
    int bind = ELF32_ST_BIND(sym->st_info);
    if(bind!=STB_LOCAL) return 0;
    if(offs) *offs = sym->st_value + head->delta;
    if(end) *end = sym->st_value + head->delta + sym->st_size;
    return sym;
}

void* ElfGetGlobalSymbolStartEnd32(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    Elf32_Sym* sym = ElfLocateSymbol(head, offs, end, symname, ver, vername, local, veropt);
    if(!sym) return NULL;
    int bind = ELF32_ST_BIND(sym->st_info);
    if(bind!=STB_GLOBAL && bind!=STB_GNU_UNIQUE) return 0;
    if(offs) *offs = sym->st_value + head->delta;
    if(end) *end = sym->st_value + head->delta + sym->st_size;
    return sym;
}

void* ElfGetWeakSymbolStartEnd32(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    Elf32_Sym* sym = ElfLocateSymbol(head, offs, end, symname, ver, vername, local, veropt);
    if(!sym) return NULL;
    int bind = ELF32_ST_BIND(sym->st_info);
    if(bind!=STB_WEAK) return 0;
    if(offs) *offs = sym->st_value + head->delta;
    if(end) *end = sym->st_value + head->delta + sym->st_size;
    return sym;
}

void* ElfGetSymbolStartEnd32(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    Elf32_Sym* sym = ElfLocateSymbol(head, offs, end, symname, ver, vername, local, veropt);
    if(!sym) return NULL;
    if(offs) *offs = sym->st_value + head->delta;
    if(end) *end = sym->st_value + head->delta + sym->st_size;
    return sym;
}

int ElfGetSymTabStartEnd32(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname)
{
    Elf32_Sym* sym = ElfSymTabLookup32(head, symname);
    if(!sym) return 0;
    if(!sym->st_shndx) return 0;
    if(!sym->st_size) return 0; //needed?
    if(offs) *offs = sym->st_value + head->delta;
    if(end) *end = sym->st_value + head->delta + sym->st_size;
    return 1;
}

EXPORT void PltResolver32(x64emu_t* emu)
{
    ptr_t addr = Pop32(emu);
    int slot = (int)Pop32(emu);
    elfheader_t *h = (elfheader_t*)from_ptrv(addr);
    library_t* lib = h->lib;
    lib_t* local_maplib = GetMaplib(lib);
    int deepbind = GetDeepBind(lib);
    printf_dump(LOG_DEBUG, "PltResolver32: Addr=%p, Slot=%d Return=%p: elf is %s (VerSym=%p)\n", from_ptrv(addr), slot, *(ptr_t*)from_ptrv(R_ESP), h->name, h->VerSym);

    Elf32_Rel * rel = (Elf32_Rel *)(from_ptrv(h->jmprel + h->delta + slot));

    Elf32_Sym *sym = &h->DynSym._32[ELF32_R_SYM(rel->r_info)];
    int bind = ELF32_ST_BIND(sym->st_info);
    const char* symname = SymName32(h, sym);
    int version = h->VerSym?((Elf32_Half*)((uintptr_t)h->VerSym+h->delta))[ELF32_R_SYM(rel->r_info)]:-1;
    if(version!=-1) version &= 0x7fff;
    const char* vername = GetSymbolVersion(h, version);
    Elf32_Half flags = GetSymbolVersionFlag(h, version);
    int veropt = flags?0:1;
    ptr_t *p = (uint32_t*)from_ptrv(rel->r_offset + h->delta);
    uintptr_t offs = 0;
    uintptr_t end = 0;

    Elf32_Sym *elfsym = NULL;
    if(bind==STB_LOCAL) {
        elfsym = ElfDynSymLookup32(h, symname);
        if(elfsym && elfsym->st_shndx) {
            offs = elfsym->st_value + h->delta;
            end = offs + elfsym->st_size;
        }
        if(!offs && !end && local_maplib && deepbind)
            GetLocalSymbolStartEnd(local_maplib, symname, &offs, &end, h, version, vername, veropt, (void**)&elfsym);
        if(!offs && !end)
            GetLocalSymbolStartEnd(my_context->maplib, symname, &offs, &end, h, version, vername, veropt, (void**)&elfsym);
        if(!offs && !end && local_maplib && !deepbind)
            GetLocalSymbolStartEnd(local_maplib, symname, &offs, &end, h, version, vername, veropt, (void**)&elfsym);
    } else if(bind==STB_WEAK) {
        if(local_maplib && deepbind)
            GetGlobalWeakSymbolStartEnd(local_maplib, symname, &offs, &end, h, version, vername, veropt, (void**)&elfsym);
        else
            GetGlobalWeakSymbolStartEnd(my_context->maplib, symname, &offs, &end, h, version, vername, veropt, (void**)&elfsym);
        if(!offs && !end && local_maplib && !deepbind)
            GetGlobalWeakSymbolStartEnd(local_maplib, symname, &offs, &end, h, version, vername, veropt, (void**)&elfsym);
    } else {
        if(!offs && !end && local_maplib && deepbind)
            GetGlobalSymbolStartEnd(local_maplib, symname, &offs, &end, h, version, vername, veropt, (void**)&elfsym);
        if(!offs && !end)
            GetGlobalSymbolStartEnd(my_context->maplib, symname, &offs, &end, h, version, vername, veropt, (void**)&elfsym);
        if(!offs && !end && local_maplib && !deepbind)
            GetGlobalSymbolStartEnd(local_maplib, symname, &offs, &end, h, version, vername, veropt, (void**)&elfsym);
    }

    if (!offs) {
        printf_log(LOG_NONE, "Error: PltResolver32: Symbol %s(ver %d: %s%s%s) not found, cannot apply R_386_JMP_SLOT %p (%p) in %s\n", symname, version, symname, vername?"@":"", vername?vername:"", p, from_ptrv(*p), h->name);
        emu->quit = 1;
        return;
    } else {
        elfheader_t* sym_elf = FindElfSymbol(my_context, elfsym);
        if(elfsym && (elfsym->st_info&0xf)==STT_GNU_IFUNC) {
            // this is an IFUNC, needs to evaluate the function first!
            printf_dump(LOG_DEBUG, "            Indirect function, will call the resolver now at %p\n", from_ptrv(offs));
            offs = (ptr_t)RunFunction(offs, 0);
        }
        offs = (uintptr_t)getAlternate(from_ptrv(offs));

        if(p) {
            printf_dump(LOG_DEBUG, "            Apply %s R_386_JMP_SLOT %p with sym=%s(ver %d: %s%s%s) (%p -> %p / %s)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":"Global"), p, symname, version, symname, vername?"@":"", vername?vername:"",from_ptrv(*p), from_ptrv(offs), ElfName(FindElfAddress(my_context, offs)));
            *p = offs;
        } else {
            printf_log(LOG_NONE, "PltResolver32: Warning, Symbol %s(ver %d: %s%s%s) found, but Jump Slot Offset is NULL \n", symname, version, symname, vername?"@":"", vername?vername:"");
        }
    }

    // jmp to function
    R_EIP = offs;
}


typedef struct my_dl_phdr_info_32_s {
    ptr_t           dlpi_addr;  //void*
    ptr_t           dlpi_name;  //const char*
    ptr_t           dlpi_phdr;  //Elf32_Phdr*
    Elf32_Half      dlpi_phnum;
} my_dl_phdr_info_32_t;

static int dl_iterate_phdr_callback(x64emu_t *emu, void* F, my_dl_phdr_info_32_t *info, size_t size, void* data)
{
    int ret = RunFunctionWithEmu(emu, 0, (uintptr_t)F, 3, to_ptrv(info), to_ulong(size), to_ptrv(data));
    return ret;
}

EXPORT int my32_dl_iterate_phdr(x64emu_t *emu, void* F, void *data) {
    printf_log(LOG_DEBUG, "Call to partially implemented 32bits dl_iterate_phdr(%p, %p)\n", F, data);
    box64context_t *context = GetEmuContext(emu);
    const char* empty = "";
    int ret = 0;
    for (int idx=0; idx<context->elfsize; ++idx) {
        if(context->elfs[idx]) {
            static my_dl_phdr_info_32_t info;
            info.dlpi_addr = to_ptrv(GetElfDelta(context->elfs[idx]));
            info.dlpi_name = to_ptrv((void*)(idx?context->elfs[idx]->name:empty));    //1st elf is program, and this one doesn't get a name
            info.dlpi_phdr = to_ptrv(context->elfs[idx]->PHEntries._32);
            info.dlpi_phnum = context->elfs[idx]->numPHEntries;
            if((ret = dl_iterate_phdr_callback(emu, F, &info, sizeof(info), data))) {
                return ret;
            }
        }
    }
    // not iterationg on native libs
    return ret;
}
