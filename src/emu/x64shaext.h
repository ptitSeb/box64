#ifndef __X64_SHAEXT_H__
#define __X64_SHAEXT_H__

#include <stdint.h>

#include "regs.h"

typedef struct x64emu_s x64emu_t;

void sha1nexte(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2);
void sha1msg1(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2);
void sha1msg2(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2);
void sha256msg1(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2);
void sha256msg2(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2);
void sha1rnds4(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2, uint8_t ib);
void sha256rnds2(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2);

#endif //__X64_SHAEXT_H__