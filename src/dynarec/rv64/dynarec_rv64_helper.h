#ifndef __DYNAREC_RV64_HELPER_H__
#define __DYNAREC_RV64_HELPER_H__

// undef to get Close to SSE Float->int conversions
//#define PRECISE_CVT

#if STEP == 0
#include "dynarec_rv64_pass0.h"
#elif STEP == 1
#include "dynarec_rv64_pass1.h"
#elif STEP == 2
#include "dynarec_rv64_pass2.h"
#elif STEP == 3
#include "dynarec_rv64_pass3.h"
#endif

#include "debug.h"
#include "rv64_emitter.h"
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
#define SMREAD() if((dyn->smread==0) && (box64_dynarec_strongmem>SMREAD_MIN)) {SMDMB();} else dyn->smread=1
// Opcode will read with option forced lock
#define SMREADLOCK(lock)    if((lock) || ((dyn->smread==0) && (box64_dynarec_strongmem>SMREAD_MIN))) {SMDMB();}
// Opcode might read (depend on nextop)
#define SMMIGHTREAD()   if(!MODREG) {SMREAD();}
// Opcode has wrote
#define SMWRITE()   dyn->smwrite=1
// Opcode has wrote (strongmem>1 only)
#define SMWRITE2()   if(box64_dynarec_strongmem>SMREAD_MIN) dyn->smwrite=1
// Opcode has wrote with option forced lock
#define SMWRITELOCK(lock)   if(lock || (box64_dynarec_strongmem>SMWRITE_MIN)) {SMDMB();} else dyn->smwrite=1
// Opcode might have wrote (depend on nextop)
#define SMMIGHTWRITE()   if(!MODREG) {SMWRITE();}
// Start of sequence
#define SMSTART()   SMEND()
// End of sequence
#define SMEND()     if(dyn->smwrite && box64_dynarec_strongmem) {FENCE();} dyn->smwrite=0; dyn->smread=0;
// Force a Data memory barrier (for LOCK: prefix)
#define SMDMB()     FENCE(); dyn->smwrite=0; dyn->smread=1

// LOCK_* define
#define LOCK_LOCK (int*)1

// GETGD    get x64 register in gd
#define GETGD gd = xRAX + ((nextop & 0x38) >> 3) + (rex.r << 3)
// GETED can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETED(D)                                                                                \
    if (MODREG) {                                                                               \
        ed = xRAX + (nextop & 7) + (rex.b << 3);                                                \
        wback = 0;                                                                              \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        LDxw(x1, wback, fixedaddress);                                                          \
        ed = x1;                                                                                \
    }
// GETSED can use r1 for ed, and r2 for wback. ed will be sign extended!
#define GETSED(D)                                                                               \
    if (MODREG) {                                                                               \
        ed = xRAX + (nextop & 7) + (rex.b << 3);                                                \
        wback = 0;                                                                              \
        if (!rex.w) {                                                                           \
            ADDW(x1, ed, xZR);                                                                  \
            ed = x1;                                                                            \
        }                                                                                       \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        if (rex.w)                                                                              \
            LD(x1, wback, fixedaddress);                                                        \
        else                                                                                    \
            LW(x1, wback, fixedaddress);                                                        \
        ed = x1;                                                                                \
    }
// GETEDx can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETEDx(D)                                                                               \
    if (MODREG) {                                                                               \
        ed = xRAX + (nextop & 7) + (rex.b << 3);                                                \
        wback = 0;                                                                              \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        LD(x1, wback, fixedaddress);                                                            \
        ed = x1;                                                                                \
    }
#define GETEDz(D)                                                                               \
    if (MODREG) {                                                                               \
        ed = xRAX + (nextop & 7) + (rex.b << 3);                                                \
        wback = 0;                                                                              \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        LDz(x1, wback, fixedaddress);                                                           \
        ed = x1;                                                                                \
    }
// GETED32 can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETED32(D)                                                                                \
    if (MODREG) {                                                                                 \
        ed = xRAX + (nextop & 7) + (rex.b << 3);                                                  \
        wback = 0;                                                                                \
    } else {                                                                                      \
        SMREAD();                                                                                 \
        addr = geted32(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        LDxw(x1, wback, fixedaddress);                                                            \
        ed = x1;                                                                                  \
    }
// GETEDH can use hint for ed, and x1 or x2 for wback (depending on hint), might also use x3. wback is 0 if ed is xEAX..xEDI
#define GETEDH(hint, D)                                                                                                                 \
    if (MODREG) {                                                                                                                       \
        ed = xRAX + (nextop & 7) + (rex.b << 3);                                                                                        \
        wback = 0;                                                                                                                      \
    } else {                                                                                                                            \
        SMREAD();                                                                                                                       \
        addr = geted(dyn, addr, ninst, nextop, &wback, (hint == x2) ? x1 : x2, (hint == x1) ? x1 : x3, &fixedaddress, rex, NULL, 1, D); \
        LDxw(hint, wback, fixedaddress);                                                                                                \
        ed = hint;                                                                                                                      \
    }
// GETEDW can use hint for wback and ret for ed. wback is 0 if ed is xEAX..xEDI
#define GETEDW(hint, ret, D)                                                                                                            \
    if (MODREG) {                                                                                                                       \
        ed = xRAX + (nextop & 7) + (rex.b << 3);                                                                                        \
        MV(ret, ed);                                                                                                                    \
        wback = 0;                                                                                                                      \
    } else {                                                                                                                            \
        SMREAD();                                                                                                                       \
        addr = geted(dyn, addr, ninst, nextop, &wback, (hint == x2) ? x1 : x2, (hint == x1) ? x1 : x3, &fixedaddress, rex, NULL, 0, D); \
        ed = ret;                                                                                                                       \
        LDxw(ed, wback, fixedaddress);                                                                                                  \
    }
// GETGW extract x64 register in gd, that is i
#define GETGW(i)                                       \
    gd = xRAX + ((nextop & 0x38) >> 3) + (rex.r << 3); \
    ZEXTH(i, gd);                                      \
    gd = i;
// GETEWW will use i for ed, and can use w for wback.
#define GETEWW(w, i, D)                                                                       \
    if (MODREG) {                                                                             \
        wback = xRAX + (nextop & 7) + (rex.b << 3);                                           \
        ZEXTH(i, wback);                                                                      \
        ed = i;                                                                               \
        wb1 = 0;                                                                              \
    } else {                                                                                  \
        SMREAD();                                                                             \
        addr = geted(dyn, addr, ninst, nextop, &wback, w, i, &fixedaddress, rex, NULL, 1, D); \
        LHU(i, wback, fixedaddress);                                                          \
        ed = i;                                                                               \
        wb1 = 1;                                                                              \
    }
// GETEW will use i for ed, and can use r3 for wback.
#define GETEW(i, D) GETEWW(x3, i, D)
// GETSEW will use i for ed, and can use r3 for wback. This is the Signed version
#define GETSEW(i, D)                                                                           \
    if (MODREG) {                                                                              \
        wback = xRAX + (nextop & 7) + (rex.b << 3);                                            \
        if (rv64_zbb)                                                                          \
            SEXTH(i, wback);                                                                   \
        else {                                                                                 \
            SLLI(i, wback, 48);                                                                \
            SRAI(i, i, 48);                                                                    \
        }                                                                                      \
        ed = i;                                                                                \
        wb1 = 0;                                                                               \
    } else {                                                                                   \
        SMREAD();                                                                              \
        addr = geted(dyn, addr, ninst, nextop, &wback, x3, i, &fixedaddress, rex, NULL, 1, D); \
        LH(i, wback, fixedaddress);                                                            \
        ed = i;                                                                                \
        wb1 = 1;                                                                               \
    }
// Write ed back to original register / memory
#define EWBACK EWBACKW(ed)
// Write w back to original register / memory (w needs to be 16bits only!)
#define EWBACKW(w)                  \
    if (wb1) {                      \
        SH(w, wback, fixedaddress); \
        SMWRITE();                  \
    } else {                        \
        SRLI(wback, wback, 16);     \
        SLLI(wback, wback, 16);     \
        OR(wback, wback, w);        \
    }
// Write back gd in correct register (gd needs to be 16bits only!)
#define GWBACK                                                \
    do {                                                      \
        int g = xRAX + ((nextop & 0x38) >> 3) + (rex.r << 3); \
        SRLI(g, g, 16);                                       \
        SLLI(g, g, 16);                                       \
        OR(g, g, gd);                                         \
    } while (0)

// GETEDO can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETEDO(O, D, S)                                                                        \
    if (MODREG) {                                                                              \
        ed = xRAX + (nextop & 7) + (rex.b << 3);                                               \
        wback = 0;                                                                             \
    } else {                                                                                   \
        SMREAD();                                                                              \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, S, &fixedaddress, rex, NULL, 1, D); \
        ADD(S, wback, O);                                                                      \
        LDxw(x1, S, fixedaddress);                                                             \
        ed = x1;                                                                               \
    }
// GETEDOz can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETEDOz(O, D, S)                                                                       \
    if (MODREG) {                                                                              \
        ed = xRAX + (nextop & 7) + (rex.b << 3);                                               \
        wback = 0;                                                                             \
    } else {                                                                                   \
        SMREAD();                                                                              \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, S, &fixedaddress, rex, NULL, 1, D); \
        ADD(S, wback, O);                                                                      \
        LDz(x1, S, fixedaddress);                                                              \
        ed = x1;                                                                               \
    }
#define WBACKO(O)         \
    if (wback) {          \
        ADD(O, wback, O); \
        SDxw(ed, O, 0);   \
        SMWRITE2();       \
    }

// FAKEED like GETED, but doesn't get anything
#define FAKEED                                   \
    if (!MODREG) {                               \
        addr = fakeed(dyn, addr, ninst, nextop); \
    }

// GETGW extract x64 register in gd, that is i, Signed extented
#define GETSGW(i)                                      \
    gd = xRAX + ((nextop & 0x38) >> 3) + (rex.r << 3); \
    SLLIW(i, gd, 16);                                  \
    SRAIW(i, i, 16);                                   \
    gd = i;

// Write back ed in wback (if wback not 0)
#define WBACK                          \
    if (wback) {                       \
        SDxw(ed, wback, fixedaddress); \
        SMWRITE();                     \
    }

// GETEB will use i for ed, and can use r3 for wback.
#define GETEB(i, D)                                                                             \
    if (MODREG) {                                                                               \
        if (rex.rex) {                                                                          \
            wback = xRAX + (nextop & 7) + (rex.b << 3);                                         \
            wb2 = 0;                                                                            \
        } else {                                                                                \
            wback = (nextop & 7);                                                               \
            wb2 = (wback >> 2) * 8;                                                             \
            wback = xRAX + (wback & 3);                                                         \
        }                                                                                       \
        if (wb2) {                                                                              \
            if (rv64_xtheadbb) {                                                                \
                TH_EXTU(i, wback, 15, 8);                                                       \
            } else {                                                                            \
                SRLI(i, wback, wb2);                                                            \
                ANDI(i, i, 0xff);                                                               \
            }                                                                                   \
        } else                                                                                  \
            ANDI(i, wback, 0xff);                                                               \
        wb1 = 0;                                                                                \
        ed = i;                                                                                 \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 1, D); \
        LBU(i, wback, fixedaddress);                                                            \
        wb1 = 1;                                                                                \
        ed = i;                                                                                 \
    }
// GETEBO will use i for ed, i is also Offset, and can use r3 for wback.
#define GETEBO(i, D)                                                                            \
    if (MODREG) {                                                                               \
        if (rex.rex) {                                                                          \
            wback = xRAX + (nextop & 7) + (rex.b << 3);                                         \
            wb2 = 0;                                                                            \
        } else {                                                                                \
            wback = (nextop & 7);                                                               \
            wb2 = (wback >> 2) * 8;                                                             \
            wback = xRAX + (wback & 3);                                                         \
        }                                                                                       \
        if (wb2) {                                                                              \
            if (rv64_xtheadbb) {                                                                \
                TH_EXTU(i, wback, 15, 8);                                                       \
            } else {                                                                            \
                SRLI(i, wback, wb2);                                                            \
                ANDI(i, i, 0xff);                                                               \
            }                                                                                   \
        } else                                                                                  \
            ANDI(i, wback, 0xff);                                                               \
        wb1 = 0;                                                                                \
        ed = i;                                                                                 \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 1, D); \
        ADD(x3, wback, i);                                                                      \
        if (wback != x3) wback = x3;                                                            \
        LBU(i, wback, fixedaddress);                                                            \
        wb1 = 1;                                                                                \
        ed = i;                                                                                 \
    }
// GETSEB sign extend EB, will use i for ed, and can use r3 for wback.
#define GETSEB(i, D)                                                                            \
    if (MODREG) {                                                                               \
        if (rex.rex) {                                                                          \
            wback = xRAX + (nextop & 7) + (rex.b << 3);                                         \
            wb2 = 0;                                                                            \
        } else {                                                                                \
            wback = (nextop & 7);                                                               \
            wb2 = (wback >> 2) * 8;                                                             \
            wback = xRAX + (wback & 3);                                                         \
        }                                                                                       \
        MV(i, wback);                                                                           \
        SLLIW(i, i, 24 - wb2);                                                                  \
        SRAIW(i, i, 24);                                                                        \
        wb1 = 0;                                                                                \
        ed = i;                                                                                 \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 1, D); \
        LB(i, wback, fixedaddress);                                                             \
        wb1 = 1;                                                                                \
        ed = i;                                                                                 \
    }
// GETEB32 will use i for ed, and can use r3 for wback.
#define GETEB32(i, D)                                                                             \
    if (MODREG) {                                                                                 \
        if (rex.rex) {                                                                            \
            wback = xRAX + (nextop & 7) + (rex.b << 3);                                           \
            wb2 = 0;                                                                              \
        } else {                                                                                  \
            wback = (nextop & 7);                                                                 \
            wb2 = (wback >> 2) * 8;                                                               \
            wback = xRAX + (wback & 3);                                                           \
        }                                                                                         \
        if (wb2) {                                                                                \
            if (rv64_xtheadbb) {                                                                  \
                TH_EXTU(i, wback, 15, 8);                                                         \
            } else {                                                                              \
                MV(i, wback);                                                                     \
                SRLI(i, i, wb2);                                                                  \
                ANDI(i, i, 0xff);                                                                 \
            }                                                                                     \
        } else                                                                                    \
            ANDI(i, wback, 0xff);                                                                 \
        wb1 = 0;                                                                                  \
        ed = i;                                                                                   \
    } else {                                                                                      \
        SMREAD();                                                                                 \
        addr = geted32(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 1, D); \
        LBU(i, wback, fixedaddress);                                                              \
        wb1 = 1;                                                                                  \
        ed = i;                                                                                   \
    }

// GETGB will use i for gd
#define GETGB(i)                                            \
    if (rex.rex) {                                          \
        gb1 = xRAX + ((nextop & 0x38) >> 3) + (rex.r << 3); \
        gb2 = 0;                                            \
    } else {                                                \
        gd = (nextop & 0x38) >> 3;                          \
        gb2 = ((gd & 4) >> 2);                              \
        gb1 = xRAX + (gd & 3);                              \
    }                                                       \
    gd = i;                                                 \
    if (gb2) {                                              \
        if (rv64_xtheadbb) {                                \
            TH_EXTU(gd, gb1, 15, 8);                        \
        } else {                                            \
            SRLI(gd, gb1, 8);                               \
            ANDI(gd, gd, 0xff);                             \
        }                                                   \
    } else                                                  \
        ANDI(gd, gb1, 0xff);

// Write gb (gd) back to original register / memory, using s1 as scratch
#define GBBACK(s1)                        \
    if (gb2) {                            \
        MOV64x(s1, 0xffffffffffff00ffLL); \
        AND(gb1, gb1, s1);                \
        SLLI(s1, gd, 8);                  \
        OR(gb1, gb1, s1);                 \
    } else {                              \
        ANDI(gb1, gb1, ~0xff);            \
        OR(gb1, gb1, gd);                 \
    }

// Write eb (ed) back to original register / memory, using s1 as scratch
#define EBBACK(s1, c)                     \
    if (wb1) {                            \
        SB(ed, wback, fixedaddress);      \
        SMWRITE();                        \
    } else if (wb2) {                     \
        MOV64x(s1, 0xffffffffffff00ffLL); \
        AND(wback, wback, s1);            \
        if (c) { ANDI(ed, ed, 0xff); }    \
        SLLI(s1, ed, 8);                  \
        OR(wback, wback, s1);             \
    } else {                              \
        ANDI(wback, wback, ~0xff);        \
        if (c) { ANDI(ed, ed, 0xff); }    \
        OR(wback, wback, ed);             \
    }

// Get direction with size Z and based of F_DF flag, on register r ready for load/store fetching
// using s as scratch.
#define GETDIR(r, s, Z)            \
    MOV32w(r, Z); /* mask=1<<10 */ \
    ANDI(s, xFlags, 1 << F_DF);    \
    BEQZ(s, 8);                    \
    SUB(r, xZR, r);

// Generic get GD, but reg value in gd (R_RAX is not added)
#define GETG gd = ((nextop & 0x38) >> 3) + (rex.r << 3)

// Get GX as a Single (might use x2)
#define GETGXSS(a)                              \
    gd = ((nextop & 0x38) >> 3) + (rex.r << 3); \
    a = sse_get_reg(dyn, ninst, x2, gd, 1)

// Get GX as a Single (might use x2), no fetching old value
#define GETGXSS_empty(a)                        \
    gd = ((nextop & 0x38) >> 3) + (rex.r << 3); \
    a = sse_get_reg_empty(dyn, ninst, x2, gd, 1)

// Get GX as a Double (might use x2)
#define GETGXSD(a)                              \
    gd = ((nextop & 0x38) >> 3) + (rex.r << 3); \
    a = sse_get_reg(dyn, ninst, x2, gd, 0)

// Get GX as a Double (might use x2), no fetching old value
#define GETGXSD_empty(a)                        \
    gd = ((nextop & 0x38) >> 3) + (rex.r << 3); \
    a = sse_get_reg_empty(dyn, ninst, x2, gd, 0)

// Get Ex as a single, not a quad (warning, x1 get used, x2 might too)
#define GETEXSS(a, D)                                                                        \
    if (MODREG) {                                                                            \
        a = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);                     \
    } else {                                                                                 \
        SMREAD();                                                                            \
        a = fpu_get_scratch(dyn);                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, D); \
        FLW(a, ed, fixedaddress);                                                            \
    }

// Get Ex as a double, not a quad (warning, x1 get used, x2 might too)
#define GETEXSD(a, D)                                                                        \
    if (MODREG) {                                                                            \
        a = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);                     \
    } else {                                                                                 \
        SMREAD();                                                                            \
        a = fpu_get_scratch(dyn);                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, D); \
        FLD(a, ed, fixedaddress);                                                            \
    }

// Will get pointer to GX in general register a, will purge SS or SD if loaded. can use gback as load address
#define GETGX()                                 \
    gd = ((nextop & 0x38) >> 3) + (rex.r << 3); \
    sse_forget_reg(dyn, ninst, gd);             \
    gback = xEmu;                               \
    gdoffset = offsetof(x64emu_t, xmm[gd])

// Get Ex address in general register a, will purge SS or SD if it's reg and is loaded. May use x3. Use wback as load address!
#define GETEX(a, D)                                                                            \
    if (MODREG) {                                                                              \
        ed = (nextop & 7) + (rex.b << 3);                                                      \
        sse_forget_reg(dyn, ninst, ed);                                                        \
        fixedaddress = offsetof(x64emu_t, xmm[ed]);                                            \
        wback = xEmu;                                                                          \
    } else {                                                                                   \
        SMREAD();                                                                              \
        ed = 16;                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, a, x3, &fixedaddress, rex, NULL, 0, D); \
        fixedaddress = 0; /* TODO: optimize this! */                                           \
    }

#define GETGM()                     \
    gd = ((nextop & 0x38) >> 3);    \
    mmx_forget_reg(dyn, ninst, gd); \
    gback = xEmu;                   \
    gdoffset = offsetof(x64emu_t, mmx[gd])

// Get EM, might use x3
#define GETEM(a, D)                                                                            \
    if (MODREG) {                                                                              \
        ed = (nextop & 7);                                                                     \
        mmx_forget_reg(dyn, ninst, ed);                                                        \
        fixedaddress = offsetof(x64emu_t, mmx[ed]);                                            \
        wback = xEmu;                                                                          \
    } else {                                                                                   \
        SMREAD();                                                                              \
        ed = 8;                                                                                \
        addr = geted(dyn, addr, ninst, nextop, &wback, a, x3, &fixedaddress, rex, NULL, 0, D); \
        fixedaddress = 0; /* TODO: optimize this! */                                           \
    }

#define SSE_LOOP_D_ITEM(GX1, EX1, F, i)    \
    LWU(GX1, gback, gdoffset + i * 4);     \
    LWU(EX1, wback, fixedaddress + i * 4); \
    F;                                     \
    SW(GX1, gback, gdoffset + i * 4);

// Loop for SSE opcode that use 32bits value and write to GX.
#define SSE_LOOP_D(GX1, EX1, F)     \
    SSE_LOOP_D_ITEM(GX1, EX1, F, 0) \
    SSE_LOOP_D_ITEM(GX1, EX1, F, 1) \
    SSE_LOOP_D_ITEM(GX1, EX1, F, 2) \
    SSE_LOOP_D_ITEM(GX1, EX1, F, 3)

#define SSE_LOOP_DS_ITEM(GX1, EX1, F, i)  \
    LW(GX1, gback, gdoffset + i * 4);     \
    LW(EX1, wback, fixedaddress + i * 4); \
    F;                                    \
    SW(GX1, gback, gdoffset + i * 4);

// Loop for SSE opcode that use 32bits value and write to GX.
#define SSE_LOOP_DS(GX1, EX1, F)     \
    SSE_LOOP_DS_ITEM(GX1, EX1, F, 0) \
    SSE_LOOP_DS_ITEM(GX1, EX1, F, 1) \
    SSE_LOOP_DS_ITEM(GX1, EX1, F, 2) \
    SSE_LOOP_DS_ITEM(GX1, EX1, F, 3)

#define MMX_LOOP_D(GX1, EX1, F)                \
    for (int i = 0; i < 2; ++i) {              \
        LWU(GX1, gback, gdoffset + i * 4);     \
        LWU(EX1, wback, fixedaddress + i * 4); \
        F;                                     \
        SW(GX1, gback, gdoffset + i * 4);      \
    }

#define MMX_LOOP_W(GX1, EX1, F)                \
    for (int i = 0; i < 4; ++i) {              \
        LHU(GX1, gback, gdoffset + i * 2);     \
        LHU(EX1, wback, fixedaddress + i * 2); \
        F;                                     \
        SH(GX1, gback, gdoffset + i * 2);      \
    }

#define MMX_LOOP_WS(GX1, EX1, F)              \
    for (int i = 0; i < 4; ++i) {             \
        LH(GX1, gback, gdoffset + i * 2);     \
        LH(EX1, wback, fixedaddress + i * 2); \
        F;                                    \
        SH(GX1, gback, gdoffset + i * 2);     \
    }

#define SSE_LOOP_W(GX1, EX1, F)                \
    for (int i = 0; i < 8; ++i) {              \
        LHU(GX1, gback, gdoffset + i * 2);     \
        LHU(EX1, wback, fixedaddress + i * 2); \
        F;                                     \
        SH(GX1, gback, gdoffset + i * 2);      \
    }

#define SSE_LOOP_WS(GX1, EX1, F)              \
    for (int i = 0; i < 8; ++i) {             \
        LH(GX1, gback, gdoffset + i * 2);     \
        LH(EX1, wback, fixedaddress + i * 2); \
        F;                                    \
        SH(GX1, gback, gdoffset + i * 2);     \
    }

#define SSE_LOOP_D_S_ITEM(EX1, F, i)       \
    LWU(EX1, wback, fixedaddress + i * 4); \
    F;                                     \
    SW(EX1, wback, fixedaddress + i * 4);

// Loop for SSE opcode that use 32bits value and write to EX.
#define SSE_LOOP_D_S(EX1, F)     \
    SSE_LOOP_D_S_ITEM(EX1, F, 0) \
    SSE_LOOP_D_S_ITEM(EX1, F, 1) \
    SSE_LOOP_D_S_ITEM(EX1, F, 2) \
    SSE_LOOP_D_S_ITEM(EX1, F, 3)

#define SSE_LOOP_Q_ITEM(GX1, EX1, F, i)   \
    LD(GX1, gback, gdoffset + i * 8);     \
    LD(EX1, wback, fixedaddress + i * 8); \
    F;                                    \
    SD(GX1, gback, gdoffset + i * 8);

// Loop for SSE opcode that use 64bits value and write to GX.
#define SSE_LOOP_Q(GX1, EX1, F)     \
    SSE_LOOP_Q_ITEM(GX1, EX1, F, 0) \
    SSE_LOOP_Q_ITEM(GX1, EX1, F, 1)


#define SSE_LOOP_FQ_ITEM(GX1, EX1, F, i)  \
    FLD(v0, gback, gdoffset + i * 8);     \
    FLD(v1, wback, fixedaddress + i * 8); \
    F;                                    \
    FSD(v0, gback, gdoffset + i * 8);

#define SSE_LOOP_FQ(GX1, EX1, F)     \
    v0 = fpu_get_scratch(dyn);       \
    v1 = fpu_get_scratch(dyn);       \
    SSE_LOOP_FQ_ITEM(GX1, EX1, F, 0) \
    SSE_LOOP_FQ_ITEM(GX1, EX1, F, 1)


#define SSE_LOOP_MV_Q_ITEM(s, i)        \
    LD(s, wback, fixedaddress + i * 8); \
    SD(s, gback, gdoffset + i * 8);

// Loop for SSE opcode that moves 64bits value from wback to gback, use s as scratch.
#define SSE_LOOP_MV_Q(s)     \
    SSE_LOOP_MV_Q_ITEM(s, 0) \
    SSE_LOOP_MV_Q_ITEM(s, 1)

#define SSE_LOOP_MV_Q_ITEM2(s, i)   \
    LD(s, gback, gdoffset + i * 8); \
    SD(s, wback, fixedaddress + i * 8);

// Loop for SSE opcode that moves 64bits value from gback to wback, use s as scratch.
#define SSE_LOOP_MV_Q2(s)     \
    SSE_LOOP_MV_Q_ITEM2(s, 0) \
    SSE_LOOP_MV_Q_ITEM2(s, 1)

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

#define Bxx_geni(OP, M, reg1, reg2, i)     \
    j64 = GET##M##i(i) - dyn->native_size; \
    B##OP(reg1, reg2, j64)

// Branch to MARK if reg1==reg2 (use j64)
#define BEQ_MARK(reg1, reg2)     Bxx_gen(EQ, MARK, reg1, reg2)
#define BEQ_MARKi(reg1, reg2, i) Bxx_geni(EQ, MARK, reg1, reg2, i)
// Branch to MARK if reg1==0 (use j64)
#define BEQZ_MARK(reg)     BEQ_MARK(reg, xZR)
#define BEQZ_MARKi(reg, i) BEQ_MARKi(reg, xZR, i)
// Branch to MARK if reg1!=reg2 (use j64)
#define BNE_MARK(reg1, reg2)     Bxx_gen(NE, MARK, reg1, reg2)
#define BNE_MARKi(reg1, reg2, i) Bxx_geni(NE, MARK, reg1, reg2, i)
// Branch to MARK if reg1!=0 (use j64)
#define BNEZ_MARK(reg)     BNE_MARK(reg, xZR)
#define BNEZ_MARKi(reg, i) BNE_MARKi(reg, xZR, i)
// Branch to MARK instruction unconditionnal (use j64)
#define B_MARK_nocond  Bxx_gen(__, MARK, 0, 0)
#define B_MARKi_nocond Bxx_geni(__, MARK, 0, 0, i)
// Branch to MARK if reg1<reg2 (use j64)
#define BLT_MARK(reg1, reg2)  Bxx_gen(LT, MARK, reg1, reg2)
#define BLT_MARKi(reg1, reg2) Bxx_geni(LT, MARK, reg1, reg2, i)
// Branch to MARK if reg1<reg2 (use j64)
#define BLTU_MARK(reg1, reg2)  Bxx_gen(LTU, MARK, reg1, reg2)
#define BLTU_MARKi(reg1, reg2) Bxx_geni(LTU, MARK, reg1, reg2, i)
// Branch to MARK if reg1>=reg2 (use j64)
#define BGE_MARK(reg1, reg2)  Bxx_gen(GE, MARK, reg1, reg2)
#define BGE_MARKi(reg1, reg2) Bxx_geni(GE, MARK, reg1, reg2, i)
// Branch to MARK2 if reg1==reg2 (use j64)
#define BEQ_MARK2(reg1, reg2) Bxx_gen(EQ, MARK2, reg1, reg2)
// Branch to MARK2 if reg1!=reg2 (use j64)
#define BNE_MARK2(reg1, reg2) Bxx_gen(NE, MARK2, reg1, reg2)
// Branch to MARK2 if reg1!=0 (use j64)
#define BNEZ_MARK2(reg) BNE_MARK2(reg, xZR)
// Branch to MARK2 if reg1<reg2 (use j64)
#define BLT_MARK2(reg1, reg2) Bxx_gen(LT, MARK2, reg1, reg2)
// Branch to MARK2 instruction unconditionnal (use j64)
#define B_MARK2_nocond Bxx_gen(__, MARK2, 0, 0)
// Branch to MARK3 if reg1==reg2 (use j64)
#define BEQ_MARK3(reg1, reg2) Bxx_gen(EQ, MARK3, reg1, reg2)
// Branch to MARK3 if reg1!=reg2 (use j64)
#define BNE_MARK3(reg1, reg2) Bxx_gen(NE, MARK3, reg1, reg2)
// Branch to MARK3 if reg1!>=reg2 (use j64)
#define BGE_MARK3(reg1, reg2) Bxx_gen(GE, MARK3, reg1, reg2)
// Branch to MARK3 if reg1!=0 (use j64)
#define BNEZ_MARK3(reg) BNE_MARK3(reg, xZR)
// Branch to MARK3 if reg1==0 (use j64)
#define BEQZ_MARK3(reg) BEQ_MARK3(reg, xZR)
// Branch to MARK3 instruction unconditionnal (use j64)
#define B_MARK3_nocond Bxx_gen(__, MARK3, 0, 0)
// Branch to MARKLOCK if reg1!=reg2 (use j64)
#define BNE_MARKLOCK(reg1, reg2) Bxx_gen(NE, MARKLOCK, reg1, reg2)
// Branch to MARKLOCK if reg1!=0 (use j64)
#define BNEZ_MARKLOCK(reg) BNE_MARKLOCK(reg, xZR)

// Branch to NEXT if reg1==reg2 (use j64)
#define BEQ_NEXT(reg1, reg2)                                                  \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    BEQ(reg1, reg2, j64)

// Branch to NEXT if reg1!=reg2 (use j64)
#define BNE_NEXT(reg1, reg2)                                                  \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    BNE(reg1, reg2, j64)

// Branch to NEXT if reg1==0 (use j64)
#define CBZ_NEXT(reg1)                                                        \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    BEQ(reg1, xZR, j64)
// Branch to NEXT if reg1!=0 (use j64)
#define CBNZ_NEXT(reg1)                                                       \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    BNE(reg1, xZR, j64)
// Branch to next instruction unconditionnal (use j64)
#define B_NEXT_nocond                                                         \
    j64 = (dyn->insts) ? (dyn->insts[ninst].epilog - (dyn->native_size)) : 0; \
    B(j64)

// Branch to MARKSEG if reg is 0 (use j64)
#define CBZ_MARKSEG(reg)                   \
    j64 = GETMARKSEG - (dyn->native_size); \
    BEQZ(reg, j64);
// Branch to MARKSEG if reg is not 0 (use j64)
#define CBNZ_MARKSEG(reg)                  \
    j64 = GETMARKSEG - (dyn->native_size); \
    BNEZ(reg, j64);

#define IFX(A)      if ((dyn->insts[ninst].x64.gen_flags & (A)))
#define IFX_PENDOR0 if ((dyn->insts[ninst].x64.gen_flags & (X_PEND) || !dyn->insts[ninst].x64.gen_flags))
#define IFXX(A)     if ((dyn->insts[ninst].x64.gen_flags == (A)))
#define IFX2X(A, B) if ((dyn->insts[ninst].x64.gen_flags == (A) || dyn->insts[ninst].x64.gen_flags == (B) || dyn->insts[ninst].x64.gen_flags == ((A) | (B))))
#define IFXN(A, B)  if ((dyn->insts[ninst].x64.gen_flags & (A) && !(dyn->insts[ninst].x64.gen_flags & (B))))

#define STORE_REG(A) SD(x##A, xEmu, offsetof(x64emu_t, regs[_##A]))
#define LOAD_REG(A)  LD(x##A, xEmu, offsetof(x64emu_t, regs[_##A]))

// Need to also store current value of some register, as they may be used by functions like setjmp
#define STORE_XEMU_CALL(s0)                             \
    if (rv64_xtheadmempair) {                           \
        ADDI(s0, xEmu, offsetof(x64emu_t, regs[_RSP])); \
        TH_SDD(xRDX, xRBX, xEmu, 1);                    \
        TH_SDD(xRSP, xRBP, s0, 0);                      \
        TH_SDD(xRSI, xRDI, s0, 1);                      \
        TH_SDD(xR8, xR9, s0, 2);                        \
        TH_SDD(xR10, xR11, s0, 3);                      \
    } else {                                            \
        STORE_REG(RBX);                                 \
        STORE_REG(RDX);                                 \
        STORE_REG(RSP);                                 \
        STORE_REG(RBP);                                 \
        STORE_REG(RDI);                                 \
        STORE_REG(RSI);                                 \
        STORE_REG(R8);                                  \
        STORE_REG(R9);                                  \
        STORE_REG(R10);                                 \
        STORE_REG(R11);                                 \
    }

#define LOAD_XEMU_CALL()

#define LOAD_XEMU_REM(s0)                               \
    if (rv64_xtheadmempair) {                           \
        ADDI(s0, xEmu, offsetof(x64emu_t, regs[_RSP])); \
        TH_LDD(xRDX, xRBX, xEmu, 1);                    \
        TH_LDD(xRSP, xRBP, s0, 0);                      \
        TH_LDD(xRSI, xRDI, s0, 1);                      \
        TH_LDD(xR8, xR9, s0, 2);                        \
        TH_LDD(xR10, xR11, s0, 3);                      \
    } else {                                            \
        LOAD_REG(RBX);                                  \
        LOAD_REG(RDX);                                  \
        LOAD_REG(RSP);                                  \
        LOAD_REG(RBP);                                  \
        LOAD_REG(RDI);                                  \
        LOAD_REG(RSI);                                  \
        LOAD_REG(R8);                                   \
        LOAD_REG(R9);                                   \
        LOAD_REG(R10);                                  \
        LOAD_REG(R11);                                  \
    }


#define SET_DFNONE()                           \
    if (!dyn->f.dfnone) {                      \
        SW(xZR, xEmu, offsetof(x64emu_t, df)); \
        dyn->f.dfnone = 1;                     \
    }
#define SET_DF(S, N)                         \
    if ((N) != d_none) {                     \
        MOV_U12(S, (N));                     \
        SW(S, xEmu, offsetof(x64emu_t, df)); \
        dyn->f.dfnone = 0;                   \
    } else                                   \
        SET_DFNONE()
#define SET_NODF() dyn->f.dfnone = 0
#define SET_DFOK() dyn->f.dfnone = 1

#define CLEAR_FLAGS() \
    IFX(X_ALL) { ANDI(xFlags, xFlags, ~((1UL << F_AF) | (1UL << F_CF) | (1UL << F_OF2) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF))); }

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
                SRLI(scratch1, scratch2, (width)-1);                  \
                if (width != 64) ANDI(scratch1, scratch1, 1);         \
            }                                                         \
            BEQZ(scratch1, 8);                                        \
            ORI(xFlags, xFlags, 1 << F_CF);                           \
        }                                                             \
        IFX(X_OF)                                                     \
        {                                                             \
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */ \
            SRLI(scratch1, scratch2, (width)-2);                      \
            SRLI(scratch2, scratch1, 1);                              \
            XOR(scratch1, scratch1, scratch2);                        \
            ANDI(scratch1, scratch1, 1);                              \
            BEQZ(scratch1, 8);                                        \
            ORI(xFlags, xFlags, 1 << F_OF2);                          \
        }                                                             \
    }

// Adjust the flags bit 11 -> bit 5, result in reg (can be same as flags, but not s1)
#define FLAGS_ADJUST_FROM11(reg, flags, s1) \
    ANDI(reg, flags, ~(1 << 5));            \
    SRLI(s1, reg, 11 - 5);                  \
    ANDI(s1, s1, 1 << 5);                   \
    OR(reg, reg, s1)

// Adjust the xFlags bit 5 -> bit 11, src and dst can be the same (and can be xFlags, but not s1)
#define FLAGS_ADJUST_TO11(dst, src, s1) \
    LUI(s1, 0xFFFFF);                   \
    ADDIW(s1, s1, 0x7DF);               \
    AND(s1, src, s1);                   \
    ANDI(dst, src, 1 << 5);             \
    SLLI(dst, dst, 11 - 5);             \
    OR(dst, dst, s1)

#if STEP == 0
#define X87_PUSH_OR_FAIL(var, dyn, ninst, scratch, t)   var = x87_do_push(dyn, ninst, scratch, t)
#define X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, scratch)     x87_do_push_empty(dyn, ninst, scratch)
#define X87_POP_OR_FAIL(dyn, ninst, scratch)            x87_do_pop(dyn, ninst, scratch)
#else
#define X87_PUSH_OR_FAIL(var, dyn, ninst, scratch, t) \
    if ((dyn->e.x87stack==8) || (dyn->e.pushed==8)) {   \
        if(box64_dynarec_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Push, stack=%d/%d on inst %d\n", dyn->e.x87stack, dyn->e.pushed, ninst); \
        dyn->abort = 1;                                 \
        return addr;                                    \
    }                                                   \
    var = x87_do_push(dyn, ninst, scratch, t);

#define X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, scratch) \
    if ((dyn->e.x87stack==8) || (dyn->e.pushed==8)) {   \
        if(box64_dynarec_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Push, stack=%d/%d on inst %d\n", dyn->e.x87stack, dyn->e.pushed, ninst); \
        dyn->abort = 1;                                 \
        return addr;                                    \
    }                                                   \
    x87_do_push_empty(dyn, ninst, scratch);

#define X87_POP_OR_FAIL(dyn, ninst, scratch) \
    if ((dyn->e.x87stack==-8) || (dyn->e.poped==8)) {   \
        if(box64_dynarec_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Pop, stack=%d/%d on inst %d\n", dyn->e.x87stack, dyn->e.poped, ninst); \
        dyn->abort = 1;                                 \
        return addr;                                    \
    }                                                   \
    x87_do_pop(dyn, ninst, scratch);
#endif

#ifndef MAYSETFLAGS
#define MAYSETFLAGS()
#endif

#ifndef READFLAGS
#define READFLAGS(A)                                \
    if (((A) != X_PEND && dyn->f.pending != SF_SET) \
        && (dyn->f.pending != SF_SET_PENDING)) {    \
        if (dyn->f.pending != SF_PENDING) {         \
            LD(x3, xEmu, offsetof(x64emu_t, df));   \
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
        && ((B)&SF_SUB)                                                                                             \
        && (dyn->insts[ninst].x64.gen_flags & (~(A))))                                                              \
        READFLAGS(((dyn->insts[ninst].x64.gen_flags & X_PEND) ? X_ALL : dyn->insts[ninst].x64.gen_flags) & (~(A))); \
    if (dyn->insts[ninst].x64.gen_flags) switch (B) {                                                               \
            case SF_SUBSET:                                                                                         \
            case SF_SET: dyn->f.pending = SF_SET; break;                                                            \
            case SF_SET_DF: dyn->f.pending = SF_SET; dyn->f.dfnone = 1; break;                                      \
            case SF_SET_NODF: dyn->f.pending = SF_SET; dyn->f.dfnone = 0; break;                                    \
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
#ifndef BARRIER_NEXT
#define BARRIER_NEXT(A)
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
#ifndef FTABLE64
#define FTABLE64(A, V)
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
            ADDI(xRIP, xRIP, _delta_ip);             \
        }                                            \
    } else {                                         \
        dyn->last_ip = (A);                          \
        if (dyn->last_ip < 0xffffffff) {             \
            MOV64x(xRIP, dyn->last_ip);              \
        } else                                       \
            TABLE64(xRIP, dyn->last_ip);             \
    }
#define GETIP_(A)                                       \
    if (dyn->last_ip && ((A)-dyn->last_ip) < 2048) {    \
        int64_t _delta_ip = (A)-dyn->last_ip;           \
        if (_delta_ip) { ADDI(xRIP, xRIP, _delta_ip); } \
    } else {                                            \
        if ((A) < 0xffffffff) {                         \
            MOV64x(xRIP, (A));                          \
        } else                                          \
            TABLE64(xRIP, (A));                         \
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

void rv64_epilog(void);
void rv64_epilog_fast(void);
void* rv64_next(x64emu_t* emu, uintptr_t addr);

#ifndef STEPNAME
#define STEPNAME3(N, M) N##M
#define STEPNAME2(N, M) STEPNAME3(N, M)
#define STEPNAME(N)     STEPNAME2(N, STEP)
#endif

#define native_pass STEPNAME(native_pass)

#define dynarec64_00     STEPNAME(dynarec64_00)
#define dynarec64_00_0   STEPNAME(dynarec64_00_0)
#define dynarec64_00_1   STEPNAME(dynarec64_00_1)
#define dynarec64_00_2   STEPNAME(dynarec64_00_2)
#define dynarec64_00_3   STEPNAME(dynarec64_00_3)
#define dynarec64_0F     STEPNAME(dynarec64_0F)
#define dynarec64_64     STEPNAME(dynarec64_64)
#define dynarec64_65     STEPNAME(dynarec64_65)
#define dynarec64_66     STEPNAME(dynarec64_66)
#define dynarec64_67     STEPNAME(dynarec64_67)
#define dynarec64_67_32  STEPNAME(dynarec64_67_32)
#define dynarec64_D8     STEPNAME(dynarec64_D8)
#define dynarec64_D9     STEPNAME(dynarec64_D9)
#define dynarec64_DA     STEPNAME(dynarec64_DA)
#define dynarec64_DB     STEPNAME(dynarec64_DB)
#define dynarec64_DC     STEPNAME(dynarec64_DC)
#define dynarec64_DD     STEPNAME(dynarec64_DD)
#define dynarec64_DE     STEPNAME(dynarec64_DE)
#define dynarec64_DF     STEPNAME(dynarec64_DF)
#define dynarec64_F0     STEPNAME(dynarec64_F0)
#define dynarec64_660F   STEPNAME(dynarec64_660F)
#define dynarec64_66F20F STEPNAME(dynarec64_66F20F)
#define dynarec64_66F30F STEPNAME(dynarec64_66F30F)
#define dynarec64_6664   STEPNAME(dynarec64_6664)
#define dynarec64_66F0   STEPNAME(dynarec64_66F0)
#define dynarec64_F20F   STEPNAME(dynarec64_F20F)
#define dynarec64_F30F   STEPNAME(dynarec64_F30F)

#define geted               STEPNAME(geted)
#define geted32             STEPNAME(geted32)
#define geted16             STEPNAME(geted16)
#define jump_to_epilog      STEPNAME(jump_to_epilog)
#define jump_to_epilog_fast STEPNAME(jump_to_epilog_fast)
#define jump_to_next        STEPNAME(jump_to_next)
#define ret_to_epilog       STEPNAME(ret_to_epilog)
#define retn_to_epilog      STEPNAME(retn_to_epilog)
#define iret_to_epilog      STEPNAME(iret_to_epilog)
#define call_c              STEPNAME(call_c)
#define call_n              STEPNAME(call_n)
#define grab_segdata        STEPNAME(grab_segdata)
#define emit_cmp8           STEPNAME(emit_cmp8)
#define emit_cmp16          STEPNAME(emit_cmp16)
#define emit_cmp32          STEPNAME(emit_cmp32)
#define emit_cmp8_0         STEPNAME(emit_cmp8_0)
#define emit_cmp16_0        STEPNAME(emit_cmp16_0)
#define emit_cmp32_0        STEPNAME(emit_cmp32_0)
#define emit_test8          STEPNAME(emit_test8)
#define emit_test16         STEPNAME(emit_test16)
#define emit_test32         STEPNAME(emit_test32)
#define emit_test32c        STEPNAME(emit_test32)
#define emit_add32          STEPNAME(emit_add32)
#define emit_add32c         STEPNAME(emit_add32c)
#define emit_add8           STEPNAME(emit_add8)
#define emit_add8c          STEPNAME(emit_add8c)
#define emit_sub32          STEPNAME(emit_sub32)
#define emit_sub32c         STEPNAME(emit_sub32c)
#define emit_sub8           STEPNAME(emit_sub8)
#define emit_sub8c          STEPNAME(emit_sub8c)
#define emit_or32           STEPNAME(emit_or32)
#define emit_or32c          STEPNAME(emit_or32c)
#define emit_xor32          STEPNAME(emit_xor32)
#define emit_xor32c         STEPNAME(emit_xor32c)
#define emit_and32          STEPNAME(emit_and32)
#define emit_and32c         STEPNAME(emit_and32c)
#define emit_or8            STEPNAME(emit_or8)
#define emit_or8c           STEPNAME(emit_or8c)
#define emit_xor8           STEPNAME(emit_xor8)
#define emit_xor8c          STEPNAME(emit_xor8c)
#define emit_and8           STEPNAME(emit_and8)
#define emit_and8c          STEPNAME(emit_and8c)
#define emit_add16          STEPNAME(emit_add16)
#define emit_add16c         STEPNAME(emit_add16c)
#define emit_sub16          STEPNAME(emit_sub16)
#define emit_sub16c         STEPNAME(emit_sub16c)
#define emit_or16           STEPNAME(emit_or16)
#define emit_or16c          STEPNAME(emit_or16c)
#define emit_xor16          STEPNAME(emit_xor16)
#define emit_xor16c         STEPNAME(emit_xor16c)
#define emit_and16          STEPNAME(emit_and16)
#define emit_and16c         STEPNAME(emit_and16c)
#define emit_inc32          STEPNAME(emit_inc32)
#define emit_inc16          STEPNAME(emit_inc16)
#define emit_inc8           STEPNAME(emit_inc8)
#define emit_dec32          STEPNAME(emit_dec32)
#define emit_dec16          STEPNAME(emit_dec16)
#define emit_dec8           STEPNAME(emit_dec8)
#define emit_adc32          STEPNAME(emit_adc32)
#define emit_adc32c         STEPNAME(emit_adc32c)
#define emit_adc8           STEPNAME(emit_adc8)
#define emit_adc8c          STEPNAME(emit_adc8c)
#define emit_adc16          STEPNAME(emit_adc16)
#define emit_adc16c         STEPNAME(emit_adc16c)
#define emit_sbb32          STEPNAME(emit_sbb32)
#define emit_sbb32c         STEPNAME(emit_sbb32c)
#define emit_sbb8           STEPNAME(emit_sbb8)
#define emit_sbb8c          STEPNAME(emit_sbb8c)
#define emit_sbb16          STEPNAME(emit_sbb16)
#define emit_sbb16c         STEPNAME(emit_sbb16c)
#define emit_neg32          STEPNAME(emit_neg32)
#define emit_neg16          STEPNAME(emit_neg16)
#define emit_neg8           STEPNAME(emit_neg8)
#define emit_shl8c          STEPNAME(emit_shl8c)
#define emit_shr8c          STEPNAME(emit_shr8c)
#define emit_sar8c          STEPNAME(emit_sar8c)
#define emit_shl8           STEPNAME(emit_shl8)
#define emit_shr8           STEPNAME(emit_shr8)
#define emit_sar8           STEPNAME(emit_sar8)
#define emit_shl16c         STEPNAME(emit_shl16c)
#define emit_shr16c         STEPNAME(emit_shr16c)
#define emit_sar16c         STEPNAME(emit_sar16c)
#define emit_shl16          STEPNAME(emit_shl16)
#define emit_shr16          STEPNAME(emit_shr16)
#define emit_sar16          STEPNAME(emit_sar16)
#define emit_shl32          STEPNAME(emit_shl32)
#define emit_shl32c         STEPNAME(emit_shl32c)
#define emit_shr32          STEPNAME(emit_shr32)
#define emit_shr32c         STEPNAME(emit_shr32c)
#define emit_sar32c         STEPNAME(emit_sar32c)
#define emit_rol32          STEPNAME(emit_rol32)
#define emit_ror32          STEPNAME(emit_ror32)
#define emit_rol32c         STEPNAME(emit_rol32c)
#define emit_ror32c         STEPNAME(emit_ror32c)
#define emit_shrd32c        STEPNAME(emit_shrd32c)
#define emit_shld32c        STEPNAME(emit_shld32c)
#define emit_shrd32         STEPNAME(emit_shld32)
#define emit_shld32         STEPNAME(emit_shld32)
#define emit_shld16c        STEPNAME(emit_shld16c)
#define emit_shrd16c        STEPNAME(emit_shrd16c)

#define emit_pf STEPNAME(emit_pf)

#define x87_do_push           STEPNAME(x87_do_push)
#define x87_do_push_empty     STEPNAME(x87_do_push_empty)
#define x87_do_pop            STEPNAME(x87_do_pop)
#define x87_get_current_cache STEPNAME(x87_get_current_cache)
#define x87_get_cache         STEPNAME(x87_get_cache)
#define x87_get_extcache      STEPNAME(x87_get_extcache)
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
#define sse_setround          STEPNAME(sse_setround)
#define mmx_get_reg           STEPNAME(mmx_get_reg)
#define mmx_get_reg_empty     STEPNAME(mmx_get_reg_empty)
#define mmx_forget_reg        STEPNAME(mmx_forget_reg)
#define sse_get_reg           STEPNAME(sse_get_reg)
#define sse_get_reg_empty     STEPNAME(sse_get_reg_empty)
#define sse_forget_reg        STEPNAME(sse_forget_reg)
#define sse_purge07cache      STEPNAME(sse_purge07cache)
#define sse_reflect_reg       STEPNAME(sse_reflect_reg)

#define fpu_pushcache       STEPNAME(fpu_pushcache)
#define fpu_popcache        STEPNAME(fpu_popcache)
#define fpu_reset_cache     STEPNAME(fpu_reset_cache)
#define fpu_propagate_stack STEPNAME(fpu_propagate_stack)
#define fpu_purgecache      STEPNAME(fpu_purgecache)
#define mmx_purgecache      STEPNAME(mmx_purgecache)
#define x87_purgecache      STEPNAME(x87_purgecache)
#define sse_purgecache      STEPNAME(sse_purgecache)
#define fpu_reflectcache    STEPNAME(fpu_reflectcache)
#define fpu_unreflectcache  STEPNAME(fpu_unreflectcache)

#define CacheTransform STEPNAME(CacheTransform)
#define rv64_move64    STEPNAME(rv64_move64)
#define rv64_move32    STEPNAME(rv64_move32)

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta);

/* setup r2 to address pointed by */
uintptr_t geted32(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta);

/* setup r2 to address pointed by */
// uintptr_t geted16(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int absmax, uint32_t mask, int s);


// generic x64 helper
void jump_to_epilog(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_epilog_fast(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_next(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst, int is32bits);
void ret_to_epilog(dynarec_rv64_t* dyn, int ninst, rex_t rex);
void retn_to_epilog(dynarec_rv64_t* dyn, int ninst, rex_t rex, int n);
void iret_to_epilog(dynarec_rv64_t* dyn, int ninst, int is64bits);
void call_c(dynarec_rv64_t* dyn, int ninst, void* fnc, int reg, int ret, int saveflags, int save_reg);
void call_n(dynarec_rv64_t* dyn, int ninst, void* fnc, int w);
void grab_segdata(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, int reg, int segment);
void emit_cmp8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_cmp16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_cmp32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_cmp8_0(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp16_0(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp32_0(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
void emit_test8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_test16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_test32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_test32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5);
void emit_add32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_add32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5);
void emit_add8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_add8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4);
void emit_sub32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_sub32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5);
void emit_sub8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sub8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4, int s5);
void emit_or32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_or32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_xor32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_xor32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_and32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_and32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_or8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_or8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4);
void emit_xor8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_xor8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_and8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_and8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_add16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
// void emit_add16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sub16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
// void emit_sub16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_or16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
// void emit_or16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_xor16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
// void emit_xor16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_and16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
// void emit_and16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_inc32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_inc16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_inc8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_dec32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_dec16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_dec8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_adc32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6);
// void emit_adc32c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_adc8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_adc8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5, int s6);
void emit_adc16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
// void emit_adc16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sbb32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
// void emit_sbb32c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sbb8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sbb8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5, int s6);
void emit_sbb16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
// void emit_sbb16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_neg32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3);
void emit_neg16(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
void emit_neg8(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
void emit_shl8c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_shr8c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_sar8c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_shl8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shr8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sar8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shl16c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_shr16c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_sar16c(dynarec_rv64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_shl16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shr16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sar16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shl32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_shl32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5);
void emit_shr32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_shr32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_sar32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_rol32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_ror32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_rol32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_ror32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_shrd32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);
void emit_shld32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);
void emit_shrd32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6);
void emit_shld32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6);
void emit_shrd16c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);
void emit_shld16c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4, int s5);

void emit_pf(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);

// x87 helper
// cache of the local stack counter, to avoid upadte at every call
int x87_stackcount(dynarec_rv64_t* dyn, int ninst, int scratch);
// restore local stack counter
void x87_unstackcount(dynarec_rv64_t* dyn, int ninst, int scratch, int count);
// fpu push. Return the Dd value to be used
int x87_do_push(dynarec_rv64_t* dyn, int ninst, int s1, int t);
// fpu push. Do not allocate a cache register. Needs a scratch register to do x87stack synch (or 0 to not do it)
void x87_do_push_empty(dynarec_rv64_t* dyn, int ninst, int s1);
// fpu pop. All previous returned Dd should be considered invalid
void x87_do_pop(dynarec_rv64_t* dyn, int ninst, int s1);
// get cache index for a x87 reg, return -1 if cache doesn't exist
int x87_get_current_cache(dynarec_rv64_t* dyn, int ninst, int st, int t);
// get cache index for a x87 reg, create the entry if needed
int x87_get_cache(dynarec_rv64_t* dyn, int ninst, int populate, int s1, int s2, int a, int t);
// get extcache index for a x87 reg
int x87_get_extcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int a);
// get vfpu register for a x87 reg, create the entry if needed
int x87_get_st(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int a, int t);
// get vfpu register for a x87 reg, create the entry if needed. Do not fetch the Stx if not already in cache
int x87_get_st_empty(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int a, int t);
// Free st, using the FFREE opcode (so it's freed but stack is not moved)
void x87_free(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int st);
// refresh a value from the cache ->emu (nothing done if value is not cached)
void x87_refresh(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st);
// refresh a value from the cache ->emu and then forget the cache (nothing done if value is not cached)
void x87_forget(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st);
// refresh the cache value from emu
void x87_reget_st(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st);
// swap 2 x87 regs
void x87_swapreg(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int a, int b);
// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_rv64_t* dyn, int ninst, int s1, int s2);
// Restore round flag
void x87_restoreround(dynarec_rv64_t* dyn, int ninst, int s1);
// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_rv64_t* dyn, int ninst, int s1, int s2);

void CacheTransform(dynarec_rv64_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3);

void rv64_move64(dynarec_rv64_t* dyn, int ninst, int reg, int64_t val);
void rv64_move32(dynarec_rv64_t* dyn, int ninst, int reg, int32_t val, int zeroup);

#if STEP < 2
#define CHECK_CACHE() 0
#else
#define CHECK_CACHE() (cacheupd = CacheNeedsTransform(dyn, ninst))
#endif
#define extcache_st_coherency STEPNAME(extcache_st_coherency)
int extcache_st_coherency(dynarec_rv64_t* dyn, int ninst, int a, int b);

#if STEP == 0
#define ST_IS_F(A)        0
#define ST_IS_I64(A)      0
#define X87_COMBINE(A, B) EXT_CACHE_ST_D
#define X87_ST0           EXT_CACHE_ST_D
#define X87_ST(A)         EXT_CACHE_ST_D
#elif STEP == 1
#define ST_IS_F(A)        (extcache_get_current_st(dyn, ninst, A) == EXT_CACHE_ST_F)
#define ST_IS_I64(A)      (extcache_get_current_st(dyn, ninst, A) == EXT_CACHE_ST_I64)
#define X87_COMBINE(A, B) extcache_combine_st(dyn, ninst, A, B)
#define X87_ST0           extcache_no_i64(dyn, ninst, 0, extcache_get_current_st(dyn, ninst, 0))
#define X87_ST(A)         extcache_no_i64(dyn, ninst, A, extcache_get_current_st(dyn, ninst, A))
#else
#define ST_IS_F(A) (extcache_get_st(dyn, ninst, A) == EXT_CACHE_ST_F)
#define ST_IS_I64(A) (extcache_get_st(dyn, ninst, A) == EXT_CACHE_ST_I64)
#if STEP == 3
#define X87_COMBINE(A, B) extcache_st_coherency(dyn, ninst, A, B)
#else
#define X87_COMBINE(A, B) extcache_get_st(dyn, ninst, A)
#endif
#define X87_ST0   extcache_get_st(dyn, ninst, 0)
#define X87_ST(A) extcache_get_st(dyn, ninst, A)
#endif

// MMX helpers
//  get float register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int a);
// get float register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int a);
// forget float register for a MMX reg, create the entry if needed
void mmx_forget_reg(dynarec_rv64_t* dyn, int ninst, int a);

// SSE/SSE2 helpers
//  get float register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a, int single);
// get float register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_rv64_t* dyn, int ninst, int s1, int a, int single);
// forget float register for a SSE reg, create the entry if needed
void sse_forget_reg(dynarec_rv64_t* dyn, int ninst, int a);
// purge the XMM0..XMM7 cache (before function call)
void sse_purge07cache(dynarec_rv64_t* dyn, int ninst, int s1);
// Push current value to the cache
void sse_reflect_reg(dynarec_rv64_t* dyn, int ninst, int a);

// common coproc helpers
// reset the cache with n
void fpu_reset_cache(dynarec_rv64_t* dyn, int ninst, int reset_n);
// propagate stack state
void fpu_propagate_stack(dynarec_rv64_t* dyn, int ninst);
// purge the FPU cache (needs 3 scratch registers)
void fpu_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1, int s2, int s3);
// purge MMX cache
void mmx_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1);
// purge x87 cache
void x87_purgecache(dynarec_rv64_t* dyn, int ninst, int next, int s1, int s2, int s3);
void fpu_reflectcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3);
void fpu_unreflectcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3);
void fpu_pushcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07);
void fpu_popcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07);


uintptr_t dynarec64_00(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_00_0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_00_1(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_00_2(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_00_3(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_64(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog);
// uintptr_t dynarec64_65(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep,int* ok, int* need_epilog);
uintptr_t dynarec64_66(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_67(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_67_32(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_D8(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_D9(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DA(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DB(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DC(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DD(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DE(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DF(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_F0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_660F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_66F20F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_66F30F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_6664(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int seg, int* ok, int* need_epilog);
uintptr_t dynarec64_66F0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_F20F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F30F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);

#if STEP < 2
#define PASS2(A)
#else
#define PASS2(A) A
#endif

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

// GOCOND will use x1 and x3
#define GOCOND(B, T1, T2)                                                           \
    case B + 0x0:                                                                   \
        INST_NAME(T1 "O " T2);                                                      \
        GO(ANDI(x1, xFlags, 1 << F_OF2), EQZ, NEZ, X_OF)                            \
        break;                                                                      \
    case B + 0x1:                                                                   \
        INST_NAME(T1 "NO " T2);                                                     \
        GO(ANDI(x1, xFlags, 1 << F_OF2), NEZ, EQZ, X_OF)                            \
        break;                                                                      \
    case B + 0x2:                                                                   \
        INST_NAME(T1 "C " T2);                                                      \
        GO(ANDI(x1, xFlags, 1 << F_CF), EQZ, NEZ, X_CF)                             \
        break;                                                                      \
    case B + 0x3:                                                                   \
        INST_NAME(T1 "NC " T2);                                                     \
        GO(ANDI(x1, xFlags, 1 << F_CF), NEZ, EQZ, X_CF)                             \
        break;                                                                      \
    case B + 0x4:                                                                   \
        INST_NAME(T1 "Z " T2);                                                      \
        GO(ANDI(x1, xFlags, 1 << F_ZF), EQZ, NEZ, X_ZF)                             \
        break;                                                                      \
    case B + 0x5:                                                                   \
        INST_NAME(T1 "NZ " T2);                                                     \
        GO(ANDI(x1, xFlags, 1 << F_ZF), NEZ, EQZ, X_ZF)                             \
        break;                                                                      \
    case B + 0x6:                                                                   \
        INST_NAME(T1 "BE " T2);                                                     \
        GO(ANDI(x1, xFlags, (1 << F_CF) | (1 << F_ZF)), EQZ, NEZ, X_CF | X_ZF)      \
        break;                                                                      \
    case B + 0x7:                                                                   \
        INST_NAME(T1 "NBE " T2);                                                    \
        GO(ANDI(x1, xFlags, (1 << F_CF) | (1 << F_ZF)), NEZ, EQZ, X_CF | X_ZF)      \
        break;                                                                      \
    case B + 0x8:                                                                   \
        INST_NAME(T1 "S " T2);                                                      \
        GO(ANDI(x1, xFlags, 1 << F_SF), EQZ, NEZ, X_SF)                             \
        break;                                                                      \
    case B + 0x9:                                                                   \
        INST_NAME(T1 "NS " T2);                                                     \
        GO(ANDI(x1, xFlags, 1 << F_SF), NEZ, EQZ, X_SF)                             \
        break;                                                                      \
    case B + 0xA:                                                                   \
        INST_NAME(T1 "P " T2);                                                      \
        GO(ANDI(x1, xFlags, 1 << F_PF), EQZ, NEZ, X_PF)                             \
        break;                                                                      \
    case B + 0xB:                                                                   \
        INST_NAME(T1 "NP " T2);                                                     \
        GO(ANDI(x1, xFlags, 1 << F_PF), NEZ, EQZ, X_PF)                             \
        break;                                                                      \
    case B + 0xC:                                                                   \
        INST_NAME(T1 "L " T2);                                                      \
        GO(SRLI(x1, xFlags, F_SF - F_OF2);                                          \
            XOR(x1, x1, xFlags);                                                    \
            ANDI(x1, x1, 1 << F_OF2), EQZ, NEZ, X_SF | X_OF)                        \
        break;                                                                      \
    case B + 0xD:                                                                   \
        INST_NAME(T1 "GE " T2);                                                     \
        GO(SRLI(x1, xFlags, F_SF - F_OF2);                                          \
            XOR(x1, x1, xFlags);                                                    \
            ANDI(x1, x1, 1 << F_OF2), NEZ, EQZ, X_SF | X_OF)                        \
        break;                                                                      \
    case B + 0xE:                                                                   \
        INST_NAME(T1 "LE " T2);                                                     \
        GO(SRLI(x1, xFlags, F_SF - F_OF2);                                          \
            XOR(x1, x1, xFlags);                                                    \
            ANDI(x1, x1, 1 << F_OF2);                                               \
            ANDI(x3, xFlags, 1 << F_ZF);                                            \
            OR(x1, x1, x3);                                                         \
            ANDI(x1, x1, (1 << F_OF2) | (1 << F_ZF)), EQZ, NEZ, X_SF | X_OF | X_ZF) \
        break;                                                                      \
    case B + 0xF:                                                                   \
        INST_NAME(T1 "G " T2);                                                      \
        GO(SRLI(x1, xFlags, F_SF - F_OF2);                                          \
            XOR(x1, x1, xFlags);                                                    \
            ANDI(x1, x1, 1 << F_OF2);                                               \
            ANDI(x3, xFlags, 1 << F_ZF);                                            \
            OR(x1, x1, x3);                                                         \
            ANDI(x1, x1, (1 << F_OF2) | (1 << F_ZF)), NEZ, EQZ, X_SF | X_OF | X_ZF) \
        break

#define NOTEST(s1)                                     \
    if (box64_dynarec_test) {                          \
        SW(xZR, xEmu, offsetof(x64emu_t, test.test));  \
        SW(xZR, xEmu, offsetof(x64emu_t, test.clean)); \
    }
#define SKIPTEST(s1)                                   \
    if (box64_dynarec_test) {                          \
        SW(xZR, xEmu, offsetof(x64emu_t, test.clean)); \
    }
#define GOTEST(s1, s2)                               \
    if (box64_dynarec_test) {                        \
        MOV32w(s2, 1);                               \
        SW(s2, xEmu, offsetof(x64emu_t, test.test)); \
    }

#define GETREX()                                   \
    rex.rex = 0;                                   \
    if (!rex.is32bits)                             \
        while (opcode >= 0x40 && opcode <= 0x4f) { \
            rex.rex = opcode;                      \
            opcode = F8;                           \
        }


#define FCOM(w, v1, v2, s1, s2, s3, s4, s5)                    \
    LHU(s3, xEmu, offsetof(x64emu_t, sw));                     \
    MOV32w(s1, 0b1011100011111111); /* mask off c0,c1,c2,c3 */ \
    AND(s3, s3, s1);                                           \
    FEQ##w(s5, v1, v1);                                        \
    FEQ##w(s4, v2, v2);                                        \
    AND(s5, s5, s4);                                           \
    BEQZ(s5, 24); /* undefined/NaN */                          \
    FEQ##w(s5, v1, v2);                                        \
    BNEZ(s5, 28);       /* equal */                            \
    FLT##w(s2, v1, v2); /* x2 = (v1<v2)?1:0 */                 \
    SLLI(s1, s2, 8);                                           \
    J(20); /* end */                                           \
    /* undefined/NaN */                                        \
    LUI(s1, 4);                                                \
    ADDI(s1, s1, 0b010100000000);                              \
    J(8); /* end */                                            \
    /* equal */                                                \
    LUI(s1, 4);                                                \
    /* end */                                                  \
    OR(s3, s3, s1);                                            \
    SH(s3, xEmu, offsetof(x64emu_t, sw));

#define FCOMS(v1, v2, s1, s2, s3, s4, s5) FCOM(S, v1, v2, s1, s2, s3, s4, s5)
#define FCOMD(v1, v2, s1, s2, s3, s4, s5) FCOM(D, v1, v2, s1, s2, s3, s4, s5)

#define FCOMI(w, v1, v2, s1, s2, s3, s4, s5)                    \
    IFX(X_OF|X_AF|X_SF|X_PEND) {                                \
        MOV64x(s2, ~((1<<F_OF2)|(1<<F_AF)|(1<<F_SF)));          \
        AND(xFlags, xFlags, s2);                                \
    }                                                           \
    IFX(X_CF|X_PF|X_ZF|X_PEND) {                                \
        MOV32w(s2, 0b01000101);                                 \
        if(rv64_zbb) {                                          \
            ANDN(xFlags, xFlags, s2);                           \
        } else {                                                \
            NOT(s3, s2);                                        \
            AND(xFlags, xFlags, s3);                            \
        }                                                       \
        FEQ##w(s5, v1, v1);                                     \
        FEQ##w(s4, v2, v2);                                     \
        AND(s5, s5, s4);                                        \
        BEQZ(s5, 5*4); /* undefined/NaN */                      \
        FEQ##w(s5, v1, v2);                                     \
        BNEZ(s5, 5*4);       /* equal */                        \
        FLT##w(s1, v1, v2); /* s1 = (v1<v2)?1:0 */              \
        J(4*4); /* end */                                       \
        /* undefined/NaN */                                     \
        MV(s1, s2);                                             \
        J(2*4); /* end */                                       \
        /* equal */                                             \
        ADDI(s1, xZR, 0b01000000);                              \
        /* end */                                               \
        OR(xFlags, xFlags, s1);                                 \
    }                                                           \
    SET_DFNONE()

#define FCOMIS(v1, v2, s1, s2, s3, s4, s5) FCOMI(S, v1, v2, s1, s2, s3, s4, s5)
#define FCOMID(v1, v2, s1, s2, s3, s4, s5) FCOMI(D, v1, v2, s1, s2, s3, s4, s5)

// reg = (reg < -32768) ? -32768 : ((reg > 32767) ? 32767 : reg)
#define SAT16(reg, s)             \
    LUI(s, 0xFFFF8); /* -32768 */ \
    BGE(reg, s, 4 + 2 * 4);       \
    MV(reg, s);                   \
    J(4 + 4 * 3);                 \
    LUI(s, 8); /* 32768 */        \
    BLT(reg, s, 4 + 4);           \
    ADDIW(reg, s, -1);

#endif //__DYNAREC_RV64_HELPER_H__
