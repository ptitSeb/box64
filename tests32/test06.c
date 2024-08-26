#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

const int thread_count = 2;
pthread_t tid[2];
const char *thread_messages[2] = {
	"First thread executing",
	"Second thread executing"
};

void *doSomething(void *arg)
{
	pthread_t id = pthread_self();
	int num = -1;

	for (int i = 0 ; i < thread_count ; ++i)
	{
		if (pthread_equal(id, tid[i]))
		{
			num = i + 1;
			if (num == 2) printf("[%02d] %s\n", num, thread_messages[i]);
			break;
		}
	}

	for (unsigned int i = 0 ; i < 0x10000 ; ++i);
	if (num == 2) printf("[%02d] Thread done.\n", num);

	return NULL;
}

int main(int argc, char const *argv[])
{
	int err;

	for (int i = 0 ; i < thread_count ; ++i)
	{
		//printf("[00] Thread %d starting\n", i + 1);
		err = pthread_create(&tid[i], NULL, doSomething, NULL);
		if (err)
		{
			printf("[00] Couldn't create thread %d: %s\n", i + 1, strerror(err));
		}
		for (unsigned int i = 0 ; i < 0x1000 ; ++i);
	}

	//printf("[00] Waiting for all threads to end...\n");
	for (int i = 0 ; i < thread_count ; ++i)
		pthread_join(tid[i], NULL);
	printf("\n[00] Done.\n");

	return 0;
}
