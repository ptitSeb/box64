#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdarg.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"
#include "librarian/library_private.h"
#include "emu/x64emu_private.h"
#include "box64context.h"
#include "sdl2rwops.h"
#include "myalign.h"
#include "threads.h"

const char* sdl2Name = "libSDL2-2.0.so.0";
#define LIBNAME sdl2
static void* my_glhandle = NULL;
// DL functions from wrappedlibdl.c
void* my_dlopen(x64emu_t* emu, void *filename, int flag);
int my_dlclose(x64emu_t* emu, void *handle);
void* my_dlsym(x64emu_t* emu, void *handle, void *symbol);

static int sdl_Yes() { return 1;}
static int sdl_No() { return 0;}
int EXPORT my2_SDL_Has3DNow(void) __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_Has3DNowExt(void) __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_HasAltiVec(void) __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_HasMMX(void) __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasMMXExt(void) __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasNEON(void) __attribute__((alias("sdl_No")));   // No neon in x86_64 ;)
int EXPORT my2_SDL_HasRDTSC(void) __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasSSE(void) __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasSSE2(void) __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasSSE3(void) __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasSSE41(void) __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_HasSSE42(void) __attribute__((alias("sdl_No")));

typedef struct {
  int32_t freq;
  uint16_t format;
  uint8_t channels;
  uint8_t silence;
  uint16_t samples;
  uint16_t padding;
  uint32_t size;
  void (*callback)(void *userdata, uint8_t *stream, int32_t len);
  void *userdata;
} SDL2_AudioSpec;

typedef struct {
    uint8_t data[16];
} SDL_JoystickGUID;

typedef union {
    SDL_JoystickGUID guid;
    uint32_t         u[4];
} SDL_JoystickGUID_Helper;

typedef struct
{
    int32_t bindType;   // enum
    union
    {
        int button;
        int axis;
        struct {
            int hat;
            int hat_mask;
        } hat;
    } value;
} SDL_GameControllerButtonBind;


typedef void  (*vFv_t)();
typedef void  (*vFiupV_t)(int64_t, uint64_t, void*, va_list);
#define ADDED_FUNCTIONS() \
    GO(SDL_Quit, vFv_t)           \
    GO(SDL_AllocRW, sdl2_allocrw) \
    GO(SDL_FreeRW, sdl2_freerw)   \
    GO(SDL_LogMessageV, vFiupV_t)
#include "generated/wrappedsdl2types.h"

typedef struct sdl2_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} sdl2_my_t;

void* getSDL2My(library_t* lib)
{
    sdl2_my_t* my = (sdl2_my_t*)calloc(1, sizeof(sdl2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}

void freeSDL2My(void* lib)
{
    (void)lib;
    //sdl2_my_t *my = (sdl2_my_t *)lib;
}
#undef SUPER

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// Timer
#define GO(A)   \
static uintptr_t my_Timer_fct_##A = 0;                                      \
static uint64_t my_Timer_##A(uint64_t a, void* b)                           \
{                                                                           \
    return (uint64_t)RunFunction(my_context, my_Timer_fct_##A, 2, a, b);    \
}
SUPER()
#undef GO
static void* find_Timer_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_Timer_fct_##A == (uintptr_t)fct) return my_Timer_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Timer_fct_##A == 0) {my_Timer_fct_##A = (uintptr_t)fct; return my_Timer_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL2 Timer callback\n");
    return NULL;
    
}
// AudioCallback
#define GO(A)   \
static uintptr_t my_AudioCallback_fct_##A = 0;                      \
static void my_AudioCallback_##A(void* a, void* b, int c)           \
{                                                                   \
    RunFunction(my_context, my_AudioCallback_fct_##A, 3, a, b, c);  \
}
SUPER()
#undef GO
static void* find_AudioCallback_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_AudioCallback_fct_##A == (uintptr_t)fct) return my_AudioCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AudioCallback_fct_##A == 0) {my_AudioCallback_fct_##A = (uintptr_t)fct; return my_AudioCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL2 AudioCallback callback\n");
    return NULL;
    
}
// eventfilter
#define GO(A)   \
static uintptr_t my_eventfilter_fct_##A = 0;                                \
static int my_eventfilter_##A(void* userdata, void* event)                  \
{                                                                           \
    return (int)RunFunction(my_context, my_eventfilter_fct_##A, 2, userdata, event);    \
}
SUPER()
#undef GO
static void* find_eventfilter_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_eventfilter_fct_##A == (uintptr_t)fct) return my_eventfilter_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_eventfilter_fct_##A == 0) {my_eventfilter_fct_##A = (uintptr_t)fct; return my_eventfilter_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL2 eventfilter callback\n");
    return NULL;
    
}
static void* reverse_eventfilter_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_context->sdl2lib->priv.w.bridge, fct))
        return (void*)CheckBridged(my_context->sdl2lib->priv.w.bridge, fct);
    #define GO(A) if(my_eventfilter_##A == fct) return (void*)my_eventfilter_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_context->sdl2lib->priv.w.bridge, iFpp, fct, 0, NULL);
}

// LogOutput
#define GO(A)   \
static uintptr_t my_LogOutput_fct_##A = 0;                                  \
static void my_LogOutput_##A(void* a, int b, int c, void* d)                \
{                                                                           \
    RunFunction(my_context, my_LogOutput_fct_##A, 4, a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_LogOutput_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_LogOutput_fct_##A == (uintptr_t)fct) return my_LogOutput_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_LogOutput_fct_##A == 0) {my_LogOutput_fct_##A = (uintptr_t)fct; return my_LogOutput_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL2 LogOutput callback\n");
    return NULL;
}
static void* reverse_LogOutput_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_context->sdl2lib->priv.w.bridge, fct))
        return (void*)CheckBridged(my_context->sdl2lib->priv.w.bridge, fct);
    #define GO(A) if(my_LogOutput_##A == fct) return (void*)my_LogOutput_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_context->sdl2lib->priv.w.bridge, vFpiip, fct, 0, NULL);
}

#undef SUPER

// TODO: track the memory for those callback
EXPORT int64_t my2_SDL_OpenAudio(x64emu_t* emu, void* d, void* o)
{
    SDL2_AudioSpec *desired = (SDL2_AudioSpec*)d;

    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // create a callback
    void *fnc = (void*)desired->callback;
    desired->callback = find_AudioCallback_Fct(fnc);
    int ret = my->SDL_OpenAudio(desired, (SDL2_AudioSpec*)o);
    if (ret!=0) {
        // error, clean the callback...
        desired->callback = fnc;
        return ret;
    }
    // put back stuff in place?
    desired->callback = fnc;

    return ret;
}

EXPORT int64_t my2_SDL_OpenAudioDevice(x64emu_t* emu, void* device, int64_t iscapture, void* d, void* o, int64_t allowed)
{
    SDL2_AudioSpec *desired = (SDL2_AudioSpec*)d;

    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // create a callback
    void *fnc = (void*)desired->callback;
    desired->callback = find_AudioCallback_Fct(fnc);
    int ret = my->SDL_OpenAudioDevice(device, iscapture, desired, (SDL2_AudioSpec*)o, allowed);
    if (ret<=0) {
        // error, clean the callback...
        desired->callback = fnc;
        return ret;
    }
    // put back stuff in place?
    desired->callback = fnc;

    return ret;
}

EXPORT void *my2_SDL_LoadFile_RW(x64emu_t* emu, void* a, void* b, int c)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->SDL_LoadFile_RW(rw, b, c);
    if(c==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT void *my2_SDL_LoadBMP_RW(x64emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->SDL_LoadBMP_RW(rw, b);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT int64_t my2_SDL_SaveBMP_RW(x64emu_t* emu, void* a, void* b, int c)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int64_t r = my->SDL_SaveBMP_RW(rw, b, c);
    if(c==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT void *my2_SDL_LoadWAV_RW(x64emu_t* emu, void* a, int b, void* c, void* d, void* e)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->SDL_LoadWAV_RW(rw, b, c, d, e);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT int64_t my2_SDL_GameControllerAddMappingsFromRW(x64emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int64_t r = my->SDL_GameControllerAddMappingsFromRW(rw, b);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_ReadU8(x64emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadU8(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_ReadBE16(x64emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadBE16(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_ReadBE32(x64emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadBE32(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_ReadBE64(x64emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadBE64(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_ReadLE16(x64emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadLE16(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_ReadLE32(x64emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadLE32(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_ReadLE64(x64emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadLE64(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_WriteU8(x64emu_t* emu, void* a, uint8_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteU8(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_WriteBE16(x64emu_t* emu, void* a, uint16_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteBE16(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_WriteBE32(x64emu_t* emu, void* a, uint64_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteBE32(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_WriteBE64(x64emu_t* emu, void* a, uint64_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteBE64(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_WriteLE16(x64emu_t* emu, void* a, uint16_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteLE16(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_WriteLE32(x64emu_t* emu, void* a, uint64_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteLE32(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_WriteLE64(x64emu_t* emu, void* a, uint64_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteLE64(rw, v);
    RWNativeEnd2(rw);
    return r;
}

EXPORT void *my2_SDL_RWFromConstMem(x64emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    void* r = my->SDL_RWFromConstMem(a, b);
    return AddNativeRW2(emu, (SDL2_RWops_t*)r);
}
EXPORT void *my2_SDL_RWFromFP(x64emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    void* r = my->SDL_RWFromFP(a, b);
    return AddNativeRW2(emu, (SDL2_RWops_t*)r);
}
EXPORT void *my2_SDL_RWFromFile(x64emu_t* emu, void* a, void* b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    void* r = my->SDL_RWFromFile(a, b);
    return AddNativeRW2(emu, (SDL2_RWops_t*)r);
}
EXPORT void *my2_SDL_RWFromMem(x64emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    void* r = my->SDL_RWFromMem(a, b);
    return AddNativeRW2(emu, (SDL2_RWops_t*)r);
}

EXPORT int64_t my2_SDL_RWseek(x64emu_t* emu, void* a, int64_t offset, int64_t whence)
{
    //sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int64_t ret = RWNativeSeek2(rw, offset, whence);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT int64_t my2_SDL_RWtell(x64emu_t* emu, void* a)
{
    //sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int64_t ret = RWNativeSeek2(rw, 0, 1);  //1 == RW_SEEK_CUR
    RWNativeEnd2(rw);
    return ret;
}
EXPORT uint64_t my2_SDL_RWread(x64emu_t* emu, void* a, void* ptr, uint64_t size, uint64_t maxnum)
{
    //sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t ret = RWNativeRead2(rw, ptr, size, maxnum);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT uint64_t my2_SDL_RWwrite(x64emu_t* emu, void* a, const void* ptr, uint64_t size, uint64_t maxnum)
{
    //sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t ret = RWNativeWrite2(rw, ptr, size, maxnum);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT int my2_SDL_RWclose(x64emu_t* emu, void* a)
{
    //sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    return RWNativeClose2(rw);
}

EXPORT int my2_SDL_SaveAllDollarTemplates(x64emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int ret = my->SDL_SaveAllDollarTemplates(rw);
    RWNativeEnd2(rw);
    return ret;
}

EXPORT int my2_SDL_SaveDollarTemplate(x64emu_t* emu, int gesture, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int ret = my->SDL_SaveDollarTemplate(gesture, rw);
    RWNativeEnd2(rw);
    return ret;
}

EXPORT void *my2_SDL_AddTimer(x64emu_t* emu, uint64_t a, void* f, void* p)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    return my->SDL_AddTimer(a, find_Timer_Fct(f), p);
}

EXPORT int my2_SDL_RemoveTimer(x64emu_t* emu, void* t)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    return my->SDL_RemoveTimer(t);
}

EXPORT void my2_SDL_SetEventFilter(x64emu_t* emu, void* p, void* userdata)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    my->SDL_SetEventFilter(find_eventfilter_Fct(p), userdata);
}
EXPORT int my2_SDL_GetEventFilter(x64emu_t* emu, void** f, void* userdata)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    int ret = my->SDL_GetEventFilter(f, userdata);
    *f = reverse_eventfilter_Fct(*f);
    return ret;
}

EXPORT void my2_SDL_LogGetOutputFunction(x64emu_t* emu, void** f, void* arg)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;

    my->SDL_LogGetOutputFunction(f, arg);
    if(*f) *f = reverse_LogOutput_Fct(*f);
}
EXPORT void my2_SDL_LogSetOutputFunction(x64emu_t* emu, void* f, void* arg)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;

    my->SDL_LogSetOutputFunction(find_LogOutput_Fct(f), arg);
}

EXPORT int my2_SDL_vsnprintf(x64emu_t* emu, void* buff, size_t s, void * fmt, x64_va_list_t b)
{
    (void)emu;
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    int r = vsnprintf(buff, s, fmt, VARARGS);
    return r;
}

EXPORT void* my2_SDL_CreateThread(x64emu_t* emu, void* f, void* n, void* p)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;

    void* et = NULL;
    void* fnc = my_prepare_thread(emu, f, p, 0, &et);
    return my->SDL_CreateThread(fnc, n, et);
}

EXPORT int my2_SDL_snprintf(x64emu_t* emu, void* buff, size_t s, void * fmt, uint64_t * b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    return vsnprintf(buff, s, fmt, VARARGS);
}

char EXPORT *my2_SDL_GetBasePath(x64emu_t* emu) {
    char* p = strdup(emu->context->fullpath);
    char* b = strrchr(p, '/');
    if(b)
        *(b+1) = '\0';
    return p;
}

EXPORT void my2_SDL_LogCritical(x64emu_t* emu, int64_t cat, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_CRITICAL == 6
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    my->SDL_LogMessageV(cat, 6, fmt, VARARGS);
}

EXPORT void my2_SDL_LogError(x64emu_t* emu, int64_t cat, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_ERROR == 5
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    my->SDL_LogMessageV(cat, 5, fmt, VARARGS);
}

EXPORT void my2_SDL_LogWarn(x64emu_t* emu, int64_t cat, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_WARN == 4
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    my->SDL_LogMessageV(cat, 4, fmt, VARARGS);
}

EXPORT void my2_SDL_LogInfo(x64emu_t* emu, int64_t cat, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_INFO == 3
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    my->SDL_LogMessageV(cat, 3, fmt, VARARGS);
}

EXPORT void my2_SDL_LogDebug(x64emu_t* emu, int64_t cat, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_DEBUG == 2
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    my->SDL_LogMessageV(cat, 2, fmt, VARARGS);
}

EXPORT void my2_SDL_LogVerbose(x64emu_t* emu, int64_t cat, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_VERBOSE == 1
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    my->SDL_LogMessageV(cat, 1, fmt, VARARGS);
}

EXPORT void my2_SDL_Log(x64emu_t* emu, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_INFO == 3
    // SDL_LOG_CATEGORY_APPLICATION == 0
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    my->SDL_LogMessageV(0, 3, fmt, VARARGS);
}

void fillGLProcWrapper(box64context_t*);
extern char* libGL;
EXPORT void* my2_SDL_GL_GetProcAddress(x64emu_t* emu, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "Calling SDL_GL_GetProcAddress(%s) => ", rname);
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // check if glxprocaddress is filled, and search for lib and fill it if needed
    if(!emu->context->glxprocaddress)
        emu->context->glxprocaddress = (procaddess_t)my->SDL_GL_GetProcAddress;
    if(!emu->context->glwrappers) {
        fillGLProcWrapper(emu->context);
        // check if libGL is loaded, load it if not (helps DeadCells)
        if(!my_glhandle && !GetLibInternal(libGL?libGL:"libGL.so.1")) {
            // use a my_dlopen to actually open that lib, like SDL2 is doing...
            my_glhandle = my_dlopen(emu, libGL?libGL:"libGL.so.1", RTLD_LAZY|RTLD_GLOBAL);
        }
    }
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, emu->context->glmymap, rname);
    int is_my = (k==kh_end(emu->context->glmymap))?0:1;
    void* symbol;
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(emu->context->box64lib, tmp);
    } else 
        symbol = my->SDL_GL_GetProcAddress(name);
    if(!symbol) {
        if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", NULL);
        return NULL;    // easy
    }
    // check if alread bridged
    uintptr_t ret = CheckBridged(emu->context->system, symbol);
    if(ret) {
        if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", (void*)ret);
        return (void*)ret; // already bridged
    }
    // get wrapper    
    k = kh_get(symbolmap, emu->context->glwrappers, rname);
    if(k==kh_end(emu->context->glwrappers) && strstr(rname, "ARB")==NULL) {
        // try again, adding ARB at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "ARB");
        k = kh_get(symbolmap, emu->context->glwrappers, tmp);
    }
    if(k==kh_end(emu->context->glwrappers) && strstr(rname, "EXT")==NULL) {
        // try again, adding EXT at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "EXT");
        k = kh_get(symbolmap, emu->context->glwrappers, tmp);
    }
    if(k==kh_end(emu->context->glwrappers)) {
        if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", NULL);
        printf_log(LOG_INFO, "Warning, no wrapper for %s\n", rname);
        return NULL;
    }
    AddOffsetSymbol(emu->context->maplib, symbol, rname);
    const char* constname = kh_key(emu->context->glwrappers, k);
    ret = AddBridge(emu->context->system, kh_value(emu->context->glwrappers, k), symbol, 0, constname);
    if(dlsym_error && box64_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", (void*)ret);
    return (void*)ret;
}

#define nb_once	16
typedef void(*sdl2_tls_dtor)(void*);
static uintptr_t dtor_emu[nb_once] = {0};
static void tls_dtor_callback(int n, void* a)
{
	if(dtor_emu[n]) {
        RunFunction(my_context, dtor_emu[n], 1, a);
	}
}
#define GO(N) \
void tls_dtor_callback_##N(void* a) \
{ \
	tls_dtor_callback(N, a); \
}

GO(0)
GO(1)
GO(2)
GO(3)
GO(4)
GO(5)
GO(6)
GO(7)
GO(8)
GO(9)
GO(10)
GO(11)
GO(12)
GO(13)
GO(14)
GO(15)
#undef GO
static const sdl2_tls_dtor dtor_cb[nb_once] = {
	 tls_dtor_callback_0, tls_dtor_callback_1, tls_dtor_callback_2, tls_dtor_callback_3
	,tls_dtor_callback_4, tls_dtor_callback_5, tls_dtor_callback_6, tls_dtor_callback_7
	,tls_dtor_callback_8, tls_dtor_callback_9, tls_dtor_callback_10,tls_dtor_callback_11
	,tls_dtor_callback_12,tls_dtor_callback_13,tls_dtor_callback_14,tls_dtor_callback_15
};
EXPORT int64_t my2_SDL_TLSSet(x64emu_t* emu, uint64_t id, void* value, void* dtor)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;

	if(!dtor)
		return my->SDL_TLSSet(id, value, NULL);
	int n = 0;
	while (n<nb_once) {
		if(!dtor_emu[n] || (dtor_emu[n])==((uintptr_t)dtor)) {
			dtor_emu[n] = (uintptr_t)dtor;
			return my->SDL_TLSSet(id, value, dtor_cb[n]);
		}
		++n;
	}
	printf_log(LOG_NONE, "Error: SDL2 SDL_TLSSet with destructor: no more slot!\n");
	//emu->quit = 1;
	return -1;
}

EXPORT void my2_SDL_AddEventWatch(x64emu_t* emu, void* p, void* userdata)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    my->SDL_AddEventWatch(find_eventfilter_Fct(p), userdata);
}
EXPORT void my2_SDL_DelEventWatch(x64emu_t* emu, void* p, void* userdata)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    my->SDL_DelEventWatch(find_eventfilter_Fct(p), userdata);
}

EXPORT void* my2_SDL_LoadObject(x64emu_t* emu, void* sofile)
{
    return my_dlopen(emu, sofile, 0);   // TODO: check correct flag value...
}
EXPORT void my2_SDL_UnloadObject(x64emu_t* emu, void* handle)
{
    my_dlclose(emu, handle);
}
EXPORT void* my2_SDL_LoadFunction(x64emu_t* emu, void* handle, void* name)
{
    return my_dlsym(emu, handle, name);
}

EXPORT int64_t my2_SDL_IsJoystickPS4(x64emu_t* emu, uint16_t vendor, uint16_t product_id)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;

    if(my->SDL_IsJoystickPS4)
        return my->SDL_IsJoystickPS4(vendor, product_id);
    // fallback
    return 0;
}
EXPORT int64_t my2_SDL_IsJoystickNintendoSwitchPro(x64emu_t* emu, uint16_t vendor, uint16_t product_id)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;

    if(my->SDL_IsJoystickNintendoSwitchPro)
        return my->SDL_IsJoystickNintendoSwitchPro(vendor, product_id);
    // fallback
    return 0;
}
EXPORT int64_t my2_SDL_IsJoystickSteamController(x64emu_t* emu, uint16_t vendor, uint16_t product_id)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;

    if(my->SDL_IsJoystickSteamController)
        return my->SDL_IsJoystickSteamController(vendor, product_id);
    // fallback
    return 0;
}
EXPORT int64_t my2_SDL_IsJoystickXbox360(x64emu_t* emu, uint16_t vendor, uint16_t product_id)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;

    if(my->SDL_IsJoystickXbox360)
        return my->SDL_IsJoystickXbox360(vendor, product_id);
    // fallback
    return 0;
}
EXPORT int64_t my2_SDL_IsJoystickXboxOne(x64emu_t* emu, uint16_t vendor, uint16_t product_id)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;

    if(my->SDL_IsJoystickXboxOne)
        return my->SDL_IsJoystickXboxOne(vendor, product_id);
    // fallback
    return 0;
}
EXPORT int64_t my2_SDL_IsJoystickXInput(x64emu_t* emu, uint64_t a, uint64_t b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;

    if(my->SDL_IsJoystickXInput)
        return my->SDL_IsJoystickXInput(a, b);
    // fallback
    return 0;
}
EXPORT int64_t my2_SDL_IsJoystickHIDAPI(x64emu_t* emu, uint64_t a, uint64_t b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;

    if(my->SDL_IsJoystickHIDAPI)
        return my->SDL_IsJoystickHIDAPI(a, b);
    // fallback
    return 0;
}

void* my_vkGetInstanceProcAddr(x64emu_t* emu, void* device, void* name);
EXPORT void* my2_SDL_Vulkan_GetVkGetInstanceProcAddr(x64emu_t* emu)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    
    if(!emu->context->vkprocaddress)
        emu->context->vkprocaddress = (vkprocaddess_t)my->SDL_Vulkan_GetVkGetInstanceProcAddr();

    if(emu->context->vkprocaddress)
        return (void*)AddCheckBridge(my_context->sdl2lib->priv.w.bridge, pFEpp, my_vkGetInstanceProcAddr, 0, "vkGetInstanceProcAddr");
    return NULL;
}

EXPORT void my2_SDL_GetJoystickGUIDInfo(SDL_JoystickGUID guid, uint16_t *vend, uint16_t *prod, uint16_t *ver)
{
    sdl2_my_t *my = (sdl2_my_t *)my_context->sdl2lib->priv.w.p2;

    if(my->SDL_GetJoystickGUIDInfo)
        my->SDL_GetJoystickGUIDInfo(guid, vend, prod, ver);
    // fallback
    else {
        uint16_t *guid16 = (uint16_t *)guid.data;
        if (guid16[1]==0x0000 && guid16[3]==0x0000 && guid16[5]==0x0000)
            {
            if(vend) *vend = guid16[2];
            if(prod) *prod = guid16[4];
            if(ver)  *ver  = guid16[6];
        } else {
            if(vend) *vend = 0;
            if(prod) *prod = 0;
            if(ver)  *ver  = 0;
        }
    }
}

#define CUSTOM_INIT \
    box64->sdl2lib = lib;                                           \
    lib->priv.w.p2 = getSDL2My(lib);                                \
    box64->sdl2allocrw = ((sdl2_my_t*)lib->priv.w.p2)->SDL_AllocRW; \
    box64->sdl2freerw  = ((sdl2_my_t*)lib->priv.w.p2)->SDL_FreeRW;  \
    lib->altmy = strdup("my2_");                                    \
    lib->priv.w.needed = 4;                                         \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libdl.so.2");               \
    lib->priv.w.neededlibs[1] = strdup("libm.so.6");                \
    lib->priv.w.neededlibs[2] = strdup("librt.so.1");               \
    lib->priv.w.neededlibs[3] = strdup("libpthread.so.0");

#define CUSTOM_FINI \
    ((sdl2_my_t *)lib->priv.w.p2)->SDL_Quit();                  \
    if(my_glhandle) my_dlclose(thread_get_emu(), my_glhandle);  \
    my_glhandle = NULL;                                         \
    freeSDL2My(lib->priv.w.p2);                                 \
    free(lib->priv.w.p2);                                       \
    ((box64context_t*)(lib->context))->sdl2lib = NULL;          \
    ((box64context_t*)(lib->context))->sdl2allocrw = NULL;      \
    ((box64context_t*)(lib->context))->sdl2freerw = NULL;


#include "wrappedlib_init.h"
