#ifndef __DYNAREC_ARM_H_
#define __DYNAREC_ARM_H_

typedef struct dynablock_s dynablock_t;
typedef struct x64emu_s x64emu_t;
typedef struct instsize_s instsize_t;

void addInst(instsize_t* insts, size_t* size, int x64_size, int native_size);

void CancelBlock64(int need_lock);
void* FillBlock64(dynablock_t* block, uintptr_t addr);

#endif //__DYNAREC_ARM_H_