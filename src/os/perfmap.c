#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "debug.h"
#include "box64context.h"
#include "perfmap.h"

#ifndef _WIN32
#include "elfloader.h"

void writePerfMap(uintptr_t func_addr, uintptr_t code_addr, size_t code_size, const char* inst_name)
{
    char pbuf[128];
    uint64_t sz = 0;
    uintptr_t start = 0;
    const char* symbname = FindNearestSymbolName(FindElfAddress(my_context, func_addr), (void*)func_addr, &start, &sz);
    if(!symbname || !strcmp(symbname, "???"))
        snprintf(pbuf, sizeof(pbuf), "0x%lx %ld %p:%s\n", code_addr, code_size, func_addr, inst_name);
    else
        snprintf(pbuf, sizeof(pbuf), "0x%lx %ld %s:%s\n", code_addr, code_size, symbname, inst_name);
    (void)!write(BOX64ENV(dynarec_perf_map_fd), pbuf, strlen(pbuf));
}
#else
void writePerfMap(uintptr_t func_addr, uintptr_t code_addr, size_t code_size, const char* inst_name) { }
#endif