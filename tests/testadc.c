#include <stdio.h>
#include <stdint.h>
#include <assert.h>

void test_adc8(uint8_t n, uint8_t m, uint8_t expected_al, uint8_t expected_dl) {
  uint8_t al;
  uint8_t dl;
  __asm__ __volatile__ (
      "movb  %[n], %%dl\n"
      "movb  %[m], %%al\n"
      "addb  %[m], %%al\n"
      "adcb  %[n], %%dl\n"
      "movb  %%al, %[al]\n"
      "movb  %%dl, %[dl]\n"
      : [al] "=r" (al), [dl] "=r" (dl)
      : [n] "r" (n), [m] "r" (m)
      : "al", "dl", "memory");
  printf("al=0x%x\n", al);
  printf("dl=0x%x\n", dl);
  assert(al == expected_al && dl == expected_dl);
}

void test_adc16(uint16_t n, uint16_t m, uint16_t expected_ax, uint16_t expected_dx) {
  uint16_t ax;
  uint16_t dx;
  __asm__ __volatile__ (
      "movw  %[n], %%dx\n"
      "movw  %[m], %%ax\n"
      "addw  %[m], %%ax\n"
      "adcw  %[n], %%dx\n"
      "movw  %%ax, %[ax]\n"
      "movw  %%dx, %[dx]\n"
      : [ax] "=r" (ax), [dx] "=r" (dx)
      : [n] "r" (n), [m] "r" (m)
      : "ax", "dx", "memory");
  printf("ax=0x%x\n", ax);
  printf("dx=0x%x\n", dx);
  assert(ax == expected_ax && dx == expected_dx);
}

void test_adc32(uint32_t n, uint32_t m, uint32_t expected_eax, uint32_t expected_edx) {
  uint32_t eax;
  uint32_t edx;
  __asm__ __volatile__ (
      "movl  %[n], %%edx\n"
      "movl  %[m], %%eax\n"
      "addl  %[m], %%eax\n"
      "adcl  %[n], %%edx\n"
      "movl  %%eax, %[eax]\n"
      "movl  %%edx, %[edx]\n"
      : [eax] "=r" (eax), [edx] "=r" (edx)
      : [n] "r" (n), [m] "r" (m)
      : "eax", "edx", "memory");
  printf("eax=0x%x\n", eax);
  printf("edx=0x%x\n", edx);
  assert(eax == expected_eax && edx == expected_edx);
}

int main(int argc, char* argv[]) {
  test_adc8(0x0, 0xFF, 0xFE, 0x1);
  test_adc16(0x0, 0xFFFF, 0xFFFE, 0x1);
  test_adc32(0x0, 0xFFFFFFFF, 0xFFFFFFFE, 0x1);
  return 0;
}
