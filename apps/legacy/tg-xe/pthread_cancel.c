/* ----------------------------------------------------- *\

Test for cancel type / state

This test make sure the pthreads respects the set state and
type of cancel for a thread. It uses mutexs and conditions
to make sure that the result and output is completely
deterministic.

Expected output:
   -----------------------------------------------------
testing Enable Deferred
joined thread


testing Disable Deferred
testing cancel (should need to enable cancel first)
enabling cancel
joined thread


testing Enable Async
joined thread


testing Disable Async
testing cancel (should need to enable cancel first)
enabling cancel
joined thread


DONE
\* ----------------------------------------------------- */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;


void *child_fn_normal(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    pthread_mutex_lock(&m);
    pthread_mutex_unlock(&m);
    pthread_cond_signal(&c);

    for (;;)
        pthread_testcancel();

    return NULL;
}

void *child_fn_disable(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    pthread_mutex_lock(&m2);
    pthread_mutex_lock(&m);
    pthread_mutex_unlock(&m);
    pthread_cond_signal(&c);
    pthread_cond_wait(&c, &m2);
    pthread_mutex_unlock(&m2);

    printf("testing cancel (should need to enable cancel first)\n");
    pthread_testcancel();

    printf("enabling cancel\n");
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();

    printf("ERROR: child not canceled\n");
    return NULL;
}

void *child_fn_async(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    pthread_mutex_lock(&m);
    pthread_mutex_unlock(&m);
    pthread_cond_signal(&c);

    for (;;)
        ;

    return NULL;
}

void *child_fn_async_disable(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    pthread_mutex_lock(&m);
    pthread_mutex_unlock(&m);
    pthread_cond_signal(&c);

    printf("testing cancel (should need to enable cancel first)\n");
    pthread_testcancel();

    printf("enabling cancel\n");
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    for (;;)
        ;

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t thread;

    pthread_mutex_lock(&m);
    printf("testing Enable Deferred\n");
    int ret = pthread_create(&thread, NULL, child_fn_normal, NULL);
    if (ret) {
        fprintf(stderr, "Error, pthread_create() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }
    pthread_cond_wait(&c, &m);
    ret = pthread_cancel(thread);
    if (ret) {
        fprintf(stderr, "Error, pthread_cancel() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    pthread_join(thread, NULL);
    printf("joined thread\n\n\n");

    printf("testing Disable Deferred\n");
    ret = pthread_create(&thread, NULL, child_fn_disable, NULL);
    if (ret) {
        fprintf(stderr, "Error, pthread_create() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }
    pthread_cond_wait(&c, &m);
    ret = pthread_cancel(thread);
    if (ret) {
        fprintf(stderr, "Error, pthread_cancel() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }
    pthread_mutex_lock(&m2);
    pthread_cond_signal(&c);
    pthread_mutex_unlock(&m2);

    pthread_join(thread, NULL);
    printf("joined thread\n\n\n");

    printf("testing Enable Async\n");
    ret = pthread_create(&thread, NULL, child_fn_async, NULL);
    if (ret) {
        fprintf(stderr, "Error, pthread_create() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }
    pthread_cond_wait(&c, &m);
    ret = pthread_cancel(thread);
    if (ret) {
        fprintf(stderr, "Error, pthread_cancel() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    pthread_join(thread, NULL);
    printf("joined thread\n\n\n");

    printf("testing Disable Async\n");
    ret = pthread_create(&thread, NULL, child_fn_async_disable, NULL);
    if (ret) {
        fprintf(stderr, "Error, pthread_create() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }
    pthread_cond_wait(&c, &m);
    ret = pthread_cancel(thread);
    if (ret) {
        fprintf(stderr, "Error, pthread_cancel() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    pthread_join(thread, NULL);
    printf("joined thread\n\n\n");

    pthread_mutex_unlock(&m);

    printf("DONE\n");

    return 0;
}

