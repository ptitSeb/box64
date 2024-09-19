#define INIT
#define FINI        \
    if(ninst)       \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst].x64.size, dyn->insts[ninst].size/4); \
    addInst(dyn->instsize, &dyn->insts_size, 0, 0);
#define EMIT(A)     \
    do {                                                \
        if(box64_dynarec_dump) print_opcode(dyn, ninst, (uint32_t)A); \
        *(uint32_t*)(dyn->block) = (uint32_t)(A);       \
        dyn->block += 4; dyn->native_size += 4;         \
        dyn->insts[ninst].size2 += 4;                   \
    }while(0)

#define MESSAGE(A, ...)  if(box64_dynarec_dump) dynarec_log(LOG_NONE, __VA_ARGS__)
#define NEW_INST                                                                                                  \
    dyn->vector_sew = dyn->insts[ninst].vector_sew_entry;                                                         \
    if (box64_dynarec_dump) print_newinst(dyn, ninst);                                                            \
    if (ninst) {                                                                                                  \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst - 1].x64.size, dyn->insts[ninst - 1].size / 4); \
        dyn->insts[ninst].ymm0_pass3 = dyn->ymm_zero;                                                             \
    }
#define INST_EPILOG
#define INST_NAME(name) inst_name_pass3(dyn, ninst, name, rex)

#define TABLE64(A, V)   {int val64offset = Table64(dyn, (V), 3); MESSAGE(LOG_DUMP, "  Table64: 0x%lx\n", (V)); AUIPC(A, SPLIT20(val64offset)); LD(A, A, SPLIT12(val64offset));}
#define FTABLE64(A, V)  {mmx87_regs_t v = {.d = V}; int val64offset = Table64(dyn, v.q, 3); MESSAGE(LOG_DUMP, "  FTable64: %g\n", v.d); AUIPC(x1, SPLIT20(val64offset)); FLD(A, x1, SPLIT12(val64offset));}

#define DEFAULT_VECTOR                                                                                       \
    if (box64_dynarec_log >= LOG_INFO || box64_dynarec_dump || box64_dynarec_missing) {                      \
        dynarec_log(LOG_NONE, "%p: Dynarec fallback to scalar version because of %sOpcode"                   \
                              " %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", \
            (void*)ip, rex.is32bits ? "x86 " : "x64 ",                                                       \
            PKip(0),                                                                                         \
            PKip(1), PKip(2), PKip(3),                                                                       \
            PKip(4), PKip(5), PKip(6),                                                                       \
            PKip(7), PKip(8), PKip(9),                                                                       \
            PKip(10), PKip(11), PKip(12),                                                                    \
            PKip(13), PKip(14));                                                                             \
        printFunctionAddr(ip, " => ");                                                                       \
        dynarec_log(LOG_NONE, "\n");                                                                         \
    }                                                                                                        \
    return 0
