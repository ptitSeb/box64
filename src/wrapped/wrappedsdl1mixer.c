#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "box64context.h"
#include "sdl1rwops.h"
#include "callback.h"

typedef void* (*pFpi_t)(void*, int32_t);
typedef void* (*pFp_t)(void*);
typedef int   (*iFip_t)(int, void*);
typedef void* (*pFpii_t)(void*, int32_t, int32_t);
typedef void  (*vFp_t)(void*);
typedef void  (*vFpp_t)(void*, void*);
typedef int   (*iFippp_t)(int, void*, void*, void*);

#define SUPER() \
    GO(Mix_LoadMUSType_RW,pFpii_t)      \
    GO(Mix_LoadMUS_RW,pFp_t)            \
    GO(Mix_LoadWAV_RW,pFpi_t)           \
    GO(Mix_SetPostMix,vFpp_t)           \
    GO(Mix_ChannelFinished,vFp_t)       \
    GO(Mix_HookMusic, vFpp_t)           \
    GO(Mix_HookMusicFinished, vFp_t)    \
    GO(Mix_RegisterEffect, iFippp_t)    \
    GO(Mix_UnregisterEffect, iFip_t)    \

typedef struct sdl1mixer_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} sdl1mixer_my_t;

static void* getSDL1MixerMy(library_t* lib)
{
    sdl1mixer_my_t* my = (sdl1mixer_my_t*)calloc(1, sizeof(sdl1mixer_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// EffectFunc
#define GO(A)   \
static uintptr_t my_EffectFunc_fct_##A = 0;                                         \
static void my_EffectFunc_##A(int chan, void *stream, int len, void *udata)         \
{                                                                                   \
    RunFunction(my_context, my_EffectFunc_fct_##A, 4, chan, stream, len, udata);    \
}
SUPER()
#undef GO
static void* find_EffectFunc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_EffectFunc_fct_##A == (uintptr_t)fct) return my_EffectFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EffectFunc_fct_##A == 0) {my_EffectFunc_fct_##A = (uintptr_t)fct; return my_EffectFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1Mixer EffectFunc callback\n");
    return NULL;
}

// EffectDone
#define GO(A)   \
static uintptr_t my_EffectDone_fct_##A = 0;                         \
static void my_EffectDone_##A(int chan, void *udata)                \
{                                                                   \
    RunFunction(my_context, my_EffectDone_fct_##A, 2, chan, udata); \
}
SUPER()
#undef GO
static void* find_EffectDone_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_EffectDone_fct_##A == (uintptr_t)fct) return my_EffectDone_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EffectDone_fct_##A == 0) {my_EffectDone_fct_##A = (uintptr_t)fct; return my_EffectDone_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1Mixer EffectDone callback\n");
    return NULL;
}

// MixFunc
#define GO(A)   \
static uintptr_t my_MixFunc_fct_##A = 0;                                \
static void my_MixFunc_##A(void *udata, uint8_t *stream, int len)       \
{                                                                       \
    RunFunction(my_context, my_MixFunc_fct_##A, 3, udata, stream, len); \
}
SUPER()
#undef GO
static void* find_MixFunc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_MixFunc_fct_##A == (uintptr_t)fct) return my_MixFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_MixFunc_fct_##A == 0) {my_MixFunc_fct_##A = (uintptr_t)fct; return my_MixFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1Mixer MixFunc callback\n");
    return NULL;
}

// ChannelFinished
#define GO(A)   \
static uintptr_t my_ChannelFinished_fct_##A = 0;                        \
static void my_ChannelFinished_##A(int channel)                         \
{                                                                       \
    RunFunction(my_context, my_ChannelFinished_fct_##A, 1, channel);    \
}
SUPER()
#undef GO
static void* find_ChannelFinished_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_ChannelFinished_fct_##A == (uintptr_t)fct) return my_ChannelFinished_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ChannelFinished_fct_##A == 0) {my_ChannelFinished_fct_##A = (uintptr_t)fct; return my_ChannelFinished_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1Mixer ChannelFinished callback\n");
    return NULL;
}

// MusicFinished
#define GO(A)   \
static uintptr_t my_MusicFinished_fct_##A = 0;              \
static void my_MusicFinished_##A()                          \
{                                                           \
    RunFunction(my_context, my_MusicFinished_fct_##A, 0);   \
}
SUPER()
#undef GO
static void* find_MusicFinished_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_MusicFinished_fct_##A == (uintptr_t)fct) return my_MusicFinished_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_MusicFinished_fct_##A == 0) {my_MusicFinished_fct_##A = (uintptr_t)fct; return my_MusicFinished_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1Mixer MusicFinished callback\n");
    return NULL;
}

#undef SUPER

static void freeSDL1MixerMy(library_t* lib)
{
    //sdl1mixer_my_t *my = lib->priv.w.p2;
}

EXPORT void* my_Mix_LoadMUSType_RW(x64emu_t* emu, void* a, int32_t b, int32_t c)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->Mix_LoadMUSType_RW(rw, b, c);
    if(c==0)
        RWNativeEnd(rw);
    return r;
}
EXPORT void* my_Mix_LoadMUS_RW(x64emu_t* emu, void* a)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->Mix_LoadMUS_RW(rw);
    RWNativeEnd(rw);  // this one never free the RWops
    return r;
}
EXPORT void* my_Mix_LoadWAV_RW(x64emu_t* emu, void* a, int32_t b)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->Mix_LoadWAV_RW(rw, b);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}

EXPORT void my_Mix_SetPostMix(x64emu_t* emu, void* a, void* b)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    my->Mix_SetPostMix(find_MixFunc_Fct(a), b);
}

EXPORT void my_Mix_ChannelFinished(x64emu_t* emu, void* cb)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    my->Mix_ChannelFinished(find_ChannelFinished_Fct(cb));
}

EXPORT void my_Mix_HookMusic(x64emu_t* emu, void* f, void* arg)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    my->Mix_HookMusic(find_MixFunc_Fct(f), arg);
}

EXPORT void my_Mix_HookMusicFinished(x64emu_t* emu, void* f)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    my->Mix_HookMusicFinished(find_MusicFinished_Fct(f));
}

EXPORT int my_Mix_RegisterEffect(x64emu_t* emu, int chan, void* f, void* d, void *arg)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    return my->Mix_RegisterEffect(chan, find_EffectFunc_Fct(f), find_EffectDone_Fct(d), arg);
}

EXPORT int my_Mix_UnregisterEffect(x64emu_t* emu, int channel, void* f)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    return my->Mix_UnregisterEffect(channel, find_EffectFunc_Fct(f));
}

const char* sdl1mixerName = "libSDL_mixer-1.2.so.0";
#define LIBNAME sdl1mixer

#define CUSTOM_INIT \
    box64->sdl1mixerlib = lib; \
    lib->priv.w.p2 = getSDL1MixerMy(lib);

#define CUSTOM_FINI \
    freeSDL1MixerMy(lib); \
    free(lib->priv.w.p2); \
    ((box64context_t*)(lib->context))->sdl1mixerlib = NULL;

#include "wrappedlib_init.h"

