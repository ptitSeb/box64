// SPDX-License-Identifier: MIT
#ifndef BOX64EC_PRIVATE_H
#define BOX64EC_PRIVATE_H

#include "box64ec.h"
#include "box64context.h"

typedef struct {
    ULONG doorbell;
} box64ec_thr_t;

extern box64context_t Box64EC_Context;

void context_to_emu(ARM64EC_NT_CONTEXT* context, x64emu_t* emu);
uint32_t Box64EC_GetEflags(x64emu_t* emu);
void emu_to_context(x64emu_t* emu, ARM64EC_NT_CONTEXT* context);

#endif
