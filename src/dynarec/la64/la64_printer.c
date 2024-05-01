#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "la64_printer.h"
#include "debug.h"

static const char* Xt[] = { "xZR", "r1", "r2", "sp", "xEmu", "x1_r5", "x2_r6", "x3_r7", "x4_r8", "x5_r9", "x6_r10", "xMASK_r11", "xRAX_r12", "xRCX_r13", "xRDX_r14", "xRBX_r15", "xRSP_r16", "xRBP_r17", "xRSI_r18", "xRDI_r19", "xRIP_r20", "r21", "r22", "xR8_r23", "xR9_r24", "xR10_r25", "xR11_r26", "xR12_r27", "xR13_r28", "xR14_r29", "xR15_r30", "xFlags_r31" };
static const char* Ft[] = { "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7", "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", "ft8", "ft9", "ft10", "ft11", "ft12", "ft13", "ft14", "ft15", "fs0", "fs1", "fs2", "fs3", "fs4", "fs5", "fs6", "fs7" };
static const char* Vt[] = { "vra0", "vra1", "vra2", "vra3", "vra4", "vra5", "vra6", "vra7", "vrt0", "vrt1", "vrt2", "vrt3", "vrt4", "vrt5", "vrt6", "vrt7", "vrt8", "vrt9", "vrt10", "vrt11", "vrt12", "vrt13", "vrt14", "vrt15", "vrs0", "vrs1", "vrs2", "vrs3", "vrs4", "vrs5", "vrs6", "vrs7" };

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

    if (isMask(opcode, "00000000000100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ADD.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SUB.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ADD.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SUB.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "0000001010iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ADDI.W", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0000001011iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ADDI.D", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "000100iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ADDU16I.D", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "000000000000010iikkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %d", "ALSL.W", Xt[Rd], Xt[Rj], Xt[Rk], imm);
        return buff;
    }
    if (isMask(opcode, "000000000000011iikkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %d", "ALSL.WU", Xt[Rd], Xt[Rj], Xt[Rk], imm);
        return buff;
    }
    if (isMask(opcode, "000000000010110iikkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %d", "ALSL.D", Xt[Rd], Xt[Rj], Xt[Rk], imm);
        return buff;
    }
    if (isMask(opcode, "0001010iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%x", "LU12I.W", Xt[Rd], imm);
        return buff;
    }
    if (isMask(opcode, "0001011iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "LU32I.D", Xt[Rd], imm);
        return buff;
    }
    if (isMask(opcode, "0000001100iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LU52I.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000000100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SLT", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SLTU", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "0000001000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "SLTI", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0000001001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SLTI", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0001100iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "PCADDI", Xt[Rd], signExtend(imm << 2, 22));
        return buff;
    }
    if (isMask(opcode, "0001101iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "PCALAU12I", Xt[Rd], signExtend(imm << 12, 32));
        return buff;
    }
    if (isMask(opcode, "0001110iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "PCADDU12I", Xt[Rd], signExtend(imm << 12, 32));
        return buff;
    }
    if (isMask(opcode, "0001111iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "PCADDU18I", Xt[Rd], signExtend(imm << 18, 38));
        return buff;
    }
    if (isMask(opcode, "00100000iiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LL.W", Xt[Rd], Xt[Rj], signExtend(imm << 2, 16));
        return buff;
    }
    if (isMask(opcode, "00100001iiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "SC.W", Xt[Rd], Xt[Rj], signExtend(imm << 2, 16));
        return buff;
    }
    if (isMask(opcode, "00100010iiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LL.D", Xt[Rd], Xt[Rj], signExtend(imm << 2, 16));
        return buff;
    }
    if (isMask(opcode, "00100011iiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "SC.D", Xt[Rd], Xt[Rj], signExtend(imm << 2, 16));
        return buff;
    }
    if (isMask(opcode, "00111000011100100iiiiiiiiiiiiiii", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %d", "DBAR", imm);
        return buff;
    }
    if (isMask(opcode, "00000000000101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "AND", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "OR", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "NOR", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XOR", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ANDN", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ORN", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000011010000000000000000000000", &a)) {
        snprintf(buff, sizeof(buff), "NOP");
        return buff;
    }
    if (isMask(opcode, "0000001101iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "ANDI", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000001110iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "ORI", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000001111iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "XORI", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000000101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SLL.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SRL.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000110000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SRA.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ROTR.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SLL.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000110010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SRL.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000110011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "SRA.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000110111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ROTR.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "0000000001000001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SLLI.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001000101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SRLI.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001001001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SRAI.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001001101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "ROTRI.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SLLI.W", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SRLI.W", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "SRAI.W", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", "ROTRI.W", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000000111000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "MUL.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "MULH.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000111010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "MULH.WU", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000111011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "MUL.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000111100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "MULH.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "MULH.DU", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "MULW.D.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000000111111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "MULW.D.WU", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "DIV.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "MOD.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "DIV.WU", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "MOD.WU", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "DIV.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "MOD.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "DIV.DU", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "MOD.DU", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000011uuuuu0iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u, %u", "BSTRINS.W", Xt[Rd], Xt[Rj], imm_up, imm);
        return buff;
    }
    if (isMask(opcode, "0000000010uuuuuuiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u, %u", "BSTRINS.D", Xt[Rd], Xt[Rj], imm_up, imm);
        return buff;
    }
    if (isMask(opcode, "00000000011uuuuu1iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u, %u", "BSTRPICK.W", Xt[Rd], Xt[Rj], imm_up, imm);
        return buff;
    }
    if (isMask(opcode, "0000000011uuuuuuiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %u, %u", "BSTRPICK.D", Xt[Rd], Xt[Rj], imm_up, imm);
        return buff;
    }
    if (isMask(opcode, "0000000000000000000100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "CLO.W", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000000101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "CLZ.W", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000000110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "CTO.W", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000000111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "CTZ.W", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000001000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "CLO.D", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000001001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "CLZ.D", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000001010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "CTO.D", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000001011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "CTZ.D", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000001100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "REVB.2H", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000001101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "REVB.4H", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000001110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "REVB.2W", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000001111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "REVB.D", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000010000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "REVH.2W", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000010001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "REVH.D", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000010010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "BITREV.4B", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000010011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "BITREV.8B", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000010100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "BITREV.W", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000010101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "BITREV.D", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000010111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "EXT.W.B", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000010110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "EXT.W.H", Xt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "010110iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "BEQ", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "010111iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "BNE", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "011000iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "BLT", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "011001iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "BGE", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "011010iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "BLTU", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "011011iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "BGEU", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "010000iiiiiiiiiiiiiiiijjjjjuuuuu", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "BEQZ", Xt[Rj], signExtend(imm + (imm_up << 16) << 2, 23));
        return buff;
    }
    if (isMask(opcode, "010001iiiiiiiiiiiiiiiijjjjjuuuuu", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "BNEZ", Xt[Rj], signExtend(imm + (imm_up << 16) << 2, 23));
        return buff;
    }
    if (isMask(opcode, "0100110000000000000000jjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "BR", Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "010011iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "JIRL", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "010100iiiiiiiiiiiiiiiiiiiiiiiiii", &a)) {
        snprintf(buff, sizeof(buff), "%-15s 0x%x", "B", (((imm & 0x3FF) << 16) | ((uint32_t)imm >> 10)) << 6 >> 4);
        return buff;
    }
    if (isMask(opcode, "0010100000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.B", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.H", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100010iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.W", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100011iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.D", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010101000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.BU", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010101001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.HU", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010101010iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "LD.WU", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100100iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ST.B", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100101iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ST.H", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100110iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ST.W", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100111iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "ST.D", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "00111000000000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "LDX.B", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00111000000001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "LDX.H", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00111000000010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "LDX.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00111000000011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "LDX.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00111000000100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "STX.B", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00111000000101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "STX.H", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00111000000110000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "STX.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00111000000111000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "STX.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00111000001000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "LDX.BU", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00111000001001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "LDX.HU", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00111000001010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "LDX.WU", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "0010101110iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "FLD.D", Ft[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010101100iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "FLD.S", Ft[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010101111iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "FST.D", Ft[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010101101iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "FST.S", Ft[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "00000001000000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FADD.S", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FADD.D", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FSUB.S", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FSUB.D", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FMUL.S", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FMUL.D", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FDIV.S", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FDIV.D", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FMAX.S", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FMAX.D", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FMIN.S", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FMIN.D", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FMAXA.S", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FMAXA.D", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000011101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FMINA.S", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000011110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FMINA.D", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FSCALEB.S", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FSCALEB.D", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCOPYSIGN.S", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000001000100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCOPYSIGN.D", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100101001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVGR2FR.W", Ft[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100101010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVGR2FR.D", Ft[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100101011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVGR2FRH.W", Ft[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100101101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVFR2GR.S", Xt[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100101110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVFR2GR.D", Xt[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100101111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVFRH2GR.S", Xt[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "01110000000010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADD.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADD.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADD.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADD.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADD.Q", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUB.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUB.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUB.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000011011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUB.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001011011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUB.Q", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSADD.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSADD.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSADD.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSADD.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSADD.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSADD.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSADD.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSADD.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSUB.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSUB.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSUB.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSUB.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSUB.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSUB.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSUB.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010011011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSUB.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHADDW.H.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHADDW.W.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHADDW.D.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHADDW.Q.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010110000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHADDW.HU.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHADDW.WU.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010110010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHADDW.DU.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010110011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHADDW.QU.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHSUBW.H.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHSUBW.W.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHSUBW.D.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHSUBW.Q.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010110100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHSUBW.HU.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHSUBW.WU.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHSUBW.DU.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010110111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VHSUBW.QU.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000111100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWEV.H.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWEV.W.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWEV.D.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000111111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWEV.Q.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWOD.H.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWOD.W.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWOD.D.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWOD.Q.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWEV.H.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWEV.W.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWEV.D.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWEV.Q.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWOD.H.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWOD.W.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWOD.D.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWOD.Q.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001011100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWEV.H.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001011101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWEV.W.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001011110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWEV.D.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001011111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWEV.Q.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWOD.H.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWOD.W.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWOD.D.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001100111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWOD.Q.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWEV.H.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWEV.W.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWEV.D.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWEV.Q.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWOD.H.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWOD.W.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWOD.D.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSUBWOD.Q.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001111100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWEV.H.BU.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWEV.W.HU.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWEV.D.WU.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000001111111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWEV.Q.DU.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWOD.H.BU.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWOD.W.HU.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWOD.D.WU.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDWOD.Q.DU.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVG.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVG.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVG.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVG.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVG.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVG.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVG.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVG.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVGR.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVGR.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVGR.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVGR.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVGR.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVGR.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVGR.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAVGR.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VABSD.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VABSD.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VABSD.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VABSD.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VABSD.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VABSD.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VABSD.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VABSD.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010111000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDA.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDA.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010111010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDA.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000010111011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VADDA.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMAX.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMAX.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMAX.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMAX.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMAX.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMAX.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMAX.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMAX.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMIN.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMIN.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMIN.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011100111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMIN.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMIN.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMIN.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMIN.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000011101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMIN.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMUL.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMUL.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMUL.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMUL.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMUH.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMUH.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMUH.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMUH.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMUH.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMUH.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMUH.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMUH.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWEV.H.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWEV.W.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWEV.D.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWEV.Q.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWOD.H.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWOD.W.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWOD.D.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100100111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWOD.Q.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100110000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWEV.H.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWEV.W.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100110010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWEV.D.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100110011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWEV.Q.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100110100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWOD.H.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWOD.W.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWOD.D.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000100110111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWOD.Q.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWEV.H.BU.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWEV.W.HU.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWEV.D.WU.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWEV.Q.DU.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWOD.H.BU.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWOD.W.HU.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWOD.D.WU.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMULWOD.Q.DU.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADD.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADD.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADD.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADD.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMSUB.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMSUB.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMSUB.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMSUB.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWEV.H.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWEV.W.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWEV.D.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101011011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWEV.Q.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101011100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWOD.H.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101011101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWOD.W.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101011110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWOD.D.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101011111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWOD.Q.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWEV.H.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWEV.W.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWEV.D.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWEV.Q.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWOD.H.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWOD.W.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWOD.D.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWOD.Q.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101111000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWEV.H.BU.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWEV.W.HU.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101111010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWEV.D.WU.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101111011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWEV.Q.DU.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101111100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWOD.H.BU.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWOD.W.HU.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWOD.D.WU.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000101111111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMADDWOD.Q.DU.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VDIV.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VDIV.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VDIV.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VDIV.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VDIV.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VDIV.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VDIV.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VDIV.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMOD.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMOD.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMOD.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMOD.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMOD.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMOD.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMOD.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VMOD.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001011100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSIGNCOV.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001011101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSIGNCOV.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001011110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSIGNCOV.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001011111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSIGNCOV.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VAND.V", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VOR.V", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VXOR.V", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VNOR.V", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VANDN.V", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VORN.V", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLL.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLL.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLL.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLL.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRL.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRL.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRL.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRL.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRA.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRA.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRA.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111011011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRA.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111011100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VROTR.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111011101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VROTR.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111011110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VROTR.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111011111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VROTR.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRLR.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRLR.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRLR.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRLR.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRAR.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRAR.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRAR.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111100111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRAR.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRLN.B.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRLN.H.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRLN.W.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRAN.B.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRAN.H.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRAN.W.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRLRN.B.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111110010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRLRN.H.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111110011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRLRN.W.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRARN.B.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRARN.H.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111110111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSRARN.W.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRLN.B.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111111010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRLN.H.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111111011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRLN.W.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRAN.B.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRAN.H.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000111111111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRAN.W.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRLN.BU.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRLN.HU.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRLN.WU.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRAN.BU.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRAN.HU.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRAN.WU.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRLRN.B.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRLRN.H.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRLRN.W.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRARN.B.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRARN.H.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRARN.W.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRLRN.BU.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRLRN.HU.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRLRN.WU.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRARN.BU.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRARN.HU.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSSRARN.WU.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VBITCLR.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VBITCLR.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VBITCLR.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000011011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VBITCLR.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000011100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VBITSET.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000011101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VBITSET.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000011110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VBITSET.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000011111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VBITSET.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VBITREV.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VBITREV.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VBITREV.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VBITREV.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFRSTP.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFRSTP.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFADD.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFADD.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFSUB.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFSUB.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFMUL.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001110010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFMUL.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFDIV.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFDIV.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFMAX.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001111010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFMAX.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFMIN.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFMIN.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001010000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFMAXA.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001010000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFMAXA.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001010000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFMINA.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001010000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFMINA.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001010001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCVT.H.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001010001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCVT.S.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001010010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFFINT.S.L", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSEQ.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSEQ.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSEQ.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSEQ.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLE.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLE.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLE.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLE.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLE.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLE.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLE.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLE.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLT.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLT.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLT.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLT.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLT.BU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLT.HU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLT.WU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110000000010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSLT.DU", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPACKEV.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPACKEV.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPACKEV.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPACKEV.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000110000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPACKOD.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPACKOD.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000110010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPACKOD.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000110011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPACKOD.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000111100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPICKEV.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPICKEV.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPICKEV.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000111111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPICKEV.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPICKOD.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPICKOD.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPICKOD.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001001000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VPICKOD.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000110100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VILVL.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VILVL.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VILVL.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000110111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VILVL.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000111000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VILVH.B", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VILVH.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000111010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VILVH.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001000111011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VILVH.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001011110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSHUF.H", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001011110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSHUF.W", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110001011110111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VSHUF.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110011100100iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "VSHUF4I.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011100101iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "VSHUF4I.H", Vt[Rd], Vt[Rj], imm);

        return buff;
    }
    if (isMask(opcode, "01110011100110iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "VSHUF4I.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011100111iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "VSHUF4I.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011100000iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "VEXTRINS.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011100001iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "VEXTRINS.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011100010iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "VEXTRINS.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011100011iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%x", "VEXTRINS.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0010110000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "VLD", Vt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010110001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "VST", Vt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "00000000000000001000000000101000", &a)) {
        snprintf(buff, sizeof(buff), "X64CLRSM");
        return buff;
    }
    if (isMask(opcode, "00000000000000001000000000001000", &a)) {
        snprintf(buff, sizeof(buff), "X64SETSM");
        return buff;
    }
    if (isMask(opcode, "00000000000000001000000000001001", &a)) {
        snprintf(buff, sizeof(buff), "X64INCTOP");
        return buff;
    }
    if (isMask(opcode, "00000000000000001000000000101001", &a)) {
        snprintf(buff, sizeof(buff), "X64DECTOP");
        return buff;
    }
    if (isMask(opcode, "000000000000000001110000iii00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %d", "X64SETTOP", imm);
        return buff;
    }
    if (isMask(opcode, "000000000000000001110100000ddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64GETTOP", Xt[Rd]);
        return buff;
    }
    if (isMask(opcode, "00000000010111iiiiiiii00000ddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%x", "X64GETEFLAGS", Xt[Rd], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010111iiiiiiii00001ddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%x", "X64SETEFLAGS", Xt[Rd], imm);
        return buff;
    }
    if (isMask(opcode, "000000000011011010iiii00000ddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%x", "X64SETJ", Xt[Rd], imm);
        return buff;
    }
    if (isMask(opcode, "0000000000000000100000jjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64INC.B", Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000100000jjjjj00001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64INC.H", Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000100000jjjjj00010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64INC.W", Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000100000jjjjj00011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64INC.D", Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000100000jjjjj00100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64DEC.B", Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000100000jjjjj00101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64DEC.H", Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000100000jjjjj00110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64DEC.W", Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000000000000100000jjjjj00111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64DEC.D", Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADD.WU", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj00001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADD.DU", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj00010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SUB.WU", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj00011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SUB.DU", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj00100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADD.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj00101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADD.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj00110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADD.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj00111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADD.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj01000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SUB.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj01001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SUB.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj01010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SUB.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj01011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SUB.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj01100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADC.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj01101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADC.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj01110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADC.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj01111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ADC.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj10000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SBC.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj10001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SBC.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj10010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SBC.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj10011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SBC.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj10100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SLL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj10101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SLL.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj10110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SLL.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj10111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SLL.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj11000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj11001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRL.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj11010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRL.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj11011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRL.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj11100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRA.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj11101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRA.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj11110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRA.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111110kkkkkjjjjj11111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64SRA.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTR.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj00001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTR.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj00010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTR.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj00011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTR.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj00100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj00101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTL.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj00110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTL.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj00111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64ROTL.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj01000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCR.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj01001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCR.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj01010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCR.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj01011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCR.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj01100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCL.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj01101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCL.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj01110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCL.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj01111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64RCL.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj10000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64AND.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj10001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64AND.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj10010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64AND.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj10011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64AND.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj10100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64OR.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj10101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64OR.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj10110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64OR.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj10111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64OR.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj11000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64XOR.B", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj11001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64XOR.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj11010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64XOR.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111111kkkkkjjjjj11011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64XOR.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SLLI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj00100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRLI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj01000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRAI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj01100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTRI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj10000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCRI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj10100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTLI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj11000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCLI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj00001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SLLI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj00101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRLI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj01001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRAI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj01101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTRI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj10001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCRI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj10101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTLI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj11001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCLI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj00010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SLLI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj00110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRLI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj01010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRAI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj01110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTRI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj10010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCRI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj10110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTLI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj11010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCLI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj00011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SLLI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj00111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRLI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj01011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64SRAI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj01111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTRI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj10011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCRI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj10111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64ROTLI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj11011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %d", "X64RCLI.D", Xt[Rj], imm);
        return buff;
    }

    snprintf(buff, sizeof(buff), "%08X ???", __builtin_bswap32(opcode));
    return buff;
}