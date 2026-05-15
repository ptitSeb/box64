#ifndef __ENV_PRIVATE_H__
#define __ENV_PRIVATE_H__

#include <stdint.h>

#include "env.h"

typedef struct mapping_s {
    char*       filename;
    char*       fullname;
    box64env_t* env;
    uintptr_t   start;  //lower address of the map for this file
    mmaplist_t* mmaplist;
} mapping_t;

char* LowerCase(const char* s);

#endif // __ENV_PRIVATE_H__
