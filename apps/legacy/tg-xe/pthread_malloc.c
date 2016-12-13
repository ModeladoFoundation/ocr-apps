/* ----------------------------------------------------- *\

Malloc test for pthreads

This test make sure the pthreads is able to malloc and
free memory in separate threads and that pthreads is able
to share and free memory across threads.

Example expected output:
   -----------------------------------------------------
   Running independent malloc tests

   Parent hogging memory
   Child1 hogging memory
   Parent freeing memory
   Parent joining child1
   Child1 freeing memory
   Child1 done

   Running shared memory malloc tests

   Parent hogging memory
   Parent setting up memory
   Parent joining child2
   Child2 inspecting memory
   Child2 freeing memory
   Child2 done
   Parent done
\* ----------------------------------------------------- */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BUFSIZE  512
#define STACKSIZE 32

#if BUFSIZE < STACKSIZE
#error BUFSIZE must be >= STACKSIZE
#endif

struct buf_stack_node {
    int buf [BUFSIZE];
    struct buf_stack_node * next;
};

struct buf_stack_node * hog_memory(int links)
{
    struct buf_stack_node * null_hog = NULL;
    struct buf_stack_node ** next_pp = &null_hog;
    for(;links;links--) {
        struct buf_stack_node * node = malloc(sizeof(struct buf_stack_node));
        assert(node != NULL && "malloc failed");
        node->next = *next_pp;
        *next_pp = node;
    }
    return *next_pp;
}

void setup_memory(struct buf_stack_node * node)
{
    int i = 0;
    for (;node;node=node->next,i++) {
        node->buf[i] = i;
    }
}

void inspect_memory(struct buf_stack_node * node)
{
    int i = 0;
    for (;node;node=node->next,i++) {
        assert(node->buf[i] == i);
    }
}

void free_memory(struct buf_stack_node * node)
{
    while (node) {
        struct buf_stack_node * tmp = node->next;
        free(node);
        node = tmp;
    }
}

void *hog_child_fn(void *arg)
{
    struct buf_stack_node * buf_stack;
    printf("Child1 hogging memory\n");
    buf_stack = hog_memory(STACKSIZE);
    printf("Child1 freeing memory\n");
    free_memory(buf_stack);
    printf("Child1 done\n");
    pthread_exit((void *) 0);
}

void *free_child_fn(void *arg)
{
    struct buf_stack_node * buf_stack = (struct buf_stack_node *)arg;
    printf("Child2 inspecting memory\n");
    inspect_memory(buf_stack);
    printf("Child2 freeing memory\n");
    free_memory(buf_stack);
    printf("Child2 done\n");
    pthread_exit((void *) 0);
}

int main(int argc, char *argv[])
{
    pthread_t child1;
    pthread_t child2;
    struct buf_stack_node * buf_stack;
    int ret;

    printf("Running independent malloc tests\n\n");

    ret = pthread_create(&child1, NULL, hog_child_fn, NULL);
    if (ret) {
        fprintf(stderr, "Error, pthread_create() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    printf("Parent hogging memory\n");
    buf_stack = hog_memory(STACKSIZE);
    printf("Parent freeing memory\n");
    free_memory(buf_stack);

    printf("Parent joining child1\n");
    ret = pthread_join(child1, NULL);
    if (ret) {
        printf("Error, pthread_join returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    printf("\nRunning shared memory malloc tests\n\n");

    printf("Parent hogging memory\n");
    buf_stack = hog_memory(STACKSIZE);
    printf("Parent setting up memory\n");
    setup_memory(buf_stack);

    ret = pthread_create(&child2, NULL, free_child_fn, (void *)buf_stack);
    if (ret) {
        fprintf(stderr, "Error, pthread_create() returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    printf("Parent joining child2\n");
    ret = pthread_join(child2, NULL);
    if (ret) {
        printf("Error, pthread_join returned: %d [%s]\n", ret, strerror(ret));
        return 1;
    }

    printf("Parent done\n");

    return 0;
}

