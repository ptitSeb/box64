#ifndef __BOX64_STACK_H_
#define __BOX64_STACK_H_

#include <stdint.h>

typedef struct box64context_s box64context_t;
typedef struct x64emu_s x64emu_t;

int CalcStackSize(box64context_t *context);
void SetupInitialStack(x64emu_t *emu);

#endif //__BOX64_STACK_H_
