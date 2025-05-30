#ifndef __EMIT_SIGNALS_H_
#define __EMIT_SIGNALS_H_

#include <stdint.h>

typedef struct x64emu_s x64emu_t;

void EmitSignal(x64emu_t* emu, int sig, void* addr, int code);
void EmitInterruption(x64emu_t* emu, int num, void* addr);
void EmitWineInt(x64emu_t* emu, int num, void* addr);
void EmitDiv0(x64emu_t* emu, void* addr, int code);
void CheckExec(x64emu_t* emu, uintptr_t addr);

#endif // __EMIT_SIGNALS_H_