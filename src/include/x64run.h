#ifndef __X64RUN_H_
#define __X64RUN_H_
#include <stdint.h>

typedef struct x64emu_s x64emu_t;
//int Run(x64emu_t *emu, int step); // 0 if run was successfull, 1 if error in x86 world
//int DynaRun(x64emu_t *emu);

//uint32_t LibSyscall(x64emu_t *emu);
//void PltResolver(x64emu_t* emu);
//extern uintptr_t pltResolver;
int GetTID();

#endif //__X64RUN_H_