#ifndef __DYNAREC_LA464_HELPER_H__
#define __DYNAREC_LA464_HELPER_H__

// undef to get Close to SSE Float->int conversions
// #define PRECISE_CVT

#if STEP == 0
#include "dynarec_la464_pass0.h"
#elif STEP == 1
#include "dynarec_la464_pass1.h"
#elif STEP == 2
#include "dynarec_la464_pass2.h"
#elif STEP == 3
#include "dynarec_la464_pass3.h"
#endif

#include "debug.h"
#include "la464_emitter.h"
#include "../emu/x64primop.h"

#define F8      *(uint8_t*)(addr++)
#define F8S     *(int8_t*)(addr++)
#define F16     *(uint16_t*)(addr += 2, addr - 2)
#define F16S    *(int16_t*)(addr += 2, addr - 2)
#define F32     *(uint32_t*)(addr += 4, addr - 4)
#define F32S    *(int32_t*)(addr += 4, addr - 4)
#define F32S64  (uint64_t)(int64_t) F32S
#define F64     *(uint64_t*)(addr += 8, addr - 8)
#define PK(a)   *(uint8_t*)(addr + a)
#define PK16(a) *(uint16_t*)(addr + a)
#define PK32(a) *(uint32_t*)(addr + a)
#define PK64(a) *(uint64_t*)(addr + a)
#define PKip(a) *(uint8_t*)(ip + a)

// Strong mem emulation helpers
#define SMREAD_MIN  2
#define SMWRITE_MIN 1
// Sequence of Read will trigger a DMB on "first" read if strongmem is >= SMREAD_MIN
// Sequence of Write will trigger a DMB on "last" write if strongmem is >= 1
// All Write operation that might use a lock all have a memory barrier if strongmem is >= SMWRITE_MIN
// Opcode will read
#define SMREAD()                                                        \
    ;                                                                   \
    if ((dyn->smread == 0) && (box64_dynarec_strongmem > SMREAD_MIN)) { \
        SMDMB();                                                        \
    } else                                                              \
        dyn->smread = 1
// Opcode will read with option forced lock
#define SMREADLOCK(lock) \
    if ((lock) || ((dyn->smread == 0) && (box64_dynarec_strongmem > SMREAD_MIN))) { SMDMB(); }
// Opcode might read (depend on nextop)
#define SMMIGHTREAD() \
    if (!MODREG) { SMREAD(); }
// Opcode has wrote
#define SMWRITE() dyn->smwrite = 1
// Opcode has wrote (strongmem>1 only)
#define SMWRITE2() \
    if (box64_dynarec_strongmem > SMREAD_MIN) dyn->smwrite = 1
// Opcode has wrote with option forced lock
#define SMWRITELOCK(lock)                                  \
    if (lock || (box64_dynarec_strongmem > SMWRITE_MIN)) { \
        SMDMB();                                           \
    } else                                                 \
        dyn->smwrite = 1
// Opcode might have wrote (depend on nextop)
#define SMMIGHTWRITE() \
    if (!MODREG) { SMWRITE(); }
// Start of sequence
#define SMSTART() SMEND()
// End of sequence
#define SMEND()                                               \
    if (dyn->smwrite && box64_dynarec_strongmem) { DBAR(0); } \
    dyn->smwrite = 0;                                         \
    dyn->smread = 0;
// Force a Data memory barrier (for LOCK: prefix)
#define SMDMB()       \
    DBAR(0);          \
    dyn->smwrite = 0; \
    dyn->smread = 1

// LOCK_* define
#define LOCK_LOCK (int*)1

// GETGD    get x64 register in gd
#define GETGD                                              \
    do {                                                   \
        gd = xRAX + ((nextop & 0x38) >> 3) + (rex.r << 3); \
        if (gd >= 21) gd++;                                \
    } while (0);

// CALL will use x7 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL(F, ret) call_c(dyn, ninst, F, x7, ret, 1, 0)
// CALL_ will use x7 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL_(F, ret, reg) call_c(dyn, ninst, F, x7, ret, 1, reg)
// CALL_S will use x7 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2. Flags are not save/restored
#define CALL_S(F, ret) call_c(dyn, ninst, F, x7, ret, 0, 0)

#define MARKi(i)    dyn->insts[ninst].mark[i] = dyn->native_size
#define GETMARKi(i) dyn->insts[ninst].mark[i]
#define MARK        MARKi(0)
#define GETMARK     GETMARKi(0)
#define MARK2       MARKi(1)
#define GETMARK2    GETMARKi(1)
#define MARK3       MARKi(2)
#define GETMARK3    GETMARKi(2)

#define MARKFi(i)    dyn->insts[ninst].markf[i] = dyn->native_size
#define GETMARKFi(i) dyn->insts[ninst].markf[i]
#define MARKF        MARKFi(0)
#define GETMARKF     GETMARKFi(0)
#define MARKF2       MARKFi(1)
#define GETMARKF2    GETMARKFi(1)

#define MARKSEG     dyn->insts[ninst].markseg = dyn->native_size
#define GETMARKSEG  dyn->insts[ninst].markseg
#define MARKLOCK    dyn->insts[ninst].marklock = dyn->native_size
#define GETMARKLOCK dyn->insts[ninst].marklock

#ifndef READFLAGS
#define READFLAGS(A)

#endif

#ifndef BARRIER
#define BARRIER(A)
#endif
#ifndef DEFAULT
#define DEFAULT \
    *ok = -1;   \
    BARRIER(2)
#endif

#ifndef TABLE64
#define TABLE64(A, V)
#endif

#define ARCH_INIT()

#if STEP < 2
#define GETIP(A)
#define GETIP_(A)
#else
// put value in the Table64 even if not using it for now to avoid difference between Step2 and Step3. Needs to be optimized later...
#define GETIP(A)                                     \
    if (dyn->last_ip && ((A)-dyn->last_ip) < 2048) { \
        uint64_t _delta_ip = (A)-dyn->last_ip;       \
        dyn->last_ip += _delta_ip;                   \
        if (_delta_ip) {                             \
            ADDI_D(xRIP, xRIP, _delta_ip);           \
        }                                            \
    } else {                                         \
        dyn->last_ip = (A);                          \
        if (dyn->last_ip < 0xffffffff) {             \
            MOV64x(xRIP, dyn->last_ip);              \
        } else                                       \
            TABLE64(xRIP, dyn->last_ip);             \
    }
#define GETIP_(A)                                         \
    if (dyn->last_ip && ((A)-dyn->last_ip) < 2048) {      \
        int64_t _delta_ip = (A)-dyn->last_ip;             \
        if (_delta_ip) { ADDI_D(xRIP, xRIP, _delta_ip); } \
    } else {                                              \
        if ((A) < 0xffffffff) {                           \
            MOV64x(xRIP, (A));                            \
        } else                                            \
            TABLE64(xRIP, (A));                           \
    }
#endif
#define CLEARIP() dyn->last_ip = 0

#define MODREG ((nextop & 0xC0) == 0xC0)

void la464_epilog(void);
void* la464_next(x64emu_t* emu, uintptr_t addr);

#ifndef STEPNAME
#define STEPNAME3(N, M) N##M
#define STEPNAME2(N, M) STEPNAME3(N, M)
#define STEPNAME(N)     STEPNAME2(N, STEP)
#endif

#define native_pass STEPNAME(native_pass)

#define dynarec64_00 STEPNAME(dynarec64_00)

#define geted          STEPNAME(geted)
#define geted32        STEPNAME(geted32)
#define jump_to_epilog STEPNAME(jump_to_epilog)
#define jump_to_next   STEPNAME(jump_to_next)
#define call_c         STEPNAME(call_c)

#define fpu_reset           STEPNAME(fpu_reset)
#define fpu_reset_cache     STEPNAME(fpu_reset_cache)
#define fpu_propagate_stack STEPNAME(fpu_propagate_stack)
#define fpu_purgecache      STEPNAME(fpu_purgecache)
#define fpu_reflectcache    STEPNAME(fpu_reflectcache)
#define fpu_unreflectcache  STEPNAME(fpu_unreflectcache)

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_la464_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta);

/* setup r2 to address pointed by */
uintptr_t geted32(dynarec_la464_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta);

// generic x64 helper
void jump_to_epilog(dynarec_la464_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_next(dynarec_la464_t* dyn, uintptr_t ip, int reg, int ninst, int is32bits);
void call_c(dynarec_la464_t* dyn, int ninst, void* fnc, int reg, int ret, int saveflags, int save_reg);

void fpu_reset(dynarec_la464_t* dyn);
// reset the cache with n
void fpu_reset_cache(dynarec_la464_t* dyn, int ninst, int reset_n);
// propagate stack state
void fpu_propagate_stack(dynarec_la464_t* dyn, int ninst);
// purge the FPU cache (needs 3 scratch registers)
void fpu_purgecache(dynarec_la464_t* dyn, int ninst, int next, int s1, int s2, int s3);
void fpu_reflectcache(dynarec_la464_t* dyn, int ninst, int s1, int s2, int s3);
void fpu_unreflectcache(dynarec_la464_t* dyn, int ninst, int s1, int s2, int s3);

uintptr_t dynarec64_00(dynarec_la464_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);

#if STEP < 3
#define PASS3(A)
#else
#define PASS3(A) A
#endif

#if STEP < 3
#define MAYUSE(A) (void)A
#else
#define MAYUSE(A)
#endif

#define NOTEST(s1)                                       \
    if (box64_dynarec_test) {                            \
        ST_W(xZR, xEmu, offsetof(x64emu_t, test.test));  \
        ST_W(xZR, xEmu, offsetof(x64emu_t, test.clean)); \
    }

#define GOTEST(s1, s2)                                 \
    if (box64_dynarec_test) {                          \
        MOV32w(s2, 1);                                 \
        ST_W(s2, xEmu, offsetof(x64emu_t, test.test)); \
    }

#endif //__DYNAREC_LA464_HELPER_H__