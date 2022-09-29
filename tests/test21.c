#include <signal.h>
#include <setjmp.h>
#include <stdio.h>

static jmp_buf context_buf;

static void segv_handler(int sig)
{
	printf("sig = %d\n", sig);
	longjmp(context_buf, 1);
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

int main()
{
	if(signal(SIGSEGV, segv_handler) == SIG_ERR)
		printf("Err = %m\n");
	//printf("handler = %p\n", segv_handler);
	test();
	return 0;
}
