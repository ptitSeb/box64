#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>

#include "sigtools.h"
#include "box64context.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "../dynarec/dynablock_private.h"
#include "dynarec_native.h"
#include "dynarec/dynarec_arch.h"
#endif
#include "debug.h"
#include "threads.h"
#include "signal_private.h"
#include "custommem.h"
#include "khash.h"

KHASH_SET_INIT_INT64(unaligned)
static kh_unaligned_t    *unaligned = NULL;
static kh_unaligned_t    *autosmc = NULL;

void add_unaligned_address(uintptr_t addr)
{
    if(!unaligned)
        unaligned = kh_init(unaligned);
    khint_t k;
    int ret;
    k = kh_put(unaligned, unaligned, addr, &ret);    // just add
}

int is_addr_unaligned(uintptr_t addr)
{
    if(!unaligned)
        return 0;
    khint_t k = kh_get(unaligned, unaligned, addr);
    return (k==kh_end(unaligned))?0:1;
}

void add_autosmc_address(uintptr_t addr)
{
    if(!autosmc)
        autosmc = kh_init(unaligned);
    khint_t k;
    int ret;
    k = kh_put(unaligned, autosmc, addr, &ret);    // just add
}

int is_addr_autosmc(uintptr_t addr)
{
    if(!autosmc)
        return 0;
    khint_t k = kh_get(unaligned, autosmc, addr);
    return (k==kh_end(autosmc))?0:1;
}

#ifdef DYNAREC
int nUnalignedRange(uintptr_t start, size_t size)
{
    if(!unaligned)
        return 0;
    int n = 0;
    uintptr_t end = start + size -1;
    uintptr_t addr;
    kh_foreach_key(unaligned, addr,
        if(addr>=start && addr<=end)
            ++n;
    );
    return n;
}
void getUnalignedRange(uintptr_t start, size_t size, uintptr_t addrs[])
{
    if(!unaligned)
        return;
    int n = 0;
    uintptr_t end = start + size -1;
    uintptr_t addr;
    kh_foreach_key(unaligned, addr,
        if(addr>=start && addr<=end)
            addrs[n++] = addr;
    );
}

int mark_db_unaligned(dynablock_t* db, uintptr_t x64pc)
{
    add_unaligned_address(x64pc);
    db->hash++; // dirty the block
    MarkDynablock(db);      // and mark it
if(BOX64ENV(showsegv)) printf_log(LOG_INFO, "Marked db %p as dirty, and address %p as needing unaligned handling\n", db, (void*)x64pc);
    return 2;   // marked, exit handling...
}

int mark_db_autosmc(dynablock_t* db, uintptr_t x64pc)
{
    add_autosmc_address(x64pc);
    db->hash++; // dirty the block
    MarkDynablock(db);      // and mark it
if(BOX64ENV(showsegv)) printf_log(LOG_INFO, "Marked db %p as dirty, and address %p as creating internal SMC\n", db, (void*)x64pc);
    return 2;   // marked, exit handling...
}
#endif //DYNAREC

#ifdef DYNAREC
#ifdef ARM64
#include "dynarec/arm64/arm64_printer.h"
#elif RV64
#include "dynarec/rv64/rv64_printer.h"
#endif
#endif
int sigbus_specialcases(siginfo_t* info, void * ucntx, void* pc, void* _fpsimd, dynablock_t* db, uintptr_t x64pc, int is32bits)
{
    if((uintptr_t)pc<0x10000)
        return 0;
#ifdef DYNAREC
    if(ARCH_UNALIGNED(db, x64pc))
        /*return*/ mark_db_unaligned(db, x64pc);    // don't force an exit for now
#endif
#ifdef ARM64
    ucontext_t *p = (ucontext_t *)ucntx;
    uint32_t opcode = *(uint32_t*)pc;
    struct fpsimd_context *fpsimd = (struct fpsimd_context *)_fpsimd;
    //printf_log(LOG_INFO, "Checking SIGBUS special cases with pc=%p, opcode=%x, fpsimd=%p\n", pc, opcode, fpsimd);
    if((opcode&0b10111111110000000000000000000000)==0b10111001000000000000000000000000) {
        // this is STR
        int scale = (opcode>>30)&3;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        uint64_t offset = (opcode>>10)&0b111111111111;
        offset<<=scale;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        uint64_t value = p->uc_mcontext.regs[val];
        if(scale==3 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<2; ++i)
                ((volatile uint32_t*)addr)[i] = (value>>(i*32))&0xffffffff;
        } else
            for(int i=0; i<(1<<scale); ++i)
                addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b10111111111000000000110000000000) == 0b10111000000000000000000000000000) {
        // this is a STUR that SIGBUS if accessing unaligned device memory
        int size = 1<<((opcode>>30)&3);
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        int64_t offset = (opcode>>12)&0b111111111;
        if((offset>>(9-1))&1)
            offset |= (0xffffffffffffffffll<<9);
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        uint64_t value = p->uc_mcontext.regs[val];
        if(size==8 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<2; ++i)
                ((volatile uint32_t*)addr)[i] = (value>>(i*32))&0xffffffff;
        } else
            for(int i=0; i<size; ++i)
                addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b00111111010000000000000000000000)==0b00111101000000000000000000000000) {
        // this is VSTR
        int scale = (opcode>>30)&3;
        if((opcode>>23)&1)
            scale+=4;
        if(scale>4)
            return 0;
        if(!fpsimd)
            return 0;
        uint64_t offset = (opcode>>10)&0b111111111111;
        offset<<=scale;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        __uint128_t value = fpsimd->vregs[val];
        if(scale>2 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<(1<<(scale-2)); ++i)
                ((volatile uint32_t*)addr)[i] = (value>>(i*32))&0xffffffff;
        } else
            for(int i=0; i<(1<<scale); ++i)
                addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b00111111011000000000110000000000)==0b00111100000000000000000000000000) {
        // this is VSTRU
        int scale = (opcode>>30)&3;
        if((opcode>>23)&1)
            scale+=4;
        if(scale>4)
            return 0;
        if(!fpsimd)
            return 0;
        int64_t offset = (opcode>>12)&0b111111111;
        if((offset>>(9-1))&1)
            offset |= (0xffffffffffffffffll<<9);
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        __uint128_t value = fpsimd->vregs[val];
        if(scale>2 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<(1<<(scale-2)); ++i)
                ((volatile uint32_t*)addr)[i] = (value>>(i*32))&0xffffffff;
        } else
            for(int i=0; i<(1<<scale); ++i)
                addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b00111111010000000000000000000000)==0b00111101010000000000000000000000) {
        // this is VLDR
        int scale = (opcode>>30)&3;
        if((opcode>>23)&1)
            scale+=4;
        if(scale>4)
            return 0;
        if(!fpsimd)
            return 0;
        uint64_t offset = (opcode>>10)&0b111111111111;
        offset<<=scale;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        __uint128_t value = 0;
        if(scale>2 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<(1<<(scale-2)); ++i)
                value |= ((__uint128_t)(((volatile uint32_t*)addr)[i]))<<(i*32);
        } else
            for(int i=0; i<(1<<scale); ++i)
                value |= ((__uint128_t)addr[i])<<(i*8);
        fpsimd->vregs[val] = value;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b00111111011000000000110000000000)==0b00111100010000000000000000000000) {
        // this is VLDRU
        int scale = (opcode>>30)&3;
        if((opcode>>23)&1)
            scale+=4;
        if(scale>4)
            return 0;
        if(!fpsimd)
            return 0;
        int64_t offset = (opcode>>12)&0b111111111;
        if((offset>>(9-1))&1)
            offset |= (0xffffffffffffffffll<<9);
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        __uint128_t value = 0;
        if(scale>2 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<(1<<(scale-2)); ++i)
                value |= ((__uint128_t)(((volatile  uint32_t*)addr)[i]))<<(i*32);
        } else
            for(int i=0; i<(1<<scale); ++i)
                value |= ((__uint128_t)addr[i])<<(i*8);
        fpsimd->vregs[val] = value;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b10111111110000000000000000000000)==0b10111001010000000000000000000000) {
        // this is LDR
        int scale = (opcode>>30)&3;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        uint64_t offset = (opcode>>10)&0b111111111111;
        offset<<=scale;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        uint64_t value = 0;
        if(scale==3 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<2; ++i)
                value |= ((uint64_t)((volatile  uint32_t*)addr)[i]) << (i*32);
        } else
            for(int i=0; i<(1<<scale); ++i)
                value |= ((uint64_t)addr[i]) << (i*8);
        p->uc_mcontext.regs[val] = value;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b10111111111000000000110000000000) == 0b10111000010000000000000000000000) {
        // this is a LDUR
        int size = 1<<((opcode>>30)&3);
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        int64_t offset = (opcode>>12)&0b111111111;
        if((offset>>(9-1))&1)
            offset |= (0xffffffffffffffffll<<9);
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        uint64_t value = 0;
        if(size==8 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<2; ++i)
                value |= ((uint64_t)((volatile  uint32_t*)addr)[i]) << (i*32);
        } else
            for(int i=0; i<size; ++i)
                value |= ((uint64_t)addr[i]) << (i*8);
        p->uc_mcontext.regs[val] = value;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b11111111110000000000000000000000)==0b01111001000000000000000000000000) {
        // this is STRH
        int scale = (opcode>>30)&3;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        uint64_t offset = (opcode>>10)&0b111111111111;
        offset<<=scale;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        uint64_t value = p->uc_mcontext.regs[val];
        for(int i=0; i<(1<<scale); ++i)
            addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b11111111111000000000110000000000)==0b01111000000000000000000000000000) {
        // this is STURH
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        int64_t offset = (opcode>>12)&0b111111111;
        if((offset>>(9-1))&1)
            offset |= (0xffffffffffffffffll<<9);
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        uint64_t value = p->uc_mcontext.regs[val];
        for(int i=0; i<2; ++i)
            addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b11111111111000000000110000000000)==0b01111000001000000000100000000000) {
        // this is STRH reg, reg
        int scale = (opcode>>30)&3;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        int dest2 = (opcode>>16)&31;
        int option = (opcode>>13)&0b111;
        int S = (opcode>>12)&1;
        if(option!=0b011)
            return 0;   // only LSL is supported
        uint64_t offset = p->uc_mcontext.regs[dest2]<<S;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest] + offset);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        uint64_t value = p->uc_mcontext.regs[val];
        for(int i=0; i<(1<<scale); ++i)
            addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b11111111110000000000000000000000)==0b10101001000000000000000000000000) {
        // This is STP reg1, reg2, [reg3 + off]
        int scale = 2+((opcode>>31)&1);
        int val1 = opcode&31;
        int val2 = (opcode>>10)&31;
        int dest = (opcode>>5)&31;
        int64_t offset = (opcode>>15)&0b1111111;
        if((offset>>(7-1))&1)
            offset |= (0xffffffffffffffffll<<7);
        offset <<= scale;
        uintptr_t addr= p->uc_mcontext.regs[dest] + offset;
        if(is32bits) addr = addr&0xffffffff;
        if((((uintptr_t)addr)&3)==0) {
            ((volatile uint32_t*)addr)[0] = p->uc_mcontext.regs[val1];
            ((volatile uint32_t*)addr)[1] = p->uc_mcontext.regs[val2];
        } else {
            __uint128_t value = ((__uint128_t)p->uc_mcontext.regs[val2])<<64 | p->uc_mcontext.regs[val1];
            for(int i=0; i<(1<<scale); ++i)
                ((volatile uint8_t*)addr)[i] = (value>>(i*8))&0xff;
        }
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b11111111110000000000000000000000)==0b10101101000000000000000000000000) {
        // This is (V)STP qreg1, qreg2, [reg3 + off]
        int scale = 2+((opcode>>30)&3);
        int val1 = opcode&31;
        int val2 = (opcode>>10)&31;
        int dest = (opcode>>5)&31;
        int64_t offset = (opcode>>15)&0b1111111;
        if((offset>>(7-1))&1)
            offset |= (0xffffffffffffffffll<<7);
        offset <<= scale;
        uintptr_t addr= p->uc_mcontext.regs[dest] + offset;
        if(is32bits) addr = addr&0xffffffff;
        if((((uintptr_t)addr)&3)==0) {
            for(int i=0; i<4; ++i)
                ((volatile uint32_t*)addr)[0+i] = (fpsimd->vregs[val1]>>(i*32))&0xffffffff;
            for(int i=0; i<4; ++i)
                ((volatile uint32_t*)addr)[4+i] = (fpsimd->vregs[val2]>>(i*32))&0xffffffff;
        } else {
            for(int i=0; i<16; ++i)
                ((volatile uint8_t*)addr)[i] = (fpsimd->vregs[val1]>>(i*8))&0xff;
            for(int i=0; i<16; ++i)
                ((volatile uint8_t*)addr)[16+i] = (fpsimd->vregs[val2]>>(i*8))&0xff;
        }
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b10111111111111111111110000000000)==0b00001101000000001000010000000000) {
        // this is ST1.D
        int idx = (opcode>>30)&1;
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest]);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        uint64_t value = fpsimd->vregs[val]>>(idx*64);
        if((((uintptr_t)addr)&3)==0) {
            for(int i=0; i<2; ++i)
                ((volatile uint32_t*)addr)[i] = (value>>(i*32))&0xffffffff;
        } else
            for(int i=0; i<8; ++i)
                addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b10111111111000000000110000000000) == 0b10111000010000000000010000000000) {
        // this is a LDR postoffset
        int size = 1<<((opcode>>30)&3);
        int val = opcode&31;
        int dest = (opcode>>5)&31;
        int64_t offset = (opcode>>12)&0b111111111;
        if((offset>>(9-1))&1)
            offset |= (0xffffffffffffffffll<<9);
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[dest]);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        uint64_t value = 0;
        if(size==8 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<2; ++i)
                value |= ((uint64_t)((volatile  uint32_t*)addr)[i]) << (i*32);
        } else
            for(int i=0; i<size; ++i)
                value |= ((uint64_t)addr[i]) << (i*8);
        p->uc_mcontext.regs[val] = value;
        p->uc_mcontext.regs[dest] += offset;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    }
    if((opcode&0b10111111111000000000110000000000) == 0b10111000000000000000010000000000) {
        // this is a STR postoffset
        int size = 1<<((opcode>>30)&3);
        int val = opcode&31;
        int src = (opcode>>5)&31;
        int64_t offset = (opcode>>12)&0b111111111;
        if((offset>>(9-1))&1)
            offset |= (0xffffffffffffffffll<<9);
        volatile uint8_t* addr = (void*)(p->uc_mcontext.regs[src]);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        uint64_t value = p->uc_mcontext.regs[val];
        if(size==8 && (((uintptr_t)addr)&3)==0) {
            for(int i=0; i<2; ++i)
                ((volatile uint32_t*)addr)[i] = (value>>(i*32))&0xffffffff;
        } else
            for(int i=0; i<size; ++i)
                addr[i] = (value>>(i*8))&0xff;
        p->uc_mcontext.regs[src] += offset;
        p->uc_mcontext.pc+=4;   // go to next opcode
        return 1;
    } else {
        printf_log(LOG_INFO, "Unsupported SIGBUS special cases with pc=%p, opcode=%x (%s)\n", pc, opcode, arm64_print(opcode, (uintptr_t)pc));
    }
#elif RV64
#define GET_FIELD(v, high, low) (((v) >> low) & ((1ULL << (high - low + 1)) - 1))
#define SIGN_EXT(val, val_sz) (((int32_t)(val) << (32 - (val_sz))) >> (32 - (val_sz)))

    ucontext_t *p = (ucontext_t *)ucntx;
    uint32_t inst = *(uint32_t*)pc;

    uint32_t funct3 = GET_FIELD(inst, 14, 12);
    uint32_t opcode = GET_FIELD(inst, 6, 0);
    if ((opcode == 0b0100011 || opcode == 0b0100111 /* F */) && (funct3 == 0b010 /* (F)SW */ || funct3 == 0b011 /* (F)SD */ || funct3 == 0b001 /* SH */)) {
        int val = (inst >> 20) & 0x1f;
        int dest = (inst >> 15) & 0x1f;
        int64_t imm = (GET_FIELD(inst, 31, 25) << 5) | (GET_FIELD(inst, 11, 7));
        imm = SIGN_EXT(imm, 12);
        volatile uint8_t *addr = (void *)(p->uc_mcontext.__gregs[dest] + imm);
        if(is32bits) addr = (uint8_t*)(((uintptr_t)addr)&0xffffffff);
        uint64_t value = opcode == 0b0100011 ? p->uc_mcontext.__gregs[val] : p->uc_mcontext.__fpregs.__d.__f[val<<1];
        for(int i = 0; i < (funct3 == 0b010 ? 4 : funct3 == 0b011 ? 8 : 2); ++i) {
            addr[i] = (value >> (i * 8)) & 0xff;
        }
        p->uc_mcontext.__gregs[0] += 4; // pc += 4
        return 1;
    } else {
        printf_log(LOG_NONE, "Unsupported SIGBUS special cases with pc=%p, opcode=%x (%s)\n", pc, inst, rv64_print(inst, (uintptr_t)pc));
    }

#undef GET_FIELD
#undef SIGN_EXT
#endif
    return 0;
#undef CHECK
}

#ifdef USE_CUSTOM_MUTEX
extern uint32_t mutex_prot;
extern uint32_t mutex_blocks;
#else
extern pthread_mutex_t mutex_prot;
extern pthread_mutex_t mutex_blocks;
#endif

// unlock mutex that are locked by current thread (for signal handling). Return a mask of unlock mutex
int unlockMutex()
{
    int ret = 0;
    int i;
    #ifdef USE_CUSTOM_MUTEX
    uint32_t tid = (uint32_t)GetTID();
    #define GO(A, B)                                    \
    i = (native_lock_storeifref2_d(&A, 0, tid) == tid); \
    if (i) {                                            \
        ret |= (1 << B);                                \
    }
    #else
    #define GO(A, B)          \
    i = checkUnlockMutex(&A); \
    if (i) {                  \
        ret |= (1 << B);      \
    }
    #endif

    GO(mutex_blocks, 0)
    GO(mutex_prot, 1)

    GO(my_context->mutex_trace, 7)
    GO(my_context->mutex_dyndump, 8)
    GO(my_context->mutex_tls, 9)
    GO(my_context->mutex_thread, 10)
    GO(my_context->mutex_bridge, 11)
    #ifdef DYNAREC
    #else
    GO(my_context->mutex_lock, 12)
    #endif
    #undef GO

    return ret;
}
int checkMutex(uint32_t mask)
{
    int ret = 0;
    int i;
    #ifdef USE_CUSTOM_MUTEX
    uint32_t tid = (uint32_t)GetTID();
    #define GO(A, B)                                    \
    if(mask&(1<<B) i = (A == tid); else i = 0;          \
    if (i) {                                            \
        ret |= (1 << B);                                \
    }
    #else
    #define GO(A, B)                            \
    i = (mask&(1<<B))?checkNolockMutex(&A):0;   \
    if (i) {                                    \
        ret |= (1 << B);                        \
    }
    #endif

    GO(mutex_blocks, 0)
    GO(mutex_prot, 1)

    GO(my_context->mutex_trace, 7)
    GO(my_context->mutex_dyndump, 8)
    GO(my_context->mutex_tls, 9)
    GO(my_context->mutex_thread, 10)
    GO(my_context->mutex_bridge, 11)
    #ifdef DYNAREC
    #else
    GO(my_context->mutex_lock, 12)
    #endif
    #undef GO

    return ret;
}

void adjustregs(x64emu_t* emu, void* pc)
{
    if(!pc) return;
// tests some special cases
    uint8_t* mem = (uint8_t*)R_RIP;
    rex_t rex = {0};
    int rep = 0;
    int is66 = 0;
    int idx = 0;
    rex.is32bits = (R_CS==0x0023);
    while ((mem[idx]>=0x40 && mem[idx]<=0x4f && !rex.is32bits) || mem[idx]==0xF2 || mem[idx]==0xF3 || mem[idx]==0x66) {
        switch(mem[idx]) {
            case 0x40 ... 0x4f:
                rex.rex = mem[idx];
                break;
            case 0xF2:
            case 0xF3:
                rep = mem[idx]-0xF1;
                break;
            case 0x66:
                is66 = 1;
                break;
        }
        ++idx;
    }
    dynarec_log(LOG_INFO, "Checking opcode: at %p rex=%02hhx is32bits=%d, rep=%d is66=%d %02hhX %02hhX %02hhX %02hhX\n", (void*)R_RIP, rex.rex, rex.is32bits, rep, is66, mem[idx+0], mem[idx+1], mem[idx+2], mem[idx+3]);
#ifdef DYNAREC
#ifdef ARM64
    if(mem[idx+0]==0xA4 || mem[idx+0]==0xA5) {
        uint32_t opcode = *(uint32_t*)pc;
        // (rep) movsX, read done, write not... so opcode is a STR?_S9_postindex(A, B, C) with C to be substracted to RSI
        // xx111000000iiiiiiiii01nnnnnttttt xx = size, iiiiiiiii = signed offset, t = value, n = address
        if((opcode & 0b00111111111000000000110000000000)==0b00111000000000000000010000000000) {
            int offset = (opcode>>12)&0b111111111;
            offset<<=31-9;
            offset>>=31-9;  // sign extend
            dynarec_log(LOG_INFO, "\tAdjusting RSI: %d\n", -offset);
            R_RSI -= offset;
        }
        return;
    }
    if(mem[idx+0]==0x8F && (mem[idx+1]&0xc0)!=0xc0) {
        // POP Ed, issue on write address, restore RSP as in before the pop
        dynarec_log(LOG_INFO, "\tAdjusting RSP: %d\n", -(is66?2:(rex.is32bits?4:8)));
        R_RSP -= is66?2:(rex.is32bits?4:8);
    }
#elif defined(LA64)
#elif defined(RV64)
#else
#error  Unsupported architecture
#endif
#endif
}

void copyUCTXreg2Emu(x64emu_t* emu, ucontext_t* p, uintptr_t ip)
{
#ifdef DYNAREC
    #define GO(R) emu->regs[_##R].q[0] = CONTEXT_REG(p, x##R)
    GO(RAX);
    GO(RCX);
    GO(RDX);
    GO(RBX);
    GO(RSP);
    GO(RBP);
    GO(RSI);
    GO(RDI);
    GO(R8);
    GO(R9);
    GO(R10);
    GO(R11);
    GO(R12);
    GO(R13);
    GO(R14);
    GO(R15);
    #undef GO
    emu->ip.q[0] = ip;
#ifdef LA64
    // TODO: move to ADJUST_ARCH
    if (cpuext.lbt) {
        #ifndef LA64_ABI_1
        struct lbt_context* lbt_ctx = NULL;
        struct sctx_info* info = (struct sctx_info*)p->uc_mcontext.__extcontext;
        while (info->magic && !lbt_ctx) {
            if (info->magic == LBT_CTX_MAGIC)
                lbt_ctx = (struct lbt_context*)((uintptr_t)info + sizeof(struct sctx_info));
            else
                info = (struct sctx_info*)((uintptr_t)info + info->size);
        }

        if (lbt_ctx) {
            uint64_t flags = CONTEXT_REG(p, xFlags) & ~0b100011010101;
            flags = flags | (lbt_ctx->eflags & 0b100011010101);
            emu->eflags.x64 = flags;
        } else {
            printf_log(LOG_NONE, "Are you on a non-LBT kernel? Use BOX64_DYNAREC_NOHOSTEXT=lbt to prevent Box64 from using it.\n");
            emu->eflags.x64 = CONTEXT_REG(p, xFlags);
        }
        #else
        uint64_t flags = CONTEXT_REG(p, xFlags) & ~0b100011010101;
        flags = flags | (*(uint32_t*)(((struct sigcontext*)&p->uc_mcontext)->sc_reserved) & 0b100011010101);
        emu->eflags.x64 = flags;
        #endif
    } else
#endif
        emu->eflags.x64 = CONTEXT_REG(p, xFlags);
#endif
}

void emu2mctx(x64_mcontext_t* ctx, x64emu_t* emu)
{
    ctx->gregs[X64_R8] = R_R8;
    ctx->gregs[X64_R9] = R_R9;
    ctx->gregs[X64_R10] = R_R10;
    ctx->gregs[X64_R11] = R_R11;
    ctx->gregs[X64_R12] = R_R12;
    ctx->gregs[X64_R13] = R_R13;
    ctx->gregs[X64_R14] = R_R14;
    ctx->gregs[X64_R15] = R_R15;
    ctx->gregs[X64_RAX] = R_RAX;
    ctx->gregs[X64_RCX] = R_RCX;
    ctx->gregs[X64_RDX] = R_RDX;
    ctx->gregs[X64_RDI] = R_RDI;
    ctx->gregs[X64_RSI] = R_RSI;
    ctx->gregs[X64_RBP] = R_RBP;
    ctx->gregs[X64_RSP] = R_RSP;
    ctx->gregs[X64_RBX] = R_RBX;
    ctx->gregs[X64_RIP] = R_RIP;
    // flags
    ctx->gregs[X64_EFL] = emu->eflags.x64 | (1<<F_RF);   // with RF set
    // get segments
    ctx->gregs[X64_CSGSFS] = ((uint64_t)(R_CS)) | (((uint64_t)(R_GS))<<16) | (((uint64_t)(R_FS))<<32) | (((uint64_t)(R_SS))<<48);
    if(R_CS==0x23) {
        // trucate regs to 32bits, just in case
        #define GO(R)   ctx->gregs[X64_R##R]&=0xFFFFFFFF
        GO(AX);
        GO(CX);
        GO(DX);
        GO(DI);
        GO(SI);
        GO(BP);
        GO(SP);
        GO(BX);
        GO(IP);
        #undef GO
    }
}

void mctx2emu(x64emu_t* emu, x64_mcontext_t* ctx)
{
    #define GO(R)   emu->regs[_##R].q[0]=ctx->gregs[X64_R##R]
    GO(AX);
    GO(CX);
    GO(DX);
    GO(DI);
    GO(SI);
    GO(BP);
    GO(SP);
    GO(BX);
    #undef GO
    #define GO(R)   emu->regs[_##R].q[0]=ctx->gregs[X64_##R]
    GO(R8);
    GO(R9);
    GO(R10);
    GO(R11);
    GO(R12);
    GO(R13);
    GO(R14);
    GO(R15);
    #undef GO
    emu->ip.q[0]=ctx->gregs[X64_RIP];
    // flags
    emu->eflags.x64=ctx->gregs[X64_EFL];
    // get segments
    uint16_t seg;
    seg = (ctx->gregs[X64_CSGSFS] >> 0)&0xffff;
    #define GO(S) if(emu->segs[_##S]!=seg)  emu->segs[_##S]=seg
    GO(CS);
    seg = (ctx->gregs[X64_CSGSFS] >> 16)&0xffff;
    GO(GS);
    seg = (ctx->gregs[X64_CSGSFS] >> 32)&0xffff;
    GO(FS);
    seg = (ctx->gregs[X64_CSGSFS] >> 48)&0xffff;
    GO(SS);
    #undef GO
}