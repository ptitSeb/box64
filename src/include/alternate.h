#ifndef __ALTERNATE_H__
#define __ALTERNATE_H__

int hasAlternate(void* addr);
void* getAlternate(void* addr);
void addAlternate(void* addr, void* alt);
void addCheckAlternate(void* addr, void* alt);
void cleanAlternate(void);

#endif //__ALTERNATE_H__
