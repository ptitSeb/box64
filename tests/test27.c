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
const char* string4 = "maentrum-foo-bar";
const char* string5 = "\0               ";
const char* string6 = "bar-foo     \0   ";
const char* string7 = " sihT foo  str i";

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
  #define GOFE(A, LA, B, LB, C)         \
    fa = _mm_cmpestra(A, LA, B, LB, C); \
    fc = _mm_cmpestrc(A, LA, B, LB, C); \
    fo = _mm_cmpestro(A, LA, B, LB, C); \
    fs = _mm_cmpestrs(A, LA, B, LB, C); \
    fz = _mm_cmpestrz(A, LA, B, LB, C)  \

  #define GOFI(A, B, C)         \
    fa = _mm_cmpistra(A, B, C); \
    fc = _mm_cmpistrc(A, B, C); \
    fo = _mm_cmpistro(A, B, C); \
    fs = _mm_cmpistrs(A, B, C); \
    fz = _mm_cmpistrz(A, B, C)  \

  #define GOE(A, LA, B, LB, C)          \
    ret = _mm_cmpestri(a.mm, LA, b.mm, LB, C);\
    printf("_mm_cmpestri(\"%s\", %d, \"%s\", %d, 0x%x) => %d\n", A, LA, B, LB, C, ret); \
    GOFE(a.mm, LA, b.mm, LB, C);              \
    printf("_mm_cmpestri(\"%s\", %d, \"%s\", %d, 0x%x) flags: a:%d s:%d z:%d c:%d o:%d\n", A, LA, B, LB, C, fa, fs, fz, fc, fo); \
    c.mm = _mm_cmpestrm(a.mm, LA, b.mm, LB, C); \
    printf("_mm_cmpestrm(\"%s\", %d, \"%s\", %d, 0x%x) = %016x-%016x\n", A, LA, B, LB, C, c.u64[1], c.u64[0]); \

  #define GO1(A, B, C)  \
    GOE(A, strlen(A), B, strlen(B), C);       \
    GOE(A, ((C)&1)?8:16, B, ((C)&1)?8:16, C); \
    GOE(A, strlen(A), B, ((C)&1)?8:16, C);    \
    GOE(A, ((C)&1)?8:16, B, strlen(B), C);    \
    ret = _mm_cmpistri(a.mm, b.mm, C); \
    printf("_mm_cmpistri(\"%s\", \"%s\", 0x%x) => %d\n", A, B, C, ret); \
    GOFI(a.mm, b.mm, C);              \
    printf("_mm_cmpestri(\"%s\", \"%s\", 0x%x) flags: a:%d s:%d z:%d c:%d o:%d\n", A, B, C, fa, fs, fz, fc, fo); \
    c.mm = _mm_cmpistrm(a.mm, b.mm, C); \
    printf("_mm_cmpestrm(\"%s\", \"%s\", 0x%x) = %016x-%016x\n", A, B, C, c.u64[1], c.u64[0])

  #define GO(A, B, C)	\
    a = load_string(A); \
    b = load_string(B); \
    GO1(A, B, C);	      \
    a = load_stringw(A);\
    b = load_stringw(B);\
    GO1(A, B, C|1);	    \
    a = load_string(B); \
    b = load_string(A); \
    GO1(B, A, C);	      \
    a = load_stringw(B);\
    b = load_stringw(A);\
    GO1(B, A, C|1);     \
    a = load_string(B); \
    b = load_string(A); \
    GO1(B, A, C|64);	  \
    a = load_stringw(B);\
    b = load_stringw(A);\
    GO1(B, A, C|1|64)	  \

  #define GO2(C) \
  GO(string1, string2, C); \
  GO(string1, string3, C); \
  GO(string1, string4, C); \
  GO(string1, string5, C); \
  GO(string1, string6, C); \
  GO(string1, string7, C); \
  GO(string2, string3, C); \
  GO(string2, string4, C); \
  GO(string2, string5, C); \
  GO(string2, string6, C); \
  GO(string2, string7, C); \
  GO(string3, string4, C); \
  GO(string3, string5, C); \
  GO(string3, string6, C); \
  GO(string3, string7, C); \
  GO(string4, string5, C); \
  GO(string4, string6, C); \
  GO(string4, string7, C); \
  GO(string5, string6, C); \
  GO(string5, string7, C); \
  GO(string6, string7, C); \


  GO2(0x00)
  GO2(0x04)
  GO2(0x08)
  GO2(0x0c)
  GO2(0x10)
  GO2(0x14)
  GO2(0x18)
  GO2(0x1c)
  GO2(0x20)
  GO2(0x24)
  GO2(0x28)
  GO2(0x2c)
  GO2(0x30)
  GO2(0x34)
  GO2(0x38)
  GO2(0x3c)

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
