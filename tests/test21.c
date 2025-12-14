#define __USE_GNU
#include <signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/ucontext.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

static jmp_buf context_buf;

static void segv_handler(int sig)
{
	printf("sig = %d\n", sig);
	siglongjmp(context_buf, 1);
}

void test()
{
	if(!sigsetjmp(context_buf, 1)) {
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
	printf("sig = %d\n", sig);
	printf("si_addr: %zx, si_code: %d, si_errno: %d", 
		info->si_addr,
		info->si_code,
		info->si_errno
	);
	if(ucntx->uc_mcontext.gregs[X_IP])
	        printf(" RIP offset: %zd",
        	        ((intptr_t)ucntx->uc_mcontext.gregs[X_IP])-((intptr_t)exec_p)
	        );
	else
		printf(" RIP=NULL");
        printf(" TRAPERR=%d TRAPNO=%d\n",
                ucntx->uc_mcontext.gregs[X_ERR],
                ucntx->uc_mcontext.gregs[X_TRAPNO]
        );
	siglongjmp(context_buf, 1);
}

static unsigned char buff_cc[] = { 0xcc, 0xc3 };
static unsigned char buff_cd03[] = { 0xcd, 0x03, 0xc3 };
static unsigned char buff_cd2d[] = { 0xcd, 0x2d, 0xc3 };
void test_cc()
{
	memcpy(exec_p, buff_cc, sizeof(buff_cc));
	if(!sigsetjmp(context_buf, 1)) {
		vFv_t f = (vFv_t)exec_p;
		f();
	}
	memcpy(exec_p, buff_cd03, sizeof(buff_cd03));
	if(!sigsetjmp(context_buf, 1)) {
		vFv_t f = (vFv_t)exec_p;
		f();
	}
	memcpy(exec_p, buff_cd2d, sizeof(buff_cd2d));
	if(!sigsetjmp(context_buf, 1)) {
		vFv_t f = (vFv_t)exec_p;
		f();
	}
        if(!sigsetjmp(context_buf, 1)) {
                vFv_t f = (vFv_t)NULL;
                f();
        }
}

int main()
{
	if(signal(SIGSEGV, segv_handler) == SIG_ERR) {
		printf("signal: Err = %d\n", errno);
		return -1;
	}
	//printf("handler = %p\n", segv_handler);
	test();
    struct sigaction action = {0};
    action.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
    action.sa_sigaction = (void*)segv_action;
    if(sigaction(SIGSEGV, &action, NULL)) {
		printf("sigaction: Err = %d\n", errno);
		return -2;
	}
    action.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
    action.sa_sigaction = (void*)segv_action;
    if(sigaction(SIGTRAP, &action, NULL)) {
		printf("sigaction 2: Err = %d\n", errno);
		return -2;
	}
	exec_p = mmap(NULL, 65536, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if(exec_p==MAP_FAILED) {
		printf("mmap: Err = %d\n", errno);
		return -3;
	}
	test_cc();
	return 0;
}
