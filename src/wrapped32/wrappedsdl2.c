#include "sdl2align32.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <sys/mman.h>
#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box32context.h"
#include "emu/x64emu_private.h"
#include "myalign32.h"
#include "gltools.h"
#include "sdl2rwops.h"

#include "x64trace.h"
#include "threads.h"

#include "sdl2align32.h"

#include "generated/wrappedsdl2defs32.h"
#define JFEi_32 pFEpi_32

extern const char* sdl2Name;
#define LIBNAME sdl2

typedef struct {
    uint8_t data[16];
} SDL2_GUID_t;

typedef struct {
    int32_t freq;
    uint16_t format;
    uint8_t channels;
    uint8_t silence;
    uint16_t samples;
    uint16_t padding;
    uint32_t size;
    void (*callback)(void* userdata, uint8_t* stream, int32_t len);
    void* userdata;
} SDL2_AudioSpec;

typedef struct __attribute__((packed, aligned(4))) {
    int32_t freq;
    uint16_t format;
    uint8_t channels;
    uint8_t silence;
    uint16_t samples;
    uint16_t padding;
    uint32_t size;
    ptr_t callback; // void (*callback)(void *userdata, uint8_t *stream, int32_t len);
    ptr_t userdata; // void *userdata;
} SDL2_AudioSpec32;

typedef void (*vFiupV_t)(int, uint32_t, void*, va_list);
#define ADDED_FUNCTIONS()         \
    GO(SDL_LogMessageV, vFiupV_t) \
    GO(SDL_AllocRW, sdl2_allocrw) \
    GO(SDL_FreeRW, sdl2_freerw)

#define ADDED_FINI()

#include "generated/wrappedsdl2types32.h"
#include "wrappercallback32.h"

#define SUPER() \
    GO(0)       \
    GO(1)       \
    GO(2)       \
    GO(3)       \
    GO(4)


// eventfilter
#define GO(A)   \
static uintptr_t my_eventfilter_fct_##A = 0;                                            \
static my_SDL2_Event_32_t event_##A = {0};                                              \
static int my_eventfilter_##A(void* userdata, void* event)                              \
{                                                                                       \
    convert_SDL2_Event_to_32(&event_##A, event);                                        \
    int ret = (int)RunFunctionFmt(my_eventfilter_fct_##A, "pp", userdata, &event_##A);  \
    return ret;                                                                         \
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
    printf_log(LOG_NONE, "Warning, no more slot for 32bits SDL2 eventfilter callback\n");
    return NULL;

}
static void* reverse_eventfilter_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_eventfilter_##A == fct) return (void*)my_eventfilter_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, iFpp_32, fct, 0, NULL);
}
// AudioCallback
#define GO(A)                                                         \
    static uintptr_t my32_2_AudioCallback_fct_##A = 0;                \
    static void my32_2_AudioCallback_##A(void* a, void* b, int c)     \
    {                                                                 \
        RunFunctionFmt(my32_2_AudioCallback_fct_##A, "ppi", a, b, c); \
    }
SUPER()
#undef GO
static void* find_AudioCallback_Fct(void* fct)
{
    if (!fct) return NULL;
    void* p;
    if ((p = GetNativeFnc((uintptr_t)fct))) return p;
#define GO(A) \
    if (my32_2_AudioCallback_fct_##A == (uintptr_t)fct) return my32_2_AudioCallback_##A;
    SUPER()
#undef GO
#define GO(A)                                          \
    if (my32_2_AudioCallback_fct_##A == 0) {           \
        my32_2_AudioCallback_fct_##A = (uintptr_t)fct; \
        return my32_2_AudioCallback_##A;               \
    }
    SUPER()
#undef GO
    printf_log(LOG_NONE, "Warning, no more slot for 32bit SDL2 AudioCallback callback\n");
    return NULL;
}

EXPORT int my32_2_SDL_GetDesktopDisplayMode(int displayIndex, void* mode)
{
    my_SDL2_DisplayMode_t my_mode = { 0 };
    int ret = my->SDL_GetDesktopDisplayMode(displayIndex, &my_mode);
    convert_SDL2_DisplayMode_to_32(mode, &my_mode);
    return ret;
}

EXPORT int my32_2_SDL_GetCurrentDisplayMode(int displayIndex, void* mode)
{
    my_SDL2_DisplayMode_t my_mode = { 0 };
    int ret = my->SDL_GetCurrentDisplayMode(displayIndex, &my_mode);
    convert_SDL2_DisplayMode_to_32(mode, &my_mode);
    return ret;
}

EXPORT int my32_2_SDL_GetWindowDisplayMode(void* window, void* mode)
{
    my_SDL2_DisplayMode_t my_mode = { 0 };
    int ret = my->SDL_GetWindowDisplayMode(window, &my_mode);
    convert_SDL2_DisplayMode_to_32(mode, &my_mode);
    return ret;
}

EXPORT int my32_2_SDL_GetDisplayMode(int dispIndex, int modeIndex, void* mode)
{
    my_SDL2_DisplayMode_t mode_l = { 0 };
    int ret = my->SDL_GetDisplayMode(dispIndex, modeIndex, &mode_l);
    convert_SDL2_DisplayMode_to_32(mode, &mode_l);
    return ret;
}

EXPORT int my32_2_SDL_SetWindowDisplayMode(void* window, void* mode)
{
    my_SDL2_DisplayMode_t mode_l = { 0 };
    convert_SDL2_DisplayMode_to_64(&mode_l, mode);
    return my->SDL_SetWindowDisplayMode(window, &mode_l);
}

EXPORT void* my32_2_SDL_GetClosestDisplayMode(int index, void* mode, void* closest)
{
    my_SDL2_DisplayMode_t mode_l = { 0 };
    my_SDL2_DisplayMode_t closest_l = { 0 };
    convert_SDL2_DisplayMode_to_64(&mode_l, mode);
    void* ret = my->SDL_GetClosestDisplayMode(index, &mode_l, &closest_l);
    if(!ret) return NULL;
    convert_SDL2_DisplayMode_to_32(closest, &closest_l);
    return closest;
}

EXPORT void* my32_2_SDL_JoystickGetDeviceGUID(void* ret, int index)
{
   *(SDL2_GUID_t*)ret = my->SDL_JoystickGetDeviceGUID(index);
   return ret;
}

EXPORT void* my32_2_SDL_JoystickGetGUIDFromString(void* ret, void* s)
{
   *(SDL2_GUID_t*)ret = my->SDL_JoystickGetGUIDFromString(s);
   return ret;
}


EXPORT int my32_2_SDL_OpenAudio(x64emu_t* emu, void* d, void* o)
{
    SDL2_AudioSpec desired = { 0 };
    SDL2_AudioSpec output = { 0 };
    SDL2_AudioSpec32* d_ = d;
    desired.channels = d_->channels;
    desired.format = d_->format;
    desired.freq = d_->freq;
    desired.samples = d_->samples;
    desired.padding = d_->padding;
    desired.silence = d_->silence;
    desired.size = d_->size;
    desired.userdata = from_ptrv(d_->userdata);
    desired.callback = find_AudioCallback_Fct(from_ptrv(d_->callback));
    int ret = my->SDL_OpenAudio(&desired, &output);
    if (ret <= 0) {
        return ret;
    }

    if (o) {
        SDL2_AudioSpec32* o_ = o;
        o_->channels = output.channels;
        o_->format = output.format;
        o_->freq = output.freq;
        o_->samples = output.samples;
        o_->padding = output.padding;
        o_->silence = output.silence;
        o_->size = output.size;
        o_->userdata = to_ptrv(output.userdata);
        o_->callback = to_ptrv(output.callback);
    }
    return ret;
}

EXPORT uint32_t my32_2_SDL_OpenAudioDevice(x64emu_t* emu, void* device, int iscapture, void* d, void* o, int allowed)
{
    SDL2_AudioSpec desired = { 0 };
    SDL2_AudioSpec output = { 0 };
    SDL2_AudioSpec32* d_ = d;
    desired.channels = d_->channels;
    desired.format = d_->format;
    desired.freq = d_->freq;
    desired.samples = d_->samples;
    desired.padding = d_->padding;
    desired.silence = d_->silence;
    desired.size = d_->size;
    desired.userdata = from_ptrv(d_->userdata);
    desired.callback = find_AudioCallback_Fct(from_ptrv(d_->callback));
    uint32_t ret = my->SDL_OpenAudioDevice(device, iscapture, &desired, &output, allowed);

    if (o) {
        SDL2_AudioSpec32* o_ = o;
        o_->channels = output.channels;
        o_->format = output.format;
        o_->freq = output.freq;
        o_->samples = output.samples;
        o_->padding = output.padding;
        o_->silence = output.silence;
        o_->size = output.size;
        o_->userdata = to_ptrv(output.userdata);
        o_->callback = to_ptrv(output.callback);
    }
    return ret;
}


EXPORT int my32_2_SDL_PollEvent(my_SDL2_Event_32_t* evt)
{
    my_SDL2_Event_t event;
    int ret = my->SDL_PollEvent(evt ? (&event) : NULL);
    if (ret && evt) {
        convert_SDL2_Event_to_32(evt, &event);
    }
    return ret;
}

EXPORT int my32_2_SDL_PeepEvents(x64emu_t* emu, my_SDL2_Event_32_t* evt, int n, uint32_t action, uint32_t minType, uint32_t maxType)
{
    my_SDL2_Event_t event[n];
    int ret = my->SDL_PeepEvents(event, n, action, minType, maxType);
    if (ret>0) {
        for(int i=0; i<ret; ++i)
            convert_SDL2_Event_to_32(evt+i, event+i);
    }
    return ret;
}

EXPORT char* my32_2_SDL_GetBasePath(x64emu_t* emu)
{
    char* p = strdup(emu->context->fullpath);
    char* b = strrchr(p, '/');
    if (b) *(b + 1) = '\0';
    return p;
}

EXPORT void my32_2_SDL_Log(x64emu_t* emu, void* fmt, void* b)
{
    myStackAlign32((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST_32;
    my->SDL_LogMessageV(0, 3, fmt, VARARGS_32);
}

EXPORT int my32_2_SDL_vsnprintf(x64emu_t* emu, void* buff, size_t s, void* fmt, void* b)
{
    myStackAlign32((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST_32;
    int r = vsnprintf(buff, s, fmt, VARARGS_32);
    return r;
}

EXPORT int my32_2_SDL_snprintf(x64emu_t* emu, void* buff, size_t s, void* fmt, uint64_t* b)
{
    (void)emu;
    myStackAlign32((const char*)fmt, (uint32_t*)b, emu->scratch);
    PREPARE_VALIST_32;
    return vsnprintf(buff, s, fmt, VARARGS_32);
}

EXPORT void* my32_2_SDL_GL_GetProcAddress(x64emu_t* emu, void* name)
{
    khint_t k;
    const char* rname = (const char*)name;
    return getGLProcAddress32(emu, NULL, (glprocaddress_t)my->SDL_GL_GetProcAddress, rname);
}

EXPORT void* my32_2_SDL_CreateRGBSurfaceWithFormatFrom(x64emu_t* emu, void* pixels, int width, int height, int depth, int pitch, uint32_t format)
{
    void* p = my->SDL_CreateRGBSurfaceWithFormatFrom(pixels, width, height, depth, pitch, format);
    inplace_SDL2_Surface_to_32(p);
    return p;
}

EXPORT void* my32_2_SDL_CreateRGBSurface(x64emu_t* emu, uint32_t flags, int width, int height, int depth, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask)
{
    void* p = my->SDL_CreateRGBSurface(flags, width, height, depth, Rmask, Gmask, Bmask, Amask);
    inplace_SDL2_Surface_to_32(p);
    return p;
}

EXPORT void* my32_2_SDL_CreateRGBSurfaceFrom(x64emu_t* emu, void* pixels, int width, int height, int depth, int pitch, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask)
{
    void* p = my->SDL_CreateRGBSurfaceFrom(pixels, width, height, depth, pitch, rmask, gmask, bmask, amask);
    inplace_SDL2_Surface_to_32(p);
    return p;
}


EXPORT void my32_2_SDL_SetWindowIcon(x64emu_t* emu, void* window, void* icon)
{
    inplace_SDL2_Surface_to_64(icon);
    my->SDL_SetWindowIcon(window, icon);
    inplace_SDL2_Surface_to_32(icon);
}

EXPORT void* my32_2_SDL_CreateColorCursor(void* s, int x, int y)
{
    inplace_SDL2_Surface_to_64(s);
    void* ret = my->SDL_CreateColorCursor(s, x, y);
    inplace_SDL2_Surface_to_32(s);
    return ret;
}

EXPORT int my32_2_SDL_FillRect(x64emu_t* emu, void* s, void* rect, uint32_t color)
{
    inplace_SDL2_Surface_to_64(s);
    int ret = my->SDL_FillRect(s, rect, color);
    inplace_SDL2_Surface_to_32(s);
    return ret;
}

EXPORT void* my32_2_SDL_CreateTextureFromSurface(void* r, void* s)
{
    inplace_SDL2_Surface_to_64(s);
    void* ret = my->SDL_CreateTextureFromSurface(r, s);
    inplace_SDL2_Surface_to_32(s);
    return ret;
}

EXPORT int my32_2_SDL_SetColorKey(void* s, int flag, uint32_t color)
{
    inplace_SDL2_Surface_to_64(s);
    int ret = my->SDL_SetColorKey(s, flag, color);
    inplace_SDL2_Surface_to_32(s);
    return ret;
}

EXPORT void* my32_2_SDL_ConvertSurface(void* s, my_SDL2_PixelFormat_32_t* fmt, uint32_t flags)
{
    inplace_SDL2_Surface_to_64(s);
    void* ret = my->SDL_ConvertSurface(s, replace_SDL2_PixelFormat_to_64_ext(fmt), flags);
    inplace_SDL2_Surface_to_32(s);
    if(ret!=s) inplace_SDL2_Surface_to_32(ret);
    return ret;
}

EXPORT void* my32_2_SDL_ConvertSurfaceFormat(void* s, uint32_t fmt, uint32_t flags)
{
    inplace_SDL2_Surface_to_64(s);
    void* ret = my->SDL_ConvertSurfaceFormat(s, fmt, flags);
    inplace_SDL2_Surface_to_32(s);
    if(ret!=s) inplace_SDL2_Surface_to_32(ret);
    return ret;
}

EXPORT void my32_2_SDL_FreeSurface(x64emu_t* emu, void* surface)
{
    inplace_SDL2_Surface_to_64(surface);
    my->SDL_FreeSurface(surface);
}

void* my32_prepare_thread(x64emu_t* emu, void* f, void* arg, int ssize, void** pet);
EXPORT void* my32_2_SDL_CreateThread(x64emu_t* emu, void* f, void* n, void* p)
{
    void* et = NULL;
    void* fnc = my32_prepare_thread(emu, f, p, 0, &et);
    return my->SDL_CreateThread(fnc, n, et);
}

EXPORT int my32_2_SDL_WaitEventTimeout(my_SDL2_Event_32_t* evt, int timeout)
{
    my_SDL2_Event_t event = {0};
    int ret = my->SDL_WaitEventTimeout(evt?(&event):NULL, timeout);
    convert_SDL2_Event_to_32(evt, &event);
    return ret;
}

EXPORT int my32_2_SDL_PushEvent(my_SDL2_Event_32_t* evt)
{
    my_SDL2_Event_t event = {0};
    convert_SDL2_Event_to_64(&event, evt);
    return my->SDL_PushEvent(&event);
}

EXPORT void my32_2_SDL_AddEventWatch(x64emu_t* emu, void* p, void* userdata)
{
    my->SDL_AddEventWatch(find_eventfilter_Fct(p), userdata);
}
EXPORT void my32_2_SDL_DelEventWatch(x64emu_t* emu, void* p, void* userdata)
{
    my->SDL_DelEventWatch(find_eventfilter_Fct(p), userdata);
}
EXPORT void my32_2_SDL_SetEventFilter(x64emu_t* emu, void* p, void* userdata)
{
    my->SDL_SetEventFilter(find_eventfilter_Fct(p), userdata);
}

EXPORT void* my32_2_SDL_RWFromFile(x64emu_t* emu, void* a, void* b)
{
    SDL2_RWops_t* r = (SDL2_RWops_t*)my->SDL_RWFromFile(a, b);
    void* ret = AddNativeRW2(emu, r);
    inplace_SDL2_RWops_to_32(ret);
    return ret;
}

EXPORT void* my32_2_SDL_RWFromMem(x64emu_t* emu, void* a, int b)
{
    SDL2_RWops_t* r = (SDL2_RWops_t*)my->SDL_RWFromMem(a, b);
    void* ret = AddNativeRW2(emu, r);
    inplace_SDL2_RWops_to_32(ret);
    return ret;
}

EXPORT void *my32_2_SDL_LoadBMP_RW(x64emu_t* emu, void* a, int b)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t* rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->SDL_LoadBMP_RW(rw, b);
    if(b==0) {
        RWNativeEnd2(rw);
        inplace_SDL2_RWops_to_32(a);
    }
    inplace_SDL2_Surface_to_32(r);
    return r;
}
EXPORT int my32_2_SDL_GameControllerAddMappingsFromRW(x64emu_t* emu, void* a, int b)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int r = my->SDL_GameControllerAddMappingsFromRW(rw, b);
    if(b==0) {
        RWNativeEnd2(rw);
        inplace_SDL2_RWops_to_32(a);
    }
    return r;
}

EXPORT int64_t my32_2_SDL_RWseek(x64emu_t* emu, void* a, int64_t offset, int whence)
{
    //sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int64_t ret = RWNativeSeek2(rw, offset, whence);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT int64_t my32_2_SDL_RWtell(x64emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int64_t ret = RWNativeSeek2(rw, 0, 1);  //1 == RW_SEEK_CUR
    RWNativeEnd2(rw);
    return ret;
}
EXPORT int64_t my32_2_SDL_RWsize(x64emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int64_t ret = RWNativeSize2(rw);  //1 == RW_SEEK_CUR
    RWNativeEnd2(rw);
    return ret;
}
EXPORT uint64_t my32_2_SDL_RWread(x64emu_t* emu, void* a, void* ptr, uint64_t size, uint64_t maxnum)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t ret = RWNativeRead2(rw, ptr, size, maxnum);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT uint64_t my32_2_SDL_RWwrite(x64emu_t* emu, void* a, const void* ptr, uint64_t size, uint64_t maxnum)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t ret = RWNativeWrite2(rw, ptr, size, maxnum);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT int my32_2_SDL_RWclose(x64emu_t* emu, void* a)
{
    //sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    return RWNativeClose2(rw);
}

EXPORT uint64_t my32_2_SDL_ReadU8(x64emu_t* emu, void* a)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadU8(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my32_2_SDL_ReadBE16(x64emu_t* emu, void* a)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadBE16(rw);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}
EXPORT uint64_t my32_2_SDL_ReadBE32(x64emu_t* emu, void* a)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadBE32(rw);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}
EXPORT uint64_t my32_2_SDL_ReadBE64(x64emu_t* emu, void* a)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadBE64(rw);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}
EXPORT uint64_t my32_2_SDL_ReadLE16(x64emu_t* emu, void* a)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadLE16(rw);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}
EXPORT uint64_t my32_2_SDL_ReadLE32(x64emu_t* emu, void* a)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadLE32(rw);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}
EXPORT uint64_t my32_2_SDL_ReadLE64(x64emu_t* emu, void* a)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadLE64(rw);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}
EXPORT uint64_t my32_2_SDL_WriteU8(x64emu_t* emu, void* a, uint8_t v)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteU8(rw, v);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}
EXPORT uint64_t my32_2_SDL_WriteBE16(x64emu_t* emu, void* a, uint16_t v)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteBE16(rw, v);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}
EXPORT uint64_t my32_2_SDL_WriteBE32(x64emu_t* emu, void* a, uint64_t v)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteBE32(rw, v);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}
EXPORT uint64_t my32_2_SDL_WriteBE64(x64emu_t* emu, void* a, uint64_t v)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteBE64(rw, v);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}
EXPORT uint64_t my32_2_SDL_WriteLE16(x64emu_t* emu, void* a, uint16_t v)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteLE16(rw, v);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}
EXPORT uint64_t my32_2_SDL_WriteLE32(x64emu_t* emu, void* a, uint64_t v)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteLE32(rw, v);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}
EXPORT uint64_t my32_2_SDL_WriteLE64(x64emu_t* emu, void* a, uint64_t v)
{
    inplace_SDL2_RWops_to_64(a);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_WriteLE64(rw, v);
    RWNativeEnd2(rw);
    inplace_SDL2_RWops_to_32(a);
    return r;
}

typedef struct SDL_version_s
{
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} SDL_version_t;

typedef struct SDL_version_32_s //removed packed attribute
{
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} SDL_version_32_t;

typedef struct SDL_SysWMinfo_s
{
    SDL_version_t version;
    int subsystem;  // 1=Windows, 2 =X11, 6=Wayland
    union
    {
        struct
        {
            void* display;  //Display
            unsigned long window;   //Window
        } x11;
        uint8_t dummy[64];
    } info;
} SDL_SysWMinfo_t;

typedef struct SDL_SysWMinfo_32_s
{
    SDL_version_32_t version;
    int subsystem;  // 1=Windows, 2 =X11, 6=Wayland
    union
    {
        struct
        {
            ptr_t display;  //Display*
            ulong_t window;   //Window
        } x11;
        uint8_t dummy[64];
    } info;
} SDL_SysWMinfo_32_t;
void* FindDisplay(void* d);
EXPORT int my32_2_SDL_GetWindowWMInfo(void* w, SDL_SysWMinfo_32_t* i)
{
    // 32bits and  64bits have the same size...
    // TODO: Check if it's true
    int ret = my->SDL_GetWindowWMInfo(w, i);
    if(i->subsystem==2) {
        // inplace conversion
        SDL_SysWMinfo_t* i_l = (SDL_SysWMinfo_t*) i;
        i->info.x11.display = to_ptrv(FindDisplay(i_l->info.x11.display));
        i->info.x11.window = to_ulong(i_l->info.x11.window);
    }
    return ret;
}

typedef struct my_SDL_MessageBoxButtonData_s
{
    uint32_t    flags;
    int         buttonid;
    const char* text;
} my_SDL_MessageBoxButtonData_t;

typedef struct my_SDL_MessageBoxData_s
{
    uint32_t    flags;
    void*       window; //SDL_Window*
    const char* title;
    const char* message;
    int         numbuttons;
    my_SDL_MessageBoxButtonData_t*  buttons;
    void*       colorScheme;    //const SDL_MessageBoxColorScheme
} my_SDL_MessageBoxData_t;

typedef struct my_SDL_MessageBoxButtonData_32_s
{
    uint32_t    flags;
    int         buttonid;
    ptr_t       text;   //const char*
} my_SDL_MessageBoxButtonData_32_t;

typedef struct my_SDL_MessageBoxData_32_s
{
    uint32_t    flags;
    ptr_t       window; //SDL_Window*
    ptr_t       title;  //const char*
    ptr_t       message;    //const char*
    int         numbuttons;
    ptr_t       buttons;    //my_SDL_MessageBoxButtonData_t*
    ptr_t       colorScheme;    //const SDL_MessageBoxColorScheme
} my_SDL_MessageBoxData_32_t;

EXPORT int my32_2_SDL_ShowMessageBox(my_SDL_MessageBoxData_32_t* msgbox, int* btn)
{
    my_SDL_MessageBoxData_t msgbox_l;
    my_SDL_MessageBoxButtonData_t btns_l[msgbox->numbuttons];
    msgbox_l.flags = msgbox->flags;
    msgbox_l.window = from_ptrv(msgbox->window);
    msgbox_l.title = from_ptrv(msgbox->title);
    msgbox_l.message = from_ptrv(msgbox->message);
    msgbox_l.numbuttons = msgbox->numbuttons;
    msgbox_l.buttons = btns_l;
    msgbox_l.colorScheme = from_ptrv(msgbox->colorScheme);
    my_SDL_MessageBoxButtonData_32_t* src = from_ptrv(msgbox->buttons);
    for(int i=0; i<msgbox_l.numbuttons; ++i) {
        btns_l[i].flags = src[i].buttonid;
        btns_l[i].buttonid = src[i].buttonid;
        btns_l[i].text = from_ptrv(src[i].buttonid);
    }
    return my->SDL_ShowMessageBox(&msgbox_l, btn);
}

EXPORT unsigned long my32_2_SDL_GetThreadID(x64emu_t* emu, void* thread)
{
    unsigned long ret = my->SDL_GetThreadID(thread);
    int max = 10;
    while (!ret && max--) {
        sched_yield();
        ret = my->SDL_GetThreadID(thread);
    }
    return ret;
}

EXPORT int my32_2_SDL_GetCPUCount(x64emu_t* emu)
{
    int ret = my->SDL_GetCPUCount();
    if(BOX64ENV(maxcpu) && ret>BOX64ENV(maxcpu))
        ret = BOX64ENV(maxcpu);
    return ret;
}



#define ALTMY my32_2_

#define CUSTOM_INIT                       \
    box64->sdl2allocrw = my->SDL_AllocRW; \
    box64->sdl2freerw = my->SDL_FreeRW;

#define NEEDED_LIBS "libdl.so.2", "libm.so.6", "librt.so.1", "libpthread.so.0"

#define CUSTOM_FINI                 \
    my_context->sdl2allocrw = NULL; \
    my_context->sdl2freerw = NULL;


#include "wrappedlib_init32.h"
