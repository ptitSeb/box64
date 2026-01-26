#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "x64emu.h"

uint32_t fallback_random32(void)
{
    return random() ^ (random()<<1);
}

uint32_t get_random32(void)
{
    uint32_t ret;
    FILE* f = fopen("/dev/urandom", "rb");
    if(f) {
        if(fread(&ret, sizeof(ret), 1, f)!=1)
            ret = fallback_random32();
        fclose(f);
    } else
        ret = fallback_random32();
    return ret;
}
uint64_t fallback_random64(void)
{
    return random() ^ (((uint64_t)random())<<18) ^ (((uint64_t)random())<<41);
}

uint64_t get_random64(void)
{
    uint64_t ret;
    FILE* f = fopen("/dev/urandom", "rb");
    if(f) {
        if(fread(&ret, sizeof(ret), 1, f)!=1)
            ret = fallback_random64();
        fclose(f);
    } else
        ret = fallback_random64();
    return ret;
}
