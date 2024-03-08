#ifndef __X86RUN_PRIVATE_H_
#define __X86RUN_PRIVATE_H_

#include <stdint.h>
#include "regs.h"
#include "x64emu_private.h"
#include "box64context.h"
typedef struct x64emu_s x64emu_t;

typedef struct rex_s {
    union {
        uint8_t rex;
        struct {
            unsigned int b:1;
            unsigned int x:1;
            unsigned int r:1;
            unsigned int w:1;
            unsigned int s:4;
        };
    };
    int     is32bits;
} rex_t;

static inline uint8_t Peek(x64emu_t *emu, int offset){return *(uint8_t*)(R_RIP + offset);}

#ifdef TEST_INTERPRETER
#define Push16(E, V)  do{E->regs[_SP].q[0] -=2; test->memsize = 2; *(uint16_t*)test->mem = (V); test->memaddr = E->regs[_SP].q[0];}while(0)
#define Push32(E, V)  do{E->regs[_SP].q[0] -=4; test->memsize = 4; *(uint32_t*)test->mem = (V); test->memaddr = E->regs[_SP].q[0];}while(0)
#define Push64(E, V)  do{E->regs[_SP].q[0] -=8; test->memsize = 8; *(uint64_t*)test->mem = (V); test->memaddr = E->regs[_SP].q[0];}while(0)
#else
static inline void Push16(x64emu_t *emu, uint16_t v)
{
    R_RSP -= 2;
    *((uint16_t*)R_RSP) = v;
}

static inline void Push32(x64emu_t *emu, uint32_t v)
{
    R_RSP -= 4;
    *((uint32_t*)R_RSP) = v;
}

static inline void Push64(x64emu_t *emu, uint64_t v)
{
    R_RSP -= 8;
    *((uint64_t*)R_RSP) = v;
}
#endif

static inline uint16_t Pop16(x64emu_t *emu)
{
    uint16_t* st = (uint16_t*)R_RSP;
    R_RSP += 2;
    return *st;
}

static inline uint32_t Pop32(x64emu_t *emu)
{
    uint32_t* st = (uint32_t*)R_RSP;
    R_RSP += 4;
    return *st;
}

static inline uint64_t Pop64(x64emu_t *emu)
{
    uint64_t* st = (uint64_t*)R_RSP;
    R_RSP += 8;
    return *st;
}

static inline void PushExit(x64emu_t* emu)
{
    R_RSP -= 8;
    *((uint64_t*)R_RSP) = my_context->exit_bridge;
}

// the op code definition can be found here: http://ref.x86asm.net/geek32.html

reg64_t* GetECommon(x64emu_t* emu, uintptr_t* addr, rex_t rex, uint8_t m, uint8_t delta);
reg64_t* GetECommonO(x64emu_t* emu, uintptr_t* addr, rex_t rex, uint8_t m, uint8_t delta, uintptr_t offset);
reg64_t* GetECommon32O(x64emu_t* emu, uintptr_t* addr, rex_t rex, uint8_t m, uint8_t delta, uintptr_t offset);
reg64_t* GetEb(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* TestEb(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* GetEbO(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* TestEbO(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* GetEd(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* TestEd(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* TestEd4(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* TestEd8(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* TestEd8xw(x64test_t *test, int w, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* TestEdt(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
uintptr_t GetEA(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
uintptr_t GetEA32(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* GetEdO(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* TestEdO(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* GetEd32O(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* TestEd32O(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* GetEb32O(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* TestEb32O(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
#define GetEw GetEd
reg64_t* TestEw(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
#define GetEw32O GetEd32O
#define TestEw32O TestEd32O
reg64_t* GetEw16(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v);
reg64_t* TestEw16(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v);
reg64_t* GetEw16off(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uintptr_t offset);
reg64_t* GetEd16off(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uintptr_t offset);
reg64_t* TestEw16off(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uintptr_t offset);
reg64_t* TestEd16off(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uintptr_t offset);
mmx87_regs_t* GetEm(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
mmx87_regs_t* TestEm(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
sse_regs_t* GetEx(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
sse_regs_t* TestEx(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
sse_regs_t* GetExO(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
sse_regs_t* TestExO(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
sse_regs_t* GetEx32O(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
sse_regs_t* TestEx32O(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* GetGd(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v);
#define GetGw GetGd
reg64_t* GetGb(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v);
mmx87_regs_t* GetGm(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v);
mmx87_regs_t* GetEm32O(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
mmx87_regs_t* TestEm32O(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
sse_regs_t* GetGx(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v);

void UpdateFlags(x64emu_t *emu);

#define CHECK_FLAGS(emu) if(emu->df) UpdateFlags(emu)
#define RESET_FLAGS(emu) emu->df = d_none

uintptr_t Run0F(x64emu_t *emu, rex_t rex, uintptr_t addr, int *step);
uintptr_t Run64(x64emu_t *emu, rex_t rex, int seg, uintptr_t addr);
uintptr_t Run66(x64emu_t *emu, rex_t rex, int rep, uintptr_t addr);
uintptr_t Run660F(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t Run66F20F(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t Run66F30F(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t Run6664(x64emu_t *emu, rex_t rex, int seg, uintptr_t addr);
uintptr_t Run66D9(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t Run66DD(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t Run66F0(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t Run67(x64emu_t *emu, rex_t rex, int rep, uintptr_t addr);
uintptr_t Run67_32(x64emu_t *emu, rex_t rex, int rep, uintptr_t addr);
uintptr_t Run6764_32(x64emu_t *emu, rex_t rex, int rep, int seg, uintptr_t addr);
uintptr_t Run670F(x64emu_t *emu, rex_t rex, int rep, uintptr_t addr);
uintptr_t Run6766(x64emu_t *emu, rex_t rex, int rep, uintptr_t addr);
uintptr_t Run67660F(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t RunD8(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t RunD9(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t RunDA(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t RunDB(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t RunDC(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t RunDD(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t RunDE(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t RunDF(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t RunF0(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t RunF20F(x64emu_t *emu, rex_t rex, uintptr_t addr, int *step);
uintptr_t RunF30F(x64emu_t *emu, rex_t rex, uintptr_t addr);

uintptr_t Test0F(x64test_t *test, rex_t rex, uintptr_t addr, int *step);
uintptr_t Test64(x64test_t *test, rex_t rex, int seg, uintptr_t addr);
uintptr_t Test66(x64test_t *test, rex_t rex, int rep, uintptr_t addr);
uintptr_t Test660F(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t Test66F20F(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t Test66F30F(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t Test6664(x64test_t *test, rex_t rex, int seg, uintptr_t addr);
uintptr_t Test66D9(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t Test66DD(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t Test66F0(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t Test67(x64test_t *test, rex_t rex, int rep, uintptr_t addr);
uintptr_t Test67_32(x64test_t *test, rex_t rex, int rep, uintptr_t addr);
uintptr_t Test6764_32(x64test_t *test, rex_t rex, int rep, int seg, uintptr_t addr);
uintptr_t Test670F(x64test_t *test, rex_t rex, int rep, uintptr_t addr);
uintptr_t Test6766(x64test_t *test, rex_t rex, int rep, uintptr_t addr);
uintptr_t Test67660F(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t TestD8(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t TestD9(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t TestDA(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t TestDB(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t TestDC(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t TestDD(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t TestDE(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t TestDF(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t TestF0(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t TestF20F(x64test_t *test, rex_t rex, uintptr_t addr, int *step);
uintptr_t TestF30F(x64test_t *test, rex_t rex, uintptr_t addr);


void x64Syscall(x64emu_t *emu);
void x64Int3(x64emu_t* emu, uintptr_t* addr);
x64emu_t* x64emu_fork(x64emu_t* e, int forktype);
void x86Syscall(x64emu_t *emu); //32bits syscall

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
