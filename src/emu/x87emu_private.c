#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "debug.h"
#include "x64emu_private.h"
#include "x87emu_private.h"
#include "bitutils.h"
//#include "x64run_private.h"

void fpu_do_free(x64emu_t* emu, int i)
{
    emu->fpu_tags |= 0b11 << (i);   // empty
    // check if all empty
    if(emu->fpu_tags != TAGS_EMPTY)
        return;
    emu->fpu_stack = 0;
}

void reset_fpu(x64emu_t* emu)
{
    memset(emu->x87, 0, sizeof(emu->x87));
    memset(emu->fpu_ld, 0, sizeof(emu->fpu_ld));
    emu->cw.x16 = 0x37F;
    emu->sw.x16 = 0x0000;
    emu->top = 0;
    emu->fpu_stack = 0;
    emu->fpu_tags = TAGS_EMPTY;
}

void fpu_fbst(x64emu_t* emu, uint8_t* d) {
    // very aproximative... but should not be much used...
    uint8_t p;
    uint8_t sign = 0x00;
    double tmp, v = ST0.d;
    if(ST0.d<0.0) 
    {
        sign = 0x80;
        v = -v;
    }
    for (int i=0; i<9; ++i) {
        tmp = floor(v/10.0);
        p = (v - 10.0*tmp);
        v = tmp;
        tmp = floor(v/10.0);
        p |= ((uint8_t)(v - 10.0*tmp))<<4;
        v = tmp;

        *(d++)=p;
    }
    tmp = floor(v/10.0);
    p = (v - 10.0*tmp);
    p |= sign;
    *(d++)=p;
    // no flags....
}

void fpu_fbld(x64emu_t* emu, uint8_t* s) {
    uint8_t p;
    uint64_t tmp = 0;
    uint64_t m = 1;
    for (int i=0; i<9; ++i) {
        p =*(s++);
        tmp += m * (p&0x0f);
        m *= 10;
        tmp += m * ((p>>4)&0x0f);
        m *= 10;
    }
    ST0.d = tmp;
    p =*(s++);
    ST0.d += m * (p&0x0f);
    if(p&0x80)
        ST0.d = -ST0.d;
}


#define FPU_t mmx87_regs_t
#define BIAS80 16383
#define BIAS64 1023
// long double (80bits) -> double (64bits)
void LD2D(void* ld, void* d)
{
    if(BOX64ENV(x87_no80bits)) {
        *(uint64_t*)d = *(uint64_t*)ld;
        return;
    }
    FPU_t result;
    #pragma pack(push, 1)
    struct {
        FPU_t f;
        int16_t b;
    } val;
    #pragma pack(pop)
    #if 1
    memcpy(&val, ld, 10);
    #else
    val.f.ud[0] = *(uint32_t*)ld;
    val.f.ud[1] = *(uint32_t*)(char*)(ld+4);
    val.b  = *(int16_t*)((char*)ld+8);
    #endif
    // do specific value first (0, infinite...)
    // bit 63 is "integer part"
    if((uint32_t)(val.b&0x7fff)==0x7fff) {
        // infinity and nans
        int t = (val.f.q&0x7fffffffffffffffLL)?0:1;
        if(t) {    // infinite
            result.ud[1] = (val.b>>4) << 20;
            result.ud[0] = 0;
        } else {      // NaN
            result.ud[1] = (val.b>>4) << 20 | ((val.f.q>>(63-20))&0x000fffff);
            result.ud[0] = (val.f.q>>(63-56))&0xffffffff;
            if(!(result.q&0x000fffffffffffffLL))
                result.q |= 1;
        }
        *(uint64_t*)d = result.q;
        return;
    }
    int32_t exp64 = (((uint32_t)(val.b&0x7fff) - BIAS80) + BIAS64);
    int32_t exp64final = exp64&0x7ff;
    if(((uint32_t)(val.b&0x7fff)==0) || (exp64<-1074)) {
        //if(val.f.q==0)
        // zero
        //if(val.f.q!=0)
        // denormal, but that's to small value for double 
        uint64_t r = (val.b&0x8000)?0x8000000000000000LL:0LL;
        *(uint64_t*)d = r;
        return;
    }

    if(exp64<=0 && val.f.q) {
        // try to see if it can be a denormal
        int one = -exp64-1022;
        uint64_t r = 0;
        if(val.b&0x8000)
            r |= 0x8000000000000000L;
        r |= val.f.q>>one;
        *(uint64_t*)d = r;
        return;

    }

    if(exp64>=0x7ff) {
        // to big value...
        result.d = HUGE_VAL;
        if(val.b&0x8000)
            result.ud[1] |= 0x80000000;
        *(uint64_t*)d = result.q;
        return;
    }

    uint64_t mant64 = (val.f.q >> 11) & 0xfffffffffffffL;
    uint32_t sign = (val.b&0x8000)?1:0;
    result.q = mant64;
    result.ud[1] |= (sign <<31)|((exp64final&0x7ff) << 20);

    *(uint64_t*)d = result.q;
}

// double (64bits) -> long double (80bits)
void D2LD(void* d, void* ld)
{
    if(BOX64ENV(x87_no80bits)) {
        *(uint64_t*)ld = *(uint64_t*)d;
        return;
    }
    #pragma pack(push, 1)
    struct {
        FPU_t f;
        int16_t b;
    } val;
    #pragma pack(pop)
    FPU_t s;
    s.q = *(uint64_t*)d;   // use memcpy to avoid risk of Bus Error?
    // do special value first
    if((s.q&0x7fffffffffffffffL)==0) {
        // zero...
        val.f.q = 0;
        val.b = (s.ud[1]&0x80000000)?0x8000:0;
        memcpy(ld, &val, 10);
        return;
    }

    int32_t sign80 = (s.ud[1]&0x80000000)?1:0;
    int32_t exp80 =  s.ud[1]&0x7ff00000;
    int32_t exp80final = (exp80>>20);
    uint64_t mant80 = s.q&0x000fffffffffffffL;
    uint64_t mant80final = (mant80 << 11);
    if(exp80final==0x7ff) {
        // NaN and Infinite
        exp80final = 0x7fff;
        if(mant80==0x0)
            mant80final = 0x8000000000000000L; //infinity
        else
            mant80final |= 0x8000000000000000L; //(quiet)NaN
    } else {
        if(exp80!=0){ 
            mant80final |= 0x8000000000000000L;
            exp80final += (BIAS80 - BIAS64);
        } else {
            // denormals -> normal (the case of 0 has been dealt with already)
            exp80final = BIAS80-BIAS64;
            int one = __builtin_clz(mant80final) + 1;
            exp80final -= one;
            mant80final<<=one;
        }
    }
    val.b = ((int16_t)(sign80)<<15)| (int16_t)(exp80final);
    val.f.q = mant80final;
    memcpy(ld, &val, 10);
    /*memcpy(ld, &f.ll, 8);
    memcpy((char*)ld + 8, &val.b, 2);*/
}

double FromLD(void* ld)
{
    if(BOX64ENV(x87_no80bits))
        return *(double*)ld;
    double ret; // cannot add = 0; it break factorio (issue when calling fmodl)
    LD2D(ld, &ret);
    return ret;
}

#ifndef HAVE_LD80BITS
long double LD2localLD(void* ld)
{
    // local implementation may not be try Quad precision, but double-double precision, so simple way to keep the 80bits precision in the conversion
    double ret; // cannot add = 0; it break factorio (issue when calling fmodl)
    LD2D(ld, &ret);
    return ret;
}
#else
long double LD2localLD(void* ld)
{
    return *(long double*)ld;
}
#endif

void fpu_loadenv(x64emu_t* emu, char* p, int b16)
{
    if(b16) {
        uint16_t* p16 = (uint16_t*)p;
        emu->cw.x16 = *p16++;
        emu->sw.x16 = *p16++;
        // tagword: 2bits*8
        // tags... (only full = 0b11 / free = 0b00)
        emu->fpu_tags = *(p16++);
        // intruction pointer: 16bits
        // data (operand) pointer: 16bits
        // last opcode: 11bits save: 16bits restaured (1st and 2nd opcode only)
    } else {
        uint32_t* p32 = (uint32_t*)p;
        emu->cw.x16 = *p32++;
        emu->sw.x16 = *p32++;
        // tagword: 2bits*8
        // tags... (only free = 0b11 / full = 0b00)
        emu->fpu_tags = *(p32++);
        // intruction pointer: 16bits
        // data (operand) pointer: 16bits
        // last opcode: 11bits save: 16bits restaured (1st and 2nd opcode only)
    }
    emu->top = emu->sw.f.F87_TOP;
}

void fpu_savenv(x64emu_t* emu, char* p, int b16)
{
    emu->sw.f.F87_TOP = emu->top&7;
    if(b16) {
        uint16_t* p16 = (uint16_t*)p;
        *p16++ = emu->cw.x16;
        *p16++ = emu->sw.x16;
        // tagword: 2bits*8
        // tags...
        *p16++ = emu->fpu_tags;
    } else {
        uint32_t* p32 = (uint32_t*)p;
        *p32++ = emu->cw.x16;
        *p32++ = emu->sw.x16;
        // tagword: 2bits*8
        // tags...
        *p32++ = emu->fpu_tags;

    }
    // other stuff are not pushed....
}

// this is the 64bits version (slightly different than the 32bits!)
typedef struct xsave32_s {
    uint16_t ControlWord;        /* 000 */
    uint16_t StatusWord;         /* 002 */
    uint8_t  TagWord;            /* 004 */
    uint8_t  Reserved1;          /* 005 */
    uint16_t ErrorOpcode;        /* 006 */
    uint32_t ErrorOffset;        /* 008 */
    uint16_t ErrorSelector;      /* 00c */
    uint16_t Reserved2;          /* 00e */
    uint32_t DataOffset;         /* 010 */
    uint16_t DataSelector;       /* 014 */
    uint16_t Reserved3;          /* 016 */
    uint32_t MxCsr;              /* 018 */
    uint32_t MxCsr_Mask;         /* 01c */
    sse_regs_t FloatRegisters[8];/* 020 */  // fpu/mmx are store in 128bits here
    sse_regs_t XmmRegisters[8];  /* 0a0 */
    uint8_t  Reserved4[56*4];    /* 120 */
} xsave32_t;
typedef struct xsave64_s {
    uint16_t ControlWord;        /* 000 */
    uint16_t StatusWord;         /* 002 */
    uint8_t  TagWord;            /* 004 */
    uint8_t  Reserved1;          /* 005 */
    uint16_t ErrorOpcode;        /* 006 */
    uint64_t ErrorOffset;        /* 008 */
    uint64_t DataOffset;         /* 010 */
    uint32_t MxCsr;              /* 018 */
    uint32_t MxCsr_Mask;         /* 01c */
    sse_regs_t FloatRegisters[8];/* 020 */  // fpu/mmx are store in 128bits here
    sse_regs_t XmmRegisters[16]; /* 0a0 */
    uint8_t  Reserved4[96];      /* 1a0 */
} xsave64_t;

void fpu_fxsave32(x64emu_t* emu, void* ed)
{
    xsave32_t *p = (xsave32_t*)ed;
    // should save flags & all
    int top = emu->top&7;
    int stack = 8-top;
    if(emu->fpu_tags == TAGS_EMPTY)
        stack = 0;
    emu->sw.f.F87_TOP = top;
    p->ControlWord = emu->cw.x16;
    p->StatusWord = emu->sw.x16;
    p->MxCsr = emu->mxcsr.x32;
    uint8_t tags = 0;
    for (int i=0; i<8; ++i)
        tags |= (((emu->fpu_tags>>(i*2))&0b11)?0:1)<<i;
    p->TagWord = tags;
    p->ErrorOpcode = 0;
    p->ErrorOffset = 0;
    p->ErrorSelector = 0;
    p->DataOffset = 0;
    p->DataSelector = 0;
    // copy FPU/MMX regs...
    for(int i=0; i<8; ++i)
        memcpy(&p->FloatRegisters[i].q[0], (i<stack)?&ST(i):&emu->mmx[i], sizeof(mmx87_regs_t));
    // copy SSE regs
    memcpy(p->XmmRegisters, emu->xmm, 8*16);
}

void fpu_fxsave64(x64emu_t* emu, void* ed)
{
    // the subtelties of the REX.W are not handled in fxsave64/fxrstor64
    xsave64_t *p = (xsave64_t*)ed;
    // should save flags & all
    int top = emu->top&7;
    int stack = 8-top;
    if(emu->fpu_tags == TAGS_EMPTY)
        stack = 0;
    emu->sw.f.F87_TOP = top;
    p->ControlWord = emu->cw.x16;
    p->StatusWord = emu->sw.x16;
    p->MxCsr = emu->mxcsr.x32;
    uint8_t tags = 0;
    for (int i=0; i<8; ++i)
        tags |= (((emu->fpu_tags>>(i*2))&0b11)?0:1)<<i;
    p->TagWord = emu->fpu_tags;
    p->ErrorOpcode = 0;
    p->ErrorOffset = 0;
    p->DataOffset = 0;
    // copy FPU/MMX regs...
    for(int i=0; i<8; ++i)
        memcpy(&p->FloatRegisters[i].q[0], (i<stack)?&ST(i):&emu->mmx[i], sizeof(mmx87_regs_t));
    // copy SSE regs
    memcpy(p->XmmRegisters, emu->xmm, 16*16);
}

void fpu_fxrstor32(x64emu_t* emu, void* ed)
{
    xsave32_t *p = (xsave32_t*)ed;
    emu->cw.x16 = p->ControlWord;
    emu->sw.x16 = p->StatusWord;
    emu->mxcsr.x32 = p->MxCsr;
    if(BOX64ENV(sse_flushto0))
        applyFlushTo0(emu);
    emu->top = emu->sw.f.F87_TOP;
    uint8_t tags = p->TagWord;
    emu->fpu_tags = 0;
    for (int i=0; i<8; ++i)
        emu->fpu_tags |= (((tags>>i)&1)?0:0b11)<<(i*2);
    int top = emu->top&7;
    int stack = 8-top;
    if(emu->fpu_tags == TAGS_EMPTY)
        stack = 0;
    // copy back MMX regs...
    for(int i=0; i<8; ++i)
        memcpy((i<stack)?&ST(i):&emu->mmx[i], &p->FloatRegisters[i].q[0], sizeof(mmx87_regs_t));
    // copy SSE regs
    memcpy(emu->xmm, p->XmmRegisters, 8*16);
}

void fpu_fxrstor64(x64emu_t* emu, void* ed)
{
    // the subtelties of the REX.W are not handled in fxsave64/fxrstor64
    xsave64_t *p = (xsave64_t*)ed;
    emu->cw.x16 = p->ControlWord;
    emu->sw.x16 = p->StatusWord;
    emu->mxcsr.x32 = p->MxCsr;
    if(BOX64ENV(sse_flushto0))
        applyFlushTo0(emu);
    emu->top = emu->sw.f.F87_TOP;
    uint8_t tags = p->TagWord;
    emu->fpu_tags = 0;
    for(int i=0; i<8; ++i)
        emu->fpu_tags |= (((tags>>i)&1)?0:0b11)<<(i*2);
    int top = emu->top&7;
    int stack = 8-top;
    if(emu->fpu_tags == TAGS_EMPTY)
        stack = 0;
    // copy back MMX regs...
    for(int i=0; i<8; ++i)
        memcpy((i<stack)?&ST(i):&emu->mmx[i], &p->FloatRegisters[i].q[0], sizeof(mmx87_regs_t));
    // copy SSE regs
    memcpy(emu->xmm, p->XmmRegisters, 16*16);
}

typedef struct xsaveheader_s {
    uint64_t xstate_bv;
    uint64_t xcomp_bv;
    uint8_t  reserved[64-16];
} xsaveheader_t;

void fpu_xsave_mask(x64emu_t* emu, void* ed, int is32bits, uint64_t mask)
{
    xsave64_t *p = (xsave64_t*)ed;
    xsaveheader_t *h = (xsaveheader_t*)(p+1);
    uint32_t rfbm = (0b111&mask);
    h->xstate_bv =(h->xstate_bv&~mask)|rfbm;
    h->xcomp_bv = 0;
    if(h->xstate_bv&0b001) {
        int top = emu->top&7;
        int stack = 8-top;
        if(emu->fpu_tags == TAGS_EMPTY)
            stack = 0;
        emu->sw.f.F87_TOP = top;
        p->ControlWord = emu->cw.x16;
        p->StatusWord = emu->sw.x16;
        p->MxCsr = emu->mxcsr.x32;
        uint8_t tags = 0;
        for (int i=0; i<8; ++i)
            tags |= (((emu->fpu_tags>>(i*2))&0b11)?0:1)<<i;
        p->TagWord = emu->fpu_tags;
        p->ErrorOpcode = 0;
        p->ErrorOffset = 0;
        p->DataOffset = 0;
        // copy FPU/MMX regs...
        for(int i=0; i<8; ++i)
            memcpy(&p->FloatRegisters[i].q[0], (i<stack)?&ST(i):&emu->mmx[i], sizeof(mmx87_regs_t));
    }
    if(((h->xstate_bv&0b010)||(h->xstate_bv&0b100))&&!(h->xstate_bv&0b001)) {
        p->MxCsr = emu->mxcsr.x32;
    }
    // copy SSE regs
    if(h->xstate_bv&0b10) {
        memcpy(&p->XmmRegisters[0], &emu->xmm[0], 16*(is32bits?8:16));
    }
    if(h->xstate_bv&0b100) {
        sse_regs_t* avx = (sse_regs_t*)(h+1);
        memcpy(&avx[0], &emu->ymm[0], 16*(is32bits?8:16));
    }
}

void fpu_xsave(x64emu_t* emu, void* ed, int is32bits)
{
    uint64_t mask = R_EAX | (((uint64_t)R_EDX)<<32);
    fpu_xsave_mask(emu, ed, is32bits, mask);
}

void fpu_xrstor(x64emu_t* emu, void* ed, int is32bits)
{
    uint64_t mask = R_EAX | (((uint64_t)R_EDX)<<32);
    return fpu_xrstor_mask(emu, ed, is32bits, mask);
}

void fpu_xrstor_mask(x64emu_t* emu, void* ed, int is32bits, uint64_t mask) {
    xsave64_t *p = (xsave64_t*)ed;
    xsaveheader_t *h = (xsaveheader_t*)(p+1);
    int compressed = (h->xcomp_bv>>63);
    uint32_t rfbm = (0b111&mask);
    uint32_t to_restore = rfbm & h->xstate_bv;
    uint32_t to_init = rfbm & ~h->xstate_bv;
    // check componant to restore
    if(to_restore&0b001) {
        emu->cw.x16 = p->ControlWord;
        emu->sw.x16 = p->StatusWord;
        emu->mxcsr.x32 = p->MxCsr;
        if(BOX64ENV(sse_flushto0))
            applyFlushTo0(emu);
        emu->top = emu->sw.f.F87_TOP;
        uint8_t tags = p->TagWord;
        emu->fpu_tags = 0;
        for(int i=0; i<8; ++i)
            emu->fpu_tags |= (((tags>>i)&1)?0:0b11)<<(i*2);
        int top = emu->top&7;
        int stack = 8-top;
        if(emu->fpu_tags == TAGS_EMPTY)
            stack = 0;
        // copy back MMX regs...
        for(int i=0; i<8; ++i)
            memcpy((i<stack)?&ST(i):&emu->mmx[i], &p->FloatRegisters[i].q[0], sizeof(mmx87_regs_t));
    } else if(to_init&0b001) {
        reset_fpu(emu);
    }
    if(((to_restore&0b010)||(to_restore&0b100))&&!(to_restore&0b001)) {
        emu->mxcsr.x32 = p->MxCsr;
    }
    if(to_restore&0b010) {
        // copy SSE regs
        memcpy(&emu->xmm[0], &p->XmmRegisters[0], 16*(is32bits?8:16));
    } else if(to_init&0b010) {
        memset(&emu->xmm[0], 0, 16*(is32bits?8:16));
    }
    if(to_restore&0b100) {
        // copy AVX upper part of regs
        sse_regs_t* avx = (sse_regs_t*)(h+1);
        memcpy(&emu->ymm[0], &avx[0], 16*(is32bits?8:16));
    } else if(to_init&0b100) {
        memset(&emu->ymm[0], 0, 16*(is32bits?8:16));
    }
}

typedef union f16_s {
    uint16_t u16;
    struct {
        uint16_t fraction:10;
        uint16_t exponant:5;
        uint16_t sign:1;
    };
} f16_t;

typedef union f32_s {
    uint32_t u32;
    struct {
        uint32_t fraction:23;
        uint32_t exponant:8;
        uint32_t sign:1;
    };
} f32_t;

uint32_t cvtf16_32(uint16_t v)
{
    f16_t in = (f16_t)v;
    f32_t ret = {0};
    ret.sign = in.sign;
    ret.fraction = in.fraction<<13;
    if(!in.exponant) {
        ret.exponant = 0;
        if (in.fraction) {
            int8_t s = 23 - (15 - LeadingZeros16(in.fraction));
            ret.exponant = 126 - s;
            ret.fraction = in.fraction << s;
        }
    } else if(in.exponant==0b11111)
        ret.exponant = 0b11111111;
    else {
        int e = in.exponant - 15;
        ret.exponant = e + 127;
    }
    return ret.u32;
}
uint16_t cvtf32_16(uint32_t v, uint8_t rounding)
{
    f32_t in = (f32_t)v;
    f16_t ret = {0};
    ret.sign = in.sign;
    rounding&=3;
    if(!in.exponant) {
        // zero and denormals
        ret.exponant = 0;
        if(in.fraction && ((rounding==1 && ret.sign) || ((rounding==2) && !ret.sign))) 
            ret.fraction = 1; // rounding artifact
        else
            ret.fraction = 0;   // no way a 32bits denormal is something else the 0 in 16bits
        return ret.u16;
    } else if(in.exponant==0b11111111) {
        // nan and infinites
        ret.exponant = 0b11111;
        ret.fraction = in.fraction;
        if(in.fraction && !ret.fraction)
            ret.fraction = 0b1000000000;
        return ret.u16;
    } else {
        // regular numbers
        int e = in.exponant - 127;
        uint16_t f = (in.fraction>>13)|0b10000000000;   // add back implicit msb
        uint16_t r = in.fraction&0b1111111111111;
        switch(rounding) {
            case 0: // nearest even
                if(r>=0b1000000000000)
                    ++f;
                break;
            case 1: // round down
                f += r?ret.sign:0;
                break;
            case 2: // round up
                f += r?(1-ret.sign):0;
                break;
            case 3: // truncate
                break;
        }
        if(f>0b11111111111) {   // implicit msb included
            ++e;
            f>>=1;
        }
        // remove implicit msb
        if(f) {
            while(!(f&0b10000000000)) {
                f<<=1;
                --e;
            }
        }
        // there is no msb to remove, as it's implicit and was not added back before
        if(!f) e = -15;
        else if(e<-14) { 
            // flush to zero
            f >>= (-15-e);
            e = -15;
            if((rounding==1 && ret.sign) || ((rounding==2) && !ret.sign)) 
                f = 1; // rounding artifact
        }
        else if(e>15) { 
            if((rounding==1 && !in.sign) || (rounding==2 && in.sign) || (rounding==3)) {
                // Clamp to max
                f=0b1111111111;
                e = 15;
            } else {
                // overflow to inifity
                f=0;
                e = 16;
            }
        } else f&=0b1111111111; // remove implicit msb (bit 11)
        ret.fraction = f;
        ret.exponant = e+15;
    }

    return ret.u16;
}
