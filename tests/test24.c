#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
// Build with `gcc -march=core2 -O2 test24.c -o test24`


uint64_t a = 0x12345678abcdefed;
uint32_t b = 0x12345678;

int main()
{
    uint64_t ret1;
    uint32_t ret2;

    asm volatile(
        "movbe %1, %0\n"
        : "=r"(ret1)
        : "m"(a)
        : "memory");
    printf("ret = 0x%lx\n", ret1);


    asm volatile(
        "movbe %1, %0\n"
        : "=r"(ret2)
        : "m"(b)
        : "memory");
    printf("ret = 0x%x\n", ret2);

    asm volatile(
        "movbe %1, %0\n"
        : "=m"(ret1)
        : "r"(a)
        : "memory");
    printf("ret = 0x%lx\n", ret1);

    asm volatile(
        "movbe %1, %0\n"
        : "=m"(ret2)
        : "r"(b)
        : "memory");
    printf("ret = 0x%x\n", ret2);
    return 0;
}
