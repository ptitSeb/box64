#ifndef __DYNAREC_ARM64_HELPER_H__
#define __DYNAREC_ARM64_HELPER_H__

// undef to get Close to SSE Float->int conversions
//#define PRECISE_CVT

#ifndef STEP_PASS
#if STEP == 0
#include "dynarec_arm64_pass0.h"
#elif STEP == 1
#include "dynarec_arm64_pass1.h"
#elif STEP == 2
#include "dynarec_arm64_pass2.h"
#elif STEP == 3
#include "dynarec_arm64_pass3.h"
#endif
#define STEP_PASS
#endif

#include "debug.h"
#include "arm64_emitter.h"
#include "../emu/x64primop.h"
#include "dynarec_arm64_consts.h"

#define F8      *(uint8_t*)(addr++)
#define F8S     *(int8_t*)(addr++)
#define F16     *(uint16_t*)(addr+=2, addr-2)
#define F16S    *(int16_t*)(addr+=2, addr-2)
#define F32     *(uint32_t*)(addr+=4, addr-4)
#define F32S    *(int32_t*)(addr+=4, addr-4)
#define F32S64  (uint64_t)(int64_t)F32S
#define F64     *(uint64_t*)(addr+=8, addr-8)
#define PK(a)   *(uint8_t*)(addr+a)
#define PK16(a)   *(uint16_t*)(addr+a)
#define PK32(a)   *(uint32_t*)(addr+a)
#define PK64(a)   *(uint64_t*)(addr+a)
#define PKip(a)   *(uint8_t*)(ip+a)

#ifndef FEMIT
#define FEMIT(A)    EMIT(A)
#endif
#ifndef ENDPREFIX
#define ENDPREFIX
#endif

//LOCK_* define
#define LOCK_LOCK   (int*)1

// GETGD    get x64 register in gd
#define GETGD gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3))
// GETVD    get x64 register in vd
#define GETVD vd = TO_NAT(vex.v)
//GETED can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETED(D)                                                                                                                                     \
    if (MODREG) {                                                                                                                                    \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                                                                    \
        wback = 0;                                                                                                                                   \
    } else {                                                                                                                                         \
        SMREAD();                                                                                                                                    \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff << (2 + rex.w), (1 << (2 + rex.w)) - 1, rex, NULL, 0, D); \
        LDxw(x1, wback, fixedaddress);                                                                                                               \
        ed = x1;                                                                                                                                     \
    }
#define GETEDx(D)                                                                                                     \
    if (MODREG) {                                                                                                     \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                                     \
        wback = 0;                                                                                                    \
    } else {                                                                                                          \
        SMREAD();                                                                                                     \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff << 3, 7, rex, NULL, 0, D); \
        LDx(x1, wback, fixedaddress);                                                                                 \
        ed = x1;                                                                                                      \
    }
#define GETEDz(D)                                                                                                                                         \
    if (MODREG) {                                                                                                                                         \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                                                                         \
        wback = 0;                                                                                                                                        \
    } else {                                                                                                                                              \
        SMREAD();                                                                                                                                         \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff << (3 - (rex.is32bits || rex.is67)), (rex.is32bits || rex.is67) ? 3 : 7, rex, NULL, 0, D); \
        LDz(x1, wback, fixedaddress);                                                                                                                     \
        ed = x1;                                                                                                                                          \
    }
#define GETEDw(D)                                                                                                     \
    if (MODREG) {                                                                                                     \
        ed = xEAX + (nextop & 7) + (rex.b << 3);                                                                      \
        wback = 0;                                                                                                    \
    } else {                                                                                                          \
        SMREAD();                                                                                                     \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff << 2, 3, rex, NULL, 0, D); \
        LDW(x1, wback, fixedaddress);                                                                                 \
        ed = x1;                                                                                                      \
    }
#define GETSEDw(D)                                                                                                    \
    if (MODREG) {                                                                                                     \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                                     \
        SXTWx(x1, ed);                                                                                                \
        wb = x1;                                                                                                      \
        wback = 0;                                                                                                    \
    } else {                                                                                                          \
        SMREAD();                                                                                                     \
        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, &unscaled, 0xfff << 2, 3, rex, NULL, 0, D); \
        LDSW(x1, wback, fixedaddress);                                                                                \
        wb = ed = x1;                                                                                                 \
    }
//GETEDH can use hint for ed, and r1 or r2 for wback (depending on hint). wback is 0 if ed is xEAX..xEDI
#define GETEDH(hint, D)                                                                                                                                                  \
    if (MODREG) {                                                                                                                                                        \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                                                                                        \
        wback = 0;                                                                                                                                                       \
    } else {                                                                                                                                                             \
        SMREAD();                                                                                                                                                        \
        addr = geted(dyn, addr, ninst, nextop, &wback, (hint == x2) ? x1 : x2, &fixedaddress, &unscaled, 0xfff << (2 + rex.w), (1 << (2 + rex.w)) - 1, rex, NULL, 0, D); \
        LDxw(hint, wback, fixedaddress);                                                                                                                                 \
        ed = hint;                                                                                                                                                       \
    }
//GETEDW can use hint for wback and ret for ed. wback is 0 if ed is xEAX..xEDI
#define GETEDW(hint, ret, D)                                                                                                                           \
    if (MODREG) {                                                                                                                                      \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                                                                                      \
        MOVxw_REG(ret, ed);                                                                                                                            \
        wback = 0;                                                                                                                                     \
    } else {                                                                                                                                           \
        SMREAD();                                                                                                                                      \
        addr = geted(dyn, addr, ninst, nextop, &wback, hint, &fixedaddress, &unscaled, 0xfff << (2 + rex.w), (1 << (2 + rex.w)) - 1, rex, NULL, 0, D); \
        ed = ret;                                                                                                                                      \
        LDxw(ed, wback, fixedaddress);                                                                                                                 \
    }
// Write back ed in wback (if wback not 0)
#define WBACK       if(wback) {STxw(ed, wback, fixedaddress); SMWRITE();}
// Write back ed in wback (if wback not 0)
#define WBACKx      if(wback) {STx(ed, wback, fixedaddress); SMWRITE();}
// Write back ed in wback (if wback not 0)
#define WBACKw      if(wback) {STW(ed, wback, fixedaddress); SMWRITE();}
//FAKEELike GETED, but doesn't get anything
#define FAKEED                                    \
    if (MODREG) {                                 \
        ed = TO_NAT((nextop & 7) + (rex.b << 3)); \
        wback = 0;                                \
    } else {                                      \
        addr = fakeed(dyn, addr, ninst, nextop);  \
    }

// GETGW extract x64 register in gd, that is i
#define GETGW(i)                                        \
    gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); \
    UXTHw(i, gd);                                       \
    gd = i;

// GETGW extract x64 register in gd, that is i, Signed extented
#define GETSGW(i)                                       \
    gd = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); \
    SXTHw(i, gd);                                       \
    gd = i;

//GETEWW will use i for ed, and can use w for wback.
#define GETEWW(w, i, D)                                                                                                         \
    if (MODREG) {                                                                                                               \
        wback = TO_NAT((nextop & 7) + (rex.b << 3));                                                                            \
        UXTHw(i, wback);                                                                                                        \
        ed = i;                                                                                                                 \
        wb1 = 0;                                                                                                                \
    } else {                                                                                                                    \
        SMREAD();                                                                                                               \
        addr = geted(dyn, addr, ninst, nextop, &wback, w, &fixedaddress, &unscaled, 0xfff << 1, (1 << 1) - 1, rex, NULL, 0, D); \
        LDH(i, wback, fixedaddress);                                                                                            \
        ed = i;                                                                                                                 \
        wb1 = 1;                                                                                                                \
    }
//Compute wback for MDREG only, no fetching
#define CALCEW()                                                                                                                \
    wback = TO_NAT((nextop & 7) + (rex.b << 3));                                                                                \
//GETEW will use i for ed, and can use r3 for wback.
#define GETEW(i, D)                                                                                                              \
    if (MODREG) {                                                                                                                \
        wback = TO_NAT((nextop & 7) + (rex.b << 3));                                                                             \
        UXTHw(i, wback);                                                                                                         \
        ed = i;                                                                                                                  \
        wb1 = 0;                                                                                                                 \
    } else {                                                                                                                     \
        SMREAD();                                                                                                                \
        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff << 1, (1 << 1) - 1, rex, NULL, 0, D); \
        LDH(i, wback, fixedaddress);                                                                                             \
        ed = i;                                                                                                                  \
        wb1 = 1;                                                                                                                 \
    }
//GETEW will use i for ed, and can use r3 for wback.
#define GETEW32(i, D)                                                                                                              \
    if (MODREG) {                                                                                                                  \
        wback = TO_NAT((nextop & 7) + (rex.b << 3));                                                                               \
        UXTHw(i, wback);                                                                                                           \
        ed = i;                                                                                                                    \
        wb1 = 0;                                                                                                                   \
    } else {                                                                                                                       \
        SMREAD();                                                                                                                  \
        addr = geted32(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff << 1, (1 << 1) - 1, rex, NULL, 0, D); \
        LDH(i, wback, fixedaddress);                                                                                               \
        ed = i;                                                                                                                    \
        wb1 = 1;                                                                                                                   \
    }
//GETSEW will use i for ed, and can use r3 for wback. This is the Signed version
#define GETSEW(i, D)                                                                                                             \
    if (MODREG) {                                                                                                                \
        wback = TO_NAT((nextop & 7) + (rex.b << 3));                                                                             \
        SXTHw(i, wback);                                                                                                         \
        ed = i;                                                                                                                  \
        wb1 = 0;                                                                                                                 \
    } else {                                                                                                                     \
        SMREAD();                                                                                                                \
        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff << 1, (1 << 1) - 1, rex, NULL, 0, D); \
        LDSHw(i, wback, fixedaddress);                                                                                           \
        ed = i;                                                                                                                  \
        wb1 = 1;                                                                                                                 \
    }
// Write ed back to original register / memory
#define EWBACK       EWBACKW(ed)
// Write w back to original register / memory
#define EWBACKW(w)   if(wb1) {STH(w, wback, fixedaddress); SMWRITE();} else {BFIx(wback, w, 0, 16);}
// Write back gd in correct register
#define GWBACK BFIx(TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)), gd, 0, 16);
// no fetch version of GETEB for MODREG path only
#define CALCEB()                                                                                             \
    if (rex.rex) {                                                                                           \
        wback = TO_NAT((nextop & 7) + (rex.b << 3));                                                         \
        wb2 = 0;                                                                                             \
    } else {                                                                                                 \
        wback = (nextop & 7);                                                                                \
        wb2 = (wback >> 2) * 8;                                                                              \
        wback = TO_NAT(wback & 3);                                                                           \
    }                                                                                                        \
//GETEB will use i for ed, and can use r3 for wback.
#define GETEB(i, D)                                                                                              \
    if (MODREG) {                                                                                                \
        if (rex.rex) {                                                                                           \
            wback = TO_NAT((nextop & 7) + (rex.b << 3));                                                         \
            wb2 = 0;                                                                                             \
        } else {                                                                                                 \
            wback = (nextop & 7);                                                                                \
            wb2 = (wback >> 2) * 8;                                                                              \
            wback = TO_NAT(wback & 3);                                                                           \
        }                                                                                                        \
        UBFXx(i, wback, wb2, 8);                                                                                 \
        wb1 = 0;                                                                                                 \
        ed = i;                                                                                                  \
    } else {                                                                                                     \
        SMREAD();                                                                                                \
        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff, 0, rex, NULL, 0, D); \
        LDB(i, wback, fixedaddress);                                                                             \
        wb1 = 1;                                                                                                 \
        ed = i;                                                                                                  \
    }
//GETSEB sign extend EB, will use i for ed, and can use r3 for wback.
#define GETSEB(i, D)                                                                                             \
    if (MODREG) {                                                                                                \
        if (rex.rex) {                                                                                           \
            wback = TO_NAT((nextop & 7) + (rex.b << 3));                                                         \
            wb2 = 0;                                                                                             \
        } else {                                                                                                 \
            wback = (nextop & 7);                                                                                \
            wb2 = (wback >> 2) * 8;                                                                              \
            wback = TO_NAT(wback & 3);                                                                           \
        }                                                                                                        \
        SBFXx(i, wback, wb2, 8);                                                                                 \
        wb1 = 0;                                                                                                 \
        ed = i;                                                                                                  \
    } else {                                                                                                     \
        SMREAD();                                                                                                \
        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff, 0, rex, NULL, 0, D); \
        LDSBx(i, wback, fixedaddress);                                                                           \
        wb1 = 1;                                                                                                 \
        ed = i;                                                                                                  \
    }
//GETEB will use i for ed, and can use r3 for wback.
#define GETEB32(i, D)                                                                                              \
    if (MODREG) {                                                                                                  \
        if (rex.rex) {                                                                                             \
            wback = TO_NAT((nextop & 7) + (rex.b << 3));                                                           \
            wb2 = 0;                                                                                               \
        } else {                                                                                                   \
            wback = (nextop & 7);                                                                                  \
            wb2 = (wback >> 2) * 8;                                                                                \
            wback = TO_NAT(wback & 3);                                                                             \
        }                                                                                                          \
        UBFXx(i, wback, wb2, 8);                                                                                   \
        wb1 = 0;                                                                                                   \
        ed = i;                                                                                                    \
    } else {                                                                                                       \
        SMREAD();                                                                                                  \
        addr = geted32(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, &unscaled, 0xfff, 0, rex, NULL, 0, D); \
        LDB(i, wback, fixedaddress);                                                                               \
        wb1 = 1;                                                                                                   \
        ed = i;                                                                                                    \
    }
// Write eb (ed) back to original register / memory
#define EBBACK   if(wb1) {STB(ed, wback, fixedaddress); SMWRITE();} else {BFIx(wback, ed, wb2, 8);}
// no fetch version of GETGB
#define CALCGB()                                             \
    if (rex.rex) {                                           \
        gb1 = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); \
        gb2 = 0;                                             \
    } else {                                                 \
        gd = (nextop & 0x38) >> 3;                           \
        gb2 = ((gd & 4) << 1);                               \
        gb1 = TO_NAT(gd & 3);                                \
    }                                                        \
//GETGB will use i for gd
#define GETGB(i)                                             \
    if (rex.rex) {                                           \
        gb1 = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); \
        gb2 = 0;                                             \
    } else {                                                 \
        gd = (nextop & 0x38) >> 3;                           \
        gb2 = ((gd & 4) << 1);                               \
        gb1 = TO_NAT(gd & 3);                                \
    }                                                        \
    gd = i;                                                  \
    UBFXx(gd, gb1, gb2, 8);
//GETSGB sign extend GB, will use i for gd
#define GETSGB(i)                                            \
    if (rex.rex) {                                           \
        gb1 = TO_NAT(((nextop & 0x38) >> 3) + (rex.r << 3)); \
        gb2 = 0;                                             \
    } else {                                                 \
        gd = (nextop & 0x38) >> 3;                           \
        gb2 = ((gd & 4) << 1);                               \
        gb1 = TO_NAT(gd & 3);                                \
    }                                                        \
    gd = i;                                                  \
    SBFXx(gd, gb1, gb2, 8);
// Write gb (gd) back to original register / memory
#define GBBACK   BFIx(gb1, gd, gb2, 8);

// Generic get GD, but reg value in gd (R_RAX is not added)
#define GETG        gd = ((nextop&0x38)>>3)+(rex.r<<3)

// Get GX as a quad (might use x1)
#define GETGX(a, w)                     \
    gd = ((nextop&0x38)>>3)+(rex.r<<3); \
    a = sse_get_reg(dyn, ninst, x1, gd, w)

// Get an empty GX (use x1)
#define GETGX_empty(a)                  \
    gd = ((nextop&0x38)>>3)+(rex.r<<3); \
    a = sse_get_reg_empty(dyn, ninst, x1, gd)

// Get VX as a quad (might use x1)
#define GETVX(a, w)                     \
    a = sse_get_reg(dyn, ninst, x1, vex.v, w)

// Get an empty VX (use x1)
#define GETVX_empty(a)                  \
    a = sse_get_reg_empty(dyn, ninst, x1, vex.v)

// Get empty VX, and non-written  EX
#define GETVX_empty_EX(vx, ex, D)           \
    GETEX_Y(ex, 0, D);                      \
    GETVX_empty(vx)

// Get VX, and EX
#define GETVXEX(vx, w1, ex, w2, D)          \
    GETEX_Y(ex, w2, D);                     \
    GETVX(vx, w1)

#define GETGY_VY(a, w1, b, w2, k1, k2)                      \
    if(w2) b = ymm_get_reg(dyn, ninst, x1, vex.v, w2, gd, k1, k2); \
    a = ymm_get_reg(dyn, ninst, x1, gd, w1, vex.v, k1, k2); \
    if(!w2) b = ymm_get_reg(dyn, ninst, x1, vex.v, w2, gd, k1, k2)

#define GETGY_empty(a, k1, k2, k3)                          \
    a = ymm_get_reg_empty(dyn, ninst, x1, gd, k1, k2, k3)

#define GETGY(a, w, k1, k2, k3)                             \
    a = ymm_get_reg(dyn, ninst, x1, gd, w, k1, k2, k3)

#define GETGY_empty_VY(a, b, w2, k1, k2)                    \
    b = ymm_get_reg(dyn, ninst, x1, vex.v, w2, gd, k1, k2); \
    a = ymm_get_reg_empty(dyn, ninst, x1, gd, vex.v, k1, k2)

// Get empty GX, and non-written VX and EX
#define GETGX_empty_VXEX(gx, vx, ex, D)     \
    GETVX(vx, 0);                           \
    GETEX_Y(ex, 0, D);                      \
    GETGX_empty(gx)

// Get empty GX, and non-written EX
#define GETGX_empty_EX(gx, ex, D)           \
    GETEX_Y(ex, 0, D);                      \
    GETGX_empty(gx)

// Get empty GX, and non-written VX
#define GETGX_empty_VX(gx, vx)              \
    GETVX(vx, 0);                           \
    GETGX_empty(gx)

// Get EX and and non-written VX and GX
#define GETGXVXEX(gx, vx, ex, D)            \
    GETVX(vx, 0);                           \
    GETEX_Y(ex, 1, D);                      \
    GETGX(gx, 0)

// Get GX and and non-written VX and EX
#define GETGX_VXEX(gx, vx, ex, D)            \
    GETVX(vx, 0);                           \
    GETEX_Y(ex, 0, D);                      \
    GETGX(gx, 1)

#define GETGXVXEX_empty(gx, vx, ex, D)      \
    GETVX(vx, 0);                           \
    GETGX(gx, 0);                           \
    GETEX_empty_Y(ex, D);

// Get empty GY, and non-written VY and EY
#define GETGY_empty_VYEY(gy, vy, ey)                                                            \
    vy = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);    \
    if(MODREG)                                                                                  \
        ey = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0, gd, vex.v, -1);              \
    else                                                                                        \
        VLDR128_U12(ey, ed, fixedaddress+16);                                                   \
    gy = ymm_get_reg_empty(dyn, ninst, x1, gd, vex.v, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1)

// Get EY and non-written VY and GY
#define GETGYVYEY(gy, vy, ey)                                                                   \
    vy = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);    \
    if(MODREG)                                                                                  \
        ey = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 1, gd, vex.v, -1);              \
    else                                                                                        \
        VLDR128_U12(ey, ed, fixedaddress+16);                                                   \
    gy = ymm_get_reg(dyn, ninst, x1, gd, 0, vex.v, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1)

// Get GY and non-written VY and EY
#define GETGY_VYEY(gy, vy, ey)                                                                  \
    vy = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);    \
    if(MODREG)                                                                                  \
        ey = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0, gd, vex.v, -1);              \
    else                                                                                        \
        VLDR128_U12(ey, ed, fixedaddress+16);                                                   \
    gy = ymm_get_reg(dyn, ninst, x1, gd, 1, vex.v, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1)

// Get empty EY and non-written VY and GY
#define GETGYVYEY_empty(gy, vy, ey)                                                             \
    vy = ymm_get_reg(dyn, ninst, x1, vex.v, 0, gd, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);    \
    gy = ymm_get_reg(dyn, ninst, x1, gd, 0, vex.v, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1);    \
    if(MODREG)                                                                                  \
        ey = ymm_get_reg_empty(dyn, ninst, x1, (nextop&7)+(rex.b<<3), gd, vex.v, -1)

// Get EY and non-written GY
#define GETGYEY(gy, ey)                                                                         \
    if(MODREG)                                                                                  \
        ey = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 1, gd, -1, -1);                 \
    else                                                                                        \
        VLDR128_U12(ey, ed, fixedaddress+16);                                                   \
    gy = ymm_get_reg(dyn, ninst, x1, gd, 0, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1, -1)

// Get EY
#define GETEY(ey)                                                                               \
    if(MODREG)                                                                                  \
        ey = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0, -1, -1, -1);                 \
    else                                                                                        \
        VLDR128_U12(ey, ed, fixedaddress+16);                                                   \

// Get written EY
#define GETEYw(ey)                                                                              \
    if(MODREG)                                                                                  \
        ey = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 1, -1, -1, -1);                 \
    else                                                                                        \
        VLDR128_U12(ey, ed, fixedaddress+16);                                                   \

// Get empty EY and non-written GY
#define GETGYEY_empty(gy, ey)                                                                   \
    gy = ymm_get_reg(dyn, ninst, x1, gd, 0, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1, -1);       \
    if(MODREG)                                                                                  \
        ey = ymm_get_reg_empty(dyn, ninst, x1, (nextop&7)+(rex.b<<3), gd, -1, -1)

// Get empty GY, and non-written EY
#define GETGY_empty_EY(gy, ey)                                                      \
    if(MODREG)                                                                      \
        ey = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0, gd, -1, -1);     \
    else                                                                            \
        VLDR128_U12(ey, ed, fixedaddress+16);                                       \
    gy = ymm_get_reg_empty(dyn, ninst, x1, gd, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1, -1)

// Get empty VY, and non-written EY
#define GETVY_empty_EY(vy, ey)                                                      \
    if(MODREG)                                                                      \
        ey = ymm_get_reg(dyn, ninst, x1, (nextop&7)+(rex.b<<3), 0, vex.v, -1, -1);  \
    else                                                                            \
        VLDR128_U12(ey, ed, fixedaddress+16);                                       \
    vy = ymm_get_reg_empty(dyn, ninst, x1, vex.v, (MODREG)?((nextop&7)+(rex.b<<3)):-1, -1, -1)

// Get EX as a quad, (x3 is used)
#define GETEX_Y(a, w, D)                                                                                       \
    if (MODREG) {                                                                                              \
        a = sse_get_reg(dyn, ninst, x3, (nextop & 7) + (rex.b << 3), w);                                       \
    } else {                                                                                                   \
        SMREAD();                                                                                              \
        addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0xffe << 4, 15, rex, NULL, 0, D); \
        unscaled = 0;                                                                                          \
        a = fpu_get_scratch(dyn, ninst);                                                                       \
        VLDR128_U12(a, ed, fixedaddress);                                                                      \
    }
// Get EX as a quad, (x3 is used)
#define GETEX_empty_Y(a, D)                                                                             \
    if(MODREG) {                                                                                        \
        a = sse_get_reg_empty(dyn, ninst, x3, (nextop&7)+(rex.b<<3));                                   \
    } else {                                                                                            \
        a = fpu_get_scratch(dyn, ninst);                                                                \
        addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, NULL, 0xffe<<4, 15, rex, NULL, 0, D);  \
        unscaled = 0;                                                                                   \
    }

// Get EX as a quad, (x1 is used)
#define GETEX(a, w, D)                                                                                              \
    if (MODREG) {                                                                                                   \
        a = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), w);                                            \
    } else {                                                                                                        \
        SMREAD();                                                                                                   \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff << 4, 15, rex, NULL, 0, D); \
        a = fpu_get_scratch(dyn, ninst);                                                                            \
        VLD128(a, ed, fixedaddress);                                                                                \
    }

// Put Back EX if it was a memory and not an emm register
#define PUTEX(a)                     \
    if (!MODREG) {                   \
        VST128(a, ed, fixedaddress); \
        SMWRITE2();                  \
    }


// Get Ex as a double, not a quad (warning, x1 get used)
#define GETEXSD(a, w, D)                                                                                           \
    if (MODREG) {                                                                                                  \
        a = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), w);                                           \
    } else {                                                                                                       \
        SMREAD();                                                                                                  \
        a = fpu_get_scratch(dyn, ninst);                                                                           \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff << 3, 7, rex, NULL, 0, D); \
        VLD64(a, ed, fixedaddress);                                                                                \
    }

// Get Ex as 64bits, not a quad (warning, x1 get used)
#define GETEX64(a, w, D) GETEXSD(a, w, D)

// Get Ex as a single, not a quad (warning, x1 get used)
#define GETEXSS(a, w, D)                                                                                           \
    if (MODREG) {                                                                                                  \
        a = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), w);                                           \
    } else {                                                                                                       \
        SMREAD();                                                                                                  \
        a = fpu_get_scratch(dyn, ninst);                                                                           \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff << 2, 3, rex, NULL, 0, D); \
        VLD32(a, ed, fixedaddress);                                                                                \
    }

// Get Ex as 32bits, not a quad (warning, x1 get used)
#define GETEX32(a, w, D)    GETEXSS(a, w, D)

// Get Ex as 16bits, not a quad (warning, x1 get used)
#define GETEX16(a, w, D)                                                                                           \
    if (MODREG) {                                                                                                  \
        a = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), w);                                           \
    } else {                                                                                                       \
        SMREAD();                                                                                                  \
        a = fpu_get_scratch(dyn, ninst);                                                                           \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff << 1, 1, rex, NULL, 0, D); \
        VLD16(a, ed, fixedaddress);                                                                                \
    }

// Get GM, might use x1, x2 and x3
#define GETGM(a)                 \
    gd = ((nextop & 0x38) >> 3); \
    a = mmx_get_reg(dyn, ninst, x1, x2, x3, gd)

// Get EM, might use x1, x2 and x3
#define GETEM(a, D)                                             \
    if(MODREG) {                                                \
        a = mmx_get_reg(dyn, ninst, x1, x2, x3, (nextop&7));    \
    } else {                                                    \
        SMREAD();                                               \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<3, 7, rex, NULL, 0, D); \
        a = fpu_get_scratch(dyn, ninst);                        \
        VLD64(a, ed, fixedaddress);                             \
    }

// Put Back EM if it was a memory and not an emm register
#define PUTEM(a)                    \
    if (!MODREG) {                  \
        VST64(a, ed, fixedaddress); \
        SMWRITE2();                 \
    }

#define YMM0(a) ymm_mark_zero(dyn, ninst, a);

// Get Direction with size Z and based of F_DF flag, on register r ready for LDR/STR fetching
// F_DF is 1<<10, so 1 ROR 11*2 (so F_OF)
#define GETDIR(r, A)                \
    MOV32w(r, A); /* mask=1<<10 */  \
    TSTw_mask(xFlags, 0b010110, 0); \
    CNEGx(r, r, cNE)

#define ALIGNED_ATOMICxw ((fixedaddress && !(fixedaddress&(((1<<(2+rex.w))-1)))) || BOX64ENV(dynarec_aligned_atomics))
#define ALIGNED_ATOMICH ((fixedaddress && !(fixedaddress&1)) || BOX64ENV(dynarec_aligned_atomics))

// CALL will use x7 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL(F, ret) call_c(dyn, ninst, F, x87pc, ret, 1, 0)
// CALL_ will use x7 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL_(F, ret, reg) call_c(dyn, ninst, F, x87pc, ret, 1, reg)
// CALL_S will use x7 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2. Flags are not save/restored
#define CALL_S(F, ret) call_c(dyn, ninst, F, x87pc, ret, 0, 0)
// CALL_D will use x7 for the call address. Use for function having 1 or 2 double args and returning 1 double
// All regs are saved, including xmm0-7, will move arg1/arg2 to D0, D1, return D0 to ret (unless any is -1)
#define CALL_D(F, ret, arg1, arg2, sav1, sav2) call_d(dyn, ninst, F, ret, arg1, arg2, sav1, sav2)

#define MARK        dyn->insts[ninst].mark = dyn->native_size
#define GETMARK     dyn->insts[ninst].mark
#define MARK2       dyn->insts[ninst].mark2 = dyn->native_size
#define GETMARK2    dyn->insts[ninst].mark2
#define MARK3       dyn->insts[ninst].mark3 = dyn->native_size
#define GETMARK3    dyn->insts[ninst].mark3
#define MARKF       dyn->insts[ninst].markf = dyn->native_size
#define GETMARKF    dyn->insts[ninst].markf
#define MARKF2      dyn->insts[ninst].markf2 = dyn->native_size
#define GETMARKF2   dyn->insts[ninst].markf2
#define MARKSEG     dyn->insts[ninst].markseg = dyn->native_size
#define GETMARKSEG  dyn->insts[ninst].markseg
#define MARKLOCK    dyn->insts[ninst].marklock = dyn->native_size
#define GETMARKLOCK dyn->insts[ninst].marklock

// Branch to MARK if cond (use j64)
#define B_MARK(cond)                    \
    j64 = GETMARK-(dyn->native_size);   \
    Bcond(cond, j64)
// Branch to MARK unconditionnal (use j64)
#define B_MARK_nocond                   \
    j64 = GETMARK-(dyn->native_size);   \
    B(j64)
// Branch to MARK if reg is 0 (use j64)
#define CBZw_MARK(reg)                  \
    j64 = GETMARK-(dyn->native_size);   \
    CBZw(reg, j64)
// Branch to MARK if reg is 0 (use j64)
#define CBZx_MARK(reg)                  \
    j64 = GETMARK-(dyn->native_size);   \
    CBZx(reg, j64)
// Branch to MARK if reg is 0 (use j64)
#define CBZxw_MARK(reg)                 \
    j64 = GETMARK-(dyn->native_size);   \
    CBZxw(reg, j64)
// Branch to MARK if reg is not 0 (use j64)
#define CBNZx_MARK(reg)                 \
    j64 = GETMARK-(dyn->native_size);   \
    CBNZx(reg, j64)
// Branch to MARK if reg is not 0 (use j64)
#define CBNZxw_MARK(reg)                \
    j64 = GETMARK-(dyn->native_size);   \
    CBNZxw(reg, j64)
// Branch to MARK if reg is not 0 (use j64)
#define CBNZw_MARK(reg)                 \
    j64 = GETMARK-(dyn->native_size);   \
    CBNZw(reg, j64)
// Test bit N of A and branch to MARK if not set
#define TBZ_MARK(A, N)                  \
    j64 = GETMARK-(dyn->native_size);   \
    TBZ(A, N, j64)
// Test bit N of A and branch to MARK if set
#define TBNZ_MARK(A, N)                 \
    j64 = GETMARK-(dyn->native_size);   \
    TBNZ(A, N, j64)
// Branch to MARK2 if cond (use j64)
#define B_MARK2(cond)                   \
    j64 = GETMARK2-(dyn->native_size);  \
    Bcond(cond, j64)
// Branch to MARK2 unconditionnal (use j64)
#define B_MARK2_nocond                  \
    j64 = GETMARK2-(dyn->native_size);  \
    B(j64)
// Branch to MARK2 if reg is 0 (use j64)
#define CBZx_MARK2(reg)                 \
    j64 = GETMARK2-(dyn->native_size);  \
    CBZx(reg, j64)
// Branch to MARK2 if reg is not 0 (use j64)
#define CBNZx_MARK2(reg)                \
    j64 = GETMARK2-(dyn->native_size);  \
    CBNZx(reg, j64)
// Branch to MARK2 if reg is not 0 (use j64)
#define CBNZw_MARK2(reg)                \
    j64 = GETMARK2-(dyn->native_size);  \
    CBNZw(reg, j64)
#define CBNZxw_MARK2(reg)               \
    j64 = GETMARK2-(dyn->native_size);  \
    CBNZxw(reg, j64)
// Test bit N of A and branch to MARK2 if set
#define TBNZ_MARK2(A, N)                \
    j64 = GETMARK2-(dyn->native_size);  \
    TBNZ(A, N, j64)
// Branch to MARK3 if cond (use j64)
#define B_MARK3(cond)                   \
    j64 = GETMARK3-(dyn->native_size);  \
    Bcond(cond, j64)
// Test bit N of A and branch to MARK3 if not set
#define TBZ_MARK2(A, N)                 \
    j64 = GETMARK2-(dyn->native_size);  \
    TBZ(A, N, j64)
// Branch to MARK3 unconditionnal (use j64)
#define B_MARK3_nocond                  \
    j64 = GETMARK3-(dyn->native_size);  \
    B(j64)
// Branch to MARK3 if reg is not 0 (use j64)
#define CBNZx_MARK3(reg)                \
    j64 = GETMARK3-(dyn->native_size);  \
    CBNZx(reg, j64)
// Branch to MARK3 if reg is not 0 (use j64)
#define CBNZw_MARK3(reg)                \
    j64 = GETMARK3-(dyn->native_size);  \
    CBNZw(reg, j64)
// Branch to MARK3 if reg is 0 (use j64)
#define CBZx_MARK3(reg)                 \
    j64 = GETMARK3-(dyn->native_size);  \
    CBZx(reg, j64)
// Branch to MARK3 if reg is 0 (use j64)
#define CBZw_MARK3(reg)                 \
    j64 = GETMARK3-(dyn->native_size);  \
    CBZw(reg, j64)
// Test bit N of A and branch to MARK3 if not set
#define TBZ_MARK3(A, N)                 \
    j64 = GETMARK3-(dyn->native_size);  \
    TBZ(A, N, j64)
// Test bit N of A and branch to MARK3 if set
#define TBNZ_MARK3(A, N)                \
    j64 = GETMARK3-(dyn->native_size);  \
    TBNZ(A, N, j64)
// Branch to next instruction if cond (use j64)
#define B_NEXT(cond)     \
    j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0; \
    Bcond(cond, j64)
// Branch to next instruction unconditionnal (use j64)
#define B_NEXT_nocond                                               \
    j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0;\
    B(j64)
// Branch to next instruction if reg is 0 (use j64)
#define CBZw_NEXT(reg)    \
    j64 =  (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0; \
    CBZw(reg, j64)
// Branch to next instruction if reg is 0 (use j64)
#define CBZx_NEXT(reg)    \
    j64 =  (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0; \
    CBZx(reg, j64)
// Branch to next instruction if reg is 0 (use j64)
#define CBZxw_NEXT(reg)    \
    j64 =  (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0; \
    CBZxw(reg, j64)
// Branch to next instruction if reg is not 0 (use j64)
#define CBNZw_NEXT(reg)   \
    j64 =  (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0; \
    CBNZw(reg, j64)
// Branch to next instruction if reg is not 0 (use j64)
#define CBNZx_NEXT(reg)   \
    j64 =  (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0; \
    CBNZx(reg, j64)
// Test bit N of A and branch to next instruction if not set
#define TBZ_NEXT(A, N)              \
    j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0; \
    TBZ(A, N, j64)
// Test bit N of A and branch to next instruction if set
#define TBNZ_NEXT(A, N)             \
    j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0; \
    TBNZ(A, N, j64)
// Branch to MARKSEG if cond (use j64)
#define B_MARKSEG(cond)    \
    j64 = GETMARKSEG-(dyn->native_size);   \
    Bcond(cond, j64)
// Branch to MARKSEG if reg is 0 (use j64)
#define CBZw_MARKSEG(reg)    \
    j64 = GETMARKSEG-(dyn->native_size);   \
    CBZw(reg, j64)
// Branch to MARKSEG if reg is not 0 (use j64)
#define CBNZw_MARKSEG(reg)              \
    j64 = GETMARKSEG-(dyn->native_size);   \
    CBNZw(reg, j64)
// Branch to MARKLOCK if cond (use j64)
#define B_MARKLOCK(cond)    \
    j64 = GETMARKLOCK-(dyn->native_size);   \
    Bcond(cond, j64)
// Branch to MARKLOCK unconditional (use j64)
#define B_MARKLOCK_nocond    \
    j64 = GETMARKLOCK-(dyn->native_size);   \
    B(j64)
// Branch to MARKLOCK if reg is not 0 (use j64)
#define CBNZw_MARKLOCK(reg)             \
    j64 = GETMARKLOCK-(dyn->native_size);  \
    CBNZw(reg, j64)
// Branch to MARKLOCK if reg is not 0 (use j64)
#define CBNZx_MARKLOCK(reg)             \
    j64 = GETMARKLOCK-(dyn->native_size);  \
    CBNZx(reg, j64)
// Branch to MARKLOCK if reg is 0 (use j64)
#define CBZx_MARKLOCK(reg)             \
    j64 = GETMARKLOCK-(dyn->native_size);  \
    CBZx(reg, j64)

#ifndef IFNATIVE
#define IFNATIVE(A)     if(dyn->insts[ninst].need_nat_flags&(A))
#define IFNATIVEN(A)    if((dyn->insts[ninst].need_nat_flags&(A))==(A))
#define IFNATIVE_BEFORE(A)     if(dyn->insts[ninst].before_nat_flags&(A))
#endif

#ifndef IFX
#define IFX(A)  if((dyn->insts[ninst].x64.gen_flags&(A)))
#define IFX2(A, B)  if((dyn->insts[ninst].x64.gen_flags&(A)) B)
#define IFX_PENDOR0  if((dyn->insts[ninst].x64.gen_flags&(X_PEND) || !dyn->insts[ninst].x64.gen_flags))
#define IFXX(A) if((dyn->insts[ninst].x64.gen_flags==(A)))
#define IFX2X(A, B) if((dyn->insts[ninst].x64.gen_flags==(A) || dyn->insts[ninst].x64.gen_flags==(B) || dyn->insts[ninst].x64.gen_flags==((A)|(B))))
#define IFXN(A, B)  if((dyn->insts[ninst].x64.gen_flags&(A) && !(dyn->insts[ninst].x64.gen_flags&(B))))
#define IFXNATIVE(X, N) if((dyn->insts[ninst].x64.gen_flags&(X)) && (dyn->insts[ninst].need_nat_flags&(N)))
#endif
#ifndef INVERTED_CARRY
#define INVERTED_CARRY          !dyn->insts[ninst].normal_carry
#define INVERTED_CARRY_BEFORE   !dyn->insts[ninst].normal_carry_before
#endif
#ifndef GEN_INVERTED_CARRY
#define GEN_INVERTED_CARRY()
#endif
#ifndef INVERT_CARRY
#define INVERT_CARRY(A)     if(dyn->insts[ninst].normal_carry) {if(cpuext.flagm) CFINV(); else {MRS_nzcv(A); EORx_mask(A, A, 1, 35, 0); MSR_nzcv(A);}}
#endif

// Generate FCOM with s1 and s2 scratch regs (the VCMP is already done)
#define FCOM(s1, s2, s3)                                                    \
    LDRH_U12(s3, xEmu, offsetof(x64emu_t, sw));   /*offset is 8bits right?*/\
    MOV32w(s1, 0b0100011100000000);                                         \
    BICw_REG(s3, s3, s1);                                                   \
    /* greater than leave 0 */                                              \
    CSETw(s1, cMI); /* 1 if less than, 0 else */                            \
    MOV32w(s2, 0b01000000); /* zero */                                      \
    CSELw(s1, s2, s1, cEQ);                                                 \
    MOV32w(s2, 0b01000101); /* unordered */                                 \
    CSELw(s1, s2, s1, cVS);                                                 \
    ORRw_REG_LSL(s3, s3, s1, 8);                                            \
    STRH_U12(s3, xEmu, offsetof(x64emu_t, sw))

// Generate FCOMI with s1 and s2 scratch regs (the VCMP is already done)
#define FCOMI(s1, s2)                                                       \
    IFX(X_OF|X_AF|X_SF) {                                                   \
        MOV32w(s2, 0b100011010101);                                         \
        BICw_REG(xFlags, xFlags, s2);                                       \
        IFX(X_CF|X_PF|X_ZF) {                                               \
            MOV32w(s2, 0b01000101);                                         \
        }                                                                   \
    } else {                                                                \
        IFX(X_CF|X_PF|X_ZF) {                                               \
            MOV32w(s2, 0b01000101);                                         \
            BICw_REG(xFlags, xFlags, s2);                                   \
        }                                                                   \
    }                                                                       \
    IFX(X_CF|X_PF|X_ZF) {                                                   \
        CSETw(s1, cMI); /* 1 if less than, 0 else */                        \
        /*s2 already set */     /* unordered */                             \
        CSELw(s1, s2, s1, cVS);                                             \
        MOV32w(s2, 0b01000000); /* zero */                                  \
        CSELw(s1, s2, s1, cEQ);                                             \
        /* greater than leave 0 */                                          \
        ORRw_REG(xFlags, xFlags, s1);                                       \
    }                                                                       \
    SET_DFNONE();                                                           \

#ifndef IF_UNALIGNED
#define IF_UNALIGNED(A)    if(dyn->insts[ninst].unaligned)
#endif

#ifndef IF_ALIGNED
#define IF_ALIGNED(A) if (!dyn->insts[ninst].unaligned)
#endif

#ifndef CALLRET_RET
#define CALLRET_RET()   NOP
#endif
#ifndef CALLRET_GETRET
#define CALLRET_GETRET()    (dyn->callrets?(dyn->callrets[dyn->callret_size].offs-dyn->native_size):0)
#endif
#ifndef CALLRET_LOOP
#define CALLRET_LOOP()  NOP
#endif

#ifndef NATIVE_RESTORE_X87PC
#define NATIVE_RESTORE_X87PC()                          \
    if(dyn->need_x87check) {                            \
        LDRH_U12(x87pc, xEmu, offsetof(x64emu_t, cw));  \
        UBFXw(x87pc, x87pc, 8, 2);                      \
    }
#endif
#ifndef X87_CHECK_PRECISION
#define X87_CHECK_PRECISION(A)               \
    if (!ST_IS_F(0) && dyn->need_x87check) { \
        CBNZw(x87pc, 4 + 8);                 \
        FCVT_S_D(A, A);                      \
        FCVT_D_S(A, A);                      \
    }
#endif

#define STORE_REG(A)    STRx_U12(x##A, xEmu, offsetof(x64emu_t, regs[_##A]))
#define STP_REGS(A, B)  STPx_S7_offset(x##A, x##B, xEmu, offsetof(x64emu_t, regs[_##A]))
#define LDP_REGS(A, B)  LDPx_S7_offset(x##A, x##B, xEmu, offsetof(x64emu_t, regs[_##A]))
#define STORE_XEMU_REGS(A)  \
    STP_REGS(RAX, RCX);     \
    STP_REGS(RDX, RBX);     \
    STP_REGS(RSP, RBP);     \
    STP_REGS(RSI, RDI);     \
    STP_REGS(R8, R9);       \
    STP_REGS(R10, R11);     \
    STP_REGS(R12, R13);     \
    STP_REGS(R14, R15);     \
    if(A==xRIP) {           \
        STPx_S7_offset(xFlags, xRIP, xEmu, offsetof(x64emu_t, eflags)); \
    } else {                \
        STRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags)); \
        if(A) {STRx_U12(A, xEmu, offsetof(x64emu_t, ip));}  \
    }

#define LOAD_REG(A)    LDRx_U12(x##A, xEmu, offsetof(x64emu_t, regs[_##A]))
#define LOAD_XEMU_REGS(A)   \
    LDP_REGS(RAX, RCX);     \
    LDP_REGS(RDX, RBX);     \
    LDP_REGS(RSP, RBP);     \
    LDP_REGS(RSI, RDI);     \
    LDP_REGS(R8, R9);       \
    LDP_REGS(R10, R11);     \
    LDP_REGS(R12, R13);     \
    LDP_REGS(R14, R15);     \
    if(A==xRIP) {           \
        LDPx_S7_offset(xFlags, xRIP, xEmu, offsetof(x64emu_t, eflags)); \
    } else {                \
        LDRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags)); \
        if(A) {LDRx_U12(A, xEmu, offsetof(x64emu_t, ip));}  \
    }

#define STORE_XEMU_MINIMUM(A)  \
    STORE_REG(RAX);         \
    STORE_REG(RCX);         \
    STORE_REG(RDX);         \
    STORE_REG(RBX);         \
    STORE_REG(RSP);         \
    STORE_REG(RBP);         \
    STORE_REG(RSI);         \
    STORE_REG(RDI);         \
    STORE_REG(R8);          \
    STORE_REG(R9);          \
    STRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags)); \
    if(A) {STRx_U12(A, xEmu, offsetof(x64emu_t, ip));}

// Need to also store current value of some register, as they may be used by functions like setjump
// so RBX, RSP, RBP, R12..R15 (other are scratch or parameters), R10-R11 not usefull, but why not
// RBX, RSP and RBP are already saved in call function
#define STORE_XEMU_CALL(A)  \
    STP_REGS(R10, R11);     \
    STP_REGS(R12, R13);     \
    STP_REGS(R14, R15);     \
    if(A) {STPx_S7_offset(xFlags, A, xEmu, offsetof(x64emu_t, eflags));}    \
    else {STRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));}

#define LOAD_XEMU_CALL(A)   \
    if(A) {LDPx_S7_offset(xFlags, A, xEmu, offsetof(x64emu_t, eflags));}    \
    else {LDRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));};             \
    if(A==xRIP) dyn->last_ip = 0

#define LOAD_XEMU_REM()     \
    LDP_REGS(R10, R11);     \
    LDP_REGS(R12, R13);     \
    LDP_REGS(R14, R15)

#if STEP == 0
#define X87_PUSH_OR_FAIL(var, dyn, ninst, scratch, t)   var = x87_do_push(dyn, ninst, scratch, t)
#define X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, scratch)     x87_do_push_empty(dyn, ninst, scratch)
#define X87_POP_OR_FAIL(dyn, ninst, scratch)            x87_do_pop(dyn, ninst, scratch)
#else
#define X87_PUSH_OR_FAIL(var, dyn, ninst, scratch, t)   \
    if ((dyn->n.x87stack==8) || (dyn->n.pushed==8)) {   \
        if(dyn->need_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Push, stack=%d/%d on inst %d\n", dyn->n.x87stack, dyn->n.pushed, ninst); \
        dyn->abort = 1;                                 \
        return addr;                                    \
    }                                                   \
    var = x87_do_push(dyn, ninst, scratch, t)

#define X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, scratch)     \
    if ((dyn->n.x87stack==8) || (dyn->n.pushed==8)) {   \
        if(dyn->need_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Push, stack=%d/%d on inst %d\n", dyn->n.x87stack, dyn->n.pushed, ninst); \
        dyn->abort = 1;                                 \
        return addr;                                    \
    }                                                   \
    x87_do_push_empty(dyn, ninst, scratch)

#define X87_POP_OR_FAIL(dyn, ninst, scratch)            \
    if ((dyn->n.x87stack==-8) || (dyn->n.poped==8)) {   \
        if(dyn->need_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Pop, stack=%d/%d on inst %d\n", dyn->n.x87stack, dyn->n.poped, ninst); \
        dyn->abort = 1;                                 \
        return addr;                                    \
    }                                                   \
    x87_do_pop(dyn, ninst, scratch)
#endif

#define FORCE_DFNONE()  STRw_U12(wZR, xEmu, offsetof(x64emu_t, df))
#define CHECK_DFNONE(N)  do {if(dyn->f==status_none_pending) {FORCE_DFNONE(); if(N) dyn->f = status_none;}} while(0)

#define SET_DFNONE()                                                    \
    do {                                                                \
        if(!dyn->insts[ninst].x64.may_set && (dyn->f!=status_none)) {   \
            dyn->f = status_none_pending;                               \
        }                                                               \
    } while(0)

#define SET_DF(S, N)                                                                                                            \
    if ((N) != d_none) {                                                                                                        \
        MOVZw(S, (N));                                                                                                          \
        STRw_U12(S, xEmu, offsetof(x64emu_t, df));                                                                              \
        dyn->f = status_set;                                                                                                    \
    } else                                                                                                                      \
        SET_DFNONE()

#ifndef READFLAGS
#define READFLAGS(A) \
    if((A)!=X_PEND                                          \
    && (dyn->f==status_unk)) {                              \
        TABLE64C(x6, const_updateflags_arm64);              \
        BLR(x6);                                            \
        dyn->f = status_none;                               \
    }
#endif

#ifndef SETFLAGS
#define SETFLAGS(A, B)                                                                          \
    if (((B)&SF_SUB)                                                                            \
    && (dyn->insts[ninst].x64.gen_flags&(~(A))))                                                \
        READFLAGS(((dyn->insts[ninst].x64.gen_flags&X_PEND)?X_ALL:dyn->insts[ninst].x64.gen_flags)&(~(A)));\
    if(dyn->insts[ninst].x64.gen_flags) switch(B) {                                             \
        case SF_SET_DF: dyn->f = status_set; break;                                             \
        case SF_SET_NODF: SET_DFNONE(); break;                                                  \
        case SF_SUBSET:                                                                         \
        case SF_SUBSET_PENDING:                                                                 \
        case SF_SET:                                                                            \
        case SF_PENDING:                                                                        \
        case SF_SET_PENDING:                                                                    \
            SET_DFNONE(); break;                                                                \
    } else SET_DFNONE()
#endif
#ifndef JUMP
#define JUMP(A, C) SMEND()
#endif
#ifndef BARRIER
#define BARRIER(A)
#endif
#ifndef SET_HASCALLRET
#define SET_HASCALLRET()
#endif
#define UFLAG_OP1(A) if(dyn->insts[ninst].x64.gen_flags) {STRxw_U12(A, xEmu, offsetof(x64emu_t, op1));}
#define UFLAG_OP2(A) if(dyn->insts[ninst].x64.gen_flags) {STRxw_U12(A, xEmu, offsetof(x64emu_t, op2));}
#define UFLAG_OP12(A1, A2) if(dyn->insts[ninst].x64.gen_flags) {STRxw_U12(A1, xEmu, offsetof(x64emu_t, op1));STRxw_U12(A2, xEmu, offsetof(x64emu_t, op2));}
#define UFLAG_RES(A) if(dyn->insts[ninst].x64.gen_flags) {STRxw_U12(A, xEmu, offsetof(x64emu_t, res));}
#define UFLAG_DF(r, A) if(dyn->insts[ninst].x64.gen_flags) {SET_DF(r, A);}
#ifndef UFLAG_IF
#define UFLAG_IF if(dyn->insts[ninst].x64.gen_flags)
#endif
#ifndef UFLAG_IF2
#define UFLAG_IF2(A) if(dyn->insts[ninst].x64.gen_flags A)
#endif
#ifndef DEFAULT
#define DEFAULT      *ok = -1; BARRIER(2)
#endif
#ifndef NEW_BARRIER_INST
#define NEW_BARRIER_INST
#endif
#ifndef TABLE64
#define TABLE64(A, V)
#endif
#ifndef TABLE64_
#define TABLE64_(A, V)
#endif
#ifndef FTABLE64
#define FTABLE64(A, V)
#endif
#ifndef TABLE64C
#define TABLE64C(A, V)
#endif

#define ARCH_INIT()      \
    SMSTART();           \
    dyn->doublepush = 0; \
    dyn->doublepop = 0;
#define ARCH_RESET()

#if STEP < 2
#define GETIP(A) MOV64x(xRIP, A)
#define GETIP_(A) MOV64x(xRIP, A)
#else
// put value in the Table64 even if not using it for now to avoid difference between Step2 and Step3. Needs to be optimized later...
#define GETIP(A)                                            \
    if(dyn->last_ip && ((A)-dyn->last_ip)<0x1000) {         \
        uint64_t _delta_ip = (A)-dyn->last_ip;              \
        dyn->last_ip += _delta_ip;                          \
        if(_delta_ip) {                                     \
            ADDx_U12(xRIP, xRIP, _delta_ip);                \
        }                                                   \
    } else if(dyn->last_ip && (dyn->last_ip-(A))<0x1000) {  \
        uint64_t _delta_ip = dyn->last_ip-(A);              \
        dyn->last_ip -= _delta_ip;                          \
        if(_delta_ip) {                                     \
            SUBx_U12(xRIP, xRIP, _delta_ip);                \
        }                                                   \
    } else {                                                \
        dyn->last_ip = (A);                                 \
        if(dyn->need_reloc) {                               \
            TABLE64(xRIP, dyn->last_ip);                    \
        } else {                                            \
            MOV64x(xRIP, dyn->last_ip);                     \
        }                                                   \
    }
#define GETIP_(A)                                           \
    if(dyn->last_ip && ((A)-dyn->last_ip)<0x1000) {         \
        uint64_t _delta_ip = (A)-dyn->last_ip;              \
        if(_delta_ip) {ADDx_U12(xRIP, xRIP, _delta_ip);}    \
    } else if(dyn->last_ip && (dyn->last_ip-(A))<0x1000) {  \
        uint64_t _delta_ip = dyn->last_ip-(A);              \
        if(_delta_ip) {SUBx_U12(xRIP, xRIP, _delta_ip);}    \
    } else {                                                \
        if(dyn->need_reloc) {                               \
            TABLE64(xRIP, (A));                             \
        } else {                                            \
            MOV64x(xRIP, (A));                              \
        }                                                   \
    }
#endif
#define CLEARIP()   dyn->last_ip=0

#define SKIP_SEVL(val) if (dyn->insts[dyn->insts[ninst].x64.jmp_insts].wfe) val += 4;

#if STEP < 2
#define PASS2IF(A, B) if(A)
#elif STEP == 2
#define PASS2IF(A, B) if(A) dyn->insts[ninst].pass2choice = B; if(dyn->insts[ninst].pass2choice == B)
#else
#define PASS2IF(A, B) if(dyn->insts[ninst].pass2choice == B)
#endif

#define MODREG  ((nextop&0xC0)==0xC0)

#ifndef STEPNAME
#define STEPNAME3(N,M) N##M
#define STEPNAME2(N,M) STEPNAME3(N,M)
#define STEPNAME(N) STEPNAME2(N, STEP)
#endif

#define native_pass        STEPNAME(native_pass)

#define updateflags_pass   STEPNAME(updateflags_pass)

#define dynarec64_00       STEPNAME(dynarec64_00)
#define dynarec64_0F       STEPNAME(dynarec64_0F)
#define dynarec64_66       STEPNAME(dynarec64_66)
#define dynarec64_D8       STEPNAME(dynarec64_D8)
#define dynarec64_D9       STEPNAME(dynarec64_D9)
#define dynarec64_DA       STEPNAME(dynarec64_DA)
#define dynarec64_DB       STEPNAME(dynarec64_DB)
#define dynarec64_DC       STEPNAME(dynarec64_DC)
#define dynarec64_DD       STEPNAME(dynarec64_DD)
#define dynarec64_DE       STEPNAME(dynarec64_DE)
#define dynarec64_DF       STEPNAME(dynarec64_DF)
#define dynarec64_F0       STEPNAME(dynarec64_F0)
#define dynarec64_660F     STEPNAME(dynarec64_660F)
#define dynarec64_66F20F   STEPNAME(dynarec64_66F20F)
#define dynarec64_66F30F   STEPNAME(dynarec64_66F30F)
#define dynarec64_66F0     STEPNAME(dynarec64_66F0)
#define dynarec64_F20F     STEPNAME(dynarec64_F20F)
#define dynarec64_F30F     STEPNAME(dynarec64_F30F)
#define dynarec64_AVX      STEPNAME(dynarec64_AVX)
#define dynarec64_AVX_0F   STEPNAME(dynarec64_AVX_0F)
#define dynarec64_AVX_0F38 STEPNAME(dynarec64_AVX_0F38)
#define dynarec64_AVX_66_0F     STEPNAME(dynarec64_AVX_66_0F)
#define dynarec64_AVX_F2_0F     STEPNAME(dynarec64_AVX_F2_0F)
#define dynarec64_AVX_F3_0F     STEPNAME(dynarec64_AVX_F3_0F)
#define dynarec64_AVX_66_0F38   STEPNAME(dynarec64_AVX_66_0F38)
#define dynarec64_AVX_66_0F3A   STEPNAME(dynarec64_AVX_66_0F3A)
#define dynarec64_AVX_F2_0F38   STEPNAME(dynarec64_AVX_F2_0F38)
#define dynarec64_AVX_F2_0F3A   STEPNAME(dynarec64_AVX_F2_0F3A)
#define dynarec64_AVX_F3_0F38   STEPNAME(dynarec64_AVX_F3_0F38)

#define geted           STEPNAME(geted)
#define geted16         STEPNAME(geted16)
#define jump_to_epilog  STEPNAME(jump_to_epilog)
#define jump_to_next    STEPNAME(jump_to_next)
#define ret_to_epilog   STEPNAME(ret_to_epilog)
#define retn_to_epilog  STEPNAME(retn_to_epilog)
#define iret_to_epilog  STEPNAME(iret_to_epilog)
#define call_c          STEPNAME(call_c)
#define call_d          STEPNAME(call_d)
#define call_n          STEPNAME(call_n)
#define grab_segdata    STEPNAME(grab_segdata)
#define emit_cmp8       STEPNAME(emit_cmp8)
#define emit_cmp16      STEPNAME(emit_cmp16)
#define emit_cmp32      STEPNAME(emit_cmp32)
#define emit_cmp8_0     STEPNAME(emit_cmp8_0)
#define emit_cmp16_0    STEPNAME(emit_cmp16_0)
#define emit_cmp32_0    STEPNAME(emit_cmp32_0)
#define emit_test8      STEPNAME(emit_test8)
#define emit_test8c     STEPNAME(emit_test8c)
#define emit_test16     STEPNAME(emit_test16)
#define emit_test16c    STEPNAME(emit_test16c)
#define emit_test32     STEPNAME(emit_test32)
#define emit_test32c    STEPNAME(emit_test32c)
#define emit_add32      STEPNAME(emit_add32)
#define emit_add32c     STEPNAME(emit_add32c)
#define emit_add8       STEPNAME(emit_add8)
#define emit_add8c      STEPNAME(emit_add8c)
#define emit_sub32      STEPNAME(emit_sub32)
#define emit_sub32c     STEPNAME(emit_sub32c)
#define emit_sub8       STEPNAME(emit_sub8)
#define emit_sub8c      STEPNAME(emit_sub8c)
#define emit_or32       STEPNAME(emit_or32)
#define emit_or32c      STEPNAME(emit_or32c)
#define emit_xor32      STEPNAME(emit_xor32)
#define emit_xor32c     STEPNAME(emit_xor32c)
#define emit_and32      STEPNAME(emit_and32)
#define emit_and32c     STEPNAME(emit_and32c)
#define emit_or8        STEPNAME(emit_or8)
#define emit_or8c       STEPNAME(emit_or8c)
#define emit_xor8       STEPNAME(emit_xor8)
#define emit_xor8c      STEPNAME(emit_xor8c)
#define emit_and8       STEPNAME(emit_and8)
#define emit_and8c      STEPNAME(emit_and8c)
#define emit_add16      STEPNAME(emit_add16)
#define emit_add16c     STEPNAME(emit_add16c)
#define emit_sub16      STEPNAME(emit_sub16)
#define emit_sub16c     STEPNAME(emit_sub16c)
#define emit_or16       STEPNAME(emit_or16)
#define emit_or16c      STEPNAME(emit_or16c)
#define emit_xor16      STEPNAME(emit_xor16)
#define emit_xor16c     STEPNAME(emit_xor16c)
#define emit_and16      STEPNAME(emit_and16)
#define emit_and16c     STEPNAME(emit_and16c)
#define emit_inc32      STEPNAME(emit_inc32)
#define emit_inc16      STEPNAME(emit_inc16)
#define emit_inc8       STEPNAME(emit_inc8)
#define emit_dec32      STEPNAME(emit_dec32)
#define emit_dec16      STEPNAME(emit_dec16)
#define emit_dec8       STEPNAME(emit_dec8)
#define emit_adc32      STEPNAME(emit_adc32)
#define emit_adc32c     STEPNAME(emit_adc32c)
#define emit_adc8       STEPNAME(emit_adc8)
#define emit_adc8c      STEPNAME(emit_adc8c)
#define emit_adc16      STEPNAME(emit_adc16)
#define emit_adc16c     STEPNAME(emit_adc16c)
#define emit_sbb32      STEPNAME(emit_sbb32)
#define emit_sbb32c     STEPNAME(emit_sbb32c)
#define emit_sbb8       STEPNAME(emit_sbb8)
#define emit_sbb8c      STEPNAME(emit_sbb8c)
#define emit_sbb16      STEPNAME(emit_sbb16)
#define emit_sbb16c     STEPNAME(emit_sbb16c)
#define emit_neg32      STEPNAME(emit_neg32)
#define emit_neg16      STEPNAME(emit_neg16)
#define emit_neg8       STEPNAME(emit_neg8)
#define emit_shl32      STEPNAME(emit_shl32)
#define emit_shl32c     STEPNAME(emit_shl32c)
#define emit_shr32      STEPNAME(emit_shr32)
#define emit_shr32c     STEPNAME(emit_shr32c)
#define emit_sar32      STEPNAME(emit_sar32)
#define emit_sar32c     STEPNAME(emit_sar32c)
#define emit_shl8       STEPNAME(emit_shl8)
#define emit_shl8c      STEPNAME(emit_shl8c)
#define emit_shr8       STEPNAME(emit_shr8)
#define emit_shr8c      STEPNAME(emit_shr8c)
#define emit_sar8       STEPNAME(emit_sar8)
#define emit_sar8c      STEPNAME(emit_sar8c)
#define emit_shl16      STEPNAME(emit_shl16)
#define emit_shl16c     STEPNAME(emit_shl16c)
#define emit_shr16      STEPNAME(emit_shr16)
#define emit_shr16c     STEPNAME(emit_shr16c)
#define emit_sar16      STEPNAME(emit_sar16)
#define emit_sar16c     STEPNAME(emit_sar16c)
#define emit_rol32c     STEPNAME(emit_rol32c)
#define emit_ror32c     STEPNAME(emit_ror32c)
#define emit_rol8c      STEPNAME(emit_rol8c)
#define emit_ror8c      STEPNAME(emit_ror8c)
#define emit_rol16c     STEPNAME(emit_rol16c)
#define emit_ror16c     STEPNAME(emit_ror16c)
#define emit_rcl8c      STEPNAME(emit_rcl8c)
#define emit_rcr8c      STEPNAME(emit_rcr8c)
#define emit_rcl16c     STEPNAME(emit_rcl16c)
#define emit_rcr16c     STEPNAME(emit_rcr16c)
#define emit_rcl32c     STEPNAME(emit_rcl32c)
#define emit_rcr32c     STEPNAME(emit_rcr32c)
#define emit_shrd32c    STEPNAME(emit_shrd32c)
#define emit_shrd32     STEPNAME(emit_shrd32)
#define emit_shld32c    STEPNAME(emit_shld32c)
#define emit_shld32     STEPNAME(emit_shld32)
#define emit_shrd16c    STEPNAME(emit_shrd16c)
#define emit_shrd16     STEPNAME(emit_shrd16)
#define emit_shld16c    STEPNAME(emit_shld16c)
#define emit_shld16     STEPNAME(emit_shld16)

#define emit_pf         STEPNAME(emit_pf)

#define x87_do_push         STEPNAME(x87_do_push)
#define x87_do_push_empty   STEPNAME(x87_do_push_empty)
#define x87_do_pop          STEPNAME(x87_do_pop)
#define x87_get_current_cache   STEPNAME(x87_get_current_cache)
#define x87_get_cache   STEPNAME(x87_get_cache)
#define x87_get_neoncache STEPNAME(x87_get_neoncache)
#define x87_get_st      STEPNAME(x87_get_st)
#define x87_get_st_empty  STEPNAME(x87_get_st)
#define x87_free        STEPNAME(x87_free)
#define x87_forget      STEPNAME(x87_forget)
#define x87_reget_st    STEPNAME(x87_reget_st)
#define x87_stackcount  STEPNAME(x87_stackcount)
#define x87_unstackcount  STEPNAME(x87_unstackcount)
#define x87_swapreg     STEPNAME(x87_swapreg)
#define x87_setround    STEPNAME(x87_setround)
#define x87_restoreround STEPNAME(x87_restoreround)
#define sse_setround    STEPNAME(sse_setround)
#define mmx_get_reg     STEPNAME(mmx_get_reg)
#define mmx_get_reg_empty STEPNAME(mmx_get_reg_empty)
#define sse_get_reg     STEPNAME(sse_get_reg)
#define sse_get_reg_empty STEPNAME(sse_get_reg_empty)
#define sse_forget_reg    STEPNAME(sse_forget_reg)
#define sse_purge07cache  STEPNAME(sse_purge07cache)
#define sse_reflect_reg   STEPNAME(sse_reflect_reg)
#define ymm_get_reg       STEPNAME(ymm_get_reg)
#define ymm_get_reg_empty STEPNAME(ymm_get_reg_empty)
#define ymm_mark_zero     STEPNAME(ymm_mark_zero)
#define fpu_get_reg_ymm   STEPNAME(fpu_get_reg_ymm)
#define doPreload         STEPNAME(doPreload)
#define doEnterBlock      STEPNAME(doEnterBlock)
#define doLeaveBlock      STEPNAME(doLeaveBlock)

#define fpu_pushcache   STEPNAME(fpu_pushcache)
#define fpu_popcache    STEPNAME(fpu_popcache)
#define fpu_reset_cache STEPNAME(fpu_reset_cache)
#define fpu_propagate_stack STEPNAME(fpu_propagate_stack)
#define fpu_purgecache  STEPNAME(fpu_purgecache)
#define mmx_purgecache  STEPNAME(mmx_purgecache)
#define x87_purgecache  STEPNAME(x87_purgecache)
#define x87_reflectcount STEPNAME(x87_reflectcount)
#define x87_unreflectcount STEPNAME(x87_unreflectcount)
#define fpu_reflectcache STEPNAME(fpu_reflectcache)
#define fpu_unreflectcache STEPNAME(fpu_unreflectcache)
#define avx_purge_ymm   STEPNAME(avx_purge_ymm)

#define CacheTransform       STEPNAME(CacheTransform)

#define arm64_move32        STEPNAME(arm64_move32)
#define arm64_move64        STEPNAME(arm64_move64)

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int* unscaled, int absmax, uint32_t mask, rex_t rex, int* l, int s, int delta);

/* setup r2 to address pointed by */
uintptr_t geted16(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int* unscaled, int absmax, uint32_t mask, rex_t rex, int s);


// generic x64 helper
void jump_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_next(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst, int is32bits);
void ret_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int ninst, rex_t rex);
void retn_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int ninst, rex_t rex, int n);
void iret_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int ninst, int is32bits, int is64bits);
void call_c(dynarec_arm_t* dyn, int ninst, arm64_consts_t fnc, int reg, int ret, int saveflags, int save_reg);
void call_d(dynarec_arm_t* dyn, int ninst, arm64_consts_t fnc, int ret, int arg1, int arg2, int sav1, int sav2);
void call_n(dynarec_arm_t* dyn, int ninst, void* fnc, int w);
void grab_segdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg, int segment, int modreg);
void emit_cmp8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_cmp16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_cmp32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_cmp8_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp16_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp32_0(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
void emit_test8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_test8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_test16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_test16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4, int s5);
void emit_test32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_test32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5);
void emit_add32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_add32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5);
void emit_add8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_add8c(dynarec_arm_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4);
void emit_sub32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_sub32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5);
void emit_sub8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_sub8c(dynarec_arm_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4, int s5);
void emit_or32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_or32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_xor32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_xor32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_and32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_and32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_or8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_or8c(dynarec_arm_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4);
void emit_xor8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_xor8c(dynarec_arm_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4);
void emit_and8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_and8c(dynarec_arm_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4);
void emit_add16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_add16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sub16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_sub16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_or16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_or16c(dynarec_arm_t* dyn, int ninst, int s1, int16_t c, int s3, int s4);
void emit_xor16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_xor16c(dynarec_arm_t* dyn, int ninst, int s1, int16_t c, int s3, int s4);
void emit_and16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_and16c(dynarec_arm_t* dyn, int ninst, int s1, int16_t c, int s3, int s4);
void emit_inc32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
void emit_inc16(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_inc8(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_dec32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
void emit_dec16(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_dec8(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_adc32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_adc32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_adc8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_adc8c(dynarec_arm_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4, int s5);
void emit_adc16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_adc16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sbb32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_sbb32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sbb8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_sbb8c(dynarec_arm_t* dyn, int ninst, int s1, uint8_t c, int s3, int s4, int s5);
void emit_sbb16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_sbb16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_neg32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
void emit_neg16(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_neg8(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_shl32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_shl32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_shr32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_shr32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_sar32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_sar32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_shl8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_shl8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_shr8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_shr8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_sar8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_sar8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_shl16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_shl16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_shr16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_shr16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_sar16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_sar16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_rol32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_ror32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_rol8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_ror8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_rol16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_ror16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_rcl8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_rcr8c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_rcl16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_rcr16c(dynarec_arm_t* dyn, int ninst, int s1, uint32_t c, int s3, int s4);
void emit_rcl32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_rcr32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_shrd32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);
void emit_shld32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);
void emit_shrd32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4);
void emit_shld32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s5, int s3, int s4);
void emit_shrd16c(dynarec_arm_t* dyn, int ninst, int s1, int s2, uint32_t c, int s3, int s4);
void emit_shrd16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s5, int s3, int s4);
void emit_shld16c(dynarec_arm_t* dyn, int ninst, int s1, int s2, uint32_t c, int s3, int s4);
void emit_shld16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s5, int s3, int s4);

void emit_pf(dynarec_arm_t* dyn, int ninst, int s1, int s4);

// x87 helper
// cache of the local stack counter, to avoid update at every call, return old internal stack counter
int x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch);
// revert local stack counter to previous version (return from x87_stackcount)
void x87_unstackcount(dynarec_arm_t* dyn, int ninst, int scratch, int count);
// fpu push. Return the Dd value to be used
int x87_do_push(dynarec_arm_t* dyn, int ninst, int s1, int t);
// fpu push. Do not allocate a cache register. Needs a scratch register to do x87stack synch (or 0 to not do it)
void x87_do_push_empty(dynarec_arm_t* dyn, int ninst, int s1);
// fpu pop. All previous returned Dd should be considered invalid
void x87_do_pop(dynarec_arm_t* dyn, int ninst, int s1);
// get cache index for a x87 reg, return -1 if cache doesn't exist
int x87_get_current_cache(dynarec_arm_t* dyn, int ninst, int st, int t);
// get cache index for a x87 reg, create the entry if needed
int x87_get_cache(dynarec_arm_t* dyn, int ninst, int populate, int s1, int s2, int a, int t);
// get neoncache index for a x87 reg
int x87_get_neoncache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a);
// get vfpu register for a x87 reg, create the entry if needed
int x87_get_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int t);
// get vfpu register for a x87 reg, create the entry if needed. Do not fetch the Stx if not already in cache
int x87_get_st_empty(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int t);
// Free st, using the FFREE opcode (so it's freed but stack is not moved)
void x87_free(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int st);
// refresh a value from the cache ->emu and then forget the cache (nothing done if value is not cached)
void x87_forget(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st);
// refresh the cache value from emu
void x87_reget_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st);
// swap 2 x87 regs
void x87_swapreg(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int b);
// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
// Restore round flag
void x87_restoreround(dynarec_arm_t* dyn, int ninst, int s1);
// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
// purge ymm_zero mask according to purge_ymm
void avx_purge_ymm(dynarec_arm_t* dyn, int ninst, uint16_t mask, int s1);

void CacheTransform(dynarec_arm_t* dyn, int ninst, int cacheupd);

void arm64_move32(dynarec_arm_t* dyn, int ninst, int reg, uint32_t val);
void arm64_move64(dynarec_arm_t* dyn, int ninst, int reg, uint64_t val);

#if STEP < 2
#define CHECK_CACHE()   0
#elif STEP == 2
#define CHECK_CACHE()   (cacheupd = dyn->insts[ninst].cacheupd = CacheNeedsTransform(dyn, ninst))
#else
#define CHECK_CACHE()   (cacheupd = dyn->insts[ninst].cacheupd)
#endif

#define neoncache_st_coherency STEPNAME(neoncache_st_coherency)
int neoncache_st_coherency(dynarec_arm_t* dyn, int ninst, int a, int b);
// scratch fpu regs for convertions

#if STEP == 0
#define ST_IS_F(A)          0
#define ST_IS_I64(A)        0
#define X87_COMBINE(A, B)   NEON_CACHE_ST_D
#define X87_ST0             NEON_CACHE_ST_D
#define X87_ST(A)           NEON_CACHE_ST_D
#elif STEP == 1
#define ST_IS_F(A) (neoncache_get_current_st(dyn, ninst, A)==NEON_CACHE_ST_F)
#define ST_IS_I64(A) (neoncache_get_current_st(dyn, ninst, A)==NEON_CACHE_ST_I64)
#define X87_COMBINE(A, B) neoncache_combine_st(dyn, ninst, A, B)
#define X87_ST0     neoncache_no_i64(dyn, ninst, 0, neoncache_get_current_st(dyn, ninst, 0))
#define X87_ST(A)   neoncache_no_i64(dyn, ninst, A, neoncache_get_current_st(dyn, ninst, A))
#else
#define ST_IS_F(A) (neoncache_get_st(dyn, ninst, A)==NEON_CACHE_ST_F)
#define ST_IS_I64(A) (neoncache_get_st(dyn, ninst, A)==NEON_CACHE_ST_I64)
#if STEP == 3
#define X87_COMBINE(A, B) neoncache_st_coherency(dyn, ninst, A, B)
#else
#define X87_COMBINE(A, B) neoncache_get_st(dyn, ninst, A)
#endif
#define X87_ST0     neoncache_get_st(dyn, ninst, 0)
#define X87_ST(A)   neoncache_get_st(dyn, ninst, A)
#endif

//MMX helpers
// get neon register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int a);
// get neon register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int a);

//SSE/SSE2 helpers
// get neon register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a, int forwrite);
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a);
// forget neon register for a SSE reg, YMM high part too
void sse_forget_reg(dynarec_arm_t* dyn, int ninst, int a);
// purge the XMM0..XMM7 cache (before function call)
void sse_purge07cache(dynarec_arm_t* dyn, int ninst, int s1);
// Push current value to the cache (ymm too)
void sse_reflect_reg(dynarec_arm_t* dyn, int ninst, int a);
// common coproc helpers
// reset the cache with n
void fpu_reset_cache(dynarec_arm_t* dyn, int ninst, int reset_n);
// propagate stack state
void fpu_propagate_stack(dynarec_arm_t* dyn, int ninst);
// purge the FPU cache (needs 3 scratch registers)
void fpu_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1, int s2, int s3, uint32_t unneeded);
// purge MMX cache
void mmx_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1);
// purge x87 cache
void x87_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1, int s2, int s3);
// temporarily set x87 stack count for C functions
void x87_reflectcount(dynarec_arm_t* dyn, int ninst, int s1, int s2);
// restore count after
void x87_unreflectcount(dynarec_arm_t* dyn, int ninst, int s1, int s2);
// global fpu helper
void fpu_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
void fpu_unreflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
void fpu_pushcache(dynarec_arm_t* dyn, int ninst, int s1, int not07);
void fpu_popcache(dynarec_arm_t* dyn, int ninst, int s1, int not07);
// avx helpers
// get neon register for a SSE reg, create the entry if needed
int ymm_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a, int forwrite, int k1, int k2, int k3);
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int ymm_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a, int k1, int k2, int k3);
// mark an ymm upper part has zero (forgetting upper part if needed)
void ymm_mark_zero(dynarec_arm_t* dyn, int ninst, int a);
// Get an YMM upper quad reg, while keeping up to 3 other YMM reg (-1 to no keep)
int fpu_get_reg_ymm(dynarec_arm_t* dyn, int ninst, int t, int ymm, int k1, int k2, int k3);
// Preload XMM/YMM regs
void doPreload(dynarec_arm_t* dyn, int ninst);
// Enter a block (atomic inrement of hot and in_used)
void doEnterBlock(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
// Leave a block (atomic decrement of in_used)
void doLeaveBlock(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);

uintptr_t dynarec64_00(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_66(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_D8(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_D9(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_DA(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_DB(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_DC(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_DD(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_DE(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_DF(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F0(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_66F20F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_66F30F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_660F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_66F0(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F20F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F30F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_0F38(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_66_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F2_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F3_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_66_0F38(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_66_0F3A(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F2_0F38(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F2_0F3A(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);
uintptr_t dynarec64_AVX_F3_0F38(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog);

#if STEP < 2
#define PASS2(A)
#else
#define PASS2(A)   A
#endif

#if STEP < 3
#define PASS3(A)
#else
#define PASS3(A)   A
#endif

#if STEP < 3
#define MAYUSE(A)   (void)A
#else
#define MAYUSE(A)
#endif

#define GOCOND(B, T1, T2)                                   \
    case B+0x0:                                             \
        INST_NAME(T1 "O " T2);                              \
        IFNATIVE(NF_VF) {                                   \
        GO( , cVC, cVS, X_OF)                               \
        } else {                                            \
        GO( TSTw_mask(xFlags, 0b010101, 0)                  \
            , cEQ, cNE, X_OF)                               \
        }                                                   \
        break;                                              \
    case B+0x1:                                             \
        INST_NAME(T1 "NO " T2);                             \
        IFNATIVE(NF_VF) {                                   \
        GO( , cVS, cVC, X_OF)                               \
        } else {                                            \
        GO( TSTw_mask(xFlags, 0b010101, 0)                  \
            , cNE, cEQ, X_OF)                               \
        }                                                   \
        break;                                              \
    case B+0x2:                                             \
        INST_NAME(T1 "C " T2);                              \
        IFNATIVE(NF_CF) {                                   \
            if(INVERTED_CARRY) {                            \
                GO( , cCS, cCC, X_CF)                       \
            } else {                                        \
                GO( , cCC, cCS, X_CF)                       \
            }                                               \
        } else {                                            \
        GO( TSTw_mask(xFlags, 0, 0)                         \
            , cEQ, cNE, X_CF)                               \
        }                                                   \
        break;                                              \
    case B+0x3:                                             \
        INST_NAME(T1 "NC " T2);                             \
        IFNATIVE(NF_CF) {                                   \
            if(INVERTED_CARRY) {                            \
                GO( , cCC, cCS, X_CF)                       \
            } else {                                        \
                GO( , cCS, cCC, X_CF)                       \
            }                                               \
        } else {                                            \
        GO( TSTw_mask(xFlags, 0, 0)                         \
            , cNE, cEQ, X_CF)                               \
        }                                                   \
        break;                                              \
    case B+0x4:                                             \
        INST_NAME(T1 "Z " T2);                              \
        IFNATIVE(NF_EQ) {                                   \
        GO( , cNE, cEQ, X_ZF)                               \
        } else {                                            \
        GO( TSTw_mask(xFlags, 0b011010, 0)                  \
            , cEQ, cNE, X_ZF)                               \
        }                                                   \
        break;                                              \
    case B+0x5:                                             \
        INST_NAME(T1 "NZ " T2);                             \
        IFNATIVE(NF_EQ) {                                   \
        GO( , cEQ, cNE, X_ZF)                               \
        } else {                                            \
        GO( TSTw_mask(xFlags, 0b011010, 0)                  \
            , cNE, cEQ, X_ZF)                               \
        }                                                   \
        break;                                              \
    case B+0x6:                                             \
        INST_NAME(T1 "BE " T2);                             \
        IFNATIVEN(NF_EQ|NF_CF) {                            \
            INVERT_CARRY(x1);                               \
            GO( , cHI, cLS, X_ZF|X_CF)                      \
        } else {                                            \
        GO( MOV32w(x1, (1<<F_CF)|(1<<F_ZF));                \
            TSTw_REG(xFlags, x1)                            \
            , cEQ, cNE, X_CF|X_ZF)                          \
        }                                                   \
        break;                                              \
    case B+0x7:                                             \
        INST_NAME(T1 "NBE " T2);                            \
        IFNATIVEN(NF_EQ|NF_CF) {                            \
            INVERT_CARRY(x1);                               \
            GO( , cLS, cHI, X_ZF|X_CF)                      \
        } else {                                            \
        GO( MOV32w(x1, (1<<F_CF)|(1<<F_ZF));                \
            TSTw_REG(xFlags, x1)                            \
            , cNE, cEQ, X_CF|X_ZF)                          \
        }                                                   \
        break;                                              \
    case B+0x8:                                             \
        INST_NAME(T1 "S " T2);                              \
        IFNATIVE(NF_SF) {                                   \
        GO( , cPL, cMI, X_SF)                               \
        } else {                                            \
        GO( TSTw_mask(xFlags, 0b011001, 0)                  \
            , cEQ, cNE, X_SF)                               \
        }                                                   \
        break;                                              \
    case B+0x9:                                             \
        INST_NAME(T1 "NS " T2);                             \
        IFNATIVE(NF_SF) {                                   \
        GO( , cMI, cPL, X_SF)                               \
        } else {                                            \
        GO( TSTw_mask(xFlags, 0b011001, 0)                  \
            , cNE, cEQ, X_SF)                               \
        }                                                   \
        break;                                              \
    case B+0xA:                                             \
        INST_NAME(T1 "P " T2);                              \
        GO( TSTw_mask(xFlags, 0b011110, 0)                  \
            , cEQ, cNE, X_PF)                               \
        break;                                              \
    case B+0xB:                                             \
        INST_NAME(T1 "NP " T2);                             \
        GO( TSTw_mask(xFlags, 0b011110, 0)                  \
            , cNE, cEQ, X_PF)                               \
        break;                                              \
    case B+0xC:                                             \
        INST_NAME(T1 "L " T2);                              \
        IFNATIVEN(NF_SF|NF_VF) {                            \
        GO( , cGE, cLT, X_SF|X_OF)                          \
        } else {                                            \
        GO( EORw_REG_LSL(x1, xFlags, xFlags, F_OF-F_SF);    \
            TSTw_mask(x1, 0b010101, 0)                      \
            , cEQ, cNE, X_SF|X_OF)                          \
        }                                                   \
        break;                                              \
    case B+0xD:                                             \
        INST_NAME(T1 "GE " T2);                             \
        IFNATIVEN(NF_SF|NF_VF) {                            \
        GO( , cLT, cGE, X_SF|X_OF)                          \
        } else {                                            \
        GO( EORw_REG_LSL(x1, xFlags, xFlags, F_OF-F_SF);    \
            TSTw_mask(x1, 0b010101, 0)                      \
            , cNE, cEQ, X_SF|X_OF)                          \
        }                                                   \
        break;                                              \
    case B+0xE:                                             \
        INST_NAME(T1 "LE " T2);                             \
        IFNATIVEN(NF_SF|NF_VF|NF_EQ) {                      \
        GO( , cGT, cLE, X_SF|X_OF|X_ZF)                     \
        } else {                                            \
        GO( EORw_REG_LSL(x1, xFlags, xFlags, F_OF-F_SF);    \
            ORRw_REG_LSL(x1, x1, xFlags, F_OF-F_ZF);        \
            TSTw_mask(x1, 0b010101, 0)                      \
            , cEQ, cNE, X_SF|X_OF|X_ZF)                     \
        }                                                   \
        break;                                              \
    case B+0xF:                                             \
        INST_NAME(T1 "G " T2);                              \
        IFNATIVEN(NF_SF|NF_VF|NF_EQ) {                      \
        GO( , cLE, cGT, X_SF|X_OF|X_ZF)                     \
        } else {                                            \
        GO( EORw_REG_LSL(x1, xFlags, xFlags, F_OF-F_SF);    \
            ORRw_REG_LSL(x1, x1, xFlags, F_OF-F_ZF);        \
            TSTw_mask(x1, 0b010101, 0)                      \
            , cNE, cEQ, X_SF|X_OF|X_ZF)                     \
        }                                                   \
        break

#define NOTEST(s1)                                           \
    if (BOX64ENV(dynarec_test)) {                            \
        STRw_U12(xZR, xEmu, offsetof(x64emu_t, test.test));  \
        STRw_U12(xZR, xEmu, offsetof(x64emu_t, test.clean)); \
    }
#define SKIPTEST(s1)                                         \
    if (BOX64ENV(dynarec_test)) {                            \
        STRw_U12(xZR, xEmu, offsetof(x64emu_t, test.clean)); \
    }
#define GOTEST(s1, s2)                                     \
    if (BOX64ENV(dynarec_test)) {                          \
        MOV32w(s2, 1);                                     \
        STRw_U12(s2, xEmu, offsetof(x64emu_t, test.test)); \
    }

#define GETREX()                                \
    rex.rex = 0;                                \
    if(!rex.is32bits)                           \
        while(opcode>=0x40 && opcode<=0x4f) {   \
            rex.rex = opcode;                   \
            opcode = F8;                        \
        }

#define COMP_ZFSF(s1, A)                        \
    IFX(X_ZF|X_SF) {                            \
        if(cpuext.flagm) {                      \
            SETF##A(s1);                        \
            IFX(X_ZF) {                         \
                IFNATIVE(NF_EQ) {} else {       \
                CSETw(s3, cEQ);                 \
                BFIw(xFlags, s3, F_ZF, 1);      \
                }                               \
            }                                   \
            IFX(X_SF) {                         \
                IFNATIVE(NF_SF) {} else {       \
                CSETw(s3, cMI);                 \
                BFIw(xFlags, s3, F_SF, 1);      \
                }                               \
            }                                   \
        } else {                                \
            IFX(X_ZF) {                         \
                ANDSw_mask(s1, s1, 0, (A)-1);   \
                IFNATIVE(NF_EQ) {} else {       \
                CSETw(s3, cEQ);                 \
                BFIw(xFlags, s3, F_ZF, 1);      \
                }                               \
            }                                   \
            IFX(X_SF) {                         \
                LSRw(s3, s1, (A)-1);            \
                BFIw(xFlags, s3, F_SF, 1);      \
            }                                   \
        }                                       \
    }

#define PURGE_YMM()                                                         \
    do {                                                                    \
        if ((ok > 0) && reset_n == -1 && dyn->insts[ninst + 1].purge_ymm)   \
            avx_purge_ymm(dyn, ninst, dyn->insts[ninst + 1].purge_ymm, x1); \
    } while (0)

#endif //__DYNAREC_ARM64_HELPER_H__
