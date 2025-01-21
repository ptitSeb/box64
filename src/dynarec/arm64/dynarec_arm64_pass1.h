#define INIT    
#define FINI
#define MESSAGE(A, ...) do {} while (0)
#define EMIT(A) do {} while (0)
#define MAYSETFLAGS()   dyn->insts[ninst].x64.may_set = 1
#define SET_NODF()      \
        if(!dyn->insts[ninst].x64.gen_flags && !dyn->insts[ninst].x64.use_flags)   \
                propagate_nodf(dyn, ninst);     \
        dyn->f.dfnone = 0
#define SETFLAGS(A,B)                                                   \
        dyn->insts[ninst].x64.set_flags = A;                            \
        dyn->insts[ninst].x64.state_flags = (B)&~SF_DF;                 \
        dyn->f.pending=(B)&SF_SET_PENDING;                              \
        dyn->f.dfnone=((B)&SF_SET)?(((B)==SF_SET_NODF)?0:1):0;          \
        if(!BOX64ENV(dynarec_df)) {dyn->f.dfnone=1; dyn->f.pending=0; }
#define NEW_INST                                \
        dyn->insts[ninst].f_entry = dyn->f;     \
        dyn->n.combined1 = dyn->n.combined2 = 0;\
        dyn->n.swapped = 0; dyn->n.barrier = 0

#define INST_EPILOG                             \
        dyn->insts[ninst].n = dyn->n;           \
        dyn->insts[ninst].f_exit = dyn->f

#define INST_NAME(name)  
