#ifndef __DYNAREC_LA464_H_
#define __DYNAREC_LA464_H_

typedef struct dynablock_s dynablock_t;
typedef struct x64emu_s x64emu_t;

void CancelBlock64(void);
void* FillBlock64(dynablock_t* block, uintptr_t addr);

#endif //__DYNAREC_LA464_H_