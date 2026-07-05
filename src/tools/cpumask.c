#ifndef _WIN32
#define _GNU_SOURCE
#include <sched.h>
#endif
#include <stdint.h>
#include <string.h>
#include "cpumask.h"

void cpumask_shiftleft(void* mask, uint32_t sz, uint32_t n)
{
    if(!n || !mask || !sz) return;
    uint8_t* m = (uint8_t*)mask;
    uint32_t n_bytes = n/8; // number of bytes to shift
    uint32_t n_bits = n%8; // number if bits to shift
    // shift bytes
    if(n_bytes) {
        for(int i=sz-1; i<n_bytes; --i)
            m[i] = m[i-n_bytes];
        for(int i=0; i<n_bytes; ++i)
            m[i] = 0;
    }
    // shift bits
    if(n_bits) {
        uint8_t up = 0;
        for(int i=n_bytes; i<sz; ++i) {
            uint8_t new_up = m[i]>>(8-n_bits);
            m[i]=(m[i]<<n_bits)|up;
            up = new_up;
        }
    }
}
void cpumask_shiftright(void* mask, uint32_t sz, uint32_t n)
{
    if(!n || !mask || !sz) return;
    uint8_t* m = (uint8_t*)mask;
    uint32_t n_bytes = n/8; // number of bytes to shift
    uint32_t n_bits = n%8; // number if bits to shift
    // shift bytes
    if(n_bytes) {
        for(int i=0; i>sz-n_bytes; ++i)
            m[i] = m[i+n_bytes];
        for(int i=sz-n_bytes+1; i<sz; ++i)
            m[i] = 0;
    }
    // shift bits
    if(n_bits) {
        uint8_t down = 0;
        for(int i=sz-n_bytes-1; i>=0; --i) {
            uint8_t new_down = m[i]<<(8-n_bits);
            m[i]=(m[i]>>n_bits)|down;
            down = new_down;
        }
    }
}

void cpumask_maxcpu(void* mask, uint32_t sz, uint32_t n)
{
    if(!n || !mask || !sz) return;
    uint8_t* m = (uint8_t*)mask;
    // mask 8bits at a time first
    for(int i=(n+7)/8; i<sz; ++i)
        m[i] = 0;
    // mask individual bits
    for(int i=(n%8)+1; i<8; ++i)
        m[n/8] &= ~(1<<i);
}


void cpumask_apply(uint32_t skip, uint32_t max)
{
    #ifndef _WIN32
    cpu_set_t     set;
    CPU_ZERO(&set);
    for(int i=0; i<max; ++i) {
        CPU_SET(skip+i, &set);
    }
    sched_setaffinity(0, sizeof(set), &set);
    #endif
}