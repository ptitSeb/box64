// Code from https://martin.uy/blog/pthread_cancel-glibc-stack-unwinding/
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

static pthread_mutex_t mutex;
static pthread_mutex_t* mutex_ptr = NULL;
static pthread_cond_t thread_state_cond;
static pthread_cond_t* thread_state_cond_ptr = NULL;
static int thread_state = 0; // Sync

void thread_cleanup(void* args) {
    pthread_mutex_lock(mutex_ptr);
    printf("Thread: thread_state = 2.\n");
    thread_state = 2;
    pthread_cond_broadcast(thread_state_cond_ptr);
    pthread_mutex_unlock(mutex_ptr);
}

static void thread_f(void) {
    int ret = -1;
    pthread_mutex_lock(mutex_ptr);
    printf("Thread: thread_state = 1.\n");
    thread_state = 1;
    pthread_cond_broadcast(thread_state_cond_ptr);
    pthread_mutex_unlock(mutex_ptr);
    while (1) {
        sleep(1000);
    }
}

static void* thread_main(void* args) {
    pthread_cleanup_push(&thread_cleanup, NULL);
    thread_f();
    // This should never be executed
    pthread_cleanup_pop(0);
    return NULL;
}

int main(void) {
    int ret = 0;
    pthread_t thread;
    pthread_attr_t thread_attributes;
    pthread_attr_t* thread_attributes_ptr = NULL;

    if (pthread_mutex_init(&mutex, NULL) != 0)
        goto error;
    mutex_ptr = &mutex;

    if (pthread_cond_init(&thread_state_cond, NULL) != 0)
        goto error;
    thread_state_cond_ptr = &thread_state_cond;

    if (pthread_attr_init(&thread_attributes) != 0)
        goto error;

    thread_attributes_ptr = &thread_attributes;

    if (pthread_create(&thread, thread_attributes_ptr, &thread_main, NULL) != 0)
        goto error;

    thread_attributes_ptr = NULL;
    if (pthread_attr_destroy(&thread_attributes) != 0)
        goto error;

    // Wait for thread to go deep into the call stack
    pthread_mutex_lock(mutex_ptr);
    while (thread_state != 1)
        pthread_cond_wait(thread_state_cond_ptr, mutex_ptr);
    printf("Main thread: thread_state == 1.\n");
    pthread_mutex_unlock(mutex_ptr);

    #ifdef __ANDROID__
    if (pthread_kill(thread,0) != 0)
        goto error;
    #else
    if (pthread_cancel(thread) != 0)
        goto error;
    #endif

    // Wait for thread to execute the cleanup function
    pthread_mutex_lock(mutex_ptr);
    while (thread_state != 2)
        pthread_cond_wait(thread_state_cond_ptr, mutex_ptr);
    printf("Main thread: thread_state == 2.\n");
    pthread_mutex_unlock(mutex_ptr);

    thread_state_cond_ptr = NULL;
    if (pthread_cond_destroy(&thread_state_cond) != 0)
        goto error;

    mutex_ptr = NULL;
    if (pthread_mutex_destroy(&mutex) != 0)
        goto error;

    goto cleanup;

error:
    ret = -1;

cleanup:
    if (thread_attributes_ptr != NULL)
        pthread_attr_destroy(thread_attributes_ptr);
    if (thread_state_cond_ptr != NULL)
        pthread_cond_destroy(thread_state_cond_ptr);
    if (mutex_ptr != NULL)
        pthread_mutex_destroy(mutex_ptr);

    if (ret == -1)
        printf("Finished with errors.\n");
    else
        printf("Finished with no errors.\n");

    return ret;
}
