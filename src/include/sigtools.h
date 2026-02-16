#ifndef __SIGTOOLS_H__
#define __SIGTOOLS_H__

#include <stdint.h>
#include <stddef.h>

#ifdef DYNAREC
#if defined(ARM64)
#include "dynarec/arm64/arm64_mapping.h"
#define CONTEXT_REG(P, X)   (P)->uc_mcontext.regs[X]
#define CONTEXT_PC(P)       (P)->uc_mcontext.pc
#elif defined(LA64)
#include "dynarec/la64/la64_mapping.h"
#define CONTEXT_REG(P, X)   (P)->uc_mcontext.__gregs[X]
#define CONTEXT_PC(P)       (P)->uc_mcontext.__pc;
#elif defined(RV64)
#include "dynarec/rv64/rv64_mapping.h"
#define CONTEXT_REG(P, X)   (P)->uc_mcontext.__gregs[X]
#define CONTEXT_PC(P)       (P)->uc_mcontext.__gregs[REG_PC]
#else
#error Unsupported Architecture
#endif //arch
#endif //dynarec

typedef struct dynablock_s dynablock_t;
typedef struct x64emu_s x64emu_t;
typedef struct x64_mcontext_s x64_mcontext_t;

// copy (general) REGS / Segments / IP / Flags from x64emu to mcontext (no x87/sse/avx)
void emu2mctx(x64_mcontext_t* ctx, x64emu_t* emu);
// copy (general) REGS / Segments / IP / Flags from mcontext to x64emu (no x87/sse/avx)
void mctx2emu(x64emu_t*, x64_mcontext_t* ctx);

void add_unaligned_address(uintptr_t addr);
int is_addr_unaligned(uintptr_t addr);
void add_autosmc_address(uintptr_t addr);
int is_addr_autosmc(uintptr_t addr);
#ifdef DYNAREC
int nUnalignedRange(uintptr_t start, size_t size);
void getUnalignedRange(uintptr_t start, size_t size, uintptr_t addrs[]);
int mark_db_unaligned(dynablock_t* db, uintptr_t x64pc);
int mark_db_autosmc(dynablock_t* db, uintptr_t x64pc);
#endif
int sigbus_specialcases(siginfo_t* info, void * ucntx, void* pc, void* _fpsimd, dynablock_t* db, uintptr_t x64pc, int is32bits);

int unlockMutex();
int checkMutex(uint32_t mask);

int write_opcode(uintptr_t rip, uintptr_t native_ip, int is32bits);
void adjustregs(x64emu_t* emu, void* pc);

void copyUCTXreg2Emu(x64emu_t* emu, ucontext_t* p, uintptr_t ip);
void copyEmu2USignalCTXreg(ucontext_t* p, x64emu_t* emu, void* new_pc);

//1<<1 is mutex_prot, 1<<8 is mutex_dyndump
#define is_memprot_locked (1<<1)
#define is_dyndump_locked (1<<8)

#endif