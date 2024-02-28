#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
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
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "emu/x87emu_private.h"
#include "x64trace.h"
#include "signals.h"
#include "dynarec_la64.h"
#include "dynarec_la64_private.h"
#include "dynarec_la64_functions.h"
#include "custommem.h"
#include "bridge.h"

// Reset scratch regs counter
void fpu_reset_scratch(dynarec_la64_t* dyn)
{
    // TODO
}

void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex)
{
    if (box64_dynarec_dump) {
        printf_x64_instruction(rex.is32bits ? my_context->dec32 : my_context->dec, &dyn->insts[ninst].x64, name);
        dynarec_log(LOG_NONE, "%s%p: %d emitted opcodes, inst=%d, barrier=%d state=%d/%d(%d), %s=%X/%X, use=%X, need=%X/%X, sm=%d/%d",
            (box64_dynarec_dump > 1) ? "\e[32m" : "",
            (void*)(dyn->native_start + dyn->insts[ninst].address),
            dyn->insts[ninst].size / 4,
            ninst,
            dyn->insts[ninst].x64.barrier,
            dyn->insts[ninst].x64.state_flags,
            dyn->f.pending,
            dyn->f.dfnone,
            dyn->insts[ninst].x64.may_set ? "may" : "set",
            dyn->insts[ninst].x64.set_flags,
            dyn->insts[ninst].x64.gen_flags,
            dyn->insts[ninst].x64.use_flags,
            dyn->insts[ninst].x64.need_before,
            dyn->insts[ninst].x64.need_after,
            dyn->smread, dyn->smwrite);
        if (dyn->insts[ninst].pred_sz) {
            dynarec_log(LOG_NONE, ", pred=");
            for (int ii = 0; ii < dyn->insts[ninst].pred_sz; ++ii)
                dynarec_log(LOG_NONE, "%s%d", ii ? "/" : "", dyn->insts[ninst].pred[ii]);
        }
        if (dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts >= 0)
            dynarec_log(LOG_NONE, ", jmp=%d", dyn->insts[ninst].x64.jmp_insts);
        if (dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts == -1)
            dynarec_log(LOG_NONE, ", jmp=out");
        if (dyn->last_ip)
            dynarec_log(LOG_NONE, ", last_ip=%p", (void*)dyn->last_ip);
        // for (int ii = 0; ii < 24; ++ii) {
        //     switch (dyn->insts[ninst].n.neoncache[ii].t) {
        //         case NEON_CACHE_ST_D: dynarec_log(LOG_NONE, " D%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
        //         case NEON_CACHE_ST_F: dynarec_log(LOG_NONE, " S%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
        //         case NEON_CACHE_ST_I64: dynarec_log(LOG_NONE, " D%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
        //         case NEON_CACHE_MM: dynarec_log(LOG_NONE, " D%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
        //         case NEON_CACHE_XMMW: dynarec_log(LOG_NONE, " Q%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
        //         case NEON_CACHE_XMMR: dynarec_log(LOG_NONE, " Q%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
        //         case NEON_CACHE_SCR: dynarec_log(LOG_NONE, " D%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
        //         case NEON_CACHE_NONE:
        //         default: break;
        //     }
        // }
        // if (dyn->n.stack || dyn->insts[ninst].n.stack_next || dyn->insts[ninst].n.x87stack)
        //     dynarec_log(LOG_NONE, " X87:%d/%d(+%d/-%d)%d", dyn->n.stack, dyn->insts[ninst].n.stack_next, dyn->insts[ninst].n.stack_push, dyn->insts[ninst].n.stack_pop, dyn->insts[ninst].n.x87stack);
        // if (dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2)
        //     dynarec_log(LOG_NONE, " %s:%d/%d", dyn->insts[ninst].n.swapped ? "SWP" : "CMB", dyn->insts[ninst].n.combined1, dyn->insts[ninst].n.combined2);
        dynarec_log(LOG_NONE, "%s\n", (box64_dynarec_dump > 1) ? "\e[m" : "");
    }
}

// CAS
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

uint8_t la64_lock_xchg_b(void* addr, uint8_t val)
{
    uint32_t ret;
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    do {
        ret = *aligned;
    } while(la64_lock_cas_d(aligned, ret, insert_byte(ret, val, addr)));
    return extract_byte(ret, addr);
}

uint16_t la64_lock_xchg_h(void* addr, uint16_t val)
{
    uint32_t ret;
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    do {
        ret = *aligned;
    } while(la64_lock_cas_d(aligned, ret, insert_half(ret, val, addr)));
    return extract_half(ret, addr);
}

int la64_lock_cas_b(void* addr, uint8_t ref, uint8_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    uint32_t tmp = *aligned;
    return la64_lock_cas_d(aligned, ref, insert_byte(tmp, val, addr));
}

int la64_lock_cas_h(void* addr, uint16_t ref, uint16_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    uint32_t tmp = *aligned;
    return la64_lock_cas_d(aligned, ref, insert_half(tmp, val, addr));
}

void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode)
{
    dynarec_log(LOG_NONE, "\t%08x\t%s\n", opcode, la64_print(opcode, (uintptr_t)dyn->block));
}

void fpu_reset(dynarec_la64_t* dyn)
{
    // TODO
}

void fpu_reset_ninst(dynarec_la64_t* dyn, int ninst)
{
    // TODO
}