#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "arm64_printer.h"
#include "debug.h"

static const char* Xt[] = {"xEmu", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "xRAX", "xRCX", "xRDX", "xRBX", "xRSP", "xRBP", "xRSI", "xRDI", "xR8", "xR9", "xR10", "xR11", "xR12", "xR13", "xR14", "xR15", "xFlags", "xRIP", "x28", "FP", "LR", "xZR"};
static const char* XtSp[] = {"xEmu", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "xRAX", "xRCX", "xRDX", "xRBX", "xRSP", "xRBP", "xRSI", "xRDI", "xR8", "xR9", "xR10", "xR11", "xR12", "xR13", "xR14", "xR15", "xFlags", "xRIP", "x28", "FP", "LR", "SP"};
static const char* Wt[] = {"w0", "w1", "w2", "w3", "w4", "w5", "w6", "w7", "w8", "w9", "wEAX", "wECX", "wEDX", "wEBX", "wESP", "wEBP", "wESI", "wEDI", "wR8", "wR9", "wR10", "wR11", "wR12", "wR13", "wR14", "wR15", "wFlags", "w27", "w28", "w29", "w30", "wZR"};
static const char* WtSp[] = {"w0", "w1", "w2", "w3", "w4", "w5", "w6", "w7", "w8", "w9", "wEAX", "wECX", "wEDX", "wEBX", "wESP", "wEBP", "wESI", "wEDI", "wR8", "wR9", "wR10", "wR11", "wR12", "wR13", "wR14", "wR15", "wFlags", "w27", "w28", "w29", "w30", "wSP"};

static const char* conds[] = {"cEQ", "cNE", "cCS", "cCC", "cMI", "cPL", "cVS", "cVC", "cHI", "cLS", "cGE", "cLT", "cGT", "cLE", "c__", "inv"};

#define abs(A) (((A)<0)?(-(A)):(A))

typedef struct arm64_print_s {
    int N, S, U, L, Q, A, R;
    int t, n, m, d, t2, a;
    int f, c, o, h, p;
    int i, r, s;
    int x, w;
} arm64_print_t;

uint64_t DecodeBitMasks(int N, int imms, int immr)
{
    int len = 31-__builtin_clz((N<<6) | ((~imms)&0b111111));
    if(len<1) return 0;
    int levels = (1<<len) - 1;
    int s = imms & levels;
    int r = immr & levels;  // this the ROR factor
    int e = 1<<len; // actual number of bits
    if(s==levels) return 0;
    uint64_t mask = (1LL<<(s+1))-1;
    if(r) { // rotate
        mask=(mask>>r)|(mask<<(e-r));
        if(e<64) {
            mask&=((1LL<<e)-1);
        }
    }
    while(e<64) {  // replicate
        mask|=(mask<<e);
        e<<=1;
    }
    return mask;
}

int isMask(uint32_t opcode, const char* mask, arm64_print_t *a)
{
    if(strlen(mask)!=32) {
        printf_log(LOG_NONE, "Error: printer mask \"%s\" in not len 32 but %ld\n", mask, strlen(mask));
        return 0;
    }
    memset(a, 0, sizeof(*a));
    int i = 31;
    while(*mask) {
        uint8_t v = (opcode>>i)&1;
        switch(*mask) {
            case '0': if(v!=0) return 0; break;
            case '1': if(v!=1) return 0; break;
            case 'N': a->N = (a->N<<1) | v; break;
            case 'S': a->S = (a->S<<1) | v; break;
            case 'U': a->U = (a->U<<1) | v; break;
            case 'A': a->A = (a->A<<1) | v; break;
            case 'R': a->R = (a->R<<1) | v; break;
            case 'L': a->L = (a->L<<1) | v; break;
            case 'Q': a->Q = (a->Q<<1) | v; break;
            case 't': a->t = (a->t<<1) | v; break;
            case '2': a->t2 = (a->t2<<1) | v; break;
            case 'n': a->n = (a->n<<1) | v; break;
            case 'p': a->p = (a->p<<1) | v; break;
            case 'm': a->m = (a->m<<1) | v; break;
            case 'a': a->a = (a->a<<1) | v; break;
            case 'd': a->d = (a->d<<1) | v; break;
            case 'f': a->f = (a->f<<1) | v; break;
            case 'c': a->c = (a->c<<1) | v; break;
            case 'i': a->i = (a->i<<1) | v; break;
            case 'r': a->r = (a->r<<1) | v; break;
            case 's': a->s = (a->s<<1) | v; break;
            case 'o': a->o = (a->o<<1) | v; break;
            case 'h': a->h = (a->h<<1) | v; break;
            case 'w': a->w = (a->w<<1) | v; break;
            case 'x': a->x = (a->x<<1) | v; break;
            default:
                printf_log(LOG_NONE, "Warning, printer mask use unhandled '%c'\n", *mask);
        }
        mask++;
        --i;
    }
    
    return 1;
}

int64_t signExtend(uint32_t val, int sz)
{
    int64_t ret = val;
    if((val>>(sz-1))&1)
        ret |= (0xffffffffffffffffll<<sz);
    return ret;
}

const char* arm64_print(uint32_t opcode, uintptr_t addr)
{
    static char buff[200];
    arm64_print_t a;
    #define Rn a.n
    #define Rt a.t
    #define Rt2 a.t2
    #define Rm a.m
    #define Rd a.d
    #define Ra a.a
    #define Rs a.s
    #define sf a.f
    #define imm a.i
    #define option a.o
    #define shift a.h
    #define hw a.w
    #define cond a.c
    #define immr a.r
    #define imms a.s
    #define opc a.c
    if(isMask(opcode, "11010101000000110010000000011111", &a)) {
        snprintf(buff, sizeof(buff), "NOP");
        return buff;
    }
    if(isMask(opcode, "11010101000000110010000001011111", &a)) {
        snprintf(buff, sizeof(buff), "WFE");
        return buff;
    }
    if(isMask(opcode, "11010101000000110010000001111111", &a)) {
        snprintf(buff, sizeof(buff), "WFI");
        return buff;
    }
    if(isMask(opcode, "11010101000000110010000000111111", &a)) {
        snprintf(buff, sizeof(buff), "YIELD");
        return buff;
    }
    if(isMask(opcode, "11010101000000110010000010111111", &a)) {
        snprintf(buff, sizeof(buff), "SEVL");
        return buff;
    }
    // --- LDR / STR
    if(isMask(opcode, "f010100011iiiiiii22222nnnnnttttt", &a)) {
        int offset = signExtend(imm, 7)<<(2+sf);
        snprintf(buff, sizeof(buff), "LDP %s, %s, [%s], %s0x%x", sf?Xt[Rt]:Wt[Rt], sf?Xt[Rt2]:Wt[Rt2], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "f010100111iiiiiii22222nnnnnttttt", &a)) {
        int offset = signExtend(imm, 7)<<(2+sf);
        snprintf(buff, sizeof(buff), "LDP %s, %s, [%s, %s0x%x]!", sf?Xt[Rt]:Wt[Rt], sf?Xt[Rt2]:Wt[Rt2], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "f010100101iiiiiii22222nnnnnttttt", &a)) {
        int offset = signExtend(imm, 7)<<(2+sf);
        if(!offset)
            snprintf(buff, sizeof(buff), "LDP %s, %s, [%s]", sf?Xt[Rt]:Wt[Rt], sf?Xt[Rt2]:Wt[Rt2], XtSp[Rn]);
        else
            snprintf(buff, sizeof(buff), "LDP %s, %s, [%s, %s0x%x]", sf?Xt[Rt]:Wt[Rt], sf?Xt[Rt2]:Wt[Rt2], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "f010100010iiiiiii22222nnnnnttttt", &a)) {
        int offset = signExtend(imm, 7)<<(2+sf);
        snprintf(buff, sizeof(buff), "STP %s, %s, [%s], %s0x%x", sf?Xt[Rt]:Wt[Rt], sf?Xt[Rt2]:Wt[Rt2], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "f010100110iiiiiii22222nnnnnttttt", &a)) {
        int offset = signExtend(imm, 7)<<(2+sf);
        snprintf(buff, sizeof(buff), "STP %s, %s, [%s, %s0x%x]!", sf?Xt[Rt]:Wt[Rt], sf?Xt[Rt2]:Wt[Rt2], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "f010100100iiiiiii22222nnnnnttttt", &a)) {
        int offset = signExtend(imm, 7)<<(2+sf);
        if(!offset)
            snprintf(buff, sizeof(buff), "STP %s, %s, [%s]", sf?Xt[Rt]:Wt[Rt], sf?Xt[Rt2]:Wt[Rt2], XtSp[Rn]);
        else
            snprintf(buff, sizeof(buff), "STP %s, %s, [%s, %s0x%x]", sf?Xt[Rt]:Wt[Rt], sf?Xt[Rt2]:Wt[Rt2], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "1x111000010iiiiiiiii01nnnnnttttt", &a)) {
        int size = (opcode>>30)&3;
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "LDUR %s, [%s], %s0x%x", (size==0b10)?Wt[Rt]:Xt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "1x111000010iiiiiiiii11nnnnnttttt", &a)) {
        int size = (opcode>>30)&3;
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "LDUR %s, [%s, %s0x%x]!", (size==0b10)?Wt[Rt]:Xt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "1x111000010iiiiiiiii00nnnnnttttt", &a)) {
        int size = (opcode>>30)&3;
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "LDUR %s, [%s, %s0x%x]", (size==0b10)?Wt[Rt]:Xt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "00111000010iiiiiiiii00nnnnnttttt", &a)) {
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "LDURB %s, [%s, %s0x%x]", Wt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "01111000010iiiiiiiii00nnnnnttttt", &a)) {
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "LDURH %s, [%s, %s0x%x]", Wt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "001110001x0iiiiiiiii00nnnnnttttt", &a)) {
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "LDURSB %s, [%s, %s0x%x]", a.x?Xt[Rt]:Wt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "011110001x0iiiiiiiii00nnnnnttttt", &a)) {
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "LDURSH %s, [%s, %s0x%x]", a.x?Xt[Rt]:Wt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "10111000100iiiiiiiii00nnnnnttttt", &a)) {
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "LDURSW %s, [%s, %s0x%x]", Xt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "1x111000000iiiiiiiii00nnnnnttttt", &a)) {
        int size = (opcode>>30)&3;
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "STUR %s, [%s, %s0x%x]", (size==0b10)?Wt[Rt]:Xt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "00111000000iiiiiiiii00nnnnnttttt", &a)) {
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "STURB %s, [%s, %s0x%x]", Wt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "01111000000iiiiiiiii00nnnnnttttt", &a)) {
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "STURH %s, [%s, %s0x%x]", Wt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "1x11100101iiiiiiiiiiiinnnnnttttt", &a)) {
        int size = (opcode>>30)&3;
        int offset = (imm)<<size;
        if(offset)
            snprintf(buff, sizeof(buff), "LDR %s, [%s, 0x%x]", (size==0b10)?Wt[Rt]:Xt[Rt], XtSp[Rn], offset);
        else
            snprintf(buff, sizeof(buff), "LDR %s, [%s]", (size==0b10)?Wt[Rt]:Xt[Rt], XtSp[Rn]);
        return buff;
    }
    if(isMask(opcode, "0x011000iiiiiiiiiiiiiiiiiiittttt", &a)) {
        int size = ((opcode>>30)&1)?3:2;
        int offset = signExtend(imm, 19)<<2;
        snprintf(buff, sizeof(buff), "LDR %s, [#%+d]\t;%p", (size==2)?Wt[Rt]:Xt[Rt], offset, (void*)(addr+offset));
        return buff;
    }
    if(isMask(opcode, "10011000iiiiiiiiiiiiiiiiiiittttt", &a)) {
        int offset = signExtend(imm, 19)<<2;
        snprintf(buff, sizeof(buff), "LDRSW %s, [#%+d]\t;%p", Xt[Rt], offset, (void*)(addr+offset));
        return buff;
    }
    if(isMask(opcode, "ff011100iiiiiiiiiiiiiiiiiiittttt", &a)) {
        int offset = signExtend(imm, 19)<<2;
        const char* Y[] = {"S", "D", "Q", "?"};
        snprintf(buff, sizeof(buff), "LDR %s%d, [#%+d]\t;%p", Y[sf], Rt, offset, (void*)(addr+offset));
        return buff;
    }
    if(isMask(opcode, "1x111000011mmmmmoooS10nnnnnttttt", &a)) {
        int size = (opcode>>30)&3;
        const char* extend[] = {"?0", "?1", "UXTW", "LSL", "?4", "?5", "SXTW", "SXTX"};
        int amount = size*a.S;
        if(option==3 && !amount)
            snprintf(buff, sizeof(buff), "LDR %s, [%s, %s]", (size==2)?Wt[Rt]:Xt[Rt], XtSp[Rn], ((option&1)==0)?Wt[Rm]:Xt[Rm]);
        else
            snprintf(buff, sizeof(buff), "LDR %s, [%s, %s, %s %d]", (size==2)?Wt[Rt]:Xt[Rt], XtSp[Rn], ((option&1)==0)?Wt[Rm]:Xt[Rm], extend[option], amount);
        return buff;
    }
    if(isMask(opcode, "1x111000000iiiiiiiii01nnnnnttttt", &a)) {
        int size = (opcode>>30)&3;
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "STR %s, [%s], %s0x%x", (size==0b10)?Wt[Rt]:Xt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "1x111000000iiiiiiiii11nnnnnttttt", &a)) {
        int size = (opcode>>30)&3;
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "STR %s, [%s, %s0x%x]!", (size==0b10)?Wt[Rt]:Xt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "1x11100100iiiiiiiiiiiinnnnnttttt", &a)) {
        int size = (opcode>>30)&3;
        int offset = (imm)<<size;
        if(offset)
            snprintf(buff, sizeof(buff), "STR %s, [%s, 0x%x]", (size==0b10)?Wt[Rt]:Xt[Rt], XtSp[Rn], offset);
        else
            snprintf(buff, sizeof(buff), "STR %s, [%s]", (size==0b10)?Wt[Rt]:Xt[Rt], XtSp[Rn]);
        return buff;
    }
    if(isMask(opcode, "1x111000001mmmmmoooS10nnnnnttttt", &a)) {
        int size = (opcode>>30)&3;
        const char* extend[] = {"?0", "?1", "UXTW", "LSL", "?4", "?5", "SXTW", "SXTX"};
        int amount = size*a.S;
        if(option==3 && !amount)
            snprintf(buff, sizeof(buff), "STR %s, [%s, %s]", (size==2)?Wt[Rt]:Xt[Rt], XtSp[Rn], ((option&1)==0)?Wt[Rm]:Xt[Rm]);
        else
            snprintf(buff, sizeof(buff), "STR %s, [%s, %s, %s %d]", (size==2)?Wt[Rt]:Xt[Rt], XtSp[Rn], ((option&1)==0)?Wt[Rm]:Xt[Rm], extend[option], amount);
        return buff;
    }
    if(isMask(opcode, "00111000001mmmmmoooS10nnnnnttttt", &a)) {
        const char* extend[] = {"?0", "?1", "UXTW", "LSL", "?4", "?5", "SXTW", "SXTX"};
        int amount = a.S;
        if(option==3 && !amount)
            snprintf(buff, sizeof(buff), "STRB %s, [%s, %s]", Wt[Rt], XtSp[Rn], ((option&1)==0)?Wt[Rm]:Xt[Rm]);
        else
            snprintf(buff, sizeof(buff), "STRB %s, [%s, %s, %s %d]", Wt[Rt], XtSp[Rn], ((option&1)==0)?Wt[Rm]:Xt[Rm], extend[option], amount);
        return buff;
    }
    if(isMask(opcode, "0x111000010iiiiiiiii01nnnnnttttt", &a)) {
        int size = a.x;
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "LDR%c %s, [%s], %s0x%x", size?'H':'B', Xt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "0x111000010iiiiiiiii11nnnnnttttt", &a)) {
        int size = a.x;
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "LDR%c %s, [%s, %s0x%x]!", size?'H':'B', Xt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "0x11100101iiiiiiiiiiiinnnnnttttt", &a)) {
        int size = a.x;
        int offset = (imm)<<size;
        if(offset)
            snprintf(buff, sizeof(buff), "LDR%c %s, [%s, 0x%x]", size?'H':'B', Xt[Rt], XtSp[Rn], offset);
        else
            snprintf(buff, sizeof(buff), "LDR%c %s, [%s]", size?'H':'B', Xt[Rt], XtSp[Rn]);
        return buff;
    }
    if(isMask(opcode, "0x111000000iiiiiiiii01nnnnnttttt", &a)) {
        int size = a.x;
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "STR%c %s, [%s], %s0x%x", size?'H':'B', Xt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "0x111000000iiiiiiiii11nnnnnttttt", &a)) {
        int size = a.x;
        int offset = signExtend(imm, 9);
        snprintf(buff, sizeof(buff), "STR%c %s, [%s, %s0x%x]!", size?'H':'B', Xt[Rt], XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    if(isMask(opcode, "0x11100100iiiiiiiiiiiinnnnnttttt", &a)) {
        int size = a.x;
        int offset = (imm)<<size;
        if(offset)
            snprintf(buff, sizeof(buff), "STR%c %s, [%s, 0x%x]", size?'H':'B', Xt[Rt], XtSp[Rn], offset);
        else
            snprintf(buff, sizeof(buff), "STR%c %s, [%s]", size?'H':'B', Xt[Rt], XtSp[Rn]);
        return buff;
    }    
    if(isMask(opcode, "1011100110iiiiiiiiiiiinnnnnttttt", &a)) {
        int offset = imm<<2;
        if(!offset)
            snprintf(buff, sizeof(buff), "LDRSW %s, [%s]", Xt[Rt], XtSp[Rn]);
        else
            snprintf(buff, sizeof(buff), "LDRSW %s, [%s, #%d]", Xt[Rt], XtSp[Rn], offset);
        return buff;
    }
    if(isMask(opcode, "011110011xiiiiiiiiiiiinnnnnttttt", &a)) {
        int offset = imm<<1;
        if(!offset)
            snprintf(buff, sizeof(buff), "LDRSH %s, [%s]", a.x?Wt[Rt]:Xt[Rt], XtSp[Rn]);
        else
            snprintf(buff, sizeof(buff), "LDRSH %s, [%s, #%d]", a.x?Wt[Rt]:Xt[Rt], XtSp[Rn], offset);
        return buff;
    }
    if(isMask(opcode, "001110011xiiiiiiiiiiiinnnnnttttt", &a)) {
        int offset = imm<<1;
        if(!offset)
            snprintf(buff, sizeof(buff), "LDRSB %s, [%s]", a.x?Wt[Rt]:Xt[Rt], XtSp[Rn]);
        else
            snprintf(buff, sizeof(buff), "LDRSB %s, [%s, #%d]", a.x?Wt[Rt]:Xt[Rt], XtSp[Rn], offset);
        return buff;
    }

    if(isMask(opcode, "ff0010000L0sssss111111nnnnnttttt", &a)) {
        if(a.L)
            snprintf(buff, sizeof(buff), "LDAXR%s %s, [%s]", (sf==0)?"B":((sf==1)?"H":""), (sf==2)?Wt[Rt]:Xt[Rt], XtSp[Rn]);
        else
            snprintf(buff, sizeof(buff), "STLXR%s %s, %s, [%s]", (sf==0)?"B":((sf==1)?"H":""), (sf==2)?Wt[Rs]:Xt[Rs], (sf==2)?Wt[Rt]:Xt[Rt], XtSp[Rn]);
        return buff;
    }

    if(isMask(opcode, "ff0010000L0sssss011111nnnnnttttt", &a)) {
        if(a.L)
            snprintf(buff, sizeof(buff), "LDXR%s %s, [%s]", (sf==0)?"B":((sf==1)?"H":""), (sf==2)?Wt[Rt]:Xt[Rt], XtSp[Rn]);
        else
            snprintf(buff, sizeof(buff), "STXR%s %s, %s, [%s]", (sf==0)?"B":((sf==1)?"H":""), (sf==2)?Wt[Rs]:Xt[Rs], (sf==2)?Wt[Rt]:Xt[Rt], XtSp[Rn]);
        return buff;
    }

    // --- MOV (REGS: see Logic MOV==ORR, MVN==ORN)
    if(isMask(opcode, "f10100101wwiiiiiiiiiiiiiiiiddddd", &a)) {
        if(!hw)
            snprintf(buff, sizeof(buff), "MOVZ %s, 0x%x", sf?Xt[Rd]:Wt[Rd], imm);
        else
            snprintf(buff, sizeof(buff), "MOVZ %s, 0x%x LSL %d", sf?Xt[Rd]:Wt[Rd], imm, 16*hw);
        return buff;
    }
    if(isMask(opcode, "f00100101wwiiiiiiiiiiiiiiiiddddd", &a)) {
        if(sf) {
            uint64_t noti=~(((uint64_t)imm)<<(hw*16));
            if(!hw)
                snprintf(buff, sizeof(buff), "MOVN %s, 0x%x\t; 0x%"PRIx64, Xt[Rd], imm, noti);
            else
                snprintf(buff, sizeof(buff), "MOVN %s, 0x%x LSL %d\t; 0x%"PRIx64, Xt[Rd], imm, 16*hw, noti);
        } else {
            uint32_t noti=~(((uint32_t)imm)<<(hw*16));
            if(!hw)
                snprintf(buff, sizeof(buff), "MOVN %s, 0x%x\t; 0x%x", Wt[Rd], imm, noti);
            else
                snprintf(buff, sizeof(buff), "MOVN %s, 0x%x LSL %d\t; 0x%x", Wt[Rd], imm, 16*hw, noti);
        }
        return buff;
    }
    if(isMask(opcode, "f11100101wwiiiiiiiiiiiiiiiiddddd", &a)) {
        if(!hw)
            snprintf(buff, sizeof(buff), "MOVK %s, 0x%x", sf?Xt[Rd]:Wt[Rd], imm);
        else
            snprintf(buff, sizeof(buff), "MOVK %s, 0x%x LSL %d", sf?Xt[Rd]:Wt[Rd], imm, 16*hw);
        return buff;
    }

    // --- MATH
    if(isMask(opcode, "f1101011001mmmmmoooiiinnnnn11111", &a)) {
        int R = 0;
        if(option==0b011 || option==0b111)
            R = 1;
        const char* extends[] ={"UXTB", "UXTH", "UXTW", "UXTX", "SXTB", "SXTH", "SXTW", "SXTX"};
        if(imm==0 && option==(sf?3:2))
            snprintf(buff, sizeof(buff), "CMP %s, %s", sf?XtSp[Rn]:WtSp[Rn], (sf&&R)?Xt[Rm]:Wt[Rm]);
        else
            snprintf(buff, sizeof(buff), "CMP %s, %s, %s 0x%x", sf?XtSp[Rn]:WtSp[Rn], (sf&&R)?Xt[Rm]:Wt[Rm], extends[option], imm);
        return buff;
    }
    if(isMask(opcode, "f11100010hiiiiiiiiiiiinnnnn11111", &a)) {
        if(shift==0)
            snprintf(buff, sizeof(buff), "CMP %s, 0x%x", sf?XtSp[Rn]:WtSp[Rn], imm);
        else
            snprintf(buff, sizeof(buff), "CMP %s, 0x%x", sf?XtSp[Rn]:WtSp[Rn], imm<<12);
        return buff;
    }
    if(isMask(opcode, "f1101011hh0mmmmmiiiiiinnnnn11111", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "???"};
        if(shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "CMP %s, %s", sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else 
            snprintf(buff, sizeof(buff), "CMP %s, %s %s %d", sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        return buff;
    }
    if(isMask(opcode, "f0010001hhiiiiiiiiiiiinnnnnddddd", &a)) {
        if((Rd==31 || Rn==31) && shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "MOV %s, %s, 0x%x", sf?XtSp[Rd]:WtSp[Rd], sf?XtSp[Rn]:WtSp[Rn], imm);
        else if(shift==0)
            snprintf(buff, sizeof(buff), "ADD %s, %s, 0x%x", sf?XtSp[Rd]:WtSp[Rd], sf?XtSp[Rn]:WtSp[Rn], imm);
        else if (shift==1)
            snprintf(buff, sizeof(buff), "ADD %s, %s, 0x%x", sf?XtSp[Rd]:WtSp[Rd], sf?XtSp[Rn]:WtSp[Rn], imm<<12);
        else 
            snprintf(buff, sizeof(buff), "ADD with unhandled shift %d", shift);
        return buff;
    }
    if(isMask(opcode, "f0110001hhiiiiiiiiiiiinnnnnddddd", &a)) {
        if(shift==0)
            snprintf(buff, sizeof(buff), "ADDS %s, %s, 0x%x", sf?Xt[Rd]:Wt[Rd], sf?XtSp[Rn]:WtSp[Rn], imm);
        else if (shift==1)
            snprintf(buff, sizeof(buff), "ADDS %s, %s, 0x%x", sf?Xt[Rd]:Wt[Rd], sf?XtSp[Rn]:WtSp[Rn], imm<<12);
        else 
            snprintf(buff, sizeof(buff), "ADDS with unhandled shift %d", shift);
        return buff;
    }
    if(isMask(opcode, "f0001011hh0mmmmmiiiiiinnnnnddddd", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "???"};
        if(shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "ADD %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else 
            snprintf(buff, sizeof(buff), "ADD %s, %s, %s %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        return buff;
    }
    if(isMask(opcode, "f0101011hh0mmmmmiiiiiinnnnnddddd", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "???"};
        if(shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "ADDS %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else 
            snprintf(buff, sizeof(buff), "ADDS %s, %s, %s %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        return buff;
    }
    if(isMask(opcode, "f1010001hhiiiiiiiiiiiinnnnnddddd", &a)) {
        if(shift==0)
            snprintf(buff, sizeof(buff), "SUB %s, %s, 0x%x", sf?XtSp[Rd]:WtSp[Rd], sf?XtSp[Rn]:WtSp[Rn], imm);
        else if (shift==1)
            snprintf(buff, sizeof(buff), "SUB %s, %s, 0x%x", sf?XtSp[Rd]:WtSp[Rd], sf?XtSp[Rn]:WtSp[Rn], imm<<12);
        else 
            snprintf(buff, sizeof(buff), "SUB with unhandled shift %d", shift);
        return buff;
    }
    if(isMask(opcode, "f1110001hhiiiiiiiiiiiinnnnnddddd", &a)) {
        if(shift==0)
            snprintf(buff, sizeof(buff), "SUBS %s, %s, 0x%x", sf?Xt[Rd]:Wt[Rd], sf?XtSp[Rn]:WtSp[Rn], imm);
        else if (shift==1)
            snprintf(buff, sizeof(buff), "SUBS %s, %s, 0x%x", sf?Xt[Rd]:Wt[Rd], sf?XtSp[Rn]:WtSp[Rn], imm<<12);
        else 
            snprintf(buff, sizeof(buff), "SUBS with unhandled shift %d", shift);
        return buff;
    }
    if(isMask(opcode, "f1001011hh0mmmmmiiiiiinnnnnddddd", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "???"};
        if(shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "SUB %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else 
            snprintf(buff, sizeof(buff), "SUB %s, %s, %s %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        return buff;
    }
    if(isMask(opcode, "f1101011hh0mmmmmiiiiiinnnnnddddd", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "???"};
        if(shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "SUBS %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else 
            snprintf(buff, sizeof(buff), "SUBS %s, %s, %s %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        return buff;
    }
    if(isMask(opcode, "f0011010000mmmmm000000nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "ADC %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        return buff;
    }
    if(isMask(opcode, "f0111010000mmmmm000000nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "ADCS %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        return buff;
    }
    if(isMask(opcode, "f1011010000mmmmm000000nnnnnddddd", &a)) {
        if(Rn==31)
            snprintf(buff, sizeof(buff), "NGC %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rm]:Wt[Rm]);
        else
            snprintf(buff, sizeof(buff), "SBC %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        return buff;
    }
    if(isMask(opcode, "f1111010000mmmmm000000nnnnnddddd", &a)) {
        if(Rn==31)
            snprintf(buff, sizeof(buff), "NGCS %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rm]:Wt[Rm]);
        else
            snprintf(buff, sizeof(buff), "SBCS %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        return buff;
    }
    if(isMask(opcode, "f1101011hh0mmmmmiiiiiinnnnnddddd", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "???"};
        if(shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "SUBS %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else 
            snprintf(buff, sizeof(buff), "SUBS %s, %s, %s %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        return buff;
    }
    if(isMask(opcode, "f1001011001mmmmmoooiiinnnnnddddd", &a)) {
        const char* shifts[2][8] = {{ "UXTB", "UXTH", "LSL", "UXTX", "SXTB", "SXTH", "SXTW", "SXTX"}, {"UXTB", "UXTH", "UXTW", "LSL", "SXTB", "SXTH", "SXTW", "SXTX" }};
        if(((sf==0 && shift==2) || (sf==1 && shift==3)) && imm==0)
            snprintf(buff, sizeof(buff), "SUB %s, %s, %s", sf?XtSp[Rd]:WtSp[Rd], sf?XtSp[Rn]:WtSp[Rn], sf?XtSp[Rm]:WtSp[Rm]);
        else if(shift==0)
            snprintf(buff, sizeof(buff), "SUB %s, %s, %s %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[sf][shift]);
        else
            snprintf(buff, sizeof(buff), "SUB %s, %s, %s %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[sf][shift], imm);
        return buff;
    }
    // ---- LOGIC
    if(isMask(opcode, "f11100100Nrrrrrrssssssnnnnnddddd", &a)) {
        uint64_t i = DecodeBitMasks(a.N, imms, immr);
        if(!sf) i&=0xffffffff;
        if(sf==0 && a.N==1)
            snprintf(buff, sizeof(buff), "invalid ANDS %s, %s, 0x%"PRIx64, Wt[Rd], Wt[Rn], i);
        else if(Rd==31)
            snprintf(buff, sizeof(buff), "TST %s, 0x%"PRIx64, sf?Xt[Rn]:Wt[Rn], i);
        else
            snprintf(buff, sizeof(buff), "ANDS %s, %s, 0x%"PRIx64, sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], i);
        return buff;
    }
    if(isMask(opcode, "f1101010hh0mmmmmiiiiiinnnnnddddd", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "ROR" };
        if(shift==0 && imm==0) {
            if(Rd==31)
                snprintf(buff, sizeof(buff), "TST %s, %s", sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
            else
                snprintf(buff, sizeof(buff), "ANDS %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        } else {
            if(Rd==31)
                snprintf(buff, sizeof(buff), "TST %s, %s, %s %d", sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
            else
                snprintf(buff, sizeof(buff), "ANDS %s, %s, %s, %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        }
        return buff;
    }
    if(isMask(opcode, "f0001010hh1mmmmmiiiiiinnnnnddddd", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "ROR" };
        if(shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "BIC %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else
            snprintf(buff, sizeof(buff), "BIC %s, %s, %s, %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        return buff;
    }
    if(isMask(opcode, "f1101010hh1mmmmmiiiiiinnnnnddddd", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "ROR" };
        if(shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "BICS %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else
            snprintf(buff, sizeof(buff), "BICS %s, %s, %s, %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        return buff;
    }
    if(isMask(opcode, "f01100100Nrrrrrrssssssnnnnnddddd", &a)) {
        uint64_t i = DecodeBitMasks(a.N, imms, immr);
        if(!sf) i&=0xffffffff;
        if(sf==0 && a.N==1)
            snprintf(buff, sizeof(buff), "invalid ORR %s, %s, 0x%"PRIx64, Wt[Rd], Wt[Rn], i);
        else
            snprintf(buff, sizeof(buff), "ORR %s, %s, 0x%"PRIx64, sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], i);
        return buff;
    }
    if(isMask(opcode, "f0101010hh1mmmmmiiiiiinnnnnddddd", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "ROR" };
        if(Rn==31) {
            if(shift==0 && imm==0)
                snprintf(buff, sizeof(buff), "MVN %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rm]:Wt[Rm]);
            else
                snprintf(buff, sizeof(buff), "MVN %s, %s, %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        } else if(shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "ORN %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else
            snprintf(buff, sizeof(buff), "ORN %s, %s, %s, %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        return buff;
    }
    if(isMask(opcode, "f0101010hh0mmmmmiiiiiinnnnnddddd", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "ROR" };
        if(Rn==31) {
            if(shift==0 && imm==0)
                snprintf(buff, sizeof(buff), "MOV %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rm]:Wt[Rm]);
            else
                snprintf(buff, sizeof(buff), "MOV %s, %s, %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        } else if(shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "ORR %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else
            snprintf(buff, sizeof(buff), "ORR %s, %s, %s, %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        return buff;
    }
    if(isMask(opcode, "f10100100Nrrrrrrssssssnnnnnddddd", &a)) {
        uint64_t i = DecodeBitMasks(a.N, imms, immr);
        if(!sf) i&=0xffffffff;
        if(sf==0 && a.N==1)
            snprintf(buff, sizeof(buff), "invalid EOR %s, %s, 0x%"PRIx64, Wt[Rd], Wt[Rn], i);
        else
            snprintf(buff, sizeof(buff), "EOR %s, %s, 0x%"PRIx64, sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], i);
        return buff;
    }
    if(isMask(opcode, "f1001010hh0mmmmmiiiiiinnnnnddddd", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "ROR" };
        if(shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "EOR %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else
            snprintf(buff, sizeof(buff), "EOR %s, %s, %s, %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        return buff;
    }
    if(isMask(opcode, "f00100100Nrrrrrrssssssnnnnnddddd", &a)) {
        uint64_t i = DecodeBitMasks(a.N, imms, immr);
        if(!sf) i&=0xffffffff;
        if(sf==0 && a.N==1)
            snprintf(buff, sizeof(buff), "invalid AND %s, %s, 0x%"PRIx64, Wt[Rd], Wt[Rn], i);
        else
            snprintf(buff, sizeof(buff), "AND %s, %s, 0x%"PRIx64, sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], i);
        return buff;
    }
    if(isMask(opcode, "f0001010hh0mmmmmiiiiiinnnnnddddd", &a)) {
        const char* shifts[] = { "LSL", "LSR", "ASR", "ROR" };
        if(shift==0 && imm==0)
            snprintf(buff, sizeof(buff), "AND %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else
            snprintf(buff, sizeof(buff), "AND %s, %s, %s, %s %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], shifts[shift], imm);
        return buff;
    }

    // ---- SHIFT
    if(isMask(opcode, "f10100110Nrrrrrrssssssnnnnnddddd", &a)) {
        if(sf && imms!=0b111111 && imms+1==immr)
            snprintf(buff, sizeof(buff), "LSL %s, %s, %d", Xt[Rd], Xt[Rn], 63-imms);
        else if(!sf && imms!=0b011111 && imms+1==immr)
            snprintf(buff, sizeof(buff), "LSL %s, %s, %d", Wt[Rd], Wt[Rn], 31-imms);
        else if(sf && imms==0b111111)
            snprintf(buff, sizeof(buff), "LSR %s, %s, %d", Xt[Rd], Xt[Rn], immr);
        else if(!sf && imms==0b011111)
            snprintf(buff, sizeof(buff), "LSR %s, %s, %d", Wt[Rd], Wt[Rn], immr);
        else if(immr==0 && imms==0b000111)
            snprintf(buff, sizeof(buff), "UXTB %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn]);
        else if(immr==0 && imms==0b001111)
            snprintf(buff, sizeof(buff), "UXTH %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn]);
        else if(imms>=immr)
            snprintf(buff, sizeof(buff), "UBFX %s, %s, %d, %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], immr, imms-immr+1);
        else
            snprintf(buff, sizeof(buff), "UBFM %s, %s, %d, %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], immr, imms);

        return buff;
    }

    if(isMask(opcode, "f0011010110mmmmm001010nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "ASR %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        return buff;
    }

    if(isMask(opcode, "f00100110Nrrrrrrssssssnnnnnddddd", &a)) {
        if(sf && imms==0b111111)
            snprintf(buff, sizeof(buff), "ASR %s, %s, %d", Xt[Rd], Xt[Rn], immr);
        else if(!sf && imms==0b011111)
            snprintf(buff, sizeof(buff), "ASR %s, %s, %d", Wt[Rd], Wt[Rn], immr);
        else if(immr==0 && imms==0b000111)
            snprintf(buff, sizeof(buff), "SXTB %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn]);
        else if(immr==0 && imms==0b001111)
            snprintf(buff, sizeof(buff), "SXTH %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn]);
        else if(sf && immr==0 && imms==0b011111)
            snprintf(buff, sizeof(buff), "SXTW %s, %s", Xt[Rd], Wt[Rn]);
        else if(imms>=immr)
            snprintf(buff, sizeof(buff), "SBFX %s, %s, %d, %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], immr, imms-immr+1);
        else
            snprintf(buff, sizeof(buff), "SBFM %s, %s, %d, %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], immr, imms);
        return buff;
    }

    if(isMask(opcode, "f00100111N0mmmmmssssssnnnnnddddd", &a)) {
        if(Rn==Rm)
            snprintf(buff, sizeof(buff), "ROR %s, %s, %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], imms);
        else
            snprintf(buff, sizeof(buff), "EXTR %s, %s, %s, %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], imms);
        return buff;
    }

    if(isMask(opcode, "f0011010110mmmmm001011nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "ROR %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        return buff;
    }

    if(isMask(opcode, "f0011010110mmmmm001001nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "LSR %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        return buff;
    }

    if(isMask(opcode, "f0011010110mmmmm001000nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "LSL %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        return buff;
    }

    if(isMask(opcode, "f01100110Nrrrrrrssssssnnnnnddddd", &a)) {
        if(imms<immr) {
            int width = imms + 1;
            int lsb = ((-immr)%(sf?64:32))&(sf?0x3f:0x1f);
            if(Rn==31)
                snprintf(buff, sizeof(buff), "BFC %s, %d, %d", sf?Xt[Rd]:Wt[Rd], lsb, width);
            else
                snprintf(buff, sizeof(buff), "BFI %s, %s, %d, %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], lsb, width);
        } else {
            if(Rn==31 && immr==0)
                snprintf(buff, sizeof(buff), "BFC %s, %d, %d", sf?Xt[Rd]:Wt[Rd], immr, imms-immr+1);
            else
                snprintf(buff, sizeof(buff), "BFXIL %s, %s, %d, %d", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], immr, imms-immr+1);
        }
        return buff;
    }
    // ---- BRANCH / TEST
    if(isMask(opcode, "1101011000011111000000nnnnn00000", &a)) {
        snprintf(buff, sizeof(buff), "BR %s", Xt[Rn]);
        return buff;
    }
    if(isMask(opcode, "1101011000111111000000nnnnn00000", &a)) {
        snprintf(buff, sizeof(buff), "BLR %s", Xt[Rn]);
        return buff;
    }
    if(isMask(opcode, "1101011001011111000000nnnnn00000", &a)) {
        if(Rn==30)
            snprintf(buff, sizeof(buff), "RET");
        else
            snprintf(buff, sizeof(buff), "RET %s", Xt[Rn]);
        return buff;
    }
    if(isMask(opcode, "01010100iiiiiiiiiiiiiiiiiii0cccc", &a)) {
        int offset = signExtend(imm, 19)<<2;
        snprintf(buff, sizeof(buff), "B.%s #+%di\t; %p", conds[cond], offset>>2, (void*)(addr + offset));
        return buff;
    }
    if(isMask(opcode, "000101iiiiiiiiiiiiiiiiiiiiiiiiii", &a)) {
        int offset = signExtend(imm, 26)<<2;
        snprintf(buff, sizeof(buff), "B #+%di\t; %p", offset>>2, (void*)(addr + offset));
        return buff;
    }
    if(isMask(opcode, "f0110100iiiiiiiiiiiiiiiiiiittttt", &a)) {
        int offset = signExtend(imm, 19)<<2;
        snprintf(buff, sizeof(buff), "CBZ %s, #%+di\t; %p", sf?Xt[Rt]:Wt[Rt], offset>>2, (void*)(addr + offset));
        return buff;
    }
    if(isMask(opcode, "f0110101iiiiiiiiiiiiiiiiiiittttt", &a)) {
        int offset = signExtend(imm, 19)<<2;
        snprintf(buff, sizeof(buff), "CBNZ %s, #%+di\t; %p", sf?Xt[Rt]:Wt[Rt], offset>>2, (void*)(addr + offset));
        return buff;
    }
    if(isMask(opcode, "s0110110sssssiiiiiiiiiiiiiittttt", &a)) {
        int offset = signExtend(imm, 14)<<2;
        snprintf(buff, sizeof(buff), "TBZ %s, %d, #%+di\t; %p", (imms<31)?Xt[Rt]:Wt[Rt], imms, offset>>2, (void*)(addr + offset));
        return buff;
    }
    if(isMask(opcode, "s0110111sssssiiiiiiiiiiiiiittttt", &a)) {
        int offset = signExtend(imm, 14)<<2;
        snprintf(buff, sizeof(buff), "TBNZ %s, %d, #%+di\t; %p", (imms<31)?Xt[Rt]:Wt[Rt], imms, offset>>2, (void*)(addr + offset));
        return buff;
    }

    if(isMask(opcode, "f0011010100mmmmmcccc01nnnnnddddd", &a)) {
        if(Rm!=31 && (cond&0b1110)!=0b1110 && Rn!=31 && Rn==Rm)
            snprintf(buff, sizeof(buff), "CINC %s, %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], conds[cond^1]);    
        else if(Rm==31 && (cond&0b1110)!=0b1110 && Rn==31)
            snprintf(buff, sizeof(buff), "CSET %s,%s", sf?Xt[Rd]:Wt[Rd], conds[cond^1]);    
        else
            snprintf(buff, sizeof(buff), "CSINC %s, %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], conds[cond]);
        return buff;
    }

    if(isMask(opcode, "f1011010100mmmmmcccc00nnnnnddddd", &a)) {
        if(Rm!=31 && (cond&0b1110)!=0b1110 && Rn!=31 && Rn==Rm)
            snprintf(buff, sizeof(buff), "CINV %s, %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], conds[cond^1]);    
        else if(Rm==31 && (cond&0b1110)!=0b1110 && Rn==31)
            snprintf(buff, sizeof(buff), "CSETM %s,%s", sf?Xt[Rd]:Wt[Rd], conds[cond^1]);    
        else
            snprintf(buff, sizeof(buff), "CSINV %s, %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], conds[cond]);
        return buff;
    }

    if(isMask(opcode, "f1011010100mmmmmcccc01nnnnnddddd", &a)) {
        if((cond&0b1110)!=0b1110 && Rn==Rm)
            snprintf(buff, sizeof(buff), "CNEG %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], conds[cond^1]);    
        else
            snprintf(buff, sizeof(buff), "CSNEG %s, %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], conds[cond]);
        return buff;
    }
    if(isMask(opcode, "f0011010100mmmmmcccc00nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "CSEL %s, %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], conds[cond]);    
        return buff;
    }
    if(isMask(opcode, "f1111010010mmmmmcccc00nnnnn0iiii", &a)) {
        snprintf(buff, sizeof(buff), "CCMP %s, %s, %s 0x%x", sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], conds[cond], imm);
        return buff;
    }

    // MISC Bits
    if(isMask(opcode, "f10110101100000000010onnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "CL%c %s, %s", option?'S':'Z', sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn]);
        return buff;
    }
    if(isMask(opcode, "f101101011000000000000nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "RBIT %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn]);
        return buff;
    }
    if(isMask(opcode, "f1011010110000000000oonnnnnddddd", &a)) {
        if(!sf && option==2)
            snprintf(buff, sizeof(buff), "REV %s, %s", Wt[Rd], Wt[Rn]);
        else if (sf && option==3) 
            snprintf(buff, sizeof(buff), "REV %s, %s", Xt[Rd], Xt[Rn]);
        else
            snprintf(buff, sizeof(buff), "REV%d %s, %s", 8<<option, sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn]);
        return buff;
    }

    // MULL ADD
    if(isMask(opcode, "10011011U01mmmmm0aaaaannnnnddddd", &a)) {
        if(Ra==31)
            snprintf(buff, sizeof(buff), "%cMULL %s, %s, %s", a.U?'U':'S', Xt[Rd], Wt[Rn], Wt[Rm]);
        else
            snprintf(buff, sizeof(buff), "%cMADDL %s, %s, %s, %s", a.U?'U':'S', Xt[Rd], Wt[Rn], Wt[Rm], Xt[Ra]);
        return buff;
    }
    if(isMask(opcode, "10011011U10mmmmm011111nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "%cMULH %s, %s, %s", a.U?'U':'S', Xt[Rd], Xt[Rn], Xt[Rm]);
        return buff;
    }
    if(isMask(opcode, "f0011011000mmmmm0aaaaannnnnddddd", &a)) {
        if(Ra==31)
            snprintf(buff, sizeof(buff), "MUL %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else
            snprintf(buff, sizeof(buff), "MADD %s, %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], sf?Xt[Ra]:Wt[Ra]);
        return buff;
    }
    if(isMask(opcode, "f0011011000mmmmm1aaaaannnnnddddd", &a)) {
        if(Ra==31)
            snprintf(buff, sizeof(buff), "MNEG %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        else
            snprintf(buff, sizeof(buff), "MSUB %s, %s, %s, %s", sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm], sf?Xt[Ra]:Wt[Ra]);
        return buff;
    }

    // DIV
    if(isMask(opcode, "f0011010110mmmmm00001onnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "%cDIV %s, %s, %s", option?'S':'U', sf?Xt[Rd]:Wt[Rd], sf?Xt[Rn]:Wt[Rn], sf?Xt[Rm]:Wt[Rm]);
        return buff;
    }

    // MRS / MSR
    if(isMask(opcode, "110101010001opppnnnnmmmm222ttttt", &a)) {
        const char* reg=NULL;
        //o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0010(2) op2=0 => nzcv
        //o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0100(4) op2=2 => fpcr
        if(a.o==1 && a.p==3 && a.n==4 && a.m==2 && a.t2==0)
            reg="nzcv";
        else if(a.o==1 && a.p==3 && a.n==4 && a.m==4 && a.t2==2)
            reg="fpcr";

        if(!reg)
            snprintf(buff, sizeof(buff), "MSR S%d_%d_%d_%d_%d, %s", 2+a.o, a.p, a.n, a.m, a.t2, Xt[Rt]);
        else
            snprintf(buff, sizeof(buff), "MSR %s, %s", reg, Xt[Rt]);
        return buff;
    }
    if(isMask(opcode, "110101010011opppnnnnmmmm222ttttt", &a)) {
        const char* reg=NULL;
        //o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0010(2) op2=0 => nzcv
        //o0=1(op0=3), op1=0b011(3) CRn=0b0100(4) CRm=0b0100(4) op2=2 => fpcr
        if(a.o==1 && a.p==3 && a.n==4 && a.m==2 && a.t2==0)
            reg="nzcv";
        else if(a.o==1 && a.p==3 && a.n==4 && a.m==4 && a.t2==2)
            reg="fpcr";

        if(!reg)
            snprintf(buff, sizeof(buff), "MRS %s, S%d_%d_%d_%d_%d", Xt[Rt], 2+a.o, a.p, a.n, a.m, a.t2);
        else
            snprintf(buff, sizeof(buff), "MRS %s, %s", Xt[Rt], reg);
        return buff;
    }

    //  ----------- NEON / FPU

    // VORR/VAND/VBIC/VORN
    if(isMask(opcode, "0Q001110101mmmmm000111nnnnnddddd", &a)) {
        char q = a.Q?'Q':'D';
        if(Rn==Rm)
            snprintf(buff, sizeof(buff), "VMOV %c%d, %c%d", q, Rd, q, Rn);
        else
            snprintf(buff, sizeof(buff), "VORR %c%d, %c%d, %c%d", q, Rd, q, Rn, q, Rm);
        return buff;
    }
    if(isMask(opcode, "0Q001110111mmmmm000111nnnnnddddd", &a)) {
        char q = a.Q?'Q':'D';
        snprintf(buff, sizeof(buff), "VORN %c%d, %c%d, %c%d", q, Rd, q, Rn, q, Rm);
        return buff;
    }
    if(isMask(opcode, "0Q001110001mmmmm000111nnnnnddddd", &a)) {
        char q = a.Q?'Q':'D';
        snprintf(buff, sizeof(buff), "VAND %c%d, %c%d, %c%d", q, Rd, q, Rn, q, Rm);
        return buff;
    }
    if(isMask(opcode, "0Q001110011mmmmm000111nnnnnddddd", &a)) {
        char q = a.Q?'Q':'D';
        snprintf(buff, sizeof(buff), "VBIC %c%d, %c%d, %c%d", q, Rd, q, Rn, q, Rm);
        return buff;
    }
    // UMOV
    if(isMask(opcode, "0Q001110000rrrrr001111nnnnnddddd", &a)) {
        char s = '?';
        int sz=0;
        if(a.Q==0 && immr&1) {s='B'; sz=0; }
        else if(a.Q==0 && (immr&3)==2) {s='H'; sz=1; }
        else if(a.Q==0 && (immr&7)==4) {s='S'; sz=2; }
        else if(a.Q==1 && (immr&15)==8) {s='D'; sz=3; }
        int index = (immr)>>(sz+1);
        if(sz>2)
            snprintf(buff, sizeof(buff), "MOV %s, V%d.%c[%d]", a.Q?Xt[Rd]:Wt[Rd], Rn, s, index);
        else
            snprintf(buff, sizeof(buff), "UMOV %s, V%d.%c[%d]", a.Q?Xt[Rd]:Wt[Rd], Rn, s, index);
        return buff;
    }
    // SMOV
    if(isMask(opcode, "0Q001110000rrrrr001011nnnnnddddd", &a)) {
        char s = '?';
        int sz=0;
        if(a.Q==0 && immr&1) {s='B'; sz=0; }
        else if(/*a.Q==0 &&*/ (immr&3)==2) {s='H'; sz=1; }
        else if(/*a.Q==0 &&*/ (immr&7)==4) {s='S'; sz=2; }
        else if(a.Q==1 && (immr&15)==8) {s='D'; sz=3; }
        int index = (immr)>>(sz+1);
        if(sz>2)
            snprintf(buff, sizeof(buff), "MOV %s, V%d.%c[%d]", a.Q?Xt[Rd]:Wt[Rd], Rn, s, index);
        else
            snprintf(buff, sizeof(buff), "SMOV %s, V%d.%c[%d]", a.Q?Xt[Rd]:Wt[Rd], Rn, s, index);
        return buff;
    }
    // VEOR
    if(isMask(opcode, "0Q101110001mmmmm000111nnnnnddddd", &a)) {
        char q = a.Q?'Q':'D';
        snprintf(buff, sizeof(buff), "VEOR %c%d, %c%d, %c%d", q, Rd, q, Rn, q, Rm);
        return buff;
    }

    // VADD / VSUB
    if(isMask(opcode, "0QU01110ff1mmmmm100001nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        const char* Vd = Y[((sf)<<1) | a.Q];
        snprintf(buff, sizeof(buff), "V%s V%d.%s, V%d.%s, V%d.%s", a.U?"SUB":"ADD", Rd, Vd, Rn, Vd, Rm, Vd);
        return buff;
    }

    // VMUL
    if(isMask(opcode, "0Q001110ff1mmmmm100111nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        const char* Vd = Y[((sf)<<1) | a.Q];
        snprintf(buff, sizeof(buff), "VMUL V%d.%s, V%d.%s, V%d.%s", Rd, Vd, Rn, Vd, Rm, Vd);
        return buff;
    }
    // VBIT / VBIF
    if(isMask(opcode, "0Q1011101o1mmmmm000111nnnnnddddd", &a)) {
        char q = a.Q?'Q':'D';
        snprintf(buff, sizeof(buff), "VBI%c %c%d, %c%d, %c%d", a.o?'F':'T', q, Rd, q, Rn, q, Rm);
        return buff;
    }
    // CMP
    if(isMask(opcode, "0Q101110ff1mmmmm100011nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        const char* Vd = Y[((sf)<<1) | a.Q];
        snprintf(buff, sizeof(buff), "VCMEQ V%d.%s, V%d.%s, V%d.%s", Rd, Vd, Rn, Vd, Rm, Vd);
        return buff;
    }
    // CMP zero
    if(isMask(opcode, "0QU01110ff100000100o10nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        const char* Z[] = {"GT", "GE", "EQ", "LE"};
        const char* Vd = Y[((sf)<<1) | a.Q];
        const char* Cond = Z[(a.o << 1 | a.U)];
        snprintf(buff, sizeof(buff), "VCM%s V%d.%s, V%d.%s, #0", Cond, Rd, Vd, Rn, Vd);
        return buff;
    }
    // CMPLT zero
    if(isMask(opcode, "0Q001110ff100000101010nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        const char* Vd = Y[((sf)<<1) | a.Q];
        snprintf(buff, sizeof(buff), "VCMLT V%d.%s, V%d.%s, #0", Rd, Vd, Rn, Vd);
        return buff;
    }
    // MIN/MAX
    if(isMask(opcode, "0QU01110ff1mmmmm0110o1nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        const char* Vd = Y[((sf)<<1) | a.Q];
        snprintf(buff, sizeof(buff), "%c%s V%d.%s, V%d.%s, V%d.%s", a.U?'U':'S', a.o?"MIN":"MAX", Rd, Vd, Rn, Vd, Rm, Vd);
        return buff;
    }

    // MOV immediate (not)shifted 8bits
    if(isMask(opcode, "0Q00111100000iii111001iiiiiddddd", &a)) {
        const char* Y[] = {"8B", "16B"};
        const char* Vd = Y[a.Q];
        snprintf(buff, sizeof(buff), "MOVI V%d.%s, #0x%x", Rd, Vd, imm);
        return buff;
    }
    // MOV immediate notshifted 16bits & 32bits
    if(isMask(opcode, "0Q00111100000iiif00001iiiiiddddd", &a)) {
        const char* Y[] = {"2S", "4S", "4H", "8H"};
        const char* Vd = Y[(sf<<1)| a.Q];
        int sh = 0;

        snprintf(buff, sizeof(buff), "MOVI V%d.%s, #0x%x", Rd, Vd, imm<<sh);
        return buff;
    }
    // MOV immediate shifted 16bits
    if(isMask(opcode, "0Q00111100000iii101001iiiiiddddd", &a)) {
        const char* Y[] = {"4H", "8H"};
        const char* Vd = Y[a.Q];

        snprintf(buff, sizeof(buff), "MOVI V%d.%s, #0x%x", Rd, Vd, imm<<8);
        return buff;
    }
    // MOV immediate 64bits
    if(isMask(opcode, "0Q10111100000iii111001iiiiiddddd", &a)) {
        uint64_t tmp64u = 0;
        for(int i=0; i<8; ++i)
            if((imm)&(1<<i))
                tmp64u |= 0xffLL<<(i*8);

        if(a.Q)
            snprintf(buff, sizeof(buff), "MOVI V%d.2D, #0x%016"PRIx64, Rd, tmp64u);
        else
            snprintf(buff, sizeof(buff), "MOVI D%d, #0x%016"PRIx64, Rd, tmp64u);
        return buff;
    }

    // Shift
    if(isMask(opcode, "0QU011110hhhhrrr000001nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        const char* Vd ="??";
        int s = 0;
        if(shift==0b0001) {Vd = Y[a.Q]; s=16-((shift)<<3 | immr);}
        else if((shift&0b1110)==0b0010) {Vd = Y[2+a.Q]; s=32-((shift)<<3 | immr);}
        else if((shift&0b1100)==0b0100) {Vd = Y[4+a.Q]; s=64-((shift)<<3 | immr);}
        else if((shift&0b1000)==0b1000) {Vd = Y[6+a.Q]; s=128-((shift)<<3 | immr);}
        snprintf(buff, sizeof(buff), "%cSHR V%d.%s, V%d.%s, #%d", a.U?'U':'S', Rd, Vd, Rn, Vd, s);
        return buff;
    }

    // INS
    if(isMask(opcode, "01101110000rrrrr0ssss1nnnnnddddd", &a)) {
        char s = '?';
        int idx1=0, idx2=0;
        if(immr&1) {s='B'; idx1=(immr)>>1; idx2 = imms; }
        else if((immr&3)==2) {s='H'; idx1=(immr)>>2; idx2=(imms)>>1;}
        else if((immr&7)==4) {s='S'; idx1=(immr)>>3; idx2=(imms)>>2;}
        else if((immr&15)==8) {s='D'; idx1=(immr)>>4; idx2=(imms)>>3;}
        snprintf(buff, sizeof(buff), "INS V%d.%c[%d], V%d.%c[%d]", Rd, s, idx1, Rn, s, idx2);
        return buff;
    }
    if(isMask(opcode, "01001110000rrrrr000111nnnnnddddd", &a)) {
        char s = '?', R = 0;
        int idx1=0;
        if(immr&1) {s='B'; idx1=(immr)>>1; }
        else if((immr&3)==2) {s='H'; idx1=(immr)>>2;}
        else if((immr&7)==4) {s='S'; idx1=(immr)>>3;}
        else if((immr&15)==8) {s='D'; idx1=(immr)>>4; R=1;}
        snprintf(buff, sizeof(buff), "INS V%d.%c[%d], %s", Rd, s, idx1, R?Xt[Rn]:Wt[Rn]);
        return buff;
    }

    // ADR
    if(isMask(opcode, "0ss10000iiiiiiiiiiiiiiiiiiiddddd", &a)) {
      snprintf(buff, sizeof(buff), "ADR, %s, %"PRId64, Xt[Rd], signExtend((imm)<<2|(imms), 20));
      return buff;
    }

    // LDR / STR
    if(isMask(opcode, "ss111101cciiiiiiiiiiiinnnnnttttt", &a)) {
        char s = '?';
        int size=imms;
        int op=0;
        if(size==0 && opc==1) {s='B';}
        else if(size==1 && opc==1) {s='H';}
        else if(size==2 && opc==1) {s='S';}
        else if(size==3 && opc==1) {s='D';}
        else if(size==0 && opc==3) {s='Q'; size = 4;}
        else if(size==0 && opc==0) {s='B'; op=1;}
        else if(size==1 && opc==0) {s='H'; op=1;}
        else if(size==2 && opc==0) {s='S'; op=1;}
        else if(size==3 && opc==0) {s='D'; op=1;}
        else if(size==0 && opc==2) {s='Q'; op=1; size = 4;}

        int offset = imm<<size;
        if(!offset)
            snprintf(buff, sizeof(buff), "%s %c%d, [%s]", op?"STR":"LDR", s, Rt, XtSp[Rn]);
        else
            snprintf(buff, sizeof(buff), "%s %c%d, [%s, 0x%x]", op?"STR":"LDR", s, Rt, XtSp[Rn], offset);
        return buff;
    }
    if(isMask(opcode, "ss111100cc1mmmmmoooS10nnnnnttttt", &a)) {
        char s = '?';
        int size=imms;
        int op=0;
        if(size==0 && opc==1) {s='B';}
        else if(size==1 && opc==1) {s='H';}
        else if(size==2 && opc==1) {s='S';}
        else if(size==3 && opc==1) {s='D';}
        else if(size==0 && opc==3) {s='Q'; size = 4;}
        else if(size==0 && opc==0) {s='B'; op=1;}
        else if(size==1 && opc==0) {s='H'; op=1;}
        else if(size==2 && opc==0) {s='S'; op=1;}
        else if(size==3 && opc==0) {s='D'; op=1;}
        else if(size==0 && opc==2) {s='Q'; op=1; size = 4;}

        const char* extend[] = {"?0", "?1", "UXTW", "LSL", "?4", "?5", "SXTW", "SXTX"};
        int amount = size*a.S;
        if(option==3 && !amount)
            snprintf(buff, sizeof(buff), "%s %c%d, [%s, %s]", op?"STR":"LDR", s, Rt, XtSp[Rn], ((option&1)==0)?Wt[Rm]:Xt[Rm]);
        else
            snprintf(buff, sizeof(buff), "%s %c%d, [%s, %s, %s %d]", op?"STR":"LDR", s, Rt, XtSp[Rn], ((option&1)==0)?Wt[Rm]:Xt[Rm], extend[option], amount);
        return buff;
    }

    // FADD
    if(isMask(opcode, "0Q0011100f1mmmmm110101nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VFADD %c%d.%d%c, %c%d.%d%c, %c%d.%d%c", s, Rd, n, d, s, Rn, n, d, s, Rm, n, d);
        return buff;
    }
    if(isMask(opcode, "00011110ff1mmmmm001010nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FADD %c%d, %c%d, %c%d", s, Rd, s, Rn, s, Rm);
        return buff;
    }

    // FSUB
    if(isMask(opcode, "0Q0011101f1mmmmm110101nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VFSUB %c%d.%d%c, %c%d.%d%c, %c%d.%d%c", s, Rd, n, d, s, Rn, n, d, s, Rm, n, d);
        return buff;
    }
    if(isMask(opcode, "00011110ff1mmmmm001110nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FSUB %c%d, %c%d, %c%d", s, Rd, s, Rn, s, Rm);
        return buff;
    }

    // FMUL
    if(isMask(opcode, "0Q1011100f1mmmmm110111nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VFMUL %c%d.%d%c, %c%d.%d%c, %c%d.%d%c", s, Rd, n, d, s, Rn, n, d, s, Rm, n, d);
        return buff;
    }
    if(isMask(opcode, "00011110ff1mmmmm000010nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FMUL %c%d, %c%d, %c%d", s, Rd, s, Rn, s, Rm);
        return buff;
    }

    // FDIV
    if(isMask(opcode, "0Q1011100f1mmmmm111111nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VFDIV %c%d.%d%c, %c%d.%d%c, %c%d.%d%c", s, Rd, n, d, s, Rn, n, d, s, Rm, n, d);
        return buff;
    }
    if(isMask(opcode, "00011110ff1mmmmm000110nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FDIV %c%d, %c%d, %c%d", s, Rd, s, Rn, s, Rm);
        return buff;
    }

    // FADDP
    if(isMask(opcode, "0Q1011100f1mmmmm110101nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VFADDP %c%d.%d%c, %c%d.%d%c, %c%d.%d%c", s, Rd, n, d, s, Rn, n, d, s, Rm, n, d);
        return buff;
    }
    if(isMask(opcode, "011111100f110000110110nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FADDP %c%d, %c%d, %c%d", s, Rd, s, Rn, s, Rm);
        return buff;
    }

    // SQRT
    if(isMask(opcode, "0Q1011101f100001111110nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VSQRT %c%d.%d%c, %c%d.%d%c", s, Rd, n, d, s, Rn, n, d);
        return buff;
    }
    if(isMask(opcode, "00011110ff100001110000nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FSQRT %c%d, %c%d", s, Rd, s, Rn);
        return buff;
    }
    // FPRINTX
    if(isMask(opcode, "0Q1011100f100001100110nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VFRINTX %c%d.%d%c, %c%d.%d%c", s, Rd, n, d, s, Rn, n, d);
        return buff;
    }
    if (isMask(opcode, "00011110ff100111010000nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FRINTX %c%d, %c%d", s, Rd, s, Rn);
        return buff;
    }
    // FRINTI
    if(isMask(opcode, "0Q1011101f100001100110nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VFRINTI %c%d.%d%c, %c%d.%d%c", s, Rd, n, d, s, Rn, n, d);
        return buff;
    }
    if(isMask(opcode, "00011110ff100111110000nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FRINTI %c%d, %c%d", s, Rd, s, Rn);
        return buff;
    }
    // FRINTN
    if(isMask(opcode, "0Q0011100f100001100010nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VFRINTN %c%d.%d%c, %c%d.%d%c", s, Rd, n, d, s, Rn, n, d);
        return buff;
    }
    if(isMask(opcode, "000111100f100100010000nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FRINTN %c%d, %c%d", s, Rd, s, Rn);
        return buff;
    }
    // FRINTM
    if(isMask(opcode, "0Q0011100f100001100110nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VFRINTM %c%d.%d%c, %c%d.%d%c", s, Rd, n, d, s, Rn, n, d);
        return buff;
    }
    if(isMask(opcode, "000111100f100101010000nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FRINTM %c%d, %c%d", s, Rd, s, Rn);
        return buff;
    }
    // FRINTP
    if(isMask(opcode, "0Q0011101f100001100010nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VFRINTP %c%d.%d%c, %c%d.%d%c", s, Rd, n, d, s, Rn, n, d);
        return buff;
    }
    if(isMask(opcode, "000111100f100100110000nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FRINTP %c%d, %c%d", s, Rd, s, Rn);
        return buff;
    }
    // FRINTZ
    if(isMask(opcode, "0Q0011101f100001100110nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VFRINTZ %c%d.%d%c, %c%d.%d%c", s, Rd, n, d, s, Rn, n, d);
        return buff;
    }
    if(isMask(opcode, "000111100f100101110000nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FRINTZ %c%d, %c%d", s, Rd, s, Rn);
        return buff;
    }

    //CMP
    if(isMask(opcode, "00011110ff1mmmmm001000nnnnn0c000", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        if(opc==1)
            snprintf(buff, sizeof(buff), "FCMP %c%d, #0.0", s, Rn);
        else
            snprintf(buff, sizeof(buff), "FCMP %c%d, %c%d", s, Rn, s, Rm);
        return buff;
    }
    //FCMEQ
    if(isMask(opcode, "010111100f1mmmmm111001nnnnnddddd", &a)) {
        char s = (sf==0)?'S':'D';
        snprintf(buff, sizeof(buff), "FCMEQ %c%d, %c%d, %c%d", s, Rd, s, Rn, s, Rm);
        return buff;
    }
    //FCMP vector
    if(isMask(opcode, "0QU01110cf1mmmmm111001nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        int n = (sf==0)?2:1;
        n *= a.Q?2:1;
        int op = (a.c<<1) | (a.U);
        const char* OP[] = {"EQ", "GE", "??", "GT"};
        snprintf(buff, sizeof(buff), "FCMP%s%s V%d.%d%c, V%d.%d%c, V%d.%d%c", OP[op], a.Q?"Q":"", Rd, n, s, Rn, n, s, Rm, n, s);
        return buff;
    }
    //FMIN/FMAX
    if(isMask(opcode, "00011110ff1mmmmm01oo10nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "F%s %c%d, %c%d, %c%d", (option==3)?"MINNM":((option==2)?"MAXNM":((!option)?"MAX":"MIN")), s, Rd, s, Rn, s, Rm);
        return buff;
    }
    if(isMask(opcode, "0QU01110of1mmmmm110001nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        int n = (sf==0)?2:1;
        n *= a.Q?2:1;
        snprintf(buff, sizeof(buff), "F%sNM%s%s V%d.%d%c, V%d.%d%c, V%d.%d%c", option?"MIN":"MAX", a.U?"P":"", a.Q?"Q":"", Rd, n, s, Rn, n, s, Rm, n, s);
        return buff;
    }
    if(isMask(opcode, "0QU01110of1mmmmm111101nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        int n = (sf==0)?2:1;
        n *= a.Q?2:1;
        snprintf(buff, sizeof(buff), "F%s%s%s V%d.%d%c, V%d.%d%c, V%d.%d%c", option?"MIN":"MAX", a.U?"P":"", a.Q?"Q":"", Rd, n, s, Rn, n, s, Rm, n, s);
        return buff;
    }
    // FMADD
    if(isMask(opcode, "00011111tt0mmmmmoaaaaannnnnddddd", &a)) {
        char s = (a.t==0b00)?'S':((a.t==0b01)?'D':'?');
        int n = (a.t==0)?1:2;
        snprintf(buff, sizeof(buff), "FM%s V%d.%d%c, V%d.%d%c, V%d.%d%c, V%d.%d%c", option?"SUB":"ADD", Rd, n, s, Ra, n, s, Rn, n, s, Rm, n, s);
        return buff;
    }
    // FNMADD
    if(isMask(opcode, "00011111tt1mmmmmoaaaaannnnnddddd", &a)) {
        char s = (a.t==0b00)?'S':((a.t==0b01)?'D':'?');
        int n = (a.t==0)?1:2;
        snprintf(buff, sizeof(buff), "FNM%s V%d.%d%c, V%d.%d%c, V%d.%d%c, V%d.%d%c", option?"SUB":"ADD", Rd, n, s, Ra, n, s, Rn, n, s, Rm, n, s);
        return buff;
    }
    // FMLA
    if(isMask(opcode, "0Q001110of1mmmmm110011nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        int n = (sf==0)?2:1;
        n *= a.Q?2:1;
        snprintf(buff, sizeof(buff), "FML%s%s V%d.%d%c, V%d.%d%c, V%d.%d%c", option?"S":"A", a.Q?"Q":"", Rd, n, s, Rn, n, s, Rm, n, s);
        return buff;
    }
    // NEG
    if(isMask(opcode, "0Q101110ff100000101110nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        const char* Vd = Y[(sf<<1) | a.Q];
        snprintf(buff, sizeof(buff), "NEG%s V%d.%s, V%d.%s", a.Q?"Q":"", Rd, Vd, Rn, Vd);
        return buff;
    }
    // SSHL vector
    if(isMask(opcode, "0QU01110ff1mmmmm010rS1nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        const char* Vd = Y[(sf<<1) | a.Q];
        snprintf(buff, sizeof(buff), "%c%s%sSHL%s V%d.%s, V%d.%s, V%d.%s", a.U?'U':'S', a.r?"R":"", a.S?"Q":"", a.Q?"Q":"", Rd, Vd, Rn, Vd, Rm, Vd);
        return buff;
    }

    // FCVT
    if(isMask(opcode, "f0011110pp10010U000000nnnnnddddd", &a)) {
        int type = a.p;
        char s = (type==0)?'S':((type==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FCVTA%c %s, %c%d", a.U?'U':'S', sf?Xt[Rd]:Wt[Rd], s, Rn);
        return buff;
    }
    if(isMask(opcode, "01U111100f100001110010nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FCVTA%c %c%d, %c%d", a.U?'U':'S', s, Rd, s, Rn);
        return buff;
    }
    if(isMask(opcode, "0QU011100f100001110010nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "VFCVTA%c %c%d.%d%c, %c%d.%d%c", a.U?'U':'S', s, Rd, n, d, s, Rn, n, d);
        return buff;
    }

    if(isMask(opcode, "f0011110pp100010000000nnnnnddddd", &a)) {
        int type = a.p;
        char s = (type==0)?'S':((type==1)?'D':'?');
        snprintf(buff, sizeof(buff), "SCVTF %c%d, %s", s, Rd, sf?Xt[Rn]:Wt[Rn]);
        return buff;
    }
    if(isMask(opcode, "0QU011100f100001110110nnnnnddddd", &a)) {
        char s = a.Q?'V':'D';
        char d = sf?'D':'S';
        int n = (a.Q && !sf)?4:2;
        snprintf(buff, sizeof(buff), "%cCVTF %c%d.%d%c, %c%d.%d%c", a.U?'U':'S', s, Rd, n, d, s, Rn, n, d);
        return buff;
    }

    if(isMask(opcode, "0001111000100010110000nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "FCVT D%d, S%d", Rd, Rn);
        return buff;
    }
    if(isMask(opcode, "0001111001100010010000nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "FCVT S%d, D%d", Rd, Rn);
        return buff;
    }
    if(isMask(opcode, "0Q00111001100001011110nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "FCVTL%s V%d.2D, V%d.%dS", a.Q?"2":"", Rd, Rn, a.Q?4:2);
        return buff;
    }
    if(isMask(opcode, "0Q10111001100001011010nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "FCVTXN%s V%d.%dS, V%d.2D", a.Q?"2":"", Rd, a.Q?4:2, Rn);
        return buff;
    }
    if(isMask(opcode, "01U11110of100001101o10nnnnnddddd", &a)) {
        char s = (sf==0)?'S':((sf==1)?'D':'?');
        const char* roundings[] = {"N", "M", "P", "Z"};
        snprintf(buff, sizeof(buff), "FCVT%s%s %c%d, %c%d", roundings[option], a.U?"U":"", s, Rd, s, Rn);
        return buff;
    }
    if(isMask(opcode, "f0011110oo1cc000000000nnnnnddddd", &a)) {
        char s = (option==0)?'S':((option==1)?'D':'?');
        const char* roundings[] = {"N", "P", "M", "Z"};
        snprintf(buff, sizeof(buff), "FCVT%sS %s, %c%d", roundings[a.c], sf?Xt[Rd]:Wt[Rd], s, Rn);
        return buff;
    }
    if(isMask(opcode, "0QU01110of100001101o10nnnnnddddd", &a)) {
        const char* Y[] = {"2S", "4S", "??", "2D"};
        const char* Z[] = {"S", "S", "??", "D"};
        const char* Vd = Y[(sf<<1) | a.Q];
        const char* roundings[] = {"N", "M", "P", "Z"};
        snprintf(buff, sizeof(buff), "VFCVT%s%s%s%s V%d.%s, V%d.%s", roundings[option], a.U?"U":"S", a.Q?"Q":"", Z[(sf<<1)|a.Q], Rd, Vd, Rn, Vd);
        return buff;
    }

    // FMOV
    if(isMask(opcode, "00011110pp100000010000nnnnnddddd", &a)) {
        int type = a.p;
        char s = (type==0)?'S':((type==1)?'D':'?');
        snprintf(buff, sizeof(buff), "FMOV %c%d, %c%d", s, Rd, s, Rn);
        return buff;
    }
    if(isMask(opcode, "f0011110pp10x11c000000nnnnnddddd", &a)) {
        int type = a.p;
        int rmode = a.x;
        int opcd = 6+a.c;
             if(sf==0 && type==0 && rmode==0 && opcd==7) {snprintf(buff, sizeof(buff), "FMOV S%d, %s", Rd, Wt[Rn]);}
        else if(sf==0 && type==0 && rmode==0 && opcd==6) {snprintf(buff, sizeof(buff), "FMOV %s, S%d", Wt[Rn], Rd);}
        else if(sf==1 && type==1 && rmode==0 && opcd==7) {snprintf(buff, sizeof(buff), "FMOV D%d, %s", Rd, Xt[Rn]);}
        else if(sf==1 && type==2 && rmode==1 && opcd==7) {snprintf(buff, sizeof(buff), "FMOV V%d.D[1], %s", Rd, Xt[Rn]);}
        else if(sf==1 && type==1 && rmode==0 && opcd==6) {snprintf(buff, sizeof(buff), "FMOV %s, D%d", Xt[Rd], Rn);}
        else if(sf==1 && type==2 && rmode==1 && opcd==6) {snprintf(buff, sizeof(buff), "FMOV %s, V%d.D[1]", Xt[Rd], Rn);}
        else snprintf(buff, sizeof(buff), "FMOV ????");
        return buff;
    }

    // TBL / TBX
    if(isMask(opcode, "0Q001110000mmmmm0ffo00nnnnnddddd", &a)) {
        char X = (option)?'X':'L';
        const char* Y = (a.Q)?"16B":"8B";
        if(sf==0)
            snprintf(buff, sizeof(buff), "TB%c V%d.%s, {V%d.16B}, V%d.%s", X, Rd, Y, Rn, Rm, Y);
        else if(sf==1)
            snprintf(buff, sizeof(buff), "TB%c V%d.%s, {V%d.16B, V%d.16B}, V%d.%s", X, Rd, Y, Rn, Rn+1, Rm, Y);
        else if(sf==2)
            snprintf(buff, sizeof(buff), "TB%c V%d.%s, {V%d.16B, V%d.16B, V%d.16B}, V%d.%s", X, Rd, Y, Rn, Rn+1, Rn+2, Rm, Y);
        else
            snprintf(buff, sizeof(buff), "TB%c V%d.%s, {V%d.16B, V%d.16B, V%d.16B, V%d.16B}, V%d.%s", X, Rd, Y, Rn, Rn+1, Rn+2, Rn+3, Rm, Y);
        return buff;
    }

    // ZIP / UZP
    if(isMask(opcode, "0Q001110ff0mmmmm0o1110nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        const char* Vd = Y[(sf<<1) | a.Q];
        snprintf(buff, sizeof(buff), "ZIP%d V%d.%s, V%d.%s, V%d.%s", a.o+1, Rd, Vd, Rn, Vd, Rm, Vd);
        return buff;
    }
    if(isMask(opcode, "0Q001110ff0mmmmm0o0110nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        const char* Vd = Y[(sf<<1) | a.Q];
        snprintf(buff, sizeof(buff), "UZP%d V%d.%s, V%d.%s, V%d.%s", a.o+1, Rd, Vd, Rn, Vd, Rm, Vd);
        return buff;
    }

    // EXT
    if(isMask(opcode, "0Q101110000mmmmm0iiii0nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B"};
        const char* Vd = Y[a.Q];
        snprintf(buff, sizeof(buff), "EXT V%d.%s, V%d.%s, V%d.%s, #%d", Rd, Vd, Rn, Vd, Rm, Vd, imm);
        return buff;
    }

    // Absolute Difference
    // SABA / SABD / UABA / UABD
    if(isMask(opcode, "0QU01110ff1mmmmm0111c1nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "???"};
        const char* Vd = Y[(sf<<1) | a.Q];
        snprintf(buff, sizeof(buff), "%cAB%c V%d.%s, V%d.%s, V%d.%s", a.U?'U':'S', a.c?'A':'D', Rd, Vd, Rn, Vd, Rm, Vd);
        return buff;
    }
    if(isMask(opcode, "0QU01110ff1mmmmm01c100nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "???"};
        const char* Vd = Y[(sf<<1) | a.Q];
        const char* Z[] = {"8H", "4S", "2D", "?"};
        const char* Va = Z[(sf<<1)];
        snprintf(buff, sizeof(buff), "%cAB%cL%s V%d.%s, V%d.%s, V%d.%s", a.U?'U':'S', a.c?'A':'D', a.Q?"2":"", Rd, Va, Rn, Vd, Rm, Vd);
        return buff;
    }
    // Add pair / accros vector
    if(isMask(opcode, "0QU01110ff1000000c1010nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "???"};
        const char* Vd = Y[(sf<<1) | a.Q];
        const char* Z[] = {"4H", "8H", "2S", "4S", "1D", "2D", "??", "???"};
        const char* Va = Z[(sf<<1) | a.Q];
        snprintf(buff, sizeof(buff), "%cAD%cLP V%d.%s, V%d.%s", a.U?'U':'S', a.c?'A':'D', Rd, Va, Rn, Vd);
        return buff;
    }
    if(isMask(opcode, "0QU01110ff110000001110nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "???"};
        const char* Vd = Y[(sf<<1) | a.Q];
        const char* Z[] = {"H", "S", "D", "?"};
        snprintf(buff, sizeof(buff), "%cADDLV V%d.%s, V%d.%s", a.U?'U':'S', Rd, Z[sf], Rn, Vd);
        return buff;
    }
    // HADD
    if(isMask(opcode, "0QU01110ff1mmmmm000001nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "???"};
        const char* Vd = Y[(sf<<1) | a.Q];
        snprintf(buff, sizeof(buff), "%cHADD V%d.%s, V%d.%s, V%d.%s", a.U?'U':'S', Rd, Vd, Rn, Vd, Rm, Vd);
        return buff;
    }
    if(isMask(opcode, "0QU01110ff1mmmmm000101nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "???"};
        const char* Vd = Y[(sf<<1) | a.Q];
        snprintf(buff, sizeof(buff), "%cRHADD V%d.%s, V%d.%s, V%d.%s", a.U?'U':'S', Rd, Vd, Rn, Vd, Rm, Vd);
        return buff;
    }
    //S/URSHR
    if(isMask(opcode, "0QU011110iiiiiii001001nnnnnddddd", &a)) {
        int shft = 0;
        int sz = 0;
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        if(imm&0b1000000) {
            sz = 3;
            shft = imm&0b111111;
        } else if(imm&0b100000) {
            sz = 2;
            shft = imm&0b1111;
        } else if(imm&0b10000) {
            sz = 1;
            shft = imm&0b111;
        } else if(imm&0b1000) {
            sz = 0;
            shft = imm&0b111;
        }
        const char* Vd = Y[(sz<<1) | a.Q];
        snprintf(buff, sizeof(buff), "%cRSHR V%d.%s, V%d.%s, #%d", a.U?'U':'S', Rd, Vd, Rn, Vd, shft);
        return buff;
    }
    //SQ(R)DMULH
    if(isMask(opcode, "0QU01110ff1mmmmm101101nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "???"};
        const char* Vd = Y[(sf<<1) | a.Q];
        snprintf(buff, sizeof(buff), "SQ%sDMULH V%d.%s, V%d.%s, V%d.%s", a.U?"R":"", Rd, Vd, Rn, Vd, Rm, Vd);
        return buff;
    }
    
    // LD1/ST1 single structure
    if(isMask(opcode, "0Q0011010Lo00000cc0Sffnnnnnttttt", &a)) {
        int scale = a.c;
        int idx = 0;
        const char* Y[] = {"B", "H", "S", "D"};
        switch(scale) {
            case 3: scale = sf; /* rep = 1; */ idx=(8*(a.Q+1))>>scale; break;
            case 0: idx = (a.Q<<3) | (a.S<<2) | sf; break;
            case 1: idx = (a.Q<<2) | (a.S<<1) | (sf>>1); break;
            case 2: if(!(sf&1))
                        idx = (a.Q<<1) | a.S;
                    else {
                        scale = 3;
                        idx = a.Q;
                    }
                    break;
        }
        if(!option && a.L && scale==3)
            snprintf(buff, sizeof(buff), "LD1R {V%d.%d%s}, [%s]", Rt, idx, Y[scale], XtSp[Rn]);
        else
            snprintf(buff, sizeof(buff), "%s1 {V%d.%s}[%d], [%s]", a.L?"LD":"ST", Rt, Y[scale], idx, XtSp[Rn]);
        return buff;
    }
    // LDUR/STUR
    if(isMask(opcode, "ff111100cL0iiiiiiiii00nnnnnttttt", &a)) {
        const char* Y[] = {"B", "H", "S", "D", "Q"};
        int sz = sf;
        if(sz==0 && a.c)
            sz = 4;
        int offset = signExtend(imm, 9);
        if(!offset)
            snprintf(buff, sizeof(buff), "%sUR %s%d, [%s]", a.L?"LD":"ST", Y[sz], Rt, XtSp[Rn]);
        else
            snprintf(buff, sizeof(buff), "%sUR %s%d, [%s, %s0x%x]", a.L?"LD":"ST", Y[sz], Rt, XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }
    // LDR/STR vector immediate
    if(isMask(opcode, "ff111101cLiiiiiiiiiiiinnnnnttttt", &a)) {
        const char* Y[] = {"B", "H", "S", "D", "Q"};
        int sz = sf;
        if(sz==0 && a.c)
            sz = 4;
        int offset = imm<<sz;
        if(!offset)
            snprintf(buff, sizeof(buff), "%sR %s%d, [%s]", a.L?"LD":"ST", Y[sz], Rt, XtSp[Rn]);
        else
            snprintf(buff, sizeof(buff), "%sR %s%d, [%s, %+d]", a.L?"LD":"ST", Y[sz], Rt, XtSp[Rn], imm);
        return buff;
    }
    // LDP/STP vector
    if(isMask(opcode, "ff1011010Liiiiiii22222nnnnnttttt", &a)) {
        const char* Y[] = {"S", "D", "Q", "?"};
        int sz = sf;
        int offset = signExtend(imm, 7)<<(2+sz);
        if(!offset)
            snprintf(buff, sizeof(buff), "%sP %s%d, %s%d, [%s]", a.L?"LD":"ST", Y[sz], Rt, Y[sz], Rt2, XtSp[Rn]);
        else
            snprintf(buff, sizeof(buff), "%sP %s%d, %s%d, [%s, %s0x%x]", a.L?"LD":"ST", Y[sz], Rt, Y[sz], Rt2, XtSp[Rn], (offset<0)?"-":"", abs(offset));
        return buff;
    }

    // (S/U)QXT(U)N
    if(isMask(opcode, "0Q101110ff100001001010nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "?", "??"};
        const char* Vd = Y[(sf<<1) | a.Q];
        const char* Z[] = {"8H", "4S", "2D", "?"};
        const char* Va = Z[sf];
        snprintf(buff, sizeof(buff), "SQXTUN%s V%d.%s, V%d.%s", a.Q?"2":"", Rd, Vd, Rn, Va);
        return buff;
    }
    if(isMask(opcode, "0QU01110ff100001010010nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "?", "??"};
        const char* Vd = Y[(sf<<1) | a.Q];
        const char* Z[] = {"8H", "4S", "2D", "?"};
        const char* Va = Z[sf];
        snprintf(buff, sizeof(buff), "%cQXTN%s V%d.%s, V%d.%s", a.U?'U':'S', a.Q?"2":"", Rd, Vd, Rn, Va);
        return buff;
    }
    if(isMask(opcode, "01U11110ff100001010010nnnnnddddd", &a)) {
        const char Z[] = {'B', 'H', 'S', 'D', '?'};
        snprintf(buff, sizeof(buff), "SQXT%sN %c%d, %c%d", a.U?"U":"", Z[sf], Rd, Z[sf+1], Rn);
        return buff;
    }

    // (S/U)SSHL(2) / (U/S)XTL(2)
    if(isMask(opcode, "0QU011110hhhhiii101001nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "?", "??"};
        const char* Z[] = {"8H", "4S", "2D", "?"};
        int sz = 3;
        if((a.h&0b1111)==0b0001) sz=0;
        else if((a.h&0b1110)==0b0010) sz=1;
        else if((a.h&0b1100)==0b0100) sz=2;
        int sh=(((a.h)<<3)|(imm)) - (8<<sz);
        const char* Vd = Y[(sz<<1)|a.Q];
        const char* Va = Z[sz];
        if(!sh)
            snprintf(buff, sizeof(buff), "%cXTL%s V%d.%s, V%d.%s", a.U?'U':'S', a.Q?"2":"", Rd, Va, Rn, Vd);
        else
            snprintf(buff, sizeof(buff), "%cSHLL%s V%d.%s, V%d.%s, #%d", a.U?'U':'S', a.Q?"2":"", Rd, Va, Rn, Vd, sh);
        return buff;
    }
    if(isMask(opcode, "0Q0011110hhhhiii010101nnnnnddddd", &a) && (a.h != 0b1000)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        int sz = 3;
        if((a.h&0b1111)==0b0001) sz=0;
        else if((a.h&0b1110)==0b0010) sz=1;
        else if((a.h&0b1100)==0b0100) sz=2;
        int sh=(((a.h)<<3)|(imm)) - (8<<sz);
        const char* Vd = Y[(sz<<1)|a.Q];
        snprintf(buff, sizeof(buff), "SHL%s V%d.%s, V%d.%s, #%d", a.Q?"Q":"", Rd, Vd, Rn, Vd, sh);
        return buff;
    }

    // CNT
    if(isMask(opcode, "0Q00111000100000010110nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B"};
        const char* Vd = Y[a.Q];
        const char* Va = Y[a.Q];
        snprintf(buff, sizeof(buff), "CNT V%d.%s, V%d.%s", Rd, Vd, Rn, Va);
        return buff;
    }

    // DUP
    if(isMask(opcode, "0Q001110000iiiii000001nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        const char* Z[] = {"B", "H", "S", "D"};
        int sz = 3;
        if((imm&0b0001)==0b0001) sz=0;
        else if((imm&0b0011)==0b0010) sz=1;
        else if((imm&0b0111)==0b0100) sz=2;
        int sh=(imm>>sz) - 1;
        const char* Vd = Y[(sz<<1)|a.Q];
        const char* Vn = Z[sz];
        snprintf(buff, sizeof(buff), "DUP V%d.%s, V%d.%s[%d]", Rd, Vd, Rn, Vn, sh);
        return buff;
    }

    if(isMask(opcode, "0Q001110000iiiii000011nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        int sz = 3;
        if((imm&0b0001)==0b0001) sz=0;
        else if((imm&0b0011)==0b0010) sz=1;
        else if((imm&0b0111)==0b0100) sz=2;
        const char* Vd = Y[(sz<<1)|a.Q];
        snprintf(buff, sizeof(buff), "DUP V%d.%s, X%d", Rd, Vd, Rn);
        return buff;
    }

    // AES
    if(isMask(opcode, "0100111000101000010f10nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "AES%c V%d.16B, V%d.16B", sf?'D':'E', Rd, Rn);
        return buff;
    }
    if(isMask(opcode, "0100111000101000011f10nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "AES%sMC V%d.16B, V%d.16B", sf?"I":"", Rd, Rn);
        return buff;
    }
    // PMULL
    if(isMask(opcode, "0Q001110ff1mmmmm111000nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "??", "??", "??", "??", "1D", "2D"};
        const char* Z[] = {"8H", "??", "??", "1Q"};
        int sz = sf;
        const char* Vn = Y[(sz<<1)|a.Q];
        const char* Vd = Z[sz];
        snprintf(buff, sizeof(buff), "PMULL%s V%d.%s, V%d.%s, V%d.%s", a.Q?"2":"", Rd, Vd, Rn, Vn, Rm, Vn);   
        return buff;
    }
    // [S/U]MULL
    if(isMask(opcode, "0QU01110ff1mmmmm110000nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "??"};
        const char* Z[] = {"8H", "4S", "2D", "??"};
        int sz = sf;
        const char* Vn = Y[(sz<<1)|a.Q];
        const char* Vd = Z[sz];
        snprintf(buff, sizeof(buff), "%cMULL%s V%d.%s, V%d.%s, V%d.%s", a.U?'U':'S', a.Q?"2":"", Rd, Vd, Rn, Vn, Rm, Vn);   
        return buff;
    }
    //XTN(2)
    if(isMask(opcode, "0Q001110ff100001001010nnnnnddddd", &a)) {
        const char* Y[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "??"};
        const char* Z[] = {"8H", "4S", "2D", "??"};
        int sz = sf;
        const char* Vd = Y[(sz<<1)|a.Q];
        const char* Vn = Z[sz];
        snprintf(buff, sizeof(buff), "XTN%s V%d.%s, V%d.%s", a.Q?"2":"", Rd, Vd, Rn, Vn);   
        return buff;
    }

    // DMB
    if(isMask(opcode, "11010101000000110011nnnn10111111", &a)) {
        const char* barrier[] = {
            "???", "???", "???", "???", // 0-3
            "???", "???", "???", "???", // 4-7
            "???", "ISHLD", "ISHST", "ISH", // 8-11
            "???", "LD", "ST", "SY"  // 12-15
        };
        snprintf(buff, sizeof(buff), "DMB %s", barrier[Rn]);
        return buff;
    }
    // DSB
    if(isMask(opcode, "11010101000000110011nnnn10011111", &a)) {
        const char* barrier[] = {
            "???", "???", "???", "???", // 0-3
            "???", "???", "???", "???", // 4-7
            "???", "ISHLD", "ISHST", "ISH", // 8-11
            "???", "LD", "ST", "SY"  // 12-15
        };
        snprintf(buff, sizeof(buff), "DSB %s", barrier[Rn]);
        return buff;
    }

    // CASxw
    if(isMask(opcode, "1f0010001L1ssssso11111nnnnnttttt", &a)) {
        snprintf(buff, sizeof(buff), "CAS%s%s %s, %s, [%s]", a.o?"A":"", a.L?"L":"", sf?Xt[Rs]:Wt[Rs], sf?Xt[Rt]:Wt[Rt], XtSp[Rn]);
        return buff;
    }
    // CAS B/H
    if(isMask(opcode, "0f0010001L1ssssso11111nnnnnttttt", &a)) {
        snprintf(buff, sizeof(buff), "CAS%s%s%s %s, %s, [%s]", a.o?"A":"", a.L?"L":"", sf?"H":"B", Xt[Rs], Xt[Rt], XtSp[Rn]);
        return buff;
    }
    // CASPxw
    if(isMask(opcode, "0f0010000L1ssssso11111nnnnnttttt", &a)) {
        snprintf(buff, sizeof(buff), "CASP%s%s %s,%s, %s,%s, [%s]", a.o?"A":"", a.L?"L":"", sf?Xt[Rs]:Wt[Rs], sf?Xt[Rs+1]:Wt[Rs+1], sf?Xt[Rt]:Wt[Rt], sf?Xt[Rt+1]:Wt[Rt+1], XtSp[Rn]);
        return buff;
    }
    // SWPxw
    if(isMask(opcode, "1f111000AR1sssss100000nnnnnttttt", &a)) {
        snprintf(buff, sizeof(buff), "SWP%s%s %s, %s, [%s]", a.A?"A":"", a.R?"L":"", sf?Xt[Rs]:Wt[Rs], sf?Xt[Rt]:Wt[Rt], XtSp[Rn]);
        return buff;
    }
    // SWP B/H
    if(isMask(opcode, "0f111000AR1sssss100000nnnnnttttt", &a)) {
        snprintf(buff, sizeof(buff), "SWP%s%s%s %s, %s, [%s]", a.A?"A":"", a.R?"L":"", sf?"H":"B", Xt[Rs], Xt[Rt], XtSp[Rn]);
        return buff;
    }
    // LDXXXxw
    if(isMask(opcode, "1f111000AR1sssss0ooo00nnnnnttttt", &a)) {
        const char* ops[] = { "ADD", "CLR", "EOR", "SET", "SMAX, SMIN", "UMAX", "UMIN" };
        if((Rt == 0b11111) && !a.A) {
            snprintf(buff, sizeof(buff), "ST%s%s %s, [%s]", ops[a.o], a.R?"L":"", sf?Xt[Rs]:Wt[Rs], XtSp[Rn]);
        } else {
            snprintf(buff, sizeof(buff), "LD%s%s%s %s, %s, [%s]", ops[a.o], a.A?"A":"", a.R?"L":"", sf?Xt[Rs]:Wt[Rs], sf?Xt[Rt]:Wt[Rt], XtSp[Rn]);
        }
        return buff;
    }
    // LDXXX B/H
    if(isMask(opcode, "0f111000AR1sssss0ooo00nnnnnttttt", &a)) {
        const char* ops[] = { "ADD", "CLR", "EOR", "SET", "SMAX, SMIN", "UMAX", "UMIN" };
        if((Rt == 0b11111) && !a.A) {
            snprintf(buff, sizeof(buff), "ST%s%s%s %s, [%s]", ops[a.o], a.R?"L":"", sf?"H":"B", Xt[Rs], XtSp[Rn]);
        } else {
            snprintf(buff, sizeof(buff), "LD%s%s%s%s %s, %s, [%s]", ops[a.o], a.A?"A":"", a.R?"L":"", sf?"H":"B", Xt[Rs], Xt[Rt], XtSp[Rn]);
        }
        return buff;
    }
    // AXFLAG
    if(isMask(opcode, "11010101000000000100000001011111", &a)) {
        snprintf(buff, sizeof(buff), "AXFLAG");
        return buff;
    }
    // XAFLAG
    if(isMask(opcode, "11010101000000000100000000111111", &a)) {
        snprintf(buff, sizeof(buff), "XAFLAG");
        return buff;
    }
    // CFINV
    if(isMask(opcode, "11010101000000000100000000011111", &a)) {
        snprintf(buff, sizeof(buff), "CFINV");
        return buff;
    }
    // RMIF
    if(isMask(opcode, "10111010000iiiiii00001nnnnn0oooo", &a)) {
        snprintf(buff, sizeof(buff), "RMIF %s, #%d, #0x%x", Xt[Rn], imm, opc);
        return buff;
    }
    // SETF
    if(isMask(opcode, "00111010000000000f0010nnnnn01101", &a)) {
        snprintf(buff, sizeof(buff), "SETF%d %s", 8<<sf, Xt[Rn]);
        return buff;
    }
    // REV64
    if(isMask(opcode, "0Q001110ff100000000010nnnnnddddd", &a)) {
        const char* T[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "??"};
        int sz = sf;
        const char* Vn = T[(sz<<1)|a.Q];
        const char* Vd = T[(sz<<1)|a.Q];
        snprintf(buff, sizeof(buff), "REV64 V%d.%s, V%d.%s",Rd, Vd, Rn, Vn);
        return buff;
    }
    //TRNx
    if(isMask(opcode, "0Q001110ff0mmmmm0o1010nnnnnddddd", &a)) {
        const char* T[] = {"8B", "16B", "4H", "8H", "2S", "4S", "??", "2D"};
        int sz = sf;
        const char* Vn = T[(sz<<1)|a.Q];
        const char* Vm = T[(sz<<1)|a.Q];
        const char* Vd = T[(sz<<1)|a.Q];
        snprintf(buff, sizeof(buff), "TRN%d V%d.%s, V%d.%s, V%d.%s", a.o+1, Rd, Vd, Rn, Vn, Rm, Vm);
        return buff;
    }
    //SHA1 stuffs
    if(isMask(opcode, "0101111000101000000010nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "SHA1H S%d, S%d", Rd, Rn);
        return buff;
    }
    if(isMask(opcode, "0101111000101000000110nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "SHA1SU1 V%d.4S, V%d.4S", Rd, Rn);
        return buff;
    }
    if(isMask(opcode, "01011110000mmmmm000000nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "SHA1C Q%d, S%d, V%d.4S", Rd, Rn, Rm);
        return buff;
    }
    if(isMask(opcode, "01011110000mmmmm001000nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "SHA1M Q%d, S%d, V%d.4S", Rd, Rn, Rm);
        return buff;
    }
    if(isMask(opcode, "01011110000mmmmm000100nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "SHA1P Q%d, S%d, V%d.4S", Rd, Rn, Rm);
        return buff;
    }
    //SHA256 stuffs
    if(isMask(opcode, "0101111000101000001010nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "SHA256SU0 V%d.4S, V%d.4S", Rd, Rn);
        return buff;
    }
    if(isMask(opcode, "01011110000mmmmm011000nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "SHA256SU1 V%d.4S, V%d.4S, V%d.4S", Rd, Rn, Rm);
        return buff;
    }
    if(isMask(opcode, "01011110000mmmmm010000nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "SHA256H Q%d, Q%d, V%d.4S", Rd, Rn, Rm);
        return buff;
    }
    if(isMask(opcode, "01011110000mmmmm010100nnnnnddddd", &a)) {
        snprintf(buff, sizeof(buff), "SHA256H2 Q%d, Q%d, V%d.4S", Rd, Rn, Rm);
        return buff;
    }
    // UDF
    if(isMask(opcode, "0000000000000000iiiiiiiiiiiiiiii", &a)) {
        snprintf(buff, sizeof(buff), "UDF 0x%x", a.i);
        return buff;
    }

    snprintf(buff, sizeof(buff), "%08X ???", __builtin_bswap32(opcode));
    return buff;
}
