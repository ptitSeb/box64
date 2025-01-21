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

#include "custommem.h"
#include "box64version.h"
#include "elfloader.h"
#include "debug.h"
#include "elfload_dump.h"
#include "elfloader_private.h"

static int SymbolMatch(elfheader_t* h, uint32_t i, int ver, const char* vername, int local, int veropt)
{
    int version = h->VerSym?((Elf32_Half*)((uintptr_t)h->VerSym+h->delta))[i]:-1;
    if(version!=-1) version &=0x7fff;
    const char* symvername = GetSymbolVersion(h, version);
    Elf32_Half flags = GetSymbolVersionFlag(h, version);
    if(ver==-1 || version==-1)
        return 1;
    if(version==0 && !local)
        return 0;
    if(version<2 && ver>1 && veropt)
        return 1;
    if(ver==0 && version<2)
        return 1;
    if(ver==1 && version<2)
        return 1;
    if(ver<2 && version>1 && flags==0)  // flag is not WEAK, so global works
        return 1;
    if(ver<2 || !symvername)
        return 0;
    return strcmp(vername, symvername)?0:1;
}

uint16_t GetParentSymbolVersionFlag32(elfheader_t* h, int index)
{
    if(!h->VerDef._32 || (index<1))
        return (uint16_t)-1;
    Elf32_Verdef *def = (Elf32_Verdef*)((uintptr_t)h->VerDef._32 + h->delta);
    while(def) {
        if(def->vd_ndx==index) {
            return def->vd_flags;
        }
        def = def->vd_next?((Elf32_Verdef*)((uintptr_t)def + def->vd_next)):NULL;
    }
    return (uint16_t)-1;
}

uint16_t GetSymbolVersionFlag32(elfheader_t* h, int version)
{
    if(version<2)
        return (uint16_t)-1;
    if(h->VerNeed._32) {
        Elf32_Verneed *ver = (Elf32_Verneed*)((uintptr_t)h->VerNeed._32 + h->delta);
        while(ver) {
            Elf32_Vernaux *aux = (Elf32_Vernaux*)((uintptr_t)ver + ver->vn_aux);
            for(int j=0; j<ver->vn_cnt; ++j) {
                if(aux->vna_other==version) 
                    return aux->vna_flags;
                aux = (Elf32_Vernaux*)((uintptr_t)aux + aux->vna_next);
            }
            ver = ver->vn_next?((Elf32_Verneed*)((uintptr_t)ver + ver->vn_next)):NULL;
        }
    }
    return GetParentSymbolVersionFlag32(h, version);  // if symbol is "internal", use Def table instead
}


static Elf32_Sym* old_elf_lookup(elfheader_t* h, const char* symname, int ver, const char* vername, int local, int veropt)
{
    // Prepare hash table
    const uint32_t *hashtab = (uint32_t*)(h->hash + h->delta);
    const uint32_t nbuckets = hashtab[0];
    const uint32_t nchains = hashtab[1];
    const uint32_t *buckets = &hashtab[2];
    const uint32_t *chains = &buckets[nbuckets];
    // get hash from symname to lookup
    const uint32_t hash = old_elf_hash(symname);
    // Search for it
    for (uint32_t i = buckets[hash % nbuckets]; i; i = chains[i]) {
        const char* name = h->DynStr + h->DynSym._32[i].st_name;
        if (!strcmp(symname, name) && SymbolMatch(h, i, ver, vername, local, veropt)) {
            return &h->DynSym._32[i];
        }
    }
    return NULL;
}


static void old_elf_hash_dump(elfheader_t* h)
{
    // Prepare hash table
    const uint32_t *hashtab = (uint32_t*)(h->hash + h->delta);
    const uint32_t nbuckets = hashtab[0];
    const uint32_t nchains = hashtab[1];
    const uint32_t *buckets = &hashtab[2];
    const uint32_t *chains = &buckets[nbuckets];
    printf_log(LOG_NONE, "------------ Dump HASH from %s\n", h->name);
    printf_log(LOG_NONE, "Buckets[%d] = \n", nbuckets);
    for(uint32_t i=0; i<nbuckets; ++i) {
        const char* name = h->DynStr + h->DynSym._32[buckets[i]].st_name;
        printf_log(LOG_NONE, "%d: %s\n", buckets[i], name);
    }
    printf_log(LOG_NONE,"Chains[%d] = ", nchains);
    for (uint32_t i = 0; i<nchains; ++i)
        printf_log_prefix(0, LOG_NONE, "%d ", chains[i]);
    printf_log(LOG_NONE, "\n------------\n");
}

static Elf32_Sym* new_elf_lookup(elfheader_t* h, const char* symname, int ver, const char* vername, int local, int veropt)
{
    // Prepare hash table
    const uint32_t *hashtab = (uint32_t*)(h->gnu_hash + h->delta);
    const uint32_t nbuckets = hashtab[0];
    const uint32_t symoffset = hashtab[1];
    const uint32_t bloom_size = hashtab[2];
    const uint32_t bloom_shift = hashtab[3];
    const uint32_t *blooms = (uint32_t*)&hashtab[4];
    const uint32_t *buckets = (uint32_t*)&blooms[bloom_size];
    const uint32_t *chains = &buckets[nbuckets];
    // get hash from symname to lookup
    const uint32_t hash = new_elf_hash(symname);
    // early check with bloom: if at least one bit is not set, a symbol is surely missing.
    uint32_t word = blooms[(hash/32)%bloom_size];
    uint32_t mask = 0
        | 1LL << (hash%32)
        | 1LL << ((hash>>bloom_shift)%32);
    if ((word & mask) != mask) {
        return NULL;
    }
    // now look at the bucket chain for the symbol
    uint32_t symidx = buckets[hash%nbuckets];
    if (symidx < symoffset)
        return NULL;
    while(1) {
        const char* name = h->DynStr + h->DynSym._32[symidx].st_name;
        const uint32_t symhash = chains[symidx-symoffset];
        if ((hash|1) == (symhash|1) && !strcmp(name, symname) && SymbolMatch(h, symidx, ver, vername, local, veropt)) {
            return &h->DynSym._32[symidx];
        }
        if(symhash&1)
            return NULL;
        symidx++;
    }
}

static void new_elf_hash_dump(elfheader_t* h)
{
    // Prepare hash table
    const uint32_t *hashtab = (uint32_t*)(h->gnu_hash + h->delta);
    const uint32_t nbuckets = hashtab[0];
    const uint32_t symoffset = hashtab[1];
    const uint32_t bloom_size = hashtab[2];
    const uint32_t bloom_shift = hashtab[3];
    const uint32_t *blooms = (uint32_t*)&hashtab[4];
    const uint32_t *buckets = (uint32_t*)&blooms[bloom_size];
    const uint32_t *chains = &buckets[nbuckets];
    printf_log(LOG_NONE, "===============Dump GNU_HASH from %s\n", h->name);
    printf_log(LOG_NONE, "Bloom: size=%d, shift=%d\n", bloom_size, bloom_shift);
    printf_log(LOG_NONE, "Buckets[%d] offset=%d = \n", nbuckets, symoffset);
    for(uint32_t i=0; i<nbuckets; ++i) {
        uint32_t symidx = buckets[i];
        printf_log(LOG_NONE, "%d:", symidx);
        while(symidx>=symoffset) {
            const char* name = h->DynStr + h->DynSym._32[symidx].st_name;
            const uint32_t hash = chains[symidx-symoffset];
            if(hash&1)
                symidx=0;
            else
                symidx++;
            printf_log(LOG_NONE, " %s (%x) -> %d", name, hash, symidx);
        }
        printf_log(LOG_NONE, "\n");
    }
    printf_log(LOG_NONE, "\n===============\n");
}

Elf32_Sym* ElfLookup32(elfheader_t* h, const char* symname, int ver, const char* vername, int local, int veropt)
{
    if(h->gnu_hash)
        return new_elf_lookup(h, symname, ver, vername, local, veropt);
    return old_elf_lookup(h, symname, ver, vername, local, veropt);
}

Elf32_Sym* ElfSymTabLookup32(elfheader_t* h, const char* symname)
{
    if(!h->SymTab._32)
        return 0;
    for(size_t i=0; i<h->numSymTab; ++i) {
        Elf32_Sym* sym = &h->SymTab._32[i];
        int type = ELF32_ST_TYPE(sym->st_info);
        if(type==STT_FUNC || type==STT_TLS || type==STT_OBJECT) {
            const char * name = h->StrTab+sym->st_name;
            if(name && !strcmp(symname, name))
                return sym;
        }
    }
    return NULL;
}

Elf32_Sym* ElfDynSymLookup32(elfheader_t* h, const char* symname)
{
    if(!h->DynSym._32)
        return 0;
    for(size_t i=0; i<h->numDynSym; ++i) {
        Elf32_Sym* sym = &h->DynSym._32[i];
        int type = ELF32_ST_TYPE(sym->st_info);
        if(type==STT_FUNC || type==STT_TLS || type==STT_OBJECT) {
            const char * name = h->DynStr+sym->st_name;
            if(name && !strcmp(symname, name))
                return sym;
        }
    }
    return NULL;
}
