#define INIT
#define FINI
#define MESSAGE(A, ...) do {} while (0)
#define EMIT(A) do {} while (0)
#define NEW_INST                                                                   \
    dyn->insts[ninst].f_entry = dyn->f;                                            \
    dyn->e.combined1 = dyn->e.combined2 = 0;                                       \
    for (int i = 0; i < 16; ++i)                                                   \
        dyn->e.olds[i].v = 0;                                                      \
    if (reset_n != -1)                                                             \
        dyn->vector_sew = ninst ? dyn->insts[ninst - 1].vector_sew : VECTOR_SEWNA; \
    dyn->e.swapped = 0;                                                            \
    dyn->e.barrier = 0

#define INST_EPILOG                             \
        dyn->insts[ninst].e = dyn->e;           \
        dyn->insts[ninst].f_exit = dyn->f

#define INST_NAME(name)
