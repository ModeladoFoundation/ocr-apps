/* ----------------------------------------------------- *\

Sanity test for pthreads

This test make sure the pthreads is able to create, join,
and return a value from a thread and that it can properly
modify global variables from within a thread. Note, the
output is slightly non-deterministic in that the first two
lines may be in either order.

Example expected output:
   -----------------------------------------------------
Child done
Joining thread
Child knew who it was
Child returned 1234
Global val was 42
\* ----------------------------------------------------- */
#include <pthread.h>
#include <stdio.h>
#include <string.h>

int val = 0;
pthread_t child_self;

void *child_fn(void *arg)
{
    child_self = pthread_self();
    val = 42;
    printf("Child done\n");
    pthread_exit((void *) 1234);
}

int main(int argc, char *argv[])
{
    pthread_t thread;

    int ret = pthread_create(&thread, NULL, child_fn, NULL);
    if (ret) {
        fprintf(stderr, "Error, pthread_create() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    printf("Joining thread\n");
    int child_ret = 0;
    ret = pthread_join(thread, (void *)&child_ret);
    if (ret) {
        printf("Error, pthread_join returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    if (pthread_equal(thread, child_self)) {
        printf("Child knew who it was\n");
    } else {
        printf("Child had identity crisis...\n");
        return 1;
    }

    if (child_ret == 1234) {
        printf("Child returned 1234\n");
    } else {
        printf("FAIL! Child returned %d instead of 1234\n", child_ret);
    }

    if (val == 42) {
        printf("Global val was 42\n");
    } else {
        printf("FAIL! Global val was %d instead of 42\n", val);
    }

    return 0;
}

