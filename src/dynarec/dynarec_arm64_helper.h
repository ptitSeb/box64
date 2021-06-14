#ifndef __DYNAREC_ARM64_HELPER_H__
#define __DYNAREC_ARM64_HELPER_H__

// undef to get Close to SSE Float->int conversions
//#define PRECISE_CVT

#if STEP == 0
#include "dynarec_arm64_pass0.h"
#elif STEP == 1
#include "dynarec_arm64_pass1.h"
#elif STEP == 2
#include "dynarec_arm64_pass2.h"
#elif STEP == 3
#include "dynarec_arm64_pass3.h"
#endif

#include "debug.h"
#include "arm64_emitter.h"
#include "../emu/x64primop.h"

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

// GETGD    get x64 register in gd
#define GETGD   gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3)
//GETED can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETED(D)  if(MODREG) {                          \
                    ed = xRAX+(nextop&7)+(rex.b<<3);    \
                    wback = 0;                          \
                } else {                                \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, D); \
                    LDRxw_U12(x1, wback, fixedaddress); \
                    ed = x1;                            \
                }
#define GETEDx(D)  if(MODREG) {                         \
                    ed = xRAX+(nextop&7)+(rex.b<<3);    \
                    wback = 0;                          \
                } else {                                \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0xfff<<3, 7, rex, 0, D); \
                    LDRx_U12(x1, wback, fixedaddress);  \
                    ed = x1;                            \
                }
#define GETEDw(D)  if((nextop&0xC0)==0xC0) {            \
                    ed = xEAX+(nextop&7)+(rex.b<<3);    \
                    wback = 0;                          \
                } else {                                \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0xfff<<2, 3, rex, 0, D); \
                    LDRw_U12(x1, wback, fixedaddress);  \
                    ed = x1;                            \
                }
//GETEDH can use hint for ed, and r1 or r2 for wback (depending on hint). wback is 0 if ed is xEAX..xEDI
#define GETEDH(hint, D) if(MODREG) {                    \
                    ed = xRAX+(nextop&7)+(rex.b<<3);    \
                    wback = 0;                          \
                } else {                                \
                    addr = geted(dyn, addr, ninst, nextop, &wback, (hint==x2)?x1:x2, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, D); \
                    LDRxw_U12(hint, wback, fixedaddress); \
                    ed = hint;                            \
                }
//GETEDW can use hint for wback and ret for ed. wback is 0 if ed is xEAX..xEDI
#define GETEDW(hint, ret, D)   if(MODREG) {              \
                    ed = xRAX+(nextop&7)+(rex.b<<3);    \
                    MOVxw_REG(ret, ed);                 \
                    wback = 0;                          \
                } else {                                \
                    addr = geted(dyn, addr, ninst, nextop, &wback, hint, &fixedaddress, 0xfff<<(2+rex.w), (1<<(2+rex.w))-1, rex, 0, D); \
                    ed = ret;                           \
                    LDRxw_U12(ed, wback, fixedaddress); \
                }
// Write back ed in wback (if wback not 0)
#define WBACK       if(wback) {STRxw_U12(ed, wback, fixedaddress);}
// Write back ed in wback (if wback not 0)
#define WBACKx      if(wback) {STRx_U12(ed, wback, fixedaddress);}
// Write back ed in wback (if wback not 0)
#define WBACKw      if(wback) {STRw_U12(ed, wback, fixedaddress);}
// Send back wb to either ed or wback
#define SBACK(wb)   if(wback) {STRxw(wb, wback, fixedaddress);} else {MOVxw_REG(ed, wb);}
//GETEDO can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETEDO(O, D)   if(MODREG) {                     \
                    ed = xRAX+(nextop&7)+(rex.b<<3);    \
                    wback = 0;                          \
                } else {                                \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, rex, 0, D); \
                    LDRxw_REG(x1, wback, O);            \
                    ed = x1;                            \
                }
#define WBACKO(O)   if(wback) {STRxw_REG(ed, wback, O);}
//FAKEELike GETED, but doesn't get anything
#define FAKEED  if(!MODREG) {   \
                    addr = fakeed(dyn, addr, ninst, nextop); \
                }
// GETGW extract x64 register in gd, that is i
#define GETGW(i) gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3); UXTHw(i, gd); gd = i;
// GETGW extract x64 register in gd, that is i, Signed extented
#define GETSGW(i) gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3); SXTHw(i, gd); gd = i;
//GETEWW will use i for ed, and can use w for wback.
#define GETEWW(w, i, D) if(MODREG) {        \
                    wback = xRAX+(nextop&7)+(rex.b<<3);\
                    UXTHw(i, wback);        \
                    ed = i;                 \
                    wb1 = 0;                \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, w, &fixedaddress, 0xfff<<1, (1<<1)-1, rex, 0, D); \
                    LDRH_U12(i, wback, fixedaddress); \
                    ed = i;                 \
                    wb1 = 1;                \
                }
//GETEW will use i for ed, and can use r3 for wback.
#define GETEW(i, D) if(MODREG) {            \
                    wback = xRAX+(nextop&7)+(rex.b<<3);\
                    UXTHw(i, wback);        \
                    ed = i;                 \
                    wb1 = 0;                \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0xfff<<1, (1<<1)-1, rex, 0, D); \
                    LDRH_U12(i, wback, fixedaddress); \
                    ed = i;                 \
                    wb1 = 1;                \
                }
//GETSEW will use i for ed, and can use r3 for wback. This is the Signed version
#define GETSEW(i, D) if(MODREG) {           \
                    wback = xRAX+(nextop&7)+(rex.b<<3);\
                    SXTHw(i, wback);        \
                    ed = i;                 \
                    wb1 = 0;                \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0xfff<<1, (1<<1)-1, rex, 0, D); \
                    LDRSHx_U12(i, wback, fixedaddress);\
                    ed = i;                 \
                    wb1 = 1;                \
                }
// Write ed back to original register / memory
#define EWBACK   if(wb1) {STRH_U12(ed, wback, fixedaddress);} else {BFIx(wback, ed, 0, 16);}
// Write w back to original register / memory
#define EWBACKW(w)   if(wb1) {STRH_U12(w, wback, fixedaddress);} else {BFIx(wback, w, 0, 16);}
// Write back gd in correct register
#define GWBACK       BFIx((xRAX+((nextop&0x38)>>3)+(rex.r<<3)), gd, 0, 16);
//GETEB will use i for ed, and can use r3 for wback.
#define GETEB(i, D) if(MODREG) {                \
                    if(rex.rex) {               \
                        wback = xRAX+(nextop&7)+(rex.b<<3);     \
                        wb2 = 0;                \
                    } else {                    \
                        wback = (nextop&7);     \
                        wb2 = (wback>>2)*8;     \
                        wback = xRAX+(wback&3); \
                    }                           \
                    UBFXx(i, wback, wb2, 8);    \
                    wb1 = 0;                    \
                    ed = i;                     \
                } else {                        \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0xfff, 0, rex, 0, D); \
                    LDRB_U12(i, wback, fixedaddress); \
                    wb1 = 1;                    \
                    ed = i;                     \
                }
//GETEBO will use i for ed, i is also Offset, and can use r3 for wback.
#define GETEBO(i, D) if(MODREG) {               \
                    if(rex.rex) {               \
                        wback = xRAX+(nextop&7)+(rex.b<<3);     \
                        wb2 = 0;                \
                    } else {                    \
                        wback = (nextop&7);     \
                        wb2 = (wback>>2)*8;     \
                        wback = xRAX+(wback&3); \
                    }                           \
                    UBFXx(i, wback, wb2, 8);    \
                    wb1 = 0;                    \
                    ed = i;                     \
                } else {                        \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, rex, 0, D); \
                    ADDx_REG(wback, wback, i);  \
                    LDRB_U12(i, wback, fixedaddress);      \
                    wb1 = 1;                    \
                    ed = i;                     \
                }
//GETSEB sign extend EB, will use i for ed, and can use r3 for wback.
#define GETSEB(i, D) if(MODREG) {                \
                    if(rex.rex) {               \
                        wback = xRAX+(nextop&7)+(rex.b<<3);     \
                        wb2 = 0;                \
                    } else {                    \
                        wback = (nextop&7);     \
                        wb2 = (wback>>2)*8;     \
                        wback = xRAX+(wback&3); \
                    }                           \
                    SBFXx(i, wback, wb2, 8);    \
                    wb1 = 0;                    \
                    ed = i;                     \
                } else {                        \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0xfff, 0, rex, 0, D); \
                    LDRSBx_U12(i, wback, fixedaddress); \
                    wb1 = 1;                    \
                    ed = i;                     \
                }
// Write eb (ed) back to original register / memory
#define EBBACK   if(wb1) {STRB_U12(ed, wback, fixedaddress);} else {BFIx(wback, ed, wb2, 8);}
//GETGB will use i for gd
#define GETGB(i)    if(rex.rex) {               \
                        gb1 = xRAX+((nextop&0x38)>>3)+(rex.r<<3);     \
                        gb2 = 0;                \
                    } else {                    \
                        gd = (nextop&0x38)>>3;  \
                        gb2 = ((gd&4)>>2);      \
                        gb1 = xRAX+(gd&3);      \
                    }                           \
                    gd = i;                     \
                    UBFXx(gd, gb1, gb2*8, 8);
//GETSGB signe extend GB, will use i for gd
#define GETSGB(i)   if(rex.rex) {               \
                        gb1 = xRAX+((nextop&0x38)>>3)+(rex.r<<3);     \
                        gb2 = 0;                \
                    } else {                    \
                        gd = (nextop&0x38)>>3;  \
                        gb2 = ((gd&4)>>2);      \
                        gb1 = xRAX+(gd&3);      \
                    }                           \
                    gd = i;                     \
                    SBFXx(gd, gb1, gb2, 8);
// Write gb (gd) back to original register / memory
#define GBBACK   BFIx(gb1, gd, gb2, 8);

// Get Direction with size Z and based of F_DF flag, on register r ready for LDR/STR fetching
// F_DF is 1<<10, so 1 ROR 11*2 (so F_OF)
#define GETDIR(r, A)                \
    MOV32w(r, A); /* mask=1<<10 */  \
    TSTw_mask(xFlags, 0b010110, 0); \
    CNEGx(r, r, cNE)

// CALL will use x7 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL(F, ret) call_c(dyn, ninst, F, x7, ret, 1, 0)
// CALL_ will use x7 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL_(F, ret, reg) call_c(dyn, ninst, F, x7, ret, 1, reg)
// CALL_S will use x7 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2. Flags are not save/restored
#define CALL_S(F, ret) call_c(dyn, ninst, F, x7, ret, 0, 0)

#define MARK    if(dyn->insts) {dyn->insts[ninst].mark = dyn->arm_size;}
#define GETMARK ((dyn->insts)?dyn->insts[ninst].mark:(dyn->arm_size+4))
#define MARK2   if(dyn->insts) {dyn->insts[ninst].mark2 = dyn->arm_size;}
#define GETMARK2 ((dyn->insts)?dyn->insts[ninst].mark2:(dyn->arm_size+4))
#define MARK3   if(dyn->insts) {dyn->insts[ninst].mark3 = dyn->arm_size;}
#define GETMARK3 ((dyn->insts)?dyn->insts[ninst].mark3:(dyn->arm_size+4))
#define MARKF   if(dyn->insts) {dyn->insts[ninst].markf = dyn->arm_size;}
#define GETMARKF ((dyn->insts)?dyn->insts[ninst].markf:(dyn->arm_size+4))
#define MARKSEG if(dyn->insts) {dyn->insts[ninst].markseg = dyn->arm_size;}
#define GETMARKSEG ((dyn->insts)?dyn->insts[ninst].markseg:(dyn->arm_size+4))
#define MARKLOCK if(dyn->insts) {dyn->insts[ninst].marklock = dyn->arm_size;}
#define GETMARKLOCK ((dyn->insts)?dyn->insts[ninst].marklock:(dyn->arm_size+4))

// Branch to MARK if cond (use j64)
#define B_MARK(cond)                \
    j64 = GETMARK-(dyn->arm_size);  \
    Bcond(cond, j64)
// Branch to MARK unconditionnal (use j64)
#define B_MARK_nocond               \
    j64 = GETMARK-(dyn->arm_size);  \
    B(j64)
// Branch to MARK if reg is 0 (use j64)
#define CBZxw_MARK(reg)             \
    j64 = GETMARK-(dyn->arm_size);  \
    CBZxw(reg, j64)
// Branch to MARK if reg is not 0 (use j64)
#define CBNZx_MARK(reg)             \
    j64 = GETMARK-(dyn->arm_size);  \
    CBNZx(reg, j64)
// Branch to MARK if reg is not 0 (use j64)
#define CBNZw_MARK(reg)             \
    j64 = GETMARK-(dyn->arm_size);  \
    CBNZw(reg, j64)
// Branch to MARK2 if cond (use j64)
#define B_MARK2(cond)               \
    j64 = GETMARK2-(dyn->arm_size); \
    Bcond(cond, j64)
// Branch to MARK2 unconditionnal (use j64)
#define B_MARK2_nocond              \
    j64 = GETMARK2-(dyn->arm_size); \
    B(j64)
// Branch to MARK2 if reg is not 0 (use j64)
#define CBNZx_MARK2(reg)            \
    j64 = GETMARK2-(dyn->arm_size); \
    CBNZx(reg, j64)
// Test bit N of A and branch to MARK2 if set
#define TBNZ_MARK2(A, N)            \
    j64 = GETMARK2-(dyn->arm_size); \
    TBNZ(A, N, j64)
// Branch to MARK3 if cond (use j64)
#define B_MARK3(cond)               \
    j64 = GETMARK3-(dyn->arm_size); \
    Bcond(cond, j64)
// Branch to MARK3 unconditionnal (use j64)
#define B_MARK3_nocond              \
    j64 = GETMARK3-(dyn->arm_size); \
    B(j64)
// Branch to MARK3 if reg is not 0 (use j64)
#define CBNZx_MARK3(reg)            \
    j64 = GETMARK3-(dyn->arm_size); \
    CBNZx(reg, j64)
// Test bit N of A and branch to MARK3 if set
#define TBNZ_MARK3(A, N)            \
    j64 = GETMARK3-(dyn->arm_size); \
    TBNZ(A, N, j64)
// Test bit N of A and branch to MARK3 if not set
#define TBZ_MARK3(A, N)             \
    j64 = GETMARK3-(dyn->arm_size); \
    TBZ(A, N, j64)
// Branch to next instruction if cond (use j64)
#define B_NEXT(cond)     \
    j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->arm_size)):0; \
    Bcond(cond, j64)
// Branch to next instruction unconditionnal (use j64)
#define B_NEXT_nocond                                               \
    j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->arm_size)):0;\
    B(j64)
// Branch to next instruction if reg is 0 (use j64)
#define CBZw_NEXT(reg)    \
    j64 =  (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->arm_size)):0; \
    CBZw(reg, j64)
// Branch to next instruction if reg is 0 (use j64)
#define CBZx_NEXT(reg)    \
    j64 =  (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->arm_size)):0; \
    CBZx(reg, j64)
// Branch to next instruction if reg is not 0 (use j64)
#define CBNZx_NEXT(reg)   \
    j64 =  (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->arm_size)):0; \
    CBNZx(reg, j64)
// Test bit N of A and branch to next instruction if not set
#define TBZ_NEXT(A, N)              \
    j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->arm_size)):0; \
    TBZ(A, N, j64)
// Test bit N of A and branch to next instruction if set
#define TBNZ_NEXT(A, N)             \
    j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->arm_size)):0; \
    TBNZ(A, N, j64)
// Branch to MARKSEG if cond (use j64)
#define B_MARKSEG(cond)    \
    j64 = GETMARKSEG-(dyn->arm_size);   \
    Bcond(cond, j64)
// Branch to MARKSEG if reg is 0 (use j64)
#define CBZw_MARKSEG(reg)    \
    j64 = GETMARKSEG-(dyn->arm_size);   \
    CBZw(reg, j64)
// Branch to MARKSEG if reg is not 0 (use j64)
#define CBNZw_MARKSEG(reg)              \
    j64 = GETMARKSEG-(dyn->arm_size);   \
    CBNZw(reg, j64)
// Branch to MARKLOCK if cond (use j64)
#define B_MARKLOCK(cond)    \
    j64 = GETMARKLOCK-(dyn->arm_size);   \
    Bcond(cond, j64)
// Branch to MARKLOCK if reg is not 0 (use j64)
#define CBNZx_MARKLOCK(reg)             \
    j64 = GETMARKLOCK-(dyn->arm_size);  \
    CBNZx(reg, j64)

#define IFX(A)  if(dyn->insts && (dyn->insts[ninst].x64.need_flags&(A)))
#define IFX_PENDOR0  if(dyn->insts && (dyn->insts[ninst].x64.need_flags&(X_PEND) || !dyn->insts[ninst].x64.need_flags))
#define IFXX(A) if(dyn->insts && (dyn->insts[ninst].x64.need_flags==(A)))
#define IFX2X(A, B) if(dyn->insts && (dyn->insts[ninst].x64.need_flags==(A) || dyn->insts[ninst].x64.need_flags==(B) || dyn->insts[ninst].x64.need_flags==((A)|(B))))
#define IFXN(A, B)  if(dyn->insts && (dyn->insts[ninst].x64.need_flags&(A) && !(dyn->insts[ninst].x64.need_flags&(B))))

// Generate FCOM with s1 and s2 scratch regs (the VCMP is already done)
#define FCOM(s1, s2, s3)                                                    \
    LDRH_U12(s3, xEmu, offsetof(x64emu_t, sw));   /*offset is 8bits right?*/\
    MOV32w(s1, 0b0100011100000000);                                         \
    BICw_REG(s3, s3, s1);                                                   \
    CSETw(s1, cMI); /* 1 if less than, 0 else */                            \
    MOV32w(s2, 0b01000101); /* unordered */                                 \
    CSELw(s1, s2, s1, cVS);                                                 \
    MOV32w(s2, 0b01000000); /* zero */                                      \
    CSELw(s1, s2, s1, cEQ);                                                 \
    /* greater than leave 0 */                                              \
    ORRw_REG_LSL(s3, s3, s1, 8);                                            \
    STRH_U12(s3, xEmu, offsetof(x64emu_t, sw))

// Generate FCOMI with s1 and s2 scratch regs (the VCMP is already done)
#define FCOMI(s1, s2)    \
    IFX(X_CF|X_PF|X_ZF|X_PEND) {                                            \
        MOV32w(s2, 0b01000101);                                             \
        BICw_REG(xFlags, xFlags, s2);                                       \
        CSETw(s1, cMI); /* 1 if less than, 0 else */                        \
        /*s2 already set */     /* unordered */                             \
        CSELw(s1, s2, s1, cVS);                                             \
        MOV32w(s2, 0b01000000); /* zero */                                  \
        CSELw(s1, s2, s1, cEQ);                                             \
        /* greater than leave 0 */                                          \
        ORRw_REG(xFlags, xFlags, s1);                                       \
    }                                                                       \
    SET_DFNONE(s1);                                                         \
    IFX(X_OF|X_PEND) {                                                      \
        BFCw(xFlags, F_OF, 1);                                              \
    }                                                                       \
    IFX(X_AF|X_PEND) {                                                      \
        BFCw(xFlags, F_AF, 1);                                              \
    }                                                                       \
    IFX(X_SF|X_PEND) {                                                      \
        BFCw(xFlags, F_SF, 1);                                              \
    }                                                                       \


#define STORE_REG(A)    STRx_U12(x##A, xEmu, offsetof(x64emu_t, regs[_##A]))
#define STP_REGS(A, B)  STPx_S7_offset(x##A, x##B, xEmu, offsetof(x64emu_t, regs[_##A]))
#define LDP_REGS(A, B)  LDPx_S7_offset(x##A, x##B, xEmu, offsetof(x64emu_t, regs[_##A]))
#define STORE_XEMU_REGS(A)  \
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
    STORE_REG(R10);         \
    STORE_REG(R11);         \
    STORE_REG(R12);         \
    STORE_REG(R13);         \
    STORE_REG(R14);         \
    STORE_REG(R15);         \
    STRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags)); \
    if(A) {STRx_U12(A, xEmu, offsetof(x64emu_t, ip));}

#define LOAD_REG(A)    LDRx_U12(x##A, xEmu, offsetof(x64emu_t, regs[_##A]))
#define LOAD_XEMU_REGS(A)  \
    LOAD_REG(RAX);         \
    LOAD_REG(RCX);         \
    LOAD_REG(RDX);         \
    LOAD_REG(RBX);         \
    LOAD_REG(RSP);         \
    LOAD_REG(RBP);         \
    LOAD_REG(RSI);         \
    LOAD_REG(RDI);         \
    LOAD_REG(R8);          \
    LOAD_REG(R9);          \
    LOAD_REG(R10);         \
    LOAD_REG(R11);         \
    LOAD_REG(R12);         \
    LOAD_REG(R13);         \
    LOAD_REG(R14);         \
    LOAD_REG(R15);         \
    LDRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags)); \
    if(A) {LDRx_U12(A, xEmu, offsetof(x64emu_t, ip));}

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

#define SET_DFNONE(S)    if(!dyn->dfnone) {MOVZw(S, d_none); STRw_U12(S, xEmu, offsetof(x64emu_t, df)); dyn->dfnone=1;}
#define SET_DF(S, N)     if((N)!=d_none) {MOVZw(S, (N)); STRw_U12(S, xEmu, offsetof(x64emu_t, df)); dyn->dfnone=0;} else SET_DFNONE(S)
#define SET_NODF()          dyn->dfnone = 0
#define SET_DFOK()          dyn->dfnone = 1

#ifndef READFLAGS
#define READFLAGS(A) \
    if(((A)!=X_PEND) && dyn->state_flags!=SF_SET && dyn->state_flags!=SF_SET_PENDING) { \
        if(dyn->state_flags!=SF_PENDING) {              \
            LDRw_U12(x3, xEmu, offsetof(x64emu_t, df)); \
            j64 = (GETMARKF)-(dyn->arm_size);           \
            CBZw(x3, j64);                              \
        }                                               \
        CALL_(UpdateFlags, -1, 0);                      \
        MARKF;                                          \
        dyn->state_flags = SF_SET;                      \
        SET_DFOK();                                     \
    }
#endif
#ifndef SETFLAGS
#define SETFLAGS(A, B)  \
    if(dyn->state_flags!=SF_SET && B==SF_SUBSET && (dyn->insts[ninst].x64.need_flags&(~((A)/*|X_PEND*/)))) \
        READFLAGS(dyn->insts[ninst].x64.need_flags&(~(A)|X_PEND));  \
    dyn->state_flags = (B==SF_SUBSET)?SF_SET:                       \
        ((B==SF_SET_PENDING && !(dyn->insts[ninst].x64.need_flags&X_PEND)?SF_SET:B))

#endif
#ifndef JUMP
#define JUMP(A) 
#endif
#ifndef BARRIER
#define BARRIER(A) 
#endif
#ifndef BARRIER_NEXT
#define BARRIER_NEXT(A)
#endif
#define UFLAG_OP1(A) if(dyn->insts && dyn->insts[ninst].x64.need_flags) {STRxw_U12(A, xEmu, offsetof(x64emu_t, op1));}
#define UFLAG_OP2(A) if(dyn->insts && dyn->insts[ninst].x64.need_flags) {STRxw_U12(A, xEmu, offsetof(x64emu_t, op2));}
#define UFLAG_OP12(A1, A2) if(dyn->insts && dyn->insts[ninst].x64.need_flags) {STRxw_U12(A1, xEmu, offsetof(x64emu_t, op1));STRxw_U12(A2, 0, offsetof(x64emu_t, op2));}
#define UFLAG_RES(A) if(dyn->insts && dyn->insts[ninst].x64.need_flags) {STRxw_U12(A, xEmu, offsetof(x64emu_t, res));}
#define UFLAG_DF(r, A) if(dyn->insts && dyn->insts[ninst].x64.need_flags) {SET_DF(r, A)}
#define UFLAG_IF if(dyn->insts && dyn->insts[ninst].x64.need_flags)
#ifndef DEFAULT
#define DEFAULT      *ok = -1; BARRIER(2)
#endif
#ifndef NEW_BARRIER_INST
#define NEW_BARRIER_INST
#endif
#ifndef TABLE64
#define TABLE64(A, V)
#endif
#ifndef FTABLE64
#define FTABLE64(A, V)
#endif

#if STEP < 2
#define GETIP(A)
#define GETIP_(A)
#else
// put value in the Table64 even if not using it for now to avoid difference between Step2 and Step3. Needs to be optimized later...
#define GETIP(A)                                        \
    if(dyn->last_ip && ((A)-dyn->last_ip)<0x1000) {     \
        uint64_t _delta_ip = (A)-dyn->last_ip;          \
        dyn->last_ip += _delta_ip;                      \
        if(_delta_ip) {                                 \
            ADDx_U12(xRIP, xRIP, _delta_ip);            \
        }                                               \
    } else {                                            \
        dyn->last_ip = (A);                             \
        if(dyn->last_ip<0xffffffff) {                   \
            MOV64x(xRIP, dyn->last_ip);                 \
        } else                                          \
            TABLE64(xRIP, dyn->last_ip);                \
    }
#define GETIP_(A)                                       \
    if(dyn->last_ip && ((A)-dyn->last_ip)<0x1000) {     \
        uint64_t _delta_ip = (A)-dyn->last_ip;          \
        if(_delta_ip) {ADDx_U12(xRIP, xRIP, _delta_ip);}\
    } else {                                            \
        if((A)<0xffffffff) {                            \
            MOV64x(xRIP, (A));                          \
        } else                                          \
            TABLE64(xRIP, (A));                         \
    }
#endif

#if STEP < 2
#define PASS2IF(A, B) if(A)
#elif STEP == 2
#define PASS2IF(A, B) if(A) dyn->insts[ninst].pass2choice = B; if(dyn->insts[ninst].pass2choice == B)
#else
#define PASS2IF(A, B) if(dyn->insts[ninst].pass2choice == B)
#endif

#define MODREG  ((nextop&0xC0)==0xC0)

void arm64_epilog();
void* arm64_next(x64emu_t* emu, uintptr_t addr);

#ifndef STEPNAME
#define STEPNAME3(N,M) N##M
#define STEPNAME2(N,M) STEPNAME3(N,M)
#define STEPNAME(N) STEPNAME2(N, STEP)
#endif

#define arm_pass        STEPNAME(arm_pass)

#define dynarec64_00       STEPNAME(dynarec64_00)
#define dynarec64_0F       STEPNAME(dynarec64_0F)
#define dynarec64_64       STEPNAME(dynarec64_64)
#define dynarec64_65       STEPNAME(dynarec64_65)
#define dynarec64_66       STEPNAME(dynarec64_66)
#define dynarec64_67       STEPNAME(dynarec64_67)
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
#define dynarec64_6664     STEPNAME(dynarec64_6664)
#define dynarec64_F20F     STEPNAME(dynarec64_F20F)
#define dynarec64_F30F     STEPNAME(dynarec64_F30F)

#define geted           STEPNAME(geted)
#define geted32         STEPNAME(geted32)
#define geted16         STEPNAME(geted16)
#define jump_to_epilog  STEPNAME(jump_to_epilog)
#define jump_to_next    STEPNAME(jump_to_next)
#define ret_to_epilog   STEPNAME(ret_to_epilog)
#define retn_to_epilog  STEPNAME(retn_to_epilog)
#define iret_to_epilog  STEPNAME(iret_to_epilog)
#define call_c          STEPNAME(call_c)
#define grab_segdata    STEPNAME(grab_segdata)
#define emit_cmp8       STEPNAME(emit_cmp8)
#define emit_cmp16      STEPNAME(emit_cmp16)
#define emit_cmp32      STEPNAME(emit_cmp32)
#define emit_cmp8_0     STEPNAME(emit_cmp8_0)
#define emit_cmp16_0    STEPNAME(emit_cmp16_0)
#define emit_cmp32_0    STEPNAME(emit_cmp32_0)
#define emit_test8      STEPNAME(emit_test8)
#define emit_test16     STEPNAME(emit_test16)
#define emit_test32     STEPNAME(emit_test32)
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
#define emit_sar32c     STEPNAME(emit_sar32c)
#define emit_rol32c     STEPNAME(emit_rol32c)
#define emit_ror32c     STEPNAME(emit_ror32c)
#define emit_shrd32c    STEPNAME(emit_shrd32c)
#define emit_shld32c    STEPNAME(emit_shld32c)

#define emit_pf         STEPNAME(emit_pf)

#define x87_do_push     STEPNAME(x87_do_push)
#define x87_do_push_empty STEPNAME(x87_do_push_empty)
#define x87_do_pop      STEPNAME(x87_do_pop)
#define x87_get_cache   STEPNAME(x87_get_cache)
#define x87_get_st      STEPNAME(x87_get_st)
#define x87_refresh     STEPNAME(x87_refresh)
#define x87_forget      STEPNAME(x87_forget)
#define x87_reget_st    STEPNAME(x87_reget_st)
#define x87_stackcount  STEPNAME(x87_stackcount)
#define x87_setround    STEPNAME(x87_setround)
#define x87_restoreround STEPNAME(x87_restoreround)
#define sse_setround    STEPNAME(sse_setround)
#define mmx_get_reg     STEPNAME(mmx_get_reg)
#define mmx_get_reg_empty STEPNAME(mmx_get_reg_empty)
#define sse_get_reg     STEPNAME(sse_get_reg)
#define sse_get_reg_empty STEPNAME(sse_get_reg_empty)
#define sse_purge07cache STEPNAME(sse_purge07cache)

#define fpu_pushcache   STEPNAME(fpu_pushcache)
#define fpu_popcache    STEPNAME(fpu_popcache)
#define fpu_reset       STEPNAME(fpu_reset)
#define fpu_purgecache  STEPNAME(fpu_purgecache)
#define mmx_purgecache  STEPNAME(mmx_purgecache)
#define x87_purgecache  STEPNAME(x87_purgecache)
#ifdef HAVE_TRACE
#define fpu_reflectcache STEPNAME(fpu_reflectcache)
#endif

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int absmax, uint32_t mask, rex_t rex, int s, int delta);

/* setup r2 to address pointed by */
uintptr_t geted32(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int absmax, uint32_t mask, rex_t rex, int s, int delta);

/* setup r2 to address pointed by */
uintptr_t geted16(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int absmax, uint32_t mask, int s);


// generic x64 helper
void jump_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_next(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst);
void ret_to_epilog(dynarec_arm_t* dyn, int ninst);
void retn_to_epilog(dynarec_arm_t* dyn, int ninst, int n);
void iret_to_epilog(dynarec_arm_t* dyn, int ninst);
void call_c(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int ret, int saveflags, int save_reg);
void grab_segdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg, int segment);
void emit_cmp8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_cmp16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_cmp32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_cmp8_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp16_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp32_0(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
void emit_test8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_test16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_test32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_add32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_add32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5);
void emit_add8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_add8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sub32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_sub32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4, int s5);
void emit_sub8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_sub8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5);
void emit_or32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_or32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_xor32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_xor32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_and32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_and32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_or8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_or8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_xor8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_xor8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_and8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_and8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_add16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_add16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sub16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_sub16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_or16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_or16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_xor16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_xor16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_and16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_and16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_inc32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
void emit_inc16(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_inc8(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_dec32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
void emit_dec16(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_dec8(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_adc32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_adc32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_adc8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_adc8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5);
void emit_adc16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_adc16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sbb32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_sbb32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sbb8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_sbb8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5);
void emit_sbb16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_sbb16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_neg32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
void emit_neg16(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_neg8(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_shl32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_shl32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int32_t c, int s3, int s4);
void emit_shr32(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_shr32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int32_t c, int s3, int s4);
void emit_sar32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int32_t c, int s3, int s4);
void emit_rol32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int32_t c, int s3, int s4);
void emit_ror32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int32_t c, int s3, int s4);
//void emit_shrd32c(dynarec_arm_t* dyn, int ninst, int s1, int s2, int32_t c, int s3, int s4);
void emit_shld32c(dynarec_arm_t* dyn, int ninst, rex_t rex, int s1, int s2, int32_t c, int s3, int s4);

void emit_pf(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);

// x87 helper
// cache of the local stack counter, to avoid upadte at every call
void x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch);
// fpu push. Return the Dd value to be used
int x87_do_push(dynarec_arm_t* dyn, int ninst);
// fpu push. Do not allocate a cache register. Needs a scratch register to do x87stack synch (or 0 to not do it)
void x87_do_push_empty(dynarec_arm_t* dyn, int ninst, int s1);
// fpu pop. All previous returned Dd should be considered invalid
void x87_do_pop(dynarec_arm_t* dyn, int ninst);
// get cache index for a x87 reg, create the entry if needed
int x87_get_cache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a);
// get vfpu register for a x87 reg, create the entry if needed
int x87_get_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a);
// refresh a value from the cache ->emu (nothing done if value is not cached)
void x87_refresh(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st);
// refresh a value from the cache ->emu and then forget the cache (nothing done if value is not cached)
void x87_forget(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st);
// refresh the cache value from emu
void x87_reget_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st);
// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
// Restore round flag
void x87_restoreround(dynarec_arm_t* dyn, int ninst, int s1);
// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);

//MMX helpers
// get neon register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a);
// get neon register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a);

//SSE/SSE2 helpers
// get neon register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a);
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a);
// purge the XMM0..XMM7 cache (before function call)
void sse_purge07cache(dynarec_arm_t* dyn, int ninst, int s1);

// common coproc helpers
// reset the cache
void fpu_reset(dynarec_arm_t* dyn, int ninst);
// purge the FPU cache (needs 3 scratch registers)
void fpu_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
// purge MMX cache
void mmx_purgecache(dynarec_arm_t* dyn, int ninst, int s1);
// purge x87 cache
void x87_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
#ifdef HAVE_TRACE
void fpu_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
#endif
void fpu_pushcache(dynarec_arm_t* dyn, int ninst, int s1);
void fpu_popcache(dynarec_arm_t* dyn, int ninst, int s1);

uintptr_t dynarec64_00(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_64(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog);
//uintptr_t dynarec64_65(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep,int* ok, int* need_epilog);
uintptr_t dynarec64_66(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_67(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_D8(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_D9(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DA(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DB(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DC(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DD(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DE(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DF(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_F0(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_660F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_6664(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_F20F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F30F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);

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
        GO( TSTw_mask(xFlags, 0b010101, 0)                  \
            , cEQ, cNE, X_OF)                               \
        break;                                              \
    case B+0x1:                                             \
        INST_NAME(T1 "NO " T2);                             \
        GO( TSTw_mask(xFlags, 0b010101, 0)                  \
            , cNE, cEQ, X_OF)                               \
        break;                                              \
    case B+0x2:                                             \
        INST_NAME(T1 "C " T2);                              \
        GO( TSTw_mask(xFlags, 0, 0)                         \
            , cEQ, cNE, X_CF)                               \
        break;                                              \
    case B+0x3:                                             \
        INST_NAME(T1 "NC " T2);                             \
        GO( TSTw_mask(xFlags, 0, 0)                         \
            , cNE, cEQ, X_CF)                               \
        break;                                              \
    case B+0x4:                                             \
        INST_NAME(T1 "Z " T2);                              \
        GO( TSTw_mask(xFlags, 0b011010, 0)                  \
            , cEQ, cNE, X_ZF)                               \
        break;                                              \
    case B+0x5:                                             \
        INST_NAME(T1 "NZ " T2);                             \
        GO( TSTw_mask(xFlags, 0b011010, 0)                  \
            , cNE, cEQ, X_ZF)                               \
        break;                                              \
    case B+0x6:                                             \
        INST_NAME(T1 "BE " T2);                             \
        GO( MOV32w(x1, (1<<F_CF)|(1<<F_ZF));                \
            TSTw_REG(xFlags, x1)                            \
            , cEQ, cNE, X_CF|X_ZF)                          \
        break;                                              \
    case B+0x7:                                             \
        INST_NAME(T1 "NBE " T2);                            \
        GO( MOV32w(x1, (1<<F_CF)|(1<<F_ZF));                \
            TSTw_REG(xFlags, x1)                            \
            , cNE, cEQ, X_CF|X_ZF)                          \
        break;                                              \
    case B+0x8:                                             \
        INST_NAME(T1 "S " T2);                              \
        GO( TSTw_mask(xFlags, 0b011001, 0)                  \
            , cEQ, cNE, X_SF)                               \
        break;                                              \
    case B+0x9:                                             \
        INST_NAME(T1 "NS " T2);                             \
        GO( TSTw_mask(xFlags, 0b011001, 0)                  \
            , cNE, cEQ, X_SF)                               \
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
        GO( EORw_REG_LSL(x1, xFlags, xFlags, F_OF-F_SF);    \
            TSTw_mask(x1, 0b010101, 0)                      \
            , cEQ, cNE, X_SF|X_OF)                          \
        break;                                              \
    case B+0xD:                                             \
        INST_NAME(T1 "GE " T2);                             \
        GO( EORw_REG_LSL(x1, xFlags, xFlags, F_OF-F_SF);    \
            TSTw_mask(x1, 0b010101, 0)                      \
            , cNE, cEQ, X_SF|X_OF)                          \
        break;                                              \
    case B+0xE:                                             \
        INST_NAME(T1 "LE " T2);                             \
        GO( EORw_REG_LSL(x1, xFlags, xFlags, F_OF-F_SF);    \
            ORRw_REG_LSL(x1, x1, xFlags, F_OF-F_ZF);        \
            TSTw_mask(x1, 0b010101, 0)                      \
            , cEQ, cNE, X_SF|X_OF|X_ZF)                     \
        break;                                              \
    case B+0xF:                                             \
        INST_NAME(T1 "G " T2);                              \
        GO( EORw_REG_LSL(x1, xFlags, xFlags, F_OF-F_SF);    \
            ORRw_REG_LSL(x1, x1, xFlags, F_OF-F_ZF);        \
            TSTw_mask(x1, 0b010101, 0)                      \
            , cNE, cEQ, X_SF|X_OF|X_ZF)                     \
        break

#endif //__DYNAREC_ARM64_HELPER_H__
