#include <stdint.h>

#include "box64stack.h"
#include "x64emu.h"
#include "x64run_private.h"
#include "x64emu_private.h"
#include "x64shaext.h"

static uint32_t rol(uint32_t a, int n)
{
    n = n&31;
    if(!n)
        return a;
    return (a<<n) | (a>>(32-n));
}

static uint32_t ror(uint32_t a, int n)
{
    n = n&31;
    if(!n)
        return a;
    return (a>>n) | (a<<(32-n));
}

static uint32_t f0(uint32_t B, uint32_t C, uint32_t D)
{
    return (B & C) ^ ((~B) & D);
}

static uint32_t f1(uint32_t B, uint32_t C, uint32_t D)
{
    return B ^ C ^ D;
}

static uint32_t f2(uint32_t B, uint32_t C, uint32_t D)
{
    return (B & C) ^ (B & D) ^ (C & D);
}

static uint32_t f3(uint32_t B, uint32_t C, uint32_t D)
{
    return B ^ C ^ D;
}

static uint32_t Ch(uint32_t E, uint32_t F, uint32_t G)
{
    return (E & F) ^ ((~E) & G);
}

static uint32_t Maj(uint32_t A, uint32_t B, uint32_t C)
{
    return (A & B) ^ (A & C) ^ (B & C);
}

static uint32_t sigma0(uint32_t A)
{
    return ror(A, 2) ^ ror(A, 13) ^ ror(A, 22);
}
static uint32_t sigma1(uint32_t E)
{
    return ror(E, 6) ^ ror(E, 11) ^ ror(E, 25);
}
static uint32_t tho0(uint32_t W)
{
    return ror(W, 7) ^ ror(W, 18) ^ (W>>3);
}
static uint32_t tho1(uint32_t W)
{
    return ror(W, 17) ^ ror(W, 19) ^ (W>>10);
}

static const uint32_t Ks[] = { 0x5A827999, 0x6ED9EBA1, 0X8F1BBCDC, 0xCA62C1D6 };

void sha1nexte(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2)
{
    uint32_t tmp = rol(xmm1->ud[3], 30);
    xmm1->ud[3] = xmm2->ud[3] + tmp;
    xmm1->ud[2] = xmm2->ud[2];
    xmm1->ud[1] = xmm2->ud[1];
    xmm1->ud[0] = xmm2->ud[0];
}

void sha1msg1(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2)
{
    uint32_t w0 = xmm1->ud[3];
    uint32_t w1 = xmm1->ud[2];
    uint32_t w2 = xmm1->ud[1];
    uint32_t w3 = xmm1->ud[0];
    uint32_t w4 = xmm2->ud[3];
    uint32_t w5 = xmm2->ud[2];
    xmm1->ud[3] = w2 ^ w0;
    xmm1->ud[2] = w3 ^ w1;
    xmm1->ud[1] = w4 ^ w2;
    xmm1->ud[0] = w5 ^ w3;
}

void sha1msg2(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2)
{
    uint32_t w13 = xmm2->ud[2];
    uint32_t w14 = xmm2->ud[1];
    uint32_t w15 = xmm2->ud[0];
    uint32_t w16 = rol(xmm1->ud[3] ^ w13, 1);
    uint32_t w17 = rol(xmm1->ud[2] ^ w14, 1);
    uint32_t w18 = rol(xmm1->ud[1] ^ w15, 1);
    uint32_t w19 = rol(xmm1->ud[0] ^ w16, 1);
    xmm1->ud[3] = w16;
    xmm1->ud[2] = w17;
    xmm1->ud[1] = w18;
    xmm1->ud[0] = w19;
}

void sha256msg1(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2)
{
    uint32_t w4 = xmm2->ud[0];
    uint32_t w3 = xmm1->ud[3];
    uint32_t w2 = xmm1->ud[2];
    uint32_t w1 = xmm1->ud[1];
    uint32_t w0 = xmm1->ud[0];
    xmm1->ud[3] = w3 + tho0(w4);
    xmm1->ud[2] = w2 + tho0(w3);
    xmm1->ud[1] = w1 + tho0(w2);
    xmm1->ud[0] = w0 + tho0(w1);
}

void sha256msg2(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2)
{
    uint32_t w14 = xmm2->ud[2];
    uint32_t w15 = xmm2->ud[3];
    uint32_t w16 = xmm1->ud[0] + tho1(w14);
    uint32_t w17 = xmm1->ud[1] + tho1(w15);
    uint32_t w18 = xmm1->ud[2] + tho1(w16);
    uint32_t w19 = xmm1->ud[3] + tho1(w17);
    xmm1->ud[3] = w19;
    xmm1->ud[2] = w18;
    xmm1->ud[1] = w17;
    xmm1->ud[0] = w16;
}

void sha1rnds4(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2, uint8_t ib)
{
    uint32_t K = Ks[ib&3];
    uint32_t(*f)(uint32_t , uint32_t , uint32_t) = NULL;
    switch (ib&3) {
        case 0: f = f0; break;
        case 1: f = f1; break;
        case 2: f = f2; break;
        case 3: f = f3; break;
    }
    uint32_t A = xmm1->ud[3];
    uint32_t B = xmm1->ud[2];
    uint32_t C = xmm1->ud[1];
    uint32_t D = xmm1->ud[0];
    uint32_t E = 0;
    for(int i=0; i<4; ++i) {
        uint32_t new_A = f(B, C, D) + rol(A, 5) + xmm2->ud[3-i] + E + K;
        E = D;
        D = C;
        C = rol(B, 30);
        B = A;
        A = new_A;
    }
    xmm1->ud[3] = A;
    xmm1->ud[2] = B;
    xmm1->ud[1] = C;
    xmm1->ud[0] = D;
}

void sha256rnds2(x64emu_t* emu, sse_regs_t* xmm1, sse_regs_t* xmm2)
{
    uint32_t A = xmm2->ud[3];
    uint32_t B = xmm2->ud[2];
    uint32_t C = xmm1->ud[3];
    uint32_t D = xmm1->ud[2];
    uint32_t E = xmm2->ud[1];
    uint32_t F = xmm2->ud[0];
    uint32_t G = xmm1->ud[1];
    uint32_t H = xmm1->ud[0];
    for(int i=0; i<2; ++i) {
        uint32_t new_A = Ch(E, F, G) + sigma1(E) + emu->xmm[0].ud[i] + H + Maj(A, B, C) + sigma0(A);
        uint32_t new_E = Ch(E, F, G) + sigma1(E) + emu->xmm[0].ud[i] + H + D;
        H = G;
        G = F;
        F = E;
        E = new_E;
        D = C;
        C = B;
        B = A;
        A = new_A;
    }
    xmm1->ud[3] = A;
    xmm1->ud[2] = B;
    xmm1->ud[1] = E;
    xmm1->ud[0] = F;
}