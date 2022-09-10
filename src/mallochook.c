#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <dlfcn.h>

#include "box64context.h"
#include "debug.h"
#include "callback.h"
#include "librarian.h"
#include "elfs/elfloader_private.h"

/*
    This file here is for handling overriding of malloc functions

 Libraries like tcmalloc overrides all malloc/free/new/delete function and implement a custom version.
 Problem is, box64 is already loaded in memoery, and already using system libc malloc for many of it's things
 before those lib are even loaded in memory.
 Also, those override function can be define in the main executable, or in a lib loaded directly by the exectable
 or even in a lib loaded later using dlsym.

 The 2 different strategies to handle this can be defined as "Embrace" and "Exterminate" (as it cannot simply be ignored, 
 or you end up with mixing free/realloc from one lib and malloc/free from libc)

 In the "Embrace" strategy, the overriden malloc function are taken into account as soon as possible, and are used for all loaded lib, including native
 In the "Exterminate" strategy, the overriden malloc function are erased, and replaced with libc malloc as soon as they are defined.

 The advantage of "Embrace" is that the system will run the function it supposed to be using, and potential side-effect and staticaly linked functions 
 will run as intended.
 The downside of "Embrace" is that is makes it impossible to load a library with dlsym that override malloc function, especialy 
 if it loads natively wrapped function

 The advantage of "Exterminate" is that you wont be emulating basic malloc and friend, wich are used extensively in every program. Also, loading lib 
 with dlopen will not a be a problem.
 The downside of "Exterminate" is that side effect are less well controled. Staticaly linked stuff and anonymous symbols might put this strategy in trouble.

*/

//#define EMBRACE

#ifndef EMBRACE
#include "bridge.h"
#include "tools/bridge_private.h"
#include "wrapper.h"
#endif

#define SUPER()                 \
GO(malloc, pFL);                \
GO(free, vFp);                  \
GO(calloc, pFLL);               \
GO(realloc, pFpL);              \
GO(aligned_alloc, pFLL);        \
GO(memalign, pFLL);             \
GO(posix_memalign, iFpLL);      \
GO(pvalloc, pFL);               \
GO(valloc, pFL);                \
GO(cfree, vFp);                 \
GO(malloc_usable_size, LFp) ;   \
GO2(_Znwm, pFL);                \
GO2(_ZnwmRKSt9nothrow_t, pFLp); \
GO2(_Znam, pFL);                \
GO2(_ZnamRKSt9nothrow_t, pFLp); \
GO2(_ZdaPv, vFp);               \
GO2(_ZdaPvm, vFpL);             \
GO2(_ZdaPvmSt11align_val_t, vFpLL);             \
GO2(_ZdlPv, vFp);                               \
GO2(_ZdlPvm, vFpL);                             \
GO2(_ZnwmSt11align_val_t, pFLL);                \
GO2(_ZnwmSt11align_val_tRKSt9nothrow_t, pFLLp); \
GO2(_ZnamSt11align_val_t, pFLL);                \
GO2(_ZnamSt11align_val_tRKSt9nothrow_t, pFLLp); \
GO2(_ZdlPvRKSt9nothrow_t, vFpp);                \
GO2(_ZdaPvSt11align_val_tRKSt9nothrow_t, vFpLp);\
GO2(_ZdlPvmSt11align_val_t, vFpLL);             \
GO2(_ZdaPvRKSt9nothrow_t, vFpp);                \
GO2(_ZdaPvSt11align_val_t, vFpL);               \
GO2(_ZdlPvSt11align_val_t, vFpL);               \
GO2(_ZdlPvSt11align_val_tRKSt9nothrow_t, vFpLp);\

typedef void* (*pFL_t)  (size_t);
typedef void* (*pFLp_t) (size_t, void* p);
typedef void  (*vFp_t)  (void*);
typedef void  (*vFpp_t) (void*, void*);
typedef void  (*vFpL_t) (void*, size_t);
typedef void* (*pFLL_t) (size_t, size_t);
typedef void* (*pFLLp_t)(size_t, size_t, void* p);
typedef void  (*vFpLp_t)(void*, size_t, void*);
typedef void  (*vFpLL_t)(void*, size_t, size_t);

#ifdef ANDROID
void*(*__libc_malloc)(size_t) = NULL;
void*(*__libc_realloc)(size_t, void*) = NULL;
void*(*__libc_calloc)(size_t, size_t) = NULL;
void (*__libc_free*)(void*) = NULL;
void*(*__libc_memalign)(size_t, size_t) = NULL;
#endif
size_t(*box_malloc_usable_size)(void*) = NULL;
#define GO(A, B)
#define GO2(A, B)   B##_t box_##A = NULL
SUPER()
#undef GO2
#undef GO

int GetTID();

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

#ifdef EMBRACE
// emulated lib/program hooked memory functions
#define GO(A, B) uintptr_t hooked_##A = 0
#define GO2(A, B) uintptr_t hooked_##A = 0
SUPER()
#undef GO
#define GO(A, B) elfheader_t* elf_##A = NULL
#define GO2(A, B) elfheader_t* elf_##A = NULL
SUPER()
#undef GO
#endif

static size_t pot(size_t l) {
    size_t ret = 0;
    while (l>(1<<ret))  ++ret;
    return 1<<ret;
}

// redefining all libc memory allocation routines
EXPORT void* malloc(size_t l)
{
    #ifdef EMBRACE
    if(hooked_malloc && elf_malloc && elf_malloc->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated malloc(%zd)\n", GetTID(), l);
        void* ret = (void*)RunSafeFunction(my_context, hooked_malloc, 1, l);
        return ret;
    }
    #endif
    return box_calloc(1, l);
}

EXPORT void free(void* p)
{
    #ifdef EMBRACE
    if(hooked_free && elf_free && elf_free->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated free(%x)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked_free, 1, p);
    } else
    #endif
        box_free(p);
}

EXPORT void* calloc(size_t n, size_t s)
{
    #ifdef EMBRACE
    if(hooked_calloc && elf_calloc && elf_calloc->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated calloc(%zd, %zd)\n", GetTID(), n, s);
        return (void*)RunSafeFunction(my_context, hooked_calloc, 2, n, s);
    }
    #endif
    return box_calloc(n, s);
}

EXPORT void* realloc(void* p, size_t s)
{
    #ifdef EMBRACE
    if(hooked_realloc && elf_realloc && elf_realloc->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated realloc(%p, %zd)\n", GetTID(), p, s);
        return (void*)RunSafeFunction(my_context, hooked_realloc, 2, p, s);
    }
    #endif
    return box_realloc(p, s);
}

EXPORT void* aligned_alloc(size_t align, size_t size)
{
    #ifdef EMBRACE
    if(hooked_aligned_alloc && elf_aligned_alloc && elf_aligned_alloc->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated aligned_alloc(%zd, %zd)\n", GetTID(), align, size);
        return (void*)RunSafeFunction(my_context, hooked_aligned_alloc, 2, align, size);
    }
    #endif
    return box_memalign(align, size);
}

EXPORT void* memalign(size_t align, size_t size)
{
    #ifdef EMBRACE
    if(hooked_memalign && elf_memalign && elf_memalign->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated memalign(%zd, %zd)\n", GetTID(), align, size);
        return (void*)RunSafeFunction(my_context, hooked_memalign, 2, align, size);
    }
    #endif
    return box_memalign(align, size);
}

EXPORT int posix_memalign(void** p, size_t align, size_t size)
{
    #ifdef EMBRACE
    if(hooked_posix_memalign && elf_posix_memalign && elf_posix_memalign->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated posix_memalign(%p, %zd, %zd)\n", GetTID(), p, align, size);
        return (int)RunSafeFunction(my_context, hooked_posix_memalign, 3, p, align, size);
    }
    #endif
    if(align%sizeof(void*) || pot(align)!=align)
        return EINVAL;
    void* ret = box_memalign(align, size);
    if(!ret)
        return ENOMEM;
    *p = ret;
    return 0;
}

EXPORT void* valloc(size_t size)
{
    #ifdef EMBRACE
    if(hooked_valloc && elf_valloc && elf_valloc->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated valloc(%zd)\n", GetTID(), size);
        return (void*)RunSafeFunction(my_context, hooked_valloc, 1, size);
    }
    #endif
    return box_memalign(box64_pagesize, size);
}

EXPORT void* pvalloc(size_t size)
{
    #ifdef EMBRACE
    if(hooked_pvalloc && elf_pvalloc && elf_pvalloc->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated pvalloc(%zd)\n", GetTID(), size);
        return (void*)RunSafeFunction(my_context, hooked_pvalloc, 1, size);
    }
    #endif
    return box_memalign(box64_pagesize, (size+box64_pagesize-1)&~(box64_pagesize-1));
}

EXPORT void cfree(void* p)
{
    #ifdef EMBRACE
    if(hooked_cfree && elf_cfree && elf_cfree->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated cfree(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked_cfree, 1, p);
    } else
    #endif
        box_free(p);
}

EXPORT size_t malloc_usable_size(void* p)
{
    #ifdef EMBRACE
    if(hooked_malloc_usable_size && elf_malloc_usable_size && elf_malloc_usable_size->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated malloc_usable_size(%p)\n", GetTID(), p);
        return (size_t)RunSafeFunction(my_context, hooked_malloc_usable_size, 1, p);
    } else
    #endif
        return box_malloc_usable_size(p);
}

EXPORT void* _Znwm(size_t sz)   //operator new(size_t)
{
    #ifdef EMBRACE
    if(hooked__Znwm && elf__Znwm && elf__Znwm->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _Znwm(%p)\n", GetTID(), p);
        return (void*)RunSafeFunction(my_context, hooked__Znwm, 1, sz);
    } else
    #endif
        if(box__Znwm)
            return box__Znwm(sz);
        return box_malloc(sz);
}

EXPORT void* _ZnwmRKSt9nothrow_t(size_t sz, void* p)   //operator new(size_t, std::nothrow_t const&)
{
    #ifdef EMBRACE
    if(hooked__ZnwmRKSt9nothrow_t && elf__ZnwmRKSt9nothrow_t && elf__ZnwmRKSt9nothrow_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZnwmRKSt9nothrow_t(%p)\n", GetTID(), p);
        return (void*)RunSafeFunction(my_context, hooked__ZnwmRKSt9nothrow_t, 2, sz, p);
    } else
    #endif
        if(box__ZnwmRKSt9nothrow_t)
            return box__ZnwmRKSt9nothrow_t(sz, p);
        return box_malloc(sz);
}

EXPORT void* _Znam(size_t sz)   //operator new[](size_t)
{
    #ifdef EMBRACE
    if(hooked__Znam && elf__Znam && elf__Znam->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _Znam(%p)\n", GetTID(), p);
        return (void*)RunSafeFunction(my_context, hooked__Znam, 1, sz);
    } else
    #endif
        if(box__Znam)
            return box__Znam(sz);
        return box_malloc(sz);
}

EXPORT void* _ZnamRKSt9nothrow_t(size_t sz, void* p)   //operator new[](size_t, std::nothrow_t const&)
{
    #ifdef EMBRACE
    if(hooked__ZnamRKSt9nothrow_t && elf__ZnamRKSt9nothrow_t && elf__ZnamRKSt9nothrow_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZnamRKSt9nothrow_t(%p)\n", GetTID(), p);
        return (void*)RunSafeFunction(my_context, hooked__ZnamRKSt9nothrow_t, 2, sz, p);
    } else
    #endif
        if(box__ZnamRKSt9nothrow_t)
            return box__ZnamRKSt9nothrow_t(sz, p);
        return box_malloc(sz);
}


EXPORT void _ZdaPv(void* p)   //operator delete[](void*)
{
    #ifdef EMBRACE
    if(hooked__ZdaPv && elf__ZdaPv && elf__ZdaPv->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZdaPv(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked__ZdaPv, 1, p);
    } else
    #endif
        if(box__ZdaPv)
            box__ZdaPv(p);
        else
            box_free(p);
}

EXPORT void _ZdaPvm(void* p, size_t sz)   //operator delete[](void*, size_t)
{
    #ifdef EMBRACE
    if(hooked__ZdaPvm && elf__ZdaPvm && elf__ZdaPvm->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZdaPvm(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked__ZdaPvm, 2, p, sz);
    } else
    #endif
        if(box__ZdaPvm)
            box__ZdaPvm(p, sz);
        else
            box_free(p);
}

EXPORT void _ZdaPvmSt11align_val_t(void* p, size_t sz, size_t align)   //operator delete[](void*, unsigned long, std::align_val_t)
{
    #ifdef EMBRACE
    if(hooked__ZdaPvmSt11align_val_t && elf__ZdaPvmSt11align_val_t && elf__ZdaPvmSt11align_val_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZdaPvmSt11align_val_t(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked__ZdaPvmSt11align_val_t, 3, p, sz, align);
    } else
    #endif
        if(box__ZdaPvmSt11align_val_t)
            box__ZdaPvmSt11align_val_t(p, sz, align);
        else
            box_free(p);
}

EXPORT void _ZdlPv(void* p)   //operator delete(void*)
{
    #ifdef EMBRACE
    if(hooked__ZdlPv && elf__ZdlPv && elf__ZdlPv->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZdlPv(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked__ZdlPv, 1, p);
    } else
    #endif
        if(box__ZdlPv)
            box__ZdlPv(p);
        else
            box_free(p);
}

EXPORT void _ZdlPvm(void* p, size_t sz)   //operator delete(void*, size_t)
{
    #ifdef EMBRACE
    if(hooked__ZdlPvm && elf__ZdlPvm && elf__ZdlPvm->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZdlPvm(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked__ZdlPvm, 2, p, sz);
    } else
    #endif
        if(box__ZdlPvm)
            box__ZdlPvm(p, sz);
        else
            box_free(p);
}

EXPORT void* _ZnwmSt11align_val_t(size_t sz, size_t align)  //// operator new(unsigned long, std::align_val_t)
{
    #ifdef EMBRACE
    if(hooked__ZnwmSt11align_val_t && elf__ZnwmSt11align_val_t && elf__ZnwmSt11align_val_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZnwmSt11align_val_t(%p)\n", GetTID(), p);
        return (void*)RunSafeFunction(my_context, hooked__ZnwmSt11align_val_t, 2, sz, align);
    } else
    #endif
        if(box__ZnwmSt11align_val_t)
            return box__ZnwmSt11align_val_t(sz, align);
        return box_memalign(sz, align);
}

EXPORT void* _ZnwmSt11align_val_tRKSt9nothrow_t(size_t sz, size_t align, void* p)  //// operator new(unsigned long, std::align_val_t, std::nothrow_t const&)
{
    #ifdef EMBRACE
    if(hooked__ZnwmSt11align_val_tRKSt9nothrow_t && elf__ZnwmSt11align_val_tRKSt9nothrow_t && elf__ZnwmSt11align_val_tRKSt9nothrow_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZnwmSt11align_val_tRKSt9nothrow_t(%p)\n", GetTID(), p);
        return (void*)RunSafeFunction(my_context, hooked__ZnwmSt11align_val_tRKSt9nothrow_t, 3, sz, align, p);
    } else
    #endif
        if(box__ZnwmSt11align_val_tRKSt9nothrow_t)
            return box__ZnwmSt11align_val_tRKSt9nothrow_t(sz, align, p);
        return box_memalign(sz, align);
}

EXPORT void* _ZnamSt11align_val_t(size_t sz, size_t align)  //// operator new[](unsigned long, std::align_val_t)
{
    #ifdef EMBRACE
    if(hooked__ZnamSt11align_val_t && elf__ZnamSt11align_val_t && elf__ZnamSt11align_val_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZnamSt11align_val_t(%p)\n", GetTID(), p);
        return (void*)RunSafeFunction(my_context, hooked__ZnamSt11align_val_t, 2, sz, align);
    } else
    #endif
        if(box__ZnamSt11align_val_t)
            return box__ZnamSt11align_val_t(sz, align);
        return box_memalign(sz, align);
}

EXPORT void* _ZnamSt11align_val_tRKSt9nothrow_t(size_t sz, size_t align, void* p)  //// operator new[](unsigned long, std::align_val_t, std::nothrow_t const&)
{
    #ifdef EMBRACE
    if(hooked__ZnamSt11align_val_tRKSt9nothrow_t && elf__ZnamSt11align_val_tRKSt9nothrow_t && elf__ZnamSt11align_val_tRKSt9nothrow_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZnamSt11align_val_tRKSt9nothrow_t(%p)\n", GetTID(), p);
        return (void*)RunSafeFunction(my_context, hooked__ZnamSt11align_val_tRKSt9nothrow_t, 3, sz, align, p);
    } else
    #endif
        if(box__ZnamSt11align_val_tRKSt9nothrow_t)
            return box__ZnamSt11align_val_tRKSt9nothrow_t(sz, align, p);
        return box_memalign(sz, align);
}

EXPORT void _ZdlPvRKSt9nothrow_t(void* p, void* n)   //operator delete(void*, std::nothrow_t const&)
{
    #ifdef EMBRACE
    if(hooked__ZdlPvRKSt9nothrow_t && elf__ZdlPvRKSt9nothrow_t && elf__ZdlPvRKSt9nothrow_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZdlPvRKSt9nothrow_t(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked__ZdlPvRKSt9nothrow_t, 2, p, n);
    } else
    #endif
        if(box__ZdlPvRKSt9nothrow_t)
            box__ZdlPvRKSt9nothrow_t(p, n);
        else
            box_free(p);
}

EXPORT void _ZdaPvSt11align_val_tRKSt9nothrow_t(void* p, size_t align, void* n)   //operator delete[](void*, std::align_val_t, std::nothrow_t const&)
{
    #ifdef EMBRACE
    if(hooked__ZdaPvSt11align_val_tRKSt9nothrow_t && elf__ZdaPvSt11align_val_tRKSt9nothrow_t && elf__ZdaPvSt11align_val_tRKSt9nothrow_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZdaPvSt11align_val_tRKSt9nothrow_t(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked__ZdaPvSt11align_val_tRKSt9nothrow_t, 3, p, align, n);
    } else
    #endif
        if(box__ZdaPvSt11align_val_tRKSt9nothrow_t)
            box__ZdaPvSt11align_val_tRKSt9nothrow_t(p, align, n);
        else
            box_free(p);
}

EXPORT void _ZdlPvmSt11align_val_t(void* p, size_t sz, size_t align)   //operator delete(void*, unsigned long, std::align_val_t)
{
    #ifdef EMBRACE
    if(hooked__ZdlPvmSt11align_val_t && elf__ZdlPvmSt11align_val_t && elf__ZdlPvmSt11align_val_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZdlPvmSt11align_val_t(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked__ZdlPvmSt11align_val_t, 3, p, sz, align);
    } else
    #endif
        if(box__ZdlPvmSt11align_val_t)
            box__ZdlPvmSt11align_val_t(p, sz, align);
        else
            box_free(p);
}

EXPORT void _ZdaPvRKSt9nothrow_t(void* p, void* n)   //operator delete[](void*, std::nothrow_t const&)
{
    #ifdef EMBRACE
    if(hooked__ZdaPvRKSt9nothrow_t && elf__ZdaPvRKSt9nothrow_t && elf__ZdaPvRKSt9nothrow_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZdaPvRKSt9nothrow_t(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked__ZdaPvRKSt9nothrow_t, 2, p, n);
    } else
    #endif
        if(box__ZdaPvRKSt9nothrow_t)
            box__ZdaPvRKSt9nothrow_t(p, n);
        else
            box_free(p);
}

EXPORT void _ZdaPvSt11align_val_t(void* p, size_t align)   //operator delete[](void*, std::align_val_t)
{
    #ifdef EMBRACE
    if(hooked__ZdaPvSt11align_val_t && elf__ZdaPvSt11align_val_t && elf__ZdaPvSt11align_val_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZdaPvSt11align_val_t(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked__ZdaPvSt11align_val_t, 2, p, align);
    } else
    #endif
        if(box__ZdaPvSt11align_val_t)
            box__ZdaPvSt11align_val_t(p, align);
        else
            box_free(p);
}

EXPORT void _ZdlPvSt11align_val_t(void* p, size_t align)   //operator delete(void*, std::align_val_t)
{
    #ifdef EMBRACE
    if(hooked__ZdlPvSt11align_val_t && elf__ZdlPvSt11align_val_t && elf__ZdlPvSt11align_val_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZdlPvSt11align_val_t(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked__ZdlPvSt11align_val_t, 2, p, align);
    } else
    #endif
        if(box__ZdlPvSt11align_val_t)
            box__ZdlPvSt11align_val_t(p, align);
        else
            box_free(p);
}

EXPORT void _ZdlPvSt11align_val_tRKSt9nothrow_t(void* p, size_t align, void* n)   //operator delete(void*, std::align_val_t, std::nothrow_t const&)
{
    #ifdef EMBRACE
    if(hooked__ZdlPvSt11align_val_tRKSt9nothrow_t && elf__ZdlPvSt11align_val_tRKSt9nothrow_t && elf__ZdlPvSt11align_val_tRKSt9nothrow_t->init_done) {
        printf_log(LOG_DEBUG, "%04d|emulated _ZdlPvSt11align_val_tRKSt9nothrow_t(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked__ZdlPvSt11align_val_tRKSt9nothrow_t, 3, p, align, n);
    } else
    #endif
        if(box__ZdlPvSt11align_val_tRKSt9nothrow_t)
            box__ZdlPvSt11align_val_tRKSt9nothrow_t(p, align, n);
        else
            box_free(p);
}


void checkHookedSymbols(lib_t *maplib, elfheader_t* h)
{
    #ifndef EMBRACE
    static int hooked = 0;
    #endif
    for (size_t i=0; i<h->numDynSym; ++i) {
        const char * symname = h->DynStr+h->DynSym[i].st_name;
        int bind = ELF64_ST_BIND(h->DynSym[i].st_info);
        int type = ELF64_ST_TYPE(h->DynSym[i].st_info);
        int vis = h->DynSym[i].st_other&0x3;
        if((type==STT_FUNC) 
        && (vis==STV_DEFAULT || vis==STV_PROTECTED) && (h->DynSym[i].st_shndx!=0 && h->DynSym[i].st_shndx<=65521)) {
            uintptr_t offs = h->DynSym[i].st_value + h->delta;
            size_t sz = h->DynSym[i].st_size;
            if(bind!=STB_LOCAL && bind!=STB_WEAK) {
                #ifdef EMBRACE
                #define GO(A, B) if (!hooked_##A && !strcmp(symname, #A)) {hooked_##A = offs; elf_##A = h; if(hooked_##A) printf_log(LOG_INFO, "Overriding %s to %p (%s)\n", #A, (void*)hooked_##A, h->name);}
                #define GO2(A, B) if (!hooked_##A &&hooked_malloc && !strcmp(symname, #A)) {hooked_##A = offs; elf_##A = h; if(hooked_##A) printf_log(LOG_INFO, "Overriding %s to %p (%s)\n", #A, (void*)hooked_##A, h->name);}
                #else
                #define GO(A, B) if(!strcmp(symname, "__libc_" #A)) {uintptr_t alt = AddCheckBridge(my_context->system, B, A, 0, #A); printf_log(LOG_INFO, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); memcpy((void*)offs, (void*)alt, sizeof(onebridge_t));}
                #define GO2(A, B)
                SUPER()
                #undef GO
                #define GO(A, B) if(!strcmp(symname, "tc_" #A)) {uintptr_t alt = AddCheckBridge(my_context->system, B, A, 0, #A); printf_log(LOG_INFO, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); memcpy((void*)offs, (void*)alt, sizeof(onebridge_t));}
                SUPER()
                #undef GO
                #undef GO2
                #define GO(A, B) if(!strcmp(symname, #A)) {uintptr_t alt = AddCheckBridge(my_context->system, B, A, 0, #A); hooked=1; printf_log(LOG_INFO, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); memcpy((void*)offs, (void*)alt, sizeof(onebridge_t));}
                #define GO2(A, B) if(hooked && !strcmp(symname, #A)) {uintptr_t alt = AddCheckBridge(my_context->system, B, A, 0, #A); printf_log(LOG_INFO, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); memcpy((void*)offs, (void*)alt, sizeof(onebridge_t));}
                #endif
                SUPER()
                #undef GO
                #undef GO2
            }
        }
    }
}

void init_malloc_hook() {
#ifdef ANDROID
    __libc_malloc = dlsym(RTLD_NEXT, "malloc");
    __libc_realloc = dlsym(RTLD_NEXT, "realloc");
    __libc_calloc = dlsym(RTLD_NEXT, "calloc");
    __libc_free = dlsym(RTLD_NEXT, "free");
    __libc_memalign = dlsym(RTLD_NEXT, "memalign");
#endif
    box_malloc_usable_size = dlsym(RTLD_NEXT, "malloc_usable_size");
    #define GO(A, B)
    #define GO2(A, B)   box_##A = (B##_t)dlsym(RTLD_NEXT, #A); if(box_##A == (B##_t)A) box_##A = NULL;
    SUPER()
    #undef GO2
    #undef GO
}

#undef SUPER