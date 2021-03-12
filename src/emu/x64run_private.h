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

reg64_t* GetECommon(x64emu_t* emu, rex_t rex, uint8_t m, uint8_t delta);
reg64_t* GetECommonO(x64emu_t* emu, rex_t rex, uint8_t m, uint8_t delta, uintptr_t offset);
reg64_t* GetECommon32O(x64emu_t* emu, rex_t rex, uint8_t m, uint8_t delta, uintptr_t offset);
reg64_t* GetEb(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* GetEbO(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* GetEd(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* GetEdO(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* GetEd32O(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
#define GetEw GetEd
reg64_t* GetEw16(x64emu_t *emu, rex_t rex, uint8_t v);
reg64_t* GetEw16off(x64emu_t *emu, rex_t rex, uint8_t v, uintptr_t offset);
mmx87_regs_t* GetEm(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta);
sse_regs_t* GetEx(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* GetGd(x64emu_t *emu, rex_t rex, uint8_t v);
#define GetGw GetGd
reg64_t* GetGb(x64emu_t *emu, rex_t rex, uint8_t v);
mmx87_regs_t* GetGm(x64emu_t *emu, rex_t rex, uint8_t v);
sse_regs_t* GetGx(x64emu_t *emu, rex_t rex, uint8_t v);

void UpdateFlags(x64emu_t *emu);

#define CHECK_FLAGS(emu) if(emu->df) UpdateFlags(emu)
#define RESET_FLAGS(emu) emu->df = d_none

int Run0F(x64emu_t *emu, rex_t rex);
int Run64(x64emu_t *emu, rex_t rex);
int Run66(x64emu_t *emu, rex_t rex, int rep);
int Run660F(x64emu_t *emu, rex_t rex);
int Run6664(x64emu_t *emu, rex_t rex);
int Run66D9(x64emu_t *emu, rex_t rex);
int Run66DD(x64emu_t *emu, rex_t rex);
int Run67(x64emu_t *emu, rex_t rex);
int RunD8(x64emu_t *emu, rex_t rex);
int RunD9(x64emu_t *emu, rex_t rex);
int RunDA(x64emu_t *emu, rex_t rex);
int RunDB(x64emu_t *emu, rex_t rex);
int RunDD(x64emu_t *emu, rex_t rex);
int RunDF(x64emu_t *emu, rex_t rex);
int RunF0(x64emu_t *emu, rex_t rex);
int RunF20F(x64emu_t *emu, rex_t rex);
int RunF30F(x64emu_t *emu, rex_t rex);
//void Run66D9(x64emu_t *emu);    // x87
//void Run6766(x64emu_t *emu);
//void RunGS(x64emu_t *emu);
//void RunFS(x64emu_t *emu);
//void RunFS66(x64emu_t *emu, uintptr_t tlsdata);
//void RunLock66(x64emu_t *emu);

void x64Syscall(x64emu_t *emu);
void x64Int3(x64emu_t* emu);
x64emu_t* x64emu_fork(x64emu_t* e, int forktype);

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