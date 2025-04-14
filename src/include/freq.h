#ifndef __FREQ_H_
#define __FREQ_H_

#include <stdint.h>

typedef struct x64emu_s x64emu_t;

uint64_t ReadTSC(x64emu_t* emu);
uint64_t ReadTSCFrequency(x64emu_t* emu);

#endif // __FREQ_H_