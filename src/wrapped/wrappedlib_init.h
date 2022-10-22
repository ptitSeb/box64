#ifndef LIBNAME
#error Meh
#endif

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
#define GO(N, W) {#N, W, 0},
#undef GOW
#define GOW(N, W) {#N, W, 1},
static const map_onesymbol_t MAPNAME(symbolmap)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GO
#define GO(N, W)
#undef GOW
#define GOW(N, W)
#undef GOM
#define GOM(N, W) {#N, W, 0},
#undef GOWM
#define GOWM(N, W) {#N, W, 1},
static const map_onesymbol_t MAPNAME(mysymbolmap)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GOM
#define GOM(N, W)
#undef GOWM
#define GOWM(N, W)
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

int FUNC(_init)(library_t* lib, box64context_t* box64)
{
    (void)box64;

    // Init first
    free(lib->path); lib->path=NULL;
#ifdef PRE_INIT
    PRE_INIT
#endif
    {
        lib->w.lib = dlopen(MAPNAME(Name), RTLD_LAZY | RTLD_GLOBAL);
        if(!lib->w.lib) {
#ifdef ALTNAME
        lib->w.lib = dlopen(ALTNAME, RTLD_LAZY | RTLD_GLOBAL);
        if(!lib->w.lib)
#endif
#ifdef ALTNAME2
            {
            lib->w.lib = dlopen(ALTNAME2, RTLD_LAZY | RTLD_GLOBAL);
            if(!lib->w.lib)
#endif
                return -1;
#ifdef ALTNAME2
                else lib->path = strdup(ALTNAME2);
            } else lib->path = strdup(ALTNAME);
#endif
        } else lib->path = strdup(MAPNAME(Name));
    }
    WrappedLib_CommonInit(lib);

    khint_t k;
    int ret;
    int cnt;

    // populates maps...
#define DOIT(mapname) \
	cnt = sizeof(MAPNAME(mapname))/sizeof(map_onesymbol_t);                         \
	for (int i = 0; i < cnt; ++i) {                                                 \
        if (MAPNAME(mapname)[i].weak) {                                             \
            k = kh_put(symbolmap, lib->w.w##mapname, MAPNAME(mapname)[i].name, &ret); \
            kh_value(lib->w.w##mapname, k) = MAPNAME(mapname)[i].w;                   \
        } else {                                                                    \
            k = kh_put(symbolmap, lib->w.mapname, MAPNAME(mapname)[i].name, &ret);    \
            kh_value(lib->w.mapname, k) = MAPNAME(mapname)[i].w;                      \
        }                                                                           \
        if (strchr(MAPNAME(mapname)[i].name, '@'))                                  \
            AddDictionnary(box64->versym, MAPNAME(mapname)[i].name);                \
	}
	DOIT(symbolmap)
	DOIT(mysymbolmap)
#undef DOIT
    cnt = sizeof(MAPNAME(stsymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, lib->w.stsymbolmap, MAPNAME(stsymbolmap)[i].name, &ret);
        kh_value(lib->w.stsymbolmap, k) = MAPNAME(stsymbolmap)[i].w;
        if(strchr(MAPNAME(stsymbolmap)[i].name, '@'))
            AddDictionnary(box64->versym, MAPNAME(stsymbolmap)[i].name);
    }
    cnt = sizeof(MAPNAME(symbol2map))/sizeof(map_onesymbol2_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbol2map, lib->w.symbol2map, MAPNAME(symbol2map)[i].name, &ret);
        kh_value(lib->w.symbol2map, k).name = MAPNAME(symbol2map)[i].name2;
        kh_value(lib->w.symbol2map, k).w = MAPNAME(symbol2map)[i].w;
        kh_value(lib->w.symbol2map, k).weak = MAPNAME(symbol2map)[i].weak;
        if(strchr(MAPNAME(symbol2map)[i].name, '@'))
            AddDictionnary(box64->versym, MAPNAME(symbol2map)[i].name);
    }
    cnt = sizeof(MAPNAME(datamap))/sizeof(map_onedata_t);
    for (int i=0; i<cnt; ++i) {
        if(MAPNAME(datamap)[i].weak) {
            k = kh_put(datamap, lib->w.wdatamap, MAPNAME(datamap)[i].name, &ret);
            kh_value(lib->w.wdatamap, k) = MAPNAME(datamap)[i].sz;
        } else {
            k = kh_put(datamap, lib->w.datamap, MAPNAME(datamap)[i].name, &ret);
            kh_value(lib->w.datamap, k) = MAPNAME(datamap)[i].sz;
        }
    }
    cnt = sizeof(MAPNAME(mydatamap))/sizeof(map_onedata_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(datamap, lib->w.mydatamap, MAPNAME(mydatamap)[i].name, &ret);
        kh_value(lib->w.mydatamap, k) = MAPNAME(mydatamap)[i].sz;
    }
#ifdef CUSTOM_INIT
    CUSTOM_INIT
#endif

    return 0;
}

void FUNC(_fini)(library_t* lib)
{
#ifdef CUSTOM_FINI
    CUSTOM_FINI
#endif
    WrappedLib_FinishFini(lib);
}

