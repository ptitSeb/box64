#ifndef __DYNABLOCK_PRIVATE_H_
#define __DYNABLOCK_PRIVATE_H_

typedef struct dynablocklist_s  dynablocklist_t;

typedef struct instsize_s {
    unsigned int x64:4;
    unsigned int nat:4;
} instsize_t;

typedef struct dynablock_s {
    dynablocklist_t* parent;
    void*           block;
    int             size;
    void*           x64_addr;
    uintptr_t       x64_size;
    uint32_t        hash;
    uint8_t         need_test;
    uint8_t         done;
    uint8_t         gone;
    uint8_t         dummy;
    int             isize;
    dynablock_t**   sons;   // sons (kind-of dummy dynablock...)
    int             sons_size;
    dynablock_t*    father; // set only in the case of a son
    instsize_t*     instsize;
} dynablock_t;

typedef struct dynablocklist_s {
    uintptr_t           text;
    int                 textsz;
    int                 maxsz;     // maxblock size (for this block or previous block)
    dynablock_t**       direct;    // direct mapping (waste of space, so the array is created at first write)
} dynablocklist_t;

#endif //__DYNABLOCK_PRIVATE_H_