// SPDX-License-Identifier: MIT
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
#include "my_cpuid.h"
#include "emu/x87emu_private.h"
#include "emu/x64shaext.h"
#include "bitutils.h"
#include "freq.h"
#include "rv64_printer.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "elfloader.h"
#include "../dynarec_helper.h"

uintptr_t dynarec64_0F(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    switch (PK(0)) {
        case 0x00 ... 0x4f: return dynarec64_0F_0(dyn, addr, ip, ninst, rex, ok, need_epilog);
        case 0x50 ... 0x9f: return dynarec64_0F_1(dyn, addr, ip, ninst, rex, ok, need_epilog);
        case 0xa0 ... 0xaf: return dynarec64_0F_2(dyn, addr, ip, ninst, rex, ok, need_epilog);
        default: return dynarec64_0F_3(dyn, addr, ip, ninst, rex, ok, need_epilog);
    }
}
