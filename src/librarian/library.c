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

#ifdef STATICBUILD
#include "library_list_static.h"
#else
#include "library_list.h"
#endif

#undef GO
#ifdef BOX32
#define GO(P, N) int wrapped##N##_init32(library_t* lib, box64context_t *box64); \
                 void wrapped##N##_fini32(library_t* lib);
#ifdef STATICBUILD
#include "library_list_static_32.h"
#else
#include "library_list_32.h"
#endif
#undef GO
#endif

#define GO(P, N) {P, wrapped##N##_init, wrapped##N##_fini},
wrappedlib_t wrappedlibs[] = {
#ifdef STATICBUILD
#include "library_list_static.h"
#else
#include "library_list.h"
#endif
};
#undef GO
#define GO(P, N) {P, wrapped##N##_init32, wrapped##N##_fini32},
wrappedlib_t wrappedlibs32[] = {
#ifdef BOX32
#ifdef STATICBUILD
#include "library_list_static_32.h"
#else
#include "library_list_32.h"
#endif
#endif
};
#undef GO

KHASH_MAP_IMPL_STR(symbolmap, symbol1_t)
KHASH_MAP_IMPL_STR(symbol2map, symbol2_t)
#ifdef STATICBUILD
KHASH_MAP_IMPL_STR(datamap, datamap_t)
#else
KHASH_MAP_IMPL_STR(datamap, uint64_t)
#endif

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

static void initDummyLib(library_t *lib);

void EmuLib_Fini(library_t* lib)
{
}
void WrappedLib_FinishFini(library_t* lib)
{
    if(lib->type!=LIB_WRAPPED)
        return;
    if(lib->w.lib)
        dlclose(lib->w.lib);
    lib->w.lib = NULL;
    if(lib->w.altprefix)
        box_free(lib->w.altprefix);
    if(lib->w.altmy)
        box_free(lib->w.altmy);
    if(lib->w.needed) {
        free_neededlib(lib->w.needed);
    }
    FreeBridge(&lib->w.bridge);
}
void DummyLib_Fini(library_t* lib)
{
}

int WrappedLib_GetWeak(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, size_t asked_size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym)
{
    // ignoring asked size on wrapped libs
    uintptr_t addr = 0;
    uintptr_t size = 0;
    int wk = 0;
    if (!getSymbolInMaps(lib, name, 0, &addr, &size, &wk, *version, *vername, local, *veropt)) {
        return 0;
    }
    if(!addr && !size)
        return 0;
    if(!wk) // found a global symbol
        return 0;
    *offs = addr;
    *sz = size;
    *weak = wk;
    if(elfsym) *elfsym = NULL;
    return 1;
}
int EmuLib_GetWeak(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, size_t asked_size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym)
{
    // symbols...
    uintptr_t start, end;
    // weak symbols...
    void* sym;
    if((sym = ElfGetWeakSymbolStartEnd(lib->e.elf, &start, &end, name, version, vername, local, veropt)))
    {
        if(asked_size && asked_size!=end-start)
            return 0;
        *offs = start;
        *sz = end-start;
        *weak = 1;
        if(elfsym) *elfsym = sym;
        return 1;
    }
    return 0;
}
int DummyLib_GetWeak(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, size_t asked_size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym)
{
    return 0;
}

int WrappedLib_GetGlobal(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, size_t asked_size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym)
{
    // ignoring asked size on wrapped libs
    uintptr_t addr = 0;
    uintptr_t size = 0;
    int wk = 0;
    if (!getSymbolInMaps(lib, name, 1, &addr, &size, &wk, *version, *vername, local, *veropt)) {
        return 0;
    }
    if(!addr && !size)
        return 0;
    if(wk)
        return 0;   //found a weak symbol
    *offs = addr;
    *sz = size;
    *weak = 0;
    if(elfsym) *elfsym = NULL;
    return 1;
}
int EmuLib_GetGlobal(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, size_t asked_size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym)
{
    uintptr_t start, end;
    void* sym;
    if((sym = ElfGetGlobalSymbolStartEnd(lib->e.elf, &start, &end, name, version, vername, local, veropt)))
    {
        if(asked_size && asked_size!=end-start)
            return 0;
        *offs = start;
        *sz = end-start;
        *weak = 0;
        if(elfsym) *elfsym = sym;
        return 1;
    }
    return 0;
}
int DummyLib_GetGlobal(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, size_t asked_size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym)
{
    return 0;
}
int EmuLib_GetLocal(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, size_t asked_size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym)
{
    // ignoring asked size on wrapped libs
    uintptr_t start, end;
    void* sym;
    if((sym = ElfGetLocalSymbolStartEnd(lib->e.elf, &start, &end, name, version, vername, local, veropt)))
    {
        if(asked_size && asked_size!=end-start)
            return 0;
        *offs = start;
        *sz = end-start;
        *weak = 0;
        if(elfsym) *elfsym = sym;
        return 1;
    }
    return 0;
}

int WrappedLib_GetLocal(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, size_t asked_size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym)
{
    (void)lib; (void)name; (void)offs; (void)sz; (void)version; (void)vername; (void)local;
    return 0;
}
int DummyLib_GetLocal(library_t* lib, const char* name, uintptr_t *offs, uintptr_t *sz, size_t asked_size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym)
{
    (void)lib; (void)name; (void)offs; (void)sz; (void)version; (void)vername; (void)local;
    return 0;
}

static void initWrappedLib(library_t *lib, box64context_t* context) {
    int nb = (box64_is32bits?sizeof(wrappedlibs32):sizeof(wrappedlibs)) / sizeof(wrappedlib_t);
    for (int i=0; i<nb; ++i) {
        wrappedlib_t* w = box64_is32bits?(&wrappedlibs32[i]):(&wrappedlibs[i]);
        if(strcmp(lib->name, w->name)==0) {
            if(w->init(lib, context)) {
                // error!
                const char* error_str = dlerror();
                if(error_str)   // don't print the message if there is no error string from last error
                    printf_log(LOG_NONE, "Error initializing native %s (last dlerror is %s)\n", lib->name, error_str);
                return; // non blocker...
            }
            printf_dump(LOG_INFO, "Using native(wrapped) %s\n", lib->name);
            lib->fini = w->fini;
            lib->getglobal = WrappedLib_GetGlobal;
            lib->getweak = WrappedLib_GetWeak;
            lib->getlocal = WrappedLib_GetLocal;
            lib->type = LIB_WRAPPED;
            lib->w.refcnt = 1;
            // Call librarian to load all dependant elf
            if(AddNeededLib(context->maplib, 0, 0, 0, lib->w.needed, NULL, context, thread_get_emu())) {
                printf_log(LOG_NONE, "Error: loading a needed libs in elf %s\n", lib->name);
                return;
            }

            linkmap_t *lm = addLinkMapLib(lib);
            if(!lm) {
                // Crashed already
                printf_dlsym_dump(LOG_DEBUG, "Failure to add lib %s linkmap\n", lib->name);
                break;
            }
            struct link_map real_lm;
            #ifndef ANDROID
            if(dlinfo(lib->w.lib, RTLD_DI_LINKMAP, &real_lm)) {
                printf_dlsym_dump(LOG_DEBUG, "Failed to dlinfo lib %s\n", lib->name);
            }
            #endif
            lm->l_addr = real_lm.l_addr;
            lm->l_name = real_lm.l_name;
            lm->l_ld = real_lm.l_ld;
            break;
        }
    }
}

static int loadEmulatedLib(const char* libname, library_t *lib, box64context_t* context, elfheader_t* verneeded)
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
            printf_dlsym_dump(LOG_DEBUG, "Error: reading elf header of %s\n", libname);    // this one can be too alarming...
            fclose(f);
            return 0;
        }

        if(CalcLoadAddr(elf_header)) {
            printf_log(LOG_NONE, "Error: reading elf header of %s\n", libname);
            FreeElfHeader(&elf_header);
            return 0;
        }
        // allocate and load elf
        if(AllocLoadElfMemory(context, elf_header, 0)) {
            printf_log(LOG_NONE, "Error: loading for elf %s\n", libname);
            FreeElfHeader(&elf_header);
            return 0;
        }
        // can close the file now
        if(verneeded && !isElfHasNeededVer(elf_header, lib->name, verneeded)) {
            // incompatible, discard and continue the search
            FreeElfHeader(&elf_header);
            return 0;
        }

        int mainelf = AddElfHeader(context, elf_header);
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
            lib->path = box_realpath(libname, NULL);
            if(!lib->path)
                lib->path = box_strdup(libname);
        }

        printf_dump(LOG_INFO, "Using emulated %s\n", libname);
        int env_changed = 0;
        #ifdef DYNAREC
        if(libname && BOX64ENV(dynarec_bleeding_edge) && strstr(libname, "libmonobdwgc-2.0.so")) {
            printf_dump(LOG_INFO, "MonoBleedingEdge detected, disable Dynarec BigBlock and enable Dynarec StrongMem\n");
            SET_BOX64ENV(dynarec_bigblock, 0);
            SET_BOX64ENV(dynarec_strongmem, 1);
            env_changed = 1;
        }
        if(libname && BOX64ENV(unityplayer) && strstr(libname, "UnityPlayer.so")) {
            printf_dump(LOG_INFO, "UnityPlayer detected, applying Unity settings\n");
            SET_BOX64ENV(unity, 1);
            env_changed = 1;
        }
        if(libname && BOX64ENV(dynarec_tbb) && strstr(libname, "libtbb.so")) {
            printf_dump(LOG_INFO, "libtbb detected, enable Dynarec StrongMem\n");
            SET_BOX64ENV(dynarec_strongmem, 1);
            env_changed = 1;
        }
        #endif
        if(libname && BOX64ENV(jvm) && strstr(libname, "libjvm.so")) {
            #ifdef DYNAREC
            printf_dump(LOG_INFO, "libjvm detected, disable Dynarec BigBlock and enable Dynarec StrongMem, hide SSE 4.2\n");
            SET_BOX64ENV(dynarec_bigblock, 0);
            SET_BOX64ENV(dynarec_strongmem, 1);
            #else
            printf_dump(LOG_INFO, "libjvm detected, hide SSE 4.2\n");
            #endif
            SET_BOX64ENV(sse42, 0);
            env_changed = 1;
        }
        if(libname && BOX64ENV(libcef) && strstr(libname, "libcef.so")) {
            printf_dump(LOG_INFO, "libcef detected, using malloc_hack=2\n");
            SET_BOX64ENV(malloc_hack, 2);
            env_changed = 1;
        }
        if (env_changed) PrintEnvVariables(&box64env, LOG_INFO);
        return 1;
    }
    return 0;
}

static void initEmulatedLib(const char* path, library_t *lib, box64context_t* context, elfheader_t* verneeded)
{
    char libname[MAX_PATH];
    strcpy(libname, path);
    int found = box64_is32bits?FileIsX86ELF(libname):FileIsX64ELF(libname);
    if(found)
        if(loadEmulatedLib(libname, lib, context, verneeded))
            return;
    if(!strchr(path, '/'))
        for(int i=0; i<context->box64_ld_lib.size; ++i)
        {
            strcpy(libname, context->box64_ld_lib.paths[i]);
            strcat(libname, path);
            if(box64_is32bits?FileIsX86ELF(libname):FileIsX64ELF(libname))
                if(loadEmulatedLib(libname, lib, context, verneeded))
                    return;
            // also try x86_64 variant
            strcpy(libname, context->box64_ld_lib.paths[i]);
            strcat(libname, box64_is32bits?"i386/":"x86_64/");
            strcat(libname, path);
            if(box64_is32bits?FileIsX86ELF(libname):FileIsX64ELF(libname))
                if(loadEmulatedLib(libname, lib, context, verneeded))
                    return;            
        }
}

static void initDummyLib(library_t *lib)
{
    memset(lib, 0, sizeof(library_t));
    lib->fini = DummyLib_Fini;
    lib->getglobal = DummyLib_GetGlobal;
    lib->getweak = DummyLib_GetWeak;
    lib->getlocal = DummyLib_GetLocal;
    lib->name = NULL;
    lib->path = NULL;
    lib->type = LIB_UNNKNOW;
}

static const char* essential_libs[] = {
    "libc.so.6", "libpthread.so.0", "librt.so.1", "libGL.so.1", "libGL.so", "libX11.so.6", 
    "libasound.so.2", "libdl.so.2", "libm.so.6", "libbsd.so.0", "libutil.so.1", "libresolv.so.2", "libresolv.so", 
    "libXxf86vm.so.1", "libXinerama.so.1", "libXrandr.so.2", "libXext.so.6", "libXfixes.so.3", "libXcursor.so.1",
    "libXrender.so.1", "libXft.so.2", "libXi.so.6", "libXss.so.1", "libXpm.so.4", "libXau.so.6", "libXdmcp.so.6",
    "libX11-xcb.so.1", "libxcb.so.1", "libxcb-xfixes.so.0", "libxcb-shape.so.0", "libxcb-shm.so.0", "libxcb-randr.so.0",
    "libxcb-image.so.0", "libxcb-keysyms.so.1", "libxcb-xtest.so.0", "libxcb-glx.so.0", "libxcb-dri2.so.0", "libxcb-dri3.so.0",
    "libXtst.so.6", "libXt.so.6", "libXcomposite.so.1", "libXdamage.so.1", "libXmu.so.6", "libxkbcommon.so.0", 
    "libxkbcommon-x11.so.0", "libpulse-simple.so.0", "libpulse.so.0", "libvulkan.so.1", "libvulkan.so",
    "ld-linux-x86-64.so.2", "crashhandler.so", "libtcmalloc_minimal.so.0", "libtcmalloc_minimal.so.4", "libanl.so.1",
    "ld-linux.so.2", "ld-linux.so.3", "libthread_db.so.1"
};
static const char* essential_libs_egl[] = {
    "libEGL.so", "libGLESv2.so"
};
static int isEssentialLib(const char* name) {
    for (unsigned int i=0; i<sizeof(essential_libs)/sizeof(essential_libs[0]); ++i)
        if(!strcmp(name, essential_libs[i]))
            return 1;
    if(BOX64ENV(wrap_egl))
        for (unsigned int i=0; i<sizeof(essential_libs_egl)/sizeof(essential_libs_egl[0]); ++i)
            if(!strcmp(name, essential_libs_egl[i]))
                return 1;
    return 0;
}

#define NLIBS       16
typedef struct lib_brick_s {
    library_t           libs[NLIBS];
    struct lib_brick_s* next;
} lib_brick_t;

static lib_brick_t* lib_head = NULL;
static lib_brick_t* cur_brick = NULL;
static size_t cur_lib = 0;
static size_t lib_cap = 0;

library_t *NewLibrary(const char* path, box64context_t* context, elfheader_t* verneeded)
{
    printf_dlsym_dump(LOG_DEBUG, "Trying to load \"%s\"\n", path);
    //library_t *lib = (library_t*)box_calloc(1, sizeof(library_t));
    if(cur_lib==lib_cap) {
        lib_brick_t *new_brick = box_calloc(1, sizeof(lib_brick_t));
        if(!lib_head)
            cur_brick = lib_head = new_brick;
        else {
            cur_brick->next = new_brick;
            cur_brick = new_brick;
        }
        lib_cap += NLIBS;
    }
    library_t *lib = &cur_brick->libs[cur_lib%NLIBS];
    ++cur_lib;
    memset(lib, 0, sizeof(library_t));

    lib->path = box_realpath(path, NULL);
    if(!lib->path)
        lib->path = box_strdup(path);
    if(BOX64ENV(libgl) && !strcmp(path, BOX64ENV(libgl)))
        lib->name = box_strdup("libGL.so.1");
    else
        lib->name = Path2Name(path);
    lib->nbdot = NbDot(lib->name);
    lib->type = LIB_UNNKNOW;
    printf_dlsym_dump(LOG_DEBUG, "Simplified name is \"%s\"\n", lib->name);
    if(BOX64ENV(nopulse)) {
        if(strstr(lib->name, "libpulse.so")==lib->name || strstr(lib->name, "libpulse-simple.so")==lib->name) {
            box_free(lib->name);
            box_free(lib->path);
            initDummyLib(lib);
            //box_free(lib);
            return NULL;
        }
    }
    if(BOX64ENV(novulkan)) {
        if(strstr(lib->name, "libvulkan.so")==lib->name) {
            box_free(lib->name);
            box_free(lib->path);
            initDummyLib(lib);
            //box_free(lib);
            return NULL;
        }
    }
    int notwrapped = FindInCollection(lib->name, &context->box64_emulated_libs);
    int essential = isEssentialLib(lib->name);
    if(!notwrapped && BOX64ENV(prefer_emulated) && !essential)
        notwrapped = 1;
    int precise = (!BOX64ENV(prefer_wrapped) && !essential && path && strchr(path, '/'))?1:0;
    if(!notwrapped && precise && strstr(path, "libtcmalloc_minimal.so"))
        precise = 0;    // allow native version for tcmalloc_minimum
    /*
    if(!notwrapped && precise && strstr(path, "libEGL.so"))
        precise = 0;    // allow native version for EGL
    if(!notwrapped && precise && strstr(path, "libGLESv2.so"))
        precise = 0;    // allow native version for GLESv2
    */
    // check if name is libSDL_sound-1.0.so.1 but with SDL2 loaded, then try emulated first...
    if(!notwrapped && !strcmp(lib->name, "libSDL_sound-1.0.so.1") && my_context->sdl2lib)
        notwrapped = 1;
    // And now, actually loading a library
    // look for native(wrapped) libs first
    if(!notwrapped && !precise)
        initWrappedLib(lib, context);
    // then look for a native one
    if(lib->type==LIB_UNNKNOW)
        initEmulatedLib(path, lib, context, verneeded);
    // still not loaded but notwrapped indicated: use wrapped...
    if(lib->type==LIB_UNNKNOW && notwrapped && !precise)
        initWrappedLib(lib, context);
    // nothing loaded, so error...
    if(lib->type==LIB_UNNKNOW)
    {
        box_free(lib->name);
        box_free(lib->path);
        initDummyLib(lib);
        //box_free(lib);
        return NULL;
    }

    return lib;
}
int AddSymbolsLibrary(lib_t *maplib, library_t* lib, x64emu_t* emu)
{
    (void)emu;

    if(lib->type==LIB_EMULATED) {
        elfheader_t *elf_header = lib->e.elf;
        // add symbols
        AddSymbols(maplib, elf_header);
    }
    return 0;
}
int FinalizeLibrary(library_t* lib, lib_t* local_maplib, int bindnow, int deepbind, x64emu_t* emu)
{
    if(!lib)
        return 0;
    lib->deepbind = deepbind;
    if(lib->type==LIB_EMULATED) {
        if(lib->e.finalized)
            return 0;
        lib->e.finalized = 1;
        elfheader_t *elf_header = my_context->elfs[lib->e.elf_index];
        // finalize relocations
        if(RelocateElf(my_context->maplib, local_maplib, bindnow, deepbind, elf_header)) {
            printf_log(LOG_NONE, "Error: relocating symbols in elf %s\n", lib->name);
            return 1;
        }
        if(RelocateElfPlt(my_context->maplib, local_maplib, bindnow, deepbind, elf_header)) {
            printf_log(LOG_NONE, "Error: relocating Plt symbols in elf %s\n", lib->name);
            return 1;
        }
#ifdef HAVE_TRACE
        if(trace_func) {
            int weak;
            int ver = -1, veropt = 0;
            const char* vername = NULL;
            if (GetGlobalSymbolStartEnd(local_maplib, trace_func, &trace_start, &trace_end, elf_header, ver, vername, veropt, NULL)) {
                SetTraceEmu(trace_start, trace_end);
                printf_dump(LOG_INFO, "TRACE on %s only (%p-%p)\n", trace_func, (void*)trace_start, (void*)trace_end);
                box_free(trace_func);
                trace_func = NULL;
            } else if(GetLibLocalSymbolStartEnd(lib, trace_func, &trace_start, &trace_end, 0, &weak, &ver, &vername, 0, &veropt, NULL)) {
                SetTraceEmu(trace_start, trace_end);
                printf_dump(LOG_INFO, "TRACE on %s only (%p-%p)\n", trace_func, (void*)trace_start, (void*)trace_end);
                box_free(trace_func);
                trace_func = NULL;
            }
        }
#endif
        RunElfInit(elf_header, emu);
    }
    return 0;
}

int FiniLibrary(library_t* lib, x64emu_t* emu)
{
    switch (lib->type) {
        case LIB_WRAPPED:
            return 0;
        case LIB_EMULATED:
            RunElfFini(lib->e.elf, emu);
            return 0;
    }
    return 1;   // bad type
}

void Free1Library(library_t **the_lib, x64emu_t* emu)
{
    if(!(*the_lib)) return;

    library_t* lib = *the_lib;

    FiniLibrary(lib, emu);

    printf_dlsym_dump(LOG_DEBUG, "Free1Library %s\n", lib->name?:"???");
    // remove lib from maplib/local_maplib...
    if(my_context) {
        MapLibRemoveLib(my_context->maplib, lib);
        MapLibRemoveLib(my_context->local_maplib, lib);
    }
    // free elf is relevant
    int lib_type = lib->type;
    // Handle needed for cleaning
    needed_libs_t* needed = (lib_type==LIB_EMULATED && lib->e.elf)?lib->e.elf->needed:((lib_type==LIB_WRAPPED)?lib->w.needed:NULL);
    if(needed)
        needed = copy_neededlib(needed);
    // free elf
    if(lib_type==LIB_EMULATED) {
        FreeElfHeader(&lib->e.elf);
    }

    // No "Fini" logic here, only memory handling
    if(lib->maplib && !lib->maplib_ref) {
        lib_t* maplib = lib->maplib;
        lib->maplib = NULL;
        FreeLibrarian(&maplib, emu);
    }
    lib->maplib = NULL;
    lib->maplib_ref = 0;
    if(lib_type!=LIB_UNNKNOW && lib->fini) {
        lib->fini(lib);
    }
    box_free(lib->name);
    box_free(lib->path);

    if(lib_type == LIB_WRAPPED) {
        if(lib->w.symbolmap)
            kh_destroy(symbolmap, lib->w.symbolmap);
        if(lib->w.wsymbolmap)
            kh_destroy(symbolmap, lib->w.wsymbolmap);
        if(lib->w.datamap)
            kh_destroy(datamap, lib->w.datamap);
        if(lib->w.wdatamap)
            kh_destroy(datamap, lib->w.wdatamap);
        if(lib->w.mydatamap)
            kh_destroy(datamap, lib->w.mydatamap);
        if(lib->w.mysymbolmap)
            kh_destroy(symbolmap, lib->w.mysymbolmap);
        if(lib->w.wmysymbolmap)
            kh_destroy(symbolmap, lib->w.wmysymbolmap);
        if(lib->w.stsymbolmap)
            kh_destroy(symbolmap, lib->w.stsymbolmap);
        if(lib->w.symbol2map)
            kh_destroy(symbol2map, lib->w.symbol2map);
    }

    // remove from the dlopen collection
    if(lib->dlopen)
        RemoveDlopen(the_lib, lib->dlopen);
    initDummyLib(lib);
    //box_free(lib);
    if(*the_lib == lib)
        *the_lib = NULL;
    // handle needed libs now
    if(needed) {
        for(int i=0; i<needed->size; ++i)
            DecRefCount(&needed->libs[i], emu);
        free_neededlib(needed);
    }
}

char* GetNameLib(library_t* lib)
{
    return lib->name;
}

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int IsSameLib(library_t* lib, const char* path)
{
    int ret = 0;
    if(!lib) 
        return 0;
    if(lib->type==LIB_UNNKNOW)
        return 0;
    char* name = Path2Name(path);
    if(!strchr(path, '/') || lib->type==LIB_WRAPPED || !lib->path) {
        if(strcmp(name, lib->name)==0)
            ret=1;
        if(lib->type==LIB_EMULATED && lib->e.elf->soname && !strcmp(lib->e.elf->soname, path))
            ret=1;
    } else {
        char rpath[PATH_MAX];
        (void) box_realpath(path, rpath);
        if(!strcmp(rpath, lib->path))
            ret=1;
        if(lib->type==LIB_EMULATED && lib->e.elf->path && !strcmp(lib->e.elf->path, rpath)) {
            ret=1;
        }
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
int GetLibWeakSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, size_t size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym)
{
    if(!name[0] || !lib || (lib->type==LIB_UNNKNOW))
        return 0;
    // get a new symbol
    if(lib->getweak(lib, name, start, end, size, weak, version, vername, local, veropt, elfsym)) {
        *end += *start;     // lib->get(...) gives size, not end
        return 1;
    }
    // nope
    return 0;
}
int GetLibGlobalSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, size_t size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym)
{
    if(!name[0] || !lib || (lib->type==LIB_UNNKNOW))
        return 0;
    // get a new symbol
    if(lib->getglobal(lib, name, start, end, size, weak, version, vername, local, veropt, elfsym)) {
        *end += *start;     // lib->get(...) gives size, not end
        return 1;
    }
    // nope
    return 0;
}
int GetLibLocalSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end, size_t size, int* weak, int* version, const char** vername, int local, int* veropt, void** elfsym)
{
    if(!name[0] || !lib || (lib->type==LIB_UNNKNOW))
        return 0;
    // get a new symbol
    if(lib->getlocal(lib, name, start, end, size, weak, version, vername, local, veropt, elfsym)) {
        *end += *start;     // lib->get(...) gives size, not end
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
        #ifdef STATICBUILD
        symbol = (void*)(kh_value(lib->w.datamap, k).addr);
        #else
        symbol = dlsym(lib->w.lib, kh_key(lib->w.datamap, k));
        #endif
        if(symbol) {
            // found!
            *addr = (uintptr_t)symbol;
            #ifdef STATICBUILD
            *size = kh_value(lib->w.datamap, k).size;
            #else
            *size = kh_value(lib->w.datamap, k);
            #endif
            *weak = 0;
            return 1;
        }
    }
    if(!noweak) {
        k = kh_get(datamap, lib->w.wdatamap, name);
        if (k!=kh_end(lib->w.wdatamap)) {
            #ifdef STATICBUILD
            symbol = (void*)(kh_value(lib->w.wdatamap, k).addr);
            #else
            symbol = dlsym(lib->w.lib, kh_key(lib->w.wdatamap, k));
            #endif
            if(symbol) {
                // found!
                *addr = (uintptr_t)symbol;
                #ifdef STATICBUILD
                *size = kh_value(lib->w.wdatamap, k).size;
                #else
                *size = kh_value(lib->w.wdatamap, k);
                #endif
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
            strcpy(buff, box64_is32bits?"my32_":"my_");
        strcat(buff, name);
        #ifdef STATICBUILD
        symbol = (void*)kh_value(lib->w.mydatamap, k).addr;
        #else
        symbol = dlsym(my_context->box64lib, buff);
        #endif
        if(!symbol)
            printf_log(LOG_NONE, "Warning, data %s not found\n", buff);
        if(symbol) {
            // found!
            *addr = (uintptr_t)symbol;
            #ifdef STATICBUILD
            *size = kh_value(lib->w.mydatamap, k).size;
            #else
            *size = kh_value(lib->w.mydatamap, k);
            #endif
            *weak = 0;
            return 1;
        }
    }
    return 0;
}
static int getSymbolInSymbolMaps(library_t*lib, const char* name, int noweak, uintptr_t *addr, uintptr_t *size, int* weak)
{
    const khint_t hash = kh_hash(symbolmap, name);
    void* symbol;
    // check in mysymbolmap
    khint_t k = kh_get_with_hash(symbolmap, lib->w.mysymbolmap, name, hash);
    if (k!=kh_end(lib->w.mysymbolmap)) {
        symbol1_t *s = &kh_value(lib->w.mysymbolmap, k);
        if(!s->resolved) {
            char buff[200];
            if(lib->w.altmy)
                strcpy(buff, lib->w.altmy);
            else
                strcpy(buff, box64_is32bits?"my32_":"my_");
            strcat(buff, name);
            #ifdef STATICBUILD
            symbol = (void*)s->addr;
            #else
            symbol = dlsym(my_context->box64lib, buff);
            #endif
            if(!symbol) {
                printf_log(LOG_NONE, "Warning, function %s not found\n", buff);
                return 0;
            }
            s->addr = AddBridge(lib->w.bridge, s->w, symbol, 0, name);
            s->resolved = 1;
        }
        *addr = s->addr;
        *size = sizeof(void*);
        *weak = 0;
        return 1;
    }
    // check in stsymbolmap (return struct...)
    k = kh_get_with_hash(symbolmap, lib->w.stsymbolmap, name, hash);
    if (k!=kh_end(lib->w.stsymbolmap)) {
        symbol1_t *s = &kh_value(lib->w.stsymbolmap, k);
        if(!s->resolved) {
            char buff[200];
            if(lib->w.altmy)
                strcpy(buff, lib->w.altmy);
            else
                strcpy(buff, box64_is32bits?"my32_":"my_");
            strcat(buff, name);
            #ifdef STATICBUILD
            symbol = (void*)s->addr;
            #else
            symbol = dlsym(my_context->box64lib, buff);
            #endif
            if(!symbol) {
                printf_log(LOG_NONE, "Warning, function %s not found\n", buff);
                return 0;
            }
            s->addr = AddBridge(lib->w.bridge, s->w, symbol, box64_is32bits?4:sizeof(void*), name);
            s->resolved = 1;
        }
        *addr = s->addr;
        *size = sizeof(void*);
        *weak = 0;
        return 1;
    }
    // check in symbolmap
    k = kh_get_with_hash(symbolmap, lib->w.symbolmap, name, hash);
    if (k!=kh_end(lib->w.symbolmap)) {
        symbol1_t *s = &kh_value(lib->w.symbolmap, k);
        if(!s->resolved) {
            #ifdef STATICBUILD
            symbol = (void*)s->addr;
            #else
            symbol = dlsym(lib->w.lib, name);
            if(!symbol && lib->w.altprefix) {
                char newname[200];
                strcpy(newname, lib->w.altprefix);
                strcat(newname, name);
                symbol = dlsym(lib->w.lib, newname);
            }
            if(!symbol)
                symbol = GetNativeSymbolUnversioned(lib->w.lib, name);
            if(!symbol && lib->w.altprefix) {
                char newname[200];
                strcpy(newname, lib->w.altprefix);
                strcat(newname, name);
                symbol = GetNativeSymbolUnversioned(lib->w.lib, newname);
            }
            #endif
            if(!symbol) {
                printf_dump(LOG_INFO, "Warning, function %s not found in lib %s\n", name, lib->name);
                return 0;
            }
            s->addr = AddBridge(lib->w.bridge, s->w, symbol, 0, name);
            s->resolved = 1;
        }
        *addr = s->addr;
        *size = sizeof(void*);
        *weak = 0;
        return 1;
    }
    if(!noweak) {
        // check in wmysymbolmap
        khint_t k = kh_get_with_hash(symbolmap, lib->w.wmysymbolmap, name, hash);
        if (k!=kh_end(lib->w.wmysymbolmap)) {
            symbol1_t *s = &kh_value(lib->w.wmysymbolmap, k);
            if(!s->resolved) {
                char buff[200];
                if(lib->w.altmy)
                    strcpy(buff, lib->w.altmy);
                else
                    strcpy(buff, box64_is32bits?"my32_":"my_");
                strcat(buff, name);
                #ifdef STATICBUILD
                symbol = (void*)s->addr;
                #else
                symbol = dlsym(my_context->box64lib, buff);
                #endif
                if(!symbol) {
                    printf_log(LOG_NONE, "Warning, function %s not found\n", buff);
                    return 0;
                }
                s->addr = AddBridge(lib->w.bridge, s->w, symbol, 0, name);
                s->resolved = 1;
            }
            *addr = s->addr;
            *size = sizeof(void*);
            *weak = 1;
            return 1;
        }
        k = kh_get_with_hash(symbolmap, lib->w.wsymbolmap, name, hash);
        if (k!=kh_end(lib->w.wsymbolmap)) {
            symbol1_t *s = &kh_value(lib->w.wsymbolmap, k);
            if(!s->resolved) {
                #ifdef STATICBUILD
                symbol = (void*)s->addr;
                #else
                symbol = dlsym(lib->w.lib, name);
                if(!symbol && lib->w.altprefix) {
                    char newname[200];
                    strcpy(newname, lib->w.altprefix);
                    strcat(newname, name);
                    symbol = dlsym(lib->w.lib, newname);
                }
                if(!symbol)
                    symbol = GetNativeSymbolUnversioned(lib->w.lib, name);
                if(!symbol && lib->w.altprefix) {
                    char newname[200];
                    strcpy(newname, lib->w.altprefix);
                    strcat(newname, name);
                    symbol = GetNativeSymbolUnversioned(lib->w.lib, newname);
                }
                #endif
                if(!symbol) {
                    printf_dump(LOG_INFO, "Warning, function %s not found in lib %s\n", name, lib->name);
                    return 0;
                }
                s->addr = AddBridge(lib->w.bridge, s->w, symbol, 0, name);
                s->resolved = 1;
            }
            *addr = s->addr;
            *size = sizeof(void*);
            *weak = 1;
            return 1;
        }
    }
    // check in symbol2map
    // 
    // NOTE: symbol2map & symbolmap share the same hash function, so we can use the same hash
    k = kh_get_with_hash(symbol2map, lib->w.symbol2map, name, hash);
    if (k!=kh_end(lib->w.symbol2map))  {
        symbol2_t *s = &kh_value(lib->w.symbol2map, k);
        if(!noweak || !s->weak)
        {
            if(!s->resolved) {
                #ifdef STATICBUILD
                symbol = (void*)s->addr;
                #else
                symbol = dlsym(lib->w.lib, kh_value(lib->w.symbol2map, k).name);
                if(!symbol)
                    symbol = dlsym(RTLD_DEFAULT, kh_value(lib->w.symbol2map, k).name);    // search globaly maybe
                if(!symbol)
                    symbol = GetNativeSymbolUnversioned(lib->w.lib, kh_value(lib->w.symbol2map, k).name);
                #endif
                if(!symbol) {
                    printf_dump(LOG_INFO, "Warning, function %s not found in lib %s\n", kh_value(lib->w.symbol2map, k).name, lib->name);
                    return 0;
                }
                s->addr = AddBridge(lib->w.bridge, s->w, symbol, 0, name);
                s->resolved = 1;
            }
            *addr = s->addr;
            *size = sizeof(void*);
            *weak = s->weak;
            return 1;
        }
    }
    return 0;
}

int getSymbolInMaps(library_t *lib, const char* name, int noweak, uintptr_t *addr, uintptr_t *size, int* weak, int version, const char* vername, int local, int veropt)
{
    if(version==-2) // don't send global native symbol for a version==-2 search
        return 0;
    // check in datamaps (but no version, it's not handled there)
    if(getSymbolInDataMaps(lib, name, noweak, addr, size, weak))
        return 1;

    if(getSymbolInSymbolMaps(lib, VersionedName(name, version, vername), noweak, addr, size, weak))
        return 1;

    if(getSymbolInSymbolMaps(lib, name, noweak, addr, size, weak))
        return 1;

    return 0;
}

int GetNeededLibsN(library_t* lib) {
    switch (lib->type) {
        case LIB_WRAPPED: return lib->w.needed?lib->w.needed->size:0;
        case LIB_EMULATED: return lib->e.elf->needed?lib->e.elf->needed->size:0;
    }
    return 0;
}
library_t* GetNeededLib(library_t* lib, int idx)
{
    switch (lib->type) {
        case LIB_WRAPPED:
            if(idx<0 || !lib->w.needed || idx>=lib->w.needed->size)
                return NULL;
            return lib->w.needed->libs[idx];
        case LIB_EMULATED:
            if(idx<0 || idx>=lib->e.elf->needed->size)
                return NULL;
            return lib->e.elf->needed->libs[idx];
    }
    return NULL;
}
char** GetNeededLibsNames(library_t* lib)
{
    switch (lib->type) {
        case LIB_WRAPPED: return lib->w.needed?lib->w.needed->names:NULL;
        case LIB_EMULATED: return lib->e.elf->needed?lib->e.elf->needed->names:NULL;
    }
    return NULL;
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

int GetDeepBind(library_t* lib)
{
    if(!lib)
        return 0;
    return lib->deepbind;
}

#ifdef BOX32
linkmap32_t* getLinkMapLib32(library_t* lib)
{
    linkmap32_t* lm = my_context->linkmap32;
    while(lm) {
        if(lm->l_lib == lib)
            return lm;
        lm = (linkmap32_t*)from_ptrv(lm->l_next);
    }
    return NULL;
}
linkmap32_t* getLinkMapElf32(elfheader_t* h)
{
    linkmap32_t* lm = my_context->linkmap32;
    while(lm) {
        if(lm->l_lib && lm->l_lib->type==LIB_EMULATED && lm->l_lib->e.elf == h)
            return lm;
        lm = (linkmap32_t*)from_ptrv(lm->l_next);
    }
    return NULL;
}
linkmap32_t* addLinkMapLib32(library_t* lib)
{
    if(!my_context->linkmap32) {
        my_context->linkmap32 = (linkmap32_t*)actual_calloc(1, sizeof(linkmap32_t));
        my_context->linkmap32->l_lib = lib;
        return my_context->linkmap32;
    }
    linkmap32_t* lm = my_context->linkmap32;
    while(lm->l_next)
        lm = (linkmap32_t*)from_ptrv(lm->l_next);
    lm->l_next = to_ptrv(actual_calloc(1, sizeof(linkmap32_t)));
    linkmap32_t* l_next = (linkmap32_t*)from_ptrv(lm->l_next);
    l_next->l_lib = lib;
    l_next->l_prev = to_ptrv(lm);
    return l_next;
}
void removeLinkMapLib32(library_t* lib)
{
    linkmap32_t* lm = getLinkMapLib32(lib);
    if(!lm) return;
    if(lm->l_next)
        ((linkmap32_t*)from_ptrv(lm->l_next))->l_prev = lm->l_prev;
    if(lm->l_prev)
        ((linkmap32_t*)from_ptrv(lm->l_prev))->l_next = lm->l_next;
    actual_free(lm);
}

void AddMainElfToLinkmap32(elfheader_t* elf)
{
    linkmap32_t* lm = addLinkMapLib32(NULL);    // main elf will have a null lib link

    lm->l_addr = (Elf32_Addr)to_ptrv(GetElfDelta(elf));
    lm->l_name = to_cstring(my_context->fullpath);
    lm->l_ld = to_ptrv(GetDynamicSection(elf));
}
#endif

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
linkmap_t* getLinkMapElf(elfheader_t* h)
{
    linkmap_t* lm = my_context->linkmap;
    while(lm) {
        if(lm->l_lib && lm->l_lib->type==LIB_EMULATED && lm->l_lib->e.elf == h)
            return lm;
        lm = lm->l_next;
    }
    return NULL;
}
linkmap_t* addLinkMapLib(library_t* lib)
{
    if(!my_context->linkmap) {
        my_context->linkmap = (linkmap_t*)actual_calloc(1, sizeof(linkmap_t));
        my_context->linkmap->l_lib = lib;
        return my_context->linkmap;
    }
    linkmap_t* lm = my_context->linkmap;
    while(lm->l_next)
        lm = lm->l_next;
    lm->l_next = (linkmap_t*)actual_calloc(1, sizeof(linkmap_t));
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
    actual_free(lm);
}

void AddMainElfToLinkmap(elfheader_t* elf)
{
    linkmap_t* lm = addLinkMapLib(NULL);    // main elf will have a null lib link

    lm->l_addr = (Elf64_Addr)GetElfDelta(elf);
    lm->l_name = my_context->fullpath;
    lm->l_ld = GetDynamicSection(elf);
}

needed_libs_t* new_neededlib(int n)
{
    needed_libs_t* ret = (needed_libs_t*)box_calloc(1, sizeof(needed_libs_t));
    ret->cap = ret->size = n;
    ret->init_size = n;
    ret->libs = (library_t**)box_calloc(n, sizeof(library_t*));
    ret->names = (char**)box_calloc(n, sizeof(char*));
    return ret;
}
void free_neededlib(needed_libs_t* needed)
{
    if(!needed)
        return;
    box_free(needed->libs);
    box_free(needed->names);
    needed->libs = NULL;
    needed->names = NULL;
    needed->cap = needed->size = 0;
    box_free(needed);
}
void add1_neededlib(needed_libs_t* needed)
{
    if(!needed)
        return;
    if(needed->size+1<=needed->cap)
        return;
    needed->cap = needed->size+1;
    needed->libs = (library_t**)box_realloc(needed->libs, needed->cap*sizeof(library_t*));
    needed->names = (char**)box_realloc(needed->names, needed->cap*sizeof(char*));
    needed->size++;
    needed->init_size++;
}
void add1lib_neededlib(needed_libs_t* needed, library_t* lib, const char* name)
{
    if(!needed || !lib)
        return;
    // check if lib is already present
    for (int i=0; i<needed->size; ++i)
        if(needed->libs[i]==lib)
            return;
    // add it
    if(needed->size==needed->cap) {
        needed->cap = needed->size+1;
        needed->libs = (library_t**)box_realloc(needed->libs, needed->cap*sizeof(library_t*));
        needed->names = (char**)box_realloc(needed->names, needed->cap*sizeof(char*));
    }
    needed->libs[needed->size] = lib;
    needed->names[needed->size] = (char*)name;
    needed->size++;
    needed->init_size++;
}
void add1lib_neededlib_name(needed_libs_t* needed, library_t* lib, const char* name)
{
    if(!needed || !name)
        return;
    // check if lib is already present
    for (int i=0; i<needed->size; ++i)
        if(!strcmp(needed->names[i], name))
            return;
    // add it
    if(needed->size==needed->cap) {
        needed->cap = needed->size+1;
        needed->libs = (library_t**)box_realloc(needed->libs, needed->cap*sizeof(library_t*));
        needed->names = (char**)box_realloc(needed->names, needed->cap*sizeof(char*));
    }
    needed->libs[needed->size] = lib;
    needed->names[needed->size] = (char*)name;
    needed->size++;
    needed->init_size++;
}
void add1libref_neededlib(needed_libs_t* needed, library_t* lib)
{
    if(!needed || !lib)
        return;
    // check if lib is already present
    for (int i=0; i<needed->size; ++i)
        if(needed->libs[i]==lib)
            return;
    // add it
    if(needed->size==needed->cap) {
        needed->cap = needed->size+4;
        needed->libs = (library_t**)box_realloc(needed->libs, needed->cap*sizeof(library_t*));
        needed->names = (char**)box_realloc(needed->names, needed->cap*sizeof(char*));
    }
    needed->libs[needed->size] = lib;
    needed->names[needed->size] = (char*)lib->name;
    needed->size++;
    IncRefCount(lib, NULL);
}
needed_libs_t* copy_neededlib(needed_libs_t* needed)
{
    if(!needed)
        return NULL;
    needed_libs_t* ret = (needed_libs_t*)box_calloc(1, sizeof(needed_libs_t));
    ret->cap = needed->cap;
    ret->size = needed->size;
    ret->init_size = needed->init_size;
    ret->libs = (library_t**)box_calloc(ret->cap, sizeof(library_t*));
    ret->names = (char**)box_calloc(ret->cap, sizeof(char*));
    memcpy(ret->libs, needed->libs, ret->size*sizeof(library_t*));
    memcpy(ret->names, needed->names, ret->size*sizeof(char*));
    return ret;
}

void setNeededLibs(library_t* lib, int n, ...)
{
    if(lib->type!=LIB_WRAPPED && lib->type!=LIB_UNNKNOW)
        return;
    lib->w.needed = new_neededlib(n);
    va_list va;
    va_start (va, n);
    for (int i=0; i<n; ++i) {
        lib->w.needed->names[i] = va_arg(va, char*);
    }
    va_end (va);
}

#define LIB_MAXCNT 255
void IncRefCount(library_t* lib, x64emu_t* emu)
{
    if(!lib || lib->type==LIB_UNNKNOW) {
        printf_log(LOG_NONE, "Warning, IncRefCount of a LIB_UNKNOWN library\n");
        return;
    }
    switch (lib->type) {
        case LIB_WRAPPED:
            if(lib->w.refcnt==LIB_MAXCNT)
                return;
            ++lib->w.refcnt;
            break;
        case LIB_EMULATED:
            if(lib->e.elf->refcnt==LIB_MAXCNT)
                return;
            ++lib->e.elf->refcnt;
    }
}

int DecRefCount(library_t** lib, x64emu_t* emu)
{
    if(!lib || !*lib)
        return 1;
    if((*lib)->type==LIB_UNNKNOW) {
        Free1Library(lib, emu);
        return 0;
    }
    int ret = 1;
    switch ((*lib)->type) {
        case LIB_WRAPPED:
            if((*lib)->w.refcnt==LIB_MAXCNT)
                return ret;
            ret=--(*lib)->w.refcnt;
            if(!ret) {
                Free1Library(lib, emu);
            }
            break;
        case LIB_EMULATED:
            if(!(*lib)->e.elf)
                ret = 0;
            else {
                if((*lib)->e.elf->refcnt==LIB_MAXCNT)
                    return ret;
                ret=--(*lib)->e.elf->refcnt;
            }
            if(!ret) {
                Free1Library(lib, emu);
            }
            break;
    }
    return ret;
}

int GetRefCount(library_t* lib)
{
    switch (lib->type) {
        case LIB_WRAPPED:
            return lib->w.refcnt;
            break;
        case LIB_EMULATED:
            return lib->e.elf->refcnt;
        default:
            return 0;
    }
}

void SetDlOpenIdx(library_t* lib, int dlopen)
{
    if(lib)
        lib->dlopen = dlopen;
}
