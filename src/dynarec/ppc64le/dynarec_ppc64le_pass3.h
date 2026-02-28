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
    }                                                            \
    AREFLAGSNEEDED()
#define INST_EPILOG
#define INST_NAME(name) inst_name_pass3(dyn, ninst, name, rex)
// TABLE64: PPC64LE loads from constant pool using 4 instructions:
//   bcl 20,31,.+4          (get PC into LR, BO=20 "always", BI=31)
//   mflr Rd                (move LR to target register)
//   addis Rd,Rd,ha(delta)  (add upper 16 bits of adjusted offset)
//   ld Rd,lo(delta)(Rd)    (load 64-bit value with lower offset)
// where delta = table_entry_addr - (addr_of_bcl + 4)
// We use helper macros to split the offset:
//   PPC64_HI16(x)  = upper 16 bits with sign adjustment for lower half
//   PPC64_LO16(x)  = lower 16 bits (sign-extended by hardware)
#define PPC64_LO16(x) ((int16_t)((x) & 0xFFFF))
#define PPC64_HI16(x) ((int16_t)((((x) >> 16) + (((x) & 0x8000) ? 1 : 0)) & 0xFFFF))
#define TABLE64(A, V)  do {                                                                 \
                if(dyn->need_reloc)                                                         \
                        AddRelocTable64Addr(dyn, ninst, (V), 3);                            \
                int val64offset = Table64(dyn, (V), 3);                                     \
                int _delta = val64offset - 4;                                               \
                MESSAGE(LOG_DUMP, "  Table64: 0x%lx (offset %d)\n", (V), val64offset);      \
                BCL(20, 31, 4);                                                             \
                MFLR(A);                                                                    \
                ADDIS(A, A, PPC64_HI16(_delta));                                            \
                LD(A, PPC64_LO16(_delta), A);                                               \
            } while(0)
#define TABLE64_(A, V)   {                                                                  \
                int val64offset = Table64(dyn, (V), 3);                                     \
                int _delta = val64offset - 4;                                               \
                MESSAGE(LOG_DUMP, "  Table64: 0x%lx (offset %d)\n", (V), val64offset);      \
                BCL(20, 31, 4);                                                             \
                MFLR(A);                                                                    \
                ADDIS(A, A, PPC64_HI16(_delta));                                            \
                LD(A, PPC64_LO16(_delta), A);                                               \
            }
#define TABLE64C(A, V) do {                                                                 \
                if(dyn->need_reloc)                                                         \
                        AddRelocTable64Const(dyn, ninst, (V), 3);                           \
                int val64offset = Table64(dyn, getConst(V), 3);                             \
                int _delta = val64offset - 4;                                               \
                MESSAGE(LOG_DUMP, "  Table64C: 0x%lx (offset %d)\n", (V), val64offset);     \
                BCL(20, 31, 4);                                                             \
                MFLR(A);                                                                    \
                ADDIS(A, A, PPC64_HI16(_delta));                                            \
                LD(A, PPC64_LO16(_delta), A);                                               \
            } while(0)
#define FTABLE64(A, V)  do {                                                                \
                mmx87_regs_t v = {.d = V};                                                  \
                int val64offset = Table64(dyn, v.q, 3);                                     \
                int _delta = val64offset - 4;                                               \
                MESSAGE(LOG_DUMP, "  FTable64: %g (offset %d)\n", v.d, val64offset);        \
                BCL(20, 31, 4);                                                             \
                MFLR(x1);                                                                   \
                ADDIS(x1, x1, PPC64_HI16(_delta));                                          \
                LFD(A, PPC64_LO16(_delta), x1);                                             \
            } while(0)
#define CALLRET_RET(A)                                                          \
    do {                                                                        \
        if((A) && ISSEP() && BOX64DRENV(dynarec_callret)) {                     \
            MESSAGE(LOG_DUMP, "   Dynablock*\n");                               \
            dyn->block += sizeof(void*);                                        \
            dyn->native_size+=sizeof(void*);                                    \
            dyn->insts[ninst].size2 += sizeof(void*);                           \
            dyn->sep[dyn->sep_size].x64_offs = addr - dyn->start;               \
            dyn->sep[dyn->sep_size].nat_offs =  dyn->native_size;               \
            ++dyn->sep_size;                                                    \
        }                                                                       \
        if(BOX64DRENV(dynarec_callret)>1) {                                     \
            dyn->callrets[dyn->callret_size].type = 0;                          \
            dyn->callrets[dyn->callret_size++].offs = dyn->native_size;         \
            EMIT(ARCH_NOP);                                                     \
        }                                                                       \
    } while(0)
#define CALLRET_LOOP()   do {dyn->callrets[dyn->callret_size].type = 1; dyn->callrets[dyn->callret_size++].offs = dyn->native_size; EMIT(ARCH_NOP); } while(0)
