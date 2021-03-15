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
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"

void arm_fstp(x64emu_t* emu, void* p)
{
    if(ST0.q!=STld(0).ref)
        D2LD(&ST0.d, p);
    else
        memcpy(p, &STld(0).ld, 10);
}

void arm_print_armreg(x64emu_t* emu, uintptr_t reg, uintptr_t n)
{
    dynarec_log(LOG_DEBUG, "R%ld=0x%lx (%ld)\n", n, reg, reg);
}

void arm_f2xm1(x64emu_t* emu)
{
    ST0.d = exp2(ST0.d) - 1.0;
}
void arm_fyl2x(x64emu_t* emu)
{
    ST(1).d = log2(ST0.d)*ST(1).d;
}
void arm_ftan(x64emu_t* emu)
{
    ST0.d = tan(ST0.d);
}
void arm_fpatan(x64emu_t* emu)
{
    ST1.d = atan2(ST1.d, ST0.d);
}
void arm_fxtract(x64emu_t* emu)
{
    int32_t tmp32s = (ST1.q&0x7ff0000000000000LL)>>52;
    tmp32s -= 1023;
    ST1.d /= exp2(tmp32s);
    ST0.d = tmp32s;
}
void arm_fprem(x64emu_t* emu)
{
    int32_t tmp32s = ST0.d / ST1.d;
    ST0.d -= ST1.d * tmp32s;
    emu->sw.f.F87_C2 = 0;
    emu->sw.f.F87_C0 = (tmp32s&1);
    emu->sw.f.F87_C3 = ((tmp32s>>1)&1);
    emu->sw.f.F87_C1 = ((tmp32s>>2)&1);
}
void arm_fyl2xp1(x64emu_t* emu)
{
    ST(1).d = log2(ST0.d + 1.0)*ST(1).d;
}
void arm_fsincos(x64emu_t* emu)
{
    sincos(ST1.d, &ST1.d, &ST0.d);
}
void arm_frndint(x64emu_t* emu)
{
    ST0.d = fpu_round(emu, ST0.d);
}
void arm_fscale(x64emu_t* emu)
{
    ST0.d *= exp2(trunc(ST1.d));
}
void arm_fsin(x64emu_t* emu)
{
    ST0.d = sin(ST0.d);
}
void arm_fcos(x64emu_t* emu)
{
    ST0.d = cos(ST0.d);
}

void arm_fbld(x64emu_t* emu, uint8_t* ed)
{
    fpu_fbld(emu, ed);
}

void arm_fild64(x64emu_t* emu, int64_t* ed)
{
    int64_t tmp;
    memcpy(&tmp, ed, sizeof(tmp));
    ST0.d = tmp;
    STll(0).ll = tmp;
    STll(0).ref = ST0.q;
}

void arm_fbstp(x64emu_t* emu, uint8_t* ed)
{
    fpu_fbst(emu, ed);
}

void arm_fistp64(x64emu_t* emu, int64_t* ed)
{
    // used of memcpy to avoid aligments issues
    if(STll(0).ref==ST(0).q) {
        memcpy(ed, &STll(0).ll, sizeof(int64_t));
    } else {
        int64_t tmp;
        if(isgreater(ST0.d, (double)(int64_t)0x7fffffffffffffffLL) || isless(ST0.d, (double)(int64_t)0x8000000000000000LL) || !isfinite(ST0.d))
            tmp = 0x8000000000000000LL;
        else
            tmp = fpu_round(emu, ST0.d);
        memcpy(ed, &tmp, sizeof(tmp));
    }
}

void arm_fistt64(x64emu_t* emu, int64_t* ed)
{
    // used of memcpy to avoid aligments issues
    int64_t tmp = ST0.d;
    memcpy(ed, &tmp, sizeof(tmp));
}

void arm_fld(x64emu_t* emu, uint8_t* ed)
{
    memcpy(&STld(0).ld, ed, 10);
    LD2D(&STld(0), &ST(0).d);
    STld(0).ref = ST0.q;
}

void arm_ud(x64emu_t* emu)
{
    emit_signal(emu, SIGILL, (void*)R_RIP, 0);
}

void arm_fsave(x64emu_t* emu, uint8_t* ed)
{
    fpu_savenv(emu, (char*)ed, 0);

    uint8_t* p = ed;
    p += 28;
    for (int i=0; i<8; ++i) {
        LD2D(p, &ST(i).d);
        p+=10;
    }
}
void arm_frstor(x64emu_t* emu, uint8_t* ed)
{
    fpu_loadenv(emu, (char*)ed, 0);

    uint8_t* p = ed;
    p += 28;
    for (int i=0; i<8; ++i) {
        D2LD(&ST(i).d, p);
        p+=10;
    }

}

void arm_fprem1(x64emu_t* emu)
{
    // simplified version
    int32_t tmp32s = round(ST0.d / ST1.d);
    ST0.d -= ST1.d*tmp32s;
    emu->sw.f.F87_C2 = 0;
    emu->sw.f.F87_C0 = (tmp32s&1);
    emu->sw.f.F87_C3 = ((tmp32s>>1)&1);
    emu->sw.f.F87_C1 = ((tmp32s>>2)&1);
}


// Get a FPU single scratch reg
int fpu_get_scratch_single(dynarec_arm_t* dyn)
{
    return dyn->fpu_scratch++;  // return an Sx
}
// Get a FPU double scratch reg
int fpu_get_scratch_double(dynarec_arm_t* dyn)
{
    return dyn->fpu_scratch++;  // return an Dx (same as Sx)
}
// Get a FPU quad scratch reg
int fpu_get_scratch_quad(dynarec_arm_t* dyn)
{
    return dyn->fpu_scratch++;  // return an Qx (same as Dx or Sx)
}
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_arm_t* dyn)
{
    dyn->fpu_scratch = 0;
    if(dyn->fpu_extra_qscratch) {
        fpu_free_reg_quad(dyn, dyn->fpu_extra_qscratch);
        dyn->fpu_extra_qscratch = 0;
    }
}
#define FPUFIRST    8
// Get a FPU double reg
int fpu_get_reg_double(dynarec_arm_t* dyn)
{
    // TODO: check upper limit?
    int i=0;
    while (dyn->fpuused[i]) ++i;
    dyn->fpuused[i] = 1;
    return i+FPUFIRST; // return a Dx
}
// Free a FPU double reg
void fpu_free_reg_double(dynarec_arm_t* dyn, int reg)
{
    // TODO: check upper limit?
    int i=reg-FPUFIRST;
    dyn->fpuused[i] = 0;
}
// Get a FPU quad reg
int fpu_get_reg_quad(dynarec_arm_t* dyn)
{
    int i=0;
    while (dyn->fpuused[i]) ++i;
    dyn->fpuused[i] = 1;
    return i+FPUFIRST; // return a Qx, it's the same as Dx on aarch64
}
// Free a FPU quad reg
void fpu_free_reg_quad(dynarec_arm_t* dyn, int reg)
{
    int i=reg-FPUFIRST;
    dyn->fpuused[i] = 0;
}
// Reset fpu regs counter
void fpu_reset_reg(dynarec_arm_t* dyn)
{
    dyn->fpu_reg = 0;
    for (int i=0; i<24; ++i)
        dyn->fpuused[i]=0;
}

#define F8      *(uint8_t*)(addr++)
#define F32     *(uint32_t*)(addr+=4, addr-4)
// Get if ED will have the correct parity. Not emiting anything. Parity is 2 for DWORD or 3 for QWORD
int getedparity(dynarec_arm_t* dyn, int ninst, uintptr_t addr, uint8_t nextop, int parity)
{

    uint32_t tested = (1<<parity)-1;
    if((nextop&0xC0)==0xC0)
        return 0;   // direct register, no parity...
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if((sib&0x7)==5) {
                uint32_t tmp = F32;
                if (sib_reg!=4) {
                    // if XXXXXX+reg<<N then check parity of XXXXX and N should be enough
                    return ((tmp&tested)==0 && (sib>>6)>=parity)?1:0;
                } else {
                    // just a constant...
                    return (tmp&tested)?0:1;
                }
            } else {
                if(sib_reg==4 && parity<3)
                    return 0;   // simple [reg]
                // don't try [reg1 + reg2<<N], unless reg1 is ESP
                return ((sib&0x7)==4 && (sib>>6)>=parity)?1:0;
            }
        } else if((nextop&7)==5) {
            uint32_t tmp = F32;
            return (tmp&tested)?0:1;
        } else {
            return 0;
        }
    } else {
        return 0; //Form [reg1 + reg2<<N + XXXXXX]
    }
}

// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop) 
{
    if((nextop&0xC0)==0xC0)
        return addr;
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            if((sib&0x7)==5) {
                addr+=4;
            }
        } else if((nextop&7)==5) {
            addr+=4;
        }
    } else {
        if((nextop&7)==4) {
            ++addr;
        }
        if(nextop&0x80) {
            addr+=4;
        } else {
            ++addr;
        }
    }
    return addr;
}
#undef F8
#undef F32

int isNativeCall(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t* calladdress, int* retn)
{
#define PK(a)       *(uint8_t*)(addr+a)
#define PK64(a)     *(uint64_t*)(addr+a)

    if(!addr)
        return 0;
    if(PK(0)==0xff && PK(1)==0x25) {  // absolute jump, maybe the GOT
        uintptr_t a1 = (PK64(2));   // need to add a check to see if the address is from the GOT !
        addr = *(uintptr_t*)a1; 
    }
    if(addr<0x10000)    // too low, that is suspicious
        return 0;
    onebridge_t *b = (onebridge_t*)(addr);
    if(b->CC==0xCC && b->S=='S' && b->C=='C' && b->w!=(wrapper_t)0 && b->f!=(uintptr_t)PltResolver) {
        // found !
        if(retn) *retn = (b->C3==0xC2)?b->N:0;
        if(calladdress) *calladdress = addr+1;
        return 1;
    }
    return 0;
#undef PK32
#undef PK
}

