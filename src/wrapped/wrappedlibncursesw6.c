#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"
#include "globalsymbols.h"

const char* libncursesw6Name = "libncursesw.so.6";
#define LIBNAME libncursesw6

static library_t* my_lib = NULL;

#define ADDED_FUNCTIONS() GO(stdscr, void*)
#include "generated/wrappedlibncurseswtypes.h"

typedef struct libncursesw6_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libncursesw6_my_t;

void* getNCursesw6My(library_t* lib)
{
    libncursesw6_my_t* my = (libncursesw6_my_t*)calloc(1, sizeof(libncursesw6_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeNCursesw6My(void* lib)
{
    //libncursesw6_my_t *my = (libncursesw6_my_t *)lib;
}

EXPORT int myw6_mvwprintw(x64emu_t* emu, void* win, int32_t y, int32_t x, void* fmt, void* b)
{
    libncursesw6_my_t *my = (libncursesw6_my_t*)my_lib->priv.w.p2;

    char* buf = NULL;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 4);
    PREPARE_VALIST;
    int ret = vasprintf(&buf, (const char*)fmt, VARARGS);
    (void)ret;
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    ret = my->mvwprintw(win, y, x, buf);
    free(buf);
    return ret;
}

EXPORT int myw6_printw(x64emu_t* emu, void* fmt, void* b)
{
    libncursesw6_my_t *my = (libncursesw6_my_t*)my_lib->priv.w.p2;

    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->vwprintw(my->stdscr, fmt, VARARGS);
}

EXPORT int myw6_vwprintw(x64emu_t* emu, void* p, void* fmt, x64_va_list_t b)
{
    libncursesw6_my_t *my = (libncursesw6_my_t*)my_lib->priv.w.p2;

    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return my->vwprintw(p, fmt, VARARGS);
}

EXPORT int myw6_mvprintw(x64emu_t* emu, int x, int y, void* fmt, void* b)
{
    libncursesw6_my_t *my = (libncursesw6_my_t*)my_lib->priv.w.p2;

    char* buf = NULL;
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 3);
    PREPARE_VALIST;
    int ret = vasprintf(&buf, (const char*)fmt, VARARGS);
    (void)ret;
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    ret = my->mvprintw(x, y, buf);
    free(buf);
    return ret;
}

EXPORT void* myw6_initscr()
{
    libncursesw6_my_t *my = (libncursesw6_my_t*)my_lib->priv.w.p2;
    void* ret = my->initscr();
    my_checkGlobalTInfo();
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getNCursesw6My(lib); \
    my_lib = lib;   \
    lib->altmy = strdup("myw6_"); \
    lib->priv.w.needed = 1; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libtinfo.so.6");

#define CUSTOM_FINI \
    freeNCursesw6My(lib->priv.w.p2); \
    free(lib->priv.w.p2);           \
    my_lib = NULL;

#include "wrappedlib_init.h"
