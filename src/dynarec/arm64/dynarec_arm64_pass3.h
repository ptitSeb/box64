#define INIT
#define FINI        \
    if(ninst)       \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst-1].x64.size, dyn->insts[ninst-1].size/4); \
    addInst(dyn->instsize, &dyn->insts_size, 0, 0);
#define EMIT(A)                                         \
    do{                                                 \
        if(dyn->need_dump) print_opcode(dyn, ninst, (uint32_t)(A)); \
        if((uintptr_t)dyn->block<dyn->tablestart)       \
            *(uint32_t*)(dyn->block) = (uint32_t)(A);   \
        dyn->block += 4; dyn->native_size += 4;         \
        dyn->insts[ninst].size2 += 4;                   \
    }while(0)

#define MESSAGE(A, ...)                                                   \
    do {                                                                  \
        if (dyn->need_dump) dynarec_log(LOG_NONE, __VA_ARGS__); \
    } while (0)
#define ENDPREFIX   dyn->insts[ninst].size2 = 0
#define NEW_INST        \
    if(ninst) {                                                  \
        if(dyn->insts[ninst].address!=(uintptr_t)dyn->block-(uintptr_t)dyn->native_start) dyn->abort = 1;   \
        addInst(dyn->instsize, &dyn->insts_size, dyn->insts[ninst-1].x64.size, dyn->insts[ninst-1].size/4); \
        dyn->insts[ninst].ymm0_pass3 = dyn->ymm_zero;   \
    }
#define INST_EPILOG
#define INST_NAME(name) inst_name_pass3(dyn, ninst, name, rex)
#define TABLE64(A, V)  do {                                                                 \
                if(dyn->need_reloc)                                                         \
                        AddRelocTable64Addr(dyn, ninst, (V), 3);                            \
                int val64offset = Table64(dyn, (V), 3);                                     \
                MESSAGE(LOG_DUMP, "  Table64: 0x%lx\n", (V)); LDRx_literal(A, val64offset); \
            } while(0)
#define TABLE64_(A, V)   {                                                                  \
                int val64offset = Table64(dyn, (V), 3);                                     \
                MESSAGE(LOG_DUMP, "  Table64: 0x%lx\n", (V)); LDRx_literal(A, val64offset); \
            }
#define TABLE64C(A, V) do {                                                                 \
                if(dyn->need_reloc)                                                         \
                        AddRelocTable64Const(dyn, ninst, (V), 3);                           \
                int val64offset = Table64(dyn, getConst(V), 3);                             \
                MESSAGE(LOG_DUMP, "  Table64C: 0x%lx\n", (V)); LDRx_literal(A, val64offset);\
            } while(0)
#define FTABLE64(A, V)  do {mmx87_regs_t v = {.d = V}; int val64offset = Table64(dyn, v.q, 3); MESSAGE(LOG_DUMP, "  FTable64: %g\n", v.d); VLDR64_literal(A, val64offset);} while(0)
#define CALLRET_RET()   do { if(BOX64DRENV(dynarec_callret)>1) {dyn->callrets[dyn->callret_size].type = 0; dyn->callrets[dyn->callret_size++].offs = dyn->native_size; EMIT(ARCH_NOP);} } while(0)
#define CALLRET_LOOP()   do {dyn->callrets[dyn->callret_size].type = 1; dyn->callrets[dyn->callret_size++].offs = dyn->native_size; EMIT(ARCH_NOP); } while(0)
