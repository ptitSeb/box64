#ifndef LIBNAME
#error Meh
#endif

#define FUNC3(M,N) wrapped##M##N
#define FUNC2(M,N) FUNC3(M,N)
#define FUNC(N) FUNC2(LIBNAME,N)
#define QUOTE(M) #M
#define PRIVATE2(P) QUOTE(wrapped##P##_private.h)
#define PRIVATE(P) PRIVATE2(P)
#define MAPNAME3(N,M) N##M
#define MAPNAME2(N,M) MAPNAME3(N,M)
#define MAPNAME(N) MAPNAME2(LIBNAME,N)
// prepare the maps
#define GO(N, W)
#define GOW(N, W)
#define GOM(N, W)
#define GOS(N, W)
#define GO2(N, W, O)
#define DATA(N, S)
#define DATAV(N, S)
#define DATAB(N, S)
#define DATAM(N, S)

// #define the 4 maps first
#undef GO
#define GO(N, W) {#N, W, 0},
static const map_onesymbol_t MAPNAME(symbolmap)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GO
#define GO(N, W)
#undef GOW
#define GOW(N, W) {#N, W, 1},
static const map_onesymbol_t MAPNAME(wsymbolmap)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GOW
#define GOW(N, W)
#undef GOM
#define GOM(N, W) {#N, W, 0},
static const map_onesymbol_t MAPNAME(mysymbolmap)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GOM
#define GOM(N, W)
#undef GOS
#define GOS(N, W) {#N, W, 0},
static const map_onesymbol_t MAPNAME(stsymbolmap)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GOS
#define GOS(N, W)
#undef GO2
#define GO2(N, W, O) {#N, W, 0, #O},
static const map_onesymbol2_t MAPNAME(symbol2map)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GO2
#define GO2(N, W, O)
#undef DATA
#undef DATAV
#undef DATAB
#define DATA(N, S) {#N, S, 0},
#define DATAV(N, S) {#N, S, 1},
#define DATAB(N, S) {#N, S, 0},
static const map_onedata_t MAPNAME(datamap)[] = {
    #include PRIVATE(LIBNAME)
};
#undef DATA
#undef DATAV
#undef DATAB
#define DATA(N, S)
#define DATAV(N, S)
#define DATAB(N, S)
#undef DATAM
#define DATAM(N, S) {#N, S, 0},
static const map_onedata_t MAPNAME(mydatamap)[] = {
    #include PRIVATE(LIBNAME)
};
#include "wrappedlib_undefs.h"



int FUNC(_init)(library_t* lib, box64context_t* box64)
{
    (void)box64;
    
    // Init first
    free(lib->path); lib->path=NULL;
#ifdef PRE_INIT
    PRE_INIT
#endif
    {
        lib->priv.w.lib = dlopen(MAPNAME(Name), RTLD_LAZY | RTLD_GLOBAL);
        if(!lib->priv.w.lib) {
#ifdef ALTNAME
        lib->priv.w.lib = dlopen(ALTNAME, RTLD_LAZY | RTLD_GLOBAL);
        if(!lib->priv.w.lib)
#endif
#ifdef ALTNAME2
            {
            lib->priv.w.lib = dlopen(ALTNAME2, RTLD_LAZY | RTLD_GLOBAL);
            if(!lib->priv.w.lib)
#endif
                return -1;
#ifdef ALTNAME2
                else lib->path = strdup(ALTNAME2);
            } else lib->path = strdup(ALTNAME);
#endif
        } else lib->path = strdup(MAPNAME(Name));
    }
    lib->priv.w.bridge = NewBridge();
// Create maps
    lib->symbolmap = kh_init(symbolmap);
    lib->wsymbolmap = kh_init(symbolmap);
    lib->mysymbolmap = kh_init(symbolmap);
    lib->stsymbolmap = kh_init(symbolmap);
    lib->symbol2map = kh_init(symbol2map);
    lib->datamap = kh_init(datamap);
    lib->wdatamap = kh_init(datamap);
    lib->mydatamap = kh_init(datamap);

    khint_t k;
    int ret;
    int cnt;

    // populates maps...
    cnt = sizeof(MAPNAME(symbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, lib->symbolmap, MAPNAME(symbolmap)[i].name, &ret);
        kh_value(lib->symbolmap, k) = MAPNAME(symbolmap)[i].w;
    }
    cnt = sizeof(MAPNAME(wsymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, lib->wsymbolmap, MAPNAME(wsymbolmap)[i].name, &ret);
        kh_value(lib->wsymbolmap, k) = MAPNAME(wsymbolmap)[i].w;
    }
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, lib->mysymbolmap, MAPNAME(mysymbolmap)[i].name, &ret);
        kh_value(lib->mysymbolmap, k) = MAPNAME(mysymbolmap)[i].w;
    }
    cnt = sizeof(MAPNAME(stsymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, lib->stsymbolmap, MAPNAME(stsymbolmap)[i].name, &ret);
        kh_value(lib->stsymbolmap, k) = MAPNAME(stsymbolmap)[i].w;
    }
    cnt = sizeof(MAPNAME(symbol2map))/sizeof(map_onesymbol2_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbol2map, lib->symbol2map, MAPNAME(symbol2map)[i].name, &ret);
        kh_value(lib->symbol2map, k).name = MAPNAME(symbol2map)[i].name2;
        kh_value(lib->symbol2map, k).w = MAPNAME(symbol2map)[i].w;
        kh_value(lib->symbol2map, k).weak = MAPNAME(symbol2map)[i].weak;
    }
    cnt = sizeof(MAPNAME(datamap))/sizeof(map_onedata_t);
    for (int i=0; i<cnt; ++i) {
        if(MAPNAME(datamap)[i].weak) {
            k = kh_put(datamap, lib->wdatamap, MAPNAME(datamap)[i].name, &ret);
            kh_value(lib->wdatamap, k) = MAPNAME(datamap)[i].sz;
        } else {
            k = kh_put(datamap, lib->datamap, MAPNAME(datamap)[i].name, &ret);
            kh_value(lib->datamap, k) = MAPNAME(datamap)[i].sz;
        }
    }
    cnt = sizeof(MAPNAME(mydatamap))/sizeof(map_onedata_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(datamap, lib->mydatamap, MAPNAME(mydatamap)[i].name, &ret);
        kh_value(lib->mydatamap, k) = MAPNAME(mydatamap)[i].sz;
    }
#ifdef CUSTOM_INIT
    CUSTOM_INIT
#endif
    
    return 0;
}

int FUNC(_fini)(library_t* lib)
{
#ifdef CUSTOM_FINI
    CUSTOM_FINI
#endif
    if(lib->priv.w.lib)
        dlclose(lib->priv.w.lib);
    lib->priv.w.lib = NULL;
    if(lib->priv.w.altprefix)
        free(lib->priv.w.altprefix);
    if(lib->priv.w.neededlibs) {
        for(int i=0; i<lib->priv.w.needed; ++i)
            free(lib->priv.w.neededlibs[i]);
        free(lib->priv.w.neededlibs);
    }
    FreeBridge(&lib->priv.w.bridge);
    return 1;
}

int FUNC(_get)(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz)
{
    uintptr_t addr = 0;
    uint32_t size = 0;
#ifdef CUSTOM_FAIL
    void* symbol = NULL;
#endif
//PRE
    if (!getSymbolInMaps(lib, name, 0, &addr, &size)) {
#ifdef CUSTOM_FAIL
    CUSTOM_FAIL
#else
        return 0;
#endif
    }
//POST
    if(!addr && !size)
        return 0;
    *offs = addr;
    *sz = size;
    return 1;
}

int FUNC(_getnoweak)(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz)
{
    uintptr_t addr = 0;
    uint32_t size = 0;
#ifdef CUSTOM_FAIL
    void* symbol = NULL;
#endif
//PRE
    if (!getSymbolInMaps(lib, name, 1, &addr, &size)) {
#ifdef CUSTOM_FAIL
    CUSTOM_FAIL
#else
        return 0;
#endif
    }
//POST
    if(!addr && !size)
        return 0;
    *offs = addr;
    *sz = size;
    return 1;
}
