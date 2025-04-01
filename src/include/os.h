#ifndef __OS_H_
#define __OS_H_

int GetTID(void);
int SchedYield(void);

#ifndef _WIN32
#include <setjmp.h>
#define LongJmp longjmp
#define SigSetJmp sigsetjmp
#else
#define LongJmp(a, b)
#define SigSetJmp(a, b) 0
#endif

#ifndef USE_CUSTOM_MUTEX
#define mutex_lock(A)    pthread_mutex_lock(A)
#define mutex_trylock(A) pthread_mutex_trylock(A)
#define mutex_unlock(A)  pthread_mutex_unlock(A)
#else
#define mutex_lock(A)                             \
    do {                                          \
        uint32_t tid = (uint32_t)GetTID();        \
        while (native_lock_storeifnull_d(A, tid)) \
            sched_yield();                        \
    } while (0)
#define mutex_trylock(A) native_lock_storeifnull_d(A, (uint32_t)GetTID())
#define mutex_unlock(A)  native_lock_storeifref_d(A, 0, (uint32_t)GetTID())
#endif

#ifndef _WIN32
#include <setjmp.h>
#define NEW_JUMPBUFF(name) \
    static __thread JUMPBUFF name
#ifdef ANDROID
#define JUMPBUFF sigjmp_buf
#define GET_JUMPBUFF(name) name
#else
#define JUMPBUFF struct __jmp_buf_tag
#define GET_JUMPBUFF(name) &name
#endif
#else
#define JUMPBUFF int
#define NEW_JUMPBUFF(name)
#define GET_JUMPBUFF(name) NULL
#endif

#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4

#endif //__OS_H_
