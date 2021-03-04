#ifndef __X86RUN_PRIVATE_H_
#define __X86RUN_PRIVATE_H_

#include <stdint.h>
#include "regs.h"
#include "x64emu_private.h"
typedef struct x64emu_s x64emu_t;

typedef union rex_s {
    uint8_t rex;
    struct {
        unsigned int b:1;
        unsigned int x:1;
        unsigned int r:1;
        unsigned int w:1;
        unsigned int s:4;
    };
} rex_t;

static inline uint8_t Fetch8(x64emu_t *emu) {return *(uint8_t*)(R_RIP++);}
static inline int8_t Fetch8s(x64emu_t *emu) {return *(int8_t*)(R_RIP++);}
static inline uint16_t Fetch16(x64emu_t *emu)
{
    uint16_t val = *(uint16_t*)R_RIP;
    R_RIP+=2;
    return val;
}
static inline int16_t Fetch16s(x64emu_t *emu)
{
    int16_t val = *(int16_t*)R_RIP;
    R_RIP+=2;
    return val;
}
static inline uint32_t Fetch32(x64emu_t *emu)
{
    uint32_t val = *(uint32_t*)R_RIP;
    R_RIP+=4;
    return val;
}
static inline int32_t Fetch32s(x64emu_t *emu)
{
    int32_t val = *(int32_t*)R_RIP;
    R_RIP+=4;
    return val;
}
static inline uint64_t Fetch64(x64emu_t *emu)
{
    uint64_t val = *(uint64_t*)R_RIP;
    R_RIP+=8;
    return val;
}
static inline int64_t Fetch64s(x64emu_t *emu)
{
    int64_t val = *(int64_t*)R_RIP;
    R_RIP+=8;
    return val;
}
static inline uint8_t Peek(x64emu_t *emu, int offset){return *(uint8_t*)(R_RIP + offset);}

static inline uint64_t Pop(x64emu_t *emu)
{
    uint64_t* st = ((uint64_t*)(R_RSP));
    R_RSP += 8;
    return *st;
}

static inline void Push(x64emu_t *emu, uint64_t v)
{
    R_RSP -= 8;
    *((uint64_t*)R_RSP) = v;
}


// the op code definition can be found here: http://ref.x86asm.net/geek32.html

static inline reg64_t* GetECommon(x64emu_t* emu, rex_t rex, uint8_t m)
{
    if (m<=7) {
        if(m==0x4) {
            uint8_t sib = Fetch8(emu);
            uintptr_t base = ((sib&0x7)==5)?Fetch32(emu):(emu->regs[(sib&0x7)].q[0]+(rex.b<<3)); // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sq[0] << (sib>>6));
            return (reg64_t*)base;
        } else if (m==0x5) { //disp32
            uintptr_t base = Fetch32(emu);
            return (reg64_t*)(base+R_RIP);
        }
        return (reg64_t*)(emu->regs[m].q[0]+(rex.b<<3));
    } else {
        uintptr_t base;
        if((m&7)==4) {
            uint8_t sib = Fetch8(emu);
            base = emu->regs[(sib&0x7)+(rex.b<<3)].q[0]; // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sq[0] << (sib>>6));
        } else {
            base = emu->regs[(m&0x7)+(rex.b<<3)].q[0];
        }
        base+=(m&0x80)?Fetch32s(emu):Fetch8s(emu);
        return (reg64_t*)base;
    }
}

static inline reg64_t* GetEb(x64emu_t *emu, rex_t rex, uint8_t v)
{
    // rex ignored here
    uint8_t m = v&0xC7;    // filter Eb
    if(m>=0xC0) {
        if(rex.rex) {
            int lowhigh = (m&4)>>2;
            return (reg64_t *)(((char*)(&emu->regs[(m&0x03)]))+lowhigh);  //?
        } else {
            return &emu->regs[(m&0x07)+(rex.b<<3)];
        }
    } else return GetECommon(emu, rex, m);
}

static inline reg64_t* GetEd(x64emu_t *emu, rex_t rex, uint8_t v)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)+(rex.b<<3)];
    } else return GetECommon(emu, rex, m);
}

#define GetEw GetEd

static inline reg64_t* GetEw16(x64emu_t *emu, rex_t rex, uint8_t v)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)];
    } else {
        uintptr_t base = 0;
        switch(m&7) {
            case 0: base = R_BX+R_SI; break;
            case 1: base = R_BX+R_DI; break;
            case 2: base = R_BP+R_SI; break;
            case 3: base = R_BP+R_DI; break;
            case 4: base =      R_SI; break;
            case 5: base =      R_DI; break;
            case 6: base =      R_BP; break;
            case 7: base =      R_BX; break;
        }
        switch((m>>6)&3) {
            case 0: if(m==6) base = Fetch16(emu); break;
            case 1: base += Fetch8s(emu); break;
            case 2: base += Fetch16s(emu); break;
            // case 3 is C0..C7, already dealt with
        }
        return (reg64_t*)base;
    }
}

static inline reg64_t* GetEw16off(x64emu_t *emu, rex_t rex, uint8_t v, uintptr_t offset)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)];
    } else {
        uint32_t base = 0;
        switch(m&7) {
            case 0: base = R_BX+R_SI; break;
            case 1: base = R_BX+R_DI; break;
            case 2: base = R_BP+R_SI; break;
            case 3: base = R_BP+R_DI; break;
            case 4: base =      R_SI; break;
            case 5: base =      R_DI; break;
            case 6: base =      R_BP; break;
            case 7: base =      R_BX; break;
        }
        switch((m>>6)&3) {
            case 0: if(m==6) base = Fetch16(emu); break;
            case 1: base += Fetch8s(emu); break;
            case 2: base += Fetch16s(emu); break;
            // case 3 is C0..C7, already dealt with
        }
        return (reg64_t*)(base+offset);
    }
}

static inline mmx_regs_t* GetEm(x64emu_t *emu, rex_t rex, uint8_t v)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->mmx[m&0x07];
    } else return (mmx_regs_t*)GetECommon(emu, rex, m);
}

static inline sse_regs_t* GetEx(x64emu_t *emu, rex_t rex, uint8_t v)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->xmm[(m&0x07)+(rex.b<<4)];
    } else return (sse_regs_t*)GetECommon(emu, rex, m);
}


static inline reg64_t* GetGd(x64emu_t *emu, rex_t rex, uint8_t v)
{
    return &emu->regs[((v&0x38)>>3)+(rex.r<<3)];
}

static inline reg64_t* GetGb(x64emu_t *emu, rex_t rex, uint8_t v)
{
    uint8_t m = (v&0x38)>>3;
    if(rex.rex) {
        return (reg64_t*)&emu->regs[m&3].byte[m>>2];
    } else
        return &emu->regs[(m&7)+(rex.r<<3)];
}

static inline mmx_regs_t* GetGm(x64emu_t *emu, rex_t rex, uint8_t v)
{
    uint8_t m = (v&0x38)>>3;
    return &emu->mmx[m&7];
}

static inline sse_regs_t* GetGx(x64emu_t *emu, rex_t rex, uint8_t v)
{
    uint8_t m = (v&0x38)>>3;
    return &emu->xmm[(m&7)+(rex.r<<3)];
}

void UpdateFlags(x64emu_t *emu);

#define CHECK_FLAGS(emu) if(emu->df) UpdateFlags(emu)
#define RESET_FLAGS(emu) emu->df = d_none

//void Run67(x64emu_t *emu);
//void Run0F(x64emu_t *emu);
//void Run660F(x64emu_t *emu);
//void Run66D9(x64emu_t *emu);    // x87
//void Run6766(x64emu_t *emu);
//void RunGS(x64emu_t *emu);
//void RunFS(x64emu_t *emu);
//void RunFS66(x64emu_t *emu, uintptr_t tlsdata);
//void RunLock(x64emu_t *emu);
//void RunLock66(x64emu_t *emu);

void x86Syscall(x64emu_t *emu);
//void x86Int3(x64emu_t* emu);
//x64emu_t* x64emu_fork(x64emu_t* e, int forktype);

uintptr_t GetSegmentBaseEmu(x64emu_t* emu, int seg);
#define GetGSBaseEmu(emu)    GetSegmentBaseEmu(emu, _GS)
#define GetFSBaseEmu(emu)    GetSegmentBaseEmu(emu, _FS)
#define GetESBaseEmu(emu)    GetSegmentBaseEmu(emu, _ES)
#define GetDSBaseEmu(emu)    GetSegmentBaseEmu(emu, _DS)

const char* GetNativeName(void* p);

#ifdef HAVE_TRACE
void PrintTrace(x64emu_t* emu, uintptr_t ip, int dynarec);
#endif

#endif //__X86RUN_PRIVATE_H_