#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "debug.h"
#include "x64emu_private.h"
#include "x87emu_private.h"
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
    if(box64_x87_no80bits) {
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
	int32_t exp64 = (((uint32_t)(val.b&0x7fff) - BIAS80) + BIAS64);
	int32_t exp64final = exp64&0x7ff;
    // do specific value first (0, infinite...)
    // bit 63 is "integer part"
    // bit 62 is sign
    if((uint32_t)(val.b&0x7fff)==0x7fff) {
        // infinity and nans
        int t = 0; //nan
        switch((val.f.ud[1]>>30)) {
            case 0: if((val.f.ud[1]&(1<<29))==0) t = 1;
                    break;
            case 2: if((val.f.ud[1]&(1<<29))==0) t = 1;
                    break;
        }
        if(t) {    // infinite
            result.d = HUGE_VAL;
        } else {      // NaN
            result.ud[1] = 0x7ff << 20;
            result.ud[0] = 0;
        }
        if(val.b&0x8000)
            result.ud[1] |= 0x80000000;
        *(uint64_t*)d = result.q;
        return;
    }
    if(((uint32_t)(val.b&0x7fff)==0) || (exp64<-1074)) {
        //if(val.f.q==0)
        // zero
        //if(val.f.q!=0)
        // denormal, but that's to small value for double 
        uint64_t r = 0;
        if(val.b&0x8000)
            r |= 0x8000000000000000L;
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
    if(box64_x87_no80bits) {
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
        if(s.ud[1]&0x8000)
            val.b = 0x8000;
        else
            val.b = 0;
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
            mant80final = 0xc000000000000000L; //(quiet)NaN
    } else {
        if(exp80!=0){ 
            mant80final |= 0x8000000000000000L;
            exp80final += (BIAS80 - BIAS64);
        } else if(mant80final!=0) {
            // denormals -> normal
            exp80final = BIAS80-1023;
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
    if(box64_x87_no80bits)
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
    emu->cw.x16 = *(uint16_t*)p;
    p+=(b16)?2:4;
    emu->sw.x16 = *(uint16_t*)p;
    emu->top = emu->sw.f.F87_TOP;
    p+=(b16)?2:4;
    // tagword: 2bits*8
    // tags... (only full = 0b11 / free = 0b00)
    emu->fpu_tags = *(uint16_t*)p;
    // intruction pointer: 16bits
    // data (operand) pointer: 16bits
    // last opcode: 11bits save: 16bits restaured (1st and 2nd opcode only)
}

void fpu_savenv(x64emu_t* emu, char* p, int b16)
{
    emu->sw.f.F87_TOP = emu->top&7;
    *(uint16_t*)p = emu->cw.x16;
    p+=2;
    if(!b16) {*(uint16_t*)p = 0; p+=2;}
    *(uint16_t*)p = emu->sw.x16;
    p+=2;
    if(!b16) {*(uint16_t*)p = 0; p+=2;}
    // tagword: 2bits*8
    // tags...
    *(uint16_t*)p = emu->fpu_tags;
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
    sse_regs_t XmmRegisters[16]; /* 0a0 */
    uint8_t  Reserved4[96];      /* 1a0 */
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
    if(top==0)  // check if stack is full or empty, based on tag[0]
        stack = (emu->fpu_tags&0b11)?8:0;
    emu->sw.f.F87_TOP = top;
    p->ControlWord = emu->cw.x16;
    p->StatusWord = emu->sw.x16;
    p->MxCsr = emu->mxcsr.x32;
    uint8_t tags = 0;
    for (int i=0; i<8; ++i)
        tags |= ((emu->fpu_tags>>(i*2))&0b11)?0:1;
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
    memcpy(&p->XmmRegisters[0], &emu->xmm[0], sizeof(emu->xmm));
}

void fpu_fxsave64(x64emu_t* emu, void* ed)
{
    xsave64_t *p = (xsave64_t*)ed;
    // should save flags & all
    int top = emu->top&7;
    int stack = 8-top;
    if(top==0)  // check if stack is full or empty, based on tag[0]
        stack = (emu->fpu_tags&0b11)?8:0;
    emu->sw.f.F87_TOP = top;
    p->ControlWord = emu->cw.x16;
    p->StatusWord = emu->sw.x16;
    p->MxCsr = emu->mxcsr.x32;
    uint8_t tags = 0;
    for (int i=0; i<8; ++i)
        tags |= ((emu->fpu_tags>>(i*2))&0b11)?0:1;
    p->TagWord = emu->fpu_tags;
    p->ErrorOpcode = 0;
    p->ErrorOffset = 0;
    p->DataOffset = 0;
    // copy FPU/MMX regs...
    for(int i=0; i<8; ++i)
        memcpy(&p->FloatRegisters[i].q[0], (i<stack)?&ST(i):&emu->mmx[i], sizeof(mmx87_regs_t));
    // copy SSE regs
    memcpy(&p->XmmRegisters[0], &emu->xmm[0], sizeof(emu->xmm));
}

void fpu_fxrstor32(x64emu_t* emu, void* ed)
{
    xsave32_t *p = (xsave32_t*)ed;
    emu->cw.x16 = p->ControlWord;
    emu->sw.x16 = p->StatusWord;
    emu->mxcsr.x32 = p->MxCsr;
    if(box64_sse_flushto0)
        applyFlushTo0(emu);
    emu->top = emu->sw.f.F87_TOP;
    uint8_t tags = p->TagWord;
    emu->fpu_tags = 0;
    for (int i=0; i<8; ++i)
        emu->fpu_tags |= (((tags>>(i*2))&1)?0:0b11)<<(i*2);
    int top = emu->top&7;
    int stack = 8-top;
    if(top==0)  // check if stack is full or empty, based on tag[0]
        stack = (emu->fpu_tags&0b11)?8:0;
    // copy back MMX regs...
    for(int i=0; i<8; ++i)
        memcpy((i<stack)?&ST(i):&emu->mmx[i], &p->FloatRegisters[i].q[0], sizeof(mmx87_regs_t));
    // copy SSE regs
    memcpy(&emu->xmm[0], &p->XmmRegisters[0], sizeof(emu->xmm));
}

void fpu_fxrstor64(x64emu_t* emu, void* ed)
{
    xsave64_t *p = (xsave64_t*)ed;
    emu->cw.x16 = p->ControlWord;
    emu->sw.x16 = p->StatusWord;
    emu->mxcsr.x32 = p->MxCsr;
    if(box64_sse_flushto0)
        applyFlushTo0(emu);
    emu->top = emu->sw.f.F87_TOP;
    uint8_t tags = p->TagWord;
    emu->fpu_tags = 0;
    for(int i=0; i<8; ++i)
        emu->fpu_tags |= (((tags>>i)&1)?0:0b11)<<(i*2);
    int top = emu->top&7;
    int stack = 8-top;
    if(top==0)  // check if stack is full or empty, based on tag[0]
        stack = (emu->fpu_tags&0b11)?8:0;
    // copy back MMX regs...
    for(int i=0; i<8; ++i)
        memcpy((i<stack)?&ST(i):&emu->mmx[i], &p->FloatRegisters[i].q[0], sizeof(mmx87_regs_t));
    // copy SSE regs
    memcpy(&emu->xmm[0], &p->XmmRegisters[0], sizeof(emu->xmm));
}
