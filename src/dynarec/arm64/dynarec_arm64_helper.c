#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

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
#include "dynarec_native.h"
#include "../dynablock_private.h"
#include "custommem.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "dynarec_arm64_functions.h"
#include "dynarec_arm64_helper.h"

static uintptr_t geted_32(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int* unscaled, int absmax, uint32_t mask, int* l, int s);

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int* unscaled, int absmax, uint32_t mask, rex_t rex, int *l, int s, int delta)
{
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(delta);

    if(l==LOCK_LOCK) { /*SMDMB();*/DMB_ISH(); }

    if(rex.is32bits)
        return geted_32(dyn, addr, ninst, nextop, ed, hint, fixaddress, unscaled, absmax, mask, l, s);

    int lock = l?((l==LOCK_LOCK)?1:2):0;
    if(unscaled)
        *unscaled = 0;
    if(lock==2)
        *l = 0;
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
            int sib_reg = ((sib>>3)&7)+(rex.x<<3);
            if((sib&0x7)==5) {
                int64_t tmp = F32S;
                if (sib_reg!=4) {
                    if(tmp && ((!((tmp>=absmin) && (tmp<=absmax) && !(tmp&mask))) || !(unscaled && (tmp>-256) && (tmp<256)))) {
                        MOV64x(scratch, tmp);
                        ADDx_REG_LSL(ret, scratch, xRAX+sib_reg, (sib>>6));
                    } else {
                        if(sib>>6) {
                            LSLx(ret, xRAX+sib_reg, (sib>>6));
                        } else
                            ret = xRAX+sib_reg;
                        if(unscaled && (tmp>-256) && (tmp<256))
                            *unscaled = 1;
                        *fixaddress = tmp;
                    }
                } else {
                    switch(lock) {
                        case 1: addLockAddress(tmp); if(fixaddress) *fixaddress=tmp; break;
                        case 2: if(isLockAddress(tmp)) *l=1; break;
                    }
                    MOV64x(ret, tmp);
                }
            } else {
                if (sib_reg!=4) {
                    ADDx_REG_LSL(ret, xRAX+(sib&0x7)+(rex.b<<3), xRAX+sib_reg, (sib>>6));
                } else {
                    ret = xRAX+(sib&0x7)+(rex.b<<3);
                }
            }
        } else if((nextop&7)==5) {
            int64_t tmp = F32S64;
            if((tmp>=absmin) && (tmp<=absmax) && !(tmp&mask)) {
                GETIP(addr+delta);
                ret = xRIP;
                *fixaddress = tmp;
            } else if(unscaled && (tmp>-256) && (tmp<256)) {
                GETIP(addr+delta);
                ret = xRIP;
                *fixaddress = tmp;
                *unscaled = 1;
            } else if(tmp>0 && tmp<0x1000) {
                GETIP(addr+delta);
                ADDx_U12(ret, xRIP, tmp);
            } else if(tmp<0 && tmp>-0x1000) {
                GETIP(addr+delta);
                SUBx_U12(ret, xRIP, -tmp);
            } else if(tmp+addr+delta<0x1000000000000LL) {  // 3 opcodes to load immediate is cheap enough
                MOV64x(ret, tmp+addr+delta);
            } else {
                MOV64x(ret, tmp);
                GETIP(addr+delta);
                ADDx_REG(ret, ret, xRIP);
            }
            switch(lock) {
                case 1: addLockAddress(addr+delta+tmp); if(fixaddress) *fixaddress=addr+delta+tmp; break;
                case 2: if(isLockAddress(addr+delta+tmp)) *l=1; break;
            }
        } else {
            ret = xRAX+(nextop&7)+(rex.b<<3);
        }
    } else {
        int64_t i64;
        uint8_t sib = 0;
        int sib_reg = 0;
        if((nextop&7)==4) {
            sib = F8;
            sib_reg = ((sib>>3)&7)+(rex.x<<3);
        }
        if(nextop&0x80)
            i64 = F32S;
        else
            i64 = F8S;
        if(i64==0 || ((i64>=absmin) && (i64<=absmax)  && !(i64&mask)) || (unscaled && (i64>-256) && (i64<256))) {
            *fixaddress = i64;
            if(unscaled && i64 && (i64>-256) && (i64<256))
                *unscaled = 1;
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    ADDx_REG_LSL(ret, xRAX+(sib&0x07)+(rex.b<<3), xRAX+sib_reg, (sib>>6));
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
                        ADDx_REG_LSL(scratch, xRAX+(sib&0x07)+(rex.b<<3), xRAX+sib_reg, (sib>>6));
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
                        ADDx_REG_LSL(ret, scratch, xRAX+sib_reg, (sib>>6));
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

static uintptr_t geted_32(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int* unscaled, int absmax, uint32_t mask, int* l, int s)
{
    MAYUSE(dyn); MAYUSE(ninst);

    int lock = l?((l==LOCK_LOCK)?1:2):0;
    if(unscaled)
        *unscaled = 0;
    if(lock==2)
        *l = 0;
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
                int64_t tmp = F32S;
                if (sib_reg!=4) {
                    if(tmp && ((!((tmp>=absmin) && (tmp<=absmax) && !(tmp&mask))) || !(unscaled && (tmp>-256) && (tmp<256)))) {
                        MOV32w(scratch, tmp);
                        ADDw_REG_LSL(ret, scratch, xRAX+sib_reg, (sib>>6));
                    } else {
                        LSLw(ret, xRAX+sib_reg, (sib>>6));
                        *fixaddress = tmp;
                        if(unscaled && (tmp>-256) && (tmp<256))
                            *unscaled = 1;
                    }
                } else {
                    switch(lock) {
                        case 1: addLockAddress((int32_t)tmp); if(fixaddress) *fixaddress=(int32_t)tmp; break;
                        case 2: if(isLockAddress((int32_t)tmp)) *l=1; break;
                    }
                    MOV32w(ret, tmp);
                }
            } else {
                if (sib_reg!=4) {
                    ADDw_REG_LSL(ret, xRAX+(sib&0x7), xRAX+sib_reg, (sib>>6));
                } else {
                    ret = xRAX+(sib&0x7);
                }
            }
        } else if((nextop&7)==5) {
            uint64_t tmp = F32;
            MOV32w(ret, tmp);
            switch(lock) {
                case 1: addLockAddress(tmp); if(fixaddress) *fixaddress=tmp; break;
                case 2: if(isLockAddress(tmp)) *l=1; break;
            }
        } else {
            ret = xRAX+(nextop&7);
            if(ret==hint) {
                MOVw_REG(hint, ret);    //to clear upper part
            }
        }
    } else {
        int64_t i32;
        uint8_t sib = 0;
        int sib_reg = 0;
        if((nextop&7)==4) {
            sib = F8;
            sib_reg = (sib>>3)&7;
        }
        if(nextop&0x80)
            i32 = F32S;
        else
            i32 = F8S;
        if(i32==0 || ((i32>=absmin) && (i32<=absmax)  && !(i32&mask)) || (unscaled && (i32>-256) && (i32<256))) {
            *fixaddress = i32;
            if(unscaled && i32 && (i32>-256) && (i32<256))
                *unscaled = 1;
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    ADDw_REG_LSL(ret, xRAX+(sib&0x07), xRAX+sib_reg, (sib>>6));
                } else {
                    ret = xRAX+(sib&0x07);
                }
            } else {
                ret = xRAX+(nextop&0x07);
            }
        } else {
            int64_t sub = (i32<0)?1:0;
            if(sub) i32 = -i32;
            if(i32<0x1000) {
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADDw_REG_LSL(scratch, xRAX+(sib&0x07), xRAX+sib_reg, (sib>>6));
                    } else {
                        scratch = xRAX+(sib&0x07);
                    }
                } else
                    scratch = xRAX+(nextop&0x07);
                if(sub) {
                    SUBw_U12(ret, scratch, i32);
                } else {
                    ADDw_U12(ret, scratch, i32);
                }
            } else {
                MOV32w(scratch, i32);
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        if(sub) {
                            SUBw_REG(scratch, xRAX+(sib&0x07), scratch);
                        } else {
                            ADDw_REG(scratch, scratch, xRAX+(sib&0x07));
                        }
                        ADDw_REG_LSL(ret, scratch, xRAX+sib_reg, (sib>>6));
                    } else {
                        PASS3(int tmp = xRAX+(sib&0x07));
                        if(sub) {
                            SUBw_REG(ret, tmp, scratch);
                        } else {
                            ADDw_REG(ret, tmp, scratch);
                        }
                    }
                } else {
                    PASS3(int tmp = xRAX+(nextop&0x07));
                    if(sub) {
                        SUBw_REG(ret, tmp, scratch);
                    } else {
                        ADDw_REG(ret, tmp, scratch);
                    }
                }
            }
        }
    }
    *ed = ret;
    return addr;
}

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted32(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int* unscaled, int absmax, uint32_t mask, rex_t rex, int* l, int s, int delta)
{
    MAYUSE(dyn); MAYUSE(ninst); MAYUSE(delta);

    int lock = l?((l==LOCK_LOCK)?1:2):0;
    if(unscaled)
        *unscaled = 0;
    if(lock==2)
        *l = 0;
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
            int sib_reg = ((sib>>3)&7)+(rex.x<<3);
            if((sib&0x7)==5) {
                int64_t tmp = F32S;
                if (sib_reg!=4) {
                    if(tmp && ((!((tmp>=absmin) && (tmp<=absmax) && !(tmp&mask))) || !(unscaled && (tmp>-256) && (tmp<256)))) {
                        MOV64x(scratch, tmp);
                        ADDw_REG_LSL(ret, scratch, xRAX+sib_reg, (sib>>6));
                    } else {
                        LSLw(ret, xRAX+sib_reg, (sib>>6));
                        *fixaddress = tmp;
                        if(unscaled && (tmp>-256) && (tmp<256))
                            *unscaled = 1;
                    }
                } else {
                    switch(lock) {
                        case 1: addLockAddress(tmp); break;
                        case 2: if(isLockAddress(tmp)) *l=1; break;
                    }
                    MOV64x(ret, tmp);
                }
            } else {
                if (sib_reg!=4) {
                    ADDw_REG_LSL(ret, xRAX+(sib&0x7)+(rex.b<<3), xRAX+sib_reg, (sib>>6));
                } else {
                    ret = xRAX+(sib&0x7)+(rex.b<<3);
                }
            }
        } else if((nextop&7)==5) {
            uint32_t tmp = F32;
            MOV32w(ret, tmp);
            GETIP(addr+delta);
            ADDw_REG(ret, ret, xRIP);
            switch(lock) {
                case 1: addLockAddress(addr+delta+tmp); break;
                case 2: if(isLockAddress(addr+delta+tmp)) *l=1; break;
            }
        } else {
            ret = xRAX+(nextop&7)+(rex.b<<3);
            if(ret==hint) {
                MOVw_REG(hint, ret);    //to clear upper part
            }
        }
    } else {
        int64_t i64;
        uint8_t sib = 0;
        int sib_reg = 0;
        if((nextop&7)==4) {
            sib = F8;
            sib_reg = ((sib>>3)&7)+(rex.x<<3);
        }
        if(nextop&0x80)
            i64 = F32S;
        else
            i64 = F8S;
        if(i64==0 || ((i64>=absmin) && (i64<=absmax)  && !(i64&mask)) || (unscaled && (i64>-256) && (i64<256))) {
            *fixaddress = i64;
            if(unscaled && (i64>-256) && (i64<256))
                *unscaled = 1;
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    ADDw_REG_LSL(ret, xRAX+(sib&0x07)+(rex.b<<3), xRAX+sib_reg, (sib>>6));
                } else {
                    ret = xRAX+(sib&0x07)+(rex.b<<3);
                }
            } else {
                ret = xRAX+(nextop&0x07)+(rex.b<<3);
            }
        } else {
            int64_t sub = (i64<0)?1:0;
            if(sub) i64 = -i64;
            if(i64<0x1000) {
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADDw_REG_LSL(scratch, xRAX+(sib&0x07)+(rex.b<<3), xRAX+sib_reg, (sib>>6));
                    } else {
                        scratch = xRAX+(sib&0x07)+(rex.b<<3);
                    }
                } else
                    scratch = xRAX+(nextop&0x07)+(rex.b<<3);
                if(sub) {
                    SUBw_U12(ret, scratch, i64);
                } else {
                    ADDw_U12(ret, scratch, i64);
                }
            } else {
                MOV32w(scratch, i64);
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        if(sub) {
                            SUBw_REG(scratch, xRAX+(sib&0x07)+(rex.b<<3), scratch);
                        } else {
                            ADDw_REG(scratch, scratch, xRAX+(sib&0x07)+(rex.b<<3));
                        }
                        ADDw_REG_LSL(ret, scratch, xRAX+sib_reg, (sib>>6));
                    } else {
                        PASS3(int tmp = xRAX+(sib&0x07)+(rex.b<<3));
                        if(sub) {
                            SUBw_REG(ret, tmp, scratch);
                        } else {
                            ADDw_REG(ret, tmp, scratch);
                        }
                    }
                } else {
                    PASS3(int tmp = xRAX+(nextop&0x07)+(rex.b<<3));
                    if(sub) {
                        SUBw_REG(ret, tmp, scratch);
                    } else {
                        ADDw_REG(ret, tmp, scratch);
                    }
                }
            }
        }
    }
    *ed = ret;
    return addr;
}

/* setup r2 to address pointed by ED, r3 as scratch also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted16(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int64_t* fixaddress, int* unscaled, int absmax, uint32_t mask, int s)
{
    MAYUSE(dyn); MAYUSE(ninst);

    if(unscaled)
        *unscaled = 0;
    uint8_t ret = x2;
    uint8_t scratch = x3;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    if(scratch==ret) scratch = x2;
    MAYUSE(scratch);
    uint32_t m = nextop&0xC7;
    uint32_t n = (m>>6)&3;
    int64_t offset = 0;
    int absmin = 0;
    if(s) absmin = -absmax;
    if(!n && (m&7)==6) {
        offset = F16S;
        MOVZw(ret, offset);
    } else {
        switch(n) {
            case 0: offset = 0; break;
            case 1: offset = F8S; break;
            case 2: offset = F16S; break;
        }
        if(offset && (offset>=absmax && offset<=absmin && !(offset&mask))) {
            *fixaddress = offset;
            offset = 0;
        }
        if(offset && (unscaled && offset>-256 && offset<256)) {
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
    MAYUSE(dyn); MAYUSE(ip); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to epilog\n");

    if(reg) {
        if(reg!=xRIP) {
            MOVx_REG(xRIP, reg);
        }
    } else {
        GETIP_(ip);
    }
    NOTEST(x2);
    TABLE64(x2, (uintptr_t)arm64_epilog);
    SMEND();
    BR(x2);
}

void jump_to_next(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst, int is32bits)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Jump to next\n");

    SMEND();
    if(reg) {
        if(reg!=xRIP) {
            MOVx_REG(xRIP, reg);
        }
        NOTEST(x2);
        uintptr_t tbl = is32bits?getJumpTable32():getJumpTable64();
        MAYUSE(tbl);
        TABLE64(x3, tbl);
        if(!is32bits) {
            #ifdef JMPTABL_SHIFT4
            UBFXx(x2, xRIP, JMPTABL_START4, JMPTABL_SHIFT4);
            LDRx_REG_LSL3(x3, x3, x2);
            #endif
            UBFXx(x2, xRIP, JMPTABL_START3, JMPTABL_SHIFT3);
            LDRx_REG_LSL3(x3, x3, x2);
        }
        UBFXx(x2, xRIP, JMPTABL_START2, JMPTABL_SHIFT2);
        LDRx_REG_LSL3(x3, x3, x2);
        UBFXx(x2, xRIP, JMPTABL_START1, JMPTABL_SHIFT1);
        LDRx_REG_LSL3(x3, x3, x2);
        UBFXx(x2, xRIP, JMPTABL_START0, JMPTABL_SHIFT0);
        LDRx_REG_LSL3(x2, x3, x2);
    } else {
        NOTEST(x2);
        uintptr_t p = getJumpTableAddress64(ip);
        MAYUSE(p);
        TABLE64(x3, p);
        GETIP_(ip);
        LDRx_U12(x2, x3, 0);
    }
    if(reg!=x1) {
        MOVx_REG(x1, xRIP);
    }
    CLEARIP();
    #ifdef HAVE_TRACE
    //MOVx(x3, 15);    no access to PC reg
    #endif
    BLR(x2); // save LR...
}

void ret_to_epilog(dynarec_arm_t* dyn, int ninst, rex_t rex)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Ret to epilog\n");
    POP1z(xRIP);
    MOVz_REG(x1, xRIP);
    SMEND();
    if(box64_dynarec_callret) {
        // pop the actual return address for ARM stack
        LDPx_S7_postindex(x2, x6, xSP, 16);
        SUBx_REG(x6, x6, xRIP); // is it the right address?
        CBNZx(x6, 2*4);
        BLR(x2);
        // not the correct return address, regular jump, but purge the stack first, it's unsync now...
        SUBx_U12(xSP, xSavedSP, 16);
    }
    uintptr_t tbl = rex.is32bits?getJumpTable32():getJumpTable64();
    NOTEST(x2);
    MOV64x(x2, tbl);
    if(!rex.is32bits) {
        #ifdef JMPTABL_SHIFT4
        UBFXx(x3, xRIP, JMPTABL_START4, JMPTABL_SHIFT4);
        LDRx_REG_LSL3(x2, x2, x3);
        #endif
        UBFXx(x3, xRIP, JMPTABL_START3, JMPTABL_SHIFT3);
        LDRx_REG_LSL3(x2, x2, x3);
    }
    UBFXx(x3, xRIP, JMPTABL_START2, JMPTABL_SHIFT2);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, JMPTABL_START1, JMPTABL_SHIFT1);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, JMPTABL_START0, JMPTABL_SHIFT0);
    LDRx_REG_LSL3(x2, x2, x3);
    BLR(x2); // save LR
    CLEARIP();
}

void retn_to_epilog(dynarec_arm_t* dyn, int ninst, rex_t rex, int n)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "Retn to epilog\n");
    POP1z(xRIP);
    if(n>0xfff) {
        MOV32w(w1, n);
        ADDz_REG(xRSP, xRSP, x1);
    } else {
        ADDz_U12(xRSP, xRSP, n);
    }
    MOVz_REG(x1, xRIP);
    SMEND();
    if(box64_dynarec_callret) {
        // pop the actual return address for ARM stack
        LDPx_S7_postindex(x2, x6, xSP, 16);
        SUBx_REG(x6, x6, xRIP); // is it the right address?
        CBNZx(x6, 2*4);
        BLR(x2);
        // not the correct return address, regular jump
        SUBx_U12(xSP, xSavedSP, 16);
    }
    uintptr_t tbl = rex.is32bits?getJumpTable32():getJumpTable64();
    NOTEST(x2);
    MOV64x(x2, tbl);
    if(!rex.is32bits) {
        #ifdef JMPTABL_SHIFT4
        UBFXx(x3, xRIP, JMPTABL_START4, JMPTABL_SHIFT4);
        LDRx_REG_LSL3(x2, x2, x3);
        #endif
        UBFXx(x3, xRIP, JMPTABL_START3, JMPTABL_SHIFT3);
        LDRx_REG_LSL3(x2, x2, x3);
    }
    UBFXx(x3, xRIP, JMPTABL_START2, JMPTABL_SHIFT2);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, JMPTABL_START1, JMPTABL_SHIFT1);
    LDRx_REG_LSL3(x2, x2, x3);
    UBFXx(x3, xRIP, JMPTABL_START0, JMPTABL_SHIFT0);
    LDRx_REG_LSL3(x2, x2, x3);
    BLR(x2); // save LR
    CLEARIP();
}

void iret_to_epilog(dynarec_arm_t* dyn, int ninst, int is64bits)
{
    //#warning TODO: is64bits
    MAYUSE(ninst);
    MESSAGE(LOG_DUMP, "IRet to epilog\n");
    SMEND();
    SET_DFNONE(x2);
    // POP IP
    NOTEST(x2);
    if(is64bits) {
        POP1(xRIP);
        POP1(x2);
        POP1(xFlags);
    } else {
        POP1_32(xRIP);
        POP1_32(x2);
        POP1_32(xFlags);
    }
    // x2 is CS
    STRH_U12(x2, xEmu, offsetof(x64emu_t, segs[_CS]));
    STRw_U12(xZR, xEmu, offsetof(x64emu_t, segs_serial[_CS]));
    // clean EFLAGS
    MOV32w(x1, 0x3F7FD7);
    ANDx_REG(xFlags, xFlags, x1);
    ORRx_mask(xFlags, xFlags, 1, 0b111111, 0); // xFlags | 0b10
    SET_DFNONE(x1);
    // POP RSP
    if(is64bits) {
        POP1(x3);   //rsp
        POP1(x2);   //ss
    } else {
        POP1_32(x3);   //rsp
        POP1_32(x2);   //ss
    }
    // POP SS
    STRH_U12(x2, xEmu, offsetof(x64emu_t, segs[_SS]));
    STRw_U12(xZR, xEmu, offsetof(x64emu_t, segs_serial[_SS]));
    // set new RSP
    MOVx_REG(xRSP, x3);
    // Ret....
    MOV64x(x2, (uintptr_t)arm64_epilog);  // epilog on purpose, CS might have changed!
    BR(x2);
    CLEARIP();
}

void call_c(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int ret, int saveflags, int savereg)
{
    MAYUSE(fnc);
    if(savereg==0)
        savereg = 7;
    if(saveflags) {
        STRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    fpu_pushcache(dyn, ninst, reg, 0);
    if(ret!=-2) {
        STPx_S7_preindex(xEmu, savereg, xSP, -16);   // ARM64 stack needs to be 16byte aligned
        STPx_S7_offset(xRAX, xRCX, xEmu, offsetof(x64emu_t, regs[_AX]));    // x9..x15, x16,x17,x18 those needs to be saved by caller
        STPx_S7_offset(xRDX, xRBX, xEmu, offsetof(x64emu_t, regs[_DX]));    // but x18 is R8 wich is lost, so it's fine to not save it?
        STPx_S7_offset(xRSP, xRBP, xEmu, offsetof(x64emu_t, regs[_SP]));
        STPx_S7_offset(xRSI, xRDI, xEmu, offsetof(x64emu_t, regs[_SI]));
        STPx_S7_offset(xR8,  xR9,  xEmu, offsetof(x64emu_t, regs[_R8]));
    }
    TABLE64(reg, (uintptr_t)fnc);
    BLR(reg);
    if(ret>=0) {
        MOVx_REG(ret, xEmu);
    }
    if(ret!=-2) {
        LDPx_S7_postindex(xEmu, savereg, xSP, 16);
        #define GO(A, B) if(ret==x##A) {                                        \
            LDRx_U12(x##B, xEmu, offsetof(x64emu_t, regs[_##B]));               \
        } else if(ret==x##B) {                                                  \
            LDRx_U12(x##A, xEmu, offsetof(x64emu_t, regs[_##A]));               \
        } else {                                                                \
            LDPx_S7_offset(x##A, x##B, xEmu, offsetof(x64emu_t, regs[_##A]));   \
        }
        GO(RAX, RCX);
        GO(RDX, RBX);
        GO(RSP, RBP);
        GO(RSI, RDI);
        GO(R8, R9);
        #undef GO
    }
    fpu_popcache(dyn, ninst, reg, 0);
    if(saveflags) {
        LDRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    }
    //SET_NODF();
}

void call_n(dynarec_arm_t* dyn, int ninst, void* fnc, int w)
{
    MAYUSE(fnc);
    STRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    fpu_pushcache(dyn, ninst, x3, 1);
    // x9..x15, x16,x17,x18 those needs to be saved by caller
    // RDI, RSI, RDX, RCX, R8, R9 are used for function call
    STPx_S7_preindex(xEmu, xRBX, xSP, -16);   // ARM64 stack needs to be 16byte aligned
    STPx_S7_offset(xRSP, xRBP, xEmu, offsetof(x64emu_t, regs[_SP]));
    // float and double args
    if(abs(w)>1) {
        MESSAGE(LOG_DUMP, "Getting %d XMM args\n", abs(w)-1);
        for(int i=0; i<abs(w)-1; ++i) {
            sse_get_reg(dyn, ninst, x7, i, w);
        }
    }
    if(w<0) {
        MESSAGE(LOG_DUMP, "Return in XMM0\n");
        sse_get_reg_empty(dyn, ninst, x7, 0);
    }
    // prepare regs for native call
    MOVx_REG(0, xRDI);
    MOVx_REG(x1, xRSI);
    MOVx_REG(x2, xRDX);
    MOVx_REG(x3, xRCX);
    MOVx_REG(x4, xR8);
    MOVx_REG(x5, xR9);
    // native call
    TABLE64(16, (uintptr_t)fnc);    // using x16 as scratch regs for call address
    BLR(16);
    // put return value in x64 regs
    if(w>0) {
        MOVx_REG(xRAX, 0);
        MOVx_REG(xRDX, x1);
    }
    // all done, restore all regs
    LDPx_S7_postindex(xEmu, xRBX, xSP, 16);
    #define GO(A, B) LDPx_S7_offset(x##A, x##B, xEmu, offsetof(x64emu_t, regs[_##A]))
    GO(RSP, RBP);
    #undef GO

    fpu_popcache(dyn, ninst, x3, 1);
    LDRx_U12(xFlags, xEmu, offsetof(x64emu_t, eflags));
    //SET_NODF();
}

void grab_segdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg, int segment)
{
    (void)addr;
    int64_t j64;
    MAYUSE(j64);
    MESSAGE(LOG_DUMP, "Get %s Offset\n", (segment==_FS)?"FS":"GS");
    int t1 = x1, t2 = x4;
    if(reg==t1) ++t1;
    if(reg==t2) ++t2;
    LDRw_U12(t2, xEmu, offsetof(x64emu_t, segs_serial[segment]));
    LDRx_U12(reg, xEmu, offsetof(x64emu_t, segs_offs[segment]));
    if(segment==_GS) {
        CBNZw_MARKSEG(t2);   // fast check
    } else {
        LDRx_U12(t1, xEmu, offsetof(x64emu_t, context));
        LDRw_U12(t1, t1, offsetof(box64context_t, sel_serial));
        SUBw_REG(t1, t1, t2);
        CBZw_MARKSEG(t1);
    }
    MOVZw(x1, segment);
    call_c(dyn, ninst, GetSegmentBaseEmu, t2, reg, 1, 0);
    MARKSEG;
    MESSAGE(LOG_DUMP, "----%s Offset\n", (segment==_FS)?"FS":"GS");
}

// x87 stuffs
int x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch)
{
    MAYUSE(scratch);
    if(!dyn->n.x87stack)
        return 0;
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, scratch);
    MESSAGE(LOG_DUMP, "\tSynch x87 Stackcount (%d)\n", dyn->n.x87stack);
    int a = dyn->n.x87stack;
    // Add x87stack to emu fpu_stack
    LDRw_U12(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    if(a>0) {
        ADDw_U12(scratch, scratch, a);
    } else {
        SUBw_U12(scratch, scratch, -a);
    }
    STRw_U12(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    // Sub x87stack to top, with and 7
    LDRw_U12(scratch, xEmu, offsetof(x64emu_t, top));
    if(a>0) {
        SUBw_U12(scratch, scratch, a);
    } else {
        ADDw_U12(scratch, scratch, -a);
    }
    ANDw_mask(scratch, scratch, 0, 2);  //mask=7
    STRw_U12(scratch, xEmu, offsetof(x64emu_t, top));
    // reset x87stack, but not the stack count of neoncache
    int ret = dyn->n.x87stack;
    dyn->n.x87stack = 0;
    dyn->n.stack_next -= dyn->n.stack;
    dyn->n.stack = 0;
    MESSAGE(LOG_DUMP, "\t------x87 Stackcount\n");
    return ret;
}

void x87_unstackcount(dynarec_arm_t* dyn, int ninst, int scratch, int count)
{
    MAYUSE(scratch);
    if(!count)
        return;
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, scratch);
    MESSAGE(LOG_DUMP, "\tUnsynch x87 Unstackcount (%d)\n", count);
    int a = -count;
    // Add x87stack to emu fpu_stack
    LDRw_U12(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    if(a>0) {
        ADDw_U12(scratch, scratch, a);
    } else {
        SUBw_U12(scratch, scratch, -a);
    }
    STRw_U12(scratch, xEmu, offsetof(x64emu_t, fpu_stack));
    // Sub x87stack to top, with and 7
    LDRw_U12(scratch, xEmu, offsetof(x64emu_t, top));
    if(a>0) {
        SUBw_U12(scratch, scratch, a);
    } else {
        ADDw_U12(scratch, scratch, -a);
    }
    ANDw_mask(scratch, scratch, 0, 2);  //mask=7
    STRw_U12(scratch, xEmu, offsetof(x64emu_t, top));
    // reset x87stack, but not the stack count of neoncache
    dyn->n.x87stack = count;
    dyn->n.stack = count;
    dyn->n.stack_next += dyn->n.stack;
    MESSAGE(LOG_DUMP, "\t------x87 Unstackcount\n");
}

int neoncache_st_coherency(dynarec_arm_t* dyn, int ninst, int a, int b)
{
    int i1 = neoncache_get_st(dyn, ninst, a);
    int i2 = neoncache_get_st(dyn, ninst, b);
    if(i1!=i2) {
        MESSAGE(LOG_DUMP, "Warning, ST cache incoherent between ST%d(%d) and ST%d(%d)\n", a, i1, b, i2);
    }

    return i1;
}

// On step 1, Float/Double for ST is actualy computed and back-propagated
// On step 2-3, the value is just read for inst[...].n.neocache[..]
// the reg returned is *2 for FLOAT
int x87_do_push(dynarec_arm_t* dyn, int ninst, int s1, int t)
{
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->n.x87stack+=1;
    dyn->n.stack+=1;
    dyn->n.stack_next+=1;
    dyn->n.stack_push+=1;
    ++dyn->n.pushed;
    if(dyn->n.poped)
        --dyn->n.poped;
    // move all regs in cache, and find a free one
    for(int j=0; j<24; ++j)
        if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D)
         ||(dyn->n.neoncache[j].t == NEON_CACHE_ST_F)
         ||(dyn->n.neoncache[j].t == NEON_CACHE_ST_I64))
            ++dyn->n.neoncache[j].n;
    int ret = -1;
    dyn->n.tags<<=2;
    for(int i=0; i<8; ++i) {
        if(dyn->n.x87cache[i]!=-1)
            ++dyn->n.x87cache[i];
        else if(ret==-1) {
            dyn->n.x87cache[i] = 0;
            ret=dyn->n.x87reg[i]=fpu_get_reg_x87(dyn, t, 0);
            dyn->n.neoncache[ret].t = X87_ST0;
        }
    }
    if(ret==-1) {
        MESSAGE(LOG_DUMP, "Incoherent x87 stack cache, aborting\n");
        dyn->abort = 1;
    }
    return ret;
}
void x87_do_push_empty(dynarec_arm_t* dyn, int ninst, int s1)
{
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->n.x87stack+=1;
    dyn->n.stack+=1;
    dyn->n.stack_next+=1;
    dyn->n.stack_push+=1;
    ++dyn->n.pushed;
    if(dyn->n.poped)
        --dyn->n.poped;
    // move all regs in cache
    for(int j=0; j<24; ++j)
        if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D)
         ||(dyn->n.neoncache[j].t == NEON_CACHE_ST_F)
         ||(dyn->n.neoncache[j].t == NEON_CACHE_ST_I64))
            ++dyn->n.neoncache[j].n;
    int ret = -1;
    dyn->n.tags<<=2;
    for(int i=0; i<8; ++i) {
        if(dyn->n.x87cache[i]!=-1)
            ++dyn->n.x87cache[i];
        else if(ret==-1)
            ret = i;
    }
    if(ret==-1) {
        MESSAGE(LOG_DUMP, "Incoherent x87 stack cache, aborting\n");
        dyn->abort = 1;
    }
}
void static internal_x87_dopop(dynarec_arm_t* dyn)
{
    for(int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]!=-1) {
            --dyn->n.x87cache[i];
            if(dyn->n.x87cache[i]==-1) {
                fpu_free_reg(dyn, dyn->n.x87reg[i]);
                dyn->n.x87reg[i] = -1;
            }
        }
}
int static internal_x87_dofree(dynarec_arm_t* dyn)
{
    if(dyn->n.tags&0b11) {
        MESSAGE(LOG_DUMP, "\t--------x87 FREED ST0, poping 1 more\n");
        return 1;
    }
    return 0;
}
void x87_do_pop(dynarec_arm_t* dyn, int ninst, int s1)
{
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    do {
        dyn->n.x87stack-=1;
        dyn->n.stack_next-=1;
        dyn->n.stack_pop+=1;
        dyn->n.tags>>=2;
        ++dyn->n.poped;
        if(dyn->n.pushed)
            --dyn->n.pushed;
        // move all regs in cache, poping ST0
        internal_x87_dopop(dyn);
    } while(internal_x87_dofree(dyn));
}
static int x87_is_stcached(dynarec_arm_t* dyn, int st)
{
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i] == st)
            return 1;
    return 0;
}
void x87_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    int ret = 0;
    for (int i=0; i<8 && !ret; ++i)
        if(dyn->n.x87cache[i] != -1)
            ret = 1;
    if(!ret && !dyn->n.x87stack)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tPurge %sx87 Cache and Synch Stackcount (%+d)---\n", next?"locally ":"", dyn->n.x87stack);
    int a = dyn->n.x87stack;
    if(a!=0) {
        // reset x87stack
        if(!next) {
            dyn->n.x87stack = 0;
        }
        // Add x87stack to emu fpu_stack
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        if(a>0) {
            ADDw_U12(s2, s2, a);
        } else {
            SUBw_U12(s2, s2, -a);
        }
        STRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        if(a>0) {
            SUBw_U12(s2, s2, a);
        } else {
            ADDw_U12(s2, s2, -a);
        }
        ANDw_mask(s2, s2, 0, 2);
        STRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        // update tags
        LDRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        if(a>0) {
            LSLw_IMM(s1, s1, a*2);
        } else {
            ORRw_mask(s1, s1, 0b010000, 0b001111);  // 0xffff0000
            LSRw_IMM(s1, s1, -a*2);
        }
        STRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    } else {
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    }
    // check if free is used
    if(dyn->n.tags) {
        LDRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        MOV32w(s3, dyn->n.tags);
        ORRw_REG(s1, s1, s3);
        STRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    }

    if(ret!=0) {
        // --- set values
        // prepare offset to fpu => s1
        ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
        // Get top
        // loop all cache entries
        for (int i=0; i<8; ++i)
            if(dyn->n.x87cache[i]!=-1) {
                int st = dyn->n.x87cache[i]+dyn->n.stack_pop;
                #if STEP == 1
                if(!next) {   // don't force promotion here
                    // pre-apply pop, because purge happens in-between
                    neoncache_promote_double(dyn, ninst, st);
                }
                #endif
                #if STEP == 3
                if(!next && neoncache_get_current_st(dyn, ninst, st)!=NEON_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "Warning, incoherency with purged ST%d cache\n", st);
                }
                #endif
                ADDw_U12(s3, s2, dyn->n.x87cache[i]);   // unadjusted count, as it's relative to real top
                ANDw_mask(s3, s3, 0, 2); //mask=7   // (emu->top + st)&7
                switch(neoncache_get_current_st(dyn, ninst, st)) {
                    case NEON_CACHE_ST_D:
                        VSTR64_REG_LSL3(dyn->n.x87reg[i], s1, s3);    // save the value
                        break;
                    case NEON_CACHE_ST_F:
                        FCVT_D_S(SCRATCH, dyn->n.x87reg[i]);
                        VSTR64_REG_LSL3(SCRATCH, s1, s3);    // save the value
                        break;
                    case NEON_CACHE_ST_I64:
                        SCVTFDD(SCRATCH, dyn->n.x87reg[i]);
                        VSTR64_REG_LSL3(SCRATCH, s1, s3);    // save the value
                        break;
                }
                if(!next) {
                    fpu_free_reg(dyn, dyn->n.x87reg[i]);
                    dyn->n.x87reg[i] = -1;
                    dyn->n.x87cache[i] = -1;
                    //dyn->n.stack_pop+=1; //no pop, but the purge because of barrier will have the n.barrier flags set
                }
            }
    }
    if(!next) {
        dyn->n.stack_next = 0;
        dyn->n.tags = 0;
        #if STEP < 2
        // refresh the cached valued, in case it's a purge outside a instruction
        dyn->insts[ninst].n.barrier = 1;
        dyn->n.pushed = 0;
        dyn->n.poped = 0;

        #endif
    }
    MESSAGE(LOG_DUMP, "\t---Purge x87 Cache and Synch Stackcount\n");
}

static void x87_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    // Synch top & stack counter
    int a = dyn->n.x87stack;
    if(a) {
        // Add x87stack to emu fpu_stack
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        if(a>0) {
            ADDw_U12(s2, s2, a);
        } else {
            SUBw_U12(s2, s2, -a);
        }
        STRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        if(a>0) {
            SUBw_U12(s2, s2, a);
        } else {
            ADDw_U12(s2, s2, -a);
        }
        ANDw_mask(s2, s2, 0, 2);  //mask=7
        STRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        // update tags
        LDRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        if(a>0) {
            LSLw_IMM(s1, s1, a*2);
        } else {
            ORRw_mask(s1, s1, 0b010000, 0b001111);  // 0xffff0000
            LSRw_IMM(s1, s1, -a*2);
        }
        STRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    }
    int ret = 0;
    for (int i=0; (i<8) && (!ret); ++i)
        if(dyn->n.x87cache[i] != -1)
            ret = 1;
    if(!ret)    // nothing to do
        return;
    // prepare offset to fpu => s1
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
    // Get top
    if(!a) {
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    }
    // loop all cache entries
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]!=-1) {
            ADDw_U12(s3, s2, dyn->n.x87cache[i]);
            ANDw_mask(s3, s3, 0, 2); // mask=7   // (emu->top + i)&7
            if(neoncache_get_st_f(dyn, ninst, dyn->n.x87cache[i])>=0) {
                FCVT_D_S(SCRATCH0, dyn->n.x87reg[i]);
                VSTR64_REG_LSL3(SCRATCH0, s1, s3);
            } else
                VSTR64_REG_LSL3(dyn->n.x87reg[i], s1, s3);
        }
}

static void x87_unreflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    // go back with the top & stack counter
    int a = dyn->n.x87stack;
    if(a) {
        // Sub x87stack to emu fpu_stack
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        if(a>0) {
            SUBw_U12(s2, s2, a);
        } else {
            ADDw_U12(s2, s2, -a);
        }
        STRw_U12(s2, xEmu, offsetof(x64emu_t, fpu_stack));
        // Add x87stack to top, with and 7
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        if(a>0) {
            ADDw_U12(s2, s2, a);
        } else {
            SUBw_U12(s2, s2, -a);
        }
        ANDw_mask(s2, s2, 0, 2);  //mask=7
        STRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        // update tags
        LDRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
        if(a>0) {
            ORRw_mask(s1, s1, 0b010000, 0b001111);  // 0xffff0000
            LSRw_IMM(s1, s1, a*2);
        } else {
            LSLw_IMM(s1, s1, -a*2);
        }
        STRH_U12(s1, xEmu, offsetof(x64emu_t, fpu_tags));
    }
}

int x87_get_current_cache(dynarec_arm_t* dyn, int ninst, int st, int t)
{
    // search in cache first
    for (int i=0; i<8; ++i) {
        if(dyn->n.x87cache[i]==st) {
            #if STEP == 1
            if(t==NEON_CACHE_ST_D && (dyn->n.neoncache[dyn->n.x87reg[i]].t==NEON_CACHE_ST_F || dyn->n.neoncache[dyn->n.x87reg[i]].t==NEON_CACHE_ST_I64))
                neoncache_promote_double(dyn, ninst, st);
            else if(t==NEON_CACHE_ST_I64 && (dyn->n.neoncache[dyn->n.x87reg[i]].t==NEON_CACHE_ST_F))
                neoncache_promote_double(dyn, ninst, st);
            else if(t==NEON_CACHE_ST_F && (dyn->n.neoncache[dyn->n.x87reg[i]].t==NEON_CACHE_ST_I64))
                neoncache_promote_double(dyn, ninst, st);
            #endif
            return i;
        }
        assert(dyn->n.x87cache[i]<8);
    }
    return -1;
}

int x87_get_cache(dynarec_arm_t* dyn, int ninst, int populate, int s1, int s2, int st, int t)
{
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    int ret = x87_get_current_cache(dyn, ninst, st, t);
    if(ret!=-1)
        return ret;
    MESSAGE(LOG_DUMP, "\tCreate %sx87 Cache for ST%d\n", populate?"and populate ":"", st);
    // get a free spot
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->n.x87cache[ret] = st;
    dyn->n.x87reg[ret] = fpu_get_reg_x87(dyn, NEON_CACHE_ST_D, st);
    if(populate) {
        ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        int a = st - dyn->n.x87stack;
        if(a) {
            if(a<0) {
                SUBw_U12(s2, s2, -a);
            } else {
                ADDw_U12(s2, s2, a);
            }
            ANDw_mask(s2, s2, 0, 2); //mask=7
        }
        VLDR64_REG_LSL3(dyn->n.x87reg[ret], s1, s2);
    }
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);

    return ret;
}
int x87_get_neoncache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    for(int ii=0; ii<24; ++ii)
        if((dyn->n.neoncache[ii].t == NEON_CACHE_ST_F
         || dyn->n.neoncache[ii].t == NEON_CACHE_ST_D
         || dyn->n.neoncache[ii].t == NEON_CACHE_ST_I64)
         && dyn->n.neoncache[ii].n==st)
            return ii;
    assert(0);
    return -1;
}
int x87_get_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int t)
{
    return dyn->n.x87reg[x87_get_cache(dyn, ninst, 1, s1, s2, a, t)];
}
int x87_get_st_empty(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int t)
{
    return dyn->n.x87reg[x87_get_cache(dyn, ninst, 0, s1, s2, a, t)];
}

void x87_forget(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tForget x87 Cache for ST%d\n", st);
    const int reg = dyn->n.x87reg[ret];
    #if STEP == 1
    if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_F || dyn->n.neoncache[reg].t==NEON_CACHE_ST_I64)
        neoncache_promote_double(dyn, ninst, st);
    #endif
    // prepare offset to fpu => s1
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
    // Get top
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    // Update
    int ast = st - dyn->n.x87stack;
    if(ast) {
        if(ast>0) {
            ADDw_U12(s2, s2, ast);
        } else {
            SUBw_U12(s2, s2, -ast);
        }
        ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
    }
    if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_F) {
        FCVT_D_S(SCRATCH, reg);
        VSTR64_REG_LSL3(SCRATCH, s1, s2);
    } else if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_I64) {
        SCVTFDD(SCRATCH, reg);
        VSTR64_REG_LSL3(SCRATCH, s1, s2);
    } else {
        VSTR64_REG_LSL3(reg, s1, s2);
    }
    MESSAGE(LOG_DUMP, "\t--------x87 Cache for ST%d\n", st);
    // and forget that cache
    fpu_free_reg(dyn, reg);
    dyn->n.neoncache[reg].v = 0;
    dyn->n.x87cache[ret] = -1;
    dyn->n.x87reg[ret] = -1;
}

void x87_reget_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    // search in cache first
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]==st) {
            // refresh the value
            MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
            #if STEP == 1
            if(dyn->n.neoncache[dyn->n.x87reg[i]].t==NEON_CACHE_ST_F || dyn->n.neoncache[dyn->n.x87reg[i]].t==NEON_CACHE_ST_I64)
                neoncache_promote_double(dyn, ninst, st);
            #endif
            ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
            LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
            int a = st - dyn->n.x87stack;
            if(a<0) {
                SUBw_U12(s2, s2, -a);
            } else {
                ADDw_U12(s2, s2, a);
            }
            ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
            VLDR64_REG_LSL3(dyn->n.x87reg[i], s1, s2);
            MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
            // ok
            return;
        }
    // Was not in the cache? creating it....
    MESSAGE(LOG_DUMP, "\tCreate x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->n.x87cache[ret] = st;
    dyn->n.x87reg[ret] = fpu_get_reg_x87(dyn, NEON_CACHE_ST_D, st);
    ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
    LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
    int a = st - dyn->n.x87stack;
    if(a<0) {
        SUBw_U12(s2, s2, -a);
    } else {
        ADDw_U12(s2, s2, a);
    }
    ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
    VLDR64_REG_LSL3(dyn->n.x87reg[ret], s1, s2);
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
}

void x87_free(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int st)
{
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i] == st)
            ret = i;
    MESSAGE(LOG_DUMP, "\tFFREE%s x87 Cache for ST%d\n", (ret!=-1)?" (and Forget)":"", st);
    if(ret!=-1) {
        const int reg = dyn->n.x87reg[ret];
        #if STEP == 1
        if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_F || dyn->n.neoncache[reg].t==NEON_CACHE_ST_I64)
            neoncache_promote_double(dyn, ninst, st);
        #endif
        // prepare offset to fpu => s1
        ADDx_U12(s1, xEmu, offsetof(x64emu_t, x87));
        // Get top
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        // Update
        int ast = st - dyn->n.x87stack;
        if(ast) {
            if(ast>0) {
                ADDw_U12(s2, s2, ast);
            } else {
                SUBw_U12(s2, s2, -ast);
            }
            ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
        }
        if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_F) {
            FCVT_D_S(SCRATCH, reg);
            VSTR64_REG_LSL3(SCRATCH, s1, s2);
        } else if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_I64) {
            SCVTFDD(SCRATCH, reg);
            VSTR64_REG_LSL3(SCRATCH, s1, s2);
        } else {
            VSTR64_REG_LSL3(reg, s1, s2);
        }
        // and forget that cache
        fpu_free_reg(dyn, reg);
        dyn->n.neoncache[reg].v = 0;
        dyn->n.x87cache[ret] = -1;
        dyn->n.x87reg[ret] = -1;
    } else {
        // Get top
        LDRw_U12(s2, xEmu, offsetof(x64emu_t, top));
        // Update
        int ast = st - dyn->n.x87stack;
        if(ast) {
            if(ast>0) {
                ADDw_U12(s2, s2, ast);
            } else {
                SUBw_U12(s2, s2, -ast);
            }
            ANDw_mask(s2, s2, 0, 2); //mask=7    // (emu->top + i)&7
        }
    }
    // add mark in the freed array
    dyn->n.tags |= 0b11<<(st*2);
    MESSAGE(LOG_DUMP, "\t--------x87 FFREE for ST%d\n", st);
}

void x87_swapreg(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int b)
{
    int i1, i2, i3;
    i1 = x87_get_cache(dyn, ninst, 1, s1, s2, b, X87_ST(b));
    i2 = x87_get_cache(dyn, ninst, 1, s1, s2, a, X87_ST(a));
    i3 = dyn->n.x87cache[i1];
    dyn->n.x87cache[i1] = dyn->n.x87cache[i2];
    dyn->n.x87cache[i2] = i3;
    // swap those too
    int j1, j2, j3;
    j1 = x87_get_neoncache(dyn, ninst, s1, s2, b);
    j2 = x87_get_neoncache(dyn, ninst, s1, s2, a);
    j3 = dyn->n.neoncache[j1].n;
    dyn->n.neoncache[j1].n = dyn->n.neoncache[j2].n;
    dyn->n.neoncache[j2].n = j3;
    // mark as swapped
    dyn->n.swapped = 1;
    dyn->n.combined1= a; dyn->n.combined2=b;
}

// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1); MAYUSE(s2);
    LDRw_U12(s1, xEmu, offsetof(x64emu_t, cw));
    BFXILw(s1, s1, 10, 2);
    UBFXw(s2, s1, 1, 1);        // bit 1 of round in bit 0 (zero extented) of s2
    BFIw(s2, s1, 1, 1);         // bit 0 of round in bit 1 of s2
    MRS_fpcr(s1);               // get fpscr
    MOVx_REG(s3, s1);
    BFIx(s1, s2, 22, 2);        // inject new round
    MSR_fpcr(s1);               // put new fpscr
    return s3;
}

// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1); MAYUSE(s2);
    LDRw_U12(s1, xEmu, offsetof(x64emu_t, mxcsr));
    RBITw(s2, s1);              // round is on bits 13-14 on x64,
    LSRw(s2, s2, 17);           // but we want the reverse of that
    MRS_fpcr(s1);               // get fpscr
    MOVx_REG(s3, s1);
    BFIx(s1, s2, 22, 2);     // inject new round
    MSR_fpcr(s1);               // put new fpscr
    return s3;
}

// Restore round flag
void x87_restoreround(dynarec_arm_t* dyn, int ninst, int s1)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1);
    MSR_fpcr(s1);               // put back fpscr
}

// MMX helpers
static int isx87Empty(dynarec_arm_t* dyn)
{
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i] != -1)
            return 0;
    return 1;
}

// get neon register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    if(!dyn->n.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if(dyn->n.mmxcache[a]!=-1)
        return dyn->n.mmxcache[a];
    ++dyn->n.mmxcount;
    int ret = dyn->n.mmxcache[a] = fpu_get_reg_emm(dyn, a);
    VLDR64_U12(ret, xEmu, offsetof(x64emu_t, mmx[a]));
    return ret;
}
// get neon register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    if(!dyn->n.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if(dyn->n.mmxcache[a]!=-1)
        return dyn->n.mmxcache[a];
    ++dyn->n.mmxcount;
    int ret = dyn->n.mmxcache[a] = fpu_get_reg_emm(dyn, a);
    return ret;
}
// purge the MMX cache only(needs 3 scratch registers)
void mmx_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1)
{
    if(!dyn->n.mmxcount)
        return;
    if(!next)
        dyn->n.mmxcount = 0;
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->n.mmxcache[i]!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge %sMMX Cache ------\n", next?"locally ":"");
                ++old;
            }
            VSTR64_U12(dyn->n.mmxcache[i], xEmu, offsetof(x64emu_t, mmx[i]));
            if(!next) {
                fpu_free_reg(dyn, dyn->n.mmxcache[i]);
                dyn->n.mmxcache[i] = -1;
            }
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge MMX Cache\n");
    }
}

static void mmx_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    for (int i=0; i<8; ++i)
        if(dyn->n.mmxcache[i]!=-1) {
            VSTR64_U12(dyn->n.mmxcache[i], xEmu, offsetof(x64emu_t, mmx[i]));
        }
}


// SSE / SSE2 helpers
// get neon register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a, int forwrite)
{
    if(dyn->n.ssecache[a].v!=-1) {
        if(forwrite) {
            dyn->n.ssecache[a].write = 1;    // update only if forwrite
            dyn->n.neoncache[dyn->n.ssecache[a].reg].t = NEON_CACHE_XMMW;
        }
        return dyn->n.ssecache[a].reg;
    }
    dyn->n.ssecache[a].reg = fpu_get_reg_xmm(dyn, forwrite?NEON_CACHE_XMMW:NEON_CACHE_XMMR, a);
    int ret =  dyn->n.ssecache[a].reg;
    dyn->n.ssecache[a].write = forwrite;
    VLDR128_U12(ret, xEmu, offsetof(x64emu_t, xmm[a]));
    return ret;
}
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
    if(dyn->n.ssecache[a].v!=-1) {
        dyn->n.ssecache[a].write = 1;
        dyn->n.neoncache[dyn->n.ssecache[a].reg].t = NEON_CACHE_XMMW;
        return dyn->n.ssecache[a].reg;
    }
    dyn->n.ssecache[a].reg = fpu_get_reg_xmm(dyn, NEON_CACHE_XMMW, a);
    dyn->n.ssecache[a].write = 1; // it will be write...
    return dyn->n.ssecache[a].reg;
}
// forget neon register for a SSE reg, create the entry if needed
void sse_forget_reg(dynarec_arm_t* dyn, int ninst, int a)
{
    if(dyn->n.ssecache[a].v==-1)
        return;
    if(dyn->n.neoncache[dyn->n.ssecache[a].reg].t == NEON_CACHE_XMMW) {
        VSTR128_U12(dyn->n.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
    }
    fpu_free_reg(dyn, dyn->n.ssecache[a].reg);
    dyn->n.ssecache[a].v = -1;
    return;
}
// purge the SSE cache for XMM0..XMM7 (to use before function native call)
void sse_purge07cache(dynarec_arm_t* dyn, int ninst, int s1)
{
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->n.ssecache[i].v!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge XMM0..7 Cache ------\n");
                ++old;
            }
            if(dyn->n.neoncache[dyn->n.ssecache[i].reg].t == NEON_CACHE_XMMW) {
                VSTR128_U12(dyn->n.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            }
            fpu_free_reg(dyn, dyn->n.ssecache[i].reg);
            dyn->n.ssecache[i].v = -1;
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge XMM0..7 Cache\n");
    }
}

// purge the SSE cache only
static void sse_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1)
{
    int old = -1;
    for (int i=0; i<16; ++i)
        if(dyn->n.ssecache[i].v!=-1) {
            if(dyn->n.ssecache[i].write) {
                if (old==-1) {
                    MESSAGE(LOG_DUMP, "\tPurge %sSSE Cache ------\n", next?"locally ":"");
                    ++old;
                }
                VSTR128_U12(dyn->n.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            }
            if(!next) {
                fpu_free_reg(dyn, dyn->n.ssecache[i].reg);
                dyn->n.ssecache[i].v = -1;
            }
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge SSE Cache\n");
    }
}

static void sse_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    for (int i=0; i<16; ++i)
        if(dyn->n.ssecache[i].v!=-1 && dyn->n.ssecache[i].write) {
            VSTR128_U12(dyn->n.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
        }
}

void sse_reflect_reg(dynarec_arm_t* dyn, int ninst, int a)
{
    if(dyn->n.ssecache[a].v==-1)
        return;
    if(dyn->n.neoncache[dyn->n.ssecache[a].reg].t == NEON_CACHE_XMMW) {
        VSTR128_U12(dyn->n.ssecache[a].reg, xEmu, offsetof(x64emu_t, xmm[a]));
        /*dyn->n.neoncache[dyn->n.ssecache[a].reg].t = NEON_CACHE_XMMR;
        dyn->n.ssecache[a].write = 0;*/
    }
}

void fpu_pushcache(dynarec_arm_t* dyn, int ninst, int s1, int not07)
{
    int start = not07?8:0;
    // only SSE regs needs to be push back to xEmu (needs to be "write")
    int n=0;
    for (int i=start; i<16; i++)
        if((dyn->n.ssecache[i].v!=-1) && (dyn->n.ssecache[i].write))
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "\tPush XMM Cache (%d)------\n", n);
    for (int i=start; i<16; ++i)
        if((dyn->n.ssecache[i].v!=-1) && (dyn->n.ssecache[i].write)) {
            VSTR128_U12(dyn->n.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
        }
    MESSAGE(LOG_DUMP, "\t------- Push XMM Cache (%d)\n", n);
}

void fpu_popcache(dynarec_arm_t* dyn, int ninst, int s1, int not07)
{
    int start = not07?8:0;
    // only SSE regs needs to be pop back from xEmu (don't need to be "write" this time)
    int n=0;
    for (int i=start; i<16; i++)
        if(dyn->n.ssecache[i].v!=-1)
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "\tPop XMM Cache (%d)------\n", n);
    for (int i=start; i<16; ++i)
        if(dyn->n.ssecache[i].v!=-1) {
            VLDR128_U12(dyn->n.ssecache[i].reg, xEmu, offsetof(x64emu_t, xmm[i]));
            /*dyn->n.ssecache[i].write = 0;   // OPTIM: it's sync, so not write anymore
            dyn->n.neoncache[dyn->n.ssecache[i].reg].t = NEON_CACHE_XMMR;*/
        }
    MESSAGE(LOG_DUMP, "\t------- Pop XMM Cache (%d)\n", n);
}

void fpu_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    x87_purgecache(dyn, ninst, next, s1, s2, s3);
    mmx_purgecache(dyn, ninst, next, s1);
    sse_purgecache(dyn, ninst, next, s1);
    if(!next)
        fpu_reset_reg(dyn);
}

static int findCacheSlot(dynarec_arm_t* dyn, int ninst, int t, int n, neoncache_t* cache)
{
    neon_cache_t f;
    f.n = n; f.t = t;
    for(int i=0; i<24; ++i) {
        if(cache->neoncache[i].v == f.v)
            return i;
        if(cache->neoncache[i].n == n) {
            switch(cache->neoncache[i].t) {
                case NEON_CACHE_ST_F:
                    if (t==NEON_CACHE_ST_D)
                        return i;
                    if (t==NEON_CACHE_ST_I64)
                        return i;
                    break;
                case NEON_CACHE_ST_D:
                    if (t==NEON_CACHE_ST_F)
                        return i;
                    if (t==NEON_CACHE_ST_I64)
                        return i;
                    break;
                case NEON_CACHE_ST_I64:
                    if (t==NEON_CACHE_ST_F)
                        return i;
                    if (t==NEON_CACHE_ST_D)
                        return i;
                    break;
                case NEON_CACHE_XMMR:
                    if(t==NEON_CACHE_XMMW)
                        return i;
                    break;
                case NEON_CACHE_XMMW:
                    if(t==NEON_CACHE_XMMR)
                        return i;
                    break;
            }
        }
    }
    return -1;
}

static void swapCache(dynarec_arm_t* dyn, int ninst, int i, int j, neoncache_t *cache)
{
    if (i==j)
        return;
    int quad = 0;
    if(cache->neoncache[i].t==NEON_CACHE_XMMR || cache->neoncache[i].t==NEON_CACHE_XMMW)
        quad =1;
    if(cache->neoncache[j].t==NEON_CACHE_XMMR || cache->neoncache[j].t==NEON_CACHE_XMMW)
        quad =1;

    if(!cache->neoncache[i].v) {
        // a mov is enough, no need to swap
        MESSAGE(LOG_DUMP, "\t  - Moving %d <- %d\n", i, j);
        if(quad) {
            VMOVQ(i, j);
        } else {
            VMOV(i, j);
        }
        cache->neoncache[i].v = cache->neoncache[j].v;
        cache->neoncache[j].v = 0;
        return;
    }
    // SWAP
    neon_cache_t tmp;
    MESSAGE(LOG_DUMP, "\t  - Swapping %d <-> %d\n", i, j);
    // There is no VSWP in Arm64 NEON to swap 2 register contents!
    // so use a scratch...
    if(quad) {
        VMOVQ(SCRATCH, i);
        VMOVQ(i, j);
        VMOVQ(j, SCRATCH);
    } else {
        VMOV(SCRATCH, i);
        VMOV(i, j);
        VMOV(j, SCRATCH);
    }
    #undef SCRATCH
    tmp.v = cache->neoncache[i].v;
    cache->neoncache[i].v = cache->neoncache[j].v;
    cache->neoncache[j].v = tmp.v;
}

static void loadCache(dynarec_arm_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, neoncache_t *cache, int i, int t, int n)
{
    if(cache->neoncache[i].v) {
        int quad = 0;
        if(t==NEON_CACHE_XMMR || t==NEON_CACHE_XMMW)
            quad = 1;
        if(cache->neoncache[i].t==NEON_CACHE_XMMR || cache->neoncache[i].t==NEON_CACHE_XMMW)
            quad = 1;
        int j = i+1;
        while(cache->neoncache[j].v)
            ++j;
        MESSAGE(LOG_DUMP, "\t  - Moving away %d\n", i);
        if(quad) {
            VMOVQ(j, i);
        } else {
            VMOV(j, i);
        }
        cache->neoncache[j].v = cache->neoncache[i].v;
    }
    switch(t) {
        case NEON_CACHE_XMMR:
        case NEON_CACHE_XMMW:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            VLDR128_U12(i, xEmu, offsetof(x64emu_t, xmm[n]));
            break;
        case NEON_CACHE_MM:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            VLDR64_U12(i, xEmu, offsetof(x64emu_t, mmx[n]));
            break;
        case NEON_CACHE_ST_D:
        case NEON_CACHE_ST_F:
        case NEON_CACHE_ST_I64:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            if((*s3_top) == 0xffff) {
                LDRw_U12(s3, xEmu, offsetof(x64emu_t, top));
                *s3_top = 0;
            }
            int a = n  - (*s3_top) - stack_cnt;
            if(a) {
                if(a<0) {
                    SUBw_U12(s3, s3, -a);
                } else {
                    ADDw_U12(s3, s3, a);
                }
                ANDw_mask(s3, s3, 0, 2); //mask=7    // (emu->top + i)&7
            }
            *s3_top += a;
            if(*s2_val!=1) {
                *s2_val = 1;
                ADDx_U12(s2, xEmu, offsetof(x64emu_t, x87));
            }
            VLDR64_REG_LSL3(i, s2, s3);
            if(t==NEON_CACHE_ST_F) {
                FCVT_S_D(i, i);
            }
            if(t==NEON_CACHE_ST_I64) {
                VFCVTZSQD(i, i);
            }
            break;
        case NEON_CACHE_NONE:
        case NEON_CACHE_SCR:
        default:    /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
    }
    cache->neoncache[i].n = n;
    cache->neoncache[i].t = t;
}

static void unloadCache(dynarec_arm_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, neoncache_t *cache, int i, int t, int n)
{
    switch(t) {
        case NEON_CACHE_XMMR:
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
        case NEON_CACHE_XMMW:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            VSTR128_U12(i, xEmu, offsetof(x64emu_t, xmm[n]));
            break;
        case NEON_CACHE_MM:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            VSTR64_U12(i, xEmu, offsetof(x64emu_t, mmx[n]));
            break;
        case NEON_CACHE_ST_D:
        case NEON_CACHE_ST_F:
        case NEON_CACHE_ST_I64:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            if((*s3_top)==0xffff) {
                LDRw_U12(s3, xEmu, offsetof(x64emu_t, top));
                *s3_top = 0;
            }
            int a = n - (*s3_top) - stack_cnt;
            if(a) {
                if(a<0) {
                    SUBw_U12(s3, s3, -a);
                } else {
                    ADDw_U12(s3, s3, a);
                }
                ANDw_mask(s3, s3, 0, 2); //mask=7    // (emu->top + i)&7
            }
            *s3_top += a;
            if(*s2_val!=1) {
                *s2_val = 1;
                ADDx_U12(s2, xEmu, offsetof(x64emu_t, x87));
            }
            if(t==NEON_CACHE_ST_F) {
                FCVT_D_S(i, i);
            } else if (t==NEON_CACHE_ST_I64) {
                SCVTFDD(i, i);
            }
            VSTR64_REG_LSL3(i, s2, s3);
            break;
        case NEON_CACHE_NONE:
        case NEON_CACHE_SCR:
        default:    /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break;
    }
    cache->neoncache[i].v = 0;
}

static void fpuCacheTransform(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
#if STEP > 1
    int i2 = dyn->insts[ninst].x64.jmp_insts;
    if(i2<0)
        return;
    MESSAGE(LOG_DUMP, "\tCache Transform ---- ninst=%d -> %d\n", ninst, i2);
    if((!i2) || (dyn->insts[i2].x64.barrier&BARRIER_FLOAT)) {
        if(dyn->n.stack_next)  {
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
            MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
            return;
        }
        for(int i=0; i<24; ++i)
            if(dyn->n.neoncache[i].v) {       // there is something at ninst for i
                fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
                MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
                return;
            }
        MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
        return;
    }
    neoncache_t cache_i2 = dyn->insts[i2].n;
    neoncacheUnwind(&cache_i2);

    if(!cache_i2.stack) {
        int purge = 1;
        for (int i=0; i<24 && purge; ++i)
            if(cache_i2.neoncache[i].v)
                purge = 0;
        if(purge) {
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
            MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
            return;
        }
    }
    int stack_cnt = dyn->n.stack_next;
    int s3_top = 0xffff;
    neoncache_t cache = dyn->n;
    int s1_val = 0;
    int s2_val = 0;
    // unload every uneeded cache
    // check SSE first, than MMX, in order, for optimisation issue
    for(int i=0; i<16; ++i) {
        int j=findCacheSlot(dyn, ninst, NEON_CACHE_XMMW, i, &cache);
        if(j>=0 && findCacheSlot(dyn, ninst, NEON_CACHE_XMMW, i, &cache_i2)==-1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.neoncache[j].t, cache.neoncache[j].n);
    }
    for(int i=0; i<8; ++i) {
        int j=findCacheSlot(dyn, ninst, NEON_CACHE_MM, i, &cache);
        if(j>=0 && findCacheSlot(dyn, ninst, NEON_CACHE_MM, i, &cache_i2)==-1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.neoncache[j].t, cache.neoncache[j].n);
    }
    for(int i=0; i<24; ++i) {
        if(cache.neoncache[i].v)
            if(findCacheSlot(dyn, ninst, cache.neoncache[i].t, cache.neoncache[i].n, &cache_i2)==-1)
                unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache.neoncache[i].t, cache.neoncache[i].n);
    }
    // and now load/swap the missing one
    for(int i=0; i<24; ++i) {
        if(cache_i2.neoncache[i].v) {
            if(cache_i2.neoncache[i].v != cache.neoncache[i].v) {
                int j;
                if((j=findCacheSlot(dyn, ninst, cache_i2.neoncache[i].t, cache_i2.neoncache[i].n, &cache))==-1)
                    loadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache_i2.neoncache[i].t, cache_i2.neoncache[i].n);
                else {
                    // it's here, lets swap if needed
                    if(j!=i)
                        swapCache(dyn, ninst, i, j, &cache);
                }
            }
            if(cache.neoncache[i].t != cache_i2.neoncache[i].t) {
                if(cache.neoncache[i].t == NEON_CACHE_ST_D && cache_i2.neoncache[i].t == NEON_CACHE_ST_F) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    FCVT_S_D(i, i);
                    cache.neoncache[i].t = NEON_CACHE_ST_F;
                } else if(cache.neoncache[i].t == NEON_CACHE_ST_F && cache_i2.neoncache[i].t == NEON_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    FCVT_D_S(i, i);
                    cache.neoncache[i].t = NEON_CACHE_ST_D;
                } else if(cache.neoncache[i].t == NEON_CACHE_ST_D && cache_i2.neoncache[i].t == NEON_CACHE_ST_I64) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    VFCVTZSQD(i, i);
                    cache.neoncache[i].t = NEON_CACHE_ST_I64;
                } else if(cache.neoncache[i].t == NEON_CACHE_ST_F && cache_i2.neoncache[i].t == NEON_CACHE_ST_I64) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    VFCVTZSQS(i, i);
                    cache.neoncache[i].t = NEON_CACHE_ST_D;
                } else if(cache.neoncache[i].t == NEON_CACHE_ST_I64 && cache_i2.neoncache[i].t == NEON_CACHE_ST_F) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    SCVTFDD(i, i);
                    FCVT_S_D(i, i);
                    cache.neoncache[i].t = NEON_CACHE_ST_F;
                } else if(cache.neoncache[i].t == NEON_CACHE_ST_I64 && cache_i2.neoncache[i].t == NEON_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    SCVTFDD(i, i);
                    cache.neoncache[i].t = NEON_CACHE_ST_D;
                } else if(cache.neoncache[i].t == NEON_CACHE_XMMR && cache_i2.neoncache[i].t == NEON_CACHE_XMMW)
                    { cache.neoncache[i].t = NEON_CACHE_XMMW; }
                else if(cache.neoncache[i].t == NEON_CACHE_XMMW && cache_i2.neoncache[i].t == NEON_CACHE_XMMR) {
                    // refresh cache...
                    MESSAGE(LOG_DUMP, "\t  - Refreh %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    VSTR128_U12(i, xEmu, offsetof(x64emu_t, xmm[cache.neoncache[i].n]));
                    cache.neoncache[i].t = NEON_CACHE_XMMR;
                }
            }
        }
    }
    if(stack_cnt != cache_i2.stack) {
        MESSAGE(LOG_DUMP, "\t    - adjust stack count %d -> %d -\n", stack_cnt, cache_i2.stack);
        int a = stack_cnt - cache_i2.stack;
        // Add x87stack to emu fpu_stack
        LDRw_U12(s3, xEmu, offsetof(x64emu_t, fpu_stack));
        if(a>0) {
            ADDw_U12(s3, s3, a);
        } else {
            SUBw_U12(s3, s3, -a);
        }
        STRw_U12(s3, xEmu, offsetof(x64emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LDRw_U12(s3, xEmu, offsetof(x64emu_t, top));
        if(a>0) {
            SUBw_U12(s3, s3, a);
        } else {
            ADDw_U12(s3, s3, -a);
        }
        ANDw_mask(s3, s3, 0, 2);   //mask=7
        STRw_U12(s3, xEmu, offsetof(x64emu_t, top));
        // update tags
        LDRH_U12(s2, xEmu, offsetof(x64emu_t, fpu_tags));
        if(a>0) {
            LSLw_IMM(s2, s2, a*2);
        } else {
            ORRw_mask(s2, s2, 0b010000, 0b001111);  // 0xffff0000
            LSRw_IMM(s2, s2, -a*2);
        }
        STRH_U12(s2, xEmu, offsetof(x64emu_t, fpu_tags));
        s3_top = 0;
        stack_cnt = cache_i2.stack;
    }
    MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
#endif
}
static void flagsCacheTransform(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    int j64;
    int jmp = dyn->insts[ninst].x64.jmp_insts;
    if(jmp<0)
        return;
    if(dyn->f.dfnone)  // flags are fully known, nothing we can do more
        return;
    MESSAGE(LOG_DUMP, "\tFlags fetch ---- ninst=%d -> %d\n", ninst, jmp);
    int go = (dyn->insts[jmp].f_entry.dfnone && !dyn->f.dfnone)?1:0;
    switch (dyn->insts[jmp].f_entry.pending) {
        case SF_UNKNOWN: break;
        case SF_SET:
            if(dyn->f.pending!=SF_SET && dyn->f.pending!=SF_SET_PENDING)
                go = 1;
            break;
        case SF_SET_PENDING:
            if(dyn->f.pending!=SF_SET
            && dyn->f.pending!=SF_SET_PENDING
            && dyn->f.pending!=SF_PENDING)
                go = 1;
            break;
        case SF_PENDING:
            if(dyn->f.pending!=SF_SET
            && dyn->f.pending!=SF_SET_PENDING
            && dyn->f.pending!=SF_PENDING)
                go = 1;
            else if (dyn->insts[jmp].f_entry.dfnone !=dyn->f.dfnone)
                go = 1;
            break;
    }
    if(go) {
        if(dyn->f.pending!=SF_PENDING) {
            LDRw_U12(s1, xEmu, offsetof(x64emu_t, df));
            j64 = (GETMARKF2)-(dyn->native_size);
            CBZw(s1, j64);
        }
        CALL_(UpdateFlags, -1, 0);
        MARKF2;
    }
#endif
}

void CacheTransform(dynarec_arm_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3) {
    if(cacheupd&2)
        fpuCacheTransform(dyn, ninst, s1, s2, s3);
    if(cacheupd&1)
        flagsCacheTransform(dyn, ninst, s1);
}

void fpu_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_reflectcache(dyn, ninst, s1, s2, s3);
    mmx_reflectcache(dyn, ninst, s1);
    sse_reflectcache(dyn, ninst, s1);
}

void fpu_unreflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    // need to undo some things on the x87 tracking
    x87_unreflectcache(dyn, ninst, s1, s2, s3);
}

void emit_pf(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    MAYUSE(dyn); MAYUSE(ninst);
    MAYUSE(s1); MAYUSE(s3); MAYUSE(s4);
    // PF: (((emu->x64emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
    ANDw_mask(s3, s1, 0b011011, 0b000010); // mask=0xE0
    LSRw(s3, s3, 5);
    TABLE64(s4, (uintptr_t)GetParityTab());
    LDRw_REG_LSL2(s4, s4, s3);
    ANDw_mask(s3, s1, 0, 0b000100); //0x1f
    LSRw_REG(s4, s4, s3);
    MVNw_REG(s4, s4);
    BFIw(xFlags, s4, F_PF, 1);
}

void arm64_move32(dynarec_arm_t* dyn, int ninst, int reg, uint32_t val)
{
    // simple cases with only 1 operations
    for(int i=0; i<2; ++i)
        if((val&(0xFFFF<<(i*16)))==val) {
            MOVZw_LSL(reg, (val>>(i*16))&0xFFFF, i*16);
            return;
        }
    // same but with negation
    for(int i=0; i<2; ++i)
        if(((~val)&(0xFFFF<<(i*16)))==(~val)) {
            MOVNw_LSL(reg, ((~val)>>(i*16))&0xFFFF, i*16);
            return;
        }
    // generic cases
    int mask = convert_bitmask_w(val);
    if(mask) {
        ORRw_mask(reg, xZR, mask&0x3F, (mask>>6)&0x3F);
    } else {
        MOVZw(reg, val&0xFFFF);
        MOVKw_LSL(reg, (val>>16)&0xFFFF, 16);
    }
}
void arm64_move64(dynarec_arm_t* dyn, int ninst, int reg, uint64_t val)
{
    // simple cases with only 1 operations
    for(int i=0; i<4; ++i)
        if((val&(0xFFFFLL<<(i*16)))==val) {
            MOVZx_LSL(reg, (val>>(i*16))&0xFFFF, i*16);
            return;
        }
    // same but with negation
    for(int i=0; i<4; ++i)
        if(((~val)&(0xFFFFLL<<(i*16)))==(~val)) {
            MOVNx_LSL(reg, ((~val)>>(i*16))&0xFFFF, i*16);
            return;
        }
    // mask
    int mask = convert_bitmask_x(val);
    if(mask) {
        ORRx_mask(reg, xZR, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
        return;
    }
    // 32bit value?
    if((val&0xFFFFFFFF)==val) {
        arm64_move32(dyn, ninst, reg, val);
        return;
    }
    int n = 0;
    // negatives values
    if((val&0xFFFF000000000000LL)==0xFFFF000000000000LL) {
        for(int i=0; i<3; ++i) {
            if(((~val)>>(i*16))&0xFFFF) {
                if(n) {
                    MOVKx_LSL(reg, (val>>(i*16))&0xFFFF, i*16);
                } else {
                    MOVNx_LSL(reg, ((~val)>>(i*16))&0xFFFF, i*16);
                    n = 1;
                }
            }
        }
        return;
    }
    // positive values
    for(int i=0; i<4; ++i) {
        if((val>>(i*16))&0xFFFF) {
            if(n) {
                MOVKx_LSL(reg, (val>>(i*16))&0xFFFF, i*16);
            } else {
                MOVZx_LSL(reg, (val>>(i*16))&0xFFFF, i*16);
                n = 1;
            }
        }
    }
}


void fpu_reset_cache(dynarec_arm_t* dyn, int ninst, int reset_n)
{
    MESSAGE(LOG_DEBUG, "Reset Caches with %d\n",reset_n);
    #if STEP > 1
    // for STEP 2 & 3, just need to refrest with current, and undo the changes (push & swap)
    dyn->n = dyn->insts[ninst].n;
    #else
    dyn->n = dyn->insts[reset_n].n;
    #endif
    neoncacheUnwind(&dyn->n);
    #if STEP == 0
    if(box64_dynarec_dump) dynarec_log(LOG_NONE, "New x87stack=%d\n", dyn->n.x87stack);
        #endif
    #if defined(HAVE_TRACE) && (STEP>2)
    if(box64_dynarec_dump)
        if(memcmp(&dyn->n, &dyn->insts[reset_n].n, sizeof(neon_cache_t))) {
            MESSAGE(LOG_DEBUG, "Warning, difference in neoncache: reset=");
            for(int i=0; i<24; ++i)
                if(dyn->insts[reset_n].n.neoncache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[reset_n].n.neoncache[i].t, dyn->insts[reset_n].n.neoncache[i].n));
            if(dyn->insts[reset_n].n.combined1 || dyn->insts[reset_n].n.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[reset_n].n.swapped?"SWP":"CMB", dyn->insts[reset_n].n.combined1, dyn->insts[reset_n].n.combined2);
            if(dyn->insts[reset_n].n.stack_push || dyn->insts[reset_n].n.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[reset_n].n.stack_push, -dyn->insts[reset_n].n.stack_pop);
            MESSAGE(LOG_DEBUG, " ==> ");
            for(int i=0; i<24; ++i)
                if(dyn->insts[ninst].n.neoncache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[ninst].n.neoncache[i].t, dyn->insts[ninst].n.neoncache[i].n));
            if(dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[ninst].n.swapped?"SWP":"CMB", dyn->insts[ninst].n.combined1, dyn->insts[ninst].n.combined2);
            if(dyn->insts[ninst].n.stack_push || dyn->insts[ninst].n.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[ninst].n.stack_push, -dyn->insts[ninst].n.stack_pop);
            MESSAGE(LOG_DEBUG, " -> ");
            for(int i=0; i<24; ++i)
                if(dyn->n.neoncache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->n.neoncache[i].t, dyn->n.neoncache[i].n));
            if(dyn->n.combined1 || dyn->n.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->n.swapped?"SWP":"CMB", dyn->n.combined1, dyn->n.combined2);
            if(dyn->n.stack_push || dyn->n.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->n.stack_push, -dyn->n.stack_pop);
            MESSAGE(LOG_DEBUG, "\n");
        }
    #endif //HAVE_TRACE
}

// propagate ST stack state, especial stack pop that are deferred
void fpu_propagate_stack(dynarec_arm_t* dyn, int ninst)
{
    if(dyn->n.stack_pop) {
        for(int j=0; j<24; ++j)
            if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D
             || dyn->n.neoncache[j].t == NEON_CACHE_ST_F
             || dyn->n.neoncache[j].t == NEON_CACHE_ST_I64)) {
                if(dyn->n.neoncache[j].n<dyn->n.stack_pop)
                    dyn->n.neoncache[j].v = 0;
                else
                    dyn->n.neoncache[j].n-=dyn->n.stack_pop;
            }
        dyn->n.stack_pop = 0;
    }
    dyn->n.stack = dyn->n.stack_next;
    dyn->n.news = 0;
    dyn->n.stack_push = 0;
    dyn->n.swapped = 0;
}
