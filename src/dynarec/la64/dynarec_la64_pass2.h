#define INIT dyn->native_size = 0
#define FINI                                                                                                                                                   \
    if (ninst) {                                                                                                                                               \
        dyn->insts[ninst].address = (dyn->insts[ninst - 1].address + dyn->insts[ninst - 1].size);                                                              \
        dyn->insts_size += 1 + ((dyn->insts[ninst].x64.size > (dyn->insts[ninst].size / 4)) ? dyn->insts[ninst].x64.size : (dyn->insts[ninst].size / 4)) / 15; \
    }

#define MESSAGE(A, ...) do {} while (0)
#define EMIT(A)                      \
    do {                             \
        dyn->insts[ninst].size += 4; \
        dyn->native_size += 4;       \
    } while (0)
#define NEW_INST                                                                                                                                                               \
    if (ninst) {                                                                                                                                                               \
        dyn->insts[ninst].address = (dyn->insts[ninst - 1].address + dyn->insts[ninst - 1].size);                                                                              \
        dyn->insts_size += 1 + ((dyn->insts[ninst - 1].x64.size > (dyn->insts[ninst - 1].size / 4)) ? dyn->insts[ninst - 1].x64.size : (dyn->insts[ninst - 1].size / 4)) / 15; \
        dyn->insts[ninst].ymm0_pass2 = dyn->ymm_zero;                                                                                                                          \
    }
#define INST_EPILOG dyn->insts[ninst].epilog = dyn->native_size;
#define INST_NAME(name)
#define TABLE64(A, V)         \
    {                         \
        Table64(dyn, (V), 2); \
        EMIT(0);              \
        EMIT(0);              \
    }
