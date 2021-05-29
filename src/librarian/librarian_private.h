#ifndef __LIBRARIAN_PRIVATE_H_
#define __LIBRARIAN_PRIVATE_H_
#include <stdint.h>

#include "custommem.h"
#include "khash.h"

typedef struct box64context_s box64context_t;
typedef struct kh_mapsymbols_s kh_mapsymbols_t;

typedef char* cstr_t;

KHASH_MAP_DECLARE_INT(mapoffsets, cstr_t);

typedef struct lib_s {
    khash_t(mapsymbols)   *mapsymbols;
    khash_t(mapsymbols)   *weaksymbols;
    khash_t(mapsymbols)   *localsymbols;
    khash_t(mapoffsets)   *mapoffsets;
    khash_t(mapsymbols)   *globaldata;
    library_t             **libraries;
    int                   libsz;
    int                   libcap;
    int                   ownlibs;
    library_t             *owner;       // in case that maplib is owned by a lib

    box64context_t*       context;
    
    bridge_t              *bridge;        // all x86 -> arm bridge
} lib_t;

#endif //__LIBRARIAN_PRIVATE_H_
