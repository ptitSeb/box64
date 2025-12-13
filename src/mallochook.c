#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <malloc.h>

#include "box64context.h"
#include "debug.h"
#include "callback.h"
#include "librarian.h"
#include "elfs/elfloader_private.h"
#include "custommem.h"
#include "symbols.h"
#include "alternate.h"

/*
    This file here is for handling overriding of malloc functions

 Libraries like tcmalloc overrides all malloc/free/new/delete function and implement a custom version.
 Problem is, box64 is already loaded in memoery, and already using system libc malloc for many of it's things
 before those lib are even loaded in memory.
 Also, those override function can be define in the main executable, or in a lib loaded directly by the exectable
 or even in a lib loaded later using dlsym.

 The 2 different strategies to handle this can be defined as "Embrace" and "Exterminate" (as it cannot simply be ignored, 
 or you end up with mixing free/realloc from one lib and malloc/free from libc)

 In the "Embrace" strategy, the overridden malloc function are taken into account as soon as possible, and are used for all loaded lib, including native
 In the "Exterminate" strategy, the overridden malloc function are erased, and replaced with libc malloc as soon as they are defined.

 The advantage of "Embrace" is that the system will run the function it supposed to be using, and potential side-effect and staticaly linked functions 
 will run as intended.
 The downside of "Embrace" is that is makes it impossible to load a library with dlsym that override malloc function, especialy 
 if it loads natively wrapped function

 The advantage of "Exterminate" is that you wont be emulating basic malloc and friend, wich are used extensively in every program. Also, loading lib 
 with dlopen will not a be a problem.
 The downside of "Exterminate" is that side effect are less well controled. Staticaly linked stuff and anonymous symbols might put this strategy in trouble.

 This is the Exterminate strategy implementation
*/
#ifndef ANDROID
#include "bridge.h"
#include "wrapper.h"

#define SUPER()                 \
GO(malloc, pFL)                 \
GO(free, vFp)                   \
GO(calloc, pFLL)                \
GO(realloc, pFpL)               \
GO(aligned_alloc, pFLL)         \
GO(memalign, pFLL)              \
GO(posix_memalign, iFpLL)       \
GO(pvalloc, pFL)                \
GO(valloc, pFL)                 \
GO(cfree, vFp)                  \
GO(malloc_usable_size, LFp)     \
GO2(_Znwm, pFL)                 \
GO2(_ZnwmRKSt9nothrow_t, pFLp)  \
GO2(_Znam, pFL)                 \
GO2(_ZnamRKSt9nothrow_t, pFLp)  \
GO2(_ZdaPv, vFp)                \
GO2(_ZdaPvm, vFpL)              \
GO2(_ZdaPvmSt11align_val_t, vFpLL)              \
GO2(_ZdlPv, vFp)                                \
GO2(_ZdlPvm, vFpL)                              \
GO2(_ZnwmSt11align_val_t, pFLL)                 \
GO2(_ZnwmSt11align_val_tRKSt9nothrow_t, pFLLp)  \
GO2(_ZnamSt11align_val_t, pFLL)                 \
GO2(_ZnamSt11align_val_tRKSt9nothrow_t, pFLLp)  \
GO2(_ZdlPvRKSt9nothrow_t, vFpp)                 \
GO2(_ZdaPvSt11align_val_tRKSt9nothrow_t, vFpLp) \
GO2(_ZdlPvmSt11align_val_t, vFpLL)              \
GO2(_ZdaPvRKSt9nothrow_t, vFpp)                 \
GO2(_ZdaPvSt11align_val_t, vFpL)                \
GO2(_ZdlPvSt11align_val_t, vFpL)                \
GO2(_ZdlPvSt11align_val_tRKSt9nothrow_t, vFpLp) \
GO2(tc_calloc, pFLL)            \
GO2(tc_cfree, vFp)              \
GO2(tc_delete, vFp)             \
GO2(tc_deletearray, vFp)        \
GO2(tc_deletearray_nothrow, vFpp)               \
GO2(tc_delete_nothrow, vFpp)    \
GO2(tc_free, vFp)               \
GO2(tc_malloc, pFL)             \
GO2(tc_malloc_size, LFp)        \
GO2(tc_new, pFL)                \
GO2(tc_new_nothrow, pFLp)       \
GO2(tc_newarray, pFL)           \
GO2(tc_newarray_nothrow, pFLp)  \
GO2(tc_pvalloc, pFL)            \
GO2(tc_valloc, pFL)             \
GO2(tc_memalign, pFLL)          \
GO2(tc_malloc_skip_new_handler_weak, pFL)       \
GO2(tc_mallocopt, iFii)         \
GO2(tc_malloc_stats, vFv)       \
GO2(tc_malloc_skip_new_handler, pFL)            \
GO2(tc_mallinfo, pFp)           \
GO2(tc_posix_memalign, iFpLL)   \
GO2(tc_realloc, pFpL)           \
GO2(safer_scalable_aligned_realloc, pFpLLp)     \
GO2(safer_scalable_free, vFpp)  \
GO2(safer_scalable_msize, LFpp) \
GO2(safer_scalable_realloc, pFpLp)              \
GO2(scalable_aligned_free, vFp)                 \
GO2(scalable_aligned_malloc, pFLL)              \
GO2(scalable_msize, LFp)        \


//GO2(tc_set_new_mode, iFi) 
//GO2(tc_version, iFi) 

typedef void  (vFv_t)   (void);
typedef int   (iFv_t)   (void);
typedef int   (iFi_t)   (int);
typedef void* (*pFL_t)  (size_t);
typedef void* (*pFLp_t) (size_t, void* p);
typedef void  (*vFp_t)  (void*);
typedef void* (*pFp_t)  (void*);
typedef size_t(*LFp_t)  (void*);
typedef int   (*iFii_t) (int, int);
typedef void  (*vFpp_t) (void*, void*);
typedef size_t(*LFpp_t) (void*, void*);
typedef void  (*vFpL_t) (void*, size_t);
typedef void* (*pFLL_t) (size_t, size_t);
typedef void* (*pFLLp_t)(size_t, size_t, void* p);
typedef void  (*vFpLp_t)(void*, size_t, void*);
typedef void* (*pFpLp_t)(void*, size_t, void*);
typedef void  (*vFpLL_t)(void*, size_t, size_t);
typedef void* (*pFpLLp_t)(void*, size_t, size_t, void*);

size_t(*box_malloc_usable_size)(void*) = NULL;

int GetTID();
uint32_t getProtection(uintptr_t addr);
// malloc_hack "2" handling
// mmap history
static int malloc_hack_2 = 0;

#define ALLOC 0
#define FREE 1

char* box_strdup(const char* s) {
    char* ret = box_calloc(1, strlen(s)+1);
    memcpy(ret, s, strlen(s));
    return ret;
}

char* box_realpath(const char* path, char* ret)
{
    if(ret)
        return realpath(path, ret);
#ifdef PATH_MAX
    size_t path_max = PATH_MAX;
#else
    size_t path_max = pathconf(path, _PC_PATH_MAX);
    if (path_max <= 0)
    path_max = 4096;
#endif
    char tmp[path_max];
    char* p = realpath(path, tmp);
    if(!p)
        return NULL;
    return box_strdup(tmp);
}

static size_t pot(size_t l) {
    size_t ret = 0;
    while (l>(1u<<ret))  ++ret;
    return 1u<<ret;
}

static int ispot(size_t l) {
    return pot(l)==l;
}

#define GO(A, B) static uintptr_t real_##A = 0;
#define GO2(A, B) static uintptr_t real_##A = 0;
SUPER()
#undef GO2
#undef GO

#ifdef BOX32
int isCustomAddr(void* p);
// Check if entire allocation (ptr to ptr+size-1) fits within 32-bit address space
#define FITS_IN_32BIT(ptr, size) (((uintptr_t)(ptr) + (size)) <= 0x100000000ULL)
void* box32_calloc(size_t n, size_t s)
{
    void* ret = box_calloc(n, s);
    if(ret && FITS_IN_32BIT(ret, n * s)) return ret;
    box_free(ret);
    malloc_trim(0);
    ret = box_calloc(n, s);
    if(ret && FITS_IN_32BIT(ret, n * s)) return ret;
    box_free(ret);
    return customCalloc32(n, s);
}
void* box32_malloc(size_t s)
{
    void* ret = box_malloc(s);
    if(ret && FITS_IN_32BIT(ret, s)) return ret;
    box_free(ret);
    malloc_trim(0);
    ret = box_malloc(s);
    if(ret && FITS_IN_32BIT(ret, s)) return ret;
    box_free(ret);
    return customMalloc32(s);
}
void* box32_realloc(void* p, size_t s)
{
    if(isCustomAddr(p))
        return customRealloc32(p, s);
    void* ret = box_realloc(p, s);
    if(!ret) return NULL;
    if(FITS_IN_32BIT(ret, s)) return ret;
    malloc_trim(0);
    void* newret = customMalloc32(s);
    memcpy(newret, ret, s);
    box_free(ret);
    return newret;
}
void box32_free(void* p)
{
    if(isCustomAddr(p))
        customFree32(p);
    else
        box_free(p);
}
void* box32_memalign(size_t align, size_t s)
{
    void* ret = box_memalign(align, s);
    if(ret && FITS_IN_32BIT(ret, s)) return ret;
    box_free(ret);
    malloc_trim(0);
    return customMemAligned32(align, s);
}
size_t box32_malloc_usable_size(void* p)
{
    if(isCustomAddr(p))
        return customGetUsableSize(p);
    else
        return box_malloc_usable_size(p);
}

char* box32_strdup(const char* s) {
    char* ret = box32_calloc(1, strlen(s)+1);
    memcpy(ret, s, strlen(s));
    return ret;
}

#endif

// redefining all libc memory allocation routines
EXPORT void* malloc(size_t l)
{
    if(malloc_hack_2 && ALLOC && real_malloc) {
        return (void*)RunFunctionFmt(real_malloc, "L", l);
    }
    return actual_calloc(1, l);
}

EXPORT void free(void* p)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real_free && FREE)
                RunFunctionFmt(real_free, "p", p);
            return;
        }
    }
    actual_free(p);
}

EXPORT void* calloc(size_t n, size_t s)
{
    if(malloc_hack_2 && ALLOC && real_calloc) {
        return (void*)RunFunctionFmt(real_calloc, "LL", n,s);
    }
    return actual_calloc(n, s);
}

EXPORT void* realloc(void* p, size_t s)
{
    if(malloc_hack_2)
        if(getMmapped((uintptr_t)p) || (!p && ALLOC && real_realloc)) {
            void* ret = p;
            if(real_realloc && ALLOC) {
                ret = (void*)RunFunctionFmt(real_realloc, "pL", p, s);
            } else {
                // found! Will realloc using regular malloc then copy from old address as much as possible, but need to check size first
                ret = actual_malloc(s);
                printf_log(LOG_DEBUG, "Malloc_Hack_2: hacking realloc(%p, %zu)", p, s);
                while(s && !(getProtection((uintptr_t)p+s)&PROT_READ)) {if(s>box64_pagesize) s-=box64_pagesize; else s=0;}
                memcpy(ret, p, s);
                printf_log(LOG_DEBUG, " -> %p (copied %zu from old)\n", ret, s);
                // Mmaped, free with original function
                if(real_free && FREE)
                    RunFunctionFmt(real_free, "p", p);
            }
            return ret;
        }
    return actual_realloc(p, s);
}

EXPORT void* memalign(size_t align, size_t size)
{
    if(malloc_hack_2 && ALLOC && real_aligned_alloc) {
        return (void*)RunFunctionFmt(real_aligned_alloc, "LL", align, size);
    }
    if(box64_is32bits && align==4)
        align = sizeof(void*);
    return actual_memalign(align, size);
}

EXPORT void* aligned_alloc(size_t align, size_t size)
{
    if(malloc_hack_2 && ALLOC && real_aligned_alloc) {
        return (void*)RunFunctionFmt(real_aligned_alloc, "LL", align, size);
    }
    if(box64_is32bits && align==4) {
        return memalign(align, size);
    }
    return actual_memalign(align, size);
}

EXPORT int posix_memalign(void** p, size_t align, size_t size)
{
    if(malloc_hack_2 && ALLOC && real_posix_memalign) {
        return RunFunctionFmt(real_posix_memalign, "pLL", p, align, size);
    }
    if(box64_is32bits && align==4)
        align = sizeof(void*);
    if((align%sizeof(void*)) || (pot(align)!=align))
        return EINVAL;
    void* ret = actual_memalign(align, size);
    if(!ret)
        return ENOMEM;
    *p = ret;
    return 0;
}

EXPORT void* valloc(size_t size)
{
    if(malloc_hack_2 && ALLOC && real_valloc) {
        return (void*)RunFunctionFmt(real_valloc, "L", size);
    }
    return actual_memalign(box64_pagesize, size);
}

EXPORT void* pvalloc(size_t size)
{
    if(malloc_hack_2 && ALLOC && real_pvalloc) {
        return (void*)RunFunctionFmt(real_pvalloc, "L", size);
    }
    return actual_memalign(box64_pagesize, (size+box64_pagesize-1)&~(box64_pagesize-1));
}

EXPORT void cfree(void* p)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real_free && FREE)
                RunFunctionFmt(real_free, "p", p);
            return;
        }
    }
    actual_free(p);
}

EXPORT size_t malloc_usable_size(void* p)
{
    if(malloc_hack_2 && real_malloc_usable_size) {
        if(getMmapped((uintptr_t)p))
            return RunFunctionFmt(real_malloc_usable_size, "p", p);
    }
    return actual_malloc_usable_size(p);
}

EXPORT void* my__Znwm(size_t sz)   //operator new(size_t)
{
    if(malloc_hack_2 && real__Znwm) {
        return (void*)RunFunctionFmt(real__Znwm, "L", sz);
    }
    return actual_malloc(sz);
}

EXPORT void* my__ZnwmRKSt9nothrow_t(size_t sz, void* p)   //operator new(size_t, std::nothrow_t const&)
{
    if(malloc_hack_2 && real__ZnwmRKSt9nothrow_t) {
        return (void*)RunFunctionFmt(real__ZnwmRKSt9nothrow_t, "Lp", sz, p);
    }
    return actual_malloc(sz);
}

EXPORT void* my__Znam(size_t sz)   //operator new[](size_t)
{
    if(malloc_hack_2 && real__Znam) {
        return (void*)RunFunctionFmt(real__Znam, "L", sz);
    }
    return actual_malloc(sz);
}

EXPORT void* my__ZnamRKSt9nothrow_t(size_t sz, void* p)   //operator new[](size_t, std::nothrow_t const&)
{
    if(malloc_hack_2 && real__ZnamRKSt9nothrow_t) {
        return (void*)RunFunctionFmt(real__ZnamRKSt9nothrow_t, "Lp", sz, p);
    }
    return actual_malloc(sz);
}


EXPORT void my__ZdaPv(void* p)   //operator delete[](void*)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real__ZdaPv)
                RunFunctionFmt(real__ZdaPv, "p", p);
            return;
        }
    }
    actual_free(p);
}

EXPORT void my__ZdaPvm(void* p, size_t sz)   //operator delete[](void*, size_t)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real__ZdaPvm)
                RunFunctionFmt(real__ZdaPvm, "pL", p, sz);
            return;
        }
    }
    actual_free(p);
}

EXPORT void my__ZdaPvmSt11align_val_t(void* p, size_t sz, size_t align)   //operator delete[](void*, unsigned long, std::align_val_t)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real__ZdaPvmSt11align_val_t)
                RunFunctionFmt(real__ZdaPvmSt11align_val_t, "pLL", p, sz, align);
            return;
        }
    }
    actual_free(p);
}

EXPORT void my__ZdlPv(void* p)   //operator delete(void*)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real__ZdlPv)
                RunFunctionFmt(real__ZdlPv, "p", p);
            return;
        }
    }
    actual_free(p);
}

EXPORT void my__ZdlPvm(void* p, size_t sz)   //operator delete(void*, size_t)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real__ZdlPvm)
                RunFunctionFmt(real__ZdlPvm, "pL", p, sz);
            return;
        }
    }
    actual_free(p);
}

EXPORT void* my__ZnwmSt11align_val_t(size_t sz, size_t align)  //// operator new(unsigned long, std::align_val_t)
{
    if(malloc_hack_2 && real__ZnwmSt11align_val_t) {
        return (void*)RunFunctionFmt(real__ZnwmSt11align_val_t, "LL", sz, align);
    }
    return actual_memalign(align, sz);
}

EXPORT void* my__ZnwmSt11align_val_tRKSt9nothrow_t(size_t sz, size_t align, void* p)  //// operator new(unsigned long, std::align_val_t, std::nothrow_t const&)
{
    if(malloc_hack_2 && real__ZnwmSt11align_val_tRKSt9nothrow_t) {
        return (void*)RunFunctionFmt(real__ZnwmSt11align_val_tRKSt9nothrow_t, "LLp", sz, align, p);
    }
    return actual_memalign(align, sz);
}

EXPORT void* my__ZnamSt11align_val_t(size_t sz, size_t align)  //// operator new[](unsigned long, std::align_val_t)
{
    if(malloc_hack_2 && real__ZnamSt11align_val_t) {
        return (void*)RunFunctionFmt(real__ZnamSt11align_val_t, "LL", sz, align);
    }
    return actual_memalign(align, sz);
}

EXPORT void* my__ZnamSt11align_val_tRKSt9nothrow_t(size_t sz, size_t align, void* p)  //// operator new[](unsigned long, std::align_val_t, std::nothrow_t const&)
{
    if(malloc_hack_2 && real__ZnamSt11align_val_tRKSt9nothrow_t) {
        return (void*)RunFunctionFmt(real__ZnamSt11align_val_tRKSt9nothrow_t, "LLp", sz, align, p);
    }
    return actual_memalign(align, sz);
}

EXPORT void my__ZdlPvRKSt9nothrow_t(void* p, void* n)   //operator delete(void*, std::nothrow_t const&)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real__ZdlPvRKSt9nothrow_t)
                RunFunctionFmt(real__ZdlPvRKSt9nothrow_t, "pp", p, n);
            return;
        }
    }
    actual_free(p);
}

EXPORT void my__ZdaPvSt11align_val_tRKSt9nothrow_t(void* p, size_t align, void* n)   //operator delete[](void*, std::align_val_t, std::nothrow_t const&)
{
    if(malloc_hack_2 && p) {
        printf_log(LOG_INFO, "%04d|Malloc_Hack_2: my__ZdaPvSt11align_val_tRKSt9nothrow_t(%p, %d, %p)\n", GetTID(), p, align, n);
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real__ZdaPvSt11align_val_tRKSt9nothrow_t)
                RunFunctionFmt(real__ZdaPvSt11align_val_tRKSt9nothrow_t, "p", p);
            return;
        }
    }
    actual_free(p);
}

EXPORT void my__ZdlPvmSt11align_val_t(void* p, size_t sz, size_t align)   //operator delete(void*, unsigned long, std::align_val_t)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real__ZdlPvmSt11align_val_t)
                RunFunctionFmt(real__ZdlPvmSt11align_val_t, "p", p);
            return;
        }
    }
    actual_free(p);
}

EXPORT void my__ZdaPvRKSt9nothrow_t(void* p, void* n)   //operator delete[](void*, std::nothrow_t const&)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real__ZdaPvRKSt9nothrow_t)
                RunFunctionFmt(real__ZdaPvRKSt9nothrow_t, "p", p);
            return;
        }
    }
    actual_free(p);
}

EXPORT void my__ZdaPvSt11align_val_t(void* p, size_t align)   //operator delete[](void*, std::align_val_t)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real__ZdaPvSt11align_val_t)
                RunFunctionFmt(real__ZdaPvSt11align_val_t, "p", p);
            return;
        }
    }
    actual_free(p);
}

EXPORT void my__ZdlPvSt11align_val_t(void* p, size_t align)   //operator delete(void*, std::align_val_t)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real__ZdlPvSt11align_val_t)
                RunFunctionFmt(real__ZdlPvSt11align_val_t, "p", p);
            return;
        }
    }
    actual_free(p);
}

EXPORT void my__ZdlPvSt11align_val_tRKSt9nothrow_t(void* p, size_t align, void* n)   //operator delete(void*, std::align_val_t, std::nothrow_t const&)
{
    if(malloc_hack_2 && p) {
        if(getMmapped((uintptr_t)p)) {
            printf_log(LOG_DEBUG, "%04d|Malloc_Hack_2: not freeing %p\n", GetTID(), p);
            // Mmaped, free with original function
            if(real__ZdlPvSt11align_val_tRKSt9nothrow_t)
                RunFunctionFmt(real__ZdlPvSt11align_val_tRKSt9nothrow_t, "pLp", p, align, n);
            return;
        }
    }
    actual_free(p);
}

EXPORT void* my_tc_calloc(size_t n, size_t s)
{
    return actual_calloc(n, s);
}

EXPORT void my_tc_cfree(void* p)
{
    actual_free(p);
}

EXPORT void my_tc_delete(void* p)
{
    actual_free(p);
}

EXPORT void my_tc_deletearray(void* p)
{
    actual_free(p);
}

EXPORT void my_tc_deletearray_nothrow(void* p, void* n)
{
    actual_free(p);
}

EXPORT void my_tc_delete_nothrow(void* p, void* n)
{
    actual_free(p);
}

EXPORT void my_tc_free(void* p)
{
    actual_free(p);
}

EXPORT void* my_tc_malloc(size_t s)
{
    return actual_calloc(1, s);
}

EXPORT size_t my_tc_malloc_size(void* p)
{
    return actual_malloc_usable_size(p);
}

EXPORT void* my_tc_new(size_t s)
{
    return actual_calloc(1, s);
}

EXPORT void* my_tc_new_nothrow(size_t s, void* n)
{
    return actual_calloc(1, s);
}

EXPORT void* my_tc_newarray(size_t s)
{
    return actual_calloc(1, s);
}

EXPORT void* my_tc_newarray_nothrow(size_t s, void* n)
{
    return actual_calloc(1, s);
}

EXPORT void* my_tc_pvalloc(size_t size)
{
    return actual_memalign(box64_pagesize, (size+box64_pagesize-1)&~(box64_pagesize-1));
}

EXPORT void* my_tc_valloc(size_t size)
{
    return actual_memalign(box64_pagesize, size);
}

EXPORT void* my_tc_memalign(size_t align, size_t size)
{
    return actual_memalign(align, size);
}

EXPORT void* my_tc_malloc_skip_new_handler_weak(size_t s)
{
    return actual_calloc(1, s);
}

EXPORT int my_tc_mallocopt(int param, int value)
{
    // ignoring...
    return 1;
}

EXPORT void my_tc_malloc_stats()
{
    // ignoring
}
/*
EXPORT int my_tc_set_new_mode(int mode)
{
    // ignoring
    static int old = 0;
    int ret = old;
    old = mode;
    return ret;
}
*/
EXPORT void* my_tc_malloc_skip_new_handler(size_t s)
{
    return actual_calloc(1, s);
}

EXPORT void* my_tc_mallinfo(void* p)
{
    // ignored, returning null stuffs
    memset(p, 0, (box64_is32bits?sizeof(ptr_t):sizeof(size_t))*10);
    return p;
}

EXPORT int my_tc_posix_memalign(void** p, size_t align, size_t size)
{
    if(align%sizeof(void*) || pot(align)!=align)
        return EINVAL;
    void* ret = actual_memalign(align, size);
    if(!ret)
        return ENOMEM;
    *p = ret;
    return 0;
}

EXPORT void* my_tc_realloc(void* p, size_t s)
{
    return actual_realloc(p, s);
}
/*
EXPORT int my_tc_version(int i)
{
    return 2;
}
*/

EXPORT void* my_safer_scalable_aligned_realloc(void* p, size_t size, size_t align, void *old)
{
    if(!ispot(align)) {
        errno = EINVAL;
        return NULL;
    }
    if(align <= 8)
        return actual_realloc(p, size);
    size_t old_size = actual_malloc_usable_size(p);
    if(old_size>=size)
        return p;
    void* new_p = actual_memalign(align, size);
    memcpy(new_p, p, (old_size<size)?old_size:size);
    actual_free(p);
    return p;
}

EXPORT void my_safer_scalable_free(void*p , void* old)
{
    actual_free(p);
}

EXPORT size_t my_safer_scalable_msize(void* p, void* old)
{
    return actual_malloc_usable_size(p);
}

EXPORT void* my_safer_scalable_realloc(void* p, size_t size, void* old)
{
    return actual_realloc(p, size);
}

EXPORT void my_scalable_aligned_free(void* p)
{
    actual_free(p);
}

EXPORT void* my_scalable_aligned_malloc(size_t size, size_t align)
{
    if(!ispot(align)) {
        errno = EINVAL;
        return NULL;
    }
    if(align <= 8)
        return actual_malloc(size);
    return actual_memalign(align, size);
}

EXPORT void* my_scalable_aligned_realloc(void* p, size_t size, size_t align)
{
    if(!ispot(align)) {
        errno = EINVAL;
        return NULL;
    }
    if(align <= 8)
        return actual_realloc(p, size);
    size_t old_size = actual_malloc_usable_size(p);
    if(old_size>=size)
        return p;
    void* new_p = actual_memalign(align, size);
    memcpy(new_p, p, (old_size<size)?old_size:size);
    actual_free(p);
    return p;
}

EXPORT size_t my_scalable_msize(void* p)
{
    return actual_malloc_usable_size(p);
}


#pragma pack(push, 1)
typedef struct reloc_jmp_s {
    uint8_t _ff;
    uint8_t _25;
    uint32_t _00;
    void* addr;
} reloc_jmp_t;
typedef struct simple_jmp_s {
    uint8_t _e9;
    uint32_t delta;
} simple_jmp_t;
#pragma pack(pop)

static void addRelocJmp(void* offs, void* where, size_t size, const char* name, elfheader_t* h, uintptr_t *real)
{
    if(real && !*real) {
        *real = (uintptr_t)offs;
    }
    addAlternate(offs, where);
}

void checkHookedSymbols(elfheader_t* h)
{
    int hooked = 0;
    int hooked_symtab = 0;
    if(BOX64ENV(malloc_hack)==1)
        return;
    if(box64_is32bits) {
        /* TODO? */
        return;
    }
    for (size_t i=0; i<h->numSymTab; ++i) {
        int type = ELF64_ST_TYPE(h->SymTab._64[i].st_info);
        int sz = ELF64_ST_TYPE(h->SymTab._64[i].st_size);
        if((type==STT_FUNC) && sz && (h->SymTab._64[i].st_shndx!=0 && h->SymTab._64[i].st_shndx<=65521)) {
            const char * symname = h->StrTab+h->SymTab._64[i].st_name;
            #define GO(A, B) if(!strcmp(symname, #A)) ++hooked; else if(!strcmp(symname, "__libc_" #A)) ++hooked;
            #define GO2(A, B)
            SUPER()
            #undef GO
            #undef GO2
        }
    }
    if(hooked<2) {
        for (size_t i=0; i<h->numDynSym && hooked<2; ++i) {
            const char * symname = h->DynStr+h->DynSym._64[i].st_name;
            int bind = ELF64_ST_BIND(h->DynSym._64[i].st_info);
            int type = ELF64_ST_TYPE(h->DynSym._64[i].st_info);
            int vis = h->DynSym._64[i].st_other&0x3;
            if((type==STT_FUNC) 
            && (vis==STV_DEFAULT || vis==STV_PROTECTED) && (h->DynSym._64[i].st_shndx!=0 && h->DynSym._64[i].st_shndx<=65521)) {
                uintptr_t offs = h->DynSym._64[i].st_value + h->delta;
                size_t sz = h->DynSym._64[i].st_size;
                if(bind!=STB_LOCAL && bind!=STB_WEAK && sz>=sizeof(reloc_jmp_t)) {
                    #define GO(A, B) if(!strcmp(symname, #A)) ++hooked; else if(!strcmp(symname, "__libc_" #A)) ++hooked;
                    #define GO2(A, B)
                    SUPER()
                    #undef GO
                    #undef GO2
                }
            }
        }
    } else
        hooked_symtab = 1;
    if(hooked<2)
        return; // only redirect on lib that hooked / redefined the operators
    printf_log(LOG_INFO, "Redirecting overridden malloc%s from %s function for %s\n", malloc_hack_2?" with hack":"", hooked_symtab?"symtab":"dynsym", ElfName(h));
    if(hooked_symtab) {
        for (size_t i=0; i<h->numSymTab; ++i) {
            int type = ELF64_ST_TYPE(h->SymTab._64[i].st_info);
            if(type==STT_FUNC) {
                const char * symname = h->StrTab+h->SymTab._64[i].st_name;
                uintptr_t offs = h->SymTab._64[i].st_value + h->delta;
                size_t sz = h->SymTab._64[i].st_size;
                #define GO(A, B) if(!strcmp(symname, "__libc_" #A)) {uintptr_t alt = AddCheckBridge(my_context->system, B, A, 0, #A); printf_log(LOG_DEBUG, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); addRelocJmp((void*)offs, (void*)alt, sz, "__libc_" #A, h, NULL);}
                #define GO2(A, B)
                SUPER()
                #undef GO
                #undef GO2
                #define GO(A, B) if(!strcmp(symname, #A)) {uintptr_t alt = AddCheckBridge(my_context->system, B, A, 0, #A); printf_log(LOG_DEBUG, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); addRelocJmp((void*)offs, (void*)alt, sz, #A, h, &real_##A);}
                #define GO2(A, B) if(!strcmp(symname, #A) && (BOX64ENV(malloc_hack)>1)) {uintptr_t alt = AddCheckBridge(my_context->system, B, my_##A, 0, "my_" #A); printf_log(LOG_DEBUG, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); addRelocJmp((void*)offs, (void*)alt, sz, "my_" #A, h, &real_##A);}
                SUPER()
                #undef GO
                #undef GO2
            }
        }
    } else {
        for (size_t i=0; i<h->numDynSym; ++i) {
            const char * symname = h->DynStr+h->DynSym._64[i].st_name;
            int bind = ELF64_ST_BIND(h->DynSym._64[i].st_info);
            int type = ELF64_ST_TYPE(h->DynSym._64[i].st_info);
            int vis = h->DynSym._64[i].st_other&0x3;
            if((type==STT_FUNC) 
            && (vis==STV_DEFAULT || vis==STV_PROTECTED) && (h->DynSym._64[i].st_shndx!=0 && h->DynSym._64[i].st_shndx<=65521)) {
                uintptr_t offs = h->DynSym._64[i].st_value + h->delta;
                size_t sz = h->DynSym._64[i].st_size;
                if(bind!=STB_LOCAL && bind!=STB_WEAK) {
                    #define GO(A, B) if(!strcmp(symname, "__libc_" #A)) {uintptr_t alt = AddCheckBridge(my_context->system, B, A, 0, #A); printf_log(LOG_DEBUG, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); addRelocJmp((void*)offs, (void*)alt, sz, "__libc_" #A, h, NULL);}
                    #define GO2(A, B)
                    SUPER()
                    #undef GO
                    #undef GO2
                    #define GO(A, B) if(!strcmp(symname, #A)) {uintptr_t alt = AddCheckBridge(my_context->system, B, A, 0, #A); printf_log(LOG_DEBUG, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); addRelocJmp((void*)offs, (void*)alt, sz, #A, h, &real_##A);}
                    #define GO2(A, B) if(!strcmp(symname, #A) && (BOX64ENV(malloc_hack)>1)) {uintptr_t alt = AddCheckBridge(my_context->system, B, my_##A, 0, "my_" #A); printf_log(LOG_DEBUG, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); addRelocJmp((void*)offs, (void*)alt, sz, "my_" #A, h, &real_##A);}
                    SUPER()
                    #undef GO
                    #undef GO2
                }
            }
        }
    }
    if(BOX64ENV(malloc_hack)==2)
        h->malloc_hook_2 = 1;
}

void startMallocHook()
{
    malloc_hack_2 = 1;
}
void endMallocHook()
{
    malloc_hack_2 = 0;
}

EXPORT int my___TBB_internal_find_original_malloc(int n, char* names[], void* ptr[])
{
    int ret = 1;
    #define GO(A, B) else if(!strcmp(names[i], #A)) {ptr[i] = A;}
    #define GO2(A, B) 
    for (int i=0; i<n; ++i)
        if (0) {}
        SUPER()
        else ret = 0;
    return ret;
    #undef GO
    #undef GO2
}

EXPORT void my___TBB_call_with_my_server_info(void* cb, void* server)
{
    // nothing
}

EXPORT int my___TBB_make_rml_server(void* factory, void* server, void* client)
{
    // nothing
    return 0;
}

EXPORT void my___RML_close_factory(void* server)
{
    // nothing
}

EXPORT int my___RML_open_factory(void* factory, void* server_version, int client_version)
{
    // nothing
    return 0;
}

void init_malloc_hook() {
    box_malloc_usable_size = dlsym(RTLD_NEXT, "malloc_usable_size");
    #if 0
    #define GO(A, B)
    #define GO2(A, B)   box_##A = (B##_t)dlsym(RTLD_NEXT, #A); if(box_##A == (B##_t)A) box_##A = NULL;
    SUPER()
    #undef GO2
    #undef GO
    #endif
}

#undef SUPER
#else//ANDROID
void init_malloc_hook() {}
void startMallocHook() {}
void endMallocHook() {}
void checkHookedSymbols(elfheader_t* h) {}
#endif //!ANDROID
