#include <stdint.h>
#include <string.h>

#include "os.h"
#include "dynarec_private.h"
#include "emu/x64run_private.h"
#include "dynablock_private.h"
#include "dynarec_arch.h"
#include "custommem.h"
#include "debug.h"

#define RELOC_TBL64C        1
#define RELOC_TBL64ADDR     2
#define RELOC_TBL64RETENDBL 3
#define RELOC_CANCELBLOCK   4
#define RELOC_TBL64TBLJMPH  5
#define RELOC_TBL64TBLJMPL  6

typedef union reloc_s {
    uint8_t type;
    struct {
        uint8_t     type;
        uint8_t     C;
        uint16_t    idx;
    } table64c;
    struct {
        uint8_t     type;
        uint8_t     nothing;
        uint16_t    idx;
    } table64addr;
    struct {
        uint8_t     type;
        uint8_t     delta;
        uint16_t    idx;
    } table64retendbl;
    struct {
        uint8_t     type;
        uint8_t     deltah;
        uint16_t    idx;
    } table64jmptblh;
    struct {
        uint32_t    type:8;
        uint32_t    deltal:24;
    } table64jmptbll;
    uint32_t x;
} reloc_t;

void AddRelocTable64Const(dynarec_native_t* dyn, int ninst, native_consts_t C, int pass)
{
    if(!dyn->need_reloc)
        return;
    if(isTable64(dyn, getConst(C)))
        return;
    if(pass<3)  {
        dyn->reloc_size++;
        return;
    }
    // generating the reloc
    reloc_t reloc = {0};
    reloc.table64c.type = RELOC_TBL64C;
    reloc.table64c.C = C;
    reloc.table64c.idx = dyn->table64size;
    dyn->relocs[dyn->reloc_size++] = reloc.x;
}

void AddRelocTable64RetEndBlock(dynarec_native_t* dyn, int ninst, uintptr_t addr, int pass)
{
    if(!dyn->need_reloc)
        return;
    if(isTable64(dyn, getJumpTableAddress64(addr)))
        return; // no need, already handled
    // TODO: check iif addr is in current map
    if(pass<3) {
        dyn->reloc_size++;
        return;
    }
    reloc_t reloc = {0};
    intptr_t delta = addr-(dyn->start + dyn->isize);
    if(delta>255 || delta<0) {
        dynarec_log(LOG_INFO, "Warning Relocation Ret End Block not possible: delta=%lld, aborting block\n", delta);
        dyn->abort = 1;
    }
    reloc.table64retendbl.type = RELOC_TBL64RETENDBL;
    reloc.table64retendbl.delta = delta;
    reloc.table64retendbl.idx = dyn->table64size;
    dyn->relocs[dyn->reloc_size++] = reloc.x;
}

void AddRelocCancelBlock(dynarec_native_t* dyn, int ninst, int pass)
{
    if(!dyn->need_reloc)
        return;
    if(pass<3) {
        dyn->reloc_size++;
        return;
    }
    reloc_t reloc = {0};
    reloc.type = RELOC_CANCELBLOCK;
    dyn->relocs[dyn->reloc_size++] = reloc.x;
}

void AddRelocTable64Addr(dynarec_native_t* dyn, int ninst, uintptr_t addr, int pass)
{
    if(!dyn->need_reloc)
        return;
    if(isTable64(dyn, addr))
        return; // no need, already handled
    // check if addr is in the same mmap area
    // else, use a cancel block instead
    uintptr_t start;
    int ok = 1;
    if(!IsAddrFileMapped(dyn->start, NULL, &start))
        ok = 0;
    else {
        uintptr_t end = start + SizeFileMapped(dyn->start);
        if(addr<start || addr>=end)
            ok = 0;
        if(ok && ((addr-start)>=0x100000000LL))
            ok = 0;
    }
    if(!ok) return AddRelocCancelBlock(dyn, ninst, pass);
    if(pass<3) {
        dyn->reloc_size++;
        return;
    }
    reloc_t reloc = {0};
    reloc.type = RELOC_TBL64ADDR;
    reloc.table64addr.idx = dyn->table64size;
    dyn->relocs[dyn->reloc_size++] = reloc.x;
}


void AddRelocTable64JmpTbl(dynarec_native_t* dyn, int ninst, uintptr_t addr, int pass)
{
    if(!dyn->need_reloc)
        return;
    if(isTable64(dyn, getJumpTableAddress64(addr)))
        return; // no need, already handled
    // check if addr is in the same mmap area
    // else, use a cancel block instead
    uintptr_t start;
    int ok = 1;
    if(!IsAddrFileMapped(dyn->start, NULL, &start))
        ok = 0;
    else {
        uintptr_t end = start + SizeFileMapped(dyn->start);
        if(addr<start || addr>=end)
            ok = 0;
        if(ok && ((addr-start)>=0x100000000LL))
            ok = 0;
    }
    if(!ok) return AddRelocCancelBlock(dyn, ninst, pass);
    if(pass<3) {
        dyn->reloc_size+=2;
        return;
    }
    uintptr_t delta = addr - start;
    reloc_t reloc = {0};
    reloc.type = RELOC_TBL64TBLJMPH;
    reloc.table64jmptblh.idx = dyn->table64size;
    reloc.table64jmptblh.deltah = delta>>24;
    dyn->relocs[dyn->reloc_size++] = reloc.x;
    reloc.type = RELOC_TBL64TBLJMPL;
    reloc.table64jmptbll.deltal = delta&0xffffff;
    dyn->relocs[dyn->reloc_size++] = reloc.x;
}

int ApplyRelocs(dynablock_t* block, intptr_t delta_block, intptr_t delta_map, uintptr_t mapping_start)
{
    if(!block || !block->relocs || !block->relocsize)
        return 0;
    size_t reloc_size = block->relocsize / sizeof(uint32_t);
    reloc_t *relocs = block->relocs;
    uint64_t *table64 = block->table64;
    int idx;

    int i = 0;
    uintptr_t addr;
    dynarec_log(LOG_DEBUG, "Will apply %zd reloc to dynablock starting at %p - %p\n", reloc_size, block->x64_addr, block->x64_addr + block->x64_size);
    while(i<reloc_size) {
        idx = -1;
        switch(relocs[i].type) {
            case RELOC_TBL64C:
                idx = relocs[i].table64c.idx;
                table64[idx] = getConst(relocs[i].table64c.C);
                dynarec_log(LOG_DEBUG, "\tApply Relocs[%d]: TABLE64[%d]=Const:%d\n", i, idx, relocs[i].table64c.C);
                break;
            case RELOC_TBL64ADDR:
                idx = relocs[i].table64addr.idx;
                table64[idx] += delta_map;
                dynarec_log(LOG_DEBUG, "\tApply Relocs[%d]: TABLE64[%d]=Addr in Map, delta=%zd\n", i, idx, delta_map);
                break;
            case RELOC_TBL64RETENDBL:
                idx = relocs[i].table64retendbl.idx;
                addr = (uintptr_t)block->x64_addr + block->x64_size + relocs[i].table64retendbl.delta;
                table64[idx] = getJumpTableAddress64(addr);
                dynarec_log(LOG_DEBUG, "\tApply Relocs[%d]: TABLE64[%d]=JmpTable64(%p)\n", i, idx, (void*)addr);
                break;
            case RELOC_CANCELBLOCK:
                dynarec_log(LOG_DEBUG, "\tApply Relocs[%d]: Cancel Block\n", i);
                block->dirty = 1;
                block->hash = 0;
                return 0;
            case RELOC_TBL64TBLJMPH:
                if(relocs[i+1].type!=RELOC_TBL64TBLJMPL)
                    return -2;  // bad sequence
                idx = relocs[i].table64jmptblh.idx;
                addr = relocs[i].table64jmptblh.deltah;
                addr = mapping_start + relocs[i+1].table64jmptbll.deltal + (addr<<24);
                table64[idx] = getJumpTableAddress64(addr);
                dynarec_log(LOG_DEBUG, "\tApply Relocs[%d,%d]: TABLE64[%d]=JmpTable64(%p)=%p\n", i, i+1, idx, (void*)addr, getJumpTableAddress64(addr));
                break;
            case RELOC_TBL64TBLJMPL:
                break;
            default: 
                dynarec_log(LOG_DEBUG, "\tUnknown Relocs[%d]: %d\n", i, relocs[i].type);
                return -1;
        }
        if(idx!=-1) {
            if(idx>=block->table64size) {
                dynarec_log(LOG_NONE, "Warning: Reloc Table64 idx out of range (%d vs %d)\n", idx, block->table64size);
            }
        }
        ++i;
    }
    return 0;
}

int RelocsHaveCancel(dynablock_t* block)
{
    if(!block->relocs || !block->relocsize)
        return 0;
    size_t reloc_size = block->relocsize/sizeof(uint32_t);
    for(size_t i=0; i<reloc_size; ++i)
        if(((reloc_t*)block->relocs)[i].type == RELOC_CANCELBLOCK)
            return 1;
    return 0;
}

uintptr_t RelocGetNext()
{
    return getConst(const_native_next);
}
