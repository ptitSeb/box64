#define INIT
#define FINI                                                                                              \
    if (ninst)                                                                                            \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst].x64.size, dyn->insts[ninst].size / 4); \
    addInst(dyn->instsize, &dyn->insts_size, 0, 0);
#define EMIT(A)                                                          \
    do {                                                                 \
        if (box64_dynarec_dump) print_opcode(dyn, ninst, (uint32_t)(A)); \
        if ((uintptr_t)dyn->block < dyn->tablestart)                     \
            *(uint32_t*)(dyn->block) = (uint32_t)(A);                    \
        dyn->block += 4;                                                 \
        dyn->native_size += 4;                                           \
        dyn->insts[ninst].size2 += 4;                                    \
    } while (0)

#define MESSAGE(A, ...) \
    if (box64_dynarec_dump) dynarec_log(LOG_NONE, __VA_ARGS__)
#define NEW_INST \
    if (ninst)   \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst - 1].x64.size, dyn->insts[ninst - 1].size / 4);
#define INST_EPILOG
#define INST_NAME(name) inst_name_pass3(dyn, ninst, name, rex)
#define TABLE64(A, V)                                 \
    {                                                 \
        int val64offset = Table64(dyn, (V), 3);       \
        MESSAGE(LOG_DUMP, "  Table64: 0x%lx\n", (V)); \
        PCADDU12I(A, SPLIT20(val64offset));           \
        LD_D(A, A, SPLIT12(val64offset));             \
    }
