#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "la64_printer.h"
#include "debug.h"

static const char* Xt[] = { "xZR", "r1", "r2", "sp", "xRDI", "xRSI", "xRDX", "xRCX", "xR8", "xR9", "xRBX", "xRSP", "xRAX", "xRBP", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "r21", "xSavedSP", "xR10", "xR11", "xR12", "xR13", "xR14", "xR15", "xRIP", "xFlags", "xEmu" };
static const char* Ft[] = { "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7", "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", "ft8", "ft9", "ft10", "ft11", "ft12", "ft13", "ft14", "ft15", "fs0", "fs1", "fs2", "fs3", "fs4", "fs5", "fs6", "fs7" };
static const char* Vt[] = { "vra0", "vra1", "vra2", "vra3", "vra4", "vra5", "vra6", "vra7", "vrt0", "vrt1", "vrt2", "vrt3", "vrt4", "vrt5", "vrt6", "vrt7", "vrt8", "vrt9", "vrt10", "vrt11", "vrt12", "vrt13", "vrt14", "vrt15", "vrs0", "vrs1", "vrs2", "vrs3", "vrs4", "vrs5", "vrs6", "vrs7" };
static const char* XVt[] = { "xvra0", "xvra1", "xvra2", "xvra3", "xvra4", "xvra5", "xvra6", "xvra7", "xvrt0", "xvrt1", "xvrt2", "xvrt3", "xvrt4", "xvrt5", "xvrt6", "xvrt7", "xvrt8", "xvrt9", "xvrt10", "xvrt11", "xvrt12", "xvrt13", "xvrt14", "xvrt15", "xvrs0", "xvrs1", "xvrs2", "xvrs3", "xvrs4", "xvrs5", "xvrs6", "xvrs7" };

void la_disasm_one(uint32_t word, lagoon_insn_t* insn)
{
    insn->offset = 0;
    insn->encoding = word;
    insn->mnemonic = NULL;
    insn->operand_count = 0;

    //// ANCHOR: disasm start

    if ((word & 0xffff8000u) == 0x00300000u) {
        insn->mnemonic = "adc.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00318000u) {
        insn->mnemonic = "adc.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00308000u) {
        insn->mnemonic = "adc.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00310000u) {
        insn->mnemonic = "adc.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00108000u) {
        insn->mnemonic = "add.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00100000u) {
        insn->mnemonic = "add.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x02c00000u) {
        insn->mnemonic = "addi.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x02800000u) {
        insn->mnemonic = "addi.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffff8000u) == 0x00298000u) {
        insn->mnemonic = "addu12i.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x00290000u) {
        insn->mnemonic = "addu12i.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xfc000000u) == 0x10000000u) {
        insn->mnemonic = "addu16i.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xffff) << 16 >> 16);
        return;
    }
    if ((word & 0xfffe0000u) == 0x002c0000u) {
        insn->mnemonic = "alsl.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 15) & 0x3) + 1);
        return;
    }
    if ((word & 0xfffe0000u) == 0x00040000u) {
        insn->mnemonic = "alsl.w";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 15) & 0x3) + 1);
        return;
    }
    if ((word & 0xfffe0000u) == 0x00060000u) {
        insn->mnemonic = "alsl.wu";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 15) & 0x3) + 1);
        return;
    }
    if ((word & 0xffff8000u) == 0x385d0000u) {
        insn->mnemonic = "amadd.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38618000u) {
        insn->mnemonic = "amadd.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x385f0000u) {
        insn->mnemonic = "amadd_db.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x386a8000u) {
        insn->mnemonic = "amadd_db.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x385f8000u) {
        insn->mnemonic = "amadd_db.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x386a0000u) {
        insn->mnemonic = "amadd_db.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x385d8000u) {
        insn->mnemonic = "amadd.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38610000u) {
        insn->mnemonic = "amadd.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38628000u) {
        insn->mnemonic = "amand.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x386b8000u) {
        insn->mnemonic = "amand_db.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x386b0000u) {
        insn->mnemonic = "amand_db.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38620000u) {
        insn->mnemonic = "amand.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38580000u) {
        insn->mnemonic = "amcas.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38598000u) {
        insn->mnemonic = "amcas.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x385a0000u) {
        insn->mnemonic = "amcas_db.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x385b8000u) {
        insn->mnemonic = "amcas_db.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x385a8000u) {
        insn->mnemonic = "amcas_db.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x385b0000u) {
        insn->mnemonic = "amcas_db.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38588000u) {
        insn->mnemonic = "amcas.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38590000u) {
        insn->mnemonic = "amcas.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38658000u) {
        insn->mnemonic = "ammax.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x386e8000u) {
        insn->mnemonic = "ammax_db.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38708000u) {
        insn->mnemonic = "ammax_db.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x386e0000u) {
        insn->mnemonic = "ammax_db.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38700000u) {
        insn->mnemonic = "ammax_db.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38678000u) {
        insn->mnemonic = "ammax.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38650000u) {
        insn->mnemonic = "ammax.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38670000u) {
        insn->mnemonic = "ammax.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38668000u) {
        insn->mnemonic = "ammin.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x386f8000u) {
        insn->mnemonic = "ammin_db.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38718000u) {
        insn->mnemonic = "ammin_db.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x386f0000u) {
        insn->mnemonic = "ammin_db.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38710000u) {
        insn->mnemonic = "ammin_db.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38688000u) {
        insn->mnemonic = "ammin.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38660000u) {
        insn->mnemonic = "ammin.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38680000u) {
        insn->mnemonic = "ammin.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38638000u) {
        insn->mnemonic = "amor.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x386c8000u) {
        insn->mnemonic = "amor_db.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x386c0000u) {
        insn->mnemonic = "amor_db.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38630000u) {
        insn->mnemonic = "amor.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x385c0000u) {
        insn->mnemonic = "amswap.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38608000u) {
        insn->mnemonic = "amswap.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x385e0000u) {
        insn->mnemonic = "amswap_db.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38698000u) {
        insn->mnemonic = "amswap_db.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x385e8000u) {
        insn->mnemonic = "amswap_db.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38690000u) {
        insn->mnemonic = "amswap_db.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x385c8000u) {
        insn->mnemonic = "amswap.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38600000u) {
        insn->mnemonic = "amswap.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38648000u) {
        insn->mnemonic = "amxor.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x386d8000u) {
        insn->mnemonic = "amxor_db.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x386d0000u) {
        insn->mnemonic = "amxor_db.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38640000u) {
        insn->mnemonic = "amxor.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00148000u) {
        insn->mnemonic = "and";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x03400000u) {
        insn->mnemonic = "andi";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xfff));
        return;
    }
    if ((word & 0xffff8000u) == 0x00168000u) {
        insn->mnemonic = "andn";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8010u) == 0x00380010u) {
        insn->mnemonic = "armadc.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x00370010u) {
        insn->mnemonic = "armadd.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x00390010u) {
        insn->mnemonic = "armand.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xfffc03e0u) == 0x005c0040u) {
        insn->mnemonic = "armmfflag";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffffc01fu) == 0x003fc01eu) {
        insn->mnemonic = "armmov.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc01fu) == 0x003fc01du) {
        insn->mnemonic = "armmov.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x00364000u) {
        insn->mnemonic = "armmove";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffc03e0u) == 0x005c0060u) {
        insn->mnemonic = "armmtflag";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffffc01fu) == 0x003fc01cu) {
        insn->mnemonic = "armnot.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x00398010u) {
        insn->mnemonic = "armor.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x003c0010u) {
        insn->mnemonic = "armrotr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x003e0010u) {
        insn->mnemonic = "armrotri.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffffc01fu) == 0x003fc01fu) {
        insn->mnemonic = "armrrx.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x00388010u) {
        insn->mnemonic = "armsbc.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x003a8010u) {
        insn->mnemonic = "armsll.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x003c8010u) {
        insn->mnemonic = "armslli.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x003b8010u) {
        insn->mnemonic = "armsra.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x003d8010u) {
        insn->mnemonic = "armsrai.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x003b0010u) {
        insn->mnemonic = "armsrl.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x003d0010u) {
        insn->mnemonic = "armsrli.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x00378010u) {
        insn->mnemonic = "armsub.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff8010u) == 0x003a0010u) {
        insn->mnemonic = "armxor.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 0) & 0xf));
        return;
    }
    if ((word & 0xffff801fu) == 0x00018000u) {
        insn->mnemonic = "asrtgt.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x00010000u) {
        insn->mnemonic = "asrtle.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfc000000u) == 0x50000000u) {
        insn->mnemonic = "b";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_SIMM;
        insn->operands[0].simm = ((int32_t)(int32_t)((((word >> 0) & 0x3ff) << 16) | ((word >> 10) & 0xffff)) << 6 >> 6 << 2);
        return;
    }
    if ((word & 0xfc000300u) == 0x48000000u) {
        insn->mnemonic = "bceqz";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 5) & 0x7));
        insn->operands[1].kind = LA_OP_SIMM;
        insn->operands[1].simm = ((int32_t)(int32_t)((((word >> 0) & 0x1f) << 16) | ((word >> 10) & 0xffff)) << 11 >> 11 << 2);
        return;
    }
    if ((word & 0xfc000300u) == 0x48000100u) {
        insn->mnemonic = "bcnez";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 5) & 0x7));
        insn->operands[1].kind = LA_OP_SIMM;
        insn->operands[1].simm = ((int32_t)(int32_t)((((word >> 0) & 0x1f) << 16) | ((word >> 10) & 0xffff)) << 11 >> 11 << 2);
        return;
    }
    if ((word & 0xfc000000u) == 0x58000000u) {
        insn->mnemonic = "beq";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 0) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xffff) << 16 >> 16 << 2);
        return;
    }
    if ((word & 0xfc000000u) == 0x40000000u) {
        insn->mnemonic = "beqz";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_SIMM;
        insn->operands[1].simm = ((int32_t)(int32_t)((((word >> 0) & 0x1f) << 16) | ((word >> 10) & 0xffff)) << 11 >> 11 << 2);
        return;
    }
    if ((word & 0xfc000000u) == 0x64000000u) {
        insn->mnemonic = "bge";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 0) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xffff) << 16 >> 16 << 2);
        return;
    }
    if ((word & 0xfc000000u) == 0x6c000000u) {
        insn->mnemonic = "bgeu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 0) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xffff) << 16 >> 16 << 2);
        return;
    }
    if ((word & 0xfffffc00u) == 0x00004800u) {
        insn->mnemonic = "bitrev.4b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00004c00u) {
        insn->mnemonic = "bitrev.8b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00005400u) {
        insn->mnemonic = "bitrev.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00005000u) {
        insn->mnemonic = "bitrev.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfc000000u) == 0x54000000u) {
        insn->mnemonic = "bl";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_SIMM;
        insn->operands[0].simm = ((int32_t)(int32_t)((((word >> 0) & 0x3ff) << 16) | ((word >> 10) & 0xffff)) << 6 >> 6 << 2);
        return;
    }
    if ((word & 0xfc000000u) == 0x60000000u) {
        insn->mnemonic = "blt";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 0) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xffff) << 16 >> 16 << 2);
        return;
    }
    if ((word & 0xfc000000u) == 0x68000000u) {
        insn->mnemonic = "bltu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 0) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xffff) << 16 >> 16 << 2);
        return;
    }
    if ((word & 0xfc000000u) == 0x5c000000u) {
        insn->mnemonic = "bne";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 0) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xffff) << 16 >> 16 << 2);
        return;
    }
    if ((word & 0xfc000000u) == 0x44000000u) {
        insn->mnemonic = "bnez";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_SIMM;
        insn->operands[1].simm = ((int32_t)(int32_t)((((word >> 0) & 0x1f) << 16) | ((word >> 10) & 0xffff)) << 11 >> 11 << 2);
        return;
    }
    if ((word & 0xffff8000u) == 0x002a0000u) {
        insn->mnemonic = "break";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_UIMM;
        insn->operands[0].uimm = (((word >> 0) & 0x7fff));
        return;
    }
    if ((word & 0xffc00000u) == 0x00800000u) {
        insn->mnemonic = "bstrins.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 16) & 0x3f));
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffe08000u) == 0x00600000u) {
        insn->mnemonic = "bstrins.w";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 16) & 0x1f));
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x00c00000u) {
        insn->mnemonic = "bstrpick.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 16) & 0x3f));
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffe08000u) == 0x00608000u) {
        insn->mnemonic = "bstrpick.w";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 16) & 0x1f));
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x000c0000u) {
        insn->mnemonic = "bytepick.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 15) & 0x7));
        return;
    }
    if ((word & 0xfffe0000u) == 0x00080000u) {
        insn->mnemonic = "bytepick.w";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 15) & 0x3));
        return;
    }
    if ((word & 0xffc00000u) == 0x06000000u) {
        insn->mnemonic = "cacop";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_UIMM;
        insn->operands[0].uimm = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xfffffc00u) == 0x00002000u) {
        insn->mnemonic = "clo.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00001000u) {
        insn->mnemonic = "clo.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00002400u) {
        insn->mnemonic = "clz.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00001400u) {
        insn->mnemonic = "clz.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00006c00u) {
        insn->mnemonic = "cpucfg";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00240000u) {
        insn->mnemonic = "crc.w.b.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00258000u) {
        insn->mnemonic = "crc.w.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00248000u) {
        insn->mnemonic = "crc.w.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00250000u) {
        insn->mnemonic = "crc.w.w.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00260000u) {
        insn->mnemonic = "crcc.w.b.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00278000u) {
        insn->mnemonic = "crcc.w.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00268000u) {
        insn->mnemonic = "crcc.w.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00270000u) {
        insn->mnemonic = "crcc.w.w.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xff000000u) == 0x04000000u) {
        insn->mnemonic = "csrxchg";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3fff));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00002800u) {
        insn->mnemonic = "cto.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00001800u) {
        insn->mnemonic = "cto.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00002c00u) {
        insn->mnemonic = "ctz.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00001c00u) {
        insn->mnemonic = "ctz.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38720000u) {
        insn->mnemonic = "dbar";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_UIMM;
        insn->operands[0].uimm = (((word >> 0) & 0x7fff));
        return;
    }
    if ((word & 0xffff8000u) == 0x002a8000u) {
        insn->mnemonic = "dbcl";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_UIMM;
        insn->operands[0].uimm = (((word >> 0) & 0x7fff));
        return;
    }
    if ((word & 0xffff8000u) == 0x00220000u) {
        insn->mnemonic = "div.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00230000u) {
        insn->mnemonic = "div.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00200000u) {
        insn->mnemonic = "div.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00210000u) {
        insn->mnemonic = "div.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffffffu) == 0x06483800u) {
        insn->mnemonic = "ertn";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xfffffc00u) == 0x00005c00u) {
        insn->mnemonic = "ext.w.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00005800u) {
        insn->mnemonic = "ext.w.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01140800u) {
        insn->mnemonic = "fabs.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01140400u) {
        insn->mnemonic = "fabs.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01010000u) {
        insn->mnemonic = "fadd.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01008000u) {
        insn->mnemonic = "fadd.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01143800u) {
        insn->mnemonic = "fclass.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01143400u) {
        insn->mnemonic = "fclass.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c200000u) {
        insn->mnemonic = "fcmp.caf.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c100000u) {
        insn->mnemonic = "fcmp.caf.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c220000u) {
        insn->mnemonic = "fcmp.ceq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c120000u) {
        insn->mnemonic = "fcmp.ceq.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c230000u) {
        insn->mnemonic = "fcmp.cle.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c130000u) {
        insn->mnemonic = "fcmp.cle.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c210000u) {
        insn->mnemonic = "fcmp.clt.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c110000u) {
        insn->mnemonic = "fcmp.clt.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c280000u) {
        insn->mnemonic = "fcmp.cne.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c180000u) {
        insn->mnemonic = "fcmp.cne.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c2a0000u) {
        insn->mnemonic = "fcmp.cor.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c1a0000u) {
        insn->mnemonic = "fcmp.cor.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c260000u) {
        insn->mnemonic = "fcmp.cueq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c160000u) {
        insn->mnemonic = "fcmp.cueq.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c270000u) {
        insn->mnemonic = "fcmp.cule.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c170000u) {
        insn->mnemonic = "fcmp.cule.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c250000u) {
        insn->mnemonic = "fcmp.cult.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c150000u) {
        insn->mnemonic = "fcmp.cult.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c240000u) {
        insn->mnemonic = "fcmp.cun.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c140000u) {
        insn->mnemonic = "fcmp.cun.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c2c0000u) {
        insn->mnemonic = "fcmp.cune.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c1c0000u) {
        insn->mnemonic = "fcmp.cune.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c208000u) {
        insn->mnemonic = "fcmp.saf.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c108000u) {
        insn->mnemonic = "fcmp.saf.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c228000u) {
        insn->mnemonic = "fcmp.seq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c128000u) {
        insn->mnemonic = "fcmp.seq.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c238000u) {
        insn->mnemonic = "fcmp.sle.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c138000u) {
        insn->mnemonic = "fcmp.sle.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c218000u) {
        insn->mnemonic = "fcmp.slt.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c118000u) {
        insn->mnemonic = "fcmp.slt.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c288000u) {
        insn->mnemonic = "fcmp.sne.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c188000u) {
        insn->mnemonic = "fcmp.sne.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c2a8000u) {
        insn->mnemonic = "fcmp.sor.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c1a8000u) {
        insn->mnemonic = "fcmp.sor.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c268000u) {
        insn->mnemonic = "fcmp.sueq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c168000u) {
        insn->mnemonic = "fcmp.sueq.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c278000u) {
        insn->mnemonic = "fcmp.sule.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c178000u) {
        insn->mnemonic = "fcmp.sule.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c258000u) {
        insn->mnemonic = "fcmp.sult.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c158000u) {
        insn->mnemonic = "fcmp.sult.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c248000u) {
        insn->mnemonic = "fcmp.sun.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c148000u) {
        insn->mnemonic = "fcmp.sun.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c2c8000u) {
        insn->mnemonic = "fcmp.sune.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8018u) == 0x0c1c8000u) {
        insn->mnemonic = "fcmp.sune.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01130000u) {
        insn->mnemonic = "fcopysign.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01128000u) {
        insn->mnemonic = "fcopysign.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01150000u) {
        insn->mnemonic = "fcvt.d.ld";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01192400u) {
        insn->mnemonic = "fcvt.d.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x0114e000u) {
        insn->mnemonic = "fcvt.ld.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01191800u) {
        insn->mnemonic = "fcvt.s.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x0114e400u) {
        insn->mnemonic = "fcvt.ud.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01070000u) {
        insn->mnemonic = "fdiv.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01068000u) {
        insn->mnemonic = "fdiv.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011d2800u) {
        insn->mnemonic = "ffint.d.l";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011d2000u) {
        insn->mnemonic = "ffint.d.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011d1800u) {
        insn->mnemonic = "ffint.s.l";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011d1000u) {
        insn->mnemonic = "ffint.s.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x2b800000u) {
        insn->mnemonic = "fld.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x2b000000u) {
        insn->mnemonic = "fld.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffff8000u) == 0x38748000u) {
        insn->mnemonic = "fldgt.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38740000u) {
        insn->mnemonic = "fldgt.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38758000u) {
        insn->mnemonic = "fldle.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38750000u) {
        insn->mnemonic = "fldle.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38340000u) {
        insn->mnemonic = "fldx.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38300000u) {
        insn->mnemonic = "fldx.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01142800u) {
        insn->mnemonic = "flogb.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01142400u) {
        insn->mnemonic = "flogb.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x08200000u) {
        insn->mnemonic = "fmadd.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_FPR;
        insn->operands[3].fpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x08100000u) {
        insn->mnemonic = "fmadd.s";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_FPR;
        insn->operands[3].fpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01090000u) {
        insn->mnemonic = "fmax.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01088000u) {
        insn->mnemonic = "fmax.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x010d0000u) {
        insn->mnemonic = "fmaxa.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x010c8000u) {
        insn->mnemonic = "fmaxa.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x010b0000u) {
        insn->mnemonic = "fmin.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x010a8000u) {
        insn->mnemonic = "fmin.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x010f0000u) {
        insn->mnemonic = "fmina.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x010e8000u) {
        insn->mnemonic = "fmina.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01149800u) {
        insn->mnemonic = "fmov.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01149400u) {
        insn->mnemonic = "fmov.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x08600000u) {
        insn->mnemonic = "fmsub.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_FPR;
        insn->operands[3].fpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x08500000u) {
        insn->mnemonic = "fmsub.s";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_FPR;
        insn->operands[3].fpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01050000u) {
        insn->mnemonic = "fmul.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01048000u) {
        insn->mnemonic = "fmul.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01141800u) {
        insn->mnemonic = "fneg.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01141400u) {
        insn->mnemonic = "fneg.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x08a00000u) {
        insn->mnemonic = "fnmadd.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_FPR;
        insn->operands[3].fpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x08900000u) {
        insn->mnemonic = "fnmadd.s";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_FPR;
        insn->operands[3].fpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x08e00000u) {
        insn->mnemonic = "fnmsub.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_FPR;
        insn->operands[3].fpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x08d00000u) {
        insn->mnemonic = "fnmsub.s";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_FPR;
        insn->operands[3].fpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01145800u) {
        insn->mnemonic = "frecip.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01145400u) {
        insn->mnemonic = "frecip.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01147800u) {
        insn->mnemonic = "frecipe.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01147400u) {
        insn->mnemonic = "frecipe.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011e4800u) {
        insn->mnemonic = "frint.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011e4400u) {
        insn->mnemonic = "frint.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01146800u) {
        insn->mnemonic = "frsqrt.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01146400u) {
        insn->mnemonic = "frsqrt.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01148800u) {
        insn->mnemonic = "frsqrte.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01148400u) {
        insn->mnemonic = "frsqrte.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01110000u) {
        insn->mnemonic = "fscaleb.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01108000u) {
        insn->mnemonic = "fscaleb.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x0d000000u) {
        insn->mnemonic = "fsel";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_FCC;
        insn->operands[3].fcc = (((word >> 15) & 0x7));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01144800u) {
        insn->mnemonic = "fsqrt.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x01144400u) {
        insn->mnemonic = "fsqrt.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x2bc00000u) {
        insn->mnemonic = "fst.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x2b400000u) {
        insn->mnemonic = "fst.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffff8000u) == 0x38768000u) {
        insn->mnemonic = "fstgt.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38760000u) {
        insn->mnemonic = "fstgt.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38778000u) {
        insn->mnemonic = "fstle.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38770000u) {
        insn->mnemonic = "fstle.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x383c0000u) {
        insn->mnemonic = "fstx.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38380000u) {
        insn->mnemonic = "fstx.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01030000u) {
        insn->mnemonic = "fsub.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x01028000u) {
        insn->mnemonic = "fsub.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_FPR;
        insn->operands[2].fpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011b2800u) {
        insn->mnemonic = "ftint.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011b2400u) {
        insn->mnemonic = "ftint.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011b0800u) {
        insn->mnemonic = "ftint.w.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011b0400u) {
        insn->mnemonic = "ftint.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011a2800u) {
        insn->mnemonic = "ftintrm.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011a2400u) {
        insn->mnemonic = "ftintrm.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011a0800u) {
        insn->mnemonic = "ftintrm.w.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011a0400u) {
        insn->mnemonic = "ftintrm.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011ae800u) {
        insn->mnemonic = "ftintrne.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011ae400u) {
        insn->mnemonic = "ftintrne.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011ac800u) {
        insn->mnemonic = "ftintrne.w.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011ac400u) {
        insn->mnemonic = "ftintrne.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011a6800u) {
        insn->mnemonic = "ftintrp.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011a6400u) {
        insn->mnemonic = "ftintrp.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011a4800u) {
        insn->mnemonic = "ftintrp.w.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011a4400u) {
        insn->mnemonic = "ftintrp.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011aa800u) {
        insn->mnemonic = "ftintrz.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011aa400u) {
        insn->mnemonic = "ftintrz.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011a8800u) {
        insn->mnemonic = "ftintrz.w.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x011a8400u) {
        insn->mnemonic = "ftintrz.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xff000000u) == 0x05000000u) {
        insn->mnemonic = "gcsrxchg";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3fff));
        return;
    }
    if ((word & 0xffffffffu) == 0x06482001u) {
        insn->mnemonic = "gtlbclr";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffffffffu) == 0x06483401u) {
        insn->mnemonic = "gtlbfill";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffffffffu) == 0x06482401u) {
        insn->mnemonic = "gtlbflush";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffffffffu) == 0x06482c01u) {
        insn->mnemonic = "gtlbrd";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffffffffu) == 0x06482801u) {
        insn->mnemonic = "gtlbsrch";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffffffffu) == 0x06483001u) {
        insn->mnemonic = "gtlbwr";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffff8000u) == 0x002b8000u) {
        insn->mnemonic = "hvcl";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_UIMM;
        insn->operands[0].uimm = (((word >> 0) & 0x7fff));
        return;
    }
    if ((word & 0xffff8000u) == 0x38728000u) {
        insn->mnemonic = "ibar";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_UIMM;
        insn->operands[0].uimm = (((word >> 0) & 0x7fff));
        return;
    }
    if ((word & 0xffff8000u) == 0x06488000u) {
        insn->mnemonic = "idle";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_UIMM;
        insn->operands[0].uimm = (((word >> 0) & 0x7fff));
        return;
    }
    if ((word & 0xffff8000u) == 0x06498000u) {
        insn->mnemonic = "invtlb";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_UIMM;
        insn->operands[0].uimm = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x06480000u) {
        insn->mnemonic = "iocsrrd.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x06480c00u) {
        insn->mnemonic = "iocsrrd.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x06480400u) {
        insn->mnemonic = "iocsrrd.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x06480800u) {
        insn->mnemonic = "iocsrrd.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x06481000u) {
        insn->mnemonic = "iocsrwr.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x06481c00u) {
        insn->mnemonic = "iocsrwr.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x06481400u) {
        insn->mnemonic = "iocsrwr.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x06481800u) {
        insn->mnemonic = "iocsrwr.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfc000000u) == 0x4c000000u) {
        insn->mnemonic = "jirl";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xffff) << 16 >> 16 << 2);
        return;
    }
    if ((word & 0xfc0003e0u) == 0x48000200u) {
        insn->mnemonic = "jiscr0";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_SIMM;
        insn->operands[0].simm = ((int32_t)(int32_t)((((word >> 0) & 0x1f) << 16) | ((word >> 10) & 0xffff)) << 11 >> 11 << 2);
        return;
    }
    if ((word & 0xfc0003e0u) == 0x48000300u) {
        insn->mnemonic = "jiscr1";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_SIMM;
        insn->operands[0].simm = ((int32_t)(int32_t)((((word >> 0) & 0x1f) << 16) | ((word >> 10) & 0xffff)) << 11 >> 11 << 2);
        return;
    }
    if ((word & 0xffc00000u) == 0x28000000u) {
        insn->mnemonic = "ld.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x2a000000u) {
        insn->mnemonic = "ld.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x28c00000u) {
        insn->mnemonic = "ld.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x28400000u) {
        insn->mnemonic = "ld.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x2a400000u) {
        insn->mnemonic = "ld.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x28800000u) {
        insn->mnemonic = "ld.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x2a800000u) {
        insn->mnemonic = "ld.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xfffc0000u) == 0x06400000u) {
        insn->mnemonic = "lddir";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x38780000u) {
        insn->mnemonic = "ldgt.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38798000u) {
        insn->mnemonic = "ldgt.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38788000u) {
        insn->mnemonic = "ldgt.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38790000u) {
        insn->mnemonic = "ldgt.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x2e800000u) {
        insn->mnemonic = "ldl.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x2e000000u) {
        insn->mnemonic = "ldl.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffff8000u) == 0x387a0000u) {
        insn->mnemonic = "ldle.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x387b8000u) {
        insn->mnemonic = "ldle.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x387a8000u) {
        insn->mnemonic = "ldle.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x387b0000u) {
        insn->mnemonic = "ldle.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc001fu) == 0x06440000u) {
        insn->mnemonic = "ldpte";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xff000000u) == 0x26000000u) {
        insn->mnemonic = "ldptr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x3fff) << 18 >> 18 << 2);
        return;
    }
    if ((word & 0xff000000u) == 0x24000000u) {
        insn->mnemonic = "ldptr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x3fff) << 18 >> 18 << 2);
        return;
    }
    if ((word & 0xffc00000u) == 0x2ec00000u) {
        insn->mnemonic = "ldr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x2e400000u) {
        insn->mnemonic = "ldr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffff8000u) == 0x38000000u) {
        insn->mnemonic = "ldx.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38200000u) {
        insn->mnemonic = "ldx.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x380c0000u) {
        insn->mnemonic = "ldx.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38040000u) {
        insn->mnemonic = "ldx.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38240000u) {
        insn->mnemonic = "ldx.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38080000u) {
        insn->mnemonic = "ldx.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38280000u) {
        insn->mnemonic = "ldx.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xff000000u) == 0x22000000u) {
        insn->mnemonic = "ll.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x3fff) << 18 >> 18 << 2);
        return;
    }
    if ((word & 0xff000000u) == 0x20000000u) {
        insn->mnemonic = "ll.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x3fff) << 18 >> 18 << 2);
        return;
    }
    if ((word & 0xfffffc00u) == 0x38578800u) {
        insn->mnemonic = "llacq.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x38578000u) {
        insn->mnemonic = "llacq.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfe000000u) == 0x14000000u) {
        insn->mnemonic = "lu12i.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_SIMM;
        insn->operands[1].simm = ((int32_t)((word >> 5) & 0xfffff) << 12 >> 12);
        return;
    }
    if ((word & 0xfe000000u) == 0x16000000u) {
        insn->mnemonic = "lu32i.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_SIMM;
        insn->operands[1].simm = ((int32_t)((word >> 5) & 0xfffff) << 12 >> 12);
        return;
    }
    if ((word & 0xffc00000u) == 0x03000000u) {
        insn->mnemonic = "lu52i.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffff8000u) == 0x00130000u) {
        insn->mnemonic = "maskeqz";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00138000u) {
        insn->mnemonic = "masknez";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00228000u) {
        insn->mnemonic = "mod.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00238000u) {
        insn->mnemonic = "mod.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00208000u) {
        insn->mnemonic = "mod.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00218000u) {
        insn->mnemonic = "mod.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffff00u) == 0x0114d400u) {
        insn->mnemonic = "movcf2fr";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FCC;
        insn->operands[1].fcc = (((word >> 5) & 0x7));
        return;
    }
    if ((word & 0xffffff00u) == 0x0114dc00u) {
        insn->mnemonic = "movcf2gr";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FCC;
        insn->operands[1].fcc = (((word >> 5) & 0x7));
        return;
    }
    if ((word & 0xfffffc00u) == 0x0114c800u) {
        insn->mnemonic = "movfcsr2gr";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FCSR;
        insn->operands[1].fcsr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x0114d000u) {
        insn->mnemonic = "movfr2cf";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x0114b800u) {
        insn->mnemonic = "movfr2gr.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x0114b400u) {
        insn->mnemonic = "movfr2gr.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x0114bc00u) {
        insn->mnemonic = "movfrh2gr.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_FPR;
        insn->operands[1].fpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x0114d800u) {
        insn->mnemonic = "movgr2cf";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x0114c000u) {
        insn->mnemonic = "movgr2fcsr";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCSR;
        insn->operands[0].fcsr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x0114a800u) {
        insn->mnemonic = "movgr2fr.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x0114a400u) {
        insn->mnemonic = "movgr2fr.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x0114ac00u) {
        insn->mnemonic = "movgr2frh.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FPR;
        insn->operands[0].fpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc1cu) == 0x00000800u) {
        insn->mnemonic = "movgr2scr";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_SCR;
        insn->operands[0].scr = (((word >> 0) & 0x3));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffffff80u) == 0x00000c00u) {
        insn->mnemonic = "movscr2gr";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_SCR;
        insn->operands[1].scr = (((word >> 5) & 0x3));
        return;
    }
    if ((word & 0xffff8000u) == 0x001d8000u) {
        insn->mnemonic = "mul.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x001c0000u) {
        insn->mnemonic = "mul.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x001e0000u) {
        insn->mnemonic = "mulh.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x001e8000u) {
        insn->mnemonic = "mulh.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x001c8000u) {
        insn->mnemonic = "mulh.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x001d0000u) {
        insn->mnemonic = "mulh.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x001f0000u) {
        insn->mnemonic = "mulw.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x001f8000u) {
        insn->mnemonic = "mulw.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00140000u) {
        insn->mnemonic = "nor";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00150000u) {
        insn->mnemonic = "or";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x03800000u) {
        insn->mnemonic = "ori";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xfff));
        return;
    }
    if ((word & 0xffff8000u) == 0x00160000u) {
        insn->mnemonic = "orn";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfe000000u) == 0x18000000u) {
        insn->mnemonic = "pcaddi";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_SIMM;
        insn->operands[1].simm = ((int32_t)((word >> 5) & 0xfffff) << 12 >> 12);
        return;
    }
    if ((word & 0xfe000000u) == 0x1c000000u) {
        insn->mnemonic = "pcaddu12i";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_SIMM;
        insn->operands[1].simm = ((int32_t)((word >> 5) & 0xfffff) << 12 >> 12);
        return;
    }
    if ((word & 0xfe000000u) == 0x1e000000u) {
        insn->mnemonic = "pcaddu18i";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_SIMM;
        insn->operands[1].simm = ((int32_t)((word >> 5) & 0xfffff) << 12 >> 12);
        return;
    }
    if ((word & 0xfe000000u) == 0x1a000000u) {
        insn->mnemonic = "pcalau12i";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_SIMM;
        insn->operands[1].simm = ((int32_t)((word >> 5) & 0xfffff) << 12 >> 12);
        return;
    }
    if ((word & 0xffc00000u) == 0x2ac00000u) {
        insn->mnemonic = "preld";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_UIMM;
        insn->operands[0].uimm = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffff8000u) == 0x382c0000u) {
        insn->mnemonic = "preldx";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_UIMM;
        insn->operands[0].uimm = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00340000u) {
        insn->mnemonic = "rcr.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00358000u) {
        insn->mnemonic = "rcr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00348000u) {
        insn->mnemonic = "rcr.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00350000u) {
        insn->mnemonic = "rcr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x00502000u) {
        insn->mnemonic = "rcri.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x00510000u) {
        insn->mnemonic = "rcri.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x00504000u) {
        insn->mnemonic = "rcri.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x00508000u) {
        insn->mnemonic = "rcri.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00006800u) {
        insn->mnemonic = "rdtime.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00006400u) {
        insn->mnemonic = "rdtimeh.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00006000u) {
        insn->mnemonic = "rdtimel.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00003000u) {
        insn->mnemonic = "revb.2h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00003800u) {
        insn->mnemonic = "revb.2w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00003400u) {
        insn->mnemonic = "revb.4h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00003c00u) {
        insn->mnemonic = "revb.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00004000u) {
        insn->mnemonic = "revh.2w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00004400u) {
        insn->mnemonic = "revh.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x001a0000u) {
        insn->mnemonic = "rotr.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x001b8000u) {
        insn->mnemonic = "rotr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x001a8000u) {
        insn->mnemonic = "rotr.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x001b0000u) {
        insn->mnemonic = "rotr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x004c2000u) {
        insn->mnemonic = "rotri.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x004d0000u) {
        insn->mnemonic = "rotri.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x004c4000u) {
        insn->mnemonic = "rotri.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x004c8000u) {
        insn->mnemonic = "rotri.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00320000u) {
        insn->mnemonic = "sbc.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00338000u) {
        insn->mnemonic = "sbc.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00328000u) {
        insn->mnemonic = "sbc.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00330000u) {
        insn->mnemonic = "sbc.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xff000000u) == 0x23000000u) {
        insn->mnemonic = "sc.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x3fff) << 18 >> 18 << 2);
        return;
    }
    if ((word & 0xffff8000u) == 0x38570000u) {
        insn->mnemonic = "sc.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xff000000u) == 0x21000000u) {
        insn->mnemonic = "sc.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x3fff) << 18 >> 18 << 2);
        return;
    }
    if ((word & 0xfffffc00u) == 0x38578c00u) {
        insn->mnemonic = "screl.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x38578400u) {
        insn->mnemonic = "screl.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffffc3e0u) == 0x0036c000u) {
        insn->mnemonic = "setarmj";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc3e0u) == 0x00368000u) {
        insn->mnemonic = "setx86j";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00007800u) {
        insn->mnemonic = "setx86loope";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x00007c00u) {
        insn->mnemonic = "setx86loopne";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00188000u) {
        insn->mnemonic = "sll.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00170000u) {
        insn->mnemonic = "sll.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x00410000u) {
        insn->mnemonic = "slli.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00408000u) {
        insn->mnemonic = "slli.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00120000u) {
        insn->mnemonic = "slt";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x02000000u) {
        insn->mnemonic = "slti";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffff8000u) == 0x00128000u) {
        insn->mnemonic = "sltu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x02400000u) {
        insn->mnemonic = "sltui";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffff8000u) == 0x00198000u) {
        insn->mnemonic = "sra.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00180000u) {
        insn->mnemonic = "sra.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x00490000u) {
        insn->mnemonic = "srai.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00488000u) {
        insn->mnemonic = "srai.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00190000u) {
        insn->mnemonic = "srl.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00178000u) {
        insn->mnemonic = "srl.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x00450000u) {
        insn->mnemonic = "srli.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00448000u) {
        insn->mnemonic = "srli.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x29000000u) {
        insn->mnemonic = "st.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x29c00000u) {
        insn->mnemonic = "st.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x29400000u) {
        insn->mnemonic = "st.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x29800000u) {
        insn->mnemonic = "st.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffff8000u) == 0x387c0000u) {
        insn->mnemonic = "stgt.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x387d8000u) {
        insn->mnemonic = "stgt.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x387c8000u) {
        insn->mnemonic = "stgt.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x387d0000u) {
        insn->mnemonic = "stgt.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x2f800000u) {
        insn->mnemonic = "stl.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x2f000000u) {
        insn->mnemonic = "stl.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffff8000u) == 0x387e0000u) {
        insn->mnemonic = "stle.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x387f8000u) {
        insn->mnemonic = "stle.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x387e8000u) {
        insn->mnemonic = "stle.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x387f0000u) {
        insn->mnemonic = "stle.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xff000000u) == 0x27000000u) {
        insn->mnemonic = "stptr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x3fff) << 18 >> 18 << 2);
        return;
    }
    if ((word & 0xff000000u) == 0x25000000u) {
        insn->mnemonic = "stptr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x3fff) << 18 >> 18 << 2);
        return;
    }
    if ((word & 0xffc00000u) == 0x2fc00000u) {
        insn->mnemonic = "str.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x2f400000u) {
        insn->mnemonic = "str.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffff8000u) == 0x38100000u) {
        insn->mnemonic = "stx.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x381c0000u) {
        insn->mnemonic = "stx.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38140000u) {
        insn->mnemonic = "stx.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x38180000u) {
        insn->mnemonic = "stx.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00118000u) {
        insn->mnemonic = "sub.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00110000u) {
        insn->mnemonic = "sub.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x002b0000u) {
        insn->mnemonic = "syscall";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_UIMM;
        insn->operands[0].uimm = (((word >> 0) & 0x7fff));
        return;
    }
    if ((word & 0xffffffffu) == 0x06482000u) {
        insn->mnemonic = "tlbclr";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffffffffu) == 0x06483400u) {
        insn->mnemonic = "tlbfill";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffffffffu) == 0x06482400u) {
        insn->mnemonic = "tlbflush";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffffffffu) == 0x06482c00u) {
        insn->mnemonic = "tlbrd";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffffffffu) == 0x06482800u) {
        insn->mnemonic = "tlbsrch";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffffffffu) == 0x06483000u) {
        insn->mnemonic = "tlbwr";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffff8000u) == 0x70600000u) {
        insn->mnemonic = "vabsd.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70620000u) {
        insn->mnemonic = "vabsd.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70618000u) {
        insn->mnemonic = "vabsd.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70638000u) {
        insn->mnemonic = "vabsd.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70608000u) {
        insn->mnemonic = "vabsd.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70628000u) {
        insn->mnemonic = "vabsd.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70610000u) {
        insn->mnemonic = "vabsd.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70630000u) {
        insn->mnemonic = "vabsd.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x700a0000u) {
        insn->mnemonic = "vadd.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x700b8000u) {
        insn->mnemonic = "vadd.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x700a8000u) {
        insn->mnemonic = "vadd.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x712d0000u) {
        insn->mnemonic = "vadd.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x700b0000u) {
        insn->mnemonic = "vadd.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x705c0000u) {
        insn->mnemonic = "vadda.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x705d8000u) {
        insn->mnemonic = "vadda.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x705c8000u) {
        insn->mnemonic = "vadda.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x705d0000u) {
        insn->mnemonic = "vadda.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x728a0000u) {
        insn->mnemonic = "vaddi.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x728b8000u) {
        insn->mnemonic = "vaddi.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x728a8000u) {
        insn->mnemonic = "vaddi.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x728b0000u) {
        insn->mnemonic = "vaddi.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x701f0000u) {
        insn->mnemonic = "vaddwev.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x702f0000u) {
        insn->mnemonic = "vaddwev.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x703f0000u) {
        insn->mnemonic = "vaddwev.d.wu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x701e0000u) {
        insn->mnemonic = "vaddwev.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x702e0000u) {
        insn->mnemonic = "vaddwev.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x703e0000u) {
        insn->mnemonic = "vaddwev.h.bu.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x701f8000u) {
        insn->mnemonic = "vaddwev.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x702f8000u) {
        insn->mnemonic = "vaddwev.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x703f8000u) {
        insn->mnemonic = "vaddwev.q.du.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x701e8000u) {
        insn->mnemonic = "vaddwev.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x702e8000u) {
        insn->mnemonic = "vaddwev.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x703e8000u) {
        insn->mnemonic = "vaddwev.w.hu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70230000u) {
        insn->mnemonic = "vaddwod.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70330000u) {
        insn->mnemonic = "vaddwod.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70410000u) {
        insn->mnemonic = "vaddwod.d.wu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70220000u) {
        insn->mnemonic = "vaddwod.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70320000u) {
        insn->mnemonic = "vaddwod.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70400000u) {
        insn->mnemonic = "vaddwod.h.bu.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70238000u) {
        insn->mnemonic = "vaddwod.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70338000u) {
        insn->mnemonic = "vaddwod.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70418000u) {
        insn->mnemonic = "vaddwod.q.du.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70228000u) {
        insn->mnemonic = "vaddwod.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70328000u) {
        insn->mnemonic = "vaddwod.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70408000u) {
        insn->mnemonic = "vaddwod.w.hu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71260000u) {
        insn->mnemonic = "vand.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x73d00000u) {
        insn->mnemonic = "vandi.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x71280000u) {
        insn->mnemonic = "vandn.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70640000u) {
        insn->mnemonic = "vavg.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70660000u) {
        insn->mnemonic = "vavg.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70658000u) {
        insn->mnemonic = "vavg.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70678000u) {
        insn->mnemonic = "vavg.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70648000u) {
        insn->mnemonic = "vavg.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70668000u) {
        insn->mnemonic = "vavg.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70650000u) {
        insn->mnemonic = "vavg.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70670000u) {
        insn->mnemonic = "vavg.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70680000u) {
        insn->mnemonic = "vavgr.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x706a0000u) {
        insn->mnemonic = "vavgr.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70698000u) {
        insn->mnemonic = "vavgr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x706b8000u) {
        insn->mnemonic = "vavgr.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70688000u) {
        insn->mnemonic = "vavgr.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x706a8000u) {
        insn->mnemonic = "vavgr.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70690000u) {
        insn->mnemonic = "vavgr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x706b0000u) {
        insn->mnemonic = "vavgr.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x710c0000u) {
        insn->mnemonic = "vbitclr.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x710d8000u) {
        insn->mnemonic = "vbitclr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x710c8000u) {
        insn->mnemonic = "vbitclr.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x710d0000u) {
        insn->mnemonic = "vbitclr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x73102000u) {
        insn->mnemonic = "vbitclri.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x73110000u) {
        insn->mnemonic = "vbitclri.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73104000u) {
        insn->mnemonic = "vbitclri.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x73108000u) {
        insn->mnemonic = "vbitclri.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71100000u) {
        insn->mnemonic = "vbitrev.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71118000u) {
        insn->mnemonic = "vbitrev.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71108000u) {
        insn->mnemonic = "vbitrev.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71110000u) {
        insn->mnemonic = "vbitrev.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x73182000u) {
        insn->mnemonic = "vbitrevi.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x73190000u) {
        insn->mnemonic = "vbitrevi.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73184000u) {
        insn->mnemonic = "vbitrevi.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x73188000u) {
        insn->mnemonic = "vbitrevi.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x0d100000u) {
        insn->mnemonic = "vbitsel.v";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_VPR;
        insn->operands[3].vpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x73c40000u) {
        insn->mnemonic = "vbitseli.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x710e0000u) {
        insn->mnemonic = "vbitset.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x710f8000u) {
        insn->mnemonic = "vbitset.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x710e8000u) {
        insn->mnemonic = "vbitset.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x710f0000u) {
        insn->mnemonic = "vbitset.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x73142000u) {
        insn->mnemonic = "vbitseti.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x73150000u) {
        insn->mnemonic = "vbitseti.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73144000u) {
        insn->mnemonic = "vbitseti.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x73148000u) {
        insn->mnemonic = "vbitseti.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x728e0000u) {
        insn->mnemonic = "vbsll.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x728e8000u) {
        insn->mnemonic = "vbsrl.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c0000u) {
        insn->mnemonic = "vclo.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c0c00u) {
        insn->mnemonic = "vclo.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c0400u) {
        insn->mnemonic = "vclo.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c0800u) {
        insn->mnemonic = "vclo.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c1000u) {
        insn->mnemonic = "vclz.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c1c00u) {
        insn->mnemonic = "vclz.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c1400u) {
        insn->mnemonic = "vclz.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c1800u) {
        insn->mnemonic = "vclz.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e00000u) {
        insn->mnemonic = "vdiv.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e40000u) {
        insn->mnemonic = "vdiv.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e18000u) {
        insn->mnemonic = "vdiv.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e58000u) {
        insn->mnemonic = "vdiv.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e08000u) {
        insn->mnemonic = "vdiv.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e48000u) {
        insn->mnemonic = "vdiv.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e10000u) {
        insn->mnemonic = "vdiv.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e50000u) {
        insn->mnemonic = "vdiv.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f1800u) {
        insn->mnemonic = "vext2xv.d.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f2000u) {
        insn->mnemonic = "vext2xv.d.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f2400u) {
        insn->mnemonic = "vext2xv.d.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f3000u) {
        insn->mnemonic = "vext2xv.du.bu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f3800u) {
        insn->mnemonic = "vext2xv.du.hu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f3c00u) {
        insn->mnemonic = "vext2xv.du.wu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f1000u) {
        insn->mnemonic = "vext2xv.h.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f2800u) {
        insn->mnemonic = "vext2xv.hu.bu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f1400u) {
        insn->mnemonic = "vext2xv.w.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f1c00u) {
        insn->mnemonic = "vext2xv.w.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f2c00u) {
        insn->mnemonic = "vext2xv.wu.bu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f3400u) {
        insn->mnemonic = "vext2xv.wu.hu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729ee800u) {
        insn->mnemonic = "vexth.d.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729ef800u) {
        insn->mnemonic = "vexth.du.wu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729ee000u) {
        insn->mnemonic = "vexth.h.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729ef000u) {
        insn->mnemonic = "vexth.hu.bu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729eec00u) {
        insn->mnemonic = "vexth.q.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729efc00u) {
        insn->mnemonic = "vexth.qu.du";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729ee400u) {
        insn->mnemonic = "vexth.w.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729ef400u) {
        insn->mnemonic = "vexth.wu.hu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x73090000u) {
        insn->mnemonic = "vextl.q.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x730d0000u) {
        insn->mnemonic = "vextl.qu.du";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x738c0000u) {
        insn->mnemonic = "vextrins.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x73800000u) {
        insn->mnemonic = "vextrins.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x73880000u) {
        insn->mnemonic = "vextrins.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x73840000u) {
        insn->mnemonic = "vextrins.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x71310000u) {
        insn->mnemonic = "vfadd.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71308000u) {
        insn->mnemonic = "vfadd.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729cd800u) {
        insn->mnemonic = "vfclass.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729cd400u) {
        insn->mnemonic = "vfclass.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c600000u) {
        insn->mnemonic = "vfcmp.caf.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c500000u) {
        insn->mnemonic = "vfcmp.caf.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c620000u) {
        insn->mnemonic = "vfcmp.ceq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c520000u) {
        insn->mnemonic = "vfcmp.ceq.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c630000u) {
        insn->mnemonic = "vfcmp.cle.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c530000u) {
        insn->mnemonic = "vfcmp.cle.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c610000u) {
        insn->mnemonic = "vfcmp.clt.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c510000u) {
        insn->mnemonic = "vfcmp.clt.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c680000u) {
        insn->mnemonic = "vfcmp.cne.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c580000u) {
        insn->mnemonic = "vfcmp.cne.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c6a0000u) {
        insn->mnemonic = "vfcmp.cor.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c5a0000u) {
        insn->mnemonic = "vfcmp.cor.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c660000u) {
        insn->mnemonic = "vfcmp.cueq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c560000u) {
        insn->mnemonic = "vfcmp.cueq.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c670000u) {
        insn->mnemonic = "vfcmp.cule.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c570000u) {
        insn->mnemonic = "vfcmp.cule.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c650000u) {
        insn->mnemonic = "vfcmp.cult.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c550000u) {
        insn->mnemonic = "vfcmp.cult.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c640000u) {
        insn->mnemonic = "vfcmp.cun.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c540000u) {
        insn->mnemonic = "vfcmp.cun.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c6c0000u) {
        insn->mnemonic = "vfcmp.cune.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c5c0000u) {
        insn->mnemonic = "vfcmp.cune.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c608000u) {
        insn->mnemonic = "vfcmp.saf.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c508000u) {
        insn->mnemonic = "vfcmp.saf.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c628000u) {
        insn->mnemonic = "vfcmp.seq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c528000u) {
        insn->mnemonic = "vfcmp.seq.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c638000u) {
        insn->mnemonic = "vfcmp.sle.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c538000u) {
        insn->mnemonic = "vfcmp.sle.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c618000u) {
        insn->mnemonic = "vfcmp.slt.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c518000u) {
        insn->mnemonic = "vfcmp.slt.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c688000u) {
        insn->mnemonic = "vfcmp.sne.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c588000u) {
        insn->mnemonic = "vfcmp.sne.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c6a8000u) {
        insn->mnemonic = "vfcmp.sor.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c5a8000u) {
        insn->mnemonic = "vfcmp.sor.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c668000u) {
        insn->mnemonic = "vfcmp.sueq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c568000u) {
        insn->mnemonic = "vfcmp.sueq.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c678000u) {
        insn->mnemonic = "vfcmp.sule.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c578000u) {
        insn->mnemonic = "vfcmp.sule.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c658000u) {
        insn->mnemonic = "vfcmp.sult.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c558000u) {
        insn->mnemonic = "vfcmp.sult.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c648000u) {
        insn->mnemonic = "vfcmp.sun.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c548000u) {
        insn->mnemonic = "vfcmp.sun.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c6c8000u) {
        insn->mnemonic = "vfcmp.sune.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c5c8000u) {
        insn->mnemonic = "vfcmp.sune.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71460000u) {
        insn->mnemonic = "vfcvt.h.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71468000u) {
        insn->mnemonic = "vfcvt.s.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729df400u) {
        insn->mnemonic = "vfcvth.d.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729dec00u) {
        insn->mnemonic = "vfcvth.s.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729df000u) {
        insn->mnemonic = "vfcvtl.d.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729de800u) {
        insn->mnemonic = "vfcvtl.s.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x713b0000u) {
        insn->mnemonic = "vfdiv.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x713a8000u) {
        insn->mnemonic = "vfdiv.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e0800u) {
        insn->mnemonic = "vffint.d.l";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e0c00u) {
        insn->mnemonic = "vffint.d.lu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71480000u) {
        insn->mnemonic = "vffint.s.l";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e0000u) {
        insn->mnemonic = "vffint.s.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e0400u) {
        insn->mnemonic = "vffint.s.wu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e1400u) {
        insn->mnemonic = "vffinth.d.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e1000u) {
        insn->mnemonic = "vffintl.d.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729cc800u) {
        insn->mnemonic = "vflogb.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729cc400u) {
        insn->mnemonic = "vflogb.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x09200000u) {
        insn->mnemonic = "vfmadd.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_VPR;
        insn->operands[3].vpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x09100000u) {
        insn->mnemonic = "vfmadd.s";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_VPR;
        insn->operands[3].vpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x713d0000u) {
        insn->mnemonic = "vfmax.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x713c8000u) {
        insn->mnemonic = "vfmax.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71410000u) {
        insn->mnemonic = "vfmaxa.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71408000u) {
        insn->mnemonic = "vfmaxa.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x713f0000u) {
        insn->mnemonic = "vfmin.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x713e8000u) {
        insn->mnemonic = "vfmin.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71430000u) {
        insn->mnemonic = "vfmina.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71428000u) {
        insn->mnemonic = "vfmina.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x09600000u) {
        insn->mnemonic = "vfmsub.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_VPR;
        insn->operands[3].vpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x09500000u) {
        insn->mnemonic = "vfmsub.s";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_VPR;
        insn->operands[3].vpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71390000u) {
        insn->mnemonic = "vfmul.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71388000u) {
        insn->mnemonic = "vfmul.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x09a00000u) {
        insn->mnemonic = "vfnmadd.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_VPR;
        insn->operands[3].vpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x09900000u) {
        insn->mnemonic = "vfnmadd.s";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_VPR;
        insn->operands[3].vpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x09e00000u) {
        insn->mnemonic = "vfnmsub.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_VPR;
        insn->operands[3].vpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x09d00000u) {
        insn->mnemonic = "vfnmsub.s";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_VPR;
        insn->operands[3].vpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729cf800u) {
        insn->mnemonic = "vfrecip.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729cf400u) {
        insn->mnemonic = "vfrecip.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d1800u) {
        insn->mnemonic = "vfrecipe.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d1400u) {
        insn->mnemonic = "vfrecipe.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d3800u) {
        insn->mnemonic = "vfrint.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d3400u) {
        insn->mnemonic = "vfrint.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d4800u) {
        insn->mnemonic = "vfrintrm.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d4400u) {
        insn->mnemonic = "vfrintrm.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d7800u) {
        insn->mnemonic = "vfrintrne.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d7400u) {
        insn->mnemonic = "vfrintrne.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d5800u) {
        insn->mnemonic = "vfrintrp.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d5400u) {
        insn->mnemonic = "vfrintrp.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d6800u) {
        insn->mnemonic = "vfrintrz.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d6400u) {
        insn->mnemonic = "vfrintrz.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d0800u) {
        insn->mnemonic = "vfrsqrt.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d0400u) {
        insn->mnemonic = "vfrsqrt.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d2800u) {
        insn->mnemonic = "vfrsqrte.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729d2400u) {
        insn->mnemonic = "vfrsqrte.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x712b0000u) {
        insn->mnemonic = "vfrstp.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x712b8000u) {
        insn->mnemonic = "vfrstp.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x729a0000u) {
        insn->mnemonic = "vfrstpi.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x729a8000u) {
        insn->mnemonic = "vfrstpi.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729ce800u) {
        insn->mnemonic = "vfsqrt.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729ce400u) {
        insn->mnemonic = "vfsqrt.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71330000u) {
        insn->mnemonic = "vfsub.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71328000u) {
        insn->mnemonic = "vfsub.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e3400u) {
        insn->mnemonic = "vftint.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e5c00u) {
        insn->mnemonic = "vftint.lu.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71498000u) {
        insn->mnemonic = "vftint.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e3000u) {
        insn->mnemonic = "vftint.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e5800u) {
        insn->mnemonic = "vftint.wu.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e8400u) {
        insn->mnemonic = "vftinth.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e8000u) {
        insn->mnemonic = "vftintl.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e3c00u) {
        insn->mnemonic = "vftintrm.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x714a0000u) {
        insn->mnemonic = "vftintrm.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e3800u) {
        insn->mnemonic = "vftintrm.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e8c00u) {
        insn->mnemonic = "vftintrmh.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e8800u) {
        insn->mnemonic = "vftintrml.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e5400u) {
        insn->mnemonic = "vftintrne.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x714b8000u) {
        insn->mnemonic = "vftintrne.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e5000u) {
        insn->mnemonic = "vftintrne.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729ea400u) {
        insn->mnemonic = "vftintrneh.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729ea000u) {
        insn->mnemonic = "vftintrnel.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e4400u) {
        insn->mnemonic = "vftintrp.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x714a8000u) {
        insn->mnemonic = "vftintrp.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e4000u) {
        insn->mnemonic = "vftintrp.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e9400u) {
        insn->mnemonic = "vftintrph.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e9000u) {
        insn->mnemonic = "vftintrpl.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e4c00u) {
        insn->mnemonic = "vftintrz.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e7400u) {
        insn->mnemonic = "vftintrz.lu.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x714b0000u) {
        insn->mnemonic = "vftintrz.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e4800u) {
        insn->mnemonic = "vftintrz.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e7000u) {
        insn->mnemonic = "vftintrz.wu.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e9c00u) {
        insn->mnemonic = "vftintrzh.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729e9800u) {
        insn->mnemonic = "vftintrzl.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70550000u) {
        insn->mnemonic = "vhaddw.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70590000u) {
        insn->mnemonic = "vhaddw.du.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70540000u) {
        insn->mnemonic = "vhaddw.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70580000u) {
        insn->mnemonic = "vhaddw.hu.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70558000u) {
        insn->mnemonic = "vhaddw.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70598000u) {
        insn->mnemonic = "vhaddw.qu.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70548000u) {
        insn->mnemonic = "vhaddw.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70588000u) {
        insn->mnemonic = "vhaddw.wu.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70570000u) {
        insn->mnemonic = "vhsubw.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x705b0000u) {
        insn->mnemonic = "vhsubw.du.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70560000u) {
        insn->mnemonic = "vhsubw.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x705a0000u) {
        insn->mnemonic = "vhsubw.hu.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70578000u) {
        insn->mnemonic = "vhsubw.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x705b8000u) {
        insn->mnemonic = "vhsubw.qu.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70568000u) {
        insn->mnemonic = "vhsubw.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x705a8000u) {
        insn->mnemonic = "vhsubw.wu.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x711c0000u) {
        insn->mnemonic = "vilvh.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x711d8000u) {
        insn->mnemonic = "vilvh.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x711c8000u) {
        insn->mnemonic = "vilvh.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x711d0000u) {
        insn->mnemonic = "vilvh.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x711a0000u) {
        insn->mnemonic = "vilvl.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x711b8000u) {
        insn->mnemonic = "vilvl.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x711a8000u) {
        insn->mnemonic = "vilvl.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x711b0000u) {
        insn->mnemonic = "vilvl.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x72eb8000u) {
        insn->mnemonic = "vinsgr2vr.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffff800u) == 0x72ebf000u) {
        insn->mnemonic = "vinsgr2vr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1));
        return;
    }
    if ((word & 0xffffe000u) == 0x72ebc000u) {
        insn->mnemonic = "vinsgr2vr.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xfffff000u) == 0x72ebe000u) {
        insn->mnemonic = "vinsgr2vr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3));
        return;
    }
    if ((word & 0xffc00000u) == 0x2c000000u) {
        insn->mnemonic = "vld";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xfffc0000u) == 0x73e00000u) {
        insn->mnemonic = "vldi";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_SIMM;
        insn->operands[1].simm = ((int32_t)((word >> 5) & 0x1fff) << 19 >> 19);
        return;
    }
    if ((word & 0xffc00000u) == 0x30800000u) {
        insn->mnemonic = "vldrepl.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xfff80000u) == 0x30100000u) {
        insn->mnemonic = "vldrepl.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1ff) << 23 >> 23 << 3);
        return;
    }
    if ((word & 0xffe00000u) == 0x30400000u) {
        insn->mnemonic = "vldrepl.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x7ff) << 21 >> 21 << 1);
        return;
    }
    if ((word & 0xfff00000u) == 0x30200000u) {
        insn->mnemonic = "vldrepl.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x3ff) << 22 >> 22 << 2);
        return;
    }
    if ((word & 0xffff8000u) == 0x38400000u) {
        insn->mnemonic = "vldx";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70a80000u) {
        insn->mnemonic = "vmadd.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70a98000u) {
        insn->mnemonic = "vmadd.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70a88000u) {
        insn->mnemonic = "vmadd.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70a90000u) {
        insn->mnemonic = "vmadd.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ad0000u) {
        insn->mnemonic = "vmaddwev.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70b50000u) {
        insn->mnemonic = "vmaddwev.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70bd0000u) {
        insn->mnemonic = "vmaddwev.d.wu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ac0000u) {
        insn->mnemonic = "vmaddwev.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70b40000u) {
        insn->mnemonic = "vmaddwev.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70bc0000u) {
        insn->mnemonic = "vmaddwev.h.bu.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ad8000u) {
        insn->mnemonic = "vmaddwev.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70b58000u) {
        insn->mnemonic = "vmaddwev.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70bd8000u) {
        insn->mnemonic = "vmaddwev.q.du.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ac8000u) {
        insn->mnemonic = "vmaddwev.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70b48000u) {
        insn->mnemonic = "vmaddwev.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70bc8000u) {
        insn->mnemonic = "vmaddwev.w.hu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70af0000u) {
        insn->mnemonic = "vmaddwod.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70b70000u) {
        insn->mnemonic = "vmaddwod.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70bf0000u) {
        insn->mnemonic = "vmaddwod.d.wu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ae0000u) {
        insn->mnemonic = "vmaddwod.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70b60000u) {
        insn->mnemonic = "vmaddwod.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70be0000u) {
        insn->mnemonic = "vmaddwod.h.bu.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70af8000u) {
        insn->mnemonic = "vmaddwod.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70b78000u) {
        insn->mnemonic = "vmaddwod.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70bf8000u) {
        insn->mnemonic = "vmaddwod.q.du.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ae8000u) {
        insn->mnemonic = "vmaddwod.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70b68000u) {
        insn->mnemonic = "vmaddwod.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70be8000u) {
        insn->mnemonic = "vmaddwod.w.hu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70700000u) {
        insn->mnemonic = "vmax.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70740000u) {
        insn->mnemonic = "vmax.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70718000u) {
        insn->mnemonic = "vmax.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70758000u) {
        insn->mnemonic = "vmax.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70708000u) {
        insn->mnemonic = "vmax.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70748000u) {
        insn->mnemonic = "vmax.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70710000u) {
        insn->mnemonic = "vmax.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70750000u) {
        insn->mnemonic = "vmax.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72900000u) {
        insn->mnemonic = "vmaxi.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72940000u) {
        insn->mnemonic = "vmaxi.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72918000u) {
        insn->mnemonic = "vmaxi.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72958000u) {
        insn->mnemonic = "vmaxi.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72908000u) {
        insn->mnemonic = "vmaxi.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72948000u) {
        insn->mnemonic = "vmaxi.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72910000u) {
        insn->mnemonic = "vmaxi.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72950000u) {
        insn->mnemonic = "vmaxi.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x729b8000u) {
        insn->mnemonic = "vmepatmsk.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70720000u) {
        insn->mnemonic = "vmin.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70760000u) {
        insn->mnemonic = "vmin.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70738000u) {
        insn->mnemonic = "vmin.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70778000u) {
        insn->mnemonic = "vmin.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70728000u) {
        insn->mnemonic = "vmin.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70768000u) {
        insn->mnemonic = "vmin.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70730000u) {
        insn->mnemonic = "vmin.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70770000u) {
        insn->mnemonic = "vmin.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72920000u) {
        insn->mnemonic = "vmini.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72960000u) {
        insn->mnemonic = "vmini.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72938000u) {
        insn->mnemonic = "vmini.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72978000u) {
        insn->mnemonic = "vmini.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72928000u) {
        insn->mnemonic = "vmini.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72968000u) {
        insn->mnemonic = "vmini.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72930000u) {
        insn->mnemonic = "vmini.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72970000u) {
        insn->mnemonic = "vmini.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e20000u) {
        insn->mnemonic = "vmod.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e60000u) {
        insn->mnemonic = "vmod.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e38000u) {
        insn->mnemonic = "vmod.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e78000u) {
        insn->mnemonic = "vmod.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e28000u) {
        insn->mnemonic = "vmod.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e68000u) {
        insn->mnemonic = "vmod.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e30000u) {
        insn->mnemonic = "vmod.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e70000u) {
        insn->mnemonic = "vmod.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c5000u) {
        insn->mnemonic = "vmskgez.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c4000u) {
        insn->mnemonic = "vmskltz.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c4c00u) {
        insn->mnemonic = "vmskltz.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c4400u) {
        insn->mnemonic = "vmskltz.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c4800u) {
        insn->mnemonic = "vmskltz.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c6000u) {
        insn->mnemonic = "vmsknz.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70aa0000u) {
        insn->mnemonic = "vmsub.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ab8000u) {
        insn->mnemonic = "vmsub.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70aa8000u) {
        insn->mnemonic = "vmsub.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ab0000u) {
        insn->mnemonic = "vmsub.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70860000u) {
        insn->mnemonic = "vmuh.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70880000u) {
        insn->mnemonic = "vmuh.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70878000u) {
        insn->mnemonic = "vmuh.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70898000u) {
        insn->mnemonic = "vmuh.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70868000u) {
        insn->mnemonic = "vmuh.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70888000u) {
        insn->mnemonic = "vmuh.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70870000u) {
        insn->mnemonic = "vmuh.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70890000u) {
        insn->mnemonic = "vmuh.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70840000u) {
        insn->mnemonic = "vmul.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70858000u) {
        insn->mnemonic = "vmul.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70848000u) {
        insn->mnemonic = "vmul.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70850000u) {
        insn->mnemonic = "vmul.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70910000u) {
        insn->mnemonic = "vmulwev.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70990000u) {
        insn->mnemonic = "vmulwev.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70a10000u) {
        insn->mnemonic = "vmulwev.d.wu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70900000u) {
        insn->mnemonic = "vmulwev.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70980000u) {
        insn->mnemonic = "vmulwev.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70a00000u) {
        insn->mnemonic = "vmulwev.h.bu.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70918000u) {
        insn->mnemonic = "vmulwev.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70998000u) {
        insn->mnemonic = "vmulwev.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70a18000u) {
        insn->mnemonic = "vmulwev.q.du.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70908000u) {
        insn->mnemonic = "vmulwev.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70988000u) {
        insn->mnemonic = "vmulwev.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70a08000u) {
        insn->mnemonic = "vmulwev.w.hu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70930000u) {
        insn->mnemonic = "vmulwod.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x709b0000u) {
        insn->mnemonic = "vmulwod.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70a30000u) {
        insn->mnemonic = "vmulwod.d.wu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70920000u) {
        insn->mnemonic = "vmulwod.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x709a0000u) {
        insn->mnemonic = "vmulwod.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70a20000u) {
        insn->mnemonic = "vmulwod.h.bu.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70938000u) {
        insn->mnemonic = "vmulwod.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x709b8000u) {
        insn->mnemonic = "vmulwod.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70a38000u) {
        insn->mnemonic = "vmulwod.q.du.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70928000u) {
        insn->mnemonic = "vmulwod.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x709a8000u) {
        insn->mnemonic = "vmulwod.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70a28000u) {
        insn->mnemonic = "vmulwod.w.hu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c3000u) {
        insn->mnemonic = "vneg.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c3c00u) {
        insn->mnemonic = "vneg.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c3400u) {
        insn->mnemonic = "vneg.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c3800u) {
        insn->mnemonic = "vneg.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71278000u) {
        insn->mnemonic = "vnor.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x73dc0000u) {
        insn->mnemonic = "vnori.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x71268000u) {
        insn->mnemonic = "vor.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x73d40000u) {
        insn->mnemonic = "vori.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x71288000u) {
        insn->mnemonic = "vorn.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71160000u) {
        insn->mnemonic = "vpackev.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71178000u) {
        insn->mnemonic = "vpackev.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71168000u) {
        insn->mnemonic = "vpackev.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71170000u) {
        insn->mnemonic = "vpackev.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71180000u) {
        insn->mnemonic = "vpackod.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71198000u) {
        insn->mnemonic = "vpackod.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71188000u) {
        insn->mnemonic = "vpackod.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71190000u) {
        insn->mnemonic = "vpackod.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c2000u) {
        insn->mnemonic = "vpcnt.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c2c00u) {
        insn->mnemonic = "vpcnt.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c2400u) {
        insn->mnemonic = "vpcnt.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729c2800u) {
        insn->mnemonic = "vpcnt.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x73e40000u) {
        insn->mnemonic = "vpermi.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x711e0000u) {
        insn->mnemonic = "vpickev.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x711f8000u) {
        insn->mnemonic = "vpickev.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x711e8000u) {
        insn->mnemonic = "vpickev.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x711f0000u) {
        insn->mnemonic = "vpickev.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71200000u) {
        insn->mnemonic = "vpickod.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71218000u) {
        insn->mnemonic = "vpickod.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71208000u) {
        insn->mnemonic = "vpickod.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71210000u) {
        insn->mnemonic = "vpickod.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x72ef8000u) {
        insn->mnemonic = "vpickve2gr.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x72f38000u) {
        insn->mnemonic = "vpickve2gr.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffff800u) == 0x72eff000u) {
        insn->mnemonic = "vpickve2gr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1));
        return;
    }
    if ((word & 0xfffff800u) == 0x72f3f000u) {
        insn->mnemonic = "vpickve2gr.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1));
        return;
    }
    if ((word & 0xffffe000u) == 0x72efc000u) {
        insn->mnemonic = "vpickve2gr.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffffe000u) == 0x72f3c000u) {
        insn->mnemonic = "vpickve2gr.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xfffff000u) == 0x72efe000u) {
        insn->mnemonic = "vpickve2gr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3));
        return;
    }
    if ((word & 0xfffff000u) == 0x72f3e000u) {
        insn->mnemonic = "vpickve2gr.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729f0000u) {
        insn->mnemonic = "vreplgr2vr.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729f0c00u) {
        insn->mnemonic = "vreplgr2vr.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729f0400u) {
        insn->mnemonic = "vreplgr2vr.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x729f0800u) {
        insn->mnemonic = "vreplgr2vr.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71220000u) {
        insn->mnemonic = "vreplve.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71238000u) {
        insn->mnemonic = "vreplve.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71228000u) {
        insn->mnemonic = "vreplve.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71230000u) {
        insn->mnemonic = "vreplve.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x72f78000u) {
        insn->mnemonic = "vreplvei.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffff800u) == 0x72f7f000u) {
        insn->mnemonic = "vreplvei.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1));
        return;
    }
    if ((word & 0xffffe000u) == 0x72f7c000u) {
        insn->mnemonic = "vreplvei.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xfffff000u) == 0x72f7e000u) {
        insn->mnemonic = "vreplvei.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ee0000u) {
        insn->mnemonic = "vrotr.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ef8000u) {
        insn->mnemonic = "vrotr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ee8000u) {
        insn->mnemonic = "vrotr.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ef0000u) {
        insn->mnemonic = "vrotr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x72a02000u) {
        insn->mnemonic = "vrotri.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x72a10000u) {
        insn->mnemonic = "vrotri.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x72a04000u) {
        insn->mnemonic = "vrotri.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x72a08000u) {
        insn->mnemonic = "vrotri.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70460000u) {
        insn->mnemonic = "vsadd.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x704a0000u) {
        insn->mnemonic = "vsadd.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70478000u) {
        insn->mnemonic = "vsadd.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x704b8000u) {
        insn->mnemonic = "vsadd.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70468000u) {
        insn->mnemonic = "vsadd.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x704a8000u) {
        insn->mnemonic = "vsadd.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70470000u) {
        insn->mnemonic = "vsadd.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x704b0000u) {
        insn->mnemonic = "vsadd.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x73242000u) {
        insn->mnemonic = "vsat.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffffe000u) == 0x73282000u) {
        insn->mnemonic = "vsat.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x73250000u) {
        insn->mnemonic = "vsat.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff0000u) == 0x73290000u) {
        insn->mnemonic = "vsat.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73244000u) {
        insn->mnemonic = "vsat.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x73284000u) {
        insn->mnemonic = "vsat.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x73248000u) {
        insn->mnemonic = "vsat.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x73288000u) {
        insn->mnemonic = "vsat.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70000000u) {
        insn->mnemonic = "vseq.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70018000u) {
        insn->mnemonic = "vseq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70008000u) {
        insn->mnemonic = "vseq.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70010000u) {
        insn->mnemonic = "vseq.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72800000u) {
        insn->mnemonic = "vseqi.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72818000u) {
        insn->mnemonic = "vseqi.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72808000u) {
        insn->mnemonic = "vseqi.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72810000u) {
        insn->mnemonic = "vseqi.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xfffffc18u) == 0x729cb000u) {
        insn->mnemonic = "vsetallnez.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x729cbc00u) {
        insn->mnemonic = "vsetallnez.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x729cb400u) {
        insn->mnemonic = "vsetallnez.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x729cb800u) {
        insn->mnemonic = "vsetallnez.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x729ca000u) {
        insn->mnemonic = "vsetanyeqz.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x729cac00u) {
        insn->mnemonic = "vsetanyeqz.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x729ca400u) {
        insn->mnemonic = "vsetanyeqz.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x729ca800u) {
        insn->mnemonic = "vsetanyeqz.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x729c9800u) {
        insn->mnemonic = "vseteqz.v";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x729c9c00u) {
        insn->mnemonic = "vsetnez.v";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x73900000u) {
        insn->mnemonic = "vshuf4i.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x739c0000u) {
        insn->mnemonic = "vshuf4i.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x73940000u) {
        insn->mnemonic = "vshuf4i.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x73980000u) {
        insn->mnemonic = "vshuf4i.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfff00000u) == 0x0d500000u) {
        insn->mnemonic = "vshuf.b";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_VPR;
        insn->operands[3].vpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x717b8000u) {
        insn->mnemonic = "vshuf.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x717a8000u) {
        insn->mnemonic = "vshuf.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x717b0000u) {
        insn->mnemonic = "vshuf.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x712e0000u) {
        insn->mnemonic = "vsigncov.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x712f8000u) {
        insn->mnemonic = "vsigncov.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x712e8000u) {
        insn->mnemonic = "vsigncov.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x712f0000u) {
        insn->mnemonic = "vsigncov.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70020000u) {
        insn->mnemonic = "vsle.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70040000u) {
        insn->mnemonic = "vsle.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70038000u) {
        insn->mnemonic = "vsle.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70058000u) {
        insn->mnemonic = "vsle.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70028000u) {
        insn->mnemonic = "vsle.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70048000u) {
        insn->mnemonic = "vsle.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70030000u) {
        insn->mnemonic = "vsle.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70050000u) {
        insn->mnemonic = "vsle.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72820000u) {
        insn->mnemonic = "vslei.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72840000u) {
        insn->mnemonic = "vslei.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72838000u) {
        insn->mnemonic = "vslei.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72858000u) {
        insn->mnemonic = "vslei.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72828000u) {
        insn->mnemonic = "vslei.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72848000u) {
        insn->mnemonic = "vslei.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72830000u) {
        insn->mnemonic = "vslei.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72850000u) {
        insn->mnemonic = "vslei.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e80000u) {
        insn->mnemonic = "vsll.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e98000u) {
        insn->mnemonic = "vsll.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e88000u) {
        insn->mnemonic = "vsll.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70e90000u) {
        insn->mnemonic = "vsll.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x732c2000u) {
        insn->mnemonic = "vslli.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x732d0000u) {
        insn->mnemonic = "vslli.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x732c4000u) {
        insn->mnemonic = "vslli.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x732c8000u) {
        insn->mnemonic = "vslli.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x73088000u) {
        insn->mnemonic = "vsllwil.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x730c8000u) {
        insn->mnemonic = "vsllwil.du.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x73082000u) {
        insn->mnemonic = "vsllwil.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffffe000u) == 0x730c2000u) {
        insn->mnemonic = "vsllwil.hu.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffffc000u) == 0x73084000u) {
        insn->mnemonic = "vsllwil.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x730c4000u) {
        insn->mnemonic = "vsllwil.wu.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x70060000u) {
        insn->mnemonic = "vslt.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70080000u) {
        insn->mnemonic = "vslt.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70078000u) {
        insn->mnemonic = "vslt.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70098000u) {
        insn->mnemonic = "vslt.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70068000u) {
        insn->mnemonic = "vslt.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70088000u) {
        insn->mnemonic = "vslt.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70070000u) {
        insn->mnemonic = "vslt.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70090000u) {
        insn->mnemonic = "vslt.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72860000u) {
        insn->mnemonic = "vslti.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72880000u) {
        insn->mnemonic = "vslti.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72878000u) {
        insn->mnemonic = "vslti.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72898000u) {
        insn->mnemonic = "vslti.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72868000u) {
        insn->mnemonic = "vslti.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72888000u) {
        insn->mnemonic = "vslti.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x72870000u) {
        insn->mnemonic = "vslti.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x72890000u) {
        insn->mnemonic = "vslti.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ec0000u) {
        insn->mnemonic = "vsra.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ed8000u) {
        insn->mnemonic = "vsra.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ec8000u) {
        insn->mnemonic = "vsra.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ed0000u) {
        insn->mnemonic = "vsra.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x73342000u) {
        insn->mnemonic = "vsrai.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x73350000u) {
        insn->mnemonic = "vsrai.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73344000u) {
        insn->mnemonic = "vsrai.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x73348000u) {
        insn->mnemonic = "vsrai.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f68000u) {
        insn->mnemonic = "vsran.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f70000u) {
        insn->mnemonic = "vsran.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f78000u) {
        insn->mnemonic = "vsran.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73584000u) {
        insn->mnemonic = "vsrani.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x735a0000u) {
        insn->mnemonic = "vsrani.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x73588000u) {
        insn->mnemonic = "vsrani.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x73590000u) {
        insn->mnemonic = "vsrani.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f20000u) {
        insn->mnemonic = "vsrar.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f38000u) {
        insn->mnemonic = "vsrar.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f28000u) {
        insn->mnemonic = "vsrar.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f30000u) {
        insn->mnemonic = "vsrar.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x72a82000u) {
        insn->mnemonic = "vsrari.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x72a90000u) {
        insn->mnemonic = "vsrari.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x72a84000u) {
        insn->mnemonic = "vsrari.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x72a88000u) {
        insn->mnemonic = "vsrari.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70fa8000u) {
        insn->mnemonic = "vsrarn.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70fb0000u) {
        insn->mnemonic = "vsrarn.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70fb8000u) {
        insn->mnemonic = "vsrarn.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x735c4000u) {
        insn->mnemonic = "vsrarni.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x735e0000u) {
        insn->mnemonic = "vsrarni.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x735c8000u) {
        insn->mnemonic = "vsrarni.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x735d0000u) {
        insn->mnemonic = "vsrarni.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ea0000u) {
        insn->mnemonic = "vsrl.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70eb8000u) {
        insn->mnemonic = "vsrl.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ea8000u) {
        insn->mnemonic = "vsrl.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70eb0000u) {
        insn->mnemonic = "vsrl.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x73302000u) {
        insn->mnemonic = "vsrli.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x73310000u) {
        insn->mnemonic = "vsrli.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73304000u) {
        insn->mnemonic = "vsrli.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x73308000u) {
        insn->mnemonic = "vsrli.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f48000u) {
        insn->mnemonic = "vsrln.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f50000u) {
        insn->mnemonic = "vsrln.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f58000u) {
        insn->mnemonic = "vsrln.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73404000u) {
        insn->mnemonic = "vsrlni.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x73420000u) {
        insn->mnemonic = "vsrlni.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x73408000u) {
        insn->mnemonic = "vsrlni.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x73410000u) {
        insn->mnemonic = "vsrlni.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f00000u) {
        insn->mnemonic = "vsrlr.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f18000u) {
        insn->mnemonic = "vsrlr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f08000u) {
        insn->mnemonic = "vsrlr.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f10000u) {
        insn->mnemonic = "vsrlr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x72a42000u) {
        insn->mnemonic = "vsrlri.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x72a50000u) {
        insn->mnemonic = "vsrlri.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x72a44000u) {
        insn->mnemonic = "vsrlri.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x72a48000u) {
        insn->mnemonic = "vsrlri.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f88000u) {
        insn->mnemonic = "vsrlrn.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f90000u) {
        insn->mnemonic = "vsrlrn.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70f98000u) {
        insn->mnemonic = "vsrlrn.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73444000u) {
        insn->mnemonic = "vsrlrni.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x73460000u) {
        insn->mnemonic = "vsrlrni.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x73448000u) {
        insn->mnemonic = "vsrlrni.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x73450000u) {
        insn->mnemonic = "vsrlrni.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70fe8000u) {
        insn->mnemonic = "vssran.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71068000u) {
        insn->mnemonic = "vssran.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ff0000u) {
        insn->mnemonic = "vssran.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71070000u) {
        insn->mnemonic = "vssran.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70ff8000u) {
        insn->mnemonic = "vssran.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71078000u) {
        insn->mnemonic = "vssran.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73604000u) {
        insn->mnemonic = "vssrani.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x73644000u) {
        insn->mnemonic = "vssrani.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x73620000u) {
        insn->mnemonic = "vssrani.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xfffe0000u) == 0x73660000u) {
        insn->mnemonic = "vssrani.du.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x73608000u) {
        insn->mnemonic = "vssrani.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x73648000u) {
        insn->mnemonic = "vssrani.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x73610000u) {
        insn->mnemonic = "vssrani.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff0000u) == 0x73650000u) {
        insn->mnemonic = "vssrani.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71028000u) {
        insn->mnemonic = "vssrarn.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x710a8000u) {
        insn->mnemonic = "vssrarn.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71030000u) {
        insn->mnemonic = "vssrarn.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x710b0000u) {
        insn->mnemonic = "vssrarn.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71038000u) {
        insn->mnemonic = "vssrarn.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x710b8000u) {
        insn->mnemonic = "vssrarn.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73684000u) {
        insn->mnemonic = "vssrarni.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x736c4000u) {
        insn->mnemonic = "vssrarni.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x736a0000u) {
        insn->mnemonic = "vssrarni.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xfffe0000u) == 0x736e0000u) {
        insn->mnemonic = "vssrarni.du.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x73688000u) {
        insn->mnemonic = "vssrarni.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x736c8000u) {
        insn->mnemonic = "vssrarni.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x73690000u) {
        insn->mnemonic = "vssrarni.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff0000u) == 0x736d0000u) {
        insn->mnemonic = "vssrarni.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70fc8000u) {
        insn->mnemonic = "vssrln.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71048000u) {
        insn->mnemonic = "vssrln.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70fd0000u) {
        insn->mnemonic = "vssrln.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71050000u) {
        insn->mnemonic = "vssrln.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70fd8000u) {
        insn->mnemonic = "vssrln.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71058000u) {
        insn->mnemonic = "vssrln.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73484000u) {
        insn->mnemonic = "vssrlni.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x734c4000u) {
        insn->mnemonic = "vssrlni.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x734a0000u) {
        insn->mnemonic = "vssrlni.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xfffe0000u) == 0x734e0000u) {
        insn->mnemonic = "vssrlni.du.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x73488000u) {
        insn->mnemonic = "vssrlni.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x734c8000u) {
        insn->mnemonic = "vssrlni.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x73490000u) {
        insn->mnemonic = "vssrlni.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff0000u) == 0x734d0000u) {
        insn->mnemonic = "vssrlni.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71008000u) {
        insn->mnemonic = "vssrlrn.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71088000u) {
        insn->mnemonic = "vssrlrn.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71010000u) {
        insn->mnemonic = "vssrlrn.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71090000u) {
        insn->mnemonic = "vssrlrn.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71018000u) {
        insn->mnemonic = "vssrlrn.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71098000u) {
        insn->mnemonic = "vssrlrn.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x73504000u) {
        insn->mnemonic = "vssrlrni.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x73544000u) {
        insn->mnemonic = "vssrlrni.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x73520000u) {
        insn->mnemonic = "vssrlrni.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xfffe0000u) == 0x73560000u) {
        insn->mnemonic = "vssrlrni.du.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x73508000u) {
        insn->mnemonic = "vssrlrni.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x73548000u) {
        insn->mnemonic = "vssrlrni.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x73510000u) {
        insn->mnemonic = "vssrlrni.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff0000u) == 0x73550000u) {
        insn->mnemonic = "vssrlrni.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70480000u) {
        insn->mnemonic = "vssub.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x704c0000u) {
        insn->mnemonic = "vssub.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70498000u) {
        insn->mnemonic = "vssub.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x704d8000u) {
        insn->mnemonic = "vssub.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70488000u) {
        insn->mnemonic = "vssub.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x704c8000u) {
        insn->mnemonic = "vssub.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70490000u) {
        insn->mnemonic = "vssub.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x704d0000u) {
        insn->mnemonic = "vssub.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x2c400000u) {
        insn->mnemonic = "vst";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xffc00000u) == 0x31800000u) {
        insn->mnemonic = "vstelm.b";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xff) << 24 >> 24);
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 18) & 0xf));
        return;
    }
    if ((word & 0xfff80000u) == 0x31100000u) {
        insn->mnemonic = "vstelm.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xff) << 24 >> 24 << 3);
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 18) & 0x1));
        return;
    }
    if ((word & 0xffe00000u) == 0x31400000u) {
        insn->mnemonic = "vstelm.h";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xff) << 24 >> 24 << 1);
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 18) & 0x7));
        return;
    }
    if ((word & 0xfff00000u) == 0x31200000u) {
        insn->mnemonic = "vstelm.w";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xff) << 24 >> 24 << 2);
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 18) & 0x3));
        return;
    }
    if ((word & 0xffff8000u) == 0x38440000u) {
        insn->mnemonic = "vstx";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x700c0000u) {
        insn->mnemonic = "vsub.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x700d8000u) {
        insn->mnemonic = "vsub.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x700c8000u) {
        insn->mnemonic = "vsub.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x712d8000u) {
        insn->mnemonic = "vsub.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x700d0000u) {
        insn->mnemonic = "vsub.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x728c0000u) {
        insn->mnemonic = "vsubi.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x728d8000u) {
        insn->mnemonic = "vsubi.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x728c8000u) {
        insn->mnemonic = "vsubi.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x728d0000u) {
        insn->mnemonic = "vsubi.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70210000u) {
        insn->mnemonic = "vsubwev.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70310000u) {
        insn->mnemonic = "vsubwev.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70200000u) {
        insn->mnemonic = "vsubwev.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70300000u) {
        insn->mnemonic = "vsubwev.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70218000u) {
        insn->mnemonic = "vsubwev.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70318000u) {
        insn->mnemonic = "vsubwev.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70208000u) {
        insn->mnemonic = "vsubwev.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70308000u) {
        insn->mnemonic = "vsubwev.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70250000u) {
        insn->mnemonic = "vsubwod.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70350000u) {
        insn->mnemonic = "vsubwod.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70240000u) {
        insn->mnemonic = "vsubwod.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70340000u) {
        insn->mnemonic = "vsubwod.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70258000u) {
        insn->mnemonic = "vsubwod.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70358000u) {
        insn->mnemonic = "vsubwod.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70248000u) {
        insn->mnemonic = "vsubwod.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x70348000u) {
        insn->mnemonic = "vsubwod.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x71270000u) {
        insn->mnemonic = "vxor.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_VPR;
        insn->operands[2].vpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x73d80000u) {
        insn->mnemonic = "vxori.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_VPR;
        insn->operands[0].vpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_VPR;
        insn->operands[1].vpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f000cu) {
        insn->mnemonic = "x86adc.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f000fu) {
        insn->mnemonic = "x86adc.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f000du) {
        insn->mnemonic = "x86adc.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f000eu) {
        insn->mnemonic = "x86adc.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0004u) {
        insn->mnemonic = "x86add.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0007u) {
        insn->mnemonic = "x86add.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0001u) {
        insn->mnemonic = "x86add.du";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0005u) {
        insn->mnemonic = "x86add.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0006u) {
        insn->mnemonic = "x86add.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0000u) {
        insn->mnemonic = "x86add.wu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8010u) {
        insn->mnemonic = "x86and.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8013u) {
        insn->mnemonic = "x86and.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8011u) {
        insn->mnemonic = "x86and.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8012u) {
        insn->mnemonic = "x86and.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffffffu) == 0x00008028u) {
        insn->mnemonic = "x86clrtm";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xfffffc1fu) == 0x00008004u) {
        insn->mnemonic = "x86dec.b";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc1fu) == 0x00008007u) {
        insn->mnemonic = "x86dec.d";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc1fu) == 0x00008005u) {
        insn->mnemonic = "x86dec.h";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc1fu) == 0x00008006u) {
        insn->mnemonic = "x86dec.w";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffffffffu) == 0x00008029u) {
        insn->mnemonic = "x86dectop";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xfffffc1fu) == 0x00008000u) {
        insn->mnemonic = "x86inc.b";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc1fu) == 0x00008003u) {
        insn->mnemonic = "x86inc.d";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc1fu) == 0x00008001u) {
        insn->mnemonic = "x86inc.h";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc1fu) == 0x00008002u) {
        insn->mnemonic = "x86inc.w";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffffffffu) == 0x00008009u) {
        insn->mnemonic = "x86inctop";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xfffc03e0u) == 0x005c0000u) {
        insn->mnemonic = "x86mfflag";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffffffe0u) == 0x00007400u) {
        insn->mnemonic = "x86mftop";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        return;
    }
    if ((word & 0xfffc03e0u) == 0x005c0020u) {
        insn->mnemonic = "x86mtflag";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffffff1fu) == 0x00007000u) {
        insn->mnemonic = "x86mttop";
        insn->operand_count = 1;
        insn->operands[0].kind = LA_OP_UIMM;
        insn->operands[0].uimm = (((word >> 5) & 0x7));
        return;
    }
    if ((word & 0xffff801fu) == 0x003e8000u) {
        insn->mnemonic = "x86mul.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003e8004u) {
        insn->mnemonic = "x86mul.bu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003e8003u) {
        insn->mnemonic = "x86mul.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003e8007u) {
        insn->mnemonic = "x86mul.du";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003e8001u) {
        insn->mnemonic = "x86mul.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003e8005u) {
        insn->mnemonic = "x86mul.hu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003e8002u) {
        insn->mnemonic = "x86mul.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003e8006u) {
        insn->mnemonic = "x86mul.wu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8014u) {
        insn->mnemonic = "x86or.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8017u) {
        insn->mnemonic = "x86or.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8015u) {
        insn->mnemonic = "x86or.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8016u) {
        insn->mnemonic = "x86or.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f800cu) {
        insn->mnemonic = "x86rcl.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f800fu) {
        insn->mnemonic = "x86rcl.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f800du) {
        insn->mnemonic = "x86rcl.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f800eu) {
        insn->mnemonic = "x86rcl.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe01fu) == 0x00542018u) {
        insn->mnemonic = "x86rcli.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff001fu) == 0x0055001bu) {
        insn->mnemonic = "x86rcli.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc01fu) == 0x00544019u) {
        insn->mnemonic = "x86rcli.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff801fu) == 0x0054801au) {
        insn->mnemonic = "x86rcli.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8008u) {
        insn->mnemonic = "x86rcr.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f800bu) {
        insn->mnemonic = "x86rcr.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8009u) {
        insn->mnemonic = "x86rcr.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f800au) {
        insn->mnemonic = "x86rcr.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe01fu) == 0x00542010u) {
        insn->mnemonic = "x86rcri.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff001fu) == 0x00550013u) {
        insn->mnemonic = "x86rcri.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc01fu) == 0x00544011u) {
        insn->mnemonic = "x86rcri.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff801fu) == 0x00548012u) {
        insn->mnemonic = "x86rcri.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8004u) {
        insn->mnemonic = "x86rotl.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8007u) {
        insn->mnemonic = "x86rotl.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8005u) {
        insn->mnemonic = "x86rotl.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8006u) {
        insn->mnemonic = "x86rotl.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe01fu) == 0x00542014u) {
        insn->mnemonic = "x86rotli.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff001fu) == 0x00550017u) {
        insn->mnemonic = "x86rotli.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc01fu) == 0x00544015u) {
        insn->mnemonic = "x86rotli.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff801fu) == 0x00548016u) {
        insn->mnemonic = "x86rotli.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8000u) {
        insn->mnemonic = "x86rotr.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8002u) {
        insn->mnemonic = "x86rotr.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8001u) {
        insn->mnemonic = "x86rotr.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8003u) {
        insn->mnemonic = "x86rotr.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe01fu) == 0x0054200cu) {
        insn->mnemonic = "x86rotri.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff001fu) == 0x0055000fu) {
        insn->mnemonic = "x86rotri.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc01fu) == 0x0054400du) {
        insn->mnemonic = "x86rotri.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff801fu) == 0x0054800eu) {
        insn->mnemonic = "x86rotri.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0010u) {
        insn->mnemonic = "x86sbc.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0013u) {
        insn->mnemonic = "x86sbc.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0011u) {
        insn->mnemonic = "x86sbc.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0012u) {
        insn->mnemonic = "x86sbc.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x00580000u) {
        insn->mnemonic = "x86settag";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffffffffu) == 0x00008008u) {
        insn->mnemonic = "x86settm";
        insn->operand_count = 0;
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0014u) {
        insn->mnemonic = "x86sll.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0017u) {
        insn->mnemonic = "x86sll.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0015u) {
        insn->mnemonic = "x86sll.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0016u) {
        insn->mnemonic = "x86sll.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe01fu) == 0x00542000u) {
        insn->mnemonic = "x86slli.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff001fu) == 0x00550003u) {
        insn->mnemonic = "x86slli.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc01fu) == 0x00544001u) {
        insn->mnemonic = "x86slli.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff801fu) == 0x00548002u) {
        insn->mnemonic = "x86slli.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f001cu) {
        insn->mnemonic = "x86sra.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f001fu) {
        insn->mnemonic = "x86sra.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f001du) {
        insn->mnemonic = "x86sra.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f001eu) {
        insn->mnemonic = "x86sra.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe01fu) == 0x00542008u) {
        insn->mnemonic = "x86srai.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff001fu) == 0x0055000bu) {
        insn->mnemonic = "x86srai.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc01fu) == 0x00544009u) {
        insn->mnemonic = "x86srai.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff801fu) == 0x0054800au) {
        insn->mnemonic = "x86srai.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0018u) {
        insn->mnemonic = "x86srl.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f001bu) {
        insn->mnemonic = "x86srl.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0019u) {
        insn->mnemonic = "x86srl.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f001au) {
        insn->mnemonic = "x86srl.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe01fu) == 0x00542004u) {
        insn->mnemonic = "x86srli.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff001fu) == 0x00550007u) {
        insn->mnemonic = "x86srli.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc01fu) == 0x00544005u) {
        insn->mnemonic = "x86srli.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff801fu) == 0x00548006u) {
        insn->mnemonic = "x86srli.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0008u) {
        insn->mnemonic = "x86sub.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f000bu) {
        insn->mnemonic = "x86sub.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0003u) {
        insn->mnemonic = "x86sub.du";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0009u) {
        insn->mnemonic = "x86sub.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f000au) {
        insn->mnemonic = "x86sub.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f0002u) {
        insn->mnemonic = "x86sub.wu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8018u) {
        insn->mnemonic = "x86xor.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f801bu) {
        insn->mnemonic = "x86xor.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f8019u) {
        insn->mnemonic = "x86xor.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff801fu) == 0x003f801au) {
        insn->mnemonic = "x86xor.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 5) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x00158000u) {
        insn->mnemonic = "xor";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x03c00000u) {
        insn->mnemonic = "xori";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xfff));
        return;
    }
    if ((word & 0xffff8000u) == 0x74600000u) {
        insn->mnemonic = "xvabsd.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74620000u) {
        insn->mnemonic = "xvabsd.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74618000u) {
        insn->mnemonic = "xvabsd.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74638000u) {
        insn->mnemonic = "xvabsd.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74608000u) {
        insn->mnemonic = "xvabsd.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74628000u) {
        insn->mnemonic = "xvabsd.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74610000u) {
        insn->mnemonic = "xvabsd.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74630000u) {
        insn->mnemonic = "xvabsd.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x740a0000u) {
        insn->mnemonic = "xvadd.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x740b8000u) {
        insn->mnemonic = "xvadd.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x740a8000u) {
        insn->mnemonic = "xvadd.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x752d0000u) {
        insn->mnemonic = "xvadd.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x740b0000u) {
        insn->mnemonic = "xvadd.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x745c0000u) {
        insn->mnemonic = "xvadda.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x745d8000u) {
        insn->mnemonic = "xvadda.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x745c8000u) {
        insn->mnemonic = "xvadda.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x745d0000u) {
        insn->mnemonic = "xvadda.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x768a0000u) {
        insn->mnemonic = "xvaddi.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x768b8000u) {
        insn->mnemonic = "xvaddi.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x768a8000u) {
        insn->mnemonic = "xvaddi.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x768b0000u) {
        insn->mnemonic = "xvaddi.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x741f0000u) {
        insn->mnemonic = "xvaddwev.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x742f0000u) {
        insn->mnemonic = "xvaddwev.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x743f0000u) {
        insn->mnemonic = "xvaddwev.d.wu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x741e0000u) {
        insn->mnemonic = "xvaddwev.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x742e0000u) {
        insn->mnemonic = "xvaddwev.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x743e0000u) {
        insn->mnemonic = "xvaddwev.h.bu.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x741f8000u) {
        insn->mnemonic = "xvaddwev.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x742f8000u) {
        insn->mnemonic = "xvaddwev.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x743f8000u) {
        insn->mnemonic = "xvaddwev.q.du.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x741e8000u) {
        insn->mnemonic = "xvaddwev.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x742e8000u) {
        insn->mnemonic = "xvaddwev.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x743e8000u) {
        insn->mnemonic = "xvaddwev.w.hu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74230000u) {
        insn->mnemonic = "xvaddwod.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74330000u) {
        insn->mnemonic = "xvaddwod.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74410000u) {
        insn->mnemonic = "xvaddwod.d.wu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74220000u) {
        insn->mnemonic = "xvaddwod.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74320000u) {
        insn->mnemonic = "xvaddwod.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74400000u) {
        insn->mnemonic = "xvaddwod.h.bu.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74238000u) {
        insn->mnemonic = "xvaddwod.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74338000u) {
        insn->mnemonic = "xvaddwod.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74418000u) {
        insn->mnemonic = "xvaddwod.q.du.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74228000u) {
        insn->mnemonic = "xvaddwod.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74328000u) {
        insn->mnemonic = "xvaddwod.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74408000u) {
        insn->mnemonic = "xvaddwod.w.hu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75260000u) {
        insn->mnemonic = "xvand.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77d00000u) {
        insn->mnemonic = "xvandi.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x75280000u) {
        insn->mnemonic = "xvandn.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74640000u) {
        insn->mnemonic = "xvavg.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74660000u) {
        insn->mnemonic = "xvavg.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74658000u) {
        insn->mnemonic = "xvavg.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74678000u) {
        insn->mnemonic = "xvavg.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74648000u) {
        insn->mnemonic = "xvavg.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74668000u) {
        insn->mnemonic = "xvavg.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74650000u) {
        insn->mnemonic = "xvavg.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74670000u) {
        insn->mnemonic = "xvavg.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74680000u) {
        insn->mnemonic = "xvavgr.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x746a0000u) {
        insn->mnemonic = "xvavgr.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74698000u) {
        insn->mnemonic = "xvavgr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x746b8000u) {
        insn->mnemonic = "xvavgr.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74688000u) {
        insn->mnemonic = "xvavgr.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x746a8000u) {
        insn->mnemonic = "xvavgr.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74690000u) {
        insn->mnemonic = "xvavgr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x746b0000u) {
        insn->mnemonic = "xvavgr.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x750c0000u) {
        insn->mnemonic = "xvbitclr.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x750d8000u) {
        insn->mnemonic = "xvbitclr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x750c8000u) {
        insn->mnemonic = "xvbitclr.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x750d0000u) {
        insn->mnemonic = "xvbitclr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x77102000u) {
        insn->mnemonic = "xvbitclri.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x77110000u) {
        insn->mnemonic = "xvbitclri.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77104000u) {
        insn->mnemonic = "xvbitclri.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x77108000u) {
        insn->mnemonic = "xvbitclri.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75100000u) {
        insn->mnemonic = "xvbitrev.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75118000u) {
        insn->mnemonic = "xvbitrev.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75108000u) {
        insn->mnemonic = "xvbitrev.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75110000u) {
        insn->mnemonic = "xvbitrev.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x77182000u) {
        insn->mnemonic = "xvbitrevi.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x77190000u) {
        insn->mnemonic = "xvbitrevi.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77184000u) {
        insn->mnemonic = "xvbitrevi.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x77188000u) {
        insn->mnemonic = "xvbitrevi.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x0d200000u) {
        insn->mnemonic = "xvbitsel.v";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_XVPR;
        insn->operands[3].xvpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77c40000u) {
        insn->mnemonic = "xvbitseli.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x750e0000u) {
        insn->mnemonic = "xvbitset.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x750f8000u) {
        insn->mnemonic = "xvbitset.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x750e8000u) {
        insn->mnemonic = "xvbitset.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x750f0000u) {
        insn->mnemonic = "xvbitset.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x77142000u) {
        insn->mnemonic = "xvbitseti.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x77150000u) {
        insn->mnemonic = "xvbitseti.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77144000u) {
        insn->mnemonic = "xvbitseti.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x77148000u) {
        insn->mnemonic = "xvbitseti.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x768e0000u) {
        insn->mnemonic = "xvbsll.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x768e8000u) {
        insn->mnemonic = "xvbsrl.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c0000u) {
        insn->mnemonic = "xvclo.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c0c00u) {
        insn->mnemonic = "xvclo.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c0400u) {
        insn->mnemonic = "xvclo.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c0800u) {
        insn->mnemonic = "xvclo.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c1000u) {
        insn->mnemonic = "xvclz.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c1c00u) {
        insn->mnemonic = "xvclz.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c1400u) {
        insn->mnemonic = "xvclz.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c1800u) {
        insn->mnemonic = "xvclz.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e00000u) {
        insn->mnemonic = "xvdiv.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e40000u) {
        insn->mnemonic = "xvdiv.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e18000u) {
        insn->mnemonic = "xvdiv.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e58000u) {
        insn->mnemonic = "xvdiv.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e08000u) {
        insn->mnemonic = "xvdiv.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e48000u) {
        insn->mnemonic = "xvdiv.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e10000u) {
        insn->mnemonic = "xvdiv.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e50000u) {
        insn->mnemonic = "xvdiv.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769ee800u) {
        insn->mnemonic = "xvexth.d.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769ef800u) {
        insn->mnemonic = "xvexth.du.wu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769ee000u) {
        insn->mnemonic = "xvexth.h.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769ef000u) {
        insn->mnemonic = "xvexth.hu.bu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769eec00u) {
        insn->mnemonic = "xvexth.q.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769efc00u) {
        insn->mnemonic = "xvexth.qu.du";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769ee400u) {
        insn->mnemonic = "xvexth.w.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769ef400u) {
        insn->mnemonic = "xvexth.wu.hu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x77090000u) {
        insn->mnemonic = "xvextl.q.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x770d0000u) {
        insn->mnemonic = "xvextl.qu.du";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x778c0000u) {
        insn->mnemonic = "xvextrins.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77800000u) {
        insn->mnemonic = "xvextrins.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77880000u) {
        insn->mnemonic = "xvextrins.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77840000u) {
        insn->mnemonic = "xvextrins.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x75310000u) {
        insn->mnemonic = "xvfadd.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75308000u) {
        insn->mnemonic = "xvfadd.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769cd800u) {
        insn->mnemonic = "xvfclass.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769cd400u) {
        insn->mnemonic = "xvfclass.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca00000u) {
        insn->mnemonic = "xvfcmp.caf.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c900000u) {
        insn->mnemonic = "xvfcmp.caf.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca20000u) {
        insn->mnemonic = "xvfcmp.ceq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c920000u) {
        insn->mnemonic = "xvfcmp.ceq.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca30000u) {
        insn->mnemonic = "xvfcmp.cle.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c930000u) {
        insn->mnemonic = "xvfcmp.cle.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca10000u) {
        insn->mnemonic = "xvfcmp.clt.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c910000u) {
        insn->mnemonic = "xvfcmp.clt.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca80000u) {
        insn->mnemonic = "xvfcmp.cne.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c980000u) {
        insn->mnemonic = "xvfcmp.cne.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0caa0000u) {
        insn->mnemonic = "xvfcmp.cor.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c9a0000u) {
        insn->mnemonic = "xvfcmp.cor.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca60000u) {
        insn->mnemonic = "xvfcmp.cueq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c960000u) {
        insn->mnemonic = "xvfcmp.cueq.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca70000u) {
        insn->mnemonic = "xvfcmp.cule.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c970000u) {
        insn->mnemonic = "xvfcmp.cule.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca50000u) {
        insn->mnemonic = "xvfcmp.cult.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c950000u) {
        insn->mnemonic = "xvfcmp.cult.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca40000u) {
        insn->mnemonic = "xvfcmp.cun.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c940000u) {
        insn->mnemonic = "xvfcmp.cun.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0cac0000u) {
        insn->mnemonic = "xvfcmp.cune.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c9c0000u) {
        insn->mnemonic = "xvfcmp.cune.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca08000u) {
        insn->mnemonic = "xvfcmp.saf.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c908000u) {
        insn->mnemonic = "xvfcmp.saf.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca28000u) {
        insn->mnemonic = "xvfcmp.seq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c928000u) {
        insn->mnemonic = "xvfcmp.seq.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca38000u) {
        insn->mnemonic = "xvfcmp.sle.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c938000u) {
        insn->mnemonic = "xvfcmp.sle.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca18000u) {
        insn->mnemonic = "xvfcmp.slt.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c918000u) {
        insn->mnemonic = "xvfcmp.slt.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca88000u) {
        insn->mnemonic = "xvfcmp.sne.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c988000u) {
        insn->mnemonic = "xvfcmp.sne.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0caa8000u) {
        insn->mnemonic = "xvfcmp.sor.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c9a8000u) {
        insn->mnemonic = "xvfcmp.sor.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca68000u) {
        insn->mnemonic = "xvfcmp.sueq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c968000u) {
        insn->mnemonic = "xvfcmp.sueq.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca78000u) {
        insn->mnemonic = "xvfcmp.sule.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c978000u) {
        insn->mnemonic = "xvfcmp.sule.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca58000u) {
        insn->mnemonic = "xvfcmp.sult.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c958000u) {
        insn->mnemonic = "xvfcmp.sult.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0ca48000u) {
        insn->mnemonic = "xvfcmp.sun.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c948000u) {
        insn->mnemonic = "xvfcmp.sun.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0cac8000u) {
        insn->mnemonic = "xvfcmp.sune.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x0c9c8000u) {
        insn->mnemonic = "xvfcmp.sune.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75460000u) {
        insn->mnemonic = "xvfcvt.h.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75468000u) {
        insn->mnemonic = "xvfcvt.s.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769df400u) {
        insn->mnemonic = "xvfcvth.d.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769dec00u) {
        insn->mnemonic = "xvfcvth.s.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769df000u) {
        insn->mnemonic = "xvfcvtl.d.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769de800u) {
        insn->mnemonic = "xvfcvtl.s.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x753b0000u) {
        insn->mnemonic = "xvfdiv.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x753a8000u) {
        insn->mnemonic = "xvfdiv.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e0800u) {
        insn->mnemonic = "xvffint.d.l";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e0c00u) {
        insn->mnemonic = "xvffint.d.lu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75480000u) {
        insn->mnemonic = "xvffint.s.l";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e0000u) {
        insn->mnemonic = "xvffint.s.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e0400u) {
        insn->mnemonic = "xvffint.s.wu";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e1400u) {
        insn->mnemonic = "xvffinth.d.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e1000u) {
        insn->mnemonic = "xvffintl.d.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769cc800u) {
        insn->mnemonic = "xvflogb.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769cc400u) {
        insn->mnemonic = "xvflogb.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x0a200000u) {
        insn->mnemonic = "xvfmadd.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_XVPR;
        insn->operands[3].xvpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x0a100000u) {
        insn->mnemonic = "xvfmadd.s";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_XVPR;
        insn->operands[3].xvpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x753d0000u) {
        insn->mnemonic = "xvfmax.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x753c8000u) {
        insn->mnemonic = "xvfmax.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75410000u) {
        insn->mnemonic = "xvfmaxa.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75408000u) {
        insn->mnemonic = "xvfmaxa.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x753f0000u) {
        insn->mnemonic = "xvfmin.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x753e8000u) {
        insn->mnemonic = "xvfmin.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75430000u) {
        insn->mnemonic = "xvfmina.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75428000u) {
        insn->mnemonic = "xvfmina.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x0a600000u) {
        insn->mnemonic = "xvfmsub.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_XVPR;
        insn->operands[3].xvpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x0a500000u) {
        insn->mnemonic = "xvfmsub.s";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_XVPR;
        insn->operands[3].xvpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75390000u) {
        insn->mnemonic = "xvfmul.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75388000u) {
        insn->mnemonic = "xvfmul.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x0aa00000u) {
        insn->mnemonic = "xvfnmadd.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_XVPR;
        insn->operands[3].xvpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x0a900000u) {
        insn->mnemonic = "xvfnmadd.s";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_XVPR;
        insn->operands[3].xvpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x0ae00000u) {
        insn->mnemonic = "xvfnmsub.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_XVPR;
        insn->operands[3].xvpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x0ad00000u) {
        insn->mnemonic = "xvfnmsub.s";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_XVPR;
        insn->operands[3].xvpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769cf800u) {
        insn->mnemonic = "xvfrecip.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769cf400u) {
        insn->mnemonic = "xvfrecip.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d1800u) {
        insn->mnemonic = "xvfrecipe.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d1400u) {
        insn->mnemonic = "xvfrecipe.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d3800u) {
        insn->mnemonic = "xvfrint.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d3400u) {
        insn->mnemonic = "xvfrint.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d4800u) {
        insn->mnemonic = "xvfrintrm.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d4400u) {
        insn->mnemonic = "xvfrintrm.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d7800u) {
        insn->mnemonic = "xvfrintrne.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d7400u) {
        insn->mnemonic = "xvfrintrne.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d5800u) {
        insn->mnemonic = "xvfrintrp.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d5400u) {
        insn->mnemonic = "xvfrintrp.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d6800u) {
        insn->mnemonic = "xvfrintrz.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d6400u) {
        insn->mnemonic = "xvfrintrz.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d0800u) {
        insn->mnemonic = "xvfrsqrt.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d0400u) {
        insn->mnemonic = "xvfrsqrt.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d2800u) {
        insn->mnemonic = "xvfrsqrte.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769d2400u) {
        insn->mnemonic = "xvfrsqrte.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x752b0000u) {
        insn->mnemonic = "xvfrstp.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x752b8000u) {
        insn->mnemonic = "xvfrstp.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x769a0000u) {
        insn->mnemonic = "xvfrstpi.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x769a8000u) {
        insn->mnemonic = "xvfrstpi.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769ce800u) {
        insn->mnemonic = "xvfsqrt.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769ce400u) {
        insn->mnemonic = "xvfsqrt.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75330000u) {
        insn->mnemonic = "xvfsub.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75328000u) {
        insn->mnemonic = "xvfsub.s";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e3400u) {
        insn->mnemonic = "xvftint.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e5c00u) {
        insn->mnemonic = "xvftint.lu.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75498000u) {
        insn->mnemonic = "xvftint.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e3000u) {
        insn->mnemonic = "xvftint.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e5800u) {
        insn->mnemonic = "xvftint.wu.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e8400u) {
        insn->mnemonic = "xvftinth.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e8000u) {
        insn->mnemonic = "xvftintl.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e3c00u) {
        insn->mnemonic = "xvftintrm.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x754a0000u) {
        insn->mnemonic = "xvftintrm.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e3800u) {
        insn->mnemonic = "xvftintrm.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e8c00u) {
        insn->mnemonic = "xvftintrmh.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e8800u) {
        insn->mnemonic = "xvftintrml.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e5400u) {
        insn->mnemonic = "xvftintrne.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x754b8000u) {
        insn->mnemonic = "xvftintrne.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e5000u) {
        insn->mnemonic = "xvftintrne.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769ea400u) {
        insn->mnemonic = "xvftintrneh.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769ea000u) {
        insn->mnemonic = "xvftintrnel.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e4400u) {
        insn->mnemonic = "xvftintrp.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x754a8000u) {
        insn->mnemonic = "xvftintrp.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e4000u) {
        insn->mnemonic = "xvftintrp.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e9400u) {
        insn->mnemonic = "xvftintrph.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e9000u) {
        insn->mnemonic = "xvftintrpl.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e4c00u) {
        insn->mnemonic = "xvftintrz.l.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e7400u) {
        insn->mnemonic = "xvftintrz.lu.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x754b0000u) {
        insn->mnemonic = "xvftintrz.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e4800u) {
        insn->mnemonic = "xvftintrz.w.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e7000u) {
        insn->mnemonic = "xvftintrz.wu.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e9c00u) {
        insn->mnemonic = "xvftintrzh.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769e9800u) {
        insn->mnemonic = "xvftintrzl.l.s";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74550000u) {
        insn->mnemonic = "xvhaddw.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74590000u) {
        insn->mnemonic = "xvhaddw.du.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74540000u) {
        insn->mnemonic = "xvhaddw.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74580000u) {
        insn->mnemonic = "xvhaddw.hu.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74558000u) {
        insn->mnemonic = "xvhaddw.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74598000u) {
        insn->mnemonic = "xvhaddw.qu.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74548000u) {
        insn->mnemonic = "xvhaddw.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74588000u) {
        insn->mnemonic = "xvhaddw.wu.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74570000u) {
        insn->mnemonic = "xvhsubw.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x745b0000u) {
        insn->mnemonic = "xvhsubw.du.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74560000u) {
        insn->mnemonic = "xvhsubw.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x745a0000u) {
        insn->mnemonic = "xvhsubw.hu.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74578000u) {
        insn->mnemonic = "xvhsubw.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x745b8000u) {
        insn->mnemonic = "xvhsubw.qu.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74568000u) {
        insn->mnemonic = "xvhsubw.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x745a8000u) {
        insn->mnemonic = "xvhsubw.wu.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x751c0000u) {
        insn->mnemonic = "xvilvh.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x751d8000u) {
        insn->mnemonic = "xvilvh.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x751c8000u) {
        insn->mnemonic = "xvilvh.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x751d0000u) {
        insn->mnemonic = "xvilvh.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x751a0000u) {
        insn->mnemonic = "xvilvl.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x751b8000u) {
        insn->mnemonic = "xvilvl.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x751a8000u) {
        insn->mnemonic = "xvilvl.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x751b0000u) {
        insn->mnemonic = "xvilvl.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffff000u) == 0x76ebe000u) {
        insn->mnemonic = "xvinsgr2vr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3));
        return;
    }
    if ((word & 0xffffe000u) == 0x76ebc000u) {
        insn->mnemonic = "xvinsgr2vr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xfffff000u) == 0x76ffe000u) {
        insn->mnemonic = "xvinsve0.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3));
        return;
    }
    if ((word & 0xffffe000u) == 0x76ffc000u) {
        insn->mnemonic = "xvinsve0.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffc00000u) == 0x2c800000u) {
        insn->mnemonic = "xvld";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xfffc0000u) == 0x77e00000u) {
        insn->mnemonic = "xvldi";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_SIMM;
        insn->operands[1].simm = ((int32_t)((word >> 5) & 0x1fff) << 19 >> 19);
        return;
    }
    if ((word & 0xffc00000u) == 0x32800000u) {
        insn->mnemonic = "xvldrepl.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xfff80000u) == 0x32100000u) {
        insn->mnemonic = "xvldrepl.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1ff) << 23 >> 23 << 3);
        return;
    }
    if ((word & 0xffe00000u) == 0x32400000u) {
        insn->mnemonic = "xvldrepl.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x7ff) << 21 >> 21 << 1);
        return;
    }
    if ((word & 0xfff00000u) == 0x32200000u) {
        insn->mnemonic = "xvldrepl.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x3ff) << 22 >> 22 << 2);
        return;
    }
    if ((word & 0xffff8000u) == 0x38480000u) {
        insn->mnemonic = "xvldx";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74a80000u) {
        insn->mnemonic = "xvmadd.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74a98000u) {
        insn->mnemonic = "xvmadd.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74a88000u) {
        insn->mnemonic = "xvmadd.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74a90000u) {
        insn->mnemonic = "xvmadd.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ad0000u) {
        insn->mnemonic = "xvmaddwev.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74b50000u) {
        insn->mnemonic = "xvmaddwev.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74bd0000u) {
        insn->mnemonic = "xvmaddwev.d.wu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ac0000u) {
        insn->mnemonic = "xvmaddwev.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74b40000u) {
        insn->mnemonic = "xvmaddwev.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74bc0000u) {
        insn->mnemonic = "xvmaddwev.h.bu.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ad8000u) {
        insn->mnemonic = "xvmaddwev.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74b58000u) {
        insn->mnemonic = "xvmaddwev.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74bd8000u) {
        insn->mnemonic = "xvmaddwev.q.du.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ac8000u) {
        insn->mnemonic = "xvmaddwev.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74b48000u) {
        insn->mnemonic = "xvmaddwev.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74bc8000u) {
        insn->mnemonic = "xvmaddwev.w.hu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74af0000u) {
        insn->mnemonic = "xvmaddwod.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74b70000u) {
        insn->mnemonic = "xvmaddwod.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74bf0000u) {
        insn->mnemonic = "xvmaddwod.d.wu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ae0000u) {
        insn->mnemonic = "xvmaddwod.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74b60000u) {
        insn->mnemonic = "xvmaddwod.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74be0000u) {
        insn->mnemonic = "xvmaddwod.h.bu.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74af8000u) {
        insn->mnemonic = "xvmaddwod.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74b78000u) {
        insn->mnemonic = "xvmaddwod.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74bf8000u) {
        insn->mnemonic = "xvmaddwod.q.du.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ae8000u) {
        insn->mnemonic = "xvmaddwod.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74b68000u) {
        insn->mnemonic = "xvmaddwod.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74be8000u) {
        insn->mnemonic = "xvmaddwod.w.hu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74700000u) {
        insn->mnemonic = "xvmax.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74740000u) {
        insn->mnemonic = "xvmax.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74718000u) {
        insn->mnemonic = "xvmax.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74758000u) {
        insn->mnemonic = "xvmax.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74708000u) {
        insn->mnemonic = "xvmax.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74748000u) {
        insn->mnemonic = "xvmax.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74710000u) {
        insn->mnemonic = "xvmax.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74750000u) {
        insn->mnemonic = "xvmax.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76900000u) {
        insn->mnemonic = "xvmaxi.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76940000u) {
        insn->mnemonic = "xvmaxi.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76918000u) {
        insn->mnemonic = "xvmaxi.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76958000u) {
        insn->mnemonic = "xvmaxi.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76908000u) {
        insn->mnemonic = "xvmaxi.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76948000u) {
        insn->mnemonic = "xvmaxi.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76910000u) {
        insn->mnemonic = "xvmaxi.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76950000u) {
        insn->mnemonic = "xvmaxi.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x769b8000u) {
        insn->mnemonic = "xvmepatmsk.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_UIMM;
        insn->operands[1].uimm = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74720000u) {
        insn->mnemonic = "xvmin.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74760000u) {
        insn->mnemonic = "xvmin.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74738000u) {
        insn->mnemonic = "xvmin.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74778000u) {
        insn->mnemonic = "xvmin.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74728000u) {
        insn->mnemonic = "xvmin.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74768000u) {
        insn->mnemonic = "xvmin.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74730000u) {
        insn->mnemonic = "xvmin.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74770000u) {
        insn->mnemonic = "xvmin.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76920000u) {
        insn->mnemonic = "xvmini.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76960000u) {
        insn->mnemonic = "xvmini.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76938000u) {
        insn->mnemonic = "xvmini.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76978000u) {
        insn->mnemonic = "xvmini.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76928000u) {
        insn->mnemonic = "xvmini.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76968000u) {
        insn->mnemonic = "xvmini.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76930000u) {
        insn->mnemonic = "xvmini.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76970000u) {
        insn->mnemonic = "xvmini.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e20000u) {
        insn->mnemonic = "xvmod.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e60000u) {
        insn->mnemonic = "xvmod.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e38000u) {
        insn->mnemonic = "xvmod.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e78000u) {
        insn->mnemonic = "xvmod.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e28000u) {
        insn->mnemonic = "xvmod.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e68000u) {
        insn->mnemonic = "xvmod.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e30000u) {
        insn->mnemonic = "xvmod.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e70000u) {
        insn->mnemonic = "xvmod.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c5000u) {
        insn->mnemonic = "xvmskgez.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c4000u) {
        insn->mnemonic = "xvmskltz.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c4c00u) {
        insn->mnemonic = "xvmskltz.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c4400u) {
        insn->mnemonic = "xvmskltz.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c4800u) {
        insn->mnemonic = "xvmskltz.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c6000u) {
        insn->mnemonic = "xvmsknz.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74aa0000u) {
        insn->mnemonic = "xvmsub.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ab8000u) {
        insn->mnemonic = "xvmsub.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74aa8000u) {
        insn->mnemonic = "xvmsub.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ab0000u) {
        insn->mnemonic = "xvmsub.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74860000u) {
        insn->mnemonic = "xvmuh.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74880000u) {
        insn->mnemonic = "xvmuh.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74878000u) {
        insn->mnemonic = "xvmuh.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74898000u) {
        insn->mnemonic = "xvmuh.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74868000u) {
        insn->mnemonic = "xvmuh.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74888000u) {
        insn->mnemonic = "xvmuh.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74870000u) {
        insn->mnemonic = "xvmuh.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74890000u) {
        insn->mnemonic = "xvmuh.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74840000u) {
        insn->mnemonic = "xvmul.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74858000u) {
        insn->mnemonic = "xvmul.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74848000u) {
        insn->mnemonic = "xvmul.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74850000u) {
        insn->mnemonic = "xvmul.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74910000u) {
        insn->mnemonic = "xvmulwev.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74990000u) {
        insn->mnemonic = "xvmulwev.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74a10000u) {
        insn->mnemonic = "xvmulwev.d.wu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74900000u) {
        insn->mnemonic = "xvmulwev.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74980000u) {
        insn->mnemonic = "xvmulwev.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74a00000u) {
        insn->mnemonic = "xvmulwev.h.bu.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74918000u) {
        insn->mnemonic = "xvmulwev.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74998000u) {
        insn->mnemonic = "xvmulwev.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74a18000u) {
        insn->mnemonic = "xvmulwev.q.du.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74908000u) {
        insn->mnemonic = "xvmulwev.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74988000u) {
        insn->mnemonic = "xvmulwev.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74a08000u) {
        insn->mnemonic = "xvmulwev.w.hu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74930000u) {
        insn->mnemonic = "xvmulwod.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x749b0000u) {
        insn->mnemonic = "xvmulwod.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74a30000u) {
        insn->mnemonic = "xvmulwod.d.wu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74920000u) {
        insn->mnemonic = "xvmulwod.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x749a0000u) {
        insn->mnemonic = "xvmulwod.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74a20000u) {
        insn->mnemonic = "xvmulwod.h.bu.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74938000u) {
        insn->mnemonic = "xvmulwod.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x749b8000u) {
        insn->mnemonic = "xvmulwod.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74a38000u) {
        insn->mnemonic = "xvmulwod.q.du.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74928000u) {
        insn->mnemonic = "xvmulwod.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x749a8000u) {
        insn->mnemonic = "xvmulwod.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74a28000u) {
        insn->mnemonic = "xvmulwod.w.hu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c3000u) {
        insn->mnemonic = "xvneg.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c3c00u) {
        insn->mnemonic = "xvneg.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c3400u) {
        insn->mnemonic = "xvneg.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c3800u) {
        insn->mnemonic = "xvneg.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75278000u) {
        insn->mnemonic = "xvnor.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77dc0000u) {
        insn->mnemonic = "xvnori.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x75268000u) {
        insn->mnemonic = "xvor.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77d40000u) {
        insn->mnemonic = "xvori.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x75288000u) {
        insn->mnemonic = "xvorn.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75160000u) {
        insn->mnemonic = "xvpackev.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75178000u) {
        insn->mnemonic = "xvpackev.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75168000u) {
        insn->mnemonic = "xvpackev.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75170000u) {
        insn->mnemonic = "xvpackev.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75180000u) {
        insn->mnemonic = "xvpackod.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75198000u) {
        insn->mnemonic = "xvpackod.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75188000u) {
        insn->mnemonic = "xvpackod.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75190000u) {
        insn->mnemonic = "xvpackod.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c2000u) {
        insn->mnemonic = "xvpcnt.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c2c00u) {
        insn->mnemonic = "xvpcnt.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c2400u) {
        insn->mnemonic = "xvpcnt.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769c2800u) {
        insn->mnemonic = "xvpcnt.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x757d0000u) {
        insn->mnemonic = "xvperm.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77e80000u) {
        insn->mnemonic = "xvpermi.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77ec0000u) {
        insn->mnemonic = "xvpermi.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77e40000u) {
        insn->mnemonic = "xvpermi.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffff8000u) == 0x751e0000u) {
        insn->mnemonic = "xvpickev.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x751f8000u) {
        insn->mnemonic = "xvpickev.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x751e8000u) {
        insn->mnemonic = "xvpickev.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x751f0000u) {
        insn->mnemonic = "xvpickev.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75200000u) {
        insn->mnemonic = "xvpickod.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75218000u) {
        insn->mnemonic = "xvpickod.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75208000u) {
        insn->mnemonic = "xvpickod.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75210000u) {
        insn->mnemonic = "xvpickod.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffff000u) == 0x76efe000u) {
        insn->mnemonic = "xvpickve2gr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3));
        return;
    }
    if ((word & 0xfffff000u) == 0x76f3e000u) {
        insn->mnemonic = "xvpickve2gr.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3));
        return;
    }
    if ((word & 0xffffe000u) == 0x76efc000u) {
        insn->mnemonic = "xvpickve2gr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffffe000u) == 0x76f3c000u) {
        insn->mnemonic = "xvpickve2gr.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_GPR;
        insn->operands[0].gpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xfffff000u) == 0x7703e000u) {
        insn->mnemonic = "xvpickve.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3));
        return;
    }
    if ((word & 0xffffe000u) == 0x7703c000u) {
        insn->mnemonic = "xvpickve.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffffc000u) == 0x76f78000u) {
        insn->mnemonic = "xvrepl128vei.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffff800u) == 0x76f7f000u) {
        insn->mnemonic = "xvrepl128vei.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1));
        return;
    }
    if ((word & 0xffffe000u) == 0x76f7c000u) {
        insn->mnemonic = "xvrepl128vei.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xfffff000u) == 0x76f7e000u) {
        insn->mnemonic = "xvrepl128vei.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f0000u) {
        insn->mnemonic = "xvreplgr2vr.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f0c00u) {
        insn->mnemonic = "xvreplgr2vr.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f0400u) {
        insn->mnemonic = "xvreplgr2vr.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x769f0800u) {
        insn->mnemonic = "xvreplgr2vr.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x77070000u) {
        insn->mnemonic = "xvreplve0.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x7707e000u) {
        insn->mnemonic = "xvreplve0.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x77078000u) {
        insn->mnemonic = "xvreplve0.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x7707f000u) {
        insn->mnemonic = "xvreplve0.q";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc00u) == 0x7707c000u) {
        insn->mnemonic = "xvreplve0.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75220000u) {
        insn->mnemonic = "xvreplve.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75238000u) {
        insn->mnemonic = "xvreplve.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75228000u) {
        insn->mnemonic = "xvreplve.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75230000u) {
        insn->mnemonic = "xvreplve.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ee0000u) {
        insn->mnemonic = "xvrotr.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ef8000u) {
        insn->mnemonic = "xvrotr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ee8000u) {
        insn->mnemonic = "xvrotr.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ef0000u) {
        insn->mnemonic = "xvrotr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x76a02000u) {
        insn->mnemonic = "xvrotri.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x76a10000u) {
        insn->mnemonic = "xvrotri.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x76a04000u) {
        insn->mnemonic = "xvrotri.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x76a08000u) {
        insn->mnemonic = "xvrotri.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74460000u) {
        insn->mnemonic = "xvsadd.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x744a0000u) {
        insn->mnemonic = "xvsadd.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74478000u) {
        insn->mnemonic = "xvsadd.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x744b8000u) {
        insn->mnemonic = "xvsadd.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74468000u) {
        insn->mnemonic = "xvsadd.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x744a8000u) {
        insn->mnemonic = "xvsadd.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74470000u) {
        insn->mnemonic = "xvsadd.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x744b0000u) {
        insn->mnemonic = "xvsadd.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x77242000u) {
        insn->mnemonic = "xvsat.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffffe000u) == 0x77282000u) {
        insn->mnemonic = "xvsat.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x77250000u) {
        insn->mnemonic = "xvsat.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff0000u) == 0x77290000u) {
        insn->mnemonic = "xvsat.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77244000u) {
        insn->mnemonic = "xvsat.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x77284000u) {
        insn->mnemonic = "xvsat.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x77248000u) {
        insn->mnemonic = "xvsat.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x77288000u) {
        insn->mnemonic = "xvsat.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74000000u) {
        insn->mnemonic = "xvseq.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74018000u) {
        insn->mnemonic = "xvseq.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74008000u) {
        insn->mnemonic = "xvseq.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74010000u) {
        insn->mnemonic = "xvseq.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76800000u) {
        insn->mnemonic = "xvseqi.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76818000u) {
        insn->mnemonic = "xvseqi.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76808000u) {
        insn->mnemonic = "xvseqi.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76810000u) {
        insn->mnemonic = "xvseqi.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xfffffc18u) == 0x769cb000u) {
        insn->mnemonic = "xvsetallnez.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x769cbc00u) {
        insn->mnemonic = "xvsetallnez.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x769cb400u) {
        insn->mnemonic = "xvsetallnez.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x769cb800u) {
        insn->mnemonic = "xvsetallnez.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x769ca000u) {
        insn->mnemonic = "xvsetanyeqz.b";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x769cac00u) {
        insn->mnemonic = "xvsetanyeqz.d";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x769ca400u) {
        insn->mnemonic = "xvsetanyeqz.h";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x769ca800u) {
        insn->mnemonic = "xvsetanyeqz.w";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x769c9800u) {
        insn->mnemonic = "xvseteqz.v";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffffc18u) == 0x769c9c00u) {
        insn->mnemonic = "xvsetnez.v";
        insn->operand_count = 2;
        insn->operands[0].kind = LA_OP_FCC;
        insn->operands[0].fcc = (((word >> 0) & 0x7));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77900000u) {
        insn->mnemonic = "xvshuf4i.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x779c0000u) {
        insn->mnemonic = "xvshuf4i.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77940000u) {
        insn->mnemonic = "xvshuf4i.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77980000u) {
        insn->mnemonic = "xvshuf4i.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xfff00000u) == 0x0d600000u) {
        insn->mnemonic = "xvshuf.b";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        insn->operands[3].kind = LA_OP_XVPR;
        insn->operands[3].xvpr = (((word >> 15) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x757b8000u) {
        insn->mnemonic = "xvshuf.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x757a8000u) {
        insn->mnemonic = "xvshuf.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x757b0000u) {
        insn->mnemonic = "xvshuf.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x752e0000u) {
        insn->mnemonic = "xvsigncov.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x752f8000u) {
        insn->mnemonic = "xvsigncov.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x752e8000u) {
        insn->mnemonic = "xvsigncov.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x752f0000u) {
        insn->mnemonic = "xvsigncov.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74020000u) {
        insn->mnemonic = "xvsle.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74040000u) {
        insn->mnemonic = "xvsle.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74038000u) {
        insn->mnemonic = "xvsle.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74058000u) {
        insn->mnemonic = "xvsle.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74028000u) {
        insn->mnemonic = "xvsle.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74048000u) {
        insn->mnemonic = "xvsle.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74030000u) {
        insn->mnemonic = "xvsle.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74050000u) {
        insn->mnemonic = "xvsle.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76820000u) {
        insn->mnemonic = "xvslei.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76840000u) {
        insn->mnemonic = "xvslei.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76838000u) {
        insn->mnemonic = "xvslei.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76858000u) {
        insn->mnemonic = "xvslei.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76828000u) {
        insn->mnemonic = "xvslei.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76848000u) {
        insn->mnemonic = "xvslei.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76830000u) {
        insn->mnemonic = "xvslei.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76850000u) {
        insn->mnemonic = "xvslei.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e80000u) {
        insn->mnemonic = "xvsll.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e98000u) {
        insn->mnemonic = "xvsll.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e88000u) {
        insn->mnemonic = "xvsll.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74e90000u) {
        insn->mnemonic = "xvsll.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x772c2000u) {
        insn->mnemonic = "xvslli.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x772d0000u) {
        insn->mnemonic = "xvslli.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x772c4000u) {
        insn->mnemonic = "xvslli.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x772c8000u) {
        insn->mnemonic = "xvslli.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x77088000u) {
        insn->mnemonic = "xvsllwil.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x770c8000u) {
        insn->mnemonic = "xvsllwil.du.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x77082000u) {
        insn->mnemonic = "xvsllwil.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffffe000u) == 0x770c2000u) {
        insn->mnemonic = "xvsllwil.hu.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffffc000u) == 0x77084000u) {
        insn->mnemonic = "xvsllwil.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x770c4000u) {
        insn->mnemonic = "xvsllwil.wu.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x74060000u) {
        insn->mnemonic = "xvslt.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74080000u) {
        insn->mnemonic = "xvslt.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74078000u) {
        insn->mnemonic = "xvslt.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74098000u) {
        insn->mnemonic = "xvslt.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74068000u) {
        insn->mnemonic = "xvslt.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74088000u) {
        insn->mnemonic = "xvslt.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74070000u) {
        insn->mnemonic = "xvslt.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74090000u) {
        insn->mnemonic = "xvslt.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76860000u) {
        insn->mnemonic = "xvslti.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76880000u) {
        insn->mnemonic = "xvslti.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76878000u) {
        insn->mnemonic = "xvslti.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76898000u) {
        insn->mnemonic = "xvslti.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76868000u) {
        insn->mnemonic = "xvslti.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76888000u) {
        insn->mnemonic = "xvslti.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x76870000u) {
        insn->mnemonic = "xvslti.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0x1f) << 27 >> 27);
        return;
    }
    if ((word & 0xffff8000u) == 0x76890000u) {
        insn->mnemonic = "xvslti.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ec0000u) {
        insn->mnemonic = "xvsra.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ed8000u) {
        insn->mnemonic = "xvsra.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ec8000u) {
        insn->mnemonic = "xvsra.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ed0000u) {
        insn->mnemonic = "xvsra.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x77342000u) {
        insn->mnemonic = "xvsrai.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x77350000u) {
        insn->mnemonic = "xvsrai.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77344000u) {
        insn->mnemonic = "xvsrai.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x77348000u) {
        insn->mnemonic = "xvsrai.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f68000u) {
        insn->mnemonic = "xvsran.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f70000u) {
        insn->mnemonic = "xvsran.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f78000u) {
        insn->mnemonic = "xvsran.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77584000u) {
        insn->mnemonic = "xvsrani.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x775a0000u) {
        insn->mnemonic = "xvsrani.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x77588000u) {
        insn->mnemonic = "xvsrani.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x77590000u) {
        insn->mnemonic = "xvsrani.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f20000u) {
        insn->mnemonic = "xvsrar.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f38000u) {
        insn->mnemonic = "xvsrar.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f28000u) {
        insn->mnemonic = "xvsrar.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f30000u) {
        insn->mnemonic = "xvsrar.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x76a82000u) {
        insn->mnemonic = "xvsrari.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x76a90000u) {
        insn->mnemonic = "xvsrari.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x76a84000u) {
        insn->mnemonic = "xvsrari.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x76a88000u) {
        insn->mnemonic = "xvsrari.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74fa8000u) {
        insn->mnemonic = "xvsrarn.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74fb0000u) {
        insn->mnemonic = "xvsrarn.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74fb8000u) {
        insn->mnemonic = "xvsrarn.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x775c4000u) {
        insn->mnemonic = "xvsrarni.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x775e0000u) {
        insn->mnemonic = "xvsrarni.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x775c8000u) {
        insn->mnemonic = "xvsrarni.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x775d0000u) {
        insn->mnemonic = "xvsrarni.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ea0000u) {
        insn->mnemonic = "xvsrl.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74eb8000u) {
        insn->mnemonic = "xvsrl.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ea8000u) {
        insn->mnemonic = "xvsrl.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74eb0000u) {
        insn->mnemonic = "xvsrl.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x77302000u) {
        insn->mnemonic = "xvsrli.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x77310000u) {
        insn->mnemonic = "xvsrli.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77304000u) {
        insn->mnemonic = "xvsrli.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x77308000u) {
        insn->mnemonic = "xvsrli.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f48000u) {
        insn->mnemonic = "xvsrln.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f50000u) {
        insn->mnemonic = "xvsrln.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f58000u) {
        insn->mnemonic = "xvsrln.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77404000u) {
        insn->mnemonic = "xvsrlni.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x77420000u) {
        insn->mnemonic = "xvsrlni.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x77408000u) {
        insn->mnemonic = "xvsrlni.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x77410000u) {
        insn->mnemonic = "xvsrlni.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f00000u) {
        insn->mnemonic = "xvsrlr.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f18000u) {
        insn->mnemonic = "xvsrlr.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f08000u) {
        insn->mnemonic = "xvsrlr.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f10000u) {
        insn->mnemonic = "xvsrlr.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffe000u) == 0x76a42000u) {
        insn->mnemonic = "xvsrlri.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7));
        return;
    }
    if ((word & 0xffff0000u) == 0x76a50000u) {
        insn->mnemonic = "xvsrlri.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffffc000u) == 0x76a44000u) {
        insn->mnemonic = "xvsrlri.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffff8000u) == 0x76a48000u) {
        insn->mnemonic = "xvsrlri.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f88000u) {
        insn->mnemonic = "xvsrlrn.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f90000u) {
        insn->mnemonic = "xvsrlrn.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74f98000u) {
        insn->mnemonic = "xvsrlrn.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77444000u) {
        insn->mnemonic = "xvsrlrni.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x77460000u) {
        insn->mnemonic = "xvsrlrni.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x77448000u) {
        insn->mnemonic = "xvsrlrni.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x77450000u) {
        insn->mnemonic = "xvsrlrni.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74fe8000u) {
        insn->mnemonic = "xvssran.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75068000u) {
        insn->mnemonic = "xvssran.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ff0000u) {
        insn->mnemonic = "xvssran.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75070000u) {
        insn->mnemonic = "xvssran.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74ff8000u) {
        insn->mnemonic = "xvssran.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75078000u) {
        insn->mnemonic = "xvssran.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77604000u) {
        insn->mnemonic = "xvssrani.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x77644000u) {
        insn->mnemonic = "xvssrani.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x77620000u) {
        insn->mnemonic = "xvssrani.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xfffe0000u) == 0x77660000u) {
        insn->mnemonic = "xvssrani.du.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x77608000u) {
        insn->mnemonic = "xvssrani.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x77648000u) {
        insn->mnemonic = "xvssrani.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x77610000u) {
        insn->mnemonic = "xvssrani.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff0000u) == 0x77650000u) {
        insn->mnemonic = "xvssrani.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75028000u) {
        insn->mnemonic = "xvssrarn.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x750a8000u) {
        insn->mnemonic = "xvssrarn.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75030000u) {
        insn->mnemonic = "xvssrarn.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x750b0000u) {
        insn->mnemonic = "xvssrarn.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75038000u) {
        insn->mnemonic = "xvssrarn.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x750b8000u) {
        insn->mnemonic = "xvssrarn.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77684000u) {
        insn->mnemonic = "xvssrarni.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x776c4000u) {
        insn->mnemonic = "xvssrarni.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x776a0000u) {
        insn->mnemonic = "xvssrarni.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xfffe0000u) == 0x776e0000u) {
        insn->mnemonic = "xvssrarni.du.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x77688000u) {
        insn->mnemonic = "xvssrarni.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x776c8000u) {
        insn->mnemonic = "xvssrarni.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x77690000u) {
        insn->mnemonic = "xvssrarni.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff0000u) == 0x776d0000u) {
        insn->mnemonic = "xvssrarni.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74fc8000u) {
        insn->mnemonic = "xvssrln.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75048000u) {
        insn->mnemonic = "xvssrln.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74fd0000u) {
        insn->mnemonic = "xvssrln.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75050000u) {
        insn->mnemonic = "xvssrln.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74fd8000u) {
        insn->mnemonic = "xvssrln.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75058000u) {
        insn->mnemonic = "xvssrln.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77484000u) {
        insn->mnemonic = "xvssrlni.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x774c4000u) {
        insn->mnemonic = "xvssrlni.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x774a0000u) {
        insn->mnemonic = "xvssrlni.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xfffe0000u) == 0x774e0000u) {
        insn->mnemonic = "xvssrlni.du.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x77488000u) {
        insn->mnemonic = "xvssrlni.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x774c8000u) {
        insn->mnemonic = "xvssrlni.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x77490000u) {
        insn->mnemonic = "xvssrlni.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff0000u) == 0x774d0000u) {
        insn->mnemonic = "xvssrlni.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75008000u) {
        insn->mnemonic = "xvssrlrn.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75088000u) {
        insn->mnemonic = "xvssrlrn.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75010000u) {
        insn->mnemonic = "xvssrlrn.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75090000u) {
        insn->mnemonic = "xvssrlrn.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75018000u) {
        insn->mnemonic = "xvssrlrn.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75098000u) {
        insn->mnemonic = "xvssrlrn.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffffc000u) == 0x77504000u) {
        insn->mnemonic = "xvssrlrni.b.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xffffc000u) == 0x77544000u) {
        insn->mnemonic = "xvssrlrni.bu.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xf));
        return;
    }
    if ((word & 0xfffe0000u) == 0x77520000u) {
        insn->mnemonic = "xvssrlrni.d.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xfffe0000u) == 0x77560000u) {
        insn->mnemonic = "xvssrlrni.du.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x7f));
        return;
    }
    if ((word & 0xffff8000u) == 0x77508000u) {
        insn->mnemonic = "xvssrlrni.h.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x77548000u) {
        insn->mnemonic = "xvssrlrni.hu.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff0000u) == 0x77510000u) {
        insn->mnemonic = "xvssrlrni.w.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff0000u) == 0x77550000u) {
        insn->mnemonic = "xvssrlrni.wu.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x3f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74480000u) {
        insn->mnemonic = "xvssub.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x744c0000u) {
        insn->mnemonic = "xvssub.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74498000u) {
        insn->mnemonic = "xvssub.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x744d8000u) {
        insn->mnemonic = "xvssub.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74488000u) {
        insn->mnemonic = "xvssub.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x744c8000u) {
        insn->mnemonic = "xvssub.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74490000u) {
        insn->mnemonic = "xvssub.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x744d0000u) {
        insn->mnemonic = "xvssub.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffc00000u) == 0x2cc00000u) {
        insn->mnemonic = "xvst";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xfff) << 20 >> 20);
        return;
    }
    if ((word & 0xff800000u) == 0x33800000u) {
        insn->mnemonic = "xvstelm.b";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xff) << 24 >> 24);
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 18) & 0x1f));
        return;
    }
    if ((word & 0xfff00000u) == 0x33100000u) {
        insn->mnemonic = "xvstelm.d";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xff) << 24 >> 24 << 3);
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 18) & 0x3));
        return;
    }
    if ((word & 0xffc00000u) == 0x33400000u) {
        insn->mnemonic = "xvstelm.h";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xff) << 24 >> 24 << 1);
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 18) & 0xf));
        return;
    }
    if ((word & 0xffe00000u) == 0x33200000u) {
        insn->mnemonic = "xvstelm.w";
        insn->operand_count = 4;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_SIMM;
        insn->operands[2].simm = ((int32_t)((word >> 10) & 0xff) << 24 >> 24 << 2);
        insn->operands[3].kind = LA_OP_UIMM;
        insn->operands[3].uimm = (((word >> 18) & 0x7));
        return;
    }
    if ((word & 0xffff8000u) == 0x384c0000u) {
        insn->mnemonic = "xvstx";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_GPR;
        insn->operands[1].gpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_GPR;
        insn->operands[2].gpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x740c0000u) {
        insn->mnemonic = "xvsub.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x740d8000u) {
        insn->mnemonic = "xvsub.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x740c8000u) {
        insn->mnemonic = "xvsub.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x752d8000u) {
        insn->mnemonic = "xvsub.q";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x740d0000u) {
        insn->mnemonic = "xvsub.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x768c0000u) {
        insn->mnemonic = "xvsubi.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x768d8000u) {
        insn->mnemonic = "xvsubi.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x768c8000u) {
        insn->mnemonic = "xvsubi.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x768d0000u) {
        insn->mnemonic = "xvsubi.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74210000u) {
        insn->mnemonic = "xvsubwev.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74310000u) {
        insn->mnemonic = "xvsubwev.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74200000u) {
        insn->mnemonic = "xvsubwev.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74300000u) {
        insn->mnemonic = "xvsubwev.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74218000u) {
        insn->mnemonic = "xvsubwev.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74318000u) {
        insn->mnemonic = "xvsubwev.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74208000u) {
        insn->mnemonic = "xvsubwev.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74308000u) {
        insn->mnemonic = "xvsubwev.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74250000u) {
        insn->mnemonic = "xvsubwod.d.w";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74350000u) {
        insn->mnemonic = "xvsubwod.d.wu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74240000u) {
        insn->mnemonic = "xvsubwod.h.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74340000u) {
        insn->mnemonic = "xvsubwod.h.bu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74258000u) {
        insn->mnemonic = "xvsubwod.q.d";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74358000u) {
        insn->mnemonic = "xvsubwod.q.du";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74248000u) {
        insn->mnemonic = "xvsubwod.w.h";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x74348000u) {
        insn->mnemonic = "xvsubwod.w.hu";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xffff8000u) == 0x75270000u) {
        insn->mnemonic = "xvxor.v";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_XVPR;
        insn->operands[2].xvpr = (((word >> 10) & 0x1f));
        return;
    }
    if ((word & 0xfffc0000u) == 0x77d80000u) {
        insn->mnemonic = "xvxori.b";
        insn->operand_count = 3;
        insn->operands[0].kind = LA_OP_XVPR;
        insn->operands[0].xvpr = (((word >> 0) & 0x1f));
        insn->operands[1].kind = LA_OP_XVPR;
        insn->operands[1].xvpr = (((word >> 5) & 0x1f));
        insn->operands[2].kind = LA_OP_UIMM;
        insn->operands[2].uimm = (((word >> 10) & 0xff));
        return;
    }
    if ((word & 0xffffffffu) == 0x06493000u) {
        insn->mnemonic = "xxx.unknown.1";
        insn->operand_count = 0;
        return;
    }

    //// ANCHOR: disasm end
}

void la_insn_to_str(const lagoon_insn_t* insn, char* buf, size_t buf_size)
{
    if (insn->mnemonic == NULL) {
        snprintf(buf, buf_size, "%08X ???", __builtin_bswap32(insn->encoding));
        return;
    }

    int pos = snprintf(buf, buf_size, "%-15s", insn->mnemonic);

    for (int i = 0; i < insn->operand_count && pos < (int)buf_size - 1; i++) {
        const la_operand_t* op = &insn->operands[i];
        char tmp[32];
        switch (op->kind) {
            case LA_OP_GPR:
                snprintf(tmp, sizeof(tmp), "%s", Xt[op->gpr & 31]);
                break;
            case LA_OP_FPR:
                snprintf(tmp, sizeof(tmp), "%s", Ft[op->fpr & 31]);
                break;
            case LA_OP_VPR:
                snprintf(tmp, sizeof(tmp), "%s", Vt[op->vpr & 31]);
                break;
            case LA_OP_XVPR:
                snprintf(tmp, sizeof(tmp), "%s", XVt[op->xvpr & 31]);
                break;
            case LA_OP_FCC:
                snprintf(tmp, sizeof(tmp), "$fcc%d", op->fcc & 7);
                break;
            case LA_OP_SCR:
                snprintf(tmp, sizeof(tmp), "$scr%d", op->scr & 3);
                break;
            case LA_OP_FCSR:
                snprintf(tmp, sizeof(tmp), "$fcsr%d", op->fcsr & 3);
                break;
            case LA_OP_SIMM:
                snprintf(tmp, sizeof(tmp), "0x%x(%d)", op->simm, op->simm);
                break;
            case LA_OP_UIMM: {
                snprintf(tmp, sizeof(tmp), "0x%x(%u)", op->uimm, op->uimm);
            } break;
        }
        pos += snprintf(buf + pos, buf_size - (size_t)pos, "%s%s",
            i == 0 ? " " : ", ", tmp);
    }
}

const char* la64_print(uint32_t opcode, uint64_t addr)
{
    static char buf[128];
    lagoon_insn_t insn;
    la_disasm_one(opcode, &insn);
    la_insn_to_str(&insn, buf, sizeof(buf));
    return buf;
}
