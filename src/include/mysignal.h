#ifndef __MYSIGNAL_H_
#define __MYSIGNAL_H_

#ifndef _WIN32
#include <signal.h>
#else
typedef struct {
    int a;
} sigset_t;

typedef sigset_t __sigset_t;

#define sigfillset(x)

#define SIGILL  4
#define SIGTRAP 5
#define SIGSEGV 11

#define pthread_sigmask(a, b, c) 0
#endif

#endif // __MYSIGNAL_H_