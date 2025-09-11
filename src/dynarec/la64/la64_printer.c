#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "la64_printer.h"
#include "debug.h"

static const char* Xt[] = { "xZR", "r1", "r2", "sp", "xRDI_r4", "xRSI_r5", "xRDX_r6", "xRCX_r7", "xR8_r8", "xR9_r9", "xRBX_r10", "xRSP_r11", "xRAX_r12", "x1_r13", "x2_r14", "x3_r15", "x4_r16", "x5_r17", "x6_r18", "r19", "x7_r20", "r21", "xRBP_r22", "xR10_r23", "xR11_r24", "xR12_r25", "xR13_r26", "xR14_r27", "xR15_r28", "xRIP_r29", "xFlags_r30", "xEmu_r31" };
static const char* Ft[] = { "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7", "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", "ft8", "ft9", "ft10", "ft11", "ft12", "ft13", "ft14", "ft15", "fs0", "fs1", "fs2", "fs3", "fs4", "fs5", "fs6", "fs7" };
static const char* Vt[] = { "vra0", "vra1", "vra2", "vra3", "vra4", "vra5", "vra6", "vra7", "vrt0", "vrt1", "vrt2", "vrt3", "vrt4", "vrt5", "vrt6", "vrt7", "vrt8", "vrt9", "vrt10", "vrt11", "vrt12", "vrt13", "vrt14", "vrt15", "vrs0", "vrs1", "vrs2", "vrs3", "vrs4", "vrs5", "vrs6", "vrs7" };
static const char* XVt[] = { "xvra0", "xvra1", "xvra2", "xvra3", "xvra4", "xvra5", "xvra6", "xvra7", "xvrt0", "xvrt1", "xvrt2", "xvrt3", "xvrt4", "xvrt5", "xvrt6", "xvrt7", "xvrt8", "xvrt9", "xvrt10", "xvrt11", "xvrt12", "xvrt13", "xvrt14", "xvrt15", "xvrs0", "xvrs1", "xvrs2", "xvrs3", "xvrs4", "xvrs5", "xvrs6", "xvrs7" };
static const char* FCCt[] = { "fcc0", "fcc1", "fcc2", "fcc3", "fcc4", "fcc5", "fcc6", "fcc7" };
static const char* FCSRt[] = { "fcsr0_ALL", "fcsr1_Enables", "fcsr2_Cause_Flag", "fcsr3_RM" };

typedef struct la64_print_s {
    int d, j, k, a;
    uint64_t i, u, c;
} la64_print_t;

int isMask(uint32_t opcode, const char* mask, la64_print_t* a)
{
    if (strlen(mask) != 32) {
        printf_log(LOG_NONE, "Error: printer mask \"%s\" in not len 32 but %ld\n", mask, strlen(mask));
        return 0;
    }
    memset(a, 0, sizeof(*a));
    int i = 31;
    while (*mask) {
        uint8_t v = (opcode >> i) & 1;
        switch (*mask) {
            case '0':
                if (v != 0) return 0;
                break;
            case '1':
                if (v != 1) return 0;
                break;
            case 'd': a->d = (a->d << 1) | v; break;
            case 'j': a->j = (a->j << 1) | v; break;
            case 'k': a->k = (a->k << 1) | v; break;
            case 'a': a->a = (a->a << 1) | v; break;
            case 'i': a->i = (a->i << 1) | v; break;
            case 'u': a->u = (a->u << 1) | v; break;
            case 'c': a->c = (a->c << 1) | v; break;
            default:
                printf_log(LOG_NONE, "Warning, printer mask use unhandled '%c'\n", *mask);
        }
        mask++;
        --i;
    }

    return 1;
}

int64_t signExtend(uint64_t val, int sz)
{
    int64_t ret = val;
    if ((val >> (sz - 1)) & 1)
        ret |= (0xffffffffffffffffll << sz);
    return ret;
}

const char* la64_print(uint32_t opcode, uintptr_t addr)
{
    static char buff[200];
    la64_print_t a;
#define Rd     a.d
#define Rj     a.j
#define Rk     a.k
#define Ra     a.a
#define Rc     a.c
#define imm    a.i
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
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "ADDI.W", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0000001011iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "ADDI.D", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "000100iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "ADDU16I.D", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "000000000000010iikkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %lu", "ALSL.W", Xt[Rd], Xt[Rj], Xt[Rk], imm);
        return buff;
    }
    if (isMask(opcode, "000000000000011iikkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %lu", "ALSL.WU", Xt[Rd], Xt[Rj], Xt[Rk], imm);
        return buff;
    }
    if (isMask(opcode, "000000000010110iikkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %lu", "ALSL.D", Xt[Rd], Xt[Rj], Xt[Rk], imm);
        return buff;
    }
    if (isMask(opcode, "0001010iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "LU12I.W", Xt[Rd], imm);
        return buff;
    }
    if (isMask(opcode, "0001011iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "LU32I.D", Xt[Rd], imm);
        return buff;
    }
    if (isMask(opcode, "0000001100iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu", "LU52I.D", Xt[Rd], Xt[Rj], imm);
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
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "SLTI", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0000001001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "SLTI", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0001100iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "PCADDI", Xt[Rd], (uint64_t)signExtend(imm << 2, 22));
        return buff;
    }
    if (isMask(opcode, "0001101iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "PCALAU12I", Xt[Rd], (uint64_t)signExtend(imm << 12, 32));
        return buff;
    }
    if (isMask(opcode, "0001110iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "PCADDU12I", Xt[Rd], (uint64_t)signExtend(imm << 12, 32));
        return buff;
    }
    if (isMask(opcode, "0001111iiiiiiiiiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "PCADDU18I", Xt[Rd], (uint64_t)signExtend(imm << 18, 38));
        return buff;
    }
    if (isMask(opcode, "00100000iiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "LL.W", Xt[Rd], Xt[Rj], signExtend(imm << 2, 16));
        return buff;
    }
    if (isMask(opcode, "00100001iiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "SC.W", Xt[Rd], Xt[Rj], signExtend(imm << 2, 16));
        return buff;
    }
    if (isMask(opcode, "00100010iiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "LL.D", Xt[Rd], Xt[Rj], signExtend(imm << 2, 16));
        return buff;
    }
    if (isMask(opcode, "00100011iiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "SC.D", Xt[Rd], Xt[Rj], signExtend(imm << 2, 16));
        return buff;
    }
    if (isMask(opcode, "00111000011100100iiiiiiiiiiiiiii", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %lu", "DBAR", imm);
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
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "ANDI", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000001110iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "ORI", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000001111iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XORI", Xt[Rd], Xt[Rj], imm);
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
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu", "SLLI.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001000101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu", "SRLI.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001001001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu", "SRAI.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001001101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu", "ROTRI.D", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu", "SLLI.W", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu", "SRLI.W", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu", "SRAI.W", Xt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu", "ROTRI.W", Xt[Rd], Xt[Rj], imm);
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
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu, %lu", "BSTRINS.W", Xt[Rd], Xt[Rj], imm_up, imm);
        return buff;
    }
    if (isMask(opcode, "0000000010uuuuuuiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu, %lu", "BSTRINS.D", Xt[Rd], Xt[Rj], imm_up, imm);
        return buff;
    }
    if (isMask(opcode, "00000000011uuuuu1iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu, %lu", "BSTRPICK.W", Xt[Rd], Xt[Rj], imm_up, imm);
        return buff;
    }
    if (isMask(opcode, "0000000011uuuuuuiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %lu, %lu", "BSTRPICK.D", Xt[Rd], Xt[Rj], imm_up, imm);
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
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "BEQ", Xt[Rj], Xt[Rd], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "010111iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "BNE", Xt[Rj], Xt[Rd], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "011000iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "BLT", Xt[Rj], Xt[Rd], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "011001iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "BGE", Xt[Rj], Xt[Rd], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "011010iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "BLTU", Xt[Rj], Xt[Rd], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "011011iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "BGEU", Xt[Rj], Xt[Rd], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "010000iiiiiiiiiiiiiiiijjjjjuuuuu", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %ld", "BEQZ", Xt[Rj], signExtend((imm + (imm_up << 16)) << 2, 23));
        return buff;
    }
    if (isMask(opcode, "010001iiiiiiiiiiiiiiiijjjjjuuuuu", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %ld", "BNEZ", Xt[Rj], signExtend((imm + (imm_up << 16)) << 2, 23));
        return buff;
    }
    if (isMask(opcode, "0100110000000000000000jjjjj00000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "BR", Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "010011iiiiiiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "JIRL", Xt[Rd], Xt[Rj], signExtend(imm << 2, 18));
        return buff;
    }
    if (isMask(opcode, "010100iiiiiiiiiiiiiiiiiiiiiiiiii", &a)) {
        snprintf(buff, sizeof(buff), "%-15s 0x%lx", "B", (((imm & 0x3FF) << 16) | (imm >> 10)) << 6 >> 4);
        return buff;
    }
    if (isMask(opcode, "0010100000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "LD.B", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "LD.H", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100010iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "LD.W", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100011iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "LD.D", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010101000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "LD.BU", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010101001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "LD.HU", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010101010iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "LD.WU", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100100iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "ST.B", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100101iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "ST.H", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100110iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "ST.W", Xt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010100111iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "ST.D", Xt[Rd], Xt[Rj], signExtend(imm, 12));
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
    if (isMask(opcode, "00000000001001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "CRC.W.B.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "CRC.W.H.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "CRC.W.W.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "CRC.W.D.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "CRCC.W.B.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "CRCC.W.H.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "CRCC.W.W.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "CRCC.W.D.W", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "0010101110iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "FLD.D", Ft[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010101100iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "FLD.S", Ft[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010101111iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "FST.D", Ft[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010101101iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "FST.S", Ft[Rd], Xt[Rj], signExtend(imm, 12));
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
    if (isMask(opcode, "0000000100010100100101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FMOV.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100100110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FMOV.D", Ft[Rd], Ft[Rj]);
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
    if (isMask(opcode, "0000000100010100110000jjjjj000cc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVGR2FCSR", FCSRt[Rc], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100110010000ccddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVFCSR2GR", Xt[Rd], FCSRt[Rc]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100110100jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVFR2CF", FCCt[Rc], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "000000010001010011010100cccddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVCF2FR", Ft[Rd], FCCt[Rc]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100110110jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVGR2CF", FCCt[Rc], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "000000010001010011011100cccddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVCF2GR", Xt[Rd], FCCt[Rc]);
        return buff;
    }
    if (isMask(opcode, "0000000100011001001001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FCVT.D.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011001000110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FCVT.S.D", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011010000001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FTINTRM.W.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011010001001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FTINTRM.L.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011010010001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FTINTRP.W.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011010011001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FTINTRP.L.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011010100001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FTINTRZ.W.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011010101001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FTINTRZ.L.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011010110001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FTINTRNE.W.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011010111001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FTINTRNE.L.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011011000001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FTINT.W.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011011001001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FTINT.L.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011101000100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FFINT.S.W", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011101000110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FFINT.S.L", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100011110010001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FRINT.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "010010iiiiiiiiiiiiiiii00cccuuuuu", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %ld", "BCEQZ", FCCt[Rc], signExtend((imm + (imm_up << 16)) << 2, 23));
        return buff;
    }
    if (isMask(opcode, "010010iiiiiiiiiiiiiiii01cccuuuuu", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %ld", "BCNEZ", FCCt[Rc], signExtend((imm + (imm_up << 16)) << 2, 23));
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
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSHUF4I.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011100101iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSHUF4I.H", Vt[Rd], Vt[Rj], imm);

        return buff;
    }
    if (isMask(opcode, "01110011100110iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSHUF4I.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011100111iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSHUF4I.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011100000iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VEXTRINS.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011100001iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VEXTRINS.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011100010iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VEXTRINS.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011100011iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VEXTRINS.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101110101110iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VINSGR2VR.B", Vt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001011101011110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VINSGR2VR.H", Vt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010111010111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VINSGR2VR.W", Vt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101110101111110ijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VINSGR2VR.D", Vt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101110111110iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.B", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001011101111110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.H", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010111011111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.W", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101110111111110ijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.D", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101111001110iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.BU", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001011110011110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.HU", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010111100111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.WU", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101111001111110ijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.DU", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101111011110iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VREPLVEI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001011110111110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VREPLVEI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010111101111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VREPLVEI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101111011111110ijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VREPLVEI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100001000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLWIL.H.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110000100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLWIL.W.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011000010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLWIL.D.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100001001000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXTL.Q.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001100001100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLWIL.HU.BU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110000110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLWIL.WU.HU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011000011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLWIL.DU.WU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100001101000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXTL.QU.DU", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001100010000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITCLRI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110001000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITCLRI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011000100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITCLRI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100010001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITCLRI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100010100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITSETI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110001010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITSETI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011000101001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITSETI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100010101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITSETI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100011000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITREVI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110001100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITREVI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011000110001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITREVI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100011001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITREVI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100100100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110010010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011001001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100100101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100101000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.BU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110010100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.HU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011001010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.WU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100101001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.DU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100101100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110010110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011001011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100101101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100110000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110011000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011001100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100110001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100110100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRAI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110011010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRAI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011001101001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRAI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100110101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRAI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLNI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLNI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101000001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLNI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100001iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLNI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101001001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100101iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLRNI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLRNI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101000101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLRNI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100011iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLRNI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.BU.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.HU.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101001101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.WU.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100111iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.DU.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101010001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101001iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.BU.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010101001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.HU.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101010101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.WU.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101011iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.DU.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRANI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010110001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRANI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101011001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRANI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101101iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRANI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRARNI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010111001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRARNI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101011101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRARNI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101111iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRARNI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011011000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101100001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110001iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.BU.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011011001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.HU.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101100101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.WU.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110011iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.DU.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011011010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101101001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110101iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.BU.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011011011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.HU.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101101101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.WU.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110111iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.DU.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011111001iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPERMI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0010110000iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "VLD", Vt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010110001iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "VST", Vt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "000011010001aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "VBITSEL.V", Vt[Rd], Vt[Rj], Vt[Rk], Vt[Ra]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101001101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRINT.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101001110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRINT.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101010001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRINTRM.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101010010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRINTRM.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101010101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRINTRP.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101010110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRINTRP.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101011001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRINTRZ.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101011010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRINTRZ.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101011101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRINTRNE.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101011110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRINTRNE.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101111010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFCVTL.S.H", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101111011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFCVTH.S.H", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101111100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFCVTL.D.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101111101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFCVTH.D.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFFINT.S.W", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110000001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFFINT.S.WU", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110000010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFFINT.D.L", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110000011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFFINT.D.LU", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110000100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFFINTL.D.W", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110000101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFFINTH.D.W", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110001100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINT.W.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110001101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINT.L.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110001110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRM.W.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110001111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRM.L.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110010000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRP.W.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110010001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRP.L.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110010010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRZ.W.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110010011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRZ.L.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110010100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRNE.W.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110010101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRNE.L.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110010110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINT.WU.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110010111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINT.LU.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110011100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRZ.WU.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110011101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRZ.LU.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110100000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTL.L.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110100001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTH.L.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110100010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRML.L.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110100011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRMH.L.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110100100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRPL.L.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110100101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRPH.L.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110100110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRZL.L.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110100111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRZH.L.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110101000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRNEL.L.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110101001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFTINTRNEH.L.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110111000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXTH.H.B", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110111001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXTH.W.H", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110111010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXTH.D.W", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110111011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXTH.Q.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110111100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXTH.HU.BU", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110111101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXTH.WU.HU", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110111110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXTH.DU.WU", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011110111111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXTH.QU.DU", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011111000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VREPLGR2VR.B", Vt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011111000001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VREPLGR2VR.H", Vt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011111000010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VREPLGR2VR.W", Vt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011111000011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VREPLGR2VR.D", Vt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010100000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VROTRI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101010000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VROTRI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010101000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VROTRI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001010100001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VROTRI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001010100100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLRI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101010010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLRI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010101001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLRI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001010100101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLRI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001010101000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRARI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101010100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRARI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010101010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRARI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001010101001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRARI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101110101110iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VINSGR2VR.B", Vt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001011101011110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VINSGR2VR.H", Vt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010111010111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VINSGR2VR.W", Vt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101110101111110ijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VINSGR2VR.D", Vt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101110111110iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.B", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001011101111110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.H", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010111011111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.W", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101110111111110ijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.D", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101111001110iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.BU", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001011110011110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.HU", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010111100111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.WU", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101111001111110ijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPICKVE2GR.DU", Xt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101111011110iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VREPLVEI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001011110111110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VREPLVEI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010111101111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VREPLVEI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100101111011111110ijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VREPLVEI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100001000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLWIL.H.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110000100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLWIL.W.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011000010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLWIL.D.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100001001000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXTL.Q.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001100001100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLWIL.HU.BU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110000110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLWIL.WU.HU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011000011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLWIL.DU.WU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100001101000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXTL.QU.DU", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001100010000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITCLRI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110001000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITCLRI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011000100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITCLRI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100010001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITCLRI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100010100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITSETI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110001010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITSETI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011000101001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITSETI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100010101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITSETI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100011000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITREVI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110001100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITREVI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011000110001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITREVI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100011001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITREVI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100100100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110010010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011001001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100100101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100101000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.BU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110010100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.HU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011001010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.WU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100101001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSAT.DU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100101100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110010110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011001011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100101101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSLLI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100110000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110011000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011001100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100110001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100110100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRAI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110011010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRAI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011001101001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRAI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100110101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRAI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLNI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLNI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101000001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLNI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100001iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLNI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101001001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100101iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLRNI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLRNI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101000101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLRNI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100011iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRLRNI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.BU.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.HU.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101001101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.WU.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110100111iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLNI.DU.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101010001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101001iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.BU.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010101001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.HU.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101010101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.WU.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101011iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRLRNI.DU.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRANI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010110001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRANI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101011001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRANI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101101iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRANI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRARNI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011010111001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRARNI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101011101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRARNI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110101111iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSRARNI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011011000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101100001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110001iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.BU.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011011001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.HU.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101100101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.WU.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110011iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRANI.DU.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.B.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011011010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.H.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101101001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.W.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110101iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.D.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.BU.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011011011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.HU.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001101101101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.WU.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110110111iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VSSRARNI.DU.Q", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011111001iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VPERMI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00001100010100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CAF.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SAF.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CLT.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SLT.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CEQ.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SEQ.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CLE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SLE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CUN.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SUN.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CULT.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SULT.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CUEQ.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SUEQ.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CULE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SULE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010110000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CNE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SNE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010110100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.COR.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SOR.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010111000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CUNE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SUNE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "0010110010iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "XVLD", XVt[Rd], Xt[Rj], signExtend(imm, 12));
        return buff;
    }
    if (isMask(opcode, "0010110011iiiiiiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %ld", "XVST", XVt[Rd], Xt[Rj], signExtend(imm, 12));
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
        snprintf(buff, sizeof(buff), "%-15s %lu", "X64SETTOP", imm);
        return buff;
    }
    if (isMask(opcode, "000000000000000001110100000ddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s", "X64GETTOP", Xt[Rd]);
        return buff;
    }
    if (isMask(opcode, "00000000010111iiiiiiii00000ddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "X64GETEFLAGS", Xt[Rd], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010111iiiiiiii00001ddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "X64SETEFLAGS", Xt[Rd], imm);
        return buff;
    }
    if (isMask(opcode, "000000000011011010iiii00000ddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "X64SETJ", Xt[Rd], imm);
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
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.H", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.W", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.D", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.BU", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.HU", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.WU", Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00000000001111101kkkkkjjjjj00111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "X64MUL.DU", Xt[Rj], Xt[Rk]);
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
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64SLLI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj00100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64SRLI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj01000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64SRAI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj01100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64ROTRI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj10000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64RCRI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj10100", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64ROTLI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010100001iiijjjjj11000", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64RCLI.B", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj00001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64SLLI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj00101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64SRLI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj01001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64SRAI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj01101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64ROTRI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj10001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64RCRI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj10101", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64ROTLI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000000000101010001iiiijjjjj11001", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64RCLI.H", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj00010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64SLLI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj00110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64SRLI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj01010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64SRAI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj01110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64ROTRI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj10010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64RCRI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj10110", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64ROTLI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "00000000010101001iiiiijjjjj11010", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64RCLI.W", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj00011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64SLLI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj00111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64SRLI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj01011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64SRAI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj01111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64ROTRI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj10011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64RCRI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj10111", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64ROTLI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000001010101iiiiiijjjjj11011", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %lu", "X64RCLI.D", Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0000000100010100111000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FCVT.LD.D", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100111001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FCVT.UD.D", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "00000001000101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s,", "FCVT.D.LD", Ft[Rd], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100110000jjjjj000cc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVGR2FCSR", FCSRt[Rc], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100110010000ccddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVFCSR2GR", Xt[Rd], FCSRt[Rc]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100110100jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVFR2CF", FCCt[Rc], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "000000010001010011010100cccddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVCF2FR", Ft[Rd], FCCt[Rc]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100110110jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVGR2CF", FCCt[Rc], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "000000010001010011011100cccddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "MOVCF2GR", Xt[Rd], FCCt[Rc]);
        return buff;
    }
    if (isMask(opcode, "010010iiiiiiiiiiiiiiii00cccuuuuu", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %ld", "BCEQZ", FCCt[Rc], signExtend((imm + (imm_up << 16)) << 2, 23));
        return buff;
    }
    if (isMask(opcode, "010010iiiiiiiiiiiiiiii01cccuuuuu", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %ld", "BCNEZ", FCCt[Rc], signExtend((imm + (imm_up << 16)) << 2, 23));
        return buff;
    }
    if (isMask(opcode, "00001100000100000kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CAF.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000100001kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SAF.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000100010kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CLT.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000100011kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SLT.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000100011kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SGT.S", FCCt[Rc], Ft[Rk], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "00001100000100100kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CEQ.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000100101kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SEQ.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000100110kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CLE.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000100111kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SLE.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000100111kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SGE.S", FCCt[Rc], Ft[Rk], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "00001100000101000kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CUN.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000101001kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SUN.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000101010kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CULT.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000101010kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CUGT.S", FCCt[Rc], Ft[Rk], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "00001100000101011kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SULT.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000101100kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CUEQ.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000101101kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SUEQ.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000101110kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CULE.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000101110kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CUGE.S", FCCt[Rc], Ft[Rk], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "00001100000101111kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SULE.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000110000kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CNE.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000110001kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SNE.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000110100kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.COR.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000110101kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SOR.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000111000kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CUNE.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100000111001kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SUNE.S", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001000000kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CAF.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001000001kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SAF.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001000010kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CLT.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001000011kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SLT.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001000011kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SGT.D", FCCt[Rc], Ft[Rk], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "00001100001000100kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CEQ.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001000101kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SEQ.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001000110kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CLE.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001000111kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SLE.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001000111kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SGE.D", FCCt[Rc], Ft[Rk], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "00001100001001000kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CUN.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001001001kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SUN.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001001010kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CULT.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001001010kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CUGT.D", FCCt[Rc], Ft[Rk], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "00001100001001011kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SULT.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001001100kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CUEQ.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001001101kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SUEQ.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001001110kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CULE.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001001110kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CUGE.D", FCCt[Rc], Ft[Rk], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "00001100001001111kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SULE.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001010000kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CNE.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001010001kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SNE.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001010100kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.COR.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001010101kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SOR.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001011000kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.CUNE.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100001011001kkkkkjjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "FCMP.SUNE.D", FCCt[Rc], Ft[Rj], Ft[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CAF.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SAF.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CLT.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SLT.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CEQ.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SEQ.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CLE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010100111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SLE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CUN.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SUN.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CULT.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SULT.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CUEQ.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SUEQ.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CULE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SULE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010110000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CNE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SNE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010110100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.COR.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SOR.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010111000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CUNE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100010111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SUNE.S", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CAF.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SAF.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CLT.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SLT.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CEQ.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SEQ.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CLE.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100100111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SLE.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CUN.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SUN.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CULT.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SULT.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CUEQ.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SUEQ.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CULE.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SULE.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100110000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CNE.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SNE.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100110100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.COR.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SOR.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100111000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CUNE.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100100111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SUNE.S", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CAF.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SAF.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CLT.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SLT.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CEQ.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SEQ.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CLE.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SLE.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CUN.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SUN.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CULT.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SULT.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CUEQ.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SUEQ.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CULE.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SULE.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CNE.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SNE.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.COR.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SOR.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.CUNE.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100011011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "VFCMP.SUNE.D", Vt[Rd], Vt[Rj], Vt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CAF.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SAF.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CLT.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SLT.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CEQ.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SEQ.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CLE.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SLE.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CUN.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SUN.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CULT.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SULT.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CUEQ.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SUEQ.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CULE.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SULE.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CNE.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SNE.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.COR.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SOR.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.CUNE.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "00001100101011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCMP.SUNE.D", Xt[Rd], Xt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSEQ.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSEQ.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSEQ.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSEQ.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLE.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLE.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLE.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLE.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLE.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLE.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLE.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLE.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLT.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLT.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLT.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLT.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLT.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLT.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLT.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLT.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADD.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADD.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADD.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADD.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUB.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUB.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUB.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000011011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUB.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSADD.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSADD.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSADD.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSADD.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSUB.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSUB.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSUB.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSUB.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSADD.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSADD.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSADD.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSADD.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSUB.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSUB.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSUB.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010011011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSUB.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHADDW.H.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHADDW.W.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHADDW.D.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHADDW.Q.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHSUBW.H.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHSUBW.W.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHSUBW.D.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHSUBW.Q.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010110000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHADDW.HU.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHADDW.WU.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010110010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHADDW.DU.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010110011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHADDW.QU.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010110100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHSUBW.HU.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHSUBW.WU.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHSUBW.DU.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010110111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVHSUBW.QU.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000111100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWEV.H.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWEV.W.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWEV.D.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100000111111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWEV.Q.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001011100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWEV.H.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001011101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWEV.W.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001011110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWEV.D.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001011111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWEV.Q.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001111100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWEV.H.BU.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWEV.W.HU.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWEV.D.WU.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001111111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWEV.Q.DU.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWOD.H.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWOD.W.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWOD.D.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWOD.Q.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWOD.H.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWOD.W.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWOD.D.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001100111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWOD.Q.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWOD.H.BU.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWOD.W.HU.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWOD.D.WU.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDWOD.Q.DU.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWEV.H.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWEV.W.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWEV.D.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101011011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWEV.Q.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101111000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWEV.H.BU.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWEV.W.HU.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101111010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWEV.D.WU.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101111011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWEV.Q.DU.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWEV.H.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWEV.W.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWEV.D.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWEV.Q.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101011100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWOD.H.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101011101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWOD.W.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101011110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWOD.D.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101011111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWOD.Q.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWOD.H.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWOD.W.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWOD.D.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWOD.Q.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101111100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWOD.H.BU.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWOD.W.HU.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWOD.D.WU.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101111111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADDWOD.Q.DU.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWEV.H.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWEV.W.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWEV.D.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWEV.Q.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100110000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWEV.H.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWEV.W.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100110010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWEV.D.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100110011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWEV.Q.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWEV.H.BU.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWEV.W.HU.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWEV.D.WU.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWEV.Q.DU.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWOD.H.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWOD.W.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWOD.D.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100100111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWOD.Q.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100110100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWOD.H.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWOD.W.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWOD.D.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100110111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWOD.Q.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWOD.H.BU.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWOD.W.HU.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWOD.D.WU.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMULWOD.Q.DU.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWEV.H.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWEV.W.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWEV.D.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWEV.Q.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWEV.H.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWEV.W.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWEV.D.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWEV.Q.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWOD.H.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWOD.W.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWOD.D.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWOD.Q.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWOD.H.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWOD.W.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWOD.D.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100001101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUBWOD.Q.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010111000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDA.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDA.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010111010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDA.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100010111011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADDA.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVABSD.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVABSD.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVABSD.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVABSD.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVABSD.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVABSD.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVABSD.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVABSD.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVG.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVG.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVG.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVG.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVG.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVG.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVG.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVG.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVGR.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVGR.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVGR.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVGR.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVGR.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVGR.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVGR.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAVGR.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMAX.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMAX.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMAX.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMAX.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMIN.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMIN.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMIN.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011100111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMIN.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011101000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMAX.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMAX.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMAX.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMAX.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMIN.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMIN.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMIN.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100011101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMIN.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMUL.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMUL.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMUL.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMUL.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMUH.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMUH.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMUH.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMUH.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMUH.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMUH.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMUH.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100100010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMUH.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADD.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADD.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADD.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMADD.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMSUB.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMSUB.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMSUB.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100101010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMSUB.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVDIV.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVDIV.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVDIV.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVDIV.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMOD.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMOD.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMOD.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMOD.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111001000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVDIV.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVDIV.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVDIV.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVDIV.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMOD.BU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMOD.HU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMOD.WU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVMOD.DU", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLL.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLL.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLL.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSLL.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRL.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRL.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRL.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRL.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRA.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRA.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRA.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111011011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRA.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111011100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVROTR.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111011101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVROTR.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111011110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVROTR.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111011111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVROTR.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRLR.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRLR.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRLR.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRLR.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111100100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRAR.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRAR.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRAR.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111100111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRAR.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111101001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRLN.B.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111101010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRLN.H.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111101011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRLN.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRAN.B.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRAN.H.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRAN.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRLRN.B.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111110010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRLRN.H.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111110011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRLRN.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRARN.B.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRARN.H.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111110111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSRARN.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRLN.B.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111111010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRLN.H.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111111011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRLN.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRAN.B.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRAN.H.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110100111111111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRAN.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRLRN.B.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRLRN.H.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRLRN.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRARN.B.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRARN.H.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRARN.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000001001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRLN.BU.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000001010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRLN.HU.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000001011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRLN.WU.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRAN.BU.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRAN.HU.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRAN.WU.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRLRN.BU.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000010010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRLRN.HU.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRLRN.WU.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRARN.BU.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRARN.HU.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSSRARN.WU.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000011000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVBITCLR.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000011001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVBITCLR.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVBITCLR.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000011011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVBITCLR.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000011100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVBITSET.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000011101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVBITSET.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000011110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVBITSET.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000011111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVBITSET.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000100000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVBITREV.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVBITREV.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVBITREV.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000100011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVBITREV.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000101100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPACKEV.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000101101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPACKEV.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000101110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPACKEV.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000101111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPACKEV.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000110000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPACKOD.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPACKOD.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000110010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPACKOD.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000110011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPACKOD.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000110100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVILVL.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVILVL.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVILVL.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000110111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVILVL.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000111000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVILVH.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVILVH.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000111010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVILVH.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000111011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVILVH.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000111100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPICKEV.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPICKEV.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPICKEV.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101000111111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPICKEV.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001000000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPICKOD.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPICKOD.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPICKOD.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001000011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPICKOD.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVREPLVE.B", XVt[Rd], XVt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVREPLVE.H", XVt[Rd], XVt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVREPLVE.W", XVt[Rd], XVt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVREPLVE.D", XVt[Rd], XVt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVAND.V", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVOR.V", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001001110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVXOR.V", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001001111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVNOR.V", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVANDN.V", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001010001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVORN.V", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFRSTP.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFRSTP.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001011010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVADD.Q", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001011011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSUB.Q", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001011100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSIGNCOV.B", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001011101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSIGNCOV.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001011110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSIGNCOV.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001011111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSIGNCOV.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001100001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFADD.S", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001100010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFADD.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001100101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFSUB.S", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001100110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFSUB.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001110001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFMUL.S", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001110010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFMUL.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFDIV.S", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFDIV.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001111001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFMAX.S", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001111010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFMAX.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001111101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFMIN.S", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001111110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFMIN.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101010000001kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFMAXA.S", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101010000010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFMAXA.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101010000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFMINA.S", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101010000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFMINA.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101010001100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCVT.H.S", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101010001101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFCVT.S.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101010010000kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFFINT.S.L", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101010010011kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFTINT.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101010010100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFTINTRM.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101010010101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFTINTRP.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101010010110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFTINTRZ.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101010010111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVFTINTRNE.W.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101011110101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSHUF.H", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101011110110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSHUF.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101011110111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVSHUF.D", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101011111010kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVPERM.W", XVt[Rd], XVt[Rj], XVt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110110100000000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSEQI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSEQI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSEQI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSEQI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000110iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000111iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001110iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001111iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVADDI.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVADDI.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010110iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVADDI.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010111iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVADDI.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100011000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSUBI.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSUBI.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100011010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSUBI.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100011011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSUBI.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100011100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBSLL.V", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100011101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBSRL.V", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100110iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100111iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101110iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101111iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100110100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVFRSTPI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100110101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVFRSTPI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011010011100000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVCLO.B", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100000001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVCLO.H", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100000010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVCLO.W", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100000011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVCLO.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100000100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVCLZ.B", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100000101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVCLZ.H", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100000110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVCLZ.W", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100000111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVCLZ.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100001000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVPCNT.B", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100001001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVPCNT.H", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100001010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVPCNT.W", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100001011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVPCNT.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100001100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVNEG.B", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100001101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVNEG.H", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100001110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVNEG.W", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100001111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVNEG.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100010000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVMSKLTZ.B", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100010001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVMSKLTZ.H", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100010010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVMSKLTZ.W", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100010011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVMSKLTZ.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100010100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVMSKGEZ.B", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100011000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVMSKNZ.B", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100100110jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVSETEQZ.V", FCCt[Rc], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100100111jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVSETNEZ.V", FCCt[Rc], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100101000jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVSETANYEQZ.B", FCCt[Rc], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100101001jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVSETANYEQZ.H", FCCt[Rc], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100101010jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVSETANYEQZ.W", FCCt[Rc], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100101011jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVSETANYEQZ.D", FCCt[Rc], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100101100jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVSETALLNEZ.B", FCCt[Rc], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100101101jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVSETALLNEZ.H", FCCt[Rc], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100101110jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVSETALLNEZ.W", FCCt[Rc], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100101111jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVSETALLNEZ.D", FCCt[Rc], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100110001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFLOGB.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100110010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFLOGB.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100110101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFCLASS.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100110110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFCLASS.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100111001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFSQRT.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100111010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFSQRT.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100111101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRECIP.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011100111110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRECIP.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101000001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRSQRT.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101000010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRSQRT.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101000101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRECIPE.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101000110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRECIPE.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101001001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRSQRTE.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101001010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRSQRTE.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101001101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRINT.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101001110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRINT.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101010001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRINTRM.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101010010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRINTRM.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101010101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRINTRP.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101010110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRINTRP.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101011001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRINTRZ.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101011010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRINTRZ.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101011101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRINTRNE.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101011110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFRINTRNE.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101111010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFCVTL.S.H", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101111011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFCVTH.S.H", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101111100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFCVTL.D.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011101111101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFCVTH.D.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFFINT.S.W", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110000001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFFINT.S.WU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110000010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFFINT.D.L", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110000011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFFINT.D.LU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110000100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFFINTL.D.W", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110000101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFFINTH.D.W", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110001100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINT.W.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110001101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINT.L.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110001110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRM.W.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110001111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRM.L.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110010000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRP.W.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110010001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRP.L.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110010010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRZ.W.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110010011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRZ.L.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110010100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRNE.W.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110010101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRNE.L.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110010110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINT.WU.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110010111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINT.LU.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110011100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRZ.WU.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110011101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRZ.LU.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110100000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTL.L.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110100001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTH.L.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110100010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRML.L.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110100011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRMH.L.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110100100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRPL.L.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110100101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRPH.L.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110100110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRZL.L.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110100111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRZH.L.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110101000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRNEL.L.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110101001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVFTINTRNEH.L.S", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110111000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVEXTH.H.B", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110111001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVEXTH.W.H", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110111010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVEXTH.D.W", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110111011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVEXTH.Q.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110111100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVEXTH.HU.BU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110111101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVEXTH.WU.HU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110111110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVEXTH.DU.WU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011110111111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVEXTH.QU.DU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLGR2VR.B", XVt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111000001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLGR2VR.H", XVt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111000010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLGR2VR.W", XVt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111000011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLGR2VR.D", XVt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111000100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXT2XV.H.B", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111000101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXT2XV.W.B", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111000110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXT2XV.D.B", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111000111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXT2XV.W.H", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111001000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXT2XV.D.H", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111001001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXT2XV.D.W", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111001010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXT2XV.HU.BU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111001011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXT2XV.WU.BU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111001100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXT2XV.DU.BU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111001101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXT2XV.WU.HU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111001110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXT2XV.DU.HU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111001111jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VEXT2XV.DU.WU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "01110110100111111iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVHSELI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011010100000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVROTRI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101101010000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVROTRI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110101000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVROTRI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011010100001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVROTRI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011010100100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLRI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101101010010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLRI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110101001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLRI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011010100101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLRI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011010101000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRARI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101101010100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRARI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110101010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRARI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011010101001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRARI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011011101011110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVINSGR2VR.W", XVt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110111010111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVINSGR2VR.D", XVt[Rd], Xt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011011101111110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVPICKVE2GR.W", Xt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110111011111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVPICKVE2GR.D", Xt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011011110011110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVPICKVE2GR.WU", Xt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110111100111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVPICKVE2GR.DU", Xt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101101111011110iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVREPL128VEI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011011110111110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVREPL128VEI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110111101111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVREPL128VEI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101101111011111110ijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVREPL128VEI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011011111111110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVINSVE0.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110111111111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVINSVE0.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100000011110iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVPICKVE.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111000000111110iijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVPICKVE.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100000111000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLVE0.B", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011100000111100000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLVE0.H", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011100000111110000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLVE0.W", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011100000111111000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLVE0.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011100000111111100jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLVE0.Q", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011100001000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLLWIL.H.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110000100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLLWIL.W.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111000010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLLWIL.D.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100001001000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVEXTL.Q.D", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011100001100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLLWIL.HU.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110000110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLLWIL.WU.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111000011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLLWIL.DU.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100001101000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVEXTL.QU.DU", XVt[Rd], XVt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011100010000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITCLRI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110001000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITCLRI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111000100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITCLRI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100010001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITCLRI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100010100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITSETI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110001010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITSETI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111000101001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITSETI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100010101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITSETI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100011000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITREVI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110001100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITREVI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111000110001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITREVI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100011001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITREVI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100100100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSAT.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110010010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSAT.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111001001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSAT.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100100101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSAT.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100101000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSAT.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110010100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSAT.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111001010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSAT.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100101001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSAT.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100101100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLLI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110010110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLLI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111001011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLLI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100101101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLLI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100110000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110011000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111001100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100110001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100110100001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRAI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110011010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRAI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111001101001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRAI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011100110101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRAI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110100000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLNI.B.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111010000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLNI.H.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011101000001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLNI.W.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110100001iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLNI.D.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110100010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLRNI.B.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111010001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLRNI.H.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011101000101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLRNI.W.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110100011iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRLRNI.D.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110100100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLNI.B.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111010010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLNI.H.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011101001001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLNI.W.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110100101iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLNI.D.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110100110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLNI.BU.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111010011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLNI.HU.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011101001101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLNI.WU.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110100111iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLNI.DU.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110101000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLRNI.B.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111010100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLRNI.H.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011101010001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLRNI.W.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110101001iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLRNI.D.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110101010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLRNI.BU.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111010101001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLRNI.HU.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011101010101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLRNI.WU.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110101011iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRLRNI.DU.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110101100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRANI.B.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111010110001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRANI.H.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011101011001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRANI.W.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110101101iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRANI.D.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110101110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRARNI.B.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111010111001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRARNI.H.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011101011101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRARNI.W.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110101111iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSRARNI.D.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110110000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRANI.B.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111011000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRANI.H.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011101100001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRANI.W.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110110001iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRANI.D.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110110010001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRANI.BU.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111011001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRANI.HU.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011101100101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRANI.WU.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110110011iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRANI.DU.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110110100001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRARNI.B.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111011010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRARNI.H.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011101101001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRARNI.W.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110110101iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRARNI.D.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110110110001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRARNI.BU.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111011011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRARNI.HU.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111011101101101iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRARNI.WU.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011101110110111iiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSSRARNI.DU.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111100000iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVEXTRINS.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111100001iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVEXTRINS.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111100010iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVEXTRINS.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111100011iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVEXTRINS.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111100100iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSHUF4I.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111100101iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSHUF4I.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111100110iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSHUF4I.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111100111iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSHUF4I.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111110001iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBITSELI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111110100iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVANDI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111110101iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVORI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111110110iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVXORI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111110111iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVNORI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111111000iiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "XVLDI", XVt[Rd], imm);
        return buff;
    }
    if (isMask(opcode, "01110111111001iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVPERMI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111111010iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVPERMI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110111111011iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVPERMI.Q", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000011010010aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "XVBITSEL.V", XVt[Rd], XVt[Rj], XVt[Rk], XVt[Ra]);
        return buff;
    }
    if (isMask(opcode, "01110110100000000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSEQI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSEQI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSEQI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSEQI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000110iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100000111iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLEI.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001110iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100001111iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSLTI.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVADDI.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVADDI.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010110iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVADDI.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100010111iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVADDI.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100011000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSUBI.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100011001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSUBI.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100011010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSUBI.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100011011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVSUBI.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100011100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBSLL.V", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100011101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVBSRL.V", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100110iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.W", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100100111iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.D", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMAXI.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.BU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.HU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101110iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.WU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100101111iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVMINI.DU", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100110100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVFRSTPI.B", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110110100110101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "XVFRSTPI.H", XVt[Rd], XVt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011110100iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VANDI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011110101iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VORI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011110110iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VXORI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011110111iiiiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VNORI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "000010010001aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "VFMADD.S", Vt[Rd], Vt[Rj], Vt[Rk], Vt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010010101aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "VFMSUB.S", Vt[Rd], Vt[Rj], Vt[Rk], Vt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010011001aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "VFNMADD.S", Vt[Rd], Vt[Rj], Vt[Rk], Vt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010011101aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "VFNMSUB.S", Vt[Rd], Vt[Rj], Vt[Rk], Vt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010100001aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "XVFMADD.S", XVt[Rd], XVt[Rj], XVt[Rk], XVt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010100101aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "XVFMSUB.S", XVt[Rd], XVt[Rj], XVt[Rk], XVt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010101001aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "XVFNMADD.S", XVt[Rd], XVt[Rj], XVt[Rk], XVt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010101101aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "XVFNMSUB.S", XVt[Rd], XVt[Rj], XVt[Rk], XVt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010010010aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "VFMADD.D", Vt[Rd], Vt[Rj], Vt[Rk], Vt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010010110aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "VFMSUB.D", Vt[Rd], Vt[Rj], Vt[Rk], Vt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010011010aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "VFNMADD.D", Vt[Rd], Vt[Rj], Vt[Rk], Vt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010011110aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "VFNMSUB.D", Vt[Rd], Vt[Rj], Vt[Rk], Vt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010100010aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "XVFMADD.D", XVt[Rd], XVt[Rj], XVt[Rk], XVt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010100110aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "XVFMSUB.D", XVt[Rd], XVt[Rj], XVt[Rk], XVt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010101010aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "XVFNMADD.D", XVt[Rd], XVt[Rj], XVt[Rk], XVt[Ra]);
        return buff;
    }
    if (isMask(opcode, "000010101110aaaaakkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "XVFNMSUB.D", XVt[Rd], XVt[Rj], XVt[Rk], XVt[Ra]);
        return buff;
    }
    if (isMask(opcode, "01110101001000100kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVREPLVE.B", XVt[Rd], XVt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001000101kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVREPLVE.H", XVt[Rd], XVt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001000110kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVREPLVE.W", XVt[Rd], XVt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "01110101001000111kkkkkjjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "XVREPLVE.D", XVt[Rd], XVt[Rj], Xt[Rk]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111000000jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLGR2VR.B", XVt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111000001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLGR2VR.H", XVt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111000010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLGR2VR.W", XVt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111011010011111000011jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "XVREPLGR2VR.D", XVt[Rd], Xt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100010101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FRECIP.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100011101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FRECIPE.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100010110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FRECIP.D", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100011110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FRECIPE.D", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011100111101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRECIP.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011100111110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRECIP.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101000101jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRECIPE.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101000110jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRECIPE.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100011001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FRSQRT.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100100001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FRSQRTE.S", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100011010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FRSQRT.D", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0000000100010100100010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "FRSQRTE.D", Ft[Rd], Ft[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101000001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRSQRT.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101000010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRSQRT.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101001001jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRSQRTE.S", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011101001010jjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VFRSQRTE.D", Vt[Rd], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011100100111jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VSETNEZ.V", FCCt[Rc], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011100101000jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VSETANYEQZ.B", FCCt[Rc], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011100101001jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VSETANYEQZ.H", FCCt[Rc], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011100101010jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VSETANYEQZ.W", FCCt[Rc], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011100101011jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VSETANYEQZ.D", FCCt[Rc], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011100101100jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VSETALLNEZ.B", FCCt[Rc], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011100101101jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VSETALLNEZ.H", FCCt[Rc], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011100101110jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VSETALLNEZ.W", FCCt[Rc], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001010011100101111jjjjj00ccc", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s", "VSETALLNEZ.D", FCCt[Rc], Vt[Rj]);
        return buff;
    }
    if (isMask(opcode, "0111001100010000001iiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITCLRI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "011100110001000001iiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITCLRI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011000100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITCLRI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "0111001100010001iiiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VBITCLRI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010100110100iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VFRSTPI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010100110101iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VFRSTPI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010100100000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VMAXI.B", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010100100001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VMAXI.H", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010100100010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VMAXI.W", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010100100011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VMAXI.D", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010100101000iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VMAXI.BU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010100101001iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VMAXI.HU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010100101010iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VMAXI.WU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110010100101011iiiiijjjjjddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%lx", "VMAXI.DU", Vt[Rd], Vt[Rj], imm);
        return buff;
    }
    if (isMask(opcode, "01110011111000iiiiiiiiiiiiiddddd", &a)) {
        snprintf(buff, sizeof(buff), "%-15s %s, 0x%lx", "VLDI", Vt[Rd], imm);
        return buff;
    }
    snprintf(buff, sizeof(buff), "%08X ???", __builtin_bswap32(opcode));
    return buff;
}
