#include <stdio.h>
#include <fenv.h>
#include <math.h>
// Build with `gcc -march=core2 -O0 test26.c -o test26 -lm`

#define TEST(fn, val)                                      \
    printf("Testing %s(%f)\n", #fn, val);                  \
    fesetround(FE_UPWARD);                                 \
    printf("FE_UPWARD: %.1f\n", (double)fn(val));          \
    printf("Current rounding mode: 0x%x\n", fegetround()); \
    fesetround(FE_DOWNWARD);                               \
    printf("FE_DOWNWARD: %.1f\n", (double)fn(val));        \
    printf("Current rounding mode: 0x%x\n", fegetround()); \
    fesetround(FE_TOWARDZERO);                             \
    printf("FE_TOWARDZERO: %.1f\n", (double)fn(val));      \
    printf("Current rounding mode: 0x%x\n", fegetround()); \
    fesetround(FE_TONEAREST);                              \
    printf("FE_TONEAREST: %.1f\n", (double)fn(val));       \
    printf("Current rounding mode: 0x%x\n\n", fegetround());

int main()
{
    TEST(rint, 1.0f);
    TEST(rint, 1.3f);
    TEST(rint, 1.5f);
    TEST(rint, 1.8f);

    TEST(rint, 2.0f);
    TEST(rint, 2.3f);
    TEST(rint, 2.5f);
    TEST(rint, 2.8f);

    TEST(rintf, 1.0f);
    TEST(rintf, 1.3f);
    TEST(rintf, 1.5f);
    TEST(rintf, 1.8f);

    TEST(rintf, 2.0f);
    TEST(rintf, 2.3f);
    TEST(rintf, 2.5f);
    TEST(rintf, 2.8f);

    TEST(nearbyint, 1.0f);
    TEST(nearbyint, 1.3f);
    TEST(nearbyint, 1.5f);
    TEST(nearbyint, 1.8f);

    TEST(nearbyint, 2.0f);
    TEST(nearbyint, 2.3f);
    TEST(nearbyint, 2.5f);
    TEST(nearbyint, 2.8f);

    TEST(nearbyintf, 1.0f);
    TEST(nearbyintf, 1.3f);
    TEST(nearbyintf, 1.5f);
    TEST(nearbyintf, 1.8f);

    TEST(nearbyintf, 2.0f);
    TEST(nearbyintf, 2.3f);
    TEST(nearbyintf, 2.5f);
    TEST(nearbyintf, 2.8f);

    TEST(llrintf, 1.0f);
    TEST(llrintf, 1.3f);
    TEST(llrintf, 1.5f);
    TEST(llrintf, 1.8f);

    TEST(llrintf, 2.0f);
    TEST(llrintf, 2.3f);
    TEST(llrintf, 2.5f);
    TEST(llrintf, 2.8f);

    TEST(llrint, 1.0f);
    TEST(llrint, 1.3f);
    TEST(llrint, 1.5f);
    TEST(llrint, 1.8f);

    TEST(llrint, 2.0f);
    TEST(llrint, 2.3f);
    TEST(llrint, 2.5f);
    TEST(llrint, 2.8f);

    TEST(lrintf, 1.0f);
    TEST(lrintf, 1.3f);
    TEST(lrintf, 1.5f);
    TEST(lrintf, 1.8f);

    TEST(lrintf, 2.0f);
    TEST(lrintf, 2.3f);
    TEST(lrintf, 2.5f);
    TEST(lrintf, 2.8f);

    TEST(lrint, 1.0f);
    TEST(lrint, 1.3f);
    TEST(lrint, 1.5f);
    TEST(lrint, 1.8f);

    TEST(lrint, 2.0f);
    TEST(lrint, 2.3f);
    TEST(lrint, 2.5f);
    TEST(lrint, 2.8f);

    return 0;
}
