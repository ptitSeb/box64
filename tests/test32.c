// gcc -mfpmath=387 -o test32 test32.c -lm
#include <stdio.h>
#include <stdint.h>
#include <math.h>

volatile double test_values[] = {
    3.0, 2.0, 0.1
};

#define TEST_X87(INSN_LOAD, INSN_OP, VAL_IDX)                  \
    do {                                                       \
        volatile double result;                                \
        volatile double* val_ptr = &test_values[VAL_IDX];      \
        __asm__ volatile(                                      \
            "fldcw %[low]\n\t" INSN_LOAD "\n\t" INSN_OP "\n\t" \
            "fstl %[res]\n\t"                                  \
            "fldcw %[orig]\n\t"                                \
            : [res] "=m"(result)                               \
            : [low] "m"(low_cw),                               \
            [orig] "m"(original_cw),                           \
            [val] "m"(*val_ptr)                                \
            : "st", "st(1)");                                  \
        printf("%-16s: %016lx\n", #INSN_OP, *(uint64_t*)&result); \
    } while (0)

uint16_t original_cw, low_cw;

int main()
{
    __asm__ volatile("fstcw %0" : "=m"(original_cw));
    low_cw = original_cw & ~((uint16_t)0x0300);

    TEST_X87("fld1; fldl %3", "fdivrp", 0);   // 1.0 / 3.0
    TEST_X87("fldl %3", "fsqrt", 1);          // sqrt(2.0)
    TEST_X87("fldl %3; fldl %3", "faddp", 2); // 0.1 + 0.1
    return 0;
}
