#define INIT    
#define FINI        \
    if(ninst)       \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst].x64.size, dyn->insts[ninst].size/4); \
    addInst(dyn->instsize, &dyn->insts_size, 0, 0);
#define EMIT(A)     \
    do {                                                \
        if(box64_dynarec_dump) {dynarec_log(LOG_NONE, "\t%08x\t%s\n", (uint32_t)(A), rv64_print(A, (uintptr_t)dyn->block));} \
        *(uint32_t*)(dyn->block) = (uint32_t)(A);       \
        dyn->block += 4; dyn->native_size += 4;         \
        dyn->insts[ninst].size2 += 4;                   \
    }while(0)

#define MESSAGE(A, ...)  if(box64_dynarec_dump) dynarec_log(LOG_NONE, __VA_ARGS__)
#define NEW_INST        \
    if(box64_dynarec_dump) {\
        dynarec_log(LOG_NONE, "%sNew instruction %d, native=%p (0x%x)%s\n", \
            (box64_dynarec_dump>1)?"\e[4;32m":"",   \
            ninst, dyn->block, dyn->native_size,    \
            (box64_dynarec_dump>1)?"\e[m":""        \
            );\
    }\
    if(ninst)                                                   \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst-1].x64.size, dyn->insts[ninst-1].size/4);
#define INST_EPILOG     
#define INST_NAME(name) \
    if(box64_dynarec_dump) {\
        printf_x64_instruction(my_context->dec, &dyn->insts[ninst].x64, name); \
        dynarec_log(LOG_NONE, "%s%p: %d emited opcodes, inst=%d, barrier=%d state=%d/%d(%d), %s=%X/%X, use=%X, need=%X/%X, sm=%d/%d", \
            (box64_dynarec_dump>1)?"\e[32m":"", \
            (void*)(dyn->native_start+dyn->insts[ninst].address),  \
            dyn->insts[ninst].size/4,           \
            ninst,                              \
            dyn->insts[ninst].x64.barrier,      \
            dyn->insts[ninst].x64.state_flags,  \
            dyn->f.pending,                     \
            dyn->f.dfnone,                      \
            dyn->insts[ninst].x64.may_set?"may":"set",              \
            dyn->insts[ninst].x64.set_flags,    \
            dyn->insts[ninst].x64.gen_flags,    \
            dyn->insts[ninst].x64.use_flags,    \
            dyn->insts[ninst].x64.need_before,  \
            dyn->insts[ninst].x64.need_after,   \
            dyn->smread, dyn->smwrite);         \
        if(dyn->insts[ninst].pred_sz) {         \
            dynarec_log(LOG_NONE, ", pred=");   \
            for(int ii=0; ii<dyn->insts[ninst].pred_sz; ++ii)                           \
                dynarec_log(LOG_NONE, "%s%d", ii?"/":"", dyn->insts[ninst].pred[ii]);   \
        }                                                                               \
        if(dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts>=0)             \
            dynarec_log(LOG_NONE, ", jmp=%d", dyn->insts[ninst].x64.jmp_insts);         \
        if(dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts==-1)            \
            dynarec_log(LOG_NONE, ", jmp=out");                                         \
        if(dyn->last_ip)                                                                \
            dynarec_log(LOG_NONE, ", last_ip=%p", (void*)dyn->last_ip);                 \
        dynarec_log(LOG_NONE, "%s\n", (box64_dynarec_dump>1)?"\e[m":"");                \
    }

#define TABLE64(A, V)   {int val64offset = Table64(dyn, (V)); MESSAGE(LOG_DUMP, "  Table64: 0x%lx\n", (V)); AUIPC(A, SPLIT20(val64offset)); LD(A, A, SPLIT12(val64offset));}
