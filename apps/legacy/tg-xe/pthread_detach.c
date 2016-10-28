/* ----------------------------------------------------- *\

Test for detaching a thread

This test make sure the pthreads respects detaching a
thread and automatically cleaning it up. The
pthread_exit(NULL) at the end of the main method will hang
until all threads have completed and are cleaned up. Note
that the execution order of this program is
non-determanistic, so the output might be reordered from
the example expected output.

Example expected output:
   -----------------------------------------------------
making thread 1 (detached)
making thread 2
child 1 done
detaching thread 2
parent done
child 2 done
\* ----------------------------------------------------- */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *child_fn(void *arg)
{
    printf("child %d done\n", (int)arg);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    pthread_t thread1;
    pthread_t thread2;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("making thread 1 (detached)\n");
    int ret = pthread_create(&thread1, &attr, child_fn, (void *)1);
    if (ret) {
        fprintf(stderr, "Error, pthread_create() with attr returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    printf("making thread 2\n");
    ret = pthread_create(&thread2, NULL, child_fn, (void *)2);
    if (ret) {
        fprintf(stderr, "Error, pthread_create() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    printf("detaching thread 2\n");
    ret = pthread_detach(thread2);
    if (ret) {
        fprintf(stderr, "Error, pthread_detach() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    printf("parent done\n");
    pthread_exit(NULL); // Use pthread_exit to allow children to finish running.
}
