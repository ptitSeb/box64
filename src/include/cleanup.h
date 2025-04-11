#ifndef __CLEANUP_H_
#define __CLEANUP_H_

#include "elfloader.h"

typedef struct x64emu_s x64emu_t;

void AddCleanup(x64emu_t *emu, void *p);
void AddCleanup1Arg(x64emu_t *emu, void *p, void* a, elfheader_t* h);
void CallCleanup(x64emu_t *emu, elfheader_t* h);
void CallAllCleanup(x64emu_t *emu);

#endif // __CLEANUP_H_