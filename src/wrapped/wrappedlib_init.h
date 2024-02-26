#ifndef LIBNAME
#error Meh
#endif

#include "debug.h"
#include "librarian/library_inner.h"

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
#define _DOIT(P,Q) QUOTE(generated/wrapped##P##Q.h)
#define DOIT(P,Q) _DOIT(P,Q)
#include DOIT(LIBNAME,defs)

// regular symbol mapped to itself
#define GO(N, W)
// regular symbol mapped to itself, but weak
#define GOW(N, W)
// symbol mapped to my_symbol
#define GOM(N, W)
// symbol mapped to my_symbol, but weak
#define GOWM(N, W)
// regular symbol mapped to itself, that returns a structure
#define GOS(N, W)
// symbol mapped to another one
#define GO2(N, W, O)
// data
#define DATA(N, S)
// data, Weak (type V)
#define DATAV(N, S)
// data, Uninitialized (type B)
#define DATAB(N, S)
// data, "my_" type
#define DATAM(N, S)

// #define the 4 maps first
#undef GO
#undef GOW
#ifdef STATICBUILD
#define GO(N, W) {#N, W, 0, &N},
#define GOW(N, W) {#N, W, 1, &N},
#else
#define GO(N, W) {#N, W, 0},
#define GOW(N, W) {#N, W, 1},
#endif
static const map_onesymbol_t MAPNAME(symbolmap)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GO
#undef GOW
#undef GOM
#undef GOWM
#define GO(N, W)
#define GOW(N, W)
#ifdef STATICBUILD
#define GOM(N, W) {#N, W, 0, &my_##N},
#define GOWM(N, W) {#N, W, 1, &my_##N},
#else
#define GOM(N, W) {#N, W, 0},
#define GOWM(N, W) {#N, W, 1},
#endif
static const map_onesymbol_t MAPNAME(mysymbolmap)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GOM
#undef GOWM
#undef GOS
#define GOM(N, W)
#define GOWM(N, W)
#ifdef STATICBUILD
#define GOS(N, W) {#N, W, 0, &my_##N},
#else
#define GOS(N, W) {#N, W, 0},
#endif
static const map_onesymbol_t MAPNAME(stsymbolmap)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GOS
#undef GO2
#define GOS(N, W)
#ifdef STATICBUILD
#define GO2(N, W, O) {#N, W, 0, #O, &O},
#else
#define GO2(N, W, O) {#N, W, 0, #O},
#endif
static const map_onesymbol2_t MAPNAME(symbol2map)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GO2
#define GO2(N, W, O)
#undef DATA
#undef DATAV
#undef DATAB
#ifdef STATICBUILD
#define DATA(N, S) {#N, S, 0, (void*)&N},
#define DATAV(N, S) {#N, S, 1, (void*)&N},
#define DATAB(N, S) {#N, S, 0, (void*)&N},
#else
#define DATA(N, S) {#N, S, 0},
#define DATAV(N, S) {#N, S, 1},
#define DATAB(N, S) {#N, S, 0},
#endif
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
#ifdef STATICBUILD
#define DATAM(N, S) {#N, S, 0, &my_##N},
#else
#define DATAM(N, S) {#N, S, 0},
#endif
static const map_onedata_t MAPNAME(mydatamap)[] = {
    #include PRIVATE(LIBNAME)
};

#undef GO
#undef GOW
#undef GOM
#undef GOWM
#undef GO2
#undef GOS
#undef DATA
#undef DATAV
#undef DATAB
#undef DATAM

#include DOIT(LIBNAME,undefs)
#undef DOIT
#undef _DOIT

#if !defined(ALTNAME) && defined(ALTNAME2)
#error Please define ALTNAME before defining ALTNAME2
#endif

#define COUNT_NARGS(...) COUNT_NARGS_AUX(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define COUNT_NARGS_AUX(_1, _2, _3, _4, _5, n, ...) COUNT_NARGS_AUX##n(This should not appear, too many libraries)
#define COUNT_NARGS_AUX1(v1, v2) 1
#define COUNT_NARGS_AUX2(v1, v2) 2
#define COUNT_NARGS_AUX3(v1, v2) 3
#define COUNT_NARGS_AUX4(v1, v2) 4
#define COUNT_NARGS_AUX5(v1, v2) 5

int FUNC(_init)(library_t* lib, box64context_t* box64)
{
    (void)box64;

    // Init first
    free(lib->path); lib->path=NULL;
#ifdef PRE_INIT
    PRE_INIT
#endif
    {
#ifndef STATICBUILD
        lib->w.lib = dlopen(MAPNAME(Name), RTLD_LAZY | RTLD_GLOBAL);
        if(!lib->w.lib) {
#ifdef ALTNAME
            lib->w.lib = dlopen(ALTNAME, RTLD_LAZY | RTLD_GLOBAL);
            if(!lib->w.lib) {
#ifdef ALTNAME2
                lib->w.lib = dlopen(ALTNAME2, RTLD_LAZY | RTLD_GLOBAL);
                if(!lib->w.lib)
#endif
#endif
                return -1;
#ifdef ALTNAME
#ifdef ALTNAME2
                else lib->path = box_strdup(ALTNAME2);
#endif
            } else lib->path = box_strdup(ALTNAME);
#endif
        } else lib->path = box_strdup(MAPNAME(Name));
#else
        lib->path = box_strdup(MAPNAME(Name));
#endif
    }
    WrappedLib_CommonInit(lib);

    khint_t k;
    int ret;
    int cnt;

    // populates maps...
#ifdef STATICBUILD
#define DOIT(mapname) \
    cnt = sizeof(MAPNAME(mapname))/sizeof(map_onesymbol_t);                         \
    for (int i = 0; i < cnt; ++i) {                                                 \
        if (MAPNAME(mapname)[i].weak) {                                             \
            k = kh_put(symbolmap, lib->w.w##mapname, MAPNAME(mapname)[i].name, &ret); \
            kh_value(lib->w.w##mapname, k).w = MAPNAME(mapname)[i].w;               \
            kh_value(lib->w.w##mapname, k).resolved = 0;                            \
            kh_value(lib->w.w##mapname, k).addr = (uintptr_t)MAPNAME(mapname)[i].addr; \
        } else {                                                                    \
            k = kh_put(symbolmap, lib->w.mapname, MAPNAME(mapname)[i].name, &ret);  \
            kh_value(lib->w.mapname, k).w = MAPNAME(mapname)[i].w;                  \
            kh_value(lib->w.mapname, k).resolved = 0;                               \
            kh_value(lib->w.mapname, k).addr = (uintptr_t)MAPNAME(mapname)[i].addr; \
        }                                                                           \
        if (strchr(MAPNAME(mapname)[i].name, '@'))                                  \
            AddDictionnary(box64->versym, MAPNAME(mapname)[i].name);                \
    }
#else
#define DOIT(mapname) \
    cnt = sizeof(MAPNAME(mapname))/sizeof(map_onesymbol_t);                         \
    for (int i = 0; i < cnt; ++i) {                                                 \
        if (MAPNAME(mapname)[i].weak) {                                             \
            k = kh_put(symbolmap, lib->w.w##mapname, MAPNAME(mapname)[i].name, &ret); \
            kh_value(lib->w.w##mapname, k).w = MAPNAME(mapname)[i].w;               \
            kh_value(lib->w.w##mapname, k).resolved = 0;                            \
        } else {                                                                    \
            k = kh_put(symbolmap, lib->w.mapname, MAPNAME(mapname)[i].name, &ret);  \
            kh_value(lib->w.mapname, k).w = MAPNAME(mapname)[i].w;                  \
            kh_value(lib->w.mapname, k).resolved = 0;                               \
        }                                                                           \
        if (strchr(MAPNAME(mapname)[i].name, '@'))                                  \
            AddDictionnary(box64->versym, MAPNAME(mapname)[i].name);                \
    }
#endif
    DOIT(symbolmap)
    DOIT(mysymbolmap)
#undef DOIT
    cnt = sizeof(MAPNAME(stsymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, lib->w.stsymbolmap, MAPNAME(stsymbolmap)[i].name, &ret);
        kh_value(lib->w.stsymbolmap, k).w = MAPNAME(stsymbolmap)[i].w;
        #ifdef STATICBUILD
        kh_value(lib->w.stsymbolmap, k).resolved = 1;
        kh_value(lib->w.stsymbolmap, k).addr = (uintptr_t)MAPNAME(stsymbolmap)[i].addr;
        #else
        kh_value(lib->w.stsymbolmap, k).resolved = 0;
        #endif
        if(strchr(MAPNAME(stsymbolmap)[i].name, '@'))
            AddDictionnary(box64->versym, MAPNAME(stsymbolmap)[i].name);
    }
    cnt = sizeof(MAPNAME(symbol2map))/sizeof(map_onesymbol2_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbol2map, lib->w.symbol2map, MAPNAME(symbol2map)[i].name, &ret);
        kh_value(lib->w.symbol2map, k).name = MAPNAME(symbol2map)[i].name2;
        kh_value(lib->w.symbol2map, k).w = MAPNAME(symbol2map)[i].w;
        kh_value(lib->w.symbol2map, k).weak = MAPNAME(symbol2map)[i].weak;
        #ifdef STATICBUILD
        kh_value(lib->w.symbol2map, k).resolved = 1;
        kh_value(lib->w.symbol2map, k).addr = (uintptr_t)MAPNAME(symbol2map)[i].addr;
        #else
        kh_value(lib->w.symbol2map, k).resolved = 0;
        #endif
        if(strchr(MAPNAME(symbol2map)[i].name, '@'))
            AddDictionnary(box64->versym, MAPNAME(symbol2map)[i].name);
    }
    cnt = sizeof(MAPNAME(datamap))/sizeof(map_onedata_t);
    for (int i=0; i<cnt; ++i) {
        if(MAPNAME(datamap)[i].weak) {
            k = kh_put(datamap, lib->w.wdatamap, MAPNAME(datamap)[i].name, &ret);
            #ifdef STATICBUILD
            kh_value(lib->w.wdatamap, k).size = MAPNAME(datamap)[i].sz;
            kh_value(lib->w.wdatamap, k).addr = (uintptr_t)MAPNAME(datamap)[i].addr;
            #else
            kh_value(lib->w.wdatamap, k) = MAPNAME(datamap)[i].sz;
            #endif
        } else {
            k = kh_put(datamap, lib->w.datamap, MAPNAME(datamap)[i].name, &ret);
            #ifdef STATICBUILD
            kh_value(lib->w.datamap, k).size = MAPNAME(datamap)[i].sz;
            kh_value(lib->w.datamap, k).addr = (uintptr_t)MAPNAME(datamap)[i].addr;
            #else
            kh_value(lib->w.datamap, k) = MAPNAME(datamap)[i].sz;
            #endif
        }
    }
    cnt = sizeof(MAPNAME(mydatamap))/sizeof(map_onedata_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(datamap, lib->w.mydatamap, MAPNAME(mydatamap)[i].name, &ret);
        #ifdef STATICBUILD
        kh_value(lib->w.mydatamap, k).size = MAPNAME(mydatamap)[i].sz;
        kh_value(lib->w.mydatamap, k).addr = (uintptr_t)MAPNAME(mydatamap)[i].addr;
        #else
        kh_value(lib->w.mydatamap, k) = MAPNAME(mydatamap)[i].sz;
        #endif
    }
#ifdef ALTMY
    SETALT(ALTMY);
#endif
#ifdef HAS_MY
    getMy(lib);
#endif
#ifdef CUSTOM_INIT
    CUSTOM_INIT
#endif
#ifdef NEEDED_LIBS
    setNeededLibs(lib, COUNT_NARGS(NEEDED_LIBS), NEEDED_LIBS);
#endif

    return 0;
}

void FUNC(_fini)(library_t* lib)
{
#ifdef HAS_MY
    freeMy();
#endif
#ifdef CUSTOM_FINI
    CUSTOM_FINI
#endif
    WrappedLib_FinishFini(lib);
}
