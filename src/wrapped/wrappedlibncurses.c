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

const char* libncursesName = "libncurses.so.5";
#define LIBNAME libncurses
#define ALTNAME "libncurses.so.6"

// this is a simple copy of libncursesw wrapper. TODO: check if ok

#define ADDED_FUNCTIONS() GO(stdscr, void*)
#include "generated/wrappedlibncursestypes.h"

#include "wrappercallback.h"

EXPORT int my_mvwprintw(x64emu_t* emu, void* win, int32_t y, int32_t x, void* fmt, void* b)
{
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

EXPORT int my_printw(x64emu_t* emu, void* fmt, void* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    return my->vwprintw(my->stdscr, fmt, VARARGS);
}

EXPORT int my_wprintw(x64emu_t* emu, void* win, void* fmt, void* b)
{
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 2);
    PREPARE_VALIST;
    return my->vwprintw(win, fmt, VARARGS);
}

EXPORT int my_vwprintw(x64emu_t* emu, void* p, void* fmt, x64_va_list_t b)
{
    #ifdef CONVERT_VALIST
    CONVERT_VALIST(b);
    #else
    myStackAlignValist(emu, (const char*)fmt, emu->scratch, b);
    PREPARE_VALIST;
    #endif
    return my->vwprintw(p, fmt, VARARGS);
}

EXPORT int my_mvprintw(x64emu_t* emu, int x, int y, void* fmt, void* b)
{
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

EXPORT void* my_initscr()
{
    void* ret = my->initscr();
    my_checkGlobalTInfo();
    return ret;
}

#define NEEDED_LIBS "libtinfo.so.5"

#include "wrappedlib_init.h"
