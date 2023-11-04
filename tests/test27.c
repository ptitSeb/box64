// build with  gcc -O0 -g -msse -msse2 -mssse3 -msse4.1 -msse4.2 test27.c -o test27
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

const char* string1 = "This is a string";
const char* string2 = "This\0 string    ";
const char* string3 = "is\0             ";
const char* string4 = "maestrum-foo-bar";
const char* string5 = "\0               ";

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

v128 load_string(const char* s)
{
  v128 ret;
  for(int i=0; i<16; ++i)
    ret.u8[i] = s[i];
  return ret;
}
v128 load_stringw(const char* s)
{
  v128 ret;
  for(int i=0; i<8; ++i)
    ret.u16[i] = s[i];
  return ret;
}

int main(int argc, const char** argv)
{
  printf("test SSE 4.2\n");

  v128 a, b, c;
  int ret;
  int fa, fc, fo, fs, fz;
  #define GO1(A, B, C)  \
    ret = _mm_cmpestri(a.mm, strlen(A), b.mm, strlen(B), C); \
    printf("_mm_cmpestri(\"%s\", %d, \"%s\", %d, 0x%x) => %d\n", A, strlen(A), B, strlen(B), C, ret); \
    fa = _mm_cmpestra(a.mm, strlen(A), b.mm, strlen(B), C); \
    fc = _mm_cmpestrc(a.mm, strlen(A), b.mm, strlen(B), C); \
    fo = _mm_cmpestro(a.mm, strlen(A), b.mm, strlen(B), C); \
    fs = _mm_cmpestrs(a.mm, strlen(A), b.mm, strlen(B), C); \
    fz = _mm_cmpestrz(a.mm, strlen(A), b.mm, strlen(B), C); \
    printf("_mm_cmpestri(\"%s\", %d, \"%s\", %d, 0x%x) flags: a:%d s:%d z:%d c:%d o:%d\n", A, strlen(A), B, strlen(B), C, fa, fs, fz, fc, fo); \
    c.mm = _mm_cmpestrm(a.mm, strlen(A), b.mm, strlen(B), C); \
    printf("mm_cmpestrm(\"%s\", %d, \"%s\", %d, 0x%x) = %016x-%016x\n", A, strlen(A), B, strlen(B), C, c.u64[1], c.u64[0]); \
    ret = _mm_cmpistri(a.mm, b.mm, C); \
    printf("_mm_cmpistri(\"%s\", \"%s\", 0x%x) => %d\n", A, B, C, ret); \
    fa = _mm_cmpistra(a.mm, b.mm, C); \
    fc = _mm_cmpistrc(a.mm, b.mm, C); \
    fo = _mm_cmpistro(a.mm, b.mm, C); \
    fs = _mm_cmpistrs(a.mm, b.mm, C); \
    fz = _mm_cmpistrz(a.mm, b.mm, C); \
    printf("_mm_cmpestri(\"%s\", \"%s\", 0x%x) flags: a:%d s:%d z:%d c:%d o:%d\n", A, B, C, fa, fs, fz, fc, fo); \
    c.mm = _mm_cmpistrm(a.mm, b.mm, C); \
    printf("mm_cmpestrm(\"%s\", \"%s\", 0x%x) = %016x-%016x\n", A, B, C, c.u64[1], c.u64[0])

  #define GO(A, B, C)	\
    a = load_string(A); \
    b = load_string(B); \
    GO1(A, B, C);	\
    a = load_stringw(A);\
    b = load_stringw(B);\
    GO1(A, B, C+1)	\

  #define GO2(C) \
  GO(string1, string2, C); \
  GO(string2, string1, C); \
  GO(string1, string3, C); \
  GO(string3, string1, C); \
  GO(string1, string4, C); \
  GO(string4, string1, C); \
  GO(string1, string5, C); \
  GO(string5, string1, C);

  GO2(0x00)
  GO2(0x04)
  GO2(0x08)
  GO2(0x0c)
  GO2(0b1001100)
  GO2(0b0101100)
  GO2(0b0110100)
  GO2(0b0110110)
  GO2(0b1110100)

  unsigned int crc = 0;
  printf("crc32(0x%x, byte:0x%x) => ", crc, 0);
  crc = _mm_crc32_u8(crc, 0);
  printf("0x%x\n", crc);
  printf("crc32(0x%x, byte:0x%x) => ", crc, 10);
  crc = _mm_crc32_u8(crc, 10);
  printf("0x%x\n", crc);
  printf("crc32(0x%x, dword:0x%x) => ", crc, 0);
  crc = _mm_crc32_u32(crc, 0);
  printf("0x%x\n", crc);
  printf("crc32(0x%x, dword:0x%x) => ", crc, 0x123456);
  crc = _mm_crc32_u32(crc, 0x123456);
  printf("0x%x\n", crc);
  printf("crc32(0x%x, word:0x%x) => ", crc, 0x8765);
  crc = _mm_crc32_u16(crc, 0x8765);
  printf("0x%x\n", crc);
  printf("crc32(0x%x, qword:0x%x) => ", crc, 0xff1234567890);
  uint64_t crc64 = _mm_crc32_u64(crc, 0xff1234567890);
  printf("0x%x\n", crc64);

  return 0;
}
