#ifndef __LIBRARY_PRIVATE_H_
#define __LIBRARY_PRIVATE_H_
#include <stdint.h>
#include <elf.h>

#include "custommem.h"
#include "khash.h"
#include "wrappedlibs.h"
#include "box64context.h"

typedef struct lib_s    lib_t;
typedef struct bridge_s bridge_t;
typedef struct kh_bridgemap_s kh_bridgemap_t;
typedef struct kh_mapsymbols_s kh_mapsymbols_t;

typedef struct x64emu_s x64emu_t;
typedef void (*wrapper_t)(x64emu_t* emu, uintptr_t fnc);

typedef struct symbol2_s {
    wrapper_t    w;
    const char*  name;
    int          weak;
} symbol2_t;

KHASH_MAP_DECLARE_STR(symbolmap, wrapper_t)
KHASH_MAP_DECLARE_STR(symbol2map, symbol2_t)
KHASH_MAP_DECLARE_STR(datamap, uint64_t)


#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

typedef struct wlib_s {
    bridge_t        *bridge;
    void*           lib;        // dlopen result
    void*           priv;       // actual private
    char*           altprefix;  // if function names are mangled..
    int             needed;
    char**          neededlibs;
    kh_symbolmap_t  *symbolmap;
    kh_symbolmap_t  *wsymbolmap;
    kh_symbolmap_t  *mysymbolmap;
    kh_symbolmap_t  *wmysymbolmap;
    kh_symbolmap_t  *stsymbolmap;
    kh_symbol2map_t *symbol2map;
    kh_datamap_t    *datamap;
    kh_datamap_t    *wdatamap;
    kh_datamap_t    *mydatamap;
    char            *altmy;      // to avoid duplicate symbol, like with SDL1/SDL2
} wlib_t;

typedef struct elib_s {
    int             elf_index;
    elfheader_t     *elf;
    int             finalized;
    kh_mapsymbols_t *mapsymbols;
    kh_mapsymbols_t *weaksymbols;
    kh_mapsymbols_t *localsymbols;
} elib_t;

typedef struct library_s {
    char*               name;   // <> path
    char*               path;   // original path
    int                 nbdot;  // nombre of "." after .so
    int                 type;   // 0: native(wrapped) 1: emulated(elf) -1: undetermined
    int                 active;
    wrappedlib_fini_t   fini;
    wrappedlib_get_t    get;        // get weak and no weak
    wrappedlib_get_t    getnoweak;  // get only non weak symbol
    wrappedlib_get_t    getlocal;
    union {
        wlib_t  w;     
        elib_t  e;
    };                              // private lib data
    char                *altmy;     // to avoid duplicate symbol, like with SDL1/SDL2
    needed_libs_t       needed;
    needed_libs_t       depended;   // used to free library
    lib_t               *maplib;    // local maplib, for dlopen'd library with LOCAL binding (most of the dlopen)
    kh_bridgemap_t  *bridgemap;
} library_t;

// type for map elements
typedef struct map_onesymbol_s {
    const char* name;
    wrapper_t   w;
    int         weak;
} map_onesymbol_t;
typedef struct map_onesymbol2_s {
    const char* name;
    wrapper_t   w;
    int         weak;
    const char* name2;
} map_onesymbol2_t;
typedef struct map_onedata_s {
    const char* name;
    uint32_t    sz;                 // TODO: convert to size_t
    int         weak;
} map_onedata_t;

int getSymbolInMaps(library_t *lib, const char* name, int noweak, uintptr_t *addr, uintptr_t *size, int version, const char* vername, int local);  // Add bridges to functions

typedef struct linkmap_s {
    // actual struct link_map
    Elf64_Addr  l_addr;
    char*       l_name;
    Elf64_Dyn*  l_ld;
    struct linkmap_s *l_next, *l_prev;
    // custom
    library_t*  l_lib;

} linkmap_t;

linkmap_t* getLinkMapLib(library_t* lib);
linkmap_t* addLinkMapLib(library_t* lib);
void removeLinkMapLib(library_t* lib);

#endif //__LIBRARY_PRIVATE_H_
