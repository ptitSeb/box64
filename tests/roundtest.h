#pragma STDC FENV_ACCESS ON
#include <assert.h>
#include <stdio.h>

#ifdef USE_ASM_ROUNDING
int fesetround_(int rounding_direction) {
  uint16_t old_cw;
  __asm__("FNSTCW %0" : "=m"(old_cw)::);
  uint16_t new_cw = (old_cw & ~0xc00) | rounding_direction;
  __asm__("FLDCW %0" ::"m"(new_cw));
  return old_cw & 0xc00;
}
int fegetround_() {
  uint16_t cw;
  __asm__("FNSTCW %0" : "=m"(cw)::);
  return cw & 0xc00;
}
#define fesetround fesetround_
#define fegetround fegetround_
#define FE_TONEAREST 0
#define FE_DOWNWARD 0x400
#define FE_UPWARD 0x800
#define FE_TOWARDZERO 0xc00
#else
#include <fenv.h>
#endif

#define FE_TONEAREST_INDEX 0
#define FE_DOWNWARD_INDEX 1
#define FE_UPWARD_INDEX 2
#define FE_TOWARDZERO_INDEX 3
int FE_MODES[] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
char *FE_MODES_STR[] = {
    "FE_TONEAREST",
    "FE_DOWNWARD",
    "FE_UPWARD",
    "FE_TOWARDZERO",
};

void assert_round(double *array) {
  assert(array[FE_DOWNWARD_INDEX] <= array[FE_TONEAREST_INDEX]);
  assert(array[FE_TONEAREST_INDEX] <= array[FE_UPWARD_INDEX]);
  if (array[FE_TOWARDZERO_INDEX] < 0)
    assert(array[FE_TOWARDZERO_INDEX] == array[FE_UPWARD_INDEX]);
  else if (array[FE_TOWARDZERO_INDEX] > 0)
    assert(array[FE_TOWARDZERO_INDEX] == array[FE_DOWNWARD_INDEX]);
  else if (array[FE_TOWARDZERO_INDEX] == 0)
    assert(array[FE_TOWARDZERO_INDEX] == array[FE_UPWARD_INDEX] ||
           array[FE_TOWARDZERO_INDEX] == array[FE_DOWNWARD_INDEX]);
}

#define TEST_(exec, expr, format)                                              \
  do {                                                                         \
    if (sizeof(#exec) == 1)                                                    \
      printf("Testing: %s\n", #expr);                                          \
    else                                                                       \
      printf("Testing: %s -> %s\n", #exec, #expr);                             \
    for (int i = 0; i < sizeof(FE_MODES) / sizeof(FE_MODES[0]); i++) {         \
      fesetround(FE_MODES[i]);                                                 \
      exec;                                                                    \
      printf("%-15s" format "\n", FE_MODES_STR[i], expr);                      \
      assert(FE_MODES[i] == fegetround());                                     \
    }                                                                          \
    printf("\n");                                                              \
  } while (0)

#define TEST(exec, expr) TEST_(exec, expr, "%a")

#if defined(i386) || defined(__i386__) || defined(__i386) ||                   \
    defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)
#define TEST_X87(instruction, st0, st1, deep_change)                           \
  do {                                                                         \
    double _st0 = (st0), _st1 = (st1);                                         \
    double array1[4], array2[4];                                               \
    double __st0, __st1;                                                       \
    printf("Testing X87 instruction: %s (ST0 = %a, ST1 = %a)\n", #instruction, \
           _st0, _st1);                                                        \
    for (int i = 0; i < sizeof(FE_MODES) / sizeof(FE_MODES[0]); i++) {         \
      fesetround(FE_MODES[i]);                                                 \
      __st0 = _st0, __st1 = _st1;                                              \
      switch (deep_change) {                                                   \
      case -1: /* the instruction pops */                                      \
        __asm__(instruction : "+t"(__st0) : "u"(__st1) : "st(1)");             \
        printf("%-15s ST0 = %a\n", FE_MODES_STR[i], __st0);                    \
        break;                                                                 \
      case 0:                                                                  \
        __asm__(instruction : "+t"(__st0) : "u"(__st1) :);                     \
        printf("%-15s ST0 = %a\n", FE_MODES_STR[i], __st0);                    \
        break;                                                                 \
      case 1: /* the instruction pushes */                                     \
        __asm__(instruction : "+t"(__st0), "=u"(__st1)::);                     \
        printf("%-15s ST0 = %a, ST1 = %a\n", FE_MODES_STR[i], __st0, __st1);   \
        array2[i] = __st1;                                                     \
      }                                                                        \
      array1[i] = __st0;                                                       \
      assert(FE_MODES[i] == fegetround());                                     \
    }                                                                          \
    if (deep_change == 1)                                                      \
      assert_round(array2);                                                    \
    assert_round(array1);                                                      \
    printf("\n");                                                              \
  } while (0)
#else
#define TEST_X87(instruction, st0, st1, deep_change)                           \
  do {                                                                         \
    double _st0 = (st0), _st1 = (st1);                                         \
    printf("Cannot test X87 instruction: %s (ST0 = %a, ST1 = %a) because it "  \
           "is not compiled to x86\n\n",                                       \
           #instruction, _st0, _st1);                                          \
  } while (0)
#endif

#define TEST_X87_1(i, st0) TEST_X87(i, st0, 0.0, 0)
#define TEST_X87_2(i, st0, st1) TEST_X87(i, st0, st1, -1)
