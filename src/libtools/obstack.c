#define _LARGEFILE_SOURCE 1
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <dlfcn.h>
#include <obstack.h>

#include "wrappedlibs.h"

#include "box64stack.h"
#include "x64emu.h"
#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"
#include "librarian/library_private.h"
#include "emu/x64emu_private.h"
#include "box64context.h"
#include "myalign.h"
#include "signals.h"
#include "fileutils.h"
#include "auxval.h"
#include "elfloader.h"
#include "bridge.h"

typedef void    (*vFv_t)    ();
typedef int32_t (*iFppp_t)  (void*, void*, void*);
typedef int32_t (*iFpLLpp_t)(void*, size_t, size_t, void*, void*);

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// chunkfun
#define GO(A)   \
static uintptr_t my_chunkfun_fct_##A = 0;                                      \
static void* my_chunkfun_##A(size_t a) \
{                                                                               \
    return (void*)RunFunction(my_chunkfun_fct_##A, 1, a);                       \
}
SUPER()
#undef GO
static void* findchunkfunFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_chunkfun_fct_##A == (uintptr_t)fct) return my_chunkfun_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_chunkfun_fct_##A == 0) {my_chunkfun_fct_##A = (uintptr_t)fct; return my_chunkfun_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc chunkfun callback\n");
    return NULL;
}
static void* reverse_chunkfunFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my_chunkfun_##A == fct) return (void*)my_chunkfun_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, pFL, fct, 0, NULL);
}
// freefun
#define GO(A)   \
static uintptr_t my_freefun_fct_##A = 0;                \
static void my_freefun_##A(void* a)                     \
{                                                       \
    RunFunction(my_freefun_fct_##A, 1, a);              \
}
SUPER()
#undef GO
static void* findfreefunFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_freefun_fct_##A == (uintptr_t)fct) return my_freefun_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_freefun_fct_##A == 0) {my_freefun_fct_##A = (uintptr_t)fct; return my_freefun_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc freefun callback\n");
    return NULL;
}
static void* reverse_freefunFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->w.bridge, fct))
        return (void*)CheckBridged(lib->w.bridge, fct);
    #define GO(A) if(my_freefun_##A == fct) return (void*)my_freefun_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->w.bridge, vFp, fct, 0, NULL);
}

#undef SUPER
#if 0
struct i386_obstack
{
  long	chunk_size;
  struct _obstack_chunk *chunk;
  char	*object_base;
  char	*next_free;
  char	*chunk_limit;
  union
  {
    uintptr_t tempint;
    void *tempptr;
  } temp;
  int   alignment_mask;
  struct _obstack_chunk *(*chunkfun) (void *, long);
  void (*freefun) (void *, struct _obstack_chunk *);
  void *extra_arg;
  unsigned use_extra_arg:1;
  unsigned maybe_empty_object:1;
  unsigned alloc_failed:1;
} __attribute__((packed));

#define SUPER() \
GO(chunk_size)  \
GO(chunk)       \
GO(object_base) \
GO(next_free)   \
GO(chunk_limit) \
GO(temp.tempint)\
GO(alignment_mask)      \
GO(extra_arg)           \
GO(use_extra_arg)       \
GO(maybe_empty_object)  \
GO(alloc_failed)

void to_i386_obstack(struct i386_obstack *_i386, struct obstack *native)
{
    #define GO(A)   _i386->A = native->A;
    SUPER();
    #undef GO
    _i386->chunkfun = findchunkfunFct(native->chunkfun);
    _i386->freefun = findfreefunFct(native->freefun);
}

void from_i386_obstack(struct i386_obstack *_i386, struct obstack *native)
{
    #define GO(A)   native->A = _i386->A;
    SUPER();
    #undef GO
    native->chunkfun = reverse_chunkfunFct(my_context->libclib, _i386->chunkfun);
    native->freefun = reverse_freefunFct(my_context->libclib, _i386->freefun);
}
#undef SUPER
EXPORT int my__obstack_begin(struct i386_obstack * obstack, size_t size, size_t alignment, void* chunkfun, void* freefun)
{
    struct obstack native = {0};
    from_i386_obstack(obstack, &native);    // is this needed?
    int ret = _obstack_begin(&native, size, alignment, findchunkfunFct(chunkfun), findfreefunFct(freefun));
    to_i386_obstack(obstack, &native);
    return ret;
}

EXPORT void my_obstack_free(struct i386_obstack * obstack, void* block)
{
    struct obstack native = {0};
    from_i386_obstack(obstack, &native);
    obstack_free(&native, block);
    to_i386_obstack(obstack, &native);  // usefull??
}
EXPORT void my__obstack_free(struct i386_obstack * obstack, void* block) __attribute__((alias("my_obstack_free")));

EXPORT void my__obstack_newchunk(x64emu_t* emu, struct i386_obstack* obstack, int s)
{
    struct obstack native = {0};
    from_i386_obstack(obstack, &native);
    _obstack_newchunk(&native, s);
    to_i386_obstack(obstack, &native);  //usefull??
}

EXPORT int32_t my_obstack_vprintf(x64emu_t* emu, struct i386_obstack* obstack, void* fmt, x64_va_list_t V)
{
    struct obstack native = {0};
    from_i386_obstack(obstack, &native);
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, V);
    PREPARE_VALIST;
    #endif
    int r = obstack_vprintf(&native, (const char*)fmt, VARARGS);
    to_i386_obstack(obstack, &native);  //usefull??
    return r;
}
#else

EXPORT int my__obstack_begin(struct obstack * obstack, size_t size, size_t alignment, void* chunkfun, void* freefun)
{
    int ret = _obstack_begin(obstack, size, alignment, findchunkfunFct(chunkfun), findfreefunFct(freefun));
    return ret;
}

EXPORT void my_obstack_free(struct obstack * obstack, void* block)
{
    obstack_free(obstack, block);
}
EXPORT void my__obstack_free(struct obstack * obstack, void* block) __attribute__((alias("my_obstack_free")));

EXPORT void my__obstack_newchunk(x64emu_t* emu, struct obstack* obstack, int s)
{
    _obstack_newchunk(obstack, s);
}

EXPORT int32_t my_obstack_vprintf(x64emu_t* emu, struct obstack* obstack, void* fmt, x64_va_list_t V)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(V);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, V);
    PREPARE_VALIST;
    #endif
    int r = obstack_vprintf(obstack, (const char*)fmt, VARARGS);
    return r;
}
#endif

EXPORT void* my_obstack_alloc_failed_handler = NULL;
void* ref_obstack_alloc_failed_handler = NULL;
vFv_t real_obstack_alloc_failed_handler = NULL;
void actual_obstack_alloc_failed_handler()
{
    if(ref_obstack_alloc_failed_handler == my_obstack_alloc_failed_handler)
        real_obstack_alloc_failed_handler();
    RunFunction((uintptr_t)my_obstack_alloc_failed_handler, 0);
}
void obstackSetup()
{
    // save the real function
    real_obstack_alloc_failed_handler = obstack_alloc_failed_handler;
    // bridge the real function for x64 world
    my_obstack_alloc_failed_handler = (void*)AddCheckBridge(my_context->system, vFv, real_obstack_alloc_failed_handler, 0, NULL);
    ref_obstack_alloc_failed_handler = my_obstack_alloc_failed_handler;
    // setup our version of the function
    obstack_alloc_failed_handler = actual_obstack_alloc_failed_handler;

}
