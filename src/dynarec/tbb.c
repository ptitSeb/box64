// SPDX-License-Identifier: MIT
//
// oneTBB publishes stack-allocated tasks with plain release stores, e.g. arena_slot::spawn() does:
//     task_pool_ptr[T] = &t;
//     tail.store(new_tail, std::memory_order_release);
// On x86 the release store is a plain MOV, and TSO store->store ordering makes the task's vptr visible before the publication.
// RISC architectures does not, so a thief can read the pointer but stale stack bytes at [task] and crash in task->execute().
// Detect oneTBB by its RTTI and emulate x86 TSO (strongmem=4) on its code range only.
// This is a self-maintained range list, similar to the volatile metadata ranges.

#include <stdint.h>
#include <string.h>

#include "box64context.h"
#include "elfs/elfloader_private.h"
#include "debug.h"
#include "env.h"
#include "tbb.h"

#define TBB_MAX_RANGES   8
#define TBB_MAX_CODE     64
#define TBB_MAX_SEGS     16
#define TBB_CLUSTER_GAP  0x10000ULL
#define TBB_RANGE_MARGIN 0x20000ULL

typedef struct tbb_range_s {
    uintptr_t start;
    uintptr_t end;
} tbb_range_t;

typedef struct tbb_seg_s {
    uintptr_t start;
    uintptr_t end;
    uint32_t flags;
} tbb_seg_t;

static tbb_range_t tbb_ranges[TBB_MAX_RANGES];
static int tbb_nranges = 0;
static int tbb_main_scanned = 0;
static int tbb_lock = 0;

// Itanium C++ ABI RTTI names of polymorphic oneTBB classes whose virtual functions live in the scheduler code (task pool, wait context).
static const char* tbb_rtti_names[] = { "N3tbb6detail2r110task_proxyE", "N3tbb6detail2d119wait_context_vertexE" };

#define TBB_ALIGN_UP(x, a)   (((x) + (uintptr_t)((a) - 1)) & ~(uintptr_t)((a) - 1))
#define TBB_ALIGN_DOWN(x, a) ((x) & ~(uintptr_t)((a) - 1))

static void tbb_lock_acquire(void)
{
    while (__atomic_exchange_n(&tbb_lock, 1, __ATOMIC_ACQUIRE)) {
        while (__atomic_load_n(&tbb_lock, __ATOMIC_RELAXED))
            ;
    }
}

static void tbb_lock_release(void)
{
    __atomic_store_n(&tbb_lock, 0, __ATOMIC_RELEASE);
}

static void* tbb_memmem(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len)
{
    if (!needle_len)
        return (void*)haystack;
    if (haystack_len < needle_len)
        return NULL;
    const uint8_t* h = (const uint8_t*)haystack;
    const uint8_t* n = (const uint8_t*)needle;
    const uint8_t* last = h + haystack_len - needle_len;
    for (const uint8_t* p = h; p <= last; ++p) {
        p = memchr(p, n[0], last - p + 1);
        if (!p)
            return NULL;
        if (!memcmp(p, n, needle_len))
            return (void*)p;
    }
    return NULL;
}

static int tbb_seg_find(const tbb_seg_t* segs, int n, uintptr_t addr)
{
    for (int i = 0; i < n; ++i)
        if (addr >= segs[i].start && addr < segs[i].end)
            return i;
    return -1;
}

static int tbb_seg_is_exec(const tbb_seg_t* segs, int n, uint64_t addr)
{
    for (int i = 0; i < n; ++i)
        if ((segs[i].flags & PF_X) && addr >= segs[i].start && addr < segs[i].end)
            return 1;
    return 0;
}

static int tbb_collect_segs(elfheader_t* h, tbb_seg_t* segs, int max)
{
    int n = 0;
    for (int i = 0; i < h->numPHEntries && n < max; ++i) {
        Elf64_Phdr* p = &h->PHEntries._64[i];
        if (p->p_type != PT_LOAD || !p->p_filesz)
            continue;
        uintptr_t start = (uintptr_t)h->image + (p->p_vaddr - h->vaddr);
        segs[n].start = start;
        segs[n].end = start + p->p_filesz;
        segs[n].flags = p->p_flags;
        ++n;
    }
    return n;
}

// Find a qword with the given value in 8-byte aligned positions.
// Returns the number of hits found, up to max.
static int tbb_find_qword(const tbb_seg_t* segs, int n, uint64_t value, uint32_t need_flags, uintptr_t* hits, int max)
{
    int count = 0;
    for (int i = 0; i < n && count < max; ++i) {
        if ((segs[i].flags & need_flags) != need_flags)
            continue;
        uintptr_t addr = TBB_ALIGN_UP(segs[i].start, 8);
        for (; addr + 8 <= segs[i].end && count < max; addr += 8)
            if (*(const uint64_t*)addr == value)
                hits[count++] = addr;
    }
    return count;
}

// Search for the vtable referencing this typeinfo and collect its code pointers.
// The typeinfo's own vptr may live in another module (e.g. libstdc++).
static int tbb_collect_vtable(const tbb_seg_t* segs, int nsegs, uintptr_t typeinfo, uintptr_t* code, int max)
{
    uintptr_t hits[8];
    int nhits = tbb_find_qword(segs, nsegs, typeinfo, PF_W, hits, 8);
    if (!nhits)
        nhits = tbb_find_qword(segs, nsegs, typeinfo, PF_R, hits, 8);
    for (int i = 0; i < nhits; ++i) {
        uintptr_t vtable = hits[i] + 8;
        int si = tbb_seg_find(segs, nsegs, hits[i]);
        if (si < 0)
            continue;
        uintptr_t candidates[8];
        int ncand = 0;
        int k = 0;
        for (; k < 8; ++k) {
            if (vtable + 8 * k + 8 > segs[si].end)
                break;
            uintptr_t fn = *(const uint64_t*)(vtable + 8 * k);
            if (!tbb_seg_is_exec(segs, nsegs, fn))
                break;
            candidates[ncand++] = fn;
        }
        printf_log(LOG_DEBUG, "TBB: vtable=%p entries=%d\n", (void*)vtable, k);
        // A vtable with less than 2 entries is not trustworthy.
        if (k < 2)
            continue;
        int ncode = 0;
        for (int m = 0; m < ncand && ncode < max; ++m) {
            int dup = 0;
            for (int c = 0; c < ncode; ++c)
                if (code[c] == candidates[m])
                    dup = 1;
            if (!dup)
                code[ncode++] = candidates[m];
        }
        return ncode;
    }
    return 0;
}

// Locate the RTTI typeinfo of name, then its vtable, and collect the code pointers of the virtual functions.
static int tbb_collect_code(elfheader_t* h, const char* name, uintptr_t* code, int max)
{
    tbb_seg_t segs[TBB_MAX_SEGS];
    int nsegs = tbb_collect_segs(h, segs, TBB_MAX_SEGS);
    if (nsegs < 2)
        return 0;

    size_t name_len = strlen(name);
    uintptr_t str = 0;
    // Prefer non-executable read-only data, but fall back to any readable segment if the RTTI name ended up in an executable one.
    for (int pass = 0; pass < 2 && !str; ++pass) {
        for (int i = 0; i < nsegs && !str; ++i) {
            if (!(segs[i].flags & PF_R))
                continue;
            if (!pass && (segs[i].flags & PF_X))
                continue;
            void* p = tbb_memmem((void*)segs[i].start, segs[i].end - segs[i].start, name, name_len);
            if (p)
                str = (uintptr_t)p;
        }
    }
    if (!str)
        return 0;

    printf_log(LOG_DEBUG, "TBB: %s str=%p\n", name, (void*)str);

    // Try every qword that looks like this typeinfo's __name pointer.
    // Some false positives can precede the real typeinfo object.
    uintptr_t hits[8];
    int nhits = tbb_find_qword(segs, nsegs, str, PF_W, hits, 8);
    if (!nhits)
        nhits = tbb_find_qword(segs, nsegs, str, PF_R, hits, 8);
    for (int i = 0; i < nhits; ++i) {
        int si = tbb_seg_find(segs, nsegs, hits[i]);
        if (si < 0 || hits[i] < segs[si].start + 8)
            continue;
        uintptr_t typeinfo = hits[i] - 8;
        if (*(const uint64_t*)(typeinfo + 8) != str)
            continue;
        printf_log(LOG_DEBUG, "TBB: %s typecand=%p\n", name, (void*)typeinfo);
        int ncode = tbb_collect_vtable(segs, nsegs, typeinfo, code, max);
        if (ncode)
            return ncode;
    }
    return 0;
}

static void tbb_add_range(uintptr_t start, uintptr_t end)
{
    start = TBB_ALIGN_DOWN(start, 0x1000);
    end = TBB_ALIGN_UP(end, 0x1000);
    if (end <= start)
        return;
    int nranges = __atomic_load_n(&tbb_nranges, __ATOMIC_RELAXED);
    for (int i = 0; i < nranges; ++i)
        if (start >= tbb_ranges[i].start && end <= tbb_ranges[i].end)
            return;
    if (nranges >= TBB_MAX_RANGES)
        return;
    tbb_ranges[nranges].start = start;
    tbb_ranges[nranges].end = end;
    __atomic_store_n(&tbb_nranges, nranges + 1, __ATOMIC_RELEASE);
    printf_log(LOG_DEBUG, "TBB detected, applying strongmem=4 to %p-%p\n", (void*)start, (void*)end);
}

static int tbb_add_cluster_ranges(uintptr_t* code, int ncode)
{
    if (ncode < 2)
        return 0;
    for (int i = 1; i < ncode; ++i) {
        uintptr_t value = code[i];
        int j = i - 1;
        while (j >= 0 && code[j] > value) {
            code[j + 1] = code[j];
            --j;
        }
        code[j + 1] = value;
    }
    int added = 0;
    int i = 0;
    while (i < ncode) {
        int j = i;
        while (j + 1 < ncode && code[j + 1] - code[i] <= TBB_CLUSTER_GAP)
            ++j;
        if (j > i) {
            uintptr_t start = code[i] > TBB_RANGE_MARGIN ? code[i] - TBB_RANGE_MARGIN : 0;
            uintptr_t end = code[j] + TBB_RANGE_MARGIN;
            int nranges = __atomic_load_n(&tbb_nranges, __ATOMIC_RELAXED);
            tbb_add_range(start, end);
            if (__atomic_load_n(&tbb_nranges, __ATOMIC_RELAXED) > nranges)
                ++added;
        }
        i = j + 1;
    }
    return added;
}

// Must be called with tbb_lock held.
static int tbb_scan_elf(elfheader_t* h)
{
    if (!h || box64_is32bits || !h->PHEntries._64)
        return 0;
    int added = 0;
    for (size_t i = 0; i < sizeof(tbb_rtti_names) / sizeof(tbb_rtti_names[0]); ++i) {
        uintptr_t code[TBB_MAX_CODE];
        int ncode = tbb_collect_code(h, tbb_rtti_names[i], code, TBB_MAX_CODE);
        added += tbb_add_cluster_ranges(code, ncode);
    }
    return added;
}

int TBBTSOAddElf(elfheader_t* h)
{
    if (!BOX64ENV(dynarec_tbb) || !h)
        return 0;
    tbb_lock_acquire();
    int added = tbb_scan_elf(h);
    tbb_lock_release();
    return added;
}

int TBBTSORangesContains(uintptr_t ip)
{
    if (!BOX64ENV(dynarec_tbb))
        return 0;

    // Lazily scan the main executable once for static oneTBB.
    // A dynamically loaded libtbb is added by TBBTSOAddElf when it is loaded.
    if (!__atomic_load_n(&tbb_main_scanned, __ATOMIC_ACQUIRE)) {
        tbb_lock_acquire();
        if (!__atomic_load_n(&tbb_main_scanned, __ATOMIC_RELAXED)) {
            if (my_context && my_context->elfs && my_context->elfsize > 0 && my_context->elfs[0]) {
                tbb_scan_elf(my_context->elfs[0]);
                __atomic_store_n(&tbb_main_scanned, 1, __ATOMIC_RELEASE);
            }
        }
        tbb_lock_release();
    }

    int nranges = __atomic_load_n(&tbb_nranges, __ATOMIC_ACQUIRE);
    for (int i = 0; i < nranges; ++i)
        if (ip >= tbb_ranges[i].start && ip < tbb_ranges[i].end)
            return 1;
    return 0;
}
