#ifndef __DYNABLOCK_PRIVATE_H_
#define __DYNABLOCK_PRIVATE_H_

typedef struct instsize_s {
    unsigned char x64:4;
    unsigned char nat:4;
} instsize_t;

typedef struct callret_s {
    uint32_t    offs:31;
    uint32_t    type:1;
} callret_t;

typedef struct dynablock_s {
    void*           block;  // block-sizeof(void*) == self
    void*           actual_block;   // the actual start of the block (so block-sizeof(void*))
    struct dynablock_s*    previous;   // a previous block that might need to be freed
    uint32_t        in_used;// will be 0 if not in_used, >0 if used be some code
    uint32_t        hot;    // simple counter on how many time the block is run
    void*           x64_addr;
    uintptr_t       x64_size;
    size_t          native_size;
    uint32_t        prefixsize; // size in byte of the prefix of the block
    int             size;
    uint32_t        hash;
    uint8_t         done;
    uint8_t         gone;
    uint8_t         dirty;      // if need to be tested as soon as it's created
    uint8_t         always_test:2;
    uint8_t         is32bits:1;
    int             callret_size;   // size of the array
    int             isize;
    size_t          arch_size;  // size of of arch dependant infos
    instsize_t*     instsize;
    void*           arch;       // arch dependant per inst info (can be NULL)
    callret_t*      callrets;   // array of callret return, with NOP / UDF depending if the block is clean or dirty
    void*           jmpnext;    // a branch jmpnext code when block is marked
    size_t          table64size;// to check table64
    void*           table64;    // to relocate the table64
    size_t          relocsize;  // number of relocations (should be an int only)
    void*           relocs;     // relocations, when block is loaded
    #ifdef GDBJIT
    void*           gdbjit_block;
    #endif
} dynablock_t;

#endif //__DYNABLOCK_PRIVATE_H_
