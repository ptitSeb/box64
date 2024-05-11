#ifndef __DYNAREC_LA64_HELPER_H__
#define __DYNAREC_LA64_HELPER_H__

// undef to get Close to SSE Float->int conversions
// #define PRECISE_CVT

#if STEP == 0
#include "dynarec_la64_pass0.h"
#elif STEP == 1
#include "dynarec_la64_pass1.h"
#elif STEP == 2
#include "dynarec_la64_pass2.h"
#elif STEP == 3
#include "dynarec_la64_pass3.h"
#endif

#include "debug.h"
#include "la64_emitter.h"
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
#define GETGD gd = TO_LA64(((nextop & 0x38) >> 3) + (rex.r << 3));

// GETGW extract x64 register in gd, that is i
#define GETGW(i)                                         \
    gd = TO_LA64(((nextop & 0x38) >> 3) + (rex.r << 3)); \
    BSTRPICK_D(i, gd, 15, 0);                            \
    gd = i;

// GETED can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETED(D)                                                                                \
    if (MODREG) {                                                                               \
        ed = TO_LA64((nextop & 7) + (rex.b << 3));                                              \
        wback = 0;                                                                              \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        LDxw(x1, wback, fixedaddress);                                                          \
        ed = x1;                                                                                \
    }

#define GETEDz(D)                                                                               \
    if (MODREG) {                                                                               \
        ed = TO_LA64((nextop & 7) + (rex.b << 3));                                              \
        wback = 0;                                                                              \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        LDz(x1, wback, fixedaddress);                                                           \
        ed = x1;                                                                                \
    }
// GETEDH can use hint for ed, and x1 or x2 for wback (depending on hint), might also use x3. wback is 0 if ed is xEAX..xEDI
#define GETEDH(hint, D)                                                                                                                 \
    if (MODREG) {                                                                                                                       \
        ed = TO_LA64((nextop & 7) + (rex.b << 3));                                                                                      \
        wback = 0;                                                                                                                      \
    } else {                                                                                                                            \
        SMREAD();                                                                                                                       \
        addr = geted(dyn, addr, ninst, nextop, &wback, (hint == x2) ? x1 : x2, (hint == x1) ? x1 : x3, &fixedaddress, rex, NULL, 1, D); \
        LDxw(hint, wback, fixedaddress);                                                                                                \
        ed = hint;                                                                                                                      \
    }
// GETEWW will use i for ed, and can use w for wback.
#define GETEWW(w, i, D)                                                                       \
    if (MODREG) {                                                                             \
        wback = TO_LA64((nextop & 7) + (rex.b << 3));                                         \
        BSTRPICK_D(i, wback, 15, 0);                                                          \
        ed = i;                                                                               \
        wb1 = 0;                                                                              \
    } else {                                                                                  \
        SMREAD();                                                                             \
        addr = geted(dyn, addr, ninst, nextop, &wback, w, i, &fixedaddress, rex, NULL, 1, D); \
        LD_HU(i, wback, fixedaddress);                                                        \
        ed = i;                                                                               \
        wb1 = 1;                                                                              \
    }
// GETEW will use i for ed, and can use r3 for wback.
#define GETEW(i, D) GETEWW(x3, i, D)

// GETEDO can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETEDO(O, D)                                                                            \
    if (MODREG) {                                                                               \
        ed = TO_LA64((nextop & 7) + (rex.b << 3));                                              \
        wback = 0;                                                                              \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, D); \
        LDXxw(x1, wback, O);                                                                    \
        ed = x1;                                                                                \
    }

// GETSED can use r1 for ed, and r2 for wback. ed will be sign extended!
#define GETSED(D)                                                                               \
    if (MODREG) {                                                                               \
        ed = TO_LA64((nextop & 7) + (rex.b << 3));                                              \
        wback = 0;                                                                              \
        if (!rex.w) {                                                                           \
            ADD_W(x1, ed, xZR);                                                                 \
            ed = x1;                                                                            \
        }                                                                                       \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        if (rex.w)                                                                              \
            LD_D(x1, wback, fixedaddress);                                                      \
        else                                                                                    \
            LD_W(x1, wback, fixedaddress);                                                      \
        ed = x1;                                                                                \
    }

// FAKEED like GETED, but doesn't get anything
#define FAKEED                                   \
    if (!MODREG) {                               \
        addr = fakeed(dyn, addr, ninst, nextop); \
    }

// GETGW extract x64 register in gd, that is i, Signed extented
#define GETSGW(i)                                        \
    gd = TO_LA64(((nextop & 0x38) >> 3) + (rex.r << 3)); \
    EXT_W_H(i, gd);                                      \
    gd = i;

// Write back ed in wback (if wback not 0)
#define WBACK                              \
    if (wback) {                           \
        if (rex.w)                         \
            ST_D(ed, wback, fixedaddress); \
        else                               \
            ST_W(ed, wback, fixedaddress); \
        SMWRITE();                         \
    }
// GETSEW will use i for ed, and can use r3 for wback. This is the Signed version
#define GETSEW(i, D)                                                                           \
    if (MODREG) {                                                                              \
        wback = TO_LA64((nextop & 7) + (rex.b << 3));                                          \
        EXT_W_H(i, wback);                                                                     \
        ed = i;                                                                                \
        wb1 = 0;                                                                               \
    } else {                                                                                   \
        SMREAD();                                                                              \
        addr = geted(dyn, addr, ninst, nextop, &wback, x3, i, &fixedaddress, rex, NULL, 1, D); \
        LD_H(i, wback, fixedaddress);                                                          \
        ed = i;                                                                                \
        wb1 = 1;                                                                               \
    }
// Write w back to original register / memory (w needs to be 16bits only!)
#define EWBACKW(w)                    \
    if (wb1) {                        \
        ST_H(w, wback, fixedaddress); \
        SMWRITE();                    \
    } else {                          \
        BSTRINS_D(wback, w, 15, 0);   \
    }
// Write ed back to original register / memory
#define EWBACK EWBACKW(ed)

// Write back gd in correct register
#define GWBACK BSTRINS_D((TO_LA64(((nextop & 0x38) >> 3) + (rex.r << 3))), gd, 15, 0);

// GETEB will use i for ed, and can use r3 for wback.
#define GETEB(i, D)                                                                             \
    if (MODREG) {                                                                               \
        if (rex.rex) {                                                                          \
            wback = TO_LA64((nextop & 7) + (rex.b << 3));                                       \
            wb2 = 0;                                                                            \
        } else {                                                                                \
            wback = (nextop & 7);                                                               \
            wb2 = (wback >> 2) * 8;                                                             \
            wback = TO_LA64((wback & 3));                                                       \
        }                                                                                       \
        BSTRPICK_D(i, wback, wb2 + 7, wb2);                                                     \
        wb1 = 0;                                                                                \
        ed = i;                                                                                 \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 1, D); \
        LD_BU(i, wback, fixedaddress);                                                          \
        wb1 = 1;                                                                                \
        ed = i;                                                                                 \
    }
// GETSEB sign extend EB, will use i for ed, and can use r3 for wback.
#define GETSEB(i, D)                                                                            \
    if (MODREG) {                                                                               \
        if (rex.rex) {                                                                          \
            wback = TO_LA64((nextop & 7) + (rex.b << 3));                                       \
            wb2 = 0;                                                                            \
        } else {                                                                                \
            wback = (nextop & 7);                                                               \
            wb2 = (wback >> 2) * 8;                                                             \
            wback = TO_LA64(wback & 3);                                                         \
        }                                                                                       \
        if (wb2) {                                                                              \
            SRLI_D(i, wback, wb2);                                                              \
            EXT_W_B(i, i);                                                                      \
        } else {                                                                                \
            EXT_W_B(i, wback);                                                                  \
        }                                                                                       \
        wb1 = 0;                                                                                \
        ed = i;                                                                                 \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, D); \
        LD_B(i, wback, fixedaddress);                                                           \
        wb1 = 1;                                                                                \
        ed = i;                                                                                 \
    }
// GETGB will use i for gd
#define GETGB(i)                                              \
    if (rex.rex) {                                            \
        gb1 = TO_LA64(((nextop & 0x38) >> 3) + (rex.r << 3)); \
        gb2 = 0;                                              \
    } else {                                                  \
        gd = (nextop & 0x38) >> 3;                            \
        gb2 = ((gd & 4) >> 2);                                \
        gb1 = TO_LA64((gd & 3));                              \
    }                                                         \
    gd = i;                                                   \
    BSTRPICK_D(gd, gb1, gb2 + 7, gb2);

// Get GX as a quad (might use x1)
#define GETGX(a, w)                             \
    gd = ((nextop & 0x38) >> 3) + (rex.r << 3); \
    a = sse_get_reg(dyn, ninst, x1, gd, w)


#define GETGX_empty(a)                          \
    gd = ((nextop & 0x38) >> 3) + (rex.r << 3); \
    a = sse_get_reg_empty(dyn, ninst, x1, gd)

// Get EX as a quad, (x1 is used)
#define GETEX(a, w, D)                                                                       \
    if (MODREG) {                                                                            \
        a = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), w);                     \
    } else {                                                                                 \
        SMREAD();                                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 1, D); \
        a = fpu_get_scratch(dyn);                                                            \
        VLD(a, ed, fixedaddress);                                                            \
    }

// Put Back EX if it was a memory and not an emm register
#define PUTEX(a)                  \
    if (!MODREG) {                \
        VST(a, ed, fixedaddress); \
        SMWRITE2();               \
    }

// Get Ex as a double, not a quad (warning, x1 get used, x2 might too)
#define GETEXSD(a, w, D)                                                                     \
    if (MODREG) {                                                                            \
        a = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), w);                     \
    } else {                                                                                 \
        SMREAD(); /* TODO */                                                                 \
        a = fpu_get_scratch(dyn);                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, D); \
        FLD_D(a, ed, fixedaddress);                                                          \
    }

// Get Ex as a single, not a quad (warning, x1 get used)
#define GETEXSS(a, w, D)                                                                     \
    if (MODREG) {                                                                            \
        a = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), w);                     \
    } else {                                                                                 \
        SMREAD();                                                                            \
        a = fpu_get_scratch(dyn);                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, D); \
        FLD_S(a, ed, fixedaddress);                                                          \
    }

// Write gb (gd) back to original register / memory, using s1 as scratch
#define GBBACK() BSTRINS_D(gb1, gd, gb2 + 7, gb2);

// Generic get GD, but reg value in gd (R_RAX is not added)
#define GETG gd = ((nextop & 0x38) >> 3) + (rex.r << 3)

// Write eb (ed) back to original register / memory, using s1 as scratch
#define EBBACK()                            \
    if (wb1) {                              \
        ST_B(ed, wback, fixedaddress);      \
        SMWRITE();                          \
    } else {                                \
        BSTRINS_D(wback, ed, wb2 + 7, wb2); \
    }

// Get direction with size Z and based of F_DF flag, on register r ready for load/store fetching
// using s as scratch.
// F_DF is not in LBT4.eflags, don't worry
#define GETDIR(r, s, Z)            \
    MOV32w(r, Z); /* mask=1<<10 */ \
    ANDI(s, xFlags, 1 << F_DF);    \
    BEQZ(s, 4 + 4);                \
    SUB_D(r, xZR, r);

// CALL will use x6 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL(F, ret) call_c(dyn, ninst, F, x6, ret, 1, 0)
// CALL_ will use x6 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL_(F, ret, reg) call_c(dyn, ninst, F, x6, ret, 1, reg)
// CALL_S will use x6 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2. Flags are not save/restored
#define CALL_S(F, ret) call_c(dyn, ninst, F, x6, ret, 0, 0)

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

#define Bxx_gen(OP, M, reg1, reg2)   \
    j64 = GET##M - dyn->native_size; \
    B##OP(reg1, reg2, j64)

#define BxxZ_gen(OP, M, reg)         \
    j64 = GET##M - dyn->native_size; \
    B##OP##Z(reg, j64)

#define BCxxZ_gen(OP, M, fcc)        \
    j64 = GET##M - dyn->native_size; \
    BC##OP##Z(fcc, j64)

// Branch to MARK if reg1!=reg2 (use j64)
#define BNE_MARK(reg1, reg2) Bxx_gen(NE, MARK, reg1, reg2)
// Branch to MARK2 if reg1!=reg2 (use j64)
#define BNE_MARK2(reg1, reg2) Bxx_gen(NE, MARK2, reg1, reg2)
// Branch to MARK3 if reg1!=reg2 (use j64)
#define BNE_MARK3(reg1, reg2) Bxx_gen(NE, MARK3, reg1, reg2)
// Branch to MARKLOCK if reg1!=reg2 (use j64)
#define BNE_MARKLOCK(reg1, reg2) Bxx_gen(NE, MARKLOCK, reg1, reg2)

// Branch to MARK if reg1==reg2 (use j64)
#define BEQ_MARK(reg1, reg2) Bxx_gen(EQ, MARK, reg1, reg2)
// Branch to MARK2 if reg1==reg2 (use j64)
#define BEQ_MARK2(reg1, reg2) Bxx_gen(EQ, MARK2, reg1, reg2)
// Branch to MARK3 if reg1==reg2 (use j64)
#define BEQ_MARK3(reg1, reg2) Bxx_gen(EQ, MARK3, reg1, reg2)
// Branch to MARKLOCK if reg1==reg2 (use j64)
#define BEQ_MARKLOCK(reg1, reg2) Bxx_gen(EQ, MARKLOCK, reg1, reg2)
// Branch to MARKLOCK if reg1==0 (use j64)
#define BEQZ_MARKLOCK(reg) BxxZ_gen(EQ, MARKLOCK, reg)

// Branch to MARK if reg1!=0 (use j64)
#define BNEZ_MARK(reg) BxxZ_gen(NE, MARK, reg)
// Branch to MARK2 if reg1!=0 (use j64)
#define BNEZ_MARK2(reg) BxxZ_gen(NE, MARK2, reg)
// Branch to MARK3 if reg1!=0 (use j64)
#define BNEZ_MARK3(reg) BxxZ_gen(NE, MARK3, reg)
// Branch to MARKLOCK if reg1!=0 (use j64)
#define BNEZ_MARKLOCK(reg) BxxZ_gen(NE, MARKLOCK, reg)

// Branch to MARK if fcc!=0 (use j64)
#define BCNEZ_MARK(fcc) BCxxZ_gen(NE, MARK, fcc)
// Branch to MARK2 if fcc!=0 (use j64)
#define BCNEZ_MARK2(fcc) BCxxZ_gen(NE, MARK2, fcc)
// Branch to MARK3 if fcc!=0 (use j64)
#define BCNEZ_MARK3(fcc) BCxxZ_gen(NE, MARK3, fcc)
// Branch to MARKLOCK if fcc!=0 (use j64)
#define BCNEZ_MARKLOCK(fcc) BxxZ_gen(NE, MARKLOCK, fcc)

// Branch to MARK if fcc==0 (use j64)
#define BCEQZ_MARK(fcc) BCxxZ_gen(EQ, MARK, fcc)
// Branch to MARK2 if fcc==0 (use j64)
#define BCEQZ_MARK2(fcc) BCxxZ_gen(EQ, MARK2, fcc)
// Branch to MARK3 if fcc==0 (use j64)
#define BCEQZ_MARK3(fcc) BCxxZ_gen(EQ, MARK3, fcc)
// Branch to MARKLOCK if fcc==0 (use j64)
#define BCEQZ_MARKLOCK(fcc) BxxZ_gen(EQ, MARKLOCK, fcc)

// Branch to MARK if reg1<reg2 (use j64)
#define BLT_MARK(reg1, reg2) Bxx_gen(LT, MARK, reg1, reg2)
// Branch to MARK if reg1<reg2 (use j64)
#define BLTU_MARK(reg1, reg2) Bxx_gen(LTU, MARK, reg1, reg2)
// Branch to MARK if reg1>=reg2 (use j64)
#define BGE_MARK(reg1, reg2) Bxx_gen(GE, MARK, reg1, reg2)

// Branch to MARK1 instruction unconditionnal (use j64)
#define B_MARK1_nocond Bxx_gen(__, MARK1, 0, 0)
// Branch to MARK2 instruction unconditionnal (use j64)
#define B_MARK2_nocond Bxx_gen(__, MARK2, 0, 0)
// Branch to MARK3 instruction unconditionnal (use j64)
#define B_MARK3_nocond Bxx_gen(__, MARK3, 0, 0)

// Branch to NEXT if reg1==0 (use j64)
#define CBZ_NEXT(reg1)                                                        \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    BEQZ(reg1, j64)
// Branch to NEXT if reg1!=0 (use j64)
#define CBNZ_NEXT(reg1)                                                       \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    BNEZ(reg1, j64)
#define B_NEXT_nocond                                                         \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    B(j64)
// Branch to NEXT if fcc==0 (use j64)
#define CBCZ_NEXT(fcc)                                                        \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    BCEQZ(fcc, j64)
// Branch to NEXT if fcc!=0 (use j64)
#define CBCNZ_NEXT(fcc)                                                       \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    BCNEZ(fcc, j64)

// Branch to NEXT if reg1==reg2 (use j64)
#define BEQ_NEXT(reg1, reg2)                                                  \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    BEQ(reg1, reg2, j64)

// Branch to NEXT if reg1!=reg2 (use j64)
#define BNE_NEXT(reg1, reg2)                                                  \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    BNE(reg1, reg2, j64)

// Branch to MARKSEG if reg is 0 (use j64)
#define CBZ_MARKSEG(reg)                   \
    j64 = GETMARKSEG - (dyn->native_size); \
    BEQZ(reg, j64);
// Branch to MARKSEG if reg is not 0 (use j64)
#define CBNZ_MARKSEG(reg)                  \
    j64 = GETMARKSEG - (dyn->native_size); \
    BNEZ(reg, j64);

#define IFX(A)      if ((dyn->insts[ninst].x64.gen_flags & (A)))
#define IFXA(A, B)  if ((dyn->insts[ninst].x64.gen_flags & (A)) && (B))
#define IFX_PENDOR0 if ((dyn->insts[ninst].x64.gen_flags & (X_PEND) || !dyn->insts[ninst].x64.gen_flags))
#define IFXX(A)     if ((dyn->insts[ninst].x64.gen_flags == (A)))
#define IFX2X(A, B) if ((dyn->insts[ninst].x64.gen_flags == (A) || dyn->insts[ninst].x64.gen_flags == (B) || dyn->insts[ninst].x64.gen_flags == ((A) | (B))))
#define IFXN(A, B)  if ((dyn->insts[ninst].x64.gen_flags & (A) && !(dyn->insts[ninst].x64.gen_flags & (B))))

#define STORE_REG(A) ST_D(x##A, xEmu, offsetof(x64emu_t, regs[_##A]))
#define LOAD_REG(A)  LD_D(x##A, xEmu, offsetof(x64emu_t, regs[_##A]))

// Need to also store current value of some register, as they may be used by functions like setjmp
#define STORE_XEMU_CALL() \
    STORE_REG(R8);        \
    STORE_REG(R9);        \
    STORE_REG(R10);       \
    STORE_REG(R11);       \
    STORE_REG(R12);       \
    STORE_REG(R13);       \
    STORE_REG(R14);       \
    STORE_REG(R15);

#define LOAD_XEMU_CALL()

#define LOAD_XEMU_REM() \
    LOAD_REG(R8);       \
    LOAD_REG(R9);       \
    LOAD_REG(R10);      \
    LOAD_REG(R11);      \
    LOAD_REG(R12);      \
    LOAD_REG(R13);      \
    LOAD_REG(R14);      \
    LOAD_REG(R15);

#define SET_DFNONE()                             \
    if (!dyn->f.dfnone) {                        \
        ST_W(xZR, xEmu, offsetof(x64emu_t, df)); \
        dyn->f.dfnone = 1;                       \
    }
#define SET_DF(S, N)                           \
    if ((N) != d_none) {                       \
        MOV32w(S, (N));                        \
        ST_W(S, xEmu, offsetof(x64emu_t, df)); \
        dyn->f.dfnone = 0;                     \
    } else                                     \
        SET_DFNONE()
#define SET_NODF() dyn->f.dfnone = 0
#define SET_DFOK() dyn->f.dfnone = 1

#define CLEAR_FLAGS_(s) \
    MOV64x(s, (1UL << F_AF) | (1UL << F_CF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF)); ANDN(xFlags, xFlags, s);

#define CLEAR_FLAGS(s) \
    IFX(X_ALL) { CLEAR_FLAGS_(s) }

#define CALC_SUB_FLAGS(op1_, op2, res, scratch1, scratch2, width)     \
    IFX(X_AF | X_CF | X_OF)                                           \
    {                                                                 \
        /* calc borrow chain */                                       \
        /* bc = (res & (~op1 | op2)) | (~op1 & op2) */                \
        OR(scratch1, op1_, op2);                                      \
        AND(scratch2, res, scratch1);                                 \
        AND(op1_, op1_, op2);                                         \
        OR(scratch2, scratch2, op1_);                                 \
        IFX(X_AF)                                                     \
        {                                                             \
            /* af = bc & 0x8 */                                       \
            ANDI(scratch1, scratch2, 8);                              \
            BEQZ(scratch1, 8);                                        \
            ORI(xFlags, xFlags, 1 << F_AF);                           \
        }                                                             \
        IFX(X_CF)                                                     \
        {                                                             \
            /* cf = bc & (1<<(width-1)) */                            \
            if ((width) == 8) {                                       \
                ANDI(scratch1, scratch2, 0x80);                       \
            } else {                                                  \
                SRLI_D(scratch1, scratch2, (width)-1);                \
                if (width != 64) ANDI(scratch1, scratch1, 1);         \
            }                                                         \
            BEQZ(scratch1, 8);                                        \
            ORI(xFlags, xFlags, 1 << F_CF);                           \
        }                                                             \
        IFX(X_OF)                                                     \
        {                                                             \
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */ \
            SRLI_D(scratch1, scratch2, (width)-2);                    \
            SRLI_D(scratch2, scratch1, 1);                            \
            XOR(scratch1, scratch1, scratch2);                        \
            ANDI(scratch1, scratch1, 1);                              \
            BEQZ(scratch1, 8);                                        \
            ORI(xFlags, xFlags, 1 << F_OF);                           \
        }                                                             \
    }

#ifndef MAYSETFLAGS
#define MAYSETFLAGS()
#endif

#ifndef READFLAGS
#define READFLAGS(A)                                \
    if (((A) != X_PEND && dyn->f.pending != SF_SET) \
        && (dyn->f.pending != SF_SET_PENDING)) {    \
        if (dyn->f.pending != SF_PENDING) {         \
            LD_D(x3, xEmu, offsetof(x64emu_t, df)); \
            j64 = (GETMARKF) - (dyn->native_size);  \
            BEQ(x3, xZR, j64);                      \
        }                                           \
        CALL_(UpdateFlags, -1, 0);                  \
        MARKF;                                      \
        dyn->f.pending = SF_SET;                    \
        SET_DFOK();                                 \
    }
#endif

#ifndef SETFLAGS
#define SETFLAGS(A, B)                                                                                              \
    if (dyn->f.pending != SF_SET                                                                                    \
        && ((B) & SF_SUB)                                                                                           \
        && (dyn->insts[ninst].x64.gen_flags & (~(A))))                                                              \
        READFLAGS(((dyn->insts[ninst].x64.gen_flags & X_PEND) ? X_ALL : dyn->insts[ninst].x64.gen_flags) & (~(A))); \
    if (dyn->insts[ninst].x64.gen_flags) switch (B) {                                                               \
            case SF_SUBSET:                                                                                         \
            case SF_SET: dyn->f.pending = SF_SET; break;                                                            \
            case SF_PENDING: dyn->f.pending = SF_PENDING; break;                                                    \
            case SF_SUBSET_PENDING:                                                                                 \
            case SF_SET_PENDING:                                                                                    \
                dyn->f.pending = (dyn->insts[ninst].x64.gen_flags & X_PEND) ? SF_SET_PENDING : SF_SET;              \
                break;                                                                                              \
        }                                                                                                           \
    else                                                                                                            \
        dyn->f.pending = SF_SET
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
    if (dyn->insts[ninst].x64.gen_flags) { SET_DF(r, A) }
#define UFLAG_IF if (dyn->insts[ninst].x64.gen_flags)
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
#define GETIP(A) TABLE64(0, 0)
#define GETIP_(A) TABLE64(0, 0)
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

void la64_epilog(void);
void la64_epilog_fast(void);
void* la64_next(x64emu_t* emu, uintptr_t addr);

#ifndef STEPNAME
#define STEPNAME3(N, M) N##M
#define STEPNAME2(N, M) STEPNAME3(N, M)
#define STEPNAME(N)     STEPNAME2(N, STEP)
#endif

#define native_pass STEPNAME(native_pass)

#define dynarec64_00   STEPNAME(dynarec64_00)
#define dynarec64_0F   STEPNAME(dynarec64_0F)
#define dynarec64_64   STEPNAME(dynarec64_64)
#define dynarec64_66   STEPNAME(dynarec64_66)
#define dynarec64_F30F STEPNAME(dynarec64_F30F)
#define dynarec64_660F STEPNAME(dynarec64_660F)
#define dynarec64_F0   STEPNAME(dynarec64_F0)
#define dynarec64_F20F STEPNAME(dynarec64_F20F)

#define geted               STEPNAME(geted)
#define geted32             STEPNAME(geted32)
#define jump_to_epilog      STEPNAME(jump_to_epilog)
#define jump_to_epilog_fast STEPNAME(jump_to_epilog_fast)
#define jump_to_next        STEPNAME(jump_to_next)
#define ret_to_epilog       STEPNAME(ret_to_epilog)
#define retn_to_epilog      STEPNAME(retn_to_epilog)
#define call_c              STEPNAME(call_c)
#define grab_segdata        STEPNAME(grab_segdata)
#define emit_cmp16          STEPNAME(emit_cmp16)
#define emit_cmp16_0        STEPNAME(emit_cmp16_0)
#define emit_cmp32          STEPNAME(emit_cmp32)
#define emit_cmp32_0        STEPNAME(emit_cmp32_0)
#define emit_cmp8           STEPNAME(emit_cmp8)
#define emit_cmp8_0         STEPNAME(emit_cmp8_0)
#define emit_test8          STEPNAME(emit_test8)
#define emit_test16         STEPNAME(emit_test16)
#define emit_test32         STEPNAME(emit_test32)
#define emit_test32c        STEPNAME(emit_test32c)
#define emit_add32          STEPNAME(emit_add32)
#define emit_add32c         STEPNAME(emit_add32c)
#define emit_add8           STEPNAME(emit_add8)
#define emit_add8c          STEPNAME(emit_add8c)
#define emit_add16          STEPNAME(emit_add16)
#define emit_adc32          STEPNAME(emit_adc32)
#define emit_sub16          STEPNAME(emit_sub16)
#define emit_sub32          STEPNAME(emit_sub32)
#define emit_sub32c         STEPNAME(emit_sub32c)
#define emit_sub8           STEPNAME(emit_sub8)
#define emit_sub8c          STEPNAME(emit_sub8c)
#define emit_sbb8           STEPNAME(emit_sbb8)
#define emit_sbb8c          STEPNAME(emit_sbb8c)
#define emit_sbb16          STEPNAME(emit_sbb16)
#define emit_sbb32          STEPNAME(emit_sbb32)
#define emit_neg8           STEPNAME(emit_neg8)
#define emit_neg32          STEPNAME(emit_neg32)
#define emit_or32           STEPNAME(emit_or32)
#define emit_or32c          STEPNAME(emit_or32c)
#define emit_or8            STEPNAME(emit_or8)
#define emit_or8c           STEPNAME(emit_or8c)
#define emit_or16           STEPNAME(emit_or16)
#define emit_xor8           STEPNAME(emit_xor8)
#define emit_xor8c          STEPNAME(emit_xor8c)
#define emit_xor16          STEPNAME(emit_xor16)
#define emit_xor32          STEPNAME(emit_xor32)
#define emit_xor32c         STEPNAME(emit_xor32c)
#define emit_and8           STEPNAME(emit_and8)
#define emit_and8c          STEPNAME(emit_and8c)
#define emit_and16          STEPNAME(emit_and16)
#define emit_and32          STEPNAME(emit_and32)
#define emit_and32c         STEPNAME(emit_and32c)
#define emit_shl16          STEPNAME(emit_shl16)
#define emit_shl32          STEPNAME(emit_shl32)
#define emit_shl32c         STEPNAME(emit_shl32c)
#define emit_shr8           STEPNAME(emit_shr8)
#define emit_shr16          STEPNAME(emit_shr16)
#define emit_shr32          STEPNAME(emit_shr32)
#define emit_shr32c         STEPNAME(emit_shr32c)
#define emit_sar16          STEPNAME(emit_sar16)
#define emit_sar32c         STEPNAME(emit_sar32c)
#define emit_ror32c         STEPNAME(emit_ror32c)
#define emit_rol32          STEPNAME(emit_rol32)

#define emit_pf STEPNAME(emit_pf)

#define x87_restoreround  STEPNAME(x87_restoreround)
#define sse_setround      STEPNAME(sse_setround)
#define x87_forget       STEPNAME(x87_forget)
#define sse_purge07cache STEPNAME(sse_purge07cache)
#define sse_get_reg       STEPNAME(sse_get_reg)
#define sse_get_reg_empty STEPNAME(sse_get_reg_empty)

#define fpu_pushcache       STEPNAME(fpu_pushcache)
#define fpu_popcache        STEPNAME(fpu_popcache)
#define fpu_reset_cache     STEPNAME(fpu_reset_cache)
#define fpu_propagate_stack STEPNAME(fpu_propagate_stack)
#define fpu_purgecache      STEPNAME(fpu_purgecache)
#define fpu_reflectcache    STEPNAME(fpu_reflectcache)
#define fpu_unreflectcache  STEPNAME(fpu_unreflectcache)

#define CacheTransform STEPNAME(CacheTransform)
#define la64_move64    STEPNAME(la64_move64)
#define la64_move32    STEPNAME(la64_move32)

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_la64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta);

/* setup r2 to address pointed by */
uintptr_t geted32(dynarec_la64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta);

// generic x64 helper
void jump_to_epilog(dynarec_la64_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_epilog_fast(dynarec_la64_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_next(dynarec_la64_t* dyn, uintptr_t ip, int reg, int ninst, int is32bits);
void ret_to_epilog(dynarec_la64_t* dyn, int ninst, rex_t rex);
void retn_to_epilog(dynarec_la64_t* dyn, int ninst, rex_t rex, int n);
void call_c(dynarec_la64_t* dyn, int ninst, void* fnc, int reg, int ret, int saveflags, int save_reg);
void grab_segdata(dynarec_la64_t* dyn, uintptr_t addr, int ninst, int reg, int segment);
void emit_cmp8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_cmp16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_cmp32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_cmp8_0(dynarec_la64_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp16_0(dynarec_la64_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp32_0(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
void emit_test8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_test16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_test32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_test32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5);
void emit_add32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_add32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5);
void emit_add8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_add8c(dynarec_la64_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4);
void emit_add16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_adc32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_sub16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sub32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_sub32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5);
void emit_sub8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sub8c(dynarec_la64_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4, int s5);
void emit_sbb8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sbb8c(dynarec_la64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5, int s6);
void emit_sbb16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sbb32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_neg8(dynarec_la64_t* dyn, int ninst, int s1, int s3, int s4);
void emit_neg32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3);
void emit_or32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_or32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_or8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_or8c(dynarec_la64_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4);
void emit_or16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_xor8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_xor8c(dynarec_la64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_xor16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_xor32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_xor32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_and8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_and8c(dynarec_la64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_and16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_and32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_and32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_shl16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shl32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_shl32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5);
void emit_shr8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shr16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shr32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_shr32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_sar16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sar32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_ror32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_rol32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);

void emit_pf(dynarec_la64_t* dyn, int ninst, int s1, int s3, int s4);

// common coproc helpers
// reset the cache with n
void fpu_reset_cache(dynarec_la64_t* dyn, int ninst, int reset_n);
// propagate stack state
void fpu_propagate_stack(dynarec_la64_t* dyn, int ninst);
// purge the FPU cache (needs 3 scratch registers)
void fpu_purgecache(dynarec_la64_t* dyn, int ninst, int next, int s1, int s2, int s3);
void fpu_reflectcache(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3);
void fpu_unreflectcache(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3);
void fpu_pushcache(dynarec_la64_t* dyn, int ninst, int s1, int not07);
void fpu_popcache(dynarec_la64_t* dyn, int ninst, int s1, int not07);
// Restore round flag
void x87_restoreround(dynarec_la64_t* dyn, int ninst, int s1);
// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_la64_t* dyn, int ninst, int s1, int s2);
// refresh a value from the cache ->emu and then forget the cache (nothing done if value is not cached)
void x87_forget(dynarec_la64_t* dyn, int ninst, int s1, int s2, int st);

// SSE/SSE2 helpers
// purge the XMM0..XMM7 cache (before function call)
void sse_purge07cache(dynarec_la64_t* dyn, int ninst, int s1);
// get lsx register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_la64_t* dyn, int ninst, int s1, int a, int forwrite);
// get lsx register for an SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_la64_t* dyn, int ninst, int s1, int a);

void CacheTransform(dynarec_la64_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3);

void la64_move64(dynarec_la64_t* dyn, int ninst, int reg, int64_t val);
void la64_move32(dynarec_la64_t* dyn, int ninst, int reg, int32_t val, int zeroup);

#if STEP < 2
#define CHECK_CACHE() 0
#else
#define CHECK_CACHE() (cacheupd = CacheNeedsTransform(dyn, ninst))
#endif

uintptr_t dynarec64_00(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F30F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_64(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog);
uintptr_t dynarec64_66(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_660F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F0(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_F20F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);

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

#define GOCOND(B, T1, T2)                                                                        \
    case B + 0x0:                                                                                \
        INST_NAME(T1 "O " T2);                                                                   \
        GO(ANDI(x1, xFlags, 1 << F_OF), EQZ, NEZ, X_OF, X64_JMP_JO);                             \
        break;                                                                                   \
    case B + 0x1:                                                                                \
        INST_NAME(T1 "NO " T2);                                                                  \
        GO(ANDI(x1, xFlags, 1 << F_OF), NEZ, EQZ, X_OF, X64_JMP_JNO);                            \
        break;                                                                                   \
    case B + 0x2:                                                                                \
        INST_NAME(T1 "C " T2);                                                                   \
        GO(ANDI(x1, xFlags, 1 << F_CF), EQZ, NEZ, X_CF, X64_JMP_JC);                             \
        break;                                                                                   \
    case B + 0x3:                                                                                \
        INST_NAME(T1 "NC " T2);                                                                  \
        GO(ANDI(x1, xFlags, 1 << F_CF), NEZ, EQZ, X_CF, X64_JMP_JNC);                            \
        break;                                                                                   \
    case B + 0x4:                                                                                \
        INST_NAME(T1 "Z " T2);                                                                   \
        GO(ANDI(x1, xFlags, 1 << F_ZF), EQZ, NEZ, X_ZF, X64_JMP_JZ);                             \
        break;                                                                                   \
    case B + 0x5:                                                                                \
        INST_NAME(T1 "NZ " T2);                                                                  \
        GO(ANDI(x1, xFlags, 1 << F_ZF), NEZ, EQZ, X_ZF, X64_JMP_JNZ);                            \
        break;                                                                                   \
    case B + 0x6:                                                                                \
        INST_NAME(T1 "BE " T2);                                                                  \
        GO(ANDI(x1, xFlags, (1 << F_CF) | (1 << F_ZF)), EQZ, NEZ, X_CF | X_ZF, X64_JMP_JBE);     \
        break;                                                                                   \
    case B + 0x7:                                                                                \
        INST_NAME(T1 "NBE " T2);                                                                 \
        GO(ANDI(x1, xFlags, (1 << F_CF) | (1 << F_ZF)), NEZ, EQZ, X_CF | X_ZF, X64_JMP_JNBE);    \
        break;                                                                                   \
    case B + 0x8:                                                                                \
        INST_NAME(T1 "S " T2);                                                                   \
        GO(ANDI(x1, xFlags, 1 << F_SF), EQZ, NEZ, X_SF, X64_JMP_JS);                             \
        break;                                                                                   \
    case B + 0x9:                                                                                \
        INST_NAME(T1 "NS " T2);                                                                  \
        GO(ANDI(x1, xFlags, 1 << F_SF), NEZ, EQZ, X_SF, X64_JMP_JNS);                            \
        break;                                                                                   \
    case B + 0xA:                                                                                \
        INST_NAME(T1 "P " T2);                                                                   \
        GO(ANDI(x1, xFlags, 1 << F_PF), EQZ, NEZ, X_PF, X64_JMP_JP);                             \
        break;                                                                                   \
    case B + 0xB:                                                                                \
        INST_NAME(T1 "NP " T2);                                                                  \
        GO(ANDI(x1, xFlags, 1 << F_PF), NEZ, EQZ, X_PF, X64_JMP_JNP);                            \
        break;                                                                                   \
    case B + 0xC:                                                                                \
        INST_NAME(T1 "L " T2);                                                                   \
        GO(SRLI_D(x1, xFlags, F_OF - F_SF);                                                      \
            XOR(x1, x1, xFlags);                                                                 \
            ANDI(x1, x1, 1 << F_SF), EQZ, NEZ, X_SF | X_OF, X64_JMP_JL);                         \
        break;                                                                                   \
    case B + 0xD:                                                                                \
        INST_NAME(T1 "GE " T2);                                                                  \
        GO(SRLI_D(x1, xFlags, F_OF - F_SF);                                                      \
            XOR(x1, x1, xFlags);                                                                 \
            ANDI(x1, x1, 1 << F_SF), NEZ, EQZ, X_SF | X_OF, X64_JMP_JGE);                        \
        break;                                                                                   \
    case B + 0xE:                                                                                \
        INST_NAME(T1 "LE " T2);                                                                  \
        GO(SRLI_D(x1, xFlags, F_OF - F_SF);                                                      \
            XOR(x1, x1, xFlags);                                                                 \
            ANDI(x1, x1, 1 << F_SF);                                                             \
            ANDI(x3, xFlags, 1 << F_ZF);                                                         \
            OR(x1, x1, x3);                                                                      \
            ANDI(x1, x1, (1 << F_SF) | (1 << F_ZF)), EQZ, NEZ, X_SF | X_OF | X_ZF, X64_JMP_JLE); \
        break;                                                                                   \
    case B + 0xF:                                                                                \
        INST_NAME(T1 "G " T2);                                                                   \
        GO(SRLI_D(x1, xFlags, F_OF - F_SF);                                                      \
            XOR(x1, x1, xFlags);                                                                 \
            ANDI(x1, x1, 1 << F_SF);                                                             \
            ANDI(x3, xFlags, 1 << F_ZF);                                                         \
            OR(x1, x1, x3);                                                                      \
            ANDI(x1, x1, (1 << F_SF) | (1 << F_ZF)), NEZ, EQZ, X_SF | X_OF | X_ZF, X64_JMP_JG);  \
        break

#define NOTEST(s1)                                       \
    if (box64_dynarec_test) {                            \
        ST_W(xZR, xEmu, offsetof(x64emu_t, test.test));  \
        ST_W(xZR, xEmu, offsetof(x64emu_t, test.clean)); \
    }

#define SKIPTEST(s1)                                     \
    if (box64_dynarec_test) {                            \
        ST_W(xZR, xEmu, offsetof(x64emu_t, test.clean)); \
    }

#define GOTEST(s1, s2)                                 \
    if (box64_dynarec_test) {                          \
        MOV32w(s2, 1);                                 \
        ST_W(s2, xEmu, offsetof(x64emu_t, test.test)); \
    }

#define GETREX()                                   \
    rex.rex = 0;                                   \
    if (!rex.is32bits)                             \
        while (opcode >= 0x40 && opcode <= 0x4f) { \
            rex.rex = opcode;                      \
            opcode = F8;                           \
        }

// Restore xFlags from LBT.eflags
#define RESTORE_EFLAGS(s)               \
    do {                                \
        if (la64_lbt) {                 \
            CLEAR_FLAGS_(s);            \
            X64_GET_EFLAGS(s, X_ALL);   \
            OR(xFlags, xFlags, s);      \
        }                               \
    } while (0)

// Spill xFlags to LBT.eflags
#define SPILL_EFLAGS()                     \
    do {                                   \
        if (la64_lbt) {                    \
            X64_SET_EFLAGS(xFlags, X_ALL); \
        }                                  \
    } while (0)

#endif //__DYNAREC_LA64_HELPER_H__