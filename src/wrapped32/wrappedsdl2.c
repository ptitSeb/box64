#include "sdl2align32.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE /* See feature_test_macros(7) */
#include <dlfcn.h>

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

#include "x64trace.h"
#include "threads.h"

#include "sdl2align32.h"

extern const char* sdl2Name;
#define LIBNAME sdl2

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

typedef struct {
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

#define ADDED_FUNCTIONS()

#define ADDED_FINI()

#include "generated/wrappedsdl2types32.h"
#include "wrappercallback32.h"

#define SUPER() \
    GO(0)       \
    GO(1)       \
    GO(2)       \
    GO(3)       \
    GO(4)


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
    my_SDL2_DisplayMode_t my_mode;
    int ret = my->SDL_GetDesktopDisplayMode(displayIndex, &my_mode);
    convert_SDL2_DisplayMode_to_32(mode, &my_mode);
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
    if (ret != 0) {
        return ret;
    }
    // put back stuff in place?
    if (o) {
        SDL2_AudioSpec32* o_ = o;
        o_->channels = output.channels;
        o_->format = output.format;
        o_->freq = output.freq;
        o_->samples = output.samples;
        o_->padding = output.padding;
        o_->silence = output.silence;
        o_->size = output.size;
        o_->userdata = o_->callback = 0;
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

#define ALTMY my32_2_

#include "wrappedlib_init32.h"