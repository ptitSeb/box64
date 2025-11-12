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

#define GETGm   gd = ((nextop&0x38)>>3)

uintptr_t dynarec64_67_32(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int rep, int* ok, int* need_epilog)
{
    (void)ip; (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop;
    uint8_t gd, ed, wback, wb, wb1, wb2, gb1, gb2, eb1, eb2;
    int64_t fixedaddress;
    int unscaled;
    int8_t  i8;
    uint8_t u8;
    int32_t i32;
    int64_t j64, i64;
    int cacheupd = 0;
    int lock;
    int v0, v1, s0;
    MAYUSE(i32);
    MAYUSE(j64);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(s0);
    MAYUSE(lock);
    MAYUSE(cacheupd);

    if(!rex.is32bits) {
        // should do a different file
        DEFAULT;
        return addr;
    }

    rep = 0;
    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    switch(opcode) {
        
        case 0x0F:
            opcode=F8;
            switch(opcode) {

                case 0x29:
                    INST_NAME("MOVAPS Ex,Gx");
                    nextop = F8;
                    GETG;
                    v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
                    if(MODREG) {
                        ed = (nextop&7)+(rex.b<<3);
                        v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                        VMOVQ(v1, v0);
                    } else {
                        addr = geted16(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, &unscaled, 0xfff<<4, 15, 0);
                        VST128(v0, ed, fixedaddress);
                        SMWRITE2();
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0x64:
            addr = dynarec64_6764_32(dyn, addr, ip, ninst, rex, rep, _FS, ok, need_epilog);
            break;

        #define GO(NO, YES)                                             \
            JUMP(addr+i8, 1);                                           \
            if(dyn->insts[ninst].x64.jmp_insts==-1 ||                   \
                CHECK_CACHE()) {                                        \
                /* out of the block */                                  \
                i32 = dyn->insts[ninst].epilog-(dyn->native_size);      \
                Bcond(NO, i32);                                         \
                if(dyn->insts[ninst].x64.jmp_insts==-1) {               \
                    if(!(dyn->insts[ninst].x64.barrier&BARRIER_FLOAT))  \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3, 0);   \
                    jump_to_next(dyn, addr+i8, 0, ninst, rex.is32bits); \
                } else {                                                \
                    CacheTransform(dyn, ninst, cacheupd);               \
                    i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);\
                    SKIP_SEVL(i32);                                     \
                    B(i32);                                             \
                }                                                       \
            } else {                                                    \
                /* inside the block */                                  \
                i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address-(dyn->native_size);    \
                SKIP_SEVL(i32);                                         \
                Bcond(YES, i32);                                        \
            }
        case 0xE0:
            INST_NAME("LOOPNZ (16bits)");
            READFLAGS(X_ZF);
            i8 = F8S;
            SMEND();
            UXTHw(x1, xRCX);
            SUBw_U12(x1, x1, 1);
            BFIx(xRCX, x1, 0, 16);
            CBZw_NEXT(x1);    // ECX is 0, no LOOP
            TSTw_mask(xFlags, 0b011010, 0); //mask=0x40
            GO(cNE, cEQ);
            break;
        case 0xE1:
            INST_NAME("LOOPZ (16bits)");
            READFLAGS(X_ZF);
            SMEND();
            i8 = F8S;
            UXTHw(x1, xRCX);
            SUBw_U12(x1, x1, 1);
            BFIx(xRCX, x1, 0, 16);
            CBZw_NEXT(x1);    // ECX is 0, no LOOP
            TSTw_mask(xFlags, 0b011010, 0); //mask=0x40
            GO(cEQ, cNE);
            break;
        case 0xE2:
            INST_NAME("LOOP (16bits)");
            i8 = F8S;
            SMEND();
            UXTHw(x1, xRCX);
            SUBSw_U12(x1, x1, 1);
            BFIx(xRCX, x1, 0, 16);
            GO(cEQ, cNE);
            break;
        case 0xE3:
            INST_NAME("JCXZ");
            i8 = F8S;
            SMEND();
            TSTw_mask(xRCX, 0, 15); //mask=0xffff
            GO(cNE, cEQ);
            break;
        #undef GO
        
        default:
            DEFAULT;
    }
    return addr;
}
