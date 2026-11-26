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
#include "emu/x64emu_private.h"
#include "gtkclass.h"
#include "myalign.h"

const char* gompName = "libgomp.so.1";
#define LIBNAME gomp

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedgomptypes.h"

#include "wrappercallback.h"

#include "wrappedgomp_common.h"

EXPORT void my_GOMP_parallel(x64emu_t* emu, void* f, void* data, unsigned num_threads, uint32_t flags)
{
    gomp_run_parallel(f, data, num_threads, flags);
}

EXPORT void my_GOMP_task(x64emu_t* emu, void* fn, void *data, void* cpyfn, long arg_size,
               long arg_align, int if_clause, unsigned flags, void **depend, int priority)
{
    gomp_run_task(fn, data, cpyfn, arg_size, arg_align, if_clause, flags, depend, priority);
}

#include "wrappedlib_init.h"
