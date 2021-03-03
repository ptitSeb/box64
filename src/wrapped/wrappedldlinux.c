#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "elfloader.h"
#include "box64context.h"

typedef struct my_tls_s {
    int         i;
    uint32_t     o;
} my_tls_t;

EXPORT void* my___tls_get_addr(x64emu_t* emu, void* p)
{
    my_tls_t *t = (my_tls_t*)p;
    return GetDTatOffset(emu->context, t->i, t->o);
}

EXPORT void* my____tls_get_addr(x64emu_t* emu)
{
    // the GNU version (with 3 '_') use register for the parameter!
    my_tls_t *t = (my_tls_t*)R_RAX;
    return GetDTatOffset(emu->context, t->i, t->o);
}


const char* ldlinuxName = "ld-linux-x86-64.so.2";
#define LIBNAME ldlinux

// define all standard library functions
#include "wrappedlib_init.h"

