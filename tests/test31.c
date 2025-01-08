// Copy from Box86/tests/test26.c (Box64/tests32/test26.c)
// Build with `gcc -march=core2 -O0 test31.c -o test31 -std=c99 -masm=intel -mfpmath=387 -frounding-math`
#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <stdint.h>
#define USE_ASM_ROUNDING
#include "roundtest.h"

#define TEST_CONVERT_(stype, s_)                                               \
  do {                                                                         \
    stype s;                                                                   \
    TEST_(s = (s_), (double)s, "%a");                                          \
    TEST_(s = (s_), (float)s, "%a");                                           \
    /* converting too large float to integer, the result is undefined, on both \
     * c99 and FISTP instruction */                                            \
    if (INT64_MIN <= s && INT64_MAX <= s)                                      \
      TEST_(s = (s_), (int64_t)s, "%" PRId64);                                 \
    if (INT32_MIN <= s && INT32_MAX <= s)                                      \
      TEST_(s = (s_), (int32_t)s, "%" PRId32);                                 \
    if (INT16_MIN <= s && INT16_MAX <= s)                                      \
      TEST_(s = (s_), (int16_t)s, "%" PRId16);                                 \
    if (INT8_MIN <= s && INT8_MAX <= s)                                        \
      TEST_(s = (s_), (int8_t)s, "%" PRId8);                                   \
    if (0 <= s && UINT64_MAX <= s)                                             \
      TEST_(s = (s_), (uint64_t)s, "%" PRIu64);                                \
    if (0 <= s && UINT32_MAX <= s)                                             \
      TEST_(s = (s_), (unsigned int)s, "%" PRIu32);                            \
    if (0 <= s && UINT16_MAX <= s)                                             \
      TEST_(s = (s_), (unsigned short)s, "%" PRIu16);                          \
    if (0 <= s && UINT8_MAX <= s)                                              \
      TEST_(s = (s_), (unsigned char)s, "%" PRIu8);                            \
  } while (0)

#define TEST_CONVERT(stype, s_)                                                \
  do {                                                                         \
    TEST_CONVERT_(stype, s_);                                                  \
    TEST_CONVERT_(stype, -(s_));                                               \
  } while (0)

#define TEST_2NUMBER(d1type, d1_, d2type, d2_, operation)                      \
  do {                                                                         \
    d1type d1;                                                                 \
    d2type d2;                                                                 \
    TEST((d1 = (d1_), d2 = (d2_)), operation);                                 \
    TEST((d1 = -(d1_), d2 = (d2_)), operation);                                \
    TEST((d1 = (d1_), d2 = -(d2_)), operation);                                \
    TEST((d1 = -(d1_), d2 = -(d2_)), operation);                               \
  } while (0)

int main() {
  double d;
  float f;
  int64_t i64;
  TEST_CONVERT(double, 0x1.123456789abcp2); // FISTTP
  TEST_(d = (0x1.123456789abcp512), (float)d, "%a");
  TEST_CONVERT(double, 0x1.123456789abcp29);
  TEST_(d = (-0x1.123456789abcp30), (int32_t)d, "%" PRId32);
  TEST_(d = (-0x1.123456789abcp62), (int64_t)d, "%" PRId64);

  TEST_CONVERT(float, 0x1.123456789abcp2f);
  TEST_CONVERT(float, 0x1.123456789abcp29f);
  TEST_(f = -0x1.123456789abcp30f, (int32_t)f, "%" PRId32);
  // to be fixed:
  //TEST_(f = -0x1.123456789abcp62f, (int64_t)f, "%" PRId64);
  // The direction of rounding when an integer is converted to a floating-point
  // number that cannot exactly represent the original value
  // https://gcc.gnu.org/onlinedocs/gcc/Floating-point-implementation.html
  // to be fixed:
  //TEST_(i64 = INT64_MAX, (double)i64, "%a"); // FILD and FSTP
  TEST(d = -0x1.1234567p0, (double)((int)d));
  TEST(d = 0x1.9234567p0, (double)((int)d));
  TEST(d = -0x1.9234567p0, (double)((int)d));

  TEST(d = 0x1.1234567p0, (double)((long int)d));
  TEST(d = -0x1.1234567p0, (double)((long int)d));
  TEST(d = 0x1.9234567p0, (double)((long int)d));
  TEST(d = -0x1.9234567p0, (double)((long int)d));

  TEST_2NUMBER(double, 1.0, double, 0x1.0000000000001p0, d1 + d2);
  TEST_2NUMBER(double, 1.0, double, 0x1.0000000000001p0, d1 - d2);
  TEST_2NUMBER(double, 1.0, double, 0x1.0000000000001p0, d2 - d1);
  TEST_2NUMBER(double, 1.0, double, 0x1.000000000000dp-4, d1 + d2);
  TEST_2NUMBER(double, 1.0, double, 0x1.000000000000dp-4, d1 - d2);
  TEST_2NUMBER(double, 1.0, double, 0x1.000000000000dp-4, d2 - d1);

  TEST_2NUMBER(double, 0x1.233445566778p0, double, 0x1.3456789abcdep0, d1 + d2);
  TEST_2NUMBER(float, 0x1.233445566778p0f, float, 0x1.3456789abcdep0f, d1 *d2);
  TEST_2NUMBER(float, 0x1.233445566778p0f, double, 0x1.3456789abcdep0, d1 *d2);
  TEST_2NUMBER(double, 0x1.233445566778p0, double, 0x1.3456789abcdep0, d1 *d2);
  TEST_2NUMBER(float, 0x1.233445566778p0f, double, 0x1.3456789abcdep0, d1 *d2);
  TEST_2NUMBER(double, 0x1.233445566778p0, float, 0x1.3456789abcdep0, d1 *d2);
  TEST_2NUMBER(float, 0x1.233445566778p0f, float, 0x1.3456789abcdep0f, d1 *d2);
  TEST_2NUMBER(double, 0x1.233445566778p0, int, 5, d1 *d2);
  TEST_2NUMBER(int, 15, double, 0x1.3456789abcdep0f, d1 *d2);
  TEST_2NUMBER(float, 0x1.233445566778p0f, int, 15, d1 *d2);
  TEST_2NUMBER(int, 15, float, 0x1.3456789abcdep0f, d1 *d2);

  TEST_2NUMBER(double, 0x1.233445566778p0, double, 0x1.3456789abcdep0, d1 / d2);
  TEST_2NUMBER(double, 0x1.233445566778p0, double, 0x1.3456789abcdep0, d1 / d2);
  TEST_2NUMBER(double, 0x1.233445566778p0, float, 0x1.3456789abcdep0f, d1 / d2);

  TEST_2NUMBER(double, 1.0, double, 0x1.0000000000001p0, d2 - d1);
  TEST_2NUMBER(double, 1.0, double, 0x1.000000000000dp-4, d1 + d2);
  TEST_2NUMBER(double, 1.0, double, 0x1.000000000000dp-4, d1 - d2);
  TEST_2NUMBER(double, 1.0, double, 0x1.000000000000dp-4, d2 - d1);

  TEST_X87_1("FSQRT", 0x1.0000000000000p2);
  TEST_X87_1("FSQRT", 0x1.0000000000001p1);
  TEST_X87_1("FSQRT", 0x1.123456789abcp31);
  TEST_X87_1("FSQRT", 0x1.123456789abdp31);

  return 0;
}
