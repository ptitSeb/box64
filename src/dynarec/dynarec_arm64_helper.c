#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_arm64.h"
#include "dynarec_arm64_private.h"
#include "dynablock_private.h"
#include "arm64_printer.h"
#include "../tools/bridge_private.h"
#include "custommem.h"

#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int* fixaddress, int absmax, uint32_t mask, rex_t rex, int s, int delta)
{
    uint8_t ret = x2;
    uint8_t scratch = x2;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    if(hint>0 && hint<xRAX) scratch = hint;
    int absmin = 0;
    if(s) absmin=-absmax;
    MAYUSE(scratch);
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if((sib&0x7)==5) {
                uint64_t tmp = F32S64;
                if (sib_reg!=4) {
                    if(tmp && ((tmp<absmin) || (tmp>absmax) || (tmp&mask))) {
                        MOV64x(scratch, tmp);
                        ADDx_REG_LSL(ret, scratch, xRAX+sib_reg+(rex.x<<3), (sib>>6));
                    } else {
                        LSLx(ret, xRAX+sib_reg+(rex.x<<3), (sib>>6));
                        *fixaddress = tmp;
                    }
                } else {
                    MOV64x(ret, tmp);
                }
            } else {
                if (sib_reg!=4) {
                    ADDx_REG_LSL(ret, xRAX+(sib&0x7)+(rex.b<<3), xRAX+sib_reg+(rex.x<<3), (sib>>6));
                } else {
                    ret = xRAX+(sib&0x7)+(rex.b<<3);
                }
            }
        } else if((nextop&7)==5) {
            uint64_t tmp = F32S64;
            MOV64x(ret, tmp);
            TABLE64(xRIP, addr+delta);
            ADDx_REG(ret, ret, xRIP);
        } else {
            ret = xRAX+(nextop&7)+(rex.b<<3);
        }
    } else {
        int64_t i64;
        uint8_t sib = 0;
        int sib_reg = 0;
        if((nextop&7)==4) {
            sib = F8;
            sib_reg = (sib>>3)&7;
        }
        if(nextop&0x80)
            i64 = F32S;
        else 
            i64 = F8S;
        if(i64==0 || ((i64>=absmin) && (i64<=absmax)  && !(i64&mask))) {
            *fixaddress = i64;
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    ADDx_REG_LSL(ret, xRAX+(sib&0x07)+(rex.b<<3), xRAX+sib_reg+(rex.x<<3), (sib>>6));
                } else {
                    ret = xRAX+(sib&0x07)+(rex.b<<3);
                }
            } else
                ret = xRAX+(nextop&0x07)+(rex.b<<3);
        } else {
            int64_t sub = (i64<0)?1:0;
            if(sub) i64 = -i64;
            if(i64<0x1000) {
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADDx_REG_LSL(scratch, xRAX+(sib&0x07)+(rex.b<<3), xRAX+sib_reg+(rex.x<<3), (sib>>6));
                    } else {
                        scratch = xRAX+(sib&0x07)+(rex.b<<3);
                    }
                } else
                    scratch = xRAX+(nextop&0x07)+(rex.b<<3);
                if(sub) {
                    SUBx_U12(ret, scratch, i64);
                } else {
                    ADDx_U12(ret, scratch, i64);
                }
            } else {
                MOV64x(scratch, i64);
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        if(sub) {
                            SUBx_REG(scratch, xRAX+(sib&0x07)+(rex.b<<3), scratch);
                        } else {
                            ADDx_REG(scratch, scratch, xRAX+(sib&0x07)+(rex.b<<3));
                        }
                        ADDx_REG_LSL(ret, scratch, xRAX+sib_reg+(rex.x<<3), (sib>>6));
                    } else {
                        PASS3(int tmp = xRAX+(sib&0x07)+(rex.b<<3));
                        if(sub) {
                            SUBx_REG(ret, tmp, scratch);
                        } else {
                            ADDx_REG(ret, tmp, scratch);
                        }
                    }
                } else {
                    PASS3(int tmp = xRAX+(nextop&0x07)+(rex.b<<3));
                    if(sub) {
                        SUBx_REG(ret, tmp, scratch);
                    } else {
                        ADDx_REG(ret, tmp, scratch);
                    }
                }
            }
        }
    }
    *ed = ret;
    return addr;
}

/* setup r2 to address pointed by ED, r3 as scratch also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted16(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int* fixaddress, int absmax, uint32_t mask, int s)
{
    uint8_t ret = x2;
    uint8_t scratch = x3;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    if(scratch==ret) scratch = x2;
    MAYUSE(scratch);
    uint32_t m = nextop&0xC7;
    uint32_t n = (m>>6)&3;
    int32_t offset = 0;
    int absmin = 0;
    if(s) absmin = -absmax;
    if(!n && m==6) {
        offset = F16;
        MOVZw(ret, offset);
    } else {
        switch(n) {
            case 0: offset = 0; break;
            case 1: offset = F8S; break;
            case 2: offset = F16S; break;
        }
        if(offset && (offset>absmax || offset<absmin || (offset&mask))) {
            *fixaddress = offset;
            offset = 0;
        }
        switch(m&7) {
            case 0: //R_BX + R_SI
                UXTHx(ret, xRBX);
                UXTHx(scratch, xRSI);
                ADDx_REG(ret, ret, scratch);
                break;
            case 1: //R_BX + R_DI
                UXTHx(ret, xRBX);
                UXTHx(scratch, xRDI);
                ADDx_REG(ret, ret, scratch);
                break;
            case 2: //R_BP + R_SI
                UXTHx(ret, xRBP);
                UXTHx(scratch, xRSI);
                ADDx_REG(ret, ret, scratch);
                break;
            case 3: //R_BP + R_DI
                UXTHx(ret, xRBP);
                UXTHx(scratch, xRDI);
                ADDx_REG(ret, ret, scratch);
                break;
            case 4: //R_SI
                UXTHx(ret, xRSI);
                break;
            case 5: //R_DI
                UXTHx(ret, xRDI);
                break;
            case 6: //R_BP
                UXTHx(ret, xRBP);
                break;
            case 7: //R_BX
                UXTHx(ret, xRBX);
                break;
        }
        if(offset) {
            if(offset<0 && offset>-0x1000) {
                SUBx_U12(ret, ret, -offset);
            } else if(offset>0 && offset<0x1000) {
                ADDx_U12(ret, ret, offset);
            } else {
                MOV64x(scratch, offset);
                ADDx_REG(ret, ret, scratch);
            }
        }
    }

    *ed = ret;
    return addr;
}

void jump_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MESSAGE(LOG_DUMP, "Jump to epilog\n");
    if(reg) {
        if(reg!=xRIP) {
            MOVx(xRIP, reg);
        }
    } else {
        TABLE64(xRIP, ip);
    }
    TABLE64(x2, (uintptr_t)arm64_epilog);
    BR(x2);
}

void jump_to_next(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MESSAGE(LOG_DUMP, "Jump to next\n");

    if(reg) {
        if(reg!=xRIP) {
            MOVx(xRIP, reg);
        }
        uintptr_t tbl = getJumpTable64();
        TABLE64(x2, tbl);
        UBFXx(x3, xRIP, 48, JMPTABL_SHIFT);
        LDRx_REG_LSL3(x2, x2, x3);
        UBFXx(x3, xRIP, 32, JMPTABL_SHIFT);
        LDRx_REG_LSL3(x2, x2, x3);
        UBFXx(x3, xRIP, 16, JMPTABL_SHIFT);
        LDRx_REG_LSL3(x2, x2, x3);
        LDRx_REG_UXTW(x3, x2, xRIP);
    } else {
        uintptr_t p = getJumpTableAddress64(ip); 
        TABLE64(x2, p);
        TABLE64(xRIP, ip);
        LDRx_U12(x3, x2, 0);
    }
    MOVx(x1, xRIP);
    #ifdef HAVE_TRACE
    //MOVx(x2, 15);    no access to PC reg 
    #endif
    BR(x3);
}

void ret_to_epilog(dynarec_arm_t* dyn, int ninst)
{
    MESSAGE(LOG_DUMP, "Ret next\n");
    POP1(xRIP);
    uintptr_t tbl = getJumpTable64();
    MOV64x(x2, tbl);
    UBFXx(x3, xRIP, 48, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, 32, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, 16, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    LDRx_REG_UXTW(x2, x2, xRIP);
    BR(x2);
}

void retn_to_epilog(dynarec_arm_t* dyn, int ninst, int n)
{
    MESSAGE(LOG_DUMP, "Retn epilog\n");
    POP1(xRIP);
    if(n>0xfff) {
        MOV32w(w1, n);
        ADDx_REG(xRSP, xRSP, x1);
    } else {
        ADDx_U12(xRSP, xRSP, n);
    }
    uintptr_t tbl = getJumpTable64();
    MOV64x(x2, tbl);
    UBFXx(x3, xRIP, 48, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, 32, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, 16, JMPTABL_SHIFT);
    LDRx_REG_LSL3(x2, x2, x3);
    LDRx_REG_UXTW(x2, x2, xRIP);
    BR(x2);
}

void iret_to_epilog(dynarec_arm_t* dyn, int ninst)
{
    MESSAGE(LOG_DUMP, "IRet epilog\n");
    // POP IP
    POP1(xRIP);
    // POP CS
    POP1(x2);
    STRH_U12(x2, xEmu, offsetof(x64emu_t, segs[_CS]));
    MOVZw(x1, 0);
    STRx_U12(x1, xEmu, offsetof(x64emu_t, segs_serial[_CS]));
    // POP EFLAGS
    POP1(xFlags);
    MOV32w(x1, 0x3F7FD7);
    ANDx_REG(xFlags, xFlags, x1);
    ORRx_U13(xFlags, xFlags, 2);
    SET_DFNONE(x1);
    // Ret....
    MOV64x(x2, (uintptr_t)arm64_epilog);  // epilog on purpose, CS might have changed!
    BR(x2);
}

void call_c(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int ret, int saveflags)
{
    if(ret!=-2) {
        STRx_S9_preindex(xEmu, xSP, -16);   // ARM64 stack needs to be 16byte aligned
    }
    fpu_pushcache(dyn, ninst, reg);
    if(saveflags) {
        STRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    MOV64x(reg, (uintptr_t)fnc);
    BLR(reg);
    fpu_popcache(dyn, ninst, reg);
    if(ret>=0) {
        MOVx(ret, xEmu);
    }
    if(ret!=-2) {
        LDRx_S9_postindex(xEmu, xSP, 16);
    }
    if(saveflags) {
        LDRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    SET_NODF();
}

void grab_tlsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg)
{
    MESSAGE(LOG_DUMP, "Get TLSData\n");
    int32_t j32;
    MAYUSE(j32);
    int t1 = x1, t2 = x4;
    if(reg==t1) ++t1;
    if(reg==t2) ++t2;
    LDRx_U12(t1, xEmu, offsetof(x64emu_t, context));
    LDRx_U12(t2, xEmu, offsetof(x64emu_t, segs_serial[_GS]));  // complete check here
    LDRx_U12(t1, t1, offsetof(box64context_t, sel_serial));
    LDRx_U12(reg, xEmu, offsetof(x64emu_t, segs_offs[_GS]));    // no condition LDR
    SUBx_REG(t1, t1, t2);
    CBZx_MARKSEG(t1);
    MOVZw(x1, _GS);
    call_c(dyn, ninst, GetSegmentBaseEmu, t2, reg, 1);
    MARKSEG;
    MESSAGE(LOG_DUMP, "----TLSData\n");
}

void grab_fsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg)
{
    int32_t j32;
    MAYUSE(j32);
    MESSAGE(LOG_DUMP, "Get FS: Offset\n");
    int t1 = x1, t2 = x4;
    if(reg==t1) ++t1;
    if(reg==t2) ++t2;
    LDRx_U12(t2, xEmu, offsetof(x64emu_t, segs_serial[_FS]));// fast check here
    LDRx_U12(reg, xEmu, offsetof(x64emu_t, segs_offs[_FS]));
    CBZx_MARKSEG(t2);
    MOVZw(x1, _FS);
    call_c(dyn, ninst, GetSegmentBaseEmu, t2, reg, 1);
    MARKSEG;
    MESSAGE(LOG_DUMP, "----FS: Offset\n");
}

// x87 stuffs
static void x87_reset(dynarec_arm_t* dyn, int ninst)
{
#if STEP > 1
    for (int i=0; i<8; ++i)
        dyn->x87cache[i] = -1;
    dyn->x87stack = 0;
#endif
}

void x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch)
{
#if STEP > 1
    if(!dyn->x87stack)
        return;
    MESSAGE(LOG_DUMP, "\tSynch x87 Stackcount (%d)\n", dyn->x87stack);
    int a = dyn->x87stack;
    // Add x87stack to emu fpu_stack
    LDRx_U12(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    if(a>0) {
        ADDx_U12(scratch, scratch, a);
    } else {
        SUBx_U12(scratch, scratch, -a);
    }
    STRx_U12(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    // Sub x87stack to top, with and 7
    LDRx_U12(scratch, xEmu, offsetof(x64emu_t, top));
    if(a>0) {
        SUBx_U12(scratch, scratch, a);
    } else {
        ADDx_U12(scratch, scratch, -a);
    }
    ADDx_U12(scratch, scratch, 7);
    STRx_U12(scratch, xEmu, offsetof(x64emu_t, top));
    // reset x87stack
    dyn->x87stack = 0;
    MESSAGE(LOG_DUMP, "\t------x87 Stackcount\n");
#endif
}

int x87_do_push(dynarec_arm_t* dyn, int ninst)
{
#if STEP > 1
    dyn->x87stack+=1;
    // move all regs in cache, and find a free one
    int ret = -1;
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1)
            ++dyn->x87cache[i];
        else if(ret==-1) {
            dyn->x87cache[i] = 0;
            ret=dyn->x87reg[i]=fpu_get_reg_double(dyn);
        }
    return ret;
#else
    return 0;
#endif
}
void x87_do_push_empty(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    dyn->x87stack+=1;
    // move all regs in cache
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1)
            ++dyn->x87cache[i];
    if(s1)
        x87_stackcount(dyn, ninst, s1);
#endif
}
void x87_do_pop(dynarec_arm_t* dyn, int ninst)
{
#if STEP > 1
    dyn->x87stack-=1;
    // move all regs in cache, poping ST0
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1) {
            --dyn->x87cache[i];
            if(dyn->x87cache[i]==-1) {
                fpu_free_reg_double(dyn, dyn->x87reg[i]);
                dyn->x87reg[i] = -1;
            }
        }
#endif
}

static void x87_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
#if STEP > 1
    int ret = 0;
    for (int i=0; i<8 && !ret; ++i)
        if(dyn->x87cache[i] != -1)
            ret = 1;
    if(!ret && !dyn->x87stack)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tPurge x87 Cache and Synch Stackcount (%+d)\n", dyn->x87stack);
    int a = dyn->x87stack;
    if(a!=0) {
        // reset x87stack
        dyn->x87stack = 0;
        // Add x87stack to emu fpu_stack
        LDRx_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        if(a>0) {
            ADDx_U12(s2, s2, a);
        } else {
            SUBx_U12(s2, s2, -a);
        }
        STRx_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LDRx_U12(s2, xEmu, offsetof(x64emu_t, top));
        // update tags (and top at the same time)
        if(a>0) {
            // new tag to fulls
            MOVZw(s3, 0);
            ADDx_U12(s1, xEmu, offsetof(x64emu_t, p_regs));
            for (int i=0; i<a; ++i) {
                SUBw_U12(s2, s2, 1);
                ANDw_U12(s2, s2, 7);    // (emu->top + st)&7
                STRw_REG_LSL2(s3, s1, s2);
            }
        } else {
            // empty tags
            MOVZw(s3, 0b11);
            ADDx_U12(s1, xEmu, offsetof(x64emu_t, p_regs));
            for (int i=0; i<-a; ++i) {
                STRw_REG_LSL2(s3, s1, s2);
                ADDw_U12(s2, s2, 1);
                ANDw_U12(s2, s2, 7);    // (emu->top + st)&7
            }
        }
        STRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    } else {
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    }
    if(ret!=0) {
        // --- set values
        // prepare offset to fpu => s1
        ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
        // Get top
        // loop all cache entries
        for (int i=0; i<8; ++i)
            if(dyn->x87cache[i]!=-1) {
                ADDw_U12(s3, s2, dyn->x87cache[i]);
                ANDw_U12(s3, s3, 7);    // (emu->top + st)&7
                VSTR64_REG_LSL3(dyn->x87reg[i], s1, s3);
                fpu_free_reg_double(dyn, dyn->x87reg[i]);
                dyn->x87reg[i] = -1;
                dyn->x87cache[i] = -1;
            }
    }
#endif
}

#ifdef HAVE_TRACE
static void x87_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
#if STEP > 1
    x87_stackcount(dyn, ninst, s1);
    int ret = 0;
    for (int i=0; (i<8) && (!ret); ++i)
        if(dyn->x87cache[i] != -1)
            ret = 1;
    if(!ret)    // nothing to do
        return;
    // prepare offset to fpu => s1
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
    // Get top
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    // loop all cache entries
    for (int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1) {
            ADDw_U12(s3, s2, dyn->x87cache[i]);
            ANDw_U12(s3, s3, 7);    // (emu->top + i)&7
            VLDR64_REG_LSL3(dyn->x87reg[i], s1, s3);
        }
#endif
}
#endif

int x87_get_cache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
#if STEP > 1
    // search in cache first
    for (int i=0; i<8; ++i)
        if(dyn->x87cache[i]==st)
            return i;
    MESSAGE(LOG_DUMP, "\tCreate x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->x87cache[ret] = st;
    dyn->x87reg[ret] = fpu_get_reg_double(dyn);
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    int a = st - dyn->x87stack;
    if(a) {
        if(a<0) {
            SUBw_U12(s2, s2, -a);
        } else {
            ADDw_U12(s2, s2, a);
        }
        ANDw_U12(s2, s2, 7);
    }
    VLDR64_REG_LSL3(dyn->x87reg[ret], s1, s2);
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);

    return ret;
#else
    return 0;
#endif
}

int x87_get_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a)
{
#if STEP > 1
    return dyn->x87reg[x87_get_cache(dyn, ninst, s1, s2, a)];
#else
    return 0;
#endif
}


void x87_refresh(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
#if STEP > 1
    x87_stackcount(dyn, ninst, s1);
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
    // prepare offset to fpu => s1
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
    // Get top
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    // Update
    if(st) {
        ADDw_U12(s2, s2, st);
        ANDw_U12(s2, s2, 7);    // (emu->top + i)&7
    }
    VLDR64_REG_LSL3(dyn->x87reg[ret], s1, s2);
    MESSAGE(LOG_DUMP, "\t--------x87 Cache for ST%d\n", st);
#endif
}

void x87_forget(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
#if STEP > 1
    x87_stackcount(dyn, ninst, s1);
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tForget x87 Cache for ST%d\n", st);
    // prepare offset to fpu => s1
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
    // Get top
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    // Update
    if(st) {
        ADDw_U12(s2, s2, st);
        ANDw_U12(s2, s2, 7);    // (emu->top + i)&7
    }
    VLDR64_REG_LSL3(dyn->x87reg[ret], s1, s2);
    MESSAGE(LOG_DUMP, "\t--------x87 Cache for ST%d\n", st);
    // and forget that cache
    fpu_free_reg_double(dyn, dyn->x87reg[ret]);
    dyn->x87cache[ret] = -1;
    dyn->x87reg[ret] = -1;
#endif
}

void x87_reget_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
#if STEP > 1
    // search in cache first
    for (int i=0; i<8; ++i)
        if(dyn->x87cache[i]==st) {
            // refresh the value
            MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
            ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
            LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
            int a = st - dyn->x87stack;
            if(a<0) {
                SUBw_U12(s2, s2, -a);
            } else {
                ADDw_U12(s2, s2, a);
            }
            ANDw_U12(s2, s2, 7);    // (emu->top + i)&7
            VLDR64_REG_LSL3(dyn->x87reg[i], s1, s2);
            MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
            // ok
            return;
        }
    // Was not in the cache? creating it....
    MESSAGE(LOG_DUMP, "\tCreate x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->x87cache[ret] = st;
    dyn->x87reg[ret] = fpu_get_reg_double(dyn);
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, mmx87));
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    int a = st - dyn->x87stack;
    if(a<0) {
        SUBw_U12(s2, s2, -a);
    } else {
        ADDw_U12(s2, s2, a);
    }
    ANDw_U12(s2, s2, 7);    // (emu->top + i)&7
    VLDR64_REG_LSL3(dyn->x87reg[ret], s1, s2);
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
#endif
}

static int round_map[] = {0, 2, 1, 3};  // map x64 -> arm round flag

// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    LDRH_U12(s1, xEmu, offsetof(x64emu_t, cw));
    UBFXx(s2, s1, 10, 2);    // extract round...
    MOV64x(s1, (uintptr_t)round_map);
    LDRw_REG_LSL2(s2, s1, s2);
    VMRS(s1);               // get fpscr
    MOVx(s3, s1);
    BFIx(s1, s2, 22, 2);     // inject new round
    VMSR(s1);               // put new fpscr
    return s3;
}

// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    LDRH_U12(s1, xEmu, offsetof(x64emu_t, mxcsr));
    UBFXx(s2, s1, 13, 2);    // extract round...
    MOV64x(s1, (uintptr_t)round_map);
    LDRw_REG_LSL2(s2, s1, s2);
    VMRS(s1);               // get fpscr
    MOVx(s3, s1);
    BFIx(s1, s2, 22, 2);     // inject new round
    VMSR(s1);               // put new fpscr
    return s3;
}

// Restore round flag
void x87_restoreround(dynarec_arm_t* dyn, int ninst, int s1)
{
    VMSR(s1);               // put back fpscr
}

// MMX helpers
static void mmx_reset(dynarec_arm_t* dyn, int ninst)
{
#if STEP > 1
    for (int i=0; i<8; ++i)
        dyn->mmxcache[i] = -1;
#endif
}
// get neon register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
#if STEP > 1
    if(dyn->mmxcache[a]!=-1)
        return dyn->mmxcache[a];
    int ret = dyn->mmxcache[a] = fpu_get_reg_double(dyn);
    VLDR64_U12(ret, xEmu, offsetof(x64emu_t, mmx87[a]));
    return ret;
#else
    return 0;
#endif
}
// get neon register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
#if STEP > 1
    if(dyn->mmxcache[a]!=-1)
        return dyn->mmxcache[a];
    int ret = dyn->mmxcache[a] = fpu_get_reg_double(dyn);
    return ret;
#else
    return 0;
#endif
}
// purge the MMX cache only(needs 3 scratch registers)
static void mmx_purgecache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->mmxcache[i]!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge MMX Cache ------\n");
                ++old;
            }
            VSTR64_U12(dyn->mmxcache[i], xEmu, offsetof(x64emu_t, mmx87[i]));
            fpu_free_reg_double(dyn, dyn->mmxcache[i]);
            dyn->mmxcache[i] = -1;
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge MMX Cache\n");
    }
#endif
}
#ifdef HAVE_TRACE
static void mmx_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    for (int i=0; i<8; ++i)
        if(dyn->mmxcache[i]!=-1) {
            VLDR64_U12(dyn->mmxcache[i], xEmu, offsetof(x64emu_t, mmx87[i]));
        }
#endif
}
#endif


// SSE / SSE2 helpers
static void sse_reset(dynarec_arm_t* dyn, int ninst)
{
#if STEP > 1
    for (int i=0; i<16; ++i)
        dyn->ssecache[i] = -1;
#endif
}
// get neon register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
#if STEP > 1
    if(dyn->ssecache[a]!=-1)
        return dyn->ssecache[a];
    int ret = dyn->ssecache[a] = fpu_get_reg_quad(dyn);
    VLDR128_U12(ret, xEmu, offsetof(x64emu_t, xmm[a]));
    return ret;
#else
    return 0;
#endif
}
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
#if STEP > 1
    if(dyn->ssecache[a]!=-1)
        return dyn->ssecache[a];
    int ret = dyn->ssecache[a] = fpu_get_reg_quad(dyn);
    return ret;
#else
    return 0;
#endif
}
// purge the SSE cache for XMM0..XMM7 (to use before function native call)
void sse_purge07cache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->ssecache[i]!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge XMM0..7 Cache ------\n");
                ++old;
            }
            VSTR128_U12(dyn->ssecache[i], xEmu, offsetof(x64emu_t, xmm[i]));
            fpu_free_reg_quad(dyn, dyn->ssecache[i]);
            dyn->ssecache[i] = -1;
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge XMM0..7 Cache\n");
    }
#endif
}

// purge the SSE cache only
static void sse_purgecache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    int old = -1;
    for (int i=0; i<16; ++i)
        if(dyn->ssecache[i]!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge SSE Cache ------\n");
                ++old;
            }
            VSTR128_U12(dyn->ssecache[i], xEmu, offsetof(x64emu_t, xmm[i]));
            fpu_free_reg_quad(dyn, dyn->ssecache[i]);
            dyn->ssecache[i] = -1;
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge SSE Cache\n");
    }
#endif
}
#ifdef HAVE_TRACE
static void sse_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    for (int i=0; i<16; ++i)
        if(dyn->ssecache[i]!=-1) {
            VSTR128_U12(dyn->ssecache[i], xEmu, offsetof(x64emu_t, xmm[i]));
        }
#endif
}
#endif

void fpu_pushcache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    // only need to push 16-31...
    int n=0;
    for (int i=8; i<32; i++)
        if(dyn->fpuused[i-8])
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "\tPush FPU Cache (%d)------\n", n);
    SUBx_U12(xSP, xSP, n*16);
    MOV_frmSP(s1);
    for (int i=8; i<32; ++i) {
        if(dyn->fpuused[i-8]) {
            VSTR128_S9_postindex(i, s1, 16);
        }
    }
    MESSAGE(LOG_DUMP, "\t------- Push FPU Cache (%d)\n", n);
#endif
}

void fpu_popcache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    // we need to push 8-31 (because on 8..15 only low part is preserved)
    int n=0;
    for (int i=8; i<32; i++)
        if(dyn->fpuused[i-8])
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "\tPop FPU Cache (%d)------\n", n);
    MOV_frmSP(s1);
    for (int i=8; i<32; ++i) {
        if(dyn->fpuused[i-8]) {
            VLDR128_S9_postindex(i, s1, 16);
        }
    }
    ADDx_U12(xSP, xSP, n*16);
    MESSAGE(LOG_DUMP, "\t------- Pop FPU Cache (%d)\n", n);
#endif
}

void fpu_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_purgecache(dyn, ninst, s1, s2, s3);
    mmx_purgecache(dyn, ninst, s1);
    sse_purgecache(dyn, ninst, s1);
    fpu_reset_reg(dyn);
}

#ifdef HAVE_TRACE
void fpu_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_reflectcache(dyn, ninst, s1, s2, s3);
    if(trace_emm)
       mmx_reflectcache(dyn, ninst, s1);
    if(trace_xmm)
       sse_reflectcache(dyn, ninst, s1);
}
#endif

void fpu_reset(dynarec_arm_t* dyn, int ninst)
{
    x87_reset(dyn, ninst);
    mmx_reset(dyn, ninst);
    sse_reset(dyn, ninst);
    fpu_reset_reg(dyn);
}

void emit_pf(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    // PF: (((emu->x64emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
    ANDw_U12(s3, s1, 0xE0); // lsr 5 masking pre-applied
    LSRw(s3, s3, 5);
    MOV64x(s4, (uintptr_t)GetParityTab());
    LDRw_REG_LSL2(s4, s4, s3);
    ANDw_U12(s3, s1, 31);
    LSRw_REG(s4, s4, s3);
    MVNx(s4, s4);
    BFIx(xFlags, s4, F_PF, 1);
}