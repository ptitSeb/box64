#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper32.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "box32context.h"
#include "librarian.h"

static const char* libpthreadName = "libpthread.so.0";
#define LIBNAME libpthread

EXPORT int my32_pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
    return pthread_rwlock_wrlock(rwlock);
}
EXPORT int my32_pthread_rwlock_rdlock(pthread_rwlock_t* rwlock)
{
    return pthread_rwlock_rdlock(rwlock);
}
EXPORT int my32_pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
    return pthread_rwlock_unlock(rwlock);
}

EXPORT int32_t my32_pthread_atfork(x64emu_t *emu, void* prepare, void* parent, void* child)
{
    // this is partly incorrect, because the emulated functions should be executed by actual fork and not by my32_atfork...
    if(my_context->atfork_sz==my_context->atfork_cap) {
        my_context->atfork_cap += 4;
        my_context->atforks = (atfork_fnc_t*)realloc(my_context->atforks, my_context->atfork_cap*sizeof(atfork_fnc_t));
    }
    int i = my_context->atfork_sz++;
    my_context->atforks[i].prepare = (uintptr_t)prepare;
    my_context->atforks[i].parent = (uintptr_t)parent;
    my_context->atforks[i].child = (uintptr_t)child;
    my_context->atforks[i].handle = NULL;
    
    return 0;
}
EXPORT int32_t my32___pthread_atfork(x64emu_t *emu, void* prepare, void* parent, void* child) __attribute__((alias("my32_pthread_atfork")));

EXPORT void my32___pthread_initialize()
{
    // nothing, the lib initialize itself now
}

#define PRE_INIT\
    if(1)                                                       \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);     \
    else

#include "wrappedlib_init32.h"

