#include <sys/syscall.h>
#include <unistd.h>
// build with `gcc test01.c -o test01 -march=core2 -Wl,--hash-style=both`
int main(int argc, char **argv)
{
    const char msg[] = "Hello x86_64 World!\n";
    //syscall(1, STDOUT_FILENO, msg, sizeof(msg)-1);
    asm (
        "mov $1, %%rax \n"
        "mov $1, %%rdi \n"
        "mov %0, %%rsi \n"
        "mov $20, %%rdx \n"
        "syscall \n"
    :
    :"r" (msg)
    :"%rax","%rdi","%rsi","%rdx"
    );
    return 0;
}
