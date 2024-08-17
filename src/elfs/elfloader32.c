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
#include "../emu/x64run_private.h"
#include "../tools/bridge_private.h"
#include "x64tls.h"
#include "box32.h"

int AllocLoadElfMemory32(box64context_t* context, elfheader_t* head, int mainbin)
{
    ptr_t offs = 0;
    loadProtectionFromMap();
    int log_level = box64_load_addr?LOG_INFO:LOG_DEBUG;

    head->multiblock_n = 0; // count PHEntrie with LOAD
    uintptr_t max_align = (box64_pagesize-1);
    for (size_t i=0; i<head->numPHEntries; ++i) 
        if(head->PHEntries._32[i].p_type == PT_LOAD && head->PHEntries._32[i].p_flags) {
            ++head->multiblock_n;
        }

    if(!head->vaddr && box64_load_addr) {
        offs = to_ptrv(find31bitBlockNearHint((void*)box64_load_addr, head->memsz, max_align));
        box64_load_addr = offs + head->memsz;
        box64_load_addr = (box64_load_addr+0x10ffffff)&~0xffffff;
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
                    uintptr_t new_addr = paddr;
                    ssize_t new_size = asize;
                    while(getProtection(new_addr) && (new_size>0)) {
                        new_size -= ALIGN(new_addr) - new_addr;
                        new_addr = ALIGN(new_addr);
                    }
                    if(new_size>0) {
                        printf_dump(log_level, "Allocating 0x%zx (0x%zx) bytes @%p, will read 0x%zx @%p for Elf \"%s\"\n", ALIGN(new_size), e->p_memsz, (void*)new_addr, e->p_filesz, (void*)head->multiblocks[n].paddr, head->name);
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
            if(box64_dynarec && (e->p_flags & PF_X)) {
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

    // can close the elf file now!
    fclose(head->file);
    head->file = NULL;
    head->fileno = -1;

    return 0;
}
