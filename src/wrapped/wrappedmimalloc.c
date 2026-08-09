#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <malloc.h>
#include <sys/resource.h>

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
// Fake the lib for now, don't load it
const char* mimallocName = "libmimalloc.so.3";
#define LIBNAME mimalloc

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedmimalloctypes.h"

#include "wrappercallback.h"

static size_t pot(size_t l) {
    size_t ret = 0;
    while (l>(1u<<ret))  ++ret;
    return 1u<<ret;
}

static int ispot(size_t l) {
    return pot(l)==l;
}

EXPORT void* my_mi_malloc_aligned(size_t size, size_t align)
{
    if(!align) return NULL;
    if(!ispot(align)) return NULL;
    return memalign(align, size);
}

EXPORT void* my_mi_realloc_aligned(void* p, size_t newsize, size_t align)
{
    if(!align) return NULL;
    if(!ispot(align)) return NULL;
    size_t size = malloc_usable_size(p);
    if((size>=newsize) && !((uintptr_t)p&(align-1)))
        return p;
    void* newp = memalign(align, newsize);
    size_t copy_size = (size<newsize)?size:newsize;
    memcpy(newp, p, copy_size);
    free(p);
    return newp;
}

EXPORT void* my_mi_zalloc(size_t size)
{
    return calloc(1, size);
}

EXPORT void my_mi_thread_init()
{
    // nothing
}

EXPORT void my_mi_collect(int force)
{
    // nothing
}

EXPORT void* my_mi_heap_new()
{
    return (void*)1ULL; // arbitrary constant, no separate heap...
}

EXPORT void* my_mi_heap_malloc(void* heap, size_t size)
{
    return malloc(size);
}

EXPORT void my_mi_register_error(void* f, void* arg)
{
    // nothing
}

EXPORT void my_mi_register_output(void* f, void* arg)
{
    // nothing
}

EXPORT void my_mi_heap_destroy(void* heap)
{
    // nothing
}

EXPORT void my_mi_option_set(uint32_t option, long value)
{
    // nothing
}

EXPORT void my_mi_stats_print_out(void* f, void* arg)
{
    // nothing
}

EXPORT void my_mi_process_info(size_t* elapsed_msecs, size_t* user_msecs, size_t* system_msecs, size_t* current_rss, size_t* peak_rss, size_t* current_commit, size_t* peak_commit, size_t* page_faults)
{
    // aproximate output....
    struct rusage rusage = {0};
    int ret = getrusage(RUSAGE_SELF, &rusage);
    *system_msecs = rusage.ru_stime.tv_sec*1000 + rusage.ru_stime.tv_usec/1000;
    *user_msecs = rusage.ru_utime.tv_sec*1000 + rusage.ru_utime.tv_usec/1000;
    *elapsed_msecs = (rusage.ru_stime.tv_sec+rusage.ru_utime.tv_sec)*1000 +(rusage.ru_stime.tv_usec+rusage.ru_utime.tv_usec)/1000;
    *current_rss = rusage.ru_ixrss;
    *peak_rss = rusage.ru_maxrss;
    *current_commit = rusage.ru_nswap;
    *peak_commit = rusage.ru_nswap;
    *page_faults = rusage.ru_majflt;
}

EXPORT void my_mi_process_init()
{
    // nothing
}

// this preinit basically open "box64" as dlopen, libmimalloc will be completly skipped
#define PRE_INIT\
    lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    if(0)

#include "wrappedlib_init.h"
