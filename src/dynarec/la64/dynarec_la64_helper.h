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
#include "dynarec_la64_consts.h"

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

// GETGD    get x64 register in gd
#define GETGD gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3));
// GETVD    get x64 register in vd
#define GETVD vd = TO_NAT(vex.v)

// GETGW extract x64 register in gd, that is i
#define GETGW(i)                                        \
    gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); \
    BSTRPICK_D(i, gd, 15, 0);                           \
    gd = i;

// GETED can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETED(D)                                                                                \
    if (MODREG) {                                                                               \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                               \
        wback = 0;                                                                              \
    } else {                                                                                    \
        SMREAD();                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        LDxw(x1, wback, fixedaddress);                                                          \
        ed = x1;                                                                                \
    }

#define GETEDz(D)                                                                               \
    if (MODREG) {                                                                               \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                               \
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
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                 \
        wback = 0;                                                                                \
    } else {                                                                                      \
        SMREAD();                                                                                 \
        addr = geted32(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        LDxw(x1, wback, fixedaddress);                                                            \
        ed = x1;                                                                                  \
    }
// GETEDH can use hint for wback and ret for ed. wback is 0 if ed is xEAX..xEDI
#define GETEDH(hint, ret, D)                                                                       \
    if (MODREG) {                                                                                  \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                  \
        wback = 0;                                                                                 \
    } else {                                                                                       \
        SMREAD();                                                                                  \
        addr = geted(dyn, addr, ninst, nextop, &wback, hint, ret, &fixedaddress, rex, NULL, 1, D); \
        ed = ret;                                                                                  \
        LDxw(ed, wback, fixedaddress);                                                             \
    }
#define GETEDW(hint, ret, D)                                                                       \
    if (MODREG) {                                                                                  \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                  \
        MV(ret, ed);                                                                               \
        wback = 0;                                                                                 \
    } else {                                                                                       \
        SMREAD();                                                                                  \
        addr = geted(dyn, addr, ninst, nextop, &wback, hint, ret, &fixedaddress, rex, NULL, 0, D); \
        ed = ret;                                                                                  \
        LDxw(ed, wback, fixedaddress);                                                             \
    }
// GETEWW will use i for ed, and can use w for wback.
#define GETEWW(w, i, D)                                                                       \
    if (MODREG) {                                                                             \
        wback = TO_NAT((nextop & 7) + (rex.b << 3));                                          \
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
#define GETEDO(O, D)                                                                                \
    if (MODREG) {                                                                                   \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                   \
        wback = 0;                                                                                  \
    } else {                                                                                        \
        SMREAD();                                                                                   \
        if (rex.is32bits) {                                                                         \
            addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
            ADDz(O, wback, O);                                                                      \
            LD_WU(x1, O, fixedaddress);                                                             \
        } else {                                                                                    \
            addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 0, D); \
            LDXxw(x1, wback, O);                                                                    \
        }                                                                                           \
        ed = x1;                                                                                    \
    }
// GETEDOz can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETEDOz(O, D, S)                                                                       \
    if (MODREG) {                                                                              \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                              \
        wback = 0;                                                                             \
    } else {                                                                                   \
        SMREAD();                                                                              \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, S, &fixedaddress, rex, NULL, 1, D); \
        ADDz(S, wback, O);                                                                     \
        LDz(x1, S, fixedaddress);                                                              \
        ed = x1;                                                                               \
    }
// GETSED can use r1 for ed, and r2 for wback. ed will be sign extended!
#define GETSED(D)                                                                               \
    if (MODREG) {                                                                               \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                               \
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
#define FAKEED                                    \
    if (MODREG) {                                 \
        ed = TO_NAT((nextop & 7) + (rex.b << 3)); \
        wback = 0;                                \
    } else {                                      \
        addr = fakeed(dyn, addr, ninst, nextop);  \
    }

// GETGW extract x64 register in gd, that is i, Signed extented
#define GETSGW(i)                                       \
    gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); \
    EXT_W_H(i, gd);                                     \
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

#define WBACKO(O)                \
    if (wback) {                 \
        if (rex.is32bits) {      \
            ADDz(O, wback, O);   \
            ST_W(ed, O, 0);      \
        } else {                 \
            SDXxw(ed, wback, O); \
        }                        \
        SMWRITE2();              \
    }

// GETSEW will use i for ed, and can use r3 for wback. This is the Signed version
#define GETSEW(i, D)                                                                           \
    if (MODREG) {                                                                              \
        wback = TO_NAT((nextop & 7) + (rex.b << 3));                                           \
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
#define GWBACK BSTRINS_D((TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3))), gd, 15, 0);

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
            wback = TO_NAT((nextop & 7) + (rex.b << 3));                                        \
            wb2 = 0;                                                                            \
        } else {                                                                                \
            wback = (nextop & 7);                                                               \
            wb2 = (wback >> 2) * 8;                                                             \
            wback = TO_NAT(wback & 3);                                                          \
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
        addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 1, D); \
        LD_B(i, wback, fixedaddress);                                                           \
        wb1 = 1;                                                                                \
        ed = i;                                                                                 \
    }
// GETEB32 will use i for ed, and can use r3 for wback.
#define GETEB32(i, D)                                                                             \
    if (MODREG) {                                                                                 \
        if (rex.rex) {                                                                            \
            wback = TO_NAT((nextop & 7) + (rex.b << 3));                                          \
            wb2 = 0;                                                                              \
        } else {                                                                                  \
            wback = (nextop & 7);                                                                 \
            wb2 = (wback >> 2) * 8;                                                               \
            wback = TO_NAT(wback & 3);                                                            \
        }                                                                                         \
        BSTRPICK_D(i, wback, wb2 + 7, wb2);                                                       \
        wb1 = 0;                                                                                  \
        ed = i;                                                                                   \
    } else {                                                                                      \
        SMREAD();                                                                                 \
        addr = geted32(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 1, D); \
        LD_BU(i, wback, fixedaddress);                                                            \
        wb1 = 1;                                                                                  \
        ed = i;                                                                                   \
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
    BSTRPICK_D(gd, gb1, gb2 + 7, gb2);

// GETEBO will use i for ed, i is also Offset, and can use r3 for wback.
#define GETEBO(i, D)                                                                                \
    if (MODREG) {                                                                                   \
        if (rex.rex) {                                                                              \
            wback = TO_NAT((nextop & 7) + (rex.b << 3));                                            \
            wb2 = 0;                                                                                \
        } else {                                                                                    \
            wback = (nextop & 7);                                                                   \
            wb2 = (wback >> 2) * 8;                                                                 \
            wback = TO_NAT(wback & 3);                                                              \
        }                                                                                           \
        BSTRPICK_D(i, wback, wb2 + 7, wb2);                                                         \
        wb1 = 0;                                                                                    \
        ed = i;                                                                                     \
    } else {                                                                                        \
        SMREAD();                                                                                   \
        if (rex.is32bits) {                                                                         \
            addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 1, D); \
            ADDz(x3, wback, i);                                                                     \
            if (wback != x3) wback = x3;                                                            \
            LD_BU(i, wback, fixedaddress);                                                          \
        } else {                                                                                    \
            addr = geted(dyn, addr, ninst, nextop, &wback, x3, x2, &fixedaddress, rex, NULL, 0, D); \
            LDX_BU(i, wback, i);                                                                    \
        }                                                                                           \
        wb1 = 1;                                                                                    \
        ed = i;                                                                                     \
    }

#define VEXTRINS_IMM_4_0(n, m) ((n & 0xf) << 4 | (m & 0xf))
#define XVPERMI_IMM_4_0(n, m)  ((n & 0xf) << 4 | (m & 0xf))

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

// Get Ex as 64bits, not a quad (warning, x1 get used, x2 might too)
#define GETEX64(a, w, D) GETEXSD(a, w, D)

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

// Get Ex as 32bits, not a quad (warning, x1 get used)
#define GETEX32(a, w, D) GETEXSS(a, w, D)

// Get Ex as 16bits, not a quad (warning, x1 get used)
#define GETEX16(a, w, D)                                                                     \
    if (MODREG) {                                                                            \
        a = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), w);                     \
    } else {                                                                                 \
        SMREAD();                                                                            \
        a = fpu_get_scratch(dyn);                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, D); \
        LD_HU(x2, ed, fixedaddress);                                                         \
        MOVGR2FR_D(a, x2);                                                                   \
    }

// Get GM, might use x1, x2 and x3
#define GETGM(a)                 \
    gd = ((nextop & 0x38) >> 3); \
    a = mmx_get_reg(dyn, ninst, x1, x2, x3, gd)

// Get EM, might use x1, x2 and x3
#define GETEM(a, D)                                                                          \
    if (MODREG) {                                                                            \
        a = mmx_get_reg(dyn, ninst, x1, x2, x3, (nextop & 7));                               \
    } else {                                                                                 \
        SMREAD();                                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 1, D); \
        a = fpu_get_scratch(dyn);                                                            \
        FLD_D(a, ed, fixedaddress);                                                          \
    }

// Put Back Em if it was a memory and not an emm register
#define PUTEM(a)                    \
    if (!MODREG) {                  \
        FST_D(a, ed, fixedaddress); \
        SMWRITE2();                 \
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

#define YMM_UNMARK_UPPER_ZERO(a)             \
    do {                                     \
        dyn->lsx.avxcache[a].zero_upper = 0; \
    } while (0)

// AVX helpers
// Get VX (might use x1)
#define GETVYx(a, w) \
    a = avx_get_reg(dyn, ninst, x1, vex.v, w, LSX_AVX_WIDTH_128)

#define GETVYy(a, w) \
    a = avx_get_reg(dyn, ninst, x1, vex.v, w, LSX_AVX_WIDTH_256)

// Get an empty VX (use x1)
#define GETVYx_empty(a) \
    a = avx_get_reg_empty(dyn, ninst, x1, vex.v, LSX_AVX_WIDTH_128)

#define GETVYy_empty(a) \
    a = avx_get_reg_empty(dyn, ninst, x1, vex.v, LSX_AVX_WIDTH_256)

// Get GX as a quad (might use x1)
#define GETGYx(a, w)                            \
    gd = ((nextop & 0x38) >> 3) + (rex.r << 3); \
    a = avx_get_reg(dyn, ninst, x1, gd, w, LSX_AVX_WIDTH_128)

#define GETGYy(a, w)                            \
    gd = ((nextop & 0x38) >> 3) + (rex.r << 3); \
    a = avx_get_reg(dyn, ninst, x1, gd, w, LSX_AVX_WIDTH_256)

#define GETGYx_empty(a)                         \
    gd = ((nextop & 0x38) >> 3) + (rex.r << 3); \
    a = avx_get_reg_empty(dyn, ninst, x1, gd, LSX_AVX_WIDTH_128)

#define GETGYy_empty(a)                         \
    gd = ((nextop & 0x38) >> 3) + (rex.r << 3); \
    a = avx_get_reg_empty(dyn, ninst, x1, gd, LSX_AVX_WIDTH_256)

// Get EY as a quad, (x1 is used)
#define GETEYx(a, w, D)                                                                      \
    if (MODREG) {                                                                            \
        a = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), w, LSX_AVX_WIDTH_128);  \
    } else {                                                                                 \
        SMREAD();                                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        a = fpu_get_scratch(dyn);                                                            \
        VLD(a, ed, fixedaddress);                                                            \
    }

#define GETEYy(a, w, D)                                                                      \
    if (MODREG) {                                                                            \
        a = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), w, LSX_AVX_WIDTH_256);  \
    } else {                                                                                 \
        SMREAD();                                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        a = fpu_get_scratch(dyn);                                                            \
        XVLD(a, ed, fixedaddress);                                                           \
    }

#define GETEYx_empty(a, D)                                                                     \
    if (MODREG) {                                                                              \
        a = avx_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), LSX_AVX_WIDTH_128); \
    } else {                                                                                   \
        SMREAD();                                                                              \
        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, D);   \
        a = fpu_get_scratch(dyn);                                                              \
    }

#define GETEYy_empty(a, D)                                                                     \
    if (MODREG) {                                                                              \
        a = avx_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), LSX_AVX_WIDTH_256); \
    } else {                                                                                   \
        SMREAD();                                                                              \
        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, D);   \
        a = fpu_get_scratch(dyn);                                                              \
    }

// Get EY as 32bits , (x1 is used)
#define GETEYSS(a, w, D)                                                                     \
    if (MODREG) {                                                                            \
        a = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), w, LSX_AVX_WIDTH_128);  \
    } else {                                                                                 \
        SMREAD();                                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        a = fpu_get_scratch(dyn);                                                            \
        FLD_S(a, ed, fixedaddress);                                                          \
    }

#define PUTEYSS(a)                  \
    if (!MODREG) {                  \
        FST_S(a, ed, fixedaddress); \
        SMWRITE2();                 \
    }

// Get EY as 32bits , (x1 is used)
#define GETEYSD(a, w, D)                                                                     \
    if (MODREG) {                                                                            \
        a = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), w, LSX_AVX_WIDTH_128);  \
    } else {                                                                                 \
        SMREAD();                                                                            \
        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, D); \
        a = fpu_get_scratch(dyn);                                                            \
        FLD_D(a, ed, fixedaddress);                                                          \
    }

#define PUTEYSD(a)                  \
    if (!MODREG) {                  \
        FST_D(a, ed, fixedaddress); \
        SMWRITE2();                 \
    }

#define GETGYxy(a, w) \
    if (vex.l) {      \
        GETGYy(a, w); \
    } else {          \
        GETGYx(a, w); \
    }

#define GETGYxy_empty(a) \
    if (vex.l) {         \
        GETGYy_empty(a); \
    } else {             \
        GETGYx_empty(a); \
    }

#define GETVYxy(a, w) \
    if (vex.l) {      \
        GETVYy(a, w); \
    } else {          \
        GETVYx(a, w); \
    }

#define GETVYxy_empty(a) \
    if (vex.l) {         \
        GETVYy_empty(a); \
    } else {             \
        GETVYx_empty(a); \
    }

#define GETEYxy(a, w, D) \
    if (vex.l) {         \
        GETEYy(a, w, D); \
    } else {             \
        GETEYx(a, w, D); \
    }

#define GETEYxy_empty(a, D) \
    if (vex.l) {            \
        GETEYy_empty(a, D); \
    } else {                \
        GETEYx_empty(a, D); \
    }

// Put Back EY if it was a memory and not an emm register
#define PUTEYy(a)                  \
    if (!MODREG) {                 \
        XVST(a, ed, fixedaddress); \
        SMWRITE2();                \
    }

#define PUTEYx(a)                 \
    if (!MODREG) {                \
        VST(a, ed, fixedaddress); \
        SMWRITE2();               \
    }

#define PUTEYxy(a) \
    if (vex.l) {   \
        PUTEYy(a); \
    } else {       \
        PUTEYx(a); \
    }

// Get empty GY, and non-written VY and EY
#define GETGY_empty_VYEY_xy(gx, vx, ex, D) \
    GETVYxy(vx, 0);                        \
    GETEYxy(ex, 0, D);                     \
    GETGYxy_empty(gx);

// Get empty GY, and non-written EY
#define GETGY_empty_EY_xy(gx, ex, D) \
    GETEYxy(ex, 0, D);               \
    GETGYxy_empty(gx);

// Get writable GY, and non-written VY and EY
#define GETGY_VYEY_xy(gx, vx, ex, D) \
    GETVYxy(vx, 0);                  \
    GETEYxy(ex, 0, D);               \
    GETGYxy(gx, 1);

// Get writable GY, and non-written EY
#define GETGY_EY_xy(gx, ex, D) \
    GETEYxy(ex, 0, D);         \
    GETGYxy(gx, 1);

// Get writable EY, and non-written VY and GY
#define GETEY_VYGY_xy(ex, vx, gx, D) \
    GETVYxy(vx, 0);                  \
    GETGYxy(gx, 0);                  \
    GETEYxy(ex, 1, D);

// Get writable EY, and non-written GY
#define GETEY_GY_xy(ex, gx, D) \
    GETGYxy(gx, 0);            \
    GETEYxy(ex, 1, D);

// Get writable GYx, and non-written VYx, EYSD or EYSS , for FMA SD/SSinsts.
#define GETGYx_VYx_EYxw(gx, vx, ex, D) \
    GETVYx(vx, 0);                     \
    if (rex.w) {                       \
        GETEYSD(ex, 0, D)              \
    } else {                           \
        GETEYSS(ex, 0, D);             \
    }                                  \
    GETGYx(gx, 1);

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
#define CALL(F, ret, arg1, arg2)                          call_c(dyn, ninst, F, x6, ret, 1, 0, arg1, arg2, 0, 0, 0, 0)
#define CALL4(F, ret, arg1, arg2, arg3, arg4)             call_c(dyn, ninst, F, x6, ret, 1, 0, arg1, arg2, arg3, arg4, 0, 0)
#define CALL6(F, ret, arg1, arg2, arg3, arg4, arg5, arg6) call_c(dyn, ninst, F, x6, ret, 1, 0, arg1, arg2, arg3, arg4, arg5, arg6)
// CALL_ will use x6 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL_(F, ret, reg, arg1, arg2)              call_c(dyn, ninst, F, x6, ret, 1, reg, arg1, arg2, 0, 0, 0, 0)
#define CALL4_(F, ret, reg, arg1, arg2, arg3, arg4) call_c(dyn, ninst, F, x6, ret, 1, reg, arg1, arg2, arg3, arg4, 0, 0)
// CALL_S will use x6 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2. Flags are not save/restored
#define CALL_S(F, ret, arg1) call_c(dyn, ninst, F, x6, ret, 0, 0, arg1, 0, 0, 0, 0, 0)

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

// Branch to MARK if fcc!=0 (use j64)
#define BCNEZ_MARK(fcc) BCxxZ_gen(NE, MARK, fcc)
// Branch to MARK2 if fcc!=0 (use j64)
#define BCNEZ_MARK2(fcc) BCxxZ_gen(NE, MARK2, fcc)
// Branch to MARK3 if fcc!=0 (use j64)
#define BCNEZ_MARK3(fcc) BCxxZ_gen(NE, MARK3, fcc)
// Branch to MARKLOCK if fcc!=0 (use j64)
#define BCNEZ_MARKLOCK(fcc) BxxZ_gen(NE, MARKLOCK, fcc)
// Branch to MARKLOCK2 if fcc!=0 (use j64)
#define BCNEZ_MARKLOCK2(fcc) BxxZ_gen(NE, MARKLOCK2, fcc)

// Branch to MARK if fcc==0 (use j64)
#define BCEQZ_MARK(fcc) BCxxZ_gen(EQ, MARK, fcc)
// Branch to MARK2 if fcc==0 (use j64)
#define BCEQZ_MARK2(fcc) BCxxZ_gen(EQ, MARK2, fcc)
// Branch to MARK3 if fcc==0 (use j64)
#define BCEQZ_MARK3(fcc) BCxxZ_gen(EQ, MARK3, fcc)
// Branch to MARKLOCK if fcc==0 (use j64)
#define BCEQZ_MARKLOCK(fcc) BxxZ_gen(EQ, MARKLOCK, fcc)
// Branch to MARKLOCK2 if fcc==0 (use j64)
#define BCEQZ_MARKLOCK2(fcc) BxxZ_gen(EQ, MARKLOCK2, fcc)

// Branch to MARK if reg1<reg2 (use j64)
#define BLT_MARK(reg1, reg2) Bxx_gen(LT, MARK, reg1, reg2)
// Branch to MARK if reg1<reg2 (use j64)
#define BLTU_MARK(reg1, reg2) Bxx_gen(LTU, MARK, reg1, reg2)
// Branch to MARK if reg1>=reg2 (use j64)
#define BGE_MARK(reg1, reg2) Bxx_gen(GE, MARK, reg1, reg2)
// Branch to MARK2 if reg1>=0 (use j64)
#define BGE_MARK2(reg, reg2) Bxx_gen(GE, MARK2, reg, reg2)
// Branch to MARK3 if reg1>=0 (use j64)
#define BGE_MARK3(reg, reg2) Bxx_gen(GE, MARK3, reg, reg2)

// Branch to MARK instruction unconditionnal (use j64)
#define B_MARK_nocond Bxx_gen(__, MARK, 0, 0)
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
#define IFXORNAT(A) if ((dyn->insts[ninst].x64.gen_flags & (A)) || dyn->insts[ninst].nat_flags_fusion)
#define IFX_PENDOR0 if ((dyn->insts[ninst].x64.gen_flags & (X_PEND) || (!dyn->insts[ninst].x64.gen_flags && !dyn->insts[ninst].nat_flags_fusion)))
#define IFXX(A)     if ((dyn->insts[ninst].x64.gen_flags == (A)))
#define IFX2X(A, B) if ((dyn->insts[ninst].x64.gen_flags == (A) || dyn->insts[ninst].x64.gen_flags == (B) || dyn->insts[ninst].x64.gen_flags == ((A) | (B))))
#define IFXN(A, B)  if ((dyn->insts[ninst].x64.gen_flags & (A) && !(dyn->insts[ninst].x64.gen_flags & (B))))

#ifndef NATIVE_RESTORE_X87PC
#define NATIVE_RESTORE_X87PC()                     \
    if (dyn->need_x87check) {                      \
        LD_D(x87pc, xEmu, offsetof(x64emu_t, cw)); \
        SRLI_D(x87pc, x87pc, 8);                   \
        ANDI(x87pc, x87pc, 0b11);                  \
    }
#endif
#ifndef X87_CHECK_PRECISION
#define X87_CHECK_PRECISION(A)               \
    if (!ST_IS_F(0) && dyn->need_x87check) { \
        BNEZ(x87pc, 4 + 8);                  \
        FCVT_S_D(A, A);                      \
        FCVT_D_S(A, A);                      \
    }
#endif

#define STORE_REG(A) ST_D(x##A, xEmu, offsetof(x64emu_t, regs[_##A]))
#define LOAD_REG(A)  LD_D(x##A, xEmu, offsetof(x64emu_t, regs[_##A]))

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

#define FORCE_DFNONE() ST_W(xZR, xEmu, offsetof(x64emu_t, df))

#define SET_DFNONE()                          \
    do {                                      \
        if (!dyn->f.dfnone) {                 \
            FORCE_DFNONE();                   \
        }                                     \
        if (!dyn->insts[ninst].x64.may_set) { \
            dyn->f.dfnone = 1;                \
        }                                     \
    } while (0)

#define SET_DF(S, N)                                           \
    if ((N) != d_none) {                                       \
        MOV32w(S, (N));                                        \
        ST_W(S, xEmu, offsetof(x64emu_t, df));                 \
        if (dyn->f.pending == SF_PENDING                       \
            && dyn->insts[ninst].x64.need_after                \
            && !(dyn->insts[ninst].x64.need_after & X_PEND)) { \
            CALL_(const_updateflags, -1, 0, 0, 0);             \
            dyn->f.pending = SF_SET;                           \
            SET_NODF();                                        \
        }                                                      \
        dyn->f.dfnone = 0;                                     \
    } else                                                     \
        SET_DFNONE()

#define SET_NODF() dyn->f.dfnone = 0
#define SET_DFOK() \
    dyn->f.dfnone = 1

#define CLEAR_FLAGS_(s)                                                                                       \
    MOV64x(s, (1UL << F_AF) | (1UL << F_CF) | (1UL << F_OF) | (1UL << F_ZF) | (1UL << F_SF) | (1UL << F_PF)); \
    ANDN(xFlags, xFlags, s);

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
            /* af = bc & 0x8 */                                       \
            ANDI(scratch1, scratch2, 8);                              \
            SLLI_D(scratch1, scratch1, F_AF - 3);                     \
            OR(xFlags, xFlags, scratch1);                             \
        }                                                             \
        IFX (X_CF) {                                                  \
            /* cf = bc & (1<<(width-1)) */                            \
            if ((width) == 8) {                                       \
                ANDI(scratch1, scratch2, 0x80);                       \
            } else {                                                  \
                SRLI_D(scratch1, scratch2, (width) - 1);              \
                if ((width) != 64) ANDI(scratch1, scratch1, 1);       \
            }                                                         \
            BEQZ(scratch1, 8);                                        \
            ORI(xFlags, xFlags, 1 << F_CF);                           \
        }                                                             \
        IFX (X_OF) {                                                  \
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */ \
            SRLI_D(scratch1, scratch2, (width) - 2);                  \
            SRLI_D(scratch2, scratch1, 1);                            \
            XOR(scratch1, scratch1, scratch2);                        \
            ANDI(scratch1, scratch1, 1);                              \
            BEQZ(scratch1, 8);                                        \
            ORI(xFlags, xFlags, 1 << F_OF);                           \
        }                                                             \
    }


#if STEP == 0
#define X87_PUSH_OR_FAIL(var, dyn, ninst, scratch, t) var = x87_do_push(dyn, ninst, scratch, t)
#define X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, scratch)   x87_do_push_empty(dyn, ninst, scratch)
#define X87_POP_OR_FAIL(dyn, ninst, scratch)          x87_do_pop(dyn, ninst, scratch)
#else
#define X87_PUSH_OR_FAIL(var, dyn, ninst, scratch, t)                                                                                                    \
    if ((dyn->lsx.x87stack == 8) || (dyn->lsx.pushed == 8)) {                                                                                            \
        if (dyn->need_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Push, stack=%d/%d on inst %d\n", dyn->lsx.x87stack, dyn->lsx.pushed, ninst); \
        dyn->abort = 1;                                                                                                                                  \
        return addr;                                                                                                                                     \
    }                                                                                                                                                    \
    var = x87_do_push(dyn, ninst, scratch, t);

#define X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, scratch)                                                                                                      \
    if ((dyn->lsx.x87stack == 8) || (dyn->lsx.pushed == 8)) {                                                                                            \
        if (dyn->need_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Push, stack=%d/%d on inst %d\n", dyn->lsx.x87stack, dyn->lsx.pushed, ninst); \
        dyn->abort = 1;                                                                                                                                  \
        return addr;                                                                                                                                     \
    }                                                                                                                                                    \
    x87_do_push_empty(dyn, ninst, scratch);

#define X87_POP_OR_FAIL(dyn, ninst, scratch)                                                                                                           \
    if ((dyn->lsx.x87stack == -8) || (dyn->lsx.poped == 8)) {                                                                                          \
        if (dyn->need_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Pop, stack=%d/%d on inst %d\n", dyn->lsx.x87stack, dyn->lsx.poped, ninst); \
        dyn->abort = 1;                                                                                                                                \
        return addr;                                                                                                                                   \
    }                                                                                                                                                  \
    x87_do_pop(dyn, ninst, scratch);
#endif

#ifndef MAYSETFLAGS
#define MAYSETFLAGS() \
    do {              \
    } while (0)
#endif

#ifndef READFLAGS
#define READFLAGS(A)                                 \
    if (((A) != X_PEND && dyn->f.pending != SF_SET)  \
        && (dyn->f.pending != SF_SET_PENDING)) {     \
        if (dyn->f.pending != SF_PENDING) {          \
            LD_WU(x3, xEmu, offsetof(x64emu_t, df)); \
            j64 = (GETMARKF) - (dyn->native_size);   \
            BEQ(x3, xZR, j64);                       \
        }                                            \
        CALL_(const_updateflags, -1, 0, 0, 0);       \
        MARKF;                                       \
        dyn->f.pending = SF_SET;                     \
        SET_DFOK();                                  \
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

#define NAT_FLAGS_OPS(op1, op2)                    \
    do {                                           \
        dyn->insts[ninst + 1].nat_flags_op1 = op1; \
        dyn->insts[ninst + 1].nat_flags_op2 = op2; \
    } while (0)

#define NAT_FLAGS_ENABLE_CARRY() dyn->insts[ninst].nat_flags_carry = 1
#define NAT_FLAGS_ENABLE_SIGN()  dyn->insts[ninst].nat_flags_sign = 1

#ifndef SETFLAGS
#define SETFLAGS(A, B, FUSION)                                                                                      \
    if (dyn->f.pending != SF_SET                                                                                    \
        && ((B) & SF_SUB)                                                                                           \
        && (dyn->insts[ninst].x64.gen_flags & (~(A))))                                                              \
        READFLAGS(((dyn->insts[ninst].x64.gen_flags & X_PEND) ? X_ALL : dyn->insts[ninst].x64.gen_flags) & (~(A))); \
    if (dyn->insts[ninst].x64.gen_flags) switch (B) {                                                               \
            case SF_SUBSET:                                                                                         \
            case SF_SET: dyn->f.pending = SF_SET; break;                                                            \
            case SF_SET_DF:                                                                                         \
                dyn->f.pending = SF_SET;                                                                            \
                dyn->f.dfnone = 1;                                                                                  \
                break;                                                                                              \
            case SF_SET_NODF:                                                                                       \
                dyn->f.pending = SF_SET;                                                                            \
                dyn->f.dfnone = 0;                                                                                  \
                break;                                                                                              \
            case SF_PENDING: dyn->f.pending = SF_PENDING; break;                                                    \
            case SF_SUBSET_PENDING:                                                                                 \
            case SF_SET_PENDING:                                                                                    \
                dyn->f.pending = (dyn->insts[ninst].x64.gen_flags & X_PEND) ? SF_SET_PENDING : SF_SET;              \
                break;                                                                                              \
        }                                                                                                           \
    else                                                                                                            \
        dyn->f.pending = SF_SET;                                                                                    \
    dyn->insts[ninst].nat_flags_nofusion = (FUSION)
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
    if (dyn->insts[ninst].x64.gen_flags) { SET_DF(r, A); }
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
#ifndef TABLE64C
#define TABLE64C(A, V)
#endif

#ifndef TABLE64_
#define TABLE64_(A, V)
#endif

#define ARCH_INIT() SMSTART()

#define ARCH_RESET()

#if STEP < 2
#define GETIP(A, scratch)
#define GETIP_(A, scratch)
#else

#define GETIP_(A, scratch)                                        \
    do {                                                          \
        ssize_t _delta_ip = (ssize_t)(A) - (ssize_t)dyn->last_ip; \
        if (!dyn->last_ip) {                                      \
            if (dyn->need_reloc) {                                \
                TABLE64(xRIP, (A));                               \
            } else {                                              \
                MOV64x(xRIP, (A));                                \
            }                                                     \
        } else if (_delta_ip == 0) {                              \
        } else if (_delta_ip >= -2048 && _delta_ip < 2048) {      \
            ADDI_D(xRIP, xRIP, _delta_ip);                        \
        } else if (_delta_ip < 0 && _delta_ip >= -0xffffffff) {   \
            MOV32w(scratch, -_delta_ip);                          \
            SUB_D(xRIP, xRIP, scratch);                           \
        } else if (_delta_ip > 0 && _delta_ip <= 0xffffffff) {    \
            MOV32w(scratch, _delta_ip);                           \
            ADD_D(xRIP, xRIP, scratch);                           \
        } else {                                                  \
            if (dyn->need_reloc) {                                \
                TABLE64(xRIP, (A));                               \
            } else {                                              \
                MOV64x(xRIP, (A));                                \
            }                                                     \
        }                                                         \
    } while (0)
#define GETIP(A, scratch) \
    GETIP_(A, scratch);   \
    dyn->last_ip = (A);

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

#ifndef STEPNAME
#define STEPNAME3(N, M) N##M
#define STEPNAME2(N, M) STEPNAME3(N, M)
#define STEPNAME(N)     STEPNAME2(N, STEP)
#endif

#define native_pass STEPNAME(native_pass)

#define dynarec64_00          STEPNAME(dynarec64_00)
#define dynarec64_0F          STEPNAME(dynarec64_0F)
#define dynarec64_64          STEPNAME(dynarec64_64)
#define dynarec64_66          STEPNAME(dynarec64_66)
#define dynarec64_6664        STEPNAME(dynarec64_6664)
#define dynarec64_67          STEPNAME(dynarec64_67)
#define dynarec64_6764        STEPNAME(dynarec64_6764)
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
#define geted32             STEPNAME(geted32)
#define jump_to_epilog      STEPNAME(jump_to_epilog)
#define jump_to_epilog_fast STEPNAME(jump_to_epilog_fast)
#define jump_to_next        STEPNAME(jump_to_next)
#define ret_to_epilog       STEPNAME(ret_to_epilog)
#define retn_to_epilog      STEPNAME(retn_to_epilog)
#define iret_to_epilog      STEPNAME(iret_to_epilog)
#define call_c              STEPNAME(call_c)
#define grab_segdata        STEPNAME(grab_segdata)
#define emit_cmp16          STEPNAME(emit_cmp16)
#define emit_cmp16_0        STEPNAME(emit_cmp16_0)
#define emit_cmp32          STEPNAME(emit_cmp32)
#define emit_cmp32_0        STEPNAME(emit_cmp32_0)
#define emit_cmp8           STEPNAME(emit_cmp8)
#define emit_cmp8_0         STEPNAME(emit_cmp8_0)
#define emit_test8          STEPNAME(emit_test8)
#define emit_test8c         STEPNAME(emit_test8c)
#define emit_test16         STEPNAME(emit_test16)
#define emit_test32         STEPNAME(emit_test32)
#define emit_test32c        STEPNAME(emit_test32c)
#define emit_add32          STEPNAME(emit_add32)
#define emit_add32c         STEPNAME(emit_add32c)
#define emit_add8           STEPNAME(emit_add8)
#define emit_add8c          STEPNAME(emit_add8c)
#define emit_add16          STEPNAME(emit_add16)
#define emit_adc32          STEPNAME(emit_adc32)
#define emit_adc8           STEPNAME(emit_adc8)
#define emit_adc8c          STEPNAME(emit_adc8c)
#define emit_adc16          STEPNAME(emit_adc16)
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
#define emit_neg16          STEPNAME(emit_neg16)
#define emit_neg32          STEPNAME(emit_neg32)
#define emit_inc8           STEPNAME(emit_inc8)
#define emit_inc16          STEPNAME(emit_inc16)
#define emit_inc32          STEPNAME(emit_inc32)
#define emit_dec8           STEPNAME(emit_dec8)
#define emit_dec16          STEPNAME(emit_dec16)
#define emit_dec32          STEPNAME(emit_dec32)
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
#define emit_shl16c         STEPNAME(emit_shl16c)
#define emit_shl32          STEPNAME(emit_shl32)
#define emit_shl32c         STEPNAME(emit_shl32c)
#define emit_shl8           STEPNAME(emit_shl8)
#define emit_shr8           STEPNAME(emit_shr8)
#define emit_sar8           STEPNAME(emit_sar8)
#define emit_shr16          STEPNAME(emit_shr16)
#define emit_shr16c         STEPNAME(emit_shr16c)
#define emit_shr32          STEPNAME(emit_shr32)
#define emit_shr32c         STEPNAME(emit_shr32c)
#define emit_sar16          STEPNAME(emit_sar16)
#define emit_sar16c         STEPNAME(emit_sar16c)
#define emit_sar32c         STEPNAME(emit_sar32c)
#define emit_shld32c        STEPNAME(emit_shld32c)
#define emit_shrd32c        STEPNAME(emit_shrd32c)
#define emit_shld32         STEPNAME(emit_shld32)
#define emit_shrd32         STEPNAME(emit_shrd32)
#define emit_ror32          STEPNAME(emit_ror32)
#define emit_ror32c         STEPNAME(emit_ror32c)
#define emit_rol8           STEPNAME(emit_rol8)
#define emit_rol8c          STEPNAME(emit_rol8c)
#define emit_rol16          STEPNAME(emit_rol16)
#define emit_rol16c         STEPNAME(emit_rol16c)
#define emit_rol32          STEPNAME(emit_rol32)
#define emit_rol32c         STEPNAME(emit_rol32c)
#define emit_rcl16c         STEPNAME(emit_rcl16c)
#define emit_rcr16c         STEPNAME(emit_rcr16c)
#define emit_ror16c         STEPNAME(emit_ror16c)

#define emit_pf STEPNAME(emit_pf)

#define x87_do_push           STEPNAME(x87_do_push)
#define x87_do_push_empty     STEPNAME(x87_do_push_empty)
#define x87_do_pop            STEPNAME(x87_do_pop)
#define x87_get_current_cache STEPNAME(x87_get_current_cache)
#define x87_get_cache         STEPNAME(x87_get_cache)
#define x87_get_lsxcache      STEPNAME(x87_get_lsxcache)
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
void ret_to_epilog(dynarec_la64_t* dyn, uintptr_t ip, int ninst, rex_t rex);
void retn_to_epilog(dynarec_la64_t* dyn, uintptr_t ip, int ninst, rex_t rex, int n);
void iret_to_epilog(dynarec_la64_t* dyn, uintptr_t ip, int ninst, int is64bits);
void call_c(dynarec_la64_t* dyn, int ninst, la64_consts_t fnc, int reg, int ret, int saveflags, int save_reg, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
void grab_segdata(dynarec_la64_t* dyn, uintptr_t addr, int ninst, int reg, int segment, int modreg);
void emit_cmp8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_cmp16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_cmp32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_cmp8_0(dynarec_la64_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp16_0(dynarec_la64_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp32_0(dynarec_la64_t* dyn, int ninst, rex_t rex, uint8_t nextop, int s1, int s3, int s4, int s5);
void emit_test8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_test8c(dynarec_la64_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4, int s5);
void emit_test16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_test32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_test32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5);
void emit_add32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_add32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5);
void emit_add8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_add8c(dynarec_la64_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4);
void emit_add16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_adc32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5, int s6);
void emit_adc8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_adc8c(dynarec_la64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5, int s6);
void emit_adc16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
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
void emit_neg16(dynarec_la64_t* dyn, int ninst, int s1, int s3, int s4);
void emit_neg32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3);
void emit_inc8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_inc16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_inc32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_dec8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_dec16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_dec32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
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
void emit_shl16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_shl32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_shl32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5);
void emit_shl8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shr8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sar8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shr16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_shr16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_shr32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_shr32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_sar16(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sar16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_sar32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_shld32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);
void emit_shrd32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);
void emit_shld32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6);
void emit_shrd32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4, int s6);
void emit_ror32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_ror32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_rol8(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_rol8c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_rol16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_rol32(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_rol32c(dynarec_la64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_rcl16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_rcr16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_ror16c(dynarec_la64_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);

void emit_pf(dynarec_la64_t* dyn, int ninst, int s1, int s3, int s4);

// common coproc helpers

// x87 helper
// cache of the local stack counter, to avoid update at every call
int x87_stackcount(dynarec_la64_t* dyn, int ninst, int scratch);
// restore local stack counter
void x87_unstackcount(dynarec_la64_t* dyn, int ninst, int scratch, int count);
// fpu push. Return the Dd value to be used
int x87_do_push(dynarec_la64_t* dyn, int ninst, int s1, int t);
// fpu push. Do not allocate a cache register. Needs a scratch register to do x87stack synch (or 0 to not do it)
void x87_do_push_empty(dynarec_la64_t* dyn, int ninst, int s1);
// fpu pop. All previous returned Dd should be considered invalid
void x87_do_pop(dynarec_la64_t* dyn, int ninst, int s1);
// get cache index for a x87 reg, return -1 if cache doesn't exist
int x87_get_current_cache(dynarec_la64_t* dyn, int ninst, int st, int t);
// get cache index for a x87 reg, create the entry if needed
int x87_get_cache(dynarec_la64_t* dyn, int ninst, int populate, int s1, int s2, int a, int t);
// get extcache index for a x87 reg
int x87_get_lsxcache(dynarec_la64_t* dyn, int ninst, int s1, int s2, int a);
// get vfpu register for a x87 reg, create the entry if needed
int x87_get_st(dynarec_la64_t* dyn, int ninst, int s1, int s2, int a, int t);
// get vfpu register for a x87 reg, create the entry if needed. Do not fetch the Stx if not already in cache
int x87_get_st_empty(dynarec_la64_t* dyn, int ninst, int s1, int s2, int a, int t);
// Free st, using the FFREE opcode (so it's freed but stack is not moved)
void x87_free(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int st);
// refresh a value from the cache ->emu (nothing done if value is not cached)
void x87_refresh(dynarec_la64_t* dyn, int ninst, int s1, int s2, int st);
// refresh a value from the cache ->emu and then forget the cache (nothing done if value is not cached)
void x87_forget(dynarec_la64_t* dyn, int ninst, int s1, int s2, int st);
// refresh the cache value from emu
void x87_reget_st(dynarec_la64_t* dyn, int ninst, int s1, int s2, int st);
// swap 2 x87 regs
void x87_swapreg(dynarec_la64_t* dyn, int ninst, int s1, int s2, int a, int b);
// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_la64_t* dyn, int ninst, int s1, int s2);
// Restore round flag
void x87_restoreround(dynarec_la64_t* dyn, int ninst, int s1);
// Set rounding according to mxcsr flags, return reg to restore flags
void x87_reflectcount(dynarec_la64_t* dyn, int ninst, int s1, int s2);
void x87_unreflectcount(dynarec_la64_t* dyn, int ninst, int s1, int s2);
void x87_purgecache(dynarec_la64_t* dyn, int ninst, int next, int s1, int s2, int s3);

// reset the cache with n
void fpu_reset_cache(dynarec_la64_t* dyn, int ninst, int reset_n);
void fpu_propagate_stack(dynarec_la64_t* dyn, int ninst);
void fpu_purgecache(dynarec_la64_t* dyn, int ninst, int next, int s1, int s2, int s3);
void mmx_purgecache(dynarec_la64_t* dyn, int ninst, int next, int s1);
void fpu_reflectcache(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3);
void fpu_unreflectcache(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3);
void fpu_pushcache(dynarec_la64_t* dyn, int ninst, int s1, int not07);
void fpu_popcache(dynarec_la64_t* dyn, int ninst, int s1, int not07);
// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_la64_t* dyn, int ninst, int s1, int s2);

// SSE/SSE2 helpers
// purge the XMM0..XMM7 cache (before function call)
void sse_purge07cache(dynarec_la64_t* dyn, int ninst, int s1);
// get lsx register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_la64_t* dyn, int ninst, int s1, int a, int forwrite);
// get lsx register for an SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_la64_t* dyn, int ninst, int s1, int a);
// forget float register for a SSE reg, create the entry if needed
void sse_forget_reg(dynarec_la64_t* dyn, int ninst, int a);
// Push current value to the cache
void sse_reflect_reg(dynarec_la64_t* dyn, int ninst, int a);

// MMX helpers
//  get lsx register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int a);
// get lsx register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_la64_t* dyn, int ninst, int s1, int s2, int s3, int a);


// AVX helpers
// get lasx register for a AVX reg, create the entry if needed
int avx_get_reg(dynarec_la64_t* dyn, int ninst, int s1, int a, int forwrite, int width);
// get lasx register for an AVX reg, but don't try to synch it if it needed to be created
int avx_get_reg_empty(dynarec_la64_t* dyn, int ninst, int s1, int a, int width);
// forget float register for a AVX reg, create the entry if needed
void avx_forget_reg(dynarec_la64_t* dyn, int ninst, int a);
// Push current value to the cache
void avx_reflect_reg(dynarec_la64_t* dyn, int ninst, int a);
void avx_reflect_reg_upper128(dynarec_la64_t* dyn, int ninst, int a, int forwrite);

void CacheTransform(dynarec_la64_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3);

void la64_move64(dynarec_la64_t* dyn, int ninst, int reg, int64_t val);
void la64_move32(dynarec_la64_t* dyn, int ninst, int reg, int32_t val, int zeroup);

#if STEP < 2
#define CHECK_CACHE() 0
#else
#define CHECK_CACHE() (cacheupd = CacheNeedsTransform(dyn, ninst))
#endif

#define lsxcache_st_coherency STEPNAME(lsxcache_st_coherency)
int lsxcache_st_coherency(dynarec_la64_t* dyn, int ninst, int a, int b);

#if STEP == 0
#define ST_IS_F(A)        0
#define ST_IS_I64(A)      0
#define X87_COMBINE(A, B) LSX_CACHE_ST_D
#define X87_ST0           LSX_CACHE_ST_D
#define X87_ST(A)         LSX_CACHE_ST_D
#elif STEP == 1
#define ST_IS_F(A)        (lsxcache_get_current_st(dyn, ninst, A) == LSX_CACHE_ST_F)
#define ST_IS_I64(A)      (lsxcache_get_current_st(dyn, ninst, A) == LSX_CACHE_ST_I64)
#define X87_COMBINE(A, B) lsxcache_combine_st(dyn, ninst, A, B)
#define X87_ST0           lsxcache_no_i64(dyn, ninst, 0, lsxcache_get_current_st(dyn, ninst, 0))
#define X87_ST(A)         lsxcache_no_i64(dyn, ninst, A, lsxcache_get_current_st(dyn, ninst, A))
#else
#define ST_IS_F(A)   (lsxcache_get_st(dyn, ninst, A) == LSX_CACHE_ST_F)
#define ST_IS_I64(A) (lsxcache_get_st(dyn, ninst, A) == LSX_CACHE_ST_I64)
#if STEP == 3
#define X87_COMBINE(A, B) lsxcache_st_coherency(dyn, ninst, A, B)
#else
#define X87_COMBINE(A, B) lsxcache_get_st(dyn, ninst, A)
#endif
#define X87_ST0   lsxcache_get_st(dyn, ninst, 0)
#define X87_ST(A) lsxcache_get_st(dyn, ninst, A)
#endif


uintptr_t dynarec64_00(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F30F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_64(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog);
uintptr_t dynarec64_66(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_6664(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int seg, int* ok, int* need_epilog);
uintptr_t dynarec64_67(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_6764(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog);
uintptr_t dynarec64_660F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_66F0(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_66F20F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_66F30F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F0(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_F20F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_0F38(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_66_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_66_0F38(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_66_0F3A(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F2_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F2_0F38(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F2_0F3A(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F3_0F(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F3_0F38(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_D8(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_D9(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DA(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DB(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DC(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DD(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DE(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DF(dynarec_la64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);


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

#define GOCOND(B, T1, T2)                                                                                    \
    case B + 0x0:                                                                                            \
        INST_NAME(T1 "O " T2);                                                                               \
        GO(ANDI(tmp1, xFlags, 1 << F_OF), EQZ, NEZ, _, _, X_OF, X64_JMP_JO);                                 \
        break;                                                                                               \
    case B + 0x1:                                                                                            \
        INST_NAME(T1 "NO " T2);                                                                              \
        GO(ANDI(tmp1, xFlags, 1 << F_OF), NEZ, EQZ, _, _, X_OF, X64_JMP_JNO);                                \
        break;                                                                                               \
    case B + 0x2:                                                                                            \
        INST_NAME(T1 "C " T2);                                                                               \
        GO(ANDI(tmp1, xFlags, 1 << F_CF), EQZ, NEZ, GEU, LTU, X_CF, X64_JMP_JC);                             \
        break;                                                                                               \
    case B + 0x3:                                                                                            \
        INST_NAME(T1 "NC " T2);                                                                              \
        GO(ANDI(tmp1, xFlags, 1 << F_CF), NEZ, EQZ, LTU, GEU, X_CF, X64_JMP_JNC);                            \
        break;                                                                                               \
    case B + 0x4:                                                                                            \
        INST_NAME(T1 "Z " T2);                                                                               \
        GO(ANDI(tmp1, xFlags, 1 << F_ZF), EQZ, NEZ, NE, EQ, X_ZF, X64_JMP_JZ);                               \
        break;                                                                                               \
    case B + 0x5:                                                                                            \
        INST_NAME(T1 "NZ " T2);                                                                              \
        GO(ANDI(tmp1, xFlags, 1 << F_ZF), NEZ, EQZ, EQ, NE, X_ZF, X64_JMP_JNZ);                              \
        break;                                                                                               \
    case B + 0x6:                                                                                            \
        INST_NAME(T1 "BE " T2);                                                                              \
        GO(ANDI(tmp1, xFlags, (1 << F_CF) | (1 << F_ZF)), EQZ, NEZ, GTU, LEU, X_CF | X_ZF, X64_JMP_JBE);     \
        break;                                                                                               \
    case B + 0x7:                                                                                            \
        INST_NAME(T1 "NBE " T2);                                                                             \
        GO(ANDI(tmp1, xFlags, (1 << F_CF) | (1 << F_ZF)), NEZ, EQZ, LEU, GTU, X_CF | X_ZF, X64_JMP_JNBE);    \
        break;                                                                                               \
    case B + 0x8:                                                                                            \
        INST_NAME(T1 "S " T2);                                                                               \
        GO(ANDI(tmp1, xFlags, 1 << F_SF), EQZ, NEZ, _, _, X_SF, X64_JMP_JS);                                 \
        break;                                                                                               \
    case B + 0x9:                                                                                            \
        INST_NAME(T1 "NS " T2);                                                                              \
        GO(ANDI(tmp1, xFlags, 1 << F_SF), NEZ, EQZ, _, _, X_SF, X64_JMP_JNS);                                \
        break;                                                                                               \
    case B + 0xA:                                                                                            \
        INST_NAME(T1 "P " T2);                                                                               \
        GO(ANDI(tmp1, xFlags, 1 << F_PF), EQZ, NEZ, _, _, X_PF, X64_JMP_JP);                                 \
        break;                                                                                               \
    case B + 0xB:                                                                                            \
        INST_NAME(T1 "NP " T2);                                                                              \
        GO(ANDI(tmp1, xFlags, 1 << F_PF), NEZ, EQZ, _, _, X_PF, X64_JMP_JNP);                                \
        break;                                                                                               \
    case B + 0xC:                                                                                            \
        INST_NAME(T1 "L " T2);                                                                               \
        GO(SRLI_D(tmp1, xFlags, F_OF - F_SF);                                                                \
            XOR(tmp1, tmp1, xFlags);                                                                         \
            ANDI(tmp1, tmp1, 1 << F_SF), EQZ, NEZ, GE, LT, X_SF | X_OF, X64_JMP_JL);                         \
        break;                                                                                               \
    case B + 0xD:                                                                                            \
        INST_NAME(T1 "GE " T2);                                                                              \
        GO(SRLI_D(tmp1, xFlags, F_OF - F_SF);                                                                \
            XOR(tmp1, tmp1, xFlags);                                                                         \
            ANDI(tmp1, tmp1, 1 << F_SF), NEZ, EQZ, LT, GE, X_SF | X_OF, X64_JMP_JGE);                        \
        break;                                                                                               \
    case B + 0xE:                                                                                            \
        INST_NAME(T1 "LE " T2);                                                                              \
        GO(SRLI_D(tmp1, xFlags, F_OF - F_SF);                                                                \
            XOR(tmp1, tmp1, xFlags);                                                                         \
            ANDI(tmp1, tmp1, 1 << F_SF);                                                                     \
            ANDI(tmp3, xFlags, 1 << F_ZF);                                                                   \
            OR(tmp1, tmp1, tmp3);                                                                            \
            ANDI(tmp1, tmp1, (1 << F_SF) | (1 << F_ZF)), EQZ, NEZ, GT, LE, X_SF | X_OF | X_ZF, X64_JMP_JLE); \
        break;                                                                                               \
    case B + 0xF:                                                                                            \
        INST_NAME(T1 "G " T2);                                                                               \
        GO(SRLI_D(tmp1, xFlags, F_OF - F_SF);                                                                \
            XOR(tmp1, tmp1, xFlags);                                                                         \
            ANDI(tmp1, tmp1, 1 << F_SF);                                                                     \
            ANDI(tmp3, xFlags, 1 << F_ZF);                                                                   \
            OR(tmp1, tmp1, tmp3);                                                                            \
            ANDI(tmp1, tmp1, (1 << F_SF) | (1 << F_ZF)), NEZ, EQZ, LE, GT, X_SF | X_OF | X_ZF, X64_JMP_JG);  \
        break

// Dummy macros
#define B__safe(a, b, c) XOR(xZR, xZR, xZR)
#define B_(a, b, c)      XOR(xZR, xZR, xZR)

#define NATIVEJUMP_safe(COND, val) \
    B##COND##_safe(dyn->insts[ninst].nat_flags_op1, dyn->insts[ninst].nat_flags_op2, val);

#define NATIVEJUMP(COND, val) \
    B##COND(dyn->insts[ninst].nat_flags_op1, dyn->insts[ninst].nat_flags_op2, val);

#define NOTEST(s1)                                       \
    if (BOX64ENV(dynarec_test)) {                        \
        ST_W(xZR, xEmu, offsetof(x64emu_t, test.test));  \
        ST_W(xZR, xEmu, offsetof(x64emu_t, test.clean)); \
    }

#define SKIPTEST(s1)                                     \
    if (BOX64ENV(dynarec_test)) {                        \
        ST_W(xZR, xEmu, offsetof(x64emu_t, test.clean)); \
    }

#define GOTEST(s1, s2)                                 \
    if (BOX64ENV(dynarec_test)) {                      \
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


#define FCOM(w, v1, v2, s1, s2, s3)                            \
    LD_HU(s3, xEmu, offsetof(x64emu_t, sw));                   \
    MOV32w(s1, 0b1011100011111111); /* mask off c0,c1,c2,c3 */ \
    AND(s3, s3, s1);                                           \
    FCMP_##w(fcc0, v1, v2, cOR);                               \
    BCEQZ(fcc0, 28); /* undefined/NaN */                       \
    FCMP_##w(fcc1, v1, v2, cEQ);                               \
    BCNEZ(fcc1, 32);             /* equal */                   \
    FCMP_##w(fcc2, v1, v2, cLT); /* x2 = (v1<v2)?1:0 */        \
    MOVCF2GR(s2, fcc2);                                        \
    SLLI_D(s1, s2, 8);                                         \
    B(20); /* end */                                           \
    /* undefined/NaN */                                        \
    LU12I_W(s1, 4);                                            \
    ADDI_D(s1, s1, 0b010100000000);                            \
    B(8); /* end */                                            \
    /* equal */                                                \
    LU12I_W(s1, 4);                                            \
    /* end */                                                  \
    OR(s3, s3, s1);                                            \
    ST_H(s3, xEmu, offsetof(x64emu_t, sw));

#define FCOMS(v1, v2, s1, s2, s3) FCOM(S, v1, v2, s1, s2, s3)
#define FCOMD(v1, v2, s1, s2, s3) FCOM(D, v1, v2, s1, s2, s3)

#define FCOMI(w, v1, v2, s1, s2)                               \
    IFX (X_OF | X_AF | X_SF | X_PEND) {                        \
        MOV64x(s2, ((1 << F_OF) | (1 << F_AF) | (1 << F_SF))); \
        ANDN(xFlags, xFlags, s2);                              \
    }                                                          \
    IFX (X_CF | X_PF | X_ZF | X_PEND) {                        \
        MOV32w(s2, 0b01000101);                                \
        ANDN(xFlags, xFlags, s2);                              \
        FCMP_##w(fcc0, v1, v2, cOR);                           \
        BCEQZ(fcc0, 24); /* undefined/NaN */                   \
        FCMP_##w(fcc1, v1, v2, cEQ);                           \
        BCNEZ(fcc1, 24);             /* equal */               \
        FCMP_##w(fcc2, v1, v2, cLT); /* s1 = (v1<v2)?1:0 */    \
        MOVCF2GR(s1, fcc2);                                    \
        B(4 * 4); /* end */                                    \
        /* undefined/NaN */                                    \
        MV(s1, s2);                                            \
        B(2 * 4); /* end */                                    \
        /* equal */                                            \
        ADDI_D(s1, xZR, 0b01000000);                           \
        /* end */                                              \
        OR(xFlags, xFlags, s1);                                \
    }                                                          \
    SPILL_EFLAGS();                                            \
    SET_DFNONE()

#define FCOMIS(v1, v2, s1, s2) FCOMI(S, v1, v2, s1, s2)
#define FCOMID(v1, v2, s1, s2) FCOMI(D, v1, v2, s1, s2)

// Restore xFlags from LBT.eflags
#define RESTORE_EFLAGS(s)             \
    do {                              \
        if (cpuext.lbt) {             \
            CLEAR_FLAGS_(s);          \
            X64_GET_EFLAGS(s, X_ALL); \
            OR(xFlags, xFlags, s);    \
        }                             \
    } while (0)

// Spill xFlags to LBT.eflags
#define SPILL_EFLAGS()                     \
    do {                                   \
        if (cpuext.lbt) {                  \
            X64_SET_EFLAGS(xFlags, X_ALL); \
        }                                  \
    } while (0)

#define PURGE_YMM()

#define ALIGNED_ATOMICxw ((fixedaddress && !(fixedaddress & (((1 << (2 + rex.w)) - 1)))) || BOX64ENV(dynarec_aligned_atomics))
#define ALIGNED_ATOMICH  ((fixedaddress && !(fixedaddress & 1)) || BOX64ENV(dynarec_aligned_atomics))


// lock op related macros
/*
  LOCK_3264_CROSS_8BYTE cross 8byte, lock lower part 8byte.
  use ll.d lock lower 8byte. ld.d load ed to likely emulate atomic 4byte/8byte operation
    op = atomic function
    s1 = original ed
    wback = ed addr
    s4 = result
    s5 = 8 byte lower block for sc
    s6 = aligned addr
*/
#define LOCK_3264_CROSS_8BYTE(op, s1, wback, s4, s5, s6) \
    MV(s6, wback);                                       \
    BSTRINS_D(s6, xZR, 2, 0);                            \
    MARKLOCK2;                                           \
    LDxw(s1, wback, 0);                                  \
    LL_D(s5, s6, 0);                                     \
    op;                                                  \
    SC_D(s5, s6, 0);                                     \
    BEQZ_MARKLOCK2(s5);                                  \
    SDxw(s4, wback, 0);

/*
    LOCK_3264_IN_8BYTE unaligned but in 8 bytes.
    use ll.d/sc.d to atomic operation. use amcas.db.d when possible
    s1 = original ed
    wback = ed addr / aligned ed addr
    s3 = offset
    s4 = result
    s5 = 8 byte block
    s6 = mask, amcas orignal val
*/
#define LOCK_32_IN_8BYTE(op, s1, wback, s3, s4, s5, s6) \
    ed = wback;                                         \
    if (wback != x2) {                                  \
        ed = x2;                                        \
        MV(ed, wback);                                  \
    }                                                   \
    BSTRINS_D(ed, xZR, 2, 0);                           \
    SLLI_W(s3, s3, 3);                                  \
    if (cpuext.lamcas) {                                \
        LD_D(s5, ed, 0);                                \
        MARKLOCK;                                       \
    } else {                                            \
        MARKLOCK;                                       \
        LL_D(s5, ed, 0);                                \
    }                                                   \
    SRL_D(s1, s5, s3);                                  \
    BSTRPICK_D(s1, s1, 31, 0);                          \
    op;                                                 \
    BSTRPICK_D(s4, s4, 31, 0);                          \
    SLL_D(s4, s4, s3);                                  \
    ADDI_D(s6, xZR, -1);                                \
    BSTRINS_D(s6, xZR, 63, 32);                         \
    SLL_D(s6, s6, s3);                                  \
    ANDN(s6, s5, s6);                                   \
    OR(s4, s6, s4);                                     \
    if (cpuext.lamcas) {                                \
        MV(s6, s5);                                     \
        AMCAS_DB_D(s5, s4, ed);                         \
        BNE_MARKLOCK(s5, s6);                           \
    } else {                                            \
        SC_D(s4, ed, 0);                                \
        BEQZ_MARKLOCK(s4);                              \
    }

/*
    LOCK_8_ALIGNED_4BYTE aligned 4 bytes 8bits op
    use ll.w/sc.w to atomic operation . use amcas.db.w when possible
    s1 = original ed
    wback = ed addr / aligned ed addr
    s3 = 4 byte block for amcas write
    s4 = result
    s5 = 4 byte block
    s6 = temp use
*/
#define LOCK_8_ALIGNED_4BYTE(op, s1, wback, s3, s4, s5, s6)   \
    if (cpuext.lamcas) {                                      \
        LD_B(s1, wback, 0); /* amcas.b rd is sign extended */ \
        MARKLOCK;                                             \
        op;                                                   \
        MV(s6, s1);                                           \
        AMCAS_DB_B(s1, s4, wback);                            \
        BNE_MARKLOCK(s1, s6);                                 \
    } else {                                                  \
        MARKLOCK;                                             \
        LL_W(s5, wback, 0);                                   \
        BSTRPICK_D(s1, s5, 7, 0);                             \
        EXT_W_B(s6, s1);                                      \
        op;                                                   \
        BSTRINS_W(s5, s4, 7, 0);                              \
        SC_W(s5, wback, 0);                                   \
        BEQZ_MARKLOCK(s5);                                    \
    }

/*
    LOCK_8_IN_4BYTE unaligned but in 4 bytes 8bits op.
    use ll.w/sc.w to atomic it. use amcas.db.w when possible
    s1 = original ed
    wback = ed addr / aligned ed addr
    s3 = offset
    s4 = result
    s5 = 4 byte block
    s6 = mask
    s7 = wback tmp
*/
#define LOCK_8_IN_4BYTE(op, s1, wback, s3, s4, s5, s6, s7) \
    if (cpuext.lamcas) {                                   \
        LD_B(s1, wback, 0);                                \
        MARKLOCK2;                                         \
        op;                                                \
        MV(s6, s1);                                        \
        AMCAS_DB_W(s1, s4, wback);                         \
        BNE_MARKLOCK2(s1, s6);                             \
    } else {                                               \
        MV(s7, wback);                                     \
        BSTRINS_D(s7, xZR, 1, 0);                          \
        SLLI_W(s3, s3, 3);                                 \
        MARKLOCK2;                                         \
        LL_W(s5, s7, 0);                                   \
        SRL_W(s1, s5, s3);                                 \
        BSTRPICK_W(s1, s1, 7, 0);                          \
        EXT_W_B(s6, s1);                                   \
        op;                                                \
        BSTRPICK_D(s4, s4, 7, 0);                          \
        SLL_W(s4, s4, s3);                                 \
        ADDI_W(s6, xZR, -1);                               \
        BSTRINS_W(s6, xZR, 31, 8);                         \
        SLL_W(s6, s6, s3);                                 \
        ANDN(s6, s5, s6);                                  \
        OR(s4, s6, s4);                                    \
        SC_W(s4, s7, 0);                                   \
        BEQZ_MARKLOCK2(s4);                                \
    }


#endif //__DYNAREC_LA64_HELPER_H__
