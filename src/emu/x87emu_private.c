#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "debug.h"
#include "x64emu_private.h"
#include "x87emu_private.h"
//#include "x64run_private.h"

void reset_fpu(x64emu_t* emu)
{
    memset(emu->fpu, 0, sizeof(emu->fpu));
    memset(emu->fpu_ld, 0, sizeof(emu->fpu_ld));
    emu->cw = 0x37F;
    emu->sw.x16 = 0x0000;
    emu->top = 0;
    emu->fpu_stack = 0;
    for(int i=0; i<9; ++i)
        emu->p_regs[i].tag = 0b11;  // STx is empty
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


#define FPU_t fpu_reg_t
#define BIAS80 16383
#define BIAS64 1023
// long double (80bits) -> double (64bits)
void LD2D(void* ld, void* d)
{
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
	val.f.l.lower = *(uint32_t*)ld;
    val.f.l.upper = *(uint32_t*)(char*)(ld+4);
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
        switch((val.f.l.upper>>30)) {
            case 0: if((val.f.l.upper&(1<<29))==0) t = 1;
                    break;
            case 2: if((val.f.l.upper&(1<<29))==0) t = 1;
                    break;
        }
        if(t) {    // infinite
            result.d = HUGE_VAL;
        } else {      // NaN
            result.l.upper = 0x7ff << 20;
            result.l.lower = 0;
        }
        if(val.b&0x8000)
            result.l.upper |= 0x80000000;
        *(uint64_t*)d = result.ll;
        return;
    }
    if(((uint32_t)(val.b&0x7fff)==0) || (exp64<=0)) {
        //if(val.f.ll==0)
        // zero
        //if(val.f.ll!=0)
        // denormal, but that's to small value for double 
        uint64_t r = 0;
        if(val.b&0x8000)
            r |= 0x8000000000000000LL;
        *(uint64_t*)d = r;
        return;
    }

    if(exp64>=0x7ff) {
        // to big value...
        result.d = HUGE_VAL;
        if(val.b&0x8000)
            result.l.upper |= 0x80000000;
        *(uint64_t*)d = result.ll;
        return;
    }

	uint64_t mant64 = (val.f.ll >> 11) & 0xfffffffffffffLL;
	uint32_t sign = (val.b&0x8000)?1:0;
    result.ll = mant64;
	result.l.upper |= (sign <<31)|((exp64final&0x7ff) << 20);

	*(uint64_t*)d = result.ll;
}

// double (64bits) -> long double (80bits)
void D2LD(void* d, void* ld)
{
    #pragma pack(push, 1)
	struct {
		FPU_t f;
		int16_t b;
	} val;
    #pragma pack(pop)
    FPU_t s;
    s.ll = *(uint64_t*)d;   // use memcpy to avoid risk of Bus Error?
    // do special value first
    if((s.ll&0x7fffffffffffffffLL)==0) {
        // zero...
        val.f.ll = 0;
        if(s.l.upper&0x8000)
            val.b = 0x8000;
        else
            val.b = 0;
        memcpy(ld, &val, 10);
        return;
    }

	int32_t sign80 = (s.l.upper&0x80000000)?1:0;
	int32_t exp80 =  s.l.upper&0x7ff00000;
	int32_t exp80final = (exp80>>20);
	int64_t mant80 = s.ll&0x000fffffffffffffLL;
	int64_t mant80final = (mant80 << 11);
    if(exp80final==0x7ff) {
        // NaN and Infinite
        exp80final = 0x7fff;
        if(mant80==0x0)
            mant80final = 0x8000000000000000LL; //infinity
        else
            mant80final = 0xc000000000000000LL; //(quiet)NaN
    } else {
        if(exp80!=0){ 
            mant80final |= 0x8000000000000000LL;
            exp80final += (BIAS80 - BIAS64);
        }
    }
	val.b = ((int16_t)(sign80)<<15)| (int16_t)(exp80final);
	val.f.ll = mant80final;
    memcpy(ld, &val, 10);
    /*memcpy(ld, &f.ll, 8);
    memcpy((char*)ld + 8, &val.b, 2);*/
}

double FromLD(void* ld)
{
    double ret;
    LD2D(ld, &ret);
    return ret;
}

void fpu_loadenv(x64emu_t* emu, char* p, int b16)
{
    emu->cw = *(uint16_t*)p;
    p+=(b16)?2:4;
    emu->sw.x16 = *(uint16_t*)p;
    emu->top = emu->sw.f.F87_TOP;
    p+=(b16)?2:4;
    // tagword: 2bits*8
    // tags... (only full = 0b11 / free = 0b00)
    uint16_t tags = *(uint16_t*)p;
    for(int i=0; i<8; ++i)
        emu->p_regs[i].tag = (tags>>(i*2))&0b11;
    // intruction pointer: 16bits
    // data (operand) pointer: 16bits
    // last opcode: 11bits save: 16bits restaured (1st and 2nd opcode only)
}

void fpu_savenv(x64emu_t* emu, char* p, int b16)
{
    emu->sw.f.F87_TOP = emu->top&7;
    *(uint16_t*)p = emu->cw;
    p+=2;
    if(!b16) {*(uint16_t*)p = 0; p+=2;}
    *(uint16_t*)p = emu->sw.x16;
    p+=2;
    if(!b16) {*(uint16_t*)p = 0; p+=2;}
    // tagword: 2bits*8
    // tags...
    uint16_t tags = 0;
    for (int i=0; i<8; ++i)
        tags |= (emu->p_regs[i].tag)<<(i*2);
    *(uint16_t*)p = tags;
    // other stuff are not pushed....
}

typedef struct xsave_s {
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
} xsave_t;

void fpu_fxsave(x64emu_t* emu, void* ed)
{
    xsave_t *p = (xsave_t*)ed;
    // should save flags & all
    emu->sw.f.F87_TOP = emu->top&7;
    p->ControlWord = emu->cw;
    p->StatusWord = emu->sw.x16;
    uint8_t tags = 0;
    for (int i=0; i<8; ++i)
        tags |= ((emu->p_regs[i].tag)<<(i*2)==0b11)?0:1;
    p->TagWord = tags;
    p->ErrorOpcode = 0;
    p->ErrorOffset = 0;
    p->ErrorSelector = 0;
    p->DataOffset = 0;
    p->DataSelector = 0;
    p->MxCsr = 0;
    p->MxCsr_Mask = 0;
    // copy MMX regs...
    for(int i=0; i<8; ++i)
        memcpy(&p->FloatRegisters[i].q[0], &emu->mmx[0], sizeof(emu->mmx[0]));
    // copy SSE regs
    memcpy(&p->XmmRegisters[0], &emu->xmm[0], sizeof(emu->xmm));
    // put also FPU regs in a reserved area... on XMM 8-15
    for(int i=0; i<8; ++i)
        memcpy(&p->XmmRegisters[8+i].q[0], &emu->fpu[0], sizeof(emu->fpu[0]));
    // put a magic sign in reserved area, box86 specific
    ((unsigned int *)p->Reserved4)[11] = 0x50515253;
}

void fpu_fxrstor(x64emu_t* emu, void* ed)
{
    xsave_t *p = (xsave_t*)ed;
    emu->cw = p->ControlWord;
    emu->sw.x16 = p->StatusWord;
    emu->top = emu->sw.f.F87_TOP;
    uint8_t tags = p->TagWord;
    for(int i=0; i<8; ++i)
        emu->p_regs[i].tag = (tags>>(i*2))?0:0b11;
    // copy back MMX regs...
    for(int i=0; i<8; ++i)
        memcpy(&emu->mmx[i], &p->FloatRegisters[i].q[0], sizeof(emu->mmx[0]));
    // copy SSE regs
    memcpy(&emu->xmm[0], &p->XmmRegisters[0], sizeof(emu->xmm));
    // check the box86 magic sign in reserved area
    if(((unsigned int *)p->Reserved4)[11] == 0x50515253) {
        // also FPU regs where a reserved area... on XMM 8-15?
        for(int i=0; i<8; ++i)
            memcpy(&emu->fpu[0], &p->XmmRegisters[8+i].q[0], sizeof(emu->fpu[0]));
    } else {
        // copy the mmx to fpu...
        for(int i=0; i<8; ++i)
            memcpy(&emu->fpu[0], &emu->mmx[i], sizeof(emu->mmx[0]));
    }
}
