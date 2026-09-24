// SPDX-License-Identifier: MIT
#ifndef BOX64EC_PRIVATE_H
#define BOX64EC_PRIVATE_H

#include "box64ec.h"
#include "box64context.h"

typedef struct {
    ULONG doorbell;
} box64ec_thr_t;

extern box64context_t Box64EC_Context;

#endif
