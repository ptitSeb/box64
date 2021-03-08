#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <elf.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "emu/x64emu_private.h"
#include "library.h"
#include "librarian.h"
#include "box64context.h"
#include "elfloader.h"
#include "elfs/elfloader_private.h"

typedef struct dlprivate_s {
    library_t   **libs;
    size_t      *count;
    size_t      *dlopened;
    size_t      lib_sz;
    size_t      lib_cap;
    char*       last_error;
} dlprivate_t;

dlprivate_t *NewDLPrivate() {
    dlprivate_t* dl =  (dlprivate_t*)calloc(1, sizeof(dlprivate_t));
    return dl;
}
void FreeDLPrivate(dlprivate_t **lib) {
    free((*lib)->last_error);
    free(*lib);
}

void* my_dlopen(x64emu_t* emu, void *filename, int flag) EXPORT;
void* my_dlmopen(x64emu_t* emu, void* mlid, void *filename, int flag) EXPORT;
char* my_dlerror(x64emu_t* emu) EXPORT;
void* my_dlsym(x64emu_t* emu, void *handle, void *symbol) EXPORT;
int my_dlclose(x64emu_t* emu, void *handle) EXPORT;
int my_dladdr(x64emu_t* emu, void *addr, void *info) EXPORT;
void* my_dlvsym(x64emu_t* emu, void *handle, void *symbol, void *version) EXPORT;
int my_dlinfo(x64emu_t* emu, void* handle, int request, void* info) EXPORT;

#define LIBNAME libdl
const char* libdlName = "libdl.so.2";

// define all standard library functions
#include "wrappedlib_init.h"

#define CLEARERR    if(dl->last_error) free(dl->last_error); dl->last_error = NULL;

extern int box64_zoom;
// Implementation
void* my_dlopen(x64emu_t* emu, void *filename, int flag)
{
    //void *dlopen(const char *filename, int flag);
    // TODO, handling special values for filename, like RTLD_SELF?
    // TODO, handling flags?
    library_t *lib = NULL;
    dlprivate_t *dl = emu->context->dlprivate;
    size_t dlopened = 0;
    int is_local = (flag&0x100)?0:1;  // if not global, then local, and that means symbols are not put in the global "pot" for pther libs
    CLEARERR
    if(filename) {
        char* rfilename = (char*)filename;
        if(box64_zoom && strstr(rfilename, "/libturbojpeg.so")) {
            void* sys = my_dlopen(emu, "libturbojpeg.so.0", flag);
            if(sys)
                return sys;
        }
        if(dlsym_error && box64_log<LOG_DEBUG) {
            printf_log(LOG_NONE, "Call to dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
        }
        printf_log(LOG_DEBUG, "Call to dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
        // check if alread dlopenned...
        for (size_t i=0; i<dl->lib_sz; ++i) {
            if(IsSameLib(dl->libs[i], rfilename)) {
                if(dl->count[i]==0 && dl->dlopened[i]) {   // need to lauch init again!
                    int idx = GetElfIndex(dl->libs[i]);
                    if(idx!=-1) {
                        if(dlsym_error || box64_log>=LOG_DEBUG) {
                                printf_log(LOG_NONE, "dlopen: Recycling, calling Init for %p (%s)\n", (void*)(i+1), rfilename);
                        }
                        ReloadLibrary(dl->libs[i], emu);    // reset memory image, redo reloc, run inits
                    }
                }
                dl->count[i] = dl->count[i]+1;
                if(dlsym_error || box64_log>=LOG_DEBUG) {
                        printf_log(LOG_NONE, "dlopen: Recycling %s/%p count=%ld (dlopened=%ld, elf_index=%d)\n", rfilename, (void*)(i+1), dl->count[i], dl->dlopened[i], GetElfIndex(dl->libs[i]));
                }
                return (void*)(i+1);
            }
        }
        dlopened = (GetLibInternal(rfilename)==NULL);
        // Then open the lib
        if(AddNeededLib(NULL, NULL, is_local, rfilename, emu->context, emu)) {
            printf_log(LOG_INFO, "Warning: Cannot dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
            if(!dl->last_error)
                dl->last_error = malloc(129);
            snprintf(dl->last_error, 129, "Cannot dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
            return NULL;
        }
        lib = GetLibInternal(rfilename);
    } else {
        // check if already dlopenned...
        for (size_t i=0; i<dl->lib_sz; ++i) {
            if(!dl->libs[i]) {
                dl->count[i] = dl->count[i]+1;
                return (void*)(i+1);
            }
        }
        if(dlsym_error && box64_log<LOG_DEBUG) {
            printf_log(LOG_NONE, "Call to dlopen(NULL, %X)\n", flag);
        }
        printf_log(LOG_DEBUG, "Call to dlopen(NULL, %X)\n", flag);
    }
    //get the lib and add it to the collection
    
    if(dl->lib_sz == dl->lib_cap) {
        dl->lib_cap += 4;
        dl->libs = (library_t**)realloc(dl->libs, sizeof(library_t*)*dl->lib_cap);
        dl->count = (size_t*)realloc(dl->count, sizeof(size_t)*dl->lib_cap);
        dl->dlopened = (size_t*)realloc(dl->dlopened, sizeof(size_t)*dl->lib_cap);
        // memset count...
        memset(dl->count+dl->lib_sz, 0, (dl->lib_cap-dl->lib_sz)*sizeof(size_t));
    }
    dl->libs[dl->lib_sz] = lib;
    dl->count[dl->lib_sz] = dl->count[dl->lib_sz]+1;
    dl->dlopened[dl->lib_sz] = dlopened;
    if(dlsym_error || box64_log>=LOG_DEBUG) {
        printf_log(LOG_NONE, "dlopen: New handle %p (%s), dlopened=%ld\n", (void*)(dl->lib_sz+1), (char*)filename, dlopened);
    }
    return (void*)(++dl->lib_sz);
}
void* my_dlmopen(x64emu_t* emu, void* lmid, void *filename, int flag)
{
    if(lmid) {
        printf_log(LOG_INFO, "Warning, dlmopen(%p, %p(\"%s\"), 0x%x) called with lmid not LMID_ID_BASE (unsupported)\n", lmid, filename, filename?(char*)filename:"self", flag);
    }
    // lmid is ignored for now...
    return my_dlopen(emu, filename, flag);
}
char* my_dlerror(x64emu_t* emu)
{
    dlprivate_t *dl = emu->context->dlprivate;
    return dl->last_error;
}

KHASH_SET_INIT_INT(libs);

int recursive_dlsym_lib(kh_libs_t* collection, library_t* lib, const char* rsymbol, uintptr_t *start, uintptr_t *end)
{
    if(!lib)
        return 0;
    khint_t k = kh_get(libs, collection, (uintptr_t)lib);
    if(k != kh_end(collection))
        return 0;
    int ret;
    kh_put(libs, collection, (uintptr_t)lib, &ret);
    // look in the library itself
    if(lib->get(lib, rsymbol, start, end))
        return 1;
    // look in other libs
    int n = GetNeededLibN(lib);
    for (int i=0; i<n; ++i) {
        library_t *l = GetNeededLib(lib, i);
        if(recursive_dlsym_lib(collection, l, rsymbol, start, end))
            return 1;
    }
        
    return 0;
}

int my_dlsym_lib(library_t* lib, const char* rsymbol, uintptr_t *start, uintptr_t *end)
{
    kh_libs_t *collection = kh_init(libs);
    int ret = recursive_dlsym_lib(collection, lib, rsymbol, start, end);
    kh_destroy(libs, collection);

    return ret;
}
void* my_dlsym(x64emu_t* emu, void *handle, void *symbol)
{
    dlprivate_t *dl = emu->context->dlprivate;
    uintptr_t start = 0, end = 0;
    char* rsymbol = (char*)symbol;
    CLEARERR
    if(dlsym_error && box64_log<LOG_DEBUG) {
        printf_log(LOG_NONE, "Call to dlsym(%p, %s) :", handle, rsymbol);
    }
    printf_log(LOG_DEBUG, "Call to dlsym(%p, \"%s\")\n", handle, rsymbol);
    if(handle==NULL) {
        // special case, look globably
        if(GetGlobalSymbolStartEnd(emu->context->maplib, rsymbol, &start, &end)) {
            if(dlsym_error && box64_log<LOG_DEBUG) {
                printf_log(LOG_NONE, "%p\n", (void*)start);
            }
            return (void*)start;
        }
        if(!dl->last_error)
            dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Symbol \"%s\" not found in %p)\n", rsymbol, handle);
        if(dlsym_error && box64_log<LOG_DEBUG) {
            printf_log(LOG_NONE, "%p\n", NULL);
        }
        return NULL;
    }
    if(handle==(void*)0xFFFFFFFF) {
        // special case, look globably but no self (RTLD_NEXT)
        elfheader_t *elf = FindElfAddress(emu->context, *(uint32_t*)R_RSP); // use return address to guess "self"
        if(GetNoSelfSymbolStartEnd(emu->context->maplib, rsymbol, &start, &end, elf)) {
            if(dlsym_error && box64_log<LOG_DEBUG) {
                printf_log(LOG_NONE, "%p\n", (void*)start);
            }
            return (void*)start;
        }
        if(!dl->last_error)
            dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Symbol \"%s\" not found in %p)\n", rsymbol, handle);
        if(dlsym_error && box64_log<LOG_DEBUG) {
            printf_log(LOG_NONE, "%p\n", NULL);
        }
        return NULL;
    }
    size_t nlib = (size_t)handle;
    --nlib;
    if(nlib<0 || nlib>=dl->lib_sz) {
        if(!dl->last_error)
            dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p)\n", handle);
        if(dlsym_error && box64_log<LOG_DEBUG) {
            printf_log(LOG_NONE, "%p\n", NULL);
        }
        return NULL;
    }
    if(dl->count[nlib]==0) {
        if(!dl->last_error)
            dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p (already closed))\n", handle);
        if(dlsym_error && box64_log<LOG_DEBUG) {
            printf_log(LOG_NONE, "%p\n", (void*)NULL);
        }
        return NULL;
    }
    if(dl->libs[nlib]) {
        if(my_dlsym_lib(dl->libs[nlib], rsymbol, &start, &end)==0) {
            // not found
            if(dlsym_error && box64_log<LOG_DEBUG) {
                printf_log(LOG_NONE, "%p\nCall to dlsym(%s, \"%s\") Symbol not found\n", NULL, GetNameLib(dl->libs[nlib]), rsymbol);
            }
            printf_log(LOG_DEBUG, " Symbol not found\n");
            if(!dl->last_error)
                dl->last_error = malloc(129);
            snprintf(dl->last_error, 129, "Symbol \"%s\" not found in %p(%s)", rsymbol, handle, GetNameLib(dl->libs[nlib]));
            return NULL;
        }
    } else {
        // still usefull?
        if(GetSymbolStartEnd(GetLocalSymbol(emu->context->maplib), rsymbol, &start, &end)) {
            if(dlsym_error && box64_log<LOG_DEBUG) {
                printf_log(LOG_NONE, "%p\n", (void*)start);
            }
            return (void*)start;
        }
        if(GetSymbolStartEnd(GetWeakSymbol(emu->context->maplib), rsymbol, &start, &end)) {
            if(dlsym_error && box64_log<LOG_DEBUG) {
                printf_log(LOG_NONE, "%p\n", (void*)start);
            }
            return (void*)start;
        }
        if(GetSymbolStartEnd(GetMapSymbol(emu->context->maplib), rsymbol, &start, &end)) {
            if(dlsym_error && box64_log<LOG_DEBUG) {
                printf_log(LOG_NONE, "%p\n", (void*)start);
            }
            return (void*)start;
        }
        // not found
        if(dlsym_error && box64_log<LOG_DEBUG) {
            printf_log(LOG_NONE, "%p\nCall to dlsym(%s, \"%s\") Symbol not found\n", NULL, "Self", rsymbol);
        }
        printf_log(LOG_DEBUG, " Symbol not found\n");
        if(!dl->last_error)
            dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Symbol \"%s\" not found in %p)\n", rsymbol, handle);
        return NULL;
    }
    if(dlsym_error && box64_log<LOG_DEBUG) {
        printf_log(LOG_NONE, "%p\n", (void*)start);
    }
    return (void*)start;
}
int my_dlclose(x64emu_t* emu, void *handle)
{
    if(dlsym_error || box64_log>=LOG_DEBUG) {
            printf_log(LOG_NONE, "Call to dlclose(%p)\n", handle);
    }
    dlprivate_t *dl = emu->context->dlprivate;
    CLEARERR
    size_t nlib = (size_t)handle;
    --nlib;
    if(nlib<0 || nlib>=dl->lib_sz) {
        if(!dl->last_error)
            dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p)\n", handle);
            if(dlsym_error || box64_log>=LOG_DEBUG) {
                printf_log(LOG_NONE, "dlclose: %s\n", dl->last_error);
            }
        return -1;
    }
    if(dl->count[nlib]==0) {
        if(!dl->last_error)
            dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p (already closed))\n", handle);
        if(dlsym_error || box64_log>=LOG_DEBUG) {
            printf_log(LOG_NONE, "dlclose: %s\n", dl->last_error);
        }
        return -1;
    }
    dl->count[nlib] = dl->count[nlib]-1;
    if(dl->count[nlib]==0 && dl->dlopened[nlib]) {   // need to call Fini...
        int idx = GetElfIndex(dl->libs[nlib]);
        if(idx!=-1) {
            if(dlsym_error || box64_log>=LOG_DEBUG) {
                printf_log(LOG_NONE, "dlclose: Call to Fini for %p\n", handle);
            }
            RunElfFini(emu->context->elfs[idx], emu);
            InactiveLibrary(dl->libs[nlib]);
        }
    }
    return 0;
}
int my_dladdr(x64emu_t* emu, void *addr, void *i)
{
    //int dladdr(void *addr, Dl_info *info);
    dlprivate_t *dl = emu->context->dlprivate;
    CLEARERR
    Dl_info *info = (Dl_info*)i;
    printf_log(LOG_DEBUG, "Warning: partially unimplement call to dladdr(%p, %p)\n", addr, info);
    
    //emu->quit = 1;
    info->dli_saddr = NULL;
    info->dli_fname = NULL;
    info->dli_sname = FindSymbolName(emu->context->maplib, addr, &info->dli_saddr, NULL, &info->dli_fname, &info->dli_fbase);
    printf_log(LOG_DEBUG, "     dladdr return saddr=%p, fname=\"%s\", sname=\"%s\"\n", info->dli_saddr, info->dli_sname?info->dli_sname:"", info->dli_fname?info->dli_fname:"");
    return (info->dli_sname)?1:0;   // success is non-null here...
}
void* my_dlvsym(x64emu_t* emu, void *handle, void *symbol, void *version)
{
    dlprivate_t *dl = emu->context->dlprivate;
    CLEARERR
    //void *dlvsym(void *handle, char *symbol, char *version);
    char* rsymbol = (char*)symbol;
    char* rversion = (char*)version;
    printf_log(LOG_INFO, "Warning: unimplement call to dlvsym(%p, %s, %s), fallback to dlsym\n", handle, rsymbol, rversion);

    return my_dlsym(emu, handle, symbol);
}

typedef struct link_map_s {
    uintptr_t   l_addr;
    char*       l_name;
    Elf64_Dyn*  l_ld;
} link_map_t;

int my_dlinfo(x64emu_t* emu, void* handle, int request, void* info)
{
    if(dlsym_error || box64_log>=LOG_DEBUG) {
            printf_log(LOG_NONE, "Call to dlinfo(%p, %d, %p)\n", handle, request, info);
    }
    dlprivate_t *dl = emu->context->dlprivate;
    CLEARERR
    size_t nlib = (size_t)handle;
    --nlib;
    if(nlib<0 || nlib>=dl->lib_sz) {
        if(!dl->last_error)
            dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p)\n", handle);
            if(dlsym_error || box64_log>=LOG_DEBUG) {
                printf_log(LOG_NONE, "dlinfo: %s\n", dl->last_error);
            }
        return -1;
    }
    if(dl->count[nlib]==0) {
        if(!dl->last_error)
            dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "Bad handle %p (already closed))\n", handle);
        if(dlsym_error || box64_log>=LOG_DEBUG) {
            printf_log(LOG_NONE, "dlinfo: %s\n", dl->last_error);
        }
        return -1;
    }
    library_t *lib = dl->libs[nlib];
    elfheader_t *h = (GetElfIndex(lib)>-1)?my_context->elfs[GetElfIndex(lib)]:NULL;
    switch(request) {
        case 2: // RTLD_DI_LINKMAP
            {
                static link_map_t map = {0};   //cheating, creating a structure on demand...
                *(link_map_t**)info = &map;
                map.l_addr = h?h->delta:0;
                map.l_name = lib->path;
                map.l_ld = h?h->Dynamic:NULL;
            }
            return 0;
        default:
            printf_log(LOG_NONE, "Warning, unsupported call to dlinfo(%p, %d, %p)\n", handle, request, info);
        if(!dl->last_error)
            dl->last_error = malloc(129);
        snprintf(dl->last_error, 129, "unsupported call to dlinfo request:%d\n", request);
    }
    return -1;
}