#ifndef __DYNAREC_H_
#define __DYNAREC_H_

typedef struct x64emu_s x64emu_t;

void DynaCall(x64emu_t* emu, uintptr_t addr); // try to use DynaRec... Fallback to EmuCall if no dynarec available

#endif // __DYNAREC_H_