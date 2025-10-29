#ifndef _BOX64_THREADS_H_
#define _BOX64_THREADS_H_
#include <box32.h>

typedef struct box64context_s box64context_t;
typedef struct x64emu_s x64emu_t;

typedef struct emuthread_s {
	uintptr_t 	fnc;
	void*		arg;
	x64emu_t*	emu;
	int			join;
	int			is32bits;
	uintptr_t	self;
	ulong_t 	hself;
	int			cancel_cap, cancel_size;
	void**		cancels;
} emuthread_t;

void CleanStackSize(box64context_t* context);

void init_pthread_helper(void);
void fini_pthread_helper(box64context_t* context);
void clean_current_emuthread(void);
#ifdef BOX32
void init_pthread_helper_32(void);
void fini_pthread_helper_32(box64context_t* context);
#endif

// prepare an "emuthread structure" in pet and return address of function pointer for a "thread creation routine"
void* my_prepare_thread(x64emu_t *emu, void* f, void* arg, int ssize, void** pet);

//check and unlock if a mutex is locked by current thread (works only for PTHREAD_MUTEX_ERRORCHECK typed mutex)
int checkUnlockMutex(void* m);
//check if a mutex is locked by current thread (works only for PTHREAD_MUTEX_ERRORCHECK typed mutex)
int checkNolockMutex(void* m);

#ifdef BAD_PKILL
void add_thread(void* t, emuthread_t* et);
void del_thread(void* t);
emuthread_t* get_thread(void* t);
#endif

#endif //_BOX64_THREADS_H_