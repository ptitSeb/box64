#ifndef __BOX64CPU_H_
#define __BOX64CPU_H_

typedef struct x64emu_s x64emu_t;

int Run(x64emu_t *emu, int step);
void EmuCall(x64emu_t* emu, uintptr_t addr);
void StopEmu(x64emu_t* emu, const char* reason, int is32bits);
void DynaRun(x64emu_t *emu);
void DynaCall(x64emu_t* emu, uintptr_t addr);

#endif // __BOX64CPU_H_
