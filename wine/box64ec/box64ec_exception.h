// SPDX-License-Identifier: MIT
#ifndef BOX64EC_EXCEPTION_H
#define BOX64EC_EXCEPTION_H

#include <stddef.h>
#include <stdint.h>
#include <windows.h>

#include "box64ec.h"

typedef struct __attribute__((aligned(16))) {
    ARM64_NT_CONTEXT Context;
    uint64_t Pad[4];
    EXCEPTION_RECORD Rec;
    uint64_t Align;
    uint64_t Sp;
    uint64_t Pc;
    uint64_t Redzone[2];
} KiUserExceptionDispatcherStackLayout;

_Static_assert(sizeof(ARM64_NT_CONTEXT) == 0x390,
               "ARM64_NT_CONTEXT size must be 0x390");
_Static_assert(offsetof(KiUserExceptionDispatcherStackLayout, Rec) == 0x3b0,
               "exception record must be at 0x3b0");
_Static_assert(offsetof(KiUserExceptionDispatcherStackLayout, Sp) == 0x450,
               "saved SP must be at 0x450");
_Static_assert(offsetof(KiUserExceptionDispatcherStackLayout, Pc) == 0x458,
               "saved PC must be at 0x458");
_Static_assert(sizeof(KiUserExceptionDispatcherStackLayout) == 0x470,
               "dispatcher frame must be 0x470 bytes");

#endif
