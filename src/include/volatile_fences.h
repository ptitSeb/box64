// SPDX-License-Identifier: MIT
#ifndef __VOLATILE_FENCES_H__
#define __VOLATILE_FENCES_H__

#include <stdint.h>

// Central registry for volatile memory-model annotations that make the x86 memory model work on weakly-ordered hosts.
//
// Two primitives are managed here:
//   - ranges: a guest address range with a strongmem level override
//   - points: a guest instruction address that needs an explicit fence
//
// Detectors and parsers register ranges and points, DynaRec queries this module and emits fences where it is told to.

typedef enum volatile_range_e {
    VOLATILE_RANGE_METADATA = 0, // Windows PE volatile metadata: no generic strongmem
    VOLATILE_RANGE_TSO,          // ranges that need full x86 TSO emulation
} volatile_range_t;

// Register a range.
int VolatileFenceAddRange(volatile_range_t kind, uintptr_t start, uintptr_t end);

// Register a metadata fence point.
void VolatileFenceAddMetadataPoint(uintptr_t ip);

// Strongmem level for ip.
int VolatileFenceGetLevel(uintptr_t ip, int default_level);

// Whether ip is a registered metadata fence point inside a metadata range.
int VolatileFenceNeedsFence(uintptr_t ip);

// Whether the Unity workaround needs a fence for this memory access.
int VolatileFenceNeedsUnityFence(uintptr_t ip, int64_t disp);

#endif // __VOLATILE_FENCES_H__
