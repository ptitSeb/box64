#ifndef __CLEANUP_H_
#define __CLEANUP_H_

#include "elfloader.h"

typedef struct x64emu_s x64emu_t;

void AddCleanup(x64emu_t *emu, void *p);
void AddCleanup1Arg(x64emu_t *emu, void *p, void* a, elfheader_t* h);
void AddQuickCleanup(x64emu_t *emu, void *p);
void CallCleanup(x64emu_t *emu, elfheader_t* h);
void CallAllCleanup(x64emu_t *emu);
void CallQuickCleanup(x64emu_t *emu, int status);

#endif // __CLEANUP_H_
