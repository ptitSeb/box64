#ifndef __CPU_MASK__H__
#define __CPU_MASK__H__

#include <stdint.h>

void cpumask_shiftleft(void* mask, uint32_t sz, uint32_t n);
void cpumask_shiftright(void* mask, uint32_t sz, uint32_t n);
void cpumask_maxcpu(void* mask, uint32_t sz, uint32_t n);

void cpumask_apply(uint32_t skip, uint32_t max);

#endif //__CPU_MASK__H__
