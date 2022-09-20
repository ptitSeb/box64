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

 This is the Exterminate strategy implementation
*/

#include "bridge.h"
#include "tools/bridge_private.h"
#include "wrapper.h"

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
GO2(tc_calloc, pFLL);           \
GO2(tc_cfree, vFp);             \
GO2(tc_delete, vFp);            \
GO2(tc_deletearray, vFp);       \
GO2(tc_deletearray_nothrow, vFpp);              \
GO2(tc_delete_nothrow, vFpp);   \
GO2(tc_free, vFp);              \
GO2(tc_malloc, pFL);            \
GO2(tc_malloc_size, LFp);       \

typedef void* (*pFL_t)  (size_t);
typedef void* (*pFLp_t) (size_t, void* p);
typedef void  (*vFp_t)  (void*);
typedef void* (*pFp_t)  (void*);
typedef size_t(*LFp_t)  (void*);
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

static size_t pot(size_t l) {
    size_t ret = 0;
    while (l>(1<<ret))  ++ret;
    return 1<<ret;
}

// redefining all libc memory allocation routines
EXPORT void* malloc(size_t l)
{
    return box_calloc(1, l);
}

EXPORT void free(void* p)
{
    box_free(p);
}

EXPORT void* calloc(size_t n, size_t s)
{
    return box_calloc(n, s);
}

EXPORT void* realloc(void* p, size_t s)
{
    return box_realloc(p, s);
}

EXPORT void* aligned_alloc(size_t align, size_t size)
{
    return box_memalign(align, size);
}

EXPORT void* memalign(size_t align, size_t size)
{
    return box_memalign(align, size);
}

EXPORT int posix_memalign(void** p, size_t align, size_t size)
{
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
    return box_memalign(box64_pagesize, size);
}

EXPORT void* pvalloc(size_t size)
{
    return box_memalign(box64_pagesize, (size+box64_pagesize-1)&~(box64_pagesize-1));
}

EXPORT void cfree(void* p)
{
    box_free(p);
}

EXPORT size_t malloc_usable_size(void* p)
{
    return box_malloc_usable_size(p);
}

EXPORT void* my__Znwm(size_t sz)   //operator new(size_t)
{
    if(box__Znwm)
        return box__Znwm(sz);
    else
        return box_malloc(sz);
}

EXPORT void* my__ZnwmRKSt9nothrow_t(size_t sz, void* p)   //operator new(size_t, std::nothrow_t const&)
{
    if(box__ZnwmRKSt9nothrow_t)
        return box__ZnwmRKSt9nothrow_t(sz, p);
    else
        return box_malloc(sz);
}

EXPORT void* my__Znam(size_t sz)   //operator new[](size_t)
{
    if(box__Znam)
        return box__Znam(sz);
    else
        return box_malloc(sz);
}

EXPORT void* my__ZnamRKSt9nothrow_t(size_t sz, void* p)   //operator new[](size_t, std::nothrow_t const&)
{
    if(box__ZnamRKSt9nothrow_t)
        return box__ZnamRKSt9nothrow_t(sz, p);
    return box_malloc(sz);
}


EXPORT void my__ZdaPv(void* p)   //operator delete[](void*)
{
    if(box__ZdaPv)
        box__ZdaPv(p);
    else
        box_free(p);
}

EXPORT void my__ZdaPvm(void* p, size_t sz)   //operator delete[](void*, size_t)
{
    if(box__ZdaPvm)
        box__ZdaPvm(p, sz);
    else
        box_free(p);
}

EXPORT void my__ZdaPvmSt11align_val_t(void* p, size_t sz, size_t align)   //operator delete[](void*, unsigned long, std::align_val_t)
{
    if(box__ZdaPvmSt11align_val_t)
        box__ZdaPvmSt11align_val_t(p, sz, align);
    else
        box_free(p);
}

EXPORT void my__ZdlPv(void* p)   //operator delete(void*)
{
    if(box__ZdlPv)
        box__ZdlPv(p);
    else
        box_free(p);
}

EXPORT void my__ZdlPvm(void* p, size_t sz)   //operator delete(void*, size_t)
{
    if(box__ZdlPvm)
        box__ZdlPvm(p, sz);
    else
        box_free(p);
}

EXPORT void* my__ZnwmSt11align_val_t(size_t sz, size_t align)  //// operator new(unsigned long, std::align_val_t)
{
    if(box__ZnwmSt11align_val_t)
        return box__ZnwmSt11align_val_t(sz, align);
    else
        return box_memalign(align, sz);
}

EXPORT void* my__ZnwmSt11align_val_tRKSt9nothrow_t(size_t sz, size_t align, void* p)  //// operator new(unsigned long, std::align_val_t, std::nothrow_t const&)
{
    if(box__ZnwmSt11align_val_tRKSt9nothrow_t)
        return box__ZnwmSt11align_val_tRKSt9nothrow_t(sz, align, p);
    else
        return box_memalign(align, sz);
}

EXPORT void* my__ZnamSt11align_val_t(size_t sz, size_t align)  //// operator new[](unsigned long, std::align_val_t)
{
    if(box__ZnamSt11align_val_t)
        return box__ZnamSt11align_val_t(sz, align);
    else
        return box_memalign(align, sz);
}

EXPORT void* my__ZnamSt11align_val_tRKSt9nothrow_t(size_t sz, size_t align, void* p)  //// operator new[](unsigned long, std::align_val_t, std::nothrow_t const&)
{
    if(box__ZnamSt11align_val_tRKSt9nothrow_t)
        return box__ZnamSt11align_val_tRKSt9nothrow_t(sz, align, p);
    else
        return box_memalign(align, sz);
}

EXPORT void my__ZdlPvRKSt9nothrow_t(void* p, void* n)   //operator delete(void*, std::nothrow_t const&)
{
    if(box__ZdlPvRKSt9nothrow_t)
        box__ZdlPvRKSt9nothrow_t(p, n);
    else
        box_free(p);
}

EXPORT void my__ZdaPvSt11align_val_tRKSt9nothrow_t(void* p, size_t align, void* n)   //operator delete[](void*, std::align_val_t, std::nothrow_t const&)
{
    if(box__ZdaPvSt11align_val_tRKSt9nothrow_t)
        box__ZdaPvSt11align_val_tRKSt9nothrow_t(p, align, n);
    else
        box_free(p);
}

EXPORT void my__ZdlPvmSt11align_val_t(void* p, size_t sz, size_t align)   //operator delete(void*, unsigned long, std::align_val_t)
{
    if(box__ZdlPvmSt11align_val_t)
        box__ZdlPvmSt11align_val_t(p, sz, align);
    else
        box_free(p);
}

EXPORT void my__ZdaPvRKSt9nothrow_t(void* p, void* n)   //operator delete[](void*, std::nothrow_t const&)
{
    if(box__ZdaPvRKSt9nothrow_t)
        box__ZdaPvRKSt9nothrow_t(p, n);
    else
        box_free(p);
}

EXPORT void my__ZdaPvSt11align_val_t(void* p, size_t align)   //operator delete[](void*, std::align_val_t)
{
    if(box__ZdaPvSt11align_val_t)
        box__ZdaPvSt11align_val_t(p, align);
    else
        box_free(p);
}

EXPORT void my__ZdlPvSt11align_val_t(void* p, size_t align)   //operator delete(void*, std::align_val_t)
{
    if(box__ZdlPvSt11align_val_t)
        box__ZdlPvSt11align_val_t(p, align);
    else
        box_free(p);
}

EXPORT void my__ZdlPvSt11align_val_tRKSt9nothrow_t(void* p, size_t align, void* n)   //operator delete(void*, std::align_val_t, std::nothrow_t const&)
{
    if(box__ZdlPvSt11align_val_tRKSt9nothrow_t)
        box__ZdlPvSt11align_val_tRKSt9nothrow_t(p, align, n);
    else
        box_free(p);
}

EXPORT void* my_tc_calloc(size_t n, size_t s)
{
    if(box_tc_calloc)
        return box_tc_calloc(n, s);
    else
        return box_calloc(n, s);
}

EXPORT void my_tc_cfree(void* p)
{
    if(box_tc_cfree)
        box_tc_cfree(p);
    else
        box_free(p);
}

EXPORT void my_tc_delete(void* p)
{
    if(box_tc_delete)
        box_tc_delete(p);
    else
        box_free(p);
}

EXPORT void my_tc_deletearray(void* p)
{
    if(box_tc_deletearray)
        box_tc_deletearray(p);
    else
        box_free(p);
}

EXPORT void my_tc_deletearray_nothrow(void* p, void* n)
{
    if(box_tc_deletearray_nothrow)
        box_tc_deletearray_nothrow(p, n);
    else
        box_free(p);
}

EXPORT void my_tc_delete_nothrow(void* p, void* n)
{
    if(box_tc_delete_nothrow)
        box_tc_delete_nothrow(p, n);
    else
        box_free(p);
}

EXPORT void my_tc_free(void* p)
{
    if(box_tc_free)
        box_tc_free(p);
    else
        box_free(p);
}

EXPORT void* my_tc_malloc(size_t s)
{
    if(box_tc_malloc)
        return box_tc_malloc(s);
    else
        return box_calloc(1, s);
}

EXPORT size_t my_tc_malloc_size(void* p)
{
    if(box_tc_malloc_size)
        return box_tc_malloc_size(p);
    else
        return box_malloc_usable_size(p);
}

#pragma pack(push, 1)
typedef struct reloc_jmp_s {
    uint8_t _ff;
    uint8_t _25;
    uint32_t _00;
    void* addr;
} reloc_jmp_t;
#pragma pack(pop)

static void addRelocJmp(void* offs, void* where)
{
    reloc_jmp_t jmp = {0};
    jmp._ff = 0xff;
    jmp._25 = 0x25;
    jmp.addr = where;
    memcpy(offs, &jmp, sizeof(jmp));
}

void checkHookedSymbols(lib_t *maplib, elfheader_t* h)
{
    static int hooked = 0;
    for (size_t i=0; i<h->numDynSym; ++i) {
        const char * symname = h->DynStr+h->DynSym[i].st_name;
        int bind = ELF64_ST_BIND(h->DynSym[i].st_info);
        int type = ELF64_ST_TYPE(h->DynSym[i].st_info);
        int vis = h->DynSym[i].st_other&0x3;
        if((type==STT_FUNC) 
        && (vis==STV_DEFAULT || vis==STV_PROTECTED) && (h->DynSym[i].st_shndx!=0 && h->DynSym[i].st_shndx<=65521)) {
            uintptr_t offs = h->DynSym[i].st_value + h->delta;
            size_t sz = h->DynSym[i].st_size;
            if(bind!=STB_LOCAL && bind!=STB_WEAK && sz>=sizeof(reloc_jmp_t)) {
                #define GO(A, B) if(!strcmp(symname, "__libc_" #A)) {uintptr_t alt = AddCheckBridge(my_context->system, B, A, 0, #A); printf_log(LOG_INFO, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); addRelocJmp((void*)offs, (void*)alt);}
                #define GO2(A, B)
                SUPER()
                #undef GO
                #define GO(A, B) if(!strcmp(symname, "tc_" #A)) {uintptr_t alt = AddCheckBridge(my_context->system, B, A, 0, #A); printf_log(LOG_INFO, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); addRelocJmp((void*)offs, (void*)alt);}
                SUPER()
                #undef GO
                #undef GO2
                #define GO(A, B) if(!strcmp(symname, #A)) {uintptr_t alt = AddCheckBridge(my_context->system, B, A, 0, #A); hooked=1; printf_log(LOG_INFO, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); addRelocJmp((void*)offs, (void*)alt);}
                #define GO2(A, B) if(hooked && !strcmp(symname, #A)) {uintptr_t alt = AddCheckBridge(my_context->system, B, my_##A, 0, #A); printf_log(LOG_INFO, "Redirecting %s function from %p (%s)\n", symname, (void*)offs, ElfName(h)); addRelocJmp((void*)offs, (void*)alt);}
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
    #if 0
    #define GO(A, B)
    #define GO2(A, B)   box_##A = (B##_t)dlsym(RTLD_NEXT, #A); if(box_##A == (B##_t)A) box_##A = NULL;
    SUPER()
    #undef GO2
    #undef GO
    #endif
}

#undef SUPER