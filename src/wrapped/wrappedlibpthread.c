#define __USE_UNIX98
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "box64context.h"
#include "librarian.h"

const char* libpthreadName = "libpthread.so.0";
#define ALTNAME "libc.so"

#define LIBNAME libpthread

//EXPORT int my_pthread_attr_setschedparam(x64emu_t* emu, void* attr, void* param)
//{
//    int policy;
//    pthread_attr_getschedpolicy(attr, &policy);
//    int pmin = sched_get_priority_min(policy);
//    int pmax = sched_get_priority_max(policy);
//    if(param) {
//        int p = *(int*)param;
//        if(p>=pmin && p<=pmax)
//            return pthread_attr_setschedparam(attr, param);
//    }
//    printf_log(LOG_INFO, "Warning, call to pthread_attr_setschedparam(%p, %p[%d]) ignored\n", attr, param, param?(*(int*)param):-1);
//    return 0;   // faking success
//}

EXPORT int32_t my_pthread_atfork(x64emu_t *emu, void* prepare, void* parent, void* child)
{
    (void)emu;
    // this is partly incorrect, because the emulated functions should be executed by actual fork and not by my_atfork...
    if(my_context->atfork_sz==my_context->atfork_cap) {
        my_context->atfork_cap += 4;
        my_context->atforks = (atfork_fnc_t*)box_realloc(my_context->atforks, my_context->atfork_cap*sizeof(atfork_fnc_t));
    }
    int i = my_context->atfork_sz++;
    my_context->atforks[i].prepare = (uintptr_t)prepare;
    my_context->atforks[i].parent = (uintptr_t)parent;
    my_context->atforks[i].child = (uintptr_t)child;
    my_context->atforks[i].handle = NULL;
    
    return 0;
}
EXPORT int32_t my___pthread_atfork(x64emu_t *emu, void* prepare, void* parent, void* child) __attribute__((alias("my_pthread_atfork")));

EXPORT void my___pthread_initialize()
{
    // nothing, the lib initialize itself now
}

#ifdef STATICBUILD
#include <semaphore.h>
#include "libtools/static_threads.h"

extern void* __pthread_getspecific(size_t);
extern int __pthread_mutex_destroy(void*);
extern int __pthread_mutex_lock(void*);
extern int __pthread_mutex_trylock(void*);
extern int __pthread_mutex_unlock(void*);
extern int __pthread_rwlock_init(void*, void*);
extern int __pthread_rwlock_rdlock(void*);
extern int __pthread_rwlock_unlock(void*);
extern int __pthread_rwlock_wrlock(void*);
extern int __pthread_setspecific(size_t, void*);
extern int pthread_sigmask(int, void*, void*);
#endif

#include "wrappedlib_init.h"
