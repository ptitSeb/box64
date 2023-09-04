#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
// Build with `gcc -march=core2 -O2 test23.c -o test23`

uint64_t pshufb(uint64_t Gm, uint64_t Em) {
uint64_t Res;
asm(
"movq %[_Gm], %%mm0\n"
"movq %[_Em], %%mm1\n"
"pshufb %%mm1, %%mm0\n"
"movq %%mm0, %[_Res]\n"
: [_Res] "+r"(Res)
: [_Gm] "r"(Gm)
, [_Em] "r"(Em)
);
return Res;
}

int main() {
uint64_t Gm = 0x12345678abcdef00;
uint64_t Em = 0x8182888971727879;
uint64_t Res = pshufb(Gm, Em);
printf("Res = 0x%lx\n", Res);
//assert(Res == 0xefcd00ef);
return 0;
}
