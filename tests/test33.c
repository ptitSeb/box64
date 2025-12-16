// gcc test33.c -o test33 -O2

#include <stdint.h>
#include <limits.h>

/* 移除自定义的MIN/MAX定义，使用标准库中的定义 */
/* 注意：标准库已经定义了这些宏，所以我们直接使用即可 */

/* 通用的取模测试函数（用于测试int8/int16/int32/int64） */
#define DEFINE_TEST_FUNCTIONS(TYPE, NAME, MIN_MACRO) \
    __attribute__((noinline, noclone)) TYPE \
    f1_##NAME (TYPE x, TYPE y) { \
        return x % y; \
    } \
    __attribute__((noinline, noclone)) TYPE \
    f2_##NAME (TYPE x, TYPE y) { \
        return x % -y; \
    } \
    __attribute__((noinline, noclone)) TYPE \
    f3_##NAME (TYPE x, TYPE y) { \
        TYPE z = -y; \
        return x % z; \
    } \
    static int test_##NAME##_direct(void) { \
        volatile TYPE v1 = 1; \
        volatile TYPE v2 = -1; \
        volatile TYPE v3 = -1; \
        \
        /* 基本测试：MIN % 1 */ \
        if ((MIN_MACRO % (TYPE)1) != 0) \
            return 1; \
        \
        /* 通过变量测试：MIN % 变量1 */ \
        if ((MIN_MACRO % v1) != 0) \
            return 1; \
        \
        /* 条件分支测试 */ \
        if (v2 == -1) { \
            if ((MIN_MACRO % (TYPE)1) != 0) \
                return 1; \
        } else if ((MIN_MACRO % -v2) != 0) { \
            return 1; \
        } \
        \
        /* 测试MIN % -(-1) */ \
        if ((MIN_MACRO % -v3) != 0) \
            return 1; \
        \
        return 0; \
    } \
    static int test_##NAME##_functions(void) { \
        /* 测试函数调用形式 */ \
        if (f1_##NAME(MIN_MACRO, 1) != 0) \
            return 1; \
        if (f2_##NAME(MIN_MACRO, -1) != 0) \
            return 1; \
        if (f3_##NAME(MIN_MACRO, -1) != 0) \
            return 1; \
        return 0; \
    }

/* 为各种整数类型定义测试函数 */
/* 使用标准库中的宏定义 */
DEFINE_TEST_FUNCTIONS(int8_t, int8, INT8_MIN)
DEFINE_TEST_FUNCTIONS(int16_t, int16, INT16_MIN)
DEFINE_TEST_FUNCTIONS(int32_t, int32, INT32_MIN)
DEFINE_TEST_FUNCTIONS(int64_t, int64, INT64_MIN)

/* 测试普通的int类型（兼容原测试） */
__attribute__((noinline, noclone)) int
f1_int (int x, int y) {
    return x % y;
}

__attribute__((noinline, noclone)) int
f2_int (int x, int y) {
    return x % -y;
}

__attribute__((noinline, noclone)) int
f3_int (int x, int y) {
    int z = -y;
    return x % z;
}

static int test_int_direct(void) {
    volatile int v1 = 1;
    volatile int v2 = -1;
    volatile int v3 = -1;
    
    /* 使用INT_MIN代替-__INT_MAX__ - 1 */
    if ((INT_MIN % 1) != 0)
        return 1;
    if ((INT_MIN % v1) != 0)
        return 1;
    if (v2 == -1) {
        if ((INT_MIN % 1) != 0)
            return 1;
    } else if ((INT_MIN % -v2) != 0) {
        return 1;
    }
    if ((INT_MIN % -v3) != 0)
        return 1;
    
    return 0;
}

static int test_int_functions(void) {
    if (f1_int(INT_MIN, 1) != 0)
        return 1;
    if (f2_int(INT_MIN, -1) != 0)
        return 1;
    if (f3_int(INT_MIN, -1) != 0)
        return 1;
    return 0;
}

/* 测试普通的long类型（可选，用于更全面的覆盖） */
__attribute__((noinline, noclone)) long
f1_long (long x, long y) {
    return x % y;
}

__attribute__((noinline, noclone)) long
f2_long (long x, long y) {
    return x % -y;
}

__attribute__((noinline, noclone)) long
f3_long (long x, long y) {
    long z = -y;
    return x % z;
}

static int test_long_direct(void) {
    volatile long v1 = 1L;
    volatile long v2 = -1L;
    volatile long v3 = -1L;
    
    if ((LONG_MIN % 1L) != 0)
        return 1;
    if ((LONG_MIN % v1) != 0)
        return 1;
    if (v2 == -1L) {
        if ((LONG_MIN % 1L) != 0)
            return 1;
    } else if ((LONG_MIN % -v2) != 0) {
        return 1;
    }
    if ((LONG_MIN % -v3) != 0)
        return 1;
    
    return 0;
}

static int test_long_functions(void) {
    if (f1_long(LONG_MIN, 1L) != 0)
        return 1;
    if (f2_long(LONG_MIN, -1L) != 0)
        return 1;
    if (f3_long(LONG_MIN, -1L) != 0)
        return 1;
    return 0;
}

/* 测试long long类型 */
__attribute__((noinline, noclone)) long long
f1_longlong (long long x, long long y) {
    return x % y;
}

__attribute__((noinline, noclone)) long long
f2_longlong (long long x, long long y) {
    return x % -y;
}

__attribute__((noinline, noclone)) long long
f3_longlong (long long x, long long y) {
    long long z = -y;
    return x % z;
}

static int test_longlong_direct(void) {
    volatile long long v1 = 1LL;
    volatile long long v2 = -1LL;
    volatile long long v3 = -1LL;
    
    if ((LLONG_MIN % 1LL) != 0)
        return 1;
    if ((LLONG_MIN % v1) != 0)
        return 1;
    if (v2 == -1LL) {
        if ((LLONG_MIN % 1LL) != 0)
            return 1;
    } else if ((LLONG_MIN % -v2) != 0) {
        return 1;
    }
    if ((LLONG_MIN % -v3) != 0)
        return 1;
    
    return 0;
}

static int test_longlong_functions(void) {
    if (f1_longlong(LLONG_MIN, 1LL) != 0)
        return 1;
    if (f2_longlong(LLONG_MIN, -1LL) != 0)
        return 1;
    if (f3_longlong(LLONG_MIN, -1LL) != 0)
        return 1;
    return 0;
}

/* 主测试函数 */
int main(void) {
    /* 测试int8_t */
    if (test_int8_direct()) __builtin_abort();
    if (test_int8_functions()) __builtin_abort();
    
    /* 测试int16_t */
    if (test_int16_direct()) __builtin_abort();
    if (test_int16_functions()) __builtin_abort();
    
    /* 测试int32_t */
    if (test_int32_direct()) __builtin_abort();
    if (test_int32_functions()) __builtin_abort();
    
    /* 测试int64_t */
    if (test_int64_direct()) __builtin_abort();
    if (test_int64_functions()) __builtin_abort();
    
    /* 测试普通的int类型 */
    if (test_int_direct()) __builtin_abort();
    if (test_int_functions()) __builtin_abort();
    
    /* 测试long类型 */
    if (test_long_direct()) __builtin_abort();
    if (test_long_functions()) __builtin_abort();
    
    /* 测试long long类型 */
    if (test_longlong_direct()) __builtin_abort();
    if (test_longlong_functions()) __builtin_abort();
    
    /* 所有测试通过 */
    return 0;
}
