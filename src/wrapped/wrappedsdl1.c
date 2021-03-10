#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "debug.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "callback.h"
#include "librarian.h"
#include "emu/x64emu_private.h"
#include "box64context.h"
#include "sdl1rwops.h"

#include "x64trace.h"
#include "threads.h"

const char* sdl1Name = "libSDL-1.2.so.0";
#define LIBNAME sdl1

int sdl_Yes() { return 1; }
int sdl_No() { return 0; }
int EXPORT my_SDL_Has3DNow() __attribute__((alias("sdl_No")));
int EXPORT my_SDL_Has3DNowExt() __attribute__((alias("sdl_No")));
int EXPORT my_SDL_HasAltiVec() __attribute__((alias("sdl_No")));
int EXPORT my_SDL_HasMMX() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasMMXExt() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasRDTSC() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasSSE() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasSSE2() __attribute__((alias("sdl_Yes")));

typedef struct {
  int32_t freq;
  uint16_t format;
  uint8_t channels;
  uint8_t silence;
  uint16_t samples;
  uint32_t size;
  void (*callback)(void *userdata, uint8_t *stream, int32_t len);
  void *userdata;
} SDL_AudioSpec;

// TODO: put the wrapper type in a dedicate include
typedef void  (*vFv_t)();
typedef void* (*pFv_t)();
typedef void* (*pFpi_t)(void*, int32_t);
typedef void* (*pFp_t)(void*);
typedef void* (*pFpp_t)(void*, void*);
typedef int (*iFup_t)(uint32_t, void*);
typedef int32_t (*iFp_t)(void*);
typedef int32_t (*iFppi_t)(void*, void*, int32_t);
typedef void* (*pFpippp_t)(void*, int32_t, void*, void*, void*);
typedef void  (*vFp_t)(void*);
typedef void* (*pFpp_t)(void*, void*);
typedef uint32_t (*uFp_t)(void*);
typedef uint64_t (*UFp_t)(void*);
typedef uint32_t (*uFu_t)(uint32_t);
typedef int32_t (*iFpp_t)(void*, void*);
typedef uint32_t (*uFpW_t)(void*, uint16_t);
typedef uint32_t (*uFpu_t)(void*, uint32_t);
typedef uint32_t (*uFpU_t)(void*, uint64_t);
typedef uint32_t (*uFupp_t)(uint32_t, void*, void*);

#define SUPER()                                     \
    GO(SDL_Quit, vFv_t)                             \
    GO(SDL_AllocRW, sdl1_allocrw)                   \
    GO(SDL_FreeRW, sdl1_freerw)                     \
    GO(SDL_OpenAudio, iFpp_t)                       \
    GO(SDL_LoadBMP_RW, pFpi_t)                      \
    GO(SDL_RWFromConstMem, pFpi_t)                  \
    GO(SDL_RWFromFP, pFpi_t)                        \
    GO(SDL_RWFromFile, pFpp_t)                      \
    GO(SDL_RWFromMem, pFpi_t)                       \
    GO(SDL_SaveBMP_RW, iFppi_t)                     \
    GO(SDL_LoadWAV_RW, pFpippp_t)                   \
    GO(SDL_ReadBE16, uFp_t)                         \
    GO(SDL_ReadBE32, uFp_t)                         \
    GO(SDL_ReadBE64, UFp_t)                         \
    GO(SDL_ReadLE16, uFp_t)                         \
    GO(SDL_ReadLE32, uFp_t)                         \
    GO(SDL_ReadLE64, UFp_t)                         \
    GO(SDL_WriteBE16, uFpW_t)                       \
    GO(SDL_WriteBE32, uFpu_t)                       \
    GO(SDL_WriteBE64, uFpU_t)                       \
    GO(SDL_WriteLE16, uFpW_t)                       \
    GO(SDL_WriteLE32, uFpu_t)                       \
    GO(SDL_WriteLE64, uFpU_t)                       \
    GO(SDL_AddTimer, uFupp_t)                       \
    GO(SDL_RemoveTimer, uFu_t)                      \
    GO(SDL_CreateThread, pFpp_t)                    \
    GO(SDL_KillThread, vFp_t)                       \
    GO(SDL_SetEventFilter, vFp_t)                   \
    GO(SDL_GL_GetProcAddress, pFp_t)                \
    GO(SDL_GetWMInfo, iFp_t)                        \
    GO(SDL_SetTimer, iFup_t)                        \
    GO(SDL_GetEventFilter, pFv_t)                   \

typedef struct sdl1_my_s {
    // functions
    #define GO(A, B)    B   A;
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} sdl1_my_t;

// event filter. Needs to be global, but there is only one, so that's should be fine
x64emu_t        *sdl1_evtfilter = NULL;
void*           sdl1_evtfnc = NULL;
int             sdl1_evtautofree = 0;
int             sdl1_evtinside = 0;


void* getSDL1My(library_t* lib)
{
    sdl1_my_t* my = (sdl1_my_t*)calloc(1, sizeof(sdl1_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO

    return my;
}
#undef SUPER

void freeSDL1My(void* lib)
{
    //sdl1_my_t *my = (sdl1_my_t *)lib;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// AudioCallback ...
#define GO(A)   \
static uintptr_t my_AudioCallback_fct_##A = 0;                                      \
static void my_AudioCallback_##A(void *userdata, uint8_t *stream, int32_t len)      \
{                                                                                   \
    RunFunction(my_context, my_AudioCallback_fct_##A, 3, userdata, stream, len);    \
}
SUPER()
#undef GO
static void* find_AudioCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AudioCallback_fct_##A == (uintptr_t)fct) return my_AudioCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AudioCallback_fct_##A == 0) {my_AudioCallback_fct_##A = (uintptr_t)fct; return my_AudioCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1 AudioCallback callback\n");
    return NULL;
}
// TimerCallback ...
#define GO(A)   \
static uintptr_t my_TimerCallback_fct_##A = 0;                                                  \
static uint32_t my_TimerCallback_##A(uint32_t interval, void *userdata)                         \
{                                                                                               \
    return (uint32_t)RunFunction(my_context, my_TimerCallback_fct_##A, 2, interval, userdata);  \
}
SUPER()
#undef GO
static void* find_TimerCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_TimerCallback_fct_##A == (uintptr_t)fct) return my_TimerCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TimerCallback_fct_##A == 0) {my_TimerCallback_fct_##A = (uintptr_t)fct; return my_TimerCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1 TimerCallback callback\n");
    return NULL;
}
// EvtFilter ...
#define GO(A)   \
static uintptr_t my_EvtFilter_fct_##A = 0;                      \
static int my_EvtFilter_##A(void* p)                            \
{                                                               \
    return RunFunction(my_context, my_EvtFilter_fct_##A, 1, p); \
}
SUPER()
#undef GO
static void* find_EvtFilter_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_EvtFilter_fct_##A == (uintptr_t)fct) return my_EvtFilter_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EvtFilter_fct_##A == 0) {my_EvtFilter_fct_##A = (uintptr_t)fct; return my_EvtFilter_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1 EvtFilter callback\n");
    return NULL;
}
static void* reverse_EvtFilterFct(void* fct)
{
    if(!fct) return fct;
    library_t* my_lib = my_context->sdl1lib;
    if(CheckBridged(my_lib->priv.w.bridge, fct))
        return (void*)CheckBridged(my_lib->priv.w.bridge, fct);
    #define GO(A) if(my_EvtFilter_##A == fct) return (void*)my_EvtFilter_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->priv.w.bridge, iFp, fct, 0, NULL);
}
#undef SUPER

// TODO: track the memory for those callback
int EXPORT my_SDL_OpenAudio(x64emu_t* emu, void* d, void* o)
{
    SDL_AudioSpec *desired = (SDL_AudioSpec*)d;
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    // create a callback
    void *fnc = (void*)desired->callback;
    desired->callback = find_AudioCallback_Fct(desired->callback);
    int ret = my->SDL_OpenAudio(desired, (SDL_AudioSpec*)o);
    if (ret!=0) {
        // error, clean the callback...
        desired->callback = fnc;
        return ret;
    }
    // put back stuff in place?
    desired->callback = fnc;

    return ret;
}

void EXPORT *my_SDL_LoadBMP_RW(x64emu_t* emu, void* a, int b)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->SDL_LoadBMP_RW(rw, b);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}
int32_t EXPORT my_SDL_SaveBMP_RW(x64emu_t* emu, void* a, void* b, int c)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    int32_t r = my->SDL_SaveBMP_RW(rw, b, c);
    if(c==0)
        RWNativeEnd(rw);
    return r;
}
void EXPORT *my_SDL_LoadWAV_RW(x64emu_t* emu, void* a, int b, void* c, void* d, void* e)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->SDL_LoadWAV_RW(rw, b, c, d, e);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_ReadBE16(x64emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_ReadBE16(rw);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_ReadBE32(x64emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_ReadBE32(rw);
    RWNativeEnd(rw);
    return r;
}
uint64_t EXPORT my_SDL_ReadBE64(x64emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint64_t r = my->SDL_ReadBE64(rw);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_ReadLE16(x64emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_ReadLE16(rw);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_ReadLE32(x64emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_ReadLE32(rw);
    RWNativeEnd(rw);
    return r;
}
uint64_t EXPORT my_SDL_ReadLE64(x64emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint64_t r = my->SDL_ReadLE64(rw);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_WriteBE16(x64emu_t* emu, void* a, uint16_t v)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_WriteBE16(rw, v);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_WriteBE32(x64emu_t* emu, void* a, uint32_t v)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_WriteBE32(rw, v);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_WriteBE64(x64emu_t* emu, void* a, uint64_t v)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_WriteBE64(rw, v);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_WriteLE16(x64emu_t* emu, void* a, uint16_t v)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_WriteLE16(rw, v);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_WriteLE32(x64emu_t* emu, void* a, uint32_t v)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_WriteLE32(rw, v);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_WriteLE64(x64emu_t* emu, void* a, uint64_t v)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_WriteLE64(rw, v);
    RWNativeEnd(rw);
    return r;
}

// SDL1 doesn't really used rw_ops->type, but box64 does, so set sensible value (from SDL2)....
void EXPORT *my_SDL_RWFromConstMem(x64emu_t* emu, void* a, int b)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* r = (SDL1_RWops_t*)my->SDL_RWFromConstMem(a, b);
    RWSetType(r, 5);
    return AddNativeRW(emu, r);
}
void EXPORT *my_SDL_RWFromFP(x64emu_t* emu, void* a, int b)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* r = (SDL1_RWops_t*)my->SDL_RWFromFP(a, b);
    RWSetType(r, 2);
    return AddNativeRW(emu, r);
}
void EXPORT *my_SDL_RWFromFile(x64emu_t* emu, void* a, void* b)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* r = (SDL1_RWops_t*)my->SDL_RWFromFile(a, b);
    RWSetType(r, 2);
    return AddNativeRW(emu, r);
}
void EXPORT *my_SDL_RWFromMem(x64emu_t* emu, void* a, int b)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL1_RWops_t* r = (SDL1_RWops_t*)my->SDL_RWFromMem(a, b);
    RWSetType(r, 4);
    return AddNativeRW(emu, r);
}

uint32_t EXPORT my_SDL_AddTimer(x64emu_t* emu, uint32_t a, void* cb, void* p)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    return my->SDL_AddTimer(a, find_TimerCallback_Fct(cb), p);
}

void EXPORT my_SDL_RemoveTimer(x64emu_t* emu, uint32_t t)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    my->SDL_RemoveTimer(t);
}

int32_t EXPORT my_SDL_SetTimer(x64emu_t* emu, uint32_t t, void* p)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    return my->SDL_SetTimer(t, find_TimerCallback_Fct(p));
}
#if 0
int32_t EXPORT my_SDL_BuildAudioCVT(x64emu_t* emu, void* a, uint32_t b, uint32_t c, int32_t d, uint32_t e, uint32_t f, int32_t g)
{
    printf_log(LOG_NONE, "Error, using Unimplemented SDL1 SDL_BuildAudioCVT\n");
    emu->quit = 1;
    return 0;
}

int32_t EXPORT my_SDL_ConvertAudio(x64emu_t* emu, void* a)
{
    printf_log(LOG_NONE, "Error, using Unimplemented SDL1 SDL_ConvertAudio\n");
    emu->quit = 1;
    return 0;
}
#endif
void EXPORT my_SDL_SetEventFilter(x64emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    my->SDL_SetEventFilter(find_EvtFilter_Fct(a));
}
void EXPORT *my_SDL_GetEventFilter(x64emu_t* emu)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    return reverse_EvtFilterFct(my->SDL_GetEventFilter());
}

void EXPORT *my_SDL_CreateThread(x64emu_t* emu, void* cb, void* p)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    void* et = NULL;
    void* fnc = my_prepare_thread(emu, cb, p, 0, &et);
    return my->SDL_CreateThread(fnc, et);
}

void EXPORT my_SDL_KillThread(x64emu_t* emu, void* p)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    my->SDL_KillThread(p);
}

void fillGLProcWrapper(box64context_t* context);
EXPORT void* my_SDL_GL_GetProcAddress(x64emu_t* emu, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    printf_log(LOG_DEBUG, "Calling SDL_GL_GetProcAddress(%s)\n", rname);
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    // check if glxprocaddress is filled, and search for lib and fill it if needed
    if(!emu->context->glwrappers)
        fillGLProcWrapper(emu->context);
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
    if(!symbol)
        return NULL;    // easy
    // check if alread bridged
    uintptr_t ret = CheckBridged(emu->context->system, symbol);
    if(ret)
        return (void*)ret; // already bridged
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
        printf_log(LOG_INFO, "Warning, no wrapper for %s\n", rname);
        return NULL;
    }
    AddOffsetSymbol(emu->context->maplib, symbol, rname);
    const char* constname = kh_key(emu->context->glwrappers, k);
    return (void*)AddBridge(emu->context->system, kh_value(emu->context->glwrappers, k), symbol, 0, constname);
}

// DL functions from wrappedlibdl.c
void* my_dlopen(x64emu_t* emu, void *filename, int flag);
int my_dlclose(x64emu_t* emu, void *handle);
void* my_dlsym(x64emu_t* emu, void *handle, void *symbol);
EXPORT void* my_SDL_LoadObject(x64emu_t* emu, void* sofile)
{
    return my_dlopen(emu, sofile, 0);   // TODO: check correct flag value...
}
EXPORT void my_SDL_UnloadObject(x64emu_t* emu, void* handle)
{
    my_dlclose(emu, handle);
}
EXPORT void* my_SDL_LoadFunction(x64emu_t* emu, void* handle, void* name)
{
    return my_dlsym(emu, handle, name);
}

typedef struct my_SDL_version {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} my_SDL_version;

typedef struct {
  my_SDL_version version;
  int subsystem;
  union {
    struct {
      void* display;
      void* window;
      void (*lock_func)(void);
      void (*unlock_func)(void);
      void* fswindow;
      void* wmwindow;
      void* gfxdisplay;
    } x11;
  } info;
} my_SDL_SysWMinfo;

EXPORT int32_t my_SDL_GetWMInfo(x64emu_t* emu, void* p)
{
    // does SDL_SysWMinfo needs alignment?
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    int ret = my->SDL_GetWMInfo(p);
    my_SDL_SysWMinfo *info = (my_SDL_SysWMinfo*)p;
    if(info->info.x11.lock_func)
        info->info.x11.lock_func = (void*)AddBridge(emu->context->system, vFv, info->info.x11.lock_func, 0, NULL);
    if(info->info.x11.unlock_func)
        info->info.x11.unlock_func = (void*)AddBridge(emu->context->system, vFv, info->info.x11.unlock_func, 0, NULL);
    return ret;
}

#define CUSTOM_INIT \
    box64->sdl1lib = lib; \
    lib->priv.w.p2 = getSDL1My(lib); \
    box64->sdl1allocrw = ((sdl1_my_t*)lib->priv.w.p2)->SDL_AllocRW; \
    box64->sdl1freerw  = ((sdl1_my_t*)lib->priv.w.p2)->SDL_FreeRW; \
    lib->priv.w.needed = 3; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libm.so.6"); \
    lib->priv.w.neededlibs[1] = strdup("libdl.so.2"); \
    lib->priv.w.neededlibs[2] = strdup("librt.so.1");

#define CUSTOM_FINI \
    ((sdl1_my_t *)lib->priv.w.p2)->SDL_Quit();              \
    freeSDL1My(lib->priv.w.p2);                             \
    free(lib->priv.w.p2);                                   \
    ((box64context_t*)(lib->context))->sdl1lib = NULL;      \
    ((box64context_t*)(lib->context))->sdl1allocrw = NULL;  \
    ((box64context_t*)(lib->context))->sdl1freerw = NULL;

#include "wrappedlib_init.h"

