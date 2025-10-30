#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <elf.h>
#include <link.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "emu/x64emu_private.h"
#include "library.h"
#include "librarian.h"
#include "box64context.h"
#include "x64tls.h"
#include "elfloader.h"
#include "elfs/elfloader_private.h"

typedef struct dllib_s {
    library_t*  lib;
    int         count;
    int         dlopened;
    int         is_self;
    int         full;
} dllib_t;

#define ERROR_LEN 128
typedef struct dlprivate_s {
    dllib_t     *dllibs;
    size_t      lib_sz;
    size_t      lib_cap;
    char        error_msg[ERROR_LEN];
    char        last_msg[ERROR_LEN];
    int         is_error;
} dlprivate_t;

dlprivate_t *NewDLPrivate() {
    dlprivate_t* dl =  (dlprivate_t*)box_calloc(1, sizeof(dlprivate_t));
    return dl;
}
void FreeDLPrivate(dlprivate_t **lib) {
    box_free(*lib);
}

// dead_cells consider the "2" value to be some king of issue?
#define MIN_NLIB 3
int GetTID();

void* my_dlopen(x64emu_t* emu, void *filename, int flag) EXPORT;
void* my_dlmopen(x64emu_t* emu, void* mlid, void *filename, int flag) EXPORT;
char* my_dlerror(x64emu_t* emu) EXPORT;
void* my_dlsym(x64emu_t* emu, void *handle, void *symbol) EXPORT;
int my_dlclose(x64emu_t* emu, void *handle) EXPORT;
int my_dladdr(x64emu_t* emu, void *addr, void *info) EXPORT;
int my_dladdr1(x64emu_t* emu, void *addr, void *info, void** extra_info, int flags) EXPORT;
void* my_dlvsym(x64emu_t* emu, void *handle, void *symbol, const char *vername) EXPORT;
int my_dlinfo(x64emu_t* emu, void* handle, int request, void* info) EXPORT;

#define LIBNAME libdl
const char* libdlName = "libdl.so.2";
#define ALTNAME "libdl.so"

#define CLEARERR    dl->is_error = 0
void dl_clear_error()
{
    dlprivate_t *dl = my_context->dlprivate;
    CLEARERR;
}

void dl_set_error(const char* msg)
{
    dlprivate_t *dl = my_context->dlprivate;
    snprintf(dl->last_msg, ERROR_LEN, "%s", msg);
    dl->is_error = 1;
}

#define SET_ERROR(M, ...) do { snprintf(dl->last_msg, ERROR_LEN, M, __VA_ARGS__); dl->is_error=1; } while(0)

library_t* dl_get_library(void* handle)
{
    dlprivate_t *dl = my_context->dlprivate;
    CLEARERR;
    size_t nlib = (size_t)handle;
    --nlib;
    // size_t is unsigned
    if(nlib>=dl->lib_sz) {
        SET_ERROR("Bad handle %p)\n", handle);
        return (void*)-1LL;
    }
    if(!dl->dllibs[nlib].count || !dl->dllibs[nlib].full) {
        SET_ERROR("Bad handle %p (already closed))\n", handle);
        return (void*)-1LL;
    }
    return dl->dllibs[nlib].lib;
}
#ifdef BOX32
char* dl_last_error()
{
    dlprivate_t *dl = my_context->dlprivate;
    return dl->last_msg;
}
#endif
void RemoveDlopen(library_t** lib, size_t idx)
{
    if(!my_context)
        return;
    dlprivate_t *dl = my_context->dlprivate;
    if(dl && idx<dl->lib_sz) {
        if(lib!=&dl->dllibs[idx].lib)
            dl->dllibs[idx].lib = NULL;
        dl->dllibs[idx].count = 0;
        dl->dllibs[idx].dlopened = 0;
        dl->dllibs[idx].full = 0;
    }
}

extern int box64_zoom;
void setupTraceMapLib(lib_t* maplib);
// Implementation
void* my_dlopen(x64emu_t* emu, void *filename, int flag)
{
    //void *dlopen(const char *filename, int flag);
    // TODO, handling special values for filename, like RTLD_SELF?
    // TODO, handling flags?
    library_t *lib = NULL;
    dlprivate_t *dl = my_context->dlprivate;
    size_t dlopened = 0;
    int is_local = (flag&0x100)?0:1;  // if not global, then local, and that means symbols are not put in the global "pot" for other libs
    int deepbind = (flag&0x0008)?1:0;  // RTLD_DEEPBIND means LOCAL before GLOBAL scope during symbol resolution
    CLEARERR;
    if(filename) {
        char* rfilename = (char*)alloca(MAX_PATH);
        strcpy(rfilename, (char*)filename);
        if(box64_zoom && rfilename && strstr(rfilename, "/libturbojpeg.so")) {
            void* sys = my_dlopen(emu, "libturbojpeg.so.0", flag);
            if(sys)
                return sys;
        }
        if(!strcmp(rfilename, "/usr/lib/x86_64-linux-gnu/d3d")) {
            void* sys = my_dlopen(emu, "d3dadapter9.so.1", flag);
            if(sys)
                return sys;
        }
        printf_dlsym(LOG_DEBUG, "%04d|Call to dlopen(\"%s\"/%p, %X)\n", GetTID(), rfilename, filename, flag);
        // Transform any ${...} that maight be present
        while(strstr(rfilename, "${ORIGIN}")) {
            char* origin = box_strdup(my_context->fullpath);
            char* p = strrchr(origin, '/');
            if(p) *p = '\0';    // remove file name to have only full path, without last '/'
            char* tmp = (char*)box_calloc(1, strlen(rfilename)-strlen("${ORIGIN}")+strlen(origin)+1);
            p = strstr(rfilename, "${ORIGIN}");
            memcpy(tmp, rfilename, p-rfilename);
            strcat(tmp, origin);
            strcat(tmp, p+strlen("${ORIGIN}"));
            strcpy(rfilename, tmp);
            box_free(tmp);
            box_free(origin);
        }
        while(strstr(rfilename, "${PLATFORM}")) {
            char* platform = box_strdup("x86_64");
            char* p = strrchr(platform, '/');
            if(p) *p = '\0';    // remove file name to have only full path, without last '/'
            char* tmp = (char*)box_calloc(1, strlen(rfilename)-strlen("${PLATFORM}")+strlen(platform)+1);
            p = strstr(rfilename, "${PLATFORM}");
            memcpy(tmp, rfilename, p-rfilename);
            strcat(tmp, platform);
            strcat(tmp, p+strlen("${PLATFORM}"));
            strcpy(rfilename, tmp);
            box_free(tmp);
            box_free(platform);
        }
        // check if already dlopened...
        for (size_t i=MIN_NLIB; i<dl->lib_sz; ++i) {
            if(dl->dllibs[i].full && IsSameLib(dl->dllibs[i].lib, rfilename)) {
                if(flag&0x4) {   // don't re-open in RTLD_NOLOAD mode
                    if(dl->dllibs[i].count==0 && dl->dllibs[i].dlopened) {
                        printf_dlsym(LOG_DEBUG, " => not present anymore\n");
                        return NULL;    // don't re-open in RTLD_NOLOAD mode
                    }
                }
                IncRefCount(dl->dllibs[i].lib, emu);
                ++dl->dllibs[i].count;
                if(!is_local && isLibLocal(dl->dllibs[i].lib))
                    promoteLocalLibGlobal(dl->dllibs[i].lib);
                printf_dlsym(LOG_DEBUG, "dlopen: Recycling %s/%p count=%ld (dlopened=%ld, elf_index=%d)\n", rfilename, (void*)(i+1), dl->dllibs[i].count, dl->dllibs[i].dlopened, GetElfIndex(dl->dllibs[i].lib));
                return (void*)(i+1);
            }
        }
        lib = GetLibInternal(rfilename);
        if(flag&0x4) {   //RTLD_NOLOAD is just a "check" if lib is already loaded
            if(lib) {
                if(dl->lib_sz == dl->lib_cap) {
                    dl->lib_cap += 4;
                    dl->dllibs = (dllib_t*)box_realloc(dl->dllibs, sizeof(dllib_t)*dl->lib_cap);
                    // memset count...
                    memset(dl->dllibs+dl->lib_sz, 0, (dl->lib_cap-dl->lib_sz)*sizeof(dllib_t));
                    if(!dl->lib_sz)
                        dl->lib_sz = MIN_NLIB;
                }
                intptr_t idx = dl->lib_sz++;
                dl->dllibs[idx].lib = lib;
                ++dl->dllibs[idx].count;
                dl->dllibs[idx].dlopened = dlopened;
                dl->dllibs[idx].is_self = lib?0:1;
                dl->dllibs[idx].full = 1;
                IncRefCount(dl->dllibs[idx].lib, emu);
                SetDlOpenIdx(lib, idx);
                refreshTLSData(emu);
                printf_dlsym(LOG_DEBUG, "dlopen: New handle %p (%s), dlopened=%ld\n", (void*)(idx+1), (char*)filename, dlopened);
                return (void*)(idx+1);

            }
            printf_dlsym(LOG_DEBUG, " => not present\n");
            return NULL;
        }
        dlopened = (lib==NULL);
        // cleanup the old deferredInit state
        int old_deferredInit = my_context->deferredInit;
        my_context->deferredInit = 1;
        elfheader_t** old_deferredInitList = my_context->deferredInitList;
        my_context->deferredInitList = NULL;
        int old_deferredInitSz = my_context->deferredInitSz;
        int old_deferredInitCap = my_context->deferredInitCap;
        my_context->deferredInitSz = my_context->deferredInitCap = 0;

        // Then open the lib
        int bindnow = (!box64_musl && (flag&0x2))?1:0;
        needed_libs_t *tmp = new_neededlib(1);
        tmp->names[0] = rfilename;
        if(AddNeededLib(NULL, is_local, bindnow, deepbind, tmp, NULL, my_context, emu)) {
            printf_dlsym(strchr(rfilename,'/')?LOG_DEBUG:LOG_INFO, "Warning: Cannot dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
            SET_ERROR("Cannot dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
            RemoveNeededLib(NULL, is_local, tmp, my_context, emu);
            if(my_context->deferredInitList)
                box_free(my_context->deferredInitList);
            my_context->deferredInit = old_deferredInit;
            my_context->deferredInitList = old_deferredInitList;
            my_context->deferredInitSz = old_deferredInitSz;
            my_context->deferredInitCap = old_deferredInitCap;
            return NULL;
        }
        setupTraceMapLib(tmp->libs[0]->maplib);
        free_neededlib(tmp);
        lib = GetLibInternal(rfilename);
        RunDeferredElfInit(emu);
        my_context->deferredInit = old_deferredInit;
        my_context->deferredInitList = old_deferredInitList;
        my_context->deferredInitSz = old_deferredInitSz;
        my_context->deferredInitCap = old_deferredInitCap;
    } else {
        // check if already dlopened...
        for (size_t i=MIN_NLIB; i<dl->lib_sz; ++i) {
            if(dl->dllibs[i].is_self) {
                ++dl->dllibs[i].count;
                return (void*)(i+1);
            }
        }
        printf_dlsym(LOG_DEBUG, "Call to dlopen(NULL, %X)\n", flag);
    }
    //get the lib and add it to the collection
    
    if(dl->lib_sz == dl->lib_cap) {
        dl->lib_cap += 4;
        dl->dllibs = (dllib_t*)box_realloc(dl->dllibs, sizeof(dllib_t)*dl->lib_cap);
        // memset count...
        memset(dl->dllibs+dl->lib_sz, 0, (dl->lib_cap-dl->lib_sz)*sizeof(dllib_t));
        if(!dl->lib_sz)
            dl->lib_sz = MIN_NLIB;
    }
    intptr_t idx = dl->lib_sz++;
    dl->dllibs[idx].lib = lib;
    ++dl->dllibs[idx].count;
    dl->dllibs[idx].dlopened = dlopened;
    dl->dllibs[idx].is_self = lib?0:1;
    dl->dllibs[idx].full = 1;
    SetDlOpenIdx(lib, idx);
    printf_dlsym(LOG_DEBUG, "dlopen: New handle %p (%s), dlopened=%ld\n", (void*)(idx+1), (char*)filename, dlopened);
    return (void*)(idx+1);
}
void* my_dlmopen(x64emu_t* emu, void* lmid, void *filename, int flag)
{
    if(lmid) {
        printf_dlsym(LOG_INFO, "Warning, dlmopen(%p, %p(\"%s\"), 0x%x) called with lmid not LMID_ID_BASE (unsupported)\n", lmid, filename, filename?(char*)filename:"self", flag);
    }
    // lmid is ignored for now...
    return my_dlopen(emu, filename, flag);
}
char* my_dlerror(x64emu_t* emu)
{
    dlprivate_t *dl = my_context->dlprivate;
    //printf_dlsym(LOG_INFO, "call to dlerror(): \"%s\"\n", dl->last_error?dl->last_error:"(nil)"); //too chatty
    if(!dl->is_error)
        return NULL;
    strncpy(dl->error_msg, dl->last_msg, ERROR_LEN);
    dl->is_error = 0;
    return dl->error_msg;
}

KHASH_SET_INIT_INT(libs);

// TODO: deepbind is probably not followed correctly here
int recursive_dlsym_lib(kh_libs_t* collection, library_t* lib, const char* rsymbol, uintptr_t *start, uintptr_t *end, int* version, const char** vername, int* veropt)
{
    if(!lib)
        return 0;
    khint_t k = kh_get(libs, collection, (uintptr_t)lib);
    if(k != kh_end(collection))
        return 0;
    int ret;
    kh_put(libs, collection, (uintptr_t)lib, &ret);
    // TODO: should use librarian functions instead!
    int weak;
    // look in the library itself
    if(lib->getglobal(lib, rsymbol, start, end, 0, &weak, version, vername, 1, veropt, NULL))
        return 1;
    if(lib->getweak(lib, rsymbol, start, end, 0, &weak, version, vername, 1, veropt, NULL))
        return 1;
    // look in other libs
    int n = GetNeededLibsN(lib);
    for (int i=0; i<n; ++i) {
        library_t *l = GetNeededLib(lib, i);
        if(recursive_dlsym_lib(collection, l, rsymbol, start, end, version, vername, veropt))
            return 1;
    }
        
    return 0;
}

int my_dlsym_lib(library_t* lib, const char* rsymbol, uintptr_t *start, uintptr_t *end, int version, const char* vername)
{
    kh_libs_t *collection = kh_init(libs);
    int veropt = 0;
    int ret = recursive_dlsym_lib(collection, lib, rsymbol, start, end, &version, &vername, &veropt);
    kh_destroy(libs, collection);

    return ret;
}

void* my_dlsym(x64emu_t* emu, void *handle, void *symbol)
{
    (void)emu;
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&mutex);
    dlprivate_t *dl = my_context->dlprivate;
    uintptr_t start = 0, end = 0;
    char* rsymbol = (char*)symbol;
    if(box64_is32bits && handle==(void*)0xffffffff)
        handle = (void*)~0LL;
    CLEARERR;
    printf_dlsym(LOG_DEBUG, "%04d|Call to dlsym(%p, \"%s\")%s", GetTID(), handle, rsymbol, BOX64ENV(dlsym_error)?"":"\n");
    if(handle==NULL) {
        // special case, look globably
        if(GetGlobalSymbolStartEnd(my_context->maplib, rsymbol, &start, &end, NULL, -1, NULL, 0, NULL)) {
            printf_dlsym_prefix(0, LOG_NEVER, "%p\n", (void*)start);
            pthread_mutex_unlock(&mutex);
            return (void*)start;
        }
        SET_ERROR("Symbol \"%s\" not found in %p)\n", rsymbol, handle);
        printf_dlsym_prefix(0, LOG_NEVER, "%p\n", NULL);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    if(handle==(void*)~0LL) {
        // special case, look globably after self in the lm chain (RTLD_NEXT)
        uintptr_t ret_addr = 0;
        #ifdef BOX32
        if(box64_is32bits)
            ret_addr = from_ptri(ptr_t, R_ESP);
        else
        #endif
            ret_addr = *(uintptr_t*)R_RSP;
        elfheader_t *elf = FindElfAddress(my_context, ret_addr); // use return address to guess "self"
        if(GetNextSymbolStartEnd(my_context->maplib, rsymbol, &start, &end, elf, 0, -1, NULL, 0, NULL)) {
            printf_dlsym_prefix(0, LOG_NEVER, "%p\n", (void*)start);
            pthread_mutex_unlock(&mutex);
            return (void*)start;
        }
        SET_ERROR("Symbol \"%s\" not found in %p)\n", rsymbol, handle);
        printf_dlsym_prefix(0, LOG_NEVER, "%p\n", NULL);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    size_t nlib = (size_t)handle;
    --nlib;
    // size_t is unsigned
    if(nlib>=dl->lib_sz) {
        SET_ERROR("Bad handle %p)\n", handle);
        printf_dlsym_prefix(0, LOG_NEVER, "%p\n", NULL);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    if(!dl->dllibs[nlib].count || !dl->dllibs[nlib].full) {
        SET_ERROR("Bad handle %p (already closed))\n", handle);
        printf_dlsym_prefix(0, LOG_NEVER, "%p\n", (void*)NULL);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    if(dl->dllibs[nlib].lib) {
        if(my_dlsym_lib(dl->dllibs[nlib].lib, rsymbol, &start, &end, -1, NULL)==0) {
            // not found
            printf_dlsym_prefix(0, LOG_NEVER, "%p\nCall to dlsym(%s, \"%s\") Symbol not found\n", NULL, GetNameLib(dl->dllibs[nlib].lib), rsymbol);
            printf_log_prefix(0, LOG_DEBUG, " Symbol not found\n");
            SET_ERROR("Symbol \"%s\" not found in %p(%s)", rsymbol, handle, GetNameLib(dl->dllibs[nlib].lib));
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
    } else {
        // for "self" dlopen
        //  => look globably
        //look in main elf first
        int found = 0;
        if(ElfGetSymTabStartEnd(my_context->elfs[0], &start, &end, rsymbol))
            found = 1;
        if(!found && GetGlobalSymbolStartEnd(my_context->maplib, rsymbol, &start, &end, NULL, -1, NULL, 0, NULL)) 
            found = 1;
        if(!found && GetSymTabStartEnd(my_context->maplib, rsymbol, &start, &end))
            found = 1;
        if(found) {
            printf_dlsym_prefix(0, LOG_NEVER, "%p\n", (void*)start);
            pthread_mutex_unlock(&mutex);
            return (void*)start;
        }
        SET_ERROR("Symbol \"%s\" not found in %p)\n", rsymbol, handle);
        printf_dlsym_prefix(0, LOG_NEVER, "%p\n", NULL);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    printf_dlsym_prefix(0, LOG_NEVER, "%p\n", (void*)start);
    pthread_mutex_unlock(&mutex);
    return (void*)start;
}
static int actualy_closing = 0;
int my_dlclose(x64emu_t* emu, void *handle)
{
    (void)emu;
    printf_dlsym(LOG_DEBUG, "Call to dlclose(%p)\n", handle);
    dlprivate_t *dl = my_context->dlprivate;
    CLEARERR;
    size_t nlib = (size_t)handle;
    --nlib;
    // size_t is unsigned
    if(nlib>=dl->lib_sz) {
        SET_ERROR("Bad handle %p)\n", handle);
        printf_dlsym(LOG_DEBUG, "dlclose: %s\n", dl->last_msg);
        return -1;
    }
    if(!dl->dllibs[nlib].count || !dl->dllibs[nlib].full) {
        SET_ERROR("Bad handle %p (already closed))\n", handle);
        printf_dlsym(LOG_DEBUG, "dlclose: %s\n", dl->last_msg);
        return -1;
    }
    --dl->dllibs[nlib].count;
    elfheader_t* h = GetElf(dl->dllibs[nlib].lib);
    if((h && !h->gnuunique) || !h || actualy_closing)
        DecRefCount(&dl->dllibs[nlib].lib, emu);
    return 0;
}
#ifdef ANDROID
#ifndef RTLD_DL_SYMENT
#define RTLD_DL_SYMENT 1
#endif
#ifndef RTLD_DL_LINKMAP
#define RTLD_DL_LINKMAP 2
#endif
#endif

int my_dladdr1(x64emu_t* emu, void *addr, void *i, void** extra_info, int flags)
{
    //int dladdr(void *addr, Dl_info *info);
    dlprivate_t *dl = my_context->dlprivate;
    CLEARERR;
    Dl_info *info = (Dl_info*)i;
    printf_log(LOG_DEBUG, "Warning: partially unimplement call to dladdr/dladdr1(%p, %p, %p, %d)\n", addr, info, extra_info, flags);
    
    //emu->quit = 1;
    library_t* lib = NULL;
    info->dli_saddr = NULL;
    info->dli_fname = NULL;
    info->dli_sname = FindSymbolName(my_context->maplib, addr, &info->dli_saddr, NULL, &info->dli_fname, &info->dli_fbase, &lib);
    printf_log(LOG_DEBUG, "     dladdr return saddr=%p, fname=\"%s\", sname=\"%s\"\n", info->dli_saddr, info->dli_sname?info->dli_sname:"", info->dli_fname?info->dli_fname:"");
    if(flags==RTLD_DL_SYMENT) {
        printf_log(LOG_INFO, "Warning, unimplement call to dladdr1 with RTLD_DL_SYMENT flags\n");
    } else if (flags==RTLD_DL_LINKMAP) {
        printf_log(LOG_INFO, "Warning, partially unimplemented call to dladdr1 with RTLD_DL_LINKMAP flags for %s\n", lib?lib->name:"current process");
        *(linkmap_t**)extra_info = getLinkMapLib(lib);
    }
    return (info->dli_sname)?1:0;   // success is non-null here...
}
int my_dladdr(x64emu_t* emu, void *addr, void *i)
{
    return my_dladdr1(emu, addr, i, NULL, 0);
}
void* my_dlvsym(x64emu_t* emu, void *handle, void *symbol, const char *vername)
{
    dlprivate_t *dl = my_context->dlprivate;
    int version = (vername)?2:-1;
    uintptr_t start, end;
    char* rsymbol = (char*)symbol;
    if(box64_is32bits && handle==(void*)0xffffffff)
        handle = (void*)~0LL;
    CLEARERR;
    printf_dlsym(LOG_DEBUG, "Call to dlvsym(%p, \"%s\", %s)%s", handle, rsymbol, vername?vername:"(nil)", BOX64ENV(dlsym_error)?"":"\n");
    if(handle==NULL) {
        // special case, look globably
        if(GetGlobalSymbolStartEnd(my_context->maplib, rsymbol, &start, &end, NULL, version, vername, 0, NULL)) {
            printf_dlsym_prefix(0, LOG_NEVER, "%p\n", (void*)start);
            return (void*)start;
        }
        SET_ERROR("Symbol \"%s\" version %s not found in %p)\n", rsymbol, vername?vername:"(nil)", handle);
        printf_dlsym_prefix(0, LOG_NEVER, "%p\n", NULL);
        return NULL;
    }
    if(handle==(void*)~0LL) {
        // special case, look globably but no self (RTLD_NEXT)
        uintptr_t ret_addr = 0;
        #ifdef BOX32
        if(box64_is32bits)
            ret_addr = from_ptri(ptr_t, R_ESP);
        else
        #endif
            ret_addr = *(uintptr_t*)R_RSP;
        elfheader_t *elf = FindElfAddress(my_context, ret_addr); // use return address to guess "self"
        if(GetNoSelfSymbolStartEnd(my_context->maplib, rsymbol, &start, &end, elf, 0, version, vername, 0, NULL)) {
                printf_dlsym_prefix(0, LOG_NEVER, "%p\n", (void*)start);
            return (void*)start;
        }
        SET_ERROR("Symbol \"%s\" version %s not found in %p)\n", rsymbol, vername?vername:"(nil)", handle);
            printf_dlsym_prefix(0, LOG_NEVER, "%p\n", NULL);
        return NULL;
    }
    size_t nlib = (size_t)handle;
    --nlib;
    // size_t is unsigned
    if(nlib>=dl->lib_sz) {
        SET_ERROR("Bad handle %p)\n", handle);
        printf_dlsym_prefix(0, LOG_NEVER, "%p\n", NULL);
        return NULL;
    }
    if(!dl->dllibs[nlib].count || !dl->dllibs[nlib].full) {
        SET_ERROR("Bad handle %p (already closed))\n", handle);
        printf_dlsym_prefix(0, LOG_NEVER, "%p\n", (void*)NULL);
        return NULL;
    }
    if(dl->dllibs[nlib].lib) {
        if(my_dlsym_lib(dl->dllibs[nlib].lib, rsymbol, &start, &end, version, vername)==0) {
            // not found
                printf_dlsym_prefix(0, LOG_NEVER, "%p\nCall to dlvsym(%s, \"%s\", %s) Symbol not found\n", NULL, GetNameLib(dl->dllibs[nlib].lib), rsymbol, vername?vername:"(nil)");
            printf_log(LOG_DEBUG, " Symbol not found\n");
            SET_ERROR("Symbol \"%s\" not found in %p(%s)", rsymbol, handle, GetNameLib(dl->dllibs[nlib].lib));
            return NULL;
        }
    } else {
        // still usefull?
        if(GetGlobalSymbolStartEnd(my_context->maplib, rsymbol, &start, &end, NULL, -1, NULL, 0, NULL)) {
            printf_dlsym_prefix(0, LOG_NEVER, "%p\n", (void*)start);
            return (void*)start;
        }
        // not found
            printf_dlsym_prefix(0, LOG_NEVER, "%p\nCall to dlvsym(%s, \"%s\", %s) Symbol not found\n", NULL, "Self", rsymbol, vername?vername:"(nil)");
        printf_log(LOG_DEBUG, " Symbol not found\n");
        SET_ERROR("Symbol \"%s\" version %s not found in %p)\n", rsymbol, vername?vername:"(nil)", handle);
        return NULL;
    }
        printf_dlsym_prefix(0, LOG_NEVER, "%p\n", (void*)start);
    return (void*)start;
}

int my_dlinfo(x64emu_t* emu, void* handle, int request, void* info)
{
    (void)emu;
    printf_dlsym(LOG_DEBUG, "Call to dlinfo(%p, %d, %p)\n", handle, request, info);
    dlprivate_t *dl = my_context->dlprivate;
    CLEARERR;
    size_t nlib = (size_t)handle;
    --nlib;
    // size_t is unsigned
    if(nlib>=dl->lib_sz) {
        SET_ERROR("Bad handle %p)\n", handle);
        printf_dlsym(LOG_DEBUG, "dlinfo: %s\n", dl->last_msg);
        return -1;
    }
    if(!dl->dllibs[nlib].count || !dl->dllibs[nlib].full) {
        SET_ERROR("Bad handle %p (already closed))\n", handle);
        printf_dlsym(LOG_DEBUG, "dlinfo: %s\n", dl->last_msg);
        return -1;
    }
    library_t *lib = dl->dllibs[nlib].lib;
    //elfheader_t *h = (GetElfIndex(lib)>-1)?my_context->elfs[GetElfIndex(lib)]:NULL;
    switch(request) {
        case 2: // RTLD_DI_LINKMAP
            {
                *(linkmap_t**)info = getLinkMapLib(lib);
            }
            return 0;
        default:
            printf_log(LOG_NONE, "Warning, unsupported call to dlinfo(%p, %d, %p)\n", handle, request, info);
            SET_ERROR("unsupported call to dlinfo request:%d\n", request);
    }
    return -1;
}

typedef struct my_dl_find_object_s {
    uint64_t    dlfo_flags;
    void*       dlfo_map_start;
    void*       dlfo_map_end;
    struct link_map *dlf_link_map;
    void*       dlfo_eh_frame;
    // other stuff...
} my_dl_find_object_t;

EXPORT int my__dl_find_object(x64emu_t* emu, void* addr, my_dl_find_object_t* result)
{
    //printf_log(LOG_INFO, "Unimplemented _dl_find_object called\n");
    uintptr_t start=0, sz=0;
    elfheader_t* h = FindElfAddress(my_context, (uintptr_t)addr);
    if(h) {
        // find an actual elf
        /*const char* name =*/ FindNearestSymbolName(h, addr, &start, &sz);
        result->dlfo_map_start = (void*)start;
        result->dlfo_map_end = (void*)(start+sz-1);
        result->dlfo_eh_frame = (void*)(h->ehframehdr+h->delta);
        result->dlfo_flags = 0;   // unused it seems
        result->dlf_link_map = (struct link_map *)getLinkMapElf(h);
        return 0;
    }
    return -1;
}

void closeAllDLOpened()
{
    dlprivate_t *dl = my_context->dlprivate;
    actualy_closing = 1;
    if(dl) {
        x64emu_t* emu = thread_get_emu();
        for(size_t i=0; i<dl->lib_sz; ++i)
            while(dl->dllibs[i].count) {
                printf_log(LOG_DEBUG, "  closing %s\n", dl->dllibs[i].lib->name);
                my_dlclose(emu, (void*)(i+1));
            }
    }
}

#ifdef STATICBUILD
//extern void* _dlfcn_hook;
#endif

#define PRE_INIT\
    if(1)                                                           \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    else

#define CUSTOM_FINI \
    closeAllDLOpened();

// define all standard library functions
#include "wrappedlib_init.h"
