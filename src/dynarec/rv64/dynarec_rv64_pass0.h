
#define INIT uintptr_t sav_addr = addr
#define FINI                           \
    dyn->isize = addr - sav_addr;      \
    dyn->insts[ninst].x64.addr = addr; \
    if (ninst) dyn->insts[ninst - 1].x64.size = dyn->insts[ninst].x64.addr - dyn->insts[ninst - 1].x64.addr

#define MESSAGE(A, ...) \
    do {                \
    } while (0)
#define MAYSETFLAGS() dyn->insts[ninst].x64.may_set = 1
#define READFLAGS(A)                     \
    dyn->insts[ninst].x64.use_flags = A; \
    dyn->f.dfnone = 1;                   \
    dyn->f.pending = SF_SET

#define READFLAGS_FUSION(A, s1, s2, s3, s4, s5)                                                                \
    if (BOX64ENV(dynarec_nativeflags) && ninst > 0) {                                                          \
        int prev = ninst - 1;                                                                                  \
        while (prev && dyn->insts[prev].no_scratch_usage)                                                      \
            prev -= 1;                                                                                         \
        if (!dyn->insts[prev].nat_flags_nofusion) {                                                            \
            if ((A) == (X_ZF))                                                                                 \
                dyn->insts[ninst].nat_flags_fusion = 1;                                                        \
            else if (dyn->insts[prev].nat_flags_carry && ((A) == (X_CF) || (A) == (X_CF | X_ZF)))              \
                dyn->insts[ninst].nat_flags_fusion = 1;                                                        \
            else if (dyn->insts[prev].nat_flags_sign && ((A) == (X_SF | X_OF) || (A) == (X_SF | X_OF | X_ZF))) \
                dyn->insts[ninst].nat_flags_fusion = 1;                                                        \
        }                                                                                                      \
    }                                                                                                          \
    READFLAGS(A);

#define SETFLAGS(A, B, FUSION)                                           \
    dyn->insts[ninst].x64.set_flags = A;                                 \
    dyn->insts[ninst].x64.state_flags = (B) & ~SF_DF;                    \
    dyn->f.pending = (B) & SF_SET_PENDING;                               \
    dyn->f.dfnone = ((B) & SF_SET) ? (((B) == SF_SET_NODF) ? 0 : 1) : 0; \
    dyn->insts[ninst].nat_flags_nofusion = (FUSION)

#define EMIT(A) dyn->native_size += 4
#define JUMP(A, C)                      \
    add_jump(dyn, ninst);               \
    add_next(dyn, (uintptr_t)A);        \
    SMEND();                            \
    dyn->insts[ninst].x64.jmp = A;      \
    dyn->insts[ninst].x64.jmp_cond = C; \
    dyn->insts[ninst].x64.jmp_insts = 0
#define BARRIER(A)                                 \
    if (A != BARRIER_MAYBE) {                      \
        fpu_purgecache(dyn, ninst, 0, x1, x2, x3); \
        dyn->insts[ninst].x64.barrier = A;         \
    } else                                         \
        dyn->insts[ninst].barrier_maybe = 1
#define SET_HASCALLRET() dyn->insts[ninst].x64.has_callret = 1
#define NEW_INST                                                 \
    ++dyn->size;                                                 \
    memset(&dyn->insts[ninst], 0, sizeof(instruction_native_t)); \
    dyn->insts[ninst].x64.addr = ip;                             \
    dyn->e.combined1 = dyn->e.combined2 = 0;                     \
    dyn->e.swapped = 0;                                          \
    dyn->e.barrier = 0;                                          \
    for (int i = 0; i < 16; ++i)                                 \
        dyn->e.olds[i].v = 0;                                    \
    dyn->insts[ninst].f_entry = dyn->f;                          \
    dyn->insts[ninst].vector_sew_entry = dyn->vector_sew;        \
    dyn->inst_sew = dyn->vector_sew;                             \
    dyn->inst_vlmul = VECTOR_LMUL1;                              \
    dyn->inst_vl = 0;                                            \
    if (ninst)                                                   \
        dyn->insts[ninst - 1].x64.size = dyn->insts[ninst].x64.addr - dyn->insts[ninst - 1].x64.addr;

#define INST_EPILOG                                      \
    dyn->insts[ninst].f_exit = dyn->f;                   \
    dyn->insts[ninst].e = dyn->e;                        \
    dyn->insts[ninst].vector_sew_exit = dyn->vector_sew; \
    dyn->insts[ninst].x64.has_next = (ok > 0) ? 1 : 0;
#define INST_NAME(name)
#define DEFAULT                                                                                                               \
    --dyn->size;                                                                                                              \
    *ok = -1;                                                                                                                 \
    if (ninst) { dyn->insts[ninst - 1].x64.size = ip - dyn->insts[ninst - 1].x64.addr; }                                      \
    if (BOX64ENV(dynarec_log) >= LOG_INFO || dyn->need_dump || BOX64ENV(dynarec_missing))                                     \
        if (!dyn->size || BOX64ENV(dynarec_log) > LOG_INFO || dyn->need_dump) {                                               \
            dynarec_stopped(dyn->insts[ninst].x64.addr, rex.is32bits);                                                        \
        }

#define SET_ELEMENT_WIDTH(s1, sew, set)                    \
    do {                                                   \
        if ((sew) != VECTOR_SEWANY && (set))               \
            dyn->vector_sew = (sew);                       \
        else if (dyn->vector_sew == VECTOR_SEWNA && (set)) \
            dyn->vector_sew = VECTOR_SEW8;                 \
    } while (0)

// mark opcode as "unaligned" possible only if the current address is not marked as already unaligned
#define IF_UNALIGNED(A) if ((dyn->insts[ninst].unaligned = (is_addr_unaligned(A) ? 0 : 1)))
#define IF_ALIGNED(A)   if ((dyn->insts[ninst].unaligned = (is_addr_unaligned(A) ? 1 : 0)))

#define NATIVE_RESTORE_X87PC()
#define X87_CHECK_PRECISION(A)                                      \
    do {                                                            \
        if (dyn->need_x87check) dyn->insts[ninst].x87precision = 1; \
    } while (0)

#define SCRATCH_USAGE(usage)                         \
    do {                                             \
        dyn->insts[ninst].no_scratch_usage = !usage; \
    } while (0)
