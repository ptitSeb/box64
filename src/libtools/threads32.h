#ifndef __THREADS_32_H_
#define __THREADS_32_H_
#include "box32.h"

typedef struct fake_pthread_mutext_s {
	int __lock;
	unsigned int __count;
  	int __owner;
	int i386__kind;
	int __kind;
	ptr_t real_mutex;
} fake_phtread_mutex_t;
#define KIND_SIGN	0xbad001

#endif //__THREADS_32_H_