// SPDX-License-Identifier: MIT
#include <string.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>

#include "box64ec_exception.h"
#include "box64ec_private.h"
#include "box64ec_syscalls.h"
#include "emu/x64emu_private.h"
#include "wine/compiler.h"
#ifdef DYNAREC
#include "custommem.h"
#include "dynablock.h"
#include "dynarec/dynablock_private.h"
#include "emu/x64run_private.h"
#include "dynarec/dynarec_arch.h"
#include "dynarec/arm64/dynarec_arm64_arch.h"
#include "box64context.h"
#include "os.h"

#include "khash.h"

extern int fillblock_active;
extern void* current_helper;
void adjustregs(x64emu_t* emu, void* pc);
extern void arm64_epilog(void);

// x64 addresses of opcodes that wrote into their own block: the next blocks stop before them
KHASH_SET_INIT_INT64(autosmc)
static kh_autosmc_t* autosmc = NULL;

int is_addr_autosmc(uintptr_t addr)
{
    return autosmc && kh_get(autosmc, autosmc, addr) != kh_end(autosmc);
}

/* Other threads change the block marks while building and freeing blocks */
static dynablock_t* find_jit_block(uintptr_t pc)
{
    dynablock_t* db;

    mutex_lock(&my_context->mutex_dyndump);
    db = FindDynablockFromNativeAddress((void*)pc);
    mutex_unlock(&my_context->mutex_dyndump);
    return db;
}

/* A write to guest code the Dynarec write-protected, from any code: drop the
 * blocks of the page and retry the write, as the Linux signal handler does.
 * Returns if the fault is not one of those. */
static void handle_smc_write(EXCEPTION_RECORD* record, ARM64_NT_CONTEXT* arm_context,
                             CHPE_V2_CPU_AREA_INFO* area, x64emu_t* emu)
{
    uintptr_t addr = record->ExceptionInformation[1];
    uint32_t prot;
    dynablock_t* db;

    if (record->ExceptionCode != STATUS_ACCESS_VIOLATION || record->NumberParameters < 2 ||
        record->ExceptionInformation[0] != 1)
        return;
    prot = getProtection(addr);
    if (prot & PROT_DYNAREC) {
        CheckHotPage(addr, prot);
        unprotectDB(addr, 1, 1);
    } else if (!(prot & PROT_WRITE))
        return;     // not a write to protected guest code (or another thread already unprotected it)
    db = (area && emu && area->InSimulation) ? find_jit_block(arm_context->Pc) : NULL;
    if (db) {
        uintptr_t x64pc = getX64Address(db, arm_context->Pc);
        int own = addr >= (uintptr_t)db->x64_addr && addr < (uintptr_t)db->x64_addr + db->x64_size;
        int same_page = (addr & ~(uintptr_t)0xfff) == ((uintptr_t)db->x64_addr & ~(uintptr_t)0xfff);
        if (x64pc && (own || (same_page && !is_addr_autosmc(x64pc)) || getNeedTest((uintptr_t)db->x64_addr))) {
            /* the rest of the block can be stale: leave it through the epilog, the
             * write is then done again from the next block */
            jit_arm_context_to_emu(arm_context, emu);
            R_RIP = x64pc;
            adjustregs(emu, (void*)(uintptr_t)arm_context->Pc);
            adjust_arch(db, emu, arm_context, x64pc);
            if (own) {
                int ret;
                mutex_lock(&my_context->mutex_dyndump);     // native_pass reads it under this lock
                if (!autosmc)
                    autosmc = kh_init(autosmc);
                kh_put(autosmc, autosmc, x64pc, &ret);
                mutex_unlock(&my_context->mutex_dyndump);
                db->to_delete = 1;
                MarkDynablock(db);
            }
            emu_to_jit_arm_context(emu, arm_context);
            arm_context->X0 = (uintptr_t)emu;
            arm_context->Pc = (uintptr_t)arm64_epilog;
        }
    }
    Box64EC_ContinueNative(arm_context, FALSE);
}

/* CALLRET landing patched to UDF: resume if the code is unchanged, as Linux signals.c */
static void handle_dirty_callret(ARM64_NT_CONTEXT* arm_context, x64emu_t* emu, dynablock_t* db)
{
    uintptr_t pc = arm_context->Pc;
    uintptr_t x64pc;
    int type = -1;

    for (int i = 0; i < db->callret_size; ++i)
        if (pc == (uintptr_t)db->block + db->callrets[i].offs)
            type = db->callrets[i].type;
    if (type < 0)
        return;
    x64pc = type ? getX64Address(db, pc) : arm_context->X27;
    int is_hotpage = checkInHotPage(x64pc);
    // protect then hash, as DBGetBlock, outside the lock: the guest code can fault
    uint32_t generation = getUnprotectGeneration();
    if (!db->gone && !is_hotpage && !db->always_test)
        protectDB((uintptr_t)db->x64_addr, db->x64_size);
    uint32_t hash = (db->gone || is_hotpage) ? 0 : X31_hash_code(db->x64_addr, db->x64_size);
    mutex_lock(&my_context->mutex_dyndump);
    if (!db->gone && !db->to_delete && (!is_hotpage || db->autocrc) && hash == db->hash) {
        arm_context->Pc += 4;
        if (db->always_test)
            protectDB((uintptr_t)db->x64_addr, 1);
        else if (generation == getUnprotectGeneration()) {
            for (int i = 0; i < db->callret_size; ++i)
                *(uint32_t*)(db->block + db->callrets[i].offs) = ARCH_NOP;
            ClearCache(db->block, db->size);
            protectDBJumpTable((uintptr_t)db->x64_addr, db->x64_size, db->block, db->jmpnext);
            if (generation != getUnprotectGeneration())
                MarkDynablock(db);
        }
        mutex_unlock(&my_context->mutex_dyndump);
        Box64EC_ContinueNative(arm_context, FALSE);
        return;
    }
    mutex_unlock(&my_context->mutex_dyndump);
    // the block is dirty: a return address only needs the epilog, a loop needs the full state
    jit_arm_context_to_emu(arm_context, emu);
    R_RIP = x64pc;
    if (type) {
        adjustregs(emu, (void*)pc);
        if (db->arch_size)
            adjust_arch(db, emu, arm_context, x64pc);
    }
    emu->test.clean = 0;
    emu_to_jit_arm_context(emu, arm_context);
    arm_context->X0 = (uintptr_t)emu;
    arm_context->Pc = (uintptr_t)arm64_epilog;
    Box64EC_ContinueNative(arm_context, FALSE);
}

/* An LRCPC access (LDAPUR, STLUR and the SIMD forms) crossing a 16 byte boundary faults: patch it to the plain
 * LDUR/STUR, which then runs again in the JIT, and mark the block for a rebuild without LRCPC for it.
 * Returns if the fault is not one of those. */
static void handle_misaligned_lrcpc(ARM64_NT_CONTEXT* arm_context, dynablock_t* db, box64ec_thr_t* state)
{
    uint32_t* code = (uint32_t*)(uintptr_t)arm_context->Pc;
    uintptr_t x64pc = getX64Address(db, arm_context->Pc);
    int handled = 0;

    if (!x64pc)
        return;
    mutex_lock(&my_context->mutex_dyndump);    // native_pass reads the lrcpc addresses under this lock
    uint32_t opcode = *code;
    uint32_t form = opcode & 0x3F200C00;
    int simd = form == 0x1D000800 || form == 0x3C000000;
    int size = opcode >> 30;    // log2 of the access size
    int rn = (opcode >> 5) & 31;
    uint64_t base = rn == 31 ? arm_context->Sp : arm_context->X[rn];
    uint64_t addr = base + ((int32_t)(opcode << 11) >> 23);

    if (simd)
        size |= ((opcode >> 23) & 1) << 2;    // 128bit access
    if (addr & ((1 << size) - 1)) {
        if (form == 0x19000000 || form == 0x1D000800) {
            add_lrcpc_address(x64pc);
            if (arch_lrcpc(db, x64pc))
                mark_db_lrcpc(db, x64pc);
            *(volatile uint32_t*)code = simd ? (opcode & 0xC0FFF3FF) | 0x3C000000 : (opcode & 0xC0FFFFFF) | 0x38000000;
            ClearCache(code, sizeof(*code));
            handled = 1;
            state->lrcpc_retry_pc = 0;
        } else if (form == 0x38000000 || form == 0x3C000000) {
            // another thread patched it after this one ran the LRCPC form: run it again, a second fault here is a real one
            handled = is_addr_lrcpc(x64pc) && state->lrcpc_retry_pc != arm_context->Pc;
            state->lrcpc_retry_pc = handled ? arm_context->Pc : 0;
        }
    }
    mutex_unlock(&my_context->mutex_dyndump);
    if (handled)
        Box64EC_ContinueNative(arm_context, FALSE);
}
#endif

/* Dispatch a host fault in Run with the guest state, on the guest stack. */
static void rethrow_interpreter_fault(EXCEPTION_RECORD* record,
                                      ARM64_NT_CONTEXT* arm_context,
                                      x64emu_t* emu)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    box64ec_thr_t* state = Box64EC_GetThreadState();
    KiUserExceptionDispatcherStackLayout* args;
    uint64_t guest_sp = R_RSP & ~(uint64_t)63;

    if (!state || !Box64EC_KiUserExceptionDispatcher || !guest_sp ||
        Box64EC_IsEmulatorStackAddress(guest_sp))
        Box64EC_FatalEnterJit(0xec08);
    args = ((KiUserExceptionDispatcherStackLayout*)(uintptr_t)guest_sp) - 1;
    memset(args, 0, sizeof(*args));
    emu_to_arm64_ec_packed(emu, &args->Context);
    args->Context.Fpcr = arm_context->Fpcr;
    args->Context.Fpsr = arm_context->Fpsr;
    args->Rec = *record;
    args->Rec.ExceptionAddress = (void*)(uintptr_t)R_RIP;
    args->Sp = R_RSP;
    args->Pc = R_RIP;
    arm_context->Sp = (ULONG64)(uintptr_t)args;
    arm_context->Fp = arm_context->Sp;
    arm_context->Pc = Box64EC_KiUserExceptionDispatcher;
    arm_context->X18 = (ULONG64)(uintptr_t)NtCurrentTeb();
    state->guest_exception_code = record->ExceptionCode;
    state->guest_exception_address = (uintptr_t)args->Rec.ExceptionAddress;
    state->guest_exception_rip = R_RIP;
    state->guest_exception_rsp = R_RSP;
    InterlockedExchange(&state->guest_exception_pending, 1);
    area->InSimulation = FALSE;
    Box64EC_ContinueNative(arm_context, FALSE);
    Box64EC_FatalEnterJit(0xec08);
}

void WINAPI ResetToConsistentState(EXCEPTION_RECORD* record, CONTEXT* context,
                                    ARM64_NT_CONTEXT* arm_context)
{
    CHPE_V2_CPU_AREA_INFO* area = Box64EC_GetCpuArea();
    x64emu_t* emu = area ? area->EmulatorData[EC_DATA_EMU] : NULL;
    box64ec_thr_t* state = Box64EC_GetThreadState();

    if (!record || !arm_context)
        return;
    /* Before the SMC check: a stale protection entry there must not retry the push forever */
    if (emu && record->ExceptionCode == STATUS_ACCESS_VIOLATION && record->NumberParameters >= 2 &&
        record->ExceptionInformation[0] == 1 &&
        Box64EC_HandleCallretStackFault(emu, (uintptr_t)record->ExceptionInformation[1], arm_context)) {
        Box64EC_ContinueNative(arm_context, FALSE);
        return;
    }
#ifdef DYNAREC
    handle_smc_write(record, arm_context, area, emu);
#endif
    if (!emu || !state || !context)
        return;
    if (InterlockedCompareExchange(&state->guest_exception_pending, 1, 1) == 1 &&
        record->ExceptionCode == state->guest_exception_code &&
        (uintptr_t)record->ExceptionAddress == state->guest_exception_address &&
        arm_context->Pc == state->guest_exception_rip &&
        arm_context->Sp == state->guest_exception_rsp &&
        InterlockedCompareExchange(&state->guest_exception_pending, 0, 1) == 1) {
        Box64EC_ResetAbandonedEmuRun(state, emu);
        merge_unmapped_context_from_emu(emu, (ARM64EC_NT_CONTEXT*)context);
        area->InSimulation = FALSE;
        return;
    }
    if (record->ExceptionCode == STATUS_ILLEGAL_INSTRUCTION &&
        arm_context->Pc == (uintptr_t)&ExitFunctionSuspendPoint) {
        arm_context->Pc = (uintptr_t)&ExitFunctionSuspendResumePoint;
        area->InSimulation = FALSE;
        if (area->SuspendDoorbell)
            *area->SuspendDoorbell = 0;
        Box64EC_ContinueNative(arm_context, FALSE);
        return;
    }
#ifdef DYNAREC
    /* FillBlock64 can read past the guest code it was given: as the Linux
     * signal handler does, go back into it to cut the block short. */
    if (fillblock_active && current_helper &&
        (uintptr_t)my_context->mutex_dyndump.OwningThread == (uintptr_t)GetTID())
        cancelFillBlock();
    /* In JIT code the guest registers are in the JIT map, and the deferred flags and
     * cached SIMD registers are described by the block. */
    dynablock_t* db = area->InSimulation ? find_jit_block(arm_context->Pc) : NULL;
    if (db && record->ExceptionCode == STATUS_ILLEGAL_INSTRUCTION && db->callret_size)
        handle_dirty_callret(arm_context, emu, db);
    if (db && record->ExceptionCode == STATUS_DATATYPE_MISALIGNMENT)
        handle_misaligned_lrcpc(arm_context, db, state);
    if (db) {
        uintptr_t x64pc = getX64Address(db, arm_context->Pc);
        jit_arm_context_to_emu(arm_context, emu);
        R_RIP = x64pc ? x64pc : arm_context->X27;
        if (x64pc) {
            adjustregs(emu, (void*)(uintptr_t)arm_context->Pc);
            adjust_arch(db, emu, arm_context, x64pc);
        }
        if (record->ExceptionCode == STATUS_ILLEGAL_INSTRUCTION) {
            if (*(uint32_t*)(uintptr_t)arm_context->Pc == BOX64EC_JIT_SUSPEND_OPCODE) {
                Box64EC_ResetAbandonedEmuRun(state, emu);
                Box64EC_HandleSuspend(emu);
                return;
            }
            if (Box64EC_IsPrivilegedInstruction((void*)(uintptr_t)R_RIP)) {
                record->ExceptionCode = STATUS_PRIVILEGED_INSTRUCTION;
                record->NumberParameters = 0;
            }
        }
    }
#endif
    if (area->InSimulation &&
        Box64EC_IsEmulatorStackAddress(arm_context->Sp)) {
        emu_to_context(emu, area->ContextAmd64);
        emu_to_context(emu, (ARM64EC_NT_CONTEXT*)context);
        rethrow_interpreter_fault(record, arm_context, emu);
    }
    merge_unmapped_context_from_emu(emu, (ARM64EC_NT_CONTEXT*)context);
}
