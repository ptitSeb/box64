#include <stdint.h>
#include <string.h>

#include "os.h"
#include "dynarec_private.h"
#include "emu/x64run_private.h"
#include "dynarec_arch.h"
#include "custommem.h"
#include "debug.h"

#define RELOC_TBL64C        1
#define RELOC_TBL64ADDR     2
#define RELOC_TBL64RETENDBL 3
#define RELOC_CANCELBLOCK   4
#define RELOC_TBL64TBLJMPH  5
#define RELOC_TBL64TBLJMPL  5

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
        dyn->reloc_size+=2;
        return;
    }
    reloc_t reloc = {0};
    reloc.type = RELOC_TBL64ADDR;
    reloc.table64addr.idx = dyn->table64size;
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