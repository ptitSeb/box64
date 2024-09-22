#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

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

EXPORT int my32_sem_close(void** sem)
{
    int ret = 0;
    ret = sem_close(*sem);
    box_free(sem);
    return ret;
}
EXPORT int my32_sem_destroy(void** sem)
{
    int ret = 0;
    ret = sem_destroy(*sem);
    box_free(*sem);
    *sem = NULL;
    return ret;
}
EXPORT int my32_sem_getvalue(void** sem, int* val)
{
    int ret = 0;
    ret = sem_getvalue(*sem, val);
    box_free(*sem);
    *sem = NULL;
    return ret;
}
EXPORT int my32_sem_init(void** sem, int pshared, uint32_t val)
{
    int ret = 0;
    *sem = box_calloc(1, sizeof(sem_t));
    ret = sem_init(*sem, pshared, val);
    return ret;
}
EXPORT void* my32_sem_open(const char* name, int flags)
{
    sem_t* sem = sem_open(name, flags);
    if(!sem)
        return sem;
    void** ret = (void**)box_calloc(1, sizeof(void*));
    *ret = sem;
    return ret;
}
EXPORT int my32_sem_post(void** sem)
{
    return sem_post(*sem);
}
EXPORT int my32_sem_timedwait(void** sem, void* t)
{
    return sem_timedwait(*sem, t);
}
EXPORT int my32_sem_trywait(void** sem)
{
    return sem_trywait(*sem);
}
EXPORT int my32_sem_wait(void** sem)
{
    return sem_wait(*sem);
}



#define PRE_INIT\
    if(1)                                                       \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);     \
    else

#include "wrappedlib_init32.h"

