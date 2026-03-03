#define INIT
#define FINI                                                                                                \
    if (ninst)                                                                                              \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst-1].x64.size, dyn->insts[ninst-1].size/4); \
    addInst(dyn->instsize, &dyn->insts_size, 0, 0);
#define EMIT(A)                                                              \
    do {                                                                     \
        if (dyn->need_dump) print_opcode(dyn, ninst, (uint32_t)A); \
        *(uint32_t*)(dyn->block) = (uint32_t)(A);                            \
        dyn->block += 4;                                                     \
        dyn->native_size += 4;                                               \
        dyn->insts[ninst].size2 += 4;                                        \
    } while (0)

#define MESSAGE(A, ...)                                                   \
    do {                                                                  \
        if (dyn->need_dump) dynarec_log(LOG_NONE, __VA_ARGS__); \
    } while (0)
#define NEW_INST                                                                                                  \
    dyn->vector_sew = dyn->insts[ninst].vector_sew_entry;                                                         \
    dyn->inst_sew = dyn->vector_sew;                                                                              \
    dyn->inst_vlmul = VECTOR_LMUL1;                                                                               \
    dyn->inst_vl = 0;                                                                                             \
    if (ninst) {                                                                                                  \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst - 1].x64.size, dyn->insts[ninst - 1].size / 4); \
        dyn->insts[ninst].ymm0_pass3 = dyn->ymm_zero;                                                             \
    }                                                                                                             \
    AREFLAGSNEEDED()
#define INST_EPILOG
#define INST_NAME(name) inst_name_pass3(dyn, ninst, name, rex)

#define TABLE64(A, V)                                 \
    do {                                              \
        if (dyn->need_reloc && !isTable64(dyn, (V)))  \
            AddRelocTable64Addr(dyn, ninst, (V), 3);  \
        int val64offset = Table64(dyn, (V), 3);       \
        MESSAGE(LOG_DUMP, "  Table64: 0x%lx\n", (V)); \
        AUIPC(A, SPLIT20(val64offset));               \
        LD(A, A, SPLIT12(val64offset));               \
    } while (0)
#define TABLE64_(A, V)                                \
    do {                                              \
        int val64offset = Table64(dyn, (V), 3);       \
        MESSAGE(LOG_DUMP, "  Table64: 0x%lx\n", (V)); \
        AUIPC(A, SPLIT20(val64offset));               \
        LD(A, A, SPLIT12(val64offset));               \
    } while (0)
#define FTABLE64(A, V)                              \
    do {                                            \
        mmx87_regs_t v = { .d = V };                \
        int val64offset = Table64(dyn, v.q, 3);     \
        MESSAGE(LOG_DUMP, "  FTable64: %g\n", v.d); \
        AUIPC(x1, SPLIT20(val64offset));            \
        FLD(A, x1, SPLIT12(val64offset));           \
    } while (0)
#define TABLE64C(A, V)                                       \
    do {                                                     \
        if (dyn->need_reloc && !isTable64(dyn, getConst(V))) \
            AddRelocTable64Const(dyn, ninst, (V), 3);        \
        int val64offset = Table64(dyn, getConst(V), 3);      \
        MESSAGE(LOG_DUMP, "  Table64: 0x%lx\n", (V));        \
        AUIPC(A, SPLIT20(val64offset));                      \
        LD(A, A, SPLIT12(val64offset));                      \
    } while (0)

#define DEFAULT_VECTOR                                                                                                                  \
    if (BOX64ENV(dynarec_log) >= LOG_INFO || dyn->need_dump || BOX64ENV(dynarec_missing) == 2) {                                        \
        dynarec_log(LOG_NONE, "%p: Dynarec fallback to scalar version because of %s Opcode ", (void*)ip, rex.is32bits ? "x86" : "x64"); \
        zydis_dec_t* dec = rex.is32bits ? my_context->dec32 : my_context->dec;                                                          \
        if (dec) {                                                                                                                      \
            dynarec_log_prefix(0, LOG_NONE, "%s", DecodeX64Trace(dec, dyn->insts[ninst].x64.addr, 1));                                  \
        } else {                                                                                                                        \
            dynarec_log_prefix(0, LOG_NONE, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",               \
                PKip(0), PKip(1), PKip(2), PKip(3), PKip(4), PKip(5), PKip(6), PKip(7), PKip(8), PKip(9),                               \
                PKip(10), PKip(11), PKip(12), PKip(13), PKip(14));                                                                      \
        }                                                                                                                               \
        PrintFunctionAddr(ip, " => ");                                                                                                  \
        dynarec_log_prefix(0, LOG_NONE, "\n");                                                                                          \
    }                                                                                                                                   \
    return 0
