#define INIT
#define FINI
#define MESSAGE(A, ...) \
    do {                \
    } while (0)
#define EMIT(A) \
    do {        \
    } while (0)
#define NEW_INST                                                                                           \
    dyn->insts[ninst].f_entry = dyn->f;                                                                    \
    dyn->lsx.combined1 = dyn->lsx.combined2 = 0;                                                           \
    dyn->lsx.swapped = 0;                                                                                  \
    dyn->lsx.barrier = 0;                                                                                  \
    AREFLAGSNEEDED();                                                                                      \
    if (dyn->insts[ninst].x64.jmp || (dyn->insts[ninst].x64.barrier & BARRIER_FLOAT)                       \
        || dyn->insts[ninst].x64.has_callret || dyn->insts[ninst].fpupurge || dyn->insts[ninst].host_call) \
    sse_merge_all(dyn, ninst)

#define INST_EPILOG                                                                            \
    do {                                                                                       \
        if (dyn->insts[ninst].x64.has_next && ninst + 1 < dyn->size                            \
            && (dyn->insts[ninst + 1].pred_sz != 1 || dyn->insts[ninst + 1].pred[0] != ninst)) \
            sse_merge_all(dyn, ninst);                                                         \
        dyn->insts[ninst].lsx = dyn->lsx;                                                      \
        dyn->insts[ninst].f_exit = dyn->f;                                                     \
        avx_cleancache(dyn, ninst);                                                            \
    } while (0)

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
