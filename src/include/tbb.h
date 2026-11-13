// SPDX-License-Identifier: MIT
#ifndef __TBB_H__
#define __TBB_H__

#include <stdint.h>

typedef struct elfheader_s elfheader_t;

// Scan one loaded ELF for oneTBB and add the TSO ranges it needs.
// Returns the number of ranges added.
int TBBTSOAddElf(elfheader_t* h);

// Non-zero if ip belongs to a detected oneTBB code range that needs x86 TSO
// (strongmem=4) emulation.
int TBBTSORangesContains(uintptr_t ip);

#endif
