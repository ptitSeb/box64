// build with  gcc -O0 -g -msse -msse2 -mssse3 -msse4.1 test17.c -o test17
// and -m32 for 32bits version
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <pmmintrin.h>
#include <immintrin.h> 

typedef unsigned char u8x16 __attribute__ ((vector_size (16)));
typedef unsigned short u16x8 __attribute__ ((vector_size (16)));
typedef unsigned int  u32x4 __attribute__ ((vector_size (16)));
typedef unsigned long int  u64x2 __attribute__ ((vector_size (16)));
typedef float  f32x4 __attribute__ ((vector_size (16)));
typedef double d64x2 __attribute__ ((vector_size (16)));

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
const v128 c128_32 = {.u32 = {
    0x00000001, 0x80000000, 0x80000005, 0x0000fffe
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
            printf("nan ");
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
 GO2pd(A, B, d128_pd, d128_pd)

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
 GO2Cps(A, B, a128_ps, b128_ps, I)  \
 GO2Cps(A, B, b128_ps, c128_ps, I)  \
 GO2Cps(A, B, a128_ps, d128_ps, I)  \
 GO2Cps(A, B, b128_ps, d128_ps, I)  \
 GO2Cps(A, B, c128_ps, d128_ps, I)  \
 GO2Cps(A, B, d128_ps, d128_ps, I)

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


 GO2(shuffle, 8, pshufb, a128_8, b128_8)
 GO2(hadd, 16, phaddw, a128_16, b128_16)
 GO2(hadd, 32, phaddd, a128_32, b128_32)
 GO2(hadds, 16, phaddsw, a128_16, b128_16)
 GO2(maddubs, 16, pmaddubsw, a128_8, b128_8)
 GO2(hsub, 16, phsubw, a128_16, b128_16)
 GO2(sign, 8, psignb, a128_8, b128_8)
 GO2(sign, 16, psignw, a128_16, b128_16)
 GO2(sign, 32, psignd, a128_32, b128_32)
 GO2(mulhrs, 16, pmulhrsw, a128_16, b128_16)
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
 GO2(min, 32, pminsd, a128_32, b128_32)
 GO2(max, 32, pmaxsd, a128_32, b128_32)
 GO2C(blend, 16, pblendw, a128_16, b128_16, 0)
 GO2C(blend, 16, pblendw, a128_16, b128_16, 0xff)
 GO2C(blend, 16, pblendw, a128_16, b128_16, 0xaa)
 GO2C(blend, 16, pblendw, a128_16, b128_16, 2)
 GO2C(alignr, 8, palignr, a128_8, b128_8, 0)
 GO2C(alignr, 8, palignr, a128_8, b128_8, 2)
 GO2C(alignr, 8, palignr, a128_8, b128_8, 7)
 GO2C(alignr, 8, palignr, a128_8, b128_8, 15)
 GO2C(alignr, 8, palignr, a128_8, b128_8, 16)
 GO2C(alignr, 8, palignr, a128_8, b128_8, 0xff)
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
 GO2(unpacklo, 8, punpcklbw, a128_8, b128_8)
 GO2(unpacklo, 16, punpcklwd, a128_16, b128_16)
 GO2(unpacklo, 32, punpckldq, a128_32, b128_32)
 GO2(packs, 16, ppacksswb, a128_16, b128_16)
 GO2(cmpgt, 8, pcmpgtb, a128_8, b128_8)
 GO2(cmpgt, 16, pcmpgtw, a128_16, b128_16)
 GO2(cmpgt, 32, pcmpgtd, a128_32, b128_32)
 GO2(packus, 16, packuswb, a128_16, b128_16)
 GO2(unpackhi, 8, punpckhbw, a128_8, b128_8)
 GO2(unpackhi, 16, punpckhwd, a128_16, b128_16)
 GO2(unpackhi, 32, punpckhdq, a128_32, b128_32)
 GO2(packs, 32, ppackssdw, a128_32, b128_32)
 GO2(unpacklo, 64, punpcklqdq, a128_64, b128_64)
 GO2(unpackhi, 64, punpckhqdq, a128_64, b128_64)
 GO1C(shuffle, 32, pshufd, a128_32, 0)
 GO1C(shuffle, 32, pshufd, a128_32, 0xff)
 GO1C(shuffle, 32, pshufd, a128_32, 0xaa)
 GO1C(shuffle, 32, pshufd, a128_32, 2)
 GO1C(srli, 16, psrlw, a128_16, 0)
 GO1C(srli, 16, psrlw, a128_16, 0xff)
 GO1C(srli, 16, psrlw, a128_16, 0xaa)
 GO1C(srli, 16, psrlw, a128_16, 2)
 GO1C(srli, 32, psrld, a128_32, 0)
 GO1C(srli, 32, psrld, a128_32, 0xff)
 GO1C(srli, 32, psrld, a128_32, 0xaa)
 GO1C(srli, 32, psrld, a128_32, 2)
 GO1C(srli, 64, psrlq, a128_64, 0)
 GO1C(srli, 64, psrlq, a128_64, 0xff)
 GO1C(srli, 64, psrlq, a128_64, 0xaa)
 GO1C(srli, 64, psrlq, a128_64, 2)
 GO1C(srai, 16, psraw, a128_16, 0)
 GO1C(srai, 16, psraw, a128_16, 0xff)
 GO1C(srai, 16, psraw, a128_16, 0xaa)
 GO1C(srai, 16, psraw, a128_16, 2)
 GO1C(srai, 32, psrad, a128_32, 0)
 GO1C(srai, 32, psrad, a128_32, 0xff)
 GO1C(srai, 32, psrad, a128_32, 0xaa)
 GO1C(srai, 32, psrad, a128_32, 2)
 GO1C(slli, 16, psllw, a128_16, 0)
 GO1C(slli, 16, psllw, a128_16, 0xff)
 GO1C(slli, 16, psllw, a128_16, 0xaa)
 GO1C(slli, 16, psllw, a128_16, 2)
 GO1C(slli, 32, pslld, a128_32, 0)
 GO1C(slli, 32, pslld, a128_32, 0xff)
 GO1C(slli, 32, pslld, a128_32, 0xaa)
 GO1C(slli, 32, pslld, a128_32, 2)
 GO1C(slli, 64, psllq, a128_64, 0)
 GO1C(slli, 64, psllq, a128_64, 0xff)
 GO1C(slli, 64, psllq, a128_64, 0xaa)
 GO1C(slli, 64, psllq, a128_64, 2)
 GO2(cmpeq, 8, pcmpeqb, a128_8, b128_8)
 GO2(cmpeq, 16, pcmpeqw, a128_16, b128_16)
 GO2(cmpeq, 32, pcmpeqd, a128_32, b128_32)
 MULITGO2pd(hadd, haddpd)
 GO2(srl, 16, psrlw, a128_16, b128_16)
 GO2(srl, 32, psrld, a128_32, b128_32)
 GO2(srl, 64, psrlq, a128_64, b128_64)
 GO2(add, 64, paddq, a128_64, b128_64)
 GO2(mullo, 16, pmullw, a128_16, b128_16)
 GO2u(subs, 8, psubusb, a128_8, b128_8)
 GO2u(subs, 16, psubusw, a128_16, b128_16)
 GO2u(min, 8, pminub, a128_8, b128_8)
 GO2f(and, pand, a128_8, b128_8)
 GO2u(adds, 8, paddusb, a128_8, b128_8)
 GO2u(adds, 16, paddusw, a128_16, b128_16)
 GO2u(max, 8, pmaxub, a128_8, b128_8)
 GO2f(andnot, pandn, a128_8, b128_8)
 GO2u(avg, 8, pavgb, a128_8, b128_8)
 GO2(sra, 16, psraw, a128_16, b128_16)
 GO2(sra, 32, psrad, a128_32, b128_32)
 GO2u(avg, 16, pavgb, a128_16, b128_16)
 GO2u(mulhi, 16, pmulhuw, a128_16, b128_16)
 GO2(mulhi, 16, pmulhw, a128_16, b128_16)
 GO2(subs, 8, psubsb, a128_8, b128_8)
 GO2(subs, 16, psubsw, a128_16, b128_16)
 GO2(min, 16, pminsw, a128_16, b128_16)
 GO2f(or, por, a128_8, b128_8)
 GO2(adds, 8, paddusb, a128_8, b128_8)
 GO2(adds, 16, paddusw, a128_16, b128_16)
 GO2(max, 16, pmaxsw, a128_16, b128_16)
 GO2f(xor, pxor, a128_8, b128_8)
 GO2(sll, 16, psllw, a128_16, b128_16)
 GO2(sll, 32, pslld, a128_32, b128_32)
 GO2(sll, 64, psllq, a128_64, b128_64)
 GO2u(mul, 32, pmuludq, a128_32, b128_32)
 GO2(madd, 16, pmaddwd, a128_16, b128_16)
 GO2u(sad, 8, psadbw, a128_8, b128_8)
 GO2(sub, 8, psubb, a128_8, b128_8)
 GO2(sub, 16, psubw, a128_16, b128_16)
 GO2(sub, 32, psubd, a128_32, b128_32)
 GO2(sub, 64, psubq, a128_64, b128_64)
 GO2(add, 8, paddb, a128_8, b128_8)
 GO2(add, 16, paddw, a128_16, b128_16)
 GO2(add, 32, paddd, a128_32, b128_32)
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
 //MULITGO2Cps(cmp, cmpps, 0)   // use avx for some reason
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
 MULTIGO1ps2dq(cvtps, cvtps2pd)
 MULITGO2Cps(dp, dpps, 0xff)
 MULITGO2Cps(dp, dpps, 0x3f)
 MULITGO2Cps(dp, dpps, 0xf3)
 MULITGO2Cps(dp, dpps, 0x53)

 return 0;
}

