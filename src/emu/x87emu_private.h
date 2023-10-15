#ifndef __X87RUN_PRIVATE_H_
#define __X87RUN_PRIVATE_H_

#include <stdint.h>
#include <math.h>
#include "regs.h"
#include "x64run_private.h"
#include "debug.h"
typedef struct x64emu_s x64emu_t;

#define PI		3.14159265358979323846
#define L2E		1.4426950408889634
#define L2T		3.3219280948873623
#define LN2		0.69314718055994531
#define LG2		0.3010299956639812

#define ST0 emu->x87[emu->top]
#define ST1 emu->x87[(emu->top+1)&7]
#define ST(a) emu->x87[(emu->top+(a))&7]

#define STld(a)  emu->fpu_ld[(emu->top+(a))&7]
#define STll(a)  emu->fpu_ll[(emu->top+(a))&7]

static inline void fpu_do_push(x64emu_t* emu)
{
    int newtop = (emu->top-1)&7;
    /*if(emu->p_regs[newtop].tag!=0b11) {// not empty, overflow!
        printf_log(LOG_NONE, "Warning: %p: FPU Stack overflow\n", (void*)emu->old_ip);    // probably better to raise something
        //emu->quit = 1;
        return;
    }*/
    if(emu->fpu_stack<8)
        ++emu->fpu_stack; 
    emu->p_regs[newtop].tag = 0;    // full
    emu->top = newtop;
}

static inline void fpu_do_pop(x64emu_t* emu)
{
    int curtop = (emu->top)&7;
    /*if(emu->p_regs[(emu->top)&7].tag==0b11) {// underflow
        printf_log(LOG_NONE, "Warning: %p: FPU Stack underflow\n", (void*)emu->old_ip);    // probably better to raise something
        //emu->quit = 1;
        return;
    }*/
    if(emu->fpu_stack>0)
        --emu->fpu_stack;
    
    emu->p_regs[curtop].tag = 0b11;    // empty
    emu->top = (emu->top+1)&7;
}

void fpu_do_free(x64emu_t* emu, int i);

void reset_fpu(x64emu_t* emu);

static inline void fpu_fcom(x64emu_t* emu, double b)
{
    emu->sw.f.F87_C1 = 0;
    if(isnan(ST0.d) || isnan(b)) {
        emu->sw.f.F87_C0 = 1;
        emu->sw.f.F87_C2 = 1;
        emu->sw.f.F87_C3 = 1;
    } else if (isgreater(ST0.d, b)) {
        emu->sw.f.F87_C0 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 0;
    } else if (isless(ST0.d, b)) {
        emu->sw.f.F87_C0 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 0;
    } 
    else {
        emu->sw.f.F87_C0 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 1;
    }
}

static inline void fpu_fcomi(x64emu_t* emu, double b)
{
    RESET_FLAGS(emu);
    CLEAR_FLAG(F_AF);
    CLEAR_FLAG(F_OF);
    CLEAR_FLAG(F_SF);
    emu->sw.f.F87_C1 = 0;
    if(isnan(ST0.d) || isnan(b)) {
        SET_FLAG(F_CF);
        SET_FLAG(F_PF);
        SET_FLAG(F_ZF);
    } else if (isgreater(ST0.d, b)) {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_PF);
        CLEAR_FLAG(F_ZF);
    } else if (isless(ST0.d, b)) {
        SET_FLAG(F_CF);
        CLEAR_FLAG(F_PF);
        CLEAR_FLAG(F_ZF);
    } 
    else {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_PF);
        SET_FLAG(F_ZF);
    }
}

static inline double fpu_round(x64emu_t* emu, double d) {
    if (!isfinite(d))
        return d;
    switch(emu->cw.f.C87_RD) {
        case ROUND_Nearest:
            return round(d);
        case ROUND_Down:
            return floor(d);
        case ROUND_Up:
            return ceil(d);
        case ROUND_Chop:
        default:
            return trunc(d);
    }
}

static inline void fpu_fxam(x64emu_t* emu) {
    emu->sw.f.F87_C1 = (ST0.ud[1]&0x80000000)?1:0;
    if(!emu->fpu_stack) {
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    if(isinf(ST0.d)) 
    {  // TODO: Unsupported and denormal not analysed...
        emu->sw.f.F87_C3 = 0;
        emu->sw.f.F87_C2 = 1;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    if(isnan(ST0.d))
    {  // TODO: Unsupported and denormal not analysed...
        emu->sw.f.F87_C3 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    if(ST0.d==0.0)
    {
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 0;
        return;
    }
    // normal...
    emu->sw.f.F87_C3 = 0;
    emu->sw.f.F87_C2 = 1;
    emu->sw.f.F87_C0 = 0;

}

static inline void fpu_ftst(x64emu_t* emu) {
    emu->sw.f.F87_C1 = 0;
    if(isinf(ST0.d) || isnan(ST0.d)) 
    {  // TODO: Unsupported and denormal not analysed...
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 1;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    if(ST0.d==0.0)
    {
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 0;
        return;
    }
    // normal...
    emu->sw.f.F87_C3 = 0;
    emu->sw.f.F87_C2 = 0;
    emu->sw.f.F87_C0 = (ST0.ud[1]&0x80000000)?1:0;
}

void fpu_fbst(x64emu_t* emu, uint8_t* d);
void fpu_fbld(x64emu_t* emu, uint8_t* s);

void fpu_loadenv(x64emu_t* emu, char* p, int b16);
void fpu_savenv(x64emu_t* emu, char* p, int b16);
void fpu_fxsave32(x64emu_t* emu, void* ed);
void fpu_fxrstor32(x64emu_t* emu, void* ed);
void fpu_fxsave64(x64emu_t* emu, void* ed);
void fpu_fxrstor64(x64emu_t* emu, void* ed);

#endif //__X87RUN_PRIVATE_H_
