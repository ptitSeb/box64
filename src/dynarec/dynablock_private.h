#ifndef __DYNABLOCK_PRIVATE_H_
#define __DYNABLOCK_PRIVATE_H_

typedef struct instsize_s {
    unsigned char x64:4;
    unsigned char nat:4;
} instsize_t;

typedef struct dynablock_s {
    void*           block;  // block-sizeof(void*) == self
    void*           actual_block;   // the actual start of the block (so block-sizeof(void*))
    struct dynablock_s*    previous;   // a previous block that might need to be freed
    int             size;
    void*           x64_addr;
    uintptr_t       x64_size;
    uint32_t        hash;
    uint8_t         done;
    uint8_t         gone;
    uint8_t         always_test;
    uint8_t         dirty;      // if need to be tested as soon as it's created
    int             isize;
    instsize_t*     instsize;
    void*           jmpnext;    // a branch jmpnext code when block is marked
} dynablock_t;

#endif //__DYNABLOCK_PRIVATE_H_