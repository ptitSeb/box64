#define INIT    
#define FINI
#define MESSAGE(A, ...)  
#define EMIT(A)     
#define NEW_INST                                \
        dyn->insts[ninst].f_entry = dyn->f;     \

#define INST_EPILOG                             \
        dyn->insts[ninst].f_exit = dyn->f

#define INST_NAME(name)  
