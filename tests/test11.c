#define _MULTI_THREADED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void foo(void);  /* Functions that use the TLS data */
void bar(void);

#define checkResults(string, val) {             \
 if (val) {                                     \
   printf("Failed with %d at %s", val, string); \
   exit(1);                                     \
 }                                              \
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

   printf("Thread %d: Entered (%d/%d)\n", (pthread_self()==thread[0])?1:2, TLS_data1, TLS_data2);

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
   printf("Thread %d: foo(), TLS data=%d %d \"%s\"\n",
          (pthread_self()==thread[0])?1:2, TLS_data1, TLS_data2, TLS_data3);
   while(!thread[1])
      usleep(300);
   if(pthread_self()==thread[0])
      pthread_join(thread[1], NULL);
   bar();
}
 
void bar() {
   printf("Thread %d: bar(), TLS data=%d %d \"%s\"\n",
          (pthread_self()==thread[0])?1:2, TLS_data1, TLS_data2, TLS_data3);
   return;
}
 

int main(int argc, char **argv)
{
  int                   rc=0;
  int                   i;
  threadparm_t          gData[NUMTHREADS];
 
  printf("Create/start %d threads\n", NUMTHREADS);
  for (i=0; i < NUMTHREADS; i++) { 
     /* Create per-thread TLS data and pass it to the thread */
     gData[i].data1 = i;
     gData[i].data2 = (i+1)*2;
     rc = pthread_create(&thread[i], NULL, thread_run, &gData[i]);
     checkResults("pthread_create()\n", rc);
     usleep(200);
  }
 
  //printf("Wait for all threads to complete, and release their resources\n");
  for (i=0; i < NUMTHREADS; i++) {
     rc = pthread_join(thread[i], NULL);
     //checkResults("pthread_join()\n", rc);
  }

  printf("Main completed\n");
  return 0;
}

