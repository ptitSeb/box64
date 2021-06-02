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

const char* libncurseswName = "libncursesw.so.5";
#define LIBNAME libncursesw

static library_t* my_lib = NULL;

typedef int         (*iFppV_t)(void*, void*, va_list);
typedef int         (*iFpiip_t)(void*, int32_t, int32_t, void*);

#define SUPER()             \
    GO(mvwprintw, iFpiip_t) \
    GO(vwprintw, iFppV_t)   \
    GO(stdscr, void*)

typedef struct libncursesw_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libncursesw_my_t;

void* getNCurseswMy(library_t* lib)
{
    libncursesw_my_t* my = (libncursesw_my_t*)calloc(1, sizeof(libncursesw_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeNCurseswMy(void* lib)
{
    //libncursesw_my_t *my = (libncursesw_my_t *)lib;
}

EXPORT int myw_mvwprintw(x64emu_t* emu, void* win, int32_t y, int32_t x, void* fmt, void* b)
{
    libncursesw_my_t *my = (libncursesw_my_t*)my_lib->priv.w.p2;

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

EXPORT int myw_printw(x64emu_t* emu, void* fmt, void* b)
{
    libncursesw_my_t *my = (libncursesw_my_t*)my_lib->priv.w.p2;

    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->vwprintw(my->stdscr, fmt, VARARGS);
}

EXPORT int myw_vwprintw(x64emu_t* emu, void* p, void* fmt, x64_va_list_t b)
{
    libncursesw_my_t *my = (libncursesw_my_t*)my_lib->priv.w.p2;

    CONVERT_VALIST(b);
    return my->vwprintw(p, fmt, VARARGS);
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getNCurseswMy(lib); \
    my_lib = lib;   \
    lib->altmy = strdup("myw_"); \
    lib->priv.w.needed = 1; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libtinfo.so.5");

#define CUSTOM_FINI \
    freeNCurseswMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);           \
    my_lib = NULL;

#include "wrappedlib_init.h"
