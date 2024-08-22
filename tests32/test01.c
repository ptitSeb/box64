#include <sys/syscall.h> 
#include <unistd.h>                                      
                                                                                
int main(int argc, char **argv)                                                 
{                                                                               
    const char msg[] = "Hello World!\n";
    //syscall(4, STDOUT_FILENO, msg, sizeof(msg)-1);
    asm (
        "movl $4, %%eax \n"
        "movl $1, %%ebx \n"
        "movl %0, %%ecx \n"
        "movl $13, %%edx \n"
        "int $0x80 \n"
    :
    :"r" (msg)
    :"%eax","%ebx","%ecx","%edx"
    );
    return 0;                                                                   
}
