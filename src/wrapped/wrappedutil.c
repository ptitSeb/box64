#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "box64stack.h"
#include "x64emu.h"
#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"
#include "librarian/library_private.h"
#include "emu/x64emu_private.h"
#include "box64context.h"

const char* utilName = "libutil.so.1";
#define LIBNAME util

EXPORT pid_t my_forkpty(x64emu_t* emu, void* amaster, void* name, void* termp, void* winp)
{
    static forkpty_t forkinfo;
    forkinfo.amaster = amaster;
    forkinfo.name = name;
    forkinfo.termp = termp;
    forkinfo.winp = winp;
    library_t* lib = GetLibInternal(utilName);
    forkinfo.f = dlsym(lib->w.lib, "forkpty");
    
    emu->quit = 1;
    emu->fork = 2;
    emu->forkpty_info = &forkinfo;
        
    return 0;
}

#ifdef STATICBUILD
#include <pty.h>
#include <utmp.h>
#endif

#ifdef STATICBUILD
#else
#define PRE_INIT\
    if(1)                                                      \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    else
#endif

#include "wrappedlib_init.h"

