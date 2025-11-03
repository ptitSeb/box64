#ifndef __X86EMU_H_
#define __X86EMU_H_

#include <stdint.h>

typedef struct x64emu_s x64emu_t;
typedef struct box64context_s box64context_t;
typedef struct elfheader_s elfheader_t;

x64emu_t *NewX64Emu(box64context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack);
x64emu_t *NewX64EmuFromStack(x64emu_t* emu, box64context_t *context, uintptr_t start, uintptr_t stack, int stacksize);
void SetupX64Emu(x64emu_t *emu, x64emu_t *ref);
void FreeX64Emu(x64emu_t **x64emu);
void FreeX64EmuFromStack(x64emu_t **emu);
void CloneEmu(x64emu_t *newemu, const x64emu_t* emu);
void CopyEmu(x64emu_t *newemu, const x64emu_t* emu);
void SetTraceEmu(uintptr_t trace_start, uintptr_t trace_end);
void print_wrapper_name(int level, x64emu_t* emu);

box64context_t* GetEmuContext(x64emu_t* emu);

void ResetFlags(x64emu_t *emu);
const char* DumpCPURegs(x64emu_t* emu, uintptr_t ip, int is32bits);

void UnimpOpcode(x64emu_t* emu, int is32bits);

double FromLD(void* ld);        // long double (80bits pointer) -> double
long double LD2localLD(void* ld);        // long double (80bits pointer) -> long double (80 or 128bits)
void LD2D(void* ld, void* d);   // long double (80bits) -> double (64bits)
void D2LD(void* d, void* ld);   // double (64bits) -> long double (64bits)

uintptr_t GetSegmentBaseEmu(x64emu_t* emu, int seg);
void UpdateFlags(x64emu_t* emu);

#endif //__X86EMU_H_
