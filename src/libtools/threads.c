// __USE_UNIX98 is needed for sttype / gettype definition
#define __USE_UNIX98
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <setjmp.h>

#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "threads.h"
#include "emu/x64emu_private.h"
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
#include "myalign.h"
#include "x64tls.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "dynarec/native_lock.h"
#endif
#ifdef BOX32
#include "box32.h"
#include "threads32.h"
#endif

//void _pthread_cleanup_push_defer(void* buffer, void* routine, void* arg);	// declare hidden functions
//void _pthread_cleanup_pop_restore(void* buffer, int exec);
typedef void (*vFppp_t)(void*, void*, void*);
typedef void (*vFpi_t)(void*, int);
typedef int (*iFppip_t)(void*, void*, int, void*);
typedef int (*iFli_t)(long unsigned int, int);
typedef struct emuthread_s emuthread_t;

static vFppp_t real_pthread_cleanup_push_defer = NULL;
static vFpi_t real_pthread_cleanup_pop_restore = NULL;
static iFppip_t real_pthread_cond_clockwait = NULL;
void _pthread_cleanup_push(void* buffer, void* routine, void* arg);	// declare hidden functions
void _pthread_cleanup_pop(void* buffer, int exec);
// with glibc 2.34+, pthread_kill changed behaviour and might break some program, so using old version if possible
// it will be pthread_kill@GLIBC_2.17 on aarch64, but it's GLIBC_2.2.5 on x86_64
static iFli_t real_phtread_kill_old = NULL;

typedef struct threadstack_s {
	void* 	stack;
	size_t 	stacksize;
} threadstack_t;

typedef struct x64_unwind_buff_s {
	struct {
		jump_buff_x64_t		__cancel_jmp_buf;
		int					__mask_was_saved;
	} __cancel_jmp_buf[1];
	void *__pad[4];
} x64_unwind_buff_t __attribute__((__aligned__));

typedef void(*vFv_t)();

KHASH_MAP_INIT_INT64(threadstack, threadstack_t*)

void CleanStackSize(box64context_t* context)
{
	threadstack_t *ts;
	if(!context || !context->stacksizes)
		return;
	mutex_lock(&context->mutex_thread);
	kh_foreach_value(context->stacksizes, ts, box_free(ts));
	kh_destroy(threadstack, context->stacksizes);
	context->stacksizes = NULL;
	mutex_unlock(&context->mutex_thread);
}

void FreeStackSize(uintptr_t attr)
{
	kh_threadstack_t* map = my_context->stacksizes;
	if(!map) return;
	mutex_lock(&my_context->mutex_thread);
	khint_t k = kh_get(threadstack, map, attr);
	if(k!=kh_end(map)) {
		box_free(kh_value(map, k));
		kh_del(threadstack, map, k);
	}
	mutex_unlock(&my_context->mutex_thread);
}

void AddStackSize(uintptr_t attr, void* stack, size_t stacksize)
{
	if(!my_context->stacksizes)
		my_context->stacksizes = kh_init(threadstack);
	kh_threadstack_t* map = my_context->stacksizes;
	khint_t k;
	int ret;
	mutex_lock(&my_context->mutex_thread);
	k = kh_put(threadstack, map, attr, &ret);
	if(ret) kh_value(map, k) = (threadstack_t*)box_calloc(1, sizeof(threadstack_t));
	threadstack_t* ts = kh_value(map, k);
	ts->stack = stack;
	ts->stacksize = stacksize;
	mutex_unlock(&my_context->mutex_thread);
}

// return stack from attr (or from current emu if attr is not found..., wich is wrong but approximate enough?)
int GetStackSize(uintptr_t attr, void** stack, size_t* stacksize)
{
	if(my_context->stacksizes && attr) {
		mutex_lock(&my_context->mutex_thread);
		khint_t k = kh_get(threadstack, my_context->stacksizes, attr);
		if(k!=kh_end(my_context->stacksizes)) {
			threadstack_t* ts = kh_value(my_context->stacksizes, k);
			*stack = ts->stack;
			*stacksize = ts->stacksize;
			mutex_unlock(&my_context->mutex_thread);
			return 1;
		}
		mutex_unlock(&my_context->mutex_thread);
	}
	return 0;
}

void my_longjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val);
#ifdef BOX32
void my32_longjmp(x64emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val);
#endif

#ifdef BAD_PKILL
// tracking of threads that are alive
KHASH_MAP_INIT_INT64(threads, emuthread_t*)
static pthread_mutex_t threads_mutex = PTHREAD_MUTEX_INITIALIZER;
static kh_threads_t* threads_alive = NULL;

void add_thread(void* t, emuthread_t* et)
{
	pthread_mutex_lock(&threads_mutex);
	if(!threads_alive)
		threads_alive = kh_init(threads);
	khint_t k;
	int ret;
	k = kh_put(threads, threads_alive, (uintptr_t)t, &ret);
	kh_value(threads_alive, k) = et;
	pthread_mutex_unlock(&threads_mutex);
}
void del_thread(void* t)
{
	pthread_mutex_lock(&threads_mutex);
	if(threads_alive) {
		khint_t k;
		k = kh_get(threads, threads_alive, (uintptr_t)t);
		if(k!=kh_end(threads_alive))
			kh_del(threads, threads_alive, k);
	}
	pthread_mutex_unlock(&threads_mutex);
}
emuthread_t* get_thread(void* t)
{
	emuthread_t* ret = NULL;
	pthread_mutex_lock(&threads_mutex);
	if(threads_alive) {
		khint_t k;
		k = kh_get(threads, threads_alive, (uintptr_t)t);
		if(k!=kh_end(threads_alive))
			ret = kh_value(threads_alive, k);
	}
	pthread_mutex_unlock(&threads_mutex);
	return ret;
}
#endif

static pthread_key_t thread_key;

void emuthread_destroy(void* p)
{
	emuthread_t *et = (emuthread_t*)p;
	if(!et)
		return;
	#ifdef BOX32
	if(et->is32bits && !et->join && et->fnc)
		to_hash_d(et->self);
	#endif
	FreeX64Emu(&et->emu);
	#ifdef BAD_PKILL
	del_thread((void*)et->self);
	#endif
	box_free(et);
}

static void emuthread_cancel(void* p)
{
	emuthread_t *et = (emuthread_t*)p;
	if(!et)
		return;
	// check cancels threads
	for(int i=et->cancel_size-1; i>=0; --i) {
		et->emu->flags.quitonlongjmp = 0;
		et->emu->quit = 0;
		#ifdef BOX32
		if(et->is32bits)
			my32_longjmp(et->emu, ((i386_unwind_buff_t*)et->cancels[i])->__cancel_jmp_buf, 1);
		else
		#endif
			my_longjmp(et->emu, ((x64_unwind_buff_t*)et->cancels[i])->__cancel_jmp_buf, 1);
		DynaRun(et->emu);	// will return after a __pthread_unwind_next()
	}
	#ifdef BOX32
	/*if(box64_is32bits)
		to_hash_d(et->self);*/ // not removing hash for old pthread_t
	#endif
	box_free(et->cancels);
	et->cancels=NULL;
	et->cancel_size = et->cancel_cap = 0;
}
void thread_forget_emu()
{
	pthread_setspecific(thread_key, NULL);
}
void thread_set_emu(x64emu_t* emu)
{
	emuthread_t *et = (emuthread_t*)pthread_getspecific(thread_key);
	if(!emu) {
		if(et) emuthread_destroy(et);
		pthread_setspecific(thread_key, NULL);
		return;
	}
	if(!et) {
		et = (emuthread_t*)box_calloc(1, sizeof(emuthread_t));
	} else {
		if(et->emu != emu)
			FreeX64Emu(&et->emu);
	}
	et->emu = emu;
	et->emu->type = EMUTYPE_MAIN;
	#ifdef BOX32
	if(box64_is32bits) {
		et->is32bits = 1;
		et->self = (uintptr_t)pthread_self();
		et->hself = to_hash(et->self);
	}
	#endif
	#ifdef BAD_PKILL
	if(!et->self)
		et->self = (uintptr_t)pthread_self();
	add_thread((void*)et->self, et);
	#endif
	pthread_setspecific(thread_key, et);
}

x64emu_t* thread_get_emu()
{
	emuthread_t *et = (emuthread_t*)pthread_getspecific(thread_key);
	if(!et) {
		// this should not happens. So if it happens, use a small stack
		int stacksize = 256*1024;
		void* stack = NULL;
		if(box64_is32bits)
			stack = mmap64(NULL, stacksize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_32BIT, -1, 0);
		else
            stack = InternalMmap(NULL, stacksize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
		if(stack!=MAP_FAILED)
			setProtection_stack((uintptr_t)stack, stacksize, PROT_READ|PROT_WRITE);
		x64emu_t *emu = NewX64Emu(my_context, my_context->exit_bridge, (uintptr_t)stack, stacksize, 1);
		SetupX64Emu(emu, NULL);
		refreshTLSData(emu);
		thread_set_emu(emu);
		return emu;
	}
	return et->emu;
}

emuthread_t* thread_get_et()
{
	return (emuthread_t*)pthread_getspecific(thread_key);
}

void thread_set_et(emuthread_t* et)
{
	#ifdef BAD_PKILL
	if(et && !et->self)
		et->self = (uintptr_t)pthread_self();
	add_thread((void*)(et?et->self:pthread_self()), et);
	#endif
	pthread_setspecific(thread_key, et);
}

static void* pthread_routine(void* p)
{
	// free current emuthread if it exist
	{
		void* t = pthread_getspecific(thread_key);
		if(t) {
			// not sure how this could happens
			printf_log(LOG_INFO, "Clean of an existing ET for Thread %04d\n", GetTID());
			emuthread_destroy(t);
		}
	}
	pthread_setspecific(thread_key, p);
	// call the function
	emuthread_t *et = (emuthread_t*)p;
	et->emu->type = EMUTYPE_MAIN;
	// setup callstack and run...
	x64emu_t* emu = et->emu;
	#ifdef BAD_PKILL
	if(!et->self)
		et->self = (uintptr_t)pthread_self();
	add_thread((void*)et->self, et);
	#endif
	refreshTLSData(emu);
	Push64(emu, 0);	// PUSH 0 (backtrace marker: return address is 0)
	Push64(emu, 0);	// PUSH BP
	R_RBP = R_RSP;	// MOV BP, SP
	R_RSP -= 64;	// Guard zone
	R_RSP &= ~15LL;
	PushExit(emu);
	R_RIP = et->fnc;
	R_RDI = (uintptr_t)et->arg;
	pthread_cleanup_push(emuthread_cancel, p);
	DynaRun(emu);
	pthread_cleanup_pop(0);
	void* ret = (void*)R_RAX;
	//void* ret = (void*)RunFunctionWithEmu(et->emu, 0, et->fnc, 1, et->arg);
	return ret;
}

#ifdef NOALIGN
#define PTHREAD_ATTR_ALIGN(A)
#define PTHREAD_ATTR_UNALIGN(A)
#define PTHREAD_ATTR(A) 	A
#else
#define PTHREAD_ATTR_ALIGN(A) pthread_attr_t aligned_attr = {0}; if(A) memcpy(&aligned_attr, A, 56)
#define PTHREAD_ATTR_UNALIGN(A) if(A) memcpy(A, &aligned_attr, 56)
#define PTHREAD_ATTR(A)		(A)?&aligned_attr:NULL
#endif

EXPORT int my_pthread_attr_destroy(x64emu_t* emu, void* attr)
{
	if(emu->context->stacksizes)
		FreeStackSize((uintptr_t)attr);
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_attr_destroy(PTHREAD_ATTR(attr));
	// no unaligned, it's destroyed
	return ret;
}

EXPORT int my_pthread_attr_getstack(x64emu_t* emu, void* attr, void** stackaddr, size_t* stacksize)
{
	PTHREAD_ATTR_ALIGN(attr);
	int ret = 0;
	if(!GetStackSize((uintptr_t)attr, stackaddr, stacksize))
		ret = pthread_attr_getstack(PTHREAD_ATTR(attr), stackaddr, stacksize);
//printf_log(LOG_INFO, "pthread_attr_getstack gives (%d) %p 0x%zx\n", ret, *stackaddr, *stacksize);
	return ret;
}

EXPORT int my_pthread_attr_setstack(x64emu_t* emu, void* attr, void* stackaddr, size_t stacksize)
{
	AddStackSize((uintptr_t)attr, stackaddr, stacksize);
	//Don't call actual setstack...
	//return pthread_attr_setstack(attr, stackaddr, stacksize);
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_attr_setstacksize(PTHREAD_ATTR(attr), stacksize);
	PTHREAD_ATTR_UNALIGN(attr);
	return ret;
}

EXPORT int my_pthread_attr_setstacksize(x64emu_t* emu, void* attr, size_t stacksize)
{
	(void)emu;
	//aarch64 have an PTHREAD_STACK_MIN of 131072 instead of 16384 on x86_64!
	if(stacksize<(size_t)PTHREAD_STACK_MIN)
		stacksize = PTHREAD_STACK_MIN;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_attr_setstacksize(PTHREAD_ATTR(attr), stacksize);
	PTHREAD_ATTR_UNALIGN(attr);
	return ret;
}

#ifndef NOALIGN
EXPORT int my_pthread_attr_getdetachstate(x64emu_t* emu, pthread_attr_t* attr, int *state)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	return pthread_attr_getdetachstate(PTHREAD_ATTR(attr), state);
}
EXPORT int my_pthread_attr_getguardsize(x64emu_t* emu, pthread_attr_t* attr, size_t* size)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	return pthread_attr_getguardsize(PTHREAD_ATTR(attr), size);
}
EXPORT int my_pthread_attr_getinheritsched(x64emu_t* emu, pthread_attr_t* attr, int* sched)
{
#ifndef TERMUX
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	return pthread_attr_getinheritsched(PTHREAD_ATTR(attr), sched);
#else
	(void)emu; (void)attr; (void)sched;
	return 0;
#endif
}
EXPORT int my_pthread_attr_getschedparam(x64emu_t* emu, pthread_attr_t* attr, void* param)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	return pthread_attr_getschedparam(PTHREAD_ATTR(attr), param);
}
EXPORT int my_pthread_attr_getschedpolicy(x64emu_t* emu, pthread_attr_t* attr, int* policy)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	return pthread_attr_getschedpolicy(PTHREAD_ATTR(attr), policy);
}
EXPORT int my_pthread_attr_getscope(x64emu_t* emu, pthread_attr_t* attr, int* scope)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	return pthread_attr_getscope(PTHREAD_ATTR(attr), scope);
}
EXPORT int my_pthread_attr_getstackaddr(x64emu_t* emu, pthread_attr_t* attr, void** addr)
{
	(void)emu;
	size_t size;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = 0;
	if(!GetStackSize((uintptr_t)attr, addr, &size ))
		ret = pthread_attr_getstack(PTHREAD_ATTR(attr), addr, &size);
//printf_log(LOG_INFO, "pthread_attr_getstackaddr gives %p\n", *addr);
	return ret;
}
EXPORT int my_pthread_attr_getstacksize(x64emu_t* emu, pthread_attr_t* attr, size_t* size)
{
	(void)emu;
	void* addr;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = 0;
	if(!GetStackSize((uintptr_t)attr, &addr, size ))
		ret = pthread_attr_getstack(PTHREAD_ATTR(attr), &addr, size);
	if(!*size)
		*size = 2*1024*1024;
	//return pthread_attr_getstacksize(getAlignedAttr(attr), size);
	return ret;
}
EXPORT int my_pthread_attr_init(x64emu_t* emu, pthread_attr_t* attr)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_attr_init(PTHREAD_ATTR(attr));
	PTHREAD_ATTR_UNALIGN(attr);
	return ret;
}
#ifndef ANDROID
EXPORT int my_pthread_attr_setaffinity_np(x64emu_t* emu, pthread_attr_t* attr, size_t cpusize, void* cpuset)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_attr_setaffinity_np(PTHREAD_ATTR(attr), cpusize, cpuset);
	PTHREAD_ATTR_UNALIGN(attr);
	return ret;
}
EXPORT int my_pthread_attr_setaffinity_np_old(x64emu_t* emu, pthread_attr_t* attr, void* cpuset)
{
	return my_pthread_attr_setaffinity_np(emu, attr, 128, cpuset);
}
#endif
EXPORT int my_pthread_attr_setdetachstate(x64emu_t* emu, pthread_attr_t* attr, int state)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_attr_setdetachstate(PTHREAD_ATTR(attr), state);
	PTHREAD_ATTR_UNALIGN(attr);
	return ret;
}
EXPORT int my_pthread_attr_setguardsize(x64emu_t* emu, pthread_attr_t* attr, size_t size)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_attr_setguardsize(PTHREAD_ATTR(attr), size);
	PTHREAD_ATTR_UNALIGN(attr);
	return ret;
}
EXPORT int my_pthread_attr_setinheritsched(x64emu_t* emu, pthread_attr_t* attr, int sched)
{
#ifndef TERMUX
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_attr_setinheritsched(PTHREAD_ATTR(attr), sched);
	PTHREAD_ATTR_UNALIGN(attr);
	return ret;
#else
	(void)emu; (void)attr; (void)sched;
	return 0;
#endif
}
EXPORT int my_pthread_attr_setschedparam(x64emu_t* emu, pthread_attr_t* attr, void* param)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_attr_setschedparam(PTHREAD_ATTR(attr), param);
	PTHREAD_ATTR_UNALIGN(attr);
	return ret;
}
EXPORT int my_pthread_attr_setschedpolicy(x64emu_t* emu, pthread_attr_t* attr, int policy)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_attr_setschedpolicy(PTHREAD_ATTR(attr), policy);
	PTHREAD_ATTR_UNALIGN(attr);
	return ret;
}
EXPORT int my_pthread_attr_setscope(x64emu_t* emu, pthread_attr_t* attr, int scope)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_attr_setscope(PTHREAD_ATTR(attr), scope);
	PTHREAD_ATTR_UNALIGN(attr);
	return ret;
}
EXPORT int my_pthread_attr_setstackaddr(x64emu_t* emu, pthread_attr_t* attr, void* addr)
{
	size_t size = 2*1024*1024;
	void* pp = NULL;
	GetStackSize((uintptr_t)attr, &pp, &size);
	AddStackSize((uintptr_t)attr, addr, size);
	return 0;
	//return pthread_attr_setstackaddr(getAlignedAttr(attr), addr);
}
#ifndef ANDROID
EXPORT int my_pthread_getattr_np(x64emu_t* emu, pthread_t thread_id, pthread_attr_t* attr)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_getattr_np(thread_id, PTHREAD_ATTR(attr));
	PTHREAD_ATTR_UNALIGN(attr);
	if(!ret && thread_id==pthread_self()) {
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
EXPORT int my_pthread_getattr_default_np(x64emu_t* emu, pthread_attr_t* attr)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_getattr_default_np(PTHREAD_ATTR(attr));
	PTHREAD_ATTR_UNALIGN(attr);
	return ret;
}
EXPORT int my_pthread_setattr_default_np(x64emu_t* emu, pthread_attr_t* attr)
{
	(void)emu;
	PTHREAD_ATTR_ALIGN(attr);
	int ret = pthread_setattr_default_np(PTHREAD_ATTR(attr));
	PTHREAD_ATTR_UNALIGN(attr);
	return ret;
}
#endif	//!ANDROID
#endif

EXPORT int my_pthread_create(x64emu_t *emu, void* t, void* attr, void* start_routine, void* arg)
{
	int stacksize = 2*1024*1024;	//default stack size is 2Mo
	void* attr_stack;
	size_t attr_stacksize;
	int own;
	void* stack;

	if(attr) {
		size_t stsize;
		PTHREAD_ATTR_ALIGN(attr);
		if(pthread_attr_getstacksize(PTHREAD_ATTR(attr), &stsize)==0)
			stacksize = stsize;
	}
	if(GetStackSize((uintptr_t)attr, &attr_stack, &attr_stacksize))
	{
		stack = attr_stack;
		stacksize = attr_stacksize;
		own = 0;
	} else {
        stack = InternalMmap(NULL, stacksize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
        if(stack!=MAP_FAILED)
	        setProtection_stack((uintptr_t)stack, stacksize, PROT_READ|PROT_WRITE);
		own = 1;
	}

	emuthread_t *et = (emuthread_t*)box_calloc(1, sizeof(emuthread_t));
    x64emu_t *emuthread = NewX64Emu(my_context, (uintptr_t)start_routine, (uintptr_t)stack, stacksize, own);
	SetupX64Emu(emuthread, emu);
	//SetFS(emuthread, GetFS(emu));
	et->emu = emuthread;
	et->fnc = (uintptr_t)start_routine;
	et->arg = arg;
	#ifdef DYNAREC
	if(BOX64ENV(dynarec)) {
		// pre-creation of the JIT code for the entry point of the thread
		DBGetBlock(emu, (uintptr_t)start_routine, 1, 0);	// function wrapping are 64bits only on box64
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
	PTHREAD_ATTR_ALIGN(attr);
	return pthread_create((pthread_t*)t, PTHREAD_ATTR(attr),
		pthread_routine, et);
	// no need too unalign for attr, it's const
}

void* my_prepare_thread(x64emu_t *emu, void* f, void* arg, int ssize, void** pet)
{
	int stacksize = (ssize)?ssize:(2*1024*1024);	//default stack size is 2Mo
    void* stack = InternalMmap(NULL, stacksize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
    if(stack!=MAP_FAILED)
		setProtection_stack((uintptr_t)stack, stacksize, PROT_READ|PROT_WRITE);
	emuthread_t *et = (emuthread_t*)box_calloc(1, sizeof(emuthread_t));
	x64emu_t *emuthread = NewX64Emu(emu->context, (uintptr_t)f, (uintptr_t)stack, stacksize, 1);
	SetupX64Emu(emuthread, emu					);
	//SetFS(emuthread, GetFS(emu));
	et->emu = emuthread;
	et->fnc = (uintptr_t)f;
	et->arg = arg;
	#ifdef DYNAREC
	if(BOX64ENV(dynarec)) {
		// pre-creation of the JIT code for the entry point of the thread
		DBGetBlock(emu, (uintptr_t)f, 1, 0);	// function wrapping are 64bits only on box64
	}
	#endif
	*pet =  et;
	return pthread_routine;
}

EXPORT void my___pthread_register_cancel(x64emu_t* emu, x64_unwind_buff_t* buff)
{
	emuthread_t *et = (emuthread_t*)pthread_getspecific(thread_key);
	if(et->cancel_cap == et->cancel_size) {
		et->cancel_cap+=8;
		et->cancels = box_realloc(et->cancels, sizeof(x64_unwind_buff_t*)*et->cancel_cap);
	}
	et->cancels[et->cancel_size++] = buff;
}

EXPORT void my___pthread_unregister_cancel(x64emu_t* emu, x64_unwind_buff_t* buff)
{
	emuthread_t *et = (emuthread_t*)pthread_getspecific(thread_key);
	for (int i=et->cancel_size-1; i>=0; --i) {
		if(et->cancels[i] == buff) {
			if(i!=et->cancel_size-1)
				memmove(et->cancels+i, et->cancels+i+1, sizeof(x64_unwind_buff_t*)*(et->cancel_size-i-1));
			et->cancel_size--;
		}
	}
}

EXPORT void my___pthread_unwind_next(x64emu_t* emu, x64_unwind_buff_t* buff)
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
static uintptr_t my_cleanup_routine_fct_##A = 0;  \
static void my_cleanup_routine_##A(void* a)    			\
{                                       		\
    RunFunction(my_cleanup_routine_fct_##A, 1, a);\
}
SUPER()
#undef GO
static void* findcleanup_routineFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cleanup_routine_fct_##A == (uintptr_t)fct) return my_cleanup_routine_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cleanup_routine_fct_##A == 0) {my_cleanup_routine_fct_##A = (uintptr_t)fct; return my_cleanup_routine_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pthread cleanup_routine callback\n");
    return NULL;
}

// key_dtor
#define GO(A)   \
static uintptr_t my_key_dtor_fct_##A = 0;  		\
static void my_key_dtor_##A(void* a)    		\
{                                       		\
    RunFunction(my_key_dtor_fct_##A, 1, a);		\
}
SUPER()
#undef GO
static void* findkey_dtorFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_key_dtor_fct_##A == (uintptr_t)fct) return my_key_dtor_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_key_dtor_fct_##A == 0) {my_key_dtor_fct_##A = (uintptr_t)fct; return my_key_dtor_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pthread key_dtor callback\n");
    return NULL;
}

#undef SUPER

// custom implementation of pthread_once...
int EXPORT my_pthread_once(x64emu_t* emu, int* once, void* cb)
{
	if(*once==1)	// quick test first
		return 0;
	if(__sync_bool_compare_and_swap(once, 0, 2)) {
		// can run the function
		// make some room and align R_RSP before doing the call (maybe it would be simpler to just use Callback functions)
		Push64(emu, R_RBP); // push rbp
		R_RBP = R_RSP;      // mov rbp, rsp
		R_RSP &= ~63LL;

		DynaCall(emu, (uintptr_t)cb);  // using DynaCall, speedup wine 7.21 initialisation

		R_RSP = R_RBP;          // mov rsp, rbp
		R_RBP = Pop64(emu);     // pop rbp
		*once = 1;
		__sync_synchronize();
	} else {
		// nope, functionis running, wait until it's done
		// this is a workaround for an issue in some EA Launcher programs,
		// were some sort of deadlock happens on a pthread_once for wine's register_builtins
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
		uint64_t t = ts.tv_sec*1000000000LL + ts.tv_nsec;
		while(*once!=1) {
			sched_yield();
			clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
			if((ts.tv_sec*1000000000LL + ts.tv_nsec)-t>10*1000000LL)	// if wait last for more than 10ms, force as completed
				*once = 1;
			__sync_synchronize();
		}
	}
	return 0;
}
EXPORT int my___pthread_once(x64emu_t* emu, void* once, void* cb) __attribute__((alias("my_pthread_once")));

EXPORT int my_pthread_key_create(x64emu_t* emu, pthread_key_t* key, void* dtor)
{
	(void)emu;
	int ret = pthread_key_create(key, findkey_dtorFct(dtor));
	return ret;
}
EXPORT int my___pthread_key_create(x64emu_t* emu, pthread_key_t* key, void* dtor) __attribute__((alias("my_pthread_key_create")));

EXPORT int my_pthread_key_delete(x64emu_t* emu, pthread_key_t key)
{
	int ret = pthread_key_delete(key);
	return ret;
}

static pthread_cond_t* alignCond(pthread_cond_t* pc)
{
#ifndef NOALIGN
	if((uintptr_t)pc&7)
		return (pthread_cond_t*)(((uintptr_t)pc+7)&~7LL);
#endif
	return pc;
}

EXPORT int my_pthread_cond_signal(x64emu_t* emu, pthread_cond_t* cond)
{
	(void)emu;
	int ret = pthread_cond_signal(alignCond(cond));
	return ret;
}
EXPORT int my_pthread_cond_timedwait(x64emu_t* emu, pthread_cond_t* cond, void* mutex, void* abstime)
{
	(void)emu;
	int ret = pthread_cond_timedwait(alignCond(cond), mutex, (const struct timespec*)abstime);
	return ret;
}
EXPORT int my_pthread_cond_wait(x64emu_t* emu, pthread_cond_t* cond, void* mutex)
{
	(void)emu;
	int ret = pthread_cond_wait(alignCond(cond), mutex);
	return ret;
}
EXPORT int my_pthread_cond_clockwait(x64emu_t *emu, pthread_cond_t* cond, void* mutex, clockid_t __clock_id, const struct timespec* __abstime)
{
	(void)emu;
	int ret;
	if(real_pthread_cond_clockwait) {
		ret = real_pthread_cond_clockwait(alignCond(cond), mutex, __clock_id, (void*)__abstime);
	} else {
		errno = EINVAL;
		ret = -1;
	}
	return ret;
}

#ifndef ANDROID
EXPORT void my__pthread_cleanup_push_defer(x64emu_t* emu, void* buffer, void* routine, void* arg)
{
    (void)emu;
	real_pthread_cleanup_push_defer(buffer, findcleanup_routineFct(routine), arg);
}

EXPORT void my__pthread_cleanup_push(x64emu_t* emu, void* buffer, void* routine, void* arg)
{
    (void)emu;
	_pthread_cleanup_push(buffer, findcleanup_routineFct(routine), arg);
}

EXPORT void my__pthread_cleanup_pop_restore(x64emu_t* emu, void* buffer, int exec)
{
    (void)emu;
	real_pthread_cleanup_pop_restore(buffer, exec);
}

EXPORT void my__pthread_cleanup_pop(x64emu_t* emu, void* buffer, int exec)
{
    (void)emu;
	_pthread_cleanup_pop(buffer, exec);
}

EXPORT int my_pthread_getaffinity_np(x64emu_t* emu, pthread_t thread, size_t cpusetsize, void* cpuset)
{
	(void)emu;
	int ret = pthread_getaffinity_np(thread, cpusetsize, cpuset);
	if(ret<0) {
		printf_log(LOG_INFO, "Warning, pthread_getaffinity_np(%p, %zd, %p) errored, with errno=%d\n", (void*)thread, cpusetsize, cpuset, errno);
	}

    return ret;
}
EXPORT int my_pthread_getaffinity_np_old(x64emu_t* emu, pthread_t thread, void* cpuset)
{
	return my_pthread_getaffinity_np(emu, thread, 128, cpuset);
}


EXPORT int my_pthread_setaffinity_np(x64emu_t* emu, pthread_t thread, size_t cpusetsize, void* cpuset)
{
	(void)emu;
	int ret = pthread_setaffinity_np(thread, cpusetsize, cpuset);
	if(ret<0) {
		printf_log(LOG_INFO, "Warning, pthread_setaffinity_np(%p, %zd, %p) errored, with errno=%d\n", (void*)thread, cpusetsize, cpuset, errno);
	}

    return ret;
}
EXPORT int my_pthread_setaffinity_np_old(x64emu_t* emu, pthread_t thread, void* cpuset)
{
	return my_pthread_setaffinity_np(emu, thread, 128, cpuset);
}
#endif

//EXPORT int my_pthread_attr_setaffinity_np(x64emu_t* emu, void* attr, uint32_t cpusetsize, void* cpuset)
//{
//	(void)emu;
//	int ret = pthread_attr_setaffinity_np(attr, cpusetsize, cpuset);
//	if(ret<0) {
//		printf_log(LOG_INFO, "Warning, pthread_attr_setaffinity_np(%p, %d, %p) errored, with errno=%d\n", attr, cpusetsize, cpuset, errno);
//	}
//
//    return ret;
//}

EXPORT int my_pthread_kill(x64emu_t* emu, void* thread, int sig)
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

EXPORT int my_pthread_kill_old(x64emu_t* emu, void* thread, int sig)
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

//EXPORT void my_pthread_exit(x64emu_t* emu, void* retval)
//{
//	(void)emu;
//	pthread_exit(retval);
//}

#ifndef NOALIGN
typedef union my_mutexattr_s {
	int					x86;
	pthread_mutexattr_t nat;
} my_mutexattr_t;
// mutexattr
EXPORT int my_pthread_mutexattr_destroy(x64emu_t* emu, my_mutexattr_t *attr)
{
	my_mutexattr_t mattr = {0};
	mattr.x86 = attr->x86;
	int ret = pthread_mutexattr_destroy(&mattr.nat);
	attr->x86 = mattr.x86;
	return ret;
}
EXPORT int my___pthread_mutexattr_destroy(x64emu_t* emu, my_mutexattr_t *attr) __attribute__((alias("my_pthread_mutexattr_destroy")));
EXPORT int my_pthread_mutexattr_getkind_np(x64emu_t* emu, my_mutexattr_t *attr, void* p)
{
	my_mutexattr_t mattr = {0};
	mattr.x86 = attr->x86;
	//int ret = pthread_mutexattr_getkind_np(&mattr.nat, p);
	int ret = pthread_mutexattr_gettype(&mattr.nat, p);
	attr->x86 = mattr.x86;
	return ret;
}
EXPORT int my_pthread_mutexattr_getprotocol(x64emu_t* emu, my_mutexattr_t *attr, void* p)
{
#ifndef TERMUX
	(void)emu;
	my_mutexattr_t mattr = {0};
	mattr.x86 = attr->x86;
	int ret = pthread_mutexattr_getprotocol(&mattr.nat, p);
	attr->x86 = mattr.x86;
	return ret;
#else
	(void)emu; (void)attr; (void)p;
	return 0;
#endif
}
EXPORT int my_pthread_mutexattr_gettype(x64emu_t* emu, my_mutexattr_t *attr, void* p)
{
	my_mutexattr_t mattr = {0};
	mattr.x86 = attr->x86;
	int ret = pthread_mutexattr_gettype(&mattr.nat, p);
	attr->x86 = mattr.x86;
	return ret;
}
#ifndef ANDROID
EXPORT int my_pthread_mutexattr_getrobust(x64emu_t* emu, my_mutexattr_t *attr, void* p)
{
	my_mutexattr_t mattr = {0};
	mattr.x86 = attr->x86;
	int ret = pthread_mutexattr_getrobust(&mattr.nat, p);
	attr->x86 = mattr.x86;
	return ret;
}
#endif
EXPORT int my_pthread_mutexattr_init(x64emu_t* emu, my_mutexattr_t *attr)
{
	my_mutexattr_t mattr = {0};
	mattr.x86 = attr->x86;
	int ret = pthread_mutexattr_init(&mattr.nat);
	attr->x86 = mattr.x86;
	return ret;
}
EXPORT int my___pthread_mutexattr_init(x64emu_t* emu, my_mutexattr_t *attr) __attribute__((alias("my_pthread_mutexattr_init")));
EXPORT int my_pthread_mutexattr_setkind_np(x64emu_t* emu, my_mutexattr_t *attr, int k)
{
	my_mutexattr_t mattr = {0};
	mattr.x86 = attr->x86;
	//int ret = pthread_mutexattr_setkind_np(&mattr.nat, k);
	int ret = pthread_mutexattr_settype(&mattr.nat, k);
	attr->x86 = mattr.x86;
	return ret;
}
EXPORT int my_pthread_mutexattr_setprioceiling(x64emu_t* emu, my_mutexattr_t *attr, int p)
{
#if !defined(TERMUX) && !defined(ANDROID)
	(void)emu;
	my_mutexattr_t mattr = {0};
	mattr.x86 = attr->x86;
	int ret = pthread_mutexattr_setprioceiling(&mattr.nat, p);
	attr->x86 = mattr.x86;
	return ret;
#else
	(void)emu; (void)attr; (void)p;
	return 0;
#endif
}
EXPORT int my_pthread_mutexattr_setprotocol(x64emu_t* emu, my_mutexattr_t *attr, int p)
{
#ifndef TERMUX
	(void)emu;
	my_mutexattr_t mattr = {0};
	mattr.x86 = attr->x86;
	int ret = pthread_mutexattr_setprotocol(&mattr.nat, p);
	attr->x86 = mattr.x86;
	return ret;
#else
	(void)emu; (void)attr; (void)p;
	return 0;
#endif
}
EXPORT int my_pthread_mutexattr_setpshared(x64emu_t* emu, my_mutexattr_t *attr, int p)
{
	my_mutexattr_t mattr = {0};
	mattr.x86 = attr->x86;
	int ret = pthread_mutexattr_setpshared(&mattr.nat, p);
	attr->x86 = mattr.x86;
	return ret;
}
EXPORT int my_pthread_mutexattr_settype(x64emu_t* emu, my_mutexattr_t *attr, int t)
{
	my_mutexattr_t mattr = {0};
	mattr.x86 = attr->x86;
	int ret = pthread_mutexattr_settype(&mattr.nat, t);
	attr->x86 = mattr.x86;
	return ret;
}
EXPORT int my___pthread_mutexattr_settype(x64emu_t* emu, my_mutexattr_t *attr, int t) __attribute__((alias("my_pthread_mutexattr_settype")));
#ifndef ANDROID
EXPORT int my_pthread_mutexattr_setrobust(x64emu_t* emu, my_mutexattr_t *attr, int t)
{
	my_mutexattr_t mattr = {0};
	mattr.x86 = attr->x86;
	int ret = pthread_mutexattr_setrobust(&mattr.nat, t);
	attr->x86 = mattr.x86;
	return ret;
}
#endif

#ifdef __SIZEOF_PTHREAD_MUTEX_T
#if __SIZEOF_PTHREAD_MUTEX_T == 48
#define MUTEX_OVERSIZED_8
#elif __SIZEOF_PTHREAD_MUTEX_T == 40
#define MUTEX_SIZE_X64
#endif
#endif

EXPORT int my_pthread_mutex_init(pthread_mutex_t *m, my_mutexattr_t *att)
{
	my_mutexattr_t mattr = {0};
	if(att)
		mattr.x86 = att->x86;
	#ifdef MUTEX_OVERSIZED_8
	uint64_t save = *(uint64_t*)(((uintptr_t)m) + 40);
	int ret = pthread_mutex_init(m, att?(&mattr.nat):NULL);
	*(uint64_t*)(((uintptr_t)m) + 40) = save;	// put back overwritten value. Nasty but should be fast and quite safe
	#elif defined(MUTEX_SIZE_X64)
	int ret = pthread_mutex_init(m, att?(&mattr.nat):NULL);
	#else
	pthread_mutex_t native;
	int ret = pthread_mutex_init(&native, att?(&mattr.nat):NULL);
	memcpy(m, &native, 40);	// 40 == sizeof(pthread_mutex_t) on x86_64
	#endif
	return ret;
}
EXPORT int my___pthread_mutex_init(pthread_mutex_t *m, my_mutexattr_t *att) __attribute__((alias("my_pthread_mutex_init")));

typedef union my_condattr_s {
	int					x86;
	pthread_condattr_t 	nat;
} my_condattr_t;
// condattr
EXPORT int my_pthread_condattr_destroy(x64emu_t* emu, my_condattr_t* c)
{
	my_condattr_t cond = {0};
	cond.x86 = c->x86;
	int ret = pthread_condattr_destroy(&cond.nat);
	c->x86 = cond.x86;
	return ret;
}
EXPORT int my_pthread_condattr_getclock(x64emu_t* emu, my_condattr_t* c, void* cl)
{
	my_condattr_t cond = {0};
	cond.x86 = c->x86;
	int ret = pthread_condattr_getclock(&cond.nat, cl);
	c->x86 = cond.x86;
	return ret;
}
EXPORT int my_pthread_condattr_getpshared(x64emu_t* emu, my_condattr_t* c, void* p)
{
	my_condattr_t cond = {0};
	cond.x86 = c->x86;
	int ret = pthread_condattr_getpshared(&cond.nat, p);
	c->x86 = cond.x86;
	return ret;
}
EXPORT int my_pthread_condattr_init(x64emu_t* emu, my_condattr_t* c)
{
	my_condattr_t cond = {0};
	cond.x86 = c->x86;
	int ret = pthread_condattr_init(&cond.nat);
	c->x86 = cond.x86;
	return ret;
}
EXPORT int my_pthread_condattr_setclock(x64emu_t* emu, my_condattr_t* c, int cl)
{
	my_condattr_t cond = {0};
	cond.x86 = c->x86;
	int ret = pthread_condattr_setclock(&cond.nat, cl);
	c->x86 = cond.x86;
	return ret;
}
EXPORT int my_pthread_condattr_setpshared(x64emu_t* emu, my_condattr_t* c, int p)
{
	my_condattr_t cond = {0};
	cond.x86 = c->x86;
	int ret = pthread_condattr_setpshared(&cond.nat, p);
	c->x86 = cond.x86;
	return ret;
}
EXPORT int my_pthread_cond_init(x64emu_t* emu, pthread_cond_t *pc, my_condattr_t* c)
{
	my_condattr_t cond = {0};
	if(c)
		cond.x86 = c->x86;
	int ret;
	#ifndef NOALIGN
	if((uintptr_t)pc & 7) {
		// cond is not allign, re-align it on the fly
		pthread_cond_t newc;
		ret = pthread_cond_init(&newc, c?(&cond.nat):NULL);
		memcpy((void*)(((uintptr_t)pc+7)&~7LL), &newc, sizeof(pthread_cond_t)-((uintptr_t)pc&7));
	} else
	#endif
	ret = pthread_cond_init(pc, c?(&cond.nat):NULL);
	if(c)
		c->x86 = cond.x86;
	return ret;
}
#ifndef NOALIGN
EXPORT int my_pthread_cond_destroy(x64emu_t* emu, pthread_cond_t *pc)
{
		return pthread_cond_destroy(alignCond(pc));
}
EXPORT int my_pthread_cond_broadcast(x64emu_t* emu, pthread_cond_t *pc)
{
		return pthread_cond_broadcast(alignCond(pc));
}
#endif

typedef struct pthread_cond_old_s {
	pthread_cond_t* cond;
} pthread_cond_old_t;

static pthread_cond_t* get_cond(pthread_cond_old_t* cond) {
	if(!cond->cond) {
		pthread_cond_t* newcond = box_calloc(1, sizeof(pthread_cond_t));
		#ifdef DYNAREC
		if(native_lock_storeifnull(&cond->cond, newcond))
			box_free(newcond);
		#else
		static pthread_mutex_t mutex_cond = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_lock(&mutex_cond);
		if(!cond->cond)
			cond->cond = newcond;
		else
			box_free(newcond);
		#endif
	}
	return cond->cond;
}

EXPORT int my_pthread_cond_broadcast_old(x64emu_t* emu, pthread_cond_old_t* cond)
{
    (void)emu;
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_broadcast(c);
}
EXPORT int my_pthread_cond_destroy_old(x64emu_t* emu, pthread_cond_old_t* cond)
{
    (void)emu;
	pthread_cond_t * c = get_cond(cond);
	int ret = pthread_cond_destroy(c);
	box_free(cond->cond);
	return ret;
}
EXPORT int my_pthread_cond_init_old(x64emu_t* emu, pthread_cond_old_t* cond, void* attr)
{
    (void)emu;
	pthread_cond_t *c = get_cond(cond);
	return pthread_cond_init(c, (const pthread_condattr_t*)attr);
}
EXPORT int my_pthread_cond_signal_old(x64emu_t* emu, pthread_cond_old_t* cond)
{
    (void)emu;
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_signal(c);
}
EXPORT int my_pthread_cond_timedwait_old(x64emu_t* emu, pthread_cond_old_t* cond, void* mutex, void* abstime)
{
    (void)emu;
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_timedwait(c, mutex, (const struct timespec*)abstime);
	#undef T
}
EXPORT int my_pthread_cond_wait_old(x64emu_t* emu, pthread_cond_old_t* cond, void* mutex)
{
    (void)emu;
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_wait(c, mutex);
}

typedef union my_barrierattr_s {
	int						x86;
	pthread_barrierattr_t 	nat;
} my_barrierattr_t;
// barrierattr
EXPORT int my_pthread_barrierattr_destroy(x64emu_t* emu, my_barrierattr_t* b)
{
	my_barrierattr_t battr = {0};
	battr.x86 = b->x86;
	int ret = pthread_barrierattr_destroy(&battr.nat);
	b->x86 = battr.x86;
	return ret;
}
EXPORT int my_pthread_barrierattr_getpshared(x64emu_t* emu, my_barrierattr_t* b, void* p)
{
	my_barrierattr_t battr = {0};
	battr.x86 = b->x86;
	int ret = pthread_barrierattr_getpshared(&battr.nat, p);
	b->x86 = battr.x86;
	return ret;
}
EXPORT int my_pthread_barrierattr_init(x64emu_t* emu, my_barrierattr_t* b)
{
	my_barrierattr_t battr = {0};
	battr.x86 = b->x86;
	int ret = pthread_barrierattr_init(&battr.nat);
	b->x86 = battr.x86;
	return ret;
}
EXPORT int my_pthread_barrierattr_setpshared(x64emu_t* emu, my_barrierattr_t* b, int p)
{
	my_barrierattr_t battr = {0};
	battr.x86 = b->x86;
	int ret = pthread_barrierattr_setpshared(&battr.nat, p);
	b->x86 = battr.x86;
	return ret;
}
EXPORT int my_pthread_barrier_init(x64emu_t* emu, pthread_barrier_t* bar, my_barrierattr_t* b, uint32_t count)
{
	my_barrierattr_t battr = {0};
	if(b)
		battr.x86 = b->x86;
	int ret = pthread_barrier_init(bar, b?(&battr.nat):NULL, count);
	if(b)
		b->x86 = battr.x86;
	return ret;
}

#endif

void init_pthread_helper()
{
	#ifdef BOX32
	if(box64_is32bits)
		init_pthread_helper_32();
	#endif
	real_pthread_cleanup_push_defer = (vFppp_t)dlsym(NULL, "_pthread_cleanup_push_defer");
	real_pthread_cleanup_pop_restore = (vFpi_t)dlsym(NULL, "_pthread_cleanup_pop_restore");
	real_pthread_cond_clockwait = (iFppip_t)dlsym(NULL, "pthread_cond_clockwait");

	// search for older symbol for pthread_kill
	{
		char buff[50];
		for(int i=0; i<34 && !real_phtread_kill_old; ++i) {
			snprintf(buff, 50, "GLIBC_2.%d", i);
			real_phtread_kill_old = (iFli_t)dlvsym(NULL, "pthread_kill", buff);
		}
	}
	if(!real_phtread_kill_old)
		real_phtread_kill_old = (iFli_t)dlvsym(NULL, "pthread_kill", "GLIBC_2.2.5");
	if(!real_phtread_kill_old) {
		printf_log(LOG_INFO, "Warning, older then 2.34 pthread_kill not found, using current one\n");
		real_phtread_kill_old = (iFli_t)pthread_kill;
	}

	pthread_key_create(&thread_key, emuthread_destroy);
	pthread_setspecific(thread_key, NULL);
}

void clean_current_emuthread()
{
	emuthread_t *et = (emuthread_t*)pthread_getspecific(thread_key);
	if(et) {
		pthread_setspecific(thread_key, NULL);
		emuthread_destroy(et);
	}
}

void fini_pthread_helper(box64context_t* context)
{
	#ifdef BOX32
	if(box64_is32bits)
		fini_pthread_helper_32(context);
	#endif
	CleanStackSize(context);
	clean_current_emuthread();
}

int checkUnlockMutex(void* m)
{
	pthread_mutex_t* mutex = (pthread_mutex_t*)m;
	int ret = pthread_mutex_unlock(mutex);
	if(ret==0) {
		return 1;
	}
	return 0;
}

int checkNolockMutex(void* m)
{
	pthread_mutex_t* mutex = (pthread_mutex_t*)m;
	int ret = pthread_mutex_trylock(mutex);
	if(ret==0) {
		pthread_mutex_unlock(mutex);
		return 0;
	}
	if(ret == EDEADLK)
		return 1;
	return 0;
}
