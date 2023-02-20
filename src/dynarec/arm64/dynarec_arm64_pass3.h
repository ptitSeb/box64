#define INIT    
#define FINI        \
    if(ninst)       \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst].x64.size, dyn->insts[ninst].size/4); \
    addInst(dyn->instsize, &dyn->insts_size, 0, 0);
#define EMIT(A)     \
    if(box64_dynarec_dump) {dynarec_log(LOG_NONE, "\t%08x\t%s\n", (uint32_t)(A), arm64_print(A, (uintptr_t)dyn->block));} \
    *(uint32_t*)(dyn->block) = (uint32_t)(A);       \
    dyn->block += 4; dyn->native_size += 4;         \
    dyn->insts[ninst].size2 += 4

#define MESSAGE(A, ...)  if(box64_dynarec_dump) dynarec_log(LOG_NONE, __VA_ARGS__)
#define NEW_INST        \
    if(ninst && isInstClean(dyn, ninst)) {                      \
        if(dyn->last_ip!=ip) dyn->last_ip = 0;                  \
    }                                                           \
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
            for(int ii=0; ii<dyn->insts[ninst].pred_sz; ++ii)\
                dynarec_log(LOG_NONE, "%s%d", ii?"/":"", dyn->insts[ninst].pred[ii]);\
        }                                       \
        if(dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts>=0)\
            dynarec_log(LOG_NONE, ", jmp=%d", dyn->insts[ninst].x64.jmp_insts);\
        if(dyn->insts[ninst].x64.jmp && dyn->insts[ninst].x64.jmp_insts==-1)\
            dynarec_log(LOG_NONE, ", jmp=out"); \
        if(dyn->last_ip)                        \
            dynarec_log(LOG_NONE, ", last_ip=%p", (void*)dyn->last_ip);\
        for(int ii=0; ii<24; ++ii) {            \
            switch(dyn->insts[ninst].n.neoncache[ii].t) {    \
                case NEON_CACHE_ST_D: dynarec_log(LOG_NONE, " D%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;  \
                case NEON_CACHE_ST_F: dynarec_log(LOG_NONE, " S%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;  \
                case NEON_CACHE_MM: dynarec_log(LOG_NONE, " D%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;    \
                case NEON_CACHE_XMMW: dynarec_log(LOG_NONE, " Q%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;  \
                case NEON_CACHE_XMMR: dynarec_log(LOG_NONE, " Q%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;  \
                case NEON_CACHE_SCR: dynarec_log(LOG_NONE, " D%d:%s", ii, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;   \
                case NEON_CACHE_NONE:           \
                default:    break;              \
            }                                   \
        }                                       \
        if(dyn->n.stack || dyn->insts[ninst].n.stack_next || dyn->insts[ninst].n.x87stack)     \
            dynarec_log(LOG_NONE, " X87:%d/%d(+%d/-%d)%d", dyn->n.stack, dyn->insts[ninst].n.stack_next, dyn->insts[ninst].n.stack_push, dyn->insts[ninst].n.stack_pop, dyn->insts[ninst].n.x87stack); \
        if(dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2)                     \
            dynarec_log(LOG_NONE, " %s:%d/%d", dyn->insts[ninst].n.swapped?"SWP":"CMB", dyn->insts[ninst].n.combined1, dyn->insts[ninst].n.combined2);   \
        dynarec_log(LOG_NONE, "%s\n", (box64_dynarec_dump>1)?"\e[m":"");                       \
    }

#define TABLE64(A, V)   {int val64offset = Table64(dyn, (V)); MESSAGE(LOG_DUMP, "  Table64: 0x%lx\n", (V)); LDRx_literal(A, val64offset);}
#define FTABLE64(A, V)  {mmx87_regs_t v = {.d = V}; int val64offset = Table64(dyn, v.q); MESSAGE(LOG_DUMP, "  FTable64: %g\n", v.d); VLDR64_literal(A, val64offset);}
