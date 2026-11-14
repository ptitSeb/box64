// SPDX-License-Identifier: MIT
#include <stdint.h>

#include "debug.h"
#include "env.h"
#include "khash.h"
#include "rbtree.h"
#include "volatile_fences.h"
#ifdef DYNAREC
#include "tbb.h"
#endif

#define TSO_STRONGMEM 4

static rbtree_t* metadata_ranges = NULL;
static rbtree_t* tso_ranges = NULL;
KHASH_SET_INIT_INT64(volatile_point);
static kh_volatile_point_t* metadata_points = NULL;

static volatile int fences_lock = 0;

static const int64_t unity_offsets[] = { 0x80, 0x84, 0xc0, 0xc4 };

static void fences_lock_acquire(void)
{
    while (__atomic_exchange_n(&fences_lock, 1, __ATOMIC_ACQUIRE)) {
        while (__atomic_load_n(&fences_lock, __ATOMIC_RELAXED))
            ;
    }
}

static void fences_lock_release(void)
{
    __atomic_store_n(&fences_lock, 0, __ATOMIC_RELEASE);
}

static int points_contains(kh_volatile_point_t* points, uintptr_t ip)
{
    if (!points)
        return 0;
    return kh_get(volatile_point, points, ip) != kh_end(points);
}

static kh_volatile_point_t* points_get(kh_volatile_point_t** points)
{
    return __atomic_load_n(points, __ATOMIC_ACQUIRE);
}

static void points_add(kh_volatile_point_t** points, uintptr_t ip)
{
    fences_lock_acquire();
    kh_volatile_point_t* set = __atomic_load_n(points, __ATOMIC_RELAXED);
    if (!set) {
        set = kh_init(volatile_point);
        __atomic_store_n(points, set, __ATOMIC_RELEASE);
    }
    int ret;
    kh_put(volatile_point, set, ip, &ret);
    fences_lock_release();
}

static int range_level(volatile_range_t kind)
{
    switch (kind) {
        case VOLATILE_RANGE_METADATA:
            return 0;
        case VOLATILE_RANGE_TSO:
            return TSO_STRONGMEM;
        default:
            return -1;
    }
}

static int range_tree_level(rbtree_t** ranges, uintptr_t ip)
{
    rbtree_t* tree = __atomic_load_n(ranges, __ATOMIC_ACQUIRE);
    if (!tree)
        return -1;
    uint64_t level = rb_get_64(tree, ip);
    return level ? (int)level - 1 : -1;
}

static int range_tree_add(rbtree_t** ranges, const char* name, uintptr_t start, uintptr_t end, int level)
{
    fences_lock_acquire();
    rbtree_t* tree = __atomic_load_n(ranges, __ATOMIC_RELAXED);
    if (!tree) {
        tree = rbtree_init(name);
        __atomic_store_n(ranges, tree, __ATOMIC_RELEASE);
    }
    int ret = rb_set_64(tree, start, end, (uint64_t)level + 1) == 0;
    fences_lock_release();
    return ret;
}

int VolatileFenceAddRange(volatile_range_t kind, uintptr_t start, uintptr_t end)
{
    if (end <= start)
        return 0;
    int level = range_level(kind);
    if (level < 0)
        return 0;

    rbtree_t** ranges;
    const char* name;
    switch (kind) {
        case VOLATILE_RANGE_METADATA:
            ranges = &metadata_ranges;
            name = "volatile metadata ranges";
            break;
        case VOLATILE_RANGE_TSO:
            ranges = &tso_ranges;
            name = "volatile TSO ranges";
            break;
        default:
            return 0;
    }

    int ret = range_tree_add(ranges, name, start, end, level);
    if (ret)
        printf_log(LOG_DEBUG, "Volatile range: %p-%p, level %d\n", (void*)start, (void*)end, level);
    return ret;
}

void VolatileFenceAddMetadataPoint(uintptr_t ip)
{
    points_add(&metadata_points, ip);
}

int VolatileFenceGetLevel(uintptr_t ip, int default_level)
{
    int level = range_tree_level(&metadata_ranges, ip);
    if (level >= 0)
        return level;

    if (BOX64ENV(dynarec_tbb)) {
#ifdef DYNAREC
        TBBTSOScanMainElf();
#endif
        level = range_tree_level(&tso_ranges, ip);
        if (level >= 0)
            return level;
    }

    return default_level;
}

int VolatileFenceNeedsFence(uintptr_t ip)
{
    return range_tree_level(&metadata_ranges, ip) >= 0 && points_contains(points_get(&metadata_points), ip);
}

int VolatileFenceNeedsUnityFence(uintptr_t ip, int64_t disp)
{
    if (!BOX64ENV(unity))
        return 0;
    if (range_tree_level(&metadata_ranges, ip) >= 0)
        return 0;
    for (size_t i = 0; i < sizeof(unity_offsets) / sizeof(unity_offsets[0]); ++i) {
        if (disp == unity_offsets[i])
            return 1;
    }
    return 0;
}
