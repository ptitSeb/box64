#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper32.h"
#include "debug.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "callback.h"
#include "librarian.h"
#include "emu/x64emu_private.h"
#include "box32context.h"
#include "sdl1rwops.h"
#include "gltools.h"

#include "x64trace.h"
#include "threads.h"

#include "sdl1align32.h"

extern const char* sdl1Name;
#define LIBNAME sdl1

static int sdl_Yes() {return 1;}
static int sdl_No() {return 0;}
EXPORT int my32_SDL_Has3DNow() __attribute__((alias("sdl_No")));
EXPORT int my32_SDL_Has3DNowExt() __attribute__((alias("sdl_No")));
EXPORT int my32_SDL_HasAltiVec() __attribute__((alias("sdl_No")));
EXPORT int my32_SDL_HasMMX() __attribute__((alias("sdl_Yes")));
EXPORT int my32_SDL_HasMMXExt() __attribute__((alias("sdl_Yes")));
EXPORT int my32_SDL_HasRDTSC() __attribute__((alias("sdl_Yes")));
EXPORT int my32_SDL_HasSSE() __attribute__((alias("sdl_Yes")));
EXPORT int my32_SDL_HasSSE2() __attribute__((alias("sdl_Yes")));

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

typedef struct {    // removed packed attribute
  int32_t freq;
  uint16_t format;
  uint8_t channels;
  uint8_t silence;
  uint16_t samples;
  uint32_t size;
  ptr_t callback; //void (*callback)(void *userdata, uint8_t *stream, int32_t len);
  ptr_t userdata; //void *userdata;
} SDL_AudioSpec32;

EXPORT void my32_SDL_Quit();

#define ADDED_FUNCTIONS() \
    GO(SDL_AllocRW, sdl1_allocrw) \
    GO(SDL_FreeRW, sdl1_freerw)

#define ADDED_FINI() \
    my32_SDL_Quit();

#include "generated/wrappedsdl1types32.h"

#include "wrappercallback32.h"

// event filter. Needs to be global, but there is only one, so that's should be fine
static x64emu_t        *sdl1_evtfilter = NULL;
static void*           sdl1_evtfnc = NULL;
static int             sdl1_evtautofree = 0;
static int             sdl1_evtinside = 0;

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// AudioCallback ...
#define GO(A)   \
static uintptr_t my32_AudioCallback_fct_##A = 0;                                      \
static void my32_AudioCallback_##A(void *userdata, uint8_t *stream, int32_t len)      \
{                                                                                   \
    RunFunctionFmt(my32_AudioCallback_fct_##A, "ppi", userdata, stream, len);   \
}
SUPER()
#undef GO
static void* find_AudioCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_AudioCallback_fct_##A == (uintptr_t)fct) return my32_AudioCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_AudioCallback_fct_##A == 0) {my32_AudioCallback_fct_##A = (uintptr_t)fct; return my32_AudioCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1 AudioCallback callback\n");
    return NULL;
}
// TimerCallback ...
#define GO(A)   \
static uintptr_t my32_TimerCallback_fct_##A = 0;                                                  \
static uint32_t my32_TimerCallback_##A(uint32_t interval, void *userdata)                         \
{                                                                                               \
    return (uint32_t)RunFunctionFmt(my32_TimerCallback_fct_##A, "up", interval, userdata);  \
}
SUPER()
#undef GO
static void* find_TimerCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_TimerCallback_fct_##A == (uintptr_t)fct) return my32_TimerCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_TimerCallback_fct_##A == 0) {my32_TimerCallback_fct_##A = (uintptr_t)fct; return my32_TimerCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1 TimerCallback callback\n");
    return NULL;
}
// EvtFilter ...
#define GO(A)   \
static uintptr_t my32_EvtFilter_fct_##A = 0;                      \
static int my32_EvtFilter_##A(void* p)                            \
{                                                               \
    return RunFunctionFmt(my32_EvtFilter_fct_##A, "p", p); \
}
SUPER()
#undef GO
static void* find_EvtFilter_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_EvtFilter_fct_##A == (uintptr_t)fct) return my32_EvtFilter_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_EvtFilter_fct_##A == 0) {my32_EvtFilter_fct_##A = (uintptr_t)fct; return my32_EvtFilter_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1 EvtFilter callback\n");
    return NULL;
}
static void* reverse_EvtFilterFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my32_EvtFilter_##A == fct) return (void*)my32_EvtFilter_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, iFp_32, fct, 0, NULL);
}
#undef SUPER

// TODO: track the memory for those callback
EXPORT int my32_SDL_OpenAudio(x64emu_t* emu, void* d, void* o)
{
    SDL_AudioSpec desired = {0};
    SDL_AudioSpec output = {0};
    SDL_AudioSpec32 *d_ = d;
    desired.channels = d_->channels;
    desired.format = d_->format;
    desired.freq = d_->freq;
    desired.samples = d_->samples;
    desired.silence = d_->silence;
    desired.size = d_->size;
    desired.userdata = from_ptrv(d_->userdata);
    desired.callback = find_AudioCallback_Fct(from_ptrv(d_->callback));
    int ret = my->SDL_OpenAudio(&desired, &output);
    if (ret!=0) {
        return ret;
    }
    // put back stuff in place?
    if (o) {
        SDL_AudioSpec32* o_ = o;
        o_->channels = output.channels;
        o_->format = output.format;
        o_->freq = output.freq;
        o_->samples = output.samples;
        o_->silence = output.silence;
        o_->size = output.size;
        o_->userdata = o_->callback = 0;
    }

    return ret;
}

EXPORT void *my32_SDL_LoadBMP_RW(x64emu_t* emu, void* a, int b)
{
    inplace_SDL_RWops_to_64(a);
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->SDL_LoadBMP_RW(rw, b);
    if(b==0) {
        RWNativeEnd(rw);
        inplace_SDL_RWops_to_32(a);
    }
    inplace_SDL_Surface_to_32(r);
    return r;
}
//EXPORT int32_t my32_SDL_SaveBMP_RW(x64emu_t* emu, void* a, void* b, int c)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    int32_t r = my->SDL_SaveBMP_RW(rw, b, c);
//    if(c==0)
//        RWNativeEnd(rw);
//    return r;
//}
//EXPORT void *my32_SDL_LoadWAV_RW(x64emu_t* emu, void* a, int b, void* c, void* d, void* e)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    void* r = my->SDL_LoadWAV_RW(rw, b, c, d, e);
//    if(b==0)
//        RWNativeEnd(rw);
//    return r;
//}
//EXPORT uint32_t my32_SDL_ReadBE16(x64emu_t* emu, void* a)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    uint32_t r = my->SDL_ReadBE16(rw);
//    RWNativeEnd(rw);
//    return r;
//}
//EXPORT uint32_t my32_SDL_ReadBE32(x64emu_t* emu, void* a)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    uint32_t r = my->SDL_ReadBE32(rw);
//    RWNativeEnd(rw);
//    return r;
//}
//EXPORT uint64_t my32_SDL_ReadBE64(x64emu_t* emu, void* a)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    uint64_t r = my->SDL_ReadBE64(rw);
//    RWNativeEnd(rw);
//    return r;
//}
//EXPORT uint32_t my32_SDL_ReadLE16(x64emu_t* emu, void* a)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    uint32_t r = my->SDL_ReadLE16(rw);
//    RWNativeEnd(rw);
//    return r;
//}
EXPORT uint32_t my32_SDL_ReadLE32(x64emu_t* emu, void* a)
{
    inplace_SDL_RWops_to_64(a);
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_ReadLE32(rw);
    RWNativeEnd(rw);
    inplace_SDL_RWops_to_32(a);
    return r;
}
//EXPORT uint64_t my32_SDL_ReadLE64(x64emu_t* emu, void* a)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    uint64_t r = my->SDL_ReadLE64(rw);
//    RWNativeEnd(rw);
//    return r;
//}
//EXPORT uint32_t my32_SDL_WriteBE16(x64emu_t* emu, void* a, uint16_t v)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    uint32_t r = my->SDL_WriteBE16(rw, v);
//    RWNativeEnd(rw);
//    return r;
//}
//EXPORT uint32_t my32_SDL_WriteBE32(x64emu_t* emu, void* a, uint32_t v)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    uint32_t r = my->SDL_WriteBE32(rw, v);
//    RWNativeEnd(rw);
//    return r;
//}
//EXPORT uint32_t my32_SDL_WriteBE64(x64emu_t* emu, void* a, uint64_t v)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    uint32_t r = my->SDL_WriteBE64(rw, v);
//    RWNativeEnd(rw);
//    return r;
//}
//EXPORT uint32_t my32_SDL_WriteLE16(x64emu_t* emu, void* a, uint16_t v)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    uint32_t r = my->SDL_WriteLE16(rw, v);
//    RWNativeEnd(rw);
//    return r;
//}
//EXPORT uint32_t my32_SDL_WriteLE32(x64emu_t* emu, void* a, uint32_t v)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    uint32_t r = my->SDL_WriteLE32(rw, v);
//    RWNativeEnd(rw);
//    return r;
//}
//EXPORT uint32_t my32_SDL_WriteLE64(x64emu_t* emu, void* a, uint64_t v)
//{
//    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
//    uint32_t r = my->SDL_WriteLE64(rw, v);
//    RWNativeEnd(rw);
//    return r;
//}

// SDL1 doesn't really used rw_ops->type, but box64 does, so set sensible value (from SDL2)....
//EXPORT void *my32_SDL_RWFromConstMem(x64emu_t* emu, void* a, int b)
//{
//    SDL1_RWops_t* r = (SDL1_RWops_t*)my->SDL_RWFromConstMem(a, b);
//    RWSetType(r, 5);
//    return AddNativeRW(emu, r);
//}
//EXPORT void *my32_SDL_RWFromFP(x64emu_t* emu, void* a, int b)
//{
//    SDL1_RWops_t* r = (SDL1_RWops_t*)my->SDL_RWFromFP(a, b);
//    RWSetType(r, 2);
//    return AddNativeRW(emu, r);
//}
EXPORT void *my32_SDL_RWFromFile(x64emu_t* emu, void* a, void* b)
{
    SDL1_RWops_t* r = (SDL1_RWops_t*)my->SDL_RWFromFile(a, b);
    RWSetType(r, 2);
    void* ret = AddNativeRW(emu, r);
    inplace_SDL_RWops_to_32(ret);
    return ret;
}
EXPORT void *my32_SDL_RWFromMem(x64emu_t* emu, void* a, int b)
{
    SDL1_RWops_t* r = (SDL1_RWops_t*)my->SDL_RWFromMem(a, b);
    RWSetType(r, 4);
    void* ret = AddNativeRW(emu, r);
    inplace_SDL_RWops_to_32(ret);
    return ret;
}

EXPORT void my32_SDL_WM_SetIcon(void* s, void* mask)
{
    inplace_SDL_Surface_to_64(s);
    my->SDL_WM_SetIcon(s, mask);
    inplace_SDL_Surface_to_32(s);
}

EXPORT void my32_SDL_GetRGB(uint32_t pixel, void* format, void* r, void* g, void* b)
{
    inplace_SDL_PixelFormat_to_64(format);
    my->SDL_GetRGB(pixel, format, r, g, b);
    inplace_SDL_PixelFormat_to_32(format);
}

EXPORT void* my32_SDL_GetVideoInfo()
{
    static my_SDL_Palette_t vm_palette;
    static my_SDL_PixelFormat_t vm_format;
    static my_SDL_VideoInfo_32_t vm = {0};
    my_SDL_VideoInfo_t* r = my->SDL_GetVideoInfo();
    if(!r) return NULL;
    vm.hw_available = r->hw_available;
    vm.wm_available = r->wm_available;
    vm.blit_hw = r->blit_hw;
    vm.blit_hw_CC = r->blit_hw_CC;
    vm.blit_hw_A = r->blit_hw_A;
    vm.blit_sw = r->blit_sw;
    vm.blit_sw_CC = r->blit_sw_CC;
    vm.blit_sw_A = r->blit_sw_A;
    vm.blit_fill = r->blit_fill;
    vm.video_mem = r->video_mem;
    vm.current_h = r->current_h;
    vm.current_w = r->current_w;
    if(r->vfmt) {
        vm.vfmt = to_ptrv(&vm_format);
        memcpy(&vm_format, r->vfmt, sizeof(vm_format));
        if(r->vfmt->palette) {
            vm_format.palette = &vm_palette;
            memcpy(&vm_palette, r->vfmt->palette, sizeof(vm_palette));
        }
        inplace_SDL_PixelFormat_to_32(&vm_format);
    } else vm.vfmt = 0;
    return &vm;
}

EXPORT void *my32_SDL_AddTimer(x64emu_t* emu, uint32_t a, void* cb, void* p)
{
    return my->SDL_AddTimer(a, find_TimerCallback_Fct(cb), p);
}

EXPORT int my32_SDL_RemoveTimer(x64emu_t* emu, void *t)
{
    return my->SDL_RemoveTimer(t);
}

EXPORT int32_t my32_SDL_SetTimer(x64emu_t* emu, uint32_t t, void* p)
{
    return my->SDL_SetTimer(t, find_TimerCallback_Fct(p));
}
#if 0
EXPORT int32_t my32_SDL_BuildAudioCVT(x64emu_t* emu, void* a, uint32_t b, uint32_t c, int32_t d, uint32_t e, uint32_t f, int32_t g)
{
    printf_log(LOG_NONE, "Error, using Unimplemented SDL1 SDL_BuildAudioCVT\n");
    emu->quit = 1;
    return 0;
}

EXPORT int32_t my32_SDL_ConvertAudio(x64emu_t* emu, void* a)
{
    printf_log(LOG_NONE, "Error, using Unimplemented SDL1 SDL_ConvertAudio\n");
    emu->quit = 1;
    return 0;
}
#endif
//EXPORT void my32_SDL_SetEventFilter(x64emu_t* emu, void* a)
//{
//    my->SDL_SetEventFilter(find_EvtFilter_Fct(a));
//}
//EXPORT void *my32_SDL_GetEventFilter(x64emu_t* emu)
//{
//    return reverse_EvtFilterFct(my->SDL_GetEventFilter());
//}
void* my32_prepare_thread(x64emu_t *emu, void* f, void* arg, int ssize, void** pet);
EXPORT void *my32_SDL_CreateThread(x64emu_t* emu, void* cb, void* p)
{
    void* et = NULL;
    void* fnc = my32_prepare_thread(emu, cb, p, 0, &et);
    if(!fnc)
        return NULL;
    return my->SDL_CreateThread(fnc, et);
}

EXPORT void my32_SDL_KillThread(x64emu_t* emu, void* p)
{
    my->SDL_KillThread(p);
}

EXPORT void* my32_SDL_GL_GetProcAddress(x64emu_t* emu, void* name)
{
    khint_t k;
    const char* rname = (const char*)name;
    return getGLProcAddress32(emu, NULL, (glprocaddress_t)my->SDL_GL_GetProcAddress, rname);
}

// DL functions from wrappedlibdl.c
void* my_dlopen(x64emu_t* emu, void *filename, int flag);
int my_dlclose(x64emu_t* emu, void *handle);
void* my_dlsym(x64emu_t* emu, void *handle, void *symbol);
EXPORT void* my32_SDL_LoadObject(x64emu_t* emu, void* sofile)
{
    return my_dlopen(emu, sofile, 0);   // TODO: check correct flag value...
}
EXPORT void my32_SDL_UnloadObject(x64emu_t* emu, void* handle)
{
    my_dlclose(emu, handle);
}
EXPORT void* my32_SDL_LoadFunction(x64emu_t* emu, void* handle, void* name)
{
    return my_dlsym(emu, handle, name);
}

static my_SDL_Surface_t* sdl1_videomode_org = NULL;
static my_SDL_Palette_t sdl_vm_palette;
static my_SDL_PixelFormat_t sdl_vm_pixelformal;
static my_SDL_Surface_t sdl_vm_surface;
static void* wrapSurface(void* s)
{
    if(!s) return s;
    if(s==&sdl_vm_surface) {
        my_SDL_Surface_32_t* src = s;
        // refressh surface...
        sdl1_videomode_org->h = src->h;
        sdl1_videomode_org->w = src->w;
        sdl1_videomode_org->pitch = src->pitch;
        sdl1_videomode_org->flags = src->flags;
        sdl1_videomode_org->locked = src->locked;
        sdl1_videomode_org->refcount = src->refcount;
        sdl1_videomode_org->offset = src->offset;
        sdl1_videomode_org->unused1 = src->unused1;
        sdl1_videomode_org->format_version = src->format_version;
        sdl1_videomode_org->pixels = from_ptrv(src->pixels);
        sdl1_videomode_org->map = from_ptrv(src->map);
        sdl1_videomode_org->clip_rect.x = src->clip_rect.x;
        sdl1_videomode_org->clip_rect.y = src->clip_rect.y;
        sdl1_videomode_org->clip_rect.h = src->clip_rect.h;
        sdl1_videomode_org->clip_rect.w = src->clip_rect.w;
        return sdl1_videomode_org;
    }
    inplace_SDL_Surface_to_64(s);
    return s;
}
static void* unwrapSurface(void* s)
{
    if(!s) return s;
    if(s==&sdl_vm_surface || s==sdl1_videomode_org) {
        my_SDL_Surface_32_t* dst = (my_SDL_Surface_32_t*)&sdl_vm_surface;
        // refressh surface...
        dst->h = sdl1_videomode_org->h;
        dst->w = sdl1_videomode_org->w;
        dst->pitch = sdl1_videomode_org->pitch;
        dst->flags = sdl1_videomode_org->flags;
        dst->locked = sdl1_videomode_org->locked;
        dst->refcount = sdl1_videomode_org->refcount;
        dst->offset = sdl1_videomode_org->offset;
        dst->unused1 = sdl1_videomode_org->unused1;
        dst->format_version = sdl1_videomode_org->format_version;
        dst->pixels = to_ptrv(sdl1_videomode_org->pixels);
        dst->map = to_ptrv(sdl1_videomode_org->map);
        dst->clip_rect.x = sdl1_videomode_org->clip_rect.x;
        dst->clip_rect.y = sdl1_videomode_org->clip_rect.y;
        dst->clip_rect.h = sdl1_videomode_org->clip_rect.h;
        dst->clip_rect.w = sdl1_videomode_org->clip_rect.w;
        return s;
    }
    inplace_SDL_Surface_to_32(s);
    return s;
}

EXPORT void my32_SDL_Quit()
{
    sdl1_videomode_org = NULL;
    my->SDL_Quit();
}

EXPORT void*  my32_SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags)
{
    my_SDL_Surface_t* ret = my->SDL_SetVideoMode(width, height, bpp, flags);
    if (!ret) return ret;
    sdl1_videomode_org = ret;
    memcpy(&sdl_vm_surface, ret, sizeof(sdl_vm_surface));
    if(ret->format) {
        memcpy(&sdl_vm_pixelformal, ret->format, sizeof(sdl_vm_pixelformal));
        if(ret->format->palette) {
            memcpy(&sdl_vm_palette, ret->format->palette, sizeof(sdl_vm_palette));
            sdl_vm_pixelformal.palette = &sdl_vm_palette;
        }
        sdl_vm_surface.format = &sdl_vm_pixelformal;
    }
    inplace_SDL_Surface_to_32(&sdl_vm_surface);
    return &sdl_vm_surface;
}

EXPORT int my32_SDL_LockSurface(void* s)
{
    int ret = my->SDL_LockSurface(wrapSurface(s));
    unwrapSurface(s);
    return ret;
}

EXPORT void my32_SDL_UnlockSurface(void* s)
{
    my->SDL_UnlockSurface(wrapSurface(s));
    unwrapSurface(s);
}

EXPORT int my32_SDL_Flip(void* s)
{
    int ret = my->SDL_Flip(wrapSurface(s));
    unwrapSurface(s);
    return ret;
}

EXPORT void* my32_SDL_GetVideoSurface()
{
    void* ret = my->SDL_GetVideoSurface();
    return unwrapSurface(ret);
}

EXPORT void* my32_SDL_CreateRGBSurfaceFrom(void* pixels, int width, int height, int depth, int pitch, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask)
{
    void* p = my->SDL_CreateRGBSurfaceFrom(pixels, width, height, depth, pitch, rmask, gmask, bmask, amask);
    return unwrapSurface(p);
}


EXPORT void my32_SDL_FreeSurface(void* s)
{
    my->SDL_FreeSurface(wrapSurface(s));
    if(s==&sdl_vm_surface)
        sdl1_videomode_org = NULL;
}

EXPORT int my32_SDL_SetColorKey(void* s, uint32_t flags, uint32_t key)
{
    int ret = my->SDL_SetColorKey(wrapSurface(s), flags, key);
    unwrapSurface(s);
    return ret;
}

EXPORT int my32_SDL_PollEvent(my_SDL_Event_32_t* evt)
{
    my_SDL_Event_t event;
    int ret = my->SDL_PollEvent(evt?(&event):NULL);
    if(ret && evt) {
        convert_SDL_Event_to_32(evt, &event);
    }
    return ret;
}

EXPORT int my32_SDL_PeepEvents(my_SDL_Event_32_t* evts, int num, uint32_t action, uint32_t mask)
{
    my_SDL_Event_t events[num];
    int ret = my->SDL_PeepEvents(events, num, action, mask);
    for(int i=0; i<ret; ++i) convert_SDL_Event_to_32(evts+i, events+i);
    return ret;
}

EXPORT int my32_SDL_PushEvent(my_SDL_Event_32_t* evt)
{
    my_SDL_Event_t event;
    convert_SDL_Event_to_64(&event, evt);
    return my->SDL_PushEvent(&event);
}

EXPORT void* my32_SDL_ListModes(my_SDL_PixelFormat_32_t* fmt, uint32_t flags)
{
    my_SDL_PixelFormat_t format;
    my_SDL_Palette_t palette;
    if(fmt) {
        memcpy(&format, fmt, sizeof(format));
        inplace_SDL_PixelFormat_to_64_nopalette(&format);
        if(fmt->palette) {
            memcpy(&palette, from_ptrv(fmt->palette), sizeof(palette));
            format.palette = &palette;
            inplace_SDL_Palette_to_64(&palette);
        }
    }
    void** ret = my->SDL_ListModes(fmt?(&format):NULL, flags);
    if(!ret)
        return ret;
    if(ret==(void**)-1LL)
        return ret;
    static ptr_t available[256];
    void** p = ret;
    int idx = 0;
    while((*p) && (idx<255)) {
        available[idx++] = to_ptrv(*p);
        ++p;
    }
    available[idx++] = 0;
    return &available;
}

EXPORT uint32_t my32_SDL_MapRGB(my_SDL_PixelFormat_32_t* fmt, uint8_t r, uint8_t g, uint8_t b)
{
    my_SDL_PixelFormat_t format;
    my_SDL_Palette_t palette;
    if(fmt) {
        memcpy(&format, fmt, sizeof(format));
        inplace_SDL_PixelFormat_to_64_nopalette(&format);
        if(fmt->palette) {
            memcpy(&palette, from_ptrv(fmt->palette), sizeof(palette));
            format.palette = &palette;
            inplace_SDL_Palette_to_64(&palette);
        }
    }
    return my->SDL_MapRGB(fmt?(&format):NULL, r, g, b);
}

EXPORT int my32_SDL_BuildAudioCVT(my_SDL_AudioCVT_32_t* cvt, uint16_t src_fmt, uint16_t src_chn, int src_rate, uint16_t dst_fmt, uint16_t dst_ch, int dst_rate)
{
    my_SDL_AudioCVT_t l_cvt;
    int ret = my->SDL_BuildAudioCVT(&l_cvt, src_fmt, src_chn, src_rate, dst_fmt, dst_ch, dst_rate);
    if(ret!=-1)
        convert_AudioCVT_to_32(cvt, &l_cvt);
    return ret;
}

EXPORT int my32_SDL_ConvertAudio(my_SDL_AudioCVT_32_t* cvt)
{
    my_SDL_AudioCVT_t l_cvt;
    convert_AudioCVT_to_64(&l_cvt, cvt);
    int ret = my->SDL_ConvertAudio(&l_cvt);
    convert_AudioCVT_to_32(cvt, &l_cvt);
    return ret;
}

//EXPORT int32_t my32_SDL_GetWMInfo(x64emu_t* emu, void* p)
//{
//    // does SDL_SysWMinfo needs alignment?
//    int ret = my->SDL_GetWMInfo(p);
//    my32_SDL_SysWMinfo *info = (my32_SDL_SysWMinfo*)p;
//    if(info->info.x11.lock_func)
//        info->info.x11.lock_func = (void*)AddCheckBridge(emu->context->system, vFv_32, info->info.x11.lock_func, 0, NULL);
//    if(info->info.x11.unlock_func)
//        info->info.x11.unlock_func = (void*)AddCheckBridge(emu->context->system, vFv_32, info->info.x11.unlock_func, 0, NULL);
//    return ret;
//}

#define CUSTOM_INIT \
    box64->sdl1allocrw = my->SDL_AllocRW;   \
    box64->sdl1freerw  = my->SDL_FreeRW;

#define NEEDED_LIBS "libm.so.6", "libdl.so.2", "librt.so.1"

#define CUSTOM_FINI \
    my_context->sdl1allocrw = NULL;         \
    my_context->sdl1freerw = NULL;

#include "wrappedlib_init32.h"
