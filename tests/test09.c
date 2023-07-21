#include <stdio.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
  
void forkexample() 
{ 
    int x = 1; 
  
    if (fork() == 0) 
        printf("Child has x = %d\n", ++x); 
    else {
        wait(NULL);
        printf("Parent has x = %d\n", --x); 
    }
} 
int main() 
{ 
    forkexample(); 
    return 0; 
} 
