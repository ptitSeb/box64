// __USE_UNIX98 is needed for sttype / gettype definition
#define __USE_UNIX98
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <dlfcn.h>

#include "os.h"
#include "debug.h"
#include "box32context.h"
#include "threads.h"
#include "emu/x64emu_private.h"
#include "tools/bridge_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "box64cpu.h"
#include "box64cpu_util.h"
#include "callback.h"
#include "custommem.h"
#include "khash.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "bridge.h"
#include "threads32.h"
#include "x64tls.h"
#include "x64_signals.h"
#ifdef DYNAREC
#include "dynablock.h"
#endif

#ifndef MAP_32BIT
#define MAP_32BIT 0x40
#endif

typedef void (*vFppp_t)(void*, void*, void*);
typedef void (*vFpi_t)(void*, int);
typedef int  (*iFLi_t)(unsigned long, int);
//starting with glibc 2.34+, those 2 functions are in libc.so as versioned symbol only
// So use dlsym to get the symbol unversioned, as simple link will not work.
static vFppp_t real_pthread_cleanup_push_defer = NULL;
static vFpi_t real_pthread_cleanup_pop_restore = NULL;
// with glibc 2.34+, pthread_kill changed behaviour and might break some program, so using old version if possible
// it will be pthread_kill@GLIBC_2.0+, need to be found, while it's GLIBC_2.0 on i386
static iFLi_t real_phtread_kill_old = NULL;
// those function can be used simply
void _pthread_cleanup_push(void* buffer, void* routine, void* arg);	// declare hidden functions
void _pthread_cleanup_pop(void* buffer, int exec);

typedef struct threadstack_s {
	void* 	stack;
	size_t 	stacksize;
} threadstack_t;

// those are define in thread.c
emuthread_t* thread_get_et();
void thread_set_et(emuthread_t* et);
void emuthread_destroy(void* p);

static pthread_attr_t* get_attr(void* attr);
static void del_attr(void* attr);

typedef void(*vFv_t)();

KHASH_MAP_INIT_INT(threadstack, threadstack_t)

void CleanStackSize(box64context_t* context);
void FreeStackSize(uintptr_t attr);
void AddStackSize(uintptr_t attr, void* stack, size_t stacksize);
int GetStackSize(uintptr_t attr, void** stack, size_t* stacksize);

void my32_longjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val);

static void emuthread_cancel(void* p)
{
	emuthread_t *et = (emuthread_t*)p;
	if(!et)
		return;
	// check cancels threads
	uintptr_t rax = et->emu->regs[_AX].q[0];
	for(int i=et->cancel_size-1; i>=0; --i) {
		et->emu->flags.quitonlongjmp = 0;
		et->emu->quit = 0;
		my32_longjmp(et->emu, ((i386_unwind_buff_t*)et->cancels[i])->__cancel_jmp_buf, 1);
		DynaRun(et->emu);	// will return after a __pthread_unwind_next()
	}
	et->emu->regs[_AX].q[0] = rax;
	box_free(et->cancels);
	et->cancels=NULL;
	et->cancel_size = et->cancel_cap = 0;
}

static void* pthread_routine(void* p)
{
	// free current emuthread if it exist
	{
		void* t = thread_get_et();
		if(t) {
			// not sure how this could happens
			printf_log(LOG_INFO, "Clean of an existing ET for Thread %04d\n", GetTID());
			emuthread_destroy(t);
		}
	}
	// call the function
	emuthread_t *et = (emuthread_t*)p;
	thread_set_et(et);
	et->is32bits = 1;
	et->emu->type = EMUTYPE_MAIN;
	et->self = (uintptr_t)pthread_self();
	et->hself = to_hash(et->self);
	// setup callstack and run...
	x64emu_t* emu = et->emu;
	refreshTLSData(emu);
	Push_32(emu, 0);	// PUSH 0 (backtrace marker: return address is 0)
	Push_32(emu, 0);	// PUSH BP
	R_EBP = R_ESP;	// MOV BP, SP
    R_ESP -= 32;	// guard area
	R_ESP &=~15;
	R_ESP -= 3*4;	// prepare alignment
	Push_32(emu, to_ptrv(et->arg));
	PushExit_32(emu);
	R_EIP = to_ptr(et->fnc);
	pthread_cleanup_push(emuthread_cancel, p);
	DynaRun(et->emu);
	pthread_cleanup_pop(0);
	void* ret = from_ptrv(R_EAX);
	return ret;
}

EXPORT int my32_pthread_attr_destroy(x64emu_t* emu, void* attr)
{
	if(my_context->stacksizes)
		FreeStackSize((uintptr_t)attr);
	int ret = pthread_attr_destroy(get_attr(attr));
	del_attr(attr);
	return ret;
}

EXPORT int my32_pthread_attr_getstack(x64emu_t* emu, void* attr, void** stackaddr, size_t* stacksize)
{
	int ret = pthread_attr_getstack(get_attr(attr), stackaddr, stacksize);
	if (ret==0)
		GetStackSize((uintptr_t)attr, stackaddr, stacksize);
	return ret;
}

EXPORT int my32_pthread_attr_setstack(x64emu_t* emu, void* attr, void* stackaddr, size_t stacksize)
{
	AddStackSize((uintptr_t)attr, stackaddr, stacksize);
	//Don't call actual setstack...
	//return pthread_attr_setstack(attr, stackaddr, stacksize);
	return pthread_attr_setstacksize(get_attr(attr), stacksize);
}

EXPORT int my32_pthread_create(x64emu_t *emu, void* t, void* attr, void* start_routine, void* arg)
{
	int stacksize = 2*1024*1024;	//default stack size is 2Mo
	void* attr_stack;
	size_t attr_stacksize;
	int own;
	void* stack = NULL;
	pthread_attr_t* my_attr = NULL;
	pthread_attr_t actual_attr;

	if(attr) {
		size_t stsize;
		static size_t minsize = 0;
		if(!minsize) {
			minsize = PTHREAD_STACK_MIN;
			if(minsize<512*1024) minsize = 512*1024;
		}
		if(pthread_attr_getstacksize(get_attr(attr), &stsize)==0)
			stacksize = stsize;
		if(stacksize<minsize)	// emu and all needs some stack space, don't go too low
			pthread_attr_setstacksize(get_attr(attr), minsize);
		if(stacksize>1*1024*1024)
			pthread_attr_setstacksize(get_attr(attr), 1*1024*1024);
	} else {
		my_attr = &actual_attr;
		pthread_attr_init(my_attr);
		pthread_attr_setstacksize(my_attr, 1*1024*1024);
	}
	if(GetStackSize((uintptr_t)attr, &attr_stack, &attr_stacksize))
	{
		stack = attr_stack;
		stacksize = attr_stacksize;
		own = 0;
		if((uintptr_t)stack>=0x100000000LL) {
			printf_log(LOG_INFO, "Address of Stack for thread too high (%p), allocationg a new one\n", stack);
			stack = NULL;
		}
	}
	if(!stack) {
		//stack = malloc(stacksize);
		stack = box_mmap(NULL, stacksize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_32BIT, -1, 0);
		setProtection_stack((uintptr_t)stack, stacksize, PROT_READ|PROT_WRITE);
		own = 1;
	}

	if((uintptr_t)stack>=0x100000000LL) {
		if(own) {
			box_munmap(stack, stacksize);
			freeProtection((uintptr_t)stack, stacksize);
		}
		return EAGAIN;
	}

	emuthread_t *et = (emuthread_t*)box_calloc(1, sizeof(emuthread_t));
    x64emu_t *emuthread = NewX64Emu(my_context, (uintptr_t)start_routine, (uintptr_t)stack, stacksize, own);
	SetupX64Emu(emuthread, emu);
	et->emu = emuthread;
	et->fnc = (uintptr_t)start_routine;
	et->arg = arg;
	if(!attr)
		et->join = 1;
	else {
		int j;
		pthread_attr_getdetachstate(get_attr(attr), &j);
		if(j==PTHREAD_CREATE_JOINABLE)
			et->join = 1;
		else
			et->join = 0;
	}
	#ifdef DYNAREC
	if(BOX64ENV(dynarec)) {
		// pre-creation of the JIT code for the entry point of the thread
		DBGetBlock(emu, (uintptr_t)start_routine, 1, 1);
	}
	if(BOX64ENV(nodynarec_delay)) {
		static int num_threads = 0;
		++num_threads;
		if(num_threads==2 && BOX64ENV(nodynarec_start)) {
			BOX64ENV(nodynarec_start) = 0;
			BOX64ENV(nodynarec_end) = 0;
		}
	}
	#endif
	// create thread
	int ret = pthread_create((pthread_t*)t, my_attr?my_attr:get_attr(attr), 
		pthread_routine, et);
	if(my_attr) pthread_attr_destroy(my_attr);
	return ret;
}

EXPORT int my32_pthread_detach(x64emu_t* emu, pthread_t p)
{
	if(pthread_equal(p ,pthread_self())) {
		emuthread_t *et = (emuthread_t*)thread_get_et();
		if(et)
			et->join = 0;
	}
	return pthread_detach(p);
}

void* my32_prepare_thread(x64emu_t *emu, void* f, void* arg, int ssize, void** pet)
{
	int stacksize = (ssize)?ssize:(2*1024*1024);	//default stack size is 2Mo
	//void* stack = malloc(stacksize);
	void* stack = mmap64(NULL, stacksize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_32BIT, -1, 0);
	emuthread_t *et = (emuthread_t*)box_calloc(1, sizeof(emuthread_t));
    x64emu_t *emuthread = NewX64Emu(emu->context, (uintptr_t)f, (uintptr_t)stack, stacksize, 1);
	SetupX64Emu(emuthread, emu);
	et->emu = emuthread;
	et->fnc = (uintptr_t)f;
	et->arg = arg;
	#ifdef DYNAREC
	if(BOX64ENV(dynarec)) {
		// pre-creation of the JIT code for the entry point of the thread
		dynablock_t *current = NULL;
		DBGetBlock(emu, (uintptr_t)f, 1, 1);
	}
	#endif
	*pet =  et;
	return pthread_routine;
}

void my32_longjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val);

EXPORT void my32___pthread_register_cancel(x64emu_t* emu, i386_unwind_buff_t* buff)
{
	buff = (i386_unwind_buff_t*)from_ptr(R_EAX);	// param is in fact on register
	emuthread_t *et = (emuthread_t*)thread_get_et();
	if(et->cancel_cap == et->cancel_size) {
		et->cancel_cap+=8;
		et->cancels = box_realloc(et->cancels, sizeof(i386_unwind_buff_t*)*et->cancel_cap);
	}
	et->cancels[et->cancel_size++] = buff;
}

EXPORT void my32___pthread_unregister_cancel(x64emu_t* emu, i386_unwind_buff_t* buff)
{
	emuthread_t *et = (emuthread_t*)thread_get_et();
	for (int i=et->cancel_size-1; i>=0; --i) {
		if(et->cancels[i] == buff) {
			if(i!=et->cancel_size-1)
				memmove(et->cancels+i, et->cancels+i+1, sizeof(i386_unwind_buff_t*)*(et->cancel_size-i-1));
			et->cancel_size--;
			return;
		}
	}
}

#define X86_RWLOCK_SIZE	32
EXPORT int my32_pthread_rwlock_init(void* rdlock, void* attr)
{
	// the structure is bigger, but the "active" part should be the same size, so just save/restoore the padding at init
	uint8_t buff[sizeof(pthread_rwlock_t)];
	if(rdlock && sizeof(pthread_rwlock_t)>X86_RWLOCK_SIZE) {
		memcpy(buff, rdlock+32, sizeof(pthread_rwlock_t)-X86_RWLOCK_SIZE);
	}
	int ret = pthread_rwlock_init(rdlock, attr);
	memcpy(rdlock+32, buff, sizeof(pthread_rwlock_t)-X86_RWLOCK_SIZE);
	return ret;
}
EXPORT int my32___pthread_rwlock_init(void*, void*) __attribute__((alias("my32_pthread_rwlock_init")));

EXPORT int my32_pthread_rwlock_destroy(void* rdlock)
{
	// the structure is bigger, but the "active" part should be the same size, so just save/restoore the padding at init
	uint8_t buff[sizeof(pthread_rwlock_t)];
	if(rdlock && sizeof(pthread_rwlock_t)>X86_RWLOCK_SIZE) {
		memcpy(buff, rdlock+32, sizeof(pthread_rwlock_t)-X86_RWLOCK_SIZE);
	}
	int ret = pthread_rwlock_destroy(rdlock);
	memcpy(rdlock+32, buff, sizeof(pthread_rwlock_t)-X86_RWLOCK_SIZE);
	return ret;
}

EXPORT void my32___pthread_unwind_next(x64emu_t* emu, void* p)
{
	emu->quit = 1;
}

KHASH_MAP_INIT_INT(once, int)

#define SUPER() \
GO(0)			\
GO(1)			\
GO(2)			\
GO(3)			\
GO(4)			\
GO(5)			\
GO(6)			\
GO(7)			\
GO(8)			\
GO(9)			\
GO(10)			\
GO(11)			\
GO(12)			\
GO(13)			\
GO(14)			\
GO(15)			\
GO(16)			\
GO(17)			\
GO(18)			\
GO(19)			\
GO(20)			\
GO(21)			\
GO(22)			\
GO(23)			\
GO(24)			\
GO(25)			\
GO(26)			\
GO(27)			\
GO(28)			\
GO(29)			

// cleanup_routine
#define GO(A)   \
static uintptr_t my32_cleanup_routine_fct_##A = 0;  				\
static void my32_cleanup_routine_##A(void* a)    					\
{                                       							\
    RunFunctionFmt(my32_cleanup_routine_fct_##A, "p", a);			\
}
SUPER()
#undef GO
static void* findcleanup_routineFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_cleanup_routine_fct_##A == (uintptr_t)fct) return my32_cleanup_routine_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_cleanup_routine_fct_##A == 0) {my32_cleanup_routine_fct_##A = (uintptr_t)fct; return my32_cleanup_routine_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pthread cleanup_routine callback\n");
    return NULL;
}

// key_destructor
#define GO(A)   \
static uintptr_t my32_key_destructor_fct_##A = 0;  					\
static void my32_key_destructor_##A(void* a)    					\
{                                       							\
    RunFunctionFmt(my32_key_destructor_fct_##A, "p", a);			\
}
SUPER()
#undef GO
static void* findkey_destructorFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my32_key_destructor_fct_##A == (uintptr_t)fct) return my32_key_destructor_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my32_key_destructor_fct_##A == 0) {my32_key_destructor_fct_##A = (uintptr_t)fct; return my32_key_destructor_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pthread key_destructor callback\n");
    return NULL;
}

#undef SUPER


int EXPORT my32_pthread_once(x64emu_t* emu, int* once, void* cb)
{
	if(*once)	// quick test first
		return 0;
	// slow test now
	#ifdef DYNAREC
	int old = native_lock_xchg_d(once, 1);
	#else
	int old = *once;	// outside of the mutex in case once is badly formed
	pthread_mutex_lock(&my_context->mutex_lock);
	old = *once;
	*once = 1;
	pthread_mutex_unlock(&my_context->mutex_lock);
	#endif
	if(old)
		return 0;
    // make some room and align R_RSP before doing the call (maybe it would be simpler to just use Callback functions)
    Push_32(emu, R_EBP); // push rbp
    R_EBP = R_ESP;      // mov rbp, rsp
    R_ESP -= 0x200;
    R_ESP &= ~63LL;
	DynaCall(emu, (uintptr_t)cb);
	R_ESP = R_EBP;          // mov rsp, rbp
	R_EBP = Pop32(emu);     // pop rbp
	return 0;
}
EXPORT int my32___pthread_once(x64emu_t* emu, void* once, void* cb) __attribute__((alias("my32_pthread_once")));

EXPORT int my32_pthread_key_create(x64emu_t* emu, void* key, void* dtor)
{
	return pthread_key_create(key, findkey_destructorFct(dtor));
}
EXPORT int my32___pthread_key_create(x64emu_t* emu, void* key, void* dtor) __attribute__((alias("my32_pthread_key_create")));

// phtread_cond_init with null attr seems to only write 1 (NULL) dword on x64, while it's 48 bytes on ARM. 
// Not sure why as sizeof(pthread_cond_init) is 48 on both platform... But Neverwinter Night init seems to rely on that
// What about cond that are statically initialized? 
// Note, this is is a versioned function (the pthread_cond_*), and this seems to correspond to an old behaviour

KHASH_MAP_INIT_INT(mapcond, pthread_cond_t*);

// should all access to that map be behind a mutex?
kh_mapcond_t *mapcond = NULL;

static pthread_cond_t* add_cond(void* cond)
{
	if(((uintptr_t)cond)&7==0)
		return cond;
	mutex_lock(&my_context->mutex_thread);
	khint_t k;
	int ret;
	pthread_cond_t *c;
	k = kh_put(mapcond, mapcond, (uintptr_t)cond, &ret);
	if(!ret)
		c = kh_value(mapcond, k);	// already there... reinit an existing one?
	else 
		c = kh_value(mapcond, k) = (pthread_cond_t*)box_calloc(1, sizeof(pthread_cond_t));
	//*(ptr_t*)cond = to_ptrv(cond);
	mutex_unlock(&my_context->mutex_thread);
	return c;
}
static pthread_cond_t* get_cond(void* cond)
{
	if(((uintptr_t)cond)&7==0)
		return cond;
	pthread_cond_t* ret;
	int r;
	mutex_lock(&my_context->mutex_thread);
	khint_t k = kh_get(mapcond, mapcond, *(uintptr_t*)cond);
	if(k==kh_end(mapcond)) {
		khint_t k = kh_get(mapcond, mapcond, (uintptr_t)cond);
		if(k==kh_end(mapcond)) {
			printf_log(LOG_DEBUG, "BOX32: Note: phtread_cond not found, create a new empty one\n");
			ret = (pthread_cond_t*)box_calloc(1, sizeof(pthread_cond_t));
			k = kh_put(mapcond, mapcond, (uintptr_t)cond, &r);
			kh_value(mapcond, k) = ret;
			//*(ptr_t*)cond = to_ptrv(cond);
			//pthread_cond_init(ret, NULL);
			memcpy(ret, cond, sizeof(pthread_cond_t));
		} else
			ret = kh_value(mapcond, k);
	} else
		ret = kh_value(mapcond, k);
	mutex_unlock(&my_context->mutex_thread);
	return ret;
}
static void del_cond(void* cond)
{
	if(((uintptr_t)cond)&7==0)
		return;
	if(!mapcond)
		return;
	mutex_lock(&my_context->mutex_thread);
	khint_t k = kh_get(mapcond, mapcond, *(uintptr_t*)cond);
	if(k!=kh_end(mapcond)) {
		box_free(kh_value(mapcond, k));
		kh_del(mapcond, mapcond, k);
	}
	mutex_unlock(&my_context->mutex_thread);
}

typedef struct __attribute__((packed, aligned(4))) pthread_cond_2_0_s {
	ptr_t cond;	// pthread_cond_t*
} pthread_cond_2_0_t;

static pthread_cond_t* get_cond_old(pthread_cond_2_0_t* cond) {
	if(!cond->cond) {
		ptr_t newcond = to_ptrv(box_calloc(1, sizeof(pthread_cond_t)));
		#ifdef DYNAREC
		if(native_lock_storeifnull_d(&cond->cond, newcond))
			box_free(from_ptrv(newcond));
		#else
		static pthread_mutex_t mutex_cond = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_lock(&mutex_cond);
		if(!cond->cond)
			cond->cond = newcond;
		else
			box_free(from_ptrv(newcond));
		#endif
	}
	return from_ptrv(cond->cond);
}

pthread_mutex_t* getAlignedMutex(pthread_mutex_t* m);

EXPORT int my32_pthread_cond_broadcast(x64emu_t* emu, void* cond)
{
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_broadcast(c);
}
EXPORT int my32_pthread_cond_broadcast_old(x64emu_t* emu, pthread_cond_2_0_t* cond)
{
	pthread_cond_t * c = get_cond_old(cond);
	return pthread_cond_broadcast(c);
}

EXPORT int my32_pthread_cond_destroy(x64emu_t* emu, void* cond)
{
	pthread_cond_t * c = get_cond(cond);
	int ret = pthread_cond_destroy(c);
	if(c!=cond) del_cond(cond);
	return ret;
}
EXPORT int my32_pthread_cond_destroy_old(x64emu_t* emu, pthread_cond_2_0_t* cond)
{
	pthread_cond_t * c = get_cond_old(cond);
	int ret = pthread_cond_destroy(c);
	box_free(from_ptrv(cond->cond));
	return ret;
}

EXPORT int my32_pthread_cond_init(x64emu_t* emu, void* cond, void* attr)
{
	pthread_cond_t *c = add_cond(cond);
	return pthread_cond_init(c, (const pthread_condattr_t*)attr);
}
EXPORT int my32_pthread_cond_init_old(x64emu_t* emu, void* cond, pthread_cond_2_0_t* attr)
{
	pthread_cond_t *c = get_cond_old(cond);
	return pthread_cond_init(c, (const pthread_condattr_t*)attr);
}

EXPORT int my32_pthread_cond_signal(x64emu_t* emu, void* cond)
{
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_signal(c);
}
EXPORT int my32_pthread_cond_signal_old(x64emu_t* emu, pthread_cond_2_0_t* cond)
{
	pthread_cond_t * c = get_cond_old(cond);
	return pthread_cond_signal(c);
}

EXPORT int my32_pthread_cond_timedwait_old(x64emu_t* emu, pthread_cond_2_0_t* cond, void* mutex, void* abstime)
{
	pthread_mutex_t* m = getAlignedMutex((pthread_mutex_t*)mutex);
	pthread_cond_t * c = get_cond_old(cond);
	struct timespec* atime = abstime;
	while(atime->tv_nsec>1000000000LL) {
		atime->tv_nsec-=1000000000LL;
		++atime->tv_sec;
	}
	return pthread_cond_timedwait(c, m, atime);
}
EXPORT int my32_pthread_cond_wait_old(x64emu_t* emu, pthread_cond_2_0_t* cond, void* mutex)
{
	pthread_mutex_t* m = getAlignedMutex((pthread_mutex_t*)mutex);
	pthread_cond_t * c = get_cond_old(cond);
	return pthread_cond_wait(c, m);
}

EXPORT int my32_pthread_cond_timedwait(x64emu_t* emu, void* cond, void* mutex, void* abstime)
{
	pthread_mutex_t* m = getAlignedMutex((pthread_mutex_t*)mutex);
	pthread_cond_t * c = get_cond(cond);
	struct timespec* atime = abstime;
	while(atime->tv_nsec>1000000000LL) {
		atime->tv_nsec-=1000000000LL;
		++atime->tv_sec;
	}
	return pthread_cond_timedwait(c, m, atime);
}
EXPORT int my32_pthread_cond_wait(x64emu_t* emu, void* cond, void* mutex)
{
	pthread_mutex_t* m = getAlignedMutex((pthread_mutex_t*)mutex);
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_wait(c, m);
}

EXPORT int my32_pthread_mutexattr_setkind_np(x64emu_t* emu, void* t, int kind)
{
    // does "kind" needs some type of translation?
    return pthread_mutexattr_settype(t, kind);
}

// pthread_attr_t on x86 is 36 bytes
static uint64_t ATTR_SIGN = 0xA055E10CDE98LL;	// random signature
typedef struct my32_x86_attr_s {
	uint64_t		sign;
	pthread_attr_t*	attr;
} my32_x86_attr_t;

static pthread_attr_t* get_attr(void* attr)
{
	if(!attr)
		return NULL;
	my32_x86_attr_t* my32_attr = (my32_x86_attr_t*)attr;
	if(my32_attr->sign!=ATTR_SIGN) {
		my32_attr->attr = (pthread_attr_t*)box_calloc(1, sizeof(pthread_attr_t));
		my32_attr->sign = ATTR_SIGN;
	}
	return my32_attr->attr;
}
static void del_attr(void* attr)
{
	if(!attr)
		return;
	my32_x86_attr_t* my32_attr = (my32_x86_attr_t*)attr;
	if(my32_attr->sign==ATTR_SIGN) {
		my32_attr->sign = 0;
		box_free(my32_attr->attr);
	}
}

EXPORT int my32_pthread_attr_init(x64emu_t* emu, void* attr)
{
	my32_x86_attr_t* my32_attr = (my32_x86_attr_t*)attr;
	my32_attr->sign = 0;	//force init
	return pthread_attr_init(get_attr(attr));
}

EXPORT int my32_pthread_getattr_np(x64emu_t* emu, uintptr_t th, void* attr)
{
	(void)emu;
	int ret = pthread_getattr_np(th, get_attr(attr));
	if(!ret && th==pthread_self()) {
		if(!emu->context->stacksizes) {
			emu->context->stacksizes = kh_init(threadstack);
		}
		void* stack = emu->init_stack;
		size_t sz = emu->size_stack;
//printf_log(LOG_INFO, "pthread_getattr_np called for self, stack=%p, sz=%lx\n", stack, sz);
		if (!sz) {
			// get default stack size
			pthread_attr_t attr;
			pthread_getattr_default_np(&attr);
			pthread_attr_getstacksize(&attr, &sz);
			pthread_attr_destroy(&attr);
			// should stack be adjusted?
		}
		AddStackSize((uintptr_t)attr, stack, sz);
	}
	return ret;
}

EXPORT int my32_pthread_attr_getdetachstate(x64emu_t* emu, void* attr, void* p)
{
	return pthread_attr_getdetachstate(get_attr(attr), p);
}
EXPORT int my32_pthread_attr_getguardsize(x64emu_t* emu, void* attr, void* p)
{
	return pthread_attr_getguardsize(get_attr(attr), p);
}
EXPORT int my32_pthread_attr_getinheritsched(x64emu_t* emu, void* attr, void* p)
{
	return pthread_attr_getinheritsched(get_attr(attr), p);
}
EXPORT int my32_pthread_attr_getschedparam(x64emu_t* emu, void* attr, void* p)
{
	return pthread_attr_getschedparam(get_attr(attr), p);
}
EXPORT int my32_pthread_attr_getschedpolicy(x64emu_t* emu, void* attr, void* p)
{
	return pthread_attr_getschedpolicy(get_attr(attr), p);
}
EXPORT int my32_pthread_attr_getscope(x64emu_t* emu, void* attr, void* p)
{
	return pthread_attr_getscope(get_attr(attr), p);
}
EXPORT int my32_pthread_attr_getstackaddr(x64emu_t* emu, void* attr, ptr_t* p)
{
	size_t size;
	void* pp;
	int ret = pthread_attr_getstack(get_attr(attr), &pp, &size);
	*p = to_ptrv(pp);
	return ret;
}
EXPORT int my32_pthread_attr_getstacksize(x64emu_t* emu, void* attr, ulong_t* p)
{
	size_t size;
	void* pp;
	int ret = pthread_attr_getstack(get_attr(attr), &pp, &size);
	*p = to_ulong(size);
	return ret;
}
EXPORT int my32_pthread_attr_setdetachstate(x64emu_t* emu, void* attr, int p)
{
	return pthread_attr_setdetachstate(get_attr(attr), p);
}
EXPORT int my32_pthread_attr_setguardsize(x64emu_t* emu, void* attr, size_t p)
{
	return pthread_attr_setguardsize(get_attr(attr), p);
}
EXPORT int my32_pthread_attr_setinheritsched(x64emu_t* emu, void* attr, int p)
{
	return pthread_attr_setinheritsched(get_attr(attr), p);
}
EXPORT int my32_pthread_attr_setschedparam(x64emu_t* emu, void* attr, void* param)
{
    int policy;
    pthread_attr_getschedpolicy(get_attr(attr), &policy);
    int pmin = sched_get_priority_min(policy);
    int pmax = sched_get_priority_max(policy);
    if(param) {
        int p = *(int*)param;
        if(p>=pmin && p<=pmax)
            return pthread_attr_setschedparam(get_attr(attr), param);
    }
    printf_log(LOG_INFO, "Warning, call to pthread_attr_setschedparam(%p, %p[%d]) ignored\n", attr, param, param?(*(int*)param):-1);
    return 0;   // faking success
}
EXPORT int my32_pthread_attr_setschedpolicy(x64emu_t* emu, void* attr, int p)
{
	return pthread_attr_setschedpolicy(get_attr(attr), p);
}
EXPORT int my32_pthread_attr_setstackaddr(x64emu_t* emu, void* attr, void* p)
{
	size_t size = 2*1024*1024;
	void* pp;
	GetStackSize((uintptr_t)attr, &pp, &size);
	AddStackSize((uintptr_t)attr, p, size);

	return 0;
}
EXPORT int my32_pthread_attr_setstacksize(x64emu_t* emu, void* attr, size_t p)
{
	if(p<0xc000 || (p&4095)) {
		errno = EINVAL;
		return -1;
	}
	size_t size;
	void* pp = NULL;
	GetStackSize((uintptr_t)attr, &pp, &size);
	AddStackSize((uintptr_t)attr, pp, p);
	// PTHREAD_STACK_MIN on x86 might be lower than the current platform...
	if(p>=0xc000 && p<PTHREAD_STACK_MIN && !(p&4095))
		p = PTHREAD_STACK_MIN;
	return pthread_attr_setstacksize(get_attr(attr), p);
}


EXPORT int my32_pthread_attr_setscope(x64emu_t* emu, void* attr, int scope)
{
    if(scope!=PTHREAD_SCOPE_SYSTEM) printf_log(LOG_INFO, "Warning, scope of call to pthread_attr_setscope(...) changed from %d to PTHREAD_SCOPE_SYSTEM\n", scope);
	return pthread_attr_setscope(get_attr(attr), PTHREAD_SCOPE_SYSTEM);
    //The scope is either PTHREAD_SCOPE_SYSTEM or PTHREAD_SCOPE_PROCESS
    // but PTHREAD_SCOPE_PROCESS doesn't seem supported on ARM linux, and PTHREAD_SCOPE_SYSTEM is default
}

#ifndef ANDROID
EXPORT void my32__pthread_cleanup_push_defer(x64emu_t* emu, void* buffer, void* routine, void* arg)
{
	real_pthread_cleanup_push_defer(buffer, findcleanup_routineFct(routine), arg);
}

EXPORT void my32__pthread_cleanup_push(x64emu_t* emu, void* buffer, void* routine, void* arg)
{
	_pthread_cleanup_push(buffer, findcleanup_routineFct(routine), arg);
}

EXPORT void my32__pthread_cleanup_pop_restore(x64emu_t* emu, void* buffer, int exec)
{
	real_pthread_cleanup_pop_restore(buffer, exec);
}

EXPORT void my32__pthread_cleanup_pop(x64emu_t* emu, void* buffer, int exec)
{
	_pthread_cleanup_pop(buffer, exec);
}

// getaffinity_np (pthread or attr) hav an "old" version (glibc-2.3.3) that only have 2 args, cpusetsize is omited
EXPORT int my32_pthread_getaffinity_np(x64emu_t* emu, pthread_t thread, int cpusetsize, void* cpuset)
{
	if(cpusetsize>0x1000) {
		// probably old version of the function, that didn't have cpusetsize....
		cpuset = from_ptrv(cpusetsize);
		cpusetsize = sizeof(cpu_set_t);
	} 

	int ret = pthread_getaffinity_np(thread, cpusetsize, cpuset);
	if(ret<0) {
		printf_log(LOG_INFO, "Warning, pthread_getaffinity_np(%p, %d, %p) errored, with errno=%d\n", (void*)thread, cpusetsize, cpuset, errno);
	}

    return ret;
}

EXPORT int my32_pthread_setaffinity_np(x64emu_t* emu, pthread_t thread, int cpusetsize, void* cpuset)
{
	if(cpusetsize>0x1000) {
		// probably old version of the function, that didn't have cpusetsize....
		cpuset = from_ptrv(cpusetsize);
		cpusetsize = sizeof(cpu_set_t);
	} 

	int ret = pthread_setaffinity_np(thread, cpusetsize, cpuset);
	if(ret<0) {
		printf_log(LOG_INFO, "Warning, pthread_setaffinity_np(%p, %d, %p) errored, with errno=%d\n", (void*)thread, cpusetsize, cpuset, errno);
	}

    return ret;
}

EXPORT int my32_pthread_attr_setaffinity_np(x64emu_t* emu, void* attr, uint32_t cpusetsize, void* cpuset)
{
	if(cpusetsize>0x1000) {
		// probably old version of the function, that didn't have cpusetsize....
		cpuset = from_ptrv(cpusetsize);
		cpusetsize = sizeof(cpu_set_t);
	} 

	int ret = pthread_attr_setaffinity_np(attr, cpusetsize, cpuset);
	if(ret<0) {
		printf_log(LOG_INFO, "Warning, pthread_attr_setaffinity_np(%p, %d, %p) errored, with errno=%d\n", attr, cpusetsize, cpuset, errno);
	}

    return ret;
}
#endif

EXPORT int my32_pthread_kill(x64emu_t* emu, void* thread, int sig)
{
	sig = signal_from_x64(sig);
	// should ESCHR result be filtered, as this is expected to be the 2.34 behaviour?
	(void)emu;
	// check for old "is everything ok?"
	if(thread==NULL && sig==0)
		return pthread_kill(pthread_self(), 0);
	#ifdef BAD_PKILL
	if(sig==0 && thread!=(void*)pthread_self())
		return get_thread(thread)?0:ESRCH;
	#endif
	return pthread_kill((pthread_t)thread, sig);
}

EXPORT int my32_pthread_kill_old(x64emu_t* emu, void* thread, int sig)
{
	sig = signal_from_x64(sig);
    // check for old "is everything ok?"
    if((thread==NULL) && (sig==0))
        return real_phtread_kill_old(pthread_self(), 0);
	#ifdef BAD_PKILL
	if(sig==0 && thread!=(void*)pthread_self())
		return get_thread(thread)?0:ESRCH;
	#endif
    return real_phtread_kill_old((pthread_t)thread, sig);
}

//EXPORT void my32_pthread_exit(x64emu_t* emu, void* retval)
//{
//	emu->quit = 1;	// to be safe
//	pthread_exit(retval);
//}

// TODO: find a better way for mutex. It should be possible to use the actual mutex most of the time, especially for simple ones
// Having the mutex table behind a mutex is far from ideal!

#include "threads32.h"

pthread_mutex_t* createNewMutex()
{
	pthread_mutex_t* ret = (pthread_mutex_t*)box32_calloc(1, sizeof(pthread_mutex_t));
	return ret;
}
// init = 0: just get the mutex
// init = 1: get the mutex and init it with optione attr (attr will disallow native mutex)
pthread_mutex_t* getAlignedMutex(pthread_mutex_t* m)
{
	fake_phtread_mutex_t* fake = (fake_phtread_mutex_t*)m;
	if(!fake->__lock && !fake->__count && !fake->__owner && !fake->__kind && !fake->i386__kind && !fake->real_mutex) {
		printf_log(LOG_DEBUG, " (init t0) ", m);
		fake->real_mutex = KIND_SIGN;
	}
	if(!fake->__lock && !fake->__count && !fake->__owner && !fake->__kind && fake->i386__kind==1 && !fake->real_mutex) {
		// recusrive mutex should also work
		printf_log(LOG_DEBUG, " (init t1) ", m);
		fake->real_mutex = KIND_SIGN;
		fake->__kind = 1;
		fake->i386__kind = 0;
	}
	if(fake->real_mutex==KIND_SIGN) {
		printf_log(LOG_DEBUG, " (t0: m=%p, l=%d) ", m, fake->__lock);
		return m;	// type 0 can fit...
	}
	if(fake->__kind==KIND_SIGN) {
		printf_log(LOG_DEBUG, " (m=%p, l=%d) ", from_ptrv(fake->real_mutex), *(int*)from_ptrv(fake->real_mutex));
		return from_ptrv(fake->real_mutex);
	}
	// this should not appens!
	printf_log(LOG_INFO, "BOX32: Warning, fallback on aligned mutex %p\n", m);
	fake->real_mutex = to_ptrv(createNewMutex());
	pthread_mutexattr_t attr = {0};
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, fake->i386__kind);
	pthread_mutex_init(from_ptrv(fake->real_mutex), &attr);
	pthread_mutexattr_destroy(&attr);
	fake->__kind==KIND_SIGN;
	printf_log(LOG_DEBUG, " (fb: m=%p) ", from_ptrv(fake->real_mutex));
	return from_ptrv(fake->real_mutex);
}
EXPORT int my32_pthread_mutex_destroy(pthread_mutex_t *m)
{
	fake_phtread_mutex_t* fake = (fake_phtread_mutex_t*)m;
	if(fake->real_mutex==KIND_SIGN) {
		//TODO: check if that save/restore is actually needed
		uint8_t saved[sizeof(pthread_mutex_t)];
		memcpy(saved, fake+1, sizeof(pthread_mutex_t)-24);
		int ret = pthread_mutex_destroy(m);
		memcpy(fake+1, saved, sizeof(pthread_mutex_t)-24);
		return ret;
	}
	if(fake->__kind!=KIND_SIGN) {
		printf_log(LOG_NONE, "BOX32: Warning, fallback on alligned mutex %p\n", m);
		errno = EINVAL;
		return -1;
	}
	pthread_mutex_t *n = from_ptrv(fake->real_mutex);
	int ret = pthread_mutex_destroy(n);
	box32_free(n);
	fake->__kind = fake->i386__kind = -1;
	return ret;
}

EXPORT int my32_pthread_mutexattr_init(x64emu_t* emu, pthread_mutexattr_t* att)
{
	// mutexattr is 4 bytes on x86, but 8 on 64bits platforms...
	uint32_t save = att?(((uint32_t*)att)[1]):0;
	int ret = pthread_mutexattr_init(att);
	if(att) ((uint32_t*)att)[1] = save;
	return ret;
}

EXPORT int my32___pthread_mutex_destroy(pthread_mutex_t *m) __attribute__((alias("my32_pthread_mutex_destroy")));

EXPORT int my32_pthread_mutex_init(pthread_mutex_t *m, pthread_mutexattr_t *att)
{
	fake_phtread_mutex_t* fake = (fake_phtread_mutex_t*)m;
	/*if(!att) {
		fake->__lock = 0;
		fake->__count = 0;
		fake->i386__kind = 0;
		fake->__owner = 0;
		fake->real_mutex = KIND_SIGN;
		printf_log(LOG_DEBUG, " (init t0) ");
		return 0;
	}*/
	if(fake->__kind==KIND_SIGN) {
		printf_log(LOG_DEBUG, "(reinit %p) ",from_ptrv(fake->real_mutex));
		return pthread_mutex_init(from_ptrv(fake->real_mutex), att);
	}
	fake->__lock = 0;
	fake->__count = 0;
	fake->__kind = KIND_SIGN;
	fake->real_mutex = to_ptrv(createNewMutex());
	int ret = pthread_mutex_init(from_ptrv(fake->real_mutex), att);
	fake->i386__kind = ((struct __pthread_mutex_s*)from_ptrv(fake->real_mutex))->__kind;
	printf_log(LOG_DEBUG, "(init t%d %p) ", fake->i386__kind, from_ptrv(fake->real_mutex));
	return ret;
}
EXPORT int my32___pthread_mutex_init(pthread_mutex_t *m, pthread_mutexattr_t *att) __attribute__((alias("my32_pthread_mutex_init")));

EXPORT int my32_pthread_mutex_lock(pthread_mutex_t *m)
{
	return pthread_mutex_lock(getAlignedMutex(m));
}
EXPORT int my32___pthread_mutex_lock(pthread_mutex_t *m) __attribute__((alias("my32_pthread_mutex_lock")));

EXPORT int my32_pthread_mutex_timedlock(pthread_mutex_t *m, const struct timespec * t)
{
	return pthread_mutex_timedlock(getAlignedMutex(m), t);
}

EXPORT int my32_pthread_mutex_trylock(pthread_mutex_t *m)
{
	return pthread_mutex_trylock(getAlignedMutex(m));
}
EXPORT int my32___pthread_mutex_trylock(pthread_mutex_t *m) __attribute__((alias("my32_pthread_mutex_trylock")));

EXPORT int my32_pthread_mutex_unlock(pthread_mutex_t *m)
{
	return pthread_mutex_unlock(getAlignedMutex(m));
}
EXPORT int my32___pthread_mutex_unlock(pthread_mutex_t *m) __attribute__((alias("my32_pthread_mutex_unlock")));

static int done = 0;
void init_pthread_helper_32()
{
	if(done)
		return;
	done = 1;
	real_pthread_cleanup_push_defer = (vFppp_t)dlsym(NULL, "_pthread_cleanup_push_defer");
	real_pthread_cleanup_pop_restore = (vFpi_t)dlsym(NULL, "_pthread_cleanup_pop_restore");

	// search for older symbol for pthread_kill
	{
		char buff[50];
		for(int i=0; i<34 && !real_phtread_kill_old; ++i) {
			snprintf(buff, 50, "GLIBC_2.%d", i);
			real_phtread_kill_old = (iFLi_t)dlvsym(NULL, "pthread_kill", buff);
		}
	}
	if(!real_phtread_kill_old) {
		printf_log(LOG_INFO, "Warning, older than 2.34 pthread_kill not found, using current one\n");
		real_phtread_kill_old = (iFLi_t)pthread_kill;
	}

	mapcond = kh_init(mapcond);
}

void clean_current_emuthread_32()
{
	emuthread_t *et = (emuthread_t*)thread_get_et();
	if(et) {
		emuthread_destroy(et);
		thread_set_et(NULL);
	}
}

void fini_pthread_helper_32(box64context_t* context)
{
	if(!done)
		return;
	done = 0;
	//CleanStackSize(context);
	pthread_cond_t *cond;
	kh_foreach_value(mapcond, cond, 
		pthread_cond_destroy(cond);
		box_free(cond);
	);
	kh_destroy(mapcond, mapcond);
	mapcond = NULL;

	clean_current_emuthread_32();
}
