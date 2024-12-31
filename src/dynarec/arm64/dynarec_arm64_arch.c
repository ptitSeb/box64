#include <stddef.h>
#include <stdio.h>
#include <ucontext.h>

#include "debug.h"
#include "dynablock.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "x64run.h"
#include "emu/x64run_private.h"
#include "dynarec/dynablock_private.h"
#include "dynarec_arm64_arch.h"

size_t get_size_arch(dynarec_arm_t* dyn)
{
    if(!box64_dynarec_nativeflags)
        return 0;
    return dyn->isize*sizeof(arch_flags_t);
}

void populate_arch(dynarec_arm_t* dyn, void* p)
{
    if(!box64_dynarec_nativeflags)
        return;

    arch_flags_t* flags = p;
    for(int i=0; i<dyn->size; ++i) {
        flags[i].defered = dyn->insts[i].f_entry.dfnone==0;
        flags[i].vf = dyn->insts[i].need_nat_flags&NF_VF;
        flags[i].nf = dyn->insts[i].need_nat_flags&NF_SF;
        flags[i].eq = dyn->insts[i].need_nat_flags&NF_EQ;
        flags[i].cf = dyn->insts[i].need_nat_flags&NF_CF;
        flags[i].inv_cf = !dyn->insts[i].normal_carry;
    }
}

int getX64AddressInst(dynablock_t* db, uintptr_t native_addr); // define is signal.c

// NZCV N
#define NZCV_N      31
// NZCV Z
#define NZCV_Z      30
// NZCV C
#define NZCV_C      29
// NZCV V
#define NZCV_V      28

void adjust_arch(dynablock_t* db, x64emu_t* emu, ucontext_t* p, uintptr_t x64pc)
{
    if(!db->arch_size || !db->arch)
        return;
    arch_flags_t* flags = db->arch;
    int ninst = getX64AddressInst(db, x64pc);
printf_log(LOG_INFO, "adjust_arch(...), db=%p, x64pc=%p, nints=%d, flags:%s %c%c%c%c%s\n", db, (void*)x64pc, ninst, flags[ninst-1].defered?"defered":"", flags[ninst-1].vf?'V':' ', flags[ninst-1].nf?'S':' ', flags[ninst-1].eq?'Z':' ', flags[ninst-1].cf?'C':' ', (flags[ninst-1].cf && flags[ninst-1].inv_cf)?"inverted":"");
    if(ninst<0)
        return;
    if(ninst==0) {
        CHECK_FLAGS(emu);
        return;
    }
    if(flags[ninst-1].defered) {
        CHECK_FLAGS(emu);
        //return;
    }
    if(flags[ninst-1].nf) {
        CONDITIONAL_SET_FLAG(p->uc_mcontext.pstate&(1<<NZCV_N), F_SF);
    }
    if(flags[ninst-1].vf) {
        CONDITIONAL_SET_FLAG(p->uc_mcontext.pstate&(1<<NZCV_V), F_OF);
    }
    if(flags[ninst-1].eq) {
        CONDITIONAL_SET_FLAG(p->uc_mcontext.pstate&(1<<NZCV_Z), F_ZF);
    }
    if(flags[ninst-1].cf) {
        if(flags[ninst-1].inv_cf) {
            CONDITIONAL_SET_FLAG((p->uc_mcontext.pstate&(1<<NZCV_C))==0, F_CF);
        } else {
            CONDITIONAL_SET_FLAG(p->uc_mcontext.pstate&(1<<NZCV_C), F_CF);
        }
    }
}