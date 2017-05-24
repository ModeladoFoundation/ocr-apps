/*
Serial version of stencil 1D
Takes two arguments

number of iterations
Grid dimension

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>



//int main (int argc, char **argv) {
int main (int argc, char *argv[]) {
#pragma ocr datablock DATABLOCK(DBK_Val, DBK_new)
    float *val, *new;

    int i, t;
    int nx, niter;

    if (argc != 3) {
        printf("Usage: %s <# iterations> < grid dimension NX> \n", *argv);
#pragma ocr shutdown DEP_EVTs()
        exit (1);
    }
    //    nx =  atoi(*++argv);
    nx = atoi(argv[1]);
    if (nx < 1) {
        printf("nx must be >= 1 : %d \n", nx );
#pragma ocr shutdown DEP_EVTs()
        exit (1);
    }
    //  niter =  atoi(*++argv);
    niter = atoi(argv[2]);
    if (niter < 1) {
        printf("niter must be >= 1 : %d \n", niter );
#pragma ocr shutdown DEP_EVTs()
        exit (1);
    }
    printf ("Perform %d iterations of 1D Stencil operation on nx = %d gridpoints\n", niter, nx);
    val = malloc (sizeof(float) * (nx + 1) );
    new = malloc (sizeof(float) * (nx + 1) );

#pragma ocr task TASK(TASK_init) DEP_DBKs(DBK_Val, DBK_new) \
  DEP_EVTs(NONE) DEP_ELEMs(nx) OEVENT(OEVT_init)
    {
      int i;
      // intialization
      for (i = 0; i < nx + 1 ; i++) {
        val [i] = 0.;
        new [i] = 0;
      }
      val [ 0 ] = 1.;
      val [ nx  - 1 ] = 1.;
    }

    // time step iteration
#pragma ocr loop TASK(TASK_timestep) DEP_DBKs(DBK_Val, DBK_new)	\
  DEP_EVTs(OEVT_init) DEP_ELEMs(niter, nx, t, i) OEVENT(OEVT_timestep)
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
#pragma ocr task TASK(TASK_print) DEP_DBKs(DBK_Val) \
  DEP_EVTs(OEVT_timestep) DEP_ELEMs(nx) OEVENT(OEVT_print)
    {
      int i;
      for (i = 0; i < nx; i++) {
        //if ( i % 5 == 0) printf ( "val [%d} =  %f\n", i, val [i] );
        printf ( "val [%d] =  %f\n", i, val [i] );
        fflush (stdout);
      }
    }
#pragma ocr shutdown DEP_EVTs(OEVT_print)
    return (0);
}
