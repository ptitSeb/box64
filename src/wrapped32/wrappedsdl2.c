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

typedef struct __attribute__((packed)) {
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
static uintptr_t my_eventfilter_fct_##A = 0;                                    \
static int my_eventfilter_##A(void* userdata, void* event)                      \
{                                                                               \
    static my_SDL2_Event_32_t evt = {0};                                        \
    convert_SDL2_Event_to_32(&evt, event);                                      \
    return (int)RunFunctionFmt(my_eventfilter_fct_##A, "pp", userdata, &evt);   \
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

EXPORT int my32_2_SDL_SetWindowDisplayMode(void* window, void* mode)
{
    my_SDL2_DisplayMode_t mode_l = { 0 };
    convert_SDL2_DisplayMode_to_64(&mode_l, mode);
    return my->SDL_SetWindowDisplayMode(window, &mode_l);
}

EXPORT void* my32_2_SDL_JoystickGetDeviceGUID(void* ret, int index)
{
   *(SDL2_GUID_t*)ret = my->SDL_JoystickGetDeviceGUID(index);
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
    return getGLProcAddress32(emu, (glprocaddress_t)my->SDL_GL_GetProcAddress, rname);
}

EXPORT void* my32_2_SDL_CreateRGBSurfaceWithFormatFrom(x64emu_t* emu, void* pixels, int width, int height, int depth, int pitch, uint32_t format)
{
    void* p = my->SDL_CreateRGBSurfaceWithFormatFrom(pixels, width, height, depth, pitch, format);
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

EXPORT void* my32_2_SDL_RWFromFile(x64emu_t* emu, void* a, void* b)
{
    SDL2_RWops_t* r = (SDL2_RWops_t*)my->SDL_RWFromFile(a, b);
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

#define ALTMY my32_2_

#define CUSTOM_INIT                       \
    box64->sdl2allocrw = my->SDL_AllocRW; \
    box64->sdl2freerw = my->SDL_FreeRW;

#define NEEDED_LIBS "libdl.so.2", "libm.so.6", "librt.so.1", "libpthread.so.0"

#define CUSTOM_FINI                 \
    my_context->sdl2allocrw = NULL; \
    my_context->sdl2freerw = NULL;


#include "wrappedlib_init32.h"