#include <stdio.h>
// Build with `gcc -march=core2 -O2 test25.c -o test25`

asm(
"foo:             \n\t"
  "flds 0(%rdi)   \n\t"
  "flds 4(%rdi)   \n\t"
  "flds 8(%rdi)   \n\t"
  "flds 12(%rdi)  \n\t"
  "flds 16(%rdi)  \n\t"
  "flds 20(%rdi)  \n\t"
  "flds 24(%rdi)  \n\t"
  "flds 28(%rdi)  \n\t"
  "cmp $0, %rdi   \n\t"
  "je     1f      \n\t"
  "jne    1f      \n\t"
  "flds 32(%rdi)  \n\t"
  "flds 36(%rdi)  \n\t"
"1:               \n\t"
  "fstps 0(%rsi)  \n\t"
  "fstps 4(%rsi)  \n\t"
  "fstps 8(%rsi)  \n\t"
  "fstps 12(%rsi) \n\t"
  "fstps 16(%rsi) \n\t"
  "fstps 20(%rsi) \n\t"
  "fstps 24(%rsi) \n\t"
  "fstps 28(%rsi) \n\t"
  "je     1f      \n\t"
  "jne    1f      \n\t"
  "fstps 32(%rdi) \n\t"
  "fstps 36(%rdi) \n\t"
"1:               \n\t"
  "ret            \n\t"
);

extern void foo(float* src, float* dst);

int main(void)
{
    float src[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    float dst[10] = { 0 };
    foo(src, dst);
    for (int i = 0; i < 10; ++i) {
        printf("%f\n", dst[i]);
    }
    return 0;
}
