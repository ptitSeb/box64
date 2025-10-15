#define _GNU_SOURCE         /* See feature_test_macros(7) */
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
#include "elfloader.h"
#include "box32context.h"

typedef struct __attribute__((packed, aligned(4))) my32_tls_s {
    int         i;
    uint32_t     o;
} my32_tls_t;

EXPORT void* my32___tls_get_addr(x64emu_t* emu, void* p)
{
    my32_tls_t *t = (my32_tls_t*)p;
    return GetDTatOffset(emu, t->i, t->o);
}

EXPORT void* my32____tls_get_addr(x64emu_t* emu)
{
    // the GNU version (with 3 '_') use register for the parameter!
    my32_tls_t *t = (my32_tls_t*)from_ptrv(R_EAX);
    return GetDTatOffset(emu, t->i, t->o);
}

EXPORT ptr_t my32___libc_stack_end;
static void stSetup32(box64context_t* context)
{
    my32___libc_stack_end = to_ptrv(context->stack);   // is this the end, or should I add stasz?
}

// don't try to load the actual ld-linux (because name is variable), just use box64 itself, as it's linked to ld-linux
static const char* ldlinuxName = "ld-linux.so.3";
#define LIBNAME ldlinux
#define ALTNAME "ld-linux.so.2"

// fake (ignored) _r_data structure
EXPORT void* my32__r_debug[5];

#define PRE_INIT\
    if(1)                                                           \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    else

#define CUSTOM_INIT         \
    stSetup32(box64);         \

#include "wrappedlib_init32.h"

