/* ----------------------------------------------------- *\

Test for using a recursive / errorcheck mutex.

This test make sure the pthreads respects detaching a
thread and automatically cleaning it up. The
pthread_exit(NULL) at the end of the main method will hang
until all threads have completed and are cleaned up. Note
that the execution order of this program is somewhat
non-determanistic, so the output might be slightly
reordered from the example expected output.

Example expected output:
   -----------------------------------------------------
TEST: recursive

main: creating child
main: joining child
child: recursing
child: done
main: joined child
main: locking mutex
SUCCESS! Value was 42


TEST: error check

main: creating child
main: joining child
child: recursing
child: mutex already locked
child: done
main: joined child
main: locking mutex
SUCCESS! Value was 1
\* ----------------------------------------------------- */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int val = 0;
pthread_mutex_t m;

void recurse(int arg)
{
    int ret = pthread_mutex_lock(&m);

    if (ret) {
        printf("child: mutex already locked\n");
        return;
    }

    if (arg) {
        val += 1;
        recurse(arg - 1);
    }

    pthread_mutex_unlock(&m);
}

void *child_fn(void *arg)
{
    printf("child: recursing\n");
    recurse((int)arg);
    printf("child: done\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    printf("TEST: recursive\n\n");
    pthread_t thread;

    pthread_mutexattr_t ma;
    pthread_mutexattr_init(&ma);
    pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m, &ma);

    printf("main: creating child\n");
    int ret = pthread_create(&thread, NULL, child_fn, (void *)42);
    if (ret) {
        fprintf(stderr, "Error, pthread_create() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }
    printf("main: joining child\n");
    ret = pthread_join(thread, NULL);

    if (ret) {
        printf("Error, pthread_join returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    printf("main: joined child\n");

    printf("main: locking mutex\n");
    pthread_mutex_unlock(&m);

    if (val == 42) {
        printf("SUCCESS! Value was %d\n", val);
    } else {
        printf("FAIL! Value was %d\n", val);
    }

    printf("\n\nTEST: error check\n\n");

    val = 0;

    pthread_mutex_destroy(&m);
    pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&m, &ma);

    printf("main: creating child\n");
    ret = pthread_create(&thread, NULL, child_fn, (void *)42);
    if (ret) {
        fprintf(stderr, "Error, pthread_create() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }
    printf("main: joining child\n");
    ret = pthread_join(thread, NULL);

    if (ret) {
        printf("Error, pthread_join returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    printf("main: joined child\n");

    printf("main: locking mutex\n");
    pthread_mutex_unlock(&m);

    if (val == 1) {
        printf("SUCCESS! Value was %d\n", val);
    } else {
        printf("FAIL! Value was %d\n", val);
    }
    return 0;
}

