#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "la64_printer.h"
#include "debug.h"

static const char* Xt[] = {"xZR", "r1", "r2", "sp", "xEmu", "x1_r5", "x2_r6", "x3_r7", "x4_r8", "x5_r9", "x6_r10", "xMASK_r11", "xRAX_r12", "xRCX_r13", "xRDX_r14", "xRBX_r15", "xRSP_r16", "xRBP_r17", "xRSI_r18", "xRDI_r19", "xRIP_r20", "r21", "r22", "xR8_r23", "xR9_r24", "xR10_r25", "xR11_r26", "xR12_r27", "xR13_r28", "xR14_r29", "xR15_r30", "xFlags_r31"};
static const char* fpnames[] = {
    "fa0",  "fa1",  "fa2",  "fa3",  "fa4",  "fa5",  "fa6",  "fa7",
    "ft0",  "ft1",  "ft2",  "ft3",  "ft4",  "ft5",  "ft6",  "ft7",
    "ft8",  "ft9",  "ft10", "ft11", "ft12", "ft13", "ft14", "ft15",
    "fs0",  "fs1",  "fs2",  "fs3",  "fs4",  "fs5",  "fs6",  "fs7",
};
static const char* vrpnames[] = {
    "vra0",  "vra1",  "vra2",  "vra3",  "vra4",  "vra5",  "vra6",  "vra7",
    "vrt0",  "vrt1",  "vrt2",  "vrt3",  "vrt4",  "vrt5",  "vrt6",  "vrt7",
    "vrt8",  "vrt9",  "vrt10", "vrt11", "vrt12", "vrt13", "vrt14", "vrt15",
    "vrs0",  "vrs1",  "vrs2",  "vrs3",  "vrs4",  "vrs5",  "vrs6",  "vrs7",
};

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
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ADD.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // SUB.W
    if(isMask(opcode, "00000000000100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SUB.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // ADD.D
    if(isMask(opcode, "00000000000100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ADD.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // SUB.D
    if(isMask(opcode, "00000000000100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SUB.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // ADDI.W
    if(isMask(opcode, "0000001010iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ADDI.W", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ADDI.D
    if(isMask(opcode, "0000001011iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ADDI.D", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ADDU16I.D
    if(isMask(opcode, "000100iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ADDU16I.D", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ALSL.W
    if(isMask(opcode, "000000000000010iikkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %d", "ALSL.W", Xt[Rd], Xt[Rj], Xt[Rk], imm);
        return buff;
    }
    // ALSL.WU
    if(isMask(opcode, "000000000000011iikkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %d", "ALSL.WU", Xt[Rd], Xt[Rj], Xt[Rk], imm);
        return buff;
    }
    // ALSL.D
    if(isMask(opcode, "000000000010110iikkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %d", "ALSL.D", Xt[Rd], Xt[Rj], Xt[Rk], imm);
        return buff;
    }
    // LU12I.W
    if(isMask(opcode, "0001010iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%x", "LU12I.W", Xt[Rd], imm);
        return buff;
    }
    // LU32I.D
    if(isMask(opcode, "0001011iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "LU32I.D", Xt[Rd], imm);
        return buff;
    }
    // LU52I.D
    if(isMask(opcode, "0000001100iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LU52I.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // SLT
    if(isMask(opcode, "00000000000100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SLT", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // SLTU
    if(isMask(opcode, "00000000000100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SLTU", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // SLTI
    if(isMask(opcode, "0000001000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "SLTI", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // SLTUI
    if(isMask(opcode, "0000001001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SLTI", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // PCADDI
    if(isMask(opcode, "0001100iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "PCADDI", Xt[Rd], signExtend(imm << 2, 22));
        return buff;
    }
    // PCALAU12I
    if(isMask(opcode, "0001101iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "PCALAU12I", Xt[Rd], signExtend(imm << 12, 32));
        return buff;
    }
    // PCADDU12I
    if(isMask(opcode, "0001110iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "PCADDU12I", Xt[Rd], signExtend(imm << 12, 32));
        return buff;
    }
    // PCADDU18I
    if(isMask(opcode, "0001111iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "PCADDU18I", Xt[Rd], signExtend(imm << 18, 38));
        return buff;
    }
    // LL.W
    if(isMask(opcode, "00100000iiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LL.W", Xt[Rd], Xt[Rj], signExtend(imm << 2, 16));
        return buff;
    }
    // SC.W
    if(isMask(opcode, "00100001iiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "SC.W", Xt[Rd], Xt[Rj], signExtend(imm << 2, 16));
        return buff;
    }
    // LL.D
    if(isMask(opcode, "00100010iiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LL.D", Xt[Rd], Xt[Rj], signExtend(imm << 2, 16));
        return buff;
    }
    // SC.D
    if(isMask(opcode, "00100011iiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "SC.D", Xt[Rd], Xt[Rj], signExtend(imm << 2, 16));
        return buff;
    }
    // DBAR
    if(isMask(opcode, "00111000011100100iiiiiiiiiiiiiii", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %d", "DBAR", imm);
        return buff;
    }
    // AND
    if(isMask(opcode, "00000000000101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "AND", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // OR
    if(isMask(opcode, "00000000000101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "OR", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // NOR
    if(isMask(opcode, "00000000000101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "NOR", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // XOR
    if(isMask(opcode, "00000000000101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XOR", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // ANDN
    if(isMask(opcode, "00000000000101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ANDN", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // ORN
    if(isMask(opcode, "00000000000101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ORN", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // NOP -- special case of ANDI
    if(isMask(opcode, "00000011010000000000000000000000", &a)) {
        snprintf(buff, sizeof(buff), "NOP");
        return buff;
    }
    // ANDI
    if(isMask(opcode, "0000001101iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "ANDI", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // ORI
    if(isMask(opcode, "0000001110iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "ORI", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // XORI
    if(isMask(opcode, "0000001111iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "XORI", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // SLL.W
    if(isMask(opcode, "00000000000101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SLL.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // SRL.W
    if(isMask(opcode, "00000000000101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SRL.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // SRA.W
    if(isMask(opcode, "00000000000110000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SRA.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // ROTR.W
    if(isMask(opcode, "00000000000110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ROTR.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // SLL.D
    if(isMask(opcode, "00000000000110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SLL.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // SRL.D
    if(isMask(opcode, "00000000000110010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SRL.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // SRA.D
    if(isMask(opcode, "00000000000110011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SRA.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // ROTR.D
    if(isMask(opcode, "00000000000110111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ROTR.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    // SLLI.D
    if(isMask(opcode, "0000000001000001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SLLI.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // SRLI.D
    if(isMask(opcode, "0000000001000101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SRLI.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // SRAI.D
    if(isMask(opcode, "0000000001001001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SRAI.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // ROTRI.D
    if(isMask(opcode, "0000000001001101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "ROTRI.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // SLLI.W
    if(isMask(opcode, "00000000010000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SLLI.W", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // SRLI.W
    if(isMask(opcode, "00000000010001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SRLI.W", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // SRAI.W
    if(isMask(opcode, "00000000010010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SRAI.W", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // ROTRI.W
    if(isMask(opcode, "00000000010011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "ROTRI.W", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    // BSTRINS.W
    if(isMask(opcode, "00000000011uuuuu0iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u, %u", "BSTRINS.W", Xt[Rd], Xt[Rj], imm_up, imm);
        return buff;
    }
    // BSTRINS.D
    if(isMask(opcode, "0000000010uuuuuuiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u, %u", "BSTRINS.D", Xt[Rd], Xt[Rj], imm_up, imm);
        return buff;
    }
    // BSTRPICK.W
    if(isMask(opcode, "00000000011uuuuu1iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u, %u", "BSTRPICK.W", Xt[Rd], Xt[Rj], imm_up, imm);
        return buff;
    }
    // BSTRPICK.D
    if(isMask(opcode, "0000000011uuuuuuiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u, %u", "BSTRPICK.D", Xt[Rd], Xt[Rj], imm_up, imm);
        return buff;
    }
    // BEQ
    if(isMask(opcode, "010110iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "BEQ", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    // BNE
    if(isMask(opcode, "010111iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "BNE", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    // BLT
    if(isMask(opcode, "011000iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "BLT", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    // BGE
    if(isMask(opcode, "011001iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "BGE", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    // BLTU
    if(isMask(opcode, "011010iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "BLTU", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    // BGEU
    if(isMask(opcode, "011011iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "BGEU", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    // BEQZ
    if(isMask(opcode, "010000iiiiiiiiiiiiiiiijjjjjuuuuu", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "BEQZ", Xt[Rj], signExtend(imm + (imm_up << 16) << 2, 23));
        return buff;
    }
    // BNEZ
    if(isMask(opcode, "010001iiiiiiiiiiiiiiiijjjjjuuuuu", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "BNEZ", Xt[Rj], signExtend(imm + (imm_up << 16) << 2, 23));
        return buff;
    }
    // BR -- special case of JIRL
    if(isMask(opcode, "0100110000000000000000jjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "JR", Xt[Rj]);
        return buff;
    }
    // JIRL
    if(isMask(opcode, "010011iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "JIRL", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    // B
    if(isMask(opcode, "010100iiiiiiiiiiiiiiiiiiiiiiiiii", &a)) {
        snprintf(buff, sizeof(buff), "%-15s 0x%x", "B", (((imm & 0x3FF) << 16) | ((uint32_t)imm >> 10)) << 6 >> 4);
        return buff;
    }
    // LD.B
    if(isMask(opcode, "0010100000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.B", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // LD.H
    if(isMask(opcode, "0010100001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.H", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // LD.W
    if(isMask(opcode, "0010100010iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.W", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // LD.D
    if(isMask(opcode, "0010100011iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.D", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // LD.BU
    if(isMask(opcode, "0010101000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.BU", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // LD.HU
    if(isMask(opcode, "0010101001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.HU", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // LD.WU
    if(isMask(opcode, "0010101010iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.WU", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ST.B
    if(isMask(opcode, "0010100100iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ST.B", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ST.H
    if(isMask(opcode, "0010100101iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ST.H", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ST.W
    if(isMask(opcode, "0010100110iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ST.W", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // ST.D
    if(isMask(opcode, "0010100111iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ST.D", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // FLD.D
    if(isMask(opcode, "0010101110iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "FLD.D", fpnames[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // FADD.D
    if(isMask(opcode, "00000001000000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FADD.D", fpnames[Rd], fpnames[Rj], fpnames[Rk]);
        return buff;
    }
    // FSUB.D
    if(isMask(opcode, "00000001000000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FSUB.D", fpnames[Rd], fpnames[Rj], fpnames[Rk]);
        return buff;
    }
    // FMUL.D
    if(isMask(opcode, "00000001000001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FMUL.D", fpnames[Rd], fpnames[Rj], fpnames[Rk]);
        return buff;
    }
    // FDIV.D
    if(isMask(opcode, "00000001000001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FDIV.D", fpnames[Rd], fpnames[Rj], fpnames[Rk]);
        return buff;
    }
    // VEXTRINS.D
    if(isMask(opcode, "01110011100000iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "VEXTRINS.D", fpnames[Rd], fpnames[Rj], signExtend(imm, 8));
        return buff;
    }
    // VLD
    if(isMask(opcode, "0010110000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "VLD.D", vrpnames[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    // EXT.W.H
    if(isMask(opcode, "0000000000000000010110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "EXT.W.H", Xt[Rd], Xt[Rj]);
        return buff;
    }
    // EXT.W.B
    if(isMask(opcode, "0000000000000000010111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "EXT.W.B", Xt[Rd], Xt[Rj]);
        return buff;
    }
    // X64CLRSM
    if(isMask(opcode, "00000000000000001000000000101000", &a)) {
        snprintf(buff, sizeof(buff), "X64CLRSM");
        return buff;
    }
    // X64SETSM
    if(isMask(opcode, "00000000000000001000000000001000", &a)) {
        snprintf(buff, sizeof(buff), "X64SETSM");
        return buff;
    }
    // X64INCTOP
    if(isMask(opcode, "00000000000000001000000000001001", &a)) {
        snprintf(buff, sizeof(buff), "X64INCTOP");
        return buff;
    }
    // X64DECTOP
    if(isMask(opcode, "00000000000000001000000000101001", &a)) {
        snprintf(buff, sizeof(buff), "X64DECTOP");
        return buff;
    }
    // X64SETTOP
    if(isMask(opcode, "000000000000000001110000iii00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %d", "X64SETTOP", imm);
        return buff;
    }
    // X64GETTOP
    if(isMask(opcode, "000000000000000001110100000ddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64GETTOP", Xt[Rd]);
        return buff;
    }
    // X64GETEFLAGS
    if(isMask(opcode, "00000000010111iiiiiiii00000ddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%x", "X64GETEFLAGS", Xt[Rd], imm);
        return buff;
    }
    // X64SETEFLAGS
    if(isMask(opcode, "00000000010111iiiiiiii00001ddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%x", "X64SETEFLAGS", Xt[Rd], imm);
        return buff;
    }
    // X64SETJ
    if(isMask(opcode, "000000000011011010iiii00000ddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%x", "X64SETJ", Xt[Rd], imm);
        return buff;
    }
    // X64INC.B
    if(isMask(opcode, "0000000000000000100000jjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64INC.B", Xt[Rj]);
        return buff;
    }
    // X64INC.H
    if(isMask(opcode, "0000000000000000100000jjjjj00001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64INC.H", Xt[Rj]);
        return buff;
    }
    // X64INC.W
    if(isMask(opcode, "0000000000000000100000jjjjj00010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64INC.W", Xt[Rj]);
        return buff;
    }
    // X64INC.D
    if(isMask(opcode, "0000000000000000100000jjjjj00011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64INC.D", Xt[Rj]);
        return buff;
    }
    // X64DEC.B
    if(isMask(opcode, "0000000000000000100000jjjjj00100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64DEC.B", Xt[Rj]);
        return buff;
    }
    // X64DEC.H
    if(isMask(opcode, "0000000000000000100000jjjjj00101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64DEC.H", Xt[Rj]);
        return buff;
    }
    // X64DEC.W
    if(isMask(opcode, "0000000000000000100000jjjjj00110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64DEC.W", Xt[Rj]);
        return buff;
    }
    // X64DEC.D
    if(isMask(opcode, "0000000000000000100000jjjjj00111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64DEC.D", Xt[Rj]);
        return buff;
    }
    // X64MUL.B
    if(isMask(opcode, "00000000001111101kkkkkjjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64MUL.H
    if(isMask(opcode, "00000000001111101kkkkkjjjjj00001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64MUL.W
    if(isMask(opcode, "00000000001111101kkkkkjjjjj00010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64MUL.D
    if(isMask(opcode, "00000000001111101kkkkkjjjjj00011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64MUL.BU
    if(isMask(opcode, "00000000001111101kkkkkjjjjj00100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64MUL.HU
    if(isMask(opcode, "00000000001111101kkkkkjjjjj00101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64MUL.WU
    if(isMask(opcode, "00000000001111101kkkkkjjjjj00110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64MUL.DU
    if(isMask(opcode, "00000000001111101kkkkkjjjjj00111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ADD.WU
    if(isMask(opcode, "00000000001111110kkkkkjjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADD.WU", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ADD.DU
    if(isMask(opcode, "00000000001111110kkkkkjjjjj00001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADD.DU", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SUB.WU
    if(isMask(opcode, "00000000001111110kkkkkjjjjj00010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SUB.WU", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SUB.DU
    if(isMask(opcode, "00000000001111110kkkkkjjjjj00011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SUB.DU", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ADD.B
    if(isMask(opcode, "00000000001111110kkkkkjjjjj00100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADD.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ADD.H
    if(isMask(opcode, "00000000001111110kkkkkjjjjj00101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADD.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ADD.W
    if(isMask(opcode, "00000000001111110kkkkkjjjjj00110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADD.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ADD.D
    if(isMask(opcode, "00000000001111110kkkkkjjjjj00111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADD.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SUB.B
    if(isMask(opcode, "00000000001111110kkkkkjjjjj01000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SUB.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SUB.H
    if(isMask(opcode, "00000000001111110kkkkkjjjjj01001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SUB.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SUB.W
    if(isMask(opcode, "00000000001111110kkkkkjjjjj01010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SUB.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SUB.D
    if(isMask(opcode, "00000000001111110kkkkkjjjjj01011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SUB.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ADC.B
    if(isMask(opcode, "00000000001111110kkkkkjjjjj01100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADC.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ADC.H
    if(isMask(opcode, "00000000001111110kkkkkjjjjj01101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADC.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ADC.W
    if(isMask(opcode, "00000000001111110kkkkkjjjjj01110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADC.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ADC.D
    if(isMask(opcode, "00000000001111110kkkkkjjjjj01111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADC.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SBC.B
    if(isMask(opcode, "00000000001111110kkkkkjjjjj10000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SBC.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SBC.H
    if(isMask(opcode, "00000000001111110kkkkkjjjjj10001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SBC.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SBC.W
    if(isMask(opcode, "00000000001111110kkkkkjjjjj10010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SBC.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SBC.D
    if(isMask(opcode, "00000000001111110kkkkkjjjjj10011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SBC.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SLL.B
    if(isMask(opcode, "00000000001111110kkkkkjjjjj10100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SLL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SLL.H
    if(isMask(opcode, "00000000001111110kkkkkjjjjj10101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SLL.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SLL.W
    if(isMask(opcode, "00000000001111110kkkkkjjjjj10110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SLL.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SLL.D
    if(isMask(opcode, "00000000001111110kkkkkjjjjj10111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SLL.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SRL.B
    if(isMask(opcode, "00000000001111110kkkkkjjjjj11000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SRL.H
    if(isMask(opcode, "00000000001111110kkkkkjjjjj11001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRL.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SRL.W
    if(isMask(opcode, "00000000001111110kkkkkjjjjj11010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRL.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SRL.D
    if(isMask(opcode, "00000000001111110kkkkkjjjjj11011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRL.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SRA.B
    if(isMask(opcode, "00000000001111110kkkkkjjjjj11100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRA.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SRA.H
    if(isMask(opcode, "00000000001111110kkkkkjjjjj11101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRA.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SRA.W
    if(isMask(opcode, "00000000001111110kkkkkjjjjj11110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRA.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SRA.D
    if(isMask(opcode, "00000000001111110kkkkkjjjjj11111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRA.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ROTR.B
    if(isMask(opcode, "00000000001111111kkkkkjjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTR.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ROTR.H
    if(isMask(opcode, "00000000001111111kkkkkjjjjj00001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTR.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ROTR.D
    if(isMask(opcode, "00000000001111111kkkkkjjjjj00010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTR.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ROTR.W
    if(isMask(opcode, "00000000001111111kkkkkjjjjj00011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTR.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ROTL.B
    if(isMask(opcode, "00000000001111111kkkkkjjjjj00100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ROTL.H
    if(isMask(opcode, "00000000001111111kkkkkjjjjj00101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTL.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ROTL.W
    if(isMask(opcode, "00000000001111111kkkkkjjjjj00110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTL.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64ROTL.D
    if(isMask(opcode, "00000000001111111kkkkkjjjjj00111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTL.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64RCR.B
    if(isMask(opcode, "00000000001111111kkkkkjjjjj01000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCR.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64RCR.H
    if(isMask(opcode, "00000000001111111kkkkkjjjjj01001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCR.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64RCR.W
    if(isMask(opcode, "00000000001111111kkkkkjjjjj01010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCR.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64RCR.D
    if(isMask(opcode, "00000000001111111kkkkkjjjjj01011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCR.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64RCL.B
    if(isMask(opcode, "00000000001111111kkkkkjjjjj01100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64RCL.H
    if(isMask(opcode, "00000000001111111kkkkkjjjjj01101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCL.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64RCL.W
    if(isMask(opcode, "00000000001111111kkkkkjjjjj01110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCL.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64RCL.D
    if(isMask(opcode, "00000000001111111kkkkkjjjjj01111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCL.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64AND.B
    if(isMask(opcode, "00000000001111111kkkkkjjjjj10000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64AND.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64AND.H
    if(isMask(opcode, "00000000001111111kkkkkjjjjj10001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64AND.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64AND.W
    if(isMask(opcode, "00000000001111111kkkkkjjjjj10010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64AND.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64AND.D
    if(isMask(opcode, "00000000001111111kkkkkjjjjj10011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64AND.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64OR.B
    if(isMask(opcode, "00000000001111111kkkkkjjjjj10100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64OR.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64OR.H
    if(isMask(opcode, "00000000001111111kkkkkjjjjj10101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64OR.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64OR.W
    if(isMask(opcode, "00000000001111111kkkkkjjjjj10110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64OR.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64OR.D
    if(isMask(opcode, "00000000001111111kkkkkjjjjj10111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64OR.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64XOR.B
    if(isMask(opcode, "00000000001111111kkkkkjjjjj11000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64XOR.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64XOR.H
    if(isMask(opcode, "00000000001111111kkkkkjjjjj11001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64XOR.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64XOR.W
    if(isMask(opcode, "00000000001111111kkkkkjjjjj11010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64XOR.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64XOR.D
    if(isMask(opcode, "00000000001111111kkkkkjjjjj11011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64XOR.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    // X64SLLI.B
    if(isMask(opcode, "0000000001010100001iiijjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SLLI.B", Xt[Rj], imm);
        return buff;
    }
    // X64SRLI.B
    if(isMask(opcode, "0000000001010100001iiijjjjj00100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRLI.B", Xt[Rj], imm);
        return buff;
    }
    // X64SRAI.B
    if(isMask(opcode, "0000000001010100001iiijjjjj01000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRAI.B", Xt[Rj], imm);
        return buff;
    }
    // X64ROTRI.B
    if(isMask(opcode, "0000000001010100001iiijjjjj01100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTRI.B", Xt[Rj], imm);
        return buff;
    }
    // X64RCRI.B
    if(isMask(opcode, "0000000001010100001iiijjjjj10000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCRI.B", Xt[Rj], imm);
        return buff;
    }
    // X64ROTLI.B
    if(isMask(opcode, "0000000001010100001iiijjjjj10100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTLI.B", Xt[Rj], imm);
        return buff;
    }
    // X64RCLI.B
    if(isMask(opcode, "0000000001010100001iiijjjjj11000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCLI.B", Xt[Rj], imm);
        return buff;
    }
    // X64SLLI.H
    if(isMask(opcode, "000000000101010001iiiijjjjj00001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SLLI.H", Xt[Rj], imm);
        return buff;
    }
    // X64SRLI.H
    if(isMask(opcode, "000000000101010001iiiijjjjj00101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRLI.H", Xt[Rj], imm);
        return buff;
    }
    // X64SRAI.H
    if(isMask(opcode, "000000000101010001iiiijjjjj01001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRAI.H", Xt[Rj], imm);
        return buff;
    }
    // X64ROTRI.H
    if(isMask(opcode, "000000000101010001iiiijjjjj01101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTRI.H", Xt[Rj], imm);
        return buff;
    }
    // X64RCRI.H
    if(isMask(opcode, "000000000101010001iiiijjjjj10001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCRI.H", Xt[Rj], imm);
        return buff;
    }
    // X64ROTLI.H
    if(isMask(opcode, "000000000101010001iiiijjjjj10101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTLI.H", Xt[Rj], imm);
        return buff;
    }
    // X64RCLI.H
    if(isMask(opcode, "000000000101010001iiiijjjjj11001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCLI.H", Xt[Rj], imm);
        return buff;
    }
    // X64SLLI.W
    if(isMask(opcode, "00000000010101001iiiiijjjjj00010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SLLI.W", Xt[Rj], imm);
        return buff;
    }
    // X64SRLI.W
    if(isMask(opcode, "00000000010101001iiiiijjjjj00110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRLI.W", Xt[Rj], imm);
        return buff;
    }
    // X64SRAI.W
    if(isMask(opcode, "00000000010101001iiiiijjjjj01010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRAI.W", Xt[Rj], imm);
        return buff;
    }
    // X64ROTRI.W
    if(isMask(opcode, "00000000010101001iiiiijjjjj01110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTRI.W", Xt[Rj], imm);
        return buff;
    }
    // X64RCRI.W
    if(isMask(opcode, "00000000010101001iiiiijjjjj10010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCRI.W", Xt[Rj], imm);
        return buff;
    }
    // X64ROTLI.W
    if(isMask(opcode, "00000000010101001iiiiijjjjj10110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTLI.W", Xt[Rj], imm);
        return buff;
    }
    // X64RCLI.W
    if(isMask(opcode, "00000000010101001iiiiijjjjj11010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCLI.W", Xt[Rj], imm);
        return buff;
    }
    // X64SLLI.D
    if(isMask(opcode, "0000000001010101iiiiiijjjjj00011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SLLI.D", Xt[Rj], imm);
        return buff;
    }
    // X64SRLI.D
    if(isMask(opcode, "0000000001010101iiiiiijjjjj00111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRLI.D", Xt[Rj], imm);
        return buff;
    }
    // X64SRAI.D
    if(isMask(opcode, "0000000001010101iiiiiijjjjj01011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRAI.D", Xt[Rj], imm);
        return buff;
    }
    // X64ROTRI.D
    if(isMask(opcode, "0000000001010101iiiiiijjjjj01111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTRI.D", Xt[Rj], imm);
        return buff;
    }
    // X64RCRI.D
    if(isMask(opcode, "0000000001010101iiiiiijjjjj10011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCRI.D", Xt[Rj], imm);
        return buff;
    }
    // X64ROTLI.D
    if(isMask(opcode, "0000000001010101iiiiiijjjjj10111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTLI.D", Xt[Rj], imm);
        return buff;
    }
    // X64RCLI.D
    if(isMask(opcode, "0000000001010101iiiiiijjjjj11011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCLI.D", Xt[Rj], imm);
        return buff;
    }

    snprintf(buff, sizeof(buff), "%08X ???", __builtin_bswap32(opcode));
    return buff;
}