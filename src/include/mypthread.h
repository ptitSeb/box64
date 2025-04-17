#ifndef __MYPTHREAD_H_
#define __MYPTHREAD_H_

#ifndef _WIN32
#include <pthread.h>
#else
#include <windows.h>

NTSTATUS  WINAPI RtlEnterCriticalSection(RTL_CRITICAL_SECTION *);
NTSTATUS  WINAPI RtlLeaveCriticalSection(RTL_CRITICAL_SECTION *);
BOOL      WINAPI RtlTryEnterCriticalSection(RTL_CRITICAL_SECTION *);
NTSTATUS  WINAPI RtlInitializeCriticalSection(RTL_CRITICAL_SECTION *);

typedef void* pthread_key_t;
typedef void* pthread_mutexattr_t;
#define pthread_mutex_t RTL_CRITICAL_SECTION
#define pthread_mutex_init(x, y) RtlInitializeCriticalSection(x)
#define pthread_mutex_lock(x) RtlEnterCriticalSection(x)
#define pthread_mutex_unlock(x) RtlLeaveCriticalSection(x)
#define pthread_mutex_trylock(x) !RtlTryEnterCriticalSection(x)

#define pthread_mutex_destroy(x) 0
#define pthread_mutexattr_init(x) 0
#define pthread_mutexattr_destroy(x) 0
#define pthread_mutexattr_settype(x, y) 0
#define pthread_atfork(a, b, c) 0
#endif

#endif // __MYPTHREAD_H_