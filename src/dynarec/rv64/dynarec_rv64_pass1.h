#define INIT
#define FINI
#define MESSAGE(A, ...) \
    do {                \
    } while (0)
#define EMIT(A) \
    do {        \
    } while (0)
#define NEW_INST                                          \
    dyn->insts[ninst].f_entry = dyn->f;                   \
    dyn->e.combined1 = dyn->e.combined2 = 0;              \
    for (int i = 0; i < 16; ++i)                          \
        dyn->e.olds[i].v = 0;                             \
    dyn->insts[ninst].vector_sew_entry = dyn->vector_sew; \
    dyn->inst_sew = dyn->vector_sew;                      \
    dyn->inst_vlmul = VECTOR_LMUL1;                       \
    dyn->inst_vl = 0;                                     \
    dyn->e.swapped = 0;                                   \
    dyn->e.barrier = 0;                                   \
    AREFLAGSNEEDED()

#define INST_EPILOG                    \
    dyn->insts[ninst].e = dyn->e;      \
    dyn->insts[ninst].f_exit = dyn->f; \
    dyn->insts[ninst].vector_sew_exit = dyn->vector_sew;

#define INST_NAME(name)


#define NATIVE_RESTORE_X87PC()
#define X87_CHECK_PRECISION(A)                  \
    do {                                        \
        if (dyn->need_x87check) {               \
            dyn->insts[ninst].x87precision = 1; \
            if (!ST_IS_F(0))                    \
                dyn->need_x87check = 2;         \
        }                                       \
    } while (0)
