#ifndef __BOX64_H
#define __BOX64_H

#include <stdint.h>
#include "debug.h"

EXPORT int Initialize();

// Run a x64 function of a x64 shared library.
EXPORT uintptr_t RunX64Function(const char *x64_libname, const char *funcname, int nargs, ...);

// Build a bridge for an arm64 symbol, so that emulator would know
// this symbol should be called in arm64 world.
EXPORT uintptr_t BuildBridge(uintptr_t arm64_symbol);

#endif