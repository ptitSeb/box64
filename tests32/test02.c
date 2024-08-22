#include <sys/syscall.h> 
#include <unistd.h>                                      
                                                                                
int main(int argc, char **argv)                                                 
{                                                                               
    const char msg[] = "Hello World!\n";
    syscall(4, STDOUT_FILENO, msg, sizeof(msg)-1);
    return 0;                                                                   
}
