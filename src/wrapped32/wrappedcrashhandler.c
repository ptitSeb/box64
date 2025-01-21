#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static const char* crashhandlerName = "crashhandler.so";
#define LIBNAME crashhandler

#define PRE_INIT                                                \
    if(!BOX64ENV(dummy_crashhandler))                           \
        return -1;                                              \
    if(1)                                                       \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);\
    else


#include "wrappedlib_init32.h"

