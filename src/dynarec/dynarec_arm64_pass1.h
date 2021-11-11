#define INIT    
#define FINI     \
    dyn->insts[ninst].x64.addr = addr; \
    if(ninst) dyn->insts[ninst-1].x64.size = dyn->insts[ninst].x64.addr - dyn->insts[ninst-1].x64.addr;
#define MESSAGE(A, ...)  
#define EMIT(A)     
#define READFLAGS(A)    dyn->insts[ninst].x64.use_flags = A
#define SETFLAGS(A,B)   {dyn->insts[ninst].x64.set_flags = A; dyn->insts[ninst].x64.state_flags = B;}

#define NEW_INST \
    dyn->insts[ninst].x64.addr = ip; \
    if(ninst) dyn->insts[ninst-1].x64.size = dyn->insts[ninst].x64.addr - dyn->insts[ninst-1].x64.addr;
#define INST_EPILOG
#define INST_NAME(name)  
