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

const char* GetSymbolVersion32(elfheader_t* h, int version)
#ifndef BOX32
{ return NULL; }
#else
 ;
#endif
const char* GetSymbolVersion64(elfheader_t* h, int version)
{
    if(version<2)
        return NULL;
    /*if(version==1)
        return "*";*/
    if(h->VerNeed._64) {
        Elf64_Verneed *ver = (Elf64_Verneed*)((uintptr_t)h->VerNeed._64 + h->delta);
        while(ver) {
            Elf64_Vernaux *aux = (Elf64_Vernaux*)((uintptr_t)ver + ver->vn_aux);
            for(int j=0; j<ver->vn_cnt; ++j) {
                if(aux->vna_other==version) 
                    return h->DynStr+aux->vna_name;
                aux = (Elf64_Vernaux*)((uintptr_t)aux + aux->vna_next);
            }
            ver = ver->vn_next?((Elf64_Verneed*)((uintptr_t)ver + ver->vn_next)):NULL;
        }
    }
    return GetParentSymbolVersion(h, version);  // if symbol is "internal", use Def table instead
}
const char* GetSymbolVersion(elfheader_t* h, int version)
{
    return box64_is32bits?GetSymbolVersion32(h, version):GetSymbolVersion64(h, version);
}

const char* GetParentSymbolVersion32(elfheader_t* h, int index)
#ifndef BOX32
{ return NULL; }
#else
 ;
#endif
const char* GetParentSymbolVersion64(elfheader_t* h, int index)
{
    if(!h->VerDef._64 || (index<1))
        return NULL;
    Elf64_Verdef *def = (Elf64_Verdef*)((uintptr_t)h->VerDef._64 + h->delta);
    while(def) {
        if(def->vd_ndx==index) {
            if(def->vd_cnt<1)
                return NULL;
            /*if(def->vd_flags&VER_FLG_BASE)
                return NULL;*/
            Elf64_Verdaux *aux = (Elf64_Verdaux*)((uintptr_t)def + def->vd_aux);
            return h->DynStr+aux->vda_name; // return Parent, so 1st aux
        }
        def = def->vd_next?((Elf64_Verdef*)((uintptr_t)def + def->vd_next)):NULL;
    }
    return NULL;
}
const char* GetParentSymbolVersion(elfheader_t* h, int version)
{
    return box64_is32bits?GetParentSymbolVersion32(h, version):GetParentSymbolVersion64(h, version);
}

uint16_t GetParentSymbolVersionFlag32(elfheader_t* h, int index)
#ifndef BOX32
 { return (uint16_t)-1; }
#else
 ;
#endif
uint16_t GetParentSymbolVersionFlag64(elfheader_t* h, int index)
{
    if(!h->VerDef._64 || (index<1))
        return (uint16_t)-1;
    Elf64_Verdef *def = (Elf64_Verdef*)((uintptr_t)h->VerDef._64 + h->delta);
    while(def) {
        if(def->vd_ndx==index) {
            return def->vd_flags;
        }
        def = def->vd_next?((Elf64_Verdef*)((uintptr_t)def + def->vd_next)):NULL;
    }
    return (uint16_t)-1;
}
uint16_t GetParentSymbolVersionFlag(elfheader_t* h, int index)
{
    return box64_is32bits?GetParentSymbolVersionFlag32(h, index):GetParentSymbolVersionFlag64(h, index);
}

uint16_t GetSymbolVersionFlag32(elfheader_t* h, int version)
#ifndef BOX32
{ return (uint16_t)-1; }
#else
 ;
#endif
uint16_t GetSymbolVersionFlag64(elfheader_t* h, int version)
{
    if(version<2)
        return (uint16_t)-1;
    if(h->VerNeed._64) {
        Elf64_Verneed *ver = (Elf64_Verneed*)((uintptr_t)h->VerNeed._64 + h->delta);
        while(ver) {
            Elf64_Vernaux *aux = (Elf64_Vernaux*)((uintptr_t)ver + ver->vn_aux);
            for(int j=0; j<ver->vn_cnt; ++j) {
                if(aux->vna_other==version) 
                    return aux->vna_flags;
                aux = (Elf64_Vernaux*)((uintptr_t)aux + aux->vna_next);
            }
            ver = ver->vn_next?((Elf64_Verneed*)((uintptr_t)ver + ver->vn_next)):NULL;
        }
    }
    return GetParentSymbolVersionFlag(h, version);  // if symbol is "internal", use Def table instead
}
uint16_t GetSymbolVersionFlag(elfheader_t* h, int index) {
    return box64_is32bits?GetSymbolVersionFlag32(h, index):GetSymbolVersionFlag64(h, index);
}


int GetVersionIndice32(elfheader_t* h, const char* vername)
#ifndef BOX32
{ return 0; }
#else
 ;
#endif
int GetVersionIndice64(elfheader_t* h, const char* vername)
{
    if(!vername)
        return 0;
    if(h->VerDef._64) {
        Elf64_Verdef *def = (Elf64_Verdef*)((uintptr_t)h->VerDef._64 + h->delta);
        while(def) {
            Elf64_Verdaux *aux = (Elf64_Verdaux*)((uintptr_t)def + def->vd_aux);
            if(!strcmp(h->DynStr+aux->vda_name, vername))
                return def->vd_ndx;
            def = def->vd_next?((Elf64_Verdef*)((uintptr_t)def + def->vd_next)):NULL;
        }
    }
    return 0;
}
int GetVersionIndice(elfheader_t* h, const char* vername)
{
    return box64_is32bits?GetVersionIndice32(h, vername):GetVersionIndice64(h, vername);
}

int GetNeededVersionCnt32(elfheader_t* h, const char* libname)
#ifndef BOX32
{ return 0; }
#else
 ;
#endif
int GetNeededVersionCnt64(elfheader_t* h, const char* libname)
{
    if(!libname)
        return 0;
    if(h->VerNeed._64) {
        Elf64_Verneed *ver = (Elf64_Verneed*)((uintptr_t)h->VerNeed._64 + h->delta);
        while(ver) {
            char *filename = h->DynStr + ver->vn_file;
            if(!strcmp(filename, libname))
                return ver->vn_cnt;
            ver = ver->vn_next?((Elf64_Verneed*)((uintptr_t)ver + ver->vn_next)):NULL;
        }
    }
    return 0;
}
int GetNeededVersionCnt(elfheader_t* h, const char* libname)
{
    return box64_is32bits?GetNeededVersionCnt32(h, libname):GetNeededVersionCnt64(h, libname);
}

const char* GetNeededVersionString32(elfheader_t* h, const char* libname, int idx)
#ifndef BOX32
{ return NULL; }
#else
 ;
#endif
const char* GetNeededVersionString64(elfheader_t* h, const char* libname, int idx)
{
    if(!libname)
        return NULL;
    if(h->VerNeed._64) {
        Elf64_Verneed *ver = (Elf64_Verneed*)((uintptr_t)h->VerNeed._64 + h->delta);
        while(ver) {
            char *filename = h->DynStr + ver->vn_file;
            Elf64_Vernaux *aux = (Elf64_Vernaux*)((uintptr_t)ver + ver->vn_aux);
            if(!strcmp(filename, libname)) {
                for(int j=0; j<ver->vn_cnt; ++j) {
                    if(j==idx) 
                        return h->DynStr+aux->vna_name;
                    aux = (Elf64_Vernaux*)((uintptr_t)aux + aux->vna_next);
                }
                return NULL;    // idx out of bound, return NULL...
           }
            ver = ver->vn_next?((Elf64_Verneed*)((uintptr_t)ver + ver->vn_next)):NULL;
        }
    }
    return NULL;
}
const char* GetNeededVersionString(elfheader_t* h, const char* libname, int idx)
{
    return box64_is32bits?GetNeededVersionString32(h, libname, idx):GetNeededVersionString64(h, libname, idx);
}

int GetNeededVersionForLib(elfheader_t* h, const char* libname, const char* ver)
{
    if(!libname || !ver)
        return 0;
    int n = GetNeededVersionCnt(h, libname);
    if(!n)
        return 0;
    for(int i=0; i<n; ++i) {
        const char* vername = GetNeededVersionString(h, libname, i);
        if(vername && !strcmp(ver, vername))
            return 1;
    }
    return 0;
}

static int SymbolMatch(elfheader_t* h, uint32_t i, int ver, const char* vername, int local, int veropt)
{
    int version = h->VerSym?((Elf64_Half*)((uintptr_t)h->VerSym+h->delta))[i]:-1;
    if(version!=-1) version &=0x7fff;
    const char* symvername = GetSymbolVersion(h, version);
    Elf64_Half flags = GetSymbolVersionFlag(h, version);
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

uint32_t old_elf_hash(const char* name)
{
    uint32_t h = 0, g;
    for(unsigned char c = *name; c; c = *++name) {
        h = (h << 4) + c;
        if((g = (h & 0xf0000000))) {
            h ^= g >> 24;
        }
        h &= ~g;
    }
    return h;
}

static Elf64_Sym* old_elf_lookup(elfheader_t* h, const char* symname, int ver, const char* vername, int local, int veropt)
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
        const char* name = h->DynStr + h->DynSym._64[i].st_name;
        if (!strcmp(symname, name) && SymbolMatch(h, i, ver, vername, local, veropt)) {
            return &h->DynSym._64[i];
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
        const char* name = h->DynStr + h->DynSym._64[buckets[i]].st_name;
        printf_log(LOG_NONE, "%d: %s\n", buckets[i], name);
    }
    printf_log(LOG_NONE,"Chains[%d] = ", nchains);
    for (uint32_t i = 0; i<nchains; ++i)
        printf_log_prefix(0, LOG_NONE, "%d ", chains[i]);
    printf_log(LOG_NONE, "\n------------\n");
}

uint32_t new_elf_hash(const char *name)
{
    uint32_t h = 5381;
    for (unsigned char c = *name; c; c = *++name)
            h = h * 33 + c;
    return h;
}

static Elf64_Sym* new_elf_lookup(elfheader_t* h, const char* symname, int ver, const char* vername, int local, int veropt)
{
    // Prepare hash table
    const uint32_t *hashtab = (uint32_t*)(h->gnu_hash + h->delta);
    const uint32_t nbuckets = hashtab[0];
    const uint32_t symoffset = hashtab[1];
    const uint32_t bloom_size = hashtab[2];
    const uint32_t bloom_shift = hashtab[3];
    const uint64_t *blooms = (uint64_t*)&hashtab[4];
    const uint32_t *buckets = (uint32_t*)&blooms[bloom_size];
    const uint32_t *chains = &buckets[nbuckets];
    // get hash from symname to lookup
    const uint32_t hash = new_elf_hash(symname);
    // early check with bloom: if at least one bit is not set, a symbol is surely missing.
    uint64_t word = blooms[(hash/64)%bloom_size];
    uint64_t mask = 0
        | 1LL << (hash%64)
        | 1LL << ((hash>>bloom_shift)%64);
    if ((word & mask) != mask) {
        return NULL;
    }
    // now look at the bucket chain for the symbol
    uint32_t symidx = buckets[hash%nbuckets];
    if (symidx < symoffset)
        return NULL;
    while(1) {
        const char* name = h->DynStr + h->DynSym._64[symidx].st_name;
        const uint32_t symhash = chains[symidx-symoffset];
        if ((hash|1) == (symhash|1) && !strcmp(name, symname) && SymbolMatch(h, symidx, ver, vername, local, veropt)) {
            return &h->DynSym._64[symidx];
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
    const uint64_t *blooms = (uint64_t*)&hashtab[4];
    const uint32_t *buckets = (uint32_t*)&blooms[bloom_size];
    const uint32_t *chains = &buckets[nbuckets];
    printf_log(LOG_NONE, "===============Dump GNU_HASH from %s\n", h->name);
    printf_log(LOG_NONE, "Bloom: size=%d, shift=%d\n", bloom_size, bloom_shift);
    printf_log(LOG_NONE, "Buckets[%d] offset=%d = \n", nbuckets, symoffset);
    for(uint32_t i=0; i<nbuckets; ++i) {
        uint32_t symidx = buckets[i];
        printf_log_prefix(0, LOG_NONE, "%d:", symidx);
        while(symidx>=symoffset) {
            const char* name = h->DynStr + h->DynSym._64[symidx].st_name;
            const uint32_t hash = chains[symidx-symoffset];
            if(hash&1)
                symidx=0;
            else
                symidx++;
            printf_log_prefix(0, LOG_NONE, " %s (%x) -> %d", name, hash, symidx);
        }
        printf_log(LOG_NONE, "\n");
    }
    printf_log(LOG_NONE, "\n===============\n");
}

#ifndef BOX32
Elf32_Sym* ElfLookup32(elfheader_t* h, const char* symname, int ver, const char* vername, int local, int veropt) { return NULL; }
Elf32_Sym* ElfSymTabLookup32(elfheader_t* h, const char* symname) { return NULL; }
Elf32_Sym* ElfDynSymLookup32(elfheader_t* h, const char* symname) { return NULL; }
#endif
Elf64_Sym* ElfLookup64(elfheader_t* h, const char* symname, int ver, const char* vername, int local, int veropt)
{
    if(h->gnu_hash)
        return new_elf_lookup(h, symname, ver, vername, local, veropt);
    return old_elf_lookup(h, symname, ver, vername, local, veropt);
}

Elf64_Sym* ElfSymTabLookup64(elfheader_t* h, const char* symname)
{
    if(!h->SymTab._64)
        return 0;
    for(size_t i=0; i<h->numSymTab; ++i) {
        Elf64_Sym* sym = &h->SymTab._64[i];
        int type = ELF64_ST_TYPE(sym->st_info);
        if(type==STT_FUNC || type==STT_TLS || type==STT_OBJECT) {
            const char * name = h->StrTab+sym->st_name;
            if(name && !strcmp(symname, name))
                return sym;
        }
    }
    return NULL;
}

Elf64_Sym* ElfDynSymLookup64(elfheader_t* h, const char* symname)
{
    if(!h->DynSym._64)
        return 0;
    for(size_t i=0; i<h->numDynSym; ++i) {
        Elf64_Sym* sym = &h->DynSym._64[i];
        int type = ELF64_ST_TYPE(sym->st_info);
        if(type==STT_FUNC || type==STT_TLS || type==STT_OBJECT) {
            const char * name = h->DynStr+sym->st_name;
            if(name && !strcmp(symname, name))
                return sym;
        }
    }
    return NULL;
}
