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
#include "custommem.h"
#include "bridge.h"

void arm_fstp(x64emu_t* emu, void* p)
{
    if(ST0.q!=STld(0).uref)
        D2LD(&ST0.d, p);
    else
        memcpy(p, &STld(0).ld, 10);
}

void arm_print_armreg(x64emu_t* emu, uintptr_t reg, uintptr_t n)
{
    (void)emu;
    dynarec_log(LOG_DEBUG, "R%lu=0x%lx (%lu)\n", n, reg, reg);
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
    emu->sw.f.F87_C2 = 0;
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
    emu->sw.f.F87_C2 = 0;
}
void arm_frndint(x64emu_t* emu)
{
    ST0.d = fpu_round(emu, ST0.d);
}
void arm_fscale(x64emu_t* emu)
{
    if(ST0.d!=0.0)
        ST0.d *= exp2(trunc(ST1.d));
}
void arm_fsin(x64emu_t* emu)
{
    ST0.d = sin(ST0.d);
    emu->sw.f.F87_C2 = 0;
}
void arm_fcos(x64emu_t* emu)
{
    ST0.d = cos(ST0.d);
    emu->sw.f.F87_C2 = 0;
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
    STll(0).sq = tmp;
    STll(0).sref = ST0.sq;
}

void arm_fbstp(x64emu_t* emu, uint8_t* ed)
{
    fpu_fbst(emu, ed);
}

void arm_fistp64(x64emu_t* emu, int64_t* ed)
{
    // used of memcpy to avoid aligments issues
    if(STll(0).sref==ST(0).sq) {
        memcpy(ed, &STll(0).sq, sizeof(int64_t));
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
    STld(0).uref = ST0.q;
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

static uint8_t ff_mult(uint8_t a, uint8_t b)
{
	int retval = 0;

	for(int i = 0; i < 8; i++) {
		if((b & 1) == 1) 
			retval ^= a;
		
		if((a & 0x80)) {
			a <<= 1;
			a  ^= 0x1b;
		} else {
			a <<= 1;
		}
		
		b >>= 1;
	}
	
	return retval;
}

void arm_aesimc(x64emu_t* emu, int xmm)
{
    sse_regs_t eax1 = emu->xmm[xmm];

    for(int j=0; j<4; ++j) {
        emu->xmm[xmm].ub[0+j*4] = ff_mult(0x0E, eax1.ub[0+j*4]) ^ ff_mult(0x0B, eax1.ub[1+j*4]) ^ ff_mult(0x0D, eax1.ub[2+j*4]) ^ ff_mult(0x09, eax1.ub[3+j*4]);
        emu->xmm[xmm].ub[1+j*4] = ff_mult(0x09, eax1.ub[0+j*4]) ^ ff_mult(0x0E, eax1.ub[1+j*4]) ^ ff_mult(0x0B, eax1.ub[2+j*4]) ^ ff_mult(0x0D, eax1.ub[3+j*4]);
        emu->xmm[xmm].ub[2+j*4] = ff_mult(0x0D, eax1.ub[0+j*4]) ^ ff_mult(0x09, eax1.ub[1+j*4]) ^ ff_mult(0x0E, eax1.ub[2+j*4]) ^ ff_mult(0x0B, eax1.ub[3+j*4]);
        emu->xmm[xmm].ub[3+j*4] = ff_mult(0x0B, eax1.ub[0+j*4]) ^ ff_mult(0x0D, eax1.ub[1+j*4]) ^ ff_mult(0x09, eax1.ub[2+j*4]) ^ ff_mult(0x0E, eax1.ub[3+j*4]);
    }
}
void arm_aesmc(x64emu_t* emu, int xmm)
{
    sse_regs_t eax1 = emu->xmm[xmm];

    for(int j=0; j<4; ++j) {
        emu->xmm[xmm].ub[0+j*4] = ff_mult(0x02, eax1.ub[0+j*4]) ^ ff_mult(0x03, eax1.ub[1+j*4]) ^               eax1.ub[2+j*4]  ^               eax1.ub[3+j*4] ;
        emu->xmm[xmm].ub[1+j*4] =               eax1.ub[0+j*4]  ^ ff_mult(0x02, eax1.ub[1+j*4]) ^ ff_mult(0x03, eax1.ub[2+j*4]) ^               eax1.ub[3+j*4] ;
        emu->xmm[xmm].ub[2+j*4] =               eax1.ub[0+j*4]  ^               eax1.ub[1+j*4]  ^ ff_mult(0x02, eax1.ub[2+j*4]) ^ ff_mult(0x03, eax1.ub[3+j*4]);
        emu->xmm[xmm].ub[3+j*4] = ff_mult(0x03, eax1.ub[0+j*4]) ^               eax1.ub[1+j*4]  ^               eax1.ub[2+j*4]  ^ ff_mult(0x02, eax1.ub[3+j*4]);
    }
}
void arm_aesdlast(x64emu_t* emu, int xmm)
{
                            //   A0 B1 C2 D3 E4 F5 G6 H7 I8 J9 Ka Lb Mc Nd Oe Pf
                            //   A  N  K  H  E  B  O  L  I  F  C  P  M  J  G  D
    const uint8_t invshiftrows[] = {0,13,10, 7, 4, 1,14,11, 8, 5, 2,15,12, 9, 6, 3};
    const uint8_t invsubbytes[256] = {
        0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
        0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
        0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
        0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
        0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
        0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
        0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
        0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
        0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
        0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
        0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
        0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
        0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
        0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
        0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d,
    };

    sse_regs_t eax1;
    for(int i=0; i<16; ++i)
        eax1.ub[i] = emu->xmm[xmm].ub[invshiftrows[i]];
    //STATE ← InvSubBytes( STATE );
    for(int i=0; i<16; ++i)
        emu->xmm[xmm].ub[i] = invsubbytes[eax1.ub[i]];

}
void arm_aeselast(x64emu_t* emu, int xmm)
{
                            //   A0 B1 C2 D3 E4 F5 G6 H7 I8 J9 Ka Lb Mc Nd Oe Pf
                            //   A  F  K  P  E  J  O  D  I  N  C  H  M  B  G  L
    const uint8_t shiftrows[] = {0, 5,10,15, 4, 9,14, 3, 8,13, 2, 7,12, 1, 6,11};
    const uint8_t subbytes[256] = {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16,
    };

    sse_regs_t eax1;
    for(int i=0; i<16; ++i)
        eax1.ub[i] = emu->xmm[xmm].ub[shiftrows[i]];
    //STATE ← SubBytes( STATE );
    for(int i=0; i<16; ++i)
        emu->xmm[xmm].ub[i] = subbytes[eax1.ub[i]];
}
void arm_aesd(x64emu_t* emu, int xmm)
{
    arm_aesdlast(emu, xmm);
    arm_aesimc(emu, xmm);
}
void arm_aese(x64emu_t* emu, int xmm)
{
    arm_aeselast(emu, xmm);
    arm_aesmc(emu, xmm);
}


#define XMM0    0
#define XMM8    16
#define X870    8
#define EMM0    8
#define SCRATCH0    24

// Get a FPU scratch reg
int fpu_get_scratch(dynarec_arm_t* dyn)
{
    return SCRATCH0 + dyn->fpu_scratch++;  // return an Sx
}
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_arm_t* dyn)
{
    dyn->fpu_scratch = 0;
}
// Get a x87 double reg
int fpu_get_reg_x87(dynarec_arm_t* dyn)
{
    int i=X870;
    while (dyn->fpuused[i]) ++i;
    dyn->fpuused[i] = 1;
    return i; // return a Dx
}
// Free a FPU double reg
void fpu_free_reg(dynarec_arm_t* dyn, int reg)
{
    // TODO: check upper limit?
    dyn->fpuused[reg] = 0;
}
// Get an MMX double reg
int fpu_get_reg_emm(dynarec_arm_t* dyn, int emm)
{
    dyn->fpuused[EMM0 + emm] = 1;
    return EMM0 + emm;
}
// Get an XMM quad reg
int fpu_get_reg_xmm(dynarec_arm_t* dyn, int xmm)
{
    if(xmm>7) {
        dyn->fpuused[XMM8 + xmm - 8] = 1;
        return XMM8 + xmm - 8;
    } else {
        dyn->fpuused[XMM0 + xmm] = 1;
        return XMM0 + xmm;
    }
}
// Reset fpu regs counter
void fpu_reset_reg(dynarec_arm_t* dyn)
{
    dyn->fpu_reg = 0;
    for (int i=0; i<32; ++i)
        dyn->fpuused[i]=0;
}

#define F8      *(uint8_t*)(addr++)
#define F32     *(uint32_t*)(addr+=4, addr-4)
#define F32S64  (uint64_t)(int64_t)*(int32_t*)(addr+=4, addr-4)
// Get if ED will have the correct parity. Not emiting anything. Parity is 2 for DWORD or 3 for QWORD
int getedparity(dynarec_arm_t* dyn, int ninst, uintptr_t addr, uint8_t nextop, int parity, int delta)
{
    (void)dyn; (void)ninst;

    uint32_t tested = (1<<parity)-1;
    if((nextop&0xC0)==0xC0)
        return 0;   // direct register, no parity...
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if((sib&0x7)==5) {
                uint64_t tmp = F32S64;
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
            uint64_t tmp = F32S64;
            tmp+=addr+delta;
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
    (void)dyn; (void)addr; (void)ninst;

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
    (void)dyn;

#define PK(a)       *(uint8_t*)(addr+a)
#define PK32(a)     *(int32_t*)(addr+a)

    if(!addr || !getProtection(addr))
        return 0;
    if(PK(0)==0xff && PK(1)==0x25) {            // "absolute" jump, maybe the GOT (well, RIP relative in fact)
        uintptr_t a1 = addr + 6 + (PK32(2));    // need to add a check to see if the address is from the GOT !
        addr = (uintptr_t)getAlternate(*(void**)a1);
    }
    if(!addr || !getProtection(addr))
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
