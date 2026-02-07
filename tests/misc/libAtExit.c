#include <stdio.h>
#include <stdlib.h>

#define EXPORT __attribute__((visibility("default")))

static void at_exit_handler(void)
{
    printf("In at_exit_handler\n");
}

EXPORT int JNI_OnLoad(void* vm, void* reserved)
{
    printf("JNI_OnLoad: registering atexit handler vm=%p reserved=%p\n", vm, reserved);
    atexit(at_exit_handler);
    return 0;
}
