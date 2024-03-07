#include <stdint.h>
#include "arm64_immenc.h"

#define one (uint64_t)1

// Returns the packed valid-N-imms-immr (1-1-6-6 bits); returns 0 if bitmask is not encodable
int convert_bitmask(uint64_t bitmask) {
	if (!bitmask || !~bitmask) return 0;
	
	uint64_t size, mask, pat;
	
	for (size = 6; size > 1; --size) {
		mask = (one << (1 << (size - 1))) - 1;
		pat = bitmask & mask;
		// printf("%016lX/%lu: %016lX %016lX %2d\n", bitmask, size, mask, pat, (1 << (size - 1)));
		if (pat != ((bitmask >> (1 << (size - 1))) & mask)) {
			// printf("%016lX/%lu: %016lX %016lX xx\n", bitmask, size, pat, (bitmask >> (1 << (size - 1))) & mask);
			break;
		}
	}
	mask = (size >= 6) ? ((uint64_t)-1) : ((one << (1 << size)) - 1);
	pat = bitmask & mask;
	for (uint64_t i = 1; i < 7 - size; ++i) {
		uint64_t boff = i * (1 << size);
		if (((bitmask >> boff) & mask) != pat) {
			// printf("%016lX/%lu: no %lu %lu %016lX %016lX %016lX\n", bitmask, size, i, size, boff, mask, pat);
			return 0;
		}
	}
	// Note that here, pat != 0 and ~pat & (1 << size) != 0 (otherwise size = 1 and bitmask = all 0 or all 1)
	int immr = 0;
	uint64_t last_bit = one << ((1 << size) - 1);
	// printf("%016lX/%lu: %016lX %016lX %lu\n", bitmask, size, mask, pat, last_bit);
	if (pat & 1) {
		while (pat & last_bit) {
			pat = ((pat - last_bit) << 1) + 1;
			++immr;
		}
	} else {
		immr = 1 << size;
		while (!(pat & 1)) {
			pat >>= 1;
			--immr;
		}
	}
	// printf("%016lX/%lu: %016lX %016lX %lu %d\n", bitmask, size, mask, pat, last_bit, immr);
	if (pat & (pat + 1)) return 0; // Not 0...01...1
	int to = 1;
	while (pat & (one << to)) ++to;
	
	// printf("%016lX/%lu: returning %c%c%02lX%02lX\n", bitmask, size, '2' + (size == 6), (uint64_t)(((0x1E << size) & 0x3F) + (to - 1)), (uint64_t)immr);
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
				val = val + (val << (1 << (n + j)));
			}
			for (int immr = 0; immr < (1 << n); ++immr) {
				int exp = ((imms0 + nones) << 6) + immr;
				int got = convert_bitmask(val);
				if (exp != got) {
					printf("0x%016lX: expected %04X, got %04X\n", val, exp, got);
				}
				okvals.emplace(val);
				val = (val >> 1) + ((val & 1) << 63);
			}
		}
	}
#pragma omp parallel for
	for (uint64_t i = 1; i < 0xFFFFFFFFFFFFFFFFu; ++i) {
		int got = convert_bitmask(val);
		if (!!got != (okvals.find(val) != okvals.cend())) {
			printf("0x%016lX: expected %s, got %04X\n", val, (!!got ? "0000" : "non0"), got);
		}
	}
}
#endif
