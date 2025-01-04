#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <elf.h>
#include <link.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "emu/x64emu_private.h"
#include "library.h"
#include "librarian.h"
#include "box32context.h"
#include "elfloader.h"
#include "elfs/elfloader_private.h"
#include "x64emu.h"


#define LIBNAME libdl
#ifdef ANDROID
    static const char* libdlName = "libdl.so";
#else
    static const char* libdlName = "libdl.so.2";
#endif

void* my_dlopen(x64emu_t* emu, void *filename, int flag) EXPORT;
void* my_dlmopen(x64emu_t* emu, void* mlid, void *filename, int flag) EXPORT;
char* my_dlerror(x64emu_t* emu) EXPORT;
void* my_dlsym(x64emu_t* emu, void *handle, void *symbol) EXPORT;
int my_dlclose(x64emu_t* emu, void *handle) EXPORT;
int my32_dladdr(x64emu_t* emu, void *addr, void *info) EXPORT;
int my32_dladdr1(x64emu_t* emu, void *addr, void *info, void** extra_info, int flags) EXPORT;
void* my_dlvsym(x64emu_t* emu, void *handle, void *symbol, const char *vername) EXPORT;
int my32_dlinfo(x64emu_t* emu, void* handle, int request, void* info) EXPORT;

#ifndef RTLD_DL_SYMENT
#define RTLD_DL_SYMENT 1
#endif
#ifndef RTLD_DL_LINKMAP
#define RTLD_DL_LINKMAP 2
#endif

void dl_clear_error();
#define CLEARERR    dl_clear_error()
void dl_set_error(const char*);
char* dl_last_error();
library_t* dl_get_library(void* handle);

typedef struct my_dl_info_32_s
{
  ptr_t dli_fname;	// const char*
  ptr_t dli_fbase;	// void*
  ptr_t dli_sname;	// const char*
  ptr_t dli_saddr;	// void*
} my_dl_info_32_t;

int my32_dladdr1(x64emu_t* emu, void *addr, void *i, void** extra_info, int flags)
{
    //int dladdr(void *addr, my_dl_info_32_t *info);
    CLEARERR;
    my_dl_info_32_t *info = (my_dl_info_32_t*)i;
    printf_log(LOG_DEBUG, "Warning: partially unimplement call to dladdr/dladdr1(%p, %p, %p, %d)\n", addr, info, extra_info, flags);
    
    //emu->quit = 1;
    library_t* lib = NULL;
    info->dli_saddr = 0;
    info->dli_fname = 0;
    void* start = NULL;
    const char* fname = NULL;
    void* base = NULL;
    const char* sname = FindSymbolName(my_context->maplib, addr, &start, NULL, &fname, &base, &lib);
    info->dli_saddr = to_ptrv(start);
    info->dli_fname = to_ptrv((void*)fname);
    info->dli_fbase = to_ptrv(base);
    info->dli_sname = to_ptrv((void*)sname);
    printf_log(LOG_DEBUG, "     dladdr return saddr=%p, fname=\"%s\", sname=\"%s\"\n", start, sname?sname:"", fname?fname:"");
    if(flags==RTLD_DL_SYMENT) {
        printf_log(LOG_INFO, "Warning, unimplement call to dladdr1 with RTLD_DL_SYMENT flags\n");
    } else if (flags==RTLD_DL_LINKMAP) {
        printf_log(LOG_INFO, "Warning, unimplemented call to dladdr1 with RTLD_DL_LINKMAP flags\n");
        *(ptr_t*)extra_info = to_ptrv(getLinkMapLib32(lib));
    }
    return (info->dli_sname)?1:0;   // success is non-null here...
}
int my32_dladdr(x64emu_t* emu, void *addr, void *i)
{
    return my32_dladdr1(emu, addr, i, NULL, 0);
}

int my32_dlinfo(x64emu_t* emu, void* handle, int request, void* info)
{
    (void)emu;
    printf_dlsym(LOG_DEBUG, "Call to dlinfo(%p, %d, %p)\n", handle, request, info);
    CLEARERR;
    library_t *lib = dl_get_library(handle);
    if(lib==(library_t*)-1LL) {
        printf_dlsym(LOG_DEBUG, "dlinfo: %s\n", dl_last_error());
        return -1;
    }
    //elfheader_t *h = (GetElfIndex(lib)>-1)?my_context->elfs[GetElfIndex(lib)]:NULL;
    switch(request) {
        case 2: // RTLD_DI_LINKMAP
            {
                *(linkmap32_t**)info = getLinkMapLib32(lib);
            }
            return 0;
        default:
            printf_log(LOG_NONE, "Warning, unsupported call to dlinfo(%p, %d, %p)\n", handle, request, info);
            {
                char tmp[129] = {0};
                snprintf(tmp, 129, "unsupported call to dlinfo request:%d\n", request);
                dl_set_error(tmp);
            }
    }
    return -1;
}

typedef struct my_dl_find_object_s {
    uint64_t    dlfo_flags;
    ptr_t       dlfo_map_start;
    ptr_t       dlfo_map_end;
    ptr_t       dlf_link_map;   //struct link_map *
    ptr_t       dlfo_eh_frame;
    // other stuff...
} my_dl_find_object_t;

EXPORT int my32__dl_find_object(x64emu_t* emu, void* addr, my_dl_find_object_t* result)
{
    //printf_log(LOG_INFO, "Unimplemented _dl_find_object called\n");
    uintptr_t start=0, sz=0;
    elfheader_t* h = FindElfAddress(my_context, (uintptr_t)addr);
    if(h) {
        // find an actual elf
        const char* name = FindNearestSymbolName(h, addr, &start, &sz);
        result->dlfo_map_start = start;
        result->dlfo_map_end = start+sz-1;
        result->dlfo_eh_frame = h->ehframehdr+h->delta;
        result->dlfo_flags = 0;   // unused it seems
        result->dlf_link_map = to_ptrv(getLinkMapElf32(h));
        return 0;
    }
    return -1;
}

//#define CUSTOM_INIT\
//        if(!box32_isglibc234) setNeededLibs(lib, 1, "libc.so.6");


void closeAllDLOpened();

#define PRE_INIT\
    if(1)                                                           \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    else

#define CUSTOM_FINI \
    closeAllDLOpened();

// define all standard library functions
#include "wrappedlib_init32.h"
