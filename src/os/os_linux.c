#include <sys/syscall.h>
#include <unistd.h>

int GetTID()
{
    return syscall(SYS_gettid);
}
