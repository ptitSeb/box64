#define INIT    
#define FINI
#define MESSAGE(A, ...)  
#define EMIT(A)     
#define READFLAGS(A)    dyn->insts[ninst].x64.use_flags = A
#define SETFLAGS(A,B)   {dyn->insts[ninst].x64.set_flags = A; dyn->insts[ninst].x64.state_flags = B;}

#define NEW_INST
#define INST_EPILOG
#define INST_NAME(name)  
