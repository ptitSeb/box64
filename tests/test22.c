#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// Build with `gcc -march=core2 -mfpmath=387 -O2 test22.c -o test22`

#if defined(__x86_64__)
uint64_t _fucomip_(double a, double b)
{
    uint32_t ret;
    asm volatile (
    "fldl %2\n"
    "fldl %1\n"
    "fucomip %%st(1)\n"
    "fstp %%st(0)\n"
    "pushf\n"
    "pop %%rax\n"
    :"=a" (ret):"m"(a), "m"(b):"cc");
    return ret;
}
uint64_t _fucompp_(double a, double b)
{
    uint32_t ret;
    asm volatile (
    "fldl %2\n"
    "fldl %1\n"
    "fucompp\n"
    "fstsw %%ax\n"
    :"=a" (ret):"m"(a), "m"(b):"cc");
    return ret;
}
uint64_t _fistpw_(double a, uint8_t rd)
{
    uint16_t ret;
    uint16_t t1, t2;
    asm volatile (
    "fldl %1\n"
    "fstcw %2\n"
    "mov %2, %%ax\n"
    "mov %4, %%ah\n"
    "mov %%ax, %3\n"
    "fldcw %3\n"
    "fistp %0\n"
    "fldcw %2\n"
    :"=m" (ret):"m"(a), "m"(t1), "m"(t2), "m"(rd):"cc");
    return ret;
}
uint64_t _fisttpw_(double a, uint8_t rd)
{
    uint16_t ret;
    uint16_t t1, t2;
    #ifdef ANDROID
    asm volatile (
    "fldl %1\n"
    "fstcw %2\n"
    "mov %2, %%ax\n"
    "mov %4, %%ah\n"
    "mov %%ax, %3\n"
    "fldcw %3\n"
    "fisttpl %0\n"
    "fldcw %2\n"
    :"=m" (ret):"m"(a), "m"(t1), "m"(t2), "m"(rd):"cc");
    #else
    asm volatile (
    "fldl %1\n"
    "fstcw %2\n"
    "mov %2, %%ax\n"
    "mov %4, %%ah\n"
    "mov %%ax, %3\n"
    "fldcw %3\n"
    "fisttp %0\n"
    "fldcw %2\n"
    :"=m" (ret):"m"(a), "m"(t1), "m"(t2), "m"(rd):"cc");
    #endif

    return ret;
}
uint64_t _fistpl_(double a, uint8_t rd)
{
    uint32_t ret;
    uint16_t t1, t2;
    asm volatile (
    "fldl %1\n"
    "fstcw %2\n"
    "mov %2, %%ax\n"
    "mov %4, %%ah\n"
    "mov %%ax, %3\n"
    "fldcw %3\n"
    "fistpl %0\n"
    "fldcw %2\n"
    :"=m" (ret):"m"(a), "m"(t1), "m"(t2), "m"(rd):"cc");
    return ret;
}
uint64_t _fisttpl_(double a, uint8_t rd)
{
    uint32_t ret;
    uint16_t t1, t2;
    asm volatile (
    "fldl %1\n"
    "fstcw %2\n"
    "mov %2, %%ax\n"
    "mov %4, %%ah\n"
    "mov %%ax, %3\n"
    "fldcw %3\n"
    "fisttpl %0\n"
    "fldcw %2\n"
    :"=m" (ret):"m"(a), "m"(t1), "m"(t2), "m"(rd):"cc");
    return ret;
}
uint64_t _fistpq_(double a, uint8_t rd)
{
    uint64_t ret;
    uint16_t t1, t2;
    asm volatile (
    "fldl %1\n"
    "fstcw %2\n"
    "mov %2, %%ax\n"
    "mov %4, %%ah\n"
    "mov %%ax, %3\n"
    "fldcw %3\n"
    "fistpq %0\n"
    "fldcw %2\n"
    :"=m" (ret):"m"(a), "m"(t1), "m"(t2), "m"(rd):"cc");
    return ret;
}
uint64_t _fisttpq_(double a, uint8_t rd)
{
    uint64_t ret;
    uint16_t t1, t2;
    asm volatile (
    "fldl %1\n"
    "fstcw %2\n"
    "mov %2, %%ax\n"
    "mov %4, %%ah\n"
    "mov %%ax, %3\n"
    "fldcw %3\n"
    "fisttpq %0\n"
    "fldcw %2\n"
    :"=m" (ret):"m"(a), "m"(t1), "m"(t2), "m"(rd):"cc");
    return ret;
}
uint64_t _frndint_(double a, uint8_t rd)
{
    uint64_t ret;
    uint16_t t1, t2;
    asm volatile (
    "fldl %1\n"
    "fstcw %2\n"
    "mov %2, %%ax\n"
    "mov %4, %%ah\n"
    "mov %%ax, %3\n"
    "fldcw %3\n"
    "frndint\n"
    "fstpl %0\n"
    "fldcw %2\n"
    :"=m" (ret):"m"(a), "m"(t1), "m"(t2), "m"(rd):"cc");
    return ret;
}
#endif

int main(int argc, const char** argv)
{
 double a, b;
 char pf, cf, zf;
 char pa, ca, za;
 uint64_t flags;
 uint64_t tests[][2] = {
  {0x0, 0x4082c00000000000LL},
  {0x8000000000000000LL, 0x4082c00000000000LL},
  {0x8000000000000000LL, 0x0},
  {0x40dfffc000000000LL, 0x40e0002000000000LL}, 	// 0x7fff and 0x8001 as double
  {0xc0e0002000000000LL, 0xc0dfffc000000000LL},		// -0x8001 and -0x7fff as double
  {0x8000000000000000LL, 0x3ff0000000000000LL},
  {0x3ff0000000000000LL, 0x3fe89d9000000000LL},
  {0x3ff0000000000000LL, 0x7ff0000000000000LL},
  {0xfff0000000000000LL, 0x7ff0000000000000LL},
  {0x3ff0002ca0000000LL, 0xaeff000025000000LL},
  {0x3ff0000050000000LL, 0xc082c00000000000LL},
  {0x0, 0x7ff8000000000001LL},
  {0x7ff8000000000001LL, 0x7ff8000000000001LL},
 };
 int n = sizeof(tests)/sizeof(tests[0]); 
 for(int i=0; i<n; ++i)
  for(int j=0; j<2; ++j) {
   *(uint64_t*)&a = tests[i][0+j];
   *(uint64_t*)&b = tests[i][1-j];
   printf("FUCOMI 0x%llx 0x%llx ", *(uint64_t*)&a, *(uint64_t*)&b);
   flags = _fucomip_(a, b);
   ca = (flags>>0)&1?'C':'-';
   za = (flags>>(0+6))&1?'Z':'-';
   pa = (flags>>(0+2))&1?'P':'-';
   printf("%c%c%c\n", za, pa, ca);
   printf("FUCOMPP 0x%llx 0x%llx ", *(uint64_t*)&a, *(uint64_t*)&b);
   flags = _fucompp_(a, b);
   ca = (flags>>8)&1?'C':'-';
   za = (flags>>(8+6))&1?'Z':'-';
   pa = (flags>>(8+2))&1?'P':'-';
   printf("%c%c%c\n", za, pa, ca);
   for(int rd=0; rd<3; ++rd) {
    printf("Rounding %d\n", rd);
    printf(" FRNDINT 0x%llx => 0x%llx\n", *(uint64_t*)&a, _frndint_(a, rd<<2));
    printf(" FRNDINT 0x%llx => 0x%llx\n", *(uint64_t*)&b, _frndint_(b, rd<<2));
    printf(" FISTP 0x%llx => word: %x\n", *(uint64_t*)&a, _fistpw_(a, rd<<2));
    printf(" FISTP 0x%llx => word: %x\n", *(uint64_t*)&b, _fistpw_(b, rd<<2));
    printf(" FISTP 0x%llx => long: %x\n", *(uint64_t*)&a, _fistpl_(a, rd<<2));
    printf(" FISTP 0x%llx => long: %x\n", *(uint64_t*)&b, _fistpl_(b, rd<<2));
    printf(" FISTP 0x%llx => quad: %llx\n", *(uint64_t*)&a, _fistpq_(a, rd<<2));
    printf(" FISTP 0x%llx => quad: %llx\n", *(uint64_t*)&b, _fistpq_(b, rd<<2));
    printf(" FISTTP 0x%llx => word: %x\n", *(uint64_t*)&a, _fisttpw_(a, rd<<2));
    printf(" FISTTP 0x%llx => word: %x\n", *(uint64_t*)&b, _fisttpw_(b, rd<<2));
    printf(" FISTTP 0x%llx => long: %x\n", *(uint64_t*)&a, _fisttpl_(a, rd<<2));
    printf(" FISTTP 0x%llx => long: %x\n", *(uint64_t*)&b, _fisttpl_(b, rd<<2));
    printf(" FISTTP 0x%llx => quad: %llx\n", *(uint64_t*)&a, _fisttpq_(a, rd<<2));
    printf(" FISTTP 0x%llx => quad: %llx\n", *(uint64_t*)&b, _fisttpq_(b, rd<<2));
  }
 }
 printf("\nDone\n");
}
