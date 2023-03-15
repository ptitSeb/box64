#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "rv64_printer.h"
#include "debug.h"

typedef struct {
    int8_t rd;
    int8_t rs1;
    int8_t rs2;
    int8_t rs3;
    int32_t imm;
    uint16_t csr;
    char *name;
    bool rvc;
    bool f;
} insn_t;

static const char gpnames[32][9] = {
    "zero", "ra",   "sp",   "gp",   "tp",   "t0_mask",   "t1",   "t2_rip",
    "s0_flags",   "s1",   "a0",   "a1",   "a2",   "a3",   "a4",   "a5",
    "a6_rax",   "a7_rcx",   "s2_rdx",   "s3_rbx",   "s4_rsp",   "s5_rbp",   "s6_rsi",   "s7_rdi",
    "s8_r8",   "s9_r9",   "s10_r10",  "s11_r11",  "t3_r12",   "t4_r13",   "t5_r14",   "t6_r15",
};

static const char fpnames[32][5] = {
    "ft0",  "ft1",  "ft2",  "ft3",  "ft4",  "ft5",  "ft6",  "ft7",
    "fs0",  "fs1",  "fa0",  "fa1",  "fa2",  "fa3",  "fa4",  "fa5",
    "fa6",  "fa7",  "fs2",  "fs3",  "fs4",  "fs5",  "fs6",  "fs7",
    "fs8",  "fs9",  "fs10", "fs11", "ft8",  "ft9",  "ft10", "ft11",
};

#define QUADRANT(data) (((data) >>  0) & 0x3 )

/**
 * normal types
*/
#define OPCODE(data) (((data) >>  2) & 0x1f)
#define RD(data)     (((data) >>  7) & 0x1f)
#define RS1(data)    (((data) >> 15) & 0x1f)
#define RS2(data)    (((data) >> 20) & 0x1f)
#define RS3(data)    (((data) >> 27) & 0x1f)
#define FUNCT2(data) (((data) >> 25) & 0x3 )
#define FUNCT3(data) (((data) >> 12) & 0x7 )
#define FUNCT7(data) (((data) >> 25) & 0x7f)
#define IMM116(data) (((data) >> 26) & 0x3f)

static inline insn_t insn_utype_read(uint32_t data)
{
    return (insn_t) {
        .imm = (int32_t)data & 0xfffff000,
        .rd = RD(data),
    };
}

static inline insn_t insn_itype_read(uint32_t data)
{
    return (insn_t) {
        .imm = (int32_t)data >> 20,
        .rs1 = RS1(data),
        .rd = RD(data),
    };
}

static inline insn_t insn_jtype_read(uint32_t data)
{
    uint32_t imm20   = (data >> 31) & 0x1;
    uint32_t imm101  = (data >> 21) & 0x3ff;
    uint32_t imm11   = (data >> 20) & 0x1;
    uint32_t imm1912 = (data >> 12) & 0xff;

    int32_t imm = (imm20 << 20) | (imm1912 << 12) | (imm11 << 11) | (imm101 << 1);
    imm = (imm << 11) >> 11;

    return (insn_t) {
        .imm = imm,
        .rd = RD(data),
    };
}

static inline insn_t insn_btype_read(uint32_t data)
{
    uint32_t imm12  = (data >> 31) & 0x1;
    uint32_t imm105 = (data >> 25) & 0x3f;
    uint32_t imm41  = (data >>  8) & 0xf;
    uint32_t imm11  = (data >>  7) & 0x1;

    int32_t imm = (imm12 << 12) | (imm11 << 11) |(imm105 << 5) | (imm41 << 1);
    imm = (imm << 19) >> 19;

    return (insn_t) {
        .imm = imm,
        .rs1 = RS1(data),
        .rs2 = RS2(data),
    };
}

static inline insn_t insn_rtype_read(uint32_t data)
{
    return (insn_t) {
        .rs1 = RS1(data),
        .rs2 = RS2(data),
        .rd = RD(data),
    };
}

static inline insn_t insn_stype_read(uint32_t data)
{
    uint32_t imm115 = (data >> 25) & 0x7f;
    uint32_t imm40  = (data >>  7) & 0x1f;

    int32_t imm = (imm115 << 5) | imm40;
    imm = (imm << 20) >> 20;
    return (insn_t) {
        .imm = imm,
        .rs1 = RS1(data),
        .rs2 = RS2(data),
    };
}

static inline insn_t insn_csrtype_read(uint32_t data)
{
    return (insn_t) {
        .csr = data >> 20,
        .rs1 = RS1(data),
        .rd =  RD(data),
    };
}

static inline insn_t insn_fprtype_read(uint32_t data)
{
    return (insn_t) {
        .rs1 = RS1(data),
        .rs2 = RS2(data),
        .rs3 = RS3(data),
        .rd =  RD(data),
    };
}

/**
 * compressed types
*/
#define COPCODE(data)     (((data) >> 13) & 0x7 )
#define CFUNCT1(data)     (((data) >> 12) & 0x1 )
#define CFUNCT2LOW(data)  (((data) >>  5) & 0x3 )
#define CFUNCT2HIGH(data) (((data) >> 10) & 0x3 )
#define RP1(data)         (((data) >>  7) & 0x7 )
#define RP2(data)         (((data) >>  2) & 0x7 )
#define RC1(data)         (((data) >>  7) & 0x1f)
#define RC2(data)         (((data) >>  2) & 0x1f)

static inline insn_t insn_catype_read(uint16_t data)
{
    return (insn_t) {
        .rd = RP1(data) + 8,
        .rs2 = RP2(data) + 8,
        .rvc = true,
    };
}

static inline insn_t insn_crtype_read(uint16_t data)
{
    return (insn_t) {
        .rs1 = RC1(data),
        .rs2 = RC2(data),
        .rvc = true,
    };
}

static inline insn_t insn_citype_read(uint16_t data)
{
    uint32_t imm40 = (data >>  2) & 0x1f;
    uint32_t imm5  = (data >> 12) & 0x1;
    int32_t imm = (imm5 << 5) | imm40;
    imm = (imm << 26) >> 26;

    return (insn_t) {
        .imm = imm,
        .rd = RC1(data),
        .rvc = true,
    };
}

static inline insn_t insn_citype_read2(uint16_t data)
{
    uint32_t imm86 = (data >>  2) & 0x7;
    uint32_t imm43 = (data >>  5) & 0x3;
    uint32_t imm5  = (data >> 12) & 0x1;

    int32_t imm = (imm86 << 6) | (imm43 << 3) | (imm5 << 5);

    return (insn_t) {
        .imm = imm,
        .rd = RC1(data),
        .rvc = true,
    };
}

static inline insn_t insn_citype_read3(uint16_t data)
{
    uint32_t imm5  = (data >>  2) & 0x1;
    uint32_t imm87 = (data >>  3) & 0x3;
    uint32_t imm6  = (data >>  5) & 0x1;
    uint32_t imm4  = (data >>  6) & 0x1;
    uint32_t imm9  = (data >> 12) & 0x1;

    int32_t imm = (imm5 << 5) | (imm87 << 7) | (imm6 << 6) | (imm4 << 4) | (imm9 << 9);
    imm = (imm << 22) >> 22;

    return (insn_t) {
        .imm = imm,
        .rd = RC1(data),
        .rvc = true,
    };
}

static inline insn_t insn_citype_read4(uint16_t data)
{
    uint32_t imm5  = (data >> 12) & 0x1;
    uint32_t imm42 = (data >>  4) & 0x7;
    uint32_t imm76 = (data >>  2) & 0x3;

    int32_t imm = (imm5 << 5) | (imm42 << 2) | (imm76 << 6);

    return (insn_t) {
        .imm = imm,
        .rd = RC1(data),
        .rvc = true,
    };
}

static inline insn_t insn_citype_read5(uint16_t data)
{
    uint32_t imm1612 = (data >>  2) & 0x1f;
    uint32_t imm17   = (data >> 12) & 0x1;

    int32_t imm = (imm1612 << 12) | (imm17 << 17);
    imm = (imm << 14) >> 14;
    return (insn_t) {
        .imm = imm,
        .rd = RC1(data),
        .rvc = true,
    };
}

static inline insn_t insn_cbtype_read(uint16_t data)
{
    uint32_t imm5  = (data >>  2) & 0x1;
    uint32_t imm21 = (data >>  3) & 0x3;
    uint32_t imm76 = (data >>  5) & 0x3;
    uint32_t imm43 = (data >> 10) & 0x3;
    uint32_t imm8  = (data >> 12) & 0x1;

    int32_t imm = (imm8 << 8) | (imm76 << 6) | (imm5 << 5) | (imm43 << 3) | (imm21 << 1);
    imm = (imm << 23) >> 23;

    return (insn_t) {
        .imm = imm,
        .rs1 = RP1(data) + 8,
        .rvc = true,
    };
}

static inline insn_t insn_cbtype_read2(uint16_t data)
{
    uint32_t imm40 = (data >>  2) & 0x1f;
    uint32_t imm5  = (data >> 12) & 0x1;
    int32_t imm = (imm5 << 5) | imm40;
    imm = (imm << 26) >> 26;

    return (insn_t) {
        .imm = imm,
        .rd = RP1(data) + 8,
        .rvc = true,
    };
}

static inline insn_t insn_cstype_read(uint16_t data)
{
    uint32_t imm76 = (data >>  5) & 0x3;
    uint32_t imm53 = (data >> 10) & 0x7;

    int32_t imm = ((imm76 << 6) | (imm53 << 3));

    return (insn_t) {
        .imm = imm,
        .rs1 = RP1(data) + 8,
        .rs2 = RP2(data) + 8,
        .rvc = true,
    };
}

static inline insn_t insn_cstype_read2(uint16_t data)
{
    uint32_t imm6  = (data >>  5) & 0x1;
    uint32_t imm2  = (data >>  6) & 0x1;
    uint32_t imm53 = (data >> 10) & 0x7;

    int32_t imm = ((imm6 << 6) | (imm2 << 2) | (imm53 << 3));

    return (insn_t) {
        .imm = imm,
        .rs1 = RP1(data) + 8,
        .rs2 = RP2(data) + 8,
        .rvc = true,
    };
}

static inline insn_t insn_cjtype_read(uint16_t data)
{
    uint32_t imm5  = (data >>  2) & 0x1;
    uint32_t imm31 = (data >>  3) & 0x7;
    uint32_t imm7  = (data >>  6) & 0x1;
    uint32_t imm6  = (data >>  7) & 0x1;
    uint32_t imm10 = (data >>  8) & 0x1;
    uint32_t imm98 = (data >>  9) & 0x3;
    uint32_t imm4  = (data >> 11) & 0x1;
    uint32_t imm11 = (data >> 12) & 0x1;

    int32_t imm = ((imm5 << 5) | (imm31 << 1) | (imm7 << 7) | (imm6 << 6) |
               (imm10 << 10) | (imm98 << 8) | (imm4 << 4) | (imm11 << 11));
    imm = (imm << 20) >> 20;
    return (insn_t) {
        .imm = imm,
        .rvc = true,
    };
}

static inline insn_t insn_cltype_read(uint16_t data)
{
    uint32_t imm6  = (data >>  5) & 0x1;
    uint32_t imm2  = (data >>  6) & 0x1;
    uint32_t imm53 = (data >> 10) & 0x7;

    int32_t imm = (imm6 << 6) | (imm2 << 2) | (imm53 << 3);

    return (insn_t) {
        .imm = imm,
        .rs1 = RP1(data) + 8,
        .rd  = RP2(data) + 8,
        .rvc = true,
    };
}

static inline insn_t insn_cltype_read2(uint16_t data)
{
    uint32_t imm76 = (data >>  5) & 0x3;
    uint32_t imm53 = (data >> 10) & 0x7;

    int32_t imm = (imm76 << 6) | (imm53 << 3);

    return (insn_t) {
        .imm = imm,
        .rs1 = RP1(data) + 8,
        .rd  = RP2(data) + 8,
        .rvc = true,
    };
}

static inline insn_t insn_csstype_read(uint16_t data)
{
    uint32_t imm86 = (data >>  7) & 0x7;
    uint32_t imm53 = (data >> 10) & 0x7;

    int32_t imm = (imm86 << 6) | (imm53 << 3);

    return (insn_t) {
        .imm = imm,
        .rs2 = RC2(data),
        .rvc = true,
    };
}

static inline insn_t insn_csstype_read2(uint16_t data)
{
    uint32_t imm76 = (data >> 7) & 0x3;
    uint32_t imm52 = (data >> 9) & 0xf;

    int32_t imm = (imm76 << 6) | (imm52 << 2);

    return (insn_t) {
        .imm = imm,
        .rs2 = RC2(data),
        .rvc = true,
    };
}

static inline insn_t insn_ciwtype_read(uint16_t data)
{
    uint32_t imm3  = (data >>  5) & 0x1;
    uint32_t imm2  = (data >>  6) & 0x1;
    uint32_t imm96 = (data >>  7) & 0xf;
    uint32_t imm54 = (data >> 11) & 0x3;

    int32_t imm = (imm3 << 3) | (imm2 << 2) | (imm96 << 6) | (imm54 << 4);

    return (insn_t) {
        .imm = imm,
        .rd = RP2(data) + 8,
        .rvc = true,
    };
}

#define RN(r) insn.f ? fpnames[insn.r] : gpnames[insn.r]

#define PRINT_none() snprintf(buff, sizeof(buff), "%s", insn.name); return buff
#define PRINT_rd_rs1_rs2() snprintf(buff, sizeof(buff), "%s\t%s, %s, %s", insn.name, RN(rd), RN(rs1), RN(rs2)); return buff
#define PRINT_rd_rs1_rs2_rs3() snprintf(buff, sizeof(buff), "%s\t%s, %s, %s, %s", insn.name, RN(rd), RN(rs1), RN(rs2), RN(rs3)); return buff
#define PRINT_rd_rs1_imm() snprintf(buff, sizeof(buff), "%s\t%s, %s, %d", insn.name, RN(rd), RN(rs1), insn.imm); return buff
#define PRINT_rd_rs1_immx() snprintf(buff, sizeof(buff), "%s\t%s, %s, 0x%x", insn.name, RN(rd), RN(rs1), insn.imm); return buff
#define PRINT_rd_imm_rs1() snprintf(buff, sizeof(buff), "%s\t%s, %d(%s)", insn.name, RN(rd), insn.imm, RN(rs1)); return buff
#define PRINT_rs2_imm_rs1() snprintf(buff, sizeof(buff), "%s\t%s, %d(%s)", insn.name, RN(rs2), insn.imm, RN(rs1)); return buff
#define PRINT_rd_imm() snprintf(buff, sizeof(buff), "%s\t%s, %d", insn.name, RN(rd), insn.imm); return buff
#define PRINT_rd_immx() snprintf(buff, sizeof(buff), "%s\t%s, 0x%x", insn.name, RN(rd), insn.imm); return buff
#define PRINT_rs1_rs2_imm() snprintf(buff, sizeof(buff), "%s\t%s, %s, %d", insn.name, RN(rs1), RN(rs2), insn.imm); return buff
#define PRINT_fd_fs1() snprintf(buff, sizeof(buff), "%s\t%s, %s", insn.name, fpnames[insn.rd], fpnames[insn.rs1]); return buff
#define PRINT_xd_fs1() snprintf(buff, sizeof(buff), "%s\t%s, %s", insn.name, gpnames[insn.rd], fpnames[insn.rs1]); return buff
#define PRINT_fd_xs1() snprintf(buff, sizeof(buff), "%s\t%s, %s", insn.name, fpnames[insn.rd], gpnames[insn.rs1]); return buff

// TODO: display csr name
#define PRINT_rd_csr_rs1() snprintf(buff, sizeof(buff), "%s\t%s, %d, %s", insn.name, RN(rd), insn.csr, RN(rs1)); return buff
#define PRINT_rd_csr_uimm() snprintf(buff, sizeof(buff), "%s\t%s, %d, %d", insn.name, RN(rd), insn.csr, (uint32_t)insn.imm); return buff

const char* rv64_print(uint32_t data, uintptr_t addr)
{
    static char buff[200] = {0};

    insn_t insn = { 0 };
    uint32_t quadrant = QUADRANT(data);
    switch (quadrant) {
    case 0x0: {
        uint32_t copcode = COPCODE(data);

        switch (copcode) {
        case 0x0: /* C.ADDI4SPN */
            insn =  insn_ciwtype_read(data);
            insn.rs1 = 2; /* sp */
            insn.name = "addi";
            assert(insn.imm != 0);
            PRINT_rd_rs1_imm();
        case 0x1: /* C.FLD */
            insn =  insn_cltype_read2(data);
            insn.name = "fld";
            insn.f = true;
            PRINT_rd_imm_rs1();
        case 0x2: /* C.LW */
            insn =  insn_cltype_read(data);
            insn.name = "lw";
            PRINT_rd_imm_rs1();
        case 0x3: /* C.LD */
            insn =  insn_cltype_read2(data);
            insn.name = "ld";
            PRINT_rd_imm_rs1();
        case 0x5: /* C.FSD */
            insn =  insn_cstype_read(data);
            insn.name = "fsd";
            insn.f = true;
            PRINT_rs2_imm_rs1();
        case 0x6: /* C.SW */
            insn =  insn_cstype_read2(data);
            insn.name = "sw";
            PRINT_rd_imm_rs1();
        case 0x7: /* C.SD */
            insn =  insn_cstype_read(data);
            insn.name = "sd";
            PRINT_rs2_imm_rs1();
        }
    }
    case 0x1: {
        uint32_t copcode = COPCODE(data);

        switch (copcode) {
        case 0x0: /* C.ADDI */
            insn =  insn_citype_read(data);
            insn.rs1 = insn.rd;
            insn.name = "addi";
            PRINT_rd_rs1_imm();
        case 0x1: /* C.ADDIW */
            insn =  insn_citype_read(data);
            assert(insn.rd != 0);
            insn.rs1 = insn.rd;
            insn.name = "addiw";
            PRINT_rd_rs1_imm();
        case 0x2: /* C.LI */
            insn =  insn_citype_read(data);
            insn.rs1 = 0;
            insn.name = "addi";
            PRINT_rd_rs1_imm();
        case 0x3: {
            int32_t rd = RC1(data);
            if (rd == 2) { /* C.ADDI16SP */
                insn =  insn_citype_read3(data);
                assert(insn.imm != 0);
                insn.rs1 = insn.rd;
                insn.name = "addi";
                PRINT_rd_rs1_imm();
            } else { /* C.LUI */
                insn =  insn_citype_read5(data);
                assert(insn.imm != 0);
                insn.name = "lui";
                PRINT_rd_immx();
            }
        }
        case 0x4: {
            uint32_t cfunct2high = CFUNCT2HIGH(data);

            switch (cfunct2high) {
            case 0x0:   /* C.SRLI */
            case 0x1:   /* C.SRAI */
            case 0x2: { /* C.ANDI */
                insn =  insn_cbtype_read2(data);
                insn.rs1 = insn.rd;

                if (cfunct2high == 0x0) {
                    insn.name = "srli";
                } else if (cfunct2high == 0x1) {
                    insn.name = "srai";
                } else {
                    insn.name = "andi";
                }
                PRINT_rd_rs1_imm();
            }
            case 0x3: {
                uint32_t cfunct1 = CFUNCT1(data);

                switch (cfunct1) {
                case 0x0: {
                    uint32_t cfunct2low = CFUNCT2LOW(data);

                    insn =  insn_catype_read(data);
                    insn.rs1 = insn.rd;

                    switch (cfunct2low) {
                    case 0x0: /* C.SUB */
                        insn.name = "sub";
                        break;
                    case 0x1: /* C.XOR */
                        insn.name = "xor";
                        break;
                    case 0x2: /* C.OR */
                        insn.name = "or";
                        break;
                    case 0x3: /* C.AND */
                        insn.name = "and";
                        break;
                    }
                    break;
                }
                case 0x1: {
                    uint32_t cfunct2low = CFUNCT2LOW(data);

                    insn =  insn_catype_read(data);
                    insn.rs1 = insn.rd;

                    switch (cfunct2low) {
                    case 0x0: /* C.SUBW */
                        insn.name = "subw";
                        break;
                    case 0x1: /* C.ADDW */
                        insn.name = "addw";
                        break;
                    }
                    break;
                }
                }
                PRINT_rd_rs1_rs2();
            }
            }
        }
        case 0x5: /* C.J */
            insn =  insn_cjtype_read(data);
            insn.rd = 0;
            insn.name = "jal";
            PRINT_rd_imm();
        case 0x6: /* C.BEQZ */
        case 0x7: /* C.BNEZ */
            insn =  insn_cbtype_read(data);
            insn.rs2 = 0;
            insn.name = copcode == 0x6 ? "beq" : "bne";
            PRINT_rs1_rs2_imm();
        }
    }
    case 0x2: {
        uint32_t copcode = COPCODE(data);
        switch (copcode) {
        case 0x0: /* C.SLLI */
            insn =  insn_citype_read(data);
            insn.rs1 = insn.rd;
            insn.name = "slli";
            PRINT_rd_rs1_imm();
        case 0x1: /* C.FLDSP */
            insn =  insn_citype_read2(data);
            insn.rs1 = 2; /* sp */
            insn.f = true;
            insn.name = "fld";
            PRINT_rd_imm_rs1();
        case 0x2: /* C.LWSP */
            insn =  insn_citype_read4(data);
            assert(insn.rd != 0);
            insn.rs1 = 2; /* sp */
            insn.name = "lw";
            PRINT_rd_imm_rs1();
        case 0x3: /* C.LDSP */
            insn =  insn_citype_read2(data);
            assert(insn.rd != 0);
            insn.rs1 = 2; /* sp */
            insn.name = "ld";
            PRINT_rd_imm_rs1();
        case 0x4: {
            uint32_t cfunct1 = CFUNCT1(data);

            switch (cfunct1) {
            case 0x0: {
                insn =  insn_crtype_read(data);

                if (insn.rs2 == 0) { /* C.JR */
                    assert(insn.rs1 != 0);
                    insn.rd = 0;
                    insn.name = "jalr";
                    PRINT_rd_imm_rs1();
                } else { /* C.MV */
                    insn.rd = insn.rs1;
                    insn.rs1 = 0;
                    insn.name = "add";
                    PRINT_rd_rs1_rs2();
                }
            }
            case 0x1: {
                insn =  insn_crtype_read(data);
                if (insn.rs1 == 0 && insn.rs2 == 0) { /* C.EBREAK */
                    insn.name = "ebreak";
                    PRINT_none();
                } else if (insn.rs2 == 0) { /* C.JALR */
                    insn.rd = 1; /* ra */
                    insn.name = "jalr";
                    PRINT_rd_imm_rs1();
                } else { /* C.ADD */
                    insn.rd = insn.rs1;
                    insn.name = "add";
                    PRINT_rd_rs1_rs2();
                }
            }
            }
        }
        case 0x5: /* C.FSDSP */
            insn =  insn_csstype_read(data);
            insn.rs1 = 2; /* sp */
            insn.f = true;
            insn.name = "fsd";
            PRINT_rs2_imm_rs1();
        case 0x6: /* C.SWSP */
            insn =  insn_csstype_read2(data);
            insn.rs1 = 2; /* sp */
            insn.name = "sw";
            PRINT_rs2_imm_rs1();
        case 0x7: /* C.SDSP */
            insn =  insn_csstype_read(data);
            insn.rs1 = 2; /* sp */
            insn.name = "sd";
            PRINT_rs2_imm_rs1();
        }
    }
    case 0x3: {
        uint32_t opcode = OPCODE(data);
        switch (opcode) {
        case 0x0: {
            uint32_t funct3 = FUNCT3(data);

            insn =  insn_itype_read(data);
            switch (funct3) {
            case 0x0: /* LB */
                insn.name = "lb";
                break;
            case 0x1: /* LH */
                insn.name = "lh";
                break;
            case 0x2: /* LW */
                insn.name = "lw";
                break;
            case 0x3: /* LD */
                insn.name = "ld";
                break;
            case 0x4: /* LBU */
                insn.name = "lbu";
                break;
            case 0x5: /* LHU */
                insn.name = "lhu";
                break;
            case 0x6: /* LWU */
                insn.name = "lwu";
                break;
            }
            PRINT_rd_imm_rs1();
        }
        case 0x1: {
            uint32_t funct3 = FUNCT3(data);

            insn =  insn_itype_read(data);
            switch (funct3) {
            case 0x2: /* FLW */
                insn.name = "flw";
                insn.f = true;
                break;
            case 0x3: /* FLD */
                insn.name = "fld";
                insn.f = true;
                break;
            }
            PRINT_rd_imm_rs1();
        }
        case 0x3: {
            uint32_t funct3 = FUNCT3(data);

            switch (funct3) {
            case 0x0: { /* FENCE */
                insn.name = "fence";
                // TODO: handle pred succ
                PRINT_none();
            }
            case 0x1: { /* FENCE.I */
                insn.name = "fence.i";
                PRINT_none();
            }
            }
        }
        case 0x4: {
            int hex = 0;
            uint32_t funct3 = FUNCT3(data);

            insn =  insn_itype_read(data);
            switch (funct3) {
            case 0x0: /* ADDI */
                insn.name = "addi";
                break;
            case 0x1: {
                uint32_t imm116 = IMM116(data);
                if (imm116 == 0) { /* SLLI */
                    insn.name = "slli";
                }
                break;
            }
            case 0x2: /* SLTI */
                insn.name = "slti";
                break;
            case 0x3: /* SLTIU */
                insn.name = "sltiu";
                break;
            case 0x4: /* XORI */
                insn.name = "xori";
                hex = 1;
                break;
            case 0x5: {
                uint32_t imm116 = IMM116(data);

                if (imm116 == 0x0) { /* SRLI */
                    insn.name = "srli";
                } else if (imm116 == 0x10) { /* SRAI */
                    insn.name = "srai";
                    insn.imm&=0b111111;
                }
                break;
            }
            case 0x6: /* ORI */
                insn.name = "ori";
                hex = 1;
                break;
            case 0x7: /* ANDI */
                insn.name = "andi";
                hex = 1;
                break;
            }
            if(hex) {
                PRINT_rd_rs1_immx();
            } else {
                PRINT_rd_rs1_imm();
            }
        }
        case 0x5: /* AUIPC */
            insn =  insn_utype_read(data);
            insn.name = "auipc";
            PRINT_rd_imm();
        case 0x6: {
            uint32_t funct3 = FUNCT3(data);
            uint32_t funct7 = FUNCT7(data);

            insn =  insn_itype_read(data);

            switch (funct3) {
            case 0x0: /* ADDIW */
                insn.name = "addiw";
                break;
            case 0x1: /* SLLIW */
                assert(funct7 == 0);
                insn.name = "slliw";
                break;
            case 0x5: {
                switch (funct7) {
                case 0x0: /* SRLIW */
                    insn.name = "srliw";
                    break;
                case 0x20: /* SRAIW */
                    insn.name = "sraiw";
                    insn.imm&=0b111111;
                    break;
                }
            }
            }
            PRINT_rd_rs1_imm();
        }
        case 0x8: {
            uint32_t funct3 = FUNCT3(data);

            insn =  insn_stype_read(data);
            switch (funct3) {
            case 0x0: /* SB */
                insn.name = "sb";
                break;
            case 0x1: /* SH */
                insn.name = "sh";
                break;
            case 0x2: /* SW */
                insn.name = "sw";
                break;
            case 0x3: /* SD */
                insn.name = "sd";
                break;
            }
            PRINT_rs2_imm_rs1();
        }
        case 0x9: {
            uint32_t funct3 = FUNCT3(data);

            insn =  insn_stype_read(data);
            switch (funct3) {
            case 0x2: /* FSW */
                insn.name = "fsw";
                insn.f = true;
                break;
            case 0x3: /* FSD */
                insn.name = "fsd";
                insn.f = true;
                break;
            }
            PRINT_rs2_imm_rs1();
        }
        case 0xc: {
            insn =  insn_rtype_read(data);

            uint32_t funct3 = FUNCT3(data);
            uint32_t funct7 = FUNCT7(data);

            switch (funct7) {
            case 0x0: {
                switch (funct3) {
                case 0x0: /* ADD */
                    insn.name = "add";
                    break;
                case 0x1: /* SLL */
                    insn.name = "sll";
                    break;
                case 0x2: /* SLT */
                    insn.name = "slt";
                    break;
                case 0x3: /* SLTU */
                    insn.name = "sltu";
                    break;
                case 0x4: /* XOR */
                    insn.name = "xor";
                    break;
                case 0x5: /* SRL */
                    insn.name = "srl";
                    break;
                case 0x6: /* OR */
                    insn.name = "or";
                    break;
                case 0x7: /* AND */
                    insn.name = "and";
                    break;
                }
            }
            break;
            case 0x1: {
                switch (funct3) {
                case 0x0: /* MUL */
                    insn.name = "mul";
                    break;
                case 0x1: /* MULH */
                    insn.name = "mulh";
                    break;
                case 0x2: /* MULHSU */
                    insn.name = "mulhsu";
                    break;
                case 0x3: /* MULHU */
                    insn.name = "mulhu";
                    break;
                case 0x4: /* DIV */
                    insn.name = "div";
                    break;
                case 0x5: /* DIVU */
                    insn.name = "divu";
                    break;
                case 0x6: /* REM */
                    insn.name = "rem";
                    break;
                case 0x7: /* REMU */
                    insn.name = "remu";
                    break;
                }
            }
            break;
            case 0x20: {
                switch (funct3) {
                case 0x0: /* SUB */
                    insn.name = "sub";
                    break;
                case 0x5: /* SRA */
                    insn.name = "sra";
                    break;
                }
            }
            break;
            }
            PRINT_rd_rs1_rs2();
        }
        case 0xd: /* LUI */
            insn =  insn_utype_read(data);
            insn.name = "lui";
            PRINT_rd_immx();
        case 0xe: {
            insn =  insn_rtype_read(data);

            uint32_t funct3 = FUNCT3(data);
            uint32_t funct7 = FUNCT7(data);

            switch (funct7) {
            case 0x0: {
                switch (funct3) {
                case 0x0: /* ADDW */
                    insn.name = "addw";
                    break;
                case 0x1: /* SLLW */
                    insn.name = "sllw";
                    break;
                case 0x5: /* SRLW */
                    insn.name = "srlw";
                    break;
                }
            }
            break;
            case 0x1: {
                switch (funct3) {
                case 0x0: /* MULW */
                    insn.name = "mulw";
                    break;
                case 0x4: /* DIVW */
                    insn.name = "divw";
                    break;
                case 0x5: /* DIVUW */
                    insn.name = "divuw";
                    break;
                case 0x6: /* REMW */
                    insn.name = "remw";
                    break;
                case 0x7: /* REMUW */
                    insn.name = "remuw";
                    break;
                }
            }
            break;
            case 0x20: {
                switch (funct3) {
                case 0x0: /* SUBW */
                    insn.name = "subw";
                    break;
                case 0x5: /* SRAW */
                    insn.name = "sraw";
                    break;
                }
            }
            break;
            }
            PRINT_rd_rs1_rs2();
        }
        case 0x10: {
            uint32_t funct2 = FUNCT2(data);

            insn =  insn_fprtype_read(data);
            switch (funct2) {
            case 0x0: /* FMADD.S */
                insn.name = "fmadd.s";
                insn.f = true;
                break;
            case 0x1: /* FMADD.D */
                insn.name = "fmadd.d";
                insn.f = true;
                break;
            }
            PRINT_rd_rs1_rs2();
        }
        case 0x11: {
            uint32_t funct2 = FUNCT2(data);

            insn =  insn_fprtype_read(data);
            switch (funct2) {
            case 0x0: /* FMSUB.S */
                insn.name = "fmsub.s";
                insn.f = true;
                break;
            case 0x1: /* FMSUB.D */
                insn.name = "fmsub.d";
                insn.f = true;
                break;
            }
            PRINT_rd_rs1_rs2();
        }
        case 0x12: {
            uint32_t funct2 = FUNCT2(data);

            insn =  insn_fprtype_read(data);
            switch (funct2) {
            case 0x0: /* FNMSUB.S */
                insn.name = "fnmsub.s";
                insn.f = true;
                break;
            case 0x1: /* FNMSUB.D */
                insn.name = "fnmsub.d";
                insn.f = true;
                break;
            }
            PRINT_rd_rs1_rs2_rs3();
        }
        case 0x13: {
            uint32_t funct2 = FUNCT2(data);

            insn =  insn_fprtype_read(data);
            switch (funct2) {
            case 0x0: /* FNMADD.S */
                insn.name = "fnmadd.s";
                insn.f = true;
                break;
            case 0x1: /* FNMADD.D */
                insn.name = "fnmadd.d";
                insn.f = true;
                break;
            }
            PRINT_rd_rs1_rs2_rs3();
        }
        case 0x14: {
            uint32_t funct7 = FUNCT7(data);

            insn =  insn_rtype_read(data);
            insn.f = true;
            switch (funct7) {
            case 0x0:  /* FADD.S */
                insn.name = "fadd.s";
                PRINT_rd_rs1_rs2();
            case 0x1:  /* FADD.D */
                insn.name = "fadd.d";
                PRINT_rd_rs1_rs2();
            case 0x4:  /* FSUB.S */
                insn.name = "fsub.s";
                PRINT_rd_rs1_rs2();
            case 0x5:  /* FSUB.D */
                insn.name = "fsub.d";
                PRINT_rd_rs1_rs2();
            case 0x8:  /* FMUL.S */
                insn.name = "fmul.s";
                PRINT_rd_rs1_rs2();
            case 0x9:  /* FMUL.D */
                insn.name = "fmul.d";
                PRINT_rd_rs1_rs2();
            case 0xc:  /* FDIV.S */
                insn.name = "fdiv.s";
                PRINT_rd_rs1_rs2();
            case 0xd:  /* FDIV.D */
                insn.name = "fdiv.d";
                PRINT_rd_rs1_rs2();
            case 0x10: {
                uint32_t funct3 = FUNCT3(data);

                switch (funct3) {
                case 0x0: /* FSGNJ.S */
                    insn.name = "fsgnj.s";
                    break;
                case 0x1: /* FSGNJN.S */
                    insn.name = "fsgnjn.s";
                    break;
                case 0x2: /* FSGNJX.S */
                    insn.name = "fsgnjx.s";
                    break;
                }
                PRINT_rd_rs1_rs2();
            }
            case 0x11: {
                uint32_t funct3 = FUNCT3(data);

                switch (funct3) {
                case 0x0: /* FSGNJ.D */
                    insn.name = "fsgnj.d";
                    break;
                case 0x1: /* FSGNJN.D */
                    insn.name = "fsgnjn.d";
                    break;
                case 0x2: /* FSGNJX.D */
                    insn.name = "fsgnjx.d";
                    break;
                }
                PRINT_rd_rs1_rs2();
            }
            case 0x14: {
                uint32_t funct3 = FUNCT3(data);

                switch (funct3) {
                case 0x0: /* FMIN.S */
                    insn.name = "fmin.s";
                    break;
                case 0x1: /* FMAX.S */
                    insn.name = "fmax.s";
                    break;
                }
                PRINT_rd_rs1_rs2();
            }
            case 0x15: {
                uint32_t funct3 = FUNCT3(data);

                switch (funct3) {
                case 0x0: /* FMIN.D */
                    insn.name = "fmin.d";
                    break;
                case 0x1: /* FMAX.D */
                    insn.name = "fmax.d";
                    break;
                }
                PRINT_rd_rs1_rs2();
            }
            case 0x20: /* FCVT.S.D */
                assert(RS2(data) == 1);
                insn.name = "fcvt.s.d";
                PRINT_fd_fs1();
            case 0x21: /* FCVT.D.S */
                assert(RS2(data) == 0);
                insn.name = "fcvt.d.s";
                PRINT_fd_fs1();
            case 0x2c: /* FSQRT.S */
                assert(insn.rs2 == 0);
                insn.name = "fsqrt.s";
                PRINT_fd_fs1();
            case 0x2d: /* FSQRT.D */
                assert(insn.rs2 == 0);
                insn.name = "fsqrt.d";
                PRINT_fd_fs1();
            case 0x50: {
                uint32_t funct3 = FUNCT3(data);

                switch (funct3) {
                case 0x0: /* FLE.S */
                    insn.name = "fle.s";
                    break;
                case 0x1: /* FLT.S */
                    insn.name = "flt.s";
                    break;
                case 0x2: /* FEQ.S */
                    insn.name = "feq.s";
                    break;
                }
                PRINT_rd_rs1_rs2();
            }
            case 0x51: {
                uint32_t funct3 = FUNCT3(data);

                switch (funct3) {
                case 0x0: /* FLE.D */
                    insn.name = "fle.d";
                    break;
                case 0x1: /* FLT.D */
                    insn.name = "flt.d";
                    break;
                case 0x2: /* FEQ.D */
                    insn.name = "feq.d";
                    break;
                }
                PRINT_rd_rs1_rs2();
            }
            case 0x60: {
                uint32_t rs2 = RS2(data);

                switch (rs2) {
                case 0x0: /* FCVT.W.S */
                    insn.name = "fcvt.w.s";
                    break;
                case 0x1: /* FCVT.WU.S */
                    insn.name = "fcvt.wu.s";
                    break;
                case 0x2: /* FCVT.L.S */
                    insn.name = "fcvt.l.s";
                    break;
                case 0x3: /* FCVT.LU.S */
                    insn.name = "fcvt.lu.s";
                    break;
                }
                PRINT_xd_fs1();
            }
            case 0x61: {
                uint32_t rs2 = RS2(data);

                switch (rs2) {
                case 0x0: /* FCVT.W.D */
                    insn.name = "fcvt.w.d";
                    break;
                case 0x1: /* FCVT.WU.D */
                    insn.name = "fcvt.wu.d";
                    break;
                case 0x2: /* FCVT.L.D */
                    insn.name = "fcvt.l.d";
                    break;
                case 0x3: /* FCVT.LU.D */
                    insn.name = "fcvt.lu.d";
                    break;
                }
                PRINT_xd_fs1();
            }
            case 0x68: {
                uint32_t rs2 = RS2(data);

                switch (rs2) {
                case 0x0: /* FCVT.S.W */
                    insn.name = "fcvt.s.w";
                    break;
                case 0x1: /* FCVT.S.WU */
                    insn.name = "fcvt.s.wu";
                    break;
                case 0x2: /* FCVT.S.L */
                    insn.name = "fcvt.s.l";
                    break;
                case 0x3: /* FCVT.S.LU */
                    insn.name = "fcvt.s.lu";
                    break;
                }
                PRINT_fd_xs1();
            }
            case 0x69: {
                uint32_t rs2 = RS2(data);

                switch (rs2) {
                case 0x0: /* FCVT.D.W */
                    insn.name = "fcvt.d.w";
                    break;
                case 0x1: /* FCVT.D.WU */
                    insn.name = "fcvt.d.wu";
                    break;
                case 0x2: /* FCVT.D.L */
                    insn.name = "fcvt.d.l";
                    break;
                case 0x3: /* FCVT.D.LU */
                    insn.name = "fcvt.d.lu";
                    break;
                }
                PRINT_xd_fs1();
            }
            case 0x70: {
                assert(RS2(data) == 0);
                uint32_t funct3 = FUNCT3(data);

                switch (funct3) {
                case 0x0: /* FMV.X.W */
                    insn.name = "fmv.x.w";
                    break;
                case 0x1: /* FCLASS.S */
                    insn.name = "fclass.s";
                    break;
                }
                PRINT_xd_fs1();
            }
            case 0x71: {
                assert(RS2(data) == 0);
                uint32_t funct3 = FUNCT3(data);

                switch (funct3) {
                case 0x0: /* FMV.X.D */
                    insn.name = "fmv.x.d";
                    break;
                case 0x1: /* FCLASS.D */
                    insn.name = "fclass.d";
                    break;
                }
                PRINT_xd_fs1();
            }
            case 0x78: /* FMV_W_X */
                assert(RS2(data) == 0 && FUNCT3(data) == 0);
                insn.name = "fmv.w.x";
                PRINT_fd_xs1();
            case 0x79: /* FMV_D_X */
                assert(RS2(data) == 0 && FUNCT3(data) == 0);
                insn.name = "fmv.d.x";
                PRINT_fd_xs1();
            }
        }
        case 0x18: {
            insn =  insn_btype_read(data);

            uint32_t funct3 = FUNCT3(data);
            switch (funct3) {
            case 0x0: /* BEQ */
                insn.name = "beq";
                break;
            case 0x1: /* BNE */
                insn.name = "bne";
                break;
            case 0x4: /* BLT */
                insn.name = "blt";
                break;
            case 0x5: /* BGE */
                insn.name = "bge";
                break;
            case 0x6: /* BLTU */
                insn.name = "bltu";
                break;
            case 0x7: /* BGEU */
                insn.name = "bgeu";
                break;
            }

            PRINT_rs1_rs2_imm();
        }
        case 0x19: /* JALR */
            insn =  insn_itype_read(data);
            insn.name = "jalr";
            PRINT_rd_imm_rs1();
        case 0x1b: /* JAL */
            insn =  insn_jtype_read(data);
            insn.name = "jal";
            PRINT_rd_imm();
        case 0x1c: {
            if (data == 0x73) { /* ECALL */
                insn.name = "ecall";
                PRINT_none();
            }

            uint32_t funct3 = FUNCT3(data);
            insn =  insn_csrtype_read(data);
            switch(funct3) {
            case 0x1: /* CSRRW */
                insn.name = "csrrw";
                PRINT_rd_csr_rs1();
            case 0x2: /* CSRRS */
                insn.name = "csrrs";
                PRINT_rd_csr_rs1();
            case 0x3: /* CSRRC */
                insn.name = "csrrc";
                PRINT_rd_csr_rs1();
            case 0x5: /* CSRRWI */
                insn.name = "csrrwi";
                PRINT_rd_csr_uimm();
            case 0x6: /* CSRRSI */
                insn.name = "csrrsi";
                PRINT_rd_csr_uimm();
            case 0x7: /* CSRRCI */
                insn.name = "csrrci";
                PRINT_rd_csr_uimm();
            }
        }
        }
    }
    }

    snprintf(buff, sizeof(buff), "%08X ???", __builtin_bswap32(data));
    return buff;
}
