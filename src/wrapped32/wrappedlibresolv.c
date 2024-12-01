#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <resolv.h>
#include <netdb.h>

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
#include "box32.h"

static const char* libresolvName = "libresolv.so.2";
#define LIBNAME libresolv

#include "generated/wrappedlibresolvtypes32.h"

#include "wrappercallback32.h"

void* convert_res_state_to_32(void* d, void* s);
void* convert_res_state_to_64(void* d, void* s);

EXPORT int my32_res_query(x64emu_t* emu, void* dname, int class, int type, void* answer, int anslen)
{
    convert_res_state_to_64(emu->res_state_64, emu->res_state_32);
    int ret = my->__res_query(dname, class, type, answer, anslen);
    emu->libc_herr = h_errno;
    return ret;
}

#include "wrappedlib_init32.h"
