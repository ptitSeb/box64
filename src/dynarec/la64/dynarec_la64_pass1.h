#define INIT
#define FINI
#define MESSAGE(A, ...)
#define EMIT(A)
#define NEW_INST                                 \
    dyn->insts[ninst].f_entry = dyn->f;          \
    dyn->lsx.combined1 = dyn->lsx.combined2 = 0; \
    dyn->lsx.swapped = 0;                        \
    dyn->lsx.barrier = 0

#define INST_EPILOG                   \
    dyn->insts[ninst].lsx = dyn->lsx; \
    dyn->insts[ninst].f_exit = dyn->f

#define INST_NAME(name)
