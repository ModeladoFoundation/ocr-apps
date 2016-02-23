/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */
#include <stdio.h>
#include <mpi.h>

double dbl;
double * ptrdbl;
double ** ptrptrdbl;

int main (int argc, char **argv)
{
    int my_ID;
    int Num_procs;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_ID);
    MPI_Comm_size(MPI_COMM_WORLD, &Num_procs);

    printf ("dbl=%p, ptrdbl=%p, ptrptrdbl=%p\n", dbl, ptrdbl, ptrptrdbl);

    static int one=1;
    static int two=2;

    printf("one=%d, two=%d\n", one, two);

    return (0);
}


