#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#if defined(__x86_64__)
#error Nope!
#else
__attribute__((naked)) uint64_t _idiv_(uint32_t a, uint32_t b)
{
    asm volatile (
    "xor %%eax, %%eax\n"
    "mov 4(%%esp), %%edx\n"
    "mov 8(%%esp), %%ecx\n"
    "shrd $0x10, %%edx, %%eax\n"
    "sar $0x10, %%edx\n"
    "idiv %%ecx\n"
    "ret"
    :::);
}
#endif

int main(int argc, const char** argv)
{
 uint32_t tests[][2] = {
  {0x000002d0, 0x00000500},
  {0xfffffa5d, 0x000186a0},
  {0x00001701, 0x000186a0},
  {0xffff9a19, 0x000186a0},
  {0xffffe7f3, 0x000186a0},
  {0x00015840, 0x000186a0},
  {0xff451330, 0x000186a0},
  {0xffff626a, 0x000186a0},
  {0x00009120, 0x000186a0},
 };
 int n = sizeof(tests)/sizeof(tests[0]); 
 uint64_t res;
 for(int i=0; i<n; ++i) {
   printf("SDHLD/SAR/IDIV 0x%x 0x%x ", tests[i][0], tests[i][1]);
   res = _idiv_(tests[i][0], tests[i][1]);
   uint32_t divi = res & 0xffffffffLL;
   uint32_t modo = res >> 32;
   printf("=> 0x%x / 0x%x (%lld/%d => %d + %d)\n", modo, divi, ((int64_t)tests[i][0])<<16, tests[i][1], divi, modo);
 }
 printf("\nDone\n");
}
