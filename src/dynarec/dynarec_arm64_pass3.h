#define INIT    
#define FINI
#define EMIT(A)     \
    if(box64_dynarec_dump) {dynarec_log(LOG_NONE, "\t%08x\t%s\n", (uint32_t)(A), arm64_print(A));} \
    *(uint32_t*)(dyn->block) = (uint32_t)(A);       \
    dyn->block += 4; dyn->arm_size += 4;\
    dyn->insts[ninst].size2 += 4

#define MESSAGE(A, ...)  if(box64_dynarec_dump) dynarec_log(LOG_NONE, __VA_ARGS__)
#define NEW_INST        
#define INST_EPILOG     
#define INST_NAME(name) \
    if(box64_dynarec_dump) {\
        printf_x64_instruction(my_context->dec, &dyn->insts[ninst].x64, name); \
        dynarec_log(LOG_NONE, "%s%p: %d emited opcodes, state=%d/%d, set=%X, use=%X, need=%X%s\n", \
            (box64_dynarec_dump>1)?"\e[32m":"", \
            (void*)(dyn->arm_start+dyn->insts[ninst].address),  \
            dyn->insts[ninst].size/4,           \
            dyn->insts[ninst].x64.state_flags,  \
            dyn->state_flags,                   \
            dyn->insts[ninst].x64.set_flags,    \
            dyn->insts[ninst].x64.use_flags,    \
            dyn->insts[ninst].x64.need_flags,   \
            (box64_dynarec_dump>1)?"\e[m":"");  \
    }

#define NEW_BARRIER_INST                            \
    if(ninst) {                                     \
    dyn->sons_x64[dyn->sons_size] = (uintptr_t)ip;  \
    dyn->sons_arm[dyn->sons_size] = dyn->block;     \
    MESSAGE(LOG_DUMP, "----> potential Son here\n");\
    ++dyn->sons_size;                               \
    }

