#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_TRACE
#include <unistd.h>
#include <sys/syscall.h>
#endif

#include "debug.h"
#include "box64stack.h"
#include "x64emu.h"
#include "x64run.h"
#include "x64run_private.h"
#include "x64emu_private.h"
#include "box64context.h"
#include "x64run.h"
#include "librarian.h"
#include "elfloader.h"
#ifdef HAVE_TRACE
#include "x64trace.h"
#endif
#include "x64tls.h"
#include "bridge.h"

#define PARITY(x)   (((emu->x64emu_parity_tab[(x) / 32] >> ((x) % 32)) & 1) == 0)
#define XOR2(x) 	(((x) ^ ((x)>>1)) & 0x1)

int32_t EXPORT my___libc_start_main(x64emu_t* emu, int *(main) (int, char * *, char * *), int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end))
{
    //TODO: register rtld_fini
    //TODO: register fini
    // let's cheat and set all args...
    if(init) {
        PushExit(emu);
        R_RIP=(uint64_t)*init;
        printf_log(LOG_DEBUG, "Calling init(%p) from __libc_start_main\n", *init);
        DynaRun(emu);
        if(emu->error)  // any error, don't bother with more
            return 0;
        emu->quit = 0;
    }
    printf_log(LOG_DEBUG, "Transfert to main(%d, %p, %p)=>%p from __libc_start_main\n", my_context->argc, my_context->argv, my_context->envv, main);
    // call main and finish
    Push64(emu, GetRBP(emu));   // set frame pointer
    SetRBP(emu, GetRSP(emu));   // save RSP
    SetRSP(emu, GetRSP(emu)&~0xFLL);    // Align RSP
    PushExit(emu);
    SetRDX(emu, (uint64_t)my_context->envv);
    SetRSI(emu, (uint64_t)my_context->argv);
    SetRDI(emu, (uint64_t)my_context->argc);
    R_RIP=(uint64_t)main;

    DynaRun(emu);

    SetRSP(emu, GetRBP(emu));   // restore RSP
    SetRBP(emu, Pop64(emu));         // restore RBP
    emu->quit = 1;  // finished!
    return 0;
}

const char* GetNativeName(void* p)
{
    static char buff[500] = {0};
    #ifdef HAVE_TRACE
    {
        const char* n = getBridgeName(p);
        if(n)
            return n;
    }
    #endif
    Dl_info info;
    if(dladdr(p, &info)==0) {
        const char *ret = GetNameOffset(my_context->maplib, p);
        if(ret)
            return ret;
        sprintf(buff, "%s(%p)", "???", p);
        return buff;
    } else {
        if(info.dli_sname) {
            strcpy(buff, info.dli_sname);
            if(info.dli_fname) {
                strcat(buff, " ("); strcat(buff, info.dli_fname); strcat(buff, ")");
            }
        } else {
            sprintf(buff, "%s(%s/%p)", "???", info.dli_fname, p);
            return buff;
        }
    }
    return buff;
}
void UpdateFlags(x64emu_t *emu)
{
    uint64_t cc;
    uint64_t lo, hi;
    uint64_t bc;
    uint64_t cnt;

    switch(emu->df) {
        case d_none:
            return;
        case d_add8:
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x100, F_CF);
            CONDITIONAL_SET_FLAG((emu->res.u16 & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);

            cc = (emu->op1.u8 & emu->op2.u8) | ((~emu->res.u8) & (emu->op1.u8 | emu->op2.u8));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_add16:
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG((emu->res.u32 & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            cc = (emu->op1.u16 & emu->op2.u16) | ((~emu->res.u16) & (emu->op1.u16 | emu->op2.u16));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_add32:
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x100000000LL, F_CF);
            CONDITIONAL_SET_FLAG((emu->res.u64 & 0xffffffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            cc = (emu->op1.u32 & emu->op2.u32) | ((~emu->res.u32) & (emu->op1.u32 | emu->op2.u32));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_add32b:
            lo = (emu->op2.u32 & 0xFFFF) + (emu->op1.u32 & 0xFFFF);
            hi = (lo >> 16) + (emu->op2.u32 >> 16) + (emu->op1.u32 >> 16);
            CONDITIONAL_SET_FLAG(hi & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            cc = (emu->op1.u32 & emu->op2.u32) | ((~emu->res.u32) & (emu->op1.u32 | emu->op2.u32));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_add64:
            lo = (emu->op2.u64 & 0xFFFFFFFF) + (emu->op1.u64 & 0xFFFFFFFF);
            hi = (lo >> 32) + (emu->op2.u64 >> 32) + (emu->op1.u64 >> 32);
            CONDITIONAL_SET_FLAG(hi & 0x100000000, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            cc = (emu->op1.u64 & emu->op2.u64) | ((~emu->res.u64) & (emu->op1.u64 | emu->op2.u64));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_and8:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u8 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
            break;
        case d_and16:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u16 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
            break;
        case d_and32:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u32 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            break;
        case d_and64:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u64 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            break;
        case d_dec8:
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG((emu->res.u8) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
            bc = (emu->res.u8 & (~emu->op1.u8 | 1)) | (~emu->op1.u8 & 1);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_dec16:
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res.u16) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
            bc = (emu->res.u16 & (~emu->op1.u16 | 1)) | (~emu->op1.u16 & 1);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_dec32:
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            bc = (emu->res.u32 & (~emu->op1.u32 | 1)) | (~emu->op1.u32 & 1);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_dec64:
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            bc = (emu->res.u64 & (~emu->op1.u64 | 1)) | (~emu->op1.u64 & 1);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_inc8:
            CONDITIONAL_SET_FLAG((emu->res.u8 & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
            cc = ((1 & emu->op1.u8) | (~emu->res.u8)) & (1 | emu->op1.u8);
            CONDITIONAL_SET_FLAG(XOR2(cc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_inc16:
            CONDITIONAL_SET_FLAG((emu->res.u16 & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
            cc = (1 & emu->op1.u16) | ((~emu->res.u16) & (1 | emu->op1.u16));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_inc32:
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            cc = (1 & emu->op1.u32) | ((~emu->res.u32) & (1 | emu->op1.u32));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_inc64:
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            cc = (1 & emu->op1.u64) | ((~emu->res.u64) & (1 | emu->op1.u64));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_imul8:
            lo = emu->res.u16 & 0xff;
            hi = (emu->res.u16>>8)&0xff;
            if (((lo & 0x80) == 0 && hi == 0x00) ||
                ((lo & 0x80) != 0 && hi == 0xFF)) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(lo & 0xff), F_PF);
            break;
        case d_imul16:
            lo = (uint16_t)emu->res.u32;
            hi = (uint16_t)(emu->res.u32 >> 16);
            if (((lo & 0x8000) == 0 && hi == 0x00) ||
                ((lo & 0x8000) != 0 && hi == 0xFFFF)) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(lo & 0xff), F_PF);
            break;
        case d_imul32:
            if (((emu->res.u32 & 0x80000000) == 0 && emu->op1.u32 == 0x00) ||
                ((emu->res.u32 & 0x80000000) != 0 && emu->op1.u32 == 0xFFFFFFFF)) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            break;
        case d_imul64:
            if (((emu->res.u64 & 0x8000000000000000LL) == 0 && emu->op1.u64 == 0x00) ||
                ((emu->res.u64 & 0x8000000000000000LL) != 0 && emu->op1.u64 == 0xFFFFFFFFFFFFFFFFLL)) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            break;
        case d_mul8:
            lo = emu->res.u16 & 0xff;
            hi = (emu->res.u16>>8)&0xff;
            if (hi == 0) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(lo & 0xff), F_PF);
            break;
        case d_mul16:
            lo = (uint16_t)emu->res.u32;
            hi = (uint16_t)(emu->res.u32 >> 16);
            if (hi == 0) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(lo & 0xff), F_PF);
            break;
        case d_mul32:
            if (emu->op1.u32 == 0) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            break;
        case d_mul64:
            if (emu->op1.u64 == 0) {
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
            } else {
                SET_FLAG(F_CF);
                SET_FLAG(F_OF);
            }
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            break;
        case d_or8:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u8 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
            break;
        case d_or16:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u16 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
            break;
        case d_or32:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u32 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            break;
        case d_or64:
            CLEAR_FLAG(F_OF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u64 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            break;
        case d_neg8:
            CONDITIONAL_SET_FLAG(emu->op1.u8 != 0, F_CF);
            CONDITIONAL_SET_FLAG((emu->res.u8 & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
            bc = emu->res.u8 | emu->op1.u8;
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_neg16:
            CONDITIONAL_SET_FLAG(emu->op1.u16 != 0, F_CF);
            CONDITIONAL_SET_FLAG((emu->res.u16 & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
            bc = emu->res.u16 | emu->op1.u16;
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_neg32:
            CONDITIONAL_SET_FLAG(emu->op1.u32 != 0, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            bc = emu->res.u32 | emu->op1.u32;
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_neg64:
            CONDITIONAL_SET_FLAG(emu->op1.u64 != 0, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            bc = emu->res.u64 | emu->op1.u64;
            CONDITIONAL_SET_FLAG(XOR2(bc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_shl8:
            if (emu->op2.u8 < 8) {
                cnt = emu->op2.u8 % 8;
                if (cnt > 0) {
                    cc = emu->op1.u8 & (1 << (8 - cnt));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res.u8 & 0xff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
                }
                if (cnt == 1) {
                    CONDITIONAL_SET_FLAG((((emu->res.u8 & 0x80) == 0x80) ^(ACCESS_FLAG(F_CF) != 0)), F_OF);
                } else {
                    CLEAR_FLAG(F_OF);
                }
            } else {
                CONDITIONAL_SET_FLAG((emu->op1.u8 << (emu->op2.u8-1)) & 0x80, F_CF);
                CLEAR_FLAG(F_OF);
                CLEAR_FLAG(F_SF);
                SET_FLAG(F_PF);
                SET_FLAG(F_ZF);
            }
            break;
        case d_shl16:
            if (emu->op2.u16 < 16) {
                cnt = emu->op2.u16 % 16;
                if (cnt > 0) {
                    cc = emu->op1.u16 & (1 << (16 - cnt));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res.u16 & 0xffff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
                }
                if (cnt == 1) {
                    CONDITIONAL_SET_FLAG(((!!(emu->res.u16 & 0x8000)) ^(ACCESS_FLAG(F_CF) != 0)), F_OF);
                } else {
                    CLEAR_FLAG(F_OF);
                }
            } else {
                CONDITIONAL_SET_FLAG((emu->op1.u16 << (emu->op2.u16-1)) & 0x8000, F_CF);
                CLEAR_FLAG(F_OF);
                CLEAR_FLAG(F_SF);
                SET_FLAG(F_PF);
                SET_FLAG(F_ZF);
            }
            break;
        case d_shl32:
            if (emu->op2.u32 < 32) {
                cnt = emu->op2.u32 % 32;
                if (cnt > 0) {
                    cc = emu->op1.u32 & (1 << (32 - cnt));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
                }
                if (cnt == 1) {
                    CONDITIONAL_SET_FLAG(((!!(emu->res.u32 & 0x80000000)) ^
                                            (ACCESS_FLAG(F_CF) != 0)), F_OF);
                } else {
                    CLEAR_FLAG(F_OF);
                }
            } else {
                CONDITIONAL_SET_FLAG((emu->op1.u32 << (emu->op2.u32-1)) & 0x80000000, F_CF);
                CLEAR_FLAG(F_OF);
                CLEAR_FLAG(F_SF);
                SET_FLAG(F_PF);
                SET_FLAG(F_ZF);
            }
            break;
        case d_shl64:
            if (emu->op2.u64 > 0) {
                cc = emu->op1.u64 & (1LL << (64 - emu->op2.u64));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            }
            if (emu->op2.u64 == 1) {
                CONDITIONAL_SET_FLAG(((!!(emu->res.u64 & 0x8000000000000000LL)) ^
                                        (ACCESS_FLAG(F_CF) != 0)), F_OF);
            } else {
                CLEAR_FLAG(F_OF);
            }
            break;
        case d_sar8:
            if (emu->op2.u8 < 8) {
                if(emu->op2.u8) {
                    cc = emu->op1.u8 & (1 << (emu->op2.u8 - 1));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res.u8 & 0xff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
                    CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
                }
            } else {
                if (emu->op1.u8&0x80) {
                    SET_FLAG(F_CF);
                    CLEAR_FLAG(F_ZF);
                    SET_FLAG(F_SF);
                    SET_FLAG(F_PF);
                } else {
                    CLEAR_FLAG(F_CF);
                    SET_FLAG(F_ZF);
                    CLEAR_FLAG(F_SF);
                    SET_FLAG(F_PF);
                }
            }
            break;
        case d_sar16:
            if (emu->op2.u16 < 16) {
                if(emu->op2.u16) {
                    cc = emu->op1.u16 & (1 << (emu->op2.u16 - 1));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res.u16 & 0xffff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
                }
            } else {
                if (emu->op1.u16&0x8000) {
                    SET_FLAG(F_CF);
                    CLEAR_FLAG(F_ZF);
                    SET_FLAG(F_SF);
                    SET_FLAG(F_PF);
                } else {
                    CLEAR_FLAG(F_CF);
                    SET_FLAG(F_ZF);
                    CLEAR_FLAG(F_SF);
                    SET_FLAG(F_PF);
                }
            }
            break;
        case d_sar32:
            if (emu->op2.u32 < 32) {
                if(emu->op2.u32) {
                    cc = emu->op1.u32 & (1 << (emu->op2.u32 - 1));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res.u32 & 0xffffffff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
                }
            } else {
                if (emu->op1.u32&0x80000000) {
                    SET_FLAG(F_CF);
                    CLEAR_FLAG(F_ZF);
                    SET_FLAG(F_SF);
                    SET_FLAG(F_PF);
                } else {
                    CLEAR_FLAG(F_CF);
                    SET_FLAG(F_ZF);
                    CLEAR_FLAG(F_SF);
                    SET_FLAG(F_PF);
                }
            }
            break;
        case d_sar64:
            if(emu->op2.u64) {
                cc = emu->op1.u64 & (1LL << (emu->op2.u64 - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(emu->res.u64 == 0, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            }
            break;
        case d_shr8:
            if (emu->op2.u8 < 8) {
                cnt = emu->op2.u8 % 8;
                if (cnt > 0) {
                    cc = emu->op1.u8 & (1 << (cnt - 1));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res.u8 & 0xff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
                }
                if (cnt == 1) {
                    CONDITIONAL_SET_FLAG(XOR2(emu->res.u8 >> 6), F_OF);
                }
            } else {
                CONDITIONAL_SET_FLAG((emu->op1.u8 >> (emu->op2.u8-1)) & 0x1, F_CF);
                CLEAR_FLAG(F_SF);
                SET_FLAG(F_PF);
                SET_FLAG(F_ZF);
            }
            break;
        case d_shr16:
            if (emu->op2.u16 < 16) {
                cnt = emu->op2.u16 % 16;
                if (cnt > 0) {
                    cc = emu->op1.u16 & (1 << (cnt - 1));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res.u16 & 0xffff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
                }
                if (cnt == 1) {
                    CONDITIONAL_SET_FLAG(XOR2(emu->res.u16 >> 14), F_OF);
                }
            } else {
                CLEAR_FLAG(F_CF);
                SET_FLAG(F_ZF);
                CLEAR_FLAG(F_SF);
                SET_FLAG(F_PF);
            }
            break;
        case d_shr32:
            if (emu->op2.u32 < 32) {
                cnt = emu->op2.u32 % 32;
                if (cnt > 0) {
                    cc = emu->op1.u32 & (1 << (cnt - 1));
                    CONDITIONAL_SET_FLAG(cc, F_CF);
                    CONDITIONAL_SET_FLAG((emu->res.u32 & 0xffffffff) == 0, F_ZF);
                    CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
                    CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
                }
                if (cnt == 1) {
                    CONDITIONAL_SET_FLAG(XOR2(emu->res.u32 >> 30), F_OF);
                }
            } else {
                CLEAR_FLAG(F_CF);
                SET_FLAG(F_ZF);
                CLEAR_FLAG(F_SF);
                SET_FLAG(F_PF);
            }
            break;
        case d_shr64:
            cnt = emu->op2.u64;
            if (cnt > 0) {
                cc = emu->op1.u64 & (1LL << (cnt - 1));
                CONDITIONAL_SET_FLAG(cc, F_CF);
                CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
                CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
                CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            }
            if (cnt == 1) {
                CONDITIONAL_SET_FLAG(XOR2(emu->res.u64 >> 62), F_OF);
            }
            break;
        case d_sub8:
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG((emu->res.u8 & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
            bc = (emu->res.u8 & (~emu->op1.u8 | emu->op2.u8)) | (~emu->op1.u8 & emu->op2.u8);
            CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sub16:
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res.u16 & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
            bc = (emu->res.u16 & (~emu->op1.u16 | emu->op2.u16)) | (~emu->op1.u16 & emu->op2.u16);
            CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sub32:
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            bc = (emu->res.u32 & (~emu->op1.u32 | emu->op2.u32)) | (~emu->op1.u32 & emu->op2.u32);
            CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sub64:
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            bc = (emu->res.u64 & (~emu->op1.u64 | emu->op2.u64)) | (~emu->op1.u64 & emu->op2.u64);
            CONDITIONAL_SET_FLAG(bc & 0x8000000000000000LL, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_xor8:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u8 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            break;
        case d_xor16:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u16 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            break;
        case d_xor32:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u32 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            break;
        case d_xor64:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u64 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            CLEAR_FLAG(F_CF);
            CLEAR_FLAG(F_AF);
            break;
        case d_cmp8:
            CLEAR_FLAG(F_CF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG((emu->res.u8 & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
            bc = (emu->res.u8 & (~emu->op1.u8 | emu->op2.u8)) | (~emu->op1.u8 & emu->op2.u8);
            CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_cmp16:
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res.u16 & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
            bc = (emu->res.u16 & (~emu->op1.u16 | emu->op2.u16)) | (~emu->op1.u16 & emu->op2.u16);
            CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_cmp32:
        	CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
        	CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
        	CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
        	bc = (emu->res.u32 & (~emu->op1.u32 | emu->op2.u32)) | (~emu->op1.u32 & emu->op2.u32);
        	CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
        	CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
        	CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_cmp64:
        	CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
        	CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
        	CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
        	bc = (emu->res.u64 & (~emu->op1.u64 | emu->op2.u64)) | (~emu->op1.u64 & emu->op2.u64);
        	CONDITIONAL_SET_FLAG(bc & 0x8000000000000000LL, F_CF);
        	CONDITIONAL_SET_FLAG(XOR2(bc >> 62), F_OF);
        	CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_tst8:
        	CLEAR_FLAG(F_OF);
        	CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
        	CONDITIONAL_SET_FLAG(emu->res.u8 == 0, F_ZF);
        	CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
        	CLEAR_FLAG(F_CF);
            break;
        case d_tst16:
            CLEAR_FLAG(F_OF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(emu->res.u16 == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
            CLEAR_FLAG(F_CF);
            break;
        case d_tst32:
        	CLEAR_FLAG(F_OF);
        	CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
        	CONDITIONAL_SET_FLAG(emu->res.u32 == 0, F_ZF);
        	CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
        	CLEAR_FLAG(F_CF);
            break;
        case d_tst64:
        	CLEAR_FLAG(F_OF);
        	CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
        	CONDITIONAL_SET_FLAG(emu->res.u64 == 0, F_ZF);
        	CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
        	CLEAR_FLAG(F_CF);
            break;
        case d_adc8:
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x100, F_CF);
            CONDITIONAL_SET_FLAG((emu->res.u8 & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
            cc = (emu->op1.u8 & emu->op2.u8) | ((~emu->res.u8) & (emu->op1.u8 | emu->op2.u8));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_adc16:
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG((emu->res.u16 & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
            cc = (emu->op1.u16 & emu->op2.u16) | ((~emu->res.u16) & (emu->op1.u16 | emu->op2.u16));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_adc32:
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x100000000L, F_CF);
            CONDITIONAL_SET_FLAG((emu->res.u32 & 0xffffffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            cc = (emu->op1.u32 & emu->op2.u32) | ((~emu->res.u32) & (emu->op1.u32 | emu->op2.u32));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_adc32b:
            if(emu->res.u32 == (emu->op1.u32+emu->op2.u32)) {
                lo = (emu->op1.u32 & 0xFFFF) + (emu->op2.u32 & 0xFFFF);
            } else {
                lo = 1 + (emu->op1.u32 & 0xFFFF) + (emu->op2.u32 & 0xFFFF);
            }
            hi = (lo >> 16) + (emu->op1.u32 >> 16) + (emu->op2.u32 >> 16);
            CONDITIONAL_SET_FLAG(hi & 0x10000, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            cc = (emu->op2.u32 & emu->op1.u32) | ((~emu->res.u32) & (emu->op2.u32 | emu->op1.u32));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_adc64:
            if(emu->res.u64 == (emu->op1.u64+emu->op2.u64)) {
                lo = (emu->op1.u64 & 0xFFFFFFFF) + (emu->op2.u64 & 0xFFFFFFFF);
            } else {
                lo = 1 + (emu->op1.u64 & 0xFFFFFFFF) + (emu->op2.u64 & 0xFFFFFFFF);
            }
            hi = (lo >> 32) + (emu->op1.u64 >> 32) + (emu->op2.u64 >> 32);
            CONDITIONAL_SET_FLAG(hi & 0x1000000000000L, F_CF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            cc = (emu->op2.u64 & emu->op1.u64) | ((~emu->res.u64) & (emu->op2.u64 | emu->op1.u64));
            CONDITIONAL_SET_FLAG(XOR2(cc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(cc & 0x8, F_AF);
            break;
        case d_sbb8:
            CONDITIONAL_SET_FLAG(emu->res.u8 & 0x80, F_SF);
            CONDITIONAL_SET_FLAG((emu->res.u8 & 0xff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u8 & 0xff), F_PF);
            bc = (emu->res.u8 & (~emu->op1.u8 | emu->op2.u8)) | (~emu->op1.u8 & emu->op2.u8);
            CONDITIONAL_SET_FLAG(bc & 0x80, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 6), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sbb16:
            CONDITIONAL_SET_FLAG(emu->res.u16 & 0x8000, F_SF);
            CONDITIONAL_SET_FLAG((emu->res.u16 & 0xffff) == 0, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u16 & 0xff), F_PF);
            bc = (emu->res.u16 & (~emu->op1.u16 | emu->op2.u16)) | (~emu->op1.u16 & emu->op2.u16);
            CONDITIONAL_SET_FLAG(bc & 0x8000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 14), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sbb32:
            CONDITIONAL_SET_FLAG(emu->res.u32 & 0x80000000, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u32, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u32 & 0xff), F_PF);
            bc = (emu->res.u32 & (~emu->op1.u32 | emu->op2.u32)) | (~emu->op1.u32 & emu->op2.u32);
            CONDITIONAL_SET_FLAG(bc & 0x80000000, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 30), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_sbb64:
            CONDITIONAL_SET_FLAG(emu->res.u64 & 0x8000000000000000LL, F_SF);
            CONDITIONAL_SET_FLAG(!emu->res.u64, F_ZF);
            CONDITIONAL_SET_FLAG(PARITY(emu->res.u64 & 0xff), F_PF);
            bc = (emu->res.u64 & (~emu->op1.u64 | emu->op2.u64)) | (~emu->op1.u64 & emu->op2.u64);
            CONDITIONAL_SET_FLAG(bc & 0x8000000000000000LL, F_CF);
            CONDITIONAL_SET_FLAG(XOR2(bc >> 62), F_OF);
            CONDITIONAL_SET_FLAG(bc & 0x8, F_AF);
            break;
        case d_rol8:
            if(emu->op2.u8 == 1) {
                CONDITIONAL_SET_FLAG((emu->res.u8 + (emu->res.u8 >> 7)) & 1, F_OF);
            }
        	CONDITIONAL_SET_FLAG(emu->res.u16 & 0x1, F_CF);
            break;
        case d_rol16:
            if(emu->op2.u16 == 1) {
                CONDITIONAL_SET_FLAG((emu->res.u16 + (emu->res.u16 >> 15)) & 1, F_OF);
            }
        	CONDITIONAL_SET_FLAG(emu->res.u16 & 0x1, F_CF);
            break;
        case d_rol32:
            if(emu->op2.u32 == 1) {
                CONDITIONAL_SET_FLAG((emu->res.u32 + (emu->res.u32 >> 31)) & 1, F_OF);
            }
        	CONDITIONAL_SET_FLAG(emu->res.u32 & 0x1, F_CF);
            break;
        case d_rol64:
            if(emu->op2.u64 == 1) {
                CONDITIONAL_SET_FLAG((emu->res.u64 + (emu->res.u64 >> 63)) & 1, F_OF);
            }
        	CONDITIONAL_SET_FLAG(emu->res.u64 & 0x1, F_CF);
            break;
        case d_ror8:
            if(emu->op2.u8 == 1) {
                CONDITIONAL_SET_FLAG(XOR2(emu->res.u8 >> 6), F_OF);
            }
            CONDITIONAL_SET_FLAG(emu->res.u8 & (1 << 7), F_CF);
            break;
        case d_ror16:
            if(emu->op2.u16 == 1) {
                CONDITIONAL_SET_FLAG(XOR2(emu->res.u16 >> 14), F_OF);
            }
            CONDITIONAL_SET_FLAG(emu->res.u16 & (1 << 15), F_CF);
            break;
        case d_ror32:
            if(emu->op2.u32 == 1) {
                CONDITIONAL_SET_FLAG(XOR2(emu->res.u32 >> 30), F_OF);
            }
            CONDITIONAL_SET_FLAG(emu->res.u32 & (1 << 31), F_CF);
            break;
        case d_ror64:
            if(emu->op2.u64 == 1) {
                CONDITIONAL_SET_FLAG(XOR2(emu->res.u64 >> 62), F_OF);
            }
            CONDITIONAL_SET_FLAG(emu->res.u64 & (1L << 63), F_CF);
            break;

        case d_rcl8:
        case d_rcl16:
        case d_rcl32:
        case d_rcl64:
        case d_rcr8:
        case d_rcr16:
        case d_rcr32:
        case d_rcr64:
        case d_unknown:
            printf_log(LOG_NONE, "Box86: %p trying to evaluate Unknown defered Flags\n", (void*)R_RIP);
            break;
    }
    RESET_FLAGS(emu);
}

uintptr_t GetSegmentBaseEmu(x64emu_t* emu, int seg)
{
    if(emu->segs_serial[seg] != emu->context->sel_serial) {
        emu->segs_offs[seg] = (uintptr_t)GetSegmentBase(emu->segs[seg]);
        emu->segs_serial[seg] = emu->context->sel_serial;
    }
    return emu->segs_offs[seg];
}


const char* getAddrFunctionName(uintptr_t addr)
{
    static char ret[1000];
    uint32_t sz = 0;
    uintptr_t start = 0;
    const char* symbname = FindNearestSymbolName(FindElfAddress(my_context, addr), (void*)addr, &start, &sz);
    if(symbname && addr>=start && (addr<(start+sz) || !sz)) {
        if(addr==start)
            sprintf(ret, "%s/%s", ElfName(FindElfAddress(my_context, addr)), symbname);
        else
            sprintf(ret, "%s/%s + %ld", ElfName(FindElfAddress(my_context, addr)), symbname, addr - start);
    } else
        sprintf(ret, "???");
    return ret;
}

void printFunctionAddr(uintptr_t nextaddr, const char* text)
{
    uint32_t sz = 0;
    uintptr_t start = 0;
    const char* symbname = FindNearestSymbolName(FindElfAddress(my_context, nextaddr), (void*)nextaddr, &start, &sz);
    if(symbname && nextaddr>=start && (nextaddr<(start+sz) || !sz)) {
        if(nextaddr==start)
            printf_log(LOG_NONE, " (%s%s/%s)", text, ElfName(FindElfAddress(my_context, nextaddr)), symbname);
        else
            printf_log(LOG_NONE, " (%s%s/%s + %ld)", text, ElfName(FindElfAddress(my_context, nextaddr)), symbname, nextaddr - start);
    }
}

#ifdef HAVE_TRACE
extern uint64_t start_cnt;
#define PK(a)   (*(uint8_t*)(ip+a))
#define PK32(a)   (*(int32_t*)((uint8_t*)(ip+a)))
#define PK64(a)   (*(int64_t*)((uint8_t*)(ip+a)))

void PrintTrace(x64emu_t* emu, uintptr_t ip, int dynarec)
{
    if(start_cnt) --start_cnt;
    if(!start_cnt && my_context->dec && (
            (trace_end == 0) 
            || ((ip >= trace_start) && (ip < trace_end))) ) {
        int tid = syscall(SYS_gettid);
        pthread_mutex_lock(&my_context->mutex_trace);
#ifdef DYNAREC
        if((my_context->trace_tid != tid) || (my_context->trace_dynarec!=dynarec)) {
            printf_log(LOG_NONE, "Thread %04d| (%s) |\n", tid, dynarec?"dyn":"int");
            my_context->trace_tid = tid;
            my_context->trace_dynarec = dynarec;
        }
#else
        if(my_context->trace_tid != tid) {
            printf_log(LOG_NONE, "Thread %04d|\n", tid);
            my_context->trace_tid = tid;
        }
#endif
        printf_log(LOG_NONE, "%s", DumpCPURegs(emu, ip));
        if(PK(0)==0xcc && PK(1)=='S' && PK(2)=='C') {
            uint64_t a = *(uint64_t*)(ip+3);
            if(a==0) {
                printf_log(LOG_NONE, "%p: Exit x86emu\n", (void*)ip);
            } else {
                printf_log(LOG_NONE, "%p: Native call to %p => %s\n", (void*)ip, (void*)a, GetNativeName(*(void**)(ip+11)));
            }
        } else {
            printf_log(LOG_NONE, "%s", DecodeX64Trace(my_context->dec, ip));
            uint8_t peek = PK(0);
            if(peek==0xC3 || peek==0xC2) {
                printf_log(LOG_NONE, " => %p", *(void**)(R_RSP));
                printFunctionAddr(*(uintptr_t*)(R_RSP), "=> ");
            } else if(peek==0x55) {
                printf_log(LOG_NONE, " => STACK_TOP: %p", *(void**)(R_RSP));
                printFunctionAddr(ip, "here: ");
            } else if(peek==0xE8) { // Call
                uintptr_t nextaddr = ip + 5 + PK64(1);
                printFunctionAddr(nextaddr, "=> ");
            } else if(peek==0xFF) {
                if(PK(1)==0x25) {
                    uintptr_t nextaddr = ip + 6 + PK64(2);
                    printFunctionAddr(nextaddr, "=> ");
                }
            }
            printf_log(LOG_NONE, "\n");
        }
        pthread_mutex_unlock(&my_context->mutex_trace);
    }
}

#endif

reg64_t* GetECommon(x64emu_t* emu, rex_t rex, uint8_t m, uint8_t delta)
{
    if (m<=7) {
        if(m==0x4) {
            uint8_t sib = Fetch8(emu);
            uintptr_t base = ((sib&0x7)==5)?((uint64_t)(int64_t)Fetch32s(emu)):(emu->regs[(sib&0x7)+(rex.b<<3)].q[0]); // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sq[0] << (sib>>6));
            return (reg64_t*)base;
        } else if (m==0x5) { //disp32
            int32_t base = Fetch32s(emu);
            return (reg64_t*)(base+R_RIP+delta);
        }
        return (reg64_t*)(emu->regs[m+(rex.b<<3)].q[0]);
    } else {
        uintptr_t base;
        if((m&7)==4) {
            uint8_t sib = Fetch8(emu);
            base = emu->regs[(sib&0x7)+(rex.b<<3)].q[0]; // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sq[0] << (sib>>6));
        } else {
            base = emu->regs[(m&0x7)+(rex.b<<3)].q[0];
        }
        base+=(m&0x80)?Fetch32s(emu):Fetch8s(emu);
        return (reg64_t*)base;
    }
}

reg64_t* GetECommonO(x64emu_t* emu, rex_t rex, uint8_t m, uint8_t delta, uintptr_t base)
{
    if (m<=7) {
        if(m==0x4) {
            uint8_t sib = Fetch8(emu);
            base += ((sib&0x7)==5)?((uint64_t)(int64_t)Fetch32s(emu)):(emu->regs[(sib&0x7)+(rex.b<<3)].q[0]); // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sq[0] << (sib>>6));
            return (reg64_t*)base;
        } else if (m==0x5) { //disp32
            base += Fetch32s(emu);
            return (reg64_t*)(base+R_RIP+delta);
        }
        return (reg64_t*)(base + emu->regs[m+(rex.b<<3)].q[0]);
    } else {
        if((m&7)==4) {
            uint8_t sib = Fetch8(emu);
            base += emu->regs[(sib&0x7)+(rex.b<<3)].q[0]; // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sq[0] << (sib>>6));
        } else {
            base += emu->regs[(m&0x7)+(rex.b<<3)].q[0];
        }
        base+=(m&0x80)?Fetch32s(emu):Fetch8s(emu);
        return (reg64_t*)base;
    }
}

reg64_t* GetECommon32O(x64emu_t* emu, rex_t rex, uint8_t m, uint8_t delta, uintptr_t base)
{
    if (m<=7) {
        if(m==0x4) {
            uint8_t sib = Fetch8(emu);
            base += ((sib&0x7)==5)?((uint64_t)(int64_t)Fetch32s(emu)):(emu->regs[(sib&0x7)+(rex.b<<3)].dword[0]); // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sdword[0] << (sib>>6));
            return (reg64_t*)base;
        } else if (m==0x5) { //disp32
            base += Fetch32s(emu);
            return (reg64_t*)(base+R_RIP+delta);
        }
        return (reg64_t*)(uintptr_t)(base + emu->regs[m+(rex.b<<3)].dword[0]);
    } else {
        if((m&7)==4) {
            uint8_t sib = Fetch8(emu);
            base += emu->regs[(sib&0x7)+(rex.b<<3)].dword[0]; // base
            base += (emu->sbiidx[((sib>>3)&7)+(rex.x<<3)]->sdword[0] << (sib>>6));
        } else {
            base += emu->regs[(m&0x7)+(rex.b<<3)].dword[0];
        }
        base+=(m&0x80)?Fetch32s(emu):Fetch8s(emu);
        return (reg64_t*)base;
    }
}

reg64_t* GetEb(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta)
{
    // rex ignored here
    uint8_t m = v&0xC7;    // filter Eb
    if(m>=0xC0) {
        if(rex.rex) {
            return &emu->regs[(m&0x07)+(rex.b<<3)];
        } else {
            int lowhigh = (m&4)>>2;
            return (reg64_t *)(((char*)(&emu->regs[(m&0x03)]))+lowhigh);  //?
        }
    } else return GetECommon(emu, rex, m, delta);
}

reg64_t* GetEbO(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset)
{
    // rex ignored here
    uint8_t m = v&0xC7;    // filter Eb
    if(m>=0xC0) {
        if(rex.rex) {
            return &emu->regs[(m&0x07)+(rex.b<<3)];
        } else {
            int lowhigh = (m&4)>>2;
            return (reg64_t *)(((char*)(&emu->regs[(m&0x03)]))+lowhigh);  //?
        }
    } else return GetECommonO(emu, rex, m, delta, offset);
}

reg64_t* GetEd(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)+(rex.b<<3)];
    } else return GetECommon(emu, rex, m, delta);
}

reg64_t* GetEdO(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)+(rex.b<<3)];
    } else return GetECommonO(emu, rex, m, delta, offset);
}

reg64_t* GetEd32O(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)+(rex.b<<3)];
    } else return GetECommon32O(emu, rex, m, delta, offset);
}

reg64_t* GetEb32O(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset)
{
    uint8_t m = v&0xC7;    // filter Eb
    if(m>=0xC0) {
        if(rex.rex) {
            return &emu->regs[(m&0x07)+(rex.b<<3)];
        } else {
            int lowhigh = (m&4)>>2;
            return (reg64_t *)(((char*)(&emu->regs[(m&0x03)]))+lowhigh);  //?
        }
    } else return GetECommon32O(emu, rex, m, delta, offset);
}

#define GetEw GetEd

reg64_t* GetEw16(x64emu_t *emu, rex_t rex, uint8_t v)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)];
    } else {
        uintptr_t base = 0;
        switch(m&7) {
            case 0: base = R_BX+R_SI; break;
            case 1: base = R_BX+R_DI; break;
            case 2: base = R_BP+R_SI; break;
            case 3: base = R_BP+R_DI; break;
            case 4: base =      R_SI; break;
            case 5: base =      R_DI; break;
            case 6: base =      R_BP; break;
            case 7: base =      R_BX; break;
        }
        switch((m>>6)&3) {
            case 0: if(m==6) base = Fetch16(emu); break;
            case 1: base += Fetch8s(emu); break;
            case 2: base += Fetch16s(emu); break;
            // case 3 is C0..C7, already dealt with
        }
        return (reg64_t*)base;
    }
}

reg64_t* GetEw16off(x64emu_t *emu, rex_t rex, uint8_t v, uintptr_t offset)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->regs[(m&0x07)];
    } else {
        uint32_t base = 0;
        switch(m&7) {
            case 0: base = R_BX+R_SI; break;
            case 1: base = R_BX+R_DI; break;
            case 2: base = R_BP+R_SI; break;
            case 3: base = R_BP+R_DI; break;
            case 4: base =      R_SI; break;
            case 5: base =      R_DI; break;
            case 6: base =      R_BP; break;
            case 7: base =      R_BX; break;
        }
        switch((m>>6)&3) {
            case 0: if(m==6) base = Fetch16(emu); break;
            case 1: base += Fetch8s(emu); break;
            case 2: base += Fetch16s(emu); break;
            // case 3 is C0..C7, already dealt with
        }
        return (reg64_t*)(base+offset);
    }
}

mmx87_regs_t* GetEm(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->mmx87[m&0x07];
    } else return (mmx87_regs_t*)GetECommon(emu, rex, m, delta);
}

sse_regs_t* GetEx(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->xmm[(m&0x07)+(rex.b<<3)];
    } else return (sse_regs_t*)GetECommon(emu, rex, m, delta);
}

sse_regs_t* GetExO(x64emu_t *emu, rex_t rex, uint8_t v, uint8_t delta, uintptr_t offset)
{
    uint8_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         return &emu->xmm[(m&0x07)+(rex.b<<3)];
    } else return (sse_regs_t*)GetECommonO(emu, rex, m, delta, offset);
}


reg64_t* GetGd(x64emu_t *emu, rex_t rex, uint8_t v)
{
    return &emu->regs[((v&0x38)>>3)+(rex.r<<3)];
}

reg64_t* GetGb(x64emu_t *emu, rex_t rex, uint8_t v)
{
    uint8_t m = (v&0x38)>>3;
    if(rex.rex)
        return &emu->regs[(m&7)+(rex.r<<3)];
    else
        return (reg64_t*)&emu->regs[m&3].byte[m>>2];
}

mmx87_regs_t* GetGm(x64emu_t *emu, rex_t rex, uint8_t v)
{
    uint8_t m = (v&0x38)>>3;
    return &emu->mmx87[m&7];
}

sse_regs_t* GetGx(x64emu_t *emu, rex_t rex, uint8_t v)
{
    uint8_t m = (v&0x38)>>3;
    return &emu->xmm[(m&7)+(rex.r<<3)];
}
