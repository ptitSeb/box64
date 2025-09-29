#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "tools/bridge_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "emu/x87emu_private.h"
#include "x64trace.h"
#include "mysignal.h"
#include "emit_signals.h"
#include "dynarec_native.h"
#include "custommem.h"
#include "bridge.h"
#include "dynarec_native_functions.h"

void native_fstp(x64emu_t* emu, void* p)
{
    if(ST0.q!=STld(0).uref)
        D2LD(&ST0.d, p);
    else
        memcpy(p, &STld(0).ld, 10);
}

void native_print_armreg(x64emu_t* emu, uintptr_t reg, uintptr_t n)
{
    (void)emu;
    dynarec_log(LOG_INFO, "Debug Register R%lu=0x%lx (%lu)\n", n, reg, reg);
}

void native_f2xm1(x64emu_t* emu)
{
    ST0.d = expm1(LN2 * ST0.d);
}
void native_fyl2x(x64emu_t* emu)
{
    ST(1).d = log2(ST0.d)*ST(1).d;
}
void native_ftan(x64emu_t* emu)
{
#pragma STDC FENV_ACCESS ON
    // seems that tan of glib doesn't follow the rounding direction mode
    ST0.d = tan(ST0.d);
    emu->sw.f.F87_C2 = 0;
}
void native_fpatan(x64emu_t* emu)
{
#pragma STDC FENV_ACCESS ON
    ST1.d = atan2(ST1.d, ST0.d);
}
void native_fxtract(x64emu_t* emu)
{
    int tmp32s;
    if(isnan(ST1.d)) {
        ST0.d = ST1.d;
    } else if(isinf(ST1.d)) {
        ST0.d = ST1.d;
        ST1.d = INFINITY;
    } else if(ST1.d==0.0) {
        ST0.d = ST1.d;
        ST1.d = -INFINITY;
    } else {
        // LD80bits doesn't have implicit "1" bit, so need to adjust for that
        ST0.d = frexp(ST1.d, &tmp32s)*2;
        ST1.d = tmp32s-1;
    }
}
void native_fprem(x64emu_t* emu)
{
    int e0, e1;
    int64_t ll;
    frexp(ST0.d, &e0);
    frexp(ST1.d, &e1);
    int32_t tmp32s = e0 - e1;
    if(tmp32s<64)
    {
        ll = (int64_t)floor(ST0.d/ST1.d);
        ST0.d = ST0.d - (ST1.d*ll);
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C1 = (ll&1)?1:0;
        emu->sw.f.F87_C3 = (ll&2)?1:0;
        emu->sw.f.F87_C0 = (ll&4)?1:0;
    } else {
        ll = (int64_t)(floor((ST0.d/ST1.d))/exp2(tmp32s - 32));
        ST0.d = ST0.d - ST1.d*ll*exp2(tmp32s - 32);
        emu->sw.f.F87_C2 = 1;
    }
}
void native_fyl2xp1(x64emu_t* emu)
{
    ST(1).d = log1p(ST0.d)*ST(1).d/LN2;
}
void native_fsincos(x64emu_t* emu)
{
#pragma STDC FENV_ACCESS ON
    // seems that sincos of glib doesn't follow the rounding direction mode
    sincos(ST1.d, &ST1.d, &ST0.d);
    emu->sw.f.F87_C2 = 0;
}
void native_frndint(x64emu_t* emu)
{
    ST0.d = fpu_round(emu, ST0.d);
}
void native_fscale(x64emu_t* emu)
{
#pragma STDC FENV_ACCESS ON
    if(ST0.d!=0.0)
        ST0.d = ldexp(ST0.d, trunc(ST1.d));
}
void native_fsin(x64emu_t* emu)
{
#pragma STDC FENV_ACCESS ON
    // seems that sin of glib doesn't follow the rounding direction mode
    ST0.d = sin(ST0.d);
    emu->sw.f.F87_C2 = 0;
}
void native_fcos(x64emu_t* emu)
{
#pragma STDC FENV_ACCESS ON
    // seems that cos of glib doesn't follow the rounding direction mode
    ST0.d = cos(ST0.d);
    emu->sw.f.F87_C2 = 0;
}

void native_fbld(x64emu_t* emu, uint8_t* ed)
{
    fpu_fbld(emu, ed);
}

void native_fild64(x64emu_t* emu, int64_t* ed)
{
    int64_t tmp;
    memcpy(&tmp, ed, sizeof(tmp));
    ST0.d = tmp;
    STll(0).sq = tmp;
    STll(0).sref = ST0.sq;
}

void native_fbstp(x64emu_t* emu, uint8_t* ed)
{
    fpu_fbst(emu, ed);
}

void native_fistp64(x64emu_t* emu, int64_t* ed)
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

void native_fistt64(x64emu_t* emu, int64_t* ed)
{
    // used of memcpy to avoid alignments issues
    int64_t tmp = ST0.d;
    memcpy(ed, &tmp, sizeof(tmp));
}

void native_fld(x64emu_t* emu, uint8_t* ed)
{
    memcpy(&STld(0).ld, ed, 10);
    LD2D(&STld(0), &ST(0).d);
    STld(0).uref = ST0.q;
}

void native_ud(x64emu_t* emu)
{
    if(BOX64ENV(dynarec_test))
        emu->test.test = 0;
    EmitSignal(emu, X64_SIGILL, (void*)R_RIP, 0);
}

void native_br(x64emu_t* emu)
{
    if(BOX64ENV(dynarec_test))
        emu->test.test = 0;
    EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xb09d);
}

void native_priv(x64emu_t* emu)
{
    emu->test.test = 0;
    EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0);
}

void native_gpf(x64emu_t* emu)
{
    emu->test.test = 0;
    EmitSignal(emu, X64_SIGSEGV, (void*)R_RIP, 0xbad0); // same effect has private opcode?
}

void native_int(x64emu_t* emu, int num)
{
    emu->test.test = 0;
    EmitInterruption(emu, num, (void*)R_RIP);
}
#ifndef _WIN32
void native_wineint(x64emu_t* emu, int num)
{
    emu->test.test = 0;
    EmitWineInt(emu, num, (void*)R_RIP);
}
#endif
void native_int3(x64emu_t* emu)
{
    EmitSignal(emu, X64_SIGTRAP, NULL, 3);
}

void native_div0(x64emu_t* emu)
{
    emu->test.test = 0;
    EmitDiv0(emu, (void*)R_RIP, 1);
}

void native_fsave(x64emu_t* emu, uint8_t* ed)
{
    fpu_savenv(emu, (char*)ed, 0);

    uint8_t* p = ed;
    p += 28;
    for (int i=0; i<8; ++i) {
        LD2D(p, &emu->x87[7-i].d);
        p+=10;
    }
    reset_fpu(emu);
}
void native_fsave16(x64emu_t* emu, uint8_t* ed)
{
    fpu_savenv(emu, (char*)ed, 1);

    uint8_t* p = ed;
    p += 14;
    for (int i=0; i<8; ++i) {
        LD2D(p, &emu->x87[7-i].d);
        p+=10;
    }
    reset_fpu(emu);
}
void native_frstor(x64emu_t* emu, uint8_t* ed)
{
    fpu_loadenv(emu, (char*)ed, 0);

    uint8_t* p = ed;
    p += 28;
    for (int i=0; i<8; ++i) {
        D2LD(&emu->x87[7-i].d, p);
        p+=10;
    }

}
void native_frstor16(x64emu_t* emu, uint8_t* ed)
{
    fpu_loadenv(emu, (char*)ed, 1);

    uint8_t* p = ed;
    p += 14;
    for (int i=0; i<8; ++i) {
        D2LD(&emu->x87[7-i].d, p);
        p+=10;
    }

}

void native_fprem1(x64emu_t* emu)
{
    int e0, e1;
    int64_t ll;
    frexp(ST0.d, &e0);
    frexp(ST1.d, &e1);
    int32_t tmp32s = e0 - e1;
    if(tmp32s<64)
    {
        ll = (int64_t)round(ST0.d/ST1.d);
        ST0.d = ST0.d - (ST1.d*ll);
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C1 = (ll&1)?1:0;
        emu->sw.f.F87_C3 = (ll&2)?1:0;
        emu->sw.f.F87_C0 = (ll&4)?1:0;
    } else {
        ll = (int64_t)(trunc((ST0.d/ST1.d))/exp2(tmp32s - 32));
        ST0.d = ST0.d - ST1.d*ll*exp2(tmp32s - 32);
        emu->sw.f.F87_C2 = 1;
    }
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

void native_aesimc(x64emu_t* emu, int xmm)
{
    sse_regs_t eax1 = emu->xmm[xmm];

    for(int j=0; j<4; ++j) {
        emu->xmm[xmm].ub[0+j*4] = ff_mult(0x0E, eax1.ub[0+j*4]) ^ ff_mult(0x0B, eax1.ub[1+j*4]) ^ ff_mult(0x0D, eax1.ub[2+j*4]) ^ ff_mult(0x09, eax1.ub[3+j*4]);
        emu->xmm[xmm].ub[1+j*4] = ff_mult(0x09, eax1.ub[0+j*4]) ^ ff_mult(0x0E, eax1.ub[1+j*4]) ^ ff_mult(0x0B, eax1.ub[2+j*4]) ^ ff_mult(0x0D, eax1.ub[3+j*4]);
        emu->xmm[xmm].ub[2+j*4] = ff_mult(0x0D, eax1.ub[0+j*4]) ^ ff_mult(0x09, eax1.ub[1+j*4]) ^ ff_mult(0x0E, eax1.ub[2+j*4]) ^ ff_mult(0x0B, eax1.ub[3+j*4]);
        emu->xmm[xmm].ub[3+j*4] = ff_mult(0x0B, eax1.ub[0+j*4]) ^ ff_mult(0x0D, eax1.ub[1+j*4]) ^ ff_mult(0x09, eax1.ub[2+j*4]) ^ ff_mult(0x0E, eax1.ub[3+j*4]);
    }
}
void native_aesimc_y(x64emu_t* emu, int ymm)
{
    sse_regs_t eay1 = emu->ymm[ymm];

    for(int j=0; j<4; ++j) {
        emu->ymm[ymm].ub[0+j*4] = ff_mult(0x0E, eay1.ub[0+j*4]) ^ ff_mult(0x0B, eay1.ub[1+j*4]) ^ ff_mult(0x0D, eay1.ub[2+j*4]) ^ ff_mult(0x09, eay1.ub[3+j*4]);
        emu->ymm[ymm].ub[1+j*4] = ff_mult(0x09, eay1.ub[0+j*4]) ^ ff_mult(0x0E, eay1.ub[1+j*4]) ^ ff_mult(0x0B, eay1.ub[2+j*4]) ^ ff_mult(0x0D, eay1.ub[3+j*4]);
        emu->ymm[ymm].ub[2+j*4] = ff_mult(0x0D, eay1.ub[0+j*4]) ^ ff_mult(0x09, eay1.ub[1+j*4]) ^ ff_mult(0x0E, eay1.ub[2+j*4]) ^ ff_mult(0x0B, eay1.ub[3+j*4]);
        emu->ymm[ymm].ub[3+j*4] = ff_mult(0x0B, eay1.ub[0+j*4]) ^ ff_mult(0x0D, eay1.ub[1+j*4]) ^ ff_mult(0x09, eay1.ub[2+j*4]) ^ ff_mult(0x0E, eay1.ub[3+j*4]);
    }
}
void native_aesmc(x64emu_t* emu, int xmm)
{
    sse_regs_t eax1 = emu->xmm[xmm];

    for(int j=0; j<4; ++j) {
        emu->xmm[xmm].ub[0+j*4] = ff_mult(0x02, eax1.ub[0+j*4]) ^ ff_mult(0x03, eax1.ub[1+j*4]) ^               eax1.ub[2+j*4]  ^               eax1.ub[3+j*4] ;
        emu->xmm[xmm].ub[1+j*4] =               eax1.ub[0+j*4]  ^ ff_mult(0x02, eax1.ub[1+j*4]) ^ ff_mult(0x03, eax1.ub[2+j*4]) ^               eax1.ub[3+j*4] ;
        emu->xmm[xmm].ub[2+j*4] =               eax1.ub[0+j*4]  ^               eax1.ub[1+j*4]  ^ ff_mult(0x02, eax1.ub[2+j*4]) ^ ff_mult(0x03, eax1.ub[3+j*4]);
        emu->xmm[xmm].ub[3+j*4] = ff_mult(0x03, eax1.ub[0+j*4]) ^               eax1.ub[1+j*4]  ^               eax1.ub[2+j*4]  ^ ff_mult(0x02, eax1.ub[3+j*4]);
    }
}
void native_aesmc_y(x64emu_t* emu, int ymm)
{
    sse_regs_t eay1 = emu->ymm[ymm];

    for(int j=0; j<4; ++j) {
        emu->ymm[ymm].ub[0+j*4] = ff_mult(0x02, eay1.ub[0+j*4]) ^ ff_mult(0x03, eay1.ub[1+j*4]) ^               eay1.ub[2+j*4]  ^               eay1.ub[3+j*4] ;
        emu->ymm[ymm].ub[1+j*4] =               eay1.ub[0+j*4]  ^ ff_mult(0x02, eay1.ub[1+j*4]) ^ ff_mult(0x03, eay1.ub[2+j*4]) ^               eay1.ub[3+j*4] ;
        emu->ymm[ymm].ub[2+j*4] =               eay1.ub[0+j*4]  ^               eay1.ub[1+j*4]  ^ ff_mult(0x02, eay1.ub[2+j*4]) ^ ff_mult(0x03, eay1.ub[3+j*4]);
        emu->ymm[ymm].ub[3+j*4] = ff_mult(0x03, eay1.ub[0+j*4]) ^               eay1.ub[1+j*4]  ^               eay1.ub[2+j*4]  ^ ff_mult(0x02, eay1.ub[3+j*4]);
    }
}
                                  //   A0 B1 C2 D3 E4 F5 G6 H7 I8 J9 Ka Lb Mc Nd Oe Pf
                                  //   A  N  K  H  E  B  O  L  I  F  C  P  M  J  G  D
static const uint8_t invshiftrows[] = {0,13,10, 7, 4, 1,14,11, 8, 5, 2,15,12, 9, 6, 3};
static const uint8_t invsubbytes[256] = {
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
void native_aesdlast(x64emu_t* emu, int xmm)
{

    sse_regs_t eax1;
    for(int i=0; i<16; ++i)
        eax1.ub[i] = emu->xmm[xmm].ub[invshiftrows[i]];
    //STATE ← InvSubBytes( STATE );
    for(int i=0; i<16; ++i)
        emu->xmm[xmm].ub[i] = invsubbytes[eax1.ub[i]];

}
void native_aesdlast_y(x64emu_t* emu, int ymm)
{

    sse_regs_t eay1;
    for(int i=0; i<16; ++i)
        eay1.ub[i] = emu->ymm[ymm].ub[invshiftrows[i]];
    //STATE ← InvSubBytes( STATE );
    for(int i=0; i<16; ++i)
        emu->ymm[ymm].ub[i] = invsubbytes[eay1.ub[i]];

}
static const uint8_t shiftrows[] = {0, 5,10,15, 4, 9,14, 3, 8,13, 2, 7,12, 1, 6,11};
static const uint8_t subbytes[256] = {
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
void native_aeselast(x64emu_t* emu, int xmm)
{
                            //   A0 B1 C2 D3 E4 F5 G6 H7 I8 J9 Ka Lb Mc Nd Oe Pf
                            //   A  F  K  P  E  J  O  D  I  N  C  H  M  B  G  L
    sse_regs_t eax1;
    for(int i=0; i<16; ++i)
        eax1.ub[i] = emu->xmm[xmm].ub[shiftrows[i]];
    //STATE ← SubBytes( STATE );
    for(int i=0; i<16; ++i)
        emu->xmm[xmm].ub[i] = subbytes[eax1.ub[i]];
}
void native_aeselast_y(x64emu_t* emu, int ymm)
{
    sse_regs_t eay1;
    for(int i=0; i<16; ++i)
        eay1.ub[i] = emu->ymm[ymm].ub[shiftrows[i]];
    for(int i=0; i<16; ++i)
        emu->ymm[ymm].ub[i] = subbytes[eay1.ub[i]];
}
void native_aesd(x64emu_t* emu, int xmm)
{
    native_aesdlast(emu, xmm);
    native_aesimc(emu, xmm);
}
void native_aesd_y(x64emu_t* emu, int ymm)
{
    native_aesdlast_y(emu, ymm);
    native_aesimc_y(emu, ymm);
}
void native_aese(x64emu_t* emu, int xmm)
{
    native_aeselast(emu, xmm);
    native_aesmc(emu, xmm);
}
void native_aese_y(x64emu_t* emu, int ymm)
{
    native_aeselast_y(emu, ymm);
    native_aesmc_y(emu, ymm);
}
void native_aeskeygenassist(x64emu_t* emu, int gx, int ex, void* p, uint32_t u8)
{
    sse_regs_t *EX = p?((sse_regs_t*)p):&emu->xmm[ex];
    sse_regs_t *GX = &emu->xmm[gx];
    for (int i = 4; i < 8; ++i)
        GX->ub[i] = subbytes[EX->ub[i]];
    for (int i = 12; i < 16; ++i)
        GX->ub[i] = subbytes[EX->ub[i]];
    GX->ud[0] = GX->ud[1];
    uint8_t tmp8u = GX->ub[4];
    GX->ud[1] = GX->ud[1] >> 8;
    GX->ub[7] = tmp8u;
    GX->ud[1] ^= u8;
    GX->ud[2] = GX->ud[3];
    tmp8u = GX->ub[12];
    GX->ud[3] = GX->ud[3] >> 8;
    GX->ub[15] = tmp8u;
    GX->ud[3] ^= u8;
}

void native_pclmul(x64emu_t* emu, int gx, int ex, void* p, uint32_t u8)
{
    sse_regs_t *EX = p?((sse_regs_t*)p):&emu->xmm[ex];
    sse_regs_t *GX = &emu->xmm[gx];
    int g = (u8&1)?1:0;
    int e = (u8&0b10000)?1:0;
    __int128 result = 0;
    __int128 op2 = EX->q[e];
    for (int i=0; i<64; ++i)
        if(GX->q[g]&(1LL<<i))
            result ^= (op2<<i);
    GX->u128 = result;
}
void native_pclmul_x(x64emu_t* emu, int gx, int vx, void* p, uint32_t u8)
{

    sse_regs_t *EX = ((uintptr_t)p>15)?((sse_regs_t*)p):&emu->xmm[(uintptr_t)p];
    sse_regs_t *GX = &emu->xmm[gx];
    sse_regs_t *VX = &emu->xmm[vx];
    int g = (u8&1)?1:0;
    int e = (u8&0b10000)?1:0;
    __int128 result = 0;
    __int128 op2 = EX->q[e];
    for (int i=0; i<64; ++i)
        if(VX->q[g]&(1LL<<i))
            result ^= (op2<<i);

    GX->u128 = result;
}
void native_pclmul_y(x64emu_t* emu, int gy, int vy, void* p, uint32_t u8)
{
    //compute both low and high values
    native_pclmul_x(emu, gy, vy, p, u8);
    sse_regs_t *EY = ((uintptr_t)p>15)?((sse_regs_t*)(p+16)):&emu->ymm[(uintptr_t)p];
    sse_regs_t *GY = &emu->ymm[gy];
    sse_regs_t *VY = &emu->ymm[vy];
    int g = (u8&1)?1:0;
    int e = (u8&0b10000)?1:0;
    __int128 result = 0;
    __int128 op2 = EY->q[e];
    for (int i=0; i<64; ++i)
        if(VY->q[g]&(1LL<<i))
            result ^= (op2<<i);

    GY->u128 = result;
}

void native_clflush(x64emu_t* emu, void* p)
{
    cleanDBFromAddressRange((uintptr_t)p, 8, 0);
}

static int flagsCacheNeedsTransform(dynarec_native_t* dyn, int ninst) {
    int jmp = dyn->insts[ninst].x64.jmp_insts;
    if(jmp<0)
        return 0;
    if(dyn->insts[ninst].f_exit.dfnone)  // flags are fully known, nothing we can do more
        return 0;
    if(dyn->insts[jmp].f_entry.dfnone && !dyn->insts[ninst].f_exit.dfnone && !dyn->insts[jmp].df_notneeded)
        return 1;
    switch (dyn->insts[jmp].f_entry.pending) {
        case SF_UNKNOWN: return 0;
        case SF_SET:
            if(dyn->insts[ninst].f_exit.pending!=SF_SET && dyn->insts[ninst].f_exit.pending!=SF_SET_PENDING)
                return 1;
            else
                return 0;
        case SF_SET_PENDING:
            if(dyn->insts[ninst].f_exit.pending==SF_SET_PENDING)
                return 0;
            return 1;
        case SF_PENDING:
            if(dyn->insts[ninst].f_exit.pending==SF_PENDING || dyn->insts[ninst].f_exit.pending==SF_SET_PENDING)
                return 0;
            return (dyn->insts[jmp].f_entry.dfnone  == dyn->insts[ninst].f_exit.dfnone)?0:1;
    }
    return 0;
}

int CacheNeedsTransform(dynarec_native_t* dyn, int ninst) {
    int ret = 0;
    if (flagsCacheNeedsTransform(dyn, ninst)) ret|=1;
    OTHER_CACHE()
    return ret;
}

int isPred(dynarec_native_t* dyn, int ninst, int pred) {
    for(int i=0; i<dyn->insts[ninst].pred_sz; ++i)
        if(dyn->insts[ninst].pred[i]==pred)
            return pred;
    return -1;
}
int getNominalPred(dynarec_native_t* dyn, int ninst) {
    if((ninst<=0) || !dyn->insts[ninst].pred_sz)
        return -1;
    if(isPred(dyn, ninst, ninst-1)!=-1)
        return ninst-1;
    return dyn->insts[ninst].pred[0];
}

#define F8      *(uint8_t*)(addr++)
// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_native_t* dyn, uintptr_t addr, int ninst, uint8_t nextop)
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
// return Ib on a mod/rm opcode without emitting anything
uint8_t geted_ib(dynarec_native_t* dyn, uintptr_t addr, int ninst, uint8_t nextop)
{
    addr = fakeed(dyn, addr, ninst, nextop);
    return F8;
}
#undef F8

void propagate_nodf(dynarec_native_t* dyn, int ninst)
{
    while(ninst>=0) {
        if(dyn->insts[ninst].df_notneeded)
            return; // already flagged
        if(dyn->insts[ninst].x64.gen_flags || dyn->insts[ninst].x64.use_flags)
            return; // flags are use, so maybe it's needed
        dyn->insts[ninst].df_notneeded = 1;
        --ninst;
    }
}

void x64disas_add_register_mapping_annotations(char* buf, const char* disas, const register_mapping_t* mappings, size_t mappings_sz)
{
    static char tmp[32];
    tmp[0] = '\0';
    int len = 0;
    // skip the mnemonic
    char* p = strchr(disas, ' ');
    if (!p) {
        sprintf(buf, "%s", disas);
        return;
    }
    p++; // skip the space
    while (*p) {
        while (*p && !(*p >= 'a' && *p <= 'e') && *p != 's' && *p != 'r') // skip non-register characters
            p++;
        if (!*p) break;
        for (int i = 0; i < mappings_sz; ++i) {
            if (!strncmp(p, mappings[i].name, strlen(mappings[i].name))) {
                len += sprintf(tmp + len, " %s,", mappings[i].native);
                p += strlen(mappings[i].name) - 1;
                break;
            }
        }
        p++;
        }
    if (tmp[0]) tmp[strlen(tmp) - 1] = '\0';
    sprintf(buf, "%-35s ;%s", disas, tmp);
}
