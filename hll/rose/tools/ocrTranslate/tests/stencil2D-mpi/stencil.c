/*
Copyright (c) 2013, Intel Corporation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
* Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

/*******************************************************************

NAME:    Stencil

PURPOSE: This program tests the efficiency with which a space-invariant,
         linear, symmetric filter (stencil) can be applied to a square
         grid or image.

USAGE:   The program takes as input the linear dimension of the grid,
         and the number of iterations on the grid

               <progname> <# iterations> <grid size>

         The output consists of diagnostics to make sure the
         algorithm worked, and of timing statistics.

FUNCTIONS CALLED:

         Other than MPI or standard C functions, the following
         functions are used in this program:

         wtime()
         bail_out()

HISTORY: - Written by Rob Van der Wijngaart, November 2006.
         - RvdW, August 2013: Removed unrolling pragmas for clarity;
           fixed bug in compuation of width of strip assigned to
           each rank;
         - RvdW, August 2013: added constant to array "in" at end of
           each iteration to force refreshing of neighbor data in
           parallel versions
         - RvdW, October 2014: introduced 2D domain decomposition
         - RvdW, October 2014: removed barrier at start of each iteration
         - RvdW, October 2014: replaced single rank/single iteration timing
           with global timing of all iterations across all ranks

*********************************************************************************/

#include "par-res-kern_general.h"
#include "par-res-kern_mpi.h"

#ifndef RADIUS
  #define RADIUS 2
#endif

#ifdef DOUBLE_USER
  #define DTYPE     double
  #define MPI_DTYPE MPI_DOUBLE
  #define EPSILON   1.e-8
  #define COEFX     1.0
  #define COEFY     1.0
  #define FSTR      "%lf"
#else
  #define DTYPE     float
  #define MPI_DTYPE MPI_FLOAT
  #define EPSILON   0.0001f
  #define COEFX     1.0f
  #define COEFY     1.0f
  #define FSTR      "%f"
#endif

/* define shorthand for indexing multi-dimensional arrays with offsets           */
#define INDEXIN(i,j)  (i+RADIUS+(j+RADIUS)*(width+2*RADIUS))
/* need to add offset of RADIUS to j to account for ghost points                 */
#define IN(i,j)       in[INDEXIN(i-istart,j-jstart)]
#define INDEXOUT(i,j) (i+(j)*(width))
#define OUT(i,j)      out[INDEXOUT(i-istart,j-jstart)]
#define WEIGHT(ii,jj) weight[ii+RADIUS][jj+RADIUS]

int main(int argc, char* argv[]) {
  int    n;
  int    iterations;      /* number of times to run the algorithm                */
  int    error=0;         /* error flag                                          */
  int ntasks;             /* used by ocrtranslator */
  /*******************************************************************************
  ** Initialize the MPI environment
  ********************************************************************************/
  MPI_Init(&argc,&argv);

  /*******************************************************************************
  ** process, test, and broadcast input parameters
  ********************************************************************************/

#ifndef STAR
  printf("ERROR: Compact stencil not supported\n");
  error = 1;
  goto BAIL;
#endif

#ifdef OCRTRANSLATE
  /* Using command line to pass ntasks to the annotation */
  if (argc != 4){
    printf("Usage: %s <# ntasks> <# iterations> <array dimension> \n",
	   *argv);
    error = 1;
    bail_out(error);
  }

  ntasks = atoi(argv[1]);
  iterations = atoi(argv[2]);
  if (iterations < 1){
    printf("ERROR: iterations must be >= 1 : %d \n",iterations);
    error = 1;
    goto BAIL;
  }

  n = atoi(argv[3]);
#else
  if (argc != 3){
    printf("Usage: %s <# iterations> <array dimension> \n",
	   *argv);
    error = 1;
    goto BAIL;
  }
  iterations = atoi(argv[1]);
  if (iterations < 1){
    printf("ERROR: iterations must be >= 1 : %d \n",iterations);
    error = 1;
    goto BAIL;
  }
  n = atoi(argv[2]);
#endif

  if (RADIUS < 0) {
    printf("ERROR: Stencil radius %d should be non-negative\n", RADIUS);
    error = 1;
    goto BAIL;
  }

  if (2*RADIUS +1 > n) {
    printf("ERROR: Stencil radius %d exceeds grid size %d\n", RADIUS, n);
    error = 1;
    goto BAIL;
  }

 BAIL:;
  bail_out(error);

#pragma ocr spmd region  NTASKS(ntasks) DEP_DBKs() DEP_EVTs() \
  DEP_ELEMs(n, iterations, error) OEVENT(OEVT_spmd)
  {
    int    Num_procs;       /* number of ranks                                     */
    int    Num_procsx, Num_procsy; /* number of ranks in each coord direction      */
    int    my_ID;           /* MPI rank                                            */
    int    my_IDx, my_IDy;  /* coordinates of rank in rank grid                    */
    int    right_nbr;       /* global rank of right neighboring tile               */
    int    left_nbr;        /* global rank of left neighboring tile                */
    int    top_nbr;         /* global rank of top neighboring tile                 */
    int    bottom_nbr;      /* global rank of bottom neighboring tile              */
#pragma ocr datablock DATABLOCK(DBK_TOP_BUF_OUT, DBK_TOP_BUF_IN, DBK_BOTTOM_BUF_OUT, DBK_BOTTOM_BUF_IN)
    DTYPE *top_buf_out;     /* communication buffer                                */
    DTYPE *top_buf_in;      /*       "         "                                   */
    DTYPE *bottom_buf_out;  /*       "         "                                   */
    DTYPE *bottom_buf_in;   /*       "         "                                   */

#pragma ocr datablock DATABLOCK(DBK_RIGHT_BUF_OUT, DBK_RIGHT_BUF_IN, DBK_LEFT_BUF_OUT, DBK_LEFT_BUF_IN)
    DTYPE *right_buf_out;   /*       "         "                                   */
    DTYPE *right_buf_in;    /*       "         "                                   */
    DTYPE *left_buf_out;    /*       "         "                                   */
    DTYPE *left_buf_in;     /*       "         "                                   */

    int    root = 0;

    int leftover;
    int iter;
    int width, height;/* linear global and local grid dimension              */
    long   nsquare;         /* total number of grid points                         */
    int    istart, iend;    /* bounds of grid tile assigned to calling rank        */
    int    jstart, jend;    /* bounds of grid tile assigned to calling rank        */
    DTYPE  norm,            /* L1 norm of solution                                 */
      local_norm,      /* contribution of calling rank to L1 norm             */
      reference_norm;
    DTYPE  f_active_points; /* interior of grid with respect to stencil            */


#pragma ocr datablock DATABLOCK(DBK_IN, DBK_OUT)
    DTYPE  * RESTRICT in;   /* input grid values                                   */
    DTYPE  * RESTRICT out;  /* output grid values                                  */

    long   total_length_in; /* total required length to store input array          */
    long   total_length_out;/* total required length to store output array         */

#pragma ocr datablock DATABLOCK(DBK_WEIGHT)
    DTYPE  weight[2*RADIUS+1][2*RADIUS+1]; /* weights of points in the stencil     */

    MPI_Request request[8];
    MPI_Status  status[8];

    MPI_Comm_rank(MPI_COMM_WORLD, &my_ID);
    MPI_Comm_size(MPI_COMM_WORLD, &Num_procs);

    if (my_ID == root) {
      nsquare = (long) n * n;
      if (nsquare < Num_procs){
	printf("ERROR: grid size %d must be at least # ranks: %ld\n",
	       nsquare, Num_procs);
	error = 1;
	goto ENDOFTESTS;
      }

    ENDOFTESTS:;
    }
    bail_out(error);

    /* determine best way to create a 2D grid of ranks (closest to square, for
       best surface/volume ratio); we do this brute force for now
    */
    for (Num_procsx=(int) (sqrt(Num_procs+1)); Num_procsx>0; Num_procsx--) {
      if (!(Num_procs%Num_procsx)) {
	Num_procsy = Num_procs/Num_procsx;
	break;
      }
    }
    my_IDx = my_ID%Num_procsx;
    my_IDy = my_ID/Num_procsx;
    /* compute neighbors; don't worry about dropping off the edges of the grid */
    right_nbr  = my_ID+1;
    left_nbr   = my_ID-1;
    top_nbr    = my_ID+Num_procsx;
    bottom_nbr = my_ID-Num_procsx;

    if (my_ID == root) {
#pragma ocr task TASK(TASK_PrintStencilInfo) DEP_DBKs() DEP_EVTs() \
  DEP_ELEMs(Num_procs, n, Num_procsx, Num_procsy, iterations) OEVENT(OEVT_printStencilInfo)
      {
	printf("MPI stencil execution on 2D grid\n");
	printf("Number of ranks        = %d\n", Num_procs);
	printf("Grid size              = %d\n", n);
	printf("Radius of stencil      = %d\n", RADIUS);
	printf("Tiles in x/y-direction = %d/%d\n", Num_procsx, Num_procsy);
	printf("Type of stencil        = star\n");
#ifdef DOUBLE_USER
	printf("Data type              = double precision\n");
#else
	printf("Data type              = single precision\n");
#endif
	printf("Number of iterations   = %d\n", iterations);
      }
    }

    // MPI_Bcast(&n,          1, MPI_INT, root, MPI_COMM_WORLD);
    // MPI_Bcast(&iterations, 1, MPI_INT, root, MPI_COMM_WORLD);

    /* compute amount of space required for input and solution arrays             */

    width = n/Num_procsx;
    leftover = n%Num_procsx;
    if (my_IDx<leftover) {
      istart = (width+1) * my_IDx;
      iend = istart + width;
    }
    else {
      istart = (width+1) * leftover + width * (my_IDx-leftover);
      iend = istart + width - 1;
    }

    width = iend - istart + 1;
    height = n/Num_procsy;
    leftover = n%Num_procsy;
    if (my_IDy<leftover) {
      jstart = (height+1) * my_IDy;
      jend = jstart + height;
    }
    else {
      jstart = (height+1) * leftover + height * (my_IDy-leftover);
      jend = jstart + height - 1;
    }

    height = jend - jstart + 1;
    total_length_in = (width+2*RADIUS)*(height+2*RADIUS)*sizeof(DTYPE);
    total_length_out = width*height*sizeof(DTYPE);
    in  = (DTYPE *) malloc(total_length_in);
    out = (DTYPE *) malloc(total_length_out);

#pragma ocr task TASK(TASK_ErrorCheck) DEP_DBKs(DBK_IN, DBK_OUT) DEP_EVTs() \
  DEP_ELEMs(width, error, my_ID, height,  total_length_in) OEVENT(OEVT_errorcheck)
    {
      if (width == 0) {
	printf("ERROR: rank %d has no work to do\n", my_ID);
	error = 1;
      }
      bail_out(error);

      if (height == 0) {
	printf("ERROR: rank %d has no work to do\n", my_ID);
	error = 1;
      }
      bail_out(error);

      if (width < RADIUS || height < RADIUS) {
	printf("ERROR: rank %d has work tile smaller then stencil radius\n",
	       my_ID);
	error = 1;
      }
      bail_out(error);

      if (total_length_in/(height+2*RADIUS) != (width+2*RADIUS)*sizeof(DTYPE)) {
	printf("ERROR: Space for %d x %d input array cannot be represented\n",
	       width+2*RADIUS, height+2*RADIUS);
	error = 1;
      }
      bail_out(error);

      if (!in || !out) {
	printf("ERROR: rank %d could not allocate space for input/output array\n",
	       my_ID);
	error = 1;
      }
      bail_out(error);
    } // End TASK_ErrorCheck

    norm = (DTYPE) 0.0;
    f_active_points = (DTYPE) (n-2*RADIUS)*(DTYPE) (n-2*RADIUS);


#pragma ocr task TASK(TASK_Init) DEP_DBKs(DBK_IN, DBK_OUT, DBK_WEIGHT) DEP_EVTs(OEVT_errorcheck) \
  DEP_ELEMs(jstart, jend, istart, iend, width) OEVENT(OEVT_init)
    {
      int ii, jj, i, j;
      /* fill the stencil weights to reflect a discrete divergence operator         */
      for (jj=-RADIUS; jj<=RADIUS; jj++) for (ii=-RADIUS; ii<=RADIUS; ii++)
					   WEIGHT(ii,jj) = (DTYPE) 0.0;

      for (ii=1; ii<=RADIUS; ii++) {
	WEIGHT(0, ii) = WEIGHT( ii,0) =  (DTYPE) (1.0/(2.0*ii*RADIUS));
	WEIGHT(0,-ii) = WEIGHT(-ii,0) = -(DTYPE) (1.0/(2.0*ii*RADIUS));
      }

      /* intialize the input and output arrays                                     */
      for (j=jstart; j<=jend; j++) for (i=istart; i<=iend; i++) {
	  IN(i,j)  = COEFX*i+COEFY*j;
	  OUT(i,j) = (DTYPE)0.0;
	}
    }

    /* allocate communication buffers for halo values                            */
    top_buf_out = (DTYPE *) malloc(sizeof(DTYPE)*RADIUS*width);
    top_buf_in     = (DTYPE *) malloc(sizeof(DTYPE)*RADIUS*width);
    bottom_buf_out = (DTYPE *) malloc(sizeof(DTYPE)*RADIUS*width);
    bottom_buf_in  = (DTYPE *) malloc(sizeof(DTYPE)*RADIUS*width);

    right_buf_out  = (DTYPE *) malloc(sizeof(DTYPE)*RADIUS*height);
    right_buf_in   = (DTYPE *) malloc(sizeof(DTYPE)*RADIUS*height);
    left_buf_out   = (DTYPE *) malloc(sizeof(DTYPE)*RADIUS*height);
    left_buf_in    = (DTYPE *) malloc(sizeof(DTYPE)*RADIUS*height);

    if (!top_buf_out || !top_buf_in ||
	!bottom_buf_in || !bottom_buf_out) {
      printf("ERROR: Rank %d could not allocated comm buffers for y-direction\n", my_ID);
      error = 1;
    }
    bail_out(error);

    if (!right_buf_out) {
      printf("ERROR: Rank %d could not allocated comm buffers for x-direction\n", my_ID);
      error = 1;
    }
    bail_out(error);

    #pragma ocr datablock DATABLOCK(DBK_LOCAL_STENCIL_TIME)
    double local_stencil_time[2];

    double stencil_time;

    #pragma ocr datablock DATABLOCK(DBK_TOP_BUF_OUT2)
    DTYPE *top_buf_out2;
    top_buf_out2 = (DTYPE *) malloc(sizeof(DTYPE)*RADIUS*width);

    #pragma ocr datablock DATABLOCK(DBK_BOTTOM_BUF_OUT2)
    DTYPE *bottom_buf_out2;
    bottom_buf_out2 = (DTYPE *) malloc(sizeof(DTYPE)*RADIUS*width);

    #pragma ocr datablock DATABLOCK(DBK_RIGHT_BUF_OUT2)
    DTYPE *right_buf_out2;
    right_buf_out2 = (DTYPE *) malloc(sizeof(DTYPE)*RADIUS*width);

    #pragma ocr datablock DATABLOCK(DBK_LEFT_BUF_OUT2)
    DTYPE *left_buf_out2;
    left_buf_out2 = (DTYPE *) malloc(sizeof(DTYPE)*RADIUS*width);


    #pragma ocr loop TASK(TASK_Iteration) \
      DEP_DBKs(DBK_IN, DBK_OUT, DBK_WEIGHT, \
               DBK_TOP_BUF_OUT, DBK_BOTTOM_BUF_OUT, DBK_LEFT_BUF_OUT, DBK_RIGHT_BUF_OUT, \
	       DBK_TOP_BUF_OUT2, DBK_BOTTOM_BUF_OUT2, DBK_RIGHT_BUF_OUT2, DBK_LEFT_BUF_OUT2, \
	       DBK_LOCAL_STENCIL_TIME) \
      DEP_EVTs(OEVT_init)						\
      DEP_ELEMs(n, iter, iterations, my_ID, my_IDy, my_IDx, Num_procsy, Num_procsx, height, width, top_nbr, bottom_nbr, left_nbr, right_nbr, istart, iend, , jstart, jend) \
      OEVENT(OEVT_iterations)
    for (iter = 0; iter<=iterations; iter++) {
      /* start timer after a warmup iteration */

      if (iter == 1) {
      	// MPI_Barrier(MPI_COMM_WORLD);
      	local_stencil_time[0] = wtime();
      }

#pragma ocr task TASK(TASK_HaloExchange)				\
  DEP_DBKs(DBK_IN, DBK_TOP_BUF_OUT, DBK_BOTTOM_BUF_OUT, DBK_LEFT_BUF_OUT, DBK_RIGHT_BUF_OUT, DBK_TOP_BUF_OUT2, DBK_BOTTOM_BUF_OUT2, DBK_RIGHT_BUF_OUT2, DBK_LEFT_BUF_OUT2) \
  DEP_EVTs()								\
  DEP_ELEMs(iter, my_IDy, my_IDx, Num_procsy, Num_procsx, height, width, top_nbr, bottom_nbr, left_nbr, right_nbr, istart, iend, jstart, jend) \
  OEVENT(OEVT_haloexchange)
      {
#pragma ocr task TASK(TASK_HaloExchangeTop) \
  DEP_DBKs(DBK_IN, DBK_TOP_BUF_OUT, DBK_TOP_BUF_OUT2)	    \
  DEP_EVTs() \
  DEP_ELEMs(iter, my_IDy, Num_procsy, iend, istart, jend, jstart, width, top_nbr) \
  OEVENT(OEVT_haloexchangetop)
	{
	  int i, ii, j, jj, k, kk;
	  /* need to fetch ghost point data from neighbors in y-direction                 */
	  if (my_IDy < Num_procsy-1) {
	    if(iter % 2 == 0) {
              #pragma ocr spmd recv RECV_EVT(EVT_TOP_BUF_IN) DEP_EVTs() OEVENT(OEVT_rtopbufin)
	      MPI_Irecv(top_buf_in, RADIUS*width, MPI_DTYPE, top_nbr, 2101,
			MPI_COMM_WORLD, &(request[1]));

	      for (kk=0,j=jend-RADIUS+1; j<=jend; j++) for (i=istart; i<=iend; i++) {
		  top_buf_out[kk++]= IN(i,j);
		}
              #pragma ocr spmd send SEND_DBK(DBK_TOP_BUF_OUT) DEP_EVTs() OEVENT(OEVT_stopbufout)
	      MPI_Isend(top_buf_out, RADIUS*width,MPI_DTYPE, top_nbr, 299,
			MPI_COMM_WORLD, &(request[0]));

              #pragma ocr task TASK(TASK_TopBufCopyEven) DEP_DBKs(DBK_IN) DEP_EVTs(EVT_TOP_BUF_IN, OEVT_stopbufout) \
		DEP_ELEMs(my_IDy, Num_procsy, iend, istart, jend, jstart, width) OEVENT(OEVT_topbufcopy)
	      {
		int kk, j, i;
		MPI_Wait(&(request[0]), &(status[0]));
		MPI_Wait(&(request[1]), &(status[1]));
		for (kk=0,j=jend+1; j<=jend+RADIUS; j++) for (i=istart; i<=iend; i++) {
		    IN(i,j) = top_buf_in[kk++];
		  }
	      }
	    } // End even iteration
	    else {
	      #pragma ocr spmd recv RECV_EVT(EVT_TOP_BUF_IN) DEP_EVTs() OEVENT(OEVT_rtopbufin)
	      MPI_Irecv(top_buf_in, RADIUS*width, MPI_DTYPE, top_nbr, 1101,
			MPI_COMM_WORLD, &(request[1]));

	      for (kk=0,j=jend-RADIUS+1; j<=jend; j++) for (i=istart; i<=iend; i++) {
		  top_buf_out2[kk++]= IN(i,j);
		}
              #pragma ocr spmd send SEND_DBK(DBK_TOP_BUF_OUT2) DEP_EVTs() OEVENT(OEVT_stopbufout)
	      MPI_Isend(top_buf_out2, RADIUS*width,MPI_DTYPE, top_nbr, 199,
			MPI_COMM_WORLD, &(request[0]));

              #pragma ocr task TASK(TASK_TopBufCopyOdd) DEP_DBKs(DBK_IN) DEP_EVTs(EVT_TOP_BUF_IN, OEVT_stopbufout) \
		DEP_ELEMs(my_IDy, Num_procsy, iend, istart, jend, jstart, width) OEVENT(OEVT_topbufcopy)
	      {
		int kk, j, i;
		MPI_Wait(&(request[0]), &(status[0]));
		MPI_Wait(&(request[1]), &(status[1]));
		for (kk=0,j=jend+1; j<=jend+RADIUS; j++) for (i=istart; i<=iend; i++) {
		    IN(i,j) = top_buf_in[kk++];
		  }
	      }
	    } // End odd iteration
	  }
	} // End HaloExchangeTop

#pragma ocr task TASK(TASK_HaloExchangeBottom) \
  DEP_DBKs(DBK_IN, DBK_BOTTOM_BUF_OUT, DBK_BOTTOM_BUF_OUT2)	       \
  DEP_EVTs() \
  DEP_ELEMs(iter, my_IDy, width, bottom_nbr, jstart, istart, iend)	\
  OEVENT(OEVT_haloexchangebottom)
	{
	  int i, ii, j, jj, k, kk;
	  if (my_IDy > 0) {
	    if(iter % 2 == 0) {
              #pragma ocr spmd recv RECV_EVT(EVT_BOTTOM_BUF_IN) DEP_EVTs() OEVENT(OEVT_rbottombufin)
	      MPI_Irecv(bottom_buf_in,RADIUS*width, MPI_DTYPE, bottom_nbr, 299,
			MPI_COMM_WORLD, &(request[3]));
	      for (kk=0,j=jstart; j<=jstart+RADIUS-1; j++) for (i=istart; i<=iend; i++) {
		  bottom_buf_out[kk++]= IN(i,j);
		}
              #pragma ocr spmd send SEND_DBK(DBK_BOTTOM_BUF_OUT) DEP_EVTs() OEVENT(OEVT_sbottombufout)
	      MPI_Isend(bottom_buf_out, RADIUS*width,MPI_DTYPE, bottom_nbr, 2101,
			MPI_COMM_WORLD, &(request[2]));

              #pragma ocr task TASK(TASK_BottomBufCopyEven) DEP_DBKs(DBK_IN) DEP_EVTs(EVT_BOTTOM_BUF_IN, OEVT_sbottombufout) \
		DEP_ELEMs(my_IDy, jstart, istart, iend, width) OEVENT(OEVT_bottombufcopy)
	      {
		int kk, j, i;
		MPI_Wait(&(request[2]), &(status[2]));
		MPI_Wait(&(request[3]), &(status[3]));
		for (kk=0,j=jstart-RADIUS; j<=jstart-1; j++) for (i=istart; i<=iend; i++) {
		    IN(i,j) = bottom_buf_in[kk++];
		  }
	      }
	    } // End even iteration
	    else {
	      #pragma ocr spmd recv RECV_EVT(EVT_BOTTOM_BUF_IN) DEP_EVTs() OEVENT(OEVT_rbottombufin)
	      MPI_Irecv(bottom_buf_in,RADIUS*width, MPI_DTYPE, bottom_nbr, 199,
			MPI_COMM_WORLD, &(request[3]));
	      for (kk=0,j=jstart; j<=jstart+RADIUS-1; j++) for (i=istart; i<=iend; i++) {
		  bottom_buf_out2[kk++]= IN(i,j);
		}
              #pragma ocr spmd send SEND_DBK(DBK_BOTTOM_BUF_OUT2) DEP_EVTs() OEVENT(OEVT_sbottombufout)
	      MPI_Isend(bottom_buf_out2, RADIUS*width,MPI_DTYPE, bottom_nbr, 1101,
			MPI_COMM_WORLD, &(request[2]));

              #pragma ocr task TASK(TASK_BottomBufCopyOdd) DEP_DBKs(DBK_IN) DEP_EVTs(EVT_BOTTOM_BUF_IN, OEVT_sbottombufout) \
		DEP_ELEMs(my_IDy, jstart, istart, iend, width) OEVENT(OEVT_bottombufcopy)
	      {
		int kk, j, i;
		MPI_Wait(&(request[2]), &(status[2]));
		MPI_Wait(&(request[3]), &(status[3]));
		for (kk=0,j=jstart-RADIUS; j<=jstart-1; j++) for (i=istart; i<=iend; i++) {
		    IN(i,j) = bottom_buf_in[kk++];
		  }
	      }
	    } // End odd iteration
	  }
	} // End HaloExchangeBottom

#pragma ocr task TASK(TASK_HaloExchangeRight) \
  DEP_DBKs(DBK_IN, DBK_RIGHT_BUF_OUT, DBK_RIGHT_BUF_OUT2)	      \
  DEP_EVTs(OEVT_haloexchangetop, OEVT_haloexchangebottom)					\
  DEP_ELEMs(iter, my_IDx, Num_procsx, right_nbr, jstart, jend, istart, iend, width, height) \
  OEVENT(OEVT_haloexchangeright)
	{
	  int i, ii, j, jj, k, kk;
	  /* need to fetch ghost point data from neighbors in x-direction                 */
	  if (my_IDx < Num_procsx-1) {
	    if(iter % 2 == 0) {
              #pragma ocr spmd recv RECV_EVT(EVT_RIGHT_BUF_IN) DEP_EVTs() OEVENT(OEVT_rrightbufin)
	      MPI_Irecv(right_buf_in, RADIUS*height, MPI_DTYPE, right_nbr, 21010,
			MPI_COMM_WORLD, &(request[1+4]));
	      for (kk=0,j=jstart; j<=jend; j++) for (i=iend-RADIUS+1; i<=iend; i++) {
		  right_buf_out[kk++]= IN(i,j);
		}

              #pragma ocr spmd send SEND_DBK(DBK_RIGHT_BUF_OUT) DEP_EVTs() OEVENT(OEVT_srightbufout)
	      MPI_Isend(right_buf_out, RADIUS*height, MPI_DTYPE, right_nbr, 2990,
			MPI_COMM_WORLD, &(request[0+4]));

              #pragma ocr task TASK(TASK_RightBufCopyEven) DEP_DBKs(DBK_IN) DEP_EVTs(OEVT_srightbufout, EVT_RIGHT_BUF_IN) \
		DEP_ELEMs(my_IDx, Num_procsx, jstart, jend, iend, istart, width) OEVENT(OEVT_rightbufcopy)
	      {
		int kk, i, j;
		MPI_Wait(&(request[0+4]), &(status[0+4]));
		MPI_Wait(&(request[1+4]), &(status[1+4]));
		for (kk=0,j=jstart; j<=jend; j++) for (i=iend+1; i<=iend+RADIUS; i++) {
		    IN(i,j) = right_buf_in[kk++];
		  }
	      }
	    } // end even iteration
	    else {
              #pragma ocr spmd recv RECV_EVT(EVT_RIGHT_BUF_IN) DEP_EVTs() OEVENT(OEVT_rrightbufin)
	      MPI_Irecv(right_buf_in, RADIUS*height, MPI_DTYPE, right_nbr, 11010,
			MPI_COMM_WORLD, &(request[1+4]));
	      for (kk=0,j=jstart; j<=jend; j++) for (i=iend-RADIUS+1; i<=iend; i++) {
		  right_buf_out2[kk++]= IN(i,j);
		}

              #pragma ocr spmd send SEND_DBK(DBK_RIGHT_BUF_OUT2) DEP_EVTs() OEVENT(OEVT_srightbufout)
	      MPI_Isend(right_buf_out2, RADIUS*height, MPI_DTYPE, right_nbr, 1990,
			MPI_COMM_WORLD, &(request[0+4]));

              #pragma ocr task TASK(TASK_RightBufCopyOdd) DEP_DBKs(DBK_IN) DEP_EVTs(OEVT_srightbufout, EVT_RIGHT_BUF_IN) \
		DEP_ELEMs(my_IDx, Num_procsx, jstart, jend, iend, istart, width) OEVENT(OEVT_rightbufcopy)
	      {
		int kk, i, j;
		MPI_Wait(&(request[0+4]), &(status[0+4]));
		MPI_Wait(&(request[1+4]), &(status[1+4]));
		for (kk=0,j=jstart; j<=jend; j++) for (i=iend+1; i<=iend+RADIUS; i++) {
		    IN(i,j) = right_buf_in[kk++];
		  }
	      }
	    } // end odd iteration
	  }
	} // End HaloExchangeRight

#pragma ocr task TASK(TASK_HaloExchangeLeft) \
  DEP_DBKs(DBK_IN, DBK_LEFT_BUF_OUT, DBK_LEFT_BUF_OUT2)	     \
  DEP_EVTs(OEVT_haloexchangetop, OEVT_haloexchangebottom) \
  DEP_ELEMs(iter, my_IDx, height, left_nbr, jstart, jend, istart, width) \
  OEVENT(OEVT_haloexchangeleft)
	{
	  int i, ii, j, jj, k, kk;
	  if (my_IDx > 0) {
	    if(iter % 2 == 0) {
              #pragma ocr spmd recv RECV_EVT(EVT_LEFT_BUF_IN) DEP_EVTs() OEVENT(OEVT_rleftbufin)
	      MPI_Irecv(left_buf_in, RADIUS*height, MPI_DTYPE, left_nbr, 2990,
			MPI_COMM_WORLD, &(request[3+4]));

	      for (kk=0,j=jstart; j<=jend; j++) for (i=istart; i<=istart+RADIUS-1; i++) {
		  left_buf_out[kk++]= IN(i,j);
		}

              #pragma ocr spmd send SEND_DBK(DBK_LEFT_BUF_OUT) DEP_EVTs() OEVENT(OEVT_sleftbufout)
	      MPI_Isend(left_buf_out, RADIUS*height, MPI_DTYPE, left_nbr, 21010,
			MPI_COMM_WORLD, &(request[2+4]));

              #pragma ocr task TASK(TASK_LeftBufCopyEven) DEP_DBKs(DBK_IN) DEP_EVTs(OEVT_sleftbufout, EVT_LEFT_BUF_IN) \
		DEP_ELEMs(jstart, jend, istart, my_IDx, width) OEVENT(OEVT_leftbufcopy)
	      {
		int kk, i, j;
		MPI_Wait(&(request[2+4]), &(status[2+4]));
		MPI_Wait(&(request[3+4]), &(status[3+4]));
		for (kk=0,j=jstart; j<=jend; j++) for (i=istart-RADIUS; i<=istart-1; i++) {
		    IN(i,j) = left_buf_in[kk++];
		  }
	      }
	    } // end even iteration
	    else {
	      #pragma ocr spmd recv RECV_EVT(EVT_LEFT_BUF_IN) DEP_EVTs() OEVENT(OEVT_rleftbufin)
	      MPI_Irecv(left_buf_in, RADIUS*height, MPI_DTYPE, left_nbr, 1990,
			MPI_COMM_WORLD, &(request[3+4]));

	      for (kk=0,j=jstart; j<=jend; j++) for (i=istart; i<=istart+RADIUS-1; i++) {
		  left_buf_out2[kk++]= IN(i,j);
		}

              #pragma ocr spmd send SEND_DBK(DBK_LEFT_BUF_OUT2) DEP_EVTs() OEVENT(OEVT_sleftbufout)
	      MPI_Isend(left_buf_out2, RADIUS*height, MPI_DTYPE, left_nbr, 11010,
			MPI_COMM_WORLD, &(request[2+4]));

              #pragma ocr task TASK(TASK_LeftBufCopyOdd) DEP_DBKs(DBK_IN) DEP_EVTs(OEVT_sleftbufout, EVT_LEFT_BUF_IN) \
		DEP_ELEMs(jstart, jend, istart, my_IDx, width) OEVENT(OEVT_leftbufcopy)
	      {
		int kk, i, j;
		MPI_Wait(&(request[2+4]), &(status[2+4]));
		MPI_Wait(&(request[3+4]), &(status[3+4]));
		for (kk=0,j=jstart; j<=jend; j++) for (i=istart-RADIUS; i<=istart-1; i++) {
		    IN(i,j) = left_buf_in[kk++];
		  }
	      }
	    } // end odd iteration
	  }
	} // End HaloExchangeLeft
      } // End Task_HaloExchange

#pragma ocr task TASK(TASK_StencilOp) DEP_DBKs(DBK_IN, DBK_OUT, DBK_WEIGHT) \
  DEP_EVTs(OEVT_haloexchange) DEP_ELEMs(jstart, jend, istart, iend, n, width) OEVENT(OEVT_stencilOp)
      {
    	int i, j, ii, jj;
    	/* Apply the stencil operator */
    	for (j=MAX(jstart,RADIUS); j<=MIN(n-RADIUS-1,jend); j++) {
    	  for (i=MAX(istart,RADIUS); i<=MIN(n-RADIUS-1,iend); i++) {
    	    for (jj=-RADIUS; jj<=RADIUS; jj++) {
    	      OUT(i,j) += WEIGHT(0,jj)*IN(i,j+jj);
    	    }
    	    for (ii=-RADIUS; ii<0; ii++) {
    	      OUT(i,j) += WEIGHT(ii,0)*IN(i+ii,j);
    	    }
    	    for (ii=1; ii<=RADIUS; ii++) {
    	      OUT(i,j) += WEIGHT(ii,0)*IN(i+ii,j);

    	    }
    	  }
    	}
    	/* add constant to solution to force refresh of neighbor data, if any */
    	for (j=jstart; j<=jend; j++) for (i=istart; i<=iend; i++) IN(i,j)+= 1.0;
      } // End StencilOp Task
    } /* end of iterations */

    /* compute L1 norm in parallel */
    #pragma ocr task TASK(TASK_ComputeNorm) \
      DEP_DBKs(DBK_OUT, DBK_LOCAL_STENCIL_TIME)		    \
      DEP_EVTs(OEVT_iterations) \
      DEP_ELEMs(jstart, jend, istart, iend, n, root, my_ID, iterations, width, my_IDx, my_IDy, f_active_points) \
      OEVENT(OEVT_computenorm)
    {
      local_stencil_time[1] = wtime();
      DTYPE local_norm = (DTYPE) 0.0;
      DTYPE norm = (DTYPE) 0.0;
      int i, j;

      for (j=MAX(jstart,RADIUS); j<=MIN(n-RADIUS-1,jend); j++) {
	for (i=MAX(istart,RADIUS); i<=MIN(n-RADIUS-1,iend); i++) {
	  local_norm += (DTYPE)ABS(OUT(i,j));
	}
      }

#ifdef VERBOSE
      printf("IDx=%d, IDy=%d, local_norm= "FSTR"\n", my_IDx, my_IDy, local_norm);
#endif

      #pragma ocr spmd reduce REDUCE_EVT(EVT_REDUCE_NORM) DEP_EVTs() OEVENT(OEVT_reduce)
      MPI_Reduce(&local_norm, &norm, 1, MPI_DTYPE, MPI_SUM, root, MPI_COMM_WORLD);

    /*******************************************************************************
     ** Analyze and output results.
     ********************************************************************************/

      if (my_ID == root) {
        #pragma ocr task TASK(TASK_Verify) DEP_DBKs() DEP_EVTs(EVT_REDUCE_NORM) \
	  DEP_ELEMs(my_ID, root, iterations, n, f_active_points) \
	  OEVENT(OEVT_verify)
	{
	  int error = 0;
	  DTYPE reference_norm;
	  /* verify correctness */

	  norm /= f_active_points;
	  if (RADIUS > 0) {
	    reference_norm = (DTYPE) (iterations+1) * (COEFX + COEFY);
	  }
	  else {
	    reference_norm = (DTYPE) 0.0;
	  }
	  if (ABS(norm-reference_norm) > EPSILON) {
	    printf("ERROR: L1 norm = "FSTR", Reference L1 norm = "FSTR"\n",
		   norm, reference_norm);
	    error = 1;
	  }
	  else {
	    printf("SUCCESS: L1 norm = "FSTR", Reference L1 norm = "FSTR"\n",
		   norm, reference_norm);
	    printf("Solution validates\n");
#ifdef VERBOSE
	    printf("Reference L1 norm = "FSTR", L1 norm = "FSTR"\n",
		   reference_norm, norm);
#endif
	  }
	  bail_out(error);
	}
      } // End Verify Task
    } // End ComputeNorm Task

    #pragma ocr task TASK(TASK_CalculateStencilTime)	\
      DEP_DBKs(DBK_LOCAL_STENCIL_TIME) \
      DEP_EVTs(OEVT_computenorm) \
      DEP_ELEMs(my_ID, root, Num_procs, stencil_time, f_active_points, iterations)	\
      OEVENT(OEVT_calculatestenciltime)
    {
      double total_execution_time = local_stencil_time[1] - local_stencil_time[0];
      #pragma ocr spmd reduce REDUCE_EVT(EVT_STENCIL_TIME) DEP_EVTs() OEVENT(OEVT_stenciltimereduce)
      MPI_Reduce(&total_execution_time, &stencil_time, 1, MPI_DOUBLE, MPI_SUM, root,
		 MPI_COMM_WORLD);

      if (my_ID == root) {
        #pragma ocr task TASK(TASK_CalculateFlops) DEP_DBKs() DEP_EVTs(EVT_STENCIL_TIME) \
	  DEP_ELEMs(my_ID, root, f_active_points, Num_procs, iterations)		\
	  OEVENT(OEVT_calculateflops)
	{
	  /* timing parameters */
	  double avgtime;
	  int stencil_size;    /* number of points in stencil */
	  DTYPE  flops;           /* floating point ops per iteration */
	  stencil_size = 4*RADIUS+1;
	  double avg_stencil_time = (double) stencil_time/Num_procs;

	  /* flops/stencil: 2 flops (fma) for each point in the stencil,
	     plus one flop for the update of the input of the array        */
	  flops = (DTYPE) (2*stencil_size+1) * f_active_points;
	  avgtime = avg_stencil_time/iterations;
	  printf("Rate (MFlops/s): "FSTR"  Avg time (s): %lf\n",
		 1.0E-06 * flops/avgtime, avgtime);
	}
      }
    } // End compute stencil time

    #pragma ocr spmd finalize DEP_EVTs(OEVT_calculatestenciltime)
    MPI_Finalize();
  } // End SPMD region annotation

  // exit(EXIT_SUCCESS);
  #pragma ocr shutdown DEP_EVTs(OEVT_spmd)
  return 0;
}
