#define _GNU_SOURCE
#define __USE_GNU
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "debug.h"
#include "library.h"
#include "elfloader.h"
#include "bridge.h"
#include "library_private.h"
#include "khash.h"
#include "box64context.h"
#include "fileutils.h"
#include "librarian.h"
#include "librarian_private.h"
#include "pathcoll.h"
#include "x64emu.h"

#include "wrappedlibs.h"
// create the native lib list
#define GO(P, N) int wrapped##N##_init(library_t* lib, box64context_t *box64); \
                 void wrapped##N##_fini(library_t* lib); \
                 int wrapped##N##_get(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int version, const char* vername, int local); \
                 int wrapped##N##_getnoweak(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int version, const char* vername, int local);
#include "library_list.h"
#undef GO

#define GO(P, N) {P, wrapped##N##_init, wrapped##N##_fini, wrapped##N##_get, wrapped##N##_getnoweak},
wrappedlib_t wrappedlibs[] = {
#include "library_list.h"
};
#undef GO

typedef struct bridged_s {
    char*       name;
    uintptr_t   start;
    uintptr_t   end;
} bridged_t;

KHASH_MAP_INIT_STR(bridgemap, bridged_t)

KHASH_MAP_IMPL_STR(symbolmap, wrapper_t)
KHASH_MAP_IMPL_STR(symbol2map, symbol2_t)
KHASH_MAP_IMPL_STR(datamap, uint64_t)

char* Path2Name(const char* path)
{
    char* name = (char*)calloc(1, MAX_PATH);
    char* p = strrchr(path, '/');
    strcpy(name, (p)?(p+1):path);
    // name should be libXXX.so.A(.BB.CCCC)
    // so keep only 2 dot after ".so" (if any)
    p = strstr(name, ".so");
    if(p) {
        p=strchr(p+3, '.');   // this one is ok
        //if(p) p=strchr(p+1, '.');// this one is ok too
        if(p) p=strchr(p+1, '.');// this one is not
        if(p) *p = '\0';
    }
    return name;
}
int NbDot(const char* name)
{
    char *p = strstr(name, ".so");
    if(!p)
        return -1;
    int ret = 0;
    while(p) {
        p = strchr(p+1, '.');
        if(p) ++ret;
    }
    return ret;
}

void NativeLib_CommonInit(library_t *lib) {
    lib->priv.w.bridge = NewBridge();
    
    lib->symbolmap = kh_init(symbolmap);
    lib->wsymbolmap = kh_init(symbolmap);
    lib->mysymbolmap = kh_init(symbolmap);
    lib->wmysymbolmap = kh_init(symbolmap);
    lib->stsymbolmap = kh_init(symbolmap);
    lib->symbol2map = kh_init(symbol2map);
    lib->datamap = kh_init(datamap);
    lib->wdatamap = kh_init(datamap);
    lib->mydatamap = kh_init(datamap);
}

void EmuLib_Fini(library_t* lib)
{
    kh_destroy(mapsymbols, lib->priv.n.mapsymbols);
    kh_destroy(mapsymbols, lib->priv.n.localsymbols);
}
void NativeLib_FinishFini(library_t* lib)
{
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
}

int WrappedLib_defget(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int version, const char* vername, int local) {
    uintptr_t addr = 0;
    uintptr_t size = 0;
    if (!getSymbolInMaps(lib, name, 0, &addr, &size, version, vername, local)) {
        return 0;
    }
    if(!addr && !size)
        return 0;
    *offs = addr;
    *sz = size;
    return 1;
}
int EmuLib_Get(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int version, const char* vername, int local)
{
    // symbols...
    uintptr_t start, end;
    if(GetSymbolStartEnd(lib->priv.n.mapsymbols, name, &start, &end, version, vername, local))
    {
        *offs = start;
        *sz = end-start;
        return 1;
    }
    // weak symbols...
    if(GetSymbolStartEnd(lib->priv.n.weaksymbols, name, &start, &end, version, vername, local))
    {
        *offs = start;
        *sz = end-start;
        return 1;
    }
    return 0;
}
int WrappedLib_defgetnoweak(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int version, const char* vername, int local) {
    uintptr_t addr = 0;
    uintptr_t size = 0;
    if (!getSymbolInMaps(lib, name, 1, &addr, &size, version, vername, local)) {
        return 0;
    }
    if(!addr && !size)
        return 0;
    *offs = addr;
    *sz = size;
    return 1;
}
int EmuLib_GetNoWeak(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int version, const char* vername, int local)
{
    uintptr_t start, end;
    if(GetSymbolStartEnd(lib->priv.n.mapsymbols, name, &start, &end, version, vername, local))
    {
        *offs = start;
        *sz = end-start;
        return 1;
    }
    return 0;
}
int EmuLib_GetLocal(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int version, const char* vername, int local)
{
    uintptr_t start, end;
    if(GetSymbolStartEnd(lib->priv.n.localsymbols, name, &start, &end, version, vername, local))
    {
        *offs = start;
        *sz = end-start;
        return 1;
    }
    return 0;
}

int NativeLib_GetLocal(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int version, const char* vername, int local)
{
    (void)lib; (void)name; (void)offs; (void)sz; (void)version; (void)vername; (void)local;
    return 0;
}

static void initNativeLib(library_t *lib, box64context_t* context) {
    int nb = sizeof(wrappedlibs) / sizeof(wrappedlib_t);
    for (int i=0; i<nb; ++i) {
        if(strcmp(lib->name, wrappedlibs[i].name)==0) {
            if(wrappedlibs[i].init(lib, context)) {
                // error!
                printf_log(LOG_NONE, "Error initializing native %s (last dlerror is %s)\n", lib->name, dlerror());
                return; // non blocker...
            }
            printf_log(LOG_INFO, "Using native(wrapped) %s\n", lib->name);
            lib->priv.w.box64lib = context->box64lib;
            lib->context = context;
            lib->fini = wrappedlibs[i].fini;
            lib->get = wrappedlibs[i].get;
            lib->getnoweak = wrappedlibs[i].getnoweak;
            lib->getlocal = NativeLib_GetLocal;
            lib->type = 0;
            // Call librarian to load all dependant elf
            if(AddNeededLib(context->maplib, &lib->needed, lib, 0, 0, (const char**)lib->priv.w.neededlibs, lib->priv.w.needed, context, thread_get_emu())) {
                printf_log(LOG_NONE, "Error: loading a needed libs in elf %s\n", lib->name);
                return;
            }
            break;
        }
    }
}

static int loadEmulatedLib(const char* libname, library_t *lib, box64context_t* context)
{
    if(FileExist(libname, IS_FILE))
    {
        FILE *f = fopen(libname, "rb");
        if(!f) {
            printf_log(LOG_NONE, "Error: Cannot open %s\n", libname);
            return 0;
        }
        elfheader_t *elf_header = LoadAndCheckElfHeader(f, libname, 0);
        if(!elf_header) {
            printf_log(LOG_DEBUG, "Error: reading elf header of %s\n", libname);    // this one can be too alarming...
            fclose(f);
            return 0;
        }
        int mainelf = AddElfHeader(context, elf_header);

        if(CalcLoadAddr(elf_header)) {
            printf_log(LOG_NONE, "Error: reading elf header of %s\n", libname);
            fclose(f);
            return 0;
        }
        // allocate memory
        if(AllocElfMemory(context, elf_header, 0)) {
            printf_log(LOG_NONE, "Error: allocating memory for elf %s\n", libname);
            fclose(f);
            return 0;
        }
        // Load elf into memory
        if(LoadElfMemory(f, context, elf_header)) {
            printf_log(LOG_NONE, "Error: loading in memory elf %s\n", libname);
            fclose(f);
            return 0;
        }
        // can close the file now
        fclose(f);

        ElfAttachLib(elf_header, lib);

        lib->type = 1;
        lib->fini = EmuLib_Fini;
        lib->get = EmuLib_Get;
        lib->getnoweak = EmuLib_GetNoWeak;
        lib->getlocal = EmuLib_GetLocal;
        lib->priv.n.elf_index = mainelf;
        lib->priv.n.mapsymbols = kh_init(mapsymbols);
        lib->priv.n.weaksymbols = kh_init(mapsymbols);
        lib->priv.n.localsymbols = kh_init(mapsymbols);

        printf_log(LOG_INFO, "Using emulated %s\n", libname);
        #ifdef DYNAREC
        if(libname && strstr(libname, "libmonobdwgc-2.0.so")) {
            printf_log(LOG_INFO, "MonoBleedingEdge detected, disable Dynarec BigBlock and enable Dynarec StrongMem\n");
            box64_dynarec_bigblock = 0;
            box64_dynarec_strongmem = 1;
        }
        #endif
        return 1;
    }
    return 0;
}

static void initEmulatedLib(const char* path, library_t *lib, box64context_t* context)
{
    char libname[MAX_PATH];
    strcpy(libname, path);
    int found = FileIsX64ELF(libname);
    if(found)
        if(loadEmulatedLib(libname, lib, context))
            return;
    if(!strchr(path, '/'))
        for(int i=0; i<context->box64_ld_lib.size; ++i)
        {
            strcpy(libname, context->box64_ld_lib.paths[i]);
            strcat(libname, path);
            if(FileIsX64ELF(libname))
                if(loadEmulatedLib(libname, lib, context))
                    return;
        }
}

extern char* libGL;
library_t *NewLibrary(const char* path, box64context_t* context)
{
    printf_log(LOG_DEBUG, "Trying to load \"%s\"\n", path);
    library_t *lib = (library_t*)calloc(1, sizeof(library_t));
    lib->path = strdup(path);
    if(libGL && !strcmp(path, libGL))
        lib->name = strdup("libGL.so.1");
    else
        lib->name = Path2Name(path);
    lib->nbdot = NbDot(lib->name);
    lib->context = context;
    lib->type = -1;
    printf_log(LOG_DEBUG, "Simplified name is \"%s\"\n", lib->name);
    if(box64_nopulse) {
        if(strstr(lib->name, "libpulse.so")==lib->name || strstr(lib->name, "libpulse-simple.so")==lib->name) {
            free(lib->name);
            free(lib->path);
            free(lib);
            return NULL;
        }
    }
    if(box64_novulkan) {
        if(strstr(lib->name, "libvulkan.so")==lib->name) {
            free(lib->name);
            free(lib->path);
            free(lib);
            return NULL;
        }
    }
    int notwrapped = FindInCollection(lib->name, &context->box64_emulated_libs);
    int precise = (path && path[0]=='/')?1:0;
    // check if name is libSDL_sound-1.0.so.1 but with SDL2 loaded, then try emulated first...
    if(!notwrapped && !strcmp(lib->name, "libSDL_sound-1.0.so.1") && my_context->sdl2lib)
        notwrapped = 1;
    // And now, actually loading a library
    // look for native(wrapped) libs first
    if(!notwrapped && !precise)
        initNativeLib(lib, context);
    // then look for a native one
    if(lib->type==-1)
        initEmulatedLib(path, lib, context);
    // still not loaded but notwrapped indicated: use wrapped...
    if(lib->type==-1 && notwrapped && !precise)
        initNativeLib(lib, context);
    // nothing loaded, so error...
    if(lib->type==-1)
    {
        free(lib->name);
        free(lib->path);
        free(lib);
        return NULL;
    }

    lib->bridgemap = kh_init(bridgemap);

    return lib;
}
int AddSymbolsLibrary(lib_t *maplib, library_t* lib, x64emu_t* emu)
{
    (void)emu;

    lib->active = 1;
    if(lib->type==1) {
        elfheader_t *elf_header = lib->context->elfs[lib->priv.n.elf_index];
        // add symbols
        AddSymbols(maplib, lib->priv.n.mapsymbols, lib->priv.n.weaksymbols, lib->priv.n.localsymbols, elf_header);
    }
    return 0;
}
int FinalizeLibrary(library_t* lib, lib_t* local_maplib, int bindnow, x64emu_t* emu)
{
    if(!lib)
        return 0;
    if(lib->type==1) {
        if(lib->priv.n.finalized)
            return 0;
        lib->priv.n.finalized = 1;
        elfheader_t *elf_header = my_context->elfs[lib->priv.n.elf_index];
        // finalize relocations
        if(RelocateElf(my_context->maplib, local_maplib, bindnow, elf_header)) {
            printf_log(LOG_NONE, "Error: relocating symbols in elf %s\n", lib->name);
            return 1;
        }
        if(RelocateElfPlt(my_context->maplib, local_maplib, bindnow, elf_header)) {
            printf_log(LOG_NONE, "Error: relocating Plt symbols in elf %s\n", lib->name);
            return 1;
        }
#ifdef HAVE_TRACE
        if(trace_func) {
            if (GetGlobalSymbolStartEnd(local_maplib, trace_func, &trace_start, &trace_end, elf_header, -1, NULL)) {
                SetTraceEmu(trace_start, trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", trace_func, (void*)trace_start, (void*)trace_end);
                free(trace_func);
                trace_func = NULL;
            } else if(GetLibLocalSymbolStartEnd(lib, trace_func, &trace_start, &trace_end, -1, NULL, 0)) {
                SetTraceEmu(trace_start, trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", trace_func, (void*)trace_start, (void*)trace_end);
                free(trace_func);
                trace_func = NULL;
            }
        }
#endif
        RunElfInit(elf_header, emu);
    }
    return 0;
}

int ReloadLibrary(library_t* lib, x64emu_t* emu)
{
    lib->active = 1;
    if(lib->type==1) {
        elfheader_t *elf_header = lib->context->elfs[lib->priv.n.elf_index];
        // reload image in memory and re-run the mapping
        char libname[MAX_PATH];
        strcpy(libname, lib->path);
        int found = FileExist(libname, IS_FILE);
        if(!found && !strchr(lib->path, '/'))
            for(int i=0; i<lib->context->box64_ld_lib.size; ++i)
            {
                strcpy(libname, lib->context->box64_ld_lib.paths[i]);
                strcat(libname, lib->path);
                if(FileExist(libname, IS_FILE))
                    break;
            }
        if(!FileExist(libname, IS_FILE)) {
            printf_log(LOG_NONE, "Error: open file to re-load elf %s\n", libname);
            return 1;   // failed to reload...
        }
        FILE *f = fopen(libname, "rb");
        if(!f) {
            printf_log(LOG_NONE, "Error: cannot open file to re-load elf %s (errno=%d/%s)\n", libname, errno, strerror(errno));
            return 1;   // failed to reload...
        }
        if(ReloadElfMemory(f, lib->context, elf_header)) {
            printf_log(LOG_NONE, "Error: re-loading in memory elf %s\n", libname);
            fclose(f);
            return 1;
        }
        // can close the file now
        fclose(f);
        // should bindnow be store in a per/library basis?
        if(RelocateElf(lib->context->maplib, lib->maplib, 0, elf_header)) {
            printf_log(LOG_NONE, "Error: relocating symbols in elf %s\n", lib->name);
            return 1;
        }
        RelocateElfPlt(lib->context->maplib, lib->maplib, 0, elf_header);
        // init (will use PltRelocator... because some other libs are not yet resolved)
        RunElfInit(elf_header, emu);
    }
    return 0;
}

void InactiveLibrary(library_t* lib)
{
    lib->active = 0;
}

void Free1Library(library_t **lib, x64emu_t* emu)
{
    if(!(*lib)) return;

    if((*lib)->type==1 && emu) {
        elfheader_t *elf_header = (*lib)->context->elfs[(*lib)->priv.n.elf_index];
        RunElfFini(elf_header, emu);
    }

    if((*lib)->maplib)
        FreeLibrarian(&(*lib)->maplib, emu);

    if((*lib)->type!=-1 && (*lib)->fini) {
        (*lib)->fini(*lib);
    }
    free((*lib)->name);
    free((*lib)->path);
    free((*lib)->altmy);

    if((*lib)->bridgemap) {
        bridged_t *br;
        kh_foreach_value_ref((*lib)->bridgemap, br,
            free(br->name);
        );
        kh_destroy(bridgemap, (*lib)->bridgemap);
    }
    if((*lib)->symbolmap)
        kh_destroy(symbolmap, (*lib)->symbolmap);
    if((*lib)->wsymbolmap)
        kh_destroy(symbolmap, (*lib)->wsymbolmap);
    if((*lib)->datamap)
        kh_destroy(datamap, (*lib)->datamap);
    if((*lib)->wdatamap)
        kh_destroy(datamap, (*lib)->wdatamap);
    if((*lib)->mydatamap)
        kh_destroy(datamap, (*lib)->mydatamap);
    if((*lib)->mysymbolmap)
        kh_destroy(symbolmap, (*lib)->mysymbolmap);
    if((*lib)->wmysymbolmap)
        kh_destroy(symbolmap, (*lib)->wmysymbolmap);
    if((*lib)->stsymbolmap)
        kh_destroy(symbolmap, (*lib)->stsymbolmap);
    if((*lib)->symbol2map)
        kh_destroy(symbol2map, (*lib)->symbol2map);
    free_neededlib(&(*lib)->needed);
    free_neededlib(&(*lib)->depended);

    free(*lib);
    *lib = NULL;
}

char* GetNameLib(library_t *lib)
{
    return lib->name;
}
int IsSameLib(library_t* lib, const char* path)
{
    int ret = 0;
    if(!lib) 
        return 0;
    char* name = Path2Name(path);
    if(strcmp(name, lib->name)==0)
        ret=1;
    else {
        int n = NbDot(name);
        if(n>=0 && n<lib->nbdot)
            if(strncmp(name, lib->name, strlen(name))==0)
                ret=1;
    }

    free(name);
    return ret;
}
int GetLibSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, int version, const char* vername, int local)
{
    if(!name[0] || !lib->active)
        return 0;
    khint_t k;
    // check first if already in the map
    k = kh_get(bridgemap, lib->bridgemap, VersionnedName(name, version, vername));
    if(k!=kh_end(lib->bridgemap)) {
        *start = kh_value(lib->bridgemap, k).start;
        *end = kh_value(lib->bridgemap, k).end;
        return 1;
    }
    // get a new symbol
    if(lib->get(lib, name, start, end, version, vername, local)) {
        *end += *start;     // lib->get(...) gives size, not end
        char* symbol = strdup(VersionnedName(name, version, vername));
        int ret;
        k = kh_put(bridgemap, lib->bridgemap, symbol, &ret);
        kh_value(lib->bridgemap, k).name = symbol;
        kh_value(lib->bridgemap, k).start = *start;
        kh_value(lib->bridgemap, k).end = *end;
        return 1;
    }
    // nope
    return 0;
}
int GetLibNoWeakSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, int version, const char* vername, int local)
{
    if(!name[0] || !lib->active)
        return 0;
    khint_t k;
    // get a new symbol
    if(lib->getnoweak(lib, name, start, end, version, vername, local)) {
        *end += *start;     // lib->get(...) gives size, not end
        // check if already in the map
        k = kh_get(bridgemap, lib->bridgemap, VersionnedName(name, version, vername));
        if(k!=kh_end(lib->bridgemap)) {
            *start = kh_value(lib->bridgemap, k).start;
            *end = kh_value(lib->bridgemap, k).end;
            return 1;
        }
        char* symbol = strdup(VersionnedName(name, version, vername));
        int ret;
        k = kh_put(bridgemap, lib->bridgemap, symbol, &ret);
        kh_value(lib->bridgemap, k).name = symbol;
        kh_value(lib->bridgemap, k).start = *start;
        kh_value(lib->bridgemap, k).end = *end;
        return 1;
    }
    // nope
    return 0;
}
int GetLibLocalSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, int version, const char* vername, int local)
{
    if(!name[0] || !lib->active)
        return 0;
    khint_t k;
    // check first if already in the map
    k = kh_get(bridgemap, lib->bridgemap, VersionnedName(name, version, vername));
    if(k!=kh_end(lib->bridgemap)) {
        *start = kh_value(lib->bridgemap, k).start;
        *end = kh_value(lib->bridgemap, k).end;
        return 1;
    }
    // get a new symbol
    if(lib->getlocal(lib, name, start, end, version, vername, local)) {
        *end += *start;     // lib->get(...) gives size, not end
        char* symbol = strdup(VersionnedName(name, version, vername));
        int ret;
        k = kh_put(bridgemap, lib->bridgemap, symbol, &ret);
        kh_value(lib->bridgemap, k).name = symbol;
        kh_value(lib->bridgemap, k).start = *start;
        kh_value(lib->bridgemap, k).end = *end;
        return 1;
    }
    // nope
    return 0;
}
int GetElfIndex(library_t* lib)
{
    if(!lib || lib->type!=1)
        return -1;
    return lib->priv.n.elf_index;
}

static int getSymbolInDataMaps(library_t*lib, const char* name, int noweak, uintptr_t *addr, uintptr_t *size)
{
    void* symbol;
    khint_t k = kh_get(datamap, lib->datamap, name);
    if (k!=kh_end(lib->datamap)) {
        symbol = dlsym(lib->priv.w.lib, kh_key(lib->datamap, k));
        if(symbol) {
            // found!
            *addr = (uintptr_t)symbol;
            *size = kh_value(lib->datamap, k);
            return 1;
        }
    }
    if(!noweak) {
        k = kh_get(datamap, lib->wdatamap, name);
        if (k!=kh_end(lib->wdatamap)) {
            symbol = dlsym(lib->priv.w.lib, kh_key(lib->wdatamap, k));
            if(symbol) {
                // found!
                *addr = (uintptr_t)symbol;
                *size = kh_value(lib->wdatamap, k);
                return 1;
            }
        }
    }
    // check in mydatamap
    k = kh_get(datamap, lib->mydatamap, name);
    if (k!=kh_end(lib->mydatamap)) {
        char buff[200];
        if(lib->altmy)
            strcpy(buff, lib->altmy);
        else
            strcpy(buff, "my_");
        strcat(buff, name);
        symbol = dlsym(lib->priv.w.box64lib, buff);
        if(!symbol)
            printf_log(LOG_NONE, "Warning, data %s not found\n", buff);
        if(symbol) {
            // found!
            *addr = (uintptr_t)symbol;
            *size = kh_value(lib->mydatamap, k);
            return 1;
        }
    }
    return 0;
}
static int getSymbolInSymbolMaps(library_t*lib, const char* name, int noweak, uintptr_t *addr, uintptr_t *size)
{
    void* symbol;
    // check in mysymbolmap
    khint_t k = kh_get(symbolmap, lib->mysymbolmap, name);
    if (k!=kh_end(lib->mysymbolmap)) {
        char buff[200];
        if(lib->altmy)
            strcpy(buff, lib->altmy);
        else
            strcpy(buff, "my_");
        strcat(buff, name);
        symbol = dlsym(lib->priv.w.box64lib, buff);
        if(!symbol) {
            printf_log(LOG_NONE, "Warning, function %s not found\n", buff);
        } else 
            AddOffsetSymbol(lib->context->maplib, symbol, name);
        *addr = AddBridge(lib->priv.w.bridge, kh_value(lib->mysymbolmap, k), symbol, 0, name);
        *size = sizeof(void*);
        return 1;
    }
    // check in stsymbolmap (return struct...)
    k = kh_get(symbolmap, lib->stsymbolmap, name);
    if (k!=kh_end(lib->stsymbolmap)) {
        char buff[200];
        if(lib->altmy)
            strcpy(buff, lib->altmy);
        else
            strcpy(buff, "my_");
        strcat(buff, name);
        symbol = dlsym(lib->priv.w.box64lib, buff);
        if(!symbol) {
            printf_log(LOG_NONE, "Warning, function %s not found\n", buff);
        } else 
            AddOffsetSymbol(lib->context->maplib, symbol, name);
        *addr = AddBridge(lib->priv.w.bridge, kh_value(lib->stsymbolmap, k), symbol, sizeof(void*), name);
        *size = sizeof(void*);
        return 1;
    }
    // check in symbolmap
    k = kh_get(symbolmap, lib->symbolmap, name);
    if (k!=kh_end(lib->symbolmap)) {
        symbol = dlsym(lib->priv.w.lib, name);
        if(!symbol && lib->priv.w.altprefix) {
            char newname[200];
            strcpy(newname, lib->priv.w.altprefix);
            strcat(newname, name);
            symbol = dlsym(lib->priv.w.lib, newname);
        }
        if(!symbol) {
            printf_log(LOG_INFO, "Warning, function %s not found in lib %s\n", name, lib->name);
            return 0;
        } else 
            AddOffsetSymbol(lib->context->maplib, symbol, name);
        *addr = AddBridge(lib->priv.w.bridge, kh_value(lib->symbolmap, k), symbol, 0, name);
        *size = sizeof(void*);
        return 1;
    }
    if(!noweak) {
        // check in wmysymbolmap
        khint_t k = kh_get(symbolmap, lib->wmysymbolmap, name);
        if (k!=kh_end(lib->wmysymbolmap)) {
            char buff[200];
            if(lib->altmy)
                strcpy(buff, lib->altmy);
            else
                strcpy(buff, "my_");
            strcat(buff, name);
            symbol = dlsym(lib->priv.w.box64lib, buff);
            if(!symbol) {
                printf_log(LOG_NONE, "Warning, function %s not found\n", buff);
            } else 
                AddOffsetSymbol(lib->context->maplib, symbol, name);
            *addr = AddBridge(lib->priv.w.bridge, kh_value(lib->wmysymbolmap, k), symbol, 0, name);
            *size = sizeof(void*);
            return 1;
        }
        k = kh_get(symbolmap, lib->wsymbolmap, name);
        if (k!=kh_end(lib->wsymbolmap)) {
            symbol = dlsym(lib->priv.w.lib, name);
            if(!symbol && lib->priv.w.altprefix) {
                char newname[200];
                strcpy(newname, lib->priv.w.altprefix);
                strcat(newname, name);
                symbol = dlsym(lib->priv.w.lib, newname);
            }
            if(!symbol) {
                printf_log(LOG_INFO, "Warning, function %s not found in lib %s\n", name, lib->name);
                return 0;
            } else 
                AddOffsetSymbol(lib->context->maplib, symbol, name);
            *addr = AddBridge(lib->priv.w.bridge, kh_value(lib->wsymbolmap, k), symbol, 0, name);
            *size = sizeof(void*);
            return 1;
        }
    }
    // check in symbol2map
    k = kh_get(symbol2map, lib->symbol2map, name);
    if (k!=kh_end(lib->symbol2map)) 
        if(!noweak || !kh_value(lib->symbol2map, k).weak)
        {
            symbol = dlsym(lib->priv.w.lib, kh_value(lib->symbol2map, k).name);
            if(!symbol)
                symbol = dlsym(RTLD_DEFAULT, kh_value(lib->symbol2map, k).name);    // search globaly maybe
            if(!symbol) {
                printf_log(LOG_INFO, "Warning, function %s not found in lib %s\n", kh_value(lib->symbol2map, k).name, lib->name);
                return 0;
            } else 
                AddOffsetSymbol(lib->context->maplib, symbol, name);
            *addr = AddBridge(lib->priv.w.bridge, kh_value(lib->symbol2map, k).w, symbol, 0, name);
            *size = sizeof(void*);
            return 1;
        }

    return 0;
}

int getSymbolInMaps(library_t *lib, const char* name, int noweak, uintptr_t *addr, uintptr_t *size, int version, const char* vername, int local)
{
    if(!lib->active)
        return 0;
    // check in datamaps (but no version, it's not handled there)
    if(getSymbolInDataMaps(lib, name, noweak, addr, size))
        return 1;

    if(getSymbolInSymbolMaps(lib, VersionnedName(name, version, vername), noweak, addr, size))
        return 1;

    if(getSymbolInSymbolMaps(lib, name, noweak, addr, size))
        return 1;

    return 0;
}

int GetNeededLibN(library_t* lib) {
    return lib->needed.size;
}
library_t* GetNeededLib(library_t* lib, int idx)
{
    if(idx<0 || idx>=lib->needed.size)
        return NULL;
    return lib->needed.libs[idx];
}
needed_libs_t* GetNeededLibs(library_t* lib)
{
    return &lib->needed;
}

void* GetHandle(library_t* lib)
{
    if(!lib)
        return NULL;
    if(lib->type!=0)
        return NULL;
    return lib->priv.w.lib;
}

lib_t* GetMaplib(library_t* lib)
{
    if(!lib)
        return NULL;
    return lib->maplib;
}
