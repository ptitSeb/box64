#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box64context.h"
#include "dynarec.h"
#include "emu/x64emu_private.h"
#include "tools/bridge_private.h"
#include "x64run.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "emu/x87emu_private.h"
#include "x64trace.h"
#include "signals.h"
#include "dynarec_rv64.h"
#include "dynarec_rv64_private.h"
#include "dynarec_rv64_functions.h"
#include "custommem.h"
#include "bridge.h"
#include "rv64_lock.h"

void fpu_reset_scratch(dynarec_rv64_t* dyn)
{
    //TODO
}

#define F8      *(uint8_t*)(addr++)
#define F32     *(uint32_t*)(addr+=4, addr-4)
#define F32S64  (uint64_t)(int64_t)*(int32_t*)(addr+=4, addr-4)
// Get if ED will have the correct parity. Not emiting anything. Parity is 2 for DWORD or 3 for QWORD
int getedparity(dynarec_rv64_t* dyn, int ninst, uintptr_t addr, uint8_t nextop, int parity, int delta)
{
    (void)dyn; (void)ninst;

    uint32_t tested = (1<<parity)-1;
    if((nextop&0xC0)==0xC0)
        return 0;   // direct register, no parity...
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if((sib&0x7)==5) {
                uint64_t tmp = F32S64;
                if (sib_reg!=4) {
                    // if XXXXXX+reg<<N then check parity of XXXXX and N should be enough
                    return ((tmp&tested)==0 && (sib>>6)>=parity)?1:0;
                } else {
                    // just a constant...
                    return (tmp&tested)?0:1;
                }
            } else {
                if(sib_reg==4 && parity<3)
                    return 0;   // simple [reg]
                // don't try [reg1 + reg2<<N], unless reg1 is ESP
                return ((sib&0x7)==4 && (sib>>6)>=parity)?1:0;
            }
        } else if((nextop&7)==5) {
            uint64_t tmp = F32S64;
            tmp+=addr+delta;
            return (tmp&tested)?0:1;
        } else {
            return 0;
        }
    } else {
        return 0; //Form [reg1 + reg2<<N + XXXXXX]
    }
}

// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_rv64_t* dyn, uintptr_t addr, int ninst, uint8_t nextop) 
{
    (void)dyn; (void)addr; (void)ninst;

    if((nextop&0xC0)==0xC0)
        return addr;
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            if((sib&0x7)==5) {
                addr+=4;
            }
        } else if((nextop&7)==5) {
            addr+=4;
        }
    } else {
        if((nextop&7)==4) {
            ++addr;
        }
        if(nextop&0x80) {
            addr+=4;
        } else {
            ++addr;
        }
    }
    return addr;
}
#undef F8
#undef F32

int isNativeCall(dynarec_rv64_t* dyn, uintptr_t addr, uintptr_t* calladdress, int* retn)
{
    (void)dyn;

#define PK(a)       *(uint8_t*)(addr+a)
#define PK32(a)     *(int32_t*)(addr+a)

    if(!addr || !getProtection(addr))
        return 0;
    if(PK(0)==0xff && PK(1)==0x25) {            // "absolute" jump, maybe the GOT (well, RIP relative in fact)
        uintptr_t a1 = addr + 6 + (PK32(2));    // need to add a check to see if the address is from the GOT !
        addr = (uintptr_t)getAlternate(*(void**)a1);
    }
    if(!addr || !getProtection(addr))
        return 0;
    onebridge_t *b = (onebridge_t*)(addr);
    if(b->CC==0xCC && b->S=='S' && b->C=='C' && b->w!=(wrapper_t)0 && b->f!=(uintptr_t)PltResolver) {
        // found !
        if(retn) *retn = (b->C3==0xC2)?b->N:0;
        if(calladdress) *calladdress = addr+1;
        return 1;
    }
    return 0;
#undef PK32
#undef PK
}

int isPred(dynarec_rv64_t* dyn, int ninst, int pred) {
    for(int i=0; i<dyn->insts[ninst].pred_sz; ++i)
        if(dyn->insts[ninst].pred[i]==pred)
            return pred;
    return -1;
}
int getNominalPred(dynarec_rv64_t* dyn, int ninst) {
    if((ninst<=0) || !dyn->insts[ninst].pred_sz)
        return -1;
    if(isPred(dyn, ninst, ninst-1)!=-1)
        return ninst-1;
    return dyn->insts[ninst].pred[0];
}

uint8_t extract_byte(uint32_t val, void* address){
    int idx = (((uintptr_t)address)&3)*8;
    return (val>>idx)&0xff;
}
uint32_t insert_byte(uint32_t val, uint8_t b, void* address){
    int idx = (((uintptr_t)address)&3)*8;
    val&=~(0xff<<idx);
    val|=(((uint32_t)b)<<idx);
    return val;
}

// will go badly if address is unaligned
uint16_t extract_half(uint32_t val, void* address){
    int idx = (((uintptr_t)address)&3)*8;
    return (val>>idx)&0xffff;
}
uint32_t insert_half(uint32_t val, uint16_t h, void* address){
    int idx = (((uintptr_t)address)&3)*8;
    val&=~(0xffff<<idx);
    val|=(((uint32_t)h)<<idx);
    return val;
}

uint8_t rv64_lock_xchg_b(void* addr, uint8_t val)
{
    uint32_t ret;
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    do {
        ret = *aligned;
    } while(rv64_lock_cas_d(aligned, ret, insert_byte(ret, val, addr)));
    return extract_byte(ret, addr);
}

int rv64_lock_cas_b(void* addr, uint8_t ref, uint8_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    uint32_t tmp = *aligned;
    return rv64_lock_cas_d(aligned, tmp, insert_byte(tmp, val, addr));
}

int rv64_lock_cas_h(void* addr, uint16_t ref, uint16_t val)
{
    uint32_t* aligned = (uint32_t*)(((uintptr_t)addr)&~3);
    uint32_t tmp = *aligned;
    return rv64_lock_cas_d(aligned, tmp, insert_half(tmp, val, addr));
}