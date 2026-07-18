#define INIT uintptr_t sav_addr = addr
#define FINI                           \
    dyn->isize = addr - sav_addr;      \
    dyn->insts[ninst].x64.addr = addr; \
    if (ninst) dyn->insts[ninst - 1].x64.size = dyn->insts[ninst].x64.addr - dyn->insts[ninst - 1].x64.addr

#define MESSAGE(A, ...) \
    do {                \
    } while (0)
#define READFLAGS(A)                                                                        \
    do {                                                                                    \
        dyn->insts[ninst].x64.use_flags = A;                                                \
        if (dyn->f != status_none_pending) dyn->f = status_none;                            \
        if (!BOX64ENV(dynarec_df) && (A) & X_PEND) dyn->insts[ninst].x64.use_flags = X_ALL; \
        dyn->f = status_none;                                                               \
    } while (0)

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
            else if (dyn->insts[prev].nat_flags_sf && dyn->insts[prev].nat_flags_sign && (A) == X_SF)          \
                dyn->insts[ninst].nat_flags_fusion = 1;                                                        \
        }                                                                                                      \
    }                                                                                                          \
    READFLAGS(A);

/* Do not update dyn->f for SF_SET_PENDING: it will use either DF_NONE or DF_SET, similar to SF_SET_NODF. */
#define SETFLAGS(A, B, FUSION)                                                                          \
    do {                                                                                                \
        dyn->insts[ninst].x64.set_flags = A;                                                            \
        dyn->insts[ninst].x64.state_flags = (B) & ~SF_DF;                                               \
        if (((B) & SF_SET_PENDING) != SF_SET_PENDING) {                                                 \
            dyn->f = ((B) & SF_SET) ? (((B) == SF_SET_NODF) ? dyn->f : status_none_pending)             \
                                    : ((dyn->f == status_none) ? status_none : status_none_pending);    \
        }                                                                                               \
        if (!BOX64ENV(dynarec_df)) {                                                                    \
            dyn->f = status_none;                                                                       \
            if ((A) == SF_PENDING) {                                                                    \
                printf_log(LOG_INFO, "Warning, some opcode use SF_PENDING, forcing deferedflags ON\n"); \
                SET_BOX64ENV(dynarec_df, 1);                                                            \
            }                                                                                           \
        }                                                                                               \
        dyn->insts[ninst].nat_flags_nofusion = (FUSION);                                                \
    } while (0)

#define EMIT(A) dyn->native_size += 4
#define JUMP(A, C)                                  \
    do {                                            \
        dyn->insts[ninst].x64.jmp = (uintptr_t)(A); \
        add_jump(dyn, ninst);                       \
        add_next(dyn, dyn->insts[ninst].x64.jmp);   \
        SMEND();                                    \
        dyn->insts[ninst].x64.jmp_cond = C;         \
        dyn->insts[ninst].x64.jmp_insts = 0;        \
    } while (0)
#define BARRIER(A)                                 \
    if (A != BARRIER_MAYBE) {                      \
        UP32_READALL();                            \
        fpu_purgecache(dyn, ninst, 0, x1, x2, x3); \
        dyn->insts[ninst].x64.barrier = A;         \
    } else                                         \
        dyn->insts[ninst].barrier_maybe = 1
#define SET_HASCALLRET() dyn->insts[ninst].x64.has_callret = 1
#define NEW_INST                                                                                                 \
    ++dyn->size;                                                                                                 \
    dyn->insts[ninst].x64.addr = ip;                                                                             \
    dyn->lsx.combined1 = dyn->lsx.combined2 = 0;                                                                 \
    dyn->lsx.swapped = 0;                                                                                        \
    dyn->lsx.barrier = 0;                                                                                        \
    dyn->insts[ninst].up32_read = 0;                                                                             \
    dyn->insts[ninst].up32_write64 = 0;                                                                          \
    dyn->insts[ninst].up32_write32 = 0;                                                                          \
    dyn->insts[ninst].up32_skip = 0;                                                                             \
    dyn->insts[ninst].up32_pending = 0;                                                                          \
    dyn->rsp_used = 0;                                                                                           \
    dyn->insts[ninst].f_entry = dyn->f;                                                                          \
    if (ninst) { dyn->insts[ninst - 1].x64.size = dyn->insts[ninst].x64.addr - dyn->insts[ninst - 1].x64.addr; } \
    AREFLAGSNEEDED()

#define INST_EPILOG                                        \
    do {                                                   \
        dyn->insts[ninst].f_exit = dyn->f;                 \
        dyn->insts[ninst].lsx = dyn->lsx;                  \
        dyn->insts[ninst].x64.has_next = (ok > 0) ? 1 : 0; \
        avx_cleancache(dyn, ninst);                        \
    } while (0)

#define INST_NAME(name)
#define DEFAULT                                                                                  \
    --dyn->size;                                                                                 \
    *ok = -1;                                                                                    \
    if (ninst) { dyn->insts[ninst - 1].x64.size = ip - dyn->insts[ninst - 1].x64.addr; }         \
    if (BOX64ENV(dynarec_log) >= LOG_INFO || dyn->need_dump || BOX64ENV(dynarec_missing) == 1) { \
        dynarec_stopped(dyn->insts[ninst].x64.addr, rex.is32bits);                               \
    }


#define NATIVE_RESTORE_X87PC()
#define X87_CHECK_PRECISION(A)                                      \
    do {                                                            \
        if (dyn->need_x87check) dyn->insts[ninst].x87precision = 1; \
    } while (0)

#define SCRATCH_USAGE(usage)                         \
    do {                                             \
        dyn->insts[ninst].no_scratch_usage = !usage; \
    } while (0)

// mark opcode as "unaligned" possible only if the current address is marked as already unaligned
#define IF_UNALIGNED(A) if((dyn->insts[ninst].unaligned = is_addr_unaligned(A)))
#define IF_ALIGNED(A)   if(!(dyn->insts[ninst].unaligned = is_addr_unaligned(A)))
