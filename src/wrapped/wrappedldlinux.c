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
#include "x64tls.h"

typedef struct my_tls_s {
    unsigned long int   i;
    unsigned long int   o;
} my_tls_t;

EXPORT void* my___tls_get_addr(void* p)
{
    my_tls_t *t = (my_tls_t*)p;
    tlsdatasize_t* ptr = getTLSData(my_context);
    return ptr->data+GetTLSBase(my_context->elfs[t->i])+t->o;
}

EXPORT void* my___libc_stack_end;
void stSetup(box64context_t* context)
{
    my___libc_stack_end = context->stack;   // is this the end, or should I add stasz?
}

#ifdef STATICBUILD
#include <link.h>
extern void* __libc_enable_secure;
extern void* __stack_chk_guard;
//extern void* __pointer_chk_guard;
//extern void* _rtld_global;
//extern void* _rtld_global_ro;
#endif

// don't try to load the actual ld-linux (because name is variable), just use box64 itself, as it's linked to ld-linux
const char* ldlinuxName = "ld-linux.so.2";
#define LIBNAME ldlinux

#ifndef STATICBUILD
#define PRE_INIT\
    if(1)                                                           \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    else
#endif

#define CUSTOM_INIT         \
    stSetup(box64);         \

// define all standard library functions
#include "wrappedlib_init.h"

