#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"
#include "librarian/library_private.h"
#include "emu/x64emu_private.h"
#include "box64context.h"
#include "gltools.h"

#include "generated/wrappedsdl3defs.h"

const char* sdl3Name = "libSDL3.so.0";
#define LIBNAME sdl3

static void* my_glhandle = NULL;
// DL functions from wrappedlibdl.c
void* my_dlopen(x64emu_t* emu, void *filename, int flag);
int my_dlclose(x64emu_t* emu, void *handle);

typedef void  (*vFv_t)(void);
#define ADDED_FUNCTIONS()           \
    GO(SDL_Quit, vFv_t)
#include "generated/wrappedsdl3types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// Timer (SDL3 callback: uint32_t(void* userdata, uint32_t timerID, uint32_t interval))
#define GO(A)   \
static uintptr_t my_Timer_fct_##A = 0;                                      \
static uint32_t my_Timer_##A(void* a, uint32_t b, uint32_t c)               \
{                                                                           \
    return (uint32_t)RunFunctionFmt(my_Timer_fct_##A, "puu", a, b, c);      \
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
    printf_log(LOG_NONE, "Warning, no more slot for SDL3 Timer callback\n");
    return NULL;
}

// TimerNS (SDL3 callback: uint64_t(void* userdata, uint32_t timerID, uint64_t interval))
#define GO(A)   \
static uintptr_t my_TimerNS_fct_##A = 0;                                    \
static uint64_t my_TimerNS_##A(void* a, uint32_t b, uint64_t c)             \
{                                                                           \
    return (uint64_t)RunFunctionFmt(my_TimerNS_fct_##A, "puU", a, b, c);    \
}
SUPER()
#undef GO
static void* find_TimerNS_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_TimerNS_fct_##A == (uintptr_t)fct) return my_TimerNS_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TimerNS_fct_##A == 0) {my_TimerNS_fct_##A = (uintptr_t)fct; return my_TimerNS_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL3 TimerNS callback\n");
    return NULL;
}

// eventfilter (SDL3 callback: bool(void* userdata, SDL_Event* event))
#define GO(A)   \
static uintptr_t my_eventfilter_fct_##A = 0;                                \
static int my_eventfilter_##A(void* userdata, void* event)                   \
{                                                                           \
    return (int)RunFunctionFmt(my_eventfilter_fct_##A, "pp", userdata, event);    \
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
    printf_log(LOG_NONE, "Warning, no more slot for SDL3 eventfilter callback\n");
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
    return (void*)AddBridge(my_lib->w.bridge, iFpp, fct, 0, NULL);
}

// LogOutput (SDL3 callback: void(void* userdata, int category, SDL_LogPriority priority, const char* message))
#define GO(A)   \
static uintptr_t my_LogOutput_fct_##A = 0;                                  \
static void my_LogOutput_##A(void* a, int b, int c, void* d)                \
{                                                                           \
    RunFunctionFmt(my_LogOutput_fct_##A, "piip", a, b, c, d);               \
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
    printf_log(LOG_NONE, "Warning, no more slot for SDL3 LogOutput callback\n");
    return NULL;
}
static void* reverse_LogOutput_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_LogOutput_##A == fct) return (void*)my_LogOutput_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpiip, fct, 0, NULL);
}

// Hint (SDL3 callback: void(void* userdata, const char* name, const char* oldValue, const char* newValue))
#define GO(A) \
static uintptr_t my_Hint_fct_##A = 0; \
static void my_Hint_##A(void* userdata, const char* name, const char* oldValue, const char* newValue) \
{ \
    RunFunctionFmt(my_Hint_fct_##A, "pppp", userdata, name, oldValue, newValue); \
}
SUPER()
#undef GO
static void* find_Hint_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Hint_fct_##A == (uintptr_t)fct) return my_Hint_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Hint_fct_##A == 0) {my_Hint_fct_##A = (uintptr_t)fct; return my_Hint_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL3 Hint callback\n");
    return NULL;
}
static void* reverse_Hint_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_Hint_##A == fct) return (void*)my_Hint_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpppp, fct, 0, NULL);
}

#undef SUPER

EXPORT void* my3_SDL_GL_GetProcAddress(x64emu_t* emu, void* name)
{
    const char* rname = (const char*)name;
    static int lib_checked = 0;
    if(!lib_checked) {
        lib_checked = 1;
        // check if libGL is loaded, load it if not
        if(!my_glhandle && !GetLibInternal(BOX64ENV(libgl)?BOX64ENV(libgl):"libGL.so.1"))
            my_glhandle = my_dlopen(emu, BOX64ENV(libgl)?BOX64ENV(libgl):"libGL.so.1", RTLD_LAZY|RTLD_GLOBAL);
    }
    return getGLProcAddress(emu, NULL, (glprocaddress_t)my->SDL_GL_GetProcAddress, rname);
}

void* my_vkGetInstanceProcAddr(x64emu_t* emu, void* device, void* name);
EXPORT void* my3_SDL_Vulkan_GetVkGetInstanceProcAddr(x64emu_t* emu)
{
    void* procaddr = my->SDL_Vulkan_GetVkGetInstanceProcAddr();
    if(!emu->context->vkprocaddress)
        emu->context->vkprocaddress = (vkprocaddess_t)procaddr;

    if(procaddr)
        return (void*)AddCheckBridge2(my_lib->w.bridge, pFEpp, my_vkGetInstanceProcAddr, procaddr, 0, "vkGetInstanceProcAddr");
    return NULL;
}

EXPORT uint32_t my3_SDL_AddTimer(x64emu_t* emu, uint32_t a, void* f, void* p)
{
    return my->SDL_AddTimer(a, find_Timer_Fct(f), p);
}

EXPORT uint32_t my3_SDL_AddTimerNS(x64emu_t* emu, uint64_t a, void* f, void* p)
{
    return my->SDL_AddTimerNS(a, find_TimerNS_Fct(f), p);
}

EXPORT void my3_SDL_SetEventFilter(x64emu_t* emu, void* p, void* userdata)
{
    my->SDL_SetEventFilter(find_eventfilter_Fct(p), userdata);
}

EXPORT int my3_SDL_GetEventFilter(x64emu_t* emu, void** f, void* userdata)
{
    int ret = my->SDL_GetEventFilter(f, userdata);
    *f = reverse_eventfilter_Fct(*f);
    return ret;
}

EXPORT int my3_SDL_AddEventWatch(x64emu_t* emu, void* p, void* userdata)
{
    return my->SDL_AddEventWatch(find_eventfilter_Fct(p), userdata);
}

EXPORT void my3_SDL_RemoveEventWatch(x64emu_t* emu, void* p, void* userdata)
{
    my->SDL_RemoveEventWatch(find_eventfilter_Fct(p), userdata);
}

struct my3_FilterEvents_data {
    uintptr_t callback;
    void *userdata;
};
static int my3_FilterEvents_callback(struct my3_FilterEvents_data* data, void* event) {
    return (int) RunFunctionFmt(data->callback, "pp", data->userdata, event);
}
EXPORT void my3_SDL_FilterEvents(x64emu_t* emu, void* filter, void* userdata) {
    struct my3_FilterEvents_data data = {
        .callback = (uintptr_t) filter,
        .userdata = userdata,
    };
    my->SDL_FilterEvents(my3_FilterEvents_callback, &data);
}

EXPORT int my3_SDL_AddHintCallback(x64emu_t* emu, char* name, void* callback, void* userdata)
{
    return my->SDL_AddHintCallback(name, find_Hint_Fct(callback), userdata);
}

EXPORT void my3_SDL_RemoveHintCallback(x64emu_t* emu, char* name, void* callback, void* userdata)
{
    my->SDL_RemoveHintCallback(name, reverse_Hint_Fct(callback), userdata);
}

EXPORT void my3_SDL_SetLogOutputFunction(x64emu_t* emu, void* f, void* arg)
{
    my->SDL_SetLogOutputFunction(find_LogOutput_Fct(f), arg);
}

EXPORT void my3_SDL_GetLogOutputFunction(x64emu_t* emu, void** f, void* arg)
{
    my->SDL_GetLogOutputFunction(f, arg);
    if(*f) *f = reverse_LogOutput_Fct(*f);
}

#undef HAS_MY

#define ALTMY my3_

#define CUSTOM_INIT         \
    getMy(lib);

#define CUSTOM_FINI                                             \
    my->SDL_Quit();                                             \
    if(my_glhandle) my_dlclose(thread_get_emu(), my_glhandle);  \
    my_glhandle = NULL;                                         \
    freeMy();

#include "wrappedlib_init.h"
