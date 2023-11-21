// build with  gcc -O0 -g -msha -msse4.2 test28.c -o test28
// and -m32 for 32bits version
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <pmmintrin.h>
#include <immintrin.h>

typedef unsigned char u8x16 __attribute__ ((vector_size (16)));
typedef unsigned short u16x8 __attribute__ ((vector_size (16)));
typedef unsigned int  u32x4 __attribute__ ((vector_size (16)));
typedef unsigned long int  u64x2 __attribute__ ((vector_size (16)));
typedef float  f32x4 __attribute__ ((vector_size (16)));
typedef double d64x2 __attribute__ ((vector_size (16)));

typedef union {
        __m128i mm;
        __m128  mf;
        __m128d md;
        u8x16   u8;
        u16x8   u16;
        u32x4   u32;
        u64x2   u64;
        f32x4   f32;
        d64x2   d64;
} v128;

static const uint32_t A[] = {0, 0, 0, 0};
static const uint32_t B[] = {1, 2, 3, 4};
static const uint32_t C[] = {0x80000000, 0x1234567, 0, 0xffffffff};

static void print_u32(v128 a) {
  printf("%08x-%08x-%08x-%08x", a.u32[3], a.u32[2], a.u32[1], a.u32[0]);
}

static void print_u32_res(v128 a, v128 b, const char* op, v128 res) {
  printf("%s(", op);
  print_u32(a);
  printf(" ,");
  print_u32(b);
  printf(") => ");
  print_u32(res);
  printf("\n");
}
static void print_u32_u8_res(v128 a, v128 b, const char* op, uint8_t ib, v128 res) {
  printf("%s(", op);
  print_u32(a);
  printf(" ,");
  print_u32(b);
  printf(", 0x%x) => ", ib);
  print_u32(res);
  printf("\n");
}

static void print_3u32_res(v128 a, v128 b, v128 c, const char* op, v128 res) {
  printf("%s(", op);
  print_u32(a);
  printf(" ,");
  print_u32(b);
  printf(" ,");
  print_u32(c);
  printf(") => ");
  print_u32(res);
  printf("\n");
}

int main(int argc, const char** argv)
{
  printf("test SHA Ext\n");

  v128 a, b, c, d;
  int ret;
  #define LOAD(a, A) a.u32[0] = A[0]; a.u32[1] = A[1]; a.u32[2] = A[2]; a.u32[3] = A[3]

  #define GO2I_(A, B, C, I)  \
    LOAD(a, A); \
    LOAD(b, B); \
    c.mm = _mm_##C##_epu32(a.mm, b.mm, I); \
    print_u32_u8_res(a, b, #C, I, c)

  #define GO2(A, B, C)  \
    LOAD(a, A); \
    LOAD(b, B); \
    c.mm = _mm_##C##_epu32(a.mm, b.mm); \
    print_u32_res(a, b, #C, c)

  #define GO3(A, B, C, D)  \
    LOAD(a, A); \
    LOAD(b, B); \
    LOAD(c, C); \
    d.mm = _mm_##D##_epu32(a.mm, b.mm, c.mm); \
    print_3u32_res(a, b, c, #D, d)


  #define GO2I(A, B, C)   \
  GO2I_(A, B, C, 0x00);   \
  GO2I_(A, B, C, 0x01);   \
  GO2I_(A, B, C, 0x02);   \
  GO2I_(A, B, C, 0x03)

  GO2I(A, B, sha1rnds4);
  GO2I(A, C, sha1rnds4);
  GO2I(B, C, sha1rnds4);

  GO2(A, B, sha1msg1);
  GO2(A, C, sha1msg1);
  GO2(B, C, sha1msg1);

  GO2(A, B, sha1msg2);
  GO2(A, C, sha1msg2);
  GO2(B, C, sha1msg2);

  GO2(A, B, sha1nexte);
  GO2(A, C, sha1nexte);
  GO2(B, C, sha1nexte);

  GO2(A, B, sha256msg1);
  GO2(A, C, sha256msg1);
  GO2(B, C, sha256msg1);

  GO2(A, B, sha256msg2);
  GO2(A, C, sha256msg2);
  GO2(B, C, sha256msg2);

  GO3(A, B, C, sha256rnds2);
  GO3(B, A, C, sha256rnds2);
  GO3(C, B, A, sha256rnds2);

  return 0;
}
