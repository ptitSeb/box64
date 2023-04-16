#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "debug.h"
#include "myalign.h"
#include "callback.h"
#include "emu/x64emu_private.h"

// Fake the lib for now, don't load it
const char* tbbbindName = "libtbbbind_2_5.so";
#define LIBNAME tbbbind

EXPORT void my___TBB_internal_initialize_system_topology(size_t group, void* numa_code, void* numa_list, void* core_type, void* core_list)
{ }

EXPORT void* my___TBB_internal_allocate_binding_handler(int slot_num, int numa_id, int core_type_id, int max_threads_per_core)
{
    // dummy
    return NULL;
}

EXPORT void my___TBB_internal_apply_affinity(void* handler, int slot)
{ }

EXPORT void my___TBB_internal_deallocate_binding_handler(void* handler)
{ }

EXPORT void my___TBB_internal_destroy_system_topology()
{ }

int getNCpu();
EXPORT int my___TBB_internal_get_default_concurrency(int numa_id, int core_type, int max_threads_per_core)
{
    return getNCpu();
}

EXPORT void my___TBB_internal_restore_affinity(void* handler, int slot)
{ }

// this preinit basically open "box64" as dlopen
#define PRE_INIT\
    lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    if(0)

#include "wrappedlib_init.h"
