#ifndef __DYNACACHE_COMPRESS__H__
#define __DYNACACHE_COMPRESS__H__

#include <stddef.h>

#include "dynacache.h"

void* dc_compress(void* src, size_t src_size, int type, size_t* dest_size);
int dc_uncompress(void* src, size_t src_size, int type, void* dest, size_t dest_size);  // return 0 is OK

#endif //__DYNACACHE_COMPRESS__H__
