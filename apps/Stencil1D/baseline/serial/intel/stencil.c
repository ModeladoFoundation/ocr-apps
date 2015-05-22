/*
Serial version of stencil 1D
Takes two arguments

number of iterations
Grid dimension

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>



int main (int argc, char **argv) {
    float *val, *new;
    int i, t;
    int nx, niter;

    if (argc != 3) {
        printf("Usage: %s <# iterations> < grid dimension NX> \n", *argv);
        exit (1);
    }
    nx =  atoi(*++argv);
    if (nx < 1) {
        printf("nx must be >= 1 : %d \n", nx );
        exit (1);
    }
    niter =  atoi(*++argv);
    if (niter < 1) {
        printf("niter must be >= 1 : %d \n", niter );
        exit (1);
    }
    printf ("Perform %d iterations of 1D Stencil operation on nx = %d gridpoints\n", niter, nx);
    val = malloc (sizeof(float) * (nx + 1) );
    new = malloc (sizeof(float) * (nx + 1) );

    // intialization
    for (i = 0; i < nx + 1 ; i++) {
        val [i] = 0.;
        new [i] = 0;
    }
    val [ 0 ] = 1.;
    val [ nx  - 1 ] = 1.;

    // time step iteration
    for (t = 0; t < niter; t++) {
        // stencil update
        for (i = 1; i < nx - 1; i++) {
            new [i] =  0.5* val [i] + 0.25 * ( val [i + 1] + val [i - 1] );
        }
        // copy values for next iteration
        for (i = 1; i < nx - 1; i++) {
         val [i] =  new [i] ;
        }
    }

    // print results
    for (i = 0; i < nx; i++) {
        //if ( i % 5 == 0) printf ( "val [%d} =  %f\n", i, val [i] );
        printf ( "val [%d] =  %f\n", i, val [i] );
        fflush (stdout);
    }
    return (0);
}


