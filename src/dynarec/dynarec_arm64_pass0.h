
#define INIT    uintptr_t sav_addr=addr
#define FINI    dyn->isize = addr-sav_addr
#define MESSAGE(A, ...)  
#define SETFLAGS(A, B)  
#define READFLAGS(A)    
#define EMIT(A)     
#define JUMP(A)         add_next(dyn, (uintptr_t)A)
#define NEW_INST        ++dyn->size
#define INST_EPILOG         
#define INST_NAME(name) 
#define DEFAULT                         \
        --dyn->size;                    \
        *ok = -1;                       \
        if(box64_dynarec_log>=LOG_INFO) {\
        dynarec_log(LOG_NONE, "%p: Dynarec stopped because of Opcode %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", \
        (void*)ip, PKip(0),             \
        PKip(1), PKip(2), PKip(3),      \
        PKip(4), PKip(5), PKip(6),      \
        PKip(7), PKip(8), PKip(9),      \
        PKip(10),PKip(11),PKip(12),     \
        PKip(13),PKip(14));             \
        printFunctionAddr(ip, " => ");  \
        dynarec_log(LOG_NONE, "\n");    \
        }
