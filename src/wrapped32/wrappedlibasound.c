#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdarg.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "bridge.h"
#include "box32.h"

#define LIBNAME libasound
static const char* libasoundName = "libasound.so.2";

typedef int (*iFpp_t)(void*, void*);
typedef size_t(*LFv_t)();

#define ADDED_FUNCTIONS() \
    GO(snd_pcm_hw_params_current, iFpp_t) \
    GO(snd_pcm_hw_params_get_channels, iFpp_t) \
    GO(snd_pcm_hw_params_sizeof, LFv_t) \

#include "generated/wrappedlibasoundtypes32.h"

EXPORT uintptr_t my32_snd_lib_error = 0;
static void default_error_handler(const char *file, int line, const char *function, int err, const char *fmt, va_list ap)
{
    (void)file; (void)line; (void)function; (void)err;
    vprintf(fmt, ap);
}

#define ADDED_INIT() \
    my32_snd_lib_error = AddCheckBridge(my_lib->w.bridge, vFpipipV_32, default_error_handler, 0, "ASoundCustomErrorHandler");

#define ADDED_FINI() \
    my32_snd_lib_error = 0;   // no removing of bridge

#include "wrappercallback32.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// snd_async_callback_t
#define GO(A)   \
static uintptr_t my32_async_fct_##A = 0;                                          \
static void* my32_async_##A(void* handler)                                        \
{                                                                               \
    return (void*)RunFunctionFmt(my32_async_fct_##A, "p", handler);         \
}
SUPER()
#undef GO
static void* findAsyncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_async_fct_##A == (uintptr_t)fct) return my32_async_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_async_fct_##A == 0) {my32_async_fct_##A = (uintptr_t)fct; return my32_async_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Asound Async callback\n");
    return NULL;
}
// snd_mixer_elem_callback_t
#define GO(A)   \
static uintptr_t my32_elem_fct_##A = 0;                                           \
static int my32_elem_##A(void* elem, uint32_t mask)                               \
{                                                                               \
    return (int)RunFunctionFmt(my32_elem_fct_##A, "pu", elem, mask);        \
}
SUPER()
#undef GO
static void* findElemFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_elem_fct_##A == (uintptr_t)fct) return my32_elem_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_elem_fct_##A == 0) {my32_elem_fct_##A = (uintptr_t)fct; return my32_elem_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Asound Elem callback\n");
    return NULL;
}
// snd_pcm_hook_func_t
#define GO(A)   \
static uintptr_t my32_pcm_hook_fct_##A = 0;                       \
static int my32_pcm_hook_##A(void* a)                             \
{                                                               \
    return (int)RunFunctionFmt(my32_pcm_hook_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* findPCMHookFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_pcm_hook_fct_##A == (uintptr_t)fct) return my32_elem_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_pcm_hook_fct_##A == 0) {my32_pcm_hook_fct_##A = (uintptr_t)fct; return my32_pcm_hook_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Asound PCMHook callback\n");
    return NULL;
}
//  snd_mixer_compare_t
#define GO(A)   \
static uintptr_t my32_mixer_compare_fct_##A = 0;                       \
static int my32_mixer_compare_##A(void* a)                             \
{                                                               \
    return (int)RunFunctionFmt(my32_mixer_compare_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* findMixerCompareFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_mixer_compare_fct_##A == (uintptr_t)fct) return my32_elem_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_mixer_compare_fct_##A == 0) {my32_mixer_compare_fct_##A = (uintptr_t)fct; return my32_mixer_compare_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Asound Mixer Compare callback\n");
    return NULL;
}
//  private_free
#define GO(A)   \
static uintptr_t my32_private_free_fct_##A = 0;                       \
static int my32_private_free_##A(void* a)                             \
{                                                               \
    return (int)RunFunctionFmt(my32_private_free_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* findPrivateFreeFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_private_free_fct_##A == (uintptr_t)fct) return my32_elem_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_private_free_fct_##A == 0) {my32_private_free_fct_##A = (uintptr_t)fct; return my32_private_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Asound PrivateFree callback\n");
    return NULL;
}
//  snd_mixer_event_t
#define GO(A)   \
static uintptr_t my32_mixer_event_fct_##A = 0;                                \
static int my32_mixer_event_##A(void* a, uint32_t b, void* c, void* d)        \
{                                                                           \
    return (int)RunFunctionFmt(my32_mixer_event_fct_##A, "pupp", a, b, c, d); \
}
SUPER()
#undef GO
static void* findMixerEventFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_mixer_event_fct_##A == (uintptr_t)fct) return my32_elem_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_mixer_event_fct_##A == 0) {my32_mixer_event_fct_##A = (uintptr_t)fct; return my32_mixer_event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Asound MixerEvent callback\n");
    return NULL;
}


//EXPORT int my32_snd_async_add_handler(x64emu_t *emu, void *handler, int fd, void* callback, void *private_data)
//{
//    return my->snd_async_add_handler(handler, fd, findAsyncFct(callback), private_data);
//}

//EXPORT int my32_snd_async_add_pcm_handler(x64emu_t *emu, void *handler, void* pcm,  void* callback, void *private_data)
//{
//    return my->snd_async_add_pcm_handler(handler, pcm, findAsyncFct(callback), private_data);
//}

static void* current_error_handler = NULL;
static void dummy32_error_handler(const char *file, int line, const char *function, int err, const char *fmt, ...)
{
    (void)function; (void)err;
    va_list ap;

    fprintf(stderr, "Warning: this is a dummy snd_lib error handler\n");
    fprintf(stderr, "Error in file %s on line %i: ", file, line);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

static void empty_error_handler(const char *file, int line, const char *function, int err, const char *fmt, ...)
{
    (void)file; (void)line; (void)function; (void)err; (void)fmt;
    // do nothing
}

//EXPORT int my32_snd_lib_error_set_handler(x64emu_t* emu, void* handler)
//{
//    current_error_handler = handler;
//    void *error_handler;
//    uint8_t *code = (uint8_t *)handler;
//    if (code) {
//        if ((code[0] == 0xC3) || ((code[0] == 0xF3) && (code[1] == 0xC3))) {
//            error_handler = &empty_error_handler;
//        } else {
//            error_handler = &dummy32_error_handler;
//            printf_log(LOG_NONE, "Warning: snd_lib_error_set_handler: using dummy error handler\n");
//        }
//    } else error_handler = NULL;
//
//    return my->snd_lib_error_set_handler(error_handler);
//}

//EXPORT void my32_snd_mixer_elem_set_callback(x64emu_t* emu, void* handler, void* f)
//{
//    my->snd_mixer_elem_set_callback(handler, findElemFct(f));
//}

//EXPORT int my32_snd_pcm_hook_add(x64emu_t* emu, void* hook, void* pcm, uint32_t type, void* f, void* data)
//{
//    return my->snd_pcm_hook_add(hook, pcm, type, findPCMHookFct(f), data);
//}

//EXPORT int my32_snd_mixer_set_compare(x64emu_t* emu, void* mixer, void* f)
//{
//    return my->snd_mixer_set_compare(mixer, findMixerCompareFct(f));
//}

//EXPORT int my32_snd_mixer_elem_new(x64emu_t* emu, void* elem, uint32_t type, int weight, void* data, void* f)
//{
//    return my->snd_mixer_elem_new(elem, type, weight, data, findPrivateFreeFct(f));
//}

//EXPORT void* my32_snd_mixer_class_get_event(x64emu_t* emu, void* class)
//{
//    void* ret = my->snd_mixer_class_get_event(class);
//    AddAutomaticBridge(my_lib->w.bridge, iFpupp, ret, 0, "snd_event_t");
//    return ret;
//}

//EXPORT void* my32_snd_mixer_class_get_compare(x64emu_t* emu, void* class)
//{
//    void* ret = my->snd_mixer_class_get_compare(class);
//    AddAutomaticBridge(my_lib->w.bridge, iFpp, ret, 0, "snd_mixer_compare_t");
//    return ret;
//}

//EXPORT int my32_snd_mixer_class_set_event(x64emu_t* emu, void* class, void* f)
//{
//    return my->snd_mixer_class_set_event(class, findMixerEventFct(f));
//}

//EXPORT int my32_snd_mixer_class_set_private_free(x64emu_t* emu, void* class, void* f)
//{
//    return my->snd_mixer_class_set_private_free(class, findPrivateFreeFct(f));
//}

//EXPORT int my32_snd_mixer_class_set_compare(x64emu_t* emu, void* class, void* f)
//{
//    return my->snd_mixer_class_set_compare(class, findMixerCompareFct(f));
//}

typedef struct _my_snd_pcm_channel_area_s {
    void *addr;
    unsigned int first;
    unsigned int step;
} my_snd_pcm_channel_area_t;
typedef struct __attribute__((packed, aligned(4))) _my_snd_pcm_channel_area_32_s {
    ptr_t addr;
    unsigned int first;
    unsigned int step;
} my_snd_pcm_channel_area_32_t;

EXPORT int my32_snd_pcm_mmap_begin(x64emu_t* emu, void* pcm, ptr_t* areas, ulong_t* offset, ulong_t* frames)
{
    my_snd_pcm_channel_area_t *l_areas;
    unsigned long l_offset;
    unsigned long l_frames = from_ulong(*frames);
    int ret = my->snd_pcm_mmap_begin(pcm, &l_areas, &l_offset, &l_frames);
    if(ret)
        return ret;
    *offset = to_ulong(l_offset);
    *frames = to_ulong(l_frames);
    static my_snd_pcm_channel_area_32_t my_areas[15] = {0};
    static void* last_pcm = NULL;
    static int last_nch = 0;
    // get the number of channels
    unsigned int nch = 0;
    if(pcm==last_pcm)
        nch = last_nch;
    else {
        void* hw=alloca(my->snd_pcm_hw_params_sizeof());
        my->snd_pcm_hw_params_current(pcm, hw);
        my->snd_pcm_hw_params_get_channels(hw, &nch);
        last_pcm = pcm;
        last_nch = nch;
    }
    if(nch>15) {printf_log(LOG_INFO, "Warning, too many channels in pcm of 32bits alsa: %d\n", nch); nch=15; }
    for(unsigned int i=0; i<nch; ++i) {
        my_areas[i].addr = to_ptrv(l_areas[i].addr);
        my_areas[i].first = l_areas[i].first;
        my_areas[i].step = l_areas[i].step;
    }
    *areas = to_ptrv(&my_areas);
    return ret;
}

EXPORT int my32_snd_device_name_hint(x64emu_t* emu, int card, void* iface, ptr_t* hints)
{
    void** hints_l = NULL;
    int ret = my->snd_device_name_hint(card, iface, &hints_l);
    if(ret) return ret;
    *hints = to_ptrv(hints_l);
    // inplace shrink
    int n = 0;
    while(hints_l[n]) ++n;
    ++n;
    for(int i=0; i<n; ++i)
        ((ptr_t*)hints_l)[i] = to_ptrv(hints_l[i]);
    return ret;
}

EXPORT int my32_snd_device_name_free_hint(x64emu_t* emu, ptr_t* hints)
{
    void** hints_l = (void**)hints;
    int n=0;
    while(hints[n]) ++n;
    for(int i=n; i>=0; --i)
        hints_l[i] = from_ptrv(hints[i]);
    return my->snd_device_name_free_hint(hints_l);
}

void* my_dlopen(x64emu_t* emu, void *filename, int flag);   // defined in wrappedlibdl.c
char* my_dlerror(x64emu_t* emu);
int my_dlclose(x64emu_t* emu, void *handle);
void* my_dlvsym(x64emu_t* emu, void *handle, void *symbol, void *version);
EXPORT void * my32_snd_dlopen(x64emu_t* emu, void* name, int mode, void* errbuf, size_t errbuflen)
{
    void* ret = my_dlopen(emu, name, mode);  // Does NULL name (so dlopen libasound) need special treatment?
    if(!ret && errbuf) {
        strncpy(errbuf, my_dlerror(emu), errbuflen);
    }
    return ret;
}
EXPORT int my32_snd_dlclose(x64emu_t* emu, void* handle)
{
    return my_dlclose(emu, handle);
}
EXPORT void* my32_snd_dlsym(x64emu_t* emu, void* handle, void* name, void* version)
{
    return my_dlvsym(emu, handle, name, version);
}

#include "wrappedlib_init32.h"
