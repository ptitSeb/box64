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
//#include "my_cpuid.h"
#include "bridge.h"
//#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

int my_setcontext(x64emu_t* emu, void* ucp);

int Run(x64emu_t *emu, int step)
{
    uint8_t opcode;
    uint8_t nextop;
    reg64_t *oped, *opgd;
    uint8_t tmp8u, tmp8u2;
    int8_t tmp8s;
    uint16_t tmp16u, tmp16u2;
    int16_t tmp16s;
    uint32_t tmp32u, tmp32u2, tmp32u3;
    uint64_t tmp64u, tmp64u2, tmp64u3;
    int32_t tmp32s, tmp32s2;
    int64_t tmp64s, tmp64s2;
    double d;
    float f;
    rex_t rex;
    int unimp = 0;

    if(emu->quit)
        return 0;

    //ref opcode: http://ref.x64asm.net/geek32.html#xA1
    printf_log(LOG_DEBUG, "Run X86 (%p), RIP=%p, Stack=%p\n", emu, (void*)R_RIP, emu->context->stack);
#define F8      *(uint8_t*)(R_RIP++)
#define F8S     *(int8_t*)(R_RIP++)
#define F16     *(uint16_t*)(R_RIP+=2, R_RIP-2)
#define F32     *(uint32_t*)(R_RIP+=4, R_RIP-4)
#define F32S    *(int32_t*)(R_RIP+=4, R_RIP-4)
#define F64     *(uint64_t*)(R_RIP+=8, R_RIP-8)
#define F64S    *(int64_t*)(R_RIP+=8, R_RIP-8)
#define PK(a)   *(uint8_t*)(R_RIP+a)
#ifdef DYNAREC
#define STEP if(step) return 0;
#else
#define STEP
#endif

#define GETED oped=GetEd(emu, rex, nextop)
#define GETGD opgd=GetGd(emu, rex, nextop)
#define GETEB oped=GetEb(emu, rex, nextop)
#define GETGB oped=GetGb(emu, rex, nextop)
#define ED  oped
#define GD  opgd
#define EB  oped
#define GB  oped->byte[0]

x64emurun:

//#include "modrm.h"
    while(1) {
#ifdef HAVE_TRACE
        __builtin_prefetch((void*)R_RIP, 0, 0); 
        emu->prev2_ip = emu->old_ip;
        if(my_context->dec && (
            (trace_end == 0) 
            || ((R_RIP >= trace_start) && (R_RIP < trace_end))) )
                PrintTrace(emu, R_RIP, 0);
#endif
        emu->old_ip = R_RIP;

        opcode = F8;
        if(opcode>=0x40 && opcode<=0x4f) {
            rex.rex = opcode;
            opcode = F8;
        } else
            rex.rex = 0;

        switch(opcode) {

        #define GO(B, OP)                                   \
        case B+0:                                           \
            nextop = F8;                                    \
            GETEB;                                          \
            GETGB;                                          \
            EB->byte[0] = OP##8(emu, EB->byte[0], GB);      \
            break;                                          \
        case B+1:                                           \
            nextop = F8;                                    \
            GETED;                                          \
            GETGD;                                          \
            if(rex.w)                                       \
                ED->q[0] = OP##64(emu, ED->q[0], GD->q[0]); \
            else                                            \
                ED->dword[0] = OP##32(emu, ED->dword[0], GD->dword[0]); \
            break;                                          \
        case B+2:                                           \
            nextop = F8;                                    \
            GETEB;                                          \
            GETGB;                                          \
            GB = OP##8(emu, GB, EB->byte[0]);               \
            break;                                          \
        case B+3:                                           \
            nextop = F8;                                    \
            GETED;                                          \
            GETGD;                                          \
            if(rex.w)                                       \
                GD->q[0] = OP##64(emu, GD->q[0], ED->q[0]); \
            else                                            \
                GD->dword[0] = OP##32(emu, GD->dword[0], ED->dword[0]); \
            break;                                          \
        case B+4:                                           \
            R_AL = OP##8(emu, R_AL, F8);                    \
            break;                                          \
        case B+5:                                           \
            if(rex.w)                                       \
                R_RAX = OP##64(emu, R_RAX, (uint64_t)(int64_t)F32S);    \
            else                                            \
                R_EAX = OP##32(emu, R_EAX, F32);            \
            break;

        GO(0x00, add)                   /* ADD 0x00 -> 0x05 */
        GO(0x08, or)                    /*  OR 0x08 -> 0x0D */
        case 0x0F:                      /* More instructions */
            if(Run0F(emu)) {
                unimp = 1;
                goto fini;
            }
            if(emu->quit)
                goto fini;
            break;
        GO(0x10, adc)                   /* ADC 0x10 -> 0x15 */
        GO(0x18, sbb)                   /* SBB 0x18 -> 0x1D */
        GO(0x20, and)                   /* AND 0x20 -> 0x25 */
        GO(0x28, sub)                   /* SUB 0x28 -> 0x2D */
        GO(0x30, xor)                   /* XOR 0x30 -> 0x35 */
        #undef GO

        case 0x38:
            nextop = F8;
            GETEB;
            GETGB;
            cmp8(emu, EB->byte[0], GB);
            break;
        case 0x39:
            nextop = F8;
            GETED;
            GETGD;
            if(rex.w)
                cmp64(emu, ED->q[0], GD->q[0]);
            else
                cmp32(emu, ED->dword[0], GD->dword[0]);
            break;
        case 0x3A:
            nextop = F8;
            GETEB;
            GETGB;
            cmp8(emu, GB, EB->byte[0]);
            break;
        case 0x3B:
            nextop = F8;
            GETED;
            GETGD;
            if(rex.w)
                cmp64(emu, GD->q[0], ED->q[0]);
            else
                cmp32(emu, GD->dword[0], ED->dword[0]);
            break;
        case 0x3C:
            cmp8(emu, R_AL, F8);
            break;
        case 0x3D:
            if(rex.w)
                cmp64(emu, R_RAX, (uint64_t)(int64_t)F32S);
            else
                cmp32(emu, R_EAX, F32);
            break;

        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:      /* Another REX */
            --R_RIP;
            break;
        case 0x54:                      /* PUSH ESP */
            if(rex.b)
                Push(emu, R_R12);
            else {
                tmp64u = R_RSP;
                Push(emu, tmp64u);
            }
            break;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x55:
        case 0x56:
        case 0x57:                      /* PUSH Reg */
            tmp8u = (opcode&7)+(rex.b<<3);
            Push(emu, emu->regs[tmp8u].q[0]);
            break;
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:                      /* POP ESP */
        case 0x5D:
        case 0x5E:
        case 0x5F:                      /* POP Reg */
            tmp8u = (opcode&7)+(rex.b<<3);
            emu->regs[tmp8u].q[0] = Pop(emu);
            break;

        #define GOCOND(B, PREFIX, CONDITIONAL)  \
        case B+0x0:                             \
            PREFIX                              \
            if(ACCESS_FLAG(F_OF))               \
                CONDITIONAL                     \
            break;                              \
        case B+0x1:                             \
            PREFIX                              \
            if(!ACCESS_FLAG(F_OF))              \
                CONDITIONAL                     \
            break;                              \
        case B+0x2:                             \
            PREFIX                              \
            if(ACCESS_FLAG(F_CF))               \
                CONDITIONAL                     \
            break;                              \
        case B+0x3:                             \
            PREFIX                              \
            if(!ACCESS_FLAG(F_CF))              \
                CONDITIONAL                     \
            break;                              \
        case B+0x4:                             \
            PREFIX                              \
            if(ACCESS_FLAG(F_ZF))               \
                CONDITIONAL                     \
            break;                              \
        case B+0x5:                             \
            PREFIX                              \
            if(!ACCESS_FLAG(F_ZF))              \
                CONDITIONAL                     \
            break;                              \
        case B+0x6:                             \
            PREFIX                              \
            if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))  \
                CONDITIONAL                     \
            break;                              \
        case B+0x7:                             \
            PREFIX                              \
            if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) \
                CONDITIONAL                     \
            break;                              \
        case B+0x8:                             \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF))               \
                CONDITIONAL                     \
            break;                              \
        case B+0x9:                             \
            PREFIX                              \
            if(!ACCESS_FLAG(F_SF))              \
                CONDITIONAL                     \
            break;                              \
        case B+0xA:                             \
            PREFIX                              \
            if(ACCESS_FLAG(F_PF))               \
                CONDITIONAL                     \
            break;                              \
        case B+0xB:                             \
            PREFIX                              \
            if(!ACCESS_FLAG(F_PF))              \
                CONDITIONAL                     \
            break;                              \
        case B+0xC:                             \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))  \
                CONDITIONAL                     \
            break;                              \
        case B+0xD:                             \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)) \
                CONDITIONAL                     \
            break;                              \
        case B+0xE:                             \
            PREFIX                              \
            if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))) \
                CONDITIONAL                     \
            break;                              \
        case B+0xF:                             \
            PREFIX                              \
            if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))) \
                CONDITIONAL                     \
            break;
        GOCOND(0x70
            ,   tmp8s = F8S; CHECK_FLAGS(emu);
            ,   R_RIP += tmp8s;
            )                           /* Jxx Ib */
        #undef GOCOND
        
        case 0x81:                      /* GRP Ed,Id */
        case 0x83:                      /* GRP Ed,Ib */
            nextop = F8;
            GETED;
            if(opcode==0x81) {
                tmp32s = F32S;
            } else {
                tmp32s = F8S;
            }
            if(rex.w) {
                tmp64u = (uint64_t)tmp32s;
                switch((nextop>>3)&7) {
                    case 0: ED->dword[0] = add64(emu, ED->dword[0], tmp64u); break;
                    case 1: ED->dword[0] =  or64(emu, ED->dword[0], tmp64u); break;
                    case 2: ED->dword[0] = adc64(emu, ED->dword[0], tmp64u); break;
                    case 3: ED->dword[0] = sbb64(emu, ED->dword[0], tmp64u); break;
                    case 4: ED->dword[0] = and64(emu, ED->dword[0], tmp64u); break;
                    case 5: ED->dword[0] = sub64(emu, ED->dword[0], tmp64u); break;
                    case 6: ED->dword[0] = xor64(emu, ED->dword[0], tmp64u); break;
                    case 7:                cmp64(emu, ED->dword[0], tmp64u); break;
                }
            } else {
                tmp32u = (uint32_t)tmp32s;
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

        case 0x85:                      /* TEST Ed,Gd */
            nextop = F8;
            GETED;
            GETGD;
            if(rex.w)
                test64(emu, ED->q[0], GD->q[0]);
            else
                test32(emu, ED->dword[0], GD->dword[0]);
            break;

        case 0x89:                    /* MOV Ed,Gd */
            nextop = F8;
            GETED;
            GETGD;
            if(rex.w)
                ED->q[0] = GD->q[0];
            else
                ED->dword[0] = GD->dword[0];
            break;

        case 0x8B:                      /* MOV Gd,Ed */
            nextop = F8;
            GETED;
            GETGD;
            if(rex.w)
                GD->q[0] = ED->q[0];
            else
                GD->dword[0] = ED->dword[0];
            break;

        case 0x8D:                      /* LEA Gd,M */
            nextop = F8;
            GETED;
            GETGD;
            if(rex.w)
                GD->q[0] = (uint64_t)ED;
            else
                GD->dword[0] = (uint32_t)(uintptr_t)ED;
            break;

        case 0xC1:                      /* GRP2 Ed,Ib */
            nextop = F8;
            GETED;
            tmp8u = F8/* & 0x1f*/; // masking done in each functions
            if(rex.w) {
                switch((nextop>>3)&7) {
                    case 0: ED->q[0] = rol64(emu, ED->q[0], tmp8u); break;
                    case 1: ED->q[0] = ror64(emu, ED->q[0], tmp8u); break;
                    case 2: ED->q[0] = rcl64(emu, ED->q[0], tmp8u); break;
                    case 3: ED->q[0] = rcr64(emu, ED->q[0], tmp8u); break;
                    case 4:
                    case 6: ED->q[0] = shl64(emu, ED->q[0], tmp8u); break;
                    case 5: ED->q[0] = shr64(emu, ED->q[0], tmp8u); break;
                    case 7: ED->q[0] = sar64(emu, ED->q[0], tmp8u); break;
                }
            } else {
                switch((nextop>>3)&7) {
                    case 0: ED->dword[0] = rol32(emu, ED->dword[0], tmp8u); break;
                    case 1: ED->dword[0] = ror32(emu, ED->dword[0], tmp8u); break;
                    case 2: ED->dword[0] = rcl32(emu, ED->dword[0], tmp8u); break;
                    case 3: ED->dword[0] = rcr32(emu, ED->dword[0], tmp8u); break;
                    case 4:
                    case 6: ED->dword[0] = shl32(emu, ED->dword[0], tmp8u); break;
                    case 5: ED->dword[0] = shr32(emu, ED->dword[0], tmp8u); break;
                    case 7: ED->dword[0] = sar32(emu, ED->dword[0], tmp8u); break;
                }
            }
            break;

        case 0xC3:                      /* RET */
            R_RIP = Pop(emu);
            STEP
            break;

        case 0xCC:                      /* INT 3 */
            x64Int3(emu);
            if(emu->quit) goto fini;
            break;

        case 0xD1:                      /* GRP2 Ed,1 */
        case 0xD3:                      /* GRP2 Ed,CL */
            nextop = F8;
            GETED;
            tmp8u = (opcode==0xD1)?1:R_CL;
            if(rex.w) {
                switch((nextop>>3)&7) {
                    case 0: ED->q[0] = rol64(emu, ED->q[0], tmp8u); break;
                    case 1: ED->q[0] = ror64(emu, ED->q[0], tmp8u); break;
                    case 2: ED->q[0] = rcl64(emu, ED->q[0], tmp8u); break;
                    case 3: ED->q[0] = rcr64(emu, ED->q[0], tmp8u); break;
                    case 4: 
                    case 6: ED->q[0] = shl64(emu, ED->q[0], tmp8u); break;
                    case 5: ED->q[0] = shr64(emu, ED->q[0], tmp8u); break;
                    case 7: ED->q[0] = sar64(emu, ED->q[0], tmp8u); break;
                }
            } else {
                switch((nextop>>3)&7) {
                    case 0: ED->dword[0] = rol32(emu, ED->dword[0], tmp8u); break;
                    case 1: ED->dword[0] = ror32(emu, ED->dword[0], tmp8u); break;
                    case 2: ED->dword[0] = rcl32(emu, ED->dword[0], tmp8u); break;
                    case 3: ED->dword[0] = rcr32(emu, ED->dword[0], tmp8u); break;
                    case 4: 
                    case 6: ED->dword[0] = shl32(emu, ED->dword[0], tmp8u); break;
                    case 5: ED->dword[0] = shr32(emu, ED->dword[0], tmp8u); break;
                    case 7: ED->dword[0] = sar32(emu, ED->dword[0], tmp8u); break;
                }
            }
            break;

        case 0xE8:                      /* CALL Id */
            tmp32s = F32S; // call is relative
            Push(emu, R_RIP);
            R_RIP += tmp32s;
            STEP
            break;
        case 0xE9:                      /* JMP Id */
            tmp32s = F32S; // jmp is relative
            R_RIP += tmp32s;
            STEP
            break;

        case 0xFF:                      /* GRP 5 Ed */
            nextop = F8;
            GETED;
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
                    if(rex.w)
                        ED->q[0] = inc64(emu, ED->q[0]);
                    else
                        ED->dword[0] = inc32(emu, ED->dword[0]);
                    break;
                case 1:                 /* DEC Ed */
                    if(rex.w)
                        ED->q[0] = dec64(emu, ED->q[0]);
                    else
                        ED->dword[0] = dec32(emu, ED->dword[0]);
                    break;
                case 2:                 /* CALL NEAR Ed */
                    tmp64u = (uintptr_t)getAlternate((void*)ED->q[0]);
                    Push(emu, R_RIP);
                    R_RIP = tmp64u;
                    STEP
                    break;
                case 3:                 /* CALL FAR Ed */
                    if(nextop>0xC0) {
                        R_RIP = emu->old_ip;
                        printf_log(LOG_NONE, "Illegal Opcode %p: %02X %02X %02X %02X\n", (void*)R_RIP, opcode, nextop, PK(2), PK(3));
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        goto fini;
                    } else {
                        Push16(emu, R_CS);
                        Push(emu, R_RIP);
                        R_RIP = ED->dword[0];
                        R_CS = (ED+1)->word[0];
                        goto fini;  // exit loop to recompute new CS...
                    }
                    break;
                case 4:                 /* JMP NEAR Ed */
                    R_RIP = (uintptr_t)getAlternate((void*)ED->q[0]);
                    STEP
                    break;
                case 5:                 /* JMP FAR Ed */
                    if(nextop>0xc0) {
                        R_RIP = emu->old_ip;
                        printf_log(LOG_NONE, "Illegal Opcode %p: 0x%02X 0x%02X %02X %02X\n", (void*)R_RIP, opcode, nextop, PK(2), PK(3));
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        goto fini;
                    } else {
                        R_RIP = ED->q[0];
                        R_CS = (ED+1)->word[0];
                        STEP
                        goto fini;  // exit loop to recompute CS...
                    }
                    break;
                case 6:                 /* Push Ed */
                    tmp64u = ED->q[0];  // rex.w ignored
                    Push(emu, tmp64u);  // avoid potential issue with push [esp+...]
                    break;
                default:
                    R_RIP = emu->old_ip;
                    printf_log(LOG_NONE, "Illegal Opcode %p: %02X %02X %02X %02X %02X %02X\n",(void*)R_RIP, opcode, nextop, PK(2), PK(3), PK(4), PK(5));
                    emu->quit=1;
                    emu->error |= ERR_ILLEGAL;
                    goto fini;
            }
            break;
        default:
            unimp = 1;
            goto fini;
        }
    }


fini:
    if(unimp) {
        R_RIP = emu->old_ip;
        emu->quit = 1;
        UnimpOpcode(emu);
    }
    // fork handling
//    if(emu->fork) {
//        if(step)
//            return 0;
//        int forktype = emu->fork;
//        emu->quit = 0;
//        emu->fork = 0;
//        emu = x64emu_fork(emu, forktype);
//        goto x64emurun;
//    }
    // setcontext handling
//    else if(emu->uc_link) {
//        emu->quit = 0;
//        my_setcontext(emu, emu->uc_link);
//        goto x64emurun;
//    }
    return 0;
}
