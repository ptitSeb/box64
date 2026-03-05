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
    while(*p) {
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
    #define A(i) getVArgs(emu, 3, V, i)
    switch(n) {
        case 0: return my->gcry_sexp_build(r_sexp, erroff, (void*)fmt);
        case 1: return my->gcry_sexp_build(r_sexp, erroff, (void*)fmt, A(0));
        case 2: return my->gcry_sexp_build(r_sexp, erroff, (void*)fmt, A(0), A(1));
        case 3: return my->gcry_sexp_build(r_sexp, erroff, (void*)fmt, A(0), A(1), A(2));
        case 4: return my->gcry_sexp_build(r_sexp, erroff, (void*)fmt, A(0), A(1), A(2), A(3));
        case 5: return my->gcry_sexp_build(r_sexp, erroff, (void*)fmt, A(0), A(1), A(2), A(3), A(4));
        case 6: return my->gcry_sexp_build(r_sexp, erroff, (void*)fmt, A(0), A(1), A(2), A(3), A(4), A(5));
        case 7: return my->gcry_sexp_build(r_sexp, erroff, (void*)fmt, A(0), A(1), A(2), A(3), A(4), A(5), A(6));
        default: printf_log(LOG_INFO, "Warning, gcry_sexp_build with %d args not handled, using generic fallback\n", n);
    }
    // Ok, alloc the array
    uintptr_t array[n];
    // transfert the datas... Not sure this is correct, it seems to be needed byref and not staight values
    for(int i=0; i<n; ++i)
        array[i] = A(i);
    #undef A
    return my->gcry_sexp_build_array(r_sexp, erroff, (void*)fmt, array);
}

#include "wrappedlib_init.h"
