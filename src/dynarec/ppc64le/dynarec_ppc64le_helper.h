#ifndef __DYNAREC_PPC64LE_HELPER_H__
#define __DYNAREC_PPC64LE_HELPER_H__

// undef to get Close to SSE Float->int conversions
// #define PRECISE_CVT

#if STEP == 0
#include "dynarec_ppc64le_pass0.h"
#elif STEP == 1
#include "dynarec_ppc64le_pass1.h"
#elif STEP == 2
#include "dynarec_ppc64le_pass2.h"
#elif STEP == 3
#include "dynarec_ppc64le_pass3.h"
#endif

#include "debug.h"
#include "ppc64le_emitter.h"
#include "../emu/x64primop.h"
#include "dynarec_ppc64le_consts.h"

// geted() displacement alignment constants.
// The i12 parameter IS the alignment mask: 0 disables inline displacement,
// nonzero enables it and specifies the required alignment.
//
// PPC64LE load/store instruction forms have different alignment requirements:
//   - D-form (LWZ/STW/LBZ/LHZ/LHA/STB/STH): byte-aligned, but we use DS_DISP
//     because LD/STD (DS-form) require 4-byte alignment and LDxw/SDxw switch
//     between them at runtime based on rex.w.
//   - DS-form (LD/STD): 4-byte aligned displacement
//   - DQ-form (LXV/STXV): 16-byte aligned displacement (low 4 bits silently
//     truncated by hardware — see ppc64le_emitter.h DQ-form section)
#define NO_DISP     0       // no inline displacement; always materialize in register
#define DS_DISP     3       // DS-form: inline if 4-byte aligned and in range
#define DQ_DISP     15      // DQ-form: inline if 16-byte aligned and in range

#define F8      *(uint8_t*)(addr++)
#define F8S     *(int8_t*)(addr++)
#define F16     *(uint16_t*)(addr += 2, addr - 2)
#define F16S    *(int16_t*)(addr += 2, addr - 2)
#define F32     *(uint32_t*)(addr += 4, addr - 4)
#define F32S    *(int32_t*)(addr += 4, addr - 4)
#define F32S64  (uint64_t)(int64_t)F32S
#define F64     *(uint64_t*)(addr += 8, addr - 8)
#define PK(a)   *(uint8_t*)(addr + a)
#define PK16(a) *(uint16_t*)(addr + a)
#define PK32(a) *(uint32_t*)(addr + a)
#define PK64(a) *(uint64_t*)(addr + a)
#define PKip(a) *(uint8_t*)(ip + a)

// LOCK_* define
#define LOCK_LOCK (int*)1

// ========================================================================
// Convenience macros for PPC64LE (equivalent to LA64's LDxw, SDxw, etc.)
// ========================================================================
// LDxw: load 32 or 64 bits depending on rex.w
// For 64-bit (LD), offset must be 4-aligned (DS-form); geted() guarantees this when i12=1.
#define LDxw(Rt, Ra, offset)                                        \
    if (rex.w) { LD(Rt, offset, Ra); } else { LWZ(Rt, offset, Ra); }
// LDz: load 32 or 64 bits depending on rex.is32bits
#define LDz(Rt, Ra, offset)                                         \
    if (rex.is32bits) { LWZ(Rt, offset, Ra); } else { LD(Rt, offset, Ra); }
// SDxw: store 32 or 64 bits depending on rex.w
// For 64-bit (STD), offset must be 4-aligned (DS-form); geted() guarantees this when i12=1.
#define SDxw(Rs, Ra, offset)                                        \
    if (rex.w) { STD(Rs, offset, Ra); } else { STW(Rs, offset, Ra); }
// SDz: store 32 or 64 bits depending on rex.is32bits
#define SDz(Rs, Ra, offset)                                         \
    if (rex.is32bits) { STW(Rs, offset, Ra); } else { STD(Rs, offset, Ra); }

// SDXxw: indexed store 32 or 64 bits
#define SDXxw(Rs, Ra, Rb) \
    if (rex.w) { STDX(Rs, Ra, Rb); } else { STWX(Rs, Ra, Rb); }

// ADDxw: add then optionally zero upper 32 bits
#define ADDxw(Rt, Ra, Rb) \
    ADD(Rt, Ra, Rb); if (!rex.w) { RLDICL(Rt, Rt, 0, 32); }
// ADDz: add based on is32bits
#define ADDz(Rt, Ra, Rb) \
    ADD(Rt, Ra, Rb); if (rex.is32bits) { RLDICL(Rt, Rt, 0, 32); }
// SUBxw: sub then optionally zero upper 32 bits
#define SUBxw(Rt, Ra, Rb) \
    SUB(Rt, Ra, Rb); if (!rex.w) { RLDICL(Rt, Rt, 0, 32); }
// ADDIxw: addi then optionally zero upper 32 bits
#define ADDIxw(Rt, Ra, imm) \
    ADDI(Rt, Ra, imm); if (!rex.w) { RLDICL(Rt, Rt, 0, 32); }
// ADDIz: addi then optionally zero upper 32 bits (based on is32bits)
#define ADDIz(Rt, Ra, imm) \
    ADDI(Rt, Ra, imm); if (rex.is32bits) { RLDICL(Rt, Rt, 0, 32); }

// ZEROUP: clear upper 32 bits
#define ZEROUP(Rd) RLDICL(Rd, Rd, 0, 32)

// ZEROUP2: clear upper 32 bits (different src/dst)
#define ZEROUP2(Rd, Rs) RLDICL(Rd, Rs, 0, 32)

// SEXT_W: sign extend 32-bit to 64-bit
#define SEXT_W(Rd, Rs) EXTSW(Rd, Rs)

// MVxw: move, optionally zero upper 32 bits
#define MVxw(Rd, Rs) \
    if (rex.w) { MV(Rd, Rs); } else { RLDICL(Rd, Rs, 0, 32); }

// MVz: move based on is32bits
#define MVz(Rd, Rs)              \
    do {                         \
        if (rex.is32bits) {      \
            ZEROUP2(Rd, Rs);     \
        } else {                 \
            MV(Rd, Rs);          \
        }                        \
    } while (0)

// MOV64y: load constant 32 or 64 depending on is32bits/is67
#define MOV64y(A, B)                    \
    do {                                \
        if (rex.is32bits || rex.is67) { \
            MOV32w(A, B);               \
        } else {                        \
            MOV64x(A, B);               \
        }                               \
    } while (0)

// SLLIxw: shift left immediate, conditional 32/64 based on rex.w
#define SLLIxw(Rd, Rs, imm)    \
    do {                       \
        if (rex.w) {           \
            SLDI(Rd, Rs, imm); \
        } else {               \
            SLWI(Rd, Rs, imm); \
            ZEROUP(Rd);        \
        }                      \
    } while (0)

// SRLIxw: shift right logical immediate, conditional 32/64 based on rex.w
#define SRLIxw(Rd, Rs, imm)    \
    do {                       \
        if (rex.w) {           \
            SRDI(Rd, Rs, imm); \
        } else {               \
            SRWI(Rd, Rs, imm); \
            ZEROUP(Rd);        \
        }                      \
    } while (0)

// SRAIxw: shift right arithmetic immediate, conditional 32/64 based on rex.w
#define SRAIxw(Rd, Rs, imm)      \
    do {                         \
        if (rex.w) {             \
            SRADI(Rd, Rs, imm);  \
        } else {                 \
            SRAWI(Rd, Rs, imm);  \
            ZEROUP(Rd);          \
        }                        \
    } while (0)

// SLLxw: variable shift left, conditional 32/64 based on rex.w
#define SLLxw(Rd, Rs, Rb)     \
    do {                      \
        if (rex.w) {          \
            SLD(Rd, Rs, Rb);  \
        } else {              \
            SLW(Rd, Rs, Rb);  \
            ZEROUP(Rd);       \
        }                     \
    } while (0)

// SRLxw: variable shift right logical, conditional 32/64 based on rex.w
#define SRLxw(Rd, Rs, Rb)     \
    do {                      \
        if (rex.w) {          \
            SRD(Rd, Rs, Rb);  \
        } else {              \
            SRW(Rd, Rs, Rb);  \
            ZEROUP(Rd);       \
        }                     \
    } while (0)

// SRAxw: variable shift right arithmetic, conditional 32/64 based on rex.w
#define SRAxw(Rd, Rs, Rb)      \
    do {                       \
        if (rex.w) {           \
            SRAD(Rd, Rs, Rb);  \
        } else {               \
            SRAW(Rd, Rs, Rb);  \
            ZEROUP(Rd);        \
        }                      \
    } while (0)

// SLLIy: shift left, zero-extend if 32-bit mode
#define SLLIy(Rd, Rs, imm)              \
    do {                                 \
        if (rex.is32bits || rex.is67) {  \
            SLWI(Rd, Rs, imm);           \
            ZEROUP(Rd);                  \
        } else                           \
            SLDI(Rd, Rs, imm);           \
    } while (0)

// SLADDy: shift-left-and-add (rd = (rj << imm) + rk), conditional 32/64
// PPC64LE has no fused shift-add, so: if imm==0 then ADD, else SLDI+ADD
// IMPORTANT: When rd==rk, shift into r0 (scratch) first to avoid clobbering rk.
// This includes the case rd==rj==rk (e.g. LEA rax,[rax+rax*4] => rax = rax*5).
// r0 is safe in X-form instructions (SLDI, SLWI, ADD) — only D-form base treats r0 as literal 0.
#define SLADDy(rd, rj, rk, imm)               \
    do {                                       \
        if ((imm) == 0) {                      \
            if (rex.is32bits || rex.is67) {     \
                ADD(rd, rj, rk);               \
                ZEROUP(rd);                    \
            } else {                           \
                ADD(rd, rj, rk);               \
            }                                  \
        } else if ((rd) == (rk)) {             \
            /* rd==rk: shift rj into r0 to avoid clobbering rk */ \
            if (rex.is32bits || rex.is67) {     \
                SLWI(0, rj, imm);              \
                ADD(rd, 0, rk);                \
                ZEROUP(rd);                    \
            } else {                           \
                SLDI(0, rj, imm);              \
                ADD(rd, 0, rk);                \
            }                                  \
        } else {                               \
            if (rex.is32bits || rex.is67) {     \
                SLWI(rd, rj, imm);             \
                ADD(rd, rd, rk);               \
                ZEROUP(rd);                    \
            } else {                           \
                SLDI(rd, rj, imm);             \
                ADD(rd, rd, rk);               \
            }                                  \
        }                                      \
    } while (0)

// ADDy: add, conditional 32/64
#define ADDy(rd, rj, rk)                \
    do {                                \
        if (rex.is32bits || rex.is67) { \
            ADD(rd, rj, rk);            \
            ZEROUP(rd);                 \
        } else                          \
            ADD(rd, rj, rk);            \
    } while (0)

// ADDIy: addi, conditional 32/64
#define ADDIy(rd, rj, imm)              \
    do {                                 \
        if (rex.is32bits || rex.is67) {  \
            ADDI(rd, rj, imm);           \
            ZEROUP(rd);                  \
        } else                           \
            ADDI(rd, rj, imm);           \
    } while (0)

// ADDxREGy: add, zero-extending rk to 32-bit first if needed (for seg+reg in 32-bit mode)
#define ADDxREGy(rd, rj, rk, s1)        \
    do {                                 \
        if (rex.is32bits || rex.is67) {  \
            ZEROUP2(s1, rk);             \
            ADD(rd, rj, s1);             \
        } else                           \
            ADD(rd, rj, rk);             \
    } while (0)

// BF_EXTRACT: extract bits hi:lo from Rs into Ra (zero-extend)
// Implements via RLDICL(Ra, Rs, (64-lo)%64, 64-(hi-lo+1))
#define BF_EXTRACT(Ra, Rs, hi, lo) \
    RLDICL(Ra, Rs, (64-(lo))%64, 64-((hi)-(lo)+1))

// BF_INSERT: insert low bits of Rs into Ra at position hi:lo
// Implements via RLDIMI(Ra, Rs, lo, 63-hi)
// IMPORTANT: When Rs == xZR (r0), r0 is NOT a zero register on PPC64LE.
// We load 0 into x4 scratch and use that instead.
#define BF_INSERT(Ra, Rs, hi, lo)                                     \
    do {                                                              \
        if ((Rs) == xZR) {                                            \
            LI(x4, 0);                                                \
            RLDIMI(Ra, x4, lo, 63-(hi));                              \
        } else {                                                      \
            RLDIMI(Ra, Rs, lo, 63-(hi));                              \
        }                                                             \
    } while (0)

// ========================================================================
// GETGD    get x64 register in gd
// ========================================================================
#define GETGD gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));

// GETED can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETED(D)                                                                                \
    if (MODREG) {                                                                               \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                               \
        wback = 0;                                                                              \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, DS_DISP, D); \
        LDxw(x1, wback, fixedaddress);                                                          \
        ed = x1;                                                                                \
    }

// Write back ed in wback (if wback not 0)
#define WBACK                              \
    if (wback) {                           \
        SDxw(ed, wback, fixedaddress);     \
        SMWRITE();                         \
    }

// GETEB will use i for ed, and can use r3 for wback.
#define GETEB(i, D)                                                                             \
    if (MODREG) {                                                                               \
        if (rex.rex) {                                                                          \
            wback = TO_NAT((nextop & 7) + (rex.b << 3));                                        \
            wb2 = 0;                                                                            \
        } else {                                                                                \
            wback = (nextop & 7);                                                               \
            wb2 = (wback >> 2) * 8;                                                             \
            wback = TO_NAT((wback & 3));                                                        \
        }                                                                                       \
        BF_EXTRACT(i, wback, wb2 + 7, wb2);                                                     \
        wb1 = 0;                                                                                \
        ed = i;                                                                                 \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, DS_DISP, D); \
        LBZ(i, fixedaddress, wback);                                                            \
        wb1 = 1;                                                                                \
        ed = i;                                                                                 \
    }

// GETGB will use i for gd
#define GETGB(i)                                             \
    if (rex.rex) {                                           \
        gb1 = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); \
        gb2 = 0;                                             \
    } else {                                                 \
        gd = (nextop & 0x38) >> 3;                           \
        gb2 = ((gd & 4) << 1);                               \
        gb1 = TO_NAT((gd & 3));                              \
    }                                                        \
    gd = i;                                                  \
    BF_EXTRACT(gd, gb1, gb2 + 7, gb2);

#define GETGBEB(i, j, D)                                         \
    GETEB(j, D);                                                 \
    if (MODREG) {                                                \
        if (rex.rex) {                                           \
            gb1 = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); \
            gb2 = 0;                                             \
        } else {                                                 \
            gd = (nextop & 0x38) >> 3;                           \
            gb2 = ((gd & 4) << 1);                               \
            gb1 = TO_NAT((gd & 3));                              \
        }                                                        \
        if (gb1 == wback && gb2 == wb2)                          \
            gd = ed;                                             \
        else {                                                   \
            GETGB(i);                                            \
        }                                                        \
    } else {                                                     \
        GETGB(i);                                                \
    }

// Write gb (gd) back to original register / memory
#define GBBACK() BF_INSERT(gb1, gd, gb2 + 7, gb2);

// Write eb (ed) back to original register / memory
#define EBBACK()                            \
    if (wb1) {                              \
        STB(ed, fixedaddress, wback);       \
        SMWRITE();                          \
    } else {                                \
        BF_INSERT(wback, ed, wb2 + 7, wb2); \
    }

// CALL will use x6 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL(F, ret, arg1, arg2)                          call_c(dyn, ninst, F, x6, ret, 1, 0, arg1, arg2, 0, 0, 0, 0)
#define CALL4(F, ret, arg1, arg2, arg3, arg4)             call_c(dyn, ninst, F, x6, ret, 1, 0, arg1, arg2, arg3, arg4, 0, 0)
#define CALL6(F, ret, arg1, arg2, arg3, arg4, arg5, arg6) call_c(dyn, ninst, F, x6, ret, 1, 0, arg1, arg2, arg3, arg4, arg5, arg6)
// CALL_ will use x6 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL_(F, ret, reg, arg1, arg2)              call_c(dyn, ninst, F, x6, ret, 1, reg, arg1, arg2, 0, 0, 0, 0)
#define CALL4_(F, ret, reg, arg1, arg2, arg3, arg4) call_c(dyn, ninst, F, x6, ret, 1, reg, arg1, arg2, arg3, arg4, 0, 0)
// CALL_S will use x6 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2. Flags are not saved/restored
#define CALL_S(F, ret, arg1) call_c(dyn, ninst, F, x6, ret, 0, 0, arg1, 0, 0, 0, 0, 0)

// ========================================================================
// MARK / GETMARK system
// ========================================================================
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

#define MARKSEG      dyn->insts[ninst].markseg = dyn->native_size
#define GETMARKSEG   dyn->insts[ninst].markseg
#define MARKLOCK     dyn->insts[ninst].marklock = dyn->native_size
#define GETMARKLOCK  dyn->insts[ninst].marklock
#define MARKLOCK2    dyn->insts[ninst].marklock2 = dyn->native_size
#define GETMARKLOCK2 dyn->insts[ninst].marklock2

// ========================================================================
// Branch-to-mark macros for PPC64LE
// ========================================================================
// PPC64LE needs compare + branch (2 instructions) for register comparisons,
// but can use direct conditional branches on CR0 after ANDId.
// For reg-reg comparison: CMPD(reg1, reg2); BC(BO, BI, offset)
// The offset must be computed AFTER the compare instruction is emitted.

// Bxx_gen: compare two registers, then branch
// Uses CMPD_ZR to handle xZR (r0) safely — see note at CMPD_ZR definition
#define Bxx_gen(COND, M, reg1, reg2)             \
    CMPD_ZR(reg1, reg2);                         \
    j64 = GET##M - dyn->native_size;             \
    B##COND(j64)

// BxxZ_gen: compare register with zero, then branch
#define BxxZ_gen(COND, M, reg)                   \
    CMPDI(reg, 0);                               \
    j64 = GET##M - dyn->native_size;             \
    B##COND(j64)

// Branch to MARK if reg1!=reg2 (use j64)
#define BNE_MARK(reg1, reg2) Bxx_gen(NE, MARK, reg1, reg2)
// Branch to MARK2 if reg1!=reg2 (use j64)
#define BNE_MARK2(reg1, reg2) Bxx_gen(NE, MARK2, reg1, reg2)
// Branch to MARK3 if reg1!=reg2 (use j64)
#define BNE_MARK3(reg1, reg2) Bxx_gen(NE, MARK3, reg1, reg2)
// Branch to MARKLOCK if reg1!=reg2 (use j64)
#define BNE_MARKLOCK(reg1, reg2) Bxx_gen(NE, MARKLOCK, reg1, reg2)
// Branch to MARKLOCK2 if reg1!=reg2 (use j64)
#define BNE_MARKLOCK2(reg1, reg2) Bxx_gen(NE, MARKLOCK2, reg1, reg2)

// Branch to MARK if reg1==reg2 (use j64)
#define BEQ_MARK(reg1, reg2) Bxx_gen(EQ, MARK, reg1, reg2)
// Branch to MARK2 if reg1==reg2 (use j64)
#define BEQ_MARK2(reg1, reg2) Bxx_gen(EQ, MARK2, reg1, reg2)
// Branch to MARK3 if reg1==reg2 (use j64)
#define BEQ_MARK3(reg1, reg2) Bxx_gen(EQ, MARK3, reg1, reg2)
// Branch to MARKLOCK if reg1==reg2 (use j64)
#define BEQ_MARKLOCK(reg1, reg2) Bxx_gen(EQ, MARKLOCK, reg1, reg2)
// Branch to MARKLOCK2 if reg1==reg2 (use j64)
#define BEQ_MARKLOCK2(reg1, reg2) Bxx_gen(EQ, MARKLOCK2, reg1, reg2)

// Branch to MARK if reg1==0 (use j64)
#define BEQZ_MARK(reg) BxxZ_gen(EQ, MARK, reg)
// Branch to MARK2 if reg1==0 (use j64)
#define BEQZ_MARK2(reg) BxxZ_gen(EQ, MARK2, reg)
// Branch to MARK3 if reg1==0 (use j64)
#define BEQZ_MARK3(reg) BxxZ_gen(EQ, MARK3, reg)
// Branch to MARKLOCK if reg1==0 (use j64)
#define BEQZ_MARKLOCK(reg) BxxZ_gen(EQ, MARKLOCK, reg)
// Branch to MARKLOCK2 if reg1==0 (use j64)
#define BEQZ_MARKLOCK2(reg) BxxZ_gen(EQ, MARKLOCK2, reg)

// Branch to MARK if reg1!=0 (use j64)
#define BNEZ_MARK(reg) BxxZ_gen(NE, MARK, reg)
// Branch to MARK2 if reg1!=0 (use j64)
#define BNEZ_MARK2(reg) BxxZ_gen(NE, MARK2, reg)
// Branch to MARK3 if reg1!=0 (use j64)
#define BNEZ_MARK3(reg) BxxZ_gen(NE, MARK3, reg)
// Branch to MARKLOCK if reg1!=0 (use j64)
#define BNEZ_MARKLOCK(reg) BxxZ_gen(NE, MARKLOCK, reg)
// Branch to MARKLOCK2 if reg1!=0 (use j64)
#define BNEZ_MARKLOCK2(reg) BxxZ_gen(NE, MARKLOCK2, reg)

// Branch to MARK if reg1<reg2 (signed) (use j64)
#define BLT_MARK(reg1, reg2) Bxx_gen(LT, MARK, reg1, reg2)
// Branch to MARK if reg1<reg2 (unsigned) (use j64)
// PPC64LE: use CMPLD_ZR for unsigned, then BLT
#define BLTU_MARK(reg1, reg2)                    \
    CMPLD_ZR(reg1, reg2);                        \
    j64 = GETMARK - dyn->native_size;            \
    BLT(j64)
// Branch to MARK if reg1>=reg2 (use j64)
#define BGE_MARK(reg1, reg2) Bxx_gen(GE, MARK, reg1, reg2)
// Branch to MARK2 if reg1>=reg2 (use j64)
#define BGE_MARK2(reg1, reg2) Bxx_gen(GE, MARK2, reg1, reg2)
// Branch to MARK3 if reg1>=reg2 (use j64)
#define BGE_MARK3(reg1, reg2) Bxx_gen(GE, MARK3, reg1, reg2)

// Branch to MARKLOCK on CR0.EQ=0 (for STWCX./STDCX. retry loops)
#define BNE_MARKLOCK_CR0                         \
    j64 = GETMARKLOCK - dyn->native_size;        \
    BNE(j64)
// Branch to MARKLOCK2 on CR0.EQ=0 (for STWCX./STDCX. retry loops)
#define BNE_MARKLOCK2_CR0                        \
    j64 = GETMARKLOCK2 - dyn->native_size;       \
    BNE(j64)
// Branch to MARK2 on CR0.EQ=0 (for STWCX./STDCX. retry loops)
#define BNE_MARK2_CR0                            \
    j64 = GETMARK2 - dyn->native_size;           \
    BNE(j64)

// Branch to MARK unconditional (use j64)
#define B_MARK_nocond                            \
    j64 = GETMARK - dyn->native_size;            \
    B(j64)
// Branch to MARK2 unconditional (use j64)
#define B_MARK2_nocond                           \
    j64 = GETMARK2 - dyn->native_size;           \
    B(j64)
// Branch to MARK3 unconditional (use j64)
#define B_MARK3_nocond                           \
    j64 = GETMARK3 - dyn->native_size;           \
    B(j64)

// Branch to NEXT if reg1==0 (use j64)
#define CBZ_NEXT(reg1)                                                        \
    CMPDI(reg1, 0);                                                           \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    BEQ(j64)
// Branch to NEXT if reg1!=0 (use j64)
#define CBNZ_NEXT(reg1)                                                       \
    CMPDI(reg1, 0);                                                           \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    BNE(j64)
#define B_NEXT_nocond                                                         \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    B(j64)

// Branch to MARKSEG if reg is 0 (use j64)
#define CBZ_MARKSEG(reg)                   \
    CMPDI(reg, 0);                         \
    j64 = GETMARKSEG - (dyn->native_size); \
    BEQ(j64);
// Branch to MARKSEG if reg is not 0 (use j64)
#define CBNZ_MARKSEG(reg)                  \
    CMPDI(reg, 0);                         \
    j64 = GETMARKSEG - (dyn->native_size); \
    BNE(j64);

// ========================================================================
// IFX / flags macros (undef pass0 versions that use set_flags,
// redefine for passes 1-3 using gen_flags)
// ========================================================================
#undef IFX
#undef IFX_PENDOR0
#undef IFXX
#undef IFX2X
#undef IFXN
#define IFX(A)      if ((dyn->insts[ninst].x64.gen_flags & (A)))
#define IFXA(A, B)  if ((dyn->insts[ninst].x64.gen_flags & (A)) && (B))
#define IFXORNAT(A) if ((dyn->insts[ninst].x64.gen_flags & (A)) || dyn->insts[ninst].nat_flags_fusion)
#define IFX_PENDOR0 if ((dyn->insts[ninst].x64.gen_flags & (X_PEND) || (!dyn->insts[ninst].x64.gen_flags && !dyn->insts[ninst].nat_flags_fusion)))
#define IFXX(A)     if ((dyn->insts[ninst].x64.gen_flags == (A)))
#define IFX2X(A, B) if ((dyn->insts[ninst].x64.gen_flags == (A) || dyn->insts[ninst].x64.gen_flags == (B) || dyn->insts[ninst].x64.gen_flags == ((A) | (B))))
#define IFXN(A, B)  if ((dyn->insts[ninst].x64.gen_flags & (A) && !(dyn->insts[ninst].x64.gen_flags & (B))))

#ifndef NATIVE_RESTORE_X87PC
#define NATIVE_RESTORE_X87PC()                     \
    if (dyn->need_x87check) {                      \
        LD(x87pc, offsetof(x64emu_t, cw), xEmu);  \
        SRDI(x87pc, x87pc, 8);                     \
        ANDId(x87pc, x87pc, 0b11);                 \
    }
#endif
#ifndef X87_CHECK_PRECISION
#define X87_CHECK_PRECISION(A)               \
    if (!ST_IS_F(0) && dyn->need_x87check) { \
        CMPDI(x87pc, 0);                     \
        BNE(4 + 4);                          \
        FRSP(A, A);                          \
    }
#endif

// STORE_REG / LOAD_REG — PPC64LE operand order: STD(Rs, offset, Ra)
#define STORE_REG(A) STD(x##A, offsetof(x64emu_t, regs[_##A]), xEmu)
#define LOAD_REG(A)  LD(x##A, offsetof(x64emu_t, regs[_##A]), xEmu)

// Need to also store current value of some register, as they may be used by functions like setjmp
#define STORE_XEMU_CALL() \
    STORE_REG(RBX);       \
    STORE_REG(RSP);       \
    STORE_REG(RBP);       \
    STORE_REG(R10);       \
    STORE_REG(R11);       \
    STORE_REG(R12);       \
    STORE_REG(R13);       \
    STORE_REG(R14);       \
    STORE_REG(R15);

#define LOAD_XEMU_CALL()

#define LOAD_XEMU_REM() \
    LOAD_REG(RBX);      \
    LOAD_REG(RSP);      \
    LOAD_REG(RBP);      \
    LOAD_REG(R10);      \
    LOAD_REG(R11);      \
    LOAD_REG(R12);      \
    LOAD_REG(R13);      \
    LOAD_REG(R14);      \
    LOAD_REG(R15);

// PPC64LE has no xZR register that can be stored — must use LI+STW for zero stores
// Use x4 (r6) as scratch instead of x1 (r3) since x1 often holds live 'ed' values
#define FORCE_DFNONE()               \
    do {                             \
        LI(x4, 0);                   \
        STW(x4, offsetof(x64emu_t, df), xEmu); \
    } while (0)

#define CHECK_DFNONE(N)                      \
    do {                                     \
        if (dyn->f == status_none_pending) { \
            FORCE_DFNONE();                  \
            if (N) dyn->f = status_none;     \
        }                                    \
    } while (0)

#define SET_DFNONE()                      \
    do {                                  \
        if (dyn->f != status_none) {      \
            dyn->f = status_none_pending; \
        }                                 \
    } while (0)

#define SET_DF(S, N)                           \
    if ((N) != d_none) {                       \
        MOV32w(S, (N));                        \
        STW(S, offsetof(x64emu_t, df), xEmu);  \
        dyn->f = status_set;                   \
    } else                                     \
        SET_DFNONE()

#define CLEAR_FLAGS_(s)                                                                                       \
    MOV64x(s, (1UL << F_AF) | (1UL << F_CF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF)); \
    ANDC(xFlags, xFlags, s);

#define CLEAR_FLAGS(s) \
    IFX (X_ALL) { CLEAR_FLAGS_(s) }

#define CALC_SUB_FLAGS(op1_, op2, res, scratch1, scratch2, width)     \
    IFX (X_AF | X_CF | X_OF) {                                        \
        /* calc borrow chain */                                       \
        /* bc = (res & (~op1 | op2)) | (~op1 & op2) */                \
        OR(scratch1, op1_, op2);                                      \
        AND(scratch2, res, scratch1);                                 \
        AND(op1_, op1_, op2);                                         \
        OR(scratch2, scratch2, op1_);                                 \
        IFX (X_AF) {                                                  \
            ANDId(scratch1, scratch2, 8);                              \
            SLDI(scratch1, scratch1, F_AF - 3);                       \
            OR(xFlags, xFlags, scratch1);                             \
        }                                                             \
        IFX (X_CF) {                                                  \
            /* cf = bc & (1<<(width-1)) */                            \
            if ((width) == 8) {                                       \
                ANDId(scratch1, scratch2, 0x80);                      \
            } else {                                                  \
                SRDI(scratch1, scratch2, (width) - 1);                \
                if ((width) != 64) ANDId(scratch1, scratch1, 1);      \
            }                                                         \
            CMPDI(scratch1, 0);                                       \
            BEQ(8);                                                   \
            ORI(xFlags, xFlags, 1 << F_CF);                           \
        }                                                             \
        IFX (X_OF) {                                                  \
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */ \
            SRDI(scratch1, scratch2, (width) - 2);                    \
            SRDI(scratch2, scratch1, 1);                              \
            XOR(scratch1, scratch1, scratch2);                        \
            ANDId(scratch1, scratch1, 1);                             \
            CMPDI(scratch1, 0);                                       \
            BEQ(8);                                                   \
            ORI(xFlags, xFlags, 1 << F_OF);                           \
        }                                                             \
    }

// ========================================================================
// X87 push/pop macros
// ========================================================================
#if STEP == 0
#define X87_PUSH_OR_FAIL(var, dyn, ninst, scratch, t) var = x87_do_push(dyn, ninst, scratch, t)
#define X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, scratch)   x87_do_push_empty(dyn, ninst, scratch)
#define X87_POP_OR_FAIL(dyn, ninst, scratch)          x87_do_pop(dyn, ninst, scratch)
#else
#define X87_PUSH_OR_FAIL(var, dyn, ninst, scratch, t)                                                                                                \
    if ((dyn->v.x87stack == 8) || (dyn->v.pushed == 8)) {                                                                                            \
        if (dyn->need_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Push, stack=%d/%d on inst %d\n", dyn->v.x87stack, dyn->v.pushed, ninst); \
        dyn->abort = 1;                                                                                                                              \
        return addr;                                                                                                                                 \
    }                                                                                                                                                \
    var = x87_do_push(dyn, ninst, scratch, t);

#define X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, scratch)                                                                                                  \
    if ((dyn->v.x87stack == 8) || (dyn->v.pushed == 8)) {                                                                                            \
        if (dyn->need_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Push, stack=%d/%d on inst %d\n", dyn->v.x87stack, dyn->v.pushed, ninst); \
        dyn->abort = 1;                                                                                                                              \
        return addr;                                                                                                                                 \
    }                                                                                                                                                \
    x87_do_push_empty(dyn, ninst, scratch);

#define X87_POP_OR_FAIL(dyn, ninst, scratch)                                                                                                       \
    if ((dyn->v.x87stack == -8) || (dyn->v.poped == 8)) {                                                                                          \
        if (dyn->need_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Pop, stack=%d/%d on inst %d\n", dyn->v.x87stack, dyn->v.poped, ninst); \
        dyn->abort = 1;                                                                                                                            \
        return addr;                                                                                                                               \
    }                                                                                                                                              \
    x87_do_pop(dyn, ninst, scratch);
#endif

// ========================================================================
// READFLAGS / SETFLAGS / GRABFLAGS
// ========================================================================
#ifndef READFLAGS
#define READFLAGS(A)                           \
    if ((A) != X_PEND                          \
        && (dyn->f == status_unk)) {           \
        CALL_(const_updateflags, -1, 0, 0, 0); \
        dyn->f = status_none;                  \
    }
#endif

#ifndef READFLAGS_FUSION
#define READFLAGS_FUSION(A, s1, s2, s3, s4, s5)                                \
    if (dyn->insts[ninst].nat_flags_fusion)                                    \
        get_free_scratch(dyn, ninst, &tmp1, &tmp2, &tmp3, s1, s2, s3, s4, s5); \
    else {                                                                     \
        tmp1 = s1;                                                             \
        tmp2 = s2;                                                             \
        tmp3 = s3;                                                             \
    }                                                                          \
    READFLAGS(A)
#endif

#define NAT_FLAGS_OPS(op1, op2, s1, s2)                                     \
    do {                                                                    \
        dyn->insts[dyn->insts[ninst].nat_next_inst].nat_flags_op1 = op1;    \
        dyn->insts[dyn->insts[ninst].nat_next_inst].nat_flags_op2 = op2;    \
        if (dyn->insts[ninst + 1].no_scratch_usage && IS_GPR(op1)) {        \
            MV(s1, op1);                                                    \
            dyn->insts[dyn->insts[ninst].nat_next_inst].nat_flags_op1 = s1; \
        }                                                                   \
        if (dyn->insts[ninst + 1].no_scratch_usage && IS_GPR(op2)) {        \
            MV(s2, op2);                                                    \
            dyn->insts[dyn->insts[ninst].nat_next_inst].nat_flags_op2 = s2; \
        }                                                                   \
    } while (0)

#define NAT_FLAGS_ENABLE_CARRY() dyn->insts[ninst].nat_flags_carry = 1
#define NAT_FLAGS_ENABLE_SIGN()  dyn->insts[ninst].nat_flags_sign = 1
#define NAT_FLAGS_ENABLE_SF()    dyn->insts[ninst].nat_flags_sf = 1

#define GRABFLAGS(A)                                             \
    if ((A) != X_PEND                                            \
        && ((dyn->f == status_unk) || (dyn->f == status_set))) { \
        CALL_(const_updateflags, -1, 0, 0, 0);                   \
        dyn->f = status_none;                                    \
    }

#ifndef SETFLAGS
#define SETFLAGS(A, B, FUSION)                                                                                          \
    do {                                                                                                                \
        if (((B) & SF_SUB)                                                                                              \
            && (dyn->insts[ninst].x64.gen_flags & (~(A))))                                                              \
            GRABFLAGS(((dyn->insts[ninst].x64.gen_flags & X_PEND) ? X_ALL : dyn->insts[ninst].x64.gen_flags) & (~(A))); \
        if (dyn->insts[ninst].x64.gen_flags) switch (B) {                                                               \
                case SF_SET_DF: dyn->f = status_set; break;                                                             \
                case SF_SET_NODF: SET_DFNONE(); break;                                                                  \
                case SF_SUBSET:                                                                                         \
                case SF_SUBSET_PENDING:                                                                                 \
                case SF_SET:                                                                                            \
                case SF_PENDING:                                                                                        \
                case SF_SET_PENDING:                                                                                    \
                    SET_DFNONE();                                                                                       \
                    break;                                                                                              \
            }                                                                                                           \
        else                                                                                                            \
            SET_DFNONE();                                                                                               \
        dyn->insts[ninst].nat_flags_nofusion = (FUSION);                                                                \
    } while (0)
#endif
#ifndef JUMP
#define JUMP(A, C)
#endif
#ifndef BARRIER
#define BARRIER(A)
#endif
#ifndef SET_HASCALLRET
#define SET_HASCALLRET()
#endif

// UFLAG macros
#define UFLAG_OP1(A) \
    if (dyn->insts[ninst].x64.gen_flags) { SDxw(A, xEmu, offsetof(x64emu_t, op1)); }
#define UFLAG_OP2(A) \
    if (dyn->insts[ninst].x64.gen_flags) { SDxw(A, xEmu, offsetof(x64emu_t, op2)); }
#define UFLAG_OP12(A1, A2)                       \
    if (dyn->insts[ninst].x64.gen_flags) {       \
        SDxw(A1, xEmu, offsetof(x64emu_t, op1)); \
        SDxw(A2, xEmu, offsetof(x64emu_t, op2)); \
    }
#define UFLAG_RES(A) \
    if (dyn->insts[ninst].x64.gen_flags) { SDxw(A, xEmu, offsetof(x64emu_t, res)); }
#define UFLAG_DF(r, A) \
    if (dyn->insts[ninst].x64.gen_flags) { SET_DF(r, A); }
#undef UFLAG_IF
#undef UFLAG_IF2
#define UFLAG_IF if (dyn->insts[ninst].x64.gen_flags)
#ifndef DEFAULT
#define DEFAULT \
    *ok = -1;   \
    BARRIER(2)
#endif

#ifndef CALLRET_LOOP
#define CALLRET_LOOP()  NOP()
#endif

#ifndef TABLE64
#define TABLE64(A, V)
#endif
#ifndef FTABLE64
#define FTABLE64(A, V)
#endif
#ifndef TABLE64C
#define TABLE64C(A, V)
#endif

#ifndef TABLE64_
#define TABLE64_(A, V)
#endif

#define ARCH_INIT() SMSTART()

#define ARCH_RESET()

// ========================================================================
// GETIP
// ========================================================================
#if STEP < 2
#define GETIP(A, scratch)
#define GETIP_(A, scratch)
#else

// PPC64LE ADDI has 16-bit signed immediate (±32768), wider than LA64's 12-bit
#define GETIP_(A, scratch)                                         \
    do {                                                           \
        ssize_t _delta_ip = (ssize_t)(A) - (ssize_t)dyn->last_ip; \
        if (!dyn->last_ip) {                                       \
            if (dyn->need_reloc) {                                 \
                TABLE64(xRIP, (A));                                \
            } else {                                               \
                MOV64x(xRIP, (A));                                 \
            }                                                      \
        } else if (_delta_ip == 0) {                               \
        } else if (_delta_ip >= -32768 && _delta_ip < 32768) {     \
            ADDI(xRIP, xRIP, _delta_ip);                           \
        } else if (_delta_ip < 0 && _delta_ip >= -0xffffffffL) {   \
            MOV32w(scratch, -_delta_ip);                           \
            SUB(xRIP, xRIP, scratch);                              \
        } else if (_delta_ip > 0 && _delta_ip <= 0xffffffffL) {    \
            MOV32w(scratch, _delta_ip);                            \
            ADD(xRIP, xRIP, scratch);                              \
        } else {                                                   \
            if (dyn->need_reloc) {                                 \
                TABLE64(xRIP, (A));                                \
            } else {                                               \
                MOV64x(xRIP, (A));                                 \
            }                                                      \
        }                                                          \
    } while (0)
#define GETIP(A, scratch) \
    GETIP_(A, scratch);   \
    dyn->last_ip = (A);

#endif
#define CLEARIP() dyn->last_ip = 0

// ========================================================================
// PASS2IF / MODREG
// ========================================================================
#if STEP < 2
#define PASS2IF(A, B) if (A)
#elif STEP == 2
#define PASS2IF(A, B)                         \
    if (A) dyn->insts[ninst].pass2choice = B; \
    if (dyn->insts[ninst].pass2choice == B)
#else
#define PASS2IF(A, B) if (dyn->insts[ninst].pass2choice == B)
#endif

#define MODREG ((nextop & 0xC0) == 0xC0)

// ========================================================================
// STEPNAME mangling
// ========================================================================
#ifndef STEPNAME
#define STEPNAME3(N, M) N##M
#define STEPNAME2(N, M) STEPNAME3(N, M)
#define STEPNAME(N)     STEPNAME2(N, STEP)
#endif

#define native_pass STEPNAME(native_pass)

#define dynarec64_00          STEPNAME(dynarec64_00)
#define dynarec64_0F          STEPNAME(dynarec64_0F)
#define dynarec64_66          STEPNAME(dynarec64_66)
#define dynarec64_F30F        STEPNAME(dynarec64_F30F)
#define dynarec64_660F        STEPNAME(dynarec64_660F)
#define dynarec64_66F0        STEPNAME(dynarec64_66F0)
#define dynarec64_66F20F      STEPNAME(dynarec64_66F20F)
#define dynarec64_66F30F      STEPNAME(dynarec64_66F30F)
#define dynarec64_F0          STEPNAME(dynarec64_F0)
#define dynarec64_F20F        STEPNAME(dynarec64_F20F)
#define dynarec64_AVX         STEPNAME(dynarec64_AVX)
#define dynarec64_AVX_0F      STEPNAME(dynarec64_AVX_0F)
#define dynarec64_AVX_0F38    STEPNAME(dynarec64_AVX_0F38)
#define dynarec64_AVX_66_0F   STEPNAME(dynarec64_AVX_66_0F)
#define dynarec64_AVX_66_0F38 STEPNAME(dynarec64_AVX_66_0F38)
#define dynarec64_AVX_66_0F3A STEPNAME(dynarec64_AVX_66_0F3A)
#define dynarec64_AVX_F2_0F   STEPNAME(dynarec64_AVX_F2_0F)
#define dynarec64_AVX_F2_0F38 STEPNAME(dynarec64_AVX_F2_0F38)
#define dynarec64_AVX_F2_0F3A STEPNAME(dynarec64_AVX_F2_0F3A)
#define dynarec64_AVX_F3_0F   STEPNAME(dynarec64_AVX_F3_0F)
#define dynarec64_AVX_F3_0F38 STEPNAME(dynarec64_AVX_F3_0F38)

#define dynarec64_D8 STEPNAME(dynarec64_D8)
#define dynarec64_D9 STEPNAME(dynarec64_D9)
#define dynarec64_DA STEPNAME(dynarec64_DA)
#define dynarec64_DB STEPNAME(dynarec64_DB)
#define dynarec64_DC STEPNAME(dynarec64_DC)
#define dynarec64_DD STEPNAME(dynarec64_DD)
#define dynarec64_DE STEPNAME(dynarec64_DE)
#define dynarec64_DF STEPNAME(dynarec64_DF)
#define dynarec64_F0 STEPNAME(dynarec64_F0)

#define geted               STEPNAME(geted)
#define geted16             STEPNAME(geted16)
#define jump_to_epilog      STEPNAME(jump_to_epilog)
#define jump_to_epilog_fast STEPNAME(jump_to_epilog_fast)
#define jump_to_next        STEPNAME(jump_to_next)
#define ret_to_next         STEPNAME(ret_to_next)
#define iret_to_next        STEPNAME(iret_to_next)
#define call_c              STEPNAME(call_c)
#define call_n              STEPNAME(call_n)
#define grab_segdata        STEPNAME(grab_segdata)
#define emit_adc16          STEPNAME(emit_adc16)
#define emit_adc32          STEPNAME(emit_adc32)
#define emit_adc8           STEPNAME(emit_adc8)
#define emit_adc8c          STEPNAME(emit_adc8c)
#define emit_add16          STEPNAME(emit_add16)
#define emit_add32          STEPNAME(emit_add32)
#define emit_add32c         STEPNAME(emit_add32c)
#define emit_add8           STEPNAME(emit_add8)
#define emit_add8c          STEPNAME(emit_add8c)
#define emit_and16          STEPNAME(emit_and16)
#define emit_and32          STEPNAME(emit_and32)
#define emit_and32c         STEPNAME(emit_and32c)
#define emit_and8           STEPNAME(emit_and8)
#define emit_and8c          STEPNAME(emit_and8c)
#define emit_cmp16          STEPNAME(emit_cmp16)
#define emit_cmp16_0        STEPNAME(emit_cmp16_0)
#define emit_cmp32          STEPNAME(emit_cmp32)
#define emit_cmp32_0        STEPNAME(emit_cmp32_0)
#define emit_cmp8           STEPNAME(emit_cmp8)
#define emit_cmp8_0         STEPNAME(emit_cmp8_0)
#define emit_dec16          STEPNAME(emit_dec16)
#define emit_dec32          STEPNAME(emit_dec32)
#define emit_dec8           STEPNAME(emit_dec8)
#define emit_inc16          STEPNAME(emit_inc16)
#define emit_inc32          STEPNAME(emit_inc32)
#define emit_inc8           STEPNAME(emit_inc8)
#define emit_neg16          STEPNAME(emit_neg16)
#define emit_neg32          STEPNAME(emit_neg32)
#define emit_neg8           STEPNAME(emit_neg8)
#define emit_or16           STEPNAME(emit_or16)
#define emit_or32           STEPNAME(emit_or32)
#define emit_or32c          STEPNAME(emit_or32c)
#define emit_or8            STEPNAME(emit_or8)
#define emit_or8c           STEPNAME(emit_or8c)
#define emit_rcl16          STEPNAME(emit_rcl16)
#define emit_rcl16c         STEPNAME(emit_rcl16c)
#define emit_rcl32          STEPNAME(emit_rcl32)
#define emit_rcl32c         STEPNAME(emit_rcl32c)
#define emit_rcl8           STEPNAME(emit_rcl8)
#define emit_rcl8c          STEPNAME(emit_rcl8c)
#define emit_rcr16          STEPNAME(emit_rcr16)
#define emit_rcr16c         STEPNAME(emit_rcr16c)
#define emit_rcr32          STEPNAME(emit_rcr32)
#define emit_rcr32c         STEPNAME(emit_rcr32c)
#define emit_rcr8           STEPNAME(emit_rcr8)
#define emit_rcr8c          STEPNAME(emit_rcr8c)
#define emit_rol16          STEPNAME(emit_rol16)
#define emit_rol16c         STEPNAME(emit_rol16c)
#define emit_rol32          STEPNAME(emit_rol32)
#define emit_rol32c         STEPNAME(emit_rol32c)
#define emit_rol8           STEPNAME(emit_rol8)
#define emit_rol8c          STEPNAME(emit_rol8c)
#define emit_ror16          STEPNAME(emit_ror16)
#define emit_ror16c         STEPNAME(emit_ror16c)
#define emit_ror32          STEPNAME(emit_ror32)
#define emit_ror32c         STEPNAME(emit_ror32c)
#define emit_ror8           STEPNAME(emit_ror8)
#define emit_ror8c          STEPNAME(emit_ror8c)
#define emit_sar16          STEPNAME(emit_sar16)
#define emit_sar16c         STEPNAME(emit_sar16c)
#define emit_sar32          STEPNAME(emit_sar32)
#define emit_sar32c         STEPNAME(emit_sar32c)
#define emit_sar8           STEPNAME(emit_sar8)
#define emit_sar8c          STEPNAME(emit_sar8c)
#define emit_sbb16          STEPNAME(emit_sbb16)
#define emit_sbb32          STEPNAME(emit_sbb32)
#define emit_sbb8           STEPNAME(emit_sbb8)
#define emit_sbb8c          STEPNAME(emit_sbb8c)
#define emit_shl16          STEPNAME(emit_shl16)
#define emit_shl16c         STEPNAME(emit_shl16c)
#define emit_shl32          STEPNAME(emit_shl32)
#define emit_shl32c         STEPNAME(emit_shl32c)
#define emit_shl8           STEPNAME(emit_shl8)
#define emit_shl8c          STEPNAME(emit_shl8c)
#define emit_shld16         STEPNAME(emit_shld16)
#define emit_shld16c        STEPNAME(emit_shld16c)
#define emit_shld32         STEPNAME(emit_shld32)
#define emit_shld32c        STEPNAME(emit_shld32c)
#define emit_shr16          STEPNAME(emit_shr16)
#define emit_shr16c         STEPNAME(emit_shr16c)
#define emit_shr32          STEPNAME(emit_shr32)
#define emit_shr32c         STEPNAME(emit_shr32c)
#define emit_shr8           STEPNAME(emit_shr8)
#define emit_shr8c          STEPNAME(emit_shr8c)
#define emit_shrd16         STEPNAME(emit_shrd16)
#define emit_shrd16c        STEPNAME(emit_shrd16c)
#define emit_shrd32         STEPNAME(emit_shrd32)
#define emit_shrd32c        STEPNAME(emit_shrd32c)
#define emit_sub16          STEPNAME(emit_sub16)
#define emit_sub32          STEPNAME(emit_sub32)
#define emit_sub32c         STEPNAME(emit_sub32c)
#define emit_sub8           STEPNAME(emit_sub8)
#define emit_sub8c          STEPNAME(emit_sub8c)
#define emit_test16         STEPNAME(emit_test16)
#define emit_test32         STEPNAME(emit_test32)
#define emit_test32c        STEPNAME(emit_test32c)
#define emit_test8          STEPNAME(emit_test8)
#define emit_test8c         STEPNAME(emit_test8c)
#define emit_xor16          STEPNAME(emit_xor16)
#define emit_xor32          STEPNAME(emit_xor32)
#define emit_xor32c         STEPNAME(emit_xor32c)
#define emit_xor8           STEPNAME(emit_xor8)
#define emit_xor8c          STEPNAME(emit_xor8c)

#define emit_pf STEPNAME(emit_pf)

#define x87_do_push           STEPNAME(x87_do_push)
#define x87_do_push_empty     STEPNAME(x87_do_push_empty)
#define x87_do_pop            STEPNAME(x87_do_pop)
#define x87_get_current_cache STEPNAME(x87_get_current_cache)
#define x87_get_cache         STEPNAME(x87_get_cache)
#define x87_get_vmxcache      STEPNAME(x87_get_vmxcache)
#define x87_get_st            STEPNAME(x87_get_st)
#define x87_get_st_empty      STEPNAME(x87_get_st)
#define x87_free              STEPNAME(x87_free)
#define x87_refresh           STEPNAME(x87_refresh)
#define x87_forget            STEPNAME(x87_forget)
#define x87_reget_st          STEPNAME(x87_reget_st)
#define x87_stackcount        STEPNAME(x87_stackcount)
#define x87_unstackcount      STEPNAME(x87_unstackcount)
#define x87_swapreg           STEPNAME(x87_swapreg)
#define x87_setround          STEPNAME(x87_setround)
#define x87_restoreround      STEPNAME(x87_restoreround)
#define x87_reflectcount      STEPNAME(x87_reflectcount)
#define x87_unreflectcount    STEPNAME(x87_unreflectcount)
#define x87_purgecache        STEPNAME(x87_purgecache)

#define sse_setround      STEPNAME(sse_setround)
#define mmx_get_reg       STEPNAME(mmx_get_reg)
#define mmx_get_reg_empty STEPNAME(mmx_get_reg_empty)
#define sse_purge07cache  STEPNAME(sse_purge07cache)
#define sse_get_reg       STEPNAME(sse_get_reg)
#define sse_get_reg_empty STEPNAME(sse_get_reg_empty)
#define sse_forget_reg    STEPNAME(sse_forget_reg)
#define sse_reflect_reg   STEPNAME(sse_reflect_reg)

#define avx_get_reg              STEPNAME(avx_get_reg)
#define avx_get_reg_empty        STEPNAME(avx_get_reg_empty)
#define avx_forget_reg           STEPNAME(sse_forget_reg)
#define avx_reflect_reg          STEPNAME(avx_reflect_reg)
#define avx_purgecache           STEPNAME(avx_purgecache)
#define avx_reflect_reg_upper128 STEPNAME(avx_reflect_reg_upper128)


#define fpu_pushcache       STEPNAME(fpu_pushcache)
#define fpu_popcache        STEPNAME(fpu_popcache)
#define fpu_reset_cache     STEPNAME(fpu_reset_cache)
#define fpu_propagate_stack STEPNAME(fpu_propagate_stack)
#define fpu_purgecache      STEPNAME(fpu_purgecache)
#define mmx_purgecache      STEPNAME(mmx_purgecache)
#define fpu_reflectcache    STEPNAME(fpu_reflectcache)
#define fpu_unreflectcache  STEPNAME(fpu_unreflectcache)

#define CacheTransform     STEPNAME(CacheTransform)
#define ppc64le_move64     STEPNAME(ppc64le_move64)
#define ppc64le_move32     STEPNAME(ppc64le_move32)

// ========================================================================
// Function declarations
// ========================================================================

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_ppc64le_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta);

/* setup r2 to address pointed by */
uintptr_t geted16(dynarec_ppc64le_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int i12);

// generic x64 helper
void jump_to_epilog(dynarec_ppc64le_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_epilog_fast(dynarec_ppc64le_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_next(dynarec_ppc64le_t* dyn, uintptr_t ip, int reg, int ninst, int is32bits);
void ret_to_next(dynarec_ppc64le_t* dyn, uintptr_t ip, int ninst, rex_t rex);
void iret_to_next(dynarec_ppc64le_t* dyn, uintptr_t ip, int ninst, int is32bits, int is64bits);
void call_c(dynarec_ppc64le_t* dyn, int ninst, ppc64le_consts_t fnc, int reg, int ret, int saveflags, int save_reg, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
void call_n(dynarec_ppc64le_t* dyn, int ninst, void* fnc, int w);
void grab_segdata(dynarec_ppc64le_t* dyn, uintptr_t addr, int ninst, int reg, int segment);
void emit_adc16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_adc32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_adc8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_adc8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5, int s6);
void emit_add16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_add32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_add32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5);
void emit_add8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_add8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4);
void emit_and16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_and32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_and32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_and8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_and8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_cmp16_0(dynarec_ppc64le_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_cmp32_0(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, uint8_t nextop, int s1, int s3, int s4, int s5);
void emit_cmp32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_cmp8_0(dynarec_ppc64le_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_dec16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_dec32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_dec8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_inc16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_inc32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_inc8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_neg16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s3, int s4);
void emit_neg32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3);
void emit_neg8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s3, int s4);
void emit_or16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_or32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_or32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_or8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_or8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4);
void emit_rcl16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_rcl16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_rcl32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_rcl32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5);
void emit_rcl8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_rcl8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_rcr16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_rcr16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_rcr32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_rcr32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5);
void emit_rcr8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_rcr8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_rol16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_rol16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_rol32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_rol32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_rol8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_rol8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_ror16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_ror16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_ror32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_ror32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_ror8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_ror8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_sar16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sar16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_sar32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_sar32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_sar8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sar8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_sbb16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sbb32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_sbb8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sbb8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5, int s6);
void emit_shl16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shl16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_shl32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_shl32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5);
void emit_shl8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shl8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_shld16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_shld16c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4, int s5);
void emit_shld32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6);
void emit_shld32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);
void emit_shr16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shr16c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_shr32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_shr32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_shr8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shr8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_shrd16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_shrd16c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4, int s5);
void emit_shrd32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6);
void emit_shrd32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);
void emit_sub16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sub32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_sub32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5);
void emit_sub8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sub8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4, int s5);
void emit_test16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_test32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_test32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5);
void emit_test8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_test8c(dynarec_ppc64le_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4, int s5);
void emit_xor16(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_xor32(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_xor32c(dynarec_ppc64le_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_xor8(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_xor8c(dynarec_ppc64le_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);

void emit_pf(dynarec_ppc64le_t* dyn, int ninst, int s1, int s3, int s4);

// common coproc helpers

// x87 helper
// cache of the local stack counter, to avoid update at every call
int x87_stackcount(dynarec_ppc64le_t* dyn, int ninst, int scratch);
// restore local stack counter
void x87_unstackcount(dynarec_ppc64le_t* dyn, int ninst, int scratch, int count);
// fpu push. Return the Dd value to be used
int x87_do_push(dynarec_ppc64le_t* dyn, int ninst, int s1, int t);
// fpu push. Do not allocate a cache register. Needs a scratch register to do x87stack synch (or 0 to not do it)
void x87_do_push_empty(dynarec_ppc64le_t* dyn, int ninst, int s1);
// fpu pop. All previous returned Dd should be considered invalid
void x87_do_pop(dynarec_ppc64le_t* dyn, int ninst, int s1);
// get cache index for a x87 reg, return -1 if cache doesn't exist
int x87_get_current_cache(dynarec_ppc64le_t* dyn, int ninst, int st, int t);
// get cache index for a x87 reg, create the entry if needed
int x87_get_cache(dynarec_ppc64le_t* dyn, int ninst, int populate, int s1, int s2, int a, int t);
// get extcache index for a x87 reg
int x87_get_vmxcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int a);
// get vfpu register for a x87 reg, create the entry if needed
int x87_get_st(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int a, int t);
// get vfpu register for a x87 reg, create the entry if needed. Do not fetch the Stx if not already in cache
int x87_get_st_empty(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int a, int t);
// Free st, using the FFREE opcode (so it's freed but stack is not moved)
void x87_free(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int st);
// refresh a value from the cache ->emu (nothing done if value is not cached)
void x87_refresh(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int st);
// refresh a value from the cache ->emu and then forget the cache (nothing done if value is not cached)
void x87_forget(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int st);
// refresh the cache value from emu
void x87_reget_st(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int st);
// swap 2 x87 regs
void x87_swapreg(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int a, int b);
// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2);
// Restore round flag
void x87_restoreround(dynarec_ppc64le_t* dyn, int ninst, int s1);
// Set rounding according to mxcsr flags, return reg to restore flags
void x87_reflectcount(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2);
void x87_unreflectcount(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2);
void x87_purgecache(dynarec_ppc64le_t* dyn, int ninst, int next, int s1, int s2, int s3);

// reset the cache with n
void fpu_reset_cache(dynarec_ppc64le_t* dyn, int ninst, int reset_n);
void fpu_propagate_stack(dynarec_ppc64le_t* dyn, int ninst);
void fpu_purgecache(dynarec_ppc64le_t* dyn, int ninst, int next, int s1, int s2, int s3);
void mmx_purgecache(dynarec_ppc64le_t* dyn, int ninst, int next, int s1);
void fpu_reflectcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3);
void fpu_unreflectcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3);
void fpu_pushcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int not07);
void fpu_popcache(dynarec_ppc64le_t* dyn, int ninst, int s1, int not07);
// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2);

// SSE/SSE2 helpers
// purge the XMM0..XMM7 cache (before function call)
void sse_purge07cache(dynarec_ppc64le_t* dyn, int ninst, int s1);
// get vmx register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_ppc64le_t* dyn, int ninst, int s1, int a, int forwrite);
// get vmx register for an SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_ppc64le_t* dyn, int ninst, int s1, int a);
// forget float register for a SSE reg, create the entry if needed
void sse_forget_reg(dynarec_ppc64le_t* dyn, int ninst, int a);
// Push current value to the cache
void sse_reflect_reg(dynarec_ppc64le_t* dyn, int ninst, int a);

// MMX helpers
//  get vmx register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int a);
// get vmx register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_ppc64le_t* dyn, int ninst, int s1, int s2, int s3, int a);


// AVX helpers
// get vmx register for a AVX reg, create the entry if needed
int avx_get_reg(dynarec_ppc64le_t* dyn, int ninst, int s1, int a, int forwrite, int width);
// get vmx register for an AVX reg, but don't try to synch it if it needed to be created
int avx_get_reg_empty(dynarec_ppc64le_t* dyn, int ninst, int s1, int a, int width);
// forget float register for a AVX reg, create the entry if needed
void avx_forget_reg(dynarec_ppc64le_t* dyn, int ninst, int a);
// Push current value to the cache
void avx_reflect_reg(dynarec_ppc64le_t* dyn, int ninst, int a);
void avx_reflect_reg_upper128(dynarec_ppc64le_t* dyn, int ninst, int a, int forwrite);

void CacheTransform(dynarec_ppc64le_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3);

void ppc64le_move64(dynarec_ppc64le_t* dyn, int ninst, int reg, int64_t val);
void ppc64le_move32(dynarec_ppc64le_t* dyn, int ninst, int reg, int32_t val, int zeroup);

// ========================================================================
// CHECK_CACHE
// ========================================================================
#if STEP < 2
#define CHECK_CACHE() 0
#else
#define CHECK_CACHE() (cacheupd = CacheNeedsTransform(dyn, ninst))
#endif

#define vmxcache_st_coherency STEPNAME(vmxcache_st_coherency)
int vmxcache_st_coherency(dynarec_ppc64le_t* dyn, int ninst, int a, int b);

// ========================================================================
// ST_IS_F / X87_COMBINE / X87_ST (using dyn->v for VMX cache)
// ========================================================================
#if STEP == 0
#define ST_IS_F(A)        0
#define ST_IS_I64(A)      0
#define X87_COMBINE(A, B) VMX_CACHE_ST_D
#define X87_ST0           VMX_CACHE_ST_D
#define X87_ST(A)         VMX_CACHE_ST_D
#elif STEP == 1
#define ST_IS_F(A)        (vmxcache_get_current_st(dyn, ninst, A) == VMX_CACHE_ST_F)
#define ST_IS_I64(A)      (vmxcache_get_current_st(dyn, ninst, A) == VMX_CACHE_ST_I64)
#define X87_COMBINE(A, B) vmxcache_combine_st(dyn, ninst, A, B)
#define X87_ST0           vmxcache_no_i64(dyn, ninst, 0, vmxcache_get_current_st(dyn, ninst, 0))
#define X87_ST(A)         vmxcache_no_i64(dyn, ninst, A, vmxcache_get_current_st(dyn, ninst, A))
#else
#define ST_IS_F(A)   (vmxcache_get_st(dyn, ninst, A) == VMX_CACHE_ST_F)
#define ST_IS_I64(A) (vmxcache_get_st(dyn, ninst, A) == VMX_CACHE_ST_I64)
#if STEP == 3
#define X87_COMBINE(A, B) vmxcache_st_coherency(dyn, ninst, A, B)
#else
#define X87_COMBINE(A, B) vmxcache_get_st(dyn, ninst, A)
#endif
#define X87_ST0   vmxcache_get_st(dyn, ninst, 0)
#define X87_ST(A) vmxcache_get_st(dyn, ninst, A)
#endif

// ========================================================================
// Opcode decoder declarations
// ========================================================================
uintptr_t dynarec64_00(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_0F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F30F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_64(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int seg, int* ok, int* need_epilog);
uintptr_t dynarec64_66(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_6664(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int seg, int* ok, int* need_epilog);
uintptr_t dynarec64_67(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_6764(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int seg, int* ok, int* need_epilog);
uintptr_t dynarec64_660F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_66F0(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_66F20F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_66F30F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F0(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F20F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_0F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_0F38(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_66_0F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_66_0F38(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_66_0F3A(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F2_0F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F2_0F38(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F2_0F3A(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F3_0F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F3_0F38(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_D8(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_D9(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_DA(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_DB(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_DC(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_DD(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_DE(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_DF(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);


// ========================================================================
// PASS3
// ========================================================================
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

// ========================================================================
// Native flag fusion macros for PPC64LE
// ========================================================================
// PPC64LE needs separate compare + branch instructions (unlike LA64 which
// has combined compare-and-branch). So:
//   B##COND##_safe(r1, r2, imm) = 3 instructions (matches BNEZ_safe/BEZ_safe sizing)
//   B##COND##_(r1, r2, imm)     = 2 instructions (matches BxxZ_gen sizing)
//
// Signed conditions use CMPD, unsigned use CMPLD.
// Near/far branch ranges: conditional ±32KB, unconditional ±32MB.
//
// IMPORTANT: xZR == r0 == 0, but r0 is NOT a hardware zero register on PPC64LE.
// When r2 == xZR (0), we must use CMPDI/CMPLDI with immediate 0 instead of
// CMPD/CMPLD against r0 (which may hold a non-zero value).
#define CMPD_ZR(r1, r2)   do { if ((r2) == xZR) CMPDI(r1, 0); else CMPD(r1, r2); } while(0)
#define CMPLD_ZR(r1, r2)  do { if ((r2) == xZR) CMPLDI(r1, 0); else CMPLD(r1, r2); } while(0)

// --- 3-arg B##COND##_safe: always 3 instructions ---
// Pattern: CMPD/CMPLD + near(Bcond+NOP) or far(inv_Bcond+B)
// imm is offset from start of this macro to target.
// Bcond sits at +4, so near Bcond offset = imm-4.
// For far case: inv_Bcond skips over B (offset=8), B sits at +8 so B offset = imm-8.

#define BEQ_safe(r1, r2, imm)                       \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        if ((imm) > -0x7000 && (imm) < 0x7000) {    \
            BEQ((imm) - 4);                          \
            NOP();                                   \
        } else {                                     \
            BNE(2 * 4);                              \
            B((imm) - 8);                            \
        }                                            \
    } while (0)

#define BNE_safe(r1, r2, imm)                        \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        if ((imm) > -0x7000 && (imm) < 0x7000) {    \
            BNE((imm) - 4);                          \
            NOP();                                   \
        } else {                                     \
            BEQ(2 * 4);                              \
            B((imm) - 8);                            \
        }                                            \
    } while (0)

#define BLT_safe(r1, r2, imm)                        \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        if ((imm) > -0x7000 && (imm) < 0x7000) {    \
            BLT((imm) - 4);                          \
            NOP();                                   \
        } else {                                     \
            BGE(2 * 4);                              \
            B((imm) - 8);                            \
        }                                            \
    } while (0)

#define BGE_safe(r1, r2, imm)                        \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        if ((imm) > -0x7000 && (imm) < 0x7000) {    \
            BGE((imm) - 4);                          \
            NOP();                                   \
        } else {                                     \
            BLT(2 * 4);                              \
            B((imm) - 8);                            \
        }                                            \
    } while (0)

#define BGT_safe(r1, r2, imm)                        \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        if ((imm) > -0x7000 && (imm) < 0x7000) {    \
            BGT((imm) - 4);                          \
            NOP();                                   \
        } else {                                     \
            BLE(2 * 4);                              \
            B((imm) - 8);                            \
        }                                            \
    } while (0)

#define BLE_safe(r1, r2, imm)                        \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        if ((imm) > -0x7000 && (imm) < 0x7000) {    \
            BLE((imm) - 4);                          \
            NOP();                                   \
        } else {                                     \
            BGT(2 * 4);                              \
            B((imm) - 8);                            \
        }                                            \
    } while (0)

// Unsigned variants use CMPLD
#define BLTU_safe(r1, r2, imm)                       \
    do {                                             \
        CMPLD_ZR(r1, r2);                            \
        if ((imm) > -0x7000 && (imm) < 0x7000) {    \
            BLT((imm) - 4);                          \
            NOP();                                   \
        } else {                                     \
            BGE(2 * 4);                              \
            B((imm) - 8);                            \
        }                                            \
    } while (0)

#define BGEU_safe(r1, r2, imm)                       \
    do {                                             \
        CMPLD_ZR(r1, r2);                            \
        if ((imm) > -0x7000 && (imm) < 0x7000) {    \
            BGE((imm) - 4);                          \
            NOP();                                   \
        } else {                                     \
            BLT(2 * 4);                              \
            B((imm) - 8);                            \
        }                                            \
    } while (0)

#define BGTU_safe(r1, r2, imm)                       \
    do {                                             \
        CMPLD_ZR(r1, r2);                            \
        if ((imm) > -0x7000 && (imm) < 0x7000) {    \
            BGT((imm) - 4);                          \
            NOP();                                   \
        } else {                                     \
            BLE(2 * 4);                              \
            B((imm) - 8);                            \
        }                                            \
    } while (0)

#define BLEU_safe(r1, r2, imm)                       \
    do {                                             \
        CMPLD_ZR(r1, r2);                            \
        if ((imm) > -0x7000 && (imm) < 0x7000) {    \
            BLE((imm) - 4);                          \
            NOP();                                   \
        } else {                                     \
            BGT(2 * 4);                              \
            B((imm) - 8);                            \
        }                                            \
    } while (0)

// B__safe: unconditional placeholder (dead code, never reached in fusion path)
// Must be 3 instructions to match _safe sizing.
#define B__safe(a, b, imm)                           \
    do {                                             \
        NOP();                                       \
        NOP();                                       \
        NOP();                                       \
    } while (0)

// --- 3-arg B##COND##_: always 2 instructions (matches BxxZ_gen sizing) ---
// Pattern: CMPD/CMPLD + Bcond
// imm is offset from start of this macro to target. Bcond at +4, so offset = imm-4.

#define BEQ_(r1, r2, imm)                            \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        BEQ((imm) - 4);                              \
    } while (0)

#define BNE_(r1, r2, imm)                            \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        BNE((imm) - 4);                              \
    } while (0)

#define BLT_(r1, r2, imm)                            \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        BLT((imm) - 4);                              \
    } while (0)

#define BGE_(r1, r2, imm)                            \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        BGE((imm) - 4);                              \
    } while (0)

#define BGT_(r1, r2, imm)                            \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        BGT((imm) - 4);                              \
    } while (0)

#define BLE_(r1, r2, imm)                            \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        BLE((imm) - 4);                              \
    } while (0)

// Unsigned variants use CMPLD
#define BLTU_(r1, r2, imm)                           \
    do {                                             \
        CMPLD_ZR(r1, r2);                            \
        BLT((imm) - 4);                              \
    } while (0)

#define BGEU_(r1, r2, imm)                           \
    do {                                             \
        CMPLD_ZR(r1, r2);                            \
        BGE((imm) - 4);                              \
    } while (0)

#define BGTU_(r1, r2, imm)                           \
    do {                                             \
        CMPLD_ZR(r1, r2);                            \
        BGT((imm) - 4);                              \
    } while (0)

#define BLEU_(r1, r2, imm)                           \
    do {                                             \
        CMPLD_ZR(r1, r2);                            \
        BLE((imm) - 4);                              \
    } while (0)

// B__: unconditional placeholder (dead code). Must be 2 instructions.
#define B__(a, b, imm)                               \
    do {                                             \
        NOP();                                       \
        NOP();                                       \
    } while (0)

// --- 3-arg S##COND##_: NATIVESET (set rd = (op1 COND op2) ? 1 : 0) ---
// Pattern: CMPD/CMPLD + MFCR + extract CR0 bit
// CR0 bits in MFCR result (32-bit): LT=bit31, GT=bit30, EQ=bit29, SO=bit28
// RLWINM(dst, dst, shift, 31, 31) extracts one bit to position 0.
//   LT: shift=1, GT: shift=2, EQ: shift=3

// Direct conditions: extract the relevant bit
#define SLT_(dst, r1, r2)                            \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        MFCR(dst);                                   \
        RLWINM(dst, dst, 1, 31, 31);                 \
    } while (0)

#define SGT_(dst, r1, r2)                            \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        MFCR(dst);                                   \
        RLWINM(dst, dst, 2, 31, 31);                 \
    } while (0)

#define SEQ_(dst, r1, r2)                            \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        MFCR(dst);                                   \
        RLWINM(dst, dst, 3, 31, 31);                 \
    } while (0)

// Inverted conditions: extract the opposite bit and XOR with 1
#define SGE_(dst, r1, r2)                            \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        MFCR(dst);                                   \
        RLWINM(dst, dst, 1, 31, 31);                 \
        XORI(dst, dst, 1);                           \
    } while (0)

#define SLE_(dst, r1, r2)                            \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        MFCR(dst);                                   \
        RLWINM(dst, dst, 2, 31, 31);                 \
        XORI(dst, dst, 1);                           \
    } while (0)

#define SNE_(dst, r1, r2)                            \
    do {                                             \
        CMPD_ZR(r1, r2);                             \
        MFCR(dst);                                   \
        RLWINM(dst, dst, 3, 31, 31);                 \
        XORI(dst, dst, 1);                           \
    } while (0)

// Unsigned variants use CMPLD
#define SLTU_(dst, r1, r2)                           \
    do {                                             \
        CMPLD_ZR(r1, r2);                            \
        MFCR(dst);                                   \
        RLWINM(dst, dst, 1, 31, 31);                 \
    } while (0)

#define SGTU_(dst, r1, r2)                           \
    do {                                             \
        CMPLD_ZR(r1, r2);                            \
        MFCR(dst);                                   \
        RLWINM(dst, dst, 2, 31, 31);                 \
    } while (0)

#define SGEU_(dst, r1, r2)                           \
    do {                                             \
        CMPLD_ZR(r1, r2);                            \
        MFCR(dst);                                   \
        RLWINM(dst, dst, 1, 31, 31);                 \
        XORI(dst, dst, 1);                           \
    } while (0)

#define SLEU_(dst, r1, r2)                           \
    do {                                             \
        CMPLD_ZR(r1, r2);                            \
        MFCR(dst);                                   \
        RLWINM(dst, dst, 2, 31, 31);                 \
        XORI(dst, dst, 1);                           \
    } while (0)

// S__: unconditional placeholder (dead code). Sets dst=1 unconditionally.
#define S__(dst, r1, r2)                             \
    do {                                             \
        LI(dst, 1);                                  \
    } while (0)

// --- 4-arg MV##COND##_: NATIVEMV (dst = (op1 COND op2) ? src : dst) ---
// Not used by PPC64LE opcode tables currently. Stub with NOPs.
#define MV__(a, b, c, d)    NOP()
#define MVGT_(a, b, c, d)   NOP()
#define MVLE_(a, b, c, d)   NOP()
#define MVLT_(a, b, c, d)   NOP()
#define MVGE_(a, b, c, d)   NOP()
#define MVGTU_(a, b, c, d)  NOP()
#define MVLEU_(a, b, c, d)  NOP()
#define MVLTU_(a, b, c, d)  NOP()
#define MVGEU_(a, b, c, d)  NOP()
#define MVEQ_(a, b, c, d)   NOP()
#define MVNE_(a, b, c, d)   NOP()

#define NATIVEJUMP_safe(COND, val) \
    B##COND##_safe(dyn->insts[ninst].nat_flags_op1, dyn->insts[ninst].nat_flags_op2, val);

#define NATIVEJUMP(COND, val) \
    B##COND##_(dyn->insts[ninst].nat_flags_op1, dyn->insts[ninst].nat_flags_op2, val);

#define NATIVESET(COND, rd) \
    S##COND##_(rd, dyn->insts[ninst].nat_flags_op1, dyn->insts[ninst].nat_flags_op2);

#define NATIVEMV(COND, rd, rs) \
    MV##COND##_(rd, rs, dyn->insts[ninst].nat_flags_op1, dyn->insts[ninst].nat_flags_op2);

// ========================================================================
// NOTEST / SKIPTEST / GOTEST
// ========================================================================
// PPC64LE: cannot store xZR, must use LI(scratch, 0) + STW
#define NOTEST(s1)                                          \
    if (BOX64ENV(dynarec_test)) {                           \
        LI(s1, 0);                                         \
        STW(s1, offsetof(x64emu_t, test.test), xEmu);      \
        STW(s1, offsetof(x64emu_t, test.clean), xEmu);     \
    }

#define SKIPTEST(s1)                                        \
    if (BOX64ENV(dynarec_test)) {                           \
        LI(s1, 0);                                         \
        STW(s1, offsetof(x64emu_t, test.clean), xEmu);     \
    }

#define GOTEST(s1, s2)                                      \
    if (BOX64ENV(dynarec_test)) {                           \
        MOV32w(s2, 1);                                     \
        STW(s2, offsetof(x64emu_t, test.test), xEmu);      \
    }

#define GETREX()                                   \
    rex.rex = 0;                                   \
    if (!rex.is32bits)                             \
        while (opcode >= 0x40 && opcode <= 0x4f) { \
            rex.rex = opcode;                      \
            opcode = F8;                           \
        }

// PPC64LE has no LBT — RESTORE_EFLAGS / SPILL_EFLAGS are empty
#define RESTORE_EFLAGS(s)
#define SPILL_EFLAGS()

// ========================================================================
// GOCOND — conditional jump/set/cmov dispatch
// ========================================================================
// PPC64LE adaptation: SRLI_D → SRDI, ANDI → ANDId (which sets CR0)
// ANDId always sets CR0, so we can branch on it directly.
// But for the GO() macro pattern, we keep using BxxZ_gen / CBNZ etc.
// as the GO() caller provides the branch decision.
#define GOCOND(B, T1, T2)                                                                                    \
    case B + 0x0:                                                                                            \
        INST_NAME(T1 "O " T2);                                                                               \
        GO(ANDId(tmp1, xFlags, 1 << F_OF), EQZ, NEZ, _, _, X_OF, X64_JMP_JO);                                \
        break;                                                                                               \
    case B + 0x1:                                                                                            \
        INST_NAME(T1 "NO " T2);                                                                              \
        GO(ANDId(tmp1, xFlags, 1 << F_OF), NEZ, EQZ, _, _, X_OF, X64_JMP_JNO);                               \
        break;                                                                                               \
    case B + 0x2:                                                                                            \
        INST_NAME(T1 "C " T2);                                                                               \
        GO(ANDId(tmp1, xFlags, 1 << F_CF), EQZ, NEZ, GEU, LTU, X_CF, X64_JMP_JC);                            \
        break;                                                                                               \
    case B + 0x3:                                                                                            \
        INST_NAME(T1 "NC " T2);                                                                              \
        GO(ANDId(tmp1, xFlags, 1 << F_CF), NEZ, EQZ, LTU, GEU, X_CF, X64_JMP_JNC);                           \
        break;                                                                                               \
    case B + 0x4:                                                                                            \
        INST_NAME(T1 "Z " T2);                                                                               \
        GO(ANDId(tmp1, xFlags, 1 << F_ZF), EQZ, NEZ, NE, EQ, X_ZF, X64_JMP_JZ);                              \
        break;                                                                                               \
    case B + 0x5:                                                                                            \
        INST_NAME(T1 "NZ " T2);                                                                              \
        GO(ANDId(tmp1, xFlags, 1 << F_ZF), NEZ, EQZ, EQ, NE, X_ZF, X64_JMP_JNZ);                             \
        break;                                                                                               \
    case B + 0x6:                                                                                            \
        INST_NAME(T1 "BE " T2);                                                                              \
        GO(ANDId(tmp1, xFlags, (1 << F_CF) | (1 << F_ZF)), EQZ, NEZ, GTU, LEU, X_CF | X_ZF, X64_JMP_JBE);    \
        break;                                                                                               \
    case B + 0x7:                                                                                            \
        INST_NAME(T1 "NBE " T2);                                                                             \
        GO(ANDId(tmp1, xFlags, (1 << F_CF) | (1 << F_ZF)), NEZ, EQZ, LEU, GTU, X_CF | X_ZF, X64_JMP_JNBE);   \
        break;                                                                                               \
    case B + 0x8:                                                                                            \
        INST_NAME(T1 "S " T2);                                                                               \
        GO(ANDId(tmp1, xFlags, 1 << F_SF), EQZ, NEZ, GE, LT, X_SF, X64_JMP_JS);                              \
        break;                                                                                               \
    case B + 0x9:                                                                                            \
        INST_NAME(T1 "NS " T2);                                                                              \
        GO(ANDId(tmp1, xFlags, 1 << F_SF), NEZ, EQZ, LT, GE, X_SF, X64_JMP_JNS);                             \
        break;                                                                                               \
    case B + 0xA:                                                                                            \
        INST_NAME(T1 "P " T2);                                                                               \
        GO(ANDId(tmp1, xFlags, 1 << F_PF), EQZ, NEZ, _, _, X_PF, X64_JMP_JP);                                \
        break;                                                                                               \
    case B + 0xB:                                                                                            \
        INST_NAME(T1 "NP " T2);                                                                              \
        GO(ANDId(tmp1, xFlags, 1 << F_PF), NEZ, EQZ, _, _, X_PF, X64_JMP_JNP);                               \
        break;                                                                                               \
    case B + 0xC:                                                                                            \
        INST_NAME(T1 "L " T2);                                                                               \
        GO(SRDI(tmp1, xFlags, F_OF - F_SF);                                                                  \
            XOR(tmp1, tmp1, xFlags);                                                                         \
            ANDId(tmp1, tmp1, 1 << F_SF), EQZ, NEZ, GE, LT, X_SF | X_OF, X64_JMP_JL);                        \
        break;                                                                                               \
    case B + 0xD:                                                                                            \
        INST_NAME(T1 "GE " T2);                                                                              \
        GO(SRDI(tmp1, xFlags, F_OF - F_SF);                                                                  \
            XOR(tmp1, tmp1, xFlags);                                                                         \
            ANDId(tmp1, tmp1, 1 << F_SF), NEZ, EQZ, LT, GE, X_SF | X_OF, X64_JMP_JGE);                       \
        break;                                                                                               \
    case B + 0xE:                                                                                            \
        INST_NAME(T1 "LE " T2);                                                                              \
        GO(SRDI(tmp1, xFlags, F_OF - F_SF);                                                                  \
            XOR(tmp1, tmp1, xFlags);                                                                         \
            ANDId(tmp1, tmp1, 1 << F_SF);                                                                    \
            ANDId(tmp3, xFlags, 1 << F_ZF);                                                                  \
            OR(tmp1, tmp1, tmp3);                                                                            \
            ANDId(tmp1, tmp1, (1 << F_SF) | (1 << F_ZF)), EQZ, NEZ, GT, LE, X_SF | X_OF | X_ZF, X64_JMP_JLE); \
        break;                                                                                               \
    case B + 0xF:                                                                                            \
        INST_NAME(T1 "G " T2);                                                                               \
        GO(SRDI(tmp1, xFlags, F_OF - F_SF);                                                                  \
            XOR(tmp1, tmp1, xFlags);                                                                         \
            ANDId(tmp1, tmp1, 1 << F_SF);                                                                    \
            ANDId(tmp3, xFlags, 1 << F_ZF);                                                                  \
            OR(tmp1, tmp1, tmp3);                                                                            \
            ANDId(tmp1, tmp1, (1 << F_SF) | (1 << F_ZF)), NEZ, EQZ, LE, GT, X_SF | X_OF | X_ZF, X64_JMP_JG);  \
        break

// ========================================================================
// FCOM — set x87 Status Word C0/C2/C3 from FP comparison
// ========================================================================
// x87 SW bits: C0=bit8, C2=bit10, C3=bit14
// GT: C3=0,C2=0,C0=0 -> 0x0000
// LT: C3=0,C2=0,C0=1 -> 0x0100
// EQ: C3=1,C2=0,C0=0 -> 0x4000
// UN: C3=1,C2=1,C0=1 -> 0x4500
// Mask to clear C0,C1,C2,C3: ~(0x4700) = 0xB8FF
// Layout after FCMPU (all MOV32w values fit in signed 16-bit = 1 insn each):
//   +0: BC SO -> +5*4    (-> UN handler at +20)
//   +4: BEQ -> +6*4      (-> EQ handler at +28)
//   +8: BLT -> +7*4      (-> LT handler at +36)
//  +12: LI s1, 0         (GT)
//  +16: B +6*4            (-> end at +40)
//  +20: LI s1, 0x4500    (UN)
//  +24: B +4*4            (-> end at +40)
//  +28: LI s1, 0x4000    (EQ)
//  +32: B +2*4            (-> end at +40)
//  +36: LI s1, 0x0100    (LT)
//  +40: OR / STH          (end)
#define FCOM(w, v1, v2, s1, s2, s3)                                 \
    LHZ(s3, offsetof(x64emu_t, sw), xEmu);                          \
    ANDId(s3, s3, 0xB8FF);  /* mask off C0(8),C2(10),C3(14) */     \
    FCMPU(0, v1, v2);                                               \
    /* Build result branchlessly using ISEL */                       \
    LI(s1, 0);             /* zero for ISEL false case */            \
    /* LT → C0 (0x0100) */                                          \
    LI(s2, 0x0100);                                                  \
    ISEL(s2, s2, s1, BI(CR0, CR_LT));  /* s2 = LT ? 0x0100 : 0 */ \
    OR(s3, s3, s2);                                                  \
    /* EQ → C3 (0x4000) */                                          \
    LI(s2, 0x4000);                                                  \
    ISEL(s2, s2, s1, BI(CR0, CR_EQ));  /* s2 = EQ ? 0x4000 : 0 */ \
    OR(s3, s3, s2);                                                  \
    /* SO → C0|C2|C3 (0x4500) */                                    \
    LI(s2, 0x4500);                                                  \
    ISEL(s2, s2, s1, BI(CR0, CR_SO));  /* s2 = SO ? 0x4500 : 0 */  \
    OR(s3, s3, s2);                                                  \
    STH(s3, offsetof(x64emu_t, sw), xEmu);

#define FCOMS(v1, v2, s1, s2, s3) FCOM(S, v1, v2, s1, s2, s3)
#define FCOMD(v1, v2, s1, s2, s3) FCOM(D, v1, v2, s1, s2, s3)

// ========================================================================
// FCOMI — set EFLAGS CF/PF/ZF from FP comparison
// ========================================================================
// GT: CF=0,PF=0,ZF=0 -> 0x00
// LT: CF=1,PF=0,ZF=0 -> 0x01 (bit 0)
// EQ: CF=0,PF=0,ZF=1 -> 0x40 (bit 6)
// UN: CF=1,PF=1,ZF=1 -> 0x45 (bits 0,2,6)
// Also clear OF(bit11), AF(bit4), SF(bit7)
// Layout after FCMPU (all values fit in signed 16-bit = 1 insn each):
//   +0: BC SO -> +5*4    (-> UN handler at +20)
//   +4: BEQ -> +6*4      (-> EQ handler at +28)
//   +8: BLT -> +7*4      (-> LT handler at +36)
//  +12: LI s1, 0         (GT)
//  +16: B +6*4            (-> end at +40)
//  +20: MR s1, s2         (UN: s2 still has 0x45)
//  +24: B +4*4            (-> end at +40)
//  +28: LI s1, 0x40      (EQ)
//  +32: B +2*4            (-> end at +40)
//  +36: LI s1, 1          (LT)
//  +40: OR                (end)
#define FCOMI(w, v1, v2, s1, s2)                                    \
    IFX (X_OF | X_AF | X_SF | X_PEND) {                             \
        MOV64x(s2, ((1 << F_OF) | (1 << F_AF) | (1 << F_SF)));     \
        ANDC(xFlags, xFlags, s2);                                   \
    }                                                               \
    IFX (X_CF | X_PF | X_ZF | X_PEND) {                             \
        MOV32w(s2, 0b01000101); /* CF|PF|ZF mask = 0x45 */         \
        ANDC(xFlags, xFlags, s2);                                   \
        FCMPU(0, v1, v2);                                           \
        BC(BO_TRUE, BI(CR0, CR_SO), 5*4);  /* -> UN at +20 */       \
        BEQ(6*4);                          /* -> EQ at +28 */       \
        BLT(7*4);                          /* -> LT at +36 */      \
        /* GT: s1 = 0 */                                            \
        LI(s1, 0);                                                  \
        B(6*4);                            /* -> end at +40 */      \
        /* UN: CF|PF|ZF = 0x45 */                                   \
        MR(s1, s2);  /* s2 already has 0x45 */                      \
        B(4*4);                            /* -> end at +40 */      \
        /* EQ: ZF = 0x40 */                                         \
        LI(s1, 0b01000000);                                         \
        B(2*4);                            /* -> end at +40 */      \
        /* LT: CF = 1 */                                            \
        LI(s1, 1);                                                  \
        /* end: merge into flags */                                 \
        OR(xFlags, xFlags, s1);                                     \
    }                                                               \
    SET_DFNONE()

#define FCOMIS(v1, v2, s1, s2) FCOMI(S, v1, v2, s1, s2)
#define FCOMID(v1, v2, s1, s2) FCOMI(D, v1, v2, s1, s2)

// SNEZ: Set if Not Equal to Zero — dst = (src != 0) ? 1 : 0
// Uses SUBFIC+SUBFE+NEG sequence: carry-based detection
#define SNEZ(dst, src)                   \
    do {                                 \
        SUBFIC(dst, src, 0);             \
        SUBFE(dst, dst, dst);            \
        NEG(dst, dst);                   \
    } while (0)

// SEQZ: Set Equal Zero — dst = (src == 0) ? 1 : 0
#define SEQZ(dst, src)                   \
    do {                                 \
        CNTLZD(dst, src);               \
        SRDI(dst, dst, 6);              \
    } while (0)

// SLTI: Set Less Than Immediate — dst = (src < imm) ? 1 : 0
// PPC64LE has no direct SLTI; use CMPDI + extract LT bit from CR0
#define SLTI(dst, src, imm)              \
    do {                                 \
        CMPDI(src, imm);                 \
        MFCR(dst);                       \
        RLWINM(dst, dst, 1, 31, 31);     \
    } while (0)

// LDXxw: Indexed load, 32/64 conditional on rex.w
#define LDXxw(rd, base, idx)             \
    do {                                 \
        if (rex.w)                       \
            LDX(rd, base, idx);          \
        else                             \
            LWZX(rd, base, idx);         \
    } while (0)

// MOD_DU: unsigned 64-bit modulo (dst = dst % divisor), uses x3 as scratch
// PPC64LE has no modulo — implement as: q = dst / divisor; dst = dst - q * divisor
#define MOD_DU(dst, dst2, divisor)       \
    do {                                 \
        DIVDU(x3, dst, divisor);         \
        MULLD(x3, x3, divisor);          \
        SUB(dst, dst, x3);              \
    } while (0)

// MOD_D: signed 64-bit modulo (dst = dst % divisor), uses x3 as scratch
#define MOD_D(dst, dst2, divisor)        \
    do {                                 \
        DIVD(x3, dst, divisor);          \
        MULLD(x3, x3, divisor);          \
        SUB(dst, dst, x3);              \
    } while (0)

// MOD_WU: unsigned 32-bit modulo (dst = dst % divisor), uses x3 as scratch
#define MOD_WU(dst, dst2, divisor)       \
    do {                                 \
        DIVWU(x3, dst, divisor);         \
        MULLW(x3, x3, divisor);          \
        SUB(dst, dst, x3);              \
    } while (0)

// MOD_W: signed 32-bit modulo (dst = dst % divisor), uses x3 as scratch
#define MOD_W(dst, dst2, divisor)        \
    do {                                 \
        DIVW(x3, dst, divisor);          \
        MULLW(x3, x3, divisor);          \
        SUB(dst, dst, x3);              \
    } while (0)

#define PURGE_YMM()

#define ALIGNED_ATOMICxw ((fixedaddress && !(fixedaddress & (((1 << (2 + rex.w)) - 1)))) || BOX64ENV(dynarec_aligned_atomics))
#define ALIGNED_ATOMICH  ((fixedaddress && !(fixedaddress & 1)) || BOX64ENV(dynarec_aligned_atomics))

// ========================================================================
// LOCK_* macros for PPC64LE using LL/SC (lwarx/stwcx., ldarx/stdcx.)
// ========================================================================
// PPC64LE has byte/half/word/dword LL/SC, so byte ops can use LBARX/STBCXd directly.

// LOCK_8_OP: atomic byte operation using LBARX/STBCXd
// s1 = old byte value (output), wback = address, op uses s1 as input and puts result in s4
// Typical: LOCK_8_OP(ADD(s4, s1, gd), s1, wback, s3, s4, s5, s6)
#define LOCK_8_OP(op, s1, wback, s3, s4, s5, s6)    \
    do {                                              \
        LWSYNC();                                     \
        MARKLOCK;                                     \
        LBARX(s1, 0, wback);                          \
        op;                                           \
        STBCXd(s4, 0, wback);                         \
        BNE_MARKLOCK_CR0;                             \
        LWSYNC();                                     \
    } while (0)

// LOCK_32_IN_8BYTE: atomic 32-bit op on unaligned address within an 8-byte block
// Uses LDARX/STDCXd on the containing 8-byte-aligned address
// s1 = old 32-bit value (output), wback = original address, op computes new value in s4
#define LOCK_32_IN_8BYTE(op, s1, wback, s3, s4, s5, s6) \
    do {                                                  \
        ANDI(s3, wback, 0b100);   /* byte offset within 8B: 0 or 4 */ \
        SLDI(s3, s3, 3);          /* bit offset: 0 or 32 */           \
        RLDICR(s6, wback, 0, 60); /* align to 8 bytes: wback & ~7 */ \
        LWSYNC();                                                      \
        MARKLOCK;                                                      \
        LDARX(s5, 0, s6);        /* load-linked 8 bytes */            \
        SRD(s1, s5, s3);         /* shift old 32-bit value down */    \
        RLDICL(s1, s1, 0, 32);   /* zero upper 32 bits */            \
        op;                       /* s4 = f(s1, operand) */           \
        /* insert s4 back: clear 32-bit field, OR in new value */     \
        LI(s1, -1);              /* temp all-ones */                  \
        RLDICL(s1, s1, 0, 32);   /* mask = 0x00000000FFFFFFFF */     \
        SLD(s1, s1, s3);         /* shift mask to position */         \
        ANDC(s5, s5, s1);        /* clear old field */                \
        RLDICL(s4, s4, 0, 32);   /* zero upper bits of new value */  \
        SLD(s4, s4, s3);         /* shift new value to position */   \
        OR(s5, s5, s4);          /* insert new value */               \
        STDCXd(s5, 0, s6);       /* store-conditional */              \
        BNE_MARKLOCK_CR0;                                              \
        LWSYNC();                                                      \
        /* restore s1 = old 32-bit value */                            \
        SRD(s1, s5, s3);                                               \
        RLDICL(s1, s1, 0, 32);                                        \
    } while (0)

// LOCK_3264_CROSS_8BYTE: best-effort atomic for values crossing an 8-byte boundary
// Uses LDARX/STDCXd as a lock on the lower aligned 8B, then non-atomic load/modify/store
#define LOCK_3264_CROSS_8BYTE(op, s1, wback, s4, s5, s6) \
    do {                                                    \
        RLDICR(s6, wback, 0, 60); /* align to 8 bytes */  \
        LWSYNC();                                            \
        MARKLOCK;                                            \
        LDARX(s5, 0, s6);        /* reservation lock */    \
        if (rex.w) { LD(s1, 0, wback); } else { LWZ(s1, 0, wback); } \
        op;                       /* s4 = f(s1, operand) */ \
        if (rex.w) { STD(s4, 0, wback); } else { STW(s4, 0, wback); } \
        STDCXd(s5, 0, s6);       /* release reservation */ \
        BNE_MARKLOCK_CR0;                                    \
        LWSYNC();                                            \
    } while (0)

// ========================================================================
// REVBxw — byte-reverse (BSWAP) for 32 or 64 bits
// ========================================================================
// Uses stack red zone + scratch register for store-load byte-reverse
// Note: tmp is a scratch GPR
#define REVBxw(Rd, Rs, tmp)                              \
    do {                                                  \
        if (rex.w) {                                      \
            STD(Rs, -8, xSP);                             \
            LI(tmp, -8);                                  \
            LDBRX(Rd, xSP, tmp);                          \
        } else {                                          \
            STW(Rs, -4, xSP);                             \
            LI(tmp, -4);                                  \
            LWBRX(Rd, xSP, tmp);                          \
            ZEROUP(Rd);                                   \
        }                                                 \
    } while (0)

#ifndef SCRATCH_USAGE
#define SCRATCH_USAGE(usage)
#endif

#endif //__DYNAREC_PPC64LE_HELPER_H__
