#define INIT        dyn->native_size = 0
#define FINI                                                                                            \
        if(ninst) {                                                                                     \
                dyn->insts[ninst].address = (dyn->insts[ninst-1].address+dyn->insts[ninst-1].size);     \
                dyn->insts_size += 1+((dyn->insts[ninst].x64.size>dyn->insts[ninst].size)?dyn->insts[ninst].x64.size:dyn->insts[ninst].size)/15; \
        }

#define MESSAGE(A, ...)  
#define EMIT(A)     dyn->insts[ninst].size+=4; dyn->native_size+=4
#define NEW_INST                                                                                        \
        if(ninst) {                                                                                     \
                dyn->insts[ninst].address = (dyn->insts[ninst-1].address+dyn->insts[ninst-1].size);     \
                if(isInstClean(dyn, ninst) && dyn->last_ip!=ip)                                         \
                        dyn->last_ip = 0;                                                               \
                dyn->insts_size += 1+((dyn->insts[ninst-1].x64.size>dyn->insts[ninst-1].size)?dyn->insts[ninst-1].x64.size:dyn->insts[ninst-1].size)/15; \
        }
#define INST_EPILOG dyn->insts[ninst].epilog = dyn->native_size; 
#define INST_NAME(name) 
#define TABLE64(A, V)   {Table64(dyn, (V)); EMIT(0);}
#define FTABLE64(A, V)  {mmx87_regs_t v = {.d = V}; Table64(dyn, v.q); EMIT(0);}