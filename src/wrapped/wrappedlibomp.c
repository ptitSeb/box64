// SPDX-License-Identifier: MIT
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
#include "myalign.h"

// libomp.so.5 (LLVM OpenMP runtime) wraps against native libomp.so
const char* libompName = "libomp.so";
#define LIBNAME libomp
#define ALTNAME "libgomp.so"

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlibomptypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \

// parallel
#define GO(A)   \
static uintptr_t my_parallel_fct_##A = 0;           \
static void my_parallel_##A(void* a)                \
{                                                   \
    RunFunctionFmt(my_parallel_fct_##A, "p", a);    \
}
SUPER()
#undef GO
static void* find_parallel_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_parallel_fct_##A == (uintptr_t)fct) return my_parallel_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_parallel_fct_##A == 0) {my_parallel_fct_##A = (uintptr_t)fct; return my_parallel_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libomp Boxed parallel callback\n");
    return NULL;
}


// task_cpy
#define GO(A)   \
static uintptr_t my_task_cpy_fct_##A = 0;                \
static void my_task_cpy_##A(void* a, void* b)            \
{                                                       \
    RunFunctionFmt(my_task_cpy_fct_##A, "pp", a, b);     \
}
SUPER()
#undef GO
static void* find_task_cpy_Fct(void* fct)
{
    if(!fct) return fct;
    #define GO(A) if(my_task_cpy_fct_##A == (uintptr_t)fct) return my_task_cpy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_task_cpy_fct_##A == 0) {my_task_cpy_fct_##A = (uintptr_t)fct; return my_task_cpy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libomp Boxed task_cpy callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my2_GOMP_parallel(x64emu_t* emu, void* f, void* data, unsigned num_threads, uint32_t flags)
{
    my->GOMP_parallel(find_parallel_Fct(f), data, num_threads, flags);
}

EXPORT void my2_GOMP_task(x64emu_t* emu, void* fn, void *data, void* cpyfn, long arg_size,
               long arg_align, int if_clause, unsigned flags, void **depend, int priority)
{
    my->GOMP_task(find_parallel_Fct(fn), data, find_task_cpy_Fct(cpyfn), arg_size, arg_align, if_clause, flags, depend, priority);
}

#define ALTMY my2_

#include "wrappedlib_init.h"
