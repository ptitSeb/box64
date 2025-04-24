
#define INIT    uintptr_t sav_addr=addr
#define FINI    \
    dyn->isize = addr-sav_addr;         \
    dyn->insts[ninst].x64.addr = addr;  \
    if(ninst) dyn->insts[ninst-1].x64.size = dyn->insts[ninst].x64.addr - dyn->insts[ninst-1].x64.addr

#define MESSAGE(A, ...) do {} while (0)
#define READFLAGS(A)    \
        dyn->insts[ninst].x64.use_flags = A; dyn->f.dfnone = 1;\
        if(!BOX64ENV(dynarec_df) && (A)&X_PEND) dyn->insts[ninst].x64.use_flags = X_ALL; \
        dyn->f.pending=SF_SET
#define SETFLAGS(A,B)   \
        dyn->insts[ninst].x64.set_flags = A;    \
        dyn->insts[ninst].x64.state_flags = (B)&~SF_DF;  \
        dyn->f.pending=(B)&SF_SET_PENDING;      \
        dyn->f.dfnone=((B)&SF_SET)?(((B)==SF_SET_NODF)?0:1):0;  \
        if(!BOX64ENV(dynarec_df)) {dyn->f.dfnone=1; dyn->f.pending=0; if((A)==SF_PENDING){printf_log(LOG_INFO, "Warning, some opcode use SF_PENDING, forcing deferedflags ON\n"); SET_BOX64ENV(dynarec_df, 1); }}
#define EMIT(A)         dyn->native_size+=4
#define JUMP(A, C)         add_jump(dyn, ninst); add_next(dyn, (uintptr_t)A); SMEND(); dyn->insts[ninst].x64.jmp = A; dyn->insts[ninst].x64.jmp_cond = C; dyn->insts[ninst].x64.jmp_insts = 0
#define BARRIER(A)      if(A!=BARRIER_MAYBE) {fpu_purgecache(dyn, ninst, 0, x1, x2, x3); dyn->insts[ninst].x64.barrier = A;} else dyn->insts[ninst].barrier_maybe = 1
#define SET_HASCALLRET()    dyn->insts[ninst].x64.has_callret = 1
#define NEW_INST \
        ++dyn->size;                            \
        dyn->insts[ninst].x64.addr = ip;        \
        dyn->n.combined1 = dyn->n.combined2 = 0;\
        dyn->n.swapped = 0; dyn->n.barrier = 0; \
        dyn->insts[ninst].f_entry = dyn->f;     \
        if(ninst) {dyn->insts[ninst-1].x64.size = dyn->insts[ninst].x64.addr - dyn->insts[ninst-1].x64.addr;}

#define INST_EPILOG                             \
        dyn->insts[ninst].f_exit = dyn->f;      \
        dyn->insts[ninst].n = dyn->n;           \
        if(dyn->insts[ninst].nat_flags_op==NAT_FLAG_OP_TOUCH && !dyn->insts[ninst].set_nat_flags)       \
                dyn->insts[ninst].nat_flags_op=NAT_FLAG_OP_UNUSABLE;                                    \
        dyn->insts[ninst].x64.has_next = (ok>0)?1:0;
#define INST_NAME(name)
#define DEFAULT                                                                                                               \
    --dyn->size;                                                                                                              \
    *ok = -1;                                                                                                                 \
    if (ninst) { dyn->insts[ninst - 1].x64.size = ip - dyn->insts[ninst - 1].x64.addr; }                                      \
    if (BOX64ENV(dynarec_log) >= LOG_INFO || dyn->need_dump || BOX64ENV(dynarec_missing) == 1)                        \
        if (!dyn->size || BOX64ENV(dynarec_log) > LOG_INFO || dyn->need_dump) {                                       \
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


#define FEMIT(A)        dyn->insts[ninst].nat_flags_op = dyn->insts[ninst].x64.set_flags?NAT_FLAG_OP_TOUCH:NAT_FLAG_OP_UNUSABLE
#define IFNATIVE(A)     if(mark_natflag(dyn, ninst, A, 0))
#define IFNATIVEN(A)    if(mark_natflag(dyn, ninst, A, 0))
#define IFX(A)  if((dyn->insts[ninst].x64.set_flags&(A)))
#define IFX2(A, B)  if((dyn->insts[ninst].x64.set_flags&(A)) B)
#define IFX_PENDOR0  if((dyn->insts[ninst].x64.set_flags&(X_PEND) || !dyn->insts[ninst].x64.set_flags))
#define IFXX(A) if((dyn->insts[ninst].x64.set_flags==(A)))
#define IFX2X(A, B) if((dyn->insts[ninst].x64.set_flags==(A) || dyn->insts[ninst].x64.set_flags==(B) || dyn->insts[ninst].x64.set_flags==((A)|(B))))
#define IFXN(A, B)  if((dyn->insts[ninst].x64.set_flags&(A) && !(dyn->insts[ninst].x64.set_flags&(B))))
#define IFXNATIVE(X, N)  if((dyn->insts[ninst].x64.set_flags&(X)) && mark_natflag(dyn, ninst, N, 0))
#define GEN_INVERTED_CARRY()        dyn->insts[ninst].gen_inverted_carry = 1
#define IFNATIVE_BEFORE(A)     if(mark_natflag(dyn, ninst, A, 1))
#define INVERT_CARRY(A) dyn->insts[ninst].invert_carry = 1
#define INVERT_CARRY_BEFORE(A) dyn->insts[ninst].invert_carry_before = 1
// mark opcode as "unaligned" possible only if the current address is not marked as already unaligned
#define IF_UNALIGNED(A) if((dyn->insts[ninst].unaligned=is_addr_unaligned(A)))
#define IF_ALIGNED(A)   if(!(dyn->insts[ninst].unaligned=is_addr_unaligned(A)))

#define NATIVE_RESTORE_X87PC()
#define X87_CHECK_PRECISION(A)                                      \
    do {                                                            \
        if (dyn->need_x87check) dyn->insts[ninst].x87precision = 1; \
    } while (0)
