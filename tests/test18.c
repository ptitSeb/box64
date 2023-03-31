// build with gcc -march=corei7 -O2 -g -maes -mpclmul test18.c -o test18
#include <stdio.h>
#include <stdint.h>
#include <wmmintrin.h>

// Hard coding types and alignment
typedef long long v2di __attribute__((vector_size(16)));
typedef union {
    unsigned long long v[2];
    v2di m;
} mm128i;

int need_stop(mm128i *var) {
    if (var->v[0] >= 0xEEEEFFFFFFFFEEEFULL) {
        if (var->v[1] == 0xFFFFFFFFFFFFFFFFULL) return 1;
        else {
            var->v[0] += 0x1111000000001111ULL;
            ++var->v[1];
        }
    } else var->v[0] += 0x1111000000001111ULL;
    if (var->v[1] >= 0xF000000000000000ULL) return 1;
    else var->v[1] += 0x1000000000000000ULL;
    return 0;
}

int main() {
    mm128i x = { .v = {0, 0} };
    do {
        mm128i y = { .v = {0, 0} };
        do {
            mm128i enc = { .m = _mm_aesenc_si128(x.m, y.m) };
            mm128i enclast = { .m = _mm_aesenclast_si128(x.m, y.m) };
            mm128i dec = { .m = _mm_aesdec_si128(x.m, y.m) };
            mm128i declast = { .m = _mm_aesdeclast_si128(x.m, y.m) };
            printf("enc    (0x%016llX%016llX, 0x%016llX%016llX) = 0x%016llX%016llX\n", x.v[1], x.v[0], y.v[1], y.v[0], enc.v[1], enc.v[0]);
            printf("enclast(0x%016llX%016llX, 0x%016llX%016llX) = 0x%016llX%016llX\n", x.v[1], x.v[0], y.v[1], y.v[0], enclast.v[1], enclast.v[0]);
            printf("dec    (0x%016llX%016llX, 0x%016llX%016llX) = 0x%016llX%016llX\n", x.v[1], x.v[0], y.v[1], y.v[0], dec.v[1], dec.v[0]);
            printf("declast(0x%016llX%016llX, 0x%016llX%016llX) = 0x%016llX%016llX\n", x.v[1], x.v[0], y.v[1], y.v[0], declast.v[1], declast.v[0]);
        } while (!need_stop(&y));
        mm128i keygenassist00 = { .m = _mm_aeskeygenassist_si128(x.m, 0) };
        printf("keygenassist(0x%016llX%016llX, 0x00) = 0x%016llX%016llX\n", x.v[1], x.v[0], keygenassist00.v[1], keygenassist00.v[0]);
        mm128i keygenassist01 = { .m = _mm_aeskeygenassist_si128(x.m, 0x01) };
        printf("keygenassist(0x%016llX%016llX, 0x01) = 0x%016llX%016llX\n", x.v[1], x.v[0], keygenassist01.v[1], keygenassist01.v[0]);
        mm128i keygenassist02 = { .m = _mm_aeskeygenassist_si128(x.m, 0x02) };
        printf("keygenassist(0x%016llX%016llX, 0x02) = 0x%016llX%016llX\n", x.v[1], x.v[0], keygenassist02.v[1], keygenassist02.v[0]);
        mm128i keygenassist04 = { .m = _mm_aeskeygenassist_si128(x.m, 0x04) };
        printf("keygenassist(0x%016llX%016llX, 0x04) = 0x%016llX%016llX\n", x.v[1], x.v[0], keygenassist04.v[1], keygenassist04.v[0]);
        mm128i keygenassist08 = { .m = _mm_aeskeygenassist_si128(x.m, 0x08) };
        printf("keygenassist(0x%016llX%016llX, 0x08) = 0x%016llX%016llX\n", x.v[1], x.v[0], keygenassist08.v[1], keygenassist08.v[0]);
        mm128i keygenassist10 = { .m = _mm_aeskeygenassist_si128(x.m, 0x10) };
        printf("keygenassist(0x%016llX%016llX, 0x10) = 0x%016llX%016llX\n", x.v[1], x.v[0], keygenassist10.v[1], keygenassist10.v[0]);
        mm128i keygenassist38 = { .m = _mm_aeskeygenassist_si128(x.m, 0x38) };
        printf("keygenassist(0x%016llX%016llX, 0x38) = 0x%016llX%016llX\n", x.v[1], x.v[0], keygenassist38.v[1], keygenassist38.v[0]);
        mm128i keygenassist4F = { .m = _mm_aeskeygenassist_si128(x.m, 0x4F) };
        printf("keygenassist(0x%016llX%016llX, 0x4F) = 0x%016llX%016llX\n", x.v[1], x.v[0], keygenassist4F.v[1], keygenassist4F.v[0]);
        mm128i keygenassistFF = { .m = _mm_aeskeygenassist_si128(x.m, 0xFF) };
        printf("keygenassist(0x%016llX%016llX, 0xFF) = 0x%016llX%016llX\n", x.v[1], x.v[0], keygenassistFF.v[1], keygenassistFF.v[0]);
        mm128i imc = { .m = _mm_aesimc_si128(x.m) };
        printf("imc         (0x%016llX%016llX) = 0x%016llX%016llX\n", x.v[1], x.v[0], imc.v[1], imc.v[0]);

        mm128i pclmul00 = { .m = _mm_clmulepi64_si128(keygenassist00.m, x.m, 0) };
        printf("pclmul(0x%016llX%016llX, 0x00) = 0x%016llX%016llX\n", x.v[1], x.v[0], pclmul00.v[1], pclmul00.v[0]);
        mm128i pclmul01 = { .m = _mm_clmulepi64_si128(keygenassist00.m, x.m, 0x01) };
        printf("pclmul(0x%016llX%016llX, 0x01) = 0x%016llX%016llX\n", x.v[1], x.v[0], pclmul01.v[1], pclmul01.v[0]);
        mm128i pclmul10 = { .m = _mm_clmulepi64_si128(keygenassist00.m, x.m, 0x10) };
        printf("pclmul(0x%016llX%016llX, 0x10) = 0x%016llX%016llX\n", x.v[1], x.v[0], pclmul10.v[1], pclmul10.v[0]);
        mm128i pclmul11 = { .m = _mm_clmulepi64_si128(keygenassist00.m, x.m, 0x11) };
        printf("pclmul(0x%016llX%016llX, 0xFF) = 0x%016llX%016llX\n", x.v[1], x.v[0], pclmul11.v[1], pclmul11.v[0]);
    } while (!need_stop(&x));
}
