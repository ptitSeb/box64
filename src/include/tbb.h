// SPDX-License-Identifier: MIT
#ifndef __TBB_H__
#define __TBB_H__

#include <stdint.h>

typedef struct elfheader_s elfheader_t;

#ifdef _WIN32
static inline int TBBTSOAddElf(elfheader_t* h)
{
    (void)h;
    return 0;
}

static inline void TBBTSOScanMainElf(void)
{
}
#else
// Scan one loaded ELF for oneTBB and register the TSO ranges it needs.
// Returns the number of ranges added.
int TBBTSOAddElf(elfheader_t* h);

// Scan the main executable once for statically linked oneTBB.
void TBBTSOScanMainElf(void);
#endif

#endif
