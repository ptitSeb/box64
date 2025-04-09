#ifndef __MYSIGNAL_H_
#define __MYSIGNAL_H_

#ifndef _WIN32
#include <signal.h>
#else
typedef struct {
    int a;
} sigset_t;

#define sigfillset(x)
#define pthread_sigmask(a, b, c)
#endif

#endif // __MYSIGNAL_H_