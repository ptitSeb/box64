#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "tools/bridge_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "emu/x87emu_private.h"
#include "x64trace.h"
#include "signals.h"
#include "dynarec_rv64.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "custommem.h"
#include "bridge.h"
#include "rv64_lock.h"


void fpu_reset_scratch(dynarec_rv64_t* dyn)
{
    //TODO
}

uint8_t extract_byte(uint32_t val, void* address){
    int idx = (((uintptr_t)address)&3)*8;
    return (val>>idx)&0xff;
}
uint32_t insert_byte(uint32_t val, uint8_t b, void* address){
    int idx = (((uintptr_t)address)&3)*8;
    val&=~(0xff<<idx);
    val|=(((uint32_t)b)<<idx);
    return val;
}

// will go badly if address is unaligned
uint16_t extract_half(uint32_t val, void* address){
    int idx = (((uintptr_t)address)&3)*8;
    return (val>>idx)&0xffff;
}
uint32_t insert_half(uint32_t val, uint16_t h, void* address){
    int idx = (((uintptr_t)address)&3)*8;
    val&=~(0xffff<<idx);
    val|=(((uint32_t)h)<<idx);
    return val;
}

uint8_t rv64_lock_xchg_b(void* addr, uint8_t val)
{
    uint32_t ret;
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    do {
        ret = *aligned;
    } while(rv64_lock_cas_d(aligned, ret, insert_byte(ret, val, addr)));
    return extract_byte(ret, addr);
}

int rv64_lock_cas_b(void* addr, uint8_t ref, uint8_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    uint32_t tmp = *aligned;
    return rv64_lock_cas_d(aligned, tmp, insert_byte(tmp, val, addr));
}

int rv64_lock_cas_h(void* addr, uint16_t ref, uint16_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    uint32_t tmp = *aligned;
    return rv64_lock_cas_d(aligned, tmp, insert_half(tmp, val, addr));
}