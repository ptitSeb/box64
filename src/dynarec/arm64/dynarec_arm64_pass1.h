#define INIT    
#define FINI
#define MESSAGE(A, ...) do {} while (0)
#define EMIT(A) do {} while (0)

#define SETFLAGS(A,B)                                                   \
        dyn->insts[ninst].x64.set_flags = A;                            \
        dyn->insts[ninst].x64.state_flags = (B)&~SF_DF;                 \
        dyn->f=((B)&SF_SET)?(((B)==SF_SET_NODF)?dyn->f:status_none_pending):(((B)&SF_SET_PENDING)?status_set:status_none_pending);  \
        if(!BOX64ENV(dynarec_df)) {dyn->f = status_none; }
#define NEW_INST                                \
        dyn->insts[ninst].f_entry = dyn->f;     \
        dyn->n.combined1 = dyn->n.combined2 = 0;\
        dyn->n.swapped = 0; dyn->n.barrier = 0

#define INST_EPILOG                             \
        dyn->insts[ninst].n = dyn->n;           \
        dyn->insts[ninst].f_exit = dyn->f

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
