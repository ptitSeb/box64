// build with  gcc -O0 -g -msse2 -msse3 -mavx -o test35 test35.c -lm -march=native
// Test NaN sign bit handling in SSE/AVX floating-point operations.
// On x86, newly generated NaNs (e.g. sqrt(-1), 0*inf, inf-inf, 0/0, inf/inf)
// must have the sign bit set (negative NaN).
// This catches a dynarec bug where the NaN fixup produces +nan instead of -nan.
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <immintrin.h>

typedef union {
    __m128  mf;
    __m128d md;
    __m128i mi;
    float   f32[4];
    double  d64[2];
    uint32_t u32[4];
    uint64_t u64[2];
} v128;

static void print_ps(v128 v) {
    for (int i = 0; i < 4; ++i) {
        if (isnan(v.f32[i]))
            printf("%cnan ", (v.u32[i] & 0x80000000) ? '-' : '+');
        else
            printf("%g ", v.f32[i]);
    }
}

static void print_pd(v128 v) {
    for (int i = 0; i < 2; ++i) {
        if (isnan(v.d64[i]))
            printf("%cnan ", (v.u64[i] & 0x8000000000000000ULL) ? '-' : '+');
        else
            printf("%g ", v.d64[i]);
    }
}

static void print_ss(v128 v) {
    if (isnan(v.f32[0]))
        printf("%cnan", (v.u32[0] & 0x80000000) ? '-' : '+');
    else
        printf("%g", v.f32[0]);
}

static void print_sd(v128 v) {
    if (isnan(v.d64[0]))
        printf("%cnan", (v.u64[0] & 0x8000000000000000ULL) ? '-' : '+');
    else
        printf("%g", v.d64[0]);
}

int main(int argc, const char** argv)
{
    v128 a, b, r;

    // ========== SQRTPD ==========
    printf("=== SQRTPD ===\n");
    // sqrt(-1.0) => -nan, sqrt(4.0) => 2.0
    a.md = _mm_set_pd(4.0, -1.0);
    r.md = _mm_sqrt_pd(a.md);
    printf("sqrtpd("); print_pd(a); printf(") = "); print_pd(r); printf("\n");

    // sqrt(-inf) => -nan, sqrt(0.0) => 0.0
    a.md = _mm_set_pd(0.0, -INFINITY);
    r.md = _mm_sqrt_pd(a.md);
    printf("sqrtpd("); print_pd(a); printf(") = "); print_pd(r); printf("\n");

    // sqrt(nan) => -nan (input nan passthrough with sign forced negative)
    a.md = _mm_set_pd(9.0, NAN);
    r.md = _mm_sqrt_pd(a.md);
    printf("sqrtpd("); print_pd(a); printf(") = "); print_pd(r); printf("\n");

    // ========== SQRTPS ==========
    printf("=== SQRTPS ===\n");
    // sqrt(-1, 4, -9, 16) => -nan, 2, -nan, 4
    a.mf = _mm_set_ps(16.0f, -9.0f, 4.0f, -1.0f);
    r.mf = _mm_sqrt_ps(a.mf);
    printf("sqrtps("); print_ps(a); printf(") = "); print_ps(r); printf("\n");

    // ========== SQRTSD ==========
    printf("=== SQRTSD ===\n");
    a.md = _mm_set_pd(99.0, 25.0);
    b.md = _mm_set_pd(77.0, -4.0);
    r.md = _mm_sqrt_sd(a.md, b.md);
    // sqrtsd: r[0] = sqrt(b[0]), r[1] = a[1]
    printf("sqrtsd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // ========== SQRTSS ==========
    printf("=== SQRTSS ===\n");
    a.mf = _mm_set_ps(1.0f, 2.0f, 3.0f, 9.0f);
    b.mf = _mm_set_ps(5.0f, 6.0f, 7.0f, -16.0f);
    r.mf = _mm_sqrt_ss(b.mf);
    // sqrtss: r[0] = sqrt(b[0]), r[1..3] = b[1..3]
    printf("sqrtss("); print_ps(b); printf(") = "); print_ps(r); printf("\n");

    // ========== MULPD ==========
    printf("=== MULPD ===\n");
    // 0 * inf => -nan
    a.md = _mm_set_pd(2.0, 0.0);
    b.md = _mm_set_pd(3.0, INFINITY);
    r.md = _mm_mul_pd(a.md, b.md);
    printf("mulpd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // inf * 0 => -nan
    a.md = _mm_set_pd(-1.0, INFINITY);
    b.md = _mm_set_pd(5.0, 0.0);
    r.md = _mm_mul_pd(a.md, b.md);
    printf("mulpd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // -inf * 0 => -nan (sign: negative * positive = negative, but it's indeterminate)
    a.md = _mm_set_pd(1.0, -INFINITY);
    b.md = _mm_set_pd(1.0, 0.0);
    r.md = _mm_mul_pd(a.md, b.md);
    printf("mulpd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // ========== MULPS ==========
    printf("=== MULPS ===\n");
    a.mf = _mm_set_ps(1.0f, 0.0f, INFINITY, 2.0f);
    b.mf = _mm_set_ps(2.0f, INFINITY, 0.0f, 3.0f);
    r.mf = _mm_mul_ps(a.mf, b.mf);
    printf("mulps("); print_ps(a); printf(", "); print_ps(b); printf(") = "); print_ps(r); printf("\n");

    // ========== DIVPD ==========
    printf("=== DIVPD ===\n");
    // 0/0 => -nan, inf/inf => -nan
    a.md = _mm_set_pd(INFINITY, 0.0);
    b.md = _mm_set_pd(INFINITY, 0.0);
    r.md = _mm_div_pd(a.md, b.md);
    printf("divpd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // -inf/inf => -nan
    a.md = _mm_set_pd(1.0, -INFINITY);
    b.md = _mm_set_pd(2.0, INFINITY);
    r.md = _mm_div_pd(a.md, b.md);
    printf("divpd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // ========== DIVPS ==========
    printf("=== DIVPS ===\n");
    a.mf = _mm_set_ps(1.0f, 0.0f, INFINITY, -INFINITY);
    b.mf = _mm_set_ps(2.0f, 0.0f, INFINITY, -INFINITY);
    r.mf = _mm_div_ps(a.mf, b.mf);
    printf("divps("); print_ps(a); printf(", "); print_ps(b); printf(") = "); print_ps(r); printf("\n");

    // ========== ADDPD (inf + -inf => -nan) ==========
    printf("=== ADDPD ===\n");
    a.md = _mm_set_pd(1.0, INFINITY);
    b.md = _mm_set_pd(2.0, -INFINITY);
    r.md = _mm_add_pd(a.md, b.md);
    printf("addpd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // ========== ADDPS ==========
    printf("=== ADDPS ===\n");
    a.mf = _mm_set_ps(1.0f, INFINITY, -INFINITY, 3.0f);
    b.mf = _mm_set_ps(2.0f, -INFINITY, INFINITY, 4.0f);
    r.mf = _mm_add_ps(a.mf, b.mf);
    printf("addps("); print_ps(a); printf(", "); print_ps(b); printf(") = "); print_ps(r); printf("\n");

    // ========== SUBPD (inf - inf => -nan) ==========
    printf("=== SUBPD ===\n");
    a.md = _mm_set_pd(1.0, INFINITY);
    b.md = _mm_set_pd(2.0, INFINITY);
    r.md = _mm_sub_pd(a.md, b.md);
    printf("subpd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // ========== SUBPS ==========
    printf("=== SUBPS ===\n");
    a.mf = _mm_set_ps(1.0f, INFINITY, -INFINITY, 5.0f);
    b.mf = _mm_set_ps(2.0f, INFINITY, -INFINITY, 3.0f);
    r.mf = _mm_sub_ps(a.mf, b.mf);
    printf("subps("); print_ps(a); printf(", "); print_ps(b); printf(") = "); print_ps(r); printf("\n");

    // ========== HADDPD ==========
    printf("=== HADDPD ===\n");
    // hadd: r[0] = a[0]+a[1], r[1] = b[0]+b[1]
    a.md = _mm_set_pd(-INFINITY, INFINITY);  // a[0]+a[1] = inf + -inf = -nan
    b.md = _mm_set_pd(1.0, 2.0);            // b[0]+b[1] = 2+1 = 3
    r.md = _mm_hadd_pd(a.md, b.md);
    printf("haddpd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // ========== HADDPS ==========
    printf("=== HADDPS ===\n");
    // hadd: r[0]=a[0]+a[1], r[1]=a[2]+a[3], r[2]=b[0]+b[1], r[3]=b[2]+b[3]
    a.mf = _mm_set_ps(-INFINITY, INFINITY, 1.0f, 2.0f);
    b.mf = _mm_set_ps(3.0f, 4.0f, -INFINITY, INFINITY);
    r.mf = _mm_hadd_ps(a.mf, b.mf);
    printf("haddps("); print_ps(a); printf(", "); print_ps(b); printf(") = "); print_ps(r); printf("\n");

    // ========== ADDSUBPD ==========
    printf("=== ADDSUBPD ===\n");
    // addsubpd: r[0] = a[0]-b[0], r[1] = a[1]+b[1]
    a.md = _mm_set_pd(INFINITY, INFINITY);
    b.md = _mm_set_pd(-INFINITY, INFINITY);  // r[0]=inf-inf=-nan, r[1]=inf+(-inf)=-nan
    r.md = _mm_addsub_pd(a.md, b.md);
    printf("addsubpd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // ========== ADDSUBPS ==========
    printf("=== ADDSUBPS ===\n");
    // addsubps: r[0]=a[0]-b[0], r[1]=a[1]+b[1], r[2]=a[2]-b[2], r[3]=a[3]+b[3]
    a.mf = _mm_set_ps(INFINITY, INFINITY, INFINITY, INFINITY);
    b.mf = _mm_set_ps(-INFINITY, INFINITY, -INFINITY, INFINITY);
    r.mf = _mm_addsub_ps(a.mf, b.mf);
    printf("addsubps("); print_ps(a); printf(", "); print_ps(b); printf(") = "); print_ps(r); printf("\n");

    // ========== MULSD ==========
    printf("=== MULSD ===\n");
    a.md = _mm_set_pd(99.0, 0.0);
    b.md = _mm_set_pd(77.0, INFINITY);
    r.md = _mm_mul_sd(a.md, b.md);
    printf("mulsd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // ========== MULSS ==========
    printf("=== MULSS ===\n");
    a.mf = _mm_set_ps(1.0f, 2.0f, 3.0f, 0.0f);
    b.mf = _mm_set_ps(4.0f, 5.0f, 6.0f, INFINITY);
    r.mf = _mm_mul_ss(a.mf, b.mf);
    printf("mulss("); print_ps(a); printf(", "); print_ps(b); printf(") = "); print_ps(r); printf("\n");

    // ========== DIVSD ==========
    printf("=== DIVSD ===\n");
    a.md = _mm_set_pd(99.0, 0.0);
    b.md = _mm_set_pd(77.0, 0.0);
    r.md = _mm_div_sd(a.md, b.md);
    printf("divsd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // ========== DIVSS ==========
    printf("=== DIVSS ===\n");
    a.mf = _mm_set_ps(1.0f, 2.0f, 3.0f, 0.0f);
    b.mf = _mm_set_ps(4.0f, 5.0f, 6.0f, 0.0f);
    r.mf = _mm_div_ss(a.mf, b.mf);
    printf("divss("); print_ps(a); printf(", "); print_ps(b); printf(") = "); print_ps(r); printf("\n");

    // ========== ADDSD ==========
    printf("=== ADDSD ===\n");
    a.md = _mm_set_pd(99.0, INFINITY);
    b.md = _mm_set_pd(77.0, -INFINITY);
    r.md = _mm_add_sd(a.md, b.md);
    printf("addsd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // ========== ADDSS ==========
    printf("=== ADDSS ===\n");
    a.mf = _mm_set_ps(1.0f, 2.0f, 3.0f, INFINITY);
    b.mf = _mm_set_ps(4.0f, 5.0f, 6.0f, -INFINITY);
    r.mf = _mm_add_ss(a.mf, b.mf);
    printf("addss("); print_ps(a); printf(", "); print_ps(b); printf(") = "); print_ps(r); printf("\n");

    // ========== SUBSD ==========
    printf("=== SUBSD ===\n");
    a.md = _mm_set_pd(99.0, INFINITY);
    b.md = _mm_set_pd(77.0, INFINITY);
    r.md = _mm_sub_sd(a.md, b.md);
    printf("subsd("); print_pd(a); printf(", "); print_pd(b); printf(") = "); print_pd(r); printf("\n");

    // ========== SUBSS ==========
    printf("=== SUBSS ===\n");
    a.mf = _mm_set_ps(1.0f, 2.0f, 3.0f, INFINITY);
    b.mf = _mm_set_ps(4.0f, 5.0f, 6.0f, INFINITY);
    r.mf = _mm_sub_ss(a.mf, b.mf);
    printf("subss("); print_ps(a); printf(", "); print_ps(b); printf(") = "); print_ps(r); printf("\n");

    // ========== NaN propagation (input NaN should propagate) ==========
    printf("=== NaN propagation ===\n");
    // When one input is NaN, result should be NaN (sign preserved from input)
    v128 nan_pos, nan_neg;
    nan_pos.u64[0] = 0x7FF8000000000000ULL; // +NaN
    nan_pos.u64[1] = 0x7FF8000000000000ULL;
    nan_neg.u64[0] = 0xFFF8000000000000ULL; // -NaN
    nan_neg.u64[1] = 0xFFF8000000000000ULL;

    a.md = _mm_set_pd(1.0, 2.0);
    r.md = _mm_add_pd(a.md, nan_neg.md);
    printf("addpd(normal, -nan) = "); print_pd(r); printf("\n");

    r.md = _mm_add_pd(a.md, nan_pos.md);
    printf("addpd(normal, +nan) = "); print_pd(r); printf("\n");

    r.md = _mm_mul_pd(nan_neg.md, a.md);
    printf("mulpd(-nan, normal) = "); print_pd(r); printf("\n");

    // ========== Summary ==========
    printf("NaN sign bit tests complete\n");

    return 0;
}
