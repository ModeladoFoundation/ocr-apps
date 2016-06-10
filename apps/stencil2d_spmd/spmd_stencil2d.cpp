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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "spmd.h"
#include <extensions/ocr-affinity.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>

#ifndef RADIUS
  #define RADIUS 2
#endif

#define STAR
#define DOUBLE

#ifdef DOUBLE
  #define DTYPE     double
  #define MPI_DTYPE MPI_DOUBLE
  #define EPSILON   1.e-8
  #define COEFX     1.0
  #define COEFY     1.0
  #define FSTR      "%f"
#else
  #define DTYPE     float
  #define MPI_DTYPE MPI_FLOAT
  #define EPSILON   0.0001f
  #define COEFX     1.0f
  #define COEFY     1.0f
  #define FSTR      "%f"
#endif

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif
#ifndef ABS
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#endif

/* define shorthand for indexing multi-dimensional arrays with offsets           */
#define INDEXIN(i,j)  (i+RADIUS+(j+RADIUS)*(wcfg.width+2*RADIUS))
/* need to add offset of RADIUS to j to account for ghost points                 */
#define IN(i,j)       in[INDEXIN(i-wcfg.istart,j-wcfg.jstart)]
#define INDEXOUT(i,j) (i+(j)*(wcfg.width))
#define OUT(i,j)      out[INDEXOUT(i-wcfg.istart,j-wcfg.jstart)]
#define WEIGHT(ii,jj) weight[ii+RADIUS + (jj+RADIUS)*(2 * RADIUS + 1)]


#ifdef FOO
int main(int argc, char ** argv) {

  int    Num_procs;       /* number of ranks                                     */
  int    Num_procsx, Num_procsy; /* number of ranks in each coord direction      */
  int    my_ID;           /* MPI rank                                            */
  int    my_IDx, my_IDy;  /* coordinates of rank in rank grid                    */
  int    right_nbr;       /* global rank of right neighboring tile               */
  int    left_nbr;        /* global rank of left neighboring tile                */
  int    top_nbr;         /* global rank of top neighboring tile                 */
  int    bottom_nbr;      /* global rank of bottom neighboring tile              */
  DTYPE *top_buf_out;     /* communication buffer                                */
  DTYPE *top_buf_in;      /*       "         "                                   */
  DTYPE *bottom_buf_out;  /*       "         "                                   */
  DTYPE *bottom_buf_in;   /*       "         "                                   */
  DTYPE *right_buf_out;   /*       "         "                                   */
  DTYPE *right_buf_in;    /*       "         "                                   */
  DTYPE *left_buf_out;    /*       "         "                                   */
  DTYPE *left_buf_in;     /*       "         "                                   */
  int    root = 0;
  int    n, width, height;/* linear global and local grid dimension              */
  long   nsquare;         /* total number of grid points                         */
  int    i, j, ii, jj, kk, it, jt, iter, leftover;  /* dummies                   */
  int    istart, iend;    /* bounds of grid tile assigned to calling rank        */
  int    jstart, jend;    /* bounds of grid tile assigned to calling rank        */
  DTYPE  norm,            /* L1 norm of solution                                 */
         local_norm,      /* contribution of calling rank to L1 norm             */
         reference_norm;
  DTYPE  f_active_points; /* interior of grid with respect to stencil            */
  DTYPE  flops;           /* floating point ops per iteration                    */
  int    iterations;      /* number of times to run the algorithm                */
  double local_stencil_time,/* timing parameters                                 */
         stencil_time,
         avgtime;
  int    stencil_size;    /* number of points in stencil                         */
  DTYPE  *in;   /* input grid values                                   */
  DTYPE  *out;  /* output grid values                                  */
  long   total_length_in; /* total required length to store input array          */
  long   total_length_out;/* total required length to store output array         */
  int    error=0;         /* error flag                                          */
  DTYPE  weight[2*RADIUS+1][2*RADIUS+1]; /* weights of points in the stencil     */
  MPI_Request request[8];
  MPI_Status  status[8];



  /* allocate communication buffers for halo values                            */
  top_buf_out = (DTYPE *) malloc(4*sizeof(DTYPE)*RADIUS*width);
  if (!top_buf_out) {
    PRINTF("ERROR: Rank %d could not allocated comm buffers for y-direction\n", my_ID);
    error = 1;
  }
  bail_out(error);
  top_buf_in     = top_buf_out +   RADIUS*width;
  bottom_buf_out = top_buf_out + 2*RADIUS*width;
  bottom_buf_in  = top_buf_out + 3*RADIUS*width;

  right_buf_out  = (DTYPE *) malloc(4*sizeof(DTYPE)*RADIUS*height);
  if (!right_buf_out) {
    PRINTF("ERROR: Rank %d could not allocated comm buffers for x-direction\n", my_ID);
    error = 1;
  }
  bail_out(error);
  right_buf_in   = right_buf_out +   RADIUS*height;
  left_buf_out   = right_buf_out + 2*RADIUS*height;
  left_buf_in    = right_buf_out + 3*RADIUS*height;

  for (iter = 0; iter<=iterations; iter++){

    /* start timer after a warmup iteration */
    if (iter == 1) {
      MPI_Barrier(MPI_COMM_WORLD);
      local_stencil_time = wtime();
    }



  } /* end of iterations                                                   */

  local_stencil_time = wtime() - local_stencil_time;
  MPI_Reduce(&local_stencil_time, &stencil_time, 1, MPI_DOUBLE, MPI_MAX, root,
             MPI_COMM_WORLD);

  /* compute L1 norm in parallel                                                */
  local_norm = (DTYPE) 0.0;
  for (j=MAX(jstart,RADIUS); j<=MIN(n-RADIUS-1,jend); j++) {
    for (i=MAX(istart,RADIUS); i<=MIN(n-RADIUS-1,iend); i++) {
      local_norm += (DTYPE)ABS(OUT(i,j));
    }
  }

  MPI_Reduce(&local_norm, &norm, 1, MPI_DTYPE, MPI_SUM, root, MPI_COMM_WORLD);

  /*******************************************************************************
  ** Analyze and output results.
  ********************************************************************************/

/* verify correctness                                                            */
  if (my_ID == root) {
    norm /= f_active_points;
    if (RADIUS > 0) {
      reference_norm = (DTYPE) (iterations+1) * (COEFX + COEFY);
    }
    else {
      reference_norm = (DTYPE) 0.0;
    }
    if (ABS(norm-reference_norm) > EPSILON) {
      PRINTF("ERROR: L1 norm = "FSTR", Reference L1 norm = "FSTR"\n",
             norm, reference_norm);
      error = 1;
    }
    else {
      PRINTF("Solution validates\n");
#ifdef VERBOSE
      PRINTF("Reference L1 norm = "FSTR", L1 norm = "FSTR"\n",
             reference_norm, norm);
#endif
    }
  }
  bail_out(error);

  if (my_ID == root) {
    /* flops/stencil: 2 flops (fma) for each point in the stencil,
       plus one flop for the update of the input of the array        */
    flops = (DTYPE) (2*stencil_size+1) * f_active_points;
    avgtime = stencil_time/iterations;
    PRINTF("Rate (MFlops/s): "FSTR"  Avg time (s): %lf\n",
           1.0E-06 * flops/avgtime, avgtime);
  }

  MPI_Finalize();
  exit(EXIT_SUCCESS);
}

#endif

struct config
{
	u64 Num_procsx;
	u64 Num_procsy;
	u64 n;
	u64 iterations;
	u64 iteration;
	ocrGuid_t workerTML;
	ocrGuid_t checkTML;
	u64 t0_sec, t0_usec;
	static u32 size_in_u64()
	{
		return (sizeof(config) + sizeof(u64) - 1) / sizeof(u64);
	}
};

struct worker_config
{
	config cfg;
	s64 my_ID;
	s64 my_IDx;
	s64 my_IDy;
	s64 right_nbr;
	s64 left_nbr;
	s64 top_nbr;
	s64 bottom_nbr;
	s64 width;
	s64 height;
	s64 istart, iend;
	s64 jstart, jend;
	static u32 size_in_u64()
	{
		return (sizeof(worker_config) + sizeof(u64) - 1) / sizeof(u64);
	}
};

#define WORKER_DEPC (7)
#define INDEX_OF_IN (0)
#define INDEX_OF_OUT (1)
#define INDEX_OF_TOP (2)
#define INDEX_OF_BOTTOM (3)
#define INDEX_OF_LEFT (4)
#define INDEX_OF_RIGHT (5)
#define INDEX_OF_WEIGHT (6)

void exchange_data(config& cfg, worker_config& wcfg, ocrGuid_t nextEDT, DTYPE* in, DTYPE* out)
{
	/* need to fetch ghost point data from neighbors in y-direction                 */
	if (wcfg.my_IDy < cfg.Num_procsy - 1) {
		ocrGuid_t top_recvEVT = NULL_GUID;
		ocrEventCreate(&top_recvEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
		ocrAddDependence(top_recvEVT, nextEDT, INDEX_OF_TOP, DB_MODE_EW);
		spmdRecv(wcfg.top_nbr, 101, top_recvEVT, NULL_GUID, false, NULL_GUID);
		//MPI_Irecv(top_buf_in, RADIUS*width, MPI_DTYPE, top_nbr, 101, MPI_COMM_WORLD, &(request[1]));
		u64 kk = 0;
		DTYPE* top_buf_out;
		ocrGuid_t top_outDBK;
		ocrDbCreate(&top_outDBK, (void**)&top_buf_out, RADIUS* wcfg.width * sizeof(DTYPE), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
		for (s64 j = wcfg.jend - RADIUS + 1; j <= wcfg.jend; j++) for (s64 i = wcfg.istart; i <= wcfg.iend; i++) {
			top_buf_out[kk++] = IN(i, j);
		}
		ocrDbRelease(top_outDBK);
		spmdGSend(wcfg.top_nbr, 99, top_outDBK, NULL_GUID, false, NULL_GUID);
		//MPI_Isend(top_buf_out, RADIUS*width, MPI_DTYPE, top_nbr, 99, MPI_COMM_WORLD, &(request[0]));
	}
	else
	{
		ocrAddDependence(NULL_GUID, nextEDT, INDEX_OF_TOP, DB_MODE_EW);
	}
	if (wcfg.my_IDy > 0) {
		ocrGuid_t bottom_recvEVT = NULL_GUID;
		ocrEventCreate(&bottom_recvEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
		ocrAddDependence(bottom_recvEVT, nextEDT, INDEX_OF_BOTTOM, DB_MODE_EW);
		spmdRecv(wcfg.bottom_nbr, 99, bottom_recvEVT, NULL_GUID, false, NULL_GUID);
		//MPI_Irecv(bottom_buf_in, RADIUS*width, MPI_DTYPE, bottom_nbr, 99, MPI_COMM_WORLD, &(request[3]));
		u64 kk = 0;
		DTYPE* bottom_buf_out;
		ocrGuid_t bottom_outDBK;
		ocrDbCreate(&bottom_outDBK, (void**)&bottom_buf_out, RADIUS* wcfg.width * sizeof(DTYPE), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
		for (s64 j = wcfg.jstart; j <= wcfg.jstart + RADIUS - 1; j++) for (s64 i = wcfg.istart; i <= wcfg.iend; i++) {
			bottom_buf_out[kk++] = IN(i, j);
		}
		spmdGSend(wcfg.bottom_nbr, 101, bottom_outDBK, NULL_GUID, false, NULL_GUID);
		//MPI_Isend(bottom_buf_out, RADIUS*width, MPI_DTYPE, bottom_nbr, 101, MPI_COMM_WORLD, &(request[2]));
	}
	else
	{
		ocrAddDependence(NULL_GUID, nextEDT, INDEX_OF_BOTTOM, DB_MODE_EW);
	}

	/* need to fetch ghost point data from neighbors in x-direction                 */
	if (wcfg.my_IDx < cfg.Num_procsx - 1) {
		ocrGuid_t right_recvEVT = NULL_GUID;
		ocrEventCreate(&right_recvEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
		ocrAddDependence(right_recvEVT, nextEDT, INDEX_OF_RIGHT, DB_MODE_EW);
		spmdRecv(wcfg.right_nbr, 1010, right_recvEVT, NULL_GUID, false, NULL_GUID);
		//MPI_Irecv(right_buf_in, RADIUS*height, MPI_DTYPE, right_nbr, 1010, MPI_COMM_WORLD, &(request[1 + 4]));
		u64 kk = 0;
		DTYPE* right_buf_out;
		ocrGuid_t right_outDBK;
		ocrDbCreate(&right_outDBK, (void**)&right_buf_out, RADIUS* wcfg.height * sizeof(DTYPE), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
		for (s64 j = wcfg.jstart; j <= wcfg.jend; j++) for (s64 i = wcfg.iend - RADIUS + 1; i <= wcfg.iend; i++) {
			right_buf_out[kk++] = IN(i, j);
		}
		spmdGSend(wcfg.right_nbr, 990, right_outDBK, NULL_GUID, false, NULL_GUID);
		//MPI_Isend(right_buf_out, RADIUS*height, MPI_DTYPE, right_nbr, 990, MPI_COMM_WORLD, &(request[0 + 4]));
	}
	else
	{
		ocrAddDependence(NULL_GUID, nextEDT, INDEX_OF_RIGHT, DB_MODE_EW);
	}
	if (wcfg.my_IDx > 0) {
		ocrGuid_t left_recvEVT = NULL_GUID;
		ocrEventCreate(&left_recvEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
		ocrAddDependence(left_recvEVT, nextEDT, INDEX_OF_LEFT, DB_MODE_EW);
		spmdRecv(wcfg.left_nbr, 990, left_recvEVT, NULL_GUID, false, NULL_GUID);
		//MPI_Irecv(left_buf_in, RADIUS*height, MPI_DTYPE, left_nbr, 990, MPI_COMM_WORLD, &(request[3 + 4]));
		u64 kk = 0;
		DTYPE* left_buf_out;
		ocrGuid_t left_outDBK;
		ocrDbCreate(&left_outDBK, (void**)&left_buf_out, RADIUS* wcfg.height * sizeof(DTYPE), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
		for (s64 j = wcfg.jstart; j <= wcfg.jend; j++) for (s64 i = wcfg.istart; i <= wcfg.istart + RADIUS - 1; i++) {
			left_buf_out[kk++] = IN(i, j);
		}
		spmdGSend(wcfg.left_nbr, 1010, left_outDBK, NULL_GUID, false, NULL_GUID);
		//MPI_Isend(left_buf_out, RADIUS*height, MPI_DTYPE, left_nbr, 1010, MPI_COMM_WORLD, &(request[2 + 4]));
	}
	else
	{
		ocrAddDependence(NULL_GUID, nextEDT, INDEX_OF_LEFT, DB_MODE_EW);
	}
}

void print_in(worker_config& wcfg, DTYPE* in)
{
	for (s64 j = wcfg.jstart - RADIUS; j <= wcfg.jend + RADIUS; j++) {
		for (s64 i = wcfg.istart - RADIUS; i <= wcfg.iend + RADIUS; i++) {
			PRINTF(FSTR ", ", IN(i, j));
		}
		PRINTF("\n");
	}
}

void print_out(worker_config& wcfg, DTYPE* out)
{
	for (s64 j = MAX(wcfg.jstart, RADIUS); j <= MIN(wcfg.cfg.n - RADIUS - 1, wcfg.jend); j++) {
		for (s64 i = MAX(wcfg.istart, RADIUS); i <= MIN(wcfg.cfg.n - RADIUS - 1, wcfg.iend); i++) {
			PRINTF(FSTR ", ", OUT(i, j));
		}
		PRINTF("\n");
	}
}

ocrGuid_t workerInitEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	config& cfg = *(config*)paramv;
	worker_config wcfg;

	wcfg.cfg = cfg;
	int error = 0;
	wcfg.my_ID = spmdMyRank();
	wcfg.my_IDx = wcfg.my_ID % cfg.Num_procsx;
	wcfg.my_IDy = wcfg.my_ID / cfg.Num_procsx;
	/* compute neighbors; don't worry about dropping off the edges of the grid */
	wcfg.right_nbr = wcfg.my_ID + 1;
	wcfg.left_nbr = wcfg.my_ID - 1;
	wcfg.top_nbr = wcfg.my_ID + cfg.Num_procsx;
	wcfg.bottom_nbr = wcfg.my_ID - cfg.Num_procsx;

	wcfg.width = cfg.n / cfg.Num_procsx;
	u64 leftover = cfg.n % cfg.Num_procsx;
	if (wcfg.my_IDx<leftover) {
		wcfg.istart = (wcfg.width + 1) * wcfg.my_IDx;
		wcfg.iend = wcfg.istart + wcfg.width;
	}
	else {
		wcfg.istart = (wcfg.width + 1) * leftover + wcfg.width * (wcfg.my_IDx - leftover);
		wcfg.iend = wcfg.istart + wcfg.width - 1;
	}

	wcfg.width = wcfg.iend - wcfg.istart + 1;
	if (wcfg.width == 0) {
		PRINTF("ERROR: rank %d has no work to do\n", (int)wcfg.my_ID);
		error = 1;
	}
	assert(!error);

	wcfg.height = cfg.n / cfg.Num_procsy;
	leftover = cfg.n % cfg.Num_procsy;
	if (wcfg.my_IDy<leftover) {
		wcfg.jstart = (wcfg.height + 1) * wcfg.my_IDy;
		wcfg.jend = wcfg.jstart + wcfg.height;
	}
	else {
		wcfg.jstart = (wcfg.height + 1) * leftover + wcfg.height * (wcfg.my_IDy - leftover);
		wcfg.jend = wcfg.jstart + wcfg.height - 1;
	}

	wcfg.height = wcfg.jend - wcfg.jstart + 1;
	if (wcfg.height == 0) {
		PRINTF("ERROR: rank %d has no work to do\n", (int)wcfg.my_ID);
		error = 1;
	}
	assert(!error);

	if (wcfg.width < RADIUS || wcfg.height < RADIUS) {
		PRINTF("ERROR: rank %d has work tile smaller then stencil radius\n",
			(int)wcfg.my_ID);
		error = 1;
	}
	assert(!error);

	u64 total_length_in = (wcfg.width + 2 * RADIUS)*(wcfg.height + 2 * RADIUS) * sizeof(DTYPE);
	if (total_length_in / (wcfg.height + 2 * RADIUS) != (wcfg.width + 2 * RADIUS) * sizeof(DTYPE)) {
		PRINTF("ERROR: Space for %d x %d input array cannot be represented\n",
			(int)wcfg.width + 2 * RADIUS, (int)wcfg.height + 2 * RADIUS);
		error = 1;
	}
	assert(!error);

	u64 total_length_out = wcfg.width*wcfg.height * sizeof(DTYPE);

	DTYPE* in;
	DTYPE* out;
	ocrGuid_t inDBK, outDBK;
	ocrDbCreate(&inDBK, (void**)&in, total_length_in, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
	ocrDbCreate(&outDBK, (void**)&out, total_length_out, DB_PROP_NONE, NULL_HINT, NO_ALLOC);

	ocrGuid_t nextEDT;
	ocrEdtCreate(&nextEDT, cfg.workerTML, worker_config::size_in_u64(), (u64*)&wcfg, WORKER_DEPC, 0, EDT_PROP_NONE, NULL_HINT, 0);

	{
		ocrGuid_t weightDBK;
		DTYPE* weightPTR;
		ocrDbCreate(&weightDBK, (void**)&weightPTR, (2 * RADIUS + 1)*(2 * RADIUS + 1) * sizeof(DTYPE), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
		DTYPE* weight = weightPTR;

		/* fill the stencil weights to reflect a discrete divergence operator         */
		for (s64 jj = -RADIUS; jj <= RADIUS; jj++) for (s64 ii = -RADIUS; ii <= RADIUS; ii++)
			WEIGHT(ii, jj) = (DTYPE) 0.0;

		for (s64 ii = 1; ii <= RADIUS; ii++) {
			WEIGHT(0, ii) = WEIGHT(ii, 0) = (DTYPE)(1.0 / (2.0*ii*RADIUS));
			WEIGHT(0, -ii) = WEIGHT(-ii, 0) = -(DTYPE)(1.0 / (2.0*ii*RADIUS));
		}
		ocrDbRelease(weightDBK);
		ocrAddDependence(weightDBK, nextEDT, INDEX_OF_WEIGHT, DB_MODE_CONST);
	}

	/* intialize the input and output arrays                                     */
	for (s64 j = wcfg.jstart; j <= wcfg.jend; j++) for (s64 i = wcfg.istart; i <= wcfg.iend; i++) {
		IN(i, j) = COEFX*i + COEFY*j;
		OUT(i, j) = (DTYPE)0.0;
	}

	exchange_data(cfg, wcfg, nextEDT, in, out);

	ocrDbRelease(inDBK);
	ocrDbRelease(outDBK);
	ocrAddDependence(inDBK, nextEDT, INDEX_OF_IN, DB_MODE_EW);
	ocrAddDependence(outDBK, nextEDT, INDEX_OF_OUT, DB_MODE_EW);
	return NULL_GUID;
}

ocrGuid_t workerEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	worker_config& wcfg = *(worker_config*)paramv;
	config& cfg = wcfg.cfg;
	//PRINTF("iteration %d worker %d,%d\n", (int)cfg.iteration, (int)wcfg.my_IDx, (int)wcfg.my_IDy);
	DTYPE* in = (DTYPE*)depv[INDEX_OF_IN].ptr;
	DTYPE* out = (DTYPE*)depv[INDEX_OF_OUT].ptr;
	DTYPE* top_buf_in = (DTYPE*)depv[INDEX_OF_TOP].ptr;
	DTYPE* bottom_buf_in = (DTYPE*)depv[INDEX_OF_BOTTOM].ptr;
	DTYPE* left_buf_in = (DTYPE*)depv[INDEX_OF_LEFT].ptr;
	DTYPE* right_buf_in = (DTYPE*)depv[INDEX_OF_RIGHT].ptr;
	DTYPE* weight = (DTYPE*)depv[INDEX_OF_WEIGHT].ptr;
	if (wcfg.my_IDy < cfg.Num_procsy - 1) {
		assert(top_buf_in);
		u64 kk = 0;
		for (s64 j = wcfg.jend + 1; j <= wcfg.jend + RADIUS; j++) for (s64 i = wcfg.istart; i <= wcfg.iend; i++) {
			IN(i, j) = top_buf_in[kk++];
		}
		ocrDbDestroy(depv[INDEX_OF_TOP].guid);
	}
	if (wcfg.my_IDy > 0) {
		assert(bottom_buf_in);
		u64 kk = 0;
		for (s64 j = wcfg.jstart - RADIUS; j <= wcfg.jstart - 1; j++) for (s64 i = wcfg.istart; i <= wcfg.iend; i++) {
			IN(i, j) = bottom_buf_in[kk++];
		}
		ocrDbDestroy(depv[INDEX_OF_BOTTOM].guid);
	}
	if (wcfg.my_IDx < cfg.Num_procsx - 1) {
		assert(right_buf_in);
		u64 kk = 0;
		for (s64 j = wcfg.jstart; j <= wcfg.jend; j++) for (s64 i = wcfg.iend + 1; i <= wcfg.iend + RADIUS; i++) {
			IN(i, j) = right_buf_in[kk++];
		}
		ocrDbDestroy(depv[INDEX_OF_RIGHT].guid);
	}
	if (wcfg.my_IDx > 0) {
		assert(left_buf_in);
		u64 kk = 0;
		for (s64 j = wcfg.jstart; j <= wcfg.jend; j++) for (s64 i = wcfg.istart - RADIUS; i <= wcfg.istart - 1; i++) {
			IN(i, j) = left_buf_in[kk++];
		}
		ocrDbDestroy(depv[INDEX_OF_LEFT].guid);
	}
	/* Apply the stencil operator */
	for (s64 j = MAX(wcfg.jstart, RADIUS); j <= MIN(cfg.n - RADIUS - 1, wcfg.jend); j++) {
		for (s64 i = MAX(wcfg.istart, RADIUS); i <= MIN(cfg.n - RADIUS - 1, wcfg.iend); i++) {
			for (s64 jj = -RADIUS; jj <= RADIUS; jj++) {
				OUT(i, j) += WEIGHT(0, jj)*IN(i, j + jj);
			}
			for (s64 ii = -RADIUS; ii<0; ii++) {
				OUT(i, j) += WEIGHT(ii, 0)*IN(i + ii, j);
			}
			for (s64 ii = 1; ii <= RADIUS; ii++) {
				OUT(i, j) += WEIGHT(ii, 0)*IN(i + ii, j);

			}
		}
	}

	/* add constant to solution to force refresh of neighbor data, if any */
	for (s64 j = wcfg.jstart; j <= wcfg.jend; j++) for (s64 i = wcfg.istart; i <= wcfg.iend; i++) IN(i, j) += 1.0;

	//end of iteration
	++cfg.iteration;

	if (cfg.iteration < cfg.iterations)
	{
		ocrGuid_t nextEDT;
		ocrEdtCreate(&nextEDT, cfg.workerTML, worker_config::size_in_u64(), (u64*)&wcfg, WORKER_DEPC, 0, EDT_PROP_NONE, NULL_HINT, 0);

		exchange_data(cfg, wcfg, nextEDT, in, out);

		ocrDbRelease(depv[INDEX_OF_IN].guid);
		ocrDbRelease(depv[INDEX_OF_OUT].guid);
		ocrDbRelease(depv[INDEX_OF_WEIGHT].guid);
		ocrAddDependence(depv[INDEX_OF_IN].guid, nextEDT, INDEX_OF_IN, DB_MODE_EW);
		ocrAddDependence(depv[INDEX_OF_OUT].guid, nextEDT, INDEX_OF_OUT, DB_MODE_EW);
		ocrAddDependence(depv[INDEX_OF_WEIGHT].guid, nextEDT, INDEX_OF_WEIGHT, DB_MODE_CONST);
	}
	else
	{
		DTYPE local_norm = (DTYPE) 0.0;
		for (s64 j = MAX(wcfg.jstart, RADIUS); j <= MIN(cfg.n - RADIUS - 1, wcfg.jend); j++) {
			for (s64 i = MAX(wcfg.istart, RADIUS); i <= MIN(cfg.n - RADIUS - 1, wcfg.iend); i++) {
				local_norm += (DTYPE)ABS(OUT(i, j));
			}
		}

		ocrDbDestroy(depv[INDEX_OF_IN].guid);
		ocrDbDestroy(depv[INDEX_OF_OUT].guid);
		ocrDbDestroy(depv[INDEX_OF_WEIGHT].guid);

		if (wcfg.my_ID == 0)
		{
			ocrGuid_t connectEVT, checkEDT;
			ocrEventCreate(&connectEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
			ocrEdtCreate(&checkEDT, cfg.checkTML, config::size_in_u64(), (u64*)&cfg, 1, 0, EDT_PROP_NONE, NULL_HINT, 0);
			ocrAddDependence(connectEVT, checkEDT, 0, DB_MODE_CONST);
			spmdPReduce(SPMD_REDUCE_TYPE_DOUBLE, SPMD_REDUCE_OP_SUM, 1, 0, &local_norm, connectEVT, NULL_GUID, false, NULL_GUID);
		}
		else
		{
			ocrGuid_t connectEVT;
			ocrEventCreate(&connectEVT, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
			spmdRankFinalize(connectEVT, false);
			spmdPReduce(SPMD_REDUCE_TYPE_DOUBLE, SPMD_REDUCE_OP_SUM, 1, 0, &local_norm, NULL_GUID, NULL_GUID, false, connectEVT);
		}
	}
	return NULL_GUID;
}

ocrGuid_t checkEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	config& cfg = *(config*)paramv;
	struct timeval t1;
	gettimeofday(&t1, 0);
	double elapsed = ((double)((t1.tv_sec - cfg.t0_sec) * 1000000 + t1.tv_usec - cfg.t0_usec)) / 1000000;

	DTYPE norm = *(DTYPE*)depv[0].ptr;
	ocrDbDestroy(depv[0].guid);
	PRINTF("\n");
	//PRINTF("norm is " FSTR "\n", norm);
	DTYPE reference_norm;
	DTYPE f_active_points = (DTYPE)(cfg.n - 2 * RADIUS)*(DTYPE)(cfg.n - 2 * RADIUS);
	norm /= f_active_points;
	if (RADIUS > 0) {
		reference_norm = (DTYPE)(cfg.iterations) * (COEFX + COEFY);
	}
	else {
		reference_norm = (DTYPE) 0.0;
	}
	if (ABS(norm - reference_norm) > EPSILON) {
		PRINTF("ERROR: L1 norm = " FSTR ", Reference L1 norm = " FSTR "\n", norm, reference_norm);
	}
	else {
		PRINTF("Solution validates\n");
		PRINTF("Reference L1 norm = " FSTR ", L1 norm = " FSTR "\n", reference_norm, norm);
	}
	/* flops/stencil: 2 flops (fma) for each point in the stencil,
	plus one flop for the update of the input of the array        */
	DTYPE flops = (DTYPE)(2 * (4 * RADIUS + 1) + 1) * f_active_points;
	double avgtime = elapsed / cfg.iterations;
	PRINTF("Rate (MFlops/s): " FSTR "  Avg time (s): %f\n", 1.0E-06 * flops / avgtime, avgtime);
	spmdRankFinalize(NULL_GUID, false);
	return NULL_GUID;
}

ocrGuid_t finishEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	ocrShutdown();
	return NULL_GUID;
}

extern "C" ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	int error = 0;
	config cfg;
	u64 nsquare;
	u64 Num_procs;
#ifndef STAR
	PRINTF("ERROR: Compact stencil not supported\n");
	error = 1;
	goto ENDOFTESTS;
#endif

	if (getArgc(depv[0].ptr) != 4) {
		PRINTF("Usage: %s <array dimension> <number of ranks> <# iterations> \n",
			getArgv(depv[0].ptr,0));
		error = 1;
		goto ENDOFTESTS;
	}

	cfg.iterations = atoi(getArgv(depv[0].ptr, 3));
	if (cfg.iterations < 1) {
		PRINTF("ERROR: iterations must be >= 1 : %d \n", cfg.iterations);
		error = 1;
		goto ENDOFTESTS;
	}

	cfg.n = atoi(getArgv(depv[0].ptr, 1));
	nsquare = cfg.n * cfg.n;
	Num_procs = atoi(getArgv(depv[0].ptr, 2));
	if (nsquare < Num_procs) {
		PRINTF("ERROR: grid size %d must be at least # ranks: %ld\n",
			nsquare, Num_procs);
		error = 1;
		goto ENDOFTESTS;
	}

	if (RADIUS < 0) {
		PRINTF("ERROR: Stencil radius %d should be non-negative\n", RADIUS);
		error = 1;
		goto ENDOFTESTS;
	}

	if (2 * RADIUS + 1 > cfg.n) {
		PRINTF("ERROR: Stencil radius %d exceeds grid size %d\n", RADIUS, cfg.n);
		error = 1;
		goto ENDOFTESTS;
	}

ENDOFTESTS:;
	ocrDbDestroy(depv[0].guid);
	if (error)
	{
		ocrShutdown();
		return NULL_GUID;
	}

	for (cfg.Num_procsx = (int)(sqrt(Num_procs + 1)); cfg.Num_procsx>0; cfg.Num_procsx--) {
		if (!(Num_procs%cfg.Num_procsx)) {
			cfg.Num_procsy = Num_procs / cfg.Num_procsx;
			break;
		}
	}
	cfg.iteration = 0;
	PRINTF("OCR SPMD stencil execution on 2D grid\n");
	PRINTF("Number of ranks        = %d\n", (int)Num_procs);
	PRINTF("Grid size              = %d\n", (int)cfg.n);
	PRINTF("Radius of stencil      = %d\n", (int)RADIUS);
	PRINTF("Tiles in x/y-direction = %d/%d\n", (int)cfg.Num_procsx, (int)cfg.Num_procsy);
	PRINTF("Type of stencil        = star\n");
#ifdef DOUBLE
	PRINTF("Data type              = double precision\n");
#else
	PRINTF("Data type              = single precision\n");
#endif
	PRINTF("Number of iterations   = %d\n", (int)cfg.iterations);

	ocrGuid_t local_aff;
	ocrAffinityGetCurrent(&local_aff);
	ocrHint_t local_hint;
	ocrHintInit(&local_hint, OCR_HINT_EDT_T);
	ocrSetHintValue(&local_hint, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(local_aff));

	ocrGuid_t workerInitTML, finishTML, finishEDT, finishEVT;
	ocrEdtTemplateCreate(&workerInitTML, workerInitEdt, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEdtTemplateCreate(&cfg.workerTML, workerEdt, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEdtTemplateCreate(&cfg.checkTML, checkEdt, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEdtTemplateCreate(&finishTML, finishEdt, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEventCreate(&finishEVT, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
	ocrEdtCreate(&finishEDT, finishTML, 0, 0, 1, 0, EDT_PROP_NONE, &local_hint, 0);
	ocrAddDependence(finishEVT, finishEDT, 0, DB_MODE_NULL);

	struct timeval t0;
	gettimeofday(&t0, 0);
	cfg.t0_sec = t0.tv_sec;
	cfg.t0_usec = t0.tv_usec;

	spmdEdtSpawn(workerInitTML, Num_procs, config::size_in_u64(), (u64*)&cfg, 0, 0, 0, 0, finishEVT);

	return NULL_GUID;
}
