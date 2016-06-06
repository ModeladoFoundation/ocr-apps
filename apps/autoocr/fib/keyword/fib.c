#include <stdio.h>
#include <stdlib.h>

#define __task __attribute__((ocrtask))

#define N 1

/**
 * Initial code: simple Fibonacci
 */
int __task fib(int n) {
    if(n > N) {
        int n1 = fib(n-1);
        int n2 = fib(n-2);
        return (n1 +n2);
    } else {
        return n;
    }
}

int main(int argc, char* argv[]) {
    int initN = atoi(argv[1]);
    int result = fib(initN);
    printf("Result: fib(%d) = %d\n", initN, result);
    return 0;
}
