#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "arm64_printer.h"
#include "dynarec_arm64_private.h"
#include "../dynarec_helper.h"
#include "dynarec_arm64_functions.h"


uintptr_t dynarec64_F0(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed, u8;
    uint8_t wback, wb1, wb2, eb1, eb2, gb1, gb2;
    int32_t i32;
    int64_t i64, j64;
    int64_t fixedaddress;
    int unscaled, mask;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(j64);

    switch(opcode) {
        case 0x00:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK ADD Eb, Gb");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETGB(x2);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                if(cpuext.atomics) {
                    UFLAG_IF {
                        LDADDALB(x2, x1, wback);
                        emit_add8(dyn, ninst, x1, x2, x4, x5);
                    } else {
                        STADDLB(x2, wback);
                    }
                } else {
                    MARKLOCK;
                    LDAXRB(x1, wback);
                    emit_add8(dyn, ninst, x1, x2, x4, x5);
                    STLXRB(x4, x1, wback);
                    CBNZx_MARKLOCK(x4);
                }
            }
            break;
        case 0x01:
            nextop = F8;
            if(MODREG) {
                INST_NAME("Invalid LOCK");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK ADD Ed, Gd");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                if(!ALIGNED_ATOMICxw) {
                    if(cpuext.uscat) {
                        ANDx_mask(x1, wback, 1, 0, 3);  // mask = F
                        CMPSw_U12(x1, 16-(1<<(2+rex.w)));
                        B_MARK(cGT);
                    } else {
                        TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                        B_MARK(cNE);
                    }
                }
                if(cpuext.atomics) {
                    UFLAG_IF {
                        LDADDALxw(gd, x1, wback);
                    } else {
                        STADDLxw(gd, wback);
                    }
                } else {
                    MARKLOCK;
                    LDAXRxw(x1, wback);
                    ADDxw_REG(x4, x1, gd);
                    STLXRxw(x3, x4, wback);
                    CBNZx_MARKLOCK(x3);
                }
                if(!ALIGNED_ATOMICxw) {
                    B_MARK2_nocond;
                    MARK;   // unaligned! also, not enough
                    LDRxw_U12(x1, wback, 0);
                    LDAXRB(x4, wback);
                    SUBxw_UXTB(x4, x4, x1); // substract with the byte only
                    CBNZw_MARK(x4); // jump if different
                    ADDxw_REG(x4, x1, gd);
                    STLXRB(x3, x4, wback);
                    CBNZx_MARK(x3);
                    STRxw_U12(x4, wback, 0);    // put the whole value
                }
                MARK2;
                UFLAG_IF {
                    emit_add32(dyn, ninst, rex, x1, gd, x3, x4);
                }
            }
            break;

        case 0x08:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK OR Eb, Gb");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETGB(x2);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                if(cpuext.atomics) {
                    LDSETALB(x2, x1, wback);
                    UFLAG_IF {
                        emit_or8(dyn, ninst, x1, x2, x4, x5);
                    }
                } else {
                    MARKLOCK;
                    LDAXRB(x1, wback);
                    emit_or8(dyn, ninst, x1, x2, x4, x5);
                    STLXRB(x4, x1, wback);
                    CBNZx_MARKLOCK(x4);
                }
            }
            break;
        case 0x09:
            nextop = F8;
            if(MODREG) {
                INST_NAME("Invalid LOCK");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK OR Ed, Gd");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                if(cpuext.atomics) {
                    LDSETALxw(gd, x1, wback);
                    UFLAG_IF {
                        emit_or32(dyn, ninst, rex, x1, gd, x3, x4);
                    }
                } else {
                    MARKLOCK;
                    LDAXRxw(x1, wback);
                    emit_or32(dyn, ninst, rex, x1, gd, x3, x4);
                    STLXRxw(x3, x1, wback);
                    CBNZx_MARKLOCK(x3);
                }
            }
            break;

        case 0x0F:
            nextop = F8;
            switch(nextop) {

                case 0xAB:
                    nextop = F8;
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK BTS Ed, Gd");
                        if (!BOX64DRENV(dynarec_safeflags)) {
                            SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                        } else {
                            SETFLAGS(X_CF, SF_SUBSET);
                        }
                        GETGD;
                        // Will fetch only 1 byte, to avoid alignment issue
                        ANDw_mask(x2, gd, 0, 0b00010);  //mask=0x000000007
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                        ASRxw(x1, gd, 3); // r1 = (gd>>3)
                        if(!rex.w && !rex.is32bits) {SXTWx(x1, x1);}
                        ADDy_REG_LSL(x3, wback, x1, 0); //(&ed)+=r1;
                        ed = x1;
                        wback = x3;
                        MOV32w(x5, 1);
                        if(cpuext.atomics) {
                            LSLw_REG(x4, x5, x2);
                            LDSETALB(x4, x4, wback);
                            IFX(X_CF) {
                                LSRw_REG(x4, x4, x2);
                                BFIw(xFlags, x4, F_CF, 1);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRB(ed, wback);
                            LSRw_REG(x4, ed, x2);
                            IFX(X_CF) {
                                BFIw(xFlags, x4, F_CF, 1);
                            }
                            LSLw_REG(x4, x5, x2);
                            ORRw_REG(ed, ed, x4);
                            STLXRB(x4, ed, wback);
                            CBNZw_MARKLOCK(x4);
                        }
                    }
                    break;
                case 0xB0:
                    switch(rex.rep) {
                        case 0:
                            nextop = F8;
                            if(MODREG) {
                                INST_NAME("Invalid LOCK");
                                UDF(0);
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK CMPXCHG Eb, Gb");
                                SETFLAGS(X_ALL, SF_SET_PENDING);
                                GETGB(x1);
                                UBFXx(x6, xRAX, 0, 8);
                                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                                if(cpuext.atomics) {
                                    UFLAG_IF {
                                        MOVw_REG(x2, x6);
                                        CASALB(x6, gd, wback);
                                        emit_cmp8(dyn, ninst, x2, x6, x3, x4, x5);
                                    } else {
                                        CASALB(x6, gd, wback);
                                    }
                                    BFIx(xRAX, x6, 0, 8);
                                } else {
                                    MARKLOCK;
                                    LDAXRB(x2, wback);
                                    CMPSxw_REG(x6, x2);
                                    B_MARK(cNE);
                                    // EAX == Ed
                                    STLXRB(x4, gd, wback);
                                    CBNZx_MARKLOCK(x4);
                                    // done
                                    MARK;
                                    UFLAG_IF {emit_cmp8(dyn, ninst, x6, x2, x3, x4, x5);}
                                    BFIx(xRAX, x2, 0, 8);
                                }
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0xB1:
                    switch(rex.rep) {
                        case 0:
                            nextop = F8;
                            if(MODREG) {
                                INST_NAME("Invalid LOCK");
                                UDF(0);
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK CMPXCHG Ed, Gd");
                                SETFLAGS(X_ALL, SF_SET_PENDING);
                                GETGD;
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                                UFLAG_IF { MOVxw_REG(x6, xRAX); }
                                if(!ALIGNED_ATOMICxw) {
                                    if(cpuext.uscat) {
                                        ANDx_mask(x1, wback, 1, 0, 3);  // mask = F
                                        CMPSw_U12(x1, 16-(1<<(2+rex.w)));
                                        B_MARK3(cGT);
                                    } else {
                                        TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                                        B_MARK3(cNE);
                                    }
                                }
                                // Aligned version
                                // disabling use of atomics for now, as it seems to make (at least)
                                //  HorizonZeroDawn and Cyberpunk2077 (both from GoG) unstable
                                //  but why?!
                                if (rex.w /* RAX should NOT be zero-upped if equal */ && cpuext.atomics && 0) {
                                    UFLAG_IF {
                                        MOVxw_REG(x1, xRAX);
                                        CASALxw(x1, gd, wback);
                                        MOVxw_REG(xRAX, x1);
                                        if (!ALIGNED_ATOMICxw) {
                                            B_MARK_nocond;
                                        }
                                    } else {
                                        CASALxw(xRAX, gd, wback);
                                        if (!ALIGNED_ATOMICxw) {
                                            B_NEXT_nocond;
                                        }
                                    }
                                } else {
                                    MARKLOCK;
                                    LDAXRxw(x1, wback);
                                    CMPSxw_REG(xRAX, x1);
                                    Bcond(cNE, 4 + (rex.w ? 8 : 12));
                                    // EAX == Ed
                                    STLXRxw(x4, gd, wback);
                                    CBNZx_MARKLOCK(x4);
                                    // done
                                    if (!rex.w) { B_MARK_nocond; }
                                    MOVxw_REG(xRAX, x1);
                                    if (!ALIGNED_ATOMICxw) {
                                        B_MARK_nocond;
                                    }
                                }
                                if(!ALIGNED_ATOMICxw) {
                                    // Unaligned version
                                    MARK3;
                                    LDRxw_U12(x1, wback, 0);
                                    LDAXRB(x3, wback); // dummy read, to arm the write...
                                    SUBxw_UXTB(x3, x3, x1);
                                    CBNZw_MARK3(x3);
                                    CMPSxw_REG(xRAX, x1);
                                    Bcond(cNE, 4 + (rex.w ? 12 : 16));
                                    // EAX == Ed
                                    STLXRB(x4, gd, wback);
                                    CBNZx_MARK3(x4);
                                    STRxw_U12(gd, wback, 0);
                                    if (!rex.w) { B_MARK_nocond; }
                                    MOVxw_REG(xRAX, x1);
                                }
                                MARK;
                                UFLAG_IF {
                                    emit_cmp32(dyn, ninst, rex, x6, x1, x3, x4, x5);
                                }
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0xB3:
                    nextop = F8;
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK BTR Ed, Gd");
                        if (!BOX64DRENV(dynarec_safeflags)) {
                            SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                        } else {
                            SETFLAGS(X_CF, SF_SUBSET);
                        }
                        GETGD;
                        // Will fetch only 1 byte, to avoid alignment issue
                        ANDw_mask(x2, gd, 0, 0b00010);  //mask=0x000000007
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                        ASRx(x1, gd, 3); // r1 = (gd>>3), there might be an issue for negative 32bits values here
                        if(!rex.w && !rex.is32bits) {SXTWx(x1, x1);}
                        ADDy_REG_LSL(x3, wback, x1, 0); //(&ed)+=r1;
                        ed = x1;
                        wback = x3;
                        MOV32w(x5, 1);
                        if(cpuext.atomics) {
                            LSLw_REG(x4, x5, x2);
                            LDCLRALB(x4, x4, wback);
                            IFX(X_CF) {
                                LSRw_REG(x4, x4, x2);
                                BFIw(xFlags, x4, F_CF, 1);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRB(ed, wback);
                            IFX(X_CF) {
                                LSRw_REG(x4, ed, x2);
                                BFIw(xFlags, x4, F_CF, 1);
                            }
                            LSLw_REG(x4, x5, x2);
                            BICw_REG(ed, ed, x4);
                            STLXRB(x4, ed, wback);
                            CBNZw_MARKLOCK(x4);
                        }
                    }
                    break;

            case 0xBA:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 4:
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                        break;
                    case 5:
                        if(MODREG) {
                            INST_NAME("Invalid LOCK");
                            UDF(0);
                            *need_epilog = 1;
                            *ok = 0;
                        } else {
                            INST_NAME("LOCK BTS Ed, Ib");
                            if (!BOX64DRENV(dynarec_safeflags)) {
                                SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                            } else {
                                SETFLAGS(X_CF, SF_SUBSET);
                            }
                            // Will fetch only 1 byte, to avoid alignment issue
                            addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 1);
                            u8 = F8;
                            if(u8>>3) {
                                ADDx_U12(x3, wback, u8>>3);
                                wback = x3;
                            }
                            ed = x1;
                            if(cpuext.atomics) {
                                MOV32w(x4, 1<<(u8&7));
                                LDSETB(x4, x4, wback);
                                IFX(X_CF) {
                                    BFXILw(xFlags, x4, u8&7, 1); // inject 1 bit from u8 to F_CF (i.e. pos 0)
                                }
                            } else {
                                MARKLOCK;
                                LDAXRB(ed, wback);
                                IFX(X_CF) {
                                    BFXILw(xFlags, ed, u8&7, 1); // inject 1 bit from u8 to F_CF (i.e. pos 0)
                                }
                                mask = convert_bitmask_xw(1LL<<(u8&7));
                                ORRxw_mask(ed, ed, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
                                STLXRB(x4, ed, wback);
                                CBNZw_MARKLOCK(x4);
                            }
                        }
                        break;
                    case 6:
                        if(MODREG) {
                            INST_NAME("Invalid LOCK");
                            UDF(0);
                            *need_epilog = 1;
                            *ok = 0;
                        } else {
                            INST_NAME("LOCK BTR Ed, Ib");
                            if (!BOX64DRENV(dynarec_safeflags)) {
                                SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                            } else {
                                SETFLAGS(X_CF, SF_SUBSET);
                            }
                            addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 1);
                            u8 = F8;
                            if(u8>>3) {
                                ADDx_U12(x3, wback, u8>>3);
                                wback = x3;
                            }
                            ed = x1;
                            if(cpuext.atomics) {
                                MOV32w(x4, 1<<(u8&7));
                                LDCLRALB(x4, x4, wback);
                                IFX(X_CF) {
                                    BFXILw(xFlags, x4, u8&7, 1); // inject 1 bit from u8 to F_CF (i.e. pos 0)
                                }
                            } else {
                                MARKLOCK;
                                LDAXRB(ed, wback);
                                IFX(X_CF) {
                                    BFXILw(xFlags, ed, u8&7, 1); // inject 1 bit from u8 to F_CF (i.e. pos 0)
                                }
                                BFCw(ed, u8&7, 1);
                                STLXRB(x4, ed, wback);
                                CBNZw_MARKLOCK(x4);
                            }
                        }
                        break;
                    case 7:
                        if(MODREG) {
                            INST_NAME("Invalid LOCK");
                            UDF(0);
                            *need_epilog = 1;
                            *ok = 0;
                        } else {
                            INST_NAME("LOCK BTC Ed, Ib");
                            if (!BOX64DRENV(dynarec_safeflags)) {
                                SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                            } else {
                                SETFLAGS(X_CF, SF_SUBSET);
                            }
                            addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 1);
                            u8 = F8;
                            if(u8>>3) {
                                ADDx_U12(x3, wback, u8>>3);
                                wback = x3;
                            }
                            ed = x1;
                            if(cpuext.atomics) {
                                MOV32w(x4, 1<<(u8&7));
                                LDEORALB(x4, x4, wback);
                                IFX(X_CF) {
                                    BFXILw(xFlags, x4, u8&7, 1); // inject 1 bit from u8 to F_CF (i.e. pos 0)
                                }
                            } else {
                                MARKLOCK;
                                LDAXRB(ed, wback);
                                IFX(X_CF) {
                                    BFXILw(xFlags, ed, u8&7, 1); // inject 1 bit from u8 to F_CF (i.e. pos 0)
                                }
                                mask = convert_bitmask_xw(1LL<<(u8&7));
                                EORxw_mask(ed, ed, (mask>>12)&1, mask&0x3F, (mask>>6)&0x3F);
                                STLXRB(x4, ed, wback);
                                CBNZw_MARKLOCK(x4);
                            }
                        }
                        break;
                    default:
                        DEFAULT;
                }
                break;
                case 0xBB:
                    nextop = F8;
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK BTC Ed, Gd");
                        if (!BOX64DRENV(dynarec_safeflags)) {
                            SETFLAGS(X_ALL&~X_ZF, SF_SUBSET);
                        } else {
                            SETFLAGS(X_CF, SF_SUBSET);
                        }
                        GETGD;
                        // Will fetch only 1 byte, to avoid alignment issue
                        ANDw_mask(x2, gd, 0, 0b00010);  //mask=0x000000007
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                        ASRx(x1, gd, 3); // r1 = (gd>>3), there might be an issue for negative 32bits values here
                        if(!rex.w && !rex.is32bits) {SXTWx(x1, x1);}
                        ADDy_REG_LSL(x3, wback, x1, 0); //(&ed)+=r1;
                        ed = x1;
                        wback = x3;
                        MOV32w(x5, 1);
                        if(cpuext.atomics) {
                            LSLw_REG(x4, x5, x2);
                            LDEORALB(x4, x4, wback);
                            IFX(X_CF) {
                                LSRw_REG(x4, x4, x2);
                                BFIw(xFlags, x4, F_CF, 1);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRB(ed, wback);
                            IFX(X_CF) {
                                LSRw_REG(x4, ed, x2);
                                BFIw(xFlags, x4, F_CF, 1);
                            }
                            LSLw_REG(x4, x5, x2);
                            EORw_REG(ed, ed, x4);
                            STLXRB(x4, ed, wback);
                            CBNZw_MARKLOCK(x4);
                        }
                    }
                    break;

                case 0xC0:
                    switch(rex.rep) {
                        case 0:
                            nextop = F8;
                            if(MODREG) {
                                INST_NAME("Invalid LOCK");
                                UDF(0);
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK XADD Eb, Gb");
                                SETFLAGS(X_ALL, SF_SET_PENDING);
                                GETGB(x1);
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                                if(cpuext.atomics) {
                                    UFLAG_IF {
                                        MOVxw_REG(x3, gd);
                                        LDADDALB(x3, gd, wback);
                                        emit_add8(dyn, ninst, x3, gd, x4, x5);
                                    } else {
                                        LDADDALB(gd, gd, wback);
                                    }
                                    GBBACK;
                                } else {
                                    MARKLOCK;
                                    LDAXRB(x5, wback);
                                    ADDw_REG(x4, x5, gd);
                                    STLXRB(x3, x4, wback);
                                    CBNZx_MARKLOCK(x3);
                                    IFX(X_ALL|X_PEND) {
                                        MOVxw_REG(x2, x5);
                                        emit_add8(dyn, ninst, x2, gd, x3, x4);
                                    }
                                    BFIz(gb1, x5, gb2, 8);
                                }
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                case 0xC1:
                    switch(rex.rep) {
                        case 0:
                            nextop = F8;
                            if(MODREG) {
                                INST_NAME("Invalid LOCK");
                                UDF(0);
                                *need_epilog = 1;
                                *ok = 0;
                            } else {
                                INST_NAME("LOCK XADD Ed, Gd");
                                SETFLAGS(X_ALL, SF_SET_PENDING);
                                GETGD;
                                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                                if(!ALIGNED_ATOMICxw) {
                                    if(cpuext.uscat) {
                                        ANDx_mask(x1, wback, 1, 0, 3);  // mask = F
                                        CMPSw_U12(x1, 16-(1<<(2+rex.w)));
                                        B_MARK(cGT);
                                    } else {
                                        TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                                        B_MARK(cNE);    // unaligned
                                    }
                                }
                                if(cpuext.atomics) {
                                    UFLAG_IF {
                                        LDADDALxw(gd, x1, wback);
                                    } else {
                                        LDADDALxw(gd, gd, wback);
                                    }
                                } else {
                                    MARKLOCK;
                                    LDAXRxw(x1, wback);
                                    ADDxw_REG(x4, x1, gd);
                                    STLXRxw(x3, x4, wback);
                                    CBNZx_MARKLOCK(x3);
                                }
                                if(!ALIGNED_ATOMICxw) {
                                    UFLAG_IF {
                                        B_MARK2_nocond;
                                    } else {
                                        if(!cpuext.atomics) MOVxw_REG(gd, x1);
                                        B_NEXT_nocond;
                                    }
                                    MARK;
                                    LDRxw_U12(x1, wback, 0);
                                    LDAXRB(x4, wback);
                                    SUBxw_UXTB(x4, x4, x1);
                                    CBNZw_MARK(x4);
                                    ADDxw_REG(x4, x1, gd);
                                    STLXRB(x3, x4, wback);
                                    CBNZx_MARK(x3);
                                    STRxw_U12(x4, wback, 0);
                                }
                                MARK2;
                                UFLAG_IF {
                                    MOVxw_REG(x3, x1);
                                    emit_add32(dyn, ninst, rex, x3, gd, x4, x5);
                                    MOVxw_REG(gd, x1);
                                } else if(!cpuext.atomics || !ALIGNED_ATOMICxw) {
                                    MOVxw_REG(gd, x1);
                                }
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                case 0xC7:
                    // rep has no impact here
                    nextop = F8;
                    switch((nextop>>3)&7) {
                        case 1:
                        if(MODREG) {
                            INST_NAME("Invalid LOCK");
                            UDF(0);
                            *need_epilog = 1;
                            *ok = 0;
                        } else {
                            if (rex.w) {
                                INST_NAME("LOCK CMPXCHG16B Gq, Eq");
                            } else {
                                INST_NAME("LOCK CMPXCHG8B Gq, Eq");
                            }
                            SETFLAGS(X_ZF, SF_SUBSET);
                            addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                            if(!ALIGNED_ATOMICxw) {
                                if(cpuext.uscat) {
                                    if(rex.w) {
                                        TSTx_mask(wback, 1, 0, 3);
                                        B_MARK2(cNE);
                                    } else {
                                        ANDx_mask(x2, wback, 1, 0, 3);  // mask = F
                                        CMPSw_U12(x2, 8);
                                        B_MARK2(cGT);
                                    }
                                } else {
                                    TSTx_mask(wback, 1, 0, 2+rex.w);    // mask=7 or F
                                    B_MARK2(cNE);    // unaligned
                                }
                            }
                            if(cpuext.atomics) {
                                MOVx_REG(x2, xRAX);
                                MOVx_REG(x3, xRDX);
                                MOVx_REG(x4, xRBX);
                                MOVx_REG(x5, xRCX);
                                CASPALxw(x2, x4, wback);
                                UFLAG_IF {
                                    CMPSxw_REG(x2, xRAX);
                                    CCMPxw(x3, xRDX, 0, cEQ);
                                    IFNATIVE(NF_EQ) {} else {CSETw(x1, cEQ);}
                                }
                                MOVx_REG(xRAX, x2);
                                MOVx_REG(xRDX, x3);
                                if(!ALIGNED_ATOMICxw) {
                                    B_MARK3_nocond;
                                }
                            } else {
                                MARKLOCK;
                                LDAXPxw(x2, x3, wback);
                                CMPSxw_REG(xRAX, x2);
                                CCMPxw(xRDX, x3, 0, cEQ);
                                B_MARK(cNE);    // EAX!=ED[0] || EDX!=Ed[1]
                                STLXPxw(x4, xRBX, xRCX, wback);
                                CBNZx_MARKLOCK(x4);
                                UFLAG_IF {
                                    IFNATIVE(NF_EQ) {} else {MOV32w(x1, 1);}
                                }
                                B_MARK3_nocond;
                                MARK;
                                STLXPxw(x4, x2, x3, wback); // write back, to be sure it was "atomic"
                                CBNZx_MARKLOCK(x4);
                                MOVxw_REG(xRAX, x2);
                                MOVxw_REG(xRDX, x3);
                                UFLAG_IF {
                                    IFNATIVE(NF_EQ) {} else {MOV32w(x1, 0);}
                                }
                                if(!ALIGNED_ATOMICxw) {
                                    B_MARK3_nocond;
                                }
                            }
                            if(!ALIGNED_ATOMICxw) {
                                MARK2;
                                if(rex.w && BOX64DRENV(dynarec_safeflags)>1) {
                                    // unaligned memory cause a GPF
                                    STORE_XEMU_CALL(xRIP);
                                    CALL_S(const_native_gpf, -1);
                                    LOAD_XEMU_CALL(xRIP);
                                } else {
                                    LDPxw_S7_offset(x2, x3, wback, 0);
                                    LDAXRB(x5, wback);
                                    SUBxw_UXTB(x5, x5, x2);
                                    CBNZw_MARK2(x5);
                                    CMPSxw_REG(xRAX, x2);
                                    CCMPxw(xRDX, x3, 0, cEQ);
                                    B_MARKSEG(cNE);    // EAX!=ED[0] || EDX!=Ed[1]
                                    STLXRB(x4, xRBX, wback);
                                    CBNZx_MARK2(x4);
                                    STPxw_S7_offset(xRBX, xRCX, wback, 0);
                                    UFLAG_IF {
                                        IFNATIVE(NF_EQ) {} else {MOV32w(x1, 1);}
                                    }
                                    B_MARK3_nocond;
                                    MARKSEG;
                                    STLXRB(x4, x5, wback); //write back
                                    CBNZx_MARK2(x4);
                                    MOVxw_REG(xRAX, x2);
                                    MOVxw_REG(xRDX, x3);
                                    UFLAG_IF {
                                        IFNATIVE(NF_EQ) {} else {MOV32w(x1, 0);}
                                    }
                                }
                            }
                            MARK3;
                            UFLAG_IF {
                                IFNATIVE(NF_EQ) {} else {BFIw(xFlags, x1, F_ZF, 1);}
                            }
                        }
                        break;
                    default:
                        DEFAULT;
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;
        case 0x10:
            INST_NAME("LOCK ADC Eb, Gb");
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                READFLAGS(X_CF);
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETGB(x2);
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                MARKLOCK;
                LDAXRB(x1, wback);
                emit_adc8(dyn, ninst, x1, x2, x4, x5);
                STLXRB(x4, x1, wback);
                CBNZx_MARKLOCK(x4);
            }
            break;
        case 0x11:
            nextop = F8;
            if(MODREG) {
                INST_NAME("Invalid LOCK");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK ADC Ed, Gd");
                READFLAGS(X_CF);
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                MARKLOCK;
                LDAXRxw(x1, wback);
                emit_adc32(dyn, ninst, rex, x1, gd, x4, x5);
                STLXRxw(x4, x1, wback);
                CBNZx_MARKLOCK(x4);
            }
            break;
        case 0x20:
            nextop = F8;
            if(MODREG) {
                INST_NAME("Invalid LOCK");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK AND Eb, Gb");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                GETGB(x5);
                if(cpuext.atomics) {
                    MVNxw_REG(x1, gd);
                    UFLAG_IF {
                        LDCLRALB(x1, x1, wback);
                        emit_and8(dyn, ninst, x1, gd, x3, x4);
                    } else {
                        STCLRLB(x1, wback);
                    }
                } else {
                    MARKLOCK;
                    LDAXRB(x1, wback);
                    emit_and8(dyn, ninst, x1, gd, x3, x4);
                    STLXRB(x3, x1, wback);
                    CBNZx_MARKLOCK(x3);
                }
            }
            break;
        case 0x21:
            nextop = F8;
            if(MODREG) {
                INST_NAME("Invalid LOCK");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK AND Ed, Gd");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                if(cpuext.atomics) {
                    MVNxw_REG(x1, gd);
                    UFLAG_IF {
                        LDCLRALxw(x1, x1, wback);
                        emit_and32(dyn, ninst, rex, x1, gd, x3, x4);
                    } else {
                        STCLRLxw(x1, wback);
                    }
                } else {
                    MARKLOCK;
                    LDAXRxw(x1, wback);
                    emit_and32(dyn, ninst, rex, x1, gd, x3, x4);
                    STLXRxw(x3, x1, wback);
                    CBNZx_MARKLOCK(x3);
                }
            }
            break;

        case 0x29:
            nextop = F8;
            if(MODREG) {
                INST_NAME("Invalid LOCK");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK SUB Ed, Gd");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                if(!ALIGNED_ATOMICxw) {
                    if(cpuext.uscat) {
                        ANDx_mask(x1, wback, 1, 0, 3);  // mask = F
                        CMPSw_U12(x1, 16-(1<<(2+rex.w)));
                        B_MARK(cGT);
                    } else {
                        TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                        B_MARK(cNE);
                    }
                }
                if(cpuext.atomics && 0) {    // disabled because 0x80000000 has no negative
                    NEGxw_REG(x1, gd);
                    UFLAG_IF {
                        LDADDALxw(x1, x1, wback);
                    } else {
                        STADDLxw(x1, wback);
                    }
                } else {
                    MARKLOCK;
                    LDAXRxw(x1, wback);
                    SUBxw_REG(x4, x1, gd);
                    STLXRxw(x3, x4, wback);
                    CBNZx_MARKLOCK(x3);
                }
                if(!ALIGNED_ATOMICxw) {
                    UFLAG_IF {
                        B_MARK2_nocond;
                    } else {
                        B_NEXT_nocond;
                    }
                    MARK;   // unaligned! also, not enough
                    LDRxw_U12(x1, wback, 0);
                    LDAXRB(x4, wback);
                    SUBxw_UXTB(x4, x4, x1);
                    CBNZw_MARK(x4);
                    SUBxw_REG(x4, x1, gd);
                    STLXRB(x3, x4, wback);
                    CBNZx_MARK(x3);
                    STRxw_U12(x4, wback, 0);    // put the whole value
                }
                UFLAG_IF {
                    MARK2;
                    emit_sub32(dyn, ninst, rex, x1, gd, x3, x4);
                }
            }
            break;

        case 0x31:
            nextop = F8;
            if (MODREG) {
                INST_NAME("Invalid LOCK");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK XOR Ed, Gd");
                SETFLAGS(X_ALL, SF_SET_PENDING);
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                if(!ALIGNED_ATOMICxw) {
                    if(cpuext.uscat) {
                        ANDx_mask(x1, wback, 1, 0, 3);  // mask = F
                        CMPSw_U12(x1, 16-(1<<(2+rex.w)));
                        B_MARK(cGT);
                    } else {
                        TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                        B_MARK(cNE);
                    }
                }
                if(cpuext.atomics) {
                    UFLAG_IF {
                        LDEORALxw(gd, x1, wback);
                    } else {
                        STEORLxw(gd, wback);
                    }
                } else {
                    MARKLOCK;
                    LDAXRxw(x1, wback);
                    EORxw_REG(x4, x1, gd);
                    STLXRxw(x3, x4, wback);
                    CBNZx_MARKLOCK(x3);
                }
                if(!ALIGNED_ATOMICxw) {
                    B_MARK2_nocond;
                    MARK;   // unaligned! also, not enough
                    LDRxw_U12(x1, wback, 0);
                    LDAXRB(x4, wback);
                    SUBxw_UXTB(x4, x4, x1);
                    CBNZw_MARK(x4);
                    EORxw_REG(x4, x1, gd);
                    STLXRB(x3, x4, wback);
                    CBNZx_MARK(x3);
                    STRxw_U12(x4, wback, 0);    // put the whole value
                }
                MARK2;
                UFLAG_IF {
                    emit_xor32(dyn, ninst, rex, x1, gd, x3, x4);
                }
            }
            break;

        case 0x80:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("ADD Eb, Ib");
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x5, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 1);
                        u8 = F8;
                        wb1 = 1;
                        if(cpuext.atomics) {
                            MOV32w(x2, u8);
                            UFLAG_IF {
                                LDADDALB(x2, x1, wback);
                                emit_add8(dyn, ninst, x1, x2, x3, x4);
                            } else {
                                STADDB(x2, wback);
                            }

                        } else {
                            MARKLOCK;
                            LDAXRB(x1, wback);
                            emit_add8c(dyn, ninst, x1, u8, x2, x4);
                            STLXRB(x3, x1, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                    }
                    break;
                case 1: //OR
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("OR Eb, Ib");
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x5, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 1);
                        u8 = F8;
                        wb1 = 1;
                        if(cpuext.atomics) {
                            MOV32w(x2, u8);
                            UFLAG_IF {
                                LDSETALB(x2, x1, wback);
                                emit_or8(dyn, ninst, x1, x2, x3, x4);
                            } else {
                                STSETLB(x2, wback);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRB(x1, wback);
                            emit_or8c(dyn, ninst, x1, u8, x2, x4);
                            STLXRB(x3, x1, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                    }
                    break;
                case 2: //ADC
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("ADC Eb, Ib");
                        READFLAGS(X_CF);
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x5, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 1);
                        u8 = F8;
                        wb1 = 1;
                        MARKLOCK;
                        LDAXRB(x1, wback);
                        emit_adc8c(dyn, ninst, x1, u8, x2, x4, x3);
                        STLXRB(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                    }
                    break;
                case 3: //SBB
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("SBB Eb, Ib");
                        READFLAGS(X_CF);
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x5, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 1);
                        u8 = F8;
                        wb1 = 1;
                        MARKLOCK;
                        LDAXRB(x1, wback);
                        emit_sbb8c(dyn, ninst, x1, u8, x2, x4, x3);
                        STLXRB(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                    }
                    break;
                case 4: //AND
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("AND Eb, Ib");
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x5, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 1);
                        u8 = F8;
                        wb1 = 1;
                        if(cpuext.atomics) {
                            MOV32w(x2, ~u8);
                            UFLAG_IF {
                                LDCLRALB(x2, x1, wback);
                                emit_and8c(dyn, ninst, x1, u8, x2, x4);
                            } else {
                                STCLRLB(x2, wback);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRB(x1, wback);
                            emit_and8c(dyn, ninst, x1, u8, x2, x4);
                            STLXRB(x3, x1, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                    }
                    break;
                case 5: //SUB
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("SUB Eb, Ib");
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x5, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 1);
                        u8 = F8;
                        wb1 = 1;
                        if(cpuext.atomics) {
                            MOV32w(x2, -u8);
                            UFLAG_IF {
                                LDADDALB(x2, x1, wback);
                                emit_sub8c(dyn, ninst, x1, u8, x2, x4, x3);
                            } else {
                                STADDLB(x2, wback);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRB(x1, wback);
                            emit_sub8c(dyn, ninst, x1, u8, x2, x4, x3);
                            STLXRB(x3, x1, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                    }
                    break;
                case 6: //XOR
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("XOR Eb, Ib");
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x5, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 1);
                        u8 = F8;
                        wb1 = 1;
                        if(cpuext.atomics) {
                            MOV32w(x2, u8);
                            UFLAG_IF {
                                LDEORALB(x2, x1, wback);
                                emit_xor8(dyn, ninst, x1, x2, x3, x4);
                            } else {
                                STEORLB(x2, wback);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRB(x1, wback);
                            emit_xor8c(dyn, ninst, x1, u8, x2, x4);
                            STLXRB(x3, x1, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                    }
                    break;
                case 7: //CMP
                    INST_NAME("Invalid LOCK");
                    UDF(0);
                    *need_epilog = 1;
                    *ok = 0;
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x81:
        case 0x83:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if(opcode==0x81) {
                            INST_NAME("LOCK ADD Ed, Id");
                        } else {
                            INST_NAME("LOCK ADD Ed, Ib");
                        }
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        if((i64<=-0x1000) || (i64>=0x1000)) {
                            MOV64xw(x5, i64);
                        }
                        if(!ALIGNED_ATOMICxw) {
                            if(cpuext.uscat) {
                                ANDx_mask(x1, wback, 1, 0, 3);  // mask = F
                                CMPSw_U12(x1, 16-(1<<(2+rex.w)));
                                B_MARK(cGT);
                            } else {
                                TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                                B_MARK(cNE);
                            }
                        }
                        if(cpuext.atomics) {
                            if((i64>-0x1000) && (i64<0x1000)) {
                                MOV64xw(x5, i64);
                            }
                            UFLAG_IF {
                                LDADDALxw(x5, x1, wback);
                            } else {
                                STADDLxw(x5, wback);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRxw(x1, wback);
                            if(i64>=0 && i64<0x1000) {
                                ADDxw_U12(x4, x1, i64);
                            } else if(i64<0 && i64>-0x1000) {
                                SUBxw_U12(x4, x1, -i64);
                            } else {
                                ADDxw_REG(x4, x1, x5);
                            }
                            STLXRxw(x3, x4, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                        if(!ALIGNED_ATOMICxw) {
                            B_MARK2_nocond;
                            MARK;   // unaligned! also, not enough
                            LDRxw_U12(x1, wback, 0);
                            LDAXRB(x4, wback);
                            SUBxw_UXTB(x4, x4, x1);
                            CBNZw_MARK(x4);
                            if(i64>=0 && i64<0x1000) {
                                ADDxw_U12(x4, x1, i64);
                            } else if(i64<0 && i64>-0x1000) {
                                SUBxw_U12(x4, x1, -i64);
                            } else {
                                ADDxw_REG(x4, x1, x5);
                            }
                            STLXRB(x3, x4, wback);
                            CBNZx_MARK(x3);
                            STRxw_U12(x4, wback, 0);    // put the whole value
                        }
                        MARK2;
                        UFLAG_IF {
                            if((i64<=-0x1000) || (i64>=0x1000)) {
                                emit_add32(dyn, ninst, rex, x1, x5, x3, x4);
                            } else {
                                emit_add32c(dyn, ninst, rex, x1, i64, x3, x4, x5);
                            }
                        }
                    }
                    break;
                case 1: //OR
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if(opcode==0x81) {INST_NAME("LOCK OR Ed, Id");} else {INST_NAME("LOCK OR Ed, Ib");}
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        if(wback==xRSP && !i64) {
                            // this is __faststorefence
                            DMB_ST();
                        } else {
                            if(cpuext.atomics) {
                                MOV64xw(x5, i64);
                                UFLAG_IF {
                                    LDSETALxw(x5, x1, wback);
                                    emit_or32(dyn, ninst, rex, x1, x5, x3, x4);
                                } else {
                                    STSETLxw(x5, wback);
                                }
                            } else {
                                MARKLOCK;
                                LDAXRxw(x1, wback);
                                emit_or32c(dyn, ninst, rex, x1, i64, x3, x4);
                                STLXRxw(x3, x1, wback);
                                CBNZx_MARKLOCK(x3);
                            }
                        }
                    }
                    break;
                case 2: //ADC
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if(opcode==0x81) {INST_NAME("LOCK ADC Ed, Id");} else {INST_NAME("LOCK ADC Ed, Ib");}
                        READFLAGS(X_CF);
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MOV64xw(x5, i64);
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        emit_adc32(dyn, ninst, rex, x1, x5, x3, x4);
                        STLXRxw(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                    }
                    break;
                case 3: //SBB
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if(opcode==0x81) {INST_NAME("LOCK SBB Ed, Id");} else {INST_NAME("LOCK SBB Ed, Ib");}
                        READFLAGS(X_CF);
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        MOV64xw(x5, i64);
                        MARKLOCK;
                        LDAXRxw(x1, wback);
                        emit_sbb32(dyn, ninst, rex, x1, x5, x3, x4);
                        STLXRxw(x3, x1, wback);
                        CBNZx_MARKLOCK(x3);
                    }
                    break;
                case 4: //AND
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if(opcode==0x81) {INST_NAME("LOCK AND Ed, Id");} else {INST_NAME("LOCK AND Ed, Ib");}
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        if(cpuext.atomics) {
                            MOV64xw(x5, ~i64);
                            UFLAG_IF {
                                LDCLRALxw(x5, x1, wback);
                                MVNxw_REG(x5, x5);
                                emit_and32(dyn, ninst, rex, x1, x5, x3, x4);
                            } else {
                                STCLRLxw(x5, wback);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRxw(x1, wback);
                            emit_and32c(dyn, ninst, rex, x1, i64, x3, x4);
                            STLXRxw(x3, x1, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                    }
                    break;
                case 5: //SUB
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if(opcode==0x81) {INST_NAME("LOCK SUB Ed, Id");} else {INST_NAME("LOCK SUB Ed, Ib");}
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        if((i64<=-0x1000) || (i64>=0x1000)) {
                            MOV64xw(x5, i64);
                        }
                        if(!ALIGNED_ATOMICxw) {
                            if(cpuext.uscat) {
                                ANDx_mask(x1, wback, 1, 0, 3);  // mask = F
                                CMPSw_U12(x1, 16-(1<<(2+rex.w)));
                                B_MARK(cGT);
                            } else {
                                TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                                B_MARK(cNE);
                            }
                        }
                        if(cpuext.atomics) {
                            if((i64>-0x1000) && (i64<0x1000)) {
                                MOV64xw(x5, -i64);
                            } else {
                                NEGxw_REG(x5, x5);
                            }
                            UFLAG_IF {
                                LDADDALxw(x5, x1, wback);
                                if((i64<=-0x1000) || (i64>=0x1000))
                                    NEGxw_REG(x5, x5);
                            } else {
                                STADDLxw(x5, wback);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRxw(x1, wback);
                            if(i64>=0 && i64<0x1000) {
                                SUBxw_U12(x4, x1, i64);
                            } else if(i64<0 && i64>-0x1000) {
                                ADDxw_U12(x4, x1, -i64);
                            } else {
                                SUBxw_REG(x4, x1, x5);
                            }
                            STLXRxw(x3, x4, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                        if(!ALIGNED_ATOMICxw) {
                            UFLAG_IF {
                                B_MARK2_nocond;
                            } else {
                                B_NEXT_nocond;
                            }
                            MARK;   // unaligned! also, not enough
                            LDRxw_U12(x1, wback, 0);
                            LDAXRB(x4, wback);
                            SUBxw_UXTB(x4, x4, x1);
                            CBNZw_MARK(x4);
                            if(i64>=0 && i64<0x1000) {
                                SUBxw_U12(x4, x1, i64);
                            } else if(i64<0 && i64>-0x1000) {
                                ADDxw_U12(x4, x1, -i64);
                            } else {
                                SUBxw_REG(x4, x1, x5);
                            }
                            STLXRB(x3, x4, wback);
                            CBNZx_MARK(x3);
                            STRxw_U12(x4, wback, 0);    // put the whole value
                        }
                        UFLAG_IF {
                            MARK2;
                            if((i64<=-0x1000) || (i64>=0x1000)) {
                                emit_sub32(dyn, ninst, rex, x1, x5, x3, x4);
                            } else {
                                emit_sub32c(dyn, ninst, rex, x1, i64, x3, x4, x5);
                            }
                        }
                    }
                    break;
                case 6: //XOR
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        if(opcode==0x81) {INST_NAME("LOCK XOR Ed, Id");} else {INST_NAME("LOCK XOR Ed, Ib");}
                        SETFLAGS(X_ALL, SF_SET_PENDING);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, (opcode==0x81)?4:1);
                        if(opcode==0x81) i64 = F32S; else i64 = F8S;
                        if(cpuext.atomics) {
                            MOV64xw(x5, i64);
                            UFLAG_IF {
                                LDEORALxw(x5, x1, wback);
                                emit_xor32(dyn, ninst, rex, x1, x5, x3, x4);
                            } else {
                                STEORLxw(x5, wback);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRxw(x1, wback);
                            emit_xor32c(dyn, ninst, rex, x1, i64, x3, x4);
                            STLXRxw(x3, x1, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                    }
                    break;
                case 7: //CMP
                    INST_NAME("Invalid LOCK");
                    UDF(0);
                    *need_epilog = 1;
                    *ok = 0;
                    break;
            }
            break;

        case 0x86:
            nextop = F8;
            if(MODREG) {
                INST_NAME("Invalid LOCK");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK XCHG Eb, Gb");
                // Do the swap
                GETGB(x4);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                if(cpuext.atomics) {
                    SWPALB(x4, x1, ed);
                } else {
                    MARKLOCK;
                    // do the swap with exclusive locking
                    LDAXRB(x1, ed);
                    // do the swap 14 -> strb(ed), 1 -> gd
                    STLXRB(x3, x4, ed);
                    CBNZx_MARKLOCK(x3);
                }
                BFIx(gb1, x1, gb2, 8);
            }
            break;
        case 0x87:
            nextop = F8;
            if(MODREG) {
                INST_NAME("Invalid LOCK");
                UDF(0);
                *need_epilog = 1;
                *ok = 0;
            } else {
                INST_NAME("LOCK XCHG Ed, Gd");
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                if(!ALIGNED_ATOMICxw) {
                    if(cpuext.uscat) {
                        ANDx_mask(x1, ed, 1, 0, 3);  // mask = F
                        CMPSw_U12(x1, 16-(1<<(2+rex.w)));
                        B_MARK(cGT);
                    } else {
                        TSTx_mask(ed, 1, 0, 1+rex.w);    // mask=3 or 7
                        B_MARK(cNE);
                    }
                }
                if(cpuext.atomics) {
                    SWPALxw(gd, gd, ed);
                    if(!ALIGNED_ATOMICxw) {
                        B_NEXT_nocond;
                    }
                } else {
                    MARKLOCK;
                    LDAXRxw(x1, ed);
                    STLXRxw(x3, gd, ed);
                    CBNZx_MARKLOCK(x3);
                    if(!ALIGNED_ATOMICxw) {
                        B_MARK2_nocond;
                    }
                }
                if(!ALIGNED_ATOMICxw) {
                    MARK;
                    LDRxw_U12(x1, ed, 0);
                    LDAXRB(x4, ed);
                    SUBxw_UXTB(x4, x4, x1);
                    CBNZw_MARK(x4);
                    STLXRB(x3, gd, ed);
                    CBNZx_MARK(x3);
                    STRxw_U12(gd, ed, 0);
                    MARK2;
                }
                if(!ALIGNED_ATOMICxw || !cpuext.atomics) {
                    MOVxw_REG(gd, x1);
                }
            }
            break;

        case 0xF6:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("Invalid LOCK");
                    UDF(0);
                    *need_epilog = 1;
                    *ok = 0;
                    break;
                case 2:
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK NOT Eb");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                        if(cpuext.atomics) {
                            MOV32w(x1, 0xff);
                            STEORLB(x1, wback);
                        } else {
                            MARKLOCK;
                            LDAXRB(x1, wback);
                            MVNw_REG(x1, x1);
                            STLXRB(x3, x1, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xF7:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK NOT Ed");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                        if(cpuext.atomics) {
                            MOV64x(x1, ~0LL);
                            STEORLxw(x1, wback);
                        } else {
                            MARKLOCK;
                            LDAXRxw(x1, wback);
                            MVNw_REG(x1, x1);
                            STLXRxw(x3, x1, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xFE:
            nextop = F8;
            switch((nextop>>3)&7)
            {
                case 0: // INC Eb
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK INC Eb");
                        SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                        MARKLOCK;
                        if(cpuext.atomics) {
                            MOV32w(x3, 1);
                            UFLAG_IF {
                                LDADDALB(x3, x1, wback);
                                emit_inc8(dyn, ninst, x1, x3, x4);
                            } else {
                                STADDLB(x3, wback);
                            }
                        } else {
                            LDAXRB(x1, wback);
                            emit_inc8(dyn, ninst, x1, x3, x4);
                            STLXRB(x3, x1, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                    }
                    break;
                case 1: //DEC Eb
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK DEC Eb");
                        SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                        if(cpuext.atomics) {
                            MOV32w(x3, -1);
                            UFLAG_IF {
                                LDADDALB(x3, x1, wback);
                                emit_dec8(dyn, ninst, x1, x3, x4);
                            } else {
                                STADDLB(x3, wback);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRB(x1, wback);
                            emit_dec8(dyn, ninst, x1, x3, x4);
                            STLXRB(x3, x1, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xFF:
            nextop = F8;
            switch((nextop>>3)&7)
            {
                case 0: // INC Ed
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK INC Ed");
                        SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                        if(!ALIGNED_ATOMICxw) {
                            if(cpuext.uscat) {
                                ANDx_mask(x1, wback, 1, 0, 3);  // mask = F
                                CMPSw_U12(x1, 16-(1<<(2+rex.w)));
                                B_MARK(cGT);
                            } else {
                                TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                                B_MARK(cNE);    // unaligned
                            }
                        }
                        if(cpuext.atomics) {
                            MOV32w(x3, 1);
                            UFLAG_IF {
                                LDADDALxw(x3, x1, wback);
                            } else {
                                STADDLxw(x3, wback);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRxw(x1, wback);
                            ADDxw_U12(x4, x1, 1);
                            STLXRxw(x3, x4, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                        if(!ALIGNED_ATOMICxw) {
                            UFLAG_IF {
                                B_MARK2_nocond;
                            } else {
                                B_NEXT_nocond;
                            }
                            MARK;
                            LDRxw_U12(x1, wback, 0);
                            LDAXRB(x4, wback);
                            SUBxw_UXTB(x4, x4, x1);
                            CBNZw_MARK(x4);
                            ADDxw_U12(x4, x1, 1);
                            STLXRB(x3, x4, wback);
                            CBNZw_MARK(x3);
                            STRxw_U12(x4, wback, 0);
                        }
                        UFLAG_IF {
                            MARK2;
                            emit_inc32(dyn, ninst, rex, x1, x3, x4);
                        }
                    }
                    break;
                case 1: //DEC Ed
                    if(MODREG) {
                        INST_NAME("Invalid LOCK");
                        UDF(0);
                        *need_epilog = 1;
                        *ok = 0;
                    } else {
                        INST_NAME("LOCK DEC Ed");
                        SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, NULL, 0, 0, rex, LOCK_LOCK, 0, 0);
                        if(!ALIGNED_ATOMICxw) {
                            if(cpuext.uscat) {
                                ANDx_mask(x1, wback, 1, 0, 3);  // mask = F
                                CMPSw_U12(x1, 16-(1<<(2+rex.w)));
                                B_MARK(cGT);
                            } else {
                                TSTx_mask(wback, 1, 0, 1+rex.w);    // mask=3 or 7
                                B_MARK(cNE);    // unaligned
                            }
                        }
                        if(cpuext.atomics) {
                            MOV64xw(x3, -1);
                            UFLAG_IF {
                                LDADDALxw(x3, x1, wback);
                            } else {
                                STADDLxw(x3, wback);
                            }
                        } else {
                            MARKLOCK;
                            LDAXRxw(x1, wback);
                            SUBxw_U12(x4, x1, 1);
                            STLXRxw(x3, x4, wback);
                            CBNZx_MARKLOCK(x3);
                        }
                        if(!ALIGNED_ATOMICxw) {
                            UFLAG_IF {
                                B_MARK2_nocond;
                            } else {
                                B_NEXT_nocond;
                            }
                            MARK;
                            LDRxw_U12(x1, wback, 0);
                            LDAXRB(x4, wback);
                            SUBxw_UXTB(x4, x4, x1);
                            CBNZw_MARK(x4);
                            SUBxw_U12(x4, x1, 1);
                            STLXRB(x3, x4, wback);
                            CBNZw_MARK(x3);
                            STRxw_U12(x4, wback, 0);
                        }
                        UFLAG_IF {
                            MARK2;
                            emit_dec32(dyn, ninst, rex, x1, x3, x4);
                        }
                    }
                    break;
                default:
                    INST_NAME("Invalid LOCK");
                    UDF(0);
                    *need_epilog = 1;
                    *ok = 0;
                    break;
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}
