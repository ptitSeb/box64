#define INIT
#define FINI                                                                                                \
    if (ninst)                                                                                              \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst-1].x64.size, dyn->insts[ninst-1].size/4); \
    addInst(dyn->instsize, &dyn->insts_size, 0, 0);
#define EMIT(A)                                                                \
    do {                                                                       \
        if (dyn->need_dump) print_opcode(dyn, ninst, (uint32_t)(A)); \
        if ((uintptr_t)dyn->block < dyn->tablestart)                           \
            *(uint32_t*)(dyn->block) = (uint32_t)(A);                          \
        dyn->block += 4;                                                       \
        dyn->native_size += 4;                                                 \
        dyn->insts[ninst].size2 += 4;                                          \
    } while (0)

#define MESSAGE(A, ...)                                                   \
    do {                                                                  \
        if (dyn->need_dump) dynarec_log(LOG_NONE, __VA_ARGS__); \
    } while (0)
#define NEW_INST \
    if (ninst) {  \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst - 1].x64.size, dyn->insts[ninst - 1].size / 4); \
        dyn->insts[ninst].ymm0_pass3 = dyn->ymm_zero; \
    }
#define INST_EPILOG
#define INST_NAME(name) inst_name_pass3(dyn, ninst, name, rex)
#define TABLE64(A, V)                                 \
    do {                                              \
        if (dyn->need_reloc && !isTable64(dyn, (V)))  \
            AddRelocTable64Addr(dyn, ninst, (V), 3);  \
        int val64offset = Table64(dyn, (V), 3);       \
        MESSAGE(LOG_DUMP, "  Table64: 0x%lx\n", (V)); \
        PCADDU12I(A, SPLIT20(val64offset));           \
        LD_D(A, A, SPLIT12(val64offset));             \
    } while (0)
#define TABLE64_(A, V)                                \
    do {                                              \
        int val64offset = Table64(dyn, (V), 3);       \
        MESSAGE(LOG_DUMP, "  Table64: 0x%lx\n", (V)); \
        PCADDU12I(A, SPLIT20(val64offset));           \
        LD_D(A, A, SPLIT12(val64offset));             \
    } while (0)
#define FTABLE64(A, V)                              \
    do {                                            \
        mmx87_regs_t v = { .d = V };                \
        int val64offset = Table64(dyn, v.q, 3);     \
        MESSAGE(LOG_DUMP, "  FTable64: %g\n", v.d); \
        PCADDU12I(x1, SPLIT20(val64offset));            \
        FLD_D(A, x1, SPLIT12(val64offset));           \
    } while (0)
#define TABLE64C(A, V)                                       \
    do {                                                     \
        if (dyn->need_reloc && !isTable64(dyn, getConst(V))) \
            AddRelocTable64Const(dyn, ninst, (V), 3);        \
        int val64offset = Table64(dyn, getConst(V), 3);      \
        MESSAGE(LOG_DUMP, "  Table64: 0x%lx\n", (V));        \
        PCADDU12I(A, SPLIT20(val64offset));                  \
        LD_D(A, A, SPLIT12(val64offset));                    \
    } while (0)
