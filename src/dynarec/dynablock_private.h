#ifndef __DYNABLOCK_PRIVATE_H_
#define __DYNABLOCK_PRIVATE_H_

typedef struct instsize_s {
    unsigned int x64:4;
    unsigned int nat:4;
} instsize_t;

typedef struct dynablock_s {
    void*           block;  // block-sizeof(void*) == self
    void*           actual_block;   // the actual start of the block (so block-sizeof(void*))
    int             size;
    void*           x64_addr;
    uintptr_t       x64_size;
    uint32_t        hash;
    uint8_t         need_test;
    uint8_t         done;
    uint8_t         gone;
    uint8_t         dummy;
    int             isize;
    instsize_t*     instsize;
    void*           jmpnext;    // a branch jmpnext code when block is marked
} dynablock_t;

#endif //__DYNABLOCK_PRIVATE_H_