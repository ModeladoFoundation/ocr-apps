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

#include <par-res-kern_general.h>
#include <par-res-kern_mpi.h>
#include <assert.h>

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

void partition_bounds(int id, int lb_g, int ub_g, int R, int* s, int* e)
{
    int N = ub_g - lb_g + 1;

    *s = id*N/R + lb_g;
    *e = (id+1)*N/R + lb_g - 1;
}

void splitDimension_Cart2D(int Num_procs, int* Num_procsx, int* Num_procsy)
{
    int nx, ny;

    nx = (int) sqrt(Num_procs+1);
    for(; nx>0; nx--)
    {
        if (!(Num_procs%nx))
        {
            ny = Num_procs/nx;
            break;
        }
    }
    *Num_procsx = nx; *Num_procsy = (Num_procs)/(*Num_procsx);
}

void getPartitionID(int i, int lb_g, int ub_g, int R, int* id)
{
    int N = ub_g - lb_g + 1;
    int s, e;

    int r;

    for( r = 0; r < R; r++ )
    {
        s = r*N/R + lb_g;
        e = (r+1)*N/R + lb_g - 1;
        if( s <= i && i <= e )
            break;
    }

    *id = r;
}

int globalRankFromCoords_Cart2D( int id_x, int id_y, int NR_X, int NR_Y )
{
    return NR_X*id_y + id_x;
}

static inline int getPolicyDomainID_Cart2D( int b, int* edtGridDims, int* pdGridDims )
{
    int id_x = b%edtGridDims[0];
    int id_y = b/edtGridDims[0];

    int PD_X = pdGridDims[0];
    int PD_Y = pdGridDims[1];

    int pd_x; getPartitionID(id_x, 0, edtGridDims[0]-1, PD_X, &pd_x);
    int pd_y; getPartitionID(id_y, 0, edtGridDims[1]-1, PD_Y, &pd_y);

    //Each edt, with id=b, is mapped to a PD.
    //All the PDs are arranged as a 2-D grid. The 2-D EDT grid is mapped to the PDs.
    //The mapping strategy involes maping a 2-D subgrid of edts to a single PD preserving "locality" within a PD.
    //
    int mapToPD = globalRankFromCoords_Cart2D(pd_x, pd_y, PD_X, PD_Y);

    return mapToPD;
}

int getLinearlytTransformedRank(int my_ID, int Num_procs, int nt)
{
    int npd_x, npd_y, nt_x, nt_y;
    int Num_procsx, Num_procsy;

    int npd = (Num_procs)/nt;

    splitDimension_Cart2D( npd, &npd_x, &npd_y ); //Split available PDs into a 2-D grid
    splitDimension_Cart2D( Num_procs, &Num_procsx, &Num_procsy ); //Split available PDs into a 2-D grid

    int pd = 0;
    int g_x = 0;
    int g_y = 0;
    int cumRanks = 0;
    int cumRanks_p = 0;
    int edtGridDims_lb_x, edtGridDims_ub_x;
    int edtGridDims_lb_y, edtGridDims_ub_y;
    int ranksPerNode, myID_local;

    int pd_x, pd_y;

    int i, j;

    do
    {
        pd_x = pd%npd_x;
        pd_y = pd/npd_x;
        partition_bounds(pd_x, 0, Num_procsx-1, npd_x, &edtGridDims_lb_x, &edtGridDims_ub_x);
        partition_bounds(pd_y, 0, Num_procsy-1, npd_y, &edtGridDims_lb_y, &edtGridDims_ub_y);

        ranksPerNode = (edtGridDims_ub_y-edtGridDims_lb_y+1) * (edtGridDims_ub_x-edtGridDims_lb_x+1);
        cumRanks_p = cumRanks;
        cumRanks += ranksPerNode;

        pd++;
    }
    while( cumRanks <= my_ID );

    pd--;
    pd_x = pd%npd_x;
    pd_y = pd/npd_x;
    partition_bounds(pd_x, 0, Num_procsx-1, npd_x, &edtGridDims_lb_x, &edtGridDims_ub_x);
    partition_bounds(pd_y, 0, Num_procsy-1, npd_y, &edtGridDims_lb_y, &edtGridDims_ub_y);
    myID_local = my_ID - cumRanks_p;
    g_x = edtGridDims_lb_x+(myID_local)%(edtGridDims_ub_x-edtGridDims_lb_x+1);
    g_y = edtGridDims_lb_y+(myID_local)/(edtGridDims_ub_x-edtGridDims_lb_x+1);

    int myID_flattened = g_y*(Num_procsx) + g_x;

    assert(myID_flattened < Num_procs);
    return myID_flattened;
}

int getHierarchicalTransformedRank(int myID, int Num_procs, int nt)
{
    int npd_x, npd_y, nt_x, nt_y;
    int Num_procsx, Num_procsy;

    int npd = (Num_procs)/nt;

    splitDimension_Cart2D( npd, &npd_x, &npd_y ); //Split available PDs into a 2-D grid
    splitDimension_Cart2D( Num_procs, &Num_procsx, &Num_procsy ); //Split available PDs into a 2-D grid

    int g_x = myID%(Num_procsx);
    int g_y = myID/(Num_procsx);

    int edtGridDims[2] = {Num_procsx, Num_procsy};
    int pdGridDims[2] = {npd_x,npd_y};

    int pd = getPolicyDomainID_Cart2D( myID, edtGridDims, pdGridDims );

    int pd_x, pd_y;

    int cumRanks = 0;
    int cumRanks_p = 0;
    int edtGridDims_lb_x, edtGridDims_ub_x;
    int edtGridDims_lb_y, edtGridDims_ub_y;
    int ranksPerNode;

    int i=0;

    while( i <= pd )
    {
        pd_x = i%npd_x;
        pd_y = i/npd_x;

        partition_bounds(pd_x, 0, Num_procsx-1, npd_x, &edtGridDims_lb_x, &edtGridDims_ub_x);
        partition_bounds(pd_y, 0, Num_procsy-1, npd_y, &edtGridDims_lb_y, &edtGridDims_ub_y);

        ranksPerNode = (edtGridDims_ub_y-edtGridDims_lb_y+1) * (edtGridDims_ub_x-edtGridDims_lb_x+1);
        cumRanks_p = cumRanks;
        cumRanks += ranksPerNode;

        i++;
    }

    int t_x = g_x - edtGridDims_lb_x;
    int t_y = g_y - edtGridDims_lb_y;

    int t = t_y*(edtGridDims_ub_x-edtGridDims_lb_x+1) + t_x;

    int myID_hierarchical = cumRanks_p + t;

    assert(myID_hierarchical < Num_procs);

    return myID_hierarchical;
}

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
  DTYPE  * RESTRICT in;   /* input grid values                                   */
  DTYPE  * RESTRICT out;  /* output grid values                                  */
  long   total_length_in; /* total required length to store input array          */
  long   total_length_out;/* total required length to store output array         */
  int    error=0;         /* error flag                                          */
  DTYPE  weight[2*RADIUS+1][2*RADIUS+1]; /* weights of points in the stencil     */
  MPI_Request request[8];
  MPI_Status  status[8];

  int nt;

  /*******************************************************************************
  ** Initialize the MPI environment
  ********************************************************************************/
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_ID);
  MPI_Comm_size(MPI_COMM_WORLD, &Num_procs);

  /*******************************************************************************
  ** process, test, and broadcast input parameters
  ********************************************************************************/

  if (my_ID == root) {
#ifndef STAR
      printf("ERROR: Compact stencil not supported\n");
      error = 1;
      goto ENDOFTESTS;
#endif

    if (argc != 4){
      printf("Usage: %s <# iterations> <array dimension> <ranksPerNode>\n",
             *argv);
      error = 1;
      goto ENDOFTESTS;
    }

    iterations  = atoi(*++argv);
    if (iterations < 1){
      printf("ERROR: iterations must be >= 1 : %d \n",iterations);
      error = 1;
      goto ENDOFTESTS;
    }

    n       = atoi(*++argv);
    nsquare = (long) n * n;
    if (nsquare < Num_procs){
      printf("ERROR: grid size %d must be at least # ranks: %ld\n",
	     nsquare, Num_procs);
      error = 1;
      goto ENDOFTESTS;
    }

    nt = atoi(*++argv);

    if (RADIUS < 0) {
      printf("ERROR: Stencil radius %d should be non-negative\n", RADIUS);
      error = 1;
      goto ENDOFTESTS;
    }

    if (2*RADIUS +1 > n) {
      printf("ERROR: Stencil radius %d exceeds grid size %d\n", RADIUS, n);
      error = 1;
      goto ENDOFTESTS;
    }

    ENDOFTESTS:;
  }
  bail_out(error);

  /* determine best way to create a 2D grid of ranks (closest to square, for
     best surface/volume ratio); we do this brute force for now
  */
  splitDimension_Cart2D( Num_procs, &Num_procsx, &Num_procsy ); //Split available PDs into a 2-D grid

  MPI_Bcast(&n,          1, MPI_INT, root, MPI_COMM_WORLD);
  MPI_Bcast(&iterations, 1, MPI_INT, root, MPI_COMM_WORLD);
  MPI_Bcast(&nt,         1, MPI_INT, root, MPI_COMM_WORLD);

  int my_ID_flattened = getLinearlytTransformedRank(my_ID, Num_procs, nt);
  my_IDx = my_ID_flattened%Num_procsx;
  my_IDy = my_ID_flattened/Num_procsx;
  //printf("rank %d to linearly mapped to rank %d\n", my_ID, my_ID_flattened);

  /* compute neighbors; don't worry about dropping off the edges of the grid */
  right_nbr = (my_IDx < Num_procsx-1) ? getHierarchicalTransformedRank(my_ID_flattened+1, Num_procs, nt) : -1;
  left_nbr = (my_IDx > 0) ? getHierarchicalTransformedRank(my_ID_flattened-1, Num_procs, nt) : -1;
  top_nbr = (my_IDy < Num_procsy-1) ? getHierarchicalTransformedRank(my_ID_flattened+Num_procsx, Num_procs, nt): -1;
  bottom_nbr = (my_IDy > 0) ? getHierarchicalTransformedRank(my_ID_flattened-Num_procsx, Num_procs, nt) : -1;

  if (my_ID == root) {
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

  /* compute amount of space required for input and solution arrays             */

  partition_bounds( my_IDx, 0, n-1, Num_procsx, &istart, &iend );
  partition_bounds( my_IDy, 0, n-1, Num_procsy, &jstart, &jend );

  width = iend - istart + 1;
  if (width == 0) {
    printf("ERROR: rank %d has no work to do\n", my_ID);
    error = 1;
  }
  bail_out(error);

  height = jend - jstart + 1;
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

  total_length_in = (width+2*RADIUS)*(height+2*RADIUS)*sizeof(DTYPE);
  if (total_length_in/(height+2*RADIUS) != (width+2*RADIUS)*sizeof(DTYPE)) {
    printf("ERROR: Space for %d x %d input array cannot be represented\n",
           width+2*RADIUS, height+2*RADIUS);
    error = 1;
  }
  bail_out(error);

  total_length_out = width*height*sizeof(DTYPE);

  in  = (DTYPE *) malloc(total_length_in);
  out = (DTYPE *) malloc(total_length_out);
  if (!in || !out) {
    printf("ERROR: rank %d could not allocate space for input/output array\n",
            my_ID);
    error = 1;
  }
  bail_out(error);

  /* fill the stencil weights to reflect a discrete divergence operator         */
  for (jj=-RADIUS; jj<=RADIUS; jj++) for (ii=-RADIUS; ii<=RADIUS; ii++)
    WEIGHT(ii,jj) = (DTYPE) 0.0;

  stencil_size = 4*RADIUS+1;
  for (ii=1; ii<=RADIUS; ii++) {
    WEIGHT(0, ii) = WEIGHT( ii,0) =  (DTYPE) (1.0/(2.0*ii*RADIUS));
    WEIGHT(0,-ii) = WEIGHT(-ii,0) = -(DTYPE) (1.0/(2.0*ii*RADIUS));
  }

  norm = (DTYPE) 0.0;
  f_active_points = (DTYPE) (n-2*RADIUS)*(DTYPE) (n-2*RADIUS);
  /* intialize the input and output arrays                                     */
  for (j=jstart; j<=jend; j++) for (i=istart; i<=iend; i++) {
    IN(i,j)  = COEFX*i+COEFY*j;
    OUT(i,j) = (DTYPE)0.0;
  }

  /* allocate communication buffers for halo values                            */
  top_buf_out = (DTYPE *) malloc(4*sizeof(DTYPE)*RADIUS*width);
  if (!top_buf_out) {
    printf("ERROR: Rank %d could not allocated comm buffers for y-direction\n", my_ID);
    error = 1;
  }
  bail_out(error);
  top_buf_in     = top_buf_out +   RADIUS*width;
  bottom_buf_out = top_buf_out + 2*RADIUS*width;
  bottom_buf_in  = top_buf_out + 3*RADIUS*width;

  right_buf_out  = (DTYPE *) malloc(4*sizeof(DTYPE)*RADIUS*height);
  if (!right_buf_out) {
    printf("ERROR: Rank %d could not allocated comm buffers for x-direction\n", my_ID);
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

    /* need to fetch ghost point data from neighbors in y-direction                 */
    if (my_IDy < Num_procsy-1) {
      MPI_Irecv(top_buf_in, RADIUS*width, MPI_DTYPE, top_nbr, 101,
                MPI_COMM_WORLD, &(request[1]));
      for (kk=0,j=jend-RADIUS+1; j<=jend; j++) for (i=istart; i<=iend; i++) {
          top_buf_out[kk++]= IN(i,j);
      }
      MPI_Isend(top_buf_out, RADIUS*width,MPI_DTYPE, top_nbr, 99,
                MPI_COMM_WORLD, &(request[0]));
    }
    if (my_IDy > 0) {
      MPI_Irecv(bottom_buf_in,RADIUS*width, MPI_DTYPE, bottom_nbr, 99,
                MPI_COMM_WORLD, &(request[3]));
      for (kk=0,j=jstart; j<=jstart+RADIUS-1; j++) for (i=istart; i<=iend; i++) {
          bottom_buf_out[kk++]= IN(i,j);
      }
      MPI_Isend(bottom_buf_out, RADIUS*width,MPI_DTYPE, bottom_nbr, 101,
                MPI_COMM_WORLD, &(request[2]));
    }
    if (my_IDy < Num_procsy-1) {
      MPI_Wait(&(request[0]), &(status[0]));
      MPI_Wait(&(request[1]), &(status[1]));
      for (kk=0,j=jend+1; j<=jend+RADIUS; j++) for (i=istart; i<=iend; i++) {
          IN(i,j) = top_buf_in[kk++];
      }
    }
    if (my_IDy > 0) {
      MPI_Wait(&(request[2]), &(status[2]));
      MPI_Wait(&(request[3]), &(status[3]));
      for (kk=0,j=jstart-RADIUS; j<=jstart-1; j++) for (i=istart; i<=iend; i++) {
          IN(i,j) = bottom_buf_in[kk++];
      }
    }

    /* need to fetch ghost point data from neighbors in x-direction                 */
    if (my_IDx < Num_procsx-1) {
      MPI_Irecv(right_buf_in, RADIUS*height, MPI_DTYPE, right_nbr, 1010,
                MPI_COMM_WORLD, &(request[1+4]));
      for (kk=0,j=jstart; j<=jend; j++) for (i=iend-RADIUS+1; i<=iend; i++) {
          right_buf_out[kk++]= IN(i,j);
      }
      MPI_Isend(right_buf_out, RADIUS*height, MPI_DTYPE, right_nbr, 990,
              MPI_COMM_WORLD, &(request[0+4]));
    }
    if (my_IDx > 0) {
      MPI_Irecv(left_buf_in, RADIUS*height, MPI_DTYPE, left_nbr, 990,
                MPI_COMM_WORLD, &(request[3+4]));
      for (kk=0,j=jstart; j<=jend; j++) for (i=istart; i<=istart+RADIUS-1; i++) {
          left_buf_out[kk++]= IN(i,j);
      }
      MPI_Isend(left_buf_out, RADIUS*height, MPI_DTYPE, left_nbr, 1010,
                MPI_COMM_WORLD, &(request[2+4]));
    }
    if (my_IDx < Num_procsx-1) {
      MPI_Wait(&(request[0+4]), &(status[0+4]));
      MPI_Wait(&(request[1+4]), &(status[1+4]));
      for (kk=0,j=jstart; j<=jend; j++) for (i=iend+1; i<=iend+RADIUS; i++) {
          IN(i,j) = right_buf_in[kk++];
      }
    }
    if (my_IDx > 0) {
      MPI_Wait(&(request[2+4]), &(status[2+4]));
      MPI_Wait(&(request[3+4]), &(status[3+4]));
      for (kk=0,j=jstart; j<=jend; j++) for (i=istart-RADIUS; i<=istart-1; i++) {
          IN(i,j) = left_buf_in[kk++];
      }
    }

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
      printf("ERROR: L1 norm = "FSTR", Reference L1 norm = "FSTR"\n",
             norm, reference_norm);
      error = 1;
    }
    else {
      printf("Solution validates\n");
#ifdef VERBOSE
      printf("Reference L1 norm = "FSTR", L1 norm = "FSTR"\n",
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
    printf("Rate (MFlops/s): "FSTR"  Avg time (s): %lf\n",
           1.0E-06 * flops/avgtime, avgtime);
  }

  MPI_Finalize();
  exit(EXIT_SUCCESS);
}
