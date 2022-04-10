#define _MULTI_THREADED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void foo(void);  /* Functions that use the TLS data */
void bar(void);

#define checkResults(string, val) {                  \
	if (val) {                                       \
		printf("Failed with %d at %s", val, string); \
		exit(1);                                     \
	}                                                \
}
 
/* 
	 Use the keyword provided by pthread.h to delcare the following variable
	 is thread specific, i.e. it is only visible to a specific thread, 
	 not shared/common to all thread.
	 These variables are stored in thread local storage (TLS) area.
 */
__thread int TLS_data1 = 10;
__thread int TLS_data2 = 20;
__thread char TLS_data3[10];

// Sync, because it's needed apparently...
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t* mutex_ptr = &mutex;
static pthread_cond_t thread_state_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t* thread_state_cond_ptr = &thread_state_cond;
static int status = 0;
 
#define  NUMTHREADS   2 
pthread_t             thread[NUMTHREADS];

typedef struct {
	int   data1;
	int   data2;
} threadparm_t; 

void *thread_run(void *parm)
{
	int               rc;
	threadparm_t     *gData;

	pthread_mutex_lock(mutex_ptr);
	if (pthread_self()==thread[0]) {
		printf("Thread 1: Entered (%d/%d)\n", TLS_data1, TLS_data2);
		fflush(stdout);
	} else {
		while (!status) pthread_cond_wait(thread_state_cond_ptr, mutex_ptr);
		printf("Thread 2: Entered (%d/%d)\n", TLS_data1, TLS_data2);
		fflush(stdout);
		pthread_mutex_unlock(mutex_ptr);
		status = 0;
		pthread_cond_broadcast(thread_state_cond_ptr);
	}

	gData = (threadparm_t *)parm;

	/* Assign the value from global variable to thread specific variable*/
	TLS_data1 = gData->data1;
	TLS_data2 = gData->data2;
	strcpy(TLS_data3, "---");
	TLS_data3[1] = (pthread_self()==thread[0])?'1':'2';

	foo();
	return NULL;
}
 
void foo() {
	if (pthread_self()==thread[0]) {
		printf("Thread 1: foo(), TLS data=%d %d \"%s\"\n", TLS_data1, TLS_data2, TLS_data3);
		fflush(stdout);
		status = 1;
		pthread_cond_broadcast(thread_state_cond_ptr);
		while (status) pthread_cond_wait(thread_state_cond_ptr, mutex_ptr);
		pthread_mutex_unlock(mutex_ptr);
	} else {
		printf("Thread 2: foo(), TLS data=%d %d \"%s\"\n", TLS_data1, TLS_data2, TLS_data3);
		fflush(stdout);
	}
	while(!thread[1])
		usleep(300);
	if(pthread_self()==thread[0])
		pthread_join(thread[1], NULL);
	bar();
}
 
void bar() {
	printf("Thread %d: bar(), TLS data=%d %d \"%s\"\n",
	        (pthread_self()==thread[0])?1:2, TLS_data1, TLS_data2, TLS_data3);
	fflush(stdout);
	return;
}
 
int main(int argc, char **argv)
{
	int                   rc=0;
	int                   i;
	threadparm_t          gData[NUMTHREADS];

	printf("Create/start %d threads\n", NUMTHREADS);
	fflush(stdout);
	for (i=0; i < NUMTHREADS; i++) { 
		/* Create per-thread TLS data and pass it to the thread */
		gData[i].data1 = i;
		gData[i].data2 = (i+1)*2;
		rc = pthread_create(&thread[i], NULL, thread_run, &gData[i]);
		checkResults("pthread_create()\n", rc);
	}

	//printf("Wait for all threads to complete, and release their resources\n");
	for (i=0; i < NUMTHREADS; i++) {
		rc = pthread_join(thread[i], NULL);
		//checkResults("pthread_join()\n", rc);
	}

	thread_state_cond_ptr = NULL;
	i = pthread_cond_destroy(&thread_state_cond);
	checkResults("destroying cond\n", i);
	mutex_ptr = NULL;
	i = pthread_mutex_destroy(&mutex);
	checkResults("destroying mutex\n", i);

	printf("Main completed\n");
	fflush(stdout);
	return 0;
}
