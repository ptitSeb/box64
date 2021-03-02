#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "box64stack.h"
#include "box64context.h"
#include "elfloader.h"
#include "debug.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "auxval.h"

EXPORTDYN
int CalcStackSize(box64context_t *context)
{
    printf_log(LOG_DEBUG, "Calc stack size, based on %d elf(s)\n", context->elfsize);
    context->stacksz = 8*1024*1024; context->stackalign=4;
    for (int i=0; i<context->elfsize; ++i)
        CalcStack(context->elfs[i], &context->stacksz, &context->stackalign);

    if (posix_memalign((void**)&context->stack, context->stackalign, context->stacksz)) {
        printf_log(LOG_NONE, "Cannot allocate aligned memory (0x%x/0x%x) for stack\n", context->stacksz, context->stackalign);
        return 1;
    }
    memset(context->stack, 0, context->stacksz);
    printf_log(LOG_DEBUG, "Stack is @%p size=0x%x align=0x%x\n", context->stack, context->stacksz, context->stackalign);

    return 0;
}

uint16_t Pop16(x64emu_t *emu)
{
    uint16_t* st = ((uint16_t*)(R_RSP));
    R_RSP += 2;
    return *st;
}

void Push16(x64emu_t *emu, uint16_t v)
{
    R_RSP -= 2;
    *((uint16_t*)R_RSP) = v;
}

uint32_t Pop32(x64emu_t *emu)
{
    uint32_t* st = ((uint32_t*)(R_RSP));
    R_RSP += 4;
    return *st;
}

void Push32(x64emu_t *emu, uint32_t v)
{
    R_RSP -= 4;
    *((uint32_t*)R_RSP) = v;
}

uint64_t Pop64(x64emu_t *emu)
{
    uint64_t* st = ((uint64_t*)(R_RSP));
    R_RSP += 8;
    return *st;
}

void Push64(x64emu_t *emu, uint64_t v)
{
    R_RSP -= 8;
    *((uint64_t*)R_RSP) = v;
}

void PushString(x64emu_t *emu, const char* s)
{
    int sz = strlen(s) + 1;
    // round to 4 bytes boundary
    R_RSP -= sz;
    memcpy((void*)R_RSP, s, sz);
}

EXPORTDYN
void SetupInitialStack(x64emu_t *emu)
{
    // start with 0
    Push(emu, 0);
    // push program executed
    PushString(emu, emu->context->argv[0]);
    uintptr_t p_arg0 = R_RSP;
    // push envs
    uintptr_t p_envv[emu->context->envc];
    for (int i=emu->context->envc-1; i>=0; --i) {
        PushString(emu, emu->context->envv[i]);
        p_envv[i] = R_RSP;
    }
    // push args, also, free the argv[] string and point to the one in the main stack
    uintptr_t p_argv[emu->context->argc];
    for (int i=emu->context->argc-1; i>=0; --i) {
        PushString(emu, emu->context->argv[i]);
        p_argv[i] = R_RSP;
        free(emu->context->argv[i]);
        emu->context->argv[i] = (char*)p_argv[i];
    }
    // align
    uintptr_t tmp = (R_RSP)&~(emu->context->stackalign-1);
    memset((void*)tmp, 0, R_RSP-tmp);
    R_RSP=tmp;

    // push some AuxVector stuffs
    PushString(emu, "x86_64");
    uintptr_t p_x86_64 = R_RSP;
    uintptr_t p_random = real_getauxval(25);
    if(!p_random) {
        for (int i=0; i<4; ++i)
            Push(emu, random());
        p_random = R_RSP;
    }
    // align
    tmp = (R_RSP)&~(emu->context->stackalign-1);
    memset((void*)tmp, 0, R_RSP-tmp);
    R_RSP=tmp;

    // push the AuxVector themselves
    Push(emu, 0); Push(emu, 0);             //AT_NULL(0)=0
    Push(emu, p_x86_64); Push(emu, 15);     //AT_PLATFORM(15)=p_x86_64*
    Push(emu, 0); Push(emu, 66);            //AT_HWCAP2(26)=0
    // Push HWCAP:
    //  FPU: 1<<0 ; VME: 1<<1 ; DE : 1<<2 ; PSE: 1<<3 ; TSC: 1<<4
    //  MSR: 1<<5 : PAE: 1<<6 : MCE: 1<<7 ; CX8: 1<<8 : APIC:1<<9
    //  SEP: 1<<11: MTRR:1<<12: PGE: 1<<13: MCA: 1<<14; CMOV:1<<15; FCMOV: 1<<16
    //  MMX: 1<<23:OSFXR:1<<24: XMM: 1<<25:XMM2: 1<<26;AMD3D:1<<31
    Push(emu, (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<8)  | (1<<15) | (1<<16) | (1<<23) | (1<<25) | (1<<26));
    Push(emu, 16);                      //AT_HWCAP(16)=...
    Push(emu, p_arg0); Push(emu, 31);   //AT_EXECFN(31)=p_arg0
    Push(emu, p_random); Push(emu, 25); //AT_RANDOM(25)=p_random
    Push(emu, real_getauxval(23)); Push(emu, 23);     //AT_SECURE(23)=0
    Push(emu, real_getauxval(14)); Push(emu, 14);     //AT_EGID(14)
    Push(emu, real_getauxval(13)); Push(emu, 13);     //AT_GID(13)
    Push(emu, real_getauxval(12)); Push(emu, 12);     //AT_EUID(12)
    Push(emu, real_getauxval(11)); Push(emu, 11);     //AT_UID(11)
    Push(emu, box64_pagesize); Push(emu, 6);          //AT_PAGESZ(6)
    Push(emu, R_RIP); Push(emu, 9);     //AT_ENTRY(9)=entrypoint
    Push(emu, 0/*emu->context->vsyscall*/); Push(emu, 32);      //AT_SYSINFO(32)=vsyscall
    if(!emu->context->auxval_start)       // store auxval start if needed
        emu->context->auxval_start = (uintptr_t*)R_RSP;
    // TODO: continue

    // push nil / envs / nil / args / argc
    Push(emu, 0);
    for (int i=emu->context->envc-1; i>=0; --i)
        Push(emu, p_envv[i]);
    Push(emu, 0);
    for (int i=emu->context->argc-1; i>=0; --i)
        Push(emu, p_argv[i]);
    Push(emu, emu->context->argc);
}