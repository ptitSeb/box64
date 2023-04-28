#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include <stdint.h>

typedef struct x64emu_s x64emu_t;

uint64_t RunFunction(box64context_t *context, uintptr_t fnc, int nargs, ...);
uint64_t RunFunctionFmt(box64context_t *context, uintptr_t fnc, const char* fmt, ...);
// save all modified register
uint64_t RunSafeFunction(box64context_t *context, uintptr_t fnc, int nargs, ...);
// use emu state to run function
uint64_t RunFunctionWithEmu(x64emu_t *emu, int QuitOnLongJumpExit, uintptr_t fnc, int nargs, ...);
// using the Windows x64 calling convention
uint64_t RunFunctionWindows(box64context_t *context, uintptr_t fnc, int nargs, ...);


#endif //__CALLBACK_H__