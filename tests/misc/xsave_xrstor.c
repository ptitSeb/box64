#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct xmm128_s {
    uint64_t q[2];
} xmm128_t;

static const xmm128_t expected_state[8] = {
    {{0x8881828384858687ULL, 0x1811121314151617ULL}},
    {{0x9992939495969798ULL, 0x2922232425262728ULL}},
    {{0xAAA3A4A5A6A7A8A9ULL, 0x3A33343536373839ULL}},
    {{0xBBB4B5B6B7B8B9BAULL, 0x4B4445464748494AULL}},
    {{0xCCC5C6C7C8C9CACBULL, 0x5C55565758595A5BULL}},
    {{0xDDD6D7D8D9DADBDCULL, 0x6D666768696A6B6CULL}},
    {{0xEEE7E8E9EAEBECEDULL, 0x7E7778797A7B7C7DULL}},
    {{0xFFF8F9FAFBFCFDFEULL, 0x8F88898A8B8C8D8EULL}},
};

static const xmm128_t poisoned_state[8] = {
    {{0xF0F1F2F3F4F5F6F7ULL, 0x0706050403020100ULL}},
    {{0xE0E1E2E3E4E5E6E7ULL, 0x1716151413121110ULL}},
    {{0xD0D1D2D3D4D5D6D7ULL, 0x2726252423222120ULL}},
    {{0xC0C1C2C3C4C5C6C7ULL, 0x3736353433323130ULL}},
    {{0xB0B1B2B3B4B5B6B7ULL, 0x4746454443424140ULL}},
    {{0xA0A1A2A3A4A5A6A7ULL, 0x5756555453525150ULL}},
    {{0x9091929394959697ULL, 0x6766656463626160ULL}},
    {{0x8081828384858687ULL, 0x7776757473727170ULL}},
};

static inline uint64_t read_xcr0(void)
{
    uint32_t eax, edx;
    __asm__ volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
    return ((uint64_t)edx << 32) | eax;
}

static inline void xsave_plain(void* area, uint64_t mask)
{
    uint32_t eax = (uint32_t)mask;
    uint32_t edx = (uint32_t)(mask >> 32);
    __asm__ volatile("xsave (%0)" : : "r"(area), "a"(eax), "d"(edx) : "memory");
}

static inline void xrstor_plain(const void* area, uint64_t mask)
{
    uint32_t eax = (uint32_t)mask;
    uint32_t edx = (uint32_t)(mask >> 32);
    __asm__ volatile("xrstor (%0)" : : "r"(area), "a"(eax), "d"(edx) : "memory");
}

static inline void xsave64_rexw(void* area, uint64_t mask)
{
    uint32_t eax = (uint32_t)mask;
    uint32_t edx = (uint32_t)(mask >> 32);
    __asm__ volatile(
        "mov %0, %%rdi\n\t"
        ".byte 0x48, 0x0f, 0xae, 0x27"
        :
        : "r"(area), "a"(eax), "d"(edx)
        : "rdi", "memory");
}

static inline void xrstor64_rexw(const void* area, uint64_t mask)
{
    uint32_t eax = (uint32_t)mask;
    uint32_t edx = (uint32_t)(mask >> 32);
    __asm__ volatile(
        "mov %0, %%rdi\n\t"
        ".byte 0x48, 0x0f, 0xae, 0x2f"
        :
        : "r"(area), "a"(eax), "d"(edx)
        : "rdi", "memory");
}

static inline void load_xmm8_15(const xmm128_t* values)
{
    __asm__ volatile(
        "movdqu   0(%0), %%xmm8\n\t"
        "movdqu  16(%0), %%xmm9\n\t"
        "movdqu  32(%0), %%xmm10\n\t"
        "movdqu  48(%0), %%xmm11\n\t"
        "movdqu  64(%0), %%xmm12\n\t"
        "movdqu  80(%0), %%xmm13\n\t"
        "movdqu  96(%0), %%xmm14\n\t"
        "movdqu 112(%0), %%xmm15\n\t"
        :
        : "r"(values)
        : "memory", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
}

static inline void store_xmm8_15(xmm128_t* values)
{
    __asm__ volatile(
        "movdqu %%xmm8,    0(%0)\n\t"
        "movdqu %%xmm9,   16(%0)\n\t"
        "movdqu %%xmm10,  32(%0)\n\t"
        "movdqu %%xmm11,  48(%0)\n\t"
        "movdqu %%xmm12,  64(%0)\n\t"
        "movdqu %%xmm13,  80(%0)\n\t"
        "movdqu %%xmm14,  96(%0)\n\t"
        "movdqu %%xmm15, 112(%0)\n\t"
        :
        : "r"(values)
        : "memory");
}

static void dump_reg(const char* name, const xmm128_t* value)
{
    printf("  %s = 0x%016" PRIx64 "%016" PRIx64 "\n", name, value->q[1], value->q[0]);
}

static int compare_xmm8_15(const char* name)
{
    static const char* reg_names[8] = {
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
    };
    xmm128_t got[8];
    int failed = 0;

    store_xmm8_15(got);
    for (int i = 0; i < 8; ++i) {
        if (memcmp(&got[i], &expected_state[i], sizeof(got[i])) != 0) {
            if (!failed)
                printf("%s: FAIL\n", name);
            failed = 1;
            dump_reg(reg_names[i], &got[i]);
        }
    }
    if (!failed)
        printf("%s: OK\n", name);
    return failed;
}

static int run_roundtrip(const char* name, void (*xsave_fn)(void*, uint64_t), void (*xrstor_fn)(const void*, uint64_t))
{
    struct __attribute__((aligned(64))) {
        unsigned char bytes[1024];
    } area;

    // XRSTOR validates reserved bytes in the XSAVE image, so start from a
    // zeroed buffer instead of poison-filled memory.
    memset(&area, 0, sizeof(area));
    load_xmm8_15(expected_state);
    xsave_fn(&area, 0x3);
    load_xmm8_15(poisoned_state);
    xrstor_fn(&area, 0x3);
    return compare_xmm8_15(name);
}

int main(void)
{
    int failed = 0;

    failed |= run_roundtrip("XSAVE/XRSTOR restores XMM8..XMM15", xsave_plain, xrstor_plain);
    failed |= run_roundtrip("XSAVE64/XRSTOR64 restores XMM8..XMM15", xsave64_rexw, xrstor64_rexw);

    return failed;
}
