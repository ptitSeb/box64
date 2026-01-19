#ifndef __X86RUN_PRIVATE_H_
#define __X86RUN_PRIVATE_H_

#include <stdint.h>
#include "regs.h"
#include "x64emu_private.h"
#include "box64context.h"
#include "symbolfuncs.h"
#include "x64emu.h"

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
    uint16_t    is32bits:1;
    uint16_t    is66:1;
    uint16_t    is67:1;
    uint16_t    isf0:1;
    uint16_t    seg:3; //seg is 0-5, but 0 is CS, so will always be no-offset
    uint16_t    rep:2; // 0 none, 1=F2 prefix, 2=F3 prefix
    uint64_t    offset;
} rex_t;

#define VEX_P_NONE  0
#define VEX_P_66    1
#define VEX_P_F3    2
#define VEX_P_F2    3
#define VEX_M_NONE  0
#define VEX_M_0F    1
#define VEX_M_0F38  2
#define VEX_M_0F3A  3
typedef struct vex_s {
    rex_t       rex;
    uint16_t    l:1;
    uint16_t    p:2;    //0: none, 1: 0x66, 2:0xF3, 3: 0xF2
    uint16_t    v:4;    // src register
    uint16_t    m:5;    // opcode map
} vex_t;

// the op code definition can be found here: http://ref.x86asm.net/geek32.html

reg64_t* GetECommon(x64emu_t* emu, uintptr_t* addr, rex_t rex, uint8_t m, uint8_t delta);
reg64_t* GetECommonO(x64emu_t* emu, uintptr_t* addr, rex_t rex, uint8_t m, uint8_t delta, uintptr_t offset);
reg64_t* GetECommon32O(x64emu_t* emu, uintptr_t* addr, rex_t rex, uint8_t m, uint8_t delta, uintptr_t offset);
reg64_t* GetECommon32O_16(x64emu_t* emu, uintptr_t* addr, rex_t rex, uint8_t m, uint8_t delta, uintptr_t offset);
reg64_t* GetEb(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* TestEb(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* GetEbO(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* TestEbO(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* GetEd(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* TestEd(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* TestEd4(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* TestEd4O(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* TestEd8(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* TestEd8O(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset);
reg64_t* TestEd8xw(x64test_t *test, int w, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
reg64_t* TestEdt(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
uintptr_t GetEA(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
uintptr_t GetEA32(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
uintptr_t GetEA32_16(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta);
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
sse_regs_t* TestEx(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v, uint8_t delta, int sz);
sse_regs_t* TestEy(x64test_t *test, uintptr_t* addr, rex_t rex, uint8_t v);
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
sse_regs_t* GetGy(x64emu_t *emu, uintptr_t* addr, rex_t rex, uint8_t v);

#define CHECK_FLAGS(emu) if(emu->df) UpdateFlags(emu)
#define RESET_FLAGS(emu) emu->df = d_none

uintptr_t Run0F(x64emu_t *emu, rex_t rex, uintptr_t addr, int *step);
uintptr_t Run66(x64emu_t *emu, rex_t rex, uintptr_t addr, int *step);
uintptr_t Run660F(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t Run66F20F(x64emu_t *emu, rex_t rex, uintptr_t addr, int *step);
uintptr_t Run66F30F(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t Run66D9(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t Run66DD(x64emu_t *emu, rex_t rex, uintptr_t addr);
uintptr_t Run66F0(x64emu_t *emu, rex_t rex, uintptr_t addr);
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
uintptr_t RunF30F(x64emu_t *emu, rex_t rex, uintptr_t addr, int *step);
uintptr_t RunAVX(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step);
uintptr_t RunAVX_0F(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step);
uintptr_t RunAVX_0F38(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step);
uintptr_t RunAVX_660F(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step);
uintptr_t RunAVX_F20F(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step);
uintptr_t RunAVX_F30F(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step);
uintptr_t RunAVX_660F38(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step);
uintptr_t RunAVX_660F3A(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step);
uintptr_t RunAVX_F20F38(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step);
uintptr_t RunAVX_F20F3A(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step);
uintptr_t RunAVX_F30F38(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step);
uintptr_t RunAVX_F30F3A(x64emu_t *emu, vex_t vex, uintptr_t addr, int *step);

uintptr_t Test0F(x64test_t *test, rex_t rex, uintptr_t addr, int *step);
uintptr_t Test66(x64test_t *test, rex_t rex, uintptr_t addr, int *step);
uintptr_t Test660F(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t Test66F20F(x64test_t *test, rex_t rex, uintptr_t addr, int *step);
uintptr_t Test66F30F(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t Test66D9(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t Test66DD(x64test_t *test, rex_t rex, uintptr_t addr);
uintptr_t Test66F0(x64test_t *test, rex_t rex, uintptr_t addr);
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
uintptr_t TestF30F(x64test_t *test, rex_t rex, uintptr_t addr, int *step);
uintptr_t TestAVX(x64test_t *test, vex_t vex, uintptr_t addr, int *step);
uintptr_t TestAVX_0F(x64test_t *test, vex_t vex, uintptr_t addr, int *step);
uintptr_t TestAVX_0F38(x64test_t *test, vex_t vex, uintptr_t addr, int *step);
uintptr_t TestAVX_660F(x64test_t *test, vex_t vex, uintptr_t addr, int *step);
uintptr_t TestAVX_F20F(x64test_t *test, vex_t vex, uintptr_t addr, int *step);
uintptr_t TestAVX_F30F(x64test_t *test, vex_t vex, uintptr_t addr, int *step);
uintptr_t TestAVX_660F38(x64test_t *test, vex_t vex, uintptr_t addr, int *step);
uintptr_t TestAVX_660F3A(x64test_t *test, vex_t vex, uintptr_t addr, int *step);
uintptr_t TestAVX_F20F38(x64test_t *test, vex_t vex, uintptr_t addr, int *step);
uintptr_t TestAVX_F20F3A(x64test_t *test, vex_t vex, uintptr_t addr, int *step);
uintptr_t TestAVX_F30F38(x64test_t *test, vex_t vex, uintptr_t addr, int *step);
uintptr_t TestAVX_F30F3A(x64test_t *test, vex_t vex, uintptr_t addr, int *step);

#ifdef HAVE_TRACE
void PrintTrace(x64emu_t* emu, uintptr_t ip, int dynarec);
#endif

#endif //__X86RUN_PRIVATE_H_
