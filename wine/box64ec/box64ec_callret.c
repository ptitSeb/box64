// SPDX-License-Identifier: MIT
#include <stddef.h>
#include <stdint.h>
#include <windows.h>
#include <ntstatus.h>

#include "box64ec_private.h"
#include "box64ec_layout.h"
#include "debug.h"
#include "emu/x64emu_private.h"
#include "wine/compiler.h"

_Static_assert(offsetof(x64emu_t, callret_sp) == BOX64EC_EMU_CALLRET_SP_OFFSET, "callret_sp");
_Static_assert(offsetof(x64emu_t, callret_sp_top) == BOX64EC_EMU_CALLRET_SP_TOP_OFFSET, "callret_sp_top");
_Static_assert(offsetof(x64emu_t, jit_frame) == BOX64EC_EMU_JIT_FRAME_OFFSET, "jit_frame");

#define BOX64EC_CALLRET_STACK_SIZE (2u * 1024u * 1024u)
/* The allocation granularity: on 16K or 64K host pages a 4K guard would share a host page with the stack */
#define BOX64EC_CALLRET_GUARD_SIZE 0x10000u
#define BOX64EC_CALLRET_COMMIT_STEP (64u * 1024u)

static NTSTATUS allocate_callret_pages(void** address, size_t* size,
                                       ULONG type, ULONG protection)
{
    NTSTATUS status;

    Box64EC_InternalMapEnter();
    status = NtAllocateVirtualMemory(NtCurrentProcess(), address, 0, size,
                                     type, protection);
    Box64EC_InternalMapLeave();
    return status;
}

/* As large as the thread stack, where Linux Box64 keeps its CALLRET pairs */
static size_t callret_stack_size(void)
{
    const NT_TIB* tib = (const NT_TIB*)NtCurrentTeb();
    MEMORY_BASIC_INFORMATION memory;
    uintptr_t top = (uintptr_t)tib->StackBase;
    size_t size;

    if (!top || NtQueryVirtualMemory(NtCurrentProcess(), (void*)(top - box64_pagesize),
                                     MemoryBasicInformation, &memory,
                                     sizeof(memory), NULL))
        return BOX64EC_CALLRET_STACK_SIZE;
    size = ALIGN(top - (uintptr_t)memory.AllocationBase);
    return size > BOX64EC_CALLRET_STACK_SIZE ? size : BOX64EC_CALLRET_STACK_SIZE;
}

NTSTATUS Box64EC_InitCallretStack(x64emu_t* emu)
{
    void* address = NULL;
    void* commit;
    size_t commit_size = box64_pagesize;
    size_t stack_size = callret_stack_size();
    size_t total_size = stack_size + 2 * BOX64EC_CALLRET_GUARD_SIZE;

    /* A guard page on each side, only the top page is committed */
    if (allocate_callret_pages(&address, &total_size, MEM_RESERVE, PAGE_NOACCESS))
        return STATUS_NO_MEMORY;
    emu->callret_stack = (uintptr_t)address + BOX64EC_CALLRET_GUARD_SIZE;
    emu->callret_size = stack_size;
    commit = (void*)(emu->callret_stack + stack_size - box64_pagesize);
    if (allocate_callret_pages(&commit, &commit_size, MEM_COMMIT, PAGE_READWRITE)) {
        Box64EC_FreeCallretStack(emu);
        return STATUS_NO_MEMORY;
    }
    Box64EC_ResetCallretStack(emu);
    return STATUS_SUCCESS;
}

void Box64EC_FreeCallretStack(x64emu_t* emu)
{
    void* address = (void*)(emu->callret_stack - BOX64EC_CALLRET_GUARD_SIZE);
    size_t size = 0;

    if (!emu->callret_stack)
        return;
    Box64EC_InternalMapEnter();
    NtFreeVirtualMemory(NtCurrentProcess(), &address, &size, MEM_RELEASE);
    Box64EC_InternalMapLeave();
    emu->callret_stack = 0;
    emu->callret_sp = 0;
    emu->callret_sp_top = 0;
    emu->callret_size = 0;
}

// Drop every activation but the first zero sentinel
void Box64EC_ResetCallretStack(x64emu_t* emu)
{
    emu->jit_frame = 0;
    if (!emu->callret_stack)
        return;
    emu->callret_sp_top = emu->callret_stack + emu->callret_size - 16;
    ((uintptr_t*)emu->callret_sp_top)[0] = 0;
    ((uintptr_t*)emu->callret_sp_top)[1] = 0;
    emu->callret_sp = emu->callret_sp_top;
}

/* A full stack drops the current activation's pairs (their returns take the jump
 * table): the faulting push (CALL, prolog sentinel or fast call record) is moved
 * just below the activation's sentinel. */
static int reset_full_callret_stack(x64emu_t* emu, uintptr_t address,
                                    ARM64_NT_CONTEXT* arm_ctx)
{
    uintptr_t top = emu->callret_sp_top;
    uint32_t insn = *(const uint32_t*)(uintptr_t)arm_ctx->Pc;
    unsigned rn = (insn >> 5) & 31;

    if (address >= emu->callret_stack || address < emu->callret_stack - 16 ||
        top < emu->callret_stack + 16 || rn == 31)
        return 0;
    if ((insn & 0xFFFF8000u) == 0xA9BF0000u && arm_ctx->X[rn] == address + 16)
        arm_ctx->X[rn] = top;
    else if ((insn & 0xFFFF8000u) == 0xA9000000u && arm_ctx->X[rn] == address) {
        unsigned rt = insn & 31;

        arm_ctx->X[rn] = top - 16;
        /* a moved fast native call record no longer sits on its CALLRET pair */
        if (rt != 31 && ((arm_ctx->X[rt] >> 32) & ~BOX64EC_FAST_RECORD_CALLRET) == BOX64EC_FAST_RECORD_TAG)
            arm_ctx->X[rt] &= ~((uint64_t)BOX64EC_FAST_RECORD_CALLRET << 32);
    } else
        return 0;
    emu->callret_sp = top;
    printf_log(LOG_DEBUG, "box64ec: private CALLRET stack full, dropping the "
               "current activation's return pairs\n");
    return 1;
}

/* Commit on demand: the faulting store has the live pointer, emu only its last spill */
int Box64EC_HandleCallretStackFault(x64emu_t* emu, uintptr_t address,
                                    ARM64_NT_CONTEXT* arm_ctx)
{
    MEMORY_BASIC_INFORMATION memory;
    uintptr_t base = emu->callret_stack;
    uintptr_t end = base + emu->callret_size;
    uintptr_t page;
    uintptr_t first;
    uintptr_t sp = emu->callret_sp;
    uint32_t insn;
    unsigned rn;
    void* commit;
    size_t commit_size;

    if (!base || address < base - 16 || address >= end)
        return 0;
    insn = *(const uint32_t*)(uintptr_t)arm_ctx->Pc;
    rn = (insn >> 5) & 31;
    if (rn != 31 && (insn & 0xFFFF8000u) == 0xA9BF0000u && arm_ctx->X[rn] == address + 16)
        sp = arm_ctx->X[rn];
    else if (rn != 31 && (insn & 0xFFFF8000u) == 0xA9000000u && arm_ctx->X[rn] == address)
        sp = arm_ctx->X[rn] + 16;
    if (sp < base + 16)
        return reset_full_callret_stack(emu, address, arm_ctx);
    if (address < sp - 16 || address >= sp)
        return 0;
    if (NtQueryVirtualMemory(NtCurrentProcess(), (void*)address,
                             MemoryBasicInformation, &memory, sizeof(memory),
                             NULL) ||
        memory.State != MEM_RESERVE)
        return 0;

    /* Each fault is a full exception round trip, grow in larger steps */
    page = address & ~(uintptr_t)(box64_pagesize - 1);
    first = page - base > BOX64EC_CALLRET_COMMIT_STEP - box64_pagesize
                ? page - (BOX64EC_CALLRET_COMMIT_STEP - box64_pagesize)
                : base;
    commit = (void*)first;
    commit_size = page + box64_pagesize - first;
    return allocate_callret_pages(&commit, &commit_size, MEM_COMMIT, PAGE_READWRITE) == STATUS_SUCCESS;
}
