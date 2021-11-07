#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#if defined(__x86_64__)
uint64_t _ucomiss_(float a, float b)
{
    uint64_t ret;
    asm volatile (
    "ucomiss %%xmm0, %%xmm1\n"
    "pushf\n"
    "pop %%rax"
    :"=a" (ret)::"xmm0","xmm1","cc");
    return ret;
}
uint64_t _minss_(float a, float b)
{
    uint64_t ret;
    asm volatile (
    "minss %%xmm1, %%xmm0\n"
    "movd %%xmm0, %%eax"
    :"=a" (ret)::"xmm0","xmm1","cc");
    return ret;
}
uint64_t _maxss_(float a, float b)
{
    uint64_t ret;
    asm volatile (
    "maxss %%xmm1, %%xmm0\n"
    "movd %%xmm0, %%eax"
    :"=a" (ret)::"xmm0","xmm1","cc");
    return ret;
}
#define CMPSS(A)                        \
uint64_t _cmpss_##A(float a, float b)   \
{                                       \
    uint64_t ret;                       \
    asm volatile (                      \
    "cmpss $" #A ", %%xmm1, %%xmm0\n"   \
    "movd %%xmm0, %%eax"                \
    :"=a" (ret)::"xmm0","xmm1","cc");   \
    return ret;                         \
}
#else
uint64_t _ucomiss_(float a, float b)
{
    uint32_t ret;
    asm volatile (
    "movss %1, %%xmm0\n"
    "movss %2, %%xmm1\n"
    "ucomiss %%xmm0, %%xmm1\n"
    "pushf\n"
    "pop %%eax"
    :"=a" (ret):"m"(a), "m"(b):"xmm0", "xmm1", "cc");
    return ret;
}
uint64_t _minss_(float a, float b)
{
    uint32_t ret;
    asm volatile (
    "movss %1, %%xmm0\n"
    "movss %2, %%xmm1\n"
    "minss %%xmm1, %%xmm0\n"
    "movd %%xmm0, %%eax"
    :"=a" (ret):"m"(a), "m"(b):"xmm0", "xmm1", "cc");
    return ret;
}
uint64_t _maxss_(float a, float b)
{
    uint32_t ret;
    asm volatile (
    "movss %1, %%xmm0\n"
    "movss %2, %%xmm1\n"
    "maxss %%xmm1, %%xmm0\n"
    "movd %%xmm0, %%eax"
    :"=a" (ret):"m"(a), "m"(b):"xmm0", "xmm1", "cc");
    return ret;
}
#define CMPSS(A)                                            \
uint64_t _cmpss_##A(float a, float b)                       \
{                                                           \
    uint32_t ret;                                           \
    asm volatile (                                          \
    "movss %1, %%xmm0\n"                                    \
    "movss %2, %%xmm1\n"                                    \
    "cmpss $" #A ", %%xmm1, %%xmm0\n"                       \
    "movd %%xmm0, %%eax"                                    \
    :"=a" (ret):"m"(a), "m"(b):"xmm0", "xmm1", "cc");       \
    return ret;                                             \
}
#endif
CMPSS(0)
CMPSS(1)
CMPSS(2)
CMPSS(3)
CMPSS(4)
CMPSS(5)
CMPSS(6)
CMPSS(7)

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
 
 return 0;
}
