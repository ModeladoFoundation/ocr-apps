#include <stdio.h>
#include <stdlib.h>

#define N 1

/**
 * Initial code: simple Fibonacci
 */
int fib(int n) {
    if(n > N) {
        return fib(n-1) + fib(n-2);
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
