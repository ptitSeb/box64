#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64run.h"
#include "x64emu_private.h"
#include "x64run_private.h"
#include "x64primop.h"
#include "x64trace.h"
#include "x87emu_private.h"
#include "box64context.h"
#include "bridge.h"

#include "modrm.h"

int Run64(x64emu_t *emu, rex_t rex)
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;   (void)tmp8u;
    int16_t tmp16s;  (void)tmp16s;
    uint16_t tmp16u; (void)tmp16u;
    int32_t tmp32s;
    uint32_t tmp32u;
    uint64_t tmp64u;
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx;
    int rep;
    uintptr_t tlsdata = GetFSBaseEmu(emu);

    opcode = F8;
    // REX prefix before the F0 are ignored
    rex.rex = 0;
    while(opcode>=0x40 && opcode<=0x4f) {
        rex.rex = opcode;
        opcode = F8;
    }
    rep = 0;
    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    switch(opcode) {
        #define GO(B, OP)                                   \
        case B+0:                                           \
            nextop = F8;                                    \
            GETEB_OFFS(0, tlsdata);                         \
            GETGB;                                          \
            EB->byte[0] = OP##8(emu, EB->byte[0], GB);      \
            break;                                          \
        case B+1:                                           \
            nextop = F8;                                    \
            GETED_OFFS(0, tlsdata);                         \
            GETGD;                                          \
            if(rex.w)                                       \
                ED->q[0] = OP##64(emu, ED->q[0], GD->q[0]); \
            else {                                          \
                if(MODREG)                                  \
                    ED->q[0] = OP##32(emu, ED->dword[0], GD->dword[0]);     \
                else                                                        \
                    ED->dword[0] = OP##32(emu, ED->dword[0], GD->dword[0]); \
            }                                               \
            break;                                          \
        case B+2:                                           \
            nextop = F8;                                    \
            GETEB_OFFS(0, tlsdata);                         \
            GETGB;                                          \
            GB = OP##8(emu, GB, EB->byte[0]);               \
            break;                                          \
        case B+3:                                           \
            nextop = F8;                                    \
            GETED_OFFS(0, tlsdata);                         \
            GETGD;                                          \
            if(rex.w)                                       \
                GD->q[0] = OP##64(emu, GD->q[0], ED->q[0]); \
            else                                            \
                GD->q[0] = OP##32(emu, GD->dword[0], ED->dword[0]); \
            break;                                          \
        case B+4:                                           \
            R_AL = OP##8(emu, R_AL, F8);                    \
            break;                                          \
        case B+5:                                           \
            if(rex.w)                                       \
                R_RAX = OP##64(emu, R_RAX, F32S64);         \
            else                                            \
                R_RAX = OP##32(emu, R_EAX, F32);            \
            break;

        GO(0x00, add)                   /* ADD 0x00 -> 0x05 */
        GO(0x08, or)                    /*  OR 0x08 -> 0x0D */
        GO(0x10, adc)                   /* ADC 0x10 -> 0x15 */
        GO(0x18, sbb)                   /* SBB 0x18 -> 0x1D */
        GO(0x20, and)                   /* AND 0x20 -> 0x25 */
        GO(0x28, sub)                   /* SUB 0x28 -> 0x2D */
        GO(0x30, xor)                   /* XOR 0x30 -> 0x35 */
        #undef GO
        case 0x0F:
            opcode = F8;
            switch(opcode) {

                case 0x10:
                    switch(rep) {
                        case 1: /* MOVSD Gx, FS:Ex */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            GX->q[0] = EX->q[0];
                            if((nextop&0xC0)!=0xC0) {
                                // EX is not a register
                                GX->q[1] = 0;
                            }
                            break;
                        case 2: /* MOVSS Gx, FS:Ex */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            GX->ud[0] = EX->ud[0];
                            if((nextop&0xC0)!=0xC0) {
                                // EX is not a register (reg to reg only move 31:0)
                                GX->ud[1] = GX->ud[2] = GX->ud[3] = 0;
                            }
                            break;
                        default:
                            return 1;
                    }
                    break;
                case 0x11:
                    switch(rep) {
                        case 1: /* MOVSD Ex, Gx */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            EX->q[0] = GX->q[0];
                            break;
                        case 2: /* MOVSS FS:Ex, Gx */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            EX->ud[0] = GX->ud[0];
                            break;
                        default:
                            return 1;
                    }
                    break;

                case 0x29:                      /* MOVAPS Ex,Gx */
                    switch(rep) {
                        case 0:
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            EX->q[0] = GX->q[0];
                            EX->q[1] = GX->q[1];
                            break;
                        default:
                            return 1;
                    }
                    break;
                
                case 0x59:
                    switch(rep) {
                        case 2: /* MULSS Gx, Ex */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            GX->f[0] *= EX->f[0];
                            break;

                        default:
                            return 1;
                    }
                    break;

                default:
                    return 1;
            }
            break;

        case 0x38:
            nextop = F8;
            GETEB_OFFS(0, tlsdata);
            GETGB;
            cmp8(emu, EB->byte[0], GB);
            break;
        case 0x39:
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            GETGD;
            if(rex.w)
                cmp64(emu, ED->q[0], GD->q[0]);
            else
                cmp32(emu, ED->dword[0], GD->dword[0]);
            break;

        case 0x3B:
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            GETGD;
            if(rex.w)
                cmp64(emu, GD->q[0], ED->q[0]);
            else
                cmp32(emu, GD->dword[0], ED->dword[0]);
            break;


        case 0x66:
            opcode = F8;

            while(opcode>=0x40 && opcode<=0x4F) {
                rex.rex = opcode;
                opcode = F8;
            }
            switch(opcode) {
                case 0x0F:
                    opcode = F8;
                    switch(opcode) {
                        case 0xD6:                      /* MOVQ Ex,Gx */
                            nextop = F8;
                            GETEX_OFFS(0, tlsdata);
                            GETGX;
                            EX->q[0] = GX->q[0];
                            if(MODREG)
                                EX->q[1] = 0;
                            break;

                        default:
                            return 1;
                    }
                    break;

                default:
                    return 1;
            }
            break;

        case 0x80:                      /* GRP Eb,Ib */
            nextop = F8;
            GETEB_OFFS(1, tlsdata);
            tmp8u = F8;
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = add8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] =  or8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = adc8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = sbb8(emu, EB->byte[0], tmp8u); break;
                case 4: EB->byte[0] = and8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = sub8(emu, EB->byte[0], tmp8u); break;
                case 6: EB->byte[0] = xor8(emu, EB->byte[0], tmp8u); break;
                case 7:               cmp8(emu, EB->byte[0], tmp8u); break;
            }
            break;
        case 0x81:                      /* GRP Ed,Id */
        case 0x83:                      /* GRP Ed,Ib */
            nextop = F8;
            GETED_OFFS((opcode==0x81)?4:1, tlsdata);
            if(opcode==0x81) {
                tmp32s = F32S;
            } else {
                tmp32s = F8S;
            }
            if(rex.w) {
                tmp64u = (uint64_t)tmp32s;
                switch((nextop>>3)&7) {
                    case 0: ED->q[0] = add64(emu, ED->q[0], tmp64u); break;
                    case 1: ED->q[0] =  or64(emu, ED->q[0], tmp64u); break;
                    case 2: ED->q[0] = adc64(emu, ED->q[0], tmp64u); break;
                    case 3: ED->q[0] = sbb64(emu, ED->q[0], tmp64u); break;
                    case 4: ED->q[0] = and64(emu, ED->q[0], tmp64u); break;
                    case 5: ED->q[0] = sub64(emu, ED->q[0], tmp64u); break;
                    case 6: ED->q[0] = xor64(emu, ED->q[0], tmp64u); break;
                    case 7:            cmp64(emu, ED->q[0], tmp64u); break;
                }
            } else {
                tmp32u = (uint32_t)tmp32s;
                if(MODREG)
                    switch((nextop>>3)&7) {
                        case 0: ED->q[0] = add32(emu, ED->dword[0], tmp32u); break;
                        case 1: ED->q[0] =  or32(emu, ED->dword[0], tmp32u); break;
                        case 2: ED->q[0] = adc32(emu, ED->dword[0], tmp32u); break;
                        case 3: ED->q[0] = sbb32(emu, ED->dword[0], tmp32u); break;
                        case 4: ED->q[0] = and32(emu, ED->dword[0], tmp32u); break;
                        case 5: ED->q[0] = sub32(emu, ED->dword[0], tmp32u); break;
                        case 6: ED->q[0] = xor32(emu, ED->dword[0], tmp32u); break;
                        case 7:            cmp32(emu, ED->dword[0], tmp32u); break;
                    }
                else
                    switch((nextop>>3)&7) {
                        case 0: ED->dword[0] = add32(emu, ED->dword[0], tmp32u); break;
                        case 1: ED->dword[0] =  or32(emu, ED->dword[0], tmp32u); break;
                        case 2: ED->dword[0] = adc32(emu, ED->dword[0], tmp32u); break;
                        case 3: ED->dword[0] = sbb32(emu, ED->dword[0], tmp32u); break;
                        case 4: ED->dword[0] = and32(emu, ED->dword[0], tmp32u); break;
                        case 5: ED->dword[0] = sub32(emu, ED->dword[0], tmp32u); break;
                        case 6: ED->dword[0] = xor32(emu, ED->dword[0], tmp32u); break;
                        case 7:                cmp32(emu, ED->dword[0], tmp32u); break;
                    }
            }
            break;

        case 0x88:                      /* MOV Eb,Gb */
            nextop = F8;
            GETEB_OFFS(0, tlsdata);
            GETGB;
            EB->byte[0] = GB;
            break;
        case 0x89:                    /* MOV Ed,Gd */
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            GETGD;
            if(rex.w) {
                ED->q[0] = GD->q[0];
            } else {
                //if ED is a reg, than the opcode works like movzx
                if(MODREG)
                    ED->q[0] = GD->dword[0];
                else
                    ED->dword[0] = GD->dword[0];
            }
            break;

        case 0x8B:                      /* MOV Gd,Ed */
            nextop = F8;
            GETED_OFFS(0, tlsdata);
            GETGD;
            if(rex.w)
                GD->q[0] = ED->q[0];
            else
                GD->q[0] = ED->dword[0];
            break;

        case 0xC6:                      /* MOV Eb,Ib */
            nextop = F8;
            GETEB_OFFS(1, tlsdata);
            EB->byte[0] = F8;
            break;
        case 0xC7:                      /* MOV Ed,Id */
            nextop = F8;
            GETED_OFFS(4, tlsdata);
            if(rex.w)
                ED->q[0] = F32S64;
            else
                if(MODREG)
                    ED->q[0] = F32;
                else
                    ED->dword[0] = F32;
            break;

       default:
            return 1;
    }
    return 0;
}
