#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>
#include <stddef.h>
#include <stdarg.h>
#include <fts.h>

#include "box64context.h"
#include "debug.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "box64stack.h"
#include "auxval.h"

static uintptr_t* auxval_start = NULL;

int init_auxval(int argc, const char **argv, char **env) {
    (void)argc; (void)argv;

    // auxval vector is after envs...
    while(*env)
        env++;
    auxval_start = (uintptr_t*)(env+1);
    return 0;
}

#ifdef BUILD_LIB
__attribute__((section(".init_array"))) static void *init_auxval_constructor = &init_auxval;
#endif

unsigned long real_getauxval(unsigned long type)
{
    if(!auxval_start)
        return 0;
    uintptr_t* p = auxval_start;
    while(*p) {
        if(*p == type)
            return p[1];
        p+=2;
    }
    return 0;
}

#ifdef BOX32
EXPORT unsigned long my32_getauxval(x64emu_t* emu, unsigned long type)
{
    ptr_t* p = (ptr_t*)emu->context->auxval_start;
    while(*p) {
        if(*p == type)
            return p[1];
        p+=2;
    }
    return 0;
}
#endif

EXPORT unsigned long my_getauxval(x64emu_t* emu, unsigned long type)
{
    #ifdef BOX32
    if(box64_is32bits)
        return my32_getauxval(emu, type);
    #endif
    uintptr_t* p = emu->context->auxval_start;
    while(*p) {
        if(*p == type)
            return p[1];
        p+=2;
    }
    return 0;
}
