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

#define SIGTRAP 5
#endif

#endif // __MYSIGNAL_H_