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

#include "os.h"
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
#include "box64cpu.h"
#include "box64cpu_util.h"
#include "box64stack.h"
#include "callback.h"
#include "box64stack.h"
#include "wine_tools.h"
#include "dictionnary.h"
#include "symbols.h"
#include "cleanup.h"
#include "globalsymbols.h"
#ifdef DYNAREC
#include "dynablock.h"
#endif
#include "../emu/x64emu_private.h"
#include "../emu/x64run_private.h"
#include "../tools/bridge_private.h"
#include "x64tls.h"

void* my__IO_2_1_stderr_ = (void*)1;
void* my__IO_2_1_stdin_  = (void*)2;
void* my__IO_2_1_stdout_ = (void*)3;

uintptr_t pltResolver64 = ~0LL;

// return the index of header (-1 if it doesn't exist)
static int getElfIndex(box64context_t* ctx, elfheader_t* head) {
    for (int i=0; i<ctx->elfsize; ++i)
        if(ctx->elfs[i]==head)
            return i;
    return -1;
}

elfheader_t* LoadAndCheckElfHeader(FILE* f, const char* name, int exec)
{
    elfheader_t *h = box64_is32bits?ParseElfHeader32(f, name, exec):ParseElfHeader64(f, name, exec);
    if(!h)
        return NULL;

    if ((h->path = box_realpath(name, NULL)) == NULL) {
        h->path = (char*)box_malloc(1);
        h->path[0] = '\0';
    }

    h->refcnt = 0;

    h->file = f;
    h->fileno = fileno(f);

    return h;
}

void FreeElfHeader(elfheader_t** head)
{
    if(!head || !*head)
        return;
    elfheader_t *h = *head;
    if(my_context)
        RemoveElfHeader(my_context, h);

    actual_free(h->PHEntries._64); //_64 or _32 doesn't mater for free, it's the same address
    actual_free(h->SHEntries._64);
    actual_free(h->SHStrTab);
    actual_free(h->StrTab);
    actual_free(h->Dynamic._64);
    actual_free(h->DynStr);
    actual_free(h->SymTab._64);
    actual_free(h->DynSym._64);

    FreeElfMemory(h);

    actual_free(h->name);
    actual_free(h->path);
    if(h->file)
        fclose(h->file);
    actual_free(h);

    *head = NULL;
}

int CalcLoadAddr(elfheader_t* head)
{
    head->memsz = 0;
    head->paddr = head->vaddr = ~(uintptr_t)0;
    head->align = box64_pagesize;
    if(box64_is32bits) {
        for (size_t i=0; i<head->numPHEntries; ++i)
            if(head->PHEntries._32[i].p_type == PT_LOAD) {
                if(head->paddr > (uintptr_t)head->PHEntries._32[i].p_paddr)
                    head->paddr = (uintptr_t)head->PHEntries._32[i].p_paddr;
                if(head->vaddr > (uintptr_t)head->PHEntries._32[i].p_vaddr)
                    head->vaddr = (uintptr_t)head->PHEntries._32[i].p_vaddr;
            }
    } else {
        for (size_t i=0; i<head->numPHEntries; ++i)
            if(head->PHEntries._64[i].p_type == PT_LOAD) {
                if(head->paddr > (uintptr_t)head->PHEntries._64[i].p_paddr)
                    head->paddr = (uintptr_t)head->PHEntries._64[i].p_paddr;
                if(head->vaddr > (uintptr_t)head->PHEntries._64[i].p_vaddr)
                    head->vaddr = (uintptr_t)head->PHEntries._64[i].p_vaddr;
            }
    }

    if(head->vaddr==~(uintptr_t)0 || head->paddr==~(uintptr_t)0) {
        printf_log(LOG_NONE, "Error: v/p Addr for Elf Load not set\n");
        return 1;
    }

    head->stacksz = 1024*1024;          //1M stack size default?
    head->stackalign = 16;   // default align for stack
    if(box64_is32bits)
        for (size_t i=0; i<head->numPHEntries; ++i) {
            if(head->PHEntries._32[i].p_type == PT_LOAD) {
                uintptr_t phend = head->PHEntries._32[i].p_vaddr - head->vaddr + head->PHEntries._32[i].p_memsz;
                if(phend > head->memsz)
                    head->memsz = phend;
                if(head->PHEntries._32[i].p_align > head->align)
                    head->align = head->PHEntries._32[i].p_align;
            }
            if(head->PHEntries._32[i].p_type == PT_GNU_STACK) {
                if(head->stacksz < head->PHEntries._32[i].p_memsz)
                    head->stacksz = head->PHEntries._32[i].p_memsz;
                if(head->stackalign < head->PHEntries._32[i].p_align)
                    head->stackalign = head->PHEntries._32[i].p_align;
            }
            if(head->PHEntries._32[i].p_type == PT_TLS) {
                head->tlsaddr = head->PHEntries._32[i].p_vaddr;
                head->tlssize = head->PHEntries._32[i].p_memsz;
                head->tlsfilesize = head->PHEntries._32[i].p_filesz;
                head->tlsalign = head->PHEntries._32[i].p_align;
                // force alignement...
                if(head->tlsalign>1)
                    while(head->tlssize&(head->tlsalign-1))
                        head->tlssize++;
            }
        }
    else
        for (size_t i=0; i<head->numPHEntries; ++i) {
            if(head->PHEntries._64[i].p_type == PT_LOAD) {
                uintptr_t phend = head->PHEntries._64[i].p_vaddr - head->vaddr + head->PHEntries._64[i].p_memsz;
                if(phend > head->memsz)
                    head->memsz = phend;
                if(head->PHEntries._64[i].p_align > head->align)
                    head->align = head->PHEntries._64[i].p_align;
            }
            if(head->PHEntries._64[i].p_type == PT_GNU_STACK) {
                if(head->stacksz < head->PHEntries._64[i].p_memsz)
                    head->stacksz = head->PHEntries._64[i].p_memsz;
                if(head->stackalign < head->PHEntries._64[i].p_align)
                    head->stackalign = head->PHEntries._64[i].p_align;
            }
            if(head->PHEntries._64[i].p_type == PT_TLS) {
                head->tlsaddr = head->PHEntries._64[i].p_vaddr;
                head->tlssize = head->PHEntries._64[i].p_memsz;
                head->tlsfilesize = head->PHEntries._64[i].p_filesz;
                head->tlsalign = head->PHEntries._64[i].p_align;
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

int AllocLoadElfMemory32(box64context_t* context, elfheader_t* head, int mainbin)
#ifndef BOX32
{ return 1; }
#else
 ;
#endif
int AllocLoadElfMemory(box64context_t* context, elfheader_t* head, int mainbin)
{
    if(box64_is32bits)
        return AllocLoadElfMemory32(context, head, mainbin);
    uintptr_t offs = 0;
    loadProtectionFromMap();
    int log_level = BOX64ENV(load_addr)?LOG_INFO:LOG_DEBUG;

    head->multiblock_n = 0; // count PHEntrie with LOAD
    uintptr_t max_align = head->align-1;
    for (size_t i=0; i<head->numPHEntries; ++i)
        if(head->PHEntries._64[i].p_type == PT_LOAD && head->PHEntries._64[i].p_flags) {
            ++head->multiblock_n;
        }

    if(!head->vaddr && BOX64ENV(load_addr)) {
        offs = (uintptr_t)find47bitBlockNearHint((void*)((BOX64ENV(load_addr)+max_align)&~max_align), head->memsz+head->align, max_align);
        BOX64ENV(load_addr) = offs + head->memsz;
        BOX64ENV(load_addr) = (BOX64ENV(load_addr)+0x10ffffffLL)&~0xffffffLL;
    }
    if(!offs && !head->vaddr)
        offs = (uintptr_t)find47bitBlockElf(head->memsz+head->align, mainbin, max_align); // limit to 47bits...
    // prereserve the whole elf image, without populating
    size_t sz = head->memsz;
    void* raw = NULL;
    void* image = NULL;
    if(!head->vaddr) {
        sz += head->align;
        raw = InternalMmap((void*)offs, sz, 0, MAP_ANONYMOUS|MAP_PRIVATE|MAP_NORESERVE, -1, 0);
        image = (void*)(((uintptr_t)raw+max_align)&~max_align);
    } else {
        image = raw = InternalMmap((void*)head->vaddr, sz, 0, MAP_ANONYMOUS|MAP_PRIVATE|MAP_NORESERVE, -1, 0);
        if(head->vaddr&(box64_pagesize-1)) {
            // load address might be lower
            if((uintptr_t)image == (head->vaddr&~(box64_pagesize-1))) {
                image = (void*)head->vaddr;
                sz += ((uintptr_t)image)-((uintptr_t)raw);
            }
        }
    }
    if(image!=MAP_FAILED && !head->vaddr && image!=(void*)offs) {
        printf_log(LOG_INFO, "%s: Mmap64 for (@%p 0x%zx) for elf \"%s\" returned %p(%p/0x%zx) instead\n", (((uintptr_t)image)&max_align)?"Error":"Warning", (void*)(head->vaddr?head->vaddr:offs), head->memsz, head->name, image, raw, head->align);
        offs = (uintptr_t)image;
        if(((uintptr_t)image)&max_align) {
            InternalMunmap(raw, sz);
            return 1;   // that's an error, alocated memory is not aligned properly
        }
    }
    if(image==MAP_FAILED || image!=(void*)(head->vaddr?head->vaddr:offs)) {
        printf_log(LOG_NONE, "%s cannot create memory map (@%p 0x%zx) for elf \"%s\"", (image==MAP_FAILED)?"Error:":"Warning:", (void*)(head->vaddr?head->vaddr:offs), head->memsz, head->name);
        if(image==MAP_FAILED) {
            printf_log_prefix(0, LOG_NONE, " error=%d/%s\n", errno, strerror(errno));
        } else {
            printf_log_prefix(0, LOG_NONE, " got %p\n", image);
        }
        if(image==MAP_FAILED)
            return 1;
        offs = (uintptr_t)image-head->vaddr;
    }
    printf_dump(log_level, "Pre-allocated 0x%zx byte at %p for %s\n", head->memsz, image, head->name);
    head->delta = offs;
    printf_dump(log_level, "Delta of %p (vaddr=%p) for Elf \"%s\"\n", (void*)offs, (void*)head->vaddr, head->name);

    head->image = image;
    head->raw = raw;
    head->raw_size = sz;
    setProtection_elf((uintptr_t)raw, sz, 0);

    head->multiblocks = (multiblock_t*)box_calloc(head->multiblock_n, sizeof(multiblock_t));
    head->tlsbase = AddTLSPartition(context, head->tlssize);
    // and now, create all individual blocks
    head->memory = (char*)0xffffffffffffffff;
    int n = 0;
    for (size_t i=0; i<head->numPHEntries; ++i) {
        if(head->PHEntries._64[i].p_type == PT_LOAD && head->PHEntries._64[i].p_flags) {
            Elf64_Phdr * e = &head->PHEntries._64[i];

            head->multiblocks[n].flags = e->p_flags;
            head->multiblocks[n].offs = e->p_offset;
            head->multiblocks[n].paddr = e->p_paddr + offs;
            head->multiblocks[n].size = e->p_filesz;
            head->multiblocks[n].align = e->p_align;
            // HACK: Mark all the code pages writable in unittest mode because some tests mix code and (writable) data...
            uint8_t prot = ((e->p_flags & PF_R)?PROT_READ:0)|(((e->p_flags & PF_W) || box64_unittest_mode)?PROT_WRITE:0)|((e->p_flags & PF_X)?PROT_EXEC:0);
            // check if alignment is correct
            uintptr_t balign = head->multiblocks[n].align-1;
            if (balign < (box64_pagesize - 1)) balign = box64_pagesize - 1;
            head->multiblocks[n].asize = (e->p_memsz + (e->p_paddr & balign) + (box64_pagesize - 1)) & ~(box64_pagesize - 1);
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
                printf_dump(log_level, "Mmaping 0x%lx(0x%lx) bytes @%p with prot %x for Elf \"%s\"\n", head->multiblocks[n].size, head->multiblocks[n].asize, (void*)head->multiblocks[n].paddr, prot, head->name);
                void* p = InternalMmap(
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
                size_t asize = head->multiblocks[n].asize+(head->multiblocks[n].paddr-paddr);
                void* p = MAP_FAILED;
                if(paddr==(paddr&~(box64_pagesize-1)) && (asize==ALIGN(asize))) {
                    printf_dump(log_level, "Allocating 0x%zx (0x%zx) bytes @%p, will read 0x%zx @%p for Elf \"%s\"\n", asize, e->p_memsz, (void*)paddr, e->p_filesz, (void*)head->multiblocks[n].paddr, head->name);
                    p = InternalMmap(
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
                        p = InternalMmap(
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
        if(head->PHEntries._64[i].p_type == PT_TLS) {
            Elf64_Phdr * e = &head->PHEntries._64[i];
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

    PatchLoadedDynamicSection(head);

    return 0;
}

void FreeElfMemory(elfheader_t* head)
{
    if(head->multiblock_n) {
#ifdef DYNAREC
        for(int i=0; i<head->multiblock_n; ++i) {
            dynarec_log(LOG_INFO, "Free DynaBlocks %p-%p for %s\n", head->multiblocks[i].p, head->multiblocks[i].p+head->multiblocks[i].asize, head->path);
            if(BOX64ENV(dynarec))
                cleanDBFromAddressRange((uintptr_t)head->multiblocks[i].p, head->multiblocks[i].asize, 1);
            freeProtection((uintptr_t)head->multiblocks[i].p, head->multiblocks[i].asize);
        }
#endif
        box_free(head->multiblocks);
    }
    // we only need to free the overall mmap, no need to free individual part as they are inside the big one
    if(head->raw && head->raw_size) {
        dynarec_log(LOG_INFO, "Unmap elf memory %p-%p for %s\n", head->raw, head->raw+head->raw_size, head->path);
        InternalMunmap(head->raw, head->raw_size);
    }
    freeProtection((uintptr_t)head->raw, head->raw_size);
}

int isElfHasNeededVer(elfheader_t* head, const char* libname, elfheader_t* verneeded)
{
    if(!verneeded || !head)
        return 1;
    if(!head->VerDef._64 || !verneeded->VerNeed._64)
        return 1;
    int cnt = GetNeededVersionCnt(verneeded, libname);
    for (int i=0; i<cnt; ++i) {
        const char* vername = GetNeededVersionString(verneeded, libname, i);
        if(vername && !GetVersionIndice(head, vername)) {
            printf_log(/*LOG_DEBUG*/LOG_INFO, "Discarding %s for missing version %s\n", head->path, vername);
            return 0;   // missing version
        }
    }
    return 1;
}

static int IsSymInElfSpace(const elfheader_t* h, Elf64_Sym* sym)
{
    if(!h || !sym)
        return 0;
    uintptr_t addr = (uintptr_t)sym;
    if(h->SymTab._64 && addr>=(uintptr_t)h->SymTab._64 && addr<(uintptr_t)&h->SymTab._64[h->numSymTab])
        return 1;
    if(h->DynSym._64 && addr>=(uintptr_t)h->DynSym._64 && addr<(uintptr_t)&h->DynSym._64[h->numDynSym])
        return 1;
    return 0;
}
static elfheader_t* FindElfSymbol(box64context_t *context, Elf64_Sym* sym)
{
    if(!sym)
        return NULL;
    for (int i=0; i<context->elfsize; ++i)
        if(IsSymInElfSpace(context->elfs[i], sym))
            return context->elfs[i];

    return NULL;
}

static int FindR64COPYRel(elfheader_t* h, const char* name, uintptr_t *offs, uint64_t** p, size_t size, int version, const char* vername, int veropt)
{
    if(!h)
        return 0;
    Elf64_Rela * rela = (Elf64_Rela *)(h->rela + h->delta);
    if(!h->rela)
        return 0;
    int cnt = h->relasz / h->relaent;
    for (int i=0; i<cnt; ++i) {
        int t = ELF64_R_TYPE(rela[i].r_info);
        Elf64_Sym *sym = &h->DynSym._64[ELF64_R_SYM(rela[i].r_info)];
        const char* symname = SymName64(h, sym);
        if((t==R_X86_64_COPY) && symname && !strcmp(symname, name) && (sym->st_size==size)) {
            int version2 = h->VerSym?((Elf64_Half*)((uintptr_t)h->VerSym+h->delta))[ELF64_R_SYM(rela[i].r_info)]:-1;
            if(version2!=-1) version2 &= 0x7fff;
            if(version && !version2) version2=-1;   // match a versioned symbol against a global "local" symbol
            const char* vername2 = GetSymbolVersion(h, version2);
            Elf64_Half flags = GetSymbolVersionFlag(h, version2);
            int veropt2 = flags?0:1;
            if(SameVersionedSymbol(name, version, vername, veropt, symname, version2, vername2, veropt2)) {
                if(offs) *offs = sym->st_value + h->delta;
                if(p) *p = (uint64_t*)(rela[i].r_offset + h->delta + rela[i].r_addend);
                return 1;
            }
        }
    }
    return 0;
}

static int RelocateElfREL(lib_t *maplib, lib_t *local_maplib, int bindnow, int deepbind, elfheader_t* head, int cnt, Elf64_Rel *rel)
{
    printf_log(LOG_NONE, "Error: REL type of Relocation unsupported (only RELA)\n");
    return 1;
}

struct tlsdesc
{
  uintptr_t entry;
  uintptr_t arg;
};
uintptr_t tlsdescUndefweak = 0;
uintptr_t GetSegmentBaseEmu(x64emu_t* emu, int seg);
EXPORT uintptr_t my__dl_tlsdesc_undefweak(x64emu_t* emu)
{
    struct tlsdesc *td = (struct tlsdesc *)R_RAX;
    return td->arg;
}

static void GrabX64CopyMainElfReloc(elfheader_t* head)
{
    if(head->rela) {
        int cnt = head->relasz / head->relaent;
        Elf64_Rela* rela = (Elf64_Rela *)(head->rela + head->delta);
        printf_dump(LOG_DEBUG, "Grabbing R_X86_64_COPY Relocation(s) in advance for %s\n", head->name);
        for (int i=0; i<cnt; ++i) {
            int t = ELF64_R_TYPE(rela[i].r_info);
            if(t == R_X86_64_COPY) {
                Elf64_Sym *sym = &head->DynSym._64[ELF64_R_SYM(rela[i].r_info)];
                const char* symname = SymName64(head, sym);
                int version = head->VerSym?((Elf64_Half*)((uintptr_t)head->VerSym+head->delta))[ELF64_R_SYM(rela[i].r_info)]:-1;
                if(version!=-1) version &=0x7fff;
                const char* vername = GetSymbolVersion(head, version);
                Elf64_Half flags = GetSymbolVersionFlag(head, version);
                int veropt = flags?0:1;
                uintptr_t offs = sym->st_value + head->delta;
                AddUniqueSymbol(my_context->globdata, symname, offs, sym->st_size, version, vername, veropt);
            }
        }
    }
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

static int RelocateElfRELA(lib_t *maplib, lib_t *local_maplib, int bindnow, int deepbind, elfheader_t* head, int cnt, Elf64_Rela *rela, int* need_resolv)
{
    int ret_ok = 0;
    for (int i=0; i<cnt; ++i) {
        int t = ELF64_R_TYPE(rela[i].r_info);
        Elf64_Sym *sym = &head->DynSym._64[ELF64_R_SYM(rela[i].r_info)];
        int bind = ELF64_ST_BIND(sym->st_info);
        //uint64_t ndx = sym->st_shndx;
        const char* symname = SymName64(head, sym);
        uint64_t *p = (uint64_t*)(rela[i].r_offset + head->delta);
        uintptr_t offs = 0;
        uintptr_t end = 0;
        size_t size = sym->st_size;
        elfheader_t* sym_elf = NULL;
        elfheader_t* last_elf = NULL;
        int version = head->VerSym?((Elf64_Half*)((uintptr_t)head->VerSym+head->delta))[ELF64_R_SYM(rela[i].r_info)]:-1;
        if(version!=-1) version &=0x7fff;
        const char* vername = GetSymbolVersion(head, version);
        Elf64_Half flags = GetSymbolVersionFlag(head, version);
        int veropt = flags?0:1;
        Elf64_Sym* elfsym = NULL;
        int vis = ELF64_ST_VISIBILITY(sym->st_other);
        if(vis==STV_PROTECTED) {
            elfsym = ElfDynSymLookup64(head, symname);
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
                    elfsym = ElfDynSymLookup64(head, symname);
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
        if(elfsym && (ELF64_ST_TYPE(elfsym->st_info)==STT_TLS))
            offs = elfsym->st_value;
        uintptr_t globoffs, globend;
        uint64_t* globp;
        uintptr_t tmp = 0;
        intptr_t delta;
        int global;
        switch(t) {
            case R_X86_64_NONE:
                break;
            case R_X86_64_PC32:
                // should be "S + A - P" with S=symbol offset, A=addend and P=place of the storage unit, write a word32
                // can be ignored (so *p = offs + addend - p)
                break;
            case R_X86_64_RELATIVE:
                printf_dump(LOG_NEVER, "Apply %s R_X86_64_RELATIVE @%p (%p -> %p)\n", BindSym(bind), p, *(void**)p, (void*)(head->delta+ rela[i].r_addend));
                *p = head->delta+ rela[i].r_addend;
                break;
            case R_X86_64_IRELATIVE:
                {
                    x64emu_t* emu = thread_get_emu();
                    EmuCall(emu, head->delta+rela[i].r_addend);
                    printf_dump(LOG_NEVER, "Apply %s R_X86_64_IRELATIVE @%p (%p -> %p()=%p)\n", BindSym(bind), p, *(void**)p, (void*)(head->delta+ rela[i].r_addend), (void*)(R_RAX));
                    *p = R_RAX;
                }
                break;
            case R_X86_64_COPY:
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
                    printf_dump(LOG_NEVER, "Apply R_X86_64_COPY @%p with sym=%s (%sver=%d/%s), @%p+0x%lx size=%ld\n", p, symname, veropt?"opt":"", version, vername?vername:"(none)", (void*)offs, rela[i].r_addend, sym->st_size);
                    if(p!=(void*)(offs+rela[i].r_addend))
                        memmove(p, (void*)(offs+rela[i].r_addend), sym->st_size);
                    sym_elf = FindElfAddress(my_context, offs);
                    if(sym_elf && sym_elf!=last_elf && sym_elf!=head) last_elf = checkElfLib(head, sym_elf->lib);
                } else {
                    printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply RELA R_X86_64_COPY @%p (%p) in %s\n", symname, p, *(void**)p, head->name);
                }
                break;
            case R_X86_64_GLOB_DAT:
                if((global = GetSymbolStartEnd(my_context->globdata, symname, &globoffs, &globend, version, vername, 1, veropt))) {
                    globp = (uint64_t*)globoffs;
                    printf_dump(LOG_NEVER, "Apply %s R_X86_64_GLOB_DAT with R_X86_64_COPY @%p/%p (%p/%p -> %p/%p) size=%zd on sym=%s (%sver=%d/%s) \n",
                        BindSym(bind), p, globp, (void*)(p?(*p):0),
                        (void*)(globp?(*globp):0), (void*)offs, (void*)globoffs, sym->st_size, symname, veropt?"opt":"", version, vername?vername:"(none)");
                    sym_elf = my_context->elfs[0];
                    *p = globoffs;
                #ifndef STATICBUILD
                    if(global==2)
                        addGlobalRef(p, symname);
                #endif
                } else {
                    if (!offs) {
                        if(strcmp(symname, "__gmon_start__") && strcmp(symname, "data_start") && strcmp(symname, "__data_start") && strcmp(symname, "collector_func_load"))
                            printf_log((bind==STB_WEAK)?LOG_DEBUG:LOG_NONE, "%s: Global Symbol %s not found, cannot apply R_X86_64_GLOB_DAT @%p (%p) in %s\n", (bind==STB_WEAK)?"Warning":"Error", symname, p, *(void**)p, head->name);
                    } else {
                        printf_dump(LOG_NEVER, "Apply %s R_X86_64_GLOB_DAT @%p (%p -> %p) on sym=%s (%sver=%d/%s, elf=%s)\n", BindSym(bind), p, (void*)(p?(*p):0), (void*)(offs + rela[i].r_addend), symname, veropt?"opt":"", version, vername?vername:"(none)", sym_elf?sym_elf->name:"(native)");
                        *p = offs + rela[i].r_addend;
                        if(sym_elf && sym_elf!=last_elf && sym_elf!=head) last_elf = checkElfLib(head, sym_elf->lib);
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
                            printf_log(LOG_NONE, "Error: Symbol %s not found, cannot apply R_X86_64_JUMP_SLOT @%p (%p) in %s (%sver=%d / %s)\n", symname, p, *(void**)p, head->name, veropt?"opt":"", version, vername?vername:"(none)");
                            ret_ok = 1;
                        }
                        // return -1;
                    } else {
                        if(p) {
                            printf_dump(LOG_NEVER, "Apply %s R_X86_64_JUMP_SLOT @%p with sym=%s (%p -> %p / %s (%sver=%d / %s))\n",
                                BindSym(bind), p, symname, *(void**)p, (void*)(offs+rela[i].r_addend), sym_elf?sym_elf->name:"native", veropt?"opt":"", version, vername?vername:"(none)");
                            *p = offs + rela[i].r_addend;
                            if(sym_elf && sym_elf!=last_elf && sym_elf!=head) last_elf = checkElfLib(head, sym_elf->lib);
                        } else {
                            printf_log(LOG_INFO, "Warning, Symbol %s found, but Jump Slot Offset is NULL \n", symname);
                        }
                    }
                } else {
                    printf_dump(LOG_NEVER, "Preparing (if needed) %s R_X86_64_JUMP_SLOT @%p (0x%lx->0x%0lx) with sym=%s to be apply later (addend=%ld)\n",
                        BindSym(bind), p, *p, *p+head->delta, symname, rela[i].r_addend);
                    *p += head->delta;
                    *need_resolv = 1;
                }
                break;
            case R_X86_64_64:
                if(GetSymbolStartEnd(my_context->globdata, symname, &globoffs, &globend, version, vername, 1, veropt)) {
                    if(offs!=globoffs) {
                        offs = globoffs;
                        sym_elf = my_context->elfs[0];
                        elfsym = ElfDynSymLookup64(sym_elf, symname);
                    }
                }
                if (!offs && !elfsym) {
                    printf_log(LOG_INFO, "%s: Symbol %s not found, cannot apply R_X86_64_64 @%p (%p) in %s\n", (bind==STB_GLOBAL)?"Error":"Warning", symname, p, *(void**)p, head->name);
                    if(bind==STB_GLOBAL)
                        ret_ok = 1;
                } else {
                    printf_dump(LOG_NEVER, "Apply %s R_X86_64_64 @%p with sym=%s (%sver=%d/%s) addend=0x%lx (%p -> %p)\n",
                        BindSym(bind), p, symname, veropt?"opt":"", version, vername?vername:"(none)", rela[i].r_addend, *(void**)p, (void*)(offs+rela[i].r_addend/*+*(uint64_t*)p*/));
                    *p /*+*/= offs+rela[i].r_addend;
                    if(sym_elf && sym_elf!=last_elf && sym_elf!=head) last_elf = checkElfLib(head, sym_elf->lib);
                }
                break;
            case R_X86_64_TPOFF64:
                // Negated offset in static TLS block
                {
                    if(!symname || !symname[0]) {
                        sym_elf = head;
                        offs = sym->st_value;
                    }
                    if(sym_elf) {
                        delta = *(int64_t*)p;
                        printf_dump(LOG_NEVER, "Applying %s %s on %s @%p (%ld -> %ld+%ld+%ld, size=%ld)\n", BindSym(bind), DumpRelType64(t), symname, p, delta, sym_elf->tlsbase, (int64_t)offs, rela[i].r_addend, end-offs);
                        *p = (uintptr_t)((int64_t)offs + rela[i].r_addend + sym_elf->tlsbase);
                        if(sym_elf && sym_elf!=last_elf && sym_elf!=head) last_elf = checkElfLib(head, sym_elf->lib);
                    } else {
                        printf_log(LOG_INFO, "Warning, cannot apply %s %s on %s @%p (%ld), no elf_header found\n", BindSym(bind), DumpRelType64(t), symname, p, (int64_t)offs);
                    }
                }
                break;
            case R_X86_64_DTPMOD64:
                // ID of module containing symbol
                if(!symname || symname[0]=='\0' || bind==STB_LOCAL) {
                    offs = getElfIndex(my_context, head);
                    sym_elf = head;
                } else {
                    offs = getElfIndex(my_context, sym_elf);
                }
                if(p) {
                    printf_dump(LOG_NEVER, "Apply %s %s @%p with sym=%s (%p -> %p)\n", BindSym(bind), "R_X86_64_DTPMOD64", p, symname, *(void**)p, (void*)offs);
                    *p = offs;
                    if(sym_elf && sym_elf!=last_elf && sym_elf!=head) last_elf = checkElfLib(head, sym_elf->lib);
                } else {
                    printf_log(LOG_INFO, "Warning, Symbol %s or Elf not found, but R_X86_64_DTPMOD64 Slot Offset is NULL \n", symname);
                }
                break;
            case R_X86_64_DTPOFF64:
                // Offset in TLS block
                if (!offs && !sym_elf) {
                    if(bind==STB_WEAK) {
                        printf_log(LOG_INFO, "Warning: Weak Symbol %s not found, cannot apply R_X86_64_DTPOFF64 @%p (%p)\n", symname, p, *(void**)p);
                    } else {
                        printf_log(LOG_INFO, "Error: Symbol %s not found, cannot apply R_X86_64_DTPOFF64 @%p (%p) in %s\n", symname, p, *(void**)p, head->name);
                    }
                    // return -1;
                } else {
                    if(!symname || symname[0]=='\0') {
                        offs = sym->st_value;
                        sym_elf = head;
                    }
                    if(p) {
                        int64_t tlsoffset = (int64_t)offs;    // it's not an offset in elf memory
                        printf_dump(LOG_NEVER, "Apply %s R_X86_64_DTPOFF64 @%p with sym=%s (%p -> %p)\n", BindSym(bind), p, symname, (void*)tlsoffset, (void*)offs);
                        *p = tlsoffset;
                        if(sym_elf && sym_elf!=last_elf && sym_elf!=head) last_elf = checkElfLib(head, sym_elf->lib);
                    } else {
                        printf_log(LOG_INFO, "Warning, Symbol %s found, but R_X86_64_DTPOFF64 Slot Offset is NULL \n", symname);
                    }
                }
                break;
            case R_X86_64_TLSDESC:
                if(!symname || !symname[0]) {
                    printf_dump(LOG_NEVER, "Apply %s R_X86_64_TLSDESC @%p with addend=%zu\n", BindSym(bind), p, rela[i].r_addend);
                    struct tlsdesc volatile *td = (struct tlsdesc volatile *)p;
                    if(!tlsdescUndefweak)
                        tlsdescUndefweak = AddBridge(my_context->system, pFE, my__dl_tlsdesc_undefweak, 0, "_dl_tlsdesc_undefweak");
                    td->entry = tlsdescUndefweak;
                    td->arg = (uintptr_t)(head->tlsbase + rela[i].r_addend);
                } else {
                    printf_log(LOG_INFO, "Warning, R_X86_64_TLSDESC used with Symbol %s(%p) not supported for now \n", symname, sym);
                }
                break;
            default:
                printf_log(LOG_INFO, "Warning, don't know of to handle rela #%d %s on %s\n", i, DumpRelType64(ELF64_R_TYPE(rela[i].r_info)), symname);
        }
    }
    return bindnow?ret_ok:0;
}

static int RelocateElfRELR(elfheader_t *head, int cnt, Elf64_Relr *relr) {
    Elf64_Addr *where = NULL;
    for (int i = 0; i < cnt; i++) {
        Elf64_Relr p = relr[i];
        if ((p & 1) == 0) {
            where = (Elf64_Addr *)(p + head->delta);
            printf_dump(LOG_NEVER, "Apply (even) RELR %p -> %p\n", *where, *where + head->delta);
            *where++ += head->delta;
        } else {
            for (long j = 0; (p >>= 1) != 0; j++)
                if ((p & 1) != 0) {
                    printf_dump(LOG_NEVER, "Apply (odd) RELR %p -> %p\n", where[j], where[j] + head->delta);
                    where[j] += head->delta;
                }
            where += CHAR_BIT * sizeof(Elf64_Relr) - 1;
        }
    }
    return 0;
}

int RelocateElf32(lib_t *maplib, lib_t *local_maplib, int bindnow, int deepbind, elfheader_t* head)
#ifndef BOX32
{ return -1; }
#else
 ;
#endif
int RelocateElf64(lib_t *maplib, lib_t *local_maplib, int bindnow, int deepbind, elfheader_t* head)
{
    if((head->flags&DF_BIND_NOW) && !bindnow) {
        bindnow = 1;
        printf_log(LOG_DEBUG, "Forcing %s to Bind Now\n", head->name);
    }
    if(head->relr) {
        int cnt = head->relrsz / head->relrent;
        DumpRelRTable64(head, cnt, (Elf64_Relr *)(head->relr + head->delta), "RelR");
        printf_dump(LOG_DEBUG, "Applying %d Relocation(s) without Addend for %s bindnow=%d, deepbind=%d\n", cnt, head->name, bindnow, deepbind);
        if(RelocateElfRELR(head, cnt, (Elf64_Relr *)(head->relr + head->delta)))
            return -1;
    }
    if(head->rel) {
        int cnt = head->relsz / head->relent;
        DumpRelTable64(head, cnt, (Elf64_Rel *)(head->rel + head->delta), "Rel");
        printf_dump(LOG_DEBUG, "Applying %d Relocation(s) for %s bindnow=%d, deepbind=%d\n", cnt, head->name, bindnow, deepbind);
        if(RelocateElfREL(maplib, local_maplib, bindnow, deepbind, head, cnt, (Elf64_Rel *)(head->rel + head->delta)))
            return -1;
    }
    if(head->rela) {
        int cnt = head->relasz / head->relaent;
        DumpRelATable64(head, cnt, (Elf64_Rela *)(head->rela + head->delta), "RelA");
        printf_dump(LOG_DEBUG, "Applying %d Relocation(s) with Addend for %s bindnow=%d, deepbind=%d\n", cnt, head->name, bindnow, deepbind);
        if(RelocateElfRELA(maplib, local_maplib, bindnow, deepbind, head, cnt, (Elf64_Rela *)(head->rela + head->delta), NULL))
            return -1;
    }
    return 0;
}
int RelocateElf(lib_t *maplib, lib_t *local_maplib, int bindnow, int deepbind, elfheader_t* head)
{
    return box64_is32bits?RelocateElf32(maplib, local_maplib, bindnow, deepbind, head):RelocateElf64(maplib, local_maplib, bindnow, deepbind, head);
}

int RelocateElfPlt32(lib_t *maplib, lib_t *local_maplib, int bindnow, int deepbind, elfheader_t* head)
#ifndef BOX32
{ return -1; }
#else
 ;
#endif
int RelocateElfPlt64(lib_t *maplib, lib_t *local_maplib, int bindnow, int deepbind, elfheader_t* head)
{
    int need_resolver = 0;
    if((head->flags&DF_BIND_NOW) && !bindnow) {
        bindnow = 1;
        printf_log(LOG_DEBUG, "Forcing %s to Bind Now\n", head->name);
    }
    if(head->pltrel) {
        int cnt = head->pltsz / head->pltent;
        if(head->pltrel==DT_REL) {
            DumpRelTable64(head, cnt, (Elf64_Rel *)(head->jmprel + head->delta), "PLT");
            printf_dump(LOG_DEBUG, "Applying %d PLT Relocation(s) for %s bindnow=%d, deepbind=%d\n", cnt, head->name, bindnow, deepbind);
            if(RelocateElfREL(maplib, local_maplib, bindnow, deepbind, head, cnt, (Elf64_Rel *)(head->jmprel + head->delta)))
                return -1;
        } else if(head->pltrel==DT_RELA) {
            DumpRelATable64(head, cnt, (Elf64_Rela *)(head->jmprel + head->delta), "PLT");
            printf_dump(LOG_DEBUG, "Applying %d PLT Relocation(s) with Addend for %s bindnow=%d, deepbind=%d\n", cnt, head->name, bindnow, deepbind);
            if(RelocateElfRELA(maplib, local_maplib, bindnow, deepbind, head, cnt, (Elf64_Rela *)(head->jmprel + head->delta), &need_resolver))
                return -1;
        }
        if(need_resolver) {
            if(pltResolver64==(uintptr_t)-1) {
                pltResolver64 = AddBridge(my_context->system, vFE, PltResolver64, 0, "PltResolver");
            }
            if(head->pltgot) {
                *(uintptr_t*)(head->pltgot+head->delta+16) = pltResolver64;
                *(uintptr_t*)(head->pltgot+head->delta+8) = (uintptr_t)head;
                printf_dump(LOG_DEBUG, "PLT Resolver injected in plt.got at %p\n", (void*)(head->pltgot+head->delta+16));
            } else if(head->got) {
                *(uintptr_t*)(head->got+head->delta+16) = pltResolver64;
                *(uintptr_t*)(head->got+head->delta+8) = (uintptr_t)head;
                printf_dump(LOG_DEBUG, "PLT Resolver injected in got at %p\n", (void*)(head->got+head->delta+16));
            }
        }
    }

    return 0;
}
int RelocateElfPlt(lib_t *maplib, lib_t *local_maplib, int bindnow, int deepbind, elfheader_t* head)
{
    return box64_is32bits?RelocateElfPlt32(maplib, local_maplib, bindnow, deepbind, head):RelocateElfPlt64(maplib, local_maplib, bindnow, deepbind, head);
}

void CalcStack(elfheader_t* elf, uint64_t* stacksz, size_t* stackalign)
{
    if(*stacksz < elf->stacksz)
        *stacksz = elf->stacksz;
    if(*stackalign < elf->stackalign)
        *stackalign = elf->stackalign;
}

uintptr_t GetEntryPoint(lib_t* maplib, elfheader_t* h)
{
    (void)maplib;
    uintptr_t ep = h->entrypoint + h->delta;
    printf_log(LOG_DEBUG, "Entry Point is %p\n", (void*)ep);
    if (BOX64ENV(dump)) {
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

#ifndef STB_GNU_UNIQUE
#define STB_GNU_UNIQUE 10
#endif

void checkHookedSymbols(elfheader_t* h); // in mallochook.c
void AddSymbols32(lib_t *maplib, elfheader_t* h)
#ifndef BOX32
{ }
#else
 ;
#endif
void AddSymbols(lib_t *maplib, elfheader_t* h)
{
    if(box64_is32bits) {
        AddSymbols32(maplib, h);
    } else {
        // if(BOX64ENV(dump) && h->hash)   old_elf_hash_dump(h);
        // if(BOX64ENV(dump) && h->gnu_hash)   new_elf_hash_dump(h);
        if (BOX64ENV(dump) && h->DynSym._64) DumpDynSym64(h);
        if (h==my_context->elfs[0])
            GrabX64CopyMainElfReloc(h);
    }
    #ifndef STATICBUILD
    checkHookedSymbols(h);
    #endif
}
extern path_collection_t box64_addlibs;
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
int LoadNeededLibs(elfheader_t* h, lib_t *maplib, int local, int bindnow, int deepbind, box64context_t *box64, x64emu_t* emu)
{
    if(h->needed)   // already done
        return 0;
    DumpDynamicRPath(h);
    // update RPATH first
    for (size_t i=0; i<h->numDynamic; ++i) {
        int tag = box64_is32bits?h->Dynamic._32[i].d_tag:h->Dynamic._64[i].d_tag;
        if(tag==DT_RPATH || tag==DT_RUNPATH) {
            char *rpathref = h->DynStrTab+h->delta+(box64_is32bits?h->Dynamic._32[i].d_un.d_val:h->Dynamic._64[i].d_un.d_val);
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
                printf_log(LOG_INFO, "Warning, RPATH with $ variable not supported yet (%s)\n", rpath);
            } else {
                printf_log(LOG_DEBUG, "Prepending path \"%s\" to BOX64_LD_LIBRARY_PATH\n", rpath);
                PrependList(&box64->box64_ld_lib, rpath, 1);
            }
            if(rpath!=rpathref)
                box_free(rpath);
        }
    }
    DumpDynamicNeeded(h);
    int cnt = 0;
    // count the number of needed libs, and also grab soname
    for (size_t i=0; i<h->numDynamic; ++i) {
        int tag = box64_is32bits?h->Dynamic._32[i].d_tag:h->Dynamic._64[i].d_tag;
        if(tag==DT_NEEDED)
            ++cnt;
        if(tag==DT_SONAME)
            h->soname = h->DynStrTab+h->delta+(box64_is32bits?h->Dynamic._32[i].d_un.d_val:h->Dynamic._64[i].d_un.d_val);
    }
    #ifndef ANDROID
    if(box64_is32bits && strstr(h->name, "lsteamclient.dll.so"))
        ++cnt;
    #endif
    h->needed = new_neededlib(cnt);
    if(h == my_context->elfs[0])
        my_context->neededlibs = h->needed;
    int j=0;
    for (size_t i=0; i<h->numDynamic; ++i)
        if((box64_is32bits?h->Dynamic._32[i].d_tag:h->Dynamic._64[i].d_tag)==DT_NEEDED)
            h->needed->names[j++] = h->DynStrTab+h->delta+(box64_is32bits?h->Dynamic._32[i].d_un.d_val:h->Dynamic._64[i].d_un.d_val);
    #ifndef ANDROID
    if(box64_is32bits && strstr(h->name, "lsteamclient.dll.so"))
        h->needed->names[j++] = "libgcc_s.so.1";
    #endif
    if(h==my_context->elfs[0] && box64_addlibs.size) {
        for(int i=0; i<box64_addlibs.size; ++i) {
            printf_log(LOG_INFO, "BOX64, Adding %s to needed libs of %s\n", box64_addlibs.paths[i], h->name);
            add1lib_neededlib_name(h->needed, NULL, box64_addlibs.paths[i]);
        }
    }
    // TODO: Add LD_LIBRARY_PATH and RPATH handling
    if(AddNeededLib(maplib, local, bindnow, deepbind, h->needed, h, box64, emu)) {
        printf_log(LOG_INFO, "Error loading one of needed lib\n");
        if(!BOX64ENV(allow_missing_libs))
            return 1;   //error...
    }
    return 0;
}

needed_libs_t* GetELfNeededLibs(elfheader_t* h)
{
    return h?h->needed:NULL;
}

int ElfCheckIfUseTCMallocMinimal(elfheader_t* h)
{
    if(!h)
        return 0;
    for (size_t i=0; i<h->numDynamic; ++i)
        if((box64_is32bits?h->Dynamic._32[i].d_tag:h->Dynamic._64[i].d_tag)==DT_NEEDED) {
            char *needed = h->DynStrTab+h->delta+(box64_is32bits?h->Dynamic._32[i].d_un.d_val:h->Dynamic._64[i].d_un.d_val);
            if(!strcmp(needed, "libtcmalloc_minimal.so.4")) // tcmalloc needs to be the 1st lib loaded
                return 1;
            else if(!strcmp(needed, "libtcmalloc_minimal.so.0")) // tcmalloc needs to be the 1st lib loaded
                return 1;
            else
                return 0;
        }
    return 0;
}

void RefreshElfTLS(elfheader_t* h, x64emu_t* emu)
{
    refreshTLSData(emu);
    if(h->tlsfilesize) {
        char* dest = (char*)(my_context->tlsdata+my_context->tlssize+h->tlsbase);
        printf_dump(LOG_DEBUG, "Refreshing main TLS block @%p from %p:0x%lx\n", dest, (void*)h->tlsaddr, h->tlsfilesize);
        memcpy(dest, (void*)(h->tlsaddr+h->delta), h->tlsfilesize);
        if (emu->tlsdata) {
            tlsdatasize_t* ptr = emu->tlsdata;
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
#ifndef STATICBUILD
void startMallocHook();
#else
void startMallocHook() {}
#endif
void RunElfInit(elfheader_t* h, x64emu_t *emu)
{
    if(!h || h->init_done)
        return;
    // reset Segs Cache
    uintptr_t p = h->initentry + h->delta;
    // Refresh no-file part of TLS in case default value changed
    RefreshElfTLS(h, emu);
    // check if in deferredInit
    if(my_context->deferredInit) {
        if(my_context->deferredInitSz==my_context->deferredInitCap) {
            my_context->deferredInitCap += 4;
            my_context->deferredInitList = (elfheader_t**)box_realloc(my_context->deferredInitList, my_context->deferredInitCap*sizeof(elfheader_t*));
        }
        my_context->deferredInitList[my_context->deferredInitSz++] = h;
        return;
    }
    h->init_done = 1;
    if(h->needed)
        for(int i=0; i<h->needed->init_size; ++i) {
            library_t *lib = h->needed->libs[i];
            elfheader_t *lib_elf = GetElf(lib);
            if(lib_elf)
                RunElfInit(lib_elf, emu);
        }
    printf_dump(LOG_DEBUG, "Calling Init for %s @%p\n", ElfName(h), (void*)p);
    if(h->initentry) {
        if(box64_is32bits)
            RunFunctionWithEmu(emu, 0, p, 3, my_context->argc, my_context->argv32, my_context->envv32);
        else
            RunFunctionWithEmu(emu, 0, p, 3, my_context->argc, my_context->argv, my_context->envv);
    }
    printf_dump(LOG_DEBUG, "Done Init for %s\n", ElfName(h));
    // and check init array now
    #ifdef BOX32
    if(box64_is32bits) {
        Elf32_Addr *addr = (Elf32_Addr*)(h->initarray + h->delta);
        for (size_t i=0; i<h->initarray_sz; ++i) {
            if(addr[i]) {
                printf_dump(LOG_DEBUG, "Calling Init[%zu] for %s @%p\n", i, ElfName(h), from_ptrv(addr[i]));
                RunFunctionWithEmu(emu, 0, (uintptr_t)addr[i], 3, my_context->argc, my_context->argv32, my_context->envv32);
            }
        }
    } else
    #endif
    {
        Elf64_Addr *addr = (Elf64_Addr*)(h->initarray + h->delta);
        for (size_t i=0; i<h->initarray_sz; ++i) {
            if(addr[i]) {
                printf_dump(LOG_DEBUG, "Calling Init[%zu] for %s @%p\n", i, ElfName(h), (void*)addr[i]);
                RunFunctionWithEmu(emu, 0, (uintptr_t)addr[i], 3, my_context->argc, my_context->argv, my_context->envv);
            }
        }
    }

    if(h->malloc_hook_2)
        startMallocHook();

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
    // Call the registered cxa_atexit functions
    CallCleanup(emu, h);
#ifdef ANDROID
    // TODO: Fix .fini_array on Android
    printf_log(LOG_DEBUG, "Android does not support Fini for %s\n", ElfName(h));
#else
    // first check fini array
    #ifdef BOX32
    if(box64_is32bits) {
        Elf32_Addr *addr = (Elf32_Addr*)(h->finiarray + h->delta);
        for (int i=h->finiarray_sz-1; i>=0; --i) {
            printf_dump(LOG_DEBUG, "Calling Fini[%d] for %s @%p\n", i, ElfName(h), from_ptrv(addr[i]));
            RunFunctionWithEmu(emu, 0, (uintptr_t)addr[i], 0);
        }
    } else
    #endif
    {
        Elf64_Addr *addr = (Elf64_Addr*)(h->finiarray + h->delta);
        for (int i=h->finiarray_sz-1; i>=0; --i) {
            printf_dump(LOG_DEBUG, "Calling Fini[%d] for %s @%p\n", i, ElfName(h), (void*)addr[i]);
            RunFunctionWithEmu(emu, 0, (uintptr_t)addr[i], 0);
        }
    }
    // then the "old-style" fini
    if(h->finientry) {
        uintptr_t p = h->finientry + h->delta;
        printf_dump(LOG_DEBUG, "Calling Fini for %s @%p\n", ElfName(h), (void*)p);
        RunFunctionWithEmu(emu, 0, p, 0);
    }
    h->init_done = 0;   // can be re-inited again...
    return;
#endif
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
    return h->image;
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
        uintptr_t base = (uintptr_t)h->multiblocks[i].p;
        uintptr_t end = (uintptr_t)h->multiblocks[i].p + h->multiblocks[i].asize - 1;
        if(base && addr>=base && addr<=end)
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
    if((uintptr_t)p<0x10000)
        return ret;
    if(!h) {
        if(getProtection((uintptr_t)p)&(PROT_READ)) {
            uintptr_t adj_p = ((uintptr_t)p)&~(sizeof(onebridge_t)-1);
            if (*(uint8_t*)(adj_p) == 0xCC && IsBridgeSignature(*(uint8_t*)(adj_p + 1), *(uint8_t*)(adj_p + 2))) {
                ret = getBridgeName((void*)adj_p);
                if(ret) {
                    if(start)
                        *start = (uintptr_t)adj_p;
                    if(sz)
                        *sz = sizeof(onebridge_t);
                }
            }
        }
        if(!ret) {
            const char* filename = NULL;
            uintptr_t start_map = 0;
            if(IsAddrFileMapped(addr, &filename, &start_map)) {
                if(start) *start = start_map;
                if(sz) *sz = SizeFileMapped(addr);
                ret = filename;
            }
        }
        return ret;
    }
    if(!h || h->fini_done)
        return ret;

    for (size_t i=0; i<h->numSymTab && distance!=0; ++i) {
        const char * symname = box64_is32bits?(h->StrTab+h->SymTab._32[i].st_name):(h->StrTab+h->SymTab._64[i].st_name);
        uintptr_t offs = (box64_is32bits?h->SymTab._32[i].st_value:h->SymTab._64[i].st_value) + h->delta;

        if(offs<=addr) {
            if(distance>addr-offs) {
                distance = addr-offs;
                ret = symname;
                s = offs;
                size = box64_is32bits?h->SymTab._32[i].st_size:h->SymTab._64[i].st_size;
            }
        }
    }
    for (size_t i=0; i<h->numDynSym && distance!=0; ++i) {
        const char * symname = h->DynStr+(box64_is32bits?h->DynSym._32[i].st_name:h->DynSym._64[i].st_name);
        uintptr_t offs = (box64_is32bits?h->DynSym._32[i].st_value:h->DynSym._64[i].st_value) + h->delta;

        if(offs<=addr) {
            if(distance>addr-offs) {
                distance = addr-offs;
                ret = symname;
                s = offs;
                size = box64_is32bits?h->DynSym._32[i].st_size:h->DynSym._64[i].st_size;
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

int SameVersionedSymbol(const char* name1, int ver1, const char* vername1, int veropt1, const char* name2, int ver2, const char* vername2, int veropt2)
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
    if(ver1<2 && ver2>1 && veropt2)
        return 1;
    if(ver2<2 && ver1>1 && veropt1)
        return 1;
    if(!strcmp(vername1, vername2))  // same vername
        return 1;
    return 0;
}

void* GetDTatOffset(x64emu_t* emu, unsigned long int index, unsigned long int offset)
{
    return (void*)((char*)GetTLSPointer(emu, emu->context->elfs[index])+offset);
}

int32_t GetTLSBase(elfheader_t* h)
{
    return h?h->tlsbase:0;
}

uint32_t GetTLSSize(elfheader_t* h)
{
    return h?h->tlssize:0;
}

void* GetTLSPointer(x64emu_t* emu, elfheader_t* h)
{
    if(!h || !h->tlssize)
        return NULL;
    refreshTLSData(emu);    // needed?
    tlsdatasize_t* ptr = emu->tlsdata;
    return ptr->data+h->tlsbase;
}

void* GetDynamicSection(elfheader_t* h)
{
    if(!h)
        return NULL;
    return box64_is32bits?((void*)h->Dynamic._32):((void*)h->Dynamic._64);
}

typedef struct {
    void*   addr;
    size_t  size;
} dynamic_info_t;

static int GetLoadedDynamicInfo(elfheader_t* h, dynamic_info_t* info)
{
    if(!h) return 0;
    if(box64_is32bits) {
        for(int i = 0; i < h->numPHEntries; i++) {
            if(h->PHEntries._32[i].p_type == PT_DYNAMIC) {
                info->addr = (void*)(h->delta + h->PHEntries._32[i].p_vaddr);
                info->size = h->PHEntries._32[i].p_memsz;
                return 1;
            }
        }
    } else {
        for(int i = 0; i < h->numPHEntries; i++) {
            if(h->PHEntries._64[i].p_type == PT_DYNAMIC) {
                info->addr = (void*)(h->delta + h->PHEntries._64[i].p_vaddr);
                info->size = h->PHEntries._64[i].p_memsz;
                return 1;
            }
        }
    }
    return 0;
}

void* GetLoadedDynamicSection(elfheader_t* h)
{
    dynamic_info_t info;
    if(GetLoadedDynamicInfo(h, &info))
        return info.addr;
    return NULL;
}

static int isDynamicTagPointer(int tag)
{
    switch(tag) {
        case DT_PLTGOT:
        case DT_HASH:
        case DT_STRTAB:
        case DT_SYMTAB:
        case DT_RELA:
        case DT_REL:
        case DT_RELR:
        case DT_DEBUG:
        case DT_JMPREL:
        case DT_INIT:
        case DT_FINI:
        case DT_INIT_ARRAY:
        case DT_FINI_ARRAY:
        case DT_PREINIT_ARRAY:
        case DT_VERNEED:
        case DT_VERDEF:
        case DT_VERSYM:
#ifdef DT_GNU_HASH
        case DT_GNU_HASH:
#else
        case 0x6ffffef5:
#endif
#ifdef DT_TLSDESC_PLT
        case DT_TLSDESC_PLT:
#else
        case 0x6ffffef6:
#endif
#ifdef DT_TLSDESC_GOT
        case DT_TLSDESC_GOT:
#else
        case 0x6ffffef7:
#endif
            return 1;
        default:
            return 0;
    }
}

void PatchLoadedDynamicSection(elfheader_t* h)
{
    if(!h || !h->delta || h->dynamic_patched)
        return;

    dynamic_info_t dyninfo;
    if(!GetLoadedDynamicInfo(h, &dyninfo))
        return;

    uintptr_t dyn_addr = (uintptr_t)dyninfo.addr;
    uintptr_t dyn_size = dyninfo.size;

    uintptr_t page_addr = dyn_addr & ~(box64_pagesize - 1);
    uintptr_t page_end = (dyn_addr + dyn_size + box64_pagesize - 1) & ~(box64_pagesize - 1);

    int need_restore = 0;
    for(uintptr_t page = page_addr; page < page_end; page += box64_pagesize) {
        uint32_t old_prot = getProtection(page);
        if(old_prot && !(old_prot & PROT_WRITE)) {
            if(mprotect((void*)page, box64_pagesize, (old_prot | PROT_WRITE) & ~PROT_CUSTOM)) {
                for(uintptr_t restore = page_addr; restore < page; restore += box64_pagesize) {
                    uint32_t restore_prot = getProtection(restore);
                    if(restore_prot && !(restore_prot & PROT_WRITE))
                        mprotect((void*)restore, box64_pagesize, restore_prot & ~PROT_CUSTOM);
                }
                return;
            }
            need_restore = 1;
        }
    }

    if(box64_is32bits) {
        Elf32_Dyn* dyn = (Elf32_Dyn*)dyn_addr;
        for(int j = 0; dyn[j].d_tag != DT_NULL; j++) {
            if(isDynamicTagPointer(dyn[j].d_tag) && dyn[j].d_un.d_ptr) {
                dyn[j].d_un.d_ptr += h->delta;
            }
        }
    } else {
        Elf64_Dyn* dyn = (Elf64_Dyn*)dyn_addr;
        for(int j = 0; dyn[j].d_tag != DT_NULL; j++) {
            if(isDynamicTagPointer(dyn[j].d_tag) && dyn[j].d_un.d_ptr) {
                dyn[j].d_un.d_ptr += h->delta;
            }
        }
    }

    if(need_restore) {
        for(uintptr_t page = page_addr; page < page_end; page += box64_pagesize) {
            uint32_t old_prot = getProtection(page);
            if(old_prot && !(old_prot & PROT_WRITE))
                mprotect((void*)page, box64_pagesize, old_prot & ~PROT_CUSTOM);
        }
    }

    h->dynamic_patched = 1;
}

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
    return (int)RunFunction(my_dl_iterate_phdr_fct_##A, 3, a, b, c);                \
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

EXPORT int my32_dl_iterate_phdr(x64emu_t *emu, void* F, void *data)
#ifdef BOX32
;
#else
{
    return -1;
}
#endif
EXPORT int my_dl_iterate_phdr(x64emu_t *emu, void* F, void *data) {
    if(box64_is32bits) return my32_dl_iterate_phdr(emu, F, data);
    printf_log(LOG_DEBUG, "Call to partially implemented dl_iterate_phdr(%p, %p)\n", F, data);
    box64context_t *context = GetEmuContext(emu);
    const char* empty = "";
    int ret = 0;
    for (int idx=0; idx<context->elfsize; ++idx) {
        if(context->elfs[idx]) {
            my_dl_phdr_info_t info;
            info.dlpi_addr = GetElfDelta(context->elfs[idx]);
            info.dlpi_name = idx?context->elfs[idx]->name:empty;    //1st elf is program, and this one doesn't get a name
            info.dlpi_phdr = context->elfs[idx]->PHEntries._64;
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

void ResetSpecialCaseMainElf32(elfheader_t* h)
#ifndef BOX32
{ }
#else
 ;
#endif
void ResetSpecialCaseMainElf(elfheader_t* h)
{
    if(box64_is32bits) {
        ResetSpecialCaseMainElf32(h);
        return;
    }
    Elf64_Sym *sym = NULL;
    for (size_t i=0; i<h->numDynSym; ++i) {
        if(h->DynSym._64[i].st_info == 17) {
            sym = h->DynSym._64+i;
            const char * symname = h->DynStr+sym->st_name;
            if(strcmp(symname, "_IO_2_1_stderr_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stderr, sym->st_size);
                my__IO_2_1_stderr_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "Set @_IO_2_1_stderr_ to %p\n", my__IO_2_1_stderr_);
            } else
            if(strcmp(symname, "_IO_2_1_stdin_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stdin, sym->st_size);
                my__IO_2_1_stdin_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "Set @_IO_2_1_stdin_ to %p\n", my__IO_2_1_stdin_);
            } else
            if(strcmp(symname, "_IO_2_1_stdout_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stdout, sym->st_size);
                my__IO_2_1_stdout_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "Set @_IO_2_1_stdout_ to %p\n", my__IO_2_1_stdout_);
            } else
            if(strcmp(symname, "_IO_stderr_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stderr, sym->st_size);
                my__IO_2_1_stderr_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "Set @_IO_stderr_ to %p\n", my__IO_2_1_stderr_);
            } else
            if(strcmp(symname, "_IO_stdin_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stdin, sym->st_size);
                my__IO_2_1_stdin_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "Set @_IO_stdin_ to %p\n", my__IO_2_1_stdin_);
            } else
            if(strcmp(symname, "_IO_stdout_")==0 && ((void*)sym->st_value+h->delta)) {
                memcpy((void*)sym->st_value+h->delta, stdout, sym->st_size);
                my__IO_2_1_stdout_ = (void*)sym->st_value+h->delta;
                printf_log(LOG_DEBUG, "Set @_IO_stdout_ to %p\n", my__IO_2_1_stdout_);
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

static Elf64_Sym* ElfLocateSymbol(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    Elf64_Sym* sym = ElfLookup64(head, symname, *ver, *vername, local, *veropt);
    if(!sym) return NULL;
    if(head->VerSym && !*veropt) {
        int idx = ((uintptr_t)sym - (uintptr_t)head->DynSym._64)/sizeof(Elf64_Sym);
        int version = ((Elf64_Half*)((uintptr_t)head->VerSym+head->delta))[idx];
        if(version!=-1) version &=0x7fff;
        const char* symvername = GetSymbolVersion(head, version);
        Elf64_Half flags = GetSymbolVersionFlag(head, version);
        if(version>1 && *ver<2 && (flags==0)) {
            *ver = version;
            *vername = symvername;
            *veropt = 1;
        } else if(flags==0 && !*veropt && version>1 && *ver>1 && !strcmp(symvername, *vername)) {
            *veropt = 1;
        }
    }
    if(!sym->st_shndx) return NULL;
    int vis = ELF64_ST_VISIBILITY(sym->st_other);
    if(vis==STV_HIDDEN && !local)
        return NULL;
    return sym;
}

void* ElfGetLocalSymbolStartEnd32(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
#ifndef BOX32
{ return NULL; }
#else
 ;
#endif
void* ElfGetLocalSymbolStartEnd64(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    Elf64_Sym* sym = ElfLocateSymbol(head, offs, end, symname, ver, vername, local, veropt);
    if(!sym) return NULL;
    int bind = ELF64_ST_BIND(sym->st_info);
    if(bind!=STB_LOCAL) return 0;
    if(offs) *offs = sym->st_value + head->delta;
    if(end) *end = sym->st_value + head->delta + sym->st_size;
    return sym;
}
void* ElfGetLocalSymbolStartEnd(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    return box64_is32bits?ElfGetLocalSymbolStartEnd32(head, offs, end, symname, ver, vername, local, veropt):ElfGetLocalSymbolStartEnd64(head, offs, end, symname, ver, vername, local, veropt);
}

void* ElfGetGlobalSymbolStartEnd32(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
#ifndef BOX32
{ return NULL; }
#else
 ;
#endif
void* ElfGetGlobalSymbolStartEnd64(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    Elf64_Sym* sym = ElfLocateSymbol(head, offs, end, symname, ver, vername, local, veropt);
    if(!sym) return NULL;
    int bind = ELF64_ST_BIND(sym->st_info);
    if(bind!=STB_GLOBAL && bind!=STB_GNU_UNIQUE) return 0;
    if(offs) *offs = sym->st_value + head->delta;
    if(end) *end = sym->st_value + head->delta + sym->st_size;
    return sym;
}
void* ElfGetGlobalSymbolStartEnd(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    return box64_is32bits?ElfGetGlobalSymbolStartEnd32(head, offs, end, symname, ver, vername, local, veropt):ElfGetGlobalSymbolStartEnd64(head, offs, end, symname, ver, vername, local, veropt);
}

void* ElfGetWeakSymbolStartEnd32(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
#ifndef BOX32
{ return NULL; }
#else
 ;
#endif
void* ElfGetWeakSymbolStartEnd64(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    Elf64_Sym* sym = ElfLocateSymbol(head, offs, end, symname, ver, vername, local, veropt);
    if(!sym) return NULL;
    int bind = ELF64_ST_BIND(sym->st_info);
    if(bind!=STB_WEAK) return 0;
    if(offs) *offs = sym->st_value + head->delta;
    if(end) *end = sym->st_value + head->delta + sym->st_size;
    return sym;
}
void* ElfGetWeakSymbolStartEnd(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    return box64_is32bits?ElfGetWeakSymbolStartEnd32(head, offs, end, symname, ver, vername, local, veropt):ElfGetWeakSymbolStartEnd64(head, offs, end, symname, ver, vername, local, veropt);
}

void* ElfGetSymbolStartEnd32(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
#ifndef BOX32
{ return NULL; }
#else
 ;
#endif
void* ElfGetSymbolStartEnd64(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    Elf64_Sym* sym = ElfLocateSymbol(head, offs, end, symname, ver, vername, local, veropt);
    if(!sym) return NULL;
    if(offs) *offs = sym->st_value + head->delta;
    if(end) *end = sym->st_value + head->delta + sym->st_size;
    return sym;
}
void* ElfGetSymbolStartEnd(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname, int* ver, const char** vername, int local, int* veropt)
{
    return box64_is32bits?ElfGetSymbolStartEnd32(head, offs, end, symname, ver, vername, local, veropt):ElfGetSymbolStartEnd64(head, offs, end, symname, ver, vername, local, veropt);
}

int ElfGetSymTabStartEnd32(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname)
#ifndef BOX32
{ return 0; }
#else
 ;
#endif
int ElfGetSymTabStartEnd64(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname)
{
    Elf64_Sym* sym = ElfSymTabLookup64(head, symname);
    if(!sym) return 0;
    if(!sym->st_shndx) return 0;
    if(!sym->st_size) return 0; //needed?
    if(offs) *offs = sym->st_value + head->delta;
    if(end) *end = sym->st_value + head->delta + sym->st_size;
    return 1;
}
int ElfGetSymTabStartEnd(elfheader_t* head, uintptr_t *offs, uintptr_t *end, const char* symname)
{
    return box64_is32bits?ElfGetSymTabStartEnd32(head, offs, end, symname):ElfGetSymTabStartEnd64(head, offs, end, symname);
}

int NeededLibs(elfheader_t* h)
{
    if(!h) return 0;
    int cnt = 0;
    // count the number of needed libs, and also grab soname
    for (size_t i=0; i<h->numDynamic; ++i) {
        int tag = box64_is32bits?h->Dynamic._32[i].d_tag:h->Dynamic._64[i].d_tag;
        if(tag==DT_NEEDED)
            ++cnt;
    }
    return cnt;
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

EXPORT void PltResolver64(x64emu_t* emu)
{
    uintptr_t addr = Pop64(emu);
    int slot = (int)Pop64(emu);
    elfheader_t *h = (elfheader_t*)addr;
    library_t* lib = h->lib;
    lib_t* local_maplib = GetMaplib(lib);
    int deepbind = GetDeepBind(lib);
    printf_dump(LOG_DEBUG, "PltResolver64: Addr=%p, Slot=%d Return=%p(%s): elf is %s (VerSym=%p, deepbind=%d, local_maplib=%p) func param: %p, %p...\n", (void*)addr, slot, *(void**)(R_RSP), getAddrFunctionName(*(uintptr_t*)R_RSP),h->name, h->VerSym, deepbind, local_maplib, (void*)R_RDI, (void*)R_RSI);

    Elf64_Rela * rel = (Elf64_Rela *)(h->jmprel + h->delta) + slot;

    Elf64_Sym *sym = &h->DynSym._64[ELF64_R_SYM(rel->r_info)];
    int bind = ELF64_ST_BIND(sym->st_info);
    const char* symname = SymName64(h, sym);
    int version = h->VerSym?((Elf64_Half*)((uintptr_t)h->VerSym+h->delta))[ELF64_R_SYM(rel->r_info)]:-1;
    if(version!=-1) version &= 0x7fff;
    const char* vername = GetSymbolVersion(h, version);
    Elf64_Half flags = GetSymbolVersionFlag(h, version);
    int veropt = flags?0:1;
    uint64_t *p = (uint64_t*)(rel->r_offset + h->delta);
    uintptr_t offs = 0;
    uintptr_t end = 0;

    Elf64_Sym *elfsym = NULL;
    if(bind==STB_LOCAL) {
        elfsym = ElfDynSymLookup64(h, symname);
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
        printf_log(LOG_NONE, "Error: PltResolver: Symbol %s %s(%sver %d: %s%s%s) not found, cannot apply R_X86_64_JUMP_SLOT %p in %s (local_maplib=%p, global maplib=%p, deepbind=%d)\n", (bind==STB_LOCAL)?"Local":((bind==STB_WEAK)?"Weak":""), symname, veropt?"opt":"", version, symname, vername?"@":"", vername?vername:"", p, (h && h->name)?h->name:"???", local_maplib, my_context->maplib, deepbind);
        emu->quit = 1;
        R_RIP = 0; //stop....
        return;
    } else {
        elfheader_t* sym_elf = FindElfSymbol(my_context, elfsym);
        if(elfsym && (elfsym->st_info&0xf)==STT_GNU_IFUNC) {
            // this is an IFUNC, needs to evaluate the function first!
            printf_dump(LOG_DEBUG, "            Indirect function, will call the resolver now at %p\n", (void*)offs);
            offs = RunFunction(offs, 0);
        }
        offs = (uintptr_t)getAlternate((void*)offs);

        if(p) {
            printf_dump(LOG_DEBUG, "            Apply %s R_X86_64_JUMP_SLOT %p with sym=%s(%sver %d: %s%s%s) (%p -> %p / %s)\n", BindSym(bind), p, symname, veropt?"opt":"", version, symname, vername?"@":"", vername?vername:"",*(void**)p, (void*)offs, ElfName(sym_elf));
            *p = offs;
            if(sym_elf && sym_elf!=h) checkElfLib(h, sym_elf->lib);
        } else {
            printf_log(LOG_NONE, "PltResolver: Warning, Symbol %s(%sver %d: %s%s%s) found, but Jump Slot Offset is NULL \n", symname, veropt?"opt":"", version, symname, vername?"@":"", vername?vername:"");
        }
    }

    // jmp to function
    R_RIP = offs;
}

const char* getAddrFunctionName(uintptr_t addr)
{
    static char rets[8][1000];
    static int idx = 0;
    char* ret = rets[idx];
    idx = (idx + 1) & 7;
    uint64_t sz = 0;
    uintptr_t start = 0;
    elfheader_t* elf = FindElfAddress(my_context, addr);
    const char* symbname = FindNearestSymbolName(elf, (void*)addr, &start, &sz);
    if (!sz) sz = 0x100; // arbitrary value...
    if (symbname && (addr >= start) && (addr < (start + sz))) {
        if (symbname[0] == '\0')
            sprintf(ret, "%s + 0x%lx + 0x%lx", ElfName(elf), start - (uintptr_t)GetBaseAddress(elf), addr - start);
        else if (addr == start)
            sprintf(ret, "%s/%s", ElfName(elf), symbname);
        else
            sprintf(ret, "%s/%s + 0x%lx", ElfName(elf), symbname, addr - start);
    } else {
        if (elf) {
            sprintf(ret, "%s + 0x%lx", ElfName(elf), addr - (uintptr_t)GetBaseAddress(elf));
        } else if(IsAddrFileMapped(addr, &symbname, &start)) {
            sprintf(ret, "%s+0x%lx", symbname, addr-start);
        } else {
            sprintf(ret, "???");
        }
    }
    return ret;
}
