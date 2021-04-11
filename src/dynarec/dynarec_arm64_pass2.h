#define INIT        dyn->arm_size = 0
#define FINI        if(ninst) {dyn->insts[ninst].address = (dyn->insts[ninst-1].address+dyn->insts[ninst-1].size);}

#define MESSAGE(A, ...)  
#define EMIT(A)     dyn->insts[ninst].size+=4; dyn->arm_size+=4
#define NEW_INST    if(ninst) {dyn->insts[ninst].address = (dyn->insts[ninst-1].address+dyn->insts[ninst-1].size);}
#define INST_EPILOG dyn->insts[ninst].epilog = dyn->arm_size; 
#define INST_NAME(name) 
#define NEW_BARRIER_INST    if(ninst) ++dyn->sons_size
#define TABLE64(A, V)   {Table64(dyn, (V)); EMIT(0);}
#define FTABLE64(A, V)  {mmx87_regs_t v = {.d = V}; Table64(dyn, v.q); EMIT(0);}