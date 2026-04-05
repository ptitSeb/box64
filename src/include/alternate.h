#ifndef __ALTERNATE_H__
#define __ALTERNATE_H__
#include <stdint.h>

#if defined(DYNAREC) && !defined(WIN32)
#define HAVE_ALTJUMP
#endif

int hasAlternate(void* addr);
void* getAlternate(void* addr);
void addAlternate(void* addr, void* alt);
void addCheckAlternate(void* addr, void* alt);
void cleanAlternate(void);
#ifdef HAVE_ALTJUMP
uintptr_t getAlternateJump(void* addr, int is32bits);
// will return MAP_FAIL (void*)-1LL if there no alternate at addr
void* getAlternateData(void* addr);
void setAlternateData(void* addr, void* data);
#endif
#endif //__ALTERNATE_H__
