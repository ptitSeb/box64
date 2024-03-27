#include <stdint.h>

const uint8_t deBruijn64tab[64] = {
    0,
    1,
    56,
    2,
    57,
    49,
    28,
    3,
    61,
    58,
    42,
    50,
    38,
    29,
    17,
    4,
    62,
    47,
    59,
    36,
    45,
    43,
    51,
    22,
    53,
    39,
    33,
    30,
    24,
    18,
    12,
    5,
    63,
    55,
    48,
    27,
    60,
    41,
    37,
    16,
    46,
    35,
    44,
    21,
    52,
    32,
    23,
    11,
    54,
    26,
    40,
    15,
    34,
    20,
    31,
    10,
    25,
    14,
    19,
    9,
    13,
    8,
    7,
    6,
};

int TrailingZeros64(uint64_t x)
{
    static const uint64_t deBruijn64 = 0x03f79d71b4ca8b09ULL;
    if (x == 0) {
        return 64;
    }

    return (int)deBruijn64tab[(x & -x) * deBruijn64 >> (64 - 6)];
}

const uint8_t lead0tab[16] = {
    // 0b0000 0b0001 0b0010 0b0011 0b0100 0b0101 0b0110 0b0111 0b1000...
    0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3
};