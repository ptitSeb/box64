#include <stdint.h>
#include <string.h>

#include "dynarec_arm64_consts.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "emu/x87emu_private.h"
#include "emu/x64primop.h"
#include "my_cpuid.h"
#include "freq.h"
#include "debug.h"
#include "custommem.h"
#include "dynarec_arm64_functions.h"
#include "emu/x64shaext.h"
#include "emu/x87emu_private.h"
#include "emu/x64compstrings.h"
#include "x64test.h"
#include "dynarec/dynarec_next.h"
#include "random.h"

static const int8_t mask_shift8[] = { -7, -6, -5, -4, -3, -2, -1, 0 };
static const int8_t mask_string8[] = { 7, 6, 5, 4, 3, 2, 1, 0 };
static const int8_t mask_string16[] = { 15, 14, 13, 12, 11, 10, 9, 8 };
static const float addsubps[4] = {-1.f, 1.f, -1.f, 1.f};
static const double addsubpd[2] = {-1., 1.};
static const float subaddps[4] = {1.f, -1.f, 1.f, -1.f};
static const double subaddpd[2] = {1., -1.};

#ifndef HAVE_TRACE
void PrintTrace() {}
#endif

uintptr_t getConst(arm64_consts_t which)
{
    switch(which) {
        case const_none: dynarec_log(LOG_NONE, "Warning, const none used\n");
            return 0;
        case const_daa8: return (uintptr_t)daa8;
        case const_das8: return (uintptr_t)das8;
        case const_aaa16: return (uintptr_t)aaa16;
        case const_aas16: return (uintptr_t)aas16;
        case const_aam16: return (uintptr_t)aam16;
        case const_aad16: return (uintptr_t)aad16;
        case const_native_br: return (uintptr_t)native_br;
        case const_native_ud: return (uintptr_t)native_ud;
        case const_native_priv: return (uintptr_t)native_priv;
        case const_native_gpf: return (uintptr_t)native_gpf;
        case const_native_int3: return (uintptr_t)native_int3;
        case const_native_int: return (uintptr_t)native_int;
        case const_native_div0: return (uintptr_t)native_div0;
        case const_native_frstor16: return (uintptr_t)native_frstor16;
        case const_native_fsave16: return (uintptr_t)native_fsave16;
        case const_native_fsave: return (uintptr_t)native_fsave;
        case const_native_aesimc: return (uintptr_t)native_aesimc;
        case const_native_aesd: return (uintptr_t)native_aesd;
        case const_native_aesd_y: return (uintptr_t)native_aesd_y;
        case const_native_aesdlast: return (uintptr_t)native_aesdlast;
        case const_native_aesdlast_y: return (uintptr_t)native_aesdlast_y;
        case const_native_aese: return (uintptr_t)native_aese;
        case const_native_aese_y: return (uintptr_t)native_aese_y;
        case const_native_aeselast: return (uintptr_t)native_aeselast;
        case const_native_aeselast_y: return (uintptr_t)native_aeselast_y;
        case const_native_aeskeygenassist: return (uintptr_t)native_aeskeygenassist;
        case const_native_pclmul: return (uintptr_t)native_pclmul;
        case const_native_pclmul_x: return (uintptr_t)native_pclmul_x;
        case const_native_pclmul_y: return (uintptr_t)native_pclmul_y;
        case const_direct_f2xm1: return (uintptr_t)direct_f2xm1;
        case const_direct_fyl2x: return (uintptr_t)direct_fyl2x;
        case const_direct_fyl2xp1: return (uintptr_t)direct_fyl2xp1;
        case const_native_fxtract: return (uintptr_t)native_fxtract;
        case const_direct_ftan: return (uintptr_t)direct_ftan;
        case const_direct_fpatan: return (uintptr_t)direct_fpatan;
        case const_direct_fcos: return (uintptr_t)direct_fcos;
        case const_direct_fsin: return (uintptr_t)direct_fsin;
        case const_native_fsincos: return (uintptr_t)native_fsincos;
        case const_direct_fscale: return (uintptr_t)direct_fscale;
        case const_native_fld: return (uintptr_t)native_fld;
        case const_native_fstp: return (uintptr_t)native_fstp;
        case const_native_frstor: return (uintptr_t)native_frstor;
        case const_native_next: return (uintptr_t)native_next;
        case const_int3: return (uintptr_t)EmuInt3;
        case const_x86syscall: return (uintptr_t)EmuX86Syscall;
        case const_x64syscall: return (uintptr_t)EmuX64Syscall;
        case const_x64syscall_linux: return (uintptr_t)EmuX64Syscall_linux;
        case const_rcl16: return (uintptr_t)rcl16;
        case const_rcl32: return (uintptr_t)rcl32;
        case const_rcl64: return (uintptr_t)rcl64;
        case const_rcr16: return (uintptr_t)rcr16;
        case const_rcr32: return (uintptr_t)rcr32;
        case const_rcr64: return (uintptr_t)rcr64;
        case const_div64: return (uintptr_t)div64;
        case const_idiv64: return (uintptr_t)idiv64;
        case const_random32: return (uintptr_t)get_random32;
        case const_random64: return (uintptr_t)get_random64;
        case const_readtsc: return (uintptr_t)ReadTSC;
        case const_helper_getcpu: return (uintptr_t)helper_getcpu;
        case const_cpuid: return (uintptr_t)my_cpuid;
        case const_getsegmentbase: return (uintptr_t)GetSegmentBaseEmu;
        case const_updateflags_arm64: return (uintptr_t)create_updateflags();
        case const_reset_fpu: return (uintptr_t)reset_fpu;
        case const_sha1msg2: return (uintptr_t)sha1msg2;
        case const_sha1rnds4: return (uintptr_t)sha1rnds4;
        case const_sha256msg1: return (uintptr_t)sha256msg1;
        case const_sha256msg2: return (uintptr_t)sha256msg2;
        case const_sha256rnds2: return (uintptr_t)sha256rnds2;
        case const_fpu_loadenv: return (uintptr_t)fpu_loadenv;
        case const_fpu_savenv: return (uintptr_t)fpu_savenv;
        case const_fpu_fxsave32: return (uintptr_t)fpu_fxsave32;
        case const_fpu_fxsave64: return (uintptr_t)fpu_fxsave64;
        case const_fpu_fxrstor32: return (uintptr_t)fpu_fxrstor32;
        case const_fpu_fxrstor64: return (uintptr_t)fpu_fxrstor64;
        case const_fpu_xsave: return (uintptr_t)fpu_xsave;
        case const_fpu_xrstor: return (uintptr_t)fpu_xrstor;
        case const_fpu_fbld: return (uintptr_t)fpu_fbld;
        case const_fpu_fbst: return (uintptr_t)fpu_fbst;
        case const_sse42_compare_string_explicit_len: return (uintptr_t)sse42_compare_string_explicit_len;
        case const_sse42_compare_string_implicit_len: return (uintptr_t)sse42_compare_string_implicit_len;
        case const_x64test_step: return (uintptr_t)x64test_step;
        case const_printtrace: return (uintptr_t)PrintTrace;
        case const_epilog: return (uintptr_t)native_epilog;
        case const_jmptbl32: return getJumpTable32();
        case const_jmptbl48: return getJumpTable48();
        case const_jmptbl64: return getJumpTable64();
        case const_context: return (uintptr_t)my_context;
        case const_8b_m7_m6_m5_m4_m3_m2_m1_0: return (uintptr_t)&mask_shift8;
        case const_8b_7_6_5_4_3_2_1_0: return (uintptr_t)&mask_string8;
        case const_8b_15_14_13_12_11_10_9_8: return (uintptr_t)&mask_string16;
        case const_4f_m1_1_m1_1: return (uintptr_t)&addsubps;
        case const_4f_1_m1_1_m1: return (uintptr_t)&subaddps;
        case const_2d_m1_1: return (uintptr_t)&addsubpd;
        case const_2d_1_m1: return (uintptr_t)&subaddpd;

        case const_last: dynarec_log(LOG_NONE, "Warning, const last used\n");
            return 0;
    }
    dynarec_log(LOG_NONE, "Warning, Unknown const %d used\n", which);
    return 0;
}