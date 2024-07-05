#define INIT    
#define FINI
#define MESSAGE(A, ...)  
#define EMIT(A)     
#define NEW_INST                                \
        dyn->insts[ninst].f_entry = dyn->f;     \
        dyn->n.combined1 = dyn->n.combined2 = 0;\
        dyn->insts[ninst].ymm0_in = dyn->ymm_zero;\
        dyn->n.swapped = 0; dyn->n.barrier = 0

#define INST_EPILOG                             \
        dyn->insts[ninst].n = dyn->n;           \
        dyn->insts[ninst].ymm0_out = dyn->ymm_zero;\
        dyn->insts[ninst].f_exit = dyn->f

#define INST_NAME(name)  
