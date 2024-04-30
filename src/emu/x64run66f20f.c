#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fenv.h>
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
#include "x64compstrings.h"

#ifdef TEST_INTERPRETER
uintptr_t Test66F20F(x64test_t *test, rex_t rex, uintptr_t addr)
#else
uintptr_t Run66F20F(x64emu_t *emu, rex_t rex, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    uint8_t tmp8u;
    int8_t tmp8s;
    int16_t tmp16s;
    uint16_t tmp16u;
    int32_t tmp32s;
    uint32_t tmp32u;
    uint64_t tmp64u;
    int64_t tmp64s, i64[4];
    float tmpf;
    double tmpd;
    #ifndef NOALIGN
    int is_nan;
    #endif
    reg64_t *oped, *opgd;
    sse_regs_t *opex, *opgx, eax1, *opex2;
    mmx87_regs_t *opem, *opgm;

    #ifdef TEST_INTERPRETER
    x64emu_t* emu = test->emu;
    #endif
    opcode = F8;

    switch(opcode) {

        case 0x38:  // SSE 4.x
            opcode = F8;
            switch(opcode) {
            
                case 0xF1:  // CRC32 Gd, Ew
                    nextop = F8;
                    GETEW(0);
                    GETGD;
                    for(int j=0; j<2; ++j) {
                        GD->dword[0] ^=  EW->byte[j];
                        for (int i = 0; i < 8; i++) {
                            if (GD->dword[0] & 1)
                                GD->dword[0] = (GD->dword[0] >> 1) ^ 0x82f63b78;
                            else
                                GD->dword[0] = (GD->dword[0] >> 1);
                        }
                    }
                    GD->dword[1] = 0;
                    break;

                default: 
                    return 0;
            }
        break;

        default:
            return 0;
    }
    return addr;
}
