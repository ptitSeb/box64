#ifndef _THREADS_H_
#define _THREADS_H_

typedef struct box64context_s box64context_t;
typedef struct x64emu_s x64emu_t;

void CleanStackSize(box64context_t* context);

void init_pthread_helper(void);
void fini_pthread_helper(box64context_t* context);
void clean_current_emuthread(void);

// prepare an "emuthread structure" in pet and return address of function pointer for a "thread creation routine"
void* my_prepare_thread(x64emu_t *emu, void* f, void* arg, int ssize, void** pet);

//check and unlock if a mutex is locked by current thread (works only for PTHREAD_MUTEX_ERRORCHECK typed mutex)
int checkUnlockMutex(void* m);

#endif //_THREADS_H_