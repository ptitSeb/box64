#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "custommem.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynablock.h"
#include "dynablock_private.h"

#include "dynarec_native.h"
#include "dynarec_arch.h"
#include "dynarec_next.h"
#include "gdbjit.h"
#include "khash.h"

KHASH_MAP_INIT_INT64(table64, uint32_t)

void printf_x64_instruction(dynarec_native_t* dyn, zydis_dec_t* dec, instruction_x64_t* inst, const char* name) {
    uint8_t *ip = (uint8_t*)inst->addr;
    if (ip[0] == 0xcc && IsBridgeSignature(ip[1], ip[2])) {
        uintptr_t a = *(uintptr_t*)(ip+3);
        if(a==0) {
            dynarec_log(LOG_NONE, "%s%p: Exit x64emu%s\n", (dyn->need_dump>1)?"\e[01;33m":"", (void*)ip, (dyn->need_dump>1)?"\e[m":"");
        } else {
            dynarec_log(LOG_NONE, "%s%p: Native call to %p%s\n", (dyn->need_dump>1)?"\e[01;33m":"", (void*)ip, (void*)a, (dyn->need_dump>1)?"\e[m":"");
        }
    } else {
        if(dec) {
            dynarec_log(LOG_NONE, "%s%p: %s", (dyn->need_dump > 1) ? "\e[01;33m" : "", ip, DecodeX64Trace(dec, inst->addr, 1));
        } else {
            dynarec_log(LOG_NONE, "%s%p: ", (dyn->need_dump>1)?"\e[01;33m":"", ip);
            for(int i=0; i<inst->size; ++i) {
                dynarec_log_prefix(0, LOG_NONE, "%02X ", ip[i]);
            }
            dynarec_log_prefix(0, LOG_NONE, " %s", name);
        }
        // print Call function name if possible
        if(ip[0]==0xE8 || ip[0]==0xE9) { // Call / Jmp
            uintptr_t nextaddr = (uintptr_t)ip + 5 + *((int32_t*)(ip+1));
            PrintFunctionAddr(nextaddr, "=> ");
        } else if(ip[0]==0xFF) {
            if(ip[1]==0x25) {
                uintptr_t nextaddr = (uintptr_t)ip + 6 + *((int32_t*)(ip+2));
                PrintFunctionAddr(nextaddr, "=> ");
            }
        }
        // end of line and colors
        dynarec_log_prefix(0, LOG_NONE, "%s\n", (dyn->need_dump>1)?"\e[m":"");
    }
}

void add_next(dynarec_native_t *dyn, uintptr_t addr) {
    if (!BOX64DRENV(dynarec_bigblock))
        return;
    // exist?
    for(int i=0; i<dyn->next_sz; ++i)
        if(dyn->next[i]==addr)
            return;
    // put in a free slot
    for(int i=0; i<dyn->next_sz; ++i)
        if(!dyn->next[i]) {
            dyn->next[i] = addr;
            return;
        }
    // add slots
    if(dyn->next_sz == dyn->next_cap) {
        printf_log(LOG_NONE, "Warning, overallocating next\n");
    }
    dyn->next[dyn->next_sz++] = addr;
}
uintptr_t get_closest_next(dynarec_native_t *dyn, uintptr_t addr) {
    // get closest, but no addresses before
    uintptr_t best = 0;
    int i = 0;
    while((i<dyn->next_sz) && (best!=addr)) {
        if(dyn->next[i]) {
            if(dyn->next[i]<addr) { // remove the address, it's before current address
                dyn->next[i] = 0;
            } else {
                if((dyn->next[i]<best) || !best)
                    best = dyn->next[i];
            }
        }
        ++i;
    }
    return best;
}
void add_jump(dynarec_native_t *dyn, int ninst) {
    // add slots
    if(dyn->jmp_sz == dyn->jmp_cap) {
        printf_log(LOG_NONE, "Warning, overallocating jmps\n");
    }
    dyn->jmps[dyn->jmp_sz++] = ninst;
}
int get_first_jump(dynarec_native_t *dyn, int next) {
    if(next<0 || next>dyn->size)
        return -2;
    return get_first_jump_addr(dyn, dyn->insts[next].x64.addr);
}
int get_first_jump_addr(dynarec_native_t *dyn, uintptr_t next) {
    for(int i=0; i<dyn->jmp_sz; ++i)
        if(dyn->insts[dyn->jmps[i]].x64.jmp == next)
            return dyn->jmps[i];
    return -2;
}

#define PK(A) (*((uint8_t*)(addr+(A))))
int is_nops(dynarec_native_t *dyn, uintptr_t addr, int n)
{
    if(!n)
        return 1;
    if(PK(0)==0x90)
        return is_nops(dyn, addr+1, n-1);
    if(n>1 && PK(0)==0x66)  // if opcode start with 0x66, and there is more after, than is *can* be a NOP
        return is_nops(dyn, addr+1, n-1);
    if(n>1 && PK(0)==0xF3 && PK(1)==0x90)
        return is_nops(dyn, addr+2, n-2);
    if(n>2 && PK(0)==0x0f && PK(1)==0x1f && PK(2)==0x00)
        return is_nops(dyn, addr+3, n-3);
    if(n>2 && PK(0)==0x8d && PK(1)==0x76 && PK(2)==0x00)    // lea esi, [esi]
        return is_nops(dyn, addr+3, n-3);
    if(n>3 && PK(0)==0x0f && PK(1)==0x1f && PK(2)==0x40 && PK(3)==0x00)
        return is_nops(dyn, addr+4, n-4);
    if(n>3 && PK(0)==0x8d && PK(1)==0x74 && PK(2)==0x26 && PK(3)==0x00)
        return is_nops(dyn, addr+4, n-4);
    if(n>4 && PK(0)==0x0f && PK(1)==0x1f && PK(2)==0x44 && PK(3)==0x00 && PK(4)==0x00)
        return is_nops(dyn, addr+5, n-5);
    if(n>5 && PK(0)==0x8d && PK(1)==0xb6 && PK(2)==0x00 && PK(3)==0x00 && PK(4)==0x00 && PK(5)==0x00)
        return is_nops(dyn, addr+6, n-6);
    if(n>6 && PK(0)==0x0f && PK(1)==0x1f && PK(2)==0x80 && PK(3)==0x00 && PK(4)==0x00 && PK(5)==0x00 && PK(6)==0x00)
        return is_nops(dyn, addr+7, n-7);
    if(n>6 && PK(0)==0x8d && PK(1)==0xb4 && PK(2)==0x26 && PK(3)==0x00 && PK(4)==0x00 && PK(5)==0x00 && PK(6)==0x00) // lea esi, [esi+0]
        return is_nops(dyn, addr+7, n-7);
    if(n>7 && PK(0)==0x0f && PK(1)==0x1f && PK(2)==0x84 && PK(3)==0x00 && PK(4)==0x00 && PK(5)==0x00 && PK(6)==0x00 && PK(7)==0x00)
        return is_nops(dyn, addr+8, n-8);
    return 0;
}
#undef PK

void addInst(instsize_t* insts, size_t* size, int x64_size, int native_size)
{
    // x64 instruction is <16 bytes
    int toadd;
    if(x64_size>native_size)
        toadd = 1 + x64_size/15;
    else
        toadd = 1 + native_size/15;
    while(toadd) {
        if(x64_size>15)
            insts[*size].x64 = 15;
        else
            insts[*size].x64 = x64_size;
        x64_size -= insts[*size].x64;
        if(native_size>15)
            insts[*size].nat = 15;
        else
            insts[*size].nat = native_size;
        native_size -= insts[*size].nat;
        ++(*size);
        --toadd;
    }
}

static kh_table64_t* khtable64 = NULL;

int isTable64(dynarec_native_t *dyn, uint64_t val)
{
    if(!khtable64)
        return 0;
    if(kh_get(table64, khtable64, val)==kh_end(khtable64))
        return 0;
    return 1;
}
// add a value to table64 (if needed) and gives back the imm19 to use in LDR_literal
int Table64(dynarec_native_t *dyn, uint64_t val, int pass)
{
    if(!khtable64)
        khtable64 = kh_init(table64);
    // find the value if already present
    khint_t k = kh_get(table64, khtable64, val);
    uint32_t idx = 0;
    if(k!=kh_end(khtable64)) {
        idx = kh_value(khtable64, k);
    } else {
        idx = dyn->table64size++;
        if(pass==3) {
            if(idx < dyn->table64cap)
                dyn->table64[idx] = val;
            else
                printf_log(LOG_NONE, "Warning, table64 bigger than expected %d vs %d\n", idx, dyn->table64cap);
        }
        int ret;
        k = kh_put(table64, khtable64, val, &ret);
        kh_value(khtable64, k) = idx;
    }
    // calculate offset
    int delta = dyn->tablestart + idx*sizeof(uint64_t) - (uintptr_t)dyn->block;
    return delta;
}

void ResetTable64(dynarec_native_t* dyn)
{
    dyn->table64size = 0;
    if(khtable64) {
        kh_clear(table64, khtable64);
    }
}

static void recurse_mark_alive(dynarec_native_t* dyn, int i)
{
    if(dyn->insts[i].x64.alive)
        return;
    dyn->insts[i].x64.alive = 1;
    if(dyn->insts[i].x64.jmp && dyn->insts[i].x64.jmp_insts!=-1)
        recurse_mark_alive(dyn, dyn->insts[i].x64.jmp_insts);
    if(i<dyn->size-1 && dyn->insts[i].x64.has_next)
        recurse_mark_alive(dyn, i+1);
}

static void sizePredecessors(dynarec_native_t* dyn)
{
    // compute total size of predecessor to allocate the array
    // mark alive...
    recurse_mark_alive(dyn, 0);
    // first compute the jumps
    int jmpto;
    for(int i=0; i<dyn->size; ++i) {
        if(dyn->insts[i].x64.alive && dyn->insts[i].x64.jmp && ((jmpto=dyn->insts[i].x64.jmp_insts)!=-1)) {
            dyn->insts[jmpto].pred_sz++;
        }
    }
    // remove "has_next" from orphan branch
    for(int i=0; i<dyn->size-1; ++i) {
        if(dyn->insts[i].x64.has_next && !dyn->insts[i+1].x64.alive)
            dyn->insts[i].x64.has_next = 0;
    }
    // second the "has_next"
    for(int i=0; i<dyn->size-1; ++i) {
        if(dyn->insts[i].x64.has_next) {
            dyn->insts[i+1].pred_sz++;
        }
    }
}
static void fillPredecessors(dynarec_native_t* dyn)
{
    // fill pred pointer
    int* p = dyn->predecessor;
    for(int i=0; i<dyn->size; ++i) {
        dyn->insts[i].pred = p;
        p += dyn->insts[i].pred_sz;
        dyn->insts[i].pred_sz=0;  // reset size, it's reused to actually fill pred[]
    }
    // fill pred
    for(int i=0; i<dyn->size; ++i) if(dyn->insts[i].x64.alive) {
        if((i!=dyn->size-1) && dyn->insts[i].x64.has_next)
            dyn->insts[i+1].pred[dyn->insts[i+1].pred_sz++] = i;
        if(dyn->insts[i].x64.jmp && (dyn->insts[i].x64.jmp_insts!=-1)) {
            int j = dyn->insts[i].x64.jmp_insts;
            dyn->insts[j].pred[dyn->insts[j].pred_sz++] = i;
        }
    }
}

// updateNeed for the current block. recursive function that goes backward
static int updateNeed(dynarec_native_t* dyn, int ninst, uint8_t need) {
    while (ninst>=0) {
        // need pending but instruction is only a subset: remove pend and use an X_ALL instead
        need |= dyn->insts[ninst].x64.need_after;
        if((need&X_PEND) && ((dyn->insts[ninst].x64.state_flags==SF_SUBSET) || (dyn->insts[ninst].x64.state_flags==SF_SET) || (dyn->insts[ninst].x64.state_flags==SF_SET_NODF))) {
            need &=~X_PEND;
            need |= X_ALL;
            STOP_NATIVE_FLAGS(dyn, ninst);
        }
        if((need&X_PEND) && dyn->insts[ninst].x64.state_flags==SF_SUBSET_PENDING) {
            need |= X_ALL&~(dyn->insts[ninst].x64.set_flags);
        }
        dyn->insts[ninst].x64.gen_flags = need&dyn->insts[ninst].x64.set_flags;
        if((need&X_PEND) && (dyn->insts[ninst].x64.state_flags&SF_PENDING))
            dyn->insts[ninst].x64.gen_flags |= X_PEND;
        dyn->insts[ninst].x64.need_after = need;
        need = dyn->insts[ninst].x64.need_after&~dyn->insts[ninst].x64.gen_flags;

        if(dyn->insts[ninst].x64.may_set)
            need |= dyn->insts[ninst].x64.gen_flags;    // forward the flags
        else if((need&X_PEND) && (dyn->insts[ninst].x64.set_flags&SF_PENDING))
            need &=~X_PEND;         // Consume X_PEND if relevant
        need |= dyn->insts[ninst].x64.use_flags;
        if(dyn->insts[ninst].x64.need_before == need)
            return ninst - 1;
        dyn->insts[ninst].x64.need_before = need;
        if(dyn->insts[ninst].x64.barrier&BARRIER_FLAGS) {
            need = need?X_PEND:0;
        }
        int ok = 0;
        for(int i=0; i<dyn->insts[ninst].pred_sz; ++i) {
            if(dyn->insts[ninst].pred[i] == ninst-1)
                ok = 1;
            else
                updateNeed(dyn, dyn->insts[ninst].pred[i], need);
        }
        --ninst;
        if(!ok)
            return ninst;
    }
    return ninst;
}

void* current_helper = NULL;
static int static_jmps[MAX_INSTS+2];
static uintptr_t static_next[MAX_INSTS+2];
static instruction_native_t static_insts[MAX_INSTS+2] = {0};
static callret_t static_callrets[MAX_INSTS+2] = {0};
static int static_preds[MAX_INSTS*2+2]; // for the worst case scenario were all instructions are conditional jumps
void* redundant_helper = NULL;
// TODO: ninst could be a uint16_t instead of an int, that could same some temp. memory

void ClearCache(void* start, size_t len)
{
#if defined(ARM64)
    // manually clear cache, I have issue with regular function on Ampere with kernel 6.12.4
    uintptr_t xstart = (uintptr_t)start;
    uintptr_t xend = (uintptr_t)start + len + 1;
    // Cache Type Info. Only grab the info once
    static uint64_t ctr_el0 = 0;
    if (ctr_el0 == 0)
        __asm __volatile("mrs %0, ctr_el0" : "=r"(ctr_el0));
    const int ctr_el0_idc = (ctr_el0>>28)&1;    // 0: datacache needs to be cleaned too, 1: no need
    const int ctr_el0_dic = (ctr_el0>>29)&1;    // 0: instruction cache needs to be cleaned, 1: no need
    const uintptr_t dcache_line_size = 4 << ((ctr_el0 >> 16) & 15);
    const uintptr_t icache_line_size = 4 << ((ctr_el0 >> 0) & 15);
    if (!ctr_el0_idc) {
        //purge each dcache line if no icache is defined...
        for (uint64_t addr=xstart&~(dcache_line_size-1); addr<xend; addr+=dcache_line_size)
            __asm __volatile("dc cvau, %0" ::"r"(addr));
    }
    __asm __volatile("dsb ish");
    if (!ctr_el0_dic) {
        // purge each icache line
        for (uint64_t addr=xstart&~(icache_line_size-1); addr<xend; addr+=icache_line_size)
            __asm __volatile("ic ivau, %0" ::"r"(addr));
        __asm __volatile("dsb ish");
    }
    __asm __volatile("isb sy");
#else
    __builtin___clear_cache(start, start+len+1);
#endif
}

void CancelBlock64(int need_lock)
{
    if(need_lock)
        mutex_lock(&my_context->mutex_dyndump);
    dynarec_native_t* helper = (dynarec_native_t*)current_helper;
    if(helper) {
        if(helper->dynablock && helper->dynablock->actual_block) {
            FreeDynarecMap((uintptr_t)helper->dynablock->actual_block);
            helper->dynablock->actual_block = NULL;
        }
    }
    current_helper = NULL;
    redundant_helper = NULL;
    if(need_lock)
        mutex_unlock(&my_context->mutex_dyndump);
}

uintptr_t native_pass0(dynarec_native_t* dyn, uintptr_t addr, int alternate, int is32bits, int inst_max);
uintptr_t native_pass1(dynarec_native_t* dyn, uintptr_t addr, int alternate, int is32bits, int inst_max);
uintptr_t native_pass2(dynarec_native_t* dyn, uintptr_t addr, int alternate, int is32bits, int inst_max);
uintptr_t native_pass3(dynarec_native_t* dyn, uintptr_t addr, int alternate, int is32bits, int inst_max);

dynablock_t* CreateEmptyBlock(uintptr_t addr, int is32bits, int is_new) {
    size_t sz = 4*sizeof(void*) + sizeof(dynablock_t);
    void* actual_p = (void*)AllocDynarecMap(addr, sz, is_new);
    void* p = actual_p + sizeof(void*);
    if(actual_p==NULL) {
        dynarec_log(LOG_INFO, "AllocDynarecMap(%p, %zu) failed, canceling block\n", (void*)addr, sz);
        CancelBlock64(0);
        return NULL;
    }
    dynablock_t* block = (dynablock_t*)(actual_p+4*sizeof(void*));
    memset(block, 0, sizeof(dynablock_t));
    // fill the block
    block->x64_addr = (void*)addr;
    block->isize = 0;
    block->done = 0;
    block->size = sz;
    block->actual_block = actual_p;
    block->block = p;
    block->jmpnext = p;
    block->is32bits = is32bits;
    *(dynablock_t**)actual_p = block;
    *(void**)(p+2*sizeof(void*)) = native_epilog;
    CreateJmpNext(block->jmpnext, p+2*sizeof(void*));
    // all done...
    ClearCache(actual_p+sizeof(void*), 3*sizeof(void*));   // need to clear the cache before execution...
    return block;
}

dynablock_t* FillBlock64(uintptr_t addr, int alternate, int is32bits, int inst_max, int is_new) {
    /*
        A Block must have this layout:

        0x0000..0x0007  : dynablock_t* : self
        0x0008..8+4*n   : actual Native instructions, (n is the total number)
        A ..    A+8*n   : Table64: n 64bits values
        B ..    B+7     : dynablock_t* : self (as part of JmpNext, that simulate another block)
        B+8 ..  B+15    : 2 Native code for jmpnext (or jmp epilog in case of empty block)
        B+16 .. B+23    : jmpnext (or jmp_epilog) address. jumpnext is used when the block needs testing
        B+24 .. B+31    : empty (in case an architecture needs more than 2 opcodes)
        B+32 .. B+32+sz : instsize (compressed array with each instruction length on x64 and native side)
        C ..    C+sz    : arch: arch specific info (likes flags info) per inst (can be absent)

    */
    if(addr>=BOX64ENV(nodynarec_start) && addr<BOX64ENV(nodynarec_end)) {
        dynarec_log(LOG_INFO, "Create empty block in no-dynarec zone\n");
        return BOX64ENV(nodynarec_delay)?NULL:CreateEmptyBlock(addr, is32bits, is_new);
    }
    int is_inhotpage = checkInHotPage(addr);
    if(is_inhotpage && !BOX64ENV(dynarec_dirty)) {
        dynarec_log(LOG_DEBUG, "Not creating dynablock at %p as in a HotPage\n", (void*)addr);
        return NULL;
    }
    if(current_helper) {
        if(current_helper==redundant_helper) {
            dynarec_log(LOG_INFO, "%04d|Warning: previous FillBlock did not cleaned up correctly (helper=%p, x64addr=%p, db=%p)\n", GetTID(), current_helper, (void*)((dynarec_native_t*)current_helper)->start, ((dynarec_native_t*)current_helper)->dynablock);
            return NULL;
        }
        dynarec_log(LOG_INFO, "Warning: some static area curruption appeared (current=%p, redundant=%p)\n", current_helper, redundant_helper);
    }
    // protect the 1st page
    protectDB(addr, 1);
    // init the helper
    dynarec_native_t helper = {0};
    dynarec_native_t* dyn = &helper;
#ifdef GDBJIT
    helper.gdbjit_block = box_calloc(1, sizeof(gdbjit_block_t));
#endif
    redundant_helper = current_helper = &helper;
    helper.dynablock = NULL;
    helper.start = addr;
    uintptr_t start = addr;
    helper.cap = MAX_INSTS;
    helper.insts = static_insts;
    helper.jmps = static_jmps;
    helper.jmp_cap = MAX_INSTS;
    helper.next = static_next;
    helper.next_cap = MAX_INSTS;
    helper.table64 = NULL;
    helper.env = GetCurEnvByAddr(addr);
    ResetTable64(&helper);
    helper.table64cap = 0;
    helper.end = addr + SizeFileMapped(addr);
    if(helper.end == helper.start)  // that means there is no mmap with a file associated to the memory
        helper.end = (uintptr_t)~0LL;
    helper.need_reloc = IsAddrNeedReloc(addr);
    // pass 0, addresses, x64 jump addresses, overall size of the block
    uintptr_t end = native_pass0(&helper, addr, alternate, is32bits, inst_max);
    if(helper.abort) {
        if(helper.size<2) {
            if(dyn->need_dump || BOX64ENV(dynarec_log))dynarec_log(LOG_NONE, "Abort dynablock on pass0\n");
            CancelBlock64(0);
            // return an Empty block to not try again, pass0 abort are definitive
            return CreateEmptyBlock(addr, is32bits, is_new);
        }
        if(dyn->need_dump || BOX64ENV(dynarec_log))dynarec_log(LOG_NONE, "Dynablock shorten on pass0 at ninst=%d\n", helper.size);
        --helper.size;
        helper.abort = 0;
    }
    if(BOX64ENV(dynarec_x87double)==2) {
        helper.need_x87check = 1;
    }
    // basic checks
    if(!helper.size) {
        dynarec_log(LOG_INFO, "Warning, null-sized dynarec block (%p)\n", (void*)addr);
        CancelBlock64(0);
        return CreateEmptyBlock(addr, is32bits, is_new);
    }
    if(!is_inhotpage && !isprotectedDB(addr, 1)) {
        dynarec_log(LOG_INFO, "Warning, write on current page on pass0, aborting dynablock creation (%p)\n", (void*)addr);
        CancelBlock64(0);
        return NULL;
    }
    // protect the block of it goes over the 1st page
    if(!is_inhotpage)
        if((addr&~(box64_pagesize-1))!=(end&~(box64_pagesize-1))) // need to protect some other pages too
            protectDB(addr, end-addr);  //end is 1byte after actual end
    // compute hash signature
    uint32_t hash = X31_hash_code((void*)addr, end-addr);
    // calculate barriers
    for(int ii=0; ii<helper.jmp_sz; ++ii) {
        int i = helper.jmps[ii];
        uintptr_t j = helper.insts[i].x64.jmp;
        helper.insts[i].x64.jmp_insts = -1;
        #ifndef ARCH_NOP
        if(j<start || j>=end || j==helper.insts[i].x64.addr)
        #else
        if(j<start || j>=end)
        #endif
        {
            helper.insts[i].x64.need_after |= X_PEND;
            if(helper.insts[i].barrier_maybe) {
                helper.insts[i].x64.barrier|=BARRIER_FLOAT;
                helper.insts[i].barrier_maybe = 0;
            }
        } else {
            // find jump address instruction
            int k=-1;
            int search = ((j>=helper.insts[0].x64.addr) && j<helper.insts[0].x64.addr+helper.isize)?1:0;
            int imin = 0;
            int imax = helper.size-1;
            int i2 = helper.size/2;
            // dichotomy search
            while(search) {
                if(helper.insts[i2].x64.addr == j) {
                    k = i2;
                    search = 0;
                } else if(helper.insts[i2].x64.addr>j) {
                    imax = i2;
                    i2 = (imax+imin)/2;
                } else {
                    imin = i2;
                    i2 = (imax+imin)/2;
                }
                if(search && (imax-imin)<2) {
                    search = 0;
                    if(helper.insts[imin].x64.addr==j)
                        k = imin;
                    else if(helper.insts[imax].x64.addr==j)
                        k = imax;
                }
            }
            /*for(int i2=0; i2<helper.size && k==-1; ++i2) {
                if(helper.insts[i2].x64.addr==j)
                    k=i2;
            }*/
            if(k!=-1) {
                // special case, loop on itself with some nop in between
                if(k<i && !helper.insts[i].x64.has_next && is_nops(&helper, helper.insts[k].x64.addr, helper.insts[i].x64.addr-helper.insts[k].x64.addr)) {
                    #ifndef ARCH_NOP
                    helper.always_test = 1;
                    k = -1;
                    #else
                    helper.insts[k].x64.self_loop = 1;
                    #endif
                }
                helper.insts[i].x64.jmp_insts = k;
                helper.insts[i].barrier_maybe = 0;
            } else {
                helper.insts[i].x64.need_after |= X_PEND;
                if(helper.insts[i].barrier_maybe) {
                    helper.insts[i].x64.barrier|=BARRIER_FLOAT;
                    helper.insts[i].barrier_maybe = 0;
                }
            }
        }
    }
    // fill predecessors with the jump address
    sizePredecessors(&helper);
    helper.predecessor = static_preds;
    fillPredecessors(&helper);

    PREUPDATE_SPECIFICS(&helper);

    int pos = helper.size-1;
    while (pos>=0)
        pos = updateNeed(&helper, pos, 0);
    // remove fpu stuff on non-executed code
    for(int i=1; i<helper.size-1; ++i)
        if(!helper.insts[i].pred_sz) {
            int ii = i;
            while(ii<helper.size && !helper.insts[ii].pred_sz) {
                fpu_reset_ninst(&helper, ii);
                RAZ_SPECIFIC(&helper, ii);
                ++ii;
            }
            i = ii;
        }
    // remove trailling dead code
    while(helper.size && !helper.insts[helper.size-1].x64.alive) {
        helper.isize-=helper.insts[helper.size-1].x64.size;
        --helper.size;
    }
    if(!helper.size) {
        // NULL block after removing dead code, how is that possible?
        dynarec_log(LOG_INFO, "Warning, null-sized dynarec block after trimming dead code (%p)\n", (void*)addr);
        CancelBlock64(0);
        return CreateEmptyBlock(addr, is32bits, is_new);
    }
    UPDATE_SPECIFICS(&helper);
    // check for still valid close loop
    for(int ii=0; ii<helper.jmp_sz && !helper.always_test; ++ii) {
        int i = helper.jmps[ii];
        if(helper.insts[i].x64.alive && (helper.insts[i].x64.jmp==helper.insts[i].x64.addr)) {
            #ifndef ARCH_NOP
            helper.always_test = 1;
            #else
            helper.insts[i].x64.self_loop = 1;
            #endif
        }
    }
    // no need for next anymore
    helper.next_sz = helper.next_cap = 0;
    helper.next = NULL;
    ResetTable64(&helper);
    helper.reloc_size = 0;
    // pass 1, float optimizations, first pass for flags
    native_pass1(&helper, addr, alternate, is32bits, inst_max);
    if(helper.abort) {
        if(dyn->need_dump || BOX64ENV(dynarec_log))dynarec_log(LOG_NONE, "Abort dynablock on pass1\n");
        CancelBlock64(0);
        return NULL;
    }
    if(BOX64ENV(dynarec_x87double)==2) {
        if(helper.need_x87check==1)
            helper.need_x87check = 0;
    }
    POSTUPDATE_SPECIFICS(&helper);
    ResetTable64(&helper);
    helper.reloc_size = 0;
    // pass 2, instruction size
    helper.callrets = static_callrets;
    native_pass2(&helper, addr, alternate, is32bits, inst_max);
    if(helper.abort) {
        if(dyn->need_dump || BOX64ENV(dynarec_log))dynarec_log(LOG_NONE, "Abort dynablock on pass2\n");
        CancelBlock64(0);
        return NULL;
    }
    // keep size of instructions for signal handling
    size_t native_size = (helper.native_size+7)&~7;   // round the size...
    // check if size is overlimit
    if((inst_max==MAX_INSTS) && (native_size>MAXBLOCK_SIZE)) {
        int imax = 0;
        size_t max_size = 0;
        while((max_size<MAXBLOCK_SIZE) && (imax<helper.size)) {
            max_size += helper.insts[imax].size;
            ++imax;
        }
        if(!imax) return NULL; //that should never happens
        --imax;
        if(dyn->need_dump || BOX64ENV(dynarec_log))dynarec_log(LOG_NONE, "Dynablock oversized, with %zu (max=%zd), recomputing cutting at %d from %d\n", native_size, MAXBLOCK_SIZE, imax, helper.size);
        CancelBlock64(0);
        return FillBlock64(addr, alternate, is32bits, imax, is_new);
    }
    size_t insts_rsize = (helper.insts_size+2)*sizeof(instsize_t);
    insts_rsize = (insts_rsize+7)&~7;   // round the size...
    size_t arch_size = ARCH_SIZE(&helper);
    size_t callret_size = helper.callret_size*sizeof(callret_t);
    size_t sep_size = helper.sep_size*sizeof(sep_t);
    size_t reloc_size = helper.reloc_size*sizeof(uint32_t);
    // ok, now allocate mapped memory, with executable flag on
    size_t sz = sizeof(void*) + native_size + helper.table64size*sizeof(uint64_t) + 4*sizeof(void*) + insts_rsize + arch_size + callret_size + sep_size;
    size_t dynablock_align = (sz&7)?(8 -(sz&7)):0;    // align dynablock
    sz += dynablock_align + sizeof(dynablock_t) + reloc_size;
    //           dynablock_t*     block (arm insts)            table64               jmpnext code       instsize     arch         callrets         sep  dynablock           relocs
    void* actual_p = (void*)AllocDynarecMap(addr, sz, is_new);
    if(actual_p==NULL) {
        dynarec_log(LOG_INFO, "AllocDynarecMap(%p, %zu) failed, canceling block\n", (void*)addr, sz);
        CancelBlock64(0);
        return NULL;
    }
    void* p = (void*)(((uintptr_t)actual_p) + sizeof(void*));
    void* tablestart = p + native_size;
    void* next = tablestart + helper.table64size*sizeof(uint64_t);
    void* instsize = next + 4*sizeof(void*);
    void* arch = instsize + insts_rsize;
    void* callrets = arch + arch_size;
    void* seps = callrets + callret_size;
    helper.block = p;
    dynablock_t* block = (dynablock_t*)(seps+sep_size+dynablock_align);
    memset(block, 0, sizeof(dynablock_t));
    void* relocs = helper.need_reloc?(block+1):NULL;
    // fill the block
    block->x64_addr = (void*)addr;
    block->isize = 0;
    block->actual_block = actual_p;
    helper.relocs = relocs;
    block->relocs = relocs;
    block->table64size = helper.table64size;
    helper.native_start = (uintptr_t)p;
    helper.tablestart = (uintptr_t)tablestart;
    helper.jmp_next = (uintptr_t)next+sizeof(void*);
    helper.instsize = (instsize_t*)instsize;
    *(dynablock_t**)actual_p = block;
    helper.table64cap = helper.table64size;
    helper.table64 = (uint64_t*)helper.tablestart;
    helper.callrets = (callret_t*)callrets;
    helper.sep = (sep_t*)seps;
    block->prefixsize = helper.prefixsize;
    block->table64 = helper.table64;
    if(callret_size)
        memcpy(helper.callrets, static_callrets, helper.callret_size*sizeof(callret_t));
    helper.callret_size = 0;
    helper.sep_size = 0;
    // pass 3, emit (log emit native opcode)
    if(dyn->need_dump) {
        dynarec_log(LOG_NONE, "%s%04d|Emitting %zu bytes for %u %s bytes (native=%zu, table64=%zu, instsize=%zu, arch=%zu, callrets=%zu, entry=%p)", (dyn->need_dump>1)?"\e[01;36m":"", GetTID(), helper.native_size, helper.isize, is32bits?"x86":"x64", native_size, helper.table64size*sizeof(uint64_t), insts_rsize, arch_size, callret_size, helper.block);
        PrintFunctionAddr(helper.start, " => ");
        dynarec_log_prefix(0, LOG_NONE, "%s\n", (dyn->need_dump>1)?"\e[m":"");
    }
    if (BOX64ENV(dynarec_gdbjit) && (!BOX64ENV(dynarec_gdbjit_end) || (addr >= BOX64ENV(dynarec_gdbjit_start) && addr < BOX64ENV(dynarec_gdbjit_end)))) {
        GdbJITNewBlock(helper.gdbjit_block, (GDB_CORE_ADDR)block->actual_block, (GDB_CORE_ADDR)block->actual_block + native_size, helper.start);
    }
    int oldtable64size = helper.table64size;
    size_t oldnativesize = helper.native_size;
    size_t oldinstsize = helper.insts_size;
    int oldsize= helper.size;
    helper.native_size = 0;
    ResetTable64(&helper); // reset table64 (but not the cap)
    helper.insts_size = 0;  // reset
    helper.reloc_size = 0;
    native_pass3(&helper, addr, alternate, is32bits, inst_max);
    if(helper.abort) {
        if(dyn->need_dump || BOX64ENV(dynarec_log))dynarec_log(LOG_NONE, "Abort dynablock on pass3\n");
        CancelBlock64(0);
        return NULL;
    }
    // no need for jmps anymore
    helper.jmp_sz = helper.jmp_cap = 0;
    helper.jmps = NULL;
    // keep size of instructions for signal handling
    block->instsize = instsize;
    helper.table64 = NULL;
    helper.instsize = NULL;
    helper.predecessor = NULL;
    block->size = sz;
    block->isize = helper.size;
    block->block = p;
    block->jmpnext = next+sizeof(void*);
    block->always_test = helper.always_test;
    block->dirty = block->always_test;
    block->is32bits = is32bits;
    block->relocsize = helper.reloc_size*sizeof(uint32_t);
    if(arch_size) {
        block->arch_size = arch_size;
        block->arch = ARCH_FILL(&helper, arch, arch_size);
        if(!block->arch) block->arch_size = 0;
    } else {
        block->arch = NULL;
        block->arch_size = arch_size;
    }
    block->callret_size = helper.callret_size;
    block->callrets = helper.callrets;
    block->sep_size = helper.sep_size;
    block->sep = helper.sep;
    block->native_size = native_size;
    *(dynablock_t**)next = block;
    for(int i=0; i<helper.sep_size; ++i) {
        // setup the dynablock reference for secondary entry points
        void* p = (block->block + helper.sep[i].nat_offs - sizeof(void*));
        *(dynablock_t**)p = block;
    }
    *(void**)(next+3*sizeof(void*)) = native_next;
    CreateJmpNext(block->jmpnext, next+3*sizeof(void*));
    ClearCache(block->jmpnext, 4*sizeof(void*));
    //block->x64_addr = (void*)start;
    block->x64_size = end-start;
    // all done...
    if (BOX64ENV(dynarec_gdbjit) && (!BOX64ENV(dynarec_gdbjit_end) || (addr >= BOX64ENV(dynarec_gdbjit_start) && addr < BOX64ENV(dynarec_gdbjit_end)))) {
        if (BOX64ENV(dynarec_gdbjit) != 3) GdbJITBlockReady(helper.gdbjit_block);
        GdbJITBlockCleanup(helper.gdbjit_block);
        #ifdef GDBJIT
        block->gdbjit_block = helper.gdbjit_block;
        #endif
    }
    ClearCache(actual_p+sizeof(void*), native_size);   // need to clear the cache before execution...
    block->hash = X31_hash_code(block->x64_addr, block->x64_size);
    // Check if something changed, to abort if it is
    if((helper.abort || (block->hash != hash))) {
        dynarec_log(LOG_DEBUG, "Warning, a block changed while being processed hash(%p:%ld)=%x/%x\n", block->x64_addr, block->x64_size, block->hash, hash);
        CancelBlock64(0);
        return NULL;
    }
    if((oldnativesize!=helper.native_size) || (oldtable64size<helper.table64size)) {
        printf_log(LOG_NONE, "Warning, size difference in block between pass2 (%zu, %d) & pass3 (%zu, %d)!\n", oldnativesize+oldtable64size*8, oldsize, helper.native_size+helper.table64size*8, helper.size);
        uint8_t *dump = (uint8_t*)helper.start;
        printf_log(LOG_NONE, "Dump of %d x64 opcodes:\n", helper.size);
        for(int i=0; i<helper.size; ++i) {
            printf_log(LOG_NONE, "%s%p:", (helper.insts[i].size2!=helper.insts[i].size)?"=====> ":"", dump);
            for(; dump<(uint8_t*)helper.insts[i+1].x64.addr; ++dump)
                printf_log_prefix(0, LOG_NONE, " %02X", *dump);
            printf_log_prefix(0, LOG_NONE, "\t%d -> %d", helper.insts[i].size2, helper.insts[i].size);
            if(helper.insts[i].ymm0_pass2 || helper.insts[i].ymm0_pass3)
                printf_log_prefix(0, LOG_NONE, "\t %04x -> %04x", helper.insts[i].ymm0_pass2, helper.insts[i].ymm0_pass3);
            printf_log_prefix(0, LOG_NONE, "\n");
        }
        printf_log(LOG_NONE, "Table64 \t%d -> %d\n", oldtable64size*8, helper.table64size*8);
        printf_log(LOG_NONE, " ------------\n");
        CancelBlock64(0);
        return NULL;
    }
    // ok, free the helper now
    ResetTable64(&helper);
    //dynaFree(helper.insts);
    helper.insts = NULL;
    if(insts_rsize/sizeof(instsize_t)<helper.insts_size) {
        printf_log(LOG_NONE, "Warning, insts_size difference in block between pass2 (%zu) and pass3 (%zu), allocated: %zu\n", oldinstsize, helper.insts_size, insts_rsize/sizeof(instsize_t));
    }
    if(!is_inhotpage && !isprotectedDB(addr, end-addr)) {
        dynarec_log(LOG_INFO, "Warning, block unprotected while being processed %p:%ld, marking as need_test\n", block->x64_addr, block->x64_size);
        block->dirty = 1;
        //protectDB(addr, end-addr);
    }
    if(getProtection(addr)&PROT_NEVERCLEAN) {
        block->always_test = 1;
    }
    else if(is_inhotpage)
        block->always_test = 2;
    if(block->always_test) {
        dynarec_log(LOG_INFO, "Note: block marked as always dirty %p:%ld\n", block->x64_addr, block->x64_size);
        #ifdef ARCH_NOP
        // mark callrets to trigger SIGILL to check clean state
        if(block->callret_size) {
            for(int i=0; i<block->callret_size; ++i)
                *(uint32_t*)(block->block+block->callrets[i].offs) = ARCH_UDF;
            ClearCache(block->block, block->size);
        }
        #endif
    }
    redundant_helper = current_helper = NULL;
    //block->done = 1;
    return block;
}
