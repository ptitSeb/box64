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

const char* libdrmName = "libdrm.so.2";
#define LIBNAME libdrm

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedlibdrmtypes.h"

#include "wrappercallback.h"

EXPORT void my_drmMsg(x64emu_t* emu, void* fmt, void* b) {
    myStackAlign(emu, (const char*)fmt, b, emu->scratch, R_EAX, 1);
    PREPARE_VALIST;
    char* buf = NULL;
    int dummy = vasprintf(&buf, (const char*)fmt, VARARGS);
    (void)dummy;
    my->drmMsg(buf);
    free(buf);
}

#include "wrappedlib_init.h"
