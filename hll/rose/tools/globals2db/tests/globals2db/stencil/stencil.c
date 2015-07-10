#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define NX 18
#define NITER 10
#define LEN 64

// globals!
 float val[LEN], new[LEN], results[LEN];

int main (int argc, char **argv) {
    //    float *val, *new, *results;
    int i, t;
    int nx=NX, niter=NITER;
    int my_ID;
    int Num_procs;
    //    int leftover, jup, jlow;
    int nloc;
    MPI_Request request[4];
    MPI_Status status;
    MPI_Status istatus [4];



    /*******************************************************************************
    ** process, test, and broadcast input parameters
    ********************************************************************************/
#if 0
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
    printf("bogus message to update log message\n");
#endif
    /*******************************************************************************
          ** Initialize the MPI environment
    ********************************************************************************/
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_ID);
    MPI_Comm_size(MPI_COMM_WORLD, &Num_procs);
    /* compute amount of space required for input and solution arrays             */
    nloc = (nx - 2) / Num_procs;
    if ((nx - 2)%Num_procs != 0) {
        printf ("ERROR: NX - 2 has to be multiple of Num_procs!\n");
        return (1);
    }
    /** assume for now there are is leftover
    leftover = (nx + 1)%Num_procs;
    if (my_ID < leftover) {
        jlow = (nloc + 1) * my_ID;
        jup = jlow + nloc + 1;
    }
    else {
        jlow = (nloc + 1) * leftover + nloc * (my_ID - leftover);
        jup = jlow + nloc;
    }
    nloc = jup - jlow;
    **/

    printf ("Process %d: Perform %d iterations of 1D Stencil operation on nx = %d gridpoints\n", my_ID, niter, nloc);

#if 0
    val = malloc (sizeof(float) * (nloc + 2) );
    new = malloc (sizeof(float) * (nloc + 2) );
    if (my_ID == 0) results = malloc (sizeof(float) * nx);
#endif
    // intialization
    for (i = 0; i <= nloc + 1 ; i++) {
        val [i] = 0.;
        new [i] = 0;
    }
    if (my_ID == 0) val [ 0 ] = 1.;
    if (my_ID == Num_procs - 1) val [ nloc + 1] = 1.;

    // time step iteration
    for (t = 0; t < niter; t++) {
        MPI_Barrier(MPI_COMM_WORLD);
        // update ghost points
        if (my_ID < Num_procs - 1) {
            MPI_Isend(&(val [nloc]), 1,MPI_FLOAT, my_ID + 1, 0,
                       MPI_COMM_WORLD, &(request[0]));
            //printf ("DEBUG: Process %d sent message to %d with tag 99\n", my_ID, my_ID + 1);
        }
        if (my_ID > 0) {
             MPI_Isend(&(val [1]), 1,MPI_FLOAT, my_ID - 1, 0,
                       MPI_COMM_WORLD, &(request[2]));
            //printf ("DEBUG: Process %d sent message to  %d with tag 101\n", my_ID, my_ID - 1);
        }
        if (my_ID < Num_procs - 1) {
            //printf ("DEBUG: Process %d waiting for message from %d with tag 99\n", my_ID, my_ID + 1);
            MPI_Wait(&(request[0]), &(istatus[0]));
            //printf ("DEBUG: Process %d after wait for message from %d with tag 99\n", my_ID, my_ID + 1);
            MPI_Recv (&(val [nloc + 1]), 1, MPI_FLOAT, my_ID + 1, 0,
                       MPI_COMM_WORLD, &status);
            //printf ("DEBUG: Process %d after receive of message from %d with tag 99 val = %f\n", my_ID, my_ID + 1, val [nloc - 1]);
        }
        if (my_ID > 0) {
            //printf ("DEBUG: Process %d waiting for message from %d with tag 101\n", my_ID, my_ID - 1);
            MPI_Wait(&(request[2]), &(istatus[2]));
            //printf ("DEBUG: Process %d after wait for message from %d with tag 101\n", my_ID, my_ID - 1);
            MPI_Recv (&(val [0]), 1, MPI_FLOAT, my_ID - 1, MPI_ANY_TAG,
                       MPI_COMM_WORLD, &status);
            //printf ("DEBUG: Process %d after receive of message from %d with tag 101 val = %f\n", my_ID, my_ID - 1, val [0]);
        }


        // stencil update
        for (i = 1; i < nloc + 1; i++) {
            new [i] =  0.5*val [i] + 0.25 * ( val [i + 1] + val [i - 1] );
        }
        // copy values for next iteration
        for (i = 1; i < nloc + 1; i++) {
            val [i] =  new [i] ;
        }
    }
    {
    int id;
    for ( id = 0; id < Num_procs; id++ ) {
        if (my_ID == id) {
            for (i = 1; i < nloc + 1; i++) {
         //if ( i % 5 == 0) printf ( "val [%d} =  %f\n", i, val [i] );
                printf ( " Local results PID %d val [%d] =  %f\n", id, i, val [i] );
        //        printf ("DEBUG %d %d\n", i, nloc - 2);
                fflush (stdout);
            }
        }
    }
    if ( my_ID == 0 ) {
        results [0] = 1.;
        for (i = 1; i < Num_procs; i++ ) {
            // assume for now that nloc is the same for all ranks
            //        printf ("DEBUG %d receive results at location %d\n", my_ID, nloc, i);
            MPI_Recv (&(results [ 1 + i * nloc ]), nloc, MPI_FLOAT, i, MPI_ANY_TAG,
                        MPI_COMM_WORLD, &status);
        }
    } else {
            MPI_Send (&(val [1]), nloc, MPI_FLOAT, 0, 0,
                       MPI_COMM_WORLD);
                      //printf ("DEBUG %d sent results \n", my_ID);
    }
    // print results
    //printf ("DEBUG %d Before output\n", my_ID);
    fflush (stdout);
    if ( my_ID == 0 ) {
        results [0] = 1;
        results [nx - 1] = 1.;
        for (i = 1; i < nloc + 1; i++) {
            results [i] = val [i];
            //printf ( "results [%d] =  %f\n", i, val [i]);
        }
        for (i = 0; i < nx; i++) {
            printf ( "results [%d] =  %f\n", i, results [i] );
            fflush (stdout);
        }
    }
    }
    return (0);
}


