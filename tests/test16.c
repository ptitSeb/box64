#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

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

int main(int argc, const char** argv)
{
 float a, b;
 uint64_t flags;
 a = 1.0f; b = 2.0f;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);
 flags = _ucomiss_(b, a);
 printf("ucomiss %f, %f => 0x%lx\n", b, a, flags);
 b = INFINITY;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);
 flags = _ucomiss_(b, a);
 printf("ucomiss %f, %f => 0x%lx\n", b, a, flags);
 b = -INFINITY;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);
 flags = _ucomiss_(b, a);
 printf("ucomiss %f, %f => 0x%lx\n", b, a, flags);
 b = NAN;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);
 flags = _ucomiss_(b, a);
 printf("ucomiss %f, %f => 0x%lx\n", b, a, flags);
 b = a;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);
 a = b = INFINITY;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);
 a = b = NAN;
 flags = _ucomiss_(a, b);
 printf("ucomiss %f, %f => 0x%lx\n", a, b, flags);

 return 0;
}
