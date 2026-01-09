#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "debug.h"
#include "x64emu.h"
#include "box64context.h"
#include "symbols.h"
#include "dictionnary.h"

typedef struct onesymbol_s {
    uintptr_t   offs;
    uint32_t    sz;
    // need to track type of symbol?
    // need to track origin?
} onesymbol_t;

typedef struct versymbol_s {
    int         version;    // -1 = no-version, 0=local, 1=global, X=versioned
    int         veropt;
    const char* vername;    // NULL or version name if version=X
    onesymbol_t sym;
} versymbol_t;

typedef struct versymbols_s {
    int sz;
    int cap;
    versymbol_t *syms;
} versymbols_t;

KHASH_MAP_IMPL_STR(mapsymbols, versymbols_t);

kh_mapsymbols_t* NewMapSymbols()
{
    kh_mapsymbols_t* map = kh_init(mapsymbols);
    return map;
}

void FreeMapSymbols(kh_mapsymbols_t** map)
{
    if(!map || !(*map))
        return;
    versymbols_t *v;
    kh_foreach_value_ref(*map, v, box_free(v->syms););

    kh_destroy(mapsymbols, *map);
    *map = NULL;
}

// Exact same version (ver<2 or vername if ver>=2)
static int SameVersion(versymbol_t* s, int ver, const char* vername, int veropt)
{
    if(ver<2 && s->veropt)
        return 1;
    if(ver<2)
        return (s->version == ver)?1:0;
    if(ver>1 && veropt && s->version<2)
        return 1;
    if(s->vername && !strcmp(s->vername, vername))
        return 1;
    return 0;
    
}

static versymbol_t* FindVersionLocal(versymbols_t* s, size_t size)
{
    if(!s || !s->sz)
        return NULL;
    for (int i=0; i<s->sz; ++i)
        if(s->syms[i].version==0 && (!size || (size==s->syms[i].sym.sz)))
            return &s->syms[i];
    return NULL;
}
static versymbol_t* FindNoVersion(versymbols_t* s, size_t size)
{
    if(!s || !s->sz)
        return NULL;
    for (int i=0; i<s->sz; ++i)
        if(s->syms[i].version==-1 && (!size || (size==s->syms[i].sym.sz)))
            return &s->syms[i];
    return NULL;
}
static versymbol_t* FindVersionGlobal(versymbols_t* s, size_t size)
{
    if(!s || !s->sz)
        return NULL;
    for (int i=0; i<s->sz; ++i) {
        if(s->syms[i].version==1 && (!size || (size==s->syms[i].sym.sz)))
            return &s->syms[i];
        if(s->syms[i].version>1 && s->syms[i].veropt && (!size || (size==s->syms[i].sym.sz)))
            return &s->syms[i];
    }
    return NULL;
}
static versymbol_t* FindVersion(versymbols_t* s, size_t size, const char* vername)
{
    if(!s || !s->sz)
        return NULL;
    for (int i=0; i<s->sz; ++i)
        if(s->syms[i].vername && !strcmp(s->syms[i].vername, vername) && (!size || (size==s->syms[i].sym.sz)))
            return &s->syms[i];
    return NULL;
}
static versymbol_t* FindFirstVersion(versymbols_t* s, size_t size)
{
    if(!s || !s->sz)
        return NULL;
    for (int i=0; i<s->sz; ++i)
        if(s->syms[i].version>1 && (!size || (size==s->syms[i].sym.sz)))
            return &s->syms[i];
    return NULL;
}

// Match version (so ver=0:0, ver=1:-1/1/X, ver=-1:any, ver=X:1/"name")
static versymbol_t* MatchVersion(versymbols_t* s, int ver, const char* vername, size_t size, int local, int veropt)
{
    if(!s || !s->sz)
        return NULL;
    versymbol_t* ret = NULL;
    if(ver==0) {
        if(local) ret = FindVersionLocal(s, size);
        if(!ret) ret = FindNoVersion(s, size);
        if(!ret) ret = FindVersionGlobal(s, size);
        return ret;
    }
    if(ver==-1) {
        if(local) ret = FindVersionLocal(s, size);
        if(!ret) ret = FindNoVersion(s, size);
        if(!ret) ret = FindVersionGlobal(s, size);
        //if(!ret) ret = FindFirstVersion(s, size);
        return ret;
    }
    if(ver==-2) {
        if(local) ret = FindVersionLocal(s, size);
        if(!ret) ret = FindVersionGlobal(s, size);
        return ret;
    }
    if(ver==1) {
        if(local) ret = FindVersionLocal(s, size);
        if(!ret) ret = FindVersionGlobal(s, size);
        if(!ret) ret = FindNoVersion(s, size);
        //if(!ret) ret = FindFirstVersion(s);
        return ret;
    }
    if(veropt && ver>1) {
        ret = FindVersionGlobal(s, size);
        if(ret) return ret;
    }
    ret = FindVersion(s, size, vername);
    if(local && !ret) ret = FindVersionLocal(s, size);
    //if(!ret) return FindVersionGlobal(s);
    return ret;
}

void AddSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz, int ver, const char* vername, int veropt)
{
    int ret;
    khint_t k = kh_put(mapsymbols, mapsymbols, name, &ret);
    versymbols_t * v = &kh_val(mapsymbols, k);
    if(ret) {v->sz = v->cap = 0; v->syms = NULL;}
    // now check if that version already exist, and update record and exit if yes
    for(int i=0; i<v->sz; ++i)
        if(SameVersion(&v->syms[i], ver, vername, veropt)) {
            v->syms[i].sym.offs = addr;
            v->syms[i].sym.sz = sz;
            return;
        }
    // add a new record
    if(v->sz == v->cap) {
        v->cap+=4;
        v->syms = (versymbol_t*)box_realloc(v->syms, v->cap*sizeof(versymbol_t));
    }
    int idx = v->sz++;
    v->syms[idx].version = ver;
    v->syms[idx].vername = vername;
    v->syms[idx].sym.offs = addr;
    v->syms[idx].sym.sz = sz;
}

void ForceUpdateSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz)
{
    int ret;
    khint_t k = kh_put(mapsymbols, mapsymbols, name, &ret);
    versymbols_t * v = &kh_val(mapsymbols, k);
    if(ret) {v->sz = v->cap = 0; v->syms = NULL;}
    // now check if that version already exist, and update record and exit if yes
    for(int i=0; i<v->sz; ++i) {
        v->syms[i].sym.offs = addr;
        v->syms[i].sym.sz = sz;
    }
}

uintptr_t FindSymbol(kh_mapsymbols_t *mapsymbols, const char* name, int ver, const char* vername, int local, int veropt)
{
    if(!mapsymbols)
        return 0;
    khint_t k = kh_get(mapsymbols, mapsymbols, name);
    if(k==kh_end(mapsymbols))
        return 0;
    versymbols_t * v = &kh_val(mapsymbols, k);
    versymbol_t * s = MatchVersion(v, ver, vername, 0, local, veropt);
    if(s)
        return s->sym.offs;
    return 0;
}

void AddUniqueSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz, int ver, const char* vername, int veropt)
{
    int ret;
    khint_t k = kh_put(mapsymbols, mapsymbols, name, &ret);
    versymbols_t * v = &kh_val(mapsymbols, k);
    if(ret) {v->sz = v->cap = 0; v->syms = NULL;}
    // now check if that version already exist, and exit if yes
    for(int i=0; i<v->sz; ++i)
        if(SameVersion(&v->syms[i], ver, vername, veropt)) {
            return;
        }
    // add a new record
    if(v->sz == v->cap) {
        v->cap+=4;
        v->syms = (versymbol_t*)box_realloc(v->syms, v->cap*sizeof(versymbol_t));
    }
    int idx = v->sz++;
    v->syms[idx].version = ver;
    v->syms[idx].vername = vername;
    v->syms[idx].sym.offs = addr;
    v->syms[idx].sym.sz = sz;
    v->syms[idx].veropt = veropt;
    printf_dump(LOG_DEBUG, " Adding unique %s (ver:%d/%s%s) with offert=%p sz=%d\n", name, ver, vername?vername:"(none)", veropt?" veropt":"", (void*)addr, sz);
}

#ifndef STATICBUILD
void** my_GetGTKDisplay();
void** my_GetGthreadsGotInitialized();
#endif
int GetSymbolStartEnd(kh_mapsymbols_t* mapsymbols, const char* name, uintptr_t* start, uintptr_t* end, int ver, const char* vername, int local, int veropt)
{
    if(!mapsymbols)
        return 0;
    khint_t k = kh_get(mapsymbols, mapsymbols, name);
    if(k==kh_end(mapsymbols)) {
        #ifndef STATICBUILD
        // some special case symbol, defined inside box64 itself
        if(!strcmp(name, "gdk_display") && !BOX64ENV(nogtk)) {
            *start = (uintptr_t)my_GetGTKDisplay();
            *end = *start+sizeof(void*);
            printf_log(LOG_INFO, "Using global gdk_display for gdk-x11 (%p:%p)\n", start, *(void**)start);
            return 2;
        }
        if(!strcmp(name, "g_threads_got_initialized") && !BOX64ENV(nogtk)) {
            *start = (uintptr_t)my_GetGthreadsGotInitialized();
            *end = *start+sizeof(int);
            printf_log(LOG_INFO, "Using global g_threads_got_initialized for gthread2 (%p:%p)\n", start, *(void**)start);
            return 2;
        }
        #endif
        return 0;
    }
    versymbols_t * v = &kh_val(mapsymbols, k);
    versymbol_t* s = MatchVersion(v, ver, vername, 0, local, veropt);
    if(s) {
        if(start) *start = s->sym.offs;
        if(end) *end = *start + s->sym.sz;
        return 1;
    }
    return 0;
}

int GetSizedSymbolStartEnd(kh_mapsymbols_t* mapsymbols, const char* name, uintptr_t* start, uintptr_t* end, size_t size, int ver, const char* vername, int local, int veropt)
{
    if(!mapsymbols)
        return 0;
    khint_t k = kh_get(mapsymbols, mapsymbols, name);
    if(k==kh_end(mapsymbols))
        return 0;
    versymbols_t * v = &kh_val(mapsymbols, k);
    versymbol_t* s = MatchVersion(v, ver, vername, size, local, veropt);
    if(s) {
        *start = s->sym.offs;
        *end = *start + s->sym.sz;
        return 1;
    }
    return 0;
}

const char* GetSymbolName(kh_mapsymbols_t* mapsymbols, void* p, uintptr_t* start, uint32_t* sz, const char** vername)
{
    uintptr_t addr = (uintptr_t)p;
    versymbols_t *s;
    kh_foreach_value_ref(mapsymbols, s, 
        for(int i=0; i<s->sz; ++i)
            if((s->syms[i].sym.offs >= addr) && (s->syms[i].sym.offs+s->syms[i].sym.sz<addr)) {
                *start  = s->syms[i].sym.offs;
                *sz = s->syms[i].sym.sz;
                if(vername)
                    *vername = s->syms[i].vername;
                return kh_key(mapsymbols, __i);
            }
    );
    return NULL;
}
