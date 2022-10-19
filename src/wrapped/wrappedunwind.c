#define _GNU_SOURCE
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

const char* unwindName = "libunwind.so.8";
#define LIBNAME unwind

EXPORT int my__ULx86_64_step(x64emu_t* emu, void* cursor) {
    return -1;
}
EXPORT int my__ULx86_64_get_reg(x64emu_t* emu, void* cursor, int rep, void* buf) {
    return -1;
}
EXPORT int my__ULx86_64_get_proc_info(x64emu_t* emu, void* cursor, void* ctx) {
    return -1;
}
EXPORT int my__ULx86_64_init_local(x64emu_t* emu, void* cursor, void* ctx) {
    return -1;
}


#define PRE_INIT                                                \
    if(1)                                                       \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);\
    else


#include "wrappedlib_init.h"

