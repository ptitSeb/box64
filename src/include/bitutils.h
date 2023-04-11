#ifndef __BITUTILS_H_
#define __BITUTILS_H_
#include <stdint.h>

extern const uint8_t deBruijn64tab[64];
extern const uint8_t lead0tab[16];

int TrailingZeros64(uint64_t x);

#endif //__BITUTILS_H_
