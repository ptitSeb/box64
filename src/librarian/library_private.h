#ifndef __LIBRARY_PRIVATE_H_
#define __LIBRARY_PRIVATE_H_
#include <stdint.h>
#include <elf.h>

#include "custommem.h"
#include "khash.h"
#include "wrappedlibs.h"
#include "box64context.h"

typedef struct lib_s            lib_t;
typedef struct bridge_s         bridge_t;
typedef struct elfheader_s      elfheader_t;
typedef struct kh_mapsymbols_s  kh_mapsymbols_t;
typedef struct x64emu_s         x64emu_t;

typedef void (*wrapper_t)(x64emu_t* emu, uintptr_t fnc);

typedef struct symbol1_s {
    wrapper_t   w;
    int         resolved;
    uintptr_t   addr;
} symbol1_t;

typedef struct symbol2_s {
    wrapper_t    w;
    const char*  name;
    int          weak;
    int          resolved;
    uintptr_t    addr;
} symbol2_t;

#ifdef STATICBUILD
typedef struct datamap_s {
    uint64_t    size;
    uintptr_t   addr;
} datamap_t;
#endif

KHASH_MAP_DECLARE_STR(symbolmap, symbol1_t)
KHASH_MAP_DECLARE_STR(symbol2map, symbol2_t)
#ifdef STATICBUILD
KHASH_MAP_DECLARE_STR(datamap, datamap_t)
#else
KHASH_MAP_DECLARE_STR(datamap, uint64_t)
#endif


#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

typedef struct wlib_s {
    bridge_t        *bridge;
    void*           lib;        // dlopen result
    void*           priv;       // actual private
    char*           altprefix;  // if function names are mangled..
    needed_libs_t*  needed;
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
    int             refcnt;      // refcounting the lib
} wlib_t;

typedef struct elib_s {
    int             elf_index;
    elfheader_t     *elf;
    int             finalized;
} elib_t;

typedef struct library_s {
    char*               name;   // <> path
    char*               path;   // original path
    int8_t              nbdot;  // nombre of "." after .so
    int8_t              type;   // 0: native(wrapped) 1: emulated(elf) -1: undetermined
    uint8_t             deepbind;
    wrappedlib_fini_t   fini;
    wrappedlib_get_t    getglobal;  // get global (non-weak)
    wrappedlib_get_t    getweak;    // get weak symbol
    wrappedlib_get_t    getlocal;   // get local symbol
    union {
        wlib_t  w;     
        elib_t  e;
    };                              // private lib data
    lib_t               *maplib;        // local maplib, for dlopen'd library with LOCAL binding (most of the dlopen)
    int                 maplib_ref;     // ref to maplib (not owned)
    size_t              dlopen;   // idx to the dlopen idx (or 0 if not dlopen)
} library_t;

// type for map elements
typedef struct map_onesymbol_s {
    const char* name;
    wrapper_t   w;
    int         weak;
#ifdef STATICBUILD
    void*       addr;
#endif
} map_onesymbol_t;
typedef struct map_onesymbol2_s {
    const char* name;
    wrapper_t   w;
    int         weak;
    const char* name2;
#ifdef STATICBUILD
    void*       addr;
#endif
} map_onesymbol2_t;
typedef struct map_onedata_s {
    const char* name;
    uint32_t    sz;                 // TODO: convert to size_t
    int         weak;
#ifdef STATICBUILD
    void*       addr;
#endif
} map_onedata_t;

int getSymbolInMaps(library_t *lib, const char* name, int noweak, uintptr_t *addr, uintptr_t *size, int *weak, int version, const char* vername, int local, int veropt);  // Add bridges to functions

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
linkmap_t* getLinkMapElf(elfheader_t* h);
linkmap_t* addLinkMapLib(library_t* lib);
void removeLinkMapLib(library_t* lib);

int FiniLibrary(library_t* lib, x64emu_t* emu);
void Free1Library(library_t **lib, x64emu_t* emu);

void RemoveDlopen(library_t** lib, size_t idx); // defined in wrappedlibdl.c

#endif //__LIBRARY_PRIVATE_H_
