// SPDX-License-Identifier: MIT
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <inttypes.h>

#include "debug.h"
#include "box64context.h"
#include "perfmap.h"

#ifndef _WIN32
#include "elfloader.h"

void writePerfMap(uintptr_t func_addr, uintptr_t code_addr, size_t code_size, const char* inst_name)
{
    char pbuf[1024];
    const char* symbname = getAddrFunctionName(func_addr);
    int n;
    if(!symbname || !strcmp(symbname, "???"))
        n = snprintf(pbuf, sizeof(pbuf), "0x%" PRIx64 " %" PRId64 " 0x%" PRIx64 ":%s\n", code_addr, code_size, func_addr, inst_name);
    else
        n = snprintf(pbuf, sizeof(pbuf), "0x%" PRIx64 " %" PRId64 " %s:%s\n", code_addr, code_size, symbname, inst_name);
    if(n < 0) return;
    if((size_t)n >= sizeof(pbuf)) n = sizeof(pbuf) - 1;
    pbuf[n - 1] = '\n';
    (void)!write(BOX64ENV(dynarec_perf_map_fd), pbuf, n);
}
#else
void writePerfMap(uintptr_t func_addr, uintptr_t code_addr, size_t code_size, const char* inst_name) { }
#endif