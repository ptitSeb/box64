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
#define LIBNAME libpthread


//int my_pthread_create(x64emu_t *emu, void* t, void* attr, void* start_routine, void* arg); //implemented in thread.c
//int my_pthread_key_create(x64emu_t* emu, void* key, void* dtor);
//int my___pthread_key_create(x64emu_t* emu, void* key, void* dtor);
//int my_pthread_once(x64emu_t* emu, void* once, void* cb);
//int my___pthread_once(x64emu_t* emu, void* once, void* cb);
//int my_pthread_cond_broadcast(x64emu_t* emu, void* cond);
//int my_pthread_cond_destroy(x64emu_t* emu, void* cond);
//int my_pthread_cond_init(x64emu_t* emu, void* cond, void* attr);
//int my_pthread_cond_signal(x64emu_t* emu, void* cond);
//int my_pthread_cond_timedwait(x64emu_t* emu, void* cond, void* mutex, void* abstime);
//int my_pthread_cond_wait(x64emu_t* emu, void* cond, void* mutex);
//int my_pthread_mutexattr_setkind_np(x64emu_t* emu, void* t, int kind);
//int my_pthread_attr_setscope(x64emu_t* emu, void* attr, int scope);
//void my__pthread_cleanup_push_defer(x64emu_t* emu, void* buffer, void* routine, void* arg);
//void my__pthread_cleanup_push(x64emu_t* emu, void* buffer, void* routine, void* arg);
//void my__pthread_cleanup_pop(x64emu_t* emu, void* buffer, int exec);
//void my__pthread_cleanup_pop_restore(x64emu_t* emu, void* buffer, int exec);
//int my_pthread_kill(x64emu_t* emu, void* thread, int sig);
//int my_pthread_getaffinity_np(x64emu_t* emu, pthread_t thread, int cpusetsize, void* cpuset);
//int my_pthread_setaffinity_np(x64emu_t* emu, pthread_t thread, int cpusetsize, void* cpuset);
//int my_pthread_attr_setaffinity_np(x64emu_t* emu, void* attr, uint32_t cpusetsize, void* cpuset);


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
        my_context->atforks = (atfork_fnc_t*)realloc(my_context->atforks, my_context->atfork_cap*sizeof(atfork_fnc_t));
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

#include "wrappedlib_init.h"
