#define INIT uintptr_t sav_addr = addr
#define FINI                           \
    dyn->isize = addr - sav_addr;      \
    dyn->insts[ninst].x64.addr = addr; \
    if (ninst) dyn->insts[ninst - 1].x64.size = dyn->insts[ninst].x64.addr - dyn->insts[ninst - 1].x64.addr

#define MESSAGE(A, ...) do {} while (0)
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
#define JUMP(A, C)         add_jump(dyn, ninst); add_next(dyn, (uintptr_t)A); SMEND(); dyn->insts[ninst].x64.jmp = A; dyn->insts[ninst].x64.jmp_cond = C; dyn->insts[ninst].x64.jmp_insts = 0
#define BARRIER(A)                                 \
    if (A != BARRIER_MAYBE) {                      \
        fpu_purgecache(dyn, ninst, 0, x1, x2, x3); \
        dyn->insts[ninst].x64.barrier = A;         \
    } else                                         \
        dyn->insts[ninst].barrier_maybe = 1
#define SET_HASCALLRET() dyn->insts[ninst].x64.has_callret = 1
#define NEW_INST                                 \
    ++dyn->size;                                 \
    dyn->insts[ninst].x64.addr = ip;             \
    dyn->lsx.combined1 = dyn->lsx.combined2 = 0; \
    dyn->lsx.swapped = 0;                        \
    dyn->lsx.barrier = 0;                        \
    dyn->insts[ninst].f_entry = dyn->f;          \
    if (ninst) { dyn->insts[ninst - 1].x64.size = dyn->insts[ninst].x64.addr - dyn->insts[ninst - 1].x64.addr; }
#define INST_EPILOG                    \
    dyn->insts[ninst].f_exit = dyn->f; \
    dyn->insts[ninst].lsx = dyn->lsx;  \
    dyn->insts[ninst].x64.has_next = (ok > 0) ? 1 : 0;
#define INST_NAME(name)
#define DEFAULT                                                                                                           \
    --dyn->size;                                                                                                          \
    *ok = -1;                                                                                                             \
    if (ninst) { dyn->insts[ninst - 1].x64.size = ip - dyn->insts[ninst - 1].x64.addr; }                                  \
    if (BOX64ENV(dynarec_log) >= LOG_INFO || dyn->need_dump || BOX64ENV(dynarec_missing) == 1) {                \
        dynarec_log(LOG_NONE, "%p: Dynarec stopped because of %s Opcode ", (void*)ip, rex.is32bits ? "x86" : "x64");      \
        zydis_dec_t* dec = rex.is32bits ? my_context->dec32 : my_context->dec;                                            \
        if (dec) {                                                                                                        \
            dynarec_log_prefix(0, LOG_NONE, "%s", DecodeX64Trace(dec, dyn->insts[ninst].x64.addr, 1));                    \
        } else {                                                                                                          \
            dynarec_log_prefix(0, LOG_NONE, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", \
                PKip(0), PKip(1), PKip(2), PKip(3), PKip(4), PKip(5), PKip(6), PKip(7), PKip(8), PKip(9),                 \
                PKip(10), PKip(11), PKip(12), PKip(13), PKip(14));                                                        \
        }                                                                                                                 \
        PrintFunctionAddr(ip, " => ");                                                                                    \
        dynarec_log_prefix(0, LOG_NONE, "\n");                                                                            \
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
