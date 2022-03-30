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
        printf("0x%llx ", v.u64[i]);
}

int main(int argc, const char** argv)
{
 float a, b;
 uint64_t flags;
 uint32_t maxf = 0x7f7fffff;
 uint32_t minf = 0xff7fffff;
 uint32_t r;

#define GO1(A, N)                                   \
a = 1.0f; b = 2.0f;                                 \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%lx\n", a, b, flags);        \
flags = A(b, a);                                    \
printf(N " %f, %f => 0x%lx\n", b, a, flags);        \
b = INFINITY;                                       \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%lx\n", a, b, flags);        \
flags = A(b, a);                                    \
printf(N " %f, %f => 0x%lx\n", b, a, flags);        \
b = -INFINITY;                                      \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%lx\n", a, b, flags);        \
flags = A(b, a);                                    \
printf(N " %f, %f => 0x%lx\n", b, a, flags);        \
b = NAN;                                            \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%lx\n", a, b, flags);        \
flags = A(b, a);                                    \
printf(N " %f, %f => 0x%lx\n", b, a, flags);        \
b = a;                                              \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%lx\n", a, b, flags);        \
flags = A(b, a);                                    \
printf(N " %f, %f => 0x%lx\n", b, a, flags);        \
a = b = INFINITY;                                   \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%lx\n", a, b, flags);        \
a = -INFINITY;                                      \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%lx\n", a, b, flags);        \
flags = A(b, a);                                    \
printf(N " %f, %f => 0x%lx\n", b, a, flags);        \
a = b = NAN;                                        \
flags = A(a, b);                                    \
printf(N " %f, %f => 0x%lx\n", a, b, flags);

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

 #define GO1(A, N, C)                               \
 a128.mm = _mm_##A##_epi##N(a128_##N.mm);           \
 printf("p%s%s(", #A, #C); print_##N(a128_##N);     \
 printf(") = "); print_##N(a128); printf("\n");
 #define GO2(A, N, C, A1, A2)                       \
 a128.mm = _mm_##A##_epi##N(A1.mm, A2.mm);          \
 printf("p%s%s(", #A, #C); print_##N(A1);           \
 printf(", "); print_##N(A2);                       \
 printf(") = "); print_##N(a128); printf("\n");


 GO1(abs, 8, b)
 GO1(abs, 16, w)
 GO1(abs, 32, d)
 GO2(shuffle, 8, b, a128_8, b128_8)
 GO2(hadd, 16, w, a128_16, b128_16)
 GO2(hadd, 32, d, a128_32, b128_32)


 return 0;
}
