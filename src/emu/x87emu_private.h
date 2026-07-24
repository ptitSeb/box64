#ifndef __X87RUN_PRIVATE_H_
#define __X87RUN_PRIVATE_H_

#include <stdint.h>
#include <math.h>
#include <fenv.h>
#include "regs.h"
#include "x64run_private.h"
#include "debug.h"
typedef struct x64emu_s x64emu_t;

#define PI      3.14159265358979323846
#define L2E     1.4426950408889634
#define L2T     3.3219280948873623
#define LN2     0.69314718055994531
#define LG2     0.3010299956639812

#define TAGS_EMPTY 0b1111111111111111

#define ST0 emu->x87[emu->top]
#define ST1 emu->x87[(emu->top+1)&7]
#define ST(a) emu->x87[(emu->top+(a))&7]

#define STld(a)  emu->fpu_ld[(emu->top+(a))&7]
#define STll(a)  emu->fpu_ll[(emu->top+(a))&7]

// Use a NaN payload as invalid marker so a valid +0.0 reference stays usable.
#define FPU_LD80_INVALID_REF UINT64_C(0x7ff8deadbeefc0de)
static inline void fpu_ld80_clear(x64emu_t* emu, int i)
{
    STld(i).uref = FPU_LD80_INVALID_REF;
}

static inline int fpu_ld80_raw_valid(x64emu_t* emu, int i)
{
    return STld(i).uref != FPU_LD80_INVALID_REF && ST(i).q==STld(i).uref;
}

static inline void fpu_ld80_copy(x64emu_t* emu, int dst, int src)
{
    ST(dst).q = ST(src).q;
    if(fpu_ld80_raw_valid(emu, src))
        STld(dst) = STld(src);
    else
        fpu_ld80_clear(emu, dst);
}

static inline void fpu_ld80_swap(x64emu_t* emu, int a, int b)
{
    uint64_t q = ST(a).q;
    fpu_ld_t tmp = STld(a);
    ST(a).q = ST(b).q;
    ST(b).q = q;
    STld(a) = STld(b);
    STld(b) = tmp;
}

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
    emu->fpu_tags<<=2;  // st0 full
    emu->fpu_tags &= TAGS_EMPTY;
    emu->top = newtop;
    fpu_ld80_clear(emu, 0);
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
    
    emu->fpu_tags>>=2;
    emu->fpu_tags |= 0b1100000000000000;    // top empty
    fpu_ld80_clear(emu, 0);
    emu->top = (emu->top+1)&7;
    // check tags
    /*while((emu->fpu_tags&0b11) && emu->fpu_stack) {
        --emu->fpu_stack;
        emu->top = (emu->top+1)&7;
        emu->fpu_tags>>=2;
        emu->fpu_tags |= 0b1100000000000000;    // top empty
    }*/
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

#ifndef HAVE_LD80BITS
// 1 => a > b
// 0 => a == b
// -1 => a < b
// 2 => unordered
static inline int fpu_ld80_raw_cmp(const longdouble_t* a, const longdouble_t* b)
{
    uint16_t a_upper = a->l.upper, b_upper = b->l.upper;
    uint64_t a_lower = a->l.lower, b_lower = b->l.lower;

    int a_sign = a_upper >> 15, b_sign = b_upper >> 15;
    int a_exponent = a_upper & 0x7fff, b_exponent = b_upper & 0x7fff;

    int a_zero = (a_exponent == 0 && a_lower == 0);
    int b_zero = (b_exponent == 0 && b_lower == 0);

    int a_nan = (a_exponent == 0x7fff && a_lower != 0x8000000000000000ULL);
    int b_nan = (b_exponent == 0x7fff && b_lower != 0x8000000000000000ULL);

    if (a_nan || b_nan)
        return 2;   /* unordered */

    if (a_zero && b_zero)
        return 0;

    if (a_sign != b_sign)
        return a_sign ? -1 : 1;

    int mag;
    if (a_exponent != b_exponent)
        mag = (a_exponent > b_exponent) ? 1 : -1;
    else if (a_lower != b_lower)
        mag = (a_lower > b_lower) ? 1 : -1;
    else
        mag = 0;

    return a_sign ? -mag : mag;
}

static inline int fpu_fcomi_ld80(x64emu_t* emu, int i)
{
    if (!fpu_ld80_raw_valid(emu, i) || !fpu_ld80_raw_valid(emu, 0))
        return 0;

    int cmp = fpu_ld80_raw_cmp(&STld(0).ld, &STld(i).ld);

    RESET_FLAGS(emu);
    CLEAR_FLAG(F_AF);
    CLEAR_FLAG(F_OF);
    CLEAR_FLAG(F_SF);
    emu->sw.f.F87_C1 = 0;

    if (cmp == 2) {
        SET_FLAG(F_CF);
        SET_FLAG(F_PF);
        SET_FLAG(F_ZF);
    } else if (cmp > 0) {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_PF);
        CLEAR_FLAG(F_ZF);
    } else if (cmp < 0) {
        SET_FLAG(F_CF);
        CLEAR_FLAG(F_PF);
        CLEAR_FLAG(F_ZF);
    } else {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_PF);
        SET_FLAG(F_ZF);
    }
    return 1;
}
#endif

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
        case ROUND_Nearest: {
            int round = fegetround();
            fesetround(FE_TONEAREST);
            double res = nearbyint(d);
            fesetround(round);
            return res;
        }
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
    if((emu->fpu_stack<=0) || (emu->fpu_tags&0b11)) {
        //Empty
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    if(isinf(ST0.d))
    {
        //Infinity
        emu->sw.f.F87_C3 = 0;
        emu->sw.f.F87_C2 = 1;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    if(isnan(ST0.d))
    {
        //NaN
        emu->sw.f.F87_C3 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    if((ST0.ud[0]|(ST0.ud[1]&0x7fffffff))==0)
    {
        //Zero
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 0;
        return;
    }
    if((ST0.ud[1]&0x7FF00000)==0)
    {
        // denormals
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 1;
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

static inline void fpu_raise_invalid(x64emu_t* emu) {
    emu->sw.f.F87_IE = 1;
    box64_feraise_invalid();
}

void fpu_fbst(x64emu_t* emu, uint8_t* d);
void fpu_fbld(x64emu_t* emu, uint8_t* s);

void fpu_loadenv(x64emu_t* emu, char* p, int b16);
void fpu_savenv(x64emu_t* emu, char* p, int b16);
void fpu_fxsave32(x64emu_t* emu, void* ed);
void fpu_fxrstor32(x64emu_t* emu, void* ed);
void fpu_fxsave64(x64emu_t* emu, void* ed);
void fpu_fxrstor64(x64emu_t* emu, void* ed);
void fpu_xsave(x64emu_t* emu, void* ed, int is32bits);
void fpu_xsave_mask(x64emu_t* emu, void* ed, int is32bits, uint64_t mask);
void fpu_xrstor(x64emu_t* emu, void* ed, int is32bits);
void fpu_xrstor_mask(x64emu_t* emu, void* ed, int is32bits, uint64_t mask);

int full_ld_fprem(x64emu_t* emu);    // try full precision fprem on ST0/ST1, result in ST0. Return 0 if it fails

uint32_t cvtf16_32(uint16_t v);
uint16_t cvtf32_16(uint32_t v, uint8_t rounding);

#endif //__X87RUN_PRIVATE_H_
