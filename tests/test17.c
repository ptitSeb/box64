#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#if defined(__x86_64__)
uint64_t _div_(uint64_t a, uint64_t b, uint64_t *r)
{
    uint64_t ret, rem;
    asm volatile (
    "xor %%rdx, %%rdx\n"
    "div %%rcx\n"
    "mov %%rdx, %%rbx\n"
    :"=a" (ret), "=b" (rem):"a" (a), "c" (b):"rdx","cc");
    *r = rem;
    return ret;
}
uint64_t _idiv_(uint64_t a, uint64_t b, uint64_t *r)
{
    uint64_t ret, rem;
    asm volatile (
    "cqo\n"
    "idiv %%rcx\n"
    "mov %%rdx, %%rbx\n"
    :"=a" (ret), "=b" (rem):"a" (a), "c" (b):"rdx","cc");
    *r = rem;
    return ret;
}
#else
#endif

int main(int argc, const char** argv)
{
  uint64_t datas[][2] = {{1,1},{10,5},{10,3},{1, (uint64_t)-1}, {10, (uint64_t)-3}, {(uint64_t)-10, (uint64_t)-3}};

  int sz = sizeof(datas)/sizeof(datas[0]);
  for(int i=0; i<sz; ++i) {
   uint64_t rem = 0;
   uint64_t d = _div_(datas[i][0], datas[i][1], &rem);
   printf("div %llu, %llu => %llu / %llu\n", datas[i][0], datas[i][1], d, rem);
 }
  for(int i=0; i<sz; ++i) {
   uint64_t rem = 0;
   uint64_t d = _idiv_(datas[i][0], datas[i][1], &rem);
   printf("idiv %lld, %lld => %lld / %lld\n", datas[i][0], datas[i][1], d, rem);
 }
  return 0;
}
