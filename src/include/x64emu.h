#ifndef __X86EMU_H_
#define __X86EMU_H_

typedef struct x64emu_s x64emu_t;
typedef struct box64context_s box64context_t;
typedef struct elfheader_s elfheader_t;

x64emu_t *NewX64Emu(box64context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack);
x64emu_t *NewX64EmuFromStack(x64emu_t* emu, box64context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack);
void SetupX64Emu(x64emu_t *emu, x64emu_t *ref);
void FreeX64Emu(x64emu_t **x64emu);
void FreeX64EmuFromStack(x64emu_t **emu);
void CloneEmu(x64emu_t *newemu, const x64emu_t* emu);
void CopyEmu(x64emu_t *newemu, const x64emu_t* emu);
void SetTraceEmu(uintptr_t trace_start, uintptr_t trace_end);

box64context_t* GetEmuContext(x64emu_t* emu);
uint32_t* GetParityTab(void);

uint32_t GetEAX(x64emu_t *emu);
uint64_t GetRAX(x64emu_t *emu);
void SetEAX(x64emu_t *emu, uint32_t v);
void SetEBX(x64emu_t *emu, uint32_t v);
void SetECX(x64emu_t *emu, uint32_t v);
void SetEDX(x64emu_t *emu, uint32_t v);
void SetEDI(x64emu_t *emu, uint32_t v);
void SetESI(x64emu_t *emu, uint32_t v);
void SetEBP(x64emu_t *emu, uint32_t v);
//void SetESP(x64emu_t *emu, uint32_t v);
void SetRAX(x64emu_t *emu, uint64_t v);
void SetRBX(x64emu_t *emu, uint64_t v);
void SetRCX(x64emu_t *emu, uint64_t v);
void SetRDX(x64emu_t *emu, uint64_t v);
void SetRDI(x64emu_t *emu, uint64_t v);
void SetRSI(x64emu_t *emu, uint64_t v);
void SetRBP(x64emu_t *emu, uint64_t v);
void SetRSP(x64emu_t *emu, uint64_t v);
void SetRIP(x64emu_t *emu, uint64_t v);
//void SetFS(x64emu_t *emu, uint16_t v);
//uint16_t GetFS(x64emu_t *emu);
uint64_t GetRSP(x64emu_t *emu);
uint64_t GetRBP(x64emu_t *emu);
void ResetFlags(x64emu_t *emu);
void ResetSegmentsCache(x64emu_t *emu);
const char* DumpCPURegs(x64emu_t* emu, uintptr_t ip, int is32bits);

void StopEmu(x64emu_t* emu, const char* reason, int is32bits);
void EmuCall(x64emu_t* emu, uintptr_t addr);
void AddCleanup(x64emu_t *emu, void *p);
void AddCleanup1Arg(x64emu_t *emu, void *p, void* a, elfheader_t* h);
void CallCleanup(x64emu_t *emu, elfheader_t* h);
void CallAllCleanup(x64emu_t *emu);
void UnimpOpcode(x64emu_t* emu, int is32bits);

uint64_t ReadTSC(x64emu_t* emu);
uint64_t ReadTSCFrequency(x64emu_t* emu);

double FromLD(void* ld);        // long double (80bits pointer) -> double
long double LD2localLD(void* ld);        // long double (80bits pointer) -> long double (80 or 128bits)
void LD2D(void* ld, void* d);   // long double (80bits) -> double (64bits)
void D2LD(void* d, void* ld);   // double (64bits) -> long double (64bits)

int printFunctionAddr(uintptr_t nextaddr, const char* text); // 0 if nothing was found
const char* getAddrFunctionName(uintptr_t addr);

#endif //__X86EMU_H_