#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "ppc64le_printer.h"

// GPR name table — r0 is a normal register
// Uses box64 dynarec role names where mapped
static const char* Rn[] = {
    "r0",   "SP",    "TOC",   "x1",    "x2",    "x3",    "x4",    "x5",     // r0-r7
    "x6",   "xRIP",  "x7",    "r11",   "r12",   "r13",   "xRAX",  "xRCX",   // r8-r15
    "xRDX", "xRBX",  "xRSP",  "xRBP",  "xRSI",  "xRDI",  "xR8",   "xR9",    // r16-r23
    "xR10", "xR11",  "xR12",  "xR13",  "xR14",  "xR15",  "xFlags","xEmu"    // r24-r31
};

// GPR name table — r0 means literal 0 (for D/DS/DQ/X-form base-register RA positions)
// Identical to Rn[] except index 0 is "0" instead of "r0"
static const char* RnZ[] = {
    "0",    "SP",    "TOC",   "x1",    "x2",    "x3",    "x4",    "x5",     // r0-r7
    "x6",   "xRIP",  "x7",   "r11",   "r12",   "r13",   "xRAX",  "xRCX",   // r8-r15
    "xRDX", "xRBX",  "xRSP",  "xRBP",  "xRSI",  "xRDI",  "xR8",   "xR9",    // r16-r23
    "xR10", "xR11",  "xR12",  "xR13",  "xR14",  "xR15",  "xFlags","xEmu"    // r24-r31
};

// FPR name table (f0-f31)
static const char* Fn[] = {
    "f0",  "f1",  "f2",  "f3",  "f4",  "f5",  "f6",  "f7",
    "f8",  "f9",  "f10", "f11", "f12", "f13", "f14", "f15",
    "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
    "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31"
};

// CR condition bit names within a CR field (used for verbose disassembly)
__attribute__((unused)) static const char* CRbit[] = { "lt", "gt", "eq", "so" };

// SPR names (for MFSPR/MTSPR)
static const char* spr_name(int spr_enc) {
    // spr field is encoded swapped: spr[5:9]||spr[0:4]
    int spr = ((spr_enc & 0x1F) << 5) | ((spr_enc >> 5) & 0x1F);
    switch (spr) {
        case 1:   return "XER";
        case 8:   return "LR";
        case 9:   return "CTR";
        case 256: return "VRSAVE";
        case 268: return "TB";
        default:  return NULL;
    }
}

// VSX register name (vs0-vs63)
static char vsx_buf[8];
static const char* VSn(int n) {
    snprintf(vsx_buf, sizeof(vsx_buf), "vs%d", n);
    return vsx_buf;
}

// VMX register name (vr0-vr31)
static char vmx_buf[8];
static const char* VRn(int n) {
    snprintf(vmx_buf, sizeof(vmx_buf), "vr%d", n);
    return vmx_buf;
}

// Extract fields from a 32-bit PPC instruction
#define OPCD(op)     (((op) >> 26) & 0x3F)
#define RT(op)       (((op) >> 21) & 0x1F)
#define RS(op)       RT(op)
#define RA(op)       (((op) >> 16) & 0x1F)
#define RB(op)       (((op) >> 11) & 0x1F)
#define RC_FIELD(op) (((op) >> 6) & 0x1F)
#define D_IMM(op)    ((int16_t)((op) & 0xFFFF))
#define DS_IMM(op)   ((int16_t)((op) & 0xFFFC))
#define DQ_IMM(op)   ((int16_t)(((op) >> 4) & 0xFFF) << 4)
#define XO_10(op)    (((op) >> 1) & 0x3FF)
#define XO_9(op)     (((op) >> 1) & 0x1FF)
#define XO_2(op)     ((op) & 0x3)
#define Rc(op)       ((op) & 1)
#define OE(op)       (((op) >> 10) & 1)
#define LI(op)       (((int32_t)((op) & 0x03FFFFFC) << 6) >> 6)  // sign-extend 26-bit
#define AA(op)       (((op) >> 1) & 1)
#define LK(op)       ((op) & 1)
#define BO(op)       RT(op)
#define BI_FIELD(op) RA(op)
#define BD(op)       ((int16_t)((op) & 0xFFFC))
#define BH(op)       (((op) >> 11) & 0x3)
#define MB_32(op)    (((op) >> 6) & 0x1F)
#define ME_32(op)    (((op) >> 1) & 0x1F)
#define SH_32(op)    RB(op)
// MD-form: 6-bit sh = sh[0:4] from bits 11-15, sh[5] from bit 1
#define SH_64(op)    ((((op) >> 11) & 0x1F) | (((op) >> 1) & 1) << 5)
// MD-form: 6-bit mb/me = scrambled: logical = (raw[1:5] || raw[0])
#define MBE_64(op)   ({ int raw = ((op) >> 5) & 0x3F; ((raw >> 1) & 0x1F) | ((raw & 1) << 5); })
// XS-form: 6-bit sh = sh[0:4] from bits 11-15, sh[5] from bit 1
#define SH_XS(op)    SH_64(op)

// BF field for compare instructions (bits 23-25 of the instruction)
#define BF(op)       (((op) >> 23) & 0x7)
// L bit for compare (bit 21, i.e., bit 0 of the RT field which is BF||0||L)
#define L_CMP(op)    (((op) >> 21) & 1)

// Sign-extend a 16-bit value
#define SEXT16(v)    ((int16_t)(v))
// Sign-extend to int for display
#define SIMM(op)     ((int)SEXT16(D_IMM(op)))

// Condition register field for BI
#define BI_CR(bi)    ((bi) / 4)
#define BI_BIT(bi)   ((bi) % 4)

// Helper: format BO/BI conditional branch as mnemonic
static const char* cond_branch_name(int bo, int bi, const char* base, char* tmp, int sz) {
    int cr = BI_CR(bi);
    int bit = BI_BIT(bi);
    const char* cond;
    if (bo == 12 || bo == 4) {
        // BO_TRUE (12) or BO_FALSE (4)
        int istrue = (bo == 12);
        switch (bit) {
            case 0: cond = istrue ? "LT" : "GE"; break;
            case 1: cond = istrue ? "GT" : "LE"; break;
            case 2: cond = istrue ? "EQ" : "NE"; break;
            case 3: cond = istrue ? "SO" : "NS"; break;
            default: cond = "??"; break;
        }
        if (cr == 0)
            snprintf(tmp, sz, "%s%s", base, cond);
        else
            snprintf(tmp, sz, "%s%s cr%d,", base, cond, cr);
    } else if (bo == 20) {
        snprintf(tmp, sz, "%s", base);  // unconditional
    } else {
        snprintf(tmp, sz, "%sBO%d,BI%d,", base, bo, bi);
    }
    return tmp;
}

static char buff[200];
static char tmp[64];

const char* ppc64le_print(uint32_t opcode, uint64_t addr)
{
    int opcd = OPCD(opcode);
    int rt = RT(opcode);
    int rs = RS(opcode);
    int ra = RA(opcode);
    int rb = RB(opcode);
    int xo;

    switch (opcd) {
    // =====================================================================
    // I-form: unconditional branch (opcode 18)
    // =====================================================================
    case 18: {
        int li = LI(opcode);
        int aa = AA(opcode);
        int lk = LK(opcode);
        const char* name = lk ? (aa ? "BLA" : "BL") : (aa ? "BA" : "B");
        uint64_t target = aa ? (uint64_t)(int64_t)li : addr + li;
        snprintf(buff, sizeof(buff), "%s 0x%" PRIx64, name, target);
        return buff;
    }

    // =====================================================================
    // B-form: conditional branch (opcode 16)
    // =====================================================================
    case 16: {
        int bo = BO(opcode);
        int bi = BI_FIELD(opcode);
        int bd = BD(opcode);
        int aa = AA(opcode);
        int lk = LK(opcode);
        uint64_t target = aa ? (uint64_t)(int64_t)(int16_t)bd : addr + bd;
        const char* base = lk ? "BCL" : "B";
        cond_branch_name(bo, bi, base, tmp, sizeof(tmp));
        snprintf(buff, sizeof(buff), "%s 0x%" PRIx64, tmp, target);
        return buff;
    }

    // =====================================================================
    // XL-form: branch to LR/CTR (opcode 19)
    // =====================================================================
    case 19: {
        xo = XO_10(opcode);
        int bo = BO(opcode);
        int bi = BI_FIELD(opcode);
        switch (xo) {
        case 16: { // BCLR
            int lk = LK(opcode);
            if (bo == 20 && bi == 0) {
                snprintf(buff, sizeof(buff), "%s", lk ? "BLRL" : "BLR");
            } else {
                cond_branch_name(bo, bi, lk ? "BL" : "B", tmp, sizeof(tmp));
                snprintf(buff, sizeof(buff), "%sLR", tmp);
            }
            return buff;
        }
        case 528: { // BCCTR
            int lk = LK(opcode);
            if (bo == 20 && bi == 0) {
                snprintf(buff, sizeof(buff), "%s", lk ? "BCTRL" : "BCTR");
            } else {
                cond_branch_name(bo, bi, lk ? "BCL" : "BC", tmp, sizeof(tmp));
                snprintf(buff, sizeof(buff), "%sCTR", tmp);
            }
            return buff;
        }
        // CR logical
        case 257: snprintf(buff, sizeof(buff), "CRAND %d, %d, %d", rt, ra, rb); return buff;
        case 129: snprintf(buff, sizeof(buff), "CRANDC %d, %d, %d", rt, ra, rb); return buff;
        case 449: {
            if (ra == rb) snprintf(buff, sizeof(buff), "CRMOVE %d, %d", rt, ra);
            else snprintf(buff, sizeof(buff), "CROR %d, %d, %d", rt, ra, rb);
            return buff;
        }
        case 417: snprintf(buff, sizeof(buff), "CRORC %d, %d, %d", rt, ra, rb); return buff;
        case 193: {
            if (rt == ra && ra == rb) snprintf(buff, sizeof(buff), "CRCLR %d", rt);
            else snprintf(buff, sizeof(buff), "CRXOR %d, %d, %d", rt, ra, rb);
            return buff;
        }
        case 225: snprintf(buff, sizeof(buff), "CRNAND %d, %d, %d", rt, ra, rb); return buff;
        case 33:  {
            if (ra == rb) snprintf(buff, sizeof(buff), "CRNOT %d, %d", rt, ra);
            else snprintf(buff, sizeof(buff), "CRNOR %d, %d, %d", rt, ra, rb);
            return buff;
        }
        case 289: {
            if (rt == ra && ra == rb) snprintf(buff, sizeof(buff), "CRSET %d", rt);
            else snprintf(buff, sizeof(buff), "CREQV %d, %d, %d", rt, ra, rb);
            return buff;
        }
        case 150: snprintf(buff, sizeof(buff), "ISYNC"); return buff;
        }
        break;
    }

    // =====================================================================
    // D-form instructions
    // =====================================================================
    // ADDI (14): RA=0 means literal 0 → LI pseudo-op
    case 14: {
        int simm = SIMM(opcode);
        if (ra == 0)
            snprintf(buff, sizeof(buff), "LI %s, %d", Rn[rt], simm);
        else
            snprintf(buff, sizeof(buff), "ADDI %s, %s, %d", Rn[rt], Rn[ra], simm);
        return buff;
    }
    // ADDIS (15): RA=0 means literal 0 → LIS pseudo-op
    case 15: {
        int simm = SIMM(opcode);
        if (ra == 0)
            snprintf(buff, sizeof(buff), "LIS %s, 0x%x", Rn[rt], (uint16_t)simm);
        else
            snprintf(buff, sizeof(buff), "ADDIS %s, %s, 0x%x", Rn[rt], Rn[ra], (uint16_t)simm);
        return buff;
    }
    // SUBFIC (8)
    case 8:
        snprintf(buff, sizeof(buff), "SUBFIC %s, %s, %d", Rn[rt], Rn[ra], SIMM(opcode));
        return buff;
    // CMPLI (10) — compare logical immediate (unsigned)
    case 10: {
        int bf = BF(opcode);
        int l = L_CMP(opcode);
        const char* name = l ? "CMPLDI" : "CMPLWI";
        if (bf == 0)
            snprintf(buff, sizeof(buff), "%s %s, %u", name, Rn[ra], (uint16_t)D_IMM(opcode));
        else
            snprintf(buff, sizeof(buff), "%s cr%d, %s, %u", name, bf, Rn[ra], (uint16_t)D_IMM(opcode));
        return buff;
    }
    // CMPI (11) — compare immediate (signed)
    case 11: {
        int bf = BF(opcode);
        int l = L_CMP(opcode);
        const char* name = l ? "CMPDI" : "CMPWI";
        if (bf == 0)
            snprintf(buff, sizeof(buff), "%s %s, %d", name, Rn[ra], SIMM(opcode));
        else
            snprintf(buff, sizeof(buff), "%s cr%d, %s, %d", name, bf, Rn[ra], SIMM(opcode));
        return buff;
    }
    // ADDIC (12)
    case 12:
        snprintf(buff, sizeof(buff), "ADDIC %s, %s, %d", Rn[rt], Rn[ra], SIMM(opcode));
        return buff;
    // ADDIC. (13)
    case 13:
        snprintf(buff, sizeof(buff), "ADDIC. %s, %s, %d", Rn[rt], Rn[ra], SIMM(opcode));
        return buff;

    // ORI (24)
    case 24:
        if (rs == 0 && ra == 0 && D_IMM(opcode) == 0)
            snprintf(buff, sizeof(buff), "NOP");
        else
            snprintf(buff, sizeof(buff), "ORI %s, %s, 0x%x", Rn[ra], Rn[rs], (uint16_t)D_IMM(opcode));
        return buff;
    // ORIS (25)
    case 25:
        snprintf(buff, sizeof(buff), "ORIS %s, %s, 0x%x", Rn[ra], Rn[rs], (uint16_t)D_IMM(opcode));
        return buff;
    // XORI (26)
    case 26:
        snprintf(buff, sizeof(buff), "XORI %s, %s, 0x%x", Rn[ra], Rn[rs], (uint16_t)D_IMM(opcode));
        return buff;
    // XORIS (27)
    case 27:
        snprintf(buff, sizeof(buff), "XORIS %s, %s, 0x%x", Rn[ra], Rn[rs], (uint16_t)D_IMM(opcode));
        return buff;
    // ANDI. (28)
    case 28:
        snprintf(buff, sizeof(buff), "ANDI. %s, %s, 0x%x", Rn[ra], Rn[rs], (uint16_t)D_IMM(opcode));
        return buff;
    // ANDIS. (29)
    case 29:
        snprintf(buff, sizeof(buff), "ANDIS. %s, %s, 0x%x", Rn[ra], Rn[rs], (uint16_t)D_IMM(opcode));
        return buff;

    // TWI (3) — trap word immediate
    case 3:
        snprintf(buff, sizeof(buff), "TWI %d, %s, %d", rt, Rn[ra], SIMM(opcode));
        return buff;
    // TDI (2) — trap doubleword immediate
    case 2:
        snprintf(buff, sizeof(buff), "TDI %d, %s, %d", rt, Rn[ra], SIMM(opcode));
        return buff;

    // Load/store D-form: RA field uses RnZ (r0 = "0")
    // LWZ (32)
    case 32: snprintf(buff, sizeof(buff), "LWZ %s, %d(%s)", Rn[rt], SIMM(opcode), RnZ[ra]); return buff;
    // LWZU (33)
    case 33: snprintf(buff, sizeof(buff), "LWZU %s, %d(%s)", Rn[rt], SIMM(opcode), Rn[ra]); return buff;
    // LBZ (34)
    case 34: snprintf(buff, sizeof(buff), "LBZ %s, %d(%s)", Rn[rt], SIMM(opcode), RnZ[ra]); return buff;
    // LBZU (35)
    case 35: snprintf(buff, sizeof(buff), "LBZU %s, %d(%s)", Rn[rt], SIMM(opcode), Rn[ra]); return buff;
    // STW (36)
    case 36: snprintf(buff, sizeof(buff), "STW %s, %d(%s)", Rn[rs], SIMM(opcode), RnZ[ra]); return buff;
    // STWU (37)
    case 37: snprintf(buff, sizeof(buff), "STWU %s, %d(%s)", Rn[rs], SIMM(opcode), Rn[ra]); return buff;
    // STB (38)
    case 38: snprintf(buff, sizeof(buff), "STB %s, %d(%s)", Rn[rs], SIMM(opcode), RnZ[ra]); return buff;
    // STBU (39)
    case 39: snprintf(buff, sizeof(buff), "STBU %s, %d(%s)", Rn[rs], SIMM(opcode), Rn[ra]); return buff;
    // LHZ (40)
    case 40: snprintf(buff, sizeof(buff), "LHZ %s, %d(%s)", Rn[rt], SIMM(opcode), RnZ[ra]); return buff;
    // LHZU (41)
    case 41: snprintf(buff, sizeof(buff), "LHZU %s, %d(%s)", Rn[rt], SIMM(opcode), Rn[ra]); return buff;
    // LHA (42)
    case 42: snprintf(buff, sizeof(buff), "LHA %s, %d(%s)", Rn[rt], SIMM(opcode), RnZ[ra]); return buff;
    // LHAU (43)
    case 43: snprintf(buff, sizeof(buff), "LHAU %s, %d(%s)", Rn[rt], SIMM(opcode), Rn[ra]); return buff;
    // STH (44)
    case 44: snprintf(buff, sizeof(buff), "STH %s, %d(%s)", Rn[rs], SIMM(opcode), RnZ[ra]); return buff;
    // STHU (45)
    case 45: snprintf(buff, sizeof(buff), "STHU %s, %d(%s)", Rn[rs], SIMM(opcode), Rn[ra]); return buff;

    // FP load/store D-form: RA field uses RnZ (r0 = "0")
    // LFS (48)
    case 48: snprintf(buff, sizeof(buff), "LFS %s, %d(%s)", Fn[rt], SIMM(opcode), RnZ[ra]); return buff;
    // LFSU (49)
    case 49: snprintf(buff, sizeof(buff), "LFSU %s, %d(%s)", Fn[rt], SIMM(opcode), Rn[ra]); return buff;
    // LFD (50)
    case 50: snprintf(buff, sizeof(buff), "LFD %s, %d(%s)", Fn[rt], SIMM(opcode), RnZ[ra]); return buff;
    // LFDU (51)
    case 51: snprintf(buff, sizeof(buff), "LFDU %s, %d(%s)", Fn[rt], SIMM(opcode), Rn[ra]); return buff;
    // STFS (52)
    case 52: snprintf(buff, sizeof(buff), "STFS %s, %d(%s)", Fn[rs], SIMM(opcode), RnZ[ra]); return buff;
    // STFSU (53)
    case 53: snprintf(buff, sizeof(buff), "STFSU %s, %d(%s)", Fn[rs], SIMM(opcode), Rn[ra]); return buff;
    // STFD (54)
    case 54: snprintf(buff, sizeof(buff), "STFD %s, %d(%s)", Fn[rs], SIMM(opcode), RnZ[ra]); return buff;
    // STFDU (55)
    case 55: snprintf(buff, sizeof(buff), "STFDU %s, %d(%s)", Fn[rs], SIMM(opcode), Rn[ra]); return buff;

    // =====================================================================
    // DS-form (opcode 58: LD/LDU/LWA, opcode 62: STD/STDU)
    // =====================================================================
    case 58: {
        int ds = DS_IMM(opcode);
        int xo2 = XO_2(opcode);
        switch (xo2) {
        case 0: snprintf(buff, sizeof(buff), "LD %s, %d(%s)", Rn[rt], ds, RnZ[ra]); return buff;
        case 1: snprintf(buff, sizeof(buff), "LDU %s, %d(%s)", Rn[rt], ds, Rn[ra]); return buff;
        case 2: snprintf(buff, sizeof(buff), "LWA %s, %d(%s)", Rn[rt], ds, RnZ[ra]); return buff;
        }
        break;
    }
    case 62: {
        int ds = DS_IMM(opcode);
        int xo2 = XO_2(opcode);
        switch (xo2) {
        case 0: snprintf(buff, sizeof(buff), "STD %s, %d(%s)", Rn[rs], ds, RnZ[ra]); return buff;
        case 1: snprintf(buff, sizeof(buff), "STDU %s, %d(%s)", Rn[rs], ds, Rn[ra]); return buff;
        }
        break;
    }

    // =====================================================================
    // DQ-form (opcode 61: LXV/STXV)
    // =====================================================================
    case 61: {
        int xo4 = opcode & 0x7;  // low 3 bits of xo (the 4th bit is TX)
        // LXV: xo=1, STXV: xo=5
        // DQ-form: T = RT[0:4], TX = bit 3 of low nibble
        int t5 = rt;
        int tx = (opcode >> 3) & 1;
        int vsr = t5 | (tx << 5);
        int dq = ((opcode >> 4) & 0xFFF) << 4;
        // Sign-extend 16-bit
        if (dq & 0x8000) dq |= (int)0xFFFF0000;
        switch (xo4 & 0x7) {
        case 1: snprintf(buff, sizeof(buff), "LXV %s, %d(%s)", VSn(vsr), dq, RnZ[ra]); return buff;
        case 5: snprintf(buff, sizeof(buff), "STXV %s, %d(%s)", VSn(vsr), dq, RnZ[ra]); return buff;
        }
        break;
    }

    // =====================================================================
    // Rotate/shift MD-form (opcode 30)
    // =====================================================================
    case 30: {
        int xo3 = (opcode >> 2) & 0x7;
        int sh6 = SH_64(opcode);
        int mbe = MBE_64(opcode);
        int rc = Rc(opcode);
        const char* dot = rc ? "." : "";
        switch (xo3) {
        case 0: // RLDICL
            if (sh6 == 0)
                snprintf(buff, sizeof(buff), "CLRLDI%s %s, %s, %d", dot, Rn[ra], Rn[rs], mbe);
            else if (mbe == 0)
                snprintf(buff, sizeof(buff), "ROTLDI%s %s, %s, %d", dot, Rn[ra], Rn[rs], sh6);
            else if (mbe == 64 - sh6)
                snprintf(buff, sizeof(buff), "SRDI%s %s, %s, %d", dot, Rn[ra], Rn[rs], mbe);
            else
                snprintf(buff, sizeof(buff), "RLDICL%s %s, %s, %d, %d", dot, Rn[ra], Rn[rs], sh6, mbe);
            return buff;
        case 1: // RLDICR
            if (mbe == 63 - sh6)
                snprintf(buff, sizeof(buff), "SLDI%s %s, %s, %d", dot, Rn[ra], Rn[rs], sh6);
            else
                snprintf(buff, sizeof(buff), "RLDICR%s %s, %s, %d, %d", dot, Rn[ra], Rn[rs], sh6, mbe);
            return buff;
        case 2: // RLDIC
            snprintf(buff, sizeof(buff), "RLDIC%s %s, %s, %d, %d", dot, Rn[ra], Rn[rs], sh6, mbe);
            return buff;
        case 3: // RLDIMI
            snprintf(buff, sizeof(buff), "RLDIMI%s %s, %s, %d, %d", dot, Rn[ra], Rn[rs], sh6, mbe);
            return buff;
        case 4: { // MDS-form: RLDCL (xo=8) or RLDCR (xo=9)
            int xo4_mds = (opcode >> 1) & 0xF;
            if (xo4_mds == 8)
                snprintf(buff, sizeof(buff), "RLDCL%s %s, %s, %s, %d", dot, Rn[ra], Rn[rs], Rn[rb], mbe);
            else if (xo4_mds == 9)
                snprintf(buff, sizeof(buff), "RLDCR%s %s, %s, %s, %d", dot, Rn[ra], Rn[rs], Rn[rb], mbe);
            else
                break;
            return buff;
        }
        }
        break;
    }

    // M-form: RLWINM (21), RLWIMI (20), RLWNM (23)
    case 21: {
        int sh = SH_32(opcode);
        int mb = MB_32(opcode);
        int me = ME_32(opcode);
        int rc = Rc(opcode);
        const char* dot = rc ? "." : "";
        if (mb == 0 && me == 31 - sh)
            snprintf(buff, sizeof(buff), "SLWI%s %s, %s, %d", dot, Rn[ra], Rn[rs], sh);
        else if (mb == (32 - sh) % 32 && me == 31 && sh != 0)
            snprintf(buff, sizeof(buff), "SRWI%s %s, %s, %d", dot, Rn[ra], Rn[rs], 32 - sh);
        else if (sh == 0 && me == 31)
            snprintf(buff, sizeof(buff), "CLRLWI%s %s, %s, %d", dot, Rn[ra], Rn[rs], mb);
        else if (sh == 0 && mb == 0)
            snprintf(buff, sizeof(buff), "CLRRWI%s %s, %s, %d", dot, Rn[ra], Rn[rs], 31 - me);
        else if (mb == 0 && me == 31)
            snprintf(buff, sizeof(buff), "ROTLWI%s %s, %s, %d", dot, Rn[ra], Rn[rs], sh);
        else
            snprintf(buff, sizeof(buff), "RLWINM%s %s, %s, %d, %d, %d", dot, Rn[ra], Rn[rs], sh, mb, me);
        return buff;
    }
    case 20: {
        int sh = SH_32(opcode);
        int mb = MB_32(opcode);
        int me = ME_32(opcode);
        int rc = Rc(opcode);
        snprintf(buff, sizeof(buff), "RLWIMI%s %s, %s, %d, %d, %d", rc ? "." : "", Rn[ra], Rn[rs], sh, mb, me);
        return buff;
    }
    case 23: {
        int mb = MB_32(opcode);
        int me = ME_32(opcode);
        int rc = Rc(opcode);
        snprintf(buff, sizeof(buff), "RLWNM%s %s, %s, %s, %d, %d", rc ? "." : "", Rn[ra], Rn[rs], Rn[rb], mb, me);
        return buff;
    }

    // =====================================================================
    // Opcode 31: X-form, XO-form, XFX-form mega-switch
    // =====================================================================
    case 31: {
        xo = XO_10(opcode);
        int rc = Rc(opcode);
        const char* dot = rc ? "." : "";

        // First check XO-form (9-bit xo) for arithmetic
        int xo9 = XO_9(opcode);
        int oe = OE(opcode);
        switch (xo9) {
        case 266: snprintf(buff, sizeof(buff), "ADD%s%s %s, %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 10:  snprintf(buff, sizeof(buff), "ADDC%s%s %s, %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 138: snprintf(buff, sizeof(buff), "ADDE%s%s %s, %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 202: snprintf(buff, sizeof(buff), "ADDZE%s%s %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra]); return buff;
        case 40:  snprintf(buff, sizeof(buff), "SUBF%s%s %s, %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 8:   snprintf(buff, sizeof(buff), "SUBFC%s%s %s, %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 136: snprintf(buff, sizeof(buff), "SUBFE%s%s %s, %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 200: snprintf(buff, sizeof(buff), "SUBFZE%s%s %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra]); return buff;
        case 104: snprintf(buff, sizeof(buff), "NEG%s%s %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra]); return buff;
        case 235: snprintf(buff, sizeof(buff), "MULLW%s%s %s, %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 75:  snprintf(buff, sizeof(buff), "MULHW%s %s, %s, %s", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 11:  snprintf(buff, sizeof(buff), "MULHWU%s %s, %s, %s", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 233: snprintf(buff, sizeof(buff), "MULLD%s%s %s, %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 73:  snprintf(buff, sizeof(buff), "MULHD%s %s, %s, %s", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 9:   snprintf(buff, sizeof(buff), "MULHDU%s %s, %s, %s", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 491: snprintf(buff, sizeof(buff), "DIVW%s%s %s, %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 459: snprintf(buff, sizeof(buff), "DIVWU%s%s %s, %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 489: snprintf(buff, sizeof(buff), "DIVD%s%s %s, %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 457: snprintf(buff, sizeof(buff), "DIVDU%s%s %s, %s, %s", oe?"O":"", dot, Rn[rt], Rn[ra], Rn[rb]); return buff;
        }

        // X-form (10-bit xo)
        switch (xo) {
        // Compare register
        case 0: {
            int bf = BF(opcode);
            int l = L_CMP(opcode);
            const char* name = l ? "CMPD" : "CMPW";
            if (bf == 0) snprintf(buff, sizeof(buff), "%s %s, %s", name, Rn[ra], Rn[rb]);
            else snprintf(buff, sizeof(buff), "%s cr%d, %s, %s", name, bf, Rn[ra], Rn[rb]);
            return buff;
        }
        case 32: {
            int bf = BF(opcode);
            int l = L_CMP(opcode);
            const char* name = l ? "CMPLD" : "CMPLW";
            if (bf == 0) snprintf(buff, sizeof(buff), "%s %s, %s", name, Rn[ra], Rn[rb]);
            else snprintf(buff, sizeof(buff), "%s cr%d, %s, %s", name, bf, Rn[ra], Rn[rb]);
            return buff;
        }

        // Indexed load/store: RA uses RnZ (r0 = "0")
        case 87:  snprintf(buff, sizeof(buff), "LBZX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 279: snprintf(buff, sizeof(buff), "LHZX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 343: snprintf(buff, sizeof(buff), "LHAX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 23:  snprintf(buff, sizeof(buff), "LWZX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 341: snprintf(buff, sizeof(buff), "LWAX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 21:  snprintf(buff, sizeof(buff), "LDX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 215: snprintf(buff, sizeof(buff), "STBX %s, %s, %s", Rn[rs], RnZ[ra], Rn[rb]); return buff;
        case 407: snprintf(buff, sizeof(buff), "STHX %s, %s, %s", Rn[rs], RnZ[ra], Rn[rb]); return buff;
        case 151: snprintf(buff, sizeof(buff), "STWX %s, %s, %s", Rn[rs], RnZ[ra], Rn[rb]); return buff;
        case 149: snprintf(buff, sizeof(buff), "STDX %s, %s, %s", Rn[rs], RnZ[ra], Rn[rb]); return buff;

        // Byte-reverse load/store (indexed, RA uses RnZ)
        case 790: snprintf(buff, sizeof(buff), "LHBRX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 534: snprintf(buff, sizeof(buff), "LWBRX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 532: snprintf(buff, sizeof(buff), "LDBRX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 918: snprintf(buff, sizeof(buff), "STHBRX %s, %s, %s", Rn[rs], RnZ[ra], Rn[rb]); return buff;
        case 662: snprintf(buff, sizeof(buff), "STWBRX %s, %s, %s", Rn[rs], RnZ[ra], Rn[rb]); return buff;
        case 660: snprintf(buff, sizeof(buff), "STDBRX %s, %s, %s", Rn[rs], RnZ[ra], Rn[rb]); return buff;

        // FP indexed load/store (RA uses RnZ)
        case 535: snprintf(buff, sizeof(buff), "LFSX %s, %s, %s", Fn[rt], RnZ[ra], Rn[rb]); return buff;
        case 599: snprintf(buff, sizeof(buff), "LFDX %s, %s, %s", Fn[rt], RnZ[ra], Rn[rb]); return buff;
        case 663: snprintf(buff, sizeof(buff), "STFSX %s, %s, %s", Fn[rs], RnZ[ra], Rn[rb]); return buff;
        case 727: snprintf(buff, sizeof(buff), "STFDX %s, %s, %s", Fn[rs], RnZ[ra], Rn[rb]); return buff;

        // VMX indexed load/store (RA uses RnZ)
        case 103: snprintf(buff, sizeof(buff), "LVX %s, %s, %s", VRn(rt), RnZ[ra], Rn[rb]); return buff;
        case 231: snprintf(buff, sizeof(buff), "STVX %s, %s, %s", VRn(rs), RnZ[ra], Rn[rb]); return buff;

        // Atomic load-reserve/store-conditional (RA uses RnZ)
        case 52:  snprintf(buff, sizeof(buff), "LBARX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 116: snprintf(buff, sizeof(buff), "LHARX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 20:  snprintf(buff, sizeof(buff), "LWARX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 84:  snprintf(buff, sizeof(buff), "LDARX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        case 276: snprintf(buff, sizeof(buff), "LQARX %s, %s, %s", Rn[rt], RnZ[ra], Rn[rb]); return buff;
        // Store-conditional (always Rc=1)
        case 694: snprintf(buff, sizeof(buff), "STBCX. %s, %s, %s", Rn[rs], RnZ[ra], Rn[rb]); return buff;
        case 726: snprintf(buff, sizeof(buff), "STHCX. %s, %s, %s", Rn[rs], RnZ[ra], Rn[rb]); return buff;
        case 150: snprintf(buff, sizeof(buff), "STWCX. %s, %s, %s", Rn[rs], RnZ[ra], Rn[rb]); return buff;
        case 214: snprintf(buff, sizeof(buff), "STDCX. %s, %s, %s", Rn[rs], RnZ[ra], Rn[rb]); return buff;
        case 182: snprintf(buff, sizeof(buff), "STQCX. %s, %s, %s", Rn[rs], RnZ[ra], Rn[rb]); return buff;

        // Extend / count
        case 954: snprintf(buff, sizeof(buff), "EXTSB%s %s, %s", dot, Rn[ra], Rn[rs]); return buff;
        case 922: snprintf(buff, sizeof(buff), "EXTSH%s %s, %s", dot, Rn[ra], Rn[rs]); return buff;
        case 986: snprintf(buff, sizeof(buff), "EXTSW%s %s, %s", dot, Rn[ra], Rn[rs]); return buff;
        case 26:  snprintf(buff, sizeof(buff), "CNTLZW%s %s, %s", dot, Rn[ra], Rn[rs]); return buff;
        case 58:  snprintf(buff, sizeof(buff), "CNTLZD%s %s, %s", dot, Rn[ra], Rn[rs]); return buff;
        case 538: snprintf(buff, sizeof(buff), "CNTTZW%s %s, %s", dot, Rn[ra], Rn[rs]); return buff;
        case 570: snprintf(buff, sizeof(buff), "CNTTZD%s %s, %s", dot, Rn[ra], Rn[rs]); return buff;
        case 378: snprintf(buff, sizeof(buff), "POPCNTW %s, %s", Rn[ra], Rn[rs]); return buff;
        case 506: snprintf(buff, sizeof(buff), "POPCNTD %s, %s", Rn[ra], Rn[rs]); return buff;
        case 122: snprintf(buff, sizeof(buff), "POPCNTB %s, %s", Rn[ra], Rn[rs]); return buff;

        // Logical register (r0 is normal in ALU)
        case 28:
            if (rs == rb) snprintf(buff, sizeof(buff), "MR%s %s, %s", dot, Rn[ra], Rn[rs]);
            else snprintf(buff, sizeof(buff), "AND%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]);
            return buff;
        case 444:
            if (rs == rb) snprintf(buff, sizeof(buff), "MR%s %s, %s", dot, Rn[ra], Rn[rs]);
            else snprintf(buff, sizeof(buff), "OR%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]);
            return buff;
        case 60:  snprintf(buff, sizeof(buff), "ANDC%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]); return buff;
        case 412: snprintf(buff, sizeof(buff), "ORC%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]); return buff;
        case 316: snprintf(buff, sizeof(buff), "XOR%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]); return buff;
        case 476: snprintf(buff, sizeof(buff), "NAND%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]); return buff;
        case 124:
            if (rs == rb) snprintf(buff, sizeof(buff), "NOT%s %s, %s", dot, Rn[ra], Rn[rs]);
            else snprintf(buff, sizeof(buff), "NOR%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]);
            return buff;
        case 284: snprintf(buff, sizeof(buff), "EQV%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]); return buff;

        // Shift register
        case 24:  snprintf(buff, sizeof(buff), "SLW%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]); return buff;
        case 536: snprintf(buff, sizeof(buff), "SRW%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]); return buff;
        case 792: snprintf(buff, sizeof(buff), "SRAW%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]); return buff;
        case 824: snprintf(buff, sizeof(buff), "SRAWI%s %s, %s, %d", dot, Rn[ra], Rn[rs], rb); return buff;
        case 27:  snprintf(buff, sizeof(buff), "SLD%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]); return buff;
        case 539: snprintf(buff, sizeof(buff), "SRD%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]); return buff;
        case 794: snprintf(buff, sizeof(buff), "SRAD%s %s, %s, %s", dot, Rn[ra], Rn[rs], Rn[rb]); return buff;

        // SRADI (XS-form, xo=413 in 9-bit field)
        // The 10-bit xo for SRADI is 413<<1|sh[5], so match on 9-bit field
        case 826: case 827: {
            int sh6 = SH_XS(opcode);
            snprintf(buff, sizeof(buff), "SRADI%s %s, %s, %d", dot, Rn[ra], Rn[rs], sh6);
            return buff;
        }

        // EXTSWSLI (XS-form, xo=445 in 9-bit field)
        case 890: case 891: {
            int sh6 = SH_XS(opcode);
            snprintf(buff, sizeof(buff), "EXTSWSLI%s %s, %s, %d", dot, Rn[ra], Rn[rs], sh6);
            return buff;
        }

        // Modulo (POWER9)
        case 267: snprintf(buff, sizeof(buff), "MODUW %s, %s, %s", Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 779: snprintf(buff, sizeof(buff), "MODSW %s, %s, %s", Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 265: snprintf(buff, sizeof(buff), "MODUD %s, %s, %s", Rn[rt], Rn[ra], Rn[rb]); return buff;
        case 777: snprintf(buff, sizeof(buff), "MODSD %s, %s, %s", Rn[rt], Rn[ra], Rn[rb]); return buff;

        // SPR move
        case 339: {
            int spr_enc = (ra | (rb << 5));
            const char* name = spr_name(spr_enc);
            if (name) snprintf(buff, sizeof(buff), "MF%s %s", name, Rn[rt]);
            else snprintf(buff, sizeof(buff), "MFSPR %s, %d", Rn[rt], ((spr_enc & 0x1F) << 5) | ((spr_enc >> 5) & 0x1F));
            return buff;
        }
        case 467: {
            int spr_enc = (ra | (rb << 5));
            const char* name = spr_name(spr_enc);
            if (name) snprintf(buff, sizeof(buff), "MT%s %s", name, Rn[rs]);
            else snprintf(buff, sizeof(buff), "MTSPR %d, %s", ((spr_enc & 0x1F) << 5) | ((spr_enc >> 5) & 0x1F), Rn[rs]);
            return buff;
        }

        // CR move
        case 19: snprintf(buff, sizeof(buff), "MFCR %s", Rn[rt]); return buff;
        case 144: {
            int fxm = (opcode >> 12) & 0xFF;
            int one = (opcode >> 20) & 1;
            if (one)
                snprintf(buff, sizeof(buff), "MTOCRF 0x%x, %s", fxm, Rn[rs]);
            else if (fxm == 0xFF)
                snprintf(buff, sizeof(buff), "MTCR %s", Rn[rs]);
            else
                snprintf(buff, sizeof(buff), "MTCRF 0x%x, %s", fxm, Rn[rs]);
            return buff;
        }

        // Sync / barriers
        case 598: {
            int l_sync = rt >> 1;
            if (l_sync == 0) snprintf(buff, sizeof(buff), "SYNC");
            else snprintf(buff, sizeof(buff), "LWSYNC");
            return buff;
        }
        case 854: snprintf(buff, sizeof(buff), "EIEIO"); return buff;

        // Cache management (RA uses RnZ)
        case 54:  snprintf(buff, sizeof(buff), "DCBST %s, %s", RnZ[ra], Rn[rb]); return buff;
        case 86:  snprintf(buff, sizeof(buff), "DCBF %s, %s", RnZ[ra], Rn[rb]); return buff;
        case 982: snprintf(buff, sizeof(buff), "ICBI %s, %s", RnZ[ra], Rn[rb]); return buff;

        // Trap register
        case 4: {
            if (rt == 31 && ra == 0 && rb == 0) snprintf(buff, sizeof(buff), "TRAP");
            else snprintf(buff, sizeof(buff), "TW %d, %s, %s", rt, Rn[ra], Rn[rb]);
            return buff;
        }
        case 68: snprintf(buff, sizeof(buff), "TD %d, %s, %s", rt, Rn[ra], Rn[rb]); return buff;

        // ISEL
        case 15: {
            // ISEL encoding: xo field contains BC bits too
            // X-form: 31 | RT | RA | RB | BC(5) | 15(5) | 0
            // The 10-bit xo = (BC << 5) | 15, so we already matched xo=15 only for BC=0
            // Actually ISEL xo = (BC[0:4] << 5) | 15, and we extract from the full 10-bit field
            // Let's re-check: the 10-bit XO for ISEL is variable. Check if low 5 bits == 15
            break;  // handled below as special case
        }

        // Move to/from VSR (XX1-form, handled specially)
        case 51: { // MFVSRD
            int tx = opcode & 1;
            int xs = rs | (tx << 5);
            snprintf(buff, sizeof(buff), "MFVSRD %s, %s", Rn[ra], VSn(xs));
            return buff;
        }
        case 179: { // MTVSRD
            int tx = opcode & 1;
            int xt = rt | (tx << 5);
            snprintf(buff, sizeof(buff), "MTVSRD %s, %s", VSn(xt), Rn[ra]);
            return buff;
        }
        case 115: { // MFVSRWZ
            int tx = opcode & 1;
            int xs = rs | (tx << 5);
            snprintf(buff, sizeof(buff), "MFVSRWZ %s, %s", Rn[ra], VSn(xs));
            return buff;
        }
        case 243: { // MTVSRWZ
            int tx = opcode & 1;
            int xt = rt | (tx << 5);
            snprintf(buff, sizeof(buff), "MTVSRWZ %s, %s", VSn(xt), Rn[ra]);
            return buff;
        }
        case 435: { // MTVSRDD
            int tx = opcode & 1;
            int xt = rt | (tx << 5);
            snprintf(buff, sizeof(buff), "MTVSRDD %s, %s, %s", VSn(xt), RnZ[ra], Rn[rb]);
            return buff;
        }
        case 307: { // MFVSRLD
            int tx = opcode & 1;
            int xs = rs | (tx << 5);
            snprintf(buff, sizeof(buff), "MFVSRLD %s, %s", Rn[ra], VSn(xs));
            return buff;
        }

        // LXVX / STXVX (XX1-form, RA uses RnZ)
        case 268: {
            int tx = opcode & 1;
            int xt = rt | (tx << 5);
            snprintf(buff, sizeof(buff), "LXVX %s, %s, %s", VSn(xt), RnZ[ra], Rn[rb]);
            return buff;
        }
        case 396: {
            int tx = opcode & 1;
            int xs = rs | (tx << 5);
            snprintf(buff, sizeof(buff), "STXVX %s, %s, %s", VSn(xs), RnZ[ra], Rn[rb]);
            return buff;
        }
        }

        // ISEL: 10-bit xo has form (BC << 5) | 15
        if ((xo & 0x1F) == 15) {
            int bc = (xo >> 5) & 0x1F;
            snprintf(buff, sizeof(buff), "ISEL %s, %s, %s, %d", Rn[rt], RnZ[ra], Rn[rb], bc);
            return buff;
        }

        break;
    }

    // =====================================================================
    // Opcode 4: VMX/Altivec (VX-form and VA-form)
    // =====================================================================
    case 4: {
        // VA-form: check bits 0-5 (6-bit xo)
        int va_xo = opcode & 0x3F;
        int rc_va = RC_FIELD(opcode);  // bits 6-10 = VRC for VA-form
        switch (va_xo) {
        case 43: snprintf(buff, sizeof(buff), "VPERM %s, %s, %s, %s", VRn(rt), VRn(ra), VRn(rb), VRn(rc_va)); return buff;
        case 44: snprintf(buff, sizeof(buff), "VSLDOI %s, %s, %s, %d", VRn(rt), VRn(ra), VRn(rb), rc_va & 0xF); return buff;
        case 42: snprintf(buff, sizeof(buff), "VSEL %s, %s, %s, %s", VRn(rt), VRn(ra), VRn(rb), VRn(rc_va)); return buff;
        case 51: snprintf(buff, sizeof(buff), "MADDLD %s, %s, %s, %s", Rn[rt], Rn[ra], Rn[rb], Rn[rc_va]); return buff;
        case 36: snprintf(buff, sizeof(buff), "VMSUMUBM %s, %s, %s, %s", VRn(rt), VRn(ra), VRn(rb), VRn(rc_va)); return buff;
        case 38: snprintf(buff, sizeof(buff), "VMSUMUHM %s, %s, %s, %s", VRn(rt), VRn(ra), VRn(rb), VRn(rc_va)); return buff;
        case 41: snprintf(buff, sizeof(buff), "VMSUMSHS %s, %s, %s, %s", VRn(rt), VRn(ra), VRn(rb), VRn(rc_va)); return buff;
        case 40: snprintf(buff, sizeof(buff), "VMSUMSHM %s, %s, %s, %s", VRn(rt), VRn(ra), VRn(rb), VRn(rc_va)); return buff;
        case 37: snprintf(buff, sizeof(buff), "VMSUMMBM %s, %s, %s, %s", VRn(rt), VRn(ra), VRn(rb), VRn(rc_va)); return buff;
        case 32: snprintf(buff, sizeof(buff), "VMHADDSHS %s, %s, %s, %s", VRn(rt), VRn(ra), VRn(rb), VRn(rc_va)); return buff;
        case 33: snprintf(buff, sizeof(buff), "VMHRADDSHS %s, %s, %s, %s", VRn(rt), VRn(ra), VRn(rb), VRn(rc_va)); return buff;
        case 34: snprintf(buff, sizeof(buff), "VMLADDUHM %s, %s, %s, %s", VRn(rt), VRn(ra), VRn(rb), VRn(rc_va)); return buff;
        }

        // VX-form: 11-bit xo (bits 0-10)
        int vx_xo = opcode & 0x7FF;
        switch (vx_xo) {
        // Integer add
        case 0:    snprintf(buff, sizeof(buff), "VADDUBM %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 64:   snprintf(buff, sizeof(buff), "VADDUHM %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 128:  snprintf(buff, sizeof(buff), "VADDUWM %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 192:  snprintf(buff, sizeof(buff), "VADDUDM %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        // Integer sub
        case 1024: snprintf(buff, sizeof(buff), "VSUBUBM %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 1088: snprintf(buff, sizeof(buff), "VSUBUHM %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 1152: snprintf(buff, sizeof(buff), "VSUBUWM %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 1216: snprintf(buff, sizeof(buff), "VSUBUDM %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        // Compare equal
        case 6:    snprintf(buff, sizeof(buff), "VCMPEQUB %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 70:   snprintf(buff, sizeof(buff), "VCMPEQUH %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 134:  snprintf(buff, sizeof(buff), "VCMPEQUW %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 199:  snprintf(buff, sizeof(buff), "VCMPEQUD %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        // Logical
        case 1028: snprintf(buff, sizeof(buff), "VAND %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 1156: snprintf(buff, sizeof(buff), "VOR %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 1220: snprintf(buff, sizeof(buff), "VXOR %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 1284: snprintf(buff, sizeof(buff), "VNOR %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 1092: snprintf(buff, sizeof(buff), "VANDC %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 1348: snprintf(buff, sizeof(buff), "VORC %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 1412: snprintf(buff, sizeof(buff), "VNAND %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 1668: snprintf(buff, sizeof(buff), "VEQV %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        // Splat
        case 780:  snprintf(buff, sizeof(buff), "VSPLTISB %s, %d", VRn(rt), (int)(int8_t)((ra & 0x1F) << 3) >> 3); return buff;
        case 844:  snprintf(buff, sizeof(buff), "VSPLTISH %s, %d", VRn(rt), (int)(int8_t)((ra & 0x1F) << 3) >> 3); return buff;
        case 908:  snprintf(buff, sizeof(buff), "VSPLTISW %s, %d", VRn(rt), (int)(int8_t)((ra & 0x1F) << 3) >> 3); return buff;
        case 524:  snprintf(buff, sizeof(buff), "VSPLTB %s, %s, %d", VRn(rt), VRn(rb), ra); return buff;
        case 588:  snprintf(buff, sizeof(buff), "VSPLTH %s, %s, %d", VRn(rt), VRn(rb), ra); return buff;
        case 652:  snprintf(buff, sizeof(buff), "VSPLTW %s, %s, %d", VRn(rt), VRn(rb), ra); return buff;
        // Shift
        case 260:  snprintf(buff, sizeof(buff), "VSLB %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 324:  snprintf(buff, sizeof(buff), "VSLH %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 388:  snprintf(buff, sizeof(buff), "VSLW %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 1476: snprintf(buff, sizeof(buff), "VSLD %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 516:  snprintf(buff, sizeof(buff), "VSRB %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 580:  snprintf(buff, sizeof(buff), "VSRH %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 644:  snprintf(buff, sizeof(buff), "VSRW %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 1732: snprintf(buff, sizeof(buff), "VSRD %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 452:  snprintf(buff, sizeof(buff), "VSL %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 708:  snprintf(buff, sizeof(buff), "VSR %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        // Merge
        case 12:   snprintf(buff, sizeof(buff), "VMRGHB %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 76:   snprintf(buff, sizeof(buff), "VMRGHH %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 140:  snprintf(buff, sizeof(buff), "VMRGHW %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 268:  snprintf(buff, sizeof(buff), "VMRGLB %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 332:  snprintf(buff, sizeof(buff), "VMRGLH %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        case 396:  snprintf(buff, sizeof(buff), "VMRGLW %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        // Bit permute
        case 1356: snprintf(buff, sizeof(buff), "VBPERMQ %s, %s, %s", VRn(rt), VRn(ra), VRn(rb)); return buff;
        }
        break;
    }

    // =====================================================================
    // Opcode 59: single-precision FP (A-form)
    // =====================================================================
    case 59: {
        int xo5 = (opcode >> 1) & 0x1F;
        int frc = RC_FIELD(opcode);
        switch (xo5) {
        case 21: snprintf(buff, sizeof(buff), "FADDS %s, %s, %s", Fn[rt], Fn[ra], Fn[rb]); return buff;
        case 20: snprintf(buff, sizeof(buff), "FSUBS %s, %s, %s", Fn[rt], Fn[ra], Fn[rb]); return buff;
        case 25: snprintf(buff, sizeof(buff), "FMULS %s, %s, %s", Fn[rt], Fn[ra], Fn[frc]); return buff;
        case 18: snprintf(buff, sizeof(buff), "FDIVS %s, %s, %s", Fn[rt], Fn[ra], Fn[rb]); return buff;
        case 22: snprintf(buff, sizeof(buff), "FSQRTS %s, %s", Fn[rt], Fn[rb]); return buff;
        }
        // Also FCFIDS (xo=846 in X-form under opcode 59)
        xo = XO_10(opcode);
        if (xo == 846) { snprintf(buff, sizeof(buff), "FCFIDS %s, %s", Fn[rt], Fn[rb]); return buff; }
        break;
    }

    // =====================================================================
    // Opcode 63: double-precision FP (A-form and X-form)
    // =====================================================================
    case 63: {
        // First try A-form (5-bit xo, bits 1-5)
        int xo5 = (opcode >> 1) & 0x1F;
        int frc = RC_FIELD(opcode);
        switch (xo5) {
        case 21: snprintf(buff, sizeof(buff), "FADD %s, %s, %s", Fn[rt], Fn[ra], Fn[rb]); return buff;
        case 20: snprintf(buff, sizeof(buff), "FSUB %s, %s, %s", Fn[rt], Fn[ra], Fn[rb]); return buff;
        case 25: snprintf(buff, sizeof(buff), "FMUL %s, %s, %s", Fn[rt], Fn[ra], Fn[frc]); return buff;
        case 18: snprintf(buff, sizeof(buff), "FDIV %s, %s, %s", Fn[rt], Fn[ra], Fn[rb]); return buff;
        case 22: snprintf(buff, sizeof(buff), "FSQRT %s, %s", Fn[rt], Fn[rb]); return buff;
        case 29: snprintf(buff, sizeof(buff), "FMADD %s, %s, %s, %s", Fn[rt], Fn[ra], Fn[frc], Fn[rb]); return buff;
        case 28: snprintf(buff, sizeof(buff), "FMSUB %s, %s, %s, %s", Fn[rt], Fn[ra], Fn[frc], Fn[rb]); return buff;
        case 31: snprintf(buff, sizeof(buff), "FNMADD %s, %s, %s, %s", Fn[rt], Fn[ra], Fn[frc], Fn[rb]); return buff;
        case 30: snprintf(buff, sizeof(buff), "FNMSUB %s, %s, %s, %s", Fn[rt], Fn[ra], Fn[frc], Fn[rb]); return buff;
        case 23: snprintf(buff, sizeof(buff), "FSEL %s, %s, %s, %s", Fn[rt], Fn[ra], Fn[frc], Fn[rb]); return buff;
        }
        // X-form (10-bit xo)
        xo = XO_10(opcode);
        switch (xo) {
        case 12:  snprintf(buff, sizeof(buff), "FRSP %s, %s", Fn[rt], Fn[rb]); return buff;
        case 814: snprintf(buff, sizeof(buff), "FCTID %s, %s", Fn[rt], Fn[rb]); return buff;
        case 815: snprintf(buff, sizeof(buff), "FCTIDZ %s, %s", Fn[rt], Fn[rb]); return buff;
        case 14:  snprintf(buff, sizeof(buff), "FCTIW %s, %s", Fn[rt], Fn[rb]); return buff;
        case 15:  snprintf(buff, sizeof(buff), "FCTIWZ %s, %s", Fn[rt], Fn[rb]); return buff;
        case 846: snprintf(buff, sizeof(buff), "FCFID %s, %s", Fn[rt], Fn[rb]); return buff;
        case 974: snprintf(buff, sizeof(buff), "FCFIDU %s, %s", Fn[rt], Fn[rb]); return buff;
        case 942: snprintf(buff, sizeof(buff), "FCTIDU %s, %s", Fn[rt], Fn[rb]); return buff;
        case 943: snprintf(buff, sizeof(buff), "FCTIDUZ %s, %s", Fn[rt], Fn[rb]); return buff;
        case 264: snprintf(buff, sizeof(buff), "FABS %s, %s", Fn[rt], Fn[rb]); return buff;
        case 136: snprintf(buff, sizeof(buff), "FNABS %s, %s", Fn[rt], Fn[rb]); return buff;
        case 40:  snprintf(buff, sizeof(buff), "FNEG %s, %s", Fn[rt], Fn[rb]); return buff;
        case 72:  snprintf(buff, sizeof(buff), "FMR %s, %s", Fn[rt], Fn[rb]); return buff;
        case 8:   snprintf(buff, sizeof(buff), "FCPSGN %s, %s, %s", Fn[rt], Fn[ra], Fn[rb]); return buff;
        case 424: snprintf(buff, sizeof(buff), "FRIZ %s, %s", Fn[rt], Fn[rb]); return buff;
        case 392: snprintf(buff, sizeof(buff), "FRIN %s, %s", Fn[rt], Fn[rb]); return buff;
        case 456: snprintf(buff, sizeof(buff), "FRIP %s, %s", Fn[rt], Fn[rb]); return buff;
        case 488: snprintf(buff, sizeof(buff), "FRIM %s, %s", Fn[rt], Fn[rb]); return buff;
        case 0: {
            int bf = BF(opcode);
            snprintf(buff, sizeof(buff), "FCMPU cr%d, %s, %s", bf, Fn[ra], Fn[rb]);
            return buff;
        }
        case 32: {
            int bf = BF(opcode);
            snprintf(buff, sizeof(buff), "FCMPO cr%d, %s, %s", bf, Fn[ra], Fn[rb]);
            return buff;
        }
        case 583: snprintf(buff, sizeof(buff), "MFFS %s", Fn[rt]); return buff;
        case 711: {
            int flm = (opcode >> 17) & 0xFF;
            snprintf(buff, sizeof(buff), "MTFSF 0x%x, %s", flm, Fn[rb]);
            return buff;
        }
        case 134: {
            int bf = (opcode >> 23) & 0x7;
            int u = (opcode >> 12) & 0xF;
            snprintf(buff, sizeof(buff), "MTFSFI %d, %d", bf, u);
            return buff;
        }
        case 70:  snprintf(buff, sizeof(buff), "MTFSB0 %d", rt); return buff;
        case 38:  snprintf(buff, sizeof(buff), "MTFSB1 %d", rt); return buff;
        }
        break;
    }

    // =====================================================================
    // Opcode 60: VSX (XX3-form, XX2-form, XX4-form)
    // =====================================================================
    case 60: {
        // Try to decode various VSX sub-formats
        // XX3-form: xo is bits 3-10 (8 bits)
        int xx3_xo = (opcode >> 3) & 0xFF;
        int ax = (opcode >> 2) & 1;
        int bx = (opcode >> 1) & 1;
        int tx = opcode & 1;
        int xa6 = ra | (ax << 5);
        int xb6 = rb | (bx << 5);
        int xt6 = rt | (tx << 5);

        // Check for XX4-form first (XXSEL, xo bits 4-5 = 3)
        int xx4_xo = (opcode >> 4) & 0x3;
        if (xx4_xo == 3) {
            int cx = (opcode >> 3) & 1;
            int xc6 = RC_FIELD(opcode) | (cx << 5);
            ax = (opcode >> 2) & 1;
            bx = (opcode >> 1) & 1;
            tx = opcode & 1;
            xa6 = ra | (ax << 5);
            xb6 = rb | (bx << 5);
            xt6 = rt | (tx << 5);
            snprintf(buff, sizeof(buff), "XXSEL %s, %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6), VSn(xc6));
            return buff;
        }

        // XX2-form: check bits 16-20 == 0 (RA field unused)
        // XO for XX2 is bits 2-10 (9 bits)
        int xx2_xo = (opcode >> 2) & 0x1FF;
        bx = (opcode >> 1) & 1;
        tx = opcode & 1;
        xb6 = rb | (bx << 5);
        xt6 = rt | (tx << 5);

        // XXSPLTIB: special encoding
        if (xx2_xo == 360) {
            int imm8 = (opcode >> 11) & 0xFF;
            snprintf(buff, sizeof(buff), "XXSPLTIB %s, %d", VSn(xt6), imm8);
            return buff;
        }

        // XX2-form instructions (ra == 0 typically, but we check xo)
        switch (xx2_xo) {
        case 475: snprintf(buff, sizeof(buff), "XXBRW %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 503: snprintf(buff, sizeof(buff), "XXBRD %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 507: snprintf(buff, sizeof(buff), "XXBRQ %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 471: snprintf(buff, sizeof(buff), "XXBRH %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 265: snprintf(buff, sizeof(buff), "XSCVDPSP %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 267: snprintf(buff, sizeof(buff), "XSCVDPSPN %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 329: snprintf(buff, sizeof(buff), "XSCVSPDP %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 331: snprintf(buff, sizeof(buff), "XSCVSPDPN %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 376: snprintf(buff, sizeof(buff), "XSCVSXDDP %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 312: snprintf(buff, sizeof(buff), "XSCVSXDSP %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 344: snprintf(buff, sizeof(buff), "XSCVDPSXDS %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 88:  snprintf(buff, sizeof(buff), "XSCVDPSXWS %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 328: snprintf(buff, sizeof(buff), "XSCVDPUXDS %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 360: break; // XXSPLTIB handled above
        case 203: snprintf(buff, sizeof(buff), "XVSQRTDP %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 139: snprintf(buff, sizeof(buff), "XVSQRTSP %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 75:  snprintf(buff, sizeof(buff), "XSSQRTDP %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 11:  snprintf(buff, sizeof(buff), "XSSQRTSP %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 473: snprintf(buff, sizeof(buff), "XVABSDP %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 505: snprintf(buff, sizeof(buff), "XVNEGDP %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 409: snprintf(buff, sizeof(buff), "XVABSSP %s, %s", VSn(xt6), VSn(xb6)); return buff;
        case 441: snprintf(buff, sizeof(buff), "XVNEGSP %s, %s", VSn(xt6), VSn(xb6)); return buff;
        }

        // Recalculate for XX3 (all three operands)
        ax = (opcode >> 2) & 1;
        bx = (opcode >> 1) & 1;
        tx = opcode & 1;
        xa6 = ra | (ax << 5);
        xb6 = rb | (bx << 5);
        xt6 = rt | (tx << 5);

        // XX3-form
        switch (xx3_xo) {
        // Logical
        case 146: snprintf(buff, sizeof(buff), "XXLOR %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 154: snprintf(buff, sizeof(buff), "XXLXOR %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 130: snprintf(buff, sizeof(buff), "XXLAND %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 138: snprintf(buff, sizeof(buff), "XXLANDC %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 162: snprintf(buff, sizeof(buff), "XXLNOR %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 186: snprintf(buff, sizeof(buff), "XXLEQV %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 170: snprintf(buff, sizeof(buff), "XXLORC %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 178: snprintf(buff, sizeof(buff), "XXLNAND %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        // Merge/permute
        case 10:  snprintf(buff, sizeof(buff), "XXMRGHD %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 42:  snprintf(buff, sizeof(buff), "XXMRGLD %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        // Vector FP arithmetic
        case 96:  snprintf(buff, sizeof(buff), "XVADDDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 104: snprintf(buff, sizeof(buff), "XVSUBDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 112: snprintf(buff, sizeof(buff), "XVMULDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 120: snprintf(buff, sizeof(buff), "XVDIVDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 64:  snprintf(buff, sizeof(buff), "XVADDSP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 72:  snprintf(buff, sizeof(buff), "XVSUBSP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 80:  snprintf(buff, sizeof(buff), "XVMULSP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 88:  snprintf(buff, sizeof(buff), "XVDIVSP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 224: snprintf(buff, sizeof(buff), "XVMAXDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 232: snprintf(buff, sizeof(buff), "XVMINDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 192: snprintf(buff, sizeof(buff), "XVMAXSP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 200: snprintf(buff, sizeof(buff), "XVMINSP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        // Scalar FP arithmetic
        case 32:  snprintf(buff, sizeof(buff), "XSADDDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 40:  snprintf(buff, sizeof(buff), "XSSUBDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 48:  snprintf(buff, sizeof(buff), "XSMULDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 56:  snprintf(buff, sizeof(buff), "XSDIVDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 0:   snprintf(buff, sizeof(buff), "XSADDSP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 8:   snprintf(buff, sizeof(buff), "XSSUBSP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 16:  snprintf(buff, sizeof(buff), "XSMULSP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 24:  snprintf(buff, sizeof(buff), "XSDIVSP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 160: snprintf(buff, sizeof(buff), "XSMAXDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 168: snprintf(buff, sizeof(buff), "XSMINDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        // Compare
        case 35:  snprintf(buff, sizeof(buff), "XSCMPUDP cr%d, %s, %s", rt >> 2, VSn(xa6), VSn(xb6)); return buff;
        case 43:  snprintf(buff, sizeof(buff), "XSCMPODP cr%d, %s, %s", rt >> 2, VSn(xa6), VSn(xb6)); return buff;
        case 99:  snprintf(buff, sizeof(buff), "XVCMPEQDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 115: snprintf(buff, sizeof(buff), "XVCMPGEDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 107: snprintf(buff, sizeof(buff), "XVCMPGTDP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 67:  snprintf(buff, sizeof(buff), "XVCMPEQSP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        case 83:  snprintf(buff, sizeof(buff), "XVCMPGESP %s, %s, %s", VSn(xt6), VSn(xa6), VSn(xb6)); return buff;
        }
        break;
    }

    } // end main switch

    // Fallback: unknown instruction
    snprintf(buff, sizeof(buff), "%08X ???", opcode);
    return buff;
}
