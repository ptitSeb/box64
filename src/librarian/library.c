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
#include <link.h>
#include <stdarg.h>

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
#include "symbols.h"
#include "elfs/elfloader_private.h"
#include "library_inner.h"

#include "wrappedlibs.h"
// create the native lib list
#define GO(P, N) int wrapped##N##_init(library_t* lib, box64context_t *box64); \
                 void wrapped##N##_fini(library_t* lib);

#include "library_list.h"

#undef GO

#define GO(P, N) {P, wrapped##N##_init, wrapped##N##_fini},
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
    char* name = (char*)box_calloc(1, MAX_PATH);
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

void WrappedLib_CommonInit(library_t *lib) {
    lib->w.bridge = NewBridge();
    
    lib->w.symbolmap = kh_init(symbolmap);
    lib->w.wsymbolmap = kh_init(symbolmap);
    lib->w.mysymbolmap = kh_init(symbolmap);
    lib->w.wmysymbolmap = kh_init(symbolmap);
    lib->w.stsymbolmap = kh_init(symbolmap);
    lib->w.symbol2map = kh_init(symbol2map);
    lib->w.datamap = kh_init(datamap);
    lib->w.wdatamap = kh_init(datamap);
    lib->w.mydatamap = kh_init(datamap);
}

void EmuLib_Fini(library_t* lib)
{
}
void WrappedLib_FinishFini(library_t* lib)
{
    if(lib->w.lib)
        dlclose(lib->w.lib);
    lib->w.lib = NULL;
    if(lib->w.altprefix)
        box_free(lib->w.altprefix);
    if(lib->w.altmy)
        box_free(lib->w.altmy);
    if(lib->w.neededlibs) {
        for(int i=0; i<lib->w.needed; ++i)
            box_free(lib->w.neededlibs[i]);
        box_free(lib->w.neededlibs);
    }
    FreeBridge(&lib->w.bridge);
}

int WrappedLib_GetWeak(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int* weak, int version, const char* vername, int local) {
    uintptr_t addr = 0;
    uintptr_t size = 0;
    int wk = 0;
    if (!getSymbolInMaps(lib, name, 0, &addr, &size, &wk, version, vername, local)) {
        return 0;
    }
    if(!addr && !size)
        return 0;
    if(!wk) // found a global symbol
        return 0;
    *offs = addr;
    *sz = size;
    *weak = wk;
    return 1;
}
int EmuLib_GetWeak(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int*weak, int version, const char* vername, int local)
{
    // symbols...
    uintptr_t start, end;
    const char* defver = GetDefaultVersion(my_context->weakdefver, name);
    // weak symbols...
    if(GetSymbolStartEnd(GetWeakSymbols(lib->e.elf), name, &start, &end, version, vername, local, defver))
    {
        *offs = start;
        *sz = end-start;
        *weak = 1;
        return 1;
    }
    return 0;
}
int WrappedLib_GetGlobal(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int* weak, int version, const char* vername, int local) {
    uintptr_t addr = 0;
    uintptr_t size = 0;
    int wk = 0;
    if (!getSymbolInMaps(lib, name, 1, &addr, &size, &wk, version, vername, local)) {
        return 0;
    }
    if(!addr && !size)
        return 0;
    if(wk)
        return 0;   //found a weak symbol
    *offs = addr;
    *sz = size;
    *weak = 0;
    return 1;
}
int EmuLib_GetGlobal(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int* weak, int version, const char* vername, int local)
{
    uintptr_t start, end;
    const char* defver = GetDefaultVersion(my_context->globaldefver, name);
    if(GetSymbolStartEnd(GetMapSymbols(lib->e.elf), name, &start, &end, version, vername, local, defver))
    {
        *offs = start;
        *sz = end-start;
        *weak = 0;
        return 1;
    }
    return 0;
}
int EmuLib_GetLocal(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int* weak, int version, const char* vername, int local)
{
    uintptr_t start, end;
    const char* defver = GetDefaultVersion(my_context->globaldefver, name);
    if(!defver) defver = GetDefaultVersion(my_context->weakdefver, name);
    if(GetSymbolStartEnd(GetLocalSymbols(lib->e.elf), name, &start, &end, version, vername, local, defver))
    {
        *offs = start;
        *sz = end-start;
        *weak = 0;
        return 1;
    }
    return 0;
}

int WrappedLib_GetLocal(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, int* weak, int version, const char* vername, int local)
{
    (void)lib; (void)name; (void)offs; (void)sz; (void)version; (void)vername; (void)local;
    return 0;
}

static void initWrappedLib(library_t *lib, box64context_t* context) {
    int nb = sizeof(wrappedlibs) / sizeof(wrappedlib_t);
    for (int i=0; i<nb; ++i) {
        if(strcmp(lib->name, wrappedlibs[i].name)==0) {
            if(wrappedlibs[i].init(lib, context)) {
                // error!
                const char* error_str = dlerror();
                if(error_str)   // don't print the message if there is no error string from last error
                    printf_log(LOG_NONE, "Error initializing native %s (last dlerror is %s)\n", lib->name, error_str);
                return; // non blocker...
            }
            printf_log(LOG_INFO, "Using native(wrapped) %s\n", lib->name);
            lib->fini = wrappedlibs[i].fini;
            lib->getglobal = WrappedLib_GetGlobal;
            lib->getweak = WrappedLib_GetWeak;
            lib->getlocal = WrappedLib_GetLocal;
            lib->type = LIB_WRAPPED;
            // Call librarian to load all dependant elf
            if(AddNeededLib(context->maplib, &lib->needed, lib, 0, 0, (const char**)lib->w.neededlibs, lib->w.needed, context, thread_get_emu())) {
                printf_log(LOG_NONE, "Error: loading a needed libs in elf %s\n", lib->name);
                return;
            }

            linkmap_t *lm = addLinkMapLib(lib);
            if(!lm) {
                // Crashed already
                printf_log(LOG_DEBUG, "Failure to add lib %s linkmap\n", lib->name);
                break;
            }
            struct link_map real_lm;
            if(dlinfo(lib->w.lib, RTLD_DI_LINKMAP, &real_lm)) {
                printf_log(LOG_DEBUG, "Failed to dlinfo lib %s\n", lib->name);
            }
            lm->l_addr = real_lm.l_addr;
            lm->l_name = real_lm.l_name;
            lm->l_ld = real_lm.l_ld;
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

        lib->type = LIB_EMULATED;
        lib->fini = EmuLib_Fini;
        lib->getglobal = EmuLib_GetGlobal;
        lib->getweak = EmuLib_GetWeak;
        lib->getlocal = EmuLib_GetLocal;
        lib->e.elf_index = mainelf;
        lib->e.elf = elf_header;

        if(lib->path && strcmp(lib->path, libname)) {
            box_free(lib->path);
            lib->path = realpath(libname, NULL);
            if(!lib->path)
                lib->path = box_strdup(libname);
        }

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

static const char* essential_libs[] = {
    "libc.so.6", "libpthread.so.0", "librt.so.1", "libGL.so.1", "libGL.so", "libX11.so.6", 
    "libasound.so.2", "libdl.so.2", "libm.so.6",
    "libXxf86vm.so.1", "libXinerama.so.1", "libXrandr.so.2", "libXext.so.6", "libXfixes.so.3", "libXcursor.so.1",
    "libXrender.so.1", "libXft.so.2", "libXi.so.6", "libXss.so.1", "libXpm.so.4", "libXau.so.6", "libXdmcp.so.6",
    "libX11-xcb.so.1", "libxcb.so.1", "libxcb-xfixes.so.0", "libxcb-shape.so.0", "libxcb-shm.so.0", "libxcb-randr.so.0",
    "libxcb-image.so.0", "libxcb-keysyms.so.1", "libxcb-xtest.so.0", "libxcb-glx.so.0", "libxcb-dri2.so.0", "libxcb-dri3.so.0",
    "libXtst.so.6", "libXt.so.6", "libXcomposite.so.1", "libXdamage.so.1", "libXmu.so.6", "libxkbcommon.so.0", 
    "libxkbcommon-x11.so.0", "libpulse-simple.so.0", "libpulse.so.0", "libvulkan.so.1", "libvulkan.so",
    "ld-linux-x86-64.so.2", "crashhandler.so", "libtcmalloc_minimal.so.0", "libtcmalloc_minimal.so.4"
};
static int isEssentialLib(const char* name) {
    for (int i=0; i<sizeof(essential_libs)/sizeof(essential_libs[0]); ++i)
        if(!strcmp(name, essential_libs[i]))
            return 1;
    return 0;
}

extern char* libGL;
library_t *NewLibrary(const char* path, box64context_t* context)
{
    printf_log(LOG_DEBUG, "Trying to load \"%s\"\n", path);
    library_t *lib = (library_t*)box_calloc(1, sizeof(library_t));
    lib->path = realpath(path, NULL);
    if(!lib->path)
        lib->path = box_strdup(path);
    if(libGL && !strcmp(path, libGL))
        lib->name = box_strdup("libGL.so.1");
    else
        lib->name = Path2Name(path);
    lib->nbdot = NbDot(lib->name);
    lib->type = LIB_UNNKNOW;
    printf_log(LOG_DEBUG, "Simplified name is \"%s\"\n", lib->name);
    if(box64_nopulse) {
        if(strstr(lib->name, "libpulse.so")==lib->name || strstr(lib->name, "libpulse-simple.so")==lib->name) {
            box_free(lib->name);
            box_free(lib->path);
            box_free(lib);
            return NULL;
        }
    }
    if(box64_novulkan) {
        if(strstr(lib->name, "libvulkan.so")==lib->name) {
            box_free(lib->name);
            box_free(lib->path);
            box_free(lib);
            return NULL;
        }
    }
    int notwrapped = FindInCollection(lib->name, &context->box64_emulated_libs);
    int essential = isEssentialLib(lib->name);
    if(!notwrapped && box64_prefer_emulated && !essential)
        notwrapped = 1;
    int precise = (!box64_prefer_wrapped && !essential && path && strchr(path, '/'))?1:0;
    if(!notwrapped && precise && strstr(path, "libtcmalloc_minimal.so"))
        precise = 0;    // allow native version for tcmalloc_minimum
    // check if name is libSDL_sound-1.0.so.1 but with SDL2 loaded, then try emulated first...
    if(!notwrapped && !strcmp(lib->name, "libSDL_sound-1.0.so.1") && my_context->sdl2lib)
        notwrapped = 1;
    // And now, actually loading a library
    // look for native(wrapped) libs first
    if(!notwrapped && !precise)
        initWrappedLib(lib, context);
    // then look for a native one
    if(lib->type==LIB_UNNKNOW)
        initEmulatedLib(path, lib, context);
    // still not loaded but notwrapped indicated: use wrapped...
    if(lib->type==LIB_UNNKNOW && notwrapped && !precise)
        initWrappedLib(lib, context);
    // nothing loaded, so error...
    if(lib->type==LIB_UNNKNOW)
    {
        box_free(lib->name);
        box_free(lib->path);
        box_free(lib);
        return NULL;
    }
    lib->gbridgemap = kh_init(bridgemap);
    lib->wbridgemap = kh_init(bridgemap);
    lib->lbridgemap = kh_init(bridgemap);

    return lib;
}
int AddSymbolsLibrary(lib_t *maplib, library_t* lib, x64emu_t* emu)
{
    (void)emu;

    lib->active = 1;
    if(lib->type==LIB_EMULATED) {
        elfheader_t *elf_header = lib->e.elf;
        // add symbols
        AddSymbols(maplib, GetMapSymbols(lib->e.elf), GetWeakSymbols(lib->e.elf), GetLocalSymbols(lib->e.elf), elf_header);
    }
    return 0;
}
int FinalizeLibrary(library_t* lib, lib_t* local_maplib, int bindnow, x64emu_t* emu)
{
    if(!lib)
        return 0;
    if(lib->type==LIB_EMULATED) {
        if(lib->e.finalized)
            return 0;
        lib->e.finalized = 1;
        elfheader_t *elf_header = my_context->elfs[lib->e.elf_index];
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
            int weak;
            if (GetGlobalSymbolStartEnd(local_maplib, trace_func, &trace_start, &trace_end, elf_header, -1, NULL)) {
                SetTraceEmu(trace_start, trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", trace_func, (void*)trace_start, (void*)trace_end);
                box_free(trace_func);
                trace_func = NULL;
            } else if(GetLibLocalSymbolStartEnd(lib, trace_func, &trace_start, &trace_end, &weak, -1, NULL, 0)) {
                SetTraceEmu(trace_start, trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", trace_func, (void*)trace_start, (void*)trace_end);
                box_free(trace_func);
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
    if(lib->type==LIB_EMULATED) {
        elfheader_t *elf_header = lib->e.elf;
        // reload image in memory and re-run the mapping
        char libname[MAX_PATH];
        strcpy(libname, lib->path);
        int found = FileExist(libname, IS_FILE);
        if(!found && !strchr(lib->path, '/'))
            for(int i=0; i<my_context->box64_ld_lib.size; ++i)
            {
                strcpy(libname, my_context->box64_ld_lib.paths[i]);
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
        if(ReloadElfMemory(f, my_context, elf_header)) {
            printf_log(LOG_NONE, "Error: re-loading in memory elf %s\n", libname);
            fclose(f);
            return 1;
        }
        // can close the file now
        fclose(f);
        // should bindnow be store in a per/library basis?
        if(RelocateElf(my_context->maplib, lib->maplib, 0, elf_header)) {
            printf_log(LOG_NONE, "Error: relocating symbols in elf %s\n", lib->name);
            return 1;
        }
        RelocateElfPlt(my_context->maplib, lib->maplib, 0, elf_header);
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

    if((*lib)->type==LIB_EMULATED && emu) {
        elfheader_t *elf_header = (*lib)->e.elf;
        RunElfFini(elf_header, emu);
    }

    if((*lib)->maplib)
        FreeLibrarian(&(*lib)->maplib, emu);

    if((*lib)->type!=LIB_UNNKNOW && (*lib)->fini) {
        (*lib)->fini(*lib);
    }
    box_free((*lib)->name);
    box_free((*lib)->path);

    if((*lib)->gbridgemap) {
        bridged_t *br;
        kh_foreach_value_ref((*lib)->gbridgemap, br,
            free(br->name);
        );
        kh_destroy(bridgemap, (*lib)->gbridgemap);
    }
    if((*lib)->wbridgemap) {
        bridged_t *br;
        kh_foreach_value_ref((*lib)->wbridgemap, br,
            free(br->name);
        );
        kh_destroy(bridgemap, (*lib)->wbridgemap);
    }
    if((*lib)->lbridgemap) {
        bridged_t *br;
        kh_foreach_value_ref((*lib)->lbridgemap, br,
            free(br->name);
        );
        kh_destroy(bridgemap, (*lib)->lbridgemap);
    }
    if((*lib)->type == LIB_WRAPPED) {
        if((*lib)->w.symbolmap)
            kh_destroy(symbolmap, (*lib)->w.symbolmap);
        if((*lib)->w.wsymbolmap)
            kh_destroy(symbolmap, (*lib)->w.wsymbolmap);
        if((*lib)->w.datamap)
            kh_destroy(datamap, (*lib)->w.datamap);
        if((*lib)->w.wdatamap)
            kh_destroy(datamap, (*lib)->w.wdatamap);
        if((*lib)->w.mydatamap)
            kh_destroy(datamap, (*lib)->w.mydatamap);
        if((*lib)->w.mysymbolmap)
            kh_destroy(symbolmap, (*lib)->w.mysymbolmap);
        if((*lib)->w.wmysymbolmap)
            kh_destroy(symbolmap, (*lib)->w.wmysymbolmap);
        if((*lib)->w.stsymbolmap)
            kh_destroy(symbolmap, (*lib)->w.stsymbolmap);
        if((*lib)->w.symbol2map)
            kh_destroy(symbol2map, (*lib)->w.symbol2map);
    }
    free_neededlib(&(*lib)->needed);
    free_neededlib(&(*lib)->dependedby);

    box_free(*lib);
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
    if(!strchr(path, '/') || lib->type==LIB_WRAPPED || !lib->path) {
        if(strcmp(name, lib->name)==0)
            ret=1;
    } else {
        char rpath[PATH_MAX];
        realpath(path, rpath);
        if(!strcmp(rpath, lib->path))
            ret=1;
    }
    if(!ret) {
        int n = NbDot(name);
        if(n>=0 && n<lib->nbdot)
            if(strncmp(name, lib->name, strlen(name))==0)
                ret=1;
    }

    box_free(name);
    return ret;
}
int GetLibWeakSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, int* weak, int version, const char* vername, int local)
{
    if(!name[0] || !lib->active)
        return 0;
    khint_t k;
    // check first if already in the map
    k = kh_get(bridgemap, lib->wbridgemap, VersionnedName(name, version, vername));
    if(k!=kh_end(lib->wbridgemap)) {
        *start = kh_value(lib->wbridgemap, k).start;
        *end = kh_value(lib->wbridgemap, k).end;
        *weak = 1;
        return 1;
    }
    if(local) {
        k = kh_get(bridgemap, lib->lbridgemap, VersionnedName(name, version, vername));
        if(k!=kh_end(lib->lbridgemap)) {
            *start = kh_value(lib->lbridgemap, k).start;
            *end = kh_value(lib->lbridgemap, k).end;
            *weak = 0;
            return 1;
        }
    }
    // get a new symbol
    if(lib->getweak(lib, name, start, end, weak, version, vername, local)) {
        *end += *start;     // lib->get(...) gives size, not end
        char* symbol = box_strdup(VersionnedName(name, version, vername));
        int ret;
        kh_bridgemap_t *map = local?lib->lbridgemap:((*weak)?lib->wbridgemap:lib->gbridgemap);
        k = kh_put(bridgemap, map, symbol, &ret);
        kh_value(map, k).name = symbol;
        kh_value(map, k).start = *start;
        kh_value(map, k).end = *end;
        return 1;
    }
    // nope
    return 0;
}
int GetLibGlobalSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, int* weak, int version, const char* vername, int local)
{
    if(!name[0] || !lib->active)
        return 0;
    khint_t k;
    // get a new symbol
    if(lib->getglobal(lib, name, start, end, weak, version, vername, local)) {
        *end += *start;     // lib->get(...) gives size, not end
        kh_bridgemap_t *map = local?lib->lbridgemap:((*weak)?lib->wbridgemap:lib->gbridgemap);
        // check if already in the map
        k = kh_get(bridgemap, map, VersionnedName(name, version, vername));
        if(k!=kh_end(map)) {
            *start = kh_value(map, k).start;
            *end = kh_value(map, k).end;
            return 1;
        }
        char* symbol = box_strdup(VersionnedName(name, version, vername));
        int ret;
        k = kh_put(bridgemap, map, symbol, &ret);
        kh_value(map, k).name = symbol;
        kh_value(map, k).start = *start;
        kh_value(map, k).end = *end;
        return 1;
    }
    // nope
    return 0;
}
int GetLibLocalSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, int* weak, int version, const char* vername, int local)
{
    if(!name[0] || !lib->active)
        return 0;
    khint_t k;
    // check first if already in the map
    k = kh_get(bridgemap, lib->lbridgemap, VersionnedName(name, version, vername));
    if(k!=kh_end(lib->lbridgemap)) {
        *start = kh_value(lib->lbridgemap, k).start;
        *end = kh_value(lib->lbridgemap, k).end;
        return 1;
    }
    // get a new symbol
    if(lib->getlocal(lib, name, start, end, weak, version, vername, local)) {
        *end += *start;     // lib->get(...) gives size, not end
        char* symbol = box_strdup(VersionnedName(name, version, vername));
        int ret;
        k = kh_put(bridgemap, lib->lbridgemap, symbol, &ret);
        kh_value(lib->lbridgemap, k).name = symbol;
        kh_value(lib->lbridgemap, k).start = *start;
        kh_value(lib->lbridgemap, k).end = *end;
        return 1;
    }
    // nope
    return 0;
}
int GetElfIndex(library_t* lib)
{
    if(!lib || lib->type!=LIB_EMULATED)
        return -1;
    return lib->e.elf_index;
}
elfheader_t* GetElf(library_t* lib)
{
    if(!lib || lib->type!=LIB_EMULATED)
        return NULL;
    return lib->e.elf;
}

static int getSymbolInDataMaps(library_t*lib, const char* name, int noweak, uintptr_t *addr, uintptr_t *size, int* weak)
{
    void* symbol;
    khint_t k = kh_get(datamap, lib->w.datamap, name);
    if (k!=kh_end(lib->w.datamap)) {
        symbol = dlsym(lib->w.lib, kh_key(lib->w.datamap, k));
        if(symbol) {
            // found!
            *addr = (uintptr_t)symbol;
            *size = kh_value(lib->w.datamap, k);
            *weak = 0;
            return 1;
        }
    }
    if(!noweak) {
        k = kh_get(datamap, lib->w.wdatamap, name);
        if (k!=kh_end(lib->w.wdatamap)) {
            symbol = dlsym(lib->w.lib, kh_key(lib->w.wdatamap, k));
            if(symbol) {
                // found!
                *addr = (uintptr_t)symbol;
                *size = kh_value(lib->w.wdatamap, k);
                *weak = 1;
                return 1;
            }
        }
    }
    // check in mydatamap
    k = kh_get(datamap, lib->w.mydatamap, name);
    if (k!=kh_end(lib->w.mydatamap)) {
        char buff[200];
        if(lib->w.altmy)
            strcpy(buff, lib->w.altmy);
        else
            strcpy(buff, "my_");
        strcat(buff, name);
        symbol = dlsym(my_context->box64lib, buff);
        if(!symbol)
            printf_log(LOG_NONE, "Warning, data %s not found\n", buff);
        if(symbol) {
            // found!
            *addr = (uintptr_t)symbol;
            *size = kh_value(lib->w.mydatamap, k);
            *weak = 0;
            return 1;
        }
    }
    return 0;
}
static int getSymbolInSymbolMaps(library_t*lib, const char* name, int noweak, uintptr_t *addr, uintptr_t *size, int* weak)
{
    void* symbol;
    // check in mysymbolmap
    khint_t k = kh_get(symbolmap, lib->w.mysymbolmap, name);
    if (k!=kh_end(lib->w.mysymbolmap)) {
        char buff[200];
        if(lib->w.altmy)
            strcpy(buff, lib->w.altmy);
        else
            strcpy(buff, "my_");
        strcat(buff, name);
        symbol = dlsym(my_context->box64lib, buff);
        if(!symbol) {
            printf_log(LOG_NONE, "Warning, function %s not found\n", buff);
        } else 
            AddOffsetSymbol(my_context->maplib, symbol, name);
        *addr = AddBridge(lib->w.bridge, kh_value(lib->w.mysymbolmap, k), symbol, 0, name);
        *size = sizeof(void*);
        *weak = 0;
        return 1;
    }
    // check in stsymbolmap (return struct...)
    k = kh_get(symbolmap, lib->w.stsymbolmap, name);
    if (k!=kh_end(lib->w.stsymbolmap)) {
        char buff[200];
        if(lib->w.altmy)
            strcpy(buff, lib->w.altmy);
        else
            strcpy(buff, "my_");
        strcat(buff, name);
        symbol = dlsym(my_context->box64lib, buff);
        if(!symbol) {
            printf_log(LOG_NONE, "Warning, function %s not found\n", buff);
        } else 
            AddOffsetSymbol(my_context->maplib, symbol, name);
        *addr = AddBridge(lib->w.bridge, kh_value(lib->w.stsymbolmap, k), symbol, sizeof(void*), name);
        *size = sizeof(void*);
        *weak = 0;
        return 1;
    }
    // check in symbolmap
    k = kh_get(symbolmap, lib->w.symbolmap, name);
    if (k!=kh_end(lib->w.symbolmap)) {
        symbol = dlsym(lib->w.lib, name);
        if(!symbol && lib->w.altprefix) {
            char newname[200];
            strcpy(newname, lib->w.altprefix);
            strcat(newname, name);
            symbol = dlsym(lib->w.lib, newname);
        }
        if(!symbol)
            symbol = GetNativeSymbolUnversionned(lib->w.lib, name);
        if(!symbol && lib->w.altprefix) {
            char newname[200];
            strcpy(newname, lib->w.altprefix);
            strcat(newname, name);
            symbol = GetNativeSymbolUnversionned(lib->w.lib, newname);
        }
        if(!symbol) {
            printf_log(LOG_INFO, "Warning, function %s not found in lib %s\n", name, lib->name);
            return 0;
        } else 
            AddOffsetSymbol(my_context->maplib, symbol, name);
        *addr = AddBridge(lib->w.bridge, kh_value(lib->w.symbolmap, k), symbol, 0, name);
        *size = sizeof(void*);
        *weak = 0;
        return 1;
    }
    if(!noweak) {
        // check in wmysymbolmap
        khint_t k = kh_get(symbolmap, lib->w.wmysymbolmap, name);
        if (k!=kh_end(lib->w.wmysymbolmap)) {
            char buff[200];
            if(lib->w.altmy)
                strcpy(buff, lib->w.altmy);
            else
                strcpy(buff, "my_");
            strcat(buff, name);
            symbol = dlsym(my_context->box64lib, buff);
            if(!symbol) {
                printf_log(LOG_NONE, "Warning, function %s not found\n", buff);
            } else 
                AddOffsetSymbol(my_context->maplib, symbol, name);
            *addr = AddBridge(lib->w.bridge, kh_value(lib->w.wmysymbolmap, k), symbol, 0, name);
            *size = sizeof(void*);
            *weak = 1;
            return 1;
        }
        k = kh_get(symbolmap, lib->w.wsymbolmap, name);
        if (k!=kh_end(lib->w.wsymbolmap)) {
            symbol = dlsym(lib->w.lib, name);
            if(!symbol && lib->w.altprefix) {
                char newname[200];
                strcpy(newname, lib->w.altprefix);
                strcat(newname, name);
                symbol = dlsym(lib->w.lib, newname);
            }
            if(!symbol)
                symbol = GetNativeSymbolUnversionned(lib->w.lib, name);
            if(!symbol && lib->w.altprefix) {
                char newname[200];
                strcpy(newname, lib->w.altprefix);
                strcat(newname, name);
                symbol = GetNativeSymbolUnversionned(lib->w.lib, newname);
            }
            if(!symbol) {
                printf_log(LOG_INFO, "Warning, function %s not found in lib %s\n", name, lib->name);
                return 0;
            } else 
                AddOffsetSymbol(my_context->maplib, symbol, name);
            *addr = AddBridge(lib->w.bridge, kh_value(lib->w.wsymbolmap, k), symbol, 0, name);
            *size = sizeof(void*);
            *weak = 1;
            return 1;
        }
    }
    // check in symbol2map
    k = kh_get(symbol2map, lib->w.symbol2map, name);
    if (k!=kh_end(lib->w.symbol2map)) 
        if(!noweak || !kh_value(lib->w.symbol2map, k).weak)
        {
            symbol = dlsym(lib->w.lib, kh_value(lib->w.symbol2map, k).name);
            if(!symbol)
                symbol = dlsym(RTLD_DEFAULT, kh_value(lib->w.symbol2map, k).name);    // search globaly maybe
            if(!symbol)
                symbol = GetNativeSymbolUnversionned(lib->w.lib, kh_value(lib->w.symbol2map, k).name);
            if(!symbol) {
                printf_log(LOG_INFO, "Warning, function %s not found in lib %s\n", kh_value(lib->w.symbol2map, k).name, lib->name);
                return 0;
            } else 
                AddOffsetSymbol(my_context->maplib, symbol, name);
            *addr = AddBridge(lib->w.bridge, kh_value(lib->w.symbol2map, k).w, symbol, 0, name);
            *size = sizeof(void*);
            *weak = kh_value(lib->w.symbol2map, k).weak;
            return 1;
        }

    return 0;
}

int getSymbolInMaps(library_t *lib, const char* name, int noweak, uintptr_t *addr, uintptr_t *size, int* weak, int version, const char* vername, int local)
{
    if(!lib->active)
        return 0;
    if(version==-2) // don't send global native symbol for a version==-2 search
        return 0;
    // check in datamaps (but no version, it's not handled there)
    if(getSymbolInDataMaps(lib, name, noweak, addr, size, weak))
        return 1;

    if(getSymbolInSymbolMaps(lib, VersionnedName(name, version, vername), noweak, addr, size, weak))
        return 1;

    if(getSymbolInSymbolMaps(lib, name, noweak, addr, size, weak))
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
    if(lib->type!=LIB_WRAPPED)
        return NULL;
    return lib->w.lib;
}

lib_t* GetMaplib(library_t* lib)
{
    if(!lib)
        return NULL;
    return lib->maplib;
}

linkmap_t* getLinkMapLib(library_t* lib)
{
    linkmap_t* lm = my_context->linkmap;
    while(lm) {
        if(lm->l_lib == lib)
            return lm;
        lm = lm->l_next;
    }
    return NULL;
}
linkmap_t* addLinkMapLib(library_t* lib)
{
    if(!my_context->linkmap) {
        my_context->linkmap = (linkmap_t*)box_calloc(1, sizeof(linkmap_t));
        my_context->linkmap->l_lib = lib;
        return my_context->linkmap;
    }
    linkmap_t* lm = my_context->linkmap;
    while(lm->l_next)
        lm = lm->l_next;
    lm->l_next = (linkmap_t*)box_calloc(1, sizeof(linkmap_t));
    lm->l_next->l_lib = lib;
    lm->l_next->l_prev = lm;
    return lm->l_next;
}
void removeLinkMapLib(library_t* lib)
{
    linkmap_t* lm = getLinkMapLib(lib);
    if(!lm) return;
    if(lm->l_next)
        lm->l_next->l_prev = lm->l_prev;
    if(lm->l_prev)
        lm->l_prev->l_next = lm->l_next;
    box_free(lm);
}

void AddMainElfToLinkmap(elfheader_t* elf)
{
    linkmap_t* lm = addLinkMapLib(NULL);    // main elf will have a null lib link

    lm->l_addr = (Elf64_Addr)GetElfDelta(elf);
    lm->l_name = my_context->fullpath;
    lm->l_ld = GetDynamicSection(elf);
}

static int is_neededlib_present(needed_libs_t* needed, library_t* lib)
{
    if(!needed || !lib)
        return 0;
    if(!needed->size)
        return 0;
    for(int i=0; i<needed->size; ++i)
        if(needed->libs[i] == lib)
            return 1;
    return 0;
}

void add_neededlib(needed_libs_t* needed, library_t* lib)
{
    if(!needed)
        return;
    if(is_neededlib_present(needed, lib))
        return;
    if(needed->size == needed->cap) {
        needed->cap += 8;
        needed->libs = (library_t**)realloc(needed->libs, needed->cap*sizeof(library_t*));
    }
    needed->libs[needed->size++] = lib;
}
void free_neededlib(needed_libs_t* needed)
{
    if(!needed)
        return;
    needed->cap = 0;
    needed->size = 0;
    if(needed->libs)
        free(needed->libs);
    needed->libs = NULL;
}
void add_dependedbylib(needed_libs_t* dependedby, library_t* lib)
{
    if(!dependedby)
        return;
    if(is_neededlib_present(dependedby, lib))
        return;
    if(dependedby->size == dependedby->cap) {
        dependedby->cap += 8;
        dependedby->libs = (library_t**)realloc(dependedby->libs, dependedby->cap*sizeof(library_t*));
    }
    dependedby->libs[dependedby->size++] = lib;
}
void free_dependedbylib(needed_libs_t* dependedby)
{
    if(!dependedby)
        return;
    dependedby->cap = 0;
    dependedby->size = 0;
    if(dependedby->libs)
        free(dependedby->libs);
    dependedby->libs = NULL;
}

void setNeededLibs(library_t* lib, int n, ...)
{
    if(lib->type!=LIB_WRAPPED && lib->type!=LIB_UNNKNOW)
        return;
    lib->w.needed = n;
    lib->w.neededlibs = (char**)box_calloc(n, sizeof(char*));
    va_list va;
    va_start (va, n);
    for (int i=0; i<n; ++i) {
        lib->w.neededlibs[i] = box_strdup(va_arg(va, char*));
    }
    va_end (va);
}
