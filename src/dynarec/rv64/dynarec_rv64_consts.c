#include <stdint.h>
#include <string.h>

#include "dynarec_rv64_consts.h"
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
#include "dynarec_rv64_functions.h"
#include "emu/x64shaext.h"
#include "emu/x87emu_private.h"
#include "emu/x64compstrings.h"
#include "x64test.h"
#include "dynarec/dynarec_next.h"
#include "bitutils.h"

#ifndef HAVE_TRACE
void PrintTrace() {}
#endif

uintptr_t getConst(rv64_consts_t which)
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
        case const_native_int3: return (uintptr_t)native_int3;
        case const_native_int: return (uintptr_t)native_int;
        case const_native_div0: return (uintptr_t)native_div0;
        case const_native_clflush: return (uintptr_t)native_clflush;
        case const_native_fprem: return (uintptr_t)native_fprem;
        case const_native_fprem1: return (uintptr_t)native_fprem1;
        case const_native_frstor16: return (uintptr_t)native_frstor16;
        case const_native_fsave16: return (uintptr_t)native_fsave16;
        case const_native_fsave: return (uintptr_t)native_fsave;
        case const_native_aesimc: return (uintptr_t)native_aesimc;
        case const_native_aesd: return (uintptr_t)(cpuext.vector ? rvv_aesd : native_aesd);
        case const_native_aesd_y: return (uintptr_t)native_aesd_y;
        case const_native_aesdlast: return (uintptr_t)(cpuext.vector ? rvv_aesdlast : native_aesdlast);
        case const_native_aesdlast_y: return (uintptr_t)native_aesdlast_y;
        case const_native_aese: return (uintptr_t)(cpuext.vector ? rvv_aese : native_aese);
        case const_native_aese_y: return (uintptr_t)native_aese_y;
        case const_native_aeselast: return (uintptr_t)(cpuext.vector ? rvv_aeselast : native_aeselast);
        case const_native_aeselast_y: return (uintptr_t)native_aeselast_y;
        case const_native_aeskeygenassist: return (uintptr_t)native_aeskeygenassist;
        case const_native_pclmul: return (uintptr_t)native_pclmul;
        case const_native_pclmul_x: return (uintptr_t)native_pclmul_x;
        case const_native_pclmul_y: return (uintptr_t)native_pclmul_y;
        case const_native_f2xm1: return (uintptr_t)native_f2xm1;
        case const_native_fyl2x: return (uintptr_t)native_fyl2x;
        case const_native_fyl2xp1: return (uintptr_t)native_fyl2xp1;
        case const_native_fxtract: return (uintptr_t)native_fxtract;
        case const_native_ftan: return (uintptr_t)native_ftan;
        case const_native_fpatan: return (uintptr_t)native_fpatan;
        case const_native_fcos: return (uintptr_t)native_fcos;
        case const_native_fsin: return (uintptr_t)native_fsin;
        case const_native_fsincos: return (uintptr_t)native_fsincos;
        case const_native_fscale: return (uintptr_t)native_fscale;
        case const_native_fld: return (uintptr_t)native_fld;
        case const_native_fstp: return (uintptr_t)native_fstp;
        case const_native_frstor: return (uintptr_t)native_frstor;
        case const_native_next: return (uintptr_t)native_next;
        case const_int3: return (uintptr_t)EmuInt3;
        case const_x86syscall: return (uintptr_t)EmuX86Syscall;
        case const_x64syscall: return (uintptr_t)EmuX64Syscall;
        case const_rcl8: return (uintptr_t)rcl8;
        case const_rcl16: return (uintptr_t)rcl16;
        case const_rcl32: return (uintptr_t)rcl32;
        case const_rcl64: return (uintptr_t)rcl64;
        case const_rcr8: return (uintptr_t)rcr8;
        case const_rcr16: return (uintptr_t)rcr16;
        case const_rcr32: return (uintptr_t)rcr32;
        case const_rcr64: return (uintptr_t)rcr64;
        case const_rol8: return (uintptr_t)rol8;
        case const_rol16: return (uintptr_t)rol16;
        case const_ror8: return (uintptr_t)ror8;
        case const_ror16: return (uintptr_t)ror16;
        case const_div64: return (uintptr_t)div64;
        case const_div8: return (uintptr_t)div8;
        case const_idiv64: return (uintptr_t)idiv64;
        case const_idiv8: return (uintptr_t)idiv8;
        case const_random32: return (uintptr_t)get_random32;
        case const_random64: return (uintptr_t)get_random64;
        case const_readtsc: return (uintptr_t)ReadTSC;
        case const_helper_getcpu: return (uintptr_t)helper_getcpu;
        case const_cpuid: return (uintptr_t)my_cpuid;
        case const_getsegmentbase: return (uintptr_t)GetSegmentBaseEmu;
        case const_updateflags: return (uintptr_t)UpdateFlags;
        case const_reset_fpu: return (uintptr_t)reset_fpu;
        case const_sha1nexte: return (uintptr_t)sha1nexte;
        case const_sha1msg1: return (uintptr_t)sha1msg1;
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
        case const_epilog_fast: return (uintptr_t)native_epilog_fast;
        case const_jmptbl32: return getJumpTable32();
        case const_jmptbl48: return getJumpTable48();
        case const_jmptbl64: return getJumpTable64();
        case const_context: return (uintptr_t)my_context;
        case const_lead0tab: return (uintptr_t)lead0tab;
        case const_deBruijn64tab: return (uintptr_t)deBruijn64tab;

        case const_last: dynarec_log(LOG_NONE, "Warning, const last used\n");
            return 0;
    }
    dynarec_log(LOG_NONE, "Warning, Unknown const %d used\n", which);
    return 0;
}