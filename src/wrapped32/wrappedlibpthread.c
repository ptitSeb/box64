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
        my_context->atforks = (atfork_fnc_t*)box_realloc(my_context->atforks, my_context->atfork_cap*sizeof(atfork_fnc_t));
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

#define SEM_SIGN 0x542554aabc123578LL     // just some random stuff
typedef struct my_sem_32_s {
    uint64_t sign;
    sem_t   *sem;
} my_sem_32_t;

static sem_t* get_sem(void* sem)
{
    if(!(((uintptr_t)sem)&0x07))
        return sem;
    // sem is unaligned, create a wrapped sem
    sem_t *new_sem = box_calloc(1, sizeof(sem_t));
    memcpy(new_sem, sem, sizeof(sem_t));
    my_sem_32_t* old = sem;
    old->sign = SEM_SIGN;
    old->sem = new_sem;
    return new_sem;
}

#define GET_SEM(sem) sem_t* _sem = (sem->sign != SEM_SIGN)?(get_sem(sem)):(sem->sem)
EXPORT int my32_sem_close(sem_t* sem)
{
    return sem_close(sem);
}
EXPORT int my32_sem_destroy(my_sem_32_t* sem)
{
    if(sem->sign != SEM_SIGN)
        return sem_destroy((sem_t*)sem);
    int ret = 0;
    ret = sem_destroy(sem->sem);
    box_free(sem->sem);
    sem->sem = NULL;
    return ret;
}
EXPORT int my32_sem_getvalue(my_sem_32_t* sem, int* val)
{
    GET_SEM(sem);
    return sem_getvalue(_sem, val);
}
EXPORT int my32_sem_init(my_sem_32_t* sem, int pshared, uint32_t val)
{
    int ret = 0;
    sem->sign = SEM_SIGN;
    sem->sem = box_calloc(1, sizeof(sem_t));
    ret = sem_init(sem->sem, pshared, val);
    return ret;
}
EXPORT void* my32_sem_open(const char* name, int flags)
{
    return sem_open(name, flags);
}
EXPORT int my32_sem_post(my_sem_32_t* sem)
{
    GET_SEM(sem);
    return sem_post(_sem);
}
EXPORT int my32_sem_timedwait(my_sem_32_t* sem, struct timespec * t)
{
    GET_SEM(sem);
    // Not sure it's usefull
    //if(!sem_trywait(_sem)))
    //    return 0;
    // some x86 game are not computing timeout correctly (ex: Anomaly Warzone Earth linux version)
    while(t->tv_nsec>=1000000000) {
        t->tv_nsec-=1000000000;
        t->tv_sec+=1;
    }
    return sem_timedwait(_sem, t);
}
EXPORT int my32_sem_trywait(my_sem_32_t* sem)
{
    GET_SEM(sem);
    return sem_trywait(_sem);
}
EXPORT int my32_sem_wait(my_sem_32_t* sem)
{
    GET_SEM(sem);
    return sem_wait(_sem);
}



#define PRE_INIT\
    if(1)                                                       \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);     \
    else

#include "wrappedlib_init32.h"

