// SPDX-License-Identifier: MIT
// build with: gcc -O0 -g -Wall -Wextra -msse4.1 -mavx -mfma test36.c -o test36

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MXCSR_DEFAULT 0x1f80
#define MXCSR_RC_SHIFT 13
#define MXCSR_RC_MASK (3U << MXCSR_RC_SHIFT)

typedef struct __attribute__((aligned(16))) {
    float v[4];
} f32x4;

typedef struct __attribute__((aligned(16))) {
    double v[2];
} f64x2;

typedef struct __attribute__((aligned(16))) {
    int32_t v[4];
} i32x4;

typedef struct __attribute__((aligned(32))) {
    int32_t v[8];
} i32x8;

typedef struct __attribute__((aligned(32))) {
    float v[8];
} f32x8;

typedef struct __attribute__((aligned(32))) {
    double v[4];
} f64x4;

static int failures;

static uint32_t float_bits(float value)
{
    uint32_t bits;

    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static uint64_t double_bits(double value)
{
    uint64_t bits;

    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static void check_u32(const char* name, uint32_t actual, uint32_t expected)
{
    if (actual != expected) {
        printf("%s: got %08x, expected %08x\n", name, actual, expected);
        ++failures;
    }
}

static void check_u64(const char* name, uint64_t actual, uint64_t expected)
{
    if (actual != expected) {
        printf("%s: got %016llx, expected %016llx\n", name,
               (unsigned long long)actual, (unsigned long long)expected);
        ++failures;
    }
}

static void load_mxcsr(uint32_t value)
{
    __asm__ volatile("ldmxcsr %0" : : "m"(value) : "memory");
}

static uint32_t store_mxcsr(void)
{
    uint32_t value;

    __asm__ volatile("stmxcsr %0" : "=m"(value) : : "memory");
    return value;
}

static float addss(float a, float b)
{
    float result;

    __asm__ volatile(
        "movss %1, %%xmm0\n\t"
        "addss %2, %%xmm0\n\t"
        "movss %%xmm0, %0"
        : "=m"(result)
        : "m"(a), "m"(b)
        : "xmm0");
    return result;
}

static double addsd(double a, double b)
{
    double result;

    __asm__ volatile(
        "movsd %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0"
        : "=m"(result)
        : "m"(a), "m"(b)
        : "xmm0");
    return result;
}

static void addps(const f32x4* a, const f32x4* b, f32x4* result)
{
    __asm__ volatile(
        "movups %1, %%xmm0\n\t"
        "addps %2, %%xmm0\n\t"
        "movups %%xmm0, %0"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "xmm0");
}

static void addpd(const f64x2* a, const f64x2* b, f64x2* result)
{
    __asm__ volatile(
        "movupd %1, %%xmm0\n\t"
        "addpd %2, %%xmm0\n\t"
        "movupd %%xmm0, %0"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "xmm0");
}

static void haddps(const f32x4* a, const f32x4* b, f32x4* result)
{
    __asm__ volatile(
        "movups %1, %%xmm0\n\t"
        "movups %2, %%xmm1\n\t"
        "haddps %%xmm1, %%xmm0\n\t"
        "movups %%xmm0, %0"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "xmm0", "xmm1");
}

static void dppd(const f64x2* a, const f64x2* b, f64x2* result)
{
    __asm__ volatile(
        "movupd %1, %%xmm0\n\t"
        "movupd %2, %%xmm1\n\t"
        "dppd $0x31, %%xmm1, %%xmm0\n\t"
        "movupd %%xmm0, %0"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "xmm0", "xmm1");
}

static void vhaddps(const f32x8* a, const f32x8* b, f32x8* result)
{
    __asm__ volatile(
        "vmovups %1, %%ymm0\n\t"
        "vhaddps %2, %%ymm0, %%ymm0\n\t"
        "vmovups %%ymm0, %0\n\t"
        "vzeroupper"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "ymm0");
}

static void vhaddpd(const f64x4* a, const f64x4* b, f64x4* result)
{
    __asm__ volatile(
        "vmovupd %1, %%ymm0\n\t"
        "vhaddpd %2, %%ymm0, %%ymm0\n\t"
        "vmovupd %%ymm0, %0\n\t"
        "vzeroupper"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "ymm0");
}

static void vhsubps(const f32x8* a, const f32x8* b, f32x8* result)
{
    __asm__ volatile(
        "vmovups %1, %%ymm0\n\t"
        "vhsubps %2, %%ymm0, %%ymm0\n\t"
        "vmovups %%ymm0, %0\n\t"
        "vzeroupper"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "ymm0");
}

static void vhsubpd(const f64x4* a, const f64x4* b, f64x4* result)
{
    __asm__ volatile(
        "vmovupd %1, %%ymm0\n\t"
        "vhsubpd %2, %%ymm0, %%ymm0\n\t"
        "vmovupd %%ymm0, %0\n\t"
        "vzeroupper"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "ymm0");
}

static void vaddsubps(const f32x8* a, const f32x8* b, f32x8* result)
{
    __asm__ volatile(
        "vmovups %1, %%ymm0\n\t"
        "vaddsubps %2, %%ymm0, %%ymm0\n\t"
        "vmovups %%ymm0, %0\n\t"
        "vzeroupper"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "ymm0");
}

static void vaddsubpd(const f64x4* a, const f64x4* b, f64x4* result)
{
    __asm__ volatile(
        "vmovupd %1, %%ymm0\n\t"
        "vaddsubpd %2, %%ymm0, %%ymm0\n\t"
        "vmovupd %%ymm0, %0\n\t"
        "vzeroupper"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "ymm0");
}

static void vdpps(const f32x8* a, const f32x8* b, f32x8* result)
{
    __asm__ volatile(
        "vmovups %1, %%ymm0\n\t"
        "vdpps $0xf1, %2, %%ymm0, %%ymm0\n\t"
        "vmovups %%ymm0, %0\n\t"
        "vzeroupper"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "ymm0");
}

static void vdppd(const f64x2* a, const f64x2* b, f64x2* result)
{
    __asm__ volatile(
        "vmovupd %1, %%xmm0\n\t"
        "vdppd $0x31, %2, %%xmm0, %%xmm0\n\t"
        "vmovupd %%xmm0, %0"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "xmm0");
}

static float cvtsi2ss(int32_t value)
{
    float result;

    __asm__ volatile(
        "pxor %%xmm0, %%xmm0\n\t"
        "cvtsi2ss %1, %%xmm0\n\t"
        "movss %%xmm0, %0"
        : "=m"(result)
        : "r"(value)
        : "xmm0");
    return result;
}

static float cvtsd2ss(double value)
{
    float result;

    __asm__ volatile(
        "pxor %%xmm0, %%xmm0\n\t"
        "cvtsd2ss %1, %%xmm0\n\t"
        "movss %%xmm0, %0"
        : "=m"(result)
        : "m"(value)
        : "xmm0");
    return result;
}

static void cvtdq2ps(const i32x4* value, f32x4* result)
{
    __asm__ volatile(
        "movdqu %1, %%xmm0\n\t"
        "cvtdq2ps %%xmm0, %%xmm0\n\t"
        "movups %%xmm0, %0"
        : "=m"(*result)
        : "m"(*value)
        : "xmm0");
}

static void cvtpd2ps(const f64x2* value, f32x4* result)
{
    __asm__ volatile(
        "movupd %1, %%xmm0\n\t"
        "cvtpd2ps %%xmm0, %%xmm0\n\t"
        "movups %%xmm0, %0"
        : "=m"(*result)
        : "m"(*value)
        : "xmm0");
}

static void vcvtdq2ps(const i32x8* value, f32x8* result)
{
    __asm__ volatile(
        "vmovdqu %1, %%ymm0\n\t"
        "vcvtdq2ps %%ymm0, %%ymm0\n\t"
        "vmovups %%ymm0, %0\n\t"
        "vzeroupper"
        : "=m"(*result)
        : "m"(*value)
        : "ymm0");
}

static void vcvtpd2ps(const f64x4* value, f32x4* result)
{
    __asm__ volatile(
        "vmovupd %1, %%ymm0\n\t"
        "vcvtpd2ps %%ymm0, %%xmm0\n\t"
        "vmovups %%xmm0, %0\n\t"
        "vzeroupper"
        : "=m"(*result)
        : "m"(*value)
        : "ymm0");
}

static float vcvtsi2ss(int32_t value)
{
    float result;

    __asm__ volatile(
        "vxorps %%xmm0, %%xmm0, %%xmm0\n\t"
        "vcvtsi2ss %1, %%xmm0, %%xmm0\n\t"
        "vmovss %%xmm0, %0"
        : "=m"(result)
        : "r"(value)
        : "xmm0");
    return result;
}

static double vcvtsi2sd(int64_t value)
{
    double result;

    __asm__ volatile(
        "vxorpd %%xmm0, %%xmm0, %%xmm0\n\t"
        "vcvtsi2sdq %1, %%xmm0, %%xmm0\n\t"
        "vmovsd %%xmm0, %0"
        : "=m"(result)
        : "r"(value)
        : "xmm0");
    return result;
}

static float vcvtsd2ss(double value)
{
    float result;

    __asm__ volatile(
        "vxorps %%xmm0, %%xmm0, %%xmm0\n\t"
        "vcvtsd2ss %1, %%xmm0, %%xmm0\n\t"
        "vmovss %%xmm0, %0"
        : "=m"(result)
        : "m"(value)
        : "xmm0");
    return result;
}

static float vfmadd132ss(float dest, float src1, float src2)
{
    float result;

    __asm__ volatile(
        "vmovss %1, %%xmm0\n\t"
        "vmovss %2, %%xmm1\n\t"
        "vmovss %3, %%xmm2\n\t"
        "vfmadd132ss %%xmm2, %%xmm1, %%xmm0\n\t"
        "vmovss %%xmm0, %0"
        : "=m"(result)
        : "m"(dest), "m"(src1), "m"(src2)
        : "xmm0", "xmm1", "xmm2");
    return result;
}

static double vfmsub132sd(double dest, double src1, double src2)
{
    double result;

    __asm__ volatile(
        "vmovsd %1, %%xmm0\n\t"
        "vmovsd %2, %%xmm1\n\t"
        "vmovsd %3, %%xmm2\n\t"
        "vfmsub132sd %%xmm2, %%xmm1, %%xmm0\n\t"
        "vmovsd %%xmm0, %0"
        : "=m"(result)
        : "m"(dest), "m"(src1), "m"(src2)
        : "xmm0", "xmm1", "xmm2");
    return result;
}

static float vfnmadd213ss(float dest, float src1, float src2)
{
    float result;

    __asm__ volatile(
        "vmovss %1, %%xmm0\n\t"
        "vmovss %2, %%xmm1\n\t"
        "vmovss %3, %%xmm2\n\t"
        "vfnmadd213ss %%xmm2, %%xmm1, %%xmm0\n\t"
        "vmovss %%xmm0, %0"
        : "=m"(result)
        : "m"(dest), "m"(src1), "m"(src2)
        : "xmm0", "xmm1", "xmm2");
    return result;
}

static double vfnmsub213sd(double dest, double src1, double src2)
{
    double result;

    __asm__ volatile(
        "vmovsd %1, %%xmm0\n\t"
        "vmovsd %2, %%xmm1\n\t"
        "vmovsd %3, %%xmm2\n\t"
        "vfnmsub213sd %%xmm2, %%xmm1, %%xmm0\n\t"
        "vmovsd %%xmm0, %0"
        : "=m"(result)
        : "m"(dest), "m"(src1), "m"(src2)
        : "xmm0", "xmm1", "xmm2");
    return result;
}

static float vfmadd231ss(float dest, float src1, float src2)
{
    float result;

    __asm__ volatile(
        "vmovss %1, %%xmm0\n\t"
        "vmovss %2, %%xmm1\n\t"
        "vmovss %3, %%xmm2\n\t"
        "vfmadd231ss %%xmm2, %%xmm1, %%xmm0\n\t"
        "vmovss %%xmm0, %0"
        : "=m"(result)
        : "m"(dest), "m"(src1), "m"(src2)
        : "xmm0", "xmm1", "xmm2");
    return result;
}

static double vfnmadd231sd(double dest, double src1, double src2)
{
    double result;

    __asm__ volatile(
        "vmovsd %1, %%xmm0\n\t"
        "vmovsd %2, %%xmm1\n\t"
        "vmovsd %3, %%xmm2\n\t"
        "vfnmadd231sd %%xmm2, %%xmm1, %%xmm0\n\t"
        "vmovsd %%xmm0, %0"
        : "=m"(result)
        : "m"(dest), "m"(src1), "m"(src2)
        : "xmm0", "xmm1", "xmm2");
    return result;
}

static float vaddss(float a, float b)
{
    float result;

    __asm__ volatile(
        "vmovss %1, %%xmm0\n\t"
        "vaddss %2, %%xmm0, %%xmm0\n\t"
        "vmovss %%xmm0, %0"
        : "=m"(result)
        : "m"(a), "m"(b)
        : "xmm0");
    return result;
}

static double vaddsd(double a, double b)
{
    double result;

    __asm__ volatile(
        "vmovsd %1, %%xmm0\n\t"
        "vaddsd %2, %%xmm0, %%xmm0\n\t"
        "vmovsd %%xmm0, %0"
        : "=m"(result)
        : "m"(a), "m"(b)
        : "xmm0");
    return result;
}

static void vaddps(const f32x8* a, const f32x8* b, f32x8* result)
{
    __asm__ volatile(
        "vmovups %1, %%ymm0\n\t"
        "vaddps %2, %%ymm0, %%ymm0\n\t"
        "vmovups %%ymm0, %0\n\t"
        "vzeroupper"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "ymm0");
}

static void vaddpd(const f64x4* a, const f64x4* b, f64x4* result)
{
    __asm__ volatile(
        "vmovupd %1, %%ymm0\n\t"
        "vaddpd %2, %%ymm0, %%ymm0\n\t"
        "vmovupd %%ymm0, %0\n\t"
        "vzeroupper"
        : "=m"(*result)
        : "m"(*a), "m"(*b)
        : "ymm0");
}

int main(void)
{
    static const uint32_t positive_expected[4] = {
        0x3f800000, 0x3f800000, 0x3f800001, 0x3f800000
    };
    static const uint32_t negative_expected[4] = {
        0xbf800000, 0xbf800001, 0xbf800000, 0xbf800000
    };
    const uint32_t saved_mxcsr = store_mxcsr();
    const float float_half_ulp = 0x1p-24f;
    const double double_half_ulp = 0x1p-53;
    const double float_midpoint = 1.0 + 0x1p-24;
    f32x4 sf_a = {{ 1.0f, 1.0f, 1.0f, 1.0f }};
    f32x4 sf_b = {{ 0x1p-24f, 0x1p-24f, 0x1p-24f, 0x1p-24f }};
    f32x4 sf_result;
    f32x4 hadd_a = {{ 1.0f, 0x1p-24f, 0.0f, 0.0f }};
    f32x4 hadd_b = {{ 0.0f, 0.0f, 0.0f, 0.0f }};
    f64x2 sd_a = {{ 1.0, 1.0 }};
    f64x2 sd_b = {{ 0x1p-53, 0x1p-53 }};
    f64x2 sd_result;
    f64x2 dpp_a = {{ 1.0, 1.0 }};
    f64x2 dpp_b = {{ 1.0, 0x1p-53 }};
    i32x4 integers = {{ 16777217, 0, 0, 0 }};
    i32x8 avx_integers = {{ 16777217, 0, 0, 0,
                            16777217, 0, 0, 0 }};
    f64x2 doubles = {{ 1.0 + 0x1p-24, 0.0 }};
    f64x4 avx_doubles = {{ 1.0 + 0x1p-24, 0.0,
                           1.0 + 0x1p-24, 0.0 }};
    f32x8 avx_sf_a = {{ 1.0f, 1.0f, 1.0f, 1.0f,
                        1.0f, 1.0f, 1.0f, 1.0f }};
    f32x8 avx_sf_b = {{ 0x1p-24f, 0x1p-24f, 0x1p-24f, 0x1p-24f,
                        0x1p-24f, 0x1p-24f, 0x1p-24f, 0x1p-24f }};
    f32x8 avx_sf_result;
    f64x4 avx_sd_a = {{ 1.0, 1.0, 1.0, 1.0 }};
    f64x4 avx_sd_b = {{ 0x1p-53, 0x1p-53, 0x1p-53, 0x1p-53 }};
    f64x4 avx_sd_result;
    f32x8 avx_hadd_sf = {{ 1.0f, 0x1p-24f, 1.0f, 0x1p-24f,
                           1.0f, 0x1p-24f, 1.0f, 0x1p-24f }};
    f64x4 avx_hadd_sd = {{ 1.0, 0x1p-53, 1.0, 0x1p-53 }};
    f32x8 avx_hsub_sf = {{ 1.0f, -0x1p-24f, 1.0f, -0x1p-24f,
                           1.0f, -0x1p-24f, 1.0f, -0x1p-24f }};
    f64x4 avx_hsub_sd = {{ 1.0, -0x1p-53, 1.0, -0x1p-53 }};
    f32x8 avx_addsub_sf_b = {{ -0x1p-24f, 0x1p-24f,
                               -0x1p-24f, 0x1p-24f,
                               -0x1p-24f, 0x1p-24f,
                               -0x1p-24f, 0x1p-24f }};
    f64x4 avx_addsub_sd_b = {{ -0x1p-53, 0x1p-53,
                               -0x1p-53, 0x1p-53 }};
    f32x8 avx_dpps_a = {{ 1.0f, 1.0f, 0.0f, 0.0f,
                          1.0f, 1.0f, 0.0f, 0.0f }};
    f32x8 avx_dpps_b = {{ 1.0f, 0x1p-24f, 0.0f, 0.0f,
                          1.0f, 0x1p-24f, 0.0f, 0.0f }};

    for (uint32_t rc = 0; rc < 4; ++rc) {
        const uint32_t mxcsr = MXCSR_DEFAULT | (rc << MXCSR_RC_SHIFT);

        load_mxcsr(mxcsr);
        check_u32("addss-positive", float_bits(addss(1.0f, float_half_ulp)),
                  positive_expected[rc]);
        load_mxcsr(mxcsr);
        check_u32("addss-negative", float_bits(addss(-1.0f, -float_half_ulp)),
                  negative_expected[rc]);
        check_u32("mxcsr-rounding-control", store_mxcsr() & MXCSR_RC_MASK,
                  mxcsr & MXCSR_RC_MASK);
    }

    load_mxcsr(MXCSR_DEFAULT | (2U << MXCSR_RC_SHIFT));
    check_u64("addsd", double_bits(addsd(1.0, double_half_ulp)),
              0x3ff0000000000001ULL);
    addps(&sf_a, &sf_b, &sf_result);
    check_u32("addps", float_bits(sf_result.v[0]), 0x3f800001);
    addpd(&sd_a, &sd_b, &sd_result);
    check_u64("addpd", double_bits(sd_result.v[0]), 0x3ff0000000000001ULL);

    haddps(&hadd_a, &hadd_b, &sf_result);
    check_u32("haddps", float_bits(sf_result.v[0]), 0x3f800001);
    dppd(&dpp_a, &dpp_b, &sd_result);
    check_u64("dppd", double_bits(sd_result.v[0]), 0x3ff0000000000001ULL);

    check_u32("cvtsi2ss", float_bits(cvtsi2ss(16777217)), 0x4b800001);
    check_u32("cvtsd2ss", float_bits(cvtsd2ss(float_midpoint)), 0x3f800001);
    cvtdq2ps(&integers, &sf_result);
    check_u32("cvtdq2ps", float_bits(sf_result.v[0]), 0x4b800001);
    cvtpd2ps(&doubles, &sf_result);
    check_u32("cvtpd2ps", float_bits(sf_result.v[0]), 0x3f800001);

    check_u32("vaddss", float_bits(vaddss(1.0f, float_half_ulp)), 0x3f800001);
    check_u64("vaddsd", double_bits(vaddsd(1.0, double_half_ulp)),
              0x3ff0000000000001ULL);
    vaddps(&avx_sf_a, &avx_sf_b, &avx_sf_result);
    check_u32("vaddps-low", float_bits(avx_sf_result.v[0]), 0x3f800001);
    check_u32("vaddps-high", float_bits(avx_sf_result.v[4]), 0x3f800001);
    vaddpd(&avx_sd_a, &avx_sd_b, &avx_sd_result);
    check_u64("vaddpd-low", double_bits(avx_sd_result.v[0]),
              0x3ff0000000000001ULL);
    check_u64("vaddpd-high", double_bits(avx_sd_result.v[2]),
              0x3ff0000000000001ULL);

    vhaddps(&avx_hadd_sf, &avx_hadd_sf, &avx_sf_result);
    check_u32("vhaddps-low", float_bits(avx_sf_result.v[0]), 0x3f800001);
    check_u32("vhaddps-high", float_bits(avx_sf_result.v[4]), 0x3f800001);
    vhaddpd(&avx_hadd_sd, &avx_hadd_sd, &avx_sd_result);
    check_u64("vhaddpd-low", double_bits(avx_sd_result.v[0]),
              0x3ff0000000000001ULL);
    check_u64("vhaddpd-high", double_bits(avx_sd_result.v[2]),
              0x3ff0000000000001ULL);
    vhsubps(&avx_hsub_sf, &avx_hsub_sf, &avx_sf_result);
    check_u32("vhsubps-low", float_bits(avx_sf_result.v[0]), 0x3f800001);
    check_u32("vhsubps-high", float_bits(avx_sf_result.v[4]), 0x3f800001);
    vhsubpd(&avx_hsub_sd, &avx_hsub_sd, &avx_sd_result);
    check_u64("vhsubpd-low", double_bits(avx_sd_result.v[0]),
              0x3ff0000000000001ULL);
    check_u64("vhsubpd-high", double_bits(avx_sd_result.v[2]),
              0x3ff0000000000001ULL);
    vaddsubps(&avx_sf_a, &avx_addsub_sf_b, &avx_sf_result);
    check_u32("vaddsubps-low", float_bits(avx_sf_result.v[0]), 0x3f800001);
    check_u32("vaddsubps-high", float_bits(avx_sf_result.v[4]), 0x3f800001);
    vaddsubpd(&avx_sd_a, &avx_addsub_sd_b, &avx_sd_result);
    check_u64("vaddsubpd-low", double_bits(avx_sd_result.v[0]),
              0x3ff0000000000001ULL);
    check_u64("vaddsubpd-high", double_bits(avx_sd_result.v[2]),
              0x3ff0000000000001ULL);
    vdpps(&avx_dpps_a, &avx_dpps_b, &avx_sf_result);
    check_u32("vdpps-low", float_bits(avx_sf_result.v[0]), 0x3f800001);
    check_u32("vdpps-high", float_bits(avx_sf_result.v[4]), 0x3f800001);
    vdppd(&dpp_a, &dpp_b, &sd_result);
    check_u64("vdppd", double_bits(sd_result.v[0]), 0x3ff0000000000001ULL);

    vcvtdq2ps(&avx_integers, &avx_sf_result);
    check_u32("vcvtdq2ps-low", float_bits(avx_sf_result.v[0]), 0x4b800001);
    check_u32("vcvtdq2ps-high", float_bits(avx_sf_result.v[4]), 0x4b800001);
    vcvtpd2ps(&avx_doubles, &sf_result);
    check_u32("vcvtpd2ps-low", float_bits(sf_result.v[0]), 0x3f800001);
    check_u32("vcvtpd2ps-high", float_bits(sf_result.v[2]), 0x3f800001);
    check_u32("vcvtsi2ss", float_bits(vcvtsi2ss(16777217)), 0x4b800001);
    check_u64("vcvtsi2sd", double_bits(vcvtsi2sd(9007199254740993LL)),
              0x4340000000000001ULL);
    check_u32("vcvtsd2ss", float_bits(vcvtsd2ss(float_midpoint)), 0x3f800001);

    check_u32("vfmadd132ss",
              float_bits(vfmadd132ss(1.0f, float_half_ulp, 1.0f)),
              0x3f800001);
    check_u64("vfmsub132sd",
              double_bits(vfmsub132sd(1.0, -double_half_ulp, 1.0)),
              0x3ff0000000000001ULL);
    check_u32("vfnmadd213ss",
              float_bits(vfnmadd213ss(-1.0f, 1.0f, float_half_ulp)),
              0x3f800001);
    check_u64("vfnmsub213sd",
              double_bits(vfnmsub213sd(-1.0, 1.0, -double_half_ulp)),
              0x3ff0000000000001ULL);
    check_u32("vfmadd231ss",
              float_bits(vfmadd231ss(float_half_ulp, 1.0f, 1.0f)),
              0x3f800001);
    check_u64("vfnmadd231sd",
              double_bits(vfnmadd231sd(double_half_ulp, -1.0, 1.0)),
              0x3ff0000000000001ULL);

    load_mxcsr(saved_mxcsr);
    if (failures == 0) {
        puts("PASS");
    }
    return failures != 0;
}
