#ifndef __DYNACACHE_H__
#define __DYNACACHE_H__

#include <stddef.h>
#include <stdint.h>

typedef struct mapping_s mapping_t;

#ifdef DYNAREC
typedef struct blocklist_s blocklist_t;

typedef struct DynaCacheBlock_s {
    blocklist_t*    block;
    size_t          size;
    size_t          free_size;
} DynaCacheBlock_t;

#define COMP_NONE   0
#define COMP_Z      1
#define COMP_Z_MAX  2
typedef struct CompressedDynaCacheBlock_s {
    DynaCacheBlock_t block;
    size_t      compsize;
    uint8_t     type;
} CompressedDynaCacheBlock_t;

void SerializeMmaplist(mapping_t* mapping);
void MmapDynaCache(mapping_t* mapping);
#endif

void DynaCacheList(const char* filter);
void DynaCacheClean();

#endif // __DYNACACHE_H__
