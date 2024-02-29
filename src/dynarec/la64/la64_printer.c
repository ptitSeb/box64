#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "la64_printer.h"
#include "debug.h"

static const char* Xt[] = {"xZR", "r1", "r2", "sp", "xEmu", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "xRAX", "xRCX", "xRDX", "xRBX", "xRSP", "xRBP", "xRSI", "xRDI", "xR8", "r21", "xR9", "xR10", "xR11", "xR12", "xR13", "xR14", "xR15", "xFlags", "xRIP", "r31"};

typedef struct la64_print_s {
    int d, j, k, a;
    int i, u;
} la64_print_t;

int isMask(uint32_t opcode, const char* mask, la64_print_t *a)
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
            case 'd': a->d = (a->d<<1) | v; break;
            case 'j': a->j = (a->j<<1) | v; break;
            case 'k': a->k = (a->k<<1) | v; break;
            case 'a': a->a = (a->a<<1) | v; break;
            case 'i': a->i = (a->i<<1) | v; break;
            case 'u': a->u = (a->u<<1) | v; break;
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

const char* la64_print(uint32_t opcode, uintptr_t addr)
{
    static char buff[200];
    la64_print_t a;
    #define Rd a.d
    #define Rj a.j
    #define Rk a.k
    #define Ra a.a
    #define imm a.i
    #define imm_up a.u
    // ADD.W
    if(isMask(opcode, "00000000000100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ADD.W %s, %s, %s", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // SUB.W
    if(isMask(opcode, "00000000000100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "SUB.W %s, %s, %s", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // ADD.D
    if(isMask(opcode, "00000000000100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ADD.D %s, %s, %s", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // SUB.D
    if(isMask(opcode, "00000000000100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "SUB.D %s, %s, %s", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // ADDI.W
    if(isMask(opcode, "0000001010iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ADDI.W %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ADDI.D
    if(isMask(opcode, "0000001011iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ADDI.D %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ADDU16I.D
    if(isMask(opcode, "000100iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ADDU16I.D %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ALSL.W
    if(isMask(opcode, "000000000000010iikkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ALSL.W %s, %s, %s, %d", Xt[Rd], Xt[Rj], Xt[Rk], imm);
        return buff;
    }
    // ALSL.WU
    if(isMask(opcode, "000000000000011iikkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ALSL.WU %s, %s, %s, %d", Xt[Rd], Xt[Rj], Xt[Rk], imm);
        return buff;
    }
    // ALSL.D
    if(isMask(opcode, "000000000010110iikkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ALSL.D %s, %s, %s, %d", Xt[Rd], Xt[Rj], Xt[Rk], imm);
        return buff;
    }
    // LU12I.W
    if(isMask(opcode, "0001010iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "LU12I.W %s, %d", Xt[Rd], imm);
        return buff;
    }
    // LU32I.D
    if(isMask(opcode, "0001011iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "LU32I.D %s, %d", Xt[Rd], imm);
        return buff;
    }
    // LU52I.D
    if(isMask(opcode, "0000001100iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "LU52I.D %s, %s, %d", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // PCADDI
    if(isMask(opcode, "0001100iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "PCADDI %s, %d", Xt[Rd], imm);
        return buff;
    }
    // PCADDU12I
    if(isMask(opcode, "0001101iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "PCADDU12I %s, %d", Xt[Rd], imm);
        return buff;
    }
    // PCADDU18I
    if(isMask(opcode, "0001110iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "PCADDU18I %s, %d", Xt[Rd], imm);
        return buff;
    }
    // PCALAU12I
    if(isMask(opcode, "0001111iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "PCALAU12I %s, %d", Xt[Rd], imm);
        return buff;
    }
    // AND
    if(isMask(opcode, "00000000000101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "AND %s, %s, %s", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // OR
    if(isMask(opcode, "00000000000101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "OR %s, %s, %s", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // NOR
    if(isMask(opcode, "00000000000101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "NOR %s, %s, %s", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // XOR
    if(isMask(opcode, "00000000000101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "XOR %s, %s, %s", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // ANDN
    if(isMask(opcode, "00000000000101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ANDN %s, %s, %s", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // ORN
    if(isMask(opcode, "00000000000101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ORN %s, %s, %s", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // ANDI
    if(isMask(opcode, "0000001101iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ANDI %s, %s, 0x%x", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // ORI
    if(isMask(opcode, "0000001110iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ORI %s, %s, 0x%x", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // XORI
    if(isMask(opcode, "0000001111iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "XORI %s, %s, 0x%x", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // SLLI.D
    if(isMask(opcode, "0000000001000001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "SLLI.D %s, %s, %u", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // SRLI.D
    if(isMask(opcode, "0000000001000101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "SRLI.D %s, %s, %u", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // SRAI.D
    if(isMask(opcode, "0000000001001001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "SRAI.D %s, %s, %u", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // ROTRI.D
    if(isMask(opcode, "0000000001001101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ROTRI.D %s, %s, %u", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // BEQZ
    if(isMask(opcode, "010000iiiiiiiiiiiiiiiijjjjjuuuuu", &a)) {
        snprintf(buff, sizeof(buff), "BEQZ %s, %d", Xt[Rj], imm + (imm_up << 16));
        return buff;
    }
    // BNEZ
    if(isMask(opcode, "010001iiiiiiiiiiiiiiiijjjjjuuuuu", &a)) {
        snprintf(buff, sizeof(buff), "BNEZ %s, %d", Xt[Rj], imm + (imm_up << 16));
        return buff;
    }
    // JIRL
    if(isMask(opcode, "010011iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "JIRL %s, %s, %d", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // LD.B
    if(isMask(opcode, "0010100000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "LD.B %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // LD.H
    if(isMask(opcode, "0010100001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "LD.H %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // LD.W
    if(isMask(opcode, "0010100010iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "LD.W %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // LD.D
    if(isMask(opcode, "0010100011iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "LD.D %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // LD.BU
    if(isMask(opcode, "0010101000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "LD.BU %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // LD.HU
    if(isMask(opcode, "0010101001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "LD.HU %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // LD.WU
    if(isMask(opcode, "0010101010iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "LD.WU %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ST.B
    if(isMask(opcode, "0010100100iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ST.B %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ST.H
    if(isMask(opcode, "0010100101iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ST.H %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ST.W
    if(isMask(opcode, "0010100110iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ST.W %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ST.D
    if(isMask(opcode, "0010100111iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "ST.D %s, %s, %d", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    snprintf(buff, sizeof(buff), "%08X ???", __builtin_bswap32(opcode));
    return buff;
}