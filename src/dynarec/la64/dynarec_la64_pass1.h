#define INIT
#define FINI
#define MESSAGE(A, ...) do {} while (0)
#define EMIT(A) do {} while (0)
#define NEW_INST                                 \
    dyn->insts[ninst].f_entry = dyn->f;          \
    dyn->lsx.combined1 = dyn->lsx.combined2 = 0; \
    dyn->lsx.swapped = 0;                        \
    dyn->lsx.barrier = 0

#define INST_EPILOG                   \
    dyn->insts[ninst].lsx = dyn->lsx; \
    dyn->insts[ninst].f_exit = dyn->f

#define INST_NAME(name)
