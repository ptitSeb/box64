#ifndef __LIBRARIAN_PRIVATE_H_
#define __LIBRARIAN_PRIVATE_H_
#include <stdint.h>

#include "custommem.h"
#include "khash.h"

typedef struct box64context_s box64context_t;

typedef struct onesymbol_s {
    uintptr_t   offs;
    uint32_t    sz;
    // need to track type of symbol?
    // need to track origin?
} onesymbol_t;

typedef struct {
    char        *name;
    library_t   *lib;
} onelib_t;

typedef char* cstr_t;

KHASH_MAP_DECLARE_STR(mapsymbols, onesymbol_t)

KHASH_MAP_DECLARE_INT(mapoffsets, cstr_t);

typedef struct lib_s {
    khash_t(mapsymbols)   *mapsymbols;
    khash_t(mapsymbols)   *weaksymbols;
    khash_t(mapsymbols)   *localsymbols;
    khash_t(mapoffsets)   *mapoffsets;
    khash_t(mapsymbols)   *globaldata;
    onelib_t              *libraries;
    int                   libsz;
    int                   libcap;
    int                   ownlibs;

    box64context_t*       context;
    
    bridge_t              *bridge;        // all x86 -> arm bridge
} lib_t;

#endif //__LIBRARIAN_PRIVATE_H_
