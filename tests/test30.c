// build with  gcc -O0 -g -msse -msse2 -mssse3 -msse4.1 -mavx test30.c -o test30 -march=native
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <pmmintrin.h>
#include <immintrin.h> 
#include <sys/mman.h>
#include <unistd.h>

typedef unsigned char u8x16 __attribute__ ((vector_size (16)));
typedef unsigned short u16x8 __attribute__ ((vector_size (16)));
typedef unsigned int  u32x4 __attribute__ ((vector_size (16)));
typedef unsigned long int  u64x2 __attribute__ ((vector_size (16)));
typedef float  f32x4 __attribute__ ((vector_size (16)));
typedef double d64x2 __attribute__ ((vector_size (16)));
int testVPMASKMOV();
int testVMASKMOVP();
static int ACCESS_TEST = 1;

typedef union {
        __m128i mm;
        __m128  mf;
        __m128d md;
        u8x16   u8;
        u16x8   u16;
        u32x4   u32;
        u64x2   u64;
        f32x4   f32;
        d64x2   d64;
} v128;

uint64_t _ucomiss_(float a, float b)
{
    uint64_t ret = 0x202;
    v128 va, vb;
    va.f32[0] = a;
    vb.f32[0] = b;
    if(_mm_ucomigt_ss(va.mf, vb.mf))
        ret |= 0x000;
    else if(_mm_ucomilt_ss(va.mf, vb.mf))
        ret |= 0x001;
    else if(_mm_ucomieq_ss(va.mf, vb.mf))
        ret |= 0x040;
    else
        ret |= 0x045;
    return ret;
}

uint64_t _minss_(float a, float b)
{
    v128 va, vb, ret;
    va.f32[0] = a;
    vb.f32[0] = b;
    ret.mf = _mm_min_ss(va.mf, vb.mf);
    return ret.u64[0];
}
uint64_t _maxss_(float a, float b)
{
    v128 va, vb, ret;
    va.f32[0] = a;
    vb.f32[0] = b;
    ret.mf = _mm_max_ss(va.mf, vb.mf);
    return ret.u64[0];
}

#define CMPSS(A, B)                         \
uint64_t _cmpss_##A(float a, float b)       \
{                                           \
    v128 va, vb, ret;                       \
    va.f32[0] = a;                          \
    vb.f32[0] = b;                          \
    ret.mf = _mm_cmp##B##_ss(va.mf, vb.mf); \
    return ret.u64[0];                      \
}
CMPSS(0, eq)
CMPSS(1, lt)
CMPSS(2, le)
CMPSS(3, unord)
CMPSS(4, neq)
CMPSS(5, nlt)
CMPSS(6, nle)
CMPSS(7, ord)
#undef CMPSS

const v128 a128_8 = {.u8 = {
    0xff, 0x80, 0x7f, 0x00, 0x01, 0x02, 0x03, 0x81,
    0xfe, 0x84, 0x72, 0x52, 0xa5, 0x00, 0xc0, 0x32
}};
const v128 a128_16 = {.u16 = {
    0xffff, 0x8000, 0x7fff, 0x0000, 0x0001, 0x0002, 0x0003, 0x8001
}};
const v128 a128_32 = {.u32 = {
    0xffffffff, 0x80000000, 0x7fffffff, 0x00000000
}};
const v128 a128_64 = {.u64 = {
    0xffffffffffffffffLL, 0x8000000000000000LL
}};

const v128 b128_8 = {.u8 = {
    0x00, 0x01, 0x05, 0x15, 0x20, 0x80, 0xff, 0x00,
    0x08, 0x07, 0x81, 0x06, 0x0a, 0x0f, 0x10, 0x01
}};
const v128 b128_16 = {.u16 = {
    0x8000, 0x7fff, 0xffff, 0xffff, 0x0050, 0x9000, 0xfffe, 0x8001
}};
const v128 b128_32 = {.u32 = {
    0x00000001, 0x80000000, 0x00000005, 0xfffffffe
}};
const v128 b128_64 = {.u64 = {
    0x0000000000000001LL, 0x8000000000000000LL
}};

const v128 c128_8 = {.u8 = {
    0xfe, 0x7e, 0x7f, 0x81, 0x10, 0x90, 0x0f, 0xf0,
    0xf8, 0x77, 0x87, 0xf6, 0x03, 0xe1, 0x50, 0x21
}};
const v128 c128_16 = {.u16 = {
    0x7ffe, 0x0020, 0x7f00, 0x0001, 0x8000, 0xa050, 0xfff1, 0x8008
}};
const v128 c128_32 = {.u32 = {
    0x00000001, 0x80000000, 0x80000005, 0x0000fffe
}};
const v128 c128_64 = {.u64 = {
    0x7fffffffffffffffLL, 0x0000000000000004LL
}};

const v128 a128_pd = {.d64 = { 1.0, 2.0}};
const v128 b128_pd = {.d64 = { 0.0, -2.0}};
const v128 c128_pd = {.d64 = { INFINITY, -INFINITY}};
const v128 d128_pd = {.d64 = { NAN, -0.0}};
const v128 a128_ps = {.f32 = { 1.0, 2.0, 3.0, -4.0}};
const v128 b128_ps = {.f32 = { 0.0, -2.0, -10.0, 0.5}};
const v128 c128_ps = {.f32 = { INFINITY, -INFINITY, -INFINITY, 1.0}};
const v128 d128_ps = {.f32 = { NAN, -0.0, -NAN, INFINITY}};

v128 reverse_pd(v128 a) {
    v128 ret;
    ret.md = _mm_shuffle_pd(a.md, a.md, 1);
    return ret;
}

void print_8(v128 v) {
    for(int i=0; i<16; ++i)
        printf("0x%x ", v.u8[i]);
}
void print_16(v128 v) {
    for(int i=0; i<8; ++i)
        printf("0x%x ", v.u16[i]);
}
void print_32(v128 v) {
    for(int i=0; i<4; ++i)
        printf("0x%x ", v.u32[i]);
}
void print_64(v128 v) {
    for(int i=0; i<2; ++i)
        printf("0x%"PRIx64" ", v.u64[i]);
}
#define print_128 print_64
void print_ps(v128 v) {
    for(int i=0; i<4; ++i)
        if(isnanf(v.f32[i]))
            printf("%cnan ", (v.u32[i]&0x80000000)?'-':'+');
        else
            printf("%g ", v.f32[i]);
}
void print_pd(v128 v) {
    for(int i=0; i<2; ++i)
        if(isnan(v.d64[i]))
            printf("0x%"PRIx64" ", v.u64[i]);
        else
            printf("%g ", v.d64[i]);
}
#define print_sd print_pd

int main(int argc, const char** argv)
{
 float a, b;
 uint32_t flags;
 uint32_t maxf = 0x7f7fffff;
 uint32_t minf = 0xff7fffff;
 uint32_t r;

#define GO1(A, N)                                   \
a = 1.0f; b = 2.0f;                                 \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", a, b, flags);  \
flags = A(b, a);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", b, a, flags);  \
b = INFINITY;                                       \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", a, b, flags);  \
flags = A(b, a);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", b, a, flags);  \
b = -INFINITY;                                      \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", a, b, flags);  \
flags = A(b, a);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", b, a, flags);  \
b = NAN;                                            \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", a, b, flags);  \
flags = A(b, a);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", b, a, flags);  \
b = a;                                              \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", a, b, flags);  \
flags = A(b, a);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", b, a, flags);  \
a = b = INFINITY;                                   \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", a, b, flags);  \
a = -INFINITY;                                      \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", a, b, flags);  \
flags = A(b, a);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", b, a, flags);  \
a = b = NAN;                                        \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%"PRIx32"\n", a, b, flags);

#define GO2(A, N)                               \
a = 1.0f; b = 2.0f;                             \
r = A(a, b);                                    \
printf(N " %g, %g => %g\n", a, b, *(float*)&r); \
r = A(b, a);                                    \
printf(N " %g, %g => %g\n", b, a, *(float*)&r); \
a = -INFINITY;                                  \
r = A(a, b);                                    \
printf(N " %g, %g => %g\n", a, b, *(float*)&r); \
r = A(b, a);                                    \
printf(N " %g, %g => %g\n", b, a, *(float*)&r); \
a = +INFINITY;                                  \
r = A(a, b);                                    \
printf(N " %g, %g => %g\n", a, b, *(float*)&r); \
r = A(b, a);                                    \
printf(N " %g, %g => %g\n", b, a, *(float*)&r); \
a = NAN;                                        \
r = A(a, b);                                    \
printf(N " %g, %g => %g\n", a, b, *(float*)&r); \
r = A(b, a);                                    \
printf(N " %g, %g => %g\n", b, a, *(float*)&r); \
b = *(float*)&maxf;                             \
r = A(a, b);                                    \
printf(N " %g, %g => %g\n", a, b, *(float*)&r); \
r = A(b, a);                                    \
printf(N " %g, %g => %g\n", b, a, *(float*)&r); \
a = -INFINITY;                                  \
r = A(a, b);                                    \
printf(N " %g, %g => %g\n", a, b, *(float*)&r); \
r = A(b, a);                                    \
printf(N " %g, %g => %g\n", b, a, *(float*)&r); \
a = +INFINITY;                                  \
r = A(a, b);                                    \
printf(N " %g, %g => %g\n", a, b, *(float*)&r); \
r = A(b, a);                                    \
printf(N " %g, %g => %g\n", b, a, *(float*)&r);

 GO1(_ucomiss_, "ucomiss")
 GO2(_minss_, "minss")
 GO2(_maxss_, "maxss")
 GO1(_cmpss_0, "cmpss 0")
 GO1(_cmpss_1, "cmpss 1")
 GO1(_cmpss_2, "cmpss 2")
 GO1(_cmpss_3, "cmpss 3")
 GO1(_cmpss_4, "cmpss 4")
 GO1(_cmpss_5, "cmpss 5")
 GO1(_cmpss_6, "cmpss 6")
 GO1(_cmpss_7, "cmpss 7")

 #undef GO1
 #undef GO2
 v128 a128;
 int i;

 #define GO1(A, N, C)                               \
 a128.mm = _mm_##A##_epi##N(a128_##N.mm);           \
 printf("%s(", #C); print_##N(a128_##N);            \
 printf(") = "); print_##N(a128); printf("\n");
 #define GO1C(A, N, C, A1, I)                       \
 a128.mm = _mm_##A##_epi##N(A1.mm, I);              \
 printf("%s(", #C); print_##N(A1);                  \
 printf("%d) = ", I); print_##N(a128); printf("\n");
 #define GO2(A, N, C, A1, A2)                       \
 a128.mm = _mm_##A##_epi##N(A1.mm, A2.mm);          \
 printf("%s(", #C); print_##N(A1);                  \
 printf(", "); print_##N(A2);                       \
 printf(") = "); print_##N(a128); printf("\n");
 #define GO2u(A, N, C, A1, A2)                      \
 a128.mm = _mm_##A##_epu##N(A1.mm, A2.mm);          \
 printf("%s(", #C); print_##N(A1);                  \
 printf(", "); print_##N(A2);                       \
 printf(") = "); print_##N(a128); printf("\n");
 #define GO2f(A, C, A1, A2)                         \
 a128.mm = _mm_##A##_si128(A1.mm, A2.mm);           \
 printf("%s(", #C); print_128(A1);                  \
 printf(", "); print_128(A2);                       \
 printf(") = "); print_128(a128); printf("\n");
 #define GO2C(A, N, C, A1, A2, I)                   \
 a128.mm = _mm_##A##_epi##N(A1.mm, A2.mm, I);       \
 printf("%s(", #C); print_##N(A1);                  \
 printf(", "); print_##N(A2);                       \
 printf("%d) = ", I); print_##N(a128); printf("\n");
 #define GO2i(A, A1, A2)                            \
 i = _mm_##A##_si128(A1.mm, A2.mm);                 \
 printf("p%s(", #A); print_64(A1);                  \
 printf(", "); print_64(A2);                        \
 printf(") = %d\n", i);
 #define GO3PS(A, N, A1, A2, A3)                    \
 a128.mf = _mm_##A##_ps(A1.mf, A2.mf, A3.mf);       \
 printf("p%s%s(", #A, "ps"); print_##N(A1);         \
 printf(", "); print_##N(A2);                       \
 printf(", "); print_##N(A3);                       \
 printf(") = "); print_##N(a128); printf("\n");
 #define GO1ipd(A, C, A1)                           \
 i = _mm_##A##_pd(A1.md);                           \
 printf("%s(", #C); print_64(A1);                   \
 printf(") = 0x%x\n", i);
 #define GO1pd(A, C, A1)                            \
 a128.md = _mm_##A##_pd(A1.md);                     \
 printf("%s(", #C); print_pd(A1);                   \
 printf(") = "); print_pd(a128); printf("\n");
 #define GO2pd(A, C, A1, A2)                        \
 a128.md = _mm_##A##_pd(A1.md, A2.md);              \
 printf("%s(", #C); print_pd(A1);                   \
 printf(", "); print_pd(A2);                        \
 printf(") = "); print_pd(a128); printf("\n");
 #define GO2Cpd(A, C, A1, A2, I)                    \
 a128.md = _mm_##A##_pd(A1.md, A2.md, I);           \
 printf("%s(", #C); print_pd(A1);                   \
 printf(", "); print_pd(A2);                        \
 printf(", %d) = ", I); print_pd(a128); printf("\n");
 #define GO1isd(A, C, A1)                           \
 i = _mm_##A##_sd(A1.md);                           \
 printf("%s(", #C); print_64(A1);                   \
 printf(") = 0x%x\n", i);
 #define GO1sd(A, C, A1)                            \
 a128.md = _mm_##A##_sd(A1.md);                     \
 printf("%s(", #C); print_sd(A1);                   \
 printf(") = "); print_sd(a128); printf("\n");
 #define GO2sd(A, C, A1, A2)                        \
 a128.md = _mm_##A##_sd(A1.md, A2.md);              \
 printf("%s(", #C); print_sd(A1);                   \
 printf(", "); print_sd(A2);                        \
 printf(") = "); print_sd(a128); printf("\n");
 #define GO2Csd(A, C, A1, A2, I)                    \
 a128.md = _mm_##A##_sd(A1.md, A2.md, I);           \
 printf("%s(", #C); print_sd(A1);                   \
 printf(", "); print_sd(A2);                        \
 printf(", %d) = ", I); print_sd(a128); printf("\n");
 #define GO1ips(A, C, A1)                           \
 i = _mm_##A##_ps(A1.mf);                           \
 printf("%s(", #C); print_32(A1);                   \
 printf(") = 0x%x\n", i);
 #define GO1ps(A, C, A1)                            \
 a128.mf = _mm_##A##_ps(A1.mf);                     \
 printf("%s(", #C); print_ps(A1);                   \
 printf(") = "); print_ps(a128); printf("\n");
 #define GO2ps(A, C, A1, A2)                        \
 a128.mf = _mm_##A##_ps(A1.mf, A2.mf);              \
 printf("%s(", #C); print_ps(A1);                   \
 printf(", "); print_ps(A2);                        \
 printf(") = "); print_ps(a128); printf("\n");
 #define GO2Cps(A, C, A1, A2, I)                    \
 a128.mf = _mm_##A##_ps(A1.mf, A2.mf, I);           \
 printf("%s(", #C); print_ps(A1);                   \
 printf(", "); print_ps(A2);                        \
 printf(", %d) = ", I); print_ps(a128); printf("\n");
 #define GO1ps2dq(A, C, A1)                         \
 a128.mm = _mm_##A##_epi32(A1.mf);                  \
 printf("%s(", #C); print_ps(A1);                   \
 printf(") = "); print_32(a128); printf("\n");
 
 #define MULITGO2pd(A, B)       \
 GO2pd(A, B, a128_pd, b128_pd)  \
 GO2pd(A, B, b128_pd, c128_pd)  \
 GO2pd(A, B, a128_pd, d128_pd)  \
 GO2pd(A, B, b128_pd, d128_pd)  \
 GO2pd(A, B, c128_pd, d128_pd)  \
 GO2pd(A, B, d128_pd, d128_pd)  \
 GO2pd(A, B, a128_pd, reverse_pd(b128_pd))  \
 GO2pd(A, B, b128_pd, reverse_pd(c128_pd))  \
 GO2pd(A, B, a128_pd, reverse_pd(d128_pd))  \
 GO2pd(A, B, b128_pd, reverse_pd(d128_pd))  \
 GO2pd(A, B, c128_pd, reverse_pd(d128_pd))  \
 GO2pd(A, B, d128_pd, reverse_pd(d128_pd))


 #define MULITGO2Cpd(A, B, I)       \
 GO2Cpd(A, B, a128_pd, b128_pd, I)  \
 GO2Cpd(A, B, b128_pd, c128_pd, I)  \
 GO2Cpd(A, B, a128_pd, d128_pd, I)  \
 GO2Cpd(A, B, b128_pd, d128_pd, I)  \
 GO2Cpd(A, B, c128_pd, d128_pd, I)  \
 GO2Cpd(A, B, d128_pd, d128_pd, I)

 #define MULITGO2ps(A, B)       \
 GO2ps(A, B, a128_ps, b128_ps)  \
 GO2ps(A, B, b128_ps, c128_ps)  \
 GO2ps(A, B, a128_ps, d128_ps)  \
 GO2ps(A, B, b128_ps, d128_ps)  \
 GO2ps(A, B, c128_ps, d128_ps)  \
 GO2ps(A, B, d128_ps, d128_ps)

 #define MULTIGO1ps2dq(A, B)    \
 GO1ps2dq(A, B, a128_ps)        \
 GO1ps2dq(A, B, b128_ps)        \
 GO1ps2dq(A, B, c128_ps)        \
 GO1ps2dq(A, B, d128_ps)

 #define MULITGO2Cps(A, B, I)       \
 GO2Cps(A, B, a128_ps, a128_ps, I)  \
 GO2Cps(A, B, a128_ps, b128_ps, I)  \
 GO2Cps(A, B, a128_ps, c128_ps, I)  \
 GO2Cps(A, B, a128_ps, d128_ps, I)  \
 GO2Cps(A, B, b128_ps, a128_ps, I)  \
 GO2Cps(A, B, b128_ps, b128_ps, I)  \
 GO2Cps(A, B, b128_ps, c128_ps, I)  \
 GO2Cps(A, B, b128_ps, d128_ps, I)  \
 GO2Cps(A, B, c128_ps, a128_ps, I)  \
 GO2Cps(A, B, c128_ps, b128_ps, I)  \
 GO2Cps(A, B, c128_ps, c128_ps, I)  \
 GO2Cps(A, B, c128_ps, d128_ps, I)  \
 GO2Cps(A, B, d128_ps, a128_ps, I)  \
 GO2Cps(A, B, d128_ps, b128_ps, I)  \
 GO2Cps(A, B, d128_ps, c128_ps, I)  \
 GO2Cps(A, B, d128_ps, d128_ps, I)  \

 #define MULITGO2Cps_nan(A, B, I)   \
 GO2Cps(A, B, a128_ps, a128_ps, I)  \
 GO2Cps(A, B, a128_ps, b128_ps, I)  \
 GO2Cps(A, B, a128_ps, c128_ps, I)  \
 GO2Cps(A, B, b128_ps, a128_ps, I)  \
 GO2Cps(A, B, b128_ps, b128_ps, I)  \
 GO2Cps(A, B, b128_ps, c128_ps, I)  \
 GO2Cps(A, B, c128_ps, a128_ps, I)  \
 GO2Cps(A, B, c128_ps, b128_ps, I)  \
 GO2Cps(A, B, c128_ps, c128_ps, I)

 #define MULITGO2Cps_naninf(A, B, I)\
 GO2Cps(A, B, a128_ps, a128_ps, I)  \
 GO2Cps(A, B, a128_ps, b128_ps, I)  \
 GO2Cps(A, B, b128_ps, a128_ps, I)  \
 GO2Cps(A, B, b128_ps, b128_ps, I)  \

 #define MULTIGO2sd(A, B)                   \
 GO2sd(A, B, a128_pd, a128_pd)              \
 GO2sd(A, B, a128_pd, b128_pd)              \
 GO2sd(A, B, a128_pd, c128_pd)              \
 GO2sd(A, B, a128_pd, d128_pd)              \
 GO2sd(A, B, b128_pd, d128_pd)              \
 GO2sd(A, B, c128_pd, d128_pd)              \
 GO2sd(A, B, a128_pd, reverse_pd(a128_pd))  \
 GO2sd(A, B, a128_pd, reverse_pd(b128_pd))  \
 GO2sd(A, B, a128_pd, reverse_pd(c128_pd))  \
 GO2sd(A, B, a128_pd, reverse_pd(d128_pd))  \
 GO2sd(A, B, b128_pd, reverse_pd(d128_pd))  \
 GO2sd(A, B, b128_pd, reverse_pd(d128_pd))

 #define MULTIGO1Ci(A, S, B, I)             \
 GO1C(A, S, B, a128_##S, I)                 \
 GO1C(A, S, B, b128_##S, I)                 \
 GO1C(A, S, B, b128_##S, I)                 \

#define MULTIGO2i(A, S, B)                  \
 GO2(A, S, B, a128_##S, a128_##S)           \
 GO2(A, S, B, a128_##S, b128_##S)           \
 GO2(A, S, B, a128_##S, c128_##S)           \
 GO2(A, S, B, b128_##S, a128_##S)           \
 GO2(A, S, B, b128_##S, b128_##S)           \
 GO2(A, S, B, b128_##S, c128_##S)           \
 GO2(A, S, B, c128_##S, a128_##S)           \
 GO2(A, S, B, c128_##S, b128_##S)           \
 GO2(A, S, B, c128_##S, c128_##S)           \

#define MULTIGO2ui(A, S, B)                 \
 GO2u(A, S, B, a128_##S, a128_##S)          \
 GO2u(A, S, B, a128_##S, b128_##S)          \
 GO2u(A, S, B, a128_##S, c128_##S)          \
 GO2u(A, S, B, b128_##S, a128_##S)          \
 GO2u(A, S, B, b128_##S, b128_##S)          \
 GO2u(A, S, B, b128_##S, c128_##S)          \
 GO2u(A, S, B, c128_##S, a128_##S)          \
 GO2u(A, S, B, c128_##S, b128_##S)          \
 GO2u(A, S, B, c128_##S, c128_##S)          \

#define MULTIGO2fi(A, B)                    \
 GO2f(A, B, a128_8, a128_8)                 \
 GO2f(A, B, a128_8, b128_8)                 \
 GO2f(A, B, a128_8, c128_8)                 \
 GO2f(A, B, b128_8, a128_8)                 \
 GO2f(A, B, b128_8, b128_8)                 \
 GO2f(A, B, b128_8, c128_8)                 \
 GO2f(A, B, c128_8, a128_8)                 \
 GO2f(A, B, c128_8, b128_8)                 \
 GO2f(A, B, c128_8, c128_8)                 \

#define MULTIGO2Ci(A, S, B, I)              \
 GO2C(A, S, B, a128_##S, a128_##S, I)       \
 GO2C(A, S, B, a128_##S, b128_##S, I)       \
 GO2C(A, S, B, a128_##S, c128_##S, I)       \
 GO2C(A, S, B, b128_##S, a128_##S, I)       \
 GO2C(A, S, B, b128_##S, b128_##S, I)       \
 GO2C(A, S, B, b128_##S, c128_##S, I)       \
 GO2C(A, S, B, c128_##S, a128_##S, I)       \
 GO2C(A, S, B, c128_##S, b128_##S, I)       \
 GO2C(A, S, B, c128_##S, c128_##S, I)       \

 GO2(shuffle, 8, pshufb, a128_8, b128_8)
 GO2(hadd, 16, phaddw, a128_16, b128_16)
 GO2(hadd, 32, phaddd, a128_32, b128_32)
 GO2(hadds, 16, phaddsw, a128_16, b128_16)
 GO2(maddubs, 16, pmaddubsw, a128_8, b128_8)
 GO2(hsub, 16, phsubw, a128_16, b128_16)
 GO2(sign, 8, psignb, a128_8, b128_8)
 GO2(sign, 16, psignw, a128_16, b128_16)
 GO2(sign, 32, psignd, a128_32, b128_32)
 GO2(mulhrs, 16, pmulhrsw, a128_16, a128_16)
 GO2(mulhrs, 16, pmulhrsw, a128_16, b128_16)
 GO2(mulhrs, 16, pmulhrsw, a128_16, c128_16)
 GO2(mulhrs, 16, pmulhrsw, b128_16, b128_16)
 GO2(mulhrs, 16, pmulhrsw, a128_16, c128_16)
 GO2(mulhrs, 16, pmulhrsw, c128_16, c128_16)
 GO3PS(blendv, 32, a128_32, b128_32, c128_32)
 GO2i(testz, a128_32, b128_32)
 GO2i(testc, a128_32, b128_32)
 GO2i(testnzc, a128_32, b128_32)
 GO1(abs, 8, pabsb)
 GO1(abs, 16, pabsw)
 GO1(abs, 32, pabsd)
 GO1(cvtepi8, 16, pmovsxbw);
 GO1(cvtepi8, 32, pmovsxbd);
 GO1(cvtepi8, 64, pmovsxbq);
 GO1(cvtepi16, 32, pmovsxwd);
 GO1(cvtepi16, 64, pmovsxwq);
 GO1(cvtepi32, 64, pmovsxdq);
 GO1(cvtepu8, 16, pmovzxbw);
 GO1(cvtepu8, 32, pmovzxbd);
 GO1(cvtepu8, 64, pmovzxbq);
 GO1(cvtepu16, 32, pmovzxwd);
 GO1(cvtepu16, 64, pmovzxwq);
 GO1(cvtepu32, 64, pmovzxdq);
 MULTIGO2i(min, 32, pminsd)
 MULTIGO2i(max, 32, pmaxsd)
 MULTIGO2Ci(blend, 16, pblendw, 0)
 MULTIGO2Ci(blend, 16, pblendw, 0xff)
 MULTIGO2Ci(blend, 16, pblendw, 0xaa)
 MULTIGO2Ci(blend, 16, pblendw, 2)
 MULTIGO2Ci(alignr, 8, palignr, 0)
 MULTIGO2Ci(alignr, 8, palignr, 2)
 MULTIGO2Ci(alignr, 8, palignr, 7)
 MULTIGO2Ci(alignr, 8, palignr, 15)
 MULTIGO2Ci(alignr, 8, palignr, 16)
 MULTIGO2Ci(alignr, 8, palignr, 0xff)
 GO1ipd(movemask, movmskpd, a128_64)
 GO1pd(sqrt, psqrtpd, a128_pd)
 GO1pd(sqrt, psqrtpd, b128_pd)
 GO1pd(sqrt, psqrtpd, c128_pd)
 GO1pd(sqrt, psqrtpd, d128_pd)
 MULITGO2pd(and, andpd)
 MULITGO2pd(andnot, andnpd)
 MULITGO2pd(or, orpd)
 MULITGO2pd(xor, xorpd)
 MULITGO2pd(add, addpd)
 MULITGO2pd(mul, mulpd)
 MULITGO2pd(sub, subpd)
 MULITGO2pd(min, minpd)
 MULITGO2pd(div, divpd)
 MULITGO2pd(max, maxpd)
 MULITGO2pd(addsub, addsubpd)
 MULITGO2Cpd(cmp, cmppd, 0)
 MULITGO2Cpd(cmp, cmppd, 1)
 MULITGO2Cpd(cmp, cmppd, 2)
 MULITGO2Cpd(cmp, cmppd, 3)
 MULITGO2Cpd(cmp, cmppd, 4)
 MULITGO2Cpd(cmp, cmppd, 5)
 MULITGO2Cpd(cmp, cmppd, 6)
 MULITGO2Cpd(cmp, cmppd, 7)
 MULITGO2Cpd(cmp, cmppd, 8)
 MULITGO2Cpd(cmp, cmppd, 9)
 MULITGO2Cpd(cmp, cmppd, 10)
 MULITGO2Cpd(cmp, cmppd, 11)
 MULITGO2Cpd(cmp, cmppd, 12)
 MULITGO2Cpd(cmp, cmppd, 13)
 MULITGO2Cpd(cmp, cmppd, 14)
 MULITGO2Cpd(cmp, cmppd, 15)
 MULITGO2Cpd(cmp, cmppd, 16)
 MULITGO2Cpd(cmp, cmppd, 17)
 MULITGO2Cpd(cmp, cmppd, 18)
 MULITGO2Cpd(cmp, cmppd, 19)
 MULITGO2Cpd(cmp, cmppd, 20)
 MULITGO2Cpd(cmp, cmppd, 21)
 MULITGO2Cpd(cmp, cmppd, 22)
 MULITGO2Cpd(cmp, cmppd, 23)
 MULITGO2Cpd(cmp, cmppd, 24)
 MULITGO2Cpd(cmp, cmppd, 25)
 MULITGO2Cpd(cmp, cmppd, 26)
 MULITGO2Cpd(cmp, cmppd, 27)
 MULITGO2Cpd(cmp, cmppd, 28)
 MULITGO2Cpd(cmp, cmppd, 29)
 MULITGO2Cpd(cmp, cmppd, 30)
 MULITGO2Cpd(cmp, cmppd, 31)
 MULITGO2Cpd(shuffle, shufpd, 0)
 MULITGO2Cpd(shuffle, shufpd, 0x15)
 MULITGO2Cpd(shuffle, shufpd, 0xff)
 MULITGO2Cpd(shuffle, shufpd, 0x02)
 MULTIGO2i(unpacklo, 8, punpcklbw)
 MULTIGO2i(unpacklo, 16, punpcklwd)
 MULTIGO2i(unpacklo, 32, punpckldq)
 MULTIGO2i(packs, 16, ppacksswb)
 MULTIGO2i(cmpgt, 8, pcmpgtb)
 MULTIGO2i(cmpgt, 16, pcmpgtw)
 MULTIGO2i(cmpgt, 32, pcmpgtd)
 MULTIGO2i(packus, 16, packuswb)
 MULTIGO2i(unpackhi, 8, punpckhbw)
 MULTIGO2i(unpackhi, 16, punpckhwd)
 MULTIGO2i(unpackhi, 32, punpckhdq)
 MULTIGO2i(packs, 32, ppackssdw)
 MULTIGO2i(unpacklo, 64, punpcklqdq)
 MULTIGO2i(unpackhi, 64, punpckhqdq)
 MULTIGO1Ci(shuffle, 32, pshufd, 0)
 MULTIGO1Ci(shuffle, 32, pshufd, 0xff)
 MULTIGO1Ci(shuffle, 32, pshufd, 0xaa)
 MULTIGO1Ci(shuffle, 32, pshufd, 2)
 MULTIGO1Ci(srli, 16, psrlw, 0)
 MULTIGO1Ci(srli, 16, psrlw, 0xff)
 MULTIGO1Ci(srli, 16, psrlw, 0xaa)
 MULTIGO1Ci(srli, 16, psrlw, 2)
 MULTIGO1Ci(srli, 32, psrld, 0)
 MULTIGO1Ci(srli, 32, psrld, 0xff)
 MULTIGO1Ci(srli, 32, psrld, 0xaa)
 MULTIGO1Ci(srli, 32, psrld, 2)
 MULTIGO1Ci(srli, 64, psrlq, 0)
 MULTIGO1Ci(srli, 64, psrlq, 0xff)
 MULTIGO1Ci(srli, 64, psrlq, 0xaa)
 MULTIGO1Ci(srli, 64, psrlq, 2)
 MULTIGO1Ci(srai, 16, psraw, 0)
 MULTIGO1Ci(srai, 16, psraw, 0xff)
 MULTIGO1Ci(srai, 16, psraw, 0xaa)
 MULTIGO1Ci(srai, 16, psraw, 2)
 MULTIGO1Ci(srai, 32, psrad, 0)
 MULTIGO1Ci(srai, 32, psrad, 0xff)
 MULTIGO1Ci(srai, 32, psrad, 0xaa)
 MULTIGO1Ci(srai, 32, psrad, 2)
 MULTIGO1Ci(slli, 16, psllw, 0)
 MULTIGO1Ci(slli, 16, psllw, 0xff)
 MULTIGO1Ci(slli, 16, psllw, 0xaa)
 MULTIGO1Ci(slli, 16, psllw, 2)
 MULTIGO1Ci(slli, 32, pslld, 0)
 MULTIGO1Ci(slli, 32, pslld, 0xff)
 MULTIGO1Ci(slli, 32, pslld, 0xaa)
 MULTIGO1Ci(slli, 32, pslld, 2)
 MULTIGO1Ci(slli, 64, psllq, 0)
 MULTIGO1Ci(slli, 64, psllq, 0xff)
 MULTIGO1Ci(slli, 64, psllq, 0xaa)
 MULTIGO1Ci(slli, 64, psllq, 2)
 MULTIGO2i(cmpeq, 8, pcmpeqb)
 MULTIGO2i(cmpeq, 16, pcmpeqw)
 MULTIGO2i(cmpeq, 32, pcmpeqd)
 MULITGO2pd(hadd, haddpd)
 MULITGO2pd(hsub, hsubpd)
 MULTIGO2i(srl, 16, psrlw)
 MULTIGO2i(srl, 32, psrld)
 MULTIGO2i(srl, 64, psrlq)
 MULTIGO2i(add, 64, paddq)
 MULTIGO2i(mullo, 16, pmullw)
 MULTIGO2ui(subs, 8, psubusb)
 MULTIGO2ui(subs, 16, psubusw)
 MULTIGO2ui(min, 8, pminub)
 MULTIGO2fi(and, pand)
 MULTIGO2ui(adds, 8, paddusb)
 MULTIGO2ui(adds, 16, paddusw)
 MULTIGO2ui(max, 8, pmaxub)
 MULTIGO2fi(andnot, pandn)
 MULTIGO2ui(avg, 8, pavgb)
 MULTIGO2i(sra, 16, psraw)
 MULTIGO2i(sra, 32, psrad)
 MULTIGO2ui(avg, 16, pavgb)
 MULTIGO2ui(mulhi, 16, pmulhuw)
 MULTIGO2i(mulhi, 16, pmulhw)
 MULTIGO2i(subs, 8, psubsb)
 MULTIGO2i(subs, 16, psubsw)
 MULTIGO2i(min, 16, pminsw)
 MULTIGO2fi(or, por)
 MULTIGO2i(adds, 8, paddusb)
 MULTIGO2i(adds, 16, paddusw)
 MULTIGO2i(max, 16, pmaxsw)
 MULTIGO2fi(xor, pxor)
 MULTIGO2i(sll, 16, psllw)
 MULTIGO2i(sll, 32, pslld)
 MULTIGO2i(sll, 64, psllq)
 MULTIGO2ui(mul, 32, pmuludq)
 MULTIGO2i(madd, 16, pmaddwd)
 MULTIGO2i(maddubs, 16, pmaddubsw)
 MULTIGO2ui(sad, 8, psadbw)
 MULTIGO2i(sub, 8, psubb)
 MULTIGO2i(sub, 16, psubw)
 MULTIGO2i(sub, 32, psubd)
 MULTIGO2i(sub, 64, psubq)
 MULTIGO2i(add, 8, paddb)
 MULTIGO2i(add, 16, paddw)
 MULTIGO2i(add, 32, paddd)
 GO2ps(movehl, pmovhlps, a128_ps, b128_ps)
 GO2ps(unpacklo, unpcklps, a128_ps, b128_ps)
 GO2ps(unpackhi, unpckhps, a128_ps, b128_ps)
 GO2ps(movelh, pmovhps, a128_ps, b128_ps)
 GO1ps(sqrt, psqrtps, a128_ps)
 GO1ps(sqrt, psqrtps, b128_ps)
 GO1ps(sqrt, psqrtps, c128_ps)
 GO1ps(sqrt, psqrtps, d128_ps)
 //GO1ps(rsqrt, prsqrtps, a128_ps)  // difference in precision
 //GO1ps(rsqrt, prsqrtps, b128_ps)  // same
 //GO1ps(rsqrt, prsqrtps, c128_ps)  // same
 //GO1ps(rsqrt, prsqrtps, d128_ps)  // difference in the handling of NAN, (-)0, and INF in Dynarec
 //GO1ps(rcp, prcpps, a128_ps)      // deference in precision
 //GO1ps(rcp, prcpps, b128_ps)      // deference in precision
 //GO1ps(rcp, prcpps, c128_ps)      // deference in precision
 GO1ps(rcp, prcpps, d128_ps)
 MULITGO2ps(and, andps)
 MULITGO2ps(andnot, andnps)
 MULITGO2ps(or, orps)
 MULITGO2ps(xor, xorps)
 MULITGO2ps(add, addps)
 MULITGO2ps(mul, mulps)
 MULITGO2ps(sub, subps)
 MULITGO2ps(min, minps)
 MULITGO2ps(div, divps)
 MULITGO2ps(max, maxps)
 MULITGO2ps(addsub, addsubps)
 MULITGO2ps(hadd, haddps)
 MULITGO2ps(hsub, hsubps)
 MULITGO2Cps(cmp, cmpps, 0)
 MULITGO2Cps(cmp, cmpps, 1)
 MULITGO2Cps(cmp, cmpps, 2)
 MULITGO2Cps(cmp, cmpps, 3)
 MULITGO2Cps(cmp, cmpps, 4)
 MULITGO2Cps(cmp, cmpps, 5)
 MULITGO2Cps(cmp, cmpps, 6)
 MULITGO2Cps(cmp, cmpps, 7)
 MULITGO2Cps(cmp, cmpps, 8)
 MULITGO2Cps(cmp, cmpps, 9)
 MULITGO2Cps(cmp, cmpps, 10)
 MULITGO2Cps(cmp, cmpps, 11)
 MULITGO2Cps(cmp, cmpps, 12)
 MULITGO2Cps(cmp, cmpps, 13)
 MULITGO2Cps(cmp, cmpps, 14)
 MULITGO2Cps(cmp, cmpps, 15)
 MULITGO2Cps(cmp, cmpps, 16)
 MULITGO2Cps(cmp, cmpps, 17)
 MULITGO2Cps(cmp, cmpps, 18)
 MULITGO2Cps(cmp, cmpps, 19)
 MULITGO2Cps(cmp, cmpps, 20)
 MULITGO2Cps(cmp, cmpps, 21)
 MULITGO2Cps(cmp, cmpps, 22)
 MULITGO2Cps(cmp, cmpps, 23)
 MULITGO2Cps(cmp, cmpps, 24)
 MULITGO2Cps(cmp, cmpps, 25)
 MULITGO2Cps(cmp, cmpps, 26)
 MULITGO2Cps(cmp, cmpps, 27)
 MULITGO2Cps(cmp, cmpps, 28)
 MULITGO2Cps(cmp, cmpps, 29)
 MULITGO2Cps(cmp, cmpps, 30)
 MULITGO2Cps(cmp, cmpps, 31)
 MULITGO2Cps(shuffle, shufps, 0)
 MULITGO2Cps(shuffle, shufps, 0x15)
 MULITGO2Cps(shuffle, shufps, 0xff)
 MULITGO2Cps(shuffle, shufps, 0x02)
 MULTIGO2sd(sqrt, sqrtsd)
 MULTIGO2sd(add, addsd)
 MULTIGO2sd(mul, mulsd)
 MULTIGO2sd(sub, subsd)
 MULTIGO2sd(min, minsd)
 MULTIGO2sd(div, divsd)
 MULTIGO2sd(max, maxsd)
 MULTIGO1ps2dq(cvtps, cvtps2dq)
 // disabling NAN and INF for DPPS on purpose, it's not implemented
 MULITGO2Cps_naninf(dp, dpps, 0xff)
 MULITGO2Cps_naninf(dp, dpps, 0x3f)
 MULITGO2Cps_naninf(dp, dpps, 0xf3)
 MULITGO2Cps_naninf(dp, dpps, 0x53)
// open this test must update test30 and ref30.txt
//  ACCESS_TEST = 2;
//  testVPMASKMOV();
//  testVMASKMOVP();
//  ACCESS_TEST = 1;
//  testVPMASKMOV();
//  testVMASKMOVP();

 return 0;
}

__m256i m256_setr_epi64x(long long a, long long b, long long c, long long d)
{
    union {
        long long q[4];
        int r[8];
    } u;
    u.q[0] = a; u.q[1] = b; u.q[2] = c; u.q[3] = d;
    return _mm256_setr_epi32(u.r[0], u.r[1], u.r[2], u.r[3], u.r[4], u.r[5], u.r[6], u.r[7]);
}

__m128i m128_setr_epi64x(long long a, long long b)
{
    union {
        long long q[2];
        int r[4];
    } u;
    u.q[0] = a; u.q[1] = b;
    return _mm_setr_epi32(u.r[0], u.r[1], u.r[2], u.r[3]);
}

int testVPMASKMOV() {
    long pageSize = sysconf(_SC_PAGESIZE);

    void *baseAddress = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (baseAddress == MAP_FAILED) {
        printf("mmap failed\n");
        return 1;
    }
    void *resultAddress = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (resultAddress == MAP_FAILED) {
        printf("mmap failed\n");
        return 1;
    }

    int *intData = (int *)((char *)baseAddress + pageSize - 4 * ACCESS_TEST * sizeof(int)); // 32 bytes for 8 integers
    int *intResult = (int *)((char *)resultAddress + pageSize - 4 * ACCESS_TEST * sizeof(int)); // 32 bytes for 8 integers

    for (int i = 0; i < 4 * ACCESS_TEST; i++) {
        intData[i] = i + 1;
    }

    __m256i mask256_int = _mm256_setr_epi32(-1, -1, -1, -1, 1 - ACCESS_TEST, 0, 1 - ACCESS_TEST, 0); // 32-bit mask
    __m128i mask128_int = _mm_setr_epi32(-1, -1, 1 - ACCESS_TEST, 0); // 32-bit mask
    __m256i mask256_long = m256_setr_epi64x(-1, -1, 1 - ACCESS_TEST, 0); // 64-bit mask
    __m128i mask128_long = m128_setr_epi64x(-1, 0); // 64-bit mask
    // ************************************************************** _mm256_maskload_epi32
    __m256i loaded_int256 = _mm256_maskload_epi32(intData, mask256_int);
    printf("VPMASKMOV ");
    for (int i = 0; i < 8; i++) {
        printf("%d ", ((int*)&loaded_int256)[i]);
    }
    printf("\n");

    memset(resultAddress, 0, pageSize);
    _mm256_maskstore_epi32(intResult, mask256_int, loaded_int256);
    printf("VPMASKMOV ");
    for (int i = 0; i < 4 * ACCESS_TEST; i++) {
        printf("%d ", intResult[i]);
    }
    printf("\n");

    // ************************************************************** _mm_maskload_epi32
    __m128i loaded_int128 = _mm_maskload_epi32(intData, mask128_int);
    printf("VPMASKMOV ");
    for (int i = 0; i < 4; i++) {
        printf("%d ", ((int*)&loaded_int128)[i]);
    }
    printf("\n");

    memset(resultAddress, 0, pageSize);
    _mm_maskstore_epi32(intResult, mask128_int, loaded_int128);
    printf("VPMASKMOV ");
    for (int i = 0; i < 2 * ACCESS_TEST; i++) {
        printf("%d ", intResult[i]);
    }
    printf("\n");

    long long *longData = (long long *)((char *)baseAddress + pageSize - 2 * ACCESS_TEST * sizeof(long long)); // 32 bytes for 4 long integers
    long long *longResult = (long long *)((char *)resultAddress + pageSize - 2 * ACCESS_TEST * sizeof(long long)); // 32 bytes for 8 integers
    for (int i = 0; i < 2 * ACCESS_TEST; i++) {
        longData[i] = i + 1;
    }

    // ************************************************************** _mm256_maskload_epi64
    __m256i loaded_long256 = _mm256_maskload_epi64(longData, mask256_long);
    printf("VPMASKMOV ");
    for (int i = 0; i < 4; i++) {
        printf("%lld ", ((long long*)&loaded_long256)[i]);
    }
    printf("\n");

    memset(resultAddress, 0, pageSize);
    _mm256_maskstore_epi64(longResult, mask256_long, loaded_long256);
    printf("VPMASKMOV ");
    for (int i = 0; i < 2 * ACCESS_TEST; i++) {
        printf("%lld ", longResult[i]);
    }
    printf("\n");

    // ************************************************************** _mm_maskload_epi64
    __m128i loaded_long128 = _mm_maskload_epi64(longData, mask128_long);
    printf("VPMASKMOV ");
    for (int i = 0; i < 2; i++) {
        printf("%lld ", ((long long*)&loaded_long128)[i]);
    }
    printf("\n");

    //  _mm_maskstore_epi64
    memset(resultAddress, 0, pageSize);
    _mm_maskstore_epi64(longResult, mask128_long, loaded_long128);
    printf("VPMASKMOV ");
    for (int i = 0; i < 1 * ACCESS_TEST; i++) {
        printf("%lld ", longResult[i]);
    }
    printf("\n");

    munmap(baseAddress, pageSize);
    munmap(resultAddress, pageSize);

    return 0;
}

int testVMASKMOVP() {
    long pageSize = sysconf(_SC_PAGESIZE);

    void *baseAddress = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (baseAddress == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }
    void *destAddress = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (destAddress == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    float *floatData = (float *)((char *)baseAddress + pageSize - 16 * ACCESS_TEST); // 16 bytes for 4 floats
    float *floatDest = (float *)((char *)destAddress + pageSize - 16 * ACCESS_TEST); // 16 bytes for 4 floats

    int mask_data[8] = { -1, 0, -1, -1, 0, 1 - ACCESS_TEST, 0, 0 };  // -1 的二进制表示是 0xFFFFFFFF（最高位为 1）
    __m256i mask256ps = _mm256_loadu_si256((__m256i const *)mask_data);
    __m256i mask256pd = _mm256_setr_epi64x(-1, -1, 0, 1 - ACCESS_TEST);
    __m128i mask128 = _mm_setr_epi32(-1, -1, 0, 1 - ACCESS_TEST);

    //=================================================================================
    //  _mm256_maskload_ps
    for (int i = 0; i < 4 * ACCESS_TEST; i++) {
        floatData[i] = (float)(i + 1);
    }

    __m256 floatVec = _mm256_maskload_ps(floatData, mask256ps);
    printf("VMASKMOVP ");
    for (int i = 0; i < 8; i++) {
        printf("%f ", ((float*)&floatVec)[i]);
    }
    printf("\n");

    //  _mm256_maskstore_ps
    memset(destAddress, 0, pageSize);
    _mm256_maskstore_ps(floatDest, mask256ps, floatVec);
    printf("VMASKMOVP ");
    for (int i = 0; i < 4 * ACCESS_TEST; i++) {
        printf("%f ", floatDest[i]);
    }
    printf("\n");

    //=================================================================================
    for (int i = 0; i < 4 * ACCESS_TEST; i++) {
        floatData[i] = (float)(i + 10);
    }

    //  _mm_maskload_ps
    __m128 floatVec128 = _mm_maskload_ps(floatData, mask128);
    printf("VMASKMOVP ");
    for (int i = 0; i < 4; i++) {
        printf("%f ", ((float*)&floatVec128)[i]);
    }
    printf("\n");

    //  _mm_maskstore_ps
    memset(destAddress, 0, pageSize);
    _mm_maskstore_ps(floatDest, mask128, floatVec128);
    printf("VMASKMOVP ");
    for (int i = 0; i < 2 * ACCESS_TEST; i++) {
        printf("%f ", floatDest[i]);
    }
    printf("\n");

    //=================================================================================
    double *doubleData = (double *)((char *)baseAddress + pageSize - 16 * ACCESS_TEST); // 16 bytes for 2 doubles
    double *doubleDest = (double *)((char *)destAddress + pageSize - 16 * ACCESS_TEST); // 16 bytes for 2 doubles
    for (int i = 0; i < 2 * ACCESS_TEST; i++) {
        doubleData[i] = (double)(i + 20);
    }

    //  _mm256_maskload_pd
    __m256d doubleVec = _mm256_maskload_pd(doubleData, mask256pd);
    printf("VMASKMOVP ");
    for (int i = 0; i < 4; i++) {
        printf("%lf ", ((double *)&doubleVec)[i]);
    }
    printf("\n");

    //  _mm256_maskstore_pd
    memset(destAddress, 0, pageSize);
    _mm256_maskstore_pd(doubleDest, mask256pd, doubleVec);
    printf("VMASKMOVP ");
    for (int i = 0; i < 2 * ACCESS_TEST; i++) {
        printf("%f ", doubleDest[i]);
    }
    printf("\n");

    //=================================================================================
    for (int i = 0; i < 2 * ACCESS_TEST; i++) {
        doubleData[i] = (double)(i + 30);
    }

    //  _mm_maskload_pd
    __m128d doubleVec128 = _mm_maskload_pd(doubleData, mask128);
    printf("VMASKMOVP ");
    for (int i = 0; i < 2; i++) {
        printf("%lf ", ((double *)&doubleVec128)[i]);
    }
    printf("\n");

    //  _mm_maskstore_pd
    memset(destAddress, 0, pageSize);
    _mm_maskstore_pd(doubleDest, mask128, doubleVec128);
    printf("VMASKMOVP ");
    for (int i = 0; i < 1 * ACCESS_TEST; i++) {
        printf("%f ", doubleDest[i]);
    }
    printf("\n");

    //=================================================================================

    munmap(baseAddress, pageSize);
    munmap(destAddress, pageSize);

    return 0;
}