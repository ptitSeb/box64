#include <stdint.h>
#include "arm64_immenc.h"

#define one (uint64_t)1

// Returns the packed valid-N-imms-immr (1-1-6-6 bits); returns 0 if bitmask is not encodable
int convert_bitmask(uint64_t bitmask) {
    if (bitmask == 0 || bitmask == ~0ULL) return 0;

    uint64_t size, mask, pat;

    for (size = 6; size > 1; --size) {
        mask = (one << (1 << (size - 1))) - 1;
        pat = bitmask & mask;

        if (pat != ((bitmask >> (1 << (size - 1))) & mask)) {
            break;
        }
    }

    mask = (size >= 6) ? ~0ULL : ((one << (1 << size)) - 1);
    pat = bitmask & mask;

    for (uint64_t i = 1; i < 7 - size; ++i) {
        uint64_t boff = i * (1 << size);
        if (((bitmask >> boff) & mask) != pat) {
            return 0;
        }
    }

    int immr = 0;
    uint64_t last_bit = one << ((1 << size) - 1);

    if (pat & 1) {
        while (pat & last_bit) {
            pat = ((pat - last_bit) << 1) | 1;
            ++immr;
        }
    } else {
        immr = 1 << size;
        while (!(pat & 1)) {
            pat >>= 1;
            --immr;
        }
    }

    if (pat & (pat + 1)) return 0;

    int to = 1;
    while (pat & (one << to)) ++to;

    return 0x2000 + ((size == 6) << 12) + ((((0x1E << size) & 0x3F) + (to - 1)) << 6) + immr;
}

#if 0
#include <unordered_set>
int main() {
    std::unordered_set<uint64_t> okvals;
    uint64_t val;

    for (int n = 1; n < 7; ++n) {
        int imms0 = ((n == 6) ? 0xC0 : 0x80) + ((0x1E << n) & 0x3F);
        for (int nones = 0; nones < (1 << n) - 1; ++nones) {
            val = (one << (nones + 1)) - 1;
            for (int j = 0; j < 6 - n; ++j) {
                val += val << (1 << (n + j));
            }
            for (int immr = 0; immr < (1 << n); ++immr) {
                int exp = ((imms0 + nones) << 6) + immr;
                int got = convert_bitmask(val);
                if (exp != got) {
                    printf("0x%016lX: expected %04X, got %04X\n", val, exp, got);
                }
                okvals.emplace(val);
                val = (val >> 1) | ((val & 1) << 63);
            }
        }
    }

#pragma omp parallel for
    for (uint64_t i = 1; i < 0xFFFFFFFFFFFFFFFFu; ++i) {
        int got = convert_bitmask(i);
        if (!!got != (okvals.find(i) != okvals.cend())) {
            printf("0x%016lX: expected %s, got %04X\n", i, (!!got ? "non0" : "0000"), got);
        }
    }
}
#endif
