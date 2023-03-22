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


// Strong mem emulation helpers
// Sequence of Read will trigger a DMB on "first" read if strongmem is 2
// Squence of Write will trigger a DMB on "last" write if strongmem is 1
// Opcode will read
#define SMREAD()    if(!dyn->smread && box64_dynarec_strongmem>1) {SMDMB();}
// Opcode will read with option forced lock
#define SMREADLOCK(lock)    if(lock || (!dyn->smread && box64_dynarec_strongmem>1)) {SMDMB();}
// Opcode migh read (depend on nextop)
#define SMMIGHTREAD()   if(!MODREG) {SMREAD();}
// Opcode has wrote
#define SMWRITE()   dyn->smwrite=1
// Opcode has wrote (strongmem>1 only)
#define SMWRITE2()   if(box64_dynarec_strongmem>1) dyn->smwrite=1
// Opcode has wrote with option forced lock
#define SMWRITELOCK(lock)   if(lock) {SMDMB();} else dyn->smwrite=1
// Opcode migh have wrote (depend on nextop)
#define SMMIGHTWRITE()   if(!MODREG) {SMWRITE();}
// Start of sequence
#define SMSTART()   SMEND()
// End of sequence
#define SMEND()     if(dyn->smwrite && box64_dynarec_strongmem) {FENCE();} dyn->smwrite=0; dyn->smread=0;
// Force a Data memory barrier (for LOCK: prefix)
#define SMDMB()     FENCE(); dyn->smwrite=0; dyn->smread=1

//LOCK_* define
#define LOCK_LOCK   (int*)1

// GETGD    get x64 register in gd
#define GETGD   gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3)
// GETED can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETED(D)  if(MODREG) {                          \
                    ed = xRAX+(nextop&7)+(rex.b<<3);    \
                    wback = 0;                          \
                } else {                                \
                    SMREAD()                            \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
                    LDxw(x1, wback, fixedaddress);      \
                    ed = x1;                            \
                }
// GETSED can use r1 for ed, and r2 for wback. ed will be sign extended!
#define GETSED(D)  if(MODREG) {                         \
                    ed = xRAX+(nextop&7)+(rex.b<<3);    \
                    wback = 0;                          \
                    if(!rex.w) {                        \
                        ADDW(x1, ed, xZR);              \
                        ed = x1;                        \
                    }                                   \
                } else {                                \
                    SMREAD()                            \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
                    if(rex.w)                           \
                        LD(x1, wback, fixedaddress);    \
                    else                                \
                        LW(x1, wback, fixedaddress);    \
                    ed = x1;                            \
                }
// GETEDx can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETEDx(D) if(MODREG) {                          \
                    ed = xRAX+(nextop&7)+(rex.b<<3);    \
                    wback = 0;                          \
                } else {                                \
                    SMREAD()                            \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, D); \
                    LD(x1, wback, fixedaddress);        \
                    ed = x1;                            \
                }
//GETEDH can use hint for ed, and r1 or r2 for wback (depending on hint). wback is 0 if ed is xEAX..xEDI
#define GETEDH(hint, D) if(MODREG) {                    \
                    ed = xRAX+(nextop&7)+(rex.b<<3);    \
                    wback = 0;                          \
                } else {                                \
                    SMREAD();                           \
                    addr = geted(dyn, addr, ninst, nextop, &wback, (hint==x2)?x1:x2, ed, &fixedaddress, rex, NULL, 1, D); \
                    LDxw(hint, wback, fixedaddress);    \
                    ed = hint;                          \
                }
// GETGW extract x64 register in gd, that is i
#define GETGW(i) gd = xRAX+((nextop&0x38)>>3)+(rex.r<<3); SLLI(i, gd, 48); SRLI(i, i, 48); gd = i;
//GETEWW will use i for ed, and can use w for wback.
#define GETEWW(w, i, D) if(MODREG) {        \
                    wback = xRAX+(nextop&7)+(rex.b<<3);\
                    SLLI(i, wback, 48);     \
                    SRLI(i, i, 48);         \
                    ed = i;                 \
                    wb1 = 0;                \
                } else {                    \
                    SMREAD();               \
                    addr = geted(dyn, addr, ninst, nextop, &wback, w, i, &fixedaddress, rex, NULL, 1, D); \
                    LHU(i, wback, fixedaddress);\
                    ed = i;                 \
                    wb1 = 1;                \
                }
//GETEW will use i for ed, and can use r3 for wback.
#define GETEW(i, D) GETEWW(x3, i, D)
//GETSEW will use i for ed, and can use r3 for wback. This is the Signed version
#define GETSEW(i, D) if(MODREG) {           \
                    wback = xRAX+(nextop&7)+(rex.b<<3);\
                    SLLI(i, wback, 48);     \
                    SRAI(i, i, 48);         \
                    ed = i;                 \
                    wb1 = 0;                \
                } else {                    \
                    SMREAD();               \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, i, &fixedaddress, rex, NULL, 1, D); \
                    LH(i, wback, fixedaddress); \
                    ed = i;                 \
                    wb1 = 1;                \
                }
// Write ed back to original register / memory
#define EWBACK       EWBACKW(ed)
// Write w back to original register / memory (w needs to be 16bits only!)
#define EWBACKW(w)   if(wb1) {SH(w, wback, fixedaddress); SMWRITE();} else {SRLI(wback, wback, 16); SLLI(wback, wback, 16); OR(wback, wback, w);}
// Write back gd in correct register (gd needs to be 16bits only!)
#define GWBACK       do{int g=xRAX+((nextop&0x38)>>3)+(rex.r<<3); SRLI(g, g, 16); SLLI(g, g, 16); OR(g, g, gd);}while(0)

// FAKEED like GETED, but doesn't get anything
#define FAKEED  if(!MODREG) {   \
                    addr = fakeed(dyn, addr, ninst, nextop); \
                }

// Write back ed in wback (if wback not 0)
#define WBACK       if(wback) {SDxw(ed, wback, fixedaddress); SMWRITE();}

// GETEB will use i for ed, and can use r3 for wback.
#define GETEB(i, D) if(MODREG) {                \
                    if(rex.rex) {               \
                        wback = xRAX+(nextop&7)+(rex.b<<3);     \
                        wb2 = 0;                \
                    } else {                    \
                        wback = (nextop&7);     \
                        wb2 = (wback>>2)*8;     \
                        wback = xRAX+(wback&3); \
                    }                           \
                    MV(i, wback);               \
                    if (wb2) SRLI(i, i, wb2);   \
                    ANDI(i, i, 0xff);           \
                    wb1 = 0;                    \
                    ed = i;                     \
                } else {                        \
                    SMREAD();                   \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 0, D); \
                    LBU(i, wback, fixedaddress);\
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
                    MV(i, wback);               \
                    SLLIW(i, i, 24-wb2);        \
                    SRAIW(i, i, 24);            \
                    wb1 = 0;                    \
                    ed = i;                     \
                } else {                        \
                    SMREAD();                   \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x3, &fixedaddress, rex, NULL, 0, D); \
                    LB(i, wback, fixedaddress); \
                    wb1 = 1;                    \
                    ed = i;                     \
                }

//GETGB will use i for gd
#define GETGB(i) if(rex.rex) {                                \
                    gb1 = xRAX+((nextop&0x38)>>3)+(rex.r<<3); \
                    gb2 = 0;                                  \
                } else {                                      \
                    gd = (nextop&0x38)>>3;                    \
                    gb2 = ((gd&4)>>2);                        \
                    gb1 = xRAX+(gd&3);                        \
                }                                             \
                gd = i;                                       \
                MV(gd, gb1);                                  \
                if (gb2) SRLI(gd, gd, gb2*8);                 \
                ANDI(gd, gd, 0xff);

// Write gb (gd) back to original register / memory, using s1 as scratch
#define GBBACK(s1) if(gb2) {                            \
                    assert(gb2 == 8);                   \
                    MOV64x(s1, 0xffffffffffff00ffLL);   \
                    AND(gb1, gb1, s1);                  \
                    ANDI(gd, gd, 0xff);                 \
                    SLLI(s1, gd, 8);                    \
                    OR(gb1, gb1, s1);                   \
                } else {                                \
                    ANDI(gb1, gb1, ~0xff);              \
                    ANDI(gd, gd, 0xff);                 \
                    OR(gb1, gb1, gd);                   \
                }

// Write eb (ed) back to original register / memory, using s1 as scratch
#define EBBACK(s1) if(wb1) {                            \
                    SB(ed, wback, fixedaddress);        \
                    SMWRITE();                          \
                } else if(wb2) {                        \
                    assert(wb2 == 8);                   \
                    MOV64x(s1, 0xffffffffffff00ffLL);   \
                    AND(wback, wback, s1);              \
                    ANDI(ed, ed, 0xff);                 \
                    SLLI(s1, ed, 8);                    \
                    OR(wback, wback, s1);               \
                } else {                                \
                    ANDI(wback, wback, ~0xff);          \
                    ANDI(ed, ed, 0xff);                 \
                    OR(wback, wback, ed);               \
                }

// Get direction with size Z and based of F_DF flag, on register r ready for load/store fetching
// using s as scratch.
#define GETDIR(r, s, Z)             \
    MOV32w(r, Z); /* mask=1<<10 */  \
    ANDI(s, xFlags, 1<<F_DF);       \
    BEQZ(s, 8);                     \
    SUB(r, xZR, r);                 \

// CALL will use x6 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL(F, ret) call_c(dyn, ninst, F, x6, ret, 1, 0)
// CALL_ will use x6 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL_(F, ret, reg) call_c(dyn, ninst, F, x6, ret, 1, reg)
// CALL_S will use x6 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2. Flags are not save/restored
#define CALL_S(F, ret) call_c(dyn, ninst, F, x6, ret, 0, 0)

#define MARK    dyn->insts[ninst].mark = dyn->native_size
#define GETMARK dyn->insts[ninst].mark
#define MARK2   dyn->insts[ninst].mark2 = dyn->native_size
#define GETMARK2 dyn->insts[ninst].mark2
#define MARK3   dyn->insts[ninst].mark3 = dyn->native_size
#define GETMARK3 dyn->insts[ninst].mark3
#define MARKF   dyn->insts[ninst].markf = dyn->native_size
#define GETMARKF dyn->insts[ninst].markf
#define MARKSEG dyn->insts[ninst].markseg = dyn->native_size
#define GETMARKSEG dyn->insts[ninst].markseg
#define MARKLOCK dyn->insts[ninst].marklock = dyn->native_size
#define GETMARKLOCK dyn->insts[ninst].marklock

#define Bxx_gen(OP, M, reg1, reg2)      \
    j64 = GET##M - dyn->native_size;    \
    B##OP (reg1, reg2, j64)

// Branch to MARK if reg1==reg2 (use j64)
#define BEQ_MARK(reg1, reg2) Bxx_gen(EQ, MARK, reg1, reg2)
// Branch to MARK if reg1!=reg2 (use j64)
#define BNE_MARK(reg1, reg2) Bxx_gen(NE, MARK, reg1, reg2)
// Branch to MARK if reg1!=0 (use j64)
#define BNEZ_MARK(reg) BNE_MARK(reg, xZR)
// Branch to MARK if reg1<reg2 (use j64)
#define BLT_MARK(reg1, reg2) Bxx_gen(LT, MARK, reg1, reg2)
// Branch to MARK if reg1>=reg2 (use j64)
#define BGE_MARK(reg1, reg2) Bxx_gen(GE, MARK, reg1, reg2)
// Branch to MARK2 if reg1==reg2 (use j64)
#define BEQ_MARK2(reg1, reg2) Bxx_gen(EQ, MARK2, reg1,reg2)
// Branch to MARK2 if reg1!=reg2 (use j64)
#define BNE_MARK2(reg1, reg2) Bxx_gen(NE, MARK2, reg1,reg2)
// Branch to MARK2 if reg1!=0 (use j64)
#define BNEZ_MARK2(reg) BNE_MARK2(reg, xZR)
// Branch to MARK2 if reg1<reg2 (use j64)
#define BLT_MARK2(reg1, reg2) Bxx_gen(LT, MARK2, reg1,reg2)
// Branch to MARK3 if reg1==reg2 (use j64)
#define BEQ_MARK3(reg1, reg2) Bxx_gen(EQ, MARK3, reg1, reg2)
// Branch to MARK3 if reg1!=reg2 (use j64)
#define BNE_MARK3(reg1, reg2) Bxx_gen(NE, MARK3, reg1, reg2)
// Branch to MARK3 if reg1!=0 (use j64)
#define BNEZ_MARK3(reg) BNE_MARK3(reg, xZR)
// Branch to MARKLOCK if reg1!=reg2 (use j64)
#define BNE_MARKLOCK(reg1, reg2) Bxx_gen(NE, MARKLOCK, reg1, reg2)
// Branch to MARKLOCK if reg1!=0 (use j64)
#define BNEZ_MARKLOCK(reg) BNE_MARKLOCK(reg, xZR)

// Branch to NEXT if reg1==0 (use j64)
#define CBZ_NEXT(reg1)                 \
    j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0; \
    BEQ(reg1, xZR, j64)
// Branch to NEXT if reg1!=0 (use j64)
#define CBNZ_NEXT(reg1)                \
    j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0; \
    BNE(reg1, xZR, j64)
// Branch to next instruction unconditionnal (use j64)
#define B_NEXT_nocond                                               \
    j64 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->native_size)):0;\
    B(j64)

// Branch to MARKSEG if reg is 0 (use j64)
#define CBZ_MARKSEG(reg)    \
    j64 = GETMARKSEG-(dyn->native_size);   \
    BEQZ(reg, j64);
// Branch to MARKSEG if reg is not 0 (use j64)
#define CBNZ_MARKSEG(reg)              \
    j64 = GETMARKSEG-(dyn->native_size);   \
    BNEZ(reg, j64);

#define IFX(A)  if((dyn->insts[ninst].x64.gen_flags&(A)))
#define IFX_PENDOR0  if((dyn->insts[ninst].x64.gen_flags&(X_PEND) || !dyn->insts[ninst].x64.gen_flags))
#define IFXX(A) if((dyn->insts[ninst].x64.gen_flags==(A)))
#define IFX2X(A, B) if((dyn->insts[ninst].x64.gen_flags==(A) || dyn->insts[ninst].x64.gen_flags==(B) || dyn->insts[ninst].x64.gen_flags==((A)|(B))))
#define IFXN(A, B)  if((dyn->insts[ninst].x64.gen_flags&(A) && !(dyn->insts[ninst].x64.gen_flags&(B))))

#define STORE_REG(A)    SD(x##A, xEmu, offsetof(x64emu_t, regs[_##A]))
#define LOAD_REG(A)     LD(x##A, xEmu, offsetof(x64emu_t, regs[_##A]))

// Need to also store current value of some register, as they may be used by functions like setjump
#define STORE_XEMU_CALL()   \
    STORE_REG(RBX);         \
    STORE_REG(RDX);         \
    STORE_REG(RSP);         \
    STORE_REG(RBP);         \
    STORE_REG(RDI);         \
    STORE_REG(RSI);         \
    STORE_REG(R8);          \
    STORE_REG(R9);          \
    STORE_REG(R10);         \
    STORE_REG(R11);         \

#define LOAD_XEMU_CALL()    \

#define LOAD_XEMU_REM()     \
    LOAD_REG(RBX);          \
    LOAD_REG(RDX);          \
    LOAD_REG(RSP);          \
    LOAD_REG(RBP);          \
    LOAD_REG(RDI);          \
    LOAD_REG(RSI);          \
    LOAD_REG(R8);           \
    LOAD_REG(R9);           \
    LOAD_REG(R10);          \
    LOAD_REG(R11);          \


#define SET_DFNONE()    if(!dyn->f.dfnone) {SW(xZR, xEmu, offsetof(x64emu_t, df)); dyn->f.dfnone=1;}
#define SET_DF(S, N)     if((N)!=d_none) {MOV_U12(S, (N)); SW(S, xEmu, offsetof(x64emu_t, df)); dyn->f.dfnone=0;} else SET_DFNONE()
#define SET_NODF()          dyn->f.dfnone = 0
#define SET_DFOK()          dyn->f.dfnone = 1

#define CLEAR_FLAGS() IFX(X_ALL) {ANDI(xFlags, xFlags, ~((1UL<<F_AF) | (1UL<<F_CF) | (1UL<<F_OF2) | (1UL<<F_ZF) | (1UL<<F_SF) | (1UL<<F_PF)));}

#define CALC_SUB_FLAGS(op1_, op2, res, scratch1, scratch2, width)         \
    IFX(X_AF | X_CF | X_OF) {                                             \
        /* calc borrow chain */                                           \
        /* bc = (res & (~op1 | op2)) | (~op1 & op2) */                    \
        OR(scratch1, op1_, op2);                                          \
        AND(scratch2, res, scratch1);                                     \
        AND(op1_, op1_, op2);                                             \
        OR(scratch2, scratch2, op1_);                                     \
        IFX(X_AF) {                                                       \
            /* af = bc & 0x8 */                                           \
            ANDI(scratch1, scratch2, 8);                                  \
            BEQZ(scratch1, 8);                                            \
            ORI(xFlags, xFlags, 1 << F_AF);                               \
        }                                                                 \
        IFX(X_CF) {                                                       \
            /* cf = bc & (1<<(width-1)) */                                \
            if ((width) == 8) {                                           \
                ANDI(scratch1, scratch2, 0x80);                           \
            } else {                                                      \
                SRLI(scratch1, scratch2, (width)-1);                      \
            }                                                             \
            BEQZ(scratch1, 8);                                            \
            ORI(xFlags, xFlags, 1 << F_CF);                               \
        }                                                                 \
        IFX(X_OF) {                                                       \
            /* of = ((bc >> (width-2)) ^ (bc >> (width-1))) & 0x1; */     \
            SRLI(scratch1, scratch2, (width)-2);                          \
            SRLI(scratch2, scratch1, 1);                                  \
            XOR(scratch1, scratch1, scratch2);                            \
            ANDI(scratch1, scratch1, 1);                                  \
            BEQZ(scratch1, 8);                                            \
            ORI(xFlags, xFlags, 1 << F_OF2);                              \
        }                                                                 \
    }

// Adjust the xFlags bit 11 -> bit 5, result in reg (can be xFlags, but not s1)
#define FLAGS_ADJUST_FROM11(reg, s1)\
    ANDI(reg, xFlags, ~(1<<5));     \
    SRLI(s1, reg, 11-5);            \
    ANDI(s1, s1, 1<<5);             \
    OR(reg, reg, s1)

// Adjust the xFlags bit 5 -> bit 11, source in reg (can be xFlags, but not s1)
#define FLAGS_ADJUST_TO11(reg, s1)  \
    MOV64x(s1, ~(1<<11));           \
    AND(xFlags, reg, s1);           \
    ANDI(s1, xFlags, 1<<5);         \
    SLLI(s1, s1, 11-5);             \
    OR(xFlags, xFlags, s1)

#ifndef MAYSETFLAGS
#define MAYSETFLAGS()
#endif

#ifndef READFLAGS
#define READFLAGS(A) \
    if(((A)!=X_PEND && dyn->f.pending!=SF_SET)          \
    && (dyn->f.pending!=SF_SET_PENDING)) {              \
        if(dyn->f.pending!=SF_PENDING) {                \
            LD(x3, xEmu, offsetof(x64emu_t, df));       \
            j64 = (GETMARKF)-(dyn->native_size);        \
            BEQ(x3, xZR, j64);                          \
        }                                               \
        CALL_(UpdateFlags, -1, 0);                      \
        FLAGS_ADJUST_FROM11(xFlags, x3);                \
        MARKF;                                          \
        dyn->f.pending = SF_SET;                        \
        SET_DFOK();                                     \
    }
#endif

#ifndef SETFLAGS
#define SETFLAGS(A, B)                                                                          \
    if(dyn->f.pending!=SF_SET                                                                   \
    && ((B)&SF_SUB)                                                                             \
    && (dyn->insts[ninst].x64.gen_flags&(~(A))))                                                \
        READFLAGS(((dyn->insts[ninst].x64.gen_flags&X_PEND)?X_ALL:dyn->insts[ninst].x64.gen_flags)&(~(A)));\
    if(dyn->insts[ninst].x64.gen_flags) switch(B) {                                             \
        case SF_SUBSET:                                                                         \
        case SF_SET: dyn->f.pending = SF_SET; break;                                            \
        case SF_PENDING: dyn->f.pending = SF_PENDING; break;                                    \
        case SF_SUBSET_PENDING:                                                                 \
        case SF_SET_PENDING:                                                                    \
            dyn->f.pending = (dyn->insts[ninst].x64.gen_flags&X_PEND)?SF_SET_PENDING:SF_SET;    \
            break;                                                                              \
    } else dyn->f.pending = SF_SET
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
#define UFLAG_OP1(A) if(dyn->insts[ninst].x64.gen_flags) {SDxw(A, xEmu, offsetof(x64emu_t, op1));}
#define UFLAG_OP2(A) if(dyn->insts[ninst].x64.gen_flags) {SDxw(A, xEmu, offsetof(x64emu_t, op2));}
#define UFLAG_OP12(A1, A2) if(dyn->insts[ninst].x64.gen_flags) {SDxw(A1, xEmu, offsetof(x64emu_t, op1));SDxw(A2, xEmu, offsetof(x64emu_t, op2));}
#define UFLAG_RES(A) if(dyn->insts[ninst].x64.gen_flags) {SDxw(A, xEmu, offsetof(x64emu_t, res));}
#define UFLAG_DF(r, A) if(dyn->insts[ninst].x64.gen_flags) {SET_DF(r, A)}
#define UFLAG_IF if(dyn->insts[ninst].x64.gen_flags)
#ifndef DEFAULT
#define DEFAULT      *ok = -1; BARRIER(2)
#endif

#ifndef TABLE64
#define TABLE64(A, V)
#endif
#ifndef FTABLE64
#define FTABLE64(A, V)
#endif

#define ARCH_INIT()


#if STEP < 2
#define GETIP(A)
#define GETIP_(A)
#else
// put value in the Table64 even if not using it for now to avoid difference between Step2 and Step3. Needs to be optimized later...
#define GETIP(A)                                        \
    if(dyn->last_ip && ((A)-dyn->last_ip)<2048) {       \
        uint64_t _delta_ip = (A)-dyn->last_ip;          \
        dyn->last_ip += _delta_ip;                      \
        if(_delta_ip) {                                 \
            ADDI(xRIP, xRIP, _delta_ip);                \
        }                                               \
    } else {                                            \
        dyn->last_ip = (A);                             \
        if(dyn->last_ip<0xffffffff) {                   \
            MOV64x(xRIP, dyn->last_ip);                 \
        } else                                          \
            TABLE64(xRIP, dyn->last_ip);                \
    }
#define GETIP_(A)                                       \
    if(dyn->last_ip && ((A)-dyn->last_ip)<2048) {       \
        int64_t _delta_ip = (A)-dyn->last_ip;           \
        if(_delta_ip) {ADDI(xRIP, xRIP, _delta_ip);}    \
    } else {                                            \
        if((A)<0xffffffff) {                            \
            MOV64x(xRIP, (A));                          \
        } else                                          \
            TABLE64(xRIP, (A));                         \
    }
#endif
#define CLEARIP()   dyn->last_ip=0

#if STEP < 2
#define PASS2IF(A, B) if(A)
#elif STEP == 2
#define PASS2IF(A, B) if(A) dyn->insts[ninst].pass2choice = B; if(dyn->insts[ninst].pass2choice == B)
#else
#define PASS2IF(A, B) if(dyn->insts[ninst].pass2choice == B)
#endif

#define MODREG  ((nextop&0xC0)==0xC0)

void rv64_epilog();
void rv64_epilog_fast();
void* rv64_next(x64emu_t* emu, uintptr_t addr);

#ifndef STEPNAME
#define STEPNAME3(N,M) N##M
#define STEPNAME2(N,M) STEPNAME3(N,M)
#define STEPNAME(N) STEPNAME2(N, STEP)
#endif

#define native_pass        STEPNAME(native_pass)

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
#define dynarec64_66F0     STEPNAME(dynarec64_66F0)
#define dynarec64_F20F     STEPNAME(dynarec64_F20F)
#define dynarec64_F30F     STEPNAME(dynarec64_F30F)

#define geted           STEPNAME(geted)
#define geted32         STEPNAME(geted32)
#define geted16         STEPNAME(geted16)
#define jump_to_epilog  STEPNAME(jump_to_epilog)
#define jump_to_epilog_fast  STEPNAME(jump_to_epilog_fast)
#define jump_to_next    STEPNAME(jump_to_next)
#define ret_to_epilog   STEPNAME(ret_to_epilog)
#define retn_to_epilog  STEPNAME(retn_to_epilog)
#define iret_to_epilog  STEPNAME(iret_to_epilog)
#define call_c          STEPNAME(call_c)
#define call_n          STEPNAME(call_n)
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
#define emit_test32c    STEPNAME(emit_test32)
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
#define x87_get_current_cache   STEPNAME(x87_get_current_cache)
#define x87_get_cache   STEPNAME(x87_get_cache)
#define x87_get_extcache STEPNAME(x87_get_extcache)
#define x87_get_st      STEPNAME(x87_get_st)
#define x87_get_st_empty  STEPNAME(x87_get_st)
#define x87_refresh     STEPNAME(x87_refresh)
#define x87_forget      STEPNAME(x87_forget)
#define x87_reget_st    STEPNAME(x87_reget_st)
#define x87_stackcount  STEPNAME(x87_stackcount)
#define x87_swapreg     STEPNAME(x87_swapreg)
#define x87_setround    STEPNAME(x87_setround)
#define x87_restoreround STEPNAME(x87_restoreround)
#define sse_setround    STEPNAME(sse_setround)
#define mmx_get_reg     STEPNAME(mmx_get_reg)
#define mmx_get_reg_empty STEPNAME(mmx_get_reg_empty)
#define sse_get_reg     STEPNAME(sse_get_reg)
#define sse_get_reg_empty STEPNAME(sse_get_reg_empty)
#define sse_forget_reg   STEPNAME(sse_forget_reg)
#define sse_purge07cache STEPNAME(sse_purge07cache)

#define fpu_pushcache   STEPNAME(fpu_pushcache)
#define fpu_popcache    STEPNAME(fpu_popcache)
#define fpu_reset       STEPNAME(fpu_reset)
#define fpu_reset_cache STEPNAME(fpu_reset_cache)
#define fpu_propagate_stack STEPNAME(fpu_propagate_stack)
#define fpu_purgecache  STEPNAME(fpu_purgecache)
#define mmx_purgecache  STEPNAME(mmx_purgecache)
#define x87_purgecache  STEPNAME(x87_purgecache)
#define sse_purgecache  STEPNAME(sse_purgecache)
#ifdef HAVE_TRACE
#define fpu_reflectcache STEPNAME(fpu_reflectcache)
#endif

#define CacheTransform       STEPNAME(CacheTransform)
#define rv64_move64     STEPNAME(rv64_move64)
#define rv64_move32     STEPNAME(rv64_move32)

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, uint8_t scratch, int64_t* fixaddress, rex_t rex, int* l, int i12, int delta);

/* setup r2 to address pointed by */
//uintptr_t geted32(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int absmax, uint32_t mask, rex_t rex, int* l, int s, int delta);

/* setup r2 to address pointed by */
//uintptr_t geted16(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int absmax, uint32_t mask, int s);


// generic x64 helper
void jump_to_epilog(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_epilog_fast(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_next(dynarec_rv64_t* dyn, uintptr_t ip, int reg, int ninst);
void ret_to_epilog(dynarec_rv64_t* dyn, int ninst);
void retn_to_epilog(dynarec_rv64_t* dyn, int ninst, int n);
//void iret_to_epilog(dynarec_rv64_t* dyn, int ninst, int is64bits);
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
//void emit_add8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_add8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s2, int s3, int s4);
void emit_sub32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_sub32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s2, int s3, int s4, int s5);
//void emit_sub8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_sub8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5);
void emit_or32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_or32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_xor32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_xor32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_and32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_and32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int64_t c, int s3, int s4);
void emit_or8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_or8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_xor8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_xor8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_and8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_and8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_add16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_add16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_sub16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_sub16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_or16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_or16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_xor16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_xor16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_and16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_and16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_inc32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
//void emit_inc16(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_inc8(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_dec32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
//void emit_dec16(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_dec8(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_adc32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
//void emit_adc32c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_adc8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_adc8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5);
//void emit_adc16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_adc16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sbb32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
//void emit_sbb32c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sbb8(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4, int s5);
void emit_sbb8c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4, int s5, int s6);
//void emit_sbb16(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3, int s4);
//void emit_sbb16c(dynarec_rv64_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
//void emit_neg32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s3, int s4);
//void emit_neg16(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
//void emit_neg8(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);
void emit_shl32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4, int s5);
void emit_shl32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4, int s5);
//void emit_shr32(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, int s3, int s4);
void emit_shr32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
void emit_sar32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
//void emit_rol32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
//void emit_ror32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, uint32_t c, int s3, int s4);
//void emit_shrd32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);
//void emit_shld32c(dynarec_rv64_t* dyn, int ninst, rex_t rex, int s1, int s2, uint32_t c, int s3, int s4);

void emit_pf(dynarec_rv64_t* dyn, int ninst, int s1, int s3, int s4);

// x87 helper
// cache of the local stack counter, to avoid upadte at every call
void x87_stackcount(dynarec_rv64_t* dyn, int ninst, int scratch);
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
// refresh a value from the cache ->emu (nothing done if value is not cached)
void x87_refresh(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st);
// refresh a value from the cache ->emu and then forget the cache (nothing done if value is not cached)
void x87_forget(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st);
// refresh the cache value from emu
void x87_reget_st(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int st);
// swap 2 x87 regs
void x87_swapreg(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int a, int b);
// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3);
// Restore round flag
void x87_restoreround(dynarec_rv64_t* dyn, int ninst, int s1);
// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3);

void CacheTransform(dynarec_rv64_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3);

void rv64_move64(dynarec_rv64_t* dyn, int ninst, int reg, int64_t val);
void rv64_move32(dynarec_rv64_t* dyn, int ninst, int reg, int32_t val, int zeroup);

#if STEP < 2
#define CHECK_CACHE()   0
#else
#define CHECK_CACHE()   (cacheupd = CacheNeedsTransform(dyn, ninst))
#endif
#define extcache_st_coherency STEPNAME(extcache_st_coherency)
int extcache_st_coherency(dynarec_rv64_t* dyn, int ninst, int a, int b);

#if STEP == 0
#define ST_IS_F(A)          0
#define X87_COMBINE(A, B)   EXT_CACHE_ST_D
#define X87_ST0             EXT_CACHE_ST_D
#define X87_ST(A)           EXT_CACHE_ST_D
#elif STEP == 1
#define ST_IS_F(A) (extcache_get_current_st(dyn, ninst, A)==EXT_CACHE_ST_F)
#define X87_COMBINE(A, B) extcache_combine_st(dyn, ninst, A, B)
#define X87_ST0     extcache_get_current_st(dyn, ninst, 0)
#define X87_ST(A)   extcache_get_current_st(dyn, ninst, A)
#else
#define ST_IS_F(A) (extcache_get_st(dyn, ninst, A)==EXT_CACHE_ST_F)
#if STEP == 3
#define X87_COMBINE(A, B) extcache_st_coherency(dyn, ninst, A, B)
#else
#define X87_COMBINE(A, B) extcache_get_st(dyn, ninst, A)
#endif
#define X87_ST0     extcache_get_st(dyn, ninst, 0)
#define X87_ST(A)   extcache_get_st(dyn, ninst, A)
#endif

//SSE/SSE2 helpers
// get neon register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_rv64_t* dyn, int ninst, int s1, int a, int single);
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_rv64_t* dyn, int ninst, int s1, int a, int single);
// forget neon register for a SSE reg, create the entry if needed
void sse_forget_reg(dynarec_rv64_t* dyn, int ninst, int a);
// purge the XMM0..XMM7 cache (before function call)
void sse_purge07cache(dynarec_rv64_t* dyn, int ninst, int s1);

// common coproc helpers
// reset the cache
void fpu_reset(dynarec_rv64_t* dyn);
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
#ifdef HAVE_TRACE
void fpu_reflectcache(dynarec_rv64_t* dyn, int ninst, int s1, int s2, int s3);
#endif
void fpu_pushcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07);
void fpu_popcache(dynarec_rv64_t* dyn, int ninst, int s1, int not07);


uintptr_t dynarec64_00(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_64(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int seg, int* ok, int* need_epilog);
//uintptr_t dynarec64_65(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep,int* ok, int* need_epilog);
uintptr_t dynarec64_66(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_67(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_D8(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_D9(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DA(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_DB(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DC(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DD(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DE(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_DF(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_F0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
uintptr_t dynarec64_660F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
//uintptr_t dynarec64_6664(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int seg, int* ok, int* need_epilog);
//uintptr_t dynarec64_66F0(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog);
//uintptr_t dynarec64_F20F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);
uintptr_t dynarec64_F30F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog);

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

// GOCOND will use x1 and x3
#define GOCOND(B, T1, T2)                                   \
    case B+0x0:                                             \
        INST_NAME(T1 "O " T2);                              \
        GO( ANDI(x1, xFlags, 1<<F_OF2)                      \
            , EQZ, NEZ, X_OF)                               \
        break;                                              \
    case B+0x1:                                             \
        INST_NAME(T1 "NO " T2);                             \
        GO( ANDI(x1, xFlags, 1<<F_OF2)                      \
            , NEZ, EQZ, X_OF)                               \
        break;                                              \
    case B+0x2:                                             \
        INST_NAME(T1 "C " T2);                              \
        GO( ANDI(x1, xFlags, 1<<F_CF)                       \
            , EQZ, NEZ, X_CF)                               \
        break;                                              \
    case B+0x3:                                             \
        INST_NAME(T1 "NC " T2);                             \
        GO( ANDI(x1, xFlags, 1<<F_CF)                       \
            , NEZ, EQZ, X_CF)                               \
        break;                                              \
    case B+0x4:                                             \
        INST_NAME(T1 "Z " T2);                              \
        GO( ANDI(x1, xFlags, 1<<F_ZF)                       \
            , EQZ, NEZ, X_ZF)                               \
        break;                                              \
    case B+0x5:                                             \
        INST_NAME(T1 "NZ " T2);                             \
        GO( ANDI(x1, xFlags, 1<<F_ZF)                       \
            , NEZ, EQZ, X_ZF)                               \
        break;                                              \
    case B+0x6:                                             \
        INST_NAME(T1 "BE " T2);                             \
        GO( ANDI(x1, xFlags, (1<<F_CF)|(1<<F_ZF))           \
            , EQZ, NEZ, X_CF|X_ZF)                          \
        break;                                              \
    case B+0x7:                                             \
        INST_NAME(T1 "NBE " T2);                            \
        GO( ANDI(x1, xFlags, (1<<F_CF)|(1<<F_ZF))           \
            , NEZ, EQZ, X_CF|X_ZF)                          \
        break;                                              \
    case B+0x8:                                             \
        INST_NAME(T1 "S " T2);                              \
        GO( ANDI(x1, xFlags, 1<<F_SF)                       \
            , EQZ, NEZ, X_SF)                               \
        break;                                              \
    case B+0x9:                                             \
        INST_NAME(T1 "NS " T2);                             \
        GO( ANDI(x1, xFlags, 1<<F_SF)                       \
            , NEZ, EQZ, X_SF)                               \
        break;                                              \
    case B+0xA:                                             \
        INST_NAME(T1 "P " T2);                              \
        GO( ANDI(x1, xFlags, 1<<F_PF)                       \
            , EQZ, NEZ, X_PF)                               \
        break;                                              \
    case B+0xB:                                             \
        INST_NAME(T1 "NP " T2);                             \
        GO( ANDI(x1, xFlags, 1<<F_PF)                       \
            , NEZ, EQZ, X_PF)                               \
        break;                                              \
    case B+0xC:                                             \
        INST_NAME(T1 "L " T2);                              \
        GO( SRLI(x1, xFlags, F_SF-F_OF2);                   \
            XOR(x1, x1, xFlags);                            \
            ANDI(x1, x1, 1<<F_OF2)                          \
            , EQZ, NEZ, X_SF|X_OF)                          \
        break;                                              \
    case B+0xD:                                             \
        INST_NAME(T1 "GE " T2);                             \
        GO( SRLI(x1, xFlags, F_SF-F_OF2);                   \
            XOR(x1, x1, xFlags);                            \
            ANDI(x1, x1, 1<<F_OF2)                          \
            , NEZ, EQZ, X_SF|X_OF)                          \
        break;                                              \
    case B+0xE:                                             \
        INST_NAME(T1 "LE " T2);                             \
        GO( SRLI(x1, xFlags, F_SF-F_OF2);                   \
            XOR(x1, x1, xFlags);                            \
            ANDI(x1, x1, 1<<F_OF2);                         \
            ANDI(x3, xFlags, 1<<F_ZF);                      \
            OR(x1, x1, x3);                                 \
            ANDI(x1, x1, (1<<F_OF2) | (1<<F_ZF))            \
            , EQZ, NEZ, X_SF|X_OF|X_ZF)                     \
        break;                                              \
    case B+0xF:                                             \
        INST_NAME(T1 "G " T2);                              \
        GO( SRLI(x1, xFlags, F_SF-F_OF2);                   \
            XOR(x1, x1, xFlags);                            \
            ANDI(x1, x1, 1<<F_OF2);                         \
            ANDI(x3, xFlags, 1<<F_ZF);                      \
            OR(x1, x1, x3);                                 \
            ANDI(x1, x1, (1<<F_OF2) | (1<<F_ZF))            \
            , NEZ, EQZ, X_SF|X_OF|X_ZF)                     \
        break

#endif //__DYNAREC_RV64_HELPER_H__
