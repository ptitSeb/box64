#ifndef __CORE_H
#define __CORE_H

#include "custommem.h"
#include "callback.h"
#include "library.h"

// exec : 0 = lib, 1 = exec
int initialize(int argc, const char **argv, char** env, x64emu_t** emulator, elfheader_t** elfheader, int exec);

int emulate(x64emu_t* emu, elfheader_t* elf_header);

#endif // __CORE_H