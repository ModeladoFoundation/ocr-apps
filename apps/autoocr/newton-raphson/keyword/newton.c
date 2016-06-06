#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LIMIT 0.0001
#define MAX_ITER 25

#define __task __attribute__((ocrtask))

double __task guess(double base, double num, double x0) {
    return x0 - (pow(x0, base) - num)/(base*pow(x0, base-1));
}

double newton(double base, double num) {
    int i = 0;
    double x1;
    double x0 = num / 2;

    while (i < MAX_ITER) {
        x1 = guess(base, num, x0);

        if ( fabs(x0-x1) < LIMIT )
            break;
        else {
            i++;
            x0 = x1;
        }
    }
    return x1;
}

int main (int argc, char **argv) {
    double base = atof(argv[1]);
    double num  = atof(argv[2]);

    if (num <= 0)
        return 0;

    double result = newton(base, num);
    printf("Result: %5.10f\n", result);

    return 0;
}
