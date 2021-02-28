#include <sys/syscall.h> 
#include <unistd.h>                                      
                                                                                
int main(int argc, char **argv)                                                 
{                                                                               
    const char msg[] = "Hello x86_64 World!\n";
    syscall(1, STDOUT_FILENO, msg, sizeof(msg)-1);
    return 0;                                                                   
}
