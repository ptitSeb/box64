#include <sys/syscall.h>
#include <sched.h>
#include <unistd.h>

#include "os.h"

int GetTID(void)
{
    return syscall(SYS_gettid);
}

int SchedYield(void)
{
    return sched_yield();
}
