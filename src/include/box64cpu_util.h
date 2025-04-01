#ifndef __BOX64CPU_UTIL_H_
#define __BOX64CPU_UTIL_H_

#include <stdint.h>
#include "emu/x64emu_private.h"
#include "box64context.h"
#include "regs.h"

static inline uint8_t Peek(x64emu_t *emu, int offset)
{
    return *(uint8_t*)(R_RIP + offset);
}

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

static inline void Push_32(x64emu_t *emu, uint32_t v)
{
    R_ESP -= 4;
    *((uint32_t*)(uintptr_t)R_ESP) = v;
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

static inline uint32_t Pop_32(x64emu_t *emu)
{
    uint32_t* st = (uint32_t*)(uintptr_t)R_RSP;
    R_ESP += 4;
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

#ifdef BOX32
static inline void PushExit_32(x64emu_t* emu)
{
    R_ESP -= 4;
    *((ptr_t*)(uintptr_t)R_ESP) = my_context->exit_bridge;
}
#endif

uint32_t GetEAX(x64emu_t *emu);
uint64_t GetRAX(x64emu_t *emu);
void SetEAX(x64emu_t *emu, uint32_t v);
void SetEBX(x64emu_t *emu, uint32_t v);
void SetECX(x64emu_t *emu, uint32_t v);
void SetEDX(x64emu_t *emu, uint32_t v);
void SetEDI(x64emu_t *emu, uint32_t v);
void SetESI(x64emu_t *emu, uint32_t v);
void SetEBP(x64emu_t *emu, uint32_t v);
void SetESP(x64emu_t *emu, uint32_t v);
void SetEIP(x64emu_t *emu, uint32_t v);
void SetRAX(x64emu_t *emu, uint64_t v);
void SetRBX(x64emu_t *emu, uint64_t v);
void SetRCX(x64emu_t *emu, uint64_t v);
void SetRDX(x64emu_t *emu, uint64_t v);
void SetRDI(x64emu_t *emu, uint64_t v);
void SetRSI(x64emu_t *emu, uint64_t v);
void SetRBP(x64emu_t *emu, uint64_t v);
void SetRSP(x64emu_t *emu, uint64_t v);
void SetRIP(x64emu_t *emu, uint64_t v);
void SetFS(x64emu_t *emu, uint16_t v);
uint16_t GetFS(x64emu_t *emu);
uint64_t GetRSP(x64emu_t *emu);
uint64_t GetRBP(x64emu_t *emu);

#endif // __BOX64CPU_UTIL_H_
