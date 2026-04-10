#define INIT dyn->native_size = 0
#define FINI                                                                                                                                                   \
    if (ninst) {                                                                                                                                               \
        dyn->insts[ninst].address = (dyn->insts[ninst - 1].address + dyn->insts[ninst - 1].size);                                                              \
        dyn->insts_size += 1+((dyn->insts[ninst-1].x64.size>(dyn->insts[ninst-1].size/4))?dyn->insts[ninst-1].x64.size:(dyn->insts[ninst-1].size/4))/15;       \
    }

#define MESSAGE(A, ...) \
    do {                \
    } while (0)
#define EMIT(A)                      \
    do {                             \
        dyn->insts[ninst].size += 4; \
        dyn->native_size += 4;       \
    } while (0)
#define NEW_INST                                                                                                                                                               \
    dyn->vector_sew = dyn->insts[ninst].vector_sew_entry;                                                                                                                      \
    dyn->inst_sew = dyn->vector_sew;                                                                                                                                           \
    dyn->inst_vlmul = VECTOR_LMUL1;                                                                                                                                            \
    dyn->inst_vl = 0;                                                                                                                                                          \
    dyn->inst_start_size = dyn->native_size;                                                                                                                          \
    if (ninst) {                                                                                                                                                               \
        dyn->insts[ninst].address = (dyn->insts[ninst - 1].address + dyn->insts[ninst - 1].size);                                                                              \
        dyn->insts_size += 1 + ((dyn->insts[ninst - 1].x64.size > (dyn->insts[ninst - 1].size / 4)) ? dyn->insts[ninst - 1].x64.size : (dyn->insts[ninst - 1].size / 4)) / 15; \
        dyn->insts[ninst].ymm0_pass2 = dyn->ymm_zero;                                                                                                                          \
    }                                                                                                                                                                          \
    AREFLAGSNEEDED()
#define INST_EPILOG                                      \
    dyn->insts[ninst].epilog = dyn->native_size;         \
    dyn->vector_sew = dyn->insts[ninst].vector_sew_exit; \
    dyn->inst_sew = dyn->vector_sew;                     \
    dyn->inst_vlmul = VECTOR_LMUL1;                      \
    dyn->inst_vl = 0;                                    \
    do {                                                 \
        if (dyn->stats && dyn->insts[ninst].x64.alive) { \
            size_t nb = dyn->native_size - dyn->inst_start_size; \
            collect_instruction_stat(dyn->stats,         \
                dyn->insts[ninst].x64.addr,              \
                dyn->insts[ninst].x64.size,              \
                nb / 4, dyn->inst_name);                 \
        }                                                \
    } while(0)
#define INST_NAME(name)                                  \
    do {                                                 \
        if (dyn->stats)                                  \
            snprintf(dyn->inst_name, sizeof(dyn->inst_name), "%s", name); \
    } while(0)
#define TABLE64(A, V)                                \
    do {                                             \
        if (dyn->need_reloc && !isTable64(dyn, (V))) \
            AddRelocTable64Addr(dyn, ninst, (V), 2); \
        Table64(dyn, (V), 2);                        \
        EMIT(0);                                     \
        EMIT(0);                                     \
    } while (0)
#define TABLE64_(A, V)        \
    do {                      \
        Table64(dyn, (V), 2); \
        EMIT(0);              \
        EMIT(0);              \
    } while (0)
#define FTABLE64(A, V)               \
    do {                             \
        mmx87_regs_t v = { .d = V }; \
        Table64(dyn, v.q, 2);        \
        EMIT(0);                     \
        EMIT(0);                     \
    } while (0)
#define TABLE64C(A, V)                                       \
    do {                                                     \
        if (dyn->need_reloc && !isTable64(dyn, getConst(V))) \
            AddRelocTable64Const(dyn, ninst, (V), 2);        \
        Table64(dyn, getConst(V), 2);                        \
        EMIT(0);                                             \
        EMIT(0);                                             \
    } while (0)
#define CALLRET_RET(A)   do {                                                                                                                   \
                if((A) && ISSEP() && BOX64DRENV(dynarec_callret)) {                                                                             \
                        dyn->insts[ninst].size+=sizeof(void*); dyn->native_size+=sizeof(void*); dyn->insts[ninst+1].sep=1; ++dyn->sep_size;     \
                }                                                                                                                               \
                if((A) && (BOX64DRENV(dynarec_callret)>1) && !dyn->always_test) {                                                               \
                        dyn->callrets[dyn->callret_size].type = 0; dyn->callrets[dyn->callret_size++].offs = dyn->native_size; EMIT(ARCH_NOP);  \
                }                                                                                                                               \
        } while(0)
#define CALLRET_LOOP()   do { dyn->callrets[dyn->callret_size].type = 1; dyn->callrets[dyn->callret_size++].offs = dyn->native_size; EMIT(ARCH_NOP); } while(0)
