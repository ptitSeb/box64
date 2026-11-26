// SPDX-License-Identifier: MIT
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
#include "myalign.h"

// libomp.so.5 (LLVM OpenMP runtime) wraps against native libomp.so
const char* libompName = "libomp.so";
#define LIBNAME libomp
#define ALTNAME "libgomp.so"

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlibomptypes.h"

#include "wrappercallback.h"

#include "wrappedgomp_common.h"

EXPORT void my2_GOMP_parallel(x64emu_t* emu, void* f, void* data, unsigned num_threads, uint32_t flags)
{
    gomp_run_parallel(f, data, num_threads, flags);
}

EXPORT void my2_GOMP_task(x64emu_t* emu, void* fn, void *data, void* cpyfn, long arg_size,
               long arg_align, int if_clause, unsigned flags, void **depend, int priority)
{
    gomp_run_task(fn, data, cpyfn, arg_size, arg_align, if_clause, flags, depend, priority);
}

#define ALTMY my2_

#include "wrappedlib_init.h"
