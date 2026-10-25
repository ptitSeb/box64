// SPDX-License-Identifier: MIT
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define BLOCK_VALUES 2048
#define N16          0x10000u
#define N24          0x1000000u
#define STEP24       31u
#define NRAND64      50000u
#define NRAND32      12500u
#define NPOW         (64u * 9u)

enum {
    F_MOV64,
    F_MOV32,
    F_MOVSXD,
    F_STORE32,
    F_COUNT
};

static uint8_t* pool;
static size_t pool_used;
static size_t pool_size;
static uint64_t checked[F_COUNT];
static uint64_t failures;

static void* alloc_code(size_t bytes)
{
    void* p;
    if (pool_used + bytes > pool_size) {
        fprintf(stderr, "code pool exhausted\n");
        exit(1);
    }
    p = pool + pool_used;
    pool_used += bytes;
    return p;
}

static size_t emit_mov64(uint8_t* p, uint64_t v, int32_t disp)
{
    p[0] = 0x48;
    p[1] = 0xB8;
    memcpy(p + 2, &v, 8);
    p[10] = 0x48;
    p[11] = 0x89;
    p[12] = 0x87;
    memcpy(p + 13, &disp, 4);
    return 17;
}

static size_t emit_mov32(uint8_t* p, uint32_t v, int32_t disp)
{
    p[0] = 0xB8;
    memcpy(p + 1, &v, 4);
    p[5] = 0x48;
    p[6] = 0x89;
    p[7] = 0x87;
    memcpy(p + 8, &disp, 4);
    return 12;
}

static size_t emit_movsxd(uint8_t* p, int32_t v, int32_t disp)
{
    p[0] = 0x48;
    p[1] = 0xC7;
    p[2] = 0xC0;
    memcpy(p + 3, &v, 4);
    p[7] = 0x48;
    p[8] = 0x89;
    p[9] = 0x87;
    memcpy(p + 10, &disp, 4);
    return 14;
}

static size_t emit_store32(uint8_t* p, int32_t v, int32_t disp)
{
    p[0] = 0x48;
    p[1] = 0xC7;
    p[2] = 0x87;
    memcpy(p + 3, &disp, 4);
    memcpy(p + 7, &v, 4);
    return 11;
}

static size_t emit_value(int form, uint8_t* p, uint64_t v, int32_t disp)
{
    switch (form) {
        case F_MOV64:
            return emit_mov64(p, v, disp);
        case F_MOV32:
            return emit_mov32(p, (uint32_t)v, disp);
        case F_MOVSXD:
            return emit_movsxd(p, (int32_t)v, disp);
        default:
            return emit_store32(p, (int32_t)v, disp);
    }
}

static uint64_t expected(int form, uint64_t v)
{
    switch (form) {
        case F_MOV64:
            return v;
        case F_MOV32:
            return (uint32_t)v;
        default:
            return (uint64_t)(int64_t)(int32_t)v;
    }
}

typedef void (*block_fn)(uint64_t*);

static void run_values(int form, const uint64_t* values, size_t count)
{
    size_t nblocks = (count + BLOCK_VALUES - 1) / BLOCK_VALUES;
    size_t i, b;
    uint8_t* start;
    uint8_t* p;
    uint8_t** blocks;

    start = alloc_code(count * 17 + nblocks);
    blocks = malloc(nblocks * sizeof(*blocks));
    if (!blocks) exit(1);

    p = start;
    for (b = 0; b < nblocks; ++b) {
        size_t base = b * BLOCK_VALUES;
        size_t n = count - base;
        if (n > BLOCK_VALUES) n = BLOCK_VALUES;
        blocks[b] = p;
        for (i = 0; i < n; ++i)
            p += emit_value(form, p, values[base + i], (int32_t)(i * 8));
        *p++ = 0xC3;
    }
    __builtin___clear_cache((char*)start, (char*)p);

    for (b = 0; b < nblocks; ++b) {
        size_t base = b * BLOCK_VALUES;
        size_t n = count - base;
        uint64_t out[BLOCK_VALUES];
        block_fn fn = (block_fn)blocks[b];
        if (n > BLOCK_VALUES) n = BLOCK_VALUES;
        fn(out);
        for (i = 0; i < n; ++i) {
            uint64_t want = expected(form, values[base + i]);
            checked[form]++;
            if (out[i] != want) {
                if (failures < 20)
                    printf("FAIL form=%d value=%016llx expected=%016llx got=%016llx\n", form,
                        (unsigned long long)values[base + i], (unsigned long long)want,
                        (unsigned long long)out[i]);
                failures++;
            }
        }
    }

    free(blocks);
}

static uint64_t rng_state = 0x123456789abcdefULL;

static uint64_t rng(void)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 7;
    rng_state ^= rng_state << 17;
    return rng_state;
}

static const uint64_t special[] = {
    0, 1, 2, 3, 0x7F, 0x80, 0xFF, 0x100, 0x7FFF, 0x8000, 0xFFFF, 0x10000,
    0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x100000000,
    0x7FFFFFFFFFFFFFFF, 0x8000000000000000, 0xFFFFFFFFFFFFFFFF,
    0xFFFF0000FFFF0000, 0x0000FFFF0000FFFF, 0x00FF00FF00FF00FF, 0xFF00FF00FF00FF00,
    0x5555555555555555, 0xAAAAAAAAAAAAAAAA, 0x3333333333333333, 0xCCCCCCCCCCCCCCCC,
    0x0F0F0F0F0F0F0F0F, 0xF0F0F0F0F0F0F0F0, 0x0101010101010101, 0x8080808080808080,
    0x123456789ABCDEF0, 0xFEDCBA9876543210, 0xDEADBEEFCAFEBABE,
    0x00000000FFFFFFFF, 0xFFFFFFFF00000000, 0x0000000100000001, 0xFFFFFFFF00000001,
    0x7FFFFFFFFFFFFFFE, 0x8000000000000001, 0xFFFFFFFFFFFFFFFE, 0xFFFFFFFEFFFFFFFF
};

static void print_group(int form)
{
    static const char* name[F_COUNT] = {
        "mov r64, imm64",
        "mov r32, imm32",
        "mov r64, imm32",
        "mov qword [rdi+disp], imm32"
    };
    printf("%s: %llu values ok\n", name[form], (unsigned long long)checked[form]);
}

int main(int argc, char** argv)
{
    int quick = argc > 1 && !strcmp(argv[1], "quick");
    uint64_t* v16;
    uint64_t* v24;
    uint64_t* r64;
    uint64_t* r32;
    uint64_t* kpow;
    uint64_t step24 = quick ? 0x400 : STEP24;
    size_t nrand64 = quick ? 2000 : NRAND64;
    size_t nrand32 = quick ? 1000 : NRAND32;
    size_t n24 = (N24 - 1) / step24 + 1;
    uint64_t i, k;
    int d;

    pool_size = 64u << 20;
    pool = mmap(NULL, pool_size, PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pool == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    v16 = malloc(N16 * sizeof(uint64_t));
    v24 = malloc(n24 * sizeof(uint64_t));
    r64 = malloc(nrand64 * sizeof(uint64_t));
    r32 = malloc(nrand32 * sizeof(uint64_t));
    kpow = malloc(NPOW * sizeof(uint64_t));
    if (!v16 || !v24 || !r64 || !r32 || !kpow) {
        perror("malloc");
        return 1;
    }

    for (i = 0; i < N16; ++i)
        v16[i] = i;
    for (i = 0; i < n24; ++i)
        v24[i] = i * step24;
    for (i = 0; i < nrand64; ++i)
        r64[i] = rng();
    for (i = 0; i < nrand32; ++i)
        r32[i] = (uint32_t)rng();
    i = 0;
    for (k = 0; k < 64; ++k) {
        for (d = -0x1000; d <= 0x1000; d += 0x400) {
            kpow[i] = (UINT64_C(1) << k) + (uint64_t)(int64_t)d;
            ++i;
        }
    }

    run_values(F_MOV64, v16, N16);
    run_values(F_MOV64, r64, nrand64);
    run_values(F_MOV64, r32, nrand32);
    run_values(F_MOV64, kpow, NPOW);
    run_values(F_MOV64, special, sizeof(special) / sizeof(special[0]));

    run_values(F_MOV32, v16, N16);
    run_values(F_MOV32, v24, n24);
    run_values(F_MOV32, r32, nrand32);
    run_values(F_MOV32, kpow, NPOW);
    run_values(F_MOV32, special, sizeof(special) / sizeof(special[0]));

    run_values(F_MOVSXD, v16, N16);
    run_values(F_MOVSXD, v24, n24);
    run_values(F_MOVSXD, r32, nrand32);
    run_values(F_MOVSXD, kpow, NPOW);
    run_values(F_MOVSXD, special, sizeof(special) / sizeof(special[0]));

    run_values(F_STORE32, v16, N16);
    run_values(F_STORE32, kpow, NPOW);
    run_values(F_STORE32, special, sizeof(special) / sizeof(special[0]));

    for (d = 0; d < F_COUNT; ++d)
        print_group(d);
    printf("move immediate: %s\n", failures ? "FAILED" : "PASS");

    free(v16);
    free(v24);
    free(r64);
    free(r32);
    free(kpow);
    return failures ? 1 : 0;
}
