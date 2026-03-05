#ifndef _LA64_PRINTER_H_
#define _LA64_PRINTER_H_

#include <stdint.h>
#include <stddef.h>

typedef enum {
    LA_ZERO = 0,
    LA_RA,
    LA_TP,
    LA_SP,
    LA_A0,
    LA_A1,
    LA_A2,
    LA_A3,
    LA_A4,
    LA_A5,
    LA_A6,
    LA_A7,
    LA_T0,
    LA_T1,
    LA_T2,
    LA_T3,
    LA_T4,
    LA_T5,
    LA_T6,
    LA_T7,
    LA_T8,
    LA_RX,
    LA_FP,
    LA_S0,
    LA_S1,
    LA_S2,
    LA_S3,
    LA_S4,
    LA_S5,
    LA_S6,
    LA_S7,
    LA_S8,
    LA_R0 = 0,
    LA_R1,
    LA_R2,
    LA_R3,
    LA_R4,
    LA_R5,
    LA_R6,
    LA_R7,
    LA_R8,
    LA_R9,
    LA_R10,
    LA_R11,
    LA_R12,
    LA_R13,
    LA_R14,
    LA_R15,
    LA_R16,
    LA_R17,
    LA_R18,
    LA_R19,
    LA_R20,
    LA_R21,
    LA_R22,
    LA_R23,
    LA_R24,
    LA_R25,
    LA_R26,
    LA_R27,
    LA_R28,
    LA_R29,
    LA_R30,
    LA_R31,
    LA_GPR_COUNT
} la_gpr_t;

typedef enum {
    LA_FA0 = 0,
    LA_FA1,
    LA_FA2,
    LA_FA3,
    LA_FA4,
    LA_FA5,
    LA_FA6,
    LA_FA7,
    LA_FT0,
    LA_FT1,
    LA_FT2,
    LA_FT3,
    LA_FT4,
    LA_FT5,
    LA_FT6,
    LA_FT7,
    LA_FT8,
    LA_FT9,
    LA_FT10,
    LA_FT11,
    LA_FT12,
    LA_FT13,
    LA_FT14,
    LA_FT15,
    LA_FS0,
    LA_FS1,
    LA_FS2,
    LA_FS3,
    LA_FS4,
    LA_FS5,
    LA_FS6,
    LA_FS7,
    LA_F0 = 0,
    LA_F1,
    LA_F2,
    LA_F3,
    LA_F4,
    LA_F5,
    LA_F6,
    LA_F7,
    LA_F8,
    LA_F9,
    LA_F10,
    LA_F11,
    LA_F12,
    LA_F13,
    LA_F14,
    LA_F15,
    LA_F16,
    LA_F17,
    LA_F18,
    LA_F19,
    LA_F20,
    LA_F21,
    LA_F22,
    LA_F23,
    LA_F24,
    LA_F25,
    LA_F26,
    LA_F27,
    LA_F28,
    LA_F29,
    LA_F30,
    LA_F31,
    LA_FPR_COUNT
} la_fpr_t;

typedef enum {
    LA_V0 = 0,
    LA_V1,
    LA_V2,
    LA_V3,
    LA_V4,
    LA_V5,
    LA_V6,
    LA_V7,
    LA_V8,
    LA_V9,
    LA_V10,
    LA_V11,
    LA_V12,
    LA_V13,
    LA_V14,
    LA_V15,
    LA_V16,
    LA_V17,
    LA_V18,
    LA_V19,
    LA_V20,
    LA_V21,
    LA_V22,
    LA_V23,
    LA_V24,
    LA_V25,
    LA_V26,
    LA_V27,
    LA_V28,
    LA_V29,
    LA_V30,
    LA_V31,
    LA_VSR_COUNT
} la_vpr_t;

typedef enum {
    LA_XV0 = 0,
    LA_XV1,
    LA_XV2,
    LA_XV3,
    LA_XV4,
    LA_XV5,
    LA_XV6,
    LA_XV7,
    LA_XV8,
    LA_XV9,
    LA_XV10,
    LA_XV11,
    LA_XV12,
    LA_XV13,
    LA_XV14,
    LA_XV15,
    LA_XV16,
    LA_XV17,
    LA_XV18,
    LA_XV19,
    LA_XV20,
    LA_XV21,
    LA_XV22,
    LA_XV23,
    LA_XV24,
    LA_XV25,
    LA_XV26,
    LA_XV27,
    LA_XV28,
    LA_XV29,
    LA_XV30,
    LA_XV31,
    LA_XVSR_COUNT
} la_xvpr_t;

typedef enum {
    LA_FCC0 = 0,
    LA_FCC1,
    LA_FCC2,
    LA_FCC3,
    LA_FCC4,
    LA_FCC5,
    LA_FCC6,
    LA_FCC7,
    LA_FCC_COUNT
} la_fcc_t;

typedef enum {
    LA_SCR0 = 0,
    LA_SCR1,
    LA_SCR2,
    LA_SCR3,
    LA_SCR_COUNT
} la_scr_t;

typedef enum {
    LA_FCSR0 = 0,
    LA_FCSR1,
    LA_FCSR2,
    LA_FCSR3,
    LA_FCSR_COUNT
} la_fcsr_t;

typedef enum {
    LA_OP_GPR,
    LA_OP_FPR,
    LA_OP_VPR,
    LA_OP_XVPR,
    LA_OP_FCC,
    LA_OP_SCR,
    LA_OP_FCSR,
    LA_OP_SIMM,
    LA_OP_UIMM,
} la_operand_kind_t;

typedef struct {
    la_operand_kind_t kind;
    union {
        la_gpr_t gpr;
        la_fpr_t fpr;
        la_vpr_t vpr;
        la_xvpr_t xvpr;
        la_fcc_t fcc;
        la_scr_t scr;
        la_fcsr_t fcsr;
        int32_t simm;
        uint32_t uimm;
    };
} la_operand_t;

#define LA_MAX_OPERANDS 4

typedef struct {
    size_t offset;
    uint32_t encoding;
    const char* mnemonic;
    int operand_count;
    la_operand_t operands[LA_MAX_OPERANDS];
} lagoon_insn_t;

void la_disasm_one(uint32_t word, lagoon_insn_t* insn);
void la_insn_to_str(const lagoon_insn_t* insn, char* buf, size_t buf_size);

const char* la64_print(uint32_t opcode, uint64_t addr);

#endif //_LA64_PRINTER_H_
