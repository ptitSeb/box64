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
#include "callback.h"
#include "librarian.h"
#include "box64context.h"
#include "emu/x64emu_private.h"
#include "myalign.h"

const char* gcryptName = "libgcrypt.so.20";
#define LIBNAME gcrypt

typedef uint32_t  (*uFpppp_t)(void*, void*, void*, void*);

#define ADDED_FUNCTIONS()                   \
    GO(gcry_sexp_build_array, uFpppp_t)     \

#include "generated/wrappedgcrypttypes.h"

#include "wrappercallback.h"

EXPORT uint32_t my_gcry_sexp_build(x64emu_t* emu, void* r_sexp, void* erroff, const char* fmt, uintptr_t* V)
{
    // count the number of elements
    int n = 0;
    const char* p = fmt;
    while(p) {
        if(*p == '%') {
            ++p;
            switch (*p) {
                case 'm':
                case 'M':
                case 's':
                case 'd':
                case 'u':
                case 'S':
                    ++n;
                    break;
                case 'b':
                    n+=2;
                    break;
            }
        };
        ++p;
    }
    // Ok, alloc the array
    uintptr_t array[n];
    // transfert the datas...
    p = fmt;
    int i = 0;
    while(p) {
        if(*p == '%') {
            ++p;
            switch (*p) {
                case 'm':
                case 'M':
                case 's':
                case 'd':
                case 'u':
                case 'S':
                    array[i] = getVArgs(emu, 3, V, i);
                    ++i;
                    break;
                case 'b':
                    array[i] = getVArgs(emu, 3, V, i);
                    ++i;
                    array[i] = getVArgs(emu, 3, V, i);
                    ++i;
                    break;
            }
        };
        ++p;
    }
    return my->gcry_sexp_build_array(r_sexp, erroff, (void*)fmt, array);
}

#include "wrappedlib_init.h"
