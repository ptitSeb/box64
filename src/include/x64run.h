#ifndef __X64RUN_H_
#define __X64RUN_H_
#include <stdint.h>

typedef struct x64emu_s x64emu_t;
typedef struct x64test_s x64test_t;
int Run(x64emu_t *emu, int step); // 0 if run was successfull, 1 if error in x86 world
int RunTest(x64test_t *test);
void DynaRun(x64emu_t *emu);

uint32_t LibSyscall(x64emu_t *emu);
void PltResolver64(x64emu_t* emu);
#ifdef BOX32
void PltResolver32(x64emu_t* emu);
#endif
extern uintptr_t pltResolver64;
int GetTID(void);

#endif //__X64RUN_H_