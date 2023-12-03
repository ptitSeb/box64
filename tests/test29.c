#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
// Build with `gcc -march=core2 -O0 test29.c -o test29`

uint64_t lock_xadd(uint64_t Gd, uint64_t volatile* Ed) {
uint64_t Res;
asm(
"lock xadd %[_Gd], (%[_Ed])\n"
"movq %[_Gd], %[_Res]\n"
: [_Res] "+r"(Res)
: [_Gd] "r"(Gd)
, [_Ed] "r"(Ed)
);
return Res;
}

uint64_t lock_xchg(uint64_t Gd, uint64_t volatile* Ed) {
uint64_t Res;
asm(
"lock xchg %[_Gd], (%[_Ed])\n"
"movq %[_Gd], %[_Res]\n"
: [_Res] "+r"(Res)
: [_Gd] "r"(Gd)
, [_Ed] "r"(Ed)
);
return Res;
}

int main() {
uint64_t Gd = 0x123;
uint64_t volatile Ed = 0x456;
uint64_t Res = lock_xadd(Gd, &Ed);
printf("Lock XADD: Res = 0x%lx, Ed = 0x%lx\n", Res, Ed);
Gd = 0x123;
Res = lock_xchg(Gd, &Ed);
printf("Lock XCHG: Res = 0x%lx, Ed = 0x%lx\n", Res, Ed);
return 0;
}
