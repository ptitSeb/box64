#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sdl2rwops.h"
#include "debug.h"
#include "wrapper.h"
#ifdef BOX32
#include "wrapper32.h"
#include "sdl2align32.h"
#endif
#include "box64context.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "librarian/library_private.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"

typedef struct SDL2_RWops_s SDL2_RWops_t;

typedef int64_t (*sdl2_size)(SDL2_RWops_t *context);
typedef int64_t (*sdl2_seek)(SDL2_RWops_t *context, int64_t offset, int32_t whence);
typedef size_t (*sdl2_read)(SDL2_RWops_t *context, void *ptr, size_t size, size_t maxnum);
typedef size_t (*sdl2_write)(SDL2_RWops_t *context, const void *ptr, size_t size, size_t num);
typedef int32_t (*sdl2_close)(SDL2_RWops_t *context);

#define BOX64RW 0x79 // random signature value

typedef struct SDL2_RWops_s {
    sdl2_size  size;
    sdl2_seek  seek;
    sdl2_read  read;
    sdl2_write write;

    sdl2_close close;

    uint32_t type;
    union {
        struct {
            int autoclose;
            FILE *fp;
        } stdio;
        struct {
            uint8_t *base;
            uint8_t *here;
            uint8_t *stop;
        } mem;
        struct {
            void *data1;
        } unknown;
        struct {
            SDL2_RWops_t *orig;
            sdl2_freerw custom_free;
        } my;
        #ifdef BOX32
        struct {
            my_SDL2_RWops_32_t *orig;
            sdl2_freerw custom_free;
        } my32;
        #endif
    } hidden;
} SDL2_RWops_t;

#ifdef BOX32
#define SUPER()                 \
    if (box64_is32bits) {       \
        GO(size, IFp_32)        \
        GO(seek, IFpIi_32)      \
        GO(read, iFppii_32)     \
        GO(write, iFppii_32)    \
        GO(close, iFp_32)       \
    } else {                    \
        GO(size, IFp)           \
        GO(seek, IFpIi)         \
        GO(read, iFppii)        \
        GO(write, iFppii)       \
        GO(close, iFp)          \
    }
#else
#define SUPER()                 \
        GO(size, IFp)           \
        GO(seek, IFpIi)         \
        GO(read, iFppii)        \
        GO(write, iFppii)       \
        GO(close, iFp)
#endif

EXPORT int64_t my2_native_size(SDL2_RWops_t *context)
{
    #ifdef BOX32
    if(box64_is32bits) {
        inplace_SDL2_RWops_to_64(context);
        int64_t ret = context->hidden.my.orig->size(context->hidden.my.orig);
        inplace_SDL2_RWops_to_32(context);
        return ret;
    }
    #endif
    return context->hidden.my.orig->size(context->hidden.my.orig);
}
EXPORT int64_t my2_native_seek(SDL2_RWops_t *context, int64_t offset, int32_t whence)
{
    #ifdef BOX32
    if(box64_is32bits) {
        inplace_SDL2_RWops_to_64(context);
        int64_t ret = context->hidden.my.orig->seek(context->hidden.my.orig, offset, whence);
        inplace_SDL2_RWops_to_32(context);
        return ret;
    }
    #endif
    return context->hidden.my.orig->seek(context->hidden.my.orig, offset, whence);
}
EXPORT size_t my2_native_read(SDL2_RWops_t *context, void *ptr, size_t size, size_t maxnum)
{
    #ifdef BOX32
    if(box64_is32bits) {
        inplace_SDL2_RWops_to_64(context);
        size_t ret = context->hidden.my.orig->read(context->hidden.my.orig, ptr, size, maxnum);
        inplace_SDL2_RWops_to_32(context);
        return ret;
    }
    #endif
    return context->hidden.my.orig->read(context->hidden.my.orig, ptr, size, maxnum);
}
EXPORT size_t my2_native_write(SDL2_RWops_t *context, const void *ptr, size_t size, size_t num)
{
    #ifdef BOX32
    if(box64_is32bits) {
        inplace_SDL2_RWops_to_64(context);
        size_t ret = context->hidden.my.orig->write(context->hidden.my.orig, ptr, size, num);
        inplace_SDL2_RWops_to_32(context);
        return ret;
    }
    #endif
    return context->hidden.my.orig->write(context->hidden.my.orig, ptr, size, num);
}
EXPORT int32_t my2_native_close(SDL2_RWops_t *context)
{
    #ifdef BOX32
    if(box64_is32bits) {
        inplace_SDL2_RWops_to_64(context);
    }
    #endif
    int32_t ret = context->hidden.my.orig->close(context->hidden.my.orig);
    context->hidden.my.custom_free(context);
    return ret;
}
EXPORT int64_t my2_emulated_size(SDL2_RWops_t *context)
{
    #ifdef BOX32
    if(box64_is32bits) {
        if(GetNativeFnc(context->hidden.my32.orig->size) == my2_native_size)
            return my2_native_size(context->hidden.my.orig);
        uintptr_t f = context->hidden.my32.orig->size;
        int64_t ret = (int64_t)RunFunctionFmt(f, "p", context->hidden.my32.orig);
        return ret;
    }
    #endif
    return (int64_t)RunFunctionFmt((uintptr_t)context->hidden.my.orig->size, "p", context->hidden.my.orig);
}
EXPORT int64_t my2_emulated_seek(SDL2_RWops_t *context, int64_t offset, int32_t whence)
{
    #ifdef BOX32
    if(box64_is32bits) {
        if(GetNativeFnc(context->hidden.my32.orig->seek) == my2_native_seek)
            return my2_native_seek(context->hidden.my.orig, offset, whence);
        uintptr_t f = context->hidden.my32.orig->seek;
        int64_t ret = (int64_t)RunFunctionFmt(f, "pIi", context->hidden.my32.orig, offset, whence);
        return ret;
    }
    #endif
    return (int64_t)RunFunctionFmt((uintptr_t)context->hidden.my.orig->seek, "pIi", context->hidden.my.orig, offset, whence);
}
EXPORT size_t my2_emulated_read(SDL2_RWops_t *context, void *ptr, size_t size, size_t maxnum)
{
    #ifdef BOX32
    if(box64_is32bits) {
        if(GetNativeFnc(context->hidden.my32.orig->read) == my2_native_read)
            return my2_native_read(context->hidden.my.orig, ptr, size, maxnum);
        uintptr_t f = context->hidden.my32.orig->read;
        size_t ret = (size_t)RunFunctionFmt(f, "ppLL", context->hidden.my32.orig, ptr, size, maxnum);
        return ret;
    }
    #endif
    return (size_t)RunFunctionFmt((uintptr_t)context->hidden.my.orig->read, "ppLL", context->hidden.my.orig, ptr, size, maxnum);
}
EXPORT size_t my2_emulated_write(SDL2_RWops_t *context, const void *ptr, size_t size, size_t num)
{
    #ifdef BOX32
    if(box64_is32bits) {
        if(GetNativeFnc(context->hidden.my32.orig->write) == my2_native_write)
            return my2_native_write(context->hidden.my.orig, ptr, size, num);
        uintptr_t f = context->hidden.my32.orig->write;
        size_t ret = (size_t)RunFunctionFmt(f, "ppLL", context->hidden.my32.orig, ptr, size, num);
        return ret;
    }
    #endif
    return (size_t)RunFunctionFmt((uintptr_t)context->hidden.my.orig->write, "ppLL", context->hidden.my.orig, ptr, size, num);
}
EXPORT int32_t my2_emulated_close(SDL2_RWops_t *context)
{
    int ret = 0;
    #ifdef BOX32
    if(box64_is32bits) {
        if(GetNativeFnc(context->hidden.my32.orig->close) == my2_native_close)
            return my2_native_close(context->hidden.my.orig);
        uintptr_t f = context->hidden.my32.orig->close;
        ret = (size_t)RunFunctionFmt(f, "p", context->hidden.my32.orig);
    } else
    #endif
        ret = (int32_t)RunFunctionFmt((uintptr_t)context->hidden.my.orig->close, "p", context->hidden.my.orig);
    context->hidden.my.custom_free(context);
    return ret;
}

static uintptr_t emulated_sdl2allocrw = 0;
EXPORT SDL2_RWops_t* my_wrapped_sdl2allocrw()
{
    return (SDL2_RWops_t*)RunFunctionFmt(emulated_sdl2allocrw, "");
}
static uintptr_t emulated_sdl2freerw = 0;
EXPORT void my_wrapped_sdl2freerw(SDL2_RWops_t* p)
{
    RunFunctionFmt(emulated_sdl2freerw, "p", p);
}

static void checkSDL2isNative()
{
    if(my_context->sdl2allocrw)
        return;
    emulated_sdl2allocrw = FindGlobalSymbol(my_context->maplib, "SDL_AllocRW", -1, NULL, 0);
    emulated_sdl2freerw = FindGlobalSymbol(my_context->maplib, "SDL_FreeRW", -1, NULL, 0);
    if(emulated_sdl2allocrw && emulated_sdl2freerw) {
        my_context->sdl2allocrw = my_wrapped_sdl2allocrw;
        my_context->sdl2freerw = my_wrapped_sdl2freerw;
    } else
        printf_log(LOG_NONE, "Warning, cannot find SDL_AllocRW and/or SDL_FreeRW function in loaded libs");
}

SDL2_RWops_t* AddNativeRW2(x64emu_t* emu, SDL2_RWops_t* ops)
{
    if(!ops)
        return NULL;
    checkSDL2isNative();
    uintptr_t fnc;
    bridge_t* system = emu->context->system;

    sdl2_allocrw Alloc = (sdl2_allocrw)emu->context->sdl2allocrw;
    sdl2_freerw Free = (sdl2_freerw)emu->context->sdl2freerw;

    SDL2_RWops_t* newrw = Alloc();
    newrw->type = BOX64RW;
    newrw->hidden.my.orig = ops;
    newrw->hidden.my.custom_free = Free;

    // get or create wrapper, add it to map and change to the emulated one if rw
    #define GO(A, W) \
    fnc = AddCheckBridge(system, W, my2_native_##A, 0, NULL); \
    newrw->A = (sdl2_##A)fnc;

    SUPER()

    #undef GO

    return newrw;
}

// put Native RW function, wrapping emulated (callback style) ones if needed
SDL2_RWops_t* RWNativeStart2(x64emu_t* emu, SDL2_RWops_t* ops)
{
    if(!ops)
        return NULL;

    if(ops->type == BOX64RW)
        return ops->hidden.my.orig;

    checkSDL2isNative();
    sdl2_allocrw Alloc = (sdl2_allocrw)emu->context->sdl2allocrw;

    SDL2_RWops_t* newrw = Alloc();
    newrw->type = BOX64RW;
    newrw->hidden.my.orig = ops;
    newrw->hidden.my.custom_free = (sdl2_freerw)emu->context->sdl2freerw;

    // create wrapper
    #define GO(A, W) \
    newrw->A = my2_emulated_##A;

    SUPER()

    #undef GO

    return newrw;
}

void RWNativeEnd2(SDL2_RWops_t* ops)
{

    if(!ops || ops->type != BOX64RW)
        return; // do nothing

    ops->hidden.my.custom_free(ops);
}

int isRWops(SDL2_RWops_t* ops)
{
    if(!ops)
        return 0;
    #define GO(A, W)      \
    if(!ops->A || (uintptr_t)ops->A < 0x1000) return 0;

    SUPER()

    #undef GO
    // check if all then hidden content is just full of 0
    if(ops->hidden.mem.base==NULL && ops->hidden.mem.here==NULL && ops->hidden.mem.stop==NULL)
        return 0;
    // check the type (not sure it's a good check here)
    if (ops->type>5 && ops->type!=BOX64RW)
        return 0;
    return 1;
}
#ifdef BOX32
int isRWops32(my_SDL2_RWops_32_t* ops)
{
    if(!ops)
        return 0;
    #define GO(A, W)      \
    if(!ops->A || (uintptr_t)ops->A < 0x1000) return 0;

    SUPER()

    #undef GO
    // check if all then hidden content is just full of 0
    if(ops->hidden[0]==0 && ops->hidden[1]==0 && ops->hidden[2]==0)
        return 0;
    // check the type (not sure it's a good check here)
    if (ops->type>5 && ops->type!=BOX64RW)
        return 0;
    return 1;
}
#endif

int64_t RWNativeSeek2(SDL2_RWops_t *ops, int64_t offset, int32_t whence)
{
    return ops->seek(ops, offset, whence);
}
int64_t RWNativeSize2(SDL2_RWops_t *ops)
{
    return ops->size(ops);
}
size_t RWNativeRead2(SDL2_RWops_t* ops, void* ptr, size_t size, size_t maxnum)
{
    return ops->read(ops, ptr, size, maxnum);
}
size_t RWNativeWrite2(SDL2_RWops_t *ops, const void *ptr, size_t size, size_t num)
{
    return ops->write(ops, ptr, size, num);
}
int32_t RWNativeClose2(SDL2_RWops_t* ops)
{
    return ops->close(ops);
}
