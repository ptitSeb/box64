// SPDX-License-Identifier: MIT
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
#include "alternate.h"

#include "dynarec_native.h"
#include "dynarec_arch.h"
#include "dynarec_next.h"
#include "gdbjit.h"
#include "khash.h"

KHASH_MAP_INIT_INT64(table64, uint32_t)
KHASH_SET_INIT_INT64(nextset)
KHASH_MAP_INIT_INT64(jumpaddr, int)

static kh_nextset_t* khnextset = NULL;
static kh_jumpaddr_t* khjumpaddr = NULL;

void printf_x64_instruction(dynarec_native_t* dyn, zydis_dec_t* dec, instruction_x64_t* inst, const char* name) {
    uint8_t *ip = (uint8_t*)inst->addr;
    if (ip[0] == 0xcc && IsBridgeSignature(ip[1], ip[2])) {
        uintptr_t a = *(uintptr_t*)(ip+3);
        if(a==0) {
            dynarec_log(LOG_NONE, "%s%p: Exit x64emu%s\n", (dyn->need_dump == 2)?"\e[01;33m":"", (void*)ip, (dyn->need_dump == 2)?"\e[m":"");
        } else {
            dynarec_log(LOG_NONE, "%s%p: Native call to %p%s\n", (dyn->need_dump == 2)?"\e[01;33m":"", (void*)ip, (void*)a, (dyn->need_dump == 2)?"\e[m":"");
        }
    } else {
        if(dec) {
            dynarec_log(LOG_NONE, "%s%p: %s", (dyn->need_dump == 2) ? "\e[01;33m" : "", ip, DecodeX64Trace(dec, inst->addr, 1));
        } else {
            dynarec_log(LOG_NONE, "%s%p: ", (dyn->need_dump == 2)?"\e[01;33m":"", ip);
            for(int i=0; i<inst->size; ++i) {
                dynarec_log_prefix(0, LOG_NONE, "%02X ", ip[i]);
            }
            dynarec_log_prefix(0, LOG_NONE, " %s", name);
        }
        // print Call function name if possible
        if(dyn->need_dump != 3 && (ip[0]==0xE8 || ip[0]==0xE9)) { // Call / Jmp
            uintptr_t nextaddr = (uintptr_t)ip + 5 + *((int32_t*)(ip+1));
            PrintFunctionAddr(nextaddr, "=> ");
        } else if(dyn->need_dump != 3 && ip[0]==0xFF) {
            if(ip[1]==0x25) {
                uintptr_t nextaddr = (uintptr_t)ip + 6 + *((int32_t*)(ip+2));
                PrintFunctionAddr(nextaddr, "=> ");
            }
        }
        // end of line and colors
        dynarec_log_prefix(0, LOG_NONE, "%s\n", (dyn->need_dump == 2)?"\e[m":"");
    }
}

void add_next(dynarec_native_t* dyn, uintptr_t addr)
{
    if (dyn->peeking_flags) {
        if (dyn->next_sz < dyn->next_cap)
            dyn->next[dyn->next_sz++] = addr;
        return;
    }
    if (!BOX64DRENV(dynarec_bigblock))
        return;
    int ret;
    kh_put(nextset, khnextset, addr, &ret);
    if(!ret)
        return;
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
    while(i<dyn->next_sz) {
        uintptr_t next = dyn->next[i];
        if(!next || next<addr) {
            if (next && !dyn->peeking_flags) {
                khint_t k = kh_get(nextset, khnextset, next);
                if(k != kh_end(khnextset))
                    kh_del(nextset, khnextset, k);
            }
            dyn->next[i] = dyn->next[--dyn->next_sz];
            continue;
        }
        if((next<best) || !best)
            best = next;
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
    if (dyn->insts[ninst].x64.jmp && !dyn->peeking_flags) {
        if(!khjumpaddr)
            khjumpaddr = kh_init(jumpaddr);
        int ret;
        khint_t k = kh_put(jumpaddr, khjumpaddr, dyn->insts[ninst].x64.jmp, &ret);
        if(ret > 0)
            kh_value(khjumpaddr, k) = ninst;
    }
}
int get_first_jump(dynarec_native_t *dyn, int next) {
    if(next<0 || next>dyn->size)
        return -2;
    return get_first_jump_addr(dyn, dyn->insts[next].x64.addr);
}
int get_first_jump_addr(dynarec_native_t *dyn, uintptr_t next) {
    if (khjumpaddr && !dyn->peeking_flags) {
        khint_t k = kh_get(jumpaddr, khjumpaddr, next);
        if(k != kh_end(khjumpaddr)) {
            int ninst = kh_value(khjumpaddr, k);
            if(ninst >= 0 && ninst < dyn->size && dyn->insts[ninst].x64.jmp == next)
                return ninst;
        }
    }
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

NEW_JUMPBUFF(dynarec_jmpbuf);
int fillblock_active = 0;

void cancelFillBlock()
{
    LongJmp(GET_JUMPBUFF(dynarec_jmpbuf), 1);
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
    fillblock_active = 0;
    current_helper = NULL;
    redundant_helper = NULL;
    if(need_lock)
        mutex_unlock(&my_context->mutex_dyndump);
}

uintptr_t native_pass0(dynarec_native_t* dyn, uintptr_t addr, int alternate, int is32bits, int inst_max);
uintptr_t native_pass1(dynarec_native_t* dyn, uintptr_t addr, int alternate, int is32bits, int inst_max);
uintptr_t native_pass2(dynarec_native_t* dyn, uintptr_t addr, int alternate, int is32bits, int inst_max);
uintptr_t native_pass3(dynarec_native_t* dyn, uintptr_t addr, int alternate, int is32bits, int inst_max);

#ifdef LA64
#define LEAFCALL_MAX_INSTS       128
#define LEAFCALL_MAX_BYTES       4096
#define LEAFCALL_MAX_EMBEDDEDS   32
#define LEAFCALL_MAX_TOTAL_INSTS 512

typedef struct leafcall_peek_s {
    dynarec_native_t dyn;
    instruction_native_t insts[LEAFCALL_MAX_INSTS + 2];
    int jmps[LEAFCALL_MAX_INSTS + 2];
    uintptr_t next[LEAFCALL_MAX_INSTS + 2];
} leafcall_peek_t;

typedef struct leafcall_analysis_s {
    uintptr_t work[LEAFCALL_MAX_INSTS];
    uintptr_t decoded[LEAFCALL_MAX_INSTS];
    uint8_t decoded_len[LEAFCALL_MAX_INSTS];
    instruction_native_t* decoded_inst;
    uintptr_t body_start;
    uintptr_t body_end;
    int work_size;
    int decoded_size;
    int saw_ret;
    int reads_rsp;
    int implicit_rsp;
    int failed;
} leafcall_analysis_t;

#ifdef LA64
typedef struct leaf_embedded_s {
    dynarec_native_t dyn;
} leaf_embedded_t;

static leaf_embedded_t static_leaf_embeddeds[LEAFCALL_MAX_EMBEDDEDS];
static instruction_native_t static_leafinsts[LEAFCALL_MAX_TOTAL_INSTS + LEAFCALL_MAX_EMBEDDEDS];
static int static_leafjmps[LEAFCALL_MAX_TOTAL_INSTS];
static int static_leafpreds[LEAFCALL_MAX_TOTAL_INSTS * 2];
static instsize_t static_leaf_instsize[LEAFCALL_INSTSIZE_SCRATCH];
static int static_leaf_embedded_count;
static int static_leafinst_count;
static int static_leafdecoded_count;
static int static_leafpred_count;
#endif

static int leafcall_readable(uintptr_t addr, uintptr_t size)
{
    if (!size)
        return 1;
    uintptr_t end = addr + size - 1;
    if (end < addr)
        return 0;
    uintptr_t cur = addr;
    while (1) {
        uint32_t prot = getProtection(cur);
        if ((prot & (PROT_READ | PROT_EXEC)) != (PROT_READ | PROT_EXEC))
            return 0;
        uintptr_t page_end = (cur & ~(box64_pagesize - 1)) + box64_pagesize - 1;
        if (end <= page_end)
            return 1;
        cur = page_end + 1;
    }
}

static int leafcall_seen(const leafcall_analysis_t* scan, uintptr_t addr)
{
    for (int i = 0; i < scan->decoded_size; ++i)
        if (scan->decoded[i] == addr)
            return 1;
    return 0;
}

static int leafcall_queue_jump(leafcall_analysis_t* scan, uintptr_t addr, uintptr_t continuation)
{
    if (!addr || addr == continuation)
        return 0;
    if (addr < scan->body_start || addr >= scan->body_end)
        return 0;
    if (leafcall_seen(scan, addr))
        return 1;
    for (int i = 0; i < scan->work_size; ++i)
        if (scan->work[i] == addr)
            return 1;
    if (scan->work_size >= LEAFCALL_MAX_INSTS)
        return 0;
    scan->work[scan->work_size++] = addr;
    return 1;
}

static int leafcall_check_segment(leafcall_analysis_t* scan, uintptr_t entry, uintptr_t continuation, int is32bits)
{
    if (!leafcall_readable(entry, 1) || leafcall_seen(scan, entry))
        return leafcall_seen(scan, entry);

    leafcall_peek_t peek;
    memset(&peek, 0, sizeof(peek));
    dynarec_native_t* dyn = &peek.dyn;
    dyn->start = entry;
    dyn->end = entry + LEAFCALL_MAX_BYTES;
    if (dyn->end < entry)
        return 0;
    uintptr_t mapped = SizeFileMapped(entry);
    if (mapped && mapped < LEAFCALL_MAX_BYTES)
        dyn->end = entry + mapped;
    dyn->cap = LEAFCALL_MAX_INSTS + 2;
    dyn->insts = peek.insts;
    dyn->jmp_cap = LEAFCALL_MAX_INSTS + 2;
    dyn->jmps = peek.jmps;
    dyn->next_cap = LEAFCALL_MAX_INSTS + 2;
    dyn->next = peek.next;
    dyn->env = GetCurEnvByAddr(entry);
    dyn->is_file_mapped = IsAddrElfOrFileMapped(entry);
    dyn->peeking_flags = 1;

    uintptr_t end = native_pass0(dyn, entry, 0, is32bits, LEAFCALL_MAX_INSTS);
    if (dyn->size <= 0 || dyn->size > LEAFCALL_MAX_INSTS || end < entry)
        return 0;

    for (int i = 0; i < dyn->size; ++i) {
        instruction_x64_t* inst = &dyn->insts[i].x64;
        uintptr_t ip = inst->addr;
        uintptr_t next = (i + 1 < dyn->size) ? dyn->insts[i + 1].x64.addr : end;
        if (ip == continuation || ip < scan->body_start || next > scan->body_end)
            return 0;
        if (leafcall_seen(scan, ip))
            continue;
        if (scan->decoded_size >= LEAFCALL_MAX_INSTS)
            return 0;
        scan->decoded[scan->decoded_size] = ip;
        scan->decoded_len[scan->decoded_size] = next - ip;
        if (scan->decoded_inst) {
            scan->decoded_inst[scan->decoded_size] = dyn->insts[i];
            scan->decoded_inst[scan->decoded_size].pred = NULL;
            scan->decoded_inst[scan->decoded_size].pred_sz = 0;
        }
        ++scan->decoded_size;

        if (dyn->insts[i].host_call)
            return 0;

        if (inst->leaf_kind == LEAF_KIND_CALL)
            return 0;

        int implicit_rsp = (inst->leaf_rsp & LEAF_RSP_PUSHPOP) ? 1 : 0;
        if (implicit_rsp) scan->implicit_rsp = 1;

        if (inst->leaf_kind == LEAF_KIND_RET) {
            scan->saw_ret = 1;
            continue;
        }

        uint16_t sp_bits = (dyn->insts[i].up32_read | dyn->insts[i].up32_write64 | dyn->insts[i].up32_write32 | dyn->insts[i].up32_zero) & (1u << _SP);
        if ((inst->leaf_rsp & LEAF_RSP_REF) || (!implicit_rsp && sp_bits))
            scan->reads_rsp = 1;

        if (!inst->has_next) {
            if (!inst->jmp || inst->jmp_cond)
                return 0;
            if (!leafcall_queue_jump(scan, inst->jmp, continuation))
                return 0;
            continue;
        }
        if (inst->jmp) {
            if (!inst->jmp_cond || !leafcall_queue_jump(scan, inst->jmp, continuation))
                return 0;
        }
    }
    return 1;
}

static int leafcall_analyze_leaf(leafcall_analysis_t* scan, uintptr_t target, uintptr_t continuation, int is32bits, instruction_native_t* decoded_inst)
{
    memset(scan, 0, sizeof(*scan));
    scan->decoded_inst = decoded_inst;
    scan->body_start = target;
    scan->body_end = target + LEAFCALL_MAX_BYTES;
    if (scan->body_end < target)
        return 0;
    uintptr_t mapped = SizeFileMapped(target);
    if (mapped && mapped < LEAFCALL_MAX_BYTES)
        scan->body_end = target + mapped;
    scan->work[scan->work_size++] = target;
    while (scan->work_size && !scan->failed) {
        uintptr_t entry = scan->work[--scan->work_size];
        if (!leafcall_check_segment(scan, entry, continuation, is32bits))
            scan->failed = 1;
    }
    if (scan->failed || !scan->saw_ret || !scan->decoded_size)
        return 0;

    for (int i = 1; i < scan->decoded_size; ++i) {
        uintptr_t addr = scan->decoded[i];
        uint8_t len = scan->decoded_len[i];
        instruction_native_t inst;
        if (decoded_inst)
            inst = decoded_inst[i];
        int j = i;
        while (j && scan->decoded[j - 1] > addr) {
            scan->decoded[j] = scan->decoded[j - 1];
            scan->decoded_len[j] = scan->decoded_len[j - 1];
            if (decoded_inst)
                decoded_inst[j] = decoded_inst[j - 1];
            --j;
        }
        scan->decoded[j] = addr;
        scan->decoded_len[j] = len;
        if (decoded_inst)
            decoded_inst[j] = inst;
    }
    if (scan->decoded[0] != target)
        return 0;
    for (int i = 1; i < scan->decoded_size; ++i)
        if (scan->decoded[i] < scan->decoded[i - 1] + scan->decoded_len[i - 1])
            return 0;
    return 1;
}

static int leaf_embedded_find(const instruction_native_t* insts, int size, uintptr_t addr)
{
    int low = 0;
    int high = size - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (insts[mid].x64.addr == addr)
            return mid;
        if (insts[mid].x64.addr < addr)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return -1;
}

static int leaf_embedded_build(uintptr_t target, uintptr_t continuation, int is32bits, uint8_t* embedded_id)
{
    instruction_native_t decoded[LEAFCALL_MAX_INSTS + 2];
    leafcall_analysis_t scan;
    if (!embedded_id || static_leaf_embedded_count >= LEAFCALL_MAX_EMBEDDEDS || BOX64ENV(dynarec_test) || BOX64ENV(dynarec_trace))
        return 0;
#ifdef HAVE_ALTJUMP
    if (getAlternateJump((void*)target, is32bits))
        return 0;
#endif
    if (!leafcall_analyze_leaf(&scan, target, continuation, is32bits, decoded))
        return 0;
    if (static_leafdecoded_count + scan.decoded_size > LEAFCALL_MAX_TOTAL_INSTS || static_leafinst_count + scan.decoded_size + 1 > (int)(sizeof(static_leafinsts) / sizeof(static_leafinsts[0])))
        return 0;

    leaf_embedded_t* embedded = &static_leaf_embeddeds[static_leaf_embedded_count];
    memset(embedded, 0, sizeof(*embedded));
    dynarec_native_t* leaf = &embedded->dyn;
    leaf->insts = &static_leafinsts[static_leafinst_count];
    memcpy(leaf->insts, decoded, scan.decoded_size * sizeof(decoded[0]));
    memset(&leaf->insts[scan.decoded_size], 0, sizeof(leaf->insts[scan.decoded_size]));
    leaf->size = scan.decoded_size;
    leaf->cap = scan.decoded_size + 1;
    leaf->start = target;
    leaf->end = scan.decoded[scan.decoded_size - 1] + scan.decoded_len[scan.decoded_size - 1];
    leaf->isize = leaf->end - target;
    leaf->env = GetCurEnvByAddr(target);
    leaf->is_file_mapped = IsAddrElfOrFileMapped(target);
    leaf->peeking_flags = 2;
    leaf->inline_leaf = 1;
    leaf->inline_is32bits = is32bits;
    leaf->inline_rsp = scan.reads_rsp ? 0 : (scan.implicit_rsp ? (is32bits ? 4 : 8) : -(is32bits ? 4 : 8));
    leaf->instsize = static_leaf_instsize;
    leaf->jmps = &static_leafjmps[static_leafdecoded_count];
    leaf->jmp_cap = scan.decoded_size;

    for (int i = 0; i < leaf->size; ++i) {
        instruction_native_t* inst = &leaf->insts[i];
        inst->x64.size = scan.decoded_len[i];
        inst->x64.alive = 0;
        inst->x64.jmp_insts = -1;
        inst->pred = NULL;
        inst->pred_sz = 0;
        if (inst->x64.has_next) {
            if (i + 1 >= leaf->size || inst->x64.addr + inst->x64.size != leaf->insts[i + 1].x64.addr)
                return 0;
        }
        if (inst->x64.jmp) {
            int target_inst = leaf_embedded_find(leaf->insts, leaf->size, inst->x64.jmp);
            if (target_inst < 0)
                return 0;
            inst->x64.jmp_insts = target_inst;
            inst->barrier_maybe = 0;
            leaf->jmps[leaf->jmp_sz++] = i;
        }
        int isret = (inst->x64.leaf_kind == LEAF_KIND_RET);
        if (isret)
            inst->x64.need_after |= X_PEND;
    }
    leaf->insts[leaf->size].x64.addr = leaf->end;

    sizePredecessors(leaf);
    int predecessor_count = 0;
    for (int i = 0; i < leaf->size; ++i)
        predecessor_count += leaf->insts[i].pred_sz;
    if (static_leafpred_count + predecessor_count > (int)(sizeof(static_leafpreds) / sizeof(static_leafpreds[0])))
        return 0;
    leaf->predecessor = &static_leafpreds[static_leafpred_count];
    fillPredecessors(leaf);
    for (int i = 0; i < leaf->size; ++i)
        if (!leaf->insts[i].x64.alive)
            return 0;

    PREUPDATE_SPECIFICS(leaf);
    int pos = leaf->size - 1;
    while (pos >= 0)
        pos = updateNeed(leaf, pos, 0);
    updateUpperLiveness(leaf);
    updateRspMerge(leaf, is32bits);
    UPDATE_SPECIFICS(leaf);

    static_leafinst_count += leaf->size + 1;
    static_leafdecoded_count += leaf->size;
    static_leafpred_count += predecessor_count;
    *embedded_id = ++static_leaf_embedded_count;
    return 1;
}

static void leaf_embedded_reset(dynarec_native_t* dyn)
{
    static_leaf_embedded_count = 0;
    static_leafinst_count = 0;
    static_leafdecoded_count = 0;
    static_leafpred_count = 0;
    memset(static_leaf_embeddeds, 0, sizeof(static_leaf_embeddeds));
    dyn->leaf_embeddeds = static_leaf_embeddeds;
}

void* dynarec_get_leaf_embedded(void* owner, int ninst)
{
    dynarec_native_t* dyn = owner;
    if (!dyn || !dyn->leaf_embeddeds || ninst < 0 || ninst >= dyn->size)
        return NULL;
    int embedded_id = dyn->insts[ninst].x64.leaf_embedded;
    if (embedded_id <= 0 || embedded_id > static_leaf_embedded_count)
        return NULL;
    leaf_embedded_t* embeddeds = dyn->leaf_embeddeds;
    return &embeddeds[embedded_id - 1].dyn;
}
#endif

#define PEEK_FLAGS_INSTS 16

typedef struct peek_flags_state_s {
    dynarec_native_t dyn;
    instruction_native_t insts[PEEK_FLAGS_INSTS + 2];
    int jmps[PEEK_FLAGS_INSTS + 2];
    uintptr_t next[PEEK_FLAGS_INSTS + 2];
} peek_flags_state_t;

static peek_flags_state_t peek_flags_state;

static int peek_flags_readable(uintptr_t addr)
{
    const uintptr_t size = PEEK_FLAGS_INSTS * 16;
    uintptr_t end = addr + size - 1;
    if (end < addr)
        return 0;
    for (uintptr_t page = addr & ~(box64_pagesize - 1); page <= end; page += box64_pagesize) {
        uint32_t prot = getProtection(page);
        if (!(prot & PROT_READ) || !(prot & PROT_EXEC) || (prot & (PROT_WRITE | PROT_DYNAREC | PROT_NOPROT | PROT_NEVERCLEAN | PROT_NEVERCLEAN_MIXED)))
            return 0;
    }
    return 1;
}

int interblock_flags_needed(dynarec_native_t* dyn, uintptr_t addr, int is32bits)
{
    if (!BOX64ENV(dynarec_df) || !addr || dyn->peeking_flags)
        return 1;
    if (BOX64ENV(dynarec_test) || BOX64ENV(dynarec_trace))
        return 1;

    if (addr >= BOX64ENV(nodynarec_start) && addr < BOX64ENV(nodynarec_end))
        return 1;
    if (checkInHotPage(addr) && !BOX64ENV(dynarec_dirty))
        return 1;
    #ifdef HAVE_ALTJUMP
    if (getAlternateJump((void*)addr, is32bits))
        return 1;
    #endif
    const uint32_t req_prot = (box64_pagesize == 4096) ? (PROT_EXEC | PROT_READ) : PROT_READ;
    uint32_t prot = getProtection_fast(addr);
    if ((prot & req_prot) != req_prot || (prot & (PROT_WRITE | PROT_DYNAREC | PROT_NOPROT | PROT_NEVERCLEAN | PROT_NEVERCLEAN_MIXED)))
        return 1;

    if (!peek_flags_readable(addr))
        return 1;

    if (current_helper && ((dynarec_native_t*)current_helper)->start == addr)
        return 1;

    memset(&peek_flags_state, 0, sizeof(peek_flags_state));
    dynarec_native_t* peek_dyn = &peek_flags_state.dyn;

    peek_dyn->start = addr;
    peek_dyn->end = addr + SizeFileMapped(addr);
    if (peek_dyn->end == peek_dyn->start)
        peek_dyn->end = (uintptr_t)~0ULL;
    peek_dyn->cap = PEEK_FLAGS_INSTS + 2;
    peek_dyn->insts = peek_flags_state.insts;
    peek_dyn->jmp_cap = PEEK_FLAGS_INSTS + 2;
    peek_dyn->jmps = peek_flags_state.jmps;
    peek_dyn->next_cap = PEEK_FLAGS_INSTS + 2;
    peek_dyn->next = peek_flags_state.next;
    peek_dyn->env = GetCurEnvByAddr(addr);
    if (peek_dyn->env && (peek_dyn->env->dynarec_test || peek_dyn->env->dynarec_trace))
        return 1;
    peek_dyn->is_file_mapped = IsAddrElfOrFileMapped(addr);

    peek_dyn->peeking_flags = 1;
    native_pass0(peek_dyn, addr, 0, is32bits, PEEK_FLAGS_INSTS);

    if (peek_dyn->size <= 0 || peek_dyn->size > PEEK_FLAGS_INSTS)
        return 1;

    for (int i = 0; i < peek_dyn->size; ++i) {
        instruction_x64_t* inst = &peek_dyn->insts[i].x64;
        if (inst->use_flags || inst->jmp || inst->has_callret || !inst->has_next)
            return 1;
        if (inst->set_flags == X_ALL && (inst->state_flags == SF_SET || inst->state_flags == SF_SET_NODF))
            return 0;
    }
    return 1;
}

dynablock_t* CreateEmptyBlock(uintptr_t addr, int is32bits, int is_new) {
    size_t sz = JMPNEXT_SIZE + sizeof(dynablock_t);
    void* actual_p = (void*)AllocDynarecMap(addr, sz, is_new);
    void* p = actual_p + sizeof(void*);
    if(actual_p==NULL) {
        dynarec_log(LOG_INFO, "AllocDynarecMap(%p, %zu) failed, canceling block\n", (void*)addr, sz);
        CancelBlock64(0);
        return NULL;
    }
    dynablock_t* block = (dynablock_t*)(actual_p+JMPNEXT_SIZE);
    memset(block, 0, sizeof(dynablock_t));
    // fill the block
    block->x64_addr = (void*)addr;
    block->x64_readaddr = addr;
    block->isize = 0;
    block->done = 0;
    block->size = sz;
    block->actual_block = actual_p;
    block->block = p;
    block->jmpnext = p;
    block->is32bits = is32bits;
    *(dynablock_t**)actual_p = block;
    *(void**)(p+JMPNEXT_SIZE-2*sizeof(void*)) = native_epilog;
    CreateJmpNext(block->jmpnext, p+JMPNEXT_SIZE-2*sizeof(void*));
    // all done...
    ClearCache(actual_p+sizeof(void*), JMPNEXT_SIZE-sizeof(void*));   // need to clear the cache before execution...
    return block;
}

dynablock_t* FillBlock64(uintptr_t addr, int is32bits, int inst_max, int is_new, int noalt) {
    /*
        A Block must have this layout:

        0x0000..0x0007  : dynablock_t* : self
        0x0008..8+4*n   : actual Native instructions, (n is the total number)
        A ..    A+8*n   : Table64: n 64bits values
        B ..    B+7     : dynablock_t* : self (as part of JmpNext, that simulate another block)
        B+8 ..  B+8+m   : Native code for jmpnext (or jmp epilog in case of empty block), m depends on arch
        B+J-8.. B+J-1   : jmpnext (or jmp_epilog) address. jumpnext is used when the block needs testing
                           (J = JMPNEXT_SIZE, varies by architecture)
        B+J ..  B+J+sz  : instsize (compressed array with each instruction length on x64 and native side)
        C ..    C+sz    : arch: arch specific info (likes flags info) per inst (can be absent)

    */
    const uint32_t req_prot = (box64_pagesize==4096)?(PROT_EXEC|PROT_READ):PROT_READ;
    uintptr_t old_addr = addr;
    #ifdef HAVE_ALTJUMP
    uintptr_t altjump = noalt?0:getAlternateJump((void*)addr, is32bits);
    if(altjump) {
        dynarec_log(LOG_INFO, "Building a Dynablock for %p with an alternate content at %p\n", (void*)addr, (void*)altjump);
        addr = altjump;
    }
    #else
    uintptr_t altjump = 0;
    #endif
    if(addr>=BOX64ENV(nodynarec_start) && addr<BOX64ENV(nodynarec_end)) {
        dynarec_log(LOG_INFO, "Create empty block in no-dynarec zone\n");
        return BOX64ENV(nodynarec_delay)?NULL:CreateEmptyBlock(old_addr, is32bits, is_new);
    }
    int is_inhotpage = checkInHotPage(addr);
    if(is_inhotpage && !BOX64ENV(dynarec_dirty)) {
        dynarec_log(LOG_DEBUG, "Not creating dynablock at %p as in a HotPage\n", (void*)addr);
        return NULL;
    }
    uint32_t prot = getProtection_fast(addr);
#ifndef _WIN32
    if((prot&req_prot)!=req_prot) {// cannot be run, get out of the Dynarec
        dynarec_log(LOG_DEBUG, "Not creating dynablock at %p because EXEC protection is missing\n", (void*)addr);
        return NULL;
    }
#endif
    if(current_helper) {
        if(current_helper==redundant_helper) {
            dynarec_log(LOG_INFO, "%04d|Warning: previous FillBlock did not cleaned up correctly (helper=%p, x64addr=%p, db=%p)\n", GetTID(), current_helper, (void*)((dynarec_native_t*)current_helper)->start, ((dynarec_native_t*)current_helper)->dynablock);
            return NULL;
        }
        dynarec_log(LOG_INFO, "Warning: some static area curruption appeared (current=%p, redundant=%p)\n", current_helper, redundant_helper);
    }
    // protect the 1st page
    protectDB(addr, 1);
    if(box64_pagesize>4096)
        prot = getProtection_fast(addr);    // update protection as it might got a NEVERCLEAN tag because of the protectDB for large pagesize
    // init the helper
    dynarec_native_t helper = {0};
    dynarec_native_t* dyn = &helper;
    if(!khnextset)
        khnextset = kh_init(nextset);
    kh_clear(nextset, khnextset);
    if(!khjumpaddr)
        khjumpaddr = kh_init(jumpaddr);
    kh_clear(jumpaddr, khjumpaddr);
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
    #ifdef LA64
    leaf_embedded_reset(&helper);
    #endif
    if(prot&PROT_NEVERCLEAN) {
        helper.always_test = 1;
    }
    ResetTable64(&helper);
    helper.table64cap = 0;
    helper.end = addr + SizeFileMapped(addr);
    if(helper.end == helper.start)  // that means there is no mmap with a file associated to the memory
        helper.end = (uintptr_t)~0LL;
    helper.need_reloc = IsAddrNeedReloc(addr);
    size_t native_size = 0;
    size_t insts_rsize = 0;
    size_t arch_size = 0;
    size_t callret_size = 0;
    size_t sep_size = 0;
    size_t reloc_size = 0;
    size_t sz = 0;
    size_t dynablock_align = 0;
    size_t oldnativesize = 0;
    size_t oldinstsize = 0;
    uintptr_t end = 0;
    void* actual_p = NULL;
    void* p = NULL;
    void* next = NULL;
    void* instsize = NULL;
    void* tablestart = NULL;
    void* arch = NULL;
    void* callrets = NULL;
    void* seps = NULL;
    dynablock_t* block = NULL;
    #define BUILD_INIT 0
    #define BUILD_PASS0 1
    #define BUILD_PASS1 2
    #define BUILD_PASS2 3
    #define BUILD_PASS3 4
    #define BUILD_POST  50
    #define BUILD_DONE 100
    #define BUILD_ABORT_NULL    200
    #define BUILD_ABORT_EMPTY   201
    int state = BUILD_INIT;
    while(state!=BUILD_DONE) switch(state) {
        case BUILD_INIT:
            state = BUILD_PASS0;
            if (SigSetJmp(GET_JUMPBUFF(dynarec_jmpbuf), 1)) {
                if(state==BUILD_PASS0 && helper.size>1) {
                    end = helper.insts[helper.size].x64.addr;
                    --helper.size;
                    printf_log(LOG_INFO, "FillBlock at %p triggered a segfault, truncating at %d\n", (void*)addr, helper.size);
                    state = BUILD_PASS1;
                } else {
                    printf_log(LOG_INFO, "FillBlock at %p triggered a segfault (state=%d, size=%d), canceling\n", (void*)addr, state, helper.size);
                    state = BUILD_ABORT_EMPTY;
                }
            } else
                fillblock_active = 1;
            break;
        case BUILD_ABORT_NULL:
            CancelBlock64(0);
            return NULL;
        case BUILD_ABORT_EMPTY:
            CancelBlock64(0);
            return CreateEmptyBlock(old_addr, is32bits, is_new);
        case BUILD_PASS0:
            // pass 0, addresses, x64 jump addresses, overall size of the block
            end = native_pass0(&helper, addr, altjump?1:0, is32bits, inst_max);
            if(helper.abort) {
                if(helper.size<2) {
                    if(dyn->need_dump || BOX64ENV(dynarec_log))dynarec_log(LOG_NONE, "Abort dynablock on pass0\n");
                    state = BUILD_ABORT_EMPTY;
                    continue;
                }
                if(dyn->need_dump || BOX64ENV(dynarec_log))dynarec_log(LOG_NONE, "Dynablock shorten on pass0 at ninst=%d\n", helper.size);
                --helper.size;
                // preserve flags at the new block exit
                helper.insts[helper.size - 1].x64.need_after |= X_PEND;
                helper.abort = 0;
            }
            // basic checks
            if(!helper.size) {
                dynarec_log(LOG_INFO, "Warning, null-sized dynarec block (%p)\n", (void*)addr);
                state = BUILD_ABORT_EMPTY;
                continue;
            }
            #ifdef LA64
            if (BOX64DRENV(dynarec_callret) >= 3) {
                for (int i = 0; i < helper.size; ++i) {
                    instruction_x64_t* inst = &helper.insts[i].x64;
                    if (inst->leaf_kind != LEAF_KIND_CALL)
                        continue;
                    uintptr_t target = inst->leaf_target;
                    uintptr_t continuation = inst->addr + inst->size;
                    uint8_t embedded_id = 0;
                    if (!target || !inst->has_next || i + 1 >= helper.size || helper.insts[i + 1].x64.addr != continuation || !leaf_embedded_build(target, continuation, is32bits, &embedded_id))
                        inst->leaf_call = 0;
                    else
                        inst->leaf_embedded = embedded_id;
                }
                for (int i = 0; i < helper.size; ++i)
                    if (helper.insts[i].x64.leaf_call)
                        helper.insts[i].x64.barrier = BARRIER_FLOAT;
            } else {
                for (int i = 0; i < helper.size; ++i) {
                    helper.insts[i].x64.leaf_call = 0;
                    helper.insts[i].x64.leaf_embedded = 0;
                    helper.insts[i].x64.leaf_kind = LEAF_KIND_NONE;
                    helper.insts[i].x64.leaf_rsp = 0;
                    helper.insts[i].x64.leaf_target = 0;
                }
            }
            #endif
            if(!is_inhotpage && !isprotectedDB(addr, 1)) {
                dynarec_log(LOG_INFO, "Warning, write on current page on pass0, aborting dynablock creation (%p)\n", (void*)addr);
                state = BUILD_ABORT_NULL;
                continue;
            }
            state = BUILD_PASS1;
            //fallthru
        case BUILD_PASS1:
            if(BOX64ENV(dynarec_x87double)==2) {
                helper.need_x87check = 1;
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
                    int need_df = helper.insts[i].x64.has_callret || !j || j == helper.insts[i].x64.addr || interblock_flags_needed(dyn, j, is32bits);
                    if (need_df)
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
            #if defined(LA64)
            updateUpperLiveness(&helper);
            updateRspMerge(&helper, is32bits);
            #endif
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
                return CreateEmptyBlock(old_addr, is32bits, is_new);
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
            native_pass1(&helper, addr, altjump?1:0, is32bits, inst_max);
            if(helper.abort) {
                if(dyn->need_dump || BOX64ENV(dynarec_log))dynarec_log(LOG_NONE, "Abort dynablock on pass1\n");
                state = BUILD_ABORT_NULL;
                continue;
            }
            state = BUILD_PASS2;
            //fallthrough
        case BUILD_PASS2:
            if(BOX64ENV(dynarec_x87double)==2) {
                if(helper.need_x87check==1)
                    helper.need_x87check = 0;
            }
            POSTUPDATE_SPECIFICS(&helper);
            ResetTable64(&helper);
            helper.reloc_size = 0;
            // pass 2, instruction size
            helper.callrets = static_callrets;
            native_pass2(&helper, addr, altjump?1:0, is32bits, inst_max);
            if(helper.abort) {
                if(dyn->need_dump || BOX64ENV(dynarec_log))dynarec_log(LOG_NONE, "Abort dynablock on pass2\n");
                state = BUILD_ABORT_NULL;
                continue;
            }
            state = BUILD_PASS3;
            //fallthrough
        case BUILD_PASS3:
            // keep size of instructions for signal handling
            native_size = (helper.native_size+7)&~7;   // round the size...
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
                return FillBlock64(old_addr, is32bits, imax, is_new, noalt);
            }
            insts_rsize = (helper.insts_size+2)*sizeof(instsize_t);
            insts_rsize = (insts_rsize+7)&~7;   // round the size...
            arch_size = ARCH_SIZE(&helper);
            callret_size = helper.callret_size*sizeof(callret_t);
            sep_size = helper.sep_size*sizeof(sep_t);
            reloc_size = helper.reloc_size*sizeof(uint32_t);
            // ok, now allocate mapped memory, with executable flag on
            sz = sizeof(void*) + native_size + helper.table64size*sizeof(uint64_t) + JMPNEXT_SIZE + insts_rsize + arch_size + callret_size + sep_size;
            dynablock_align = (sz&7)?(8 -(sz&7)):0;    // align dynablock
            sz += dynablock_align + sizeof(dynablock_t) + reloc_size;
            //           dynablock_t*     block (arm insts)            table64               jmpnext code       instsize     arch         callrets         sep  dynablock           relocs
            actual_p = (void*)AllocDynarecMap(old_addr, sz, is_new);
            if(actual_p==NULL) {
                dynarec_log(LOG_INFO, "AllocDynarecMap(%p, %zu) failed, canceling block\n", (void*)addr, sz);
                state = BUILD_ABORT_NULL;
                continue;
            }
            p = (void*)(((uintptr_t)actual_p) + sizeof(void*));
            tablestart = p + native_size;
            next = tablestart + helper.table64size*sizeof(uint64_t);
            instsize = next + JMPNEXT_SIZE;
            arch = instsize + insts_rsize;
            callrets = arch + arch_size;
            seps = callrets + callret_size;
            helper.block = p;
            block = (dynablock_t*)(seps+sep_size+dynablock_align);
            memset(block, 0, sizeof(dynablock_t));
            void* relocs = helper.need_reloc?(block+1):NULL;
            // fill the block
            block->x64_addr = (void*)addr;
            block->x64_readaddr = addr;
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
            helper.dynablock = block;
            if(callret_size)
                memcpy(helper.callrets, static_callrets, helper.callret_size*sizeof(callret_t));
            helper.callret_size = 0;
            helper.sep_size = 0;
            // pass 3, emit (log emit native opcode)
            if(dyn->need_dump && dyn->need_dump != 3) {
                dynarec_log(LOG_NONE, "%s%04d|Emitting %zu bytes for %u %s bytes (native=%zu, table64=%zu, instsize=%zu, arch=%zu, callrets=%zu, entry=%p)", (dyn->need_dump>1)?"\e[01;36m":"", GetTID(), helper.native_size, helper.isize, is32bits?"x86":"x64", native_size, helper.table64size*sizeof(uint64_t), insts_rsize, arch_size, callret_size, helper.block);
                PrintFunctionAddr(helper.start, " => ");
                dynarec_log_prefix(0, LOG_NONE, "%s\n", (dyn->need_dump>1)?"\e[m":"");
            }
            if (BOX64ENV(dynarec_gdbjit) && (!BOX64ENV(dynarec_gdbjit_end) || (addr >= BOX64ENV(dynarec_gdbjit_start) && addr < BOX64ENV(dynarec_gdbjit_end)))) {
                GdbJITNewBlock(helper.gdbjit_block, (GDB_CORE_ADDR)block->actual_block, (GDB_CORE_ADDR)block->actual_block + native_size, helper.start);
            }
            int oldtable64size = helper.table64size;
            oldnativesize = helper.native_size;
            oldinstsize = helper.insts_size;
            int oldsize= helper.size;
            helper.native_size = 0;
            ResetTable64(&helper); // reset table64 (but not the cap)
            helper.insts_size = 0;  // reset
            helper.reloc_size = 0;
            native_pass3(&helper, addr, altjump?1:0, is32bits, inst_max);
            if(helper.abort) {
                if(dyn->need_dump || BOX64ENV(dynarec_log))dynarec_log(LOG_NONE, "Abort dynablock on pass3\n");
                state = BUILD_ABORT_NULL;
                continue;
            }
            state = BUILD_POST;
            //fallthrough
        case BUILD_POST:
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
            #ifdef ARCH_CRC_INLINE
            block->always_test = 0;
            block->autocrc = helper.always_test?1:0;
            #else
            block->always_test = helper.always_test;
            block->autocrc = 0;
            #endif
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
            *(void**)(next+JMPNEXT_SIZE-sizeof(void*)) = native_next;
            CreateJmpNext(block->jmpnext, next+JMPNEXT_SIZE-sizeof(void*));
            ClearCache(block->jmpnext, JMPNEXT_SIZE-sizeof(void*));
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
            block->hash = X31_hash_code((void*)block->x64_readaddr, block->x64_size);
            // Check if something changed, to abort if it is
            if((helper.abort || (block->hash != hash))) {
                dynarec_log(LOG_DEBUG, "Warning, a block changed while being processed hash(%p:%ld)=%x/%x\n", block->x64_readaddr, block->x64_size, block->hash, hash);
                state = BUILD_ABORT_NULL;
                continue;
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
                state = BUILD_ABORT_NULL;
                continue;
            }
            state = BUILD_DONE;
    }
    fillblock_active = 0;   // disable the use of the LongJump if Segfault/Sigbus
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
#ifdef ARCH_CRC_INLINE
    if(is_inhotpage && !block->autocrc)
        block->always_test = 2;
#else
    if(is_inhotpage)
        block->always_test = 2;
#endif
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
    if(altjump) block->x64_addr = (void*)old_addr; // set the not-alt addr if a shadow jump was used
    redundant_helper = current_helper = NULL;
    //block->done = 1;
    return block;
}
