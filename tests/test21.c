#define __USE_GNU
#include <signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/ucontext.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

static jmp_buf context_buf;

static void segv_handler(int sig)
{
	printf("sig = %d\n", sig);
	siglongjmp(context_buf, 1);
}

void test()
{
	if(!setjmp(context_buf)) {
		int *bad_ptr = (int*)0xffffffffdeadbeef;
		printf("*bad_ptr = %d\n", *bad_ptr);
	} else {
		printf("got bad_ptr\n");
	}
}

void* exec_p = NULL;
typedef void(*vFv_t)(void);
#define X_IP		16
#define X_TRAPNO 	20
#define X_ERR		19
static void segv_action(int sig, siginfo_t* info, ucontext_t* ucntx)
{
	if(!exec_p) {
		segv_handler(sig);
		return;
	}
	printf("sig = %d\n", sig);
	uintptr_t rip = (intptr_t)ucntx->uc_mcontext.gregs[X_IP];
	if(info->si_addr>=exec_p && info->si_addr<(exec_p+10))
		printf("si_addr: exec_p+%zx, ", (uintptr_t)info->si_addr-(uintptr_t)exec_p);
	else
		printf("si_addr: %zx, ", info->si_addr);
	printf("si_code: %d, si_errno: %d, ", info->si_code, info->si_errno);
	if(rip>=((intptr_t)exec_p) && rip<((intptr_t)exec_p+5))
		printf("RIP offset: %zd, ", rip-((intptr_t)exec_p));
	printf("TRAPERR=0x%x TRAPNO=%d\n", 
		ucntx->uc_mcontext.gregs[X_ERR],
		ucntx->uc_mcontext.gregs[X_TRAPNO]
	);
	siglongjmp(context_buf, 1);
}

static unsigned char buff_cc[] = { 0xcc, 0xc3 };
static unsigned char buff_cd03[] = { 0xcd, 0x03, 0xc3 };
static unsigned char buff_cd2d[] = { 0xcd, 0x2d, 0xc3 };
static uint8_t buff_simplef[] = { 0xb8, 1, 0, 0, 0, 0xc3 };
void test_cc()
{
	memcpy(exec_p, buff_cc, sizeof(buff_cc));
	if(!setjmp(context_buf)) {
		vFv_t f = (vFv_t)exec_p;
		f();
	}
	memcpy(exec_p, buff_cd03, sizeof(buff_cd03));
	if(!setjmp(context_buf)) {
		vFv_t f = (vFv_t)exec_p;
		f();
	}
	memcpy(exec_p, buff_cd2d, sizeof(buff_cd2d));
	if(!setjmp(context_buf)) {
		vFv_t f = (vFv_t)exec_p;
		f();
	}
}

void test_segfault()
{
	printf("from non-existant memory\n");
	// writing to existing protected memory
	if(!setjmp(context_buf)) {
		int *bad_ptr = (int*)0xffffffffdeadbeef;
		*(uint8_t*)bad_ptr = 0xc3;
	} else {
		printf("segfault, good\n");
	}
	// writing to existing protected memory
	if(!setjmp(context_buf)) {
		int *bad_ptr = (int*)0xffffffffdeadbeef;
		if(*(uint8_t*)bad_ptr == 0xc3)
			printf("should not be readable or writeable!\n");
		else
			printf("should not be readable!\n");
		printf("aborting test\n");
		return;
	} else {
		printf("segfault, good\n");
	}
	// writing to existing protected memory
	if(!setjmp(context_buf)) {
		void* bad_ptr = (int*)0xffffffffdeadbeef;
		void(*f)() = bad_ptr;
		f();
		printf("should not work!!! aboting test\n");
		return;
	} else {
		printf("segfault, good\n");
	}
	printf("from NULL memory\n");
	// writing to existing protected memory
	if(!setjmp(context_buf)) {
		int *bad_ptr = (int*)NULL;
		*(uint8_t*)bad_ptr = 0xc3;
	} else {
		printf("segfault, good\n");
	}
	// writing to existing protected memory
	if(!setjmp(context_buf)) {
		int *bad_ptr = (int*)NULL;
		if(*(uint8_t*)bad_ptr == 0xc3)
			printf("should not be readable or writeable!\n");
		else
			printf("should not be readable!\n");
		printf("aborting test\n");
		return;
	} else {
		printf("segfault, good\n");
	}
	// writing to existing protected memory
	if(!setjmp(context_buf)) {
		void* bad_ptr = (int*)NULL;
		void(*f)() = bad_ptr;
		f();
		printf("should not work!!! aboting test\n");
		return;
	} else {
		printf("segfault, good\n");
	}
	printf("from existant memory\n");
	printf("exec_p prot = 0\n");
	mprotect(exec_p, 65536, 0);
	// writing to existing protected memory
	if(!setjmp(context_buf)) {
		*(uint8_t*)exec_p = 0xc3;
	} else {
		printf("segfault, good\n");
	}
	// reading for exising protected memory
	if(!setjmp(context_buf)) {
		if(*(uint8_t*)exec_p == 0xc3)
			printf("Error, this value should not be 0xc3\n");
	} else {
		printf("segfault, good\n");
	}
	printf("exec_p prot = PROT_READ\n");
	mprotect(exec_p, 65536, PROT_READ);
	// writing to existing protected memory
	if(!setjmp(context_buf)) {
		*(uint8_t*)exec_p = 0xc3;
	} else {
		printf("segfault, good\n");
	}
	// reading should work
	if(!setjmp(context_buf)) {
		if(*(uint8_t*)exec_p == 0xc3)
			printf("Error, this value should not be 0xc3\n");
	} else {
		printf("segfault, not good....\n");
	}
	// reading should work
	if(!setjmp(context_buf)) {
		if(*(uint8_t*)exec_p == 0xc3)
			printf("Error, this value should not be 0xc3\n");
	} else {
		printf("segfault, good\n");
	}
	printf("exec_p prot = PROT_READ|PROT_WRITE\n");
	mprotect(exec_p, 65536, PROT_READ|PROT_WRITE);
	// writing should
	if(!setjmp(context_buf)) {
		*(uint8_t*)exec_p = 0xc3;
	} else {
		printf("segfault, not good, aborting test\n");
		return;
	}
	// reading should work
	if(!setjmp(context_buf)) {
		if(*(uint8_t*)exec_p != 0xc3) {
			printf("Error, this value should be 0xc3, aborting test\n");
			return;
		}
	} else {
		printf("segfault, not good....\n");
	}
	// should not be able to run
	if(!setjmp(context_buf)) {
		vFv_t f = (vFv_t)exec_p;
		f();
	} else {
		printf("Cannot run, good\n");
	}
	printf("exec_p prot = PROT_READ|PROT_WRITE|PROT_EXEC\n");
	mprotect(exec_p, 65536, PROT_READ|PROT_WRITE|PROT_EXEC);
	if(!setjmp(context_buf)) {
		vFv_t f = (vFv_t)exec_p;
		f();
	} else {
		printf("Cannot run, not good!\n");
	}
	printf("exec_p prot = PROT_READ|PROT_WRITE\n");
	mprotect(exec_p, 65536, PROT_READ|PROT_WRITE);
	memcpy(exec_p, buff_simplef, sizeof(buff_simplef));
	if(!setjmp(context_buf)) {
		int(*f)() = exec_p;
		if(f()!=1) {
			printf("function return should be 1\n");
		}
	} else {
		printf("Cannot run, good!\n");
	}
	printf("exec_p prot = PROT_READ|PROT_WRITE|PROT_EXEC\n");
	mprotect(exec_p, 65536, PROT_READ|PROT_WRITE|PROT_EXEC);
	if(!setjmp(context_buf)) {
		int(*f)() = exec_p;
		if(f()!=1) {
			printf("function return should be 1\n");
		}
		((uint8_t*)exec_p)[1] = 2;
		if(f()!=2) {
			printf("function return should be 2\n");
		}
	} else {
		printf("Cannot run, not good, aborting test!\n");
	}
}

int main()
{
    struct sigaction action = {0};
    action.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
    action.sa_sigaction = (void*)segv_action;
    if(sigaction(SIGSEGV, &action, NULL)) {
		printf("sigaction: Err = %d\n", errno);
		return -2;
	}
    if(sigaction(SIGTRAP, &action, NULL)) {
		printf("sigaction 2: Err = %d\n", errno);
		return -2;
	}
	exec_p = NULL;
	test();
	exec_p = mmap(NULL, 65536, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if(exec_p==MAP_FAILED) {
		printf("mmap: Err = %d\n", errno);
		return -3;
	}
	test_cc();
	test_segfault();
	return 0;
}
