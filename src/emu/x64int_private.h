#ifndef __X64INT_PRIVATE_H_
#define __X64INT_PRIVATE_H_

void x64Syscall(x64emu_t *emu);
void x64Syscall_linux(x64emu_t *emu);
void x64Int3(x64emu_t* emu, uintptr_t* addr);
x64emu_t* x64emu_fork(x64emu_t* e, int forktype);
void x86Syscall(x64emu_t *emu); //32bits syscall
void x86Int3(x64emu_t* emu, uintptr_t* addr);

#endif // __X64INT_PRIVATE_H_
