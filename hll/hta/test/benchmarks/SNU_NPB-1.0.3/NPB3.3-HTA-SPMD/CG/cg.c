//-------------------------------------------------------------------------//
//                                                                         //
//  This benchmark is an OpenMP C version of the NPB CG code. This OpenMP  //
//  C version is developed by the Center for Manycore Programming at Seoul //
//  National University and derived from the OpenMP Fortran versions in    //
//  "NPB3.3-OMP" developed by NAS.                                         //
//                                                                         //
//  Permission to use, copy, distribute and modify this software for any   //
//  purpose with or without fee is hereby granted. This software is        //
//  provided "as is" without express or implied warranty.                  //
//                                                                         //
//  Information on NPB 3.3, including the technical report, the original   //
//  specifications, source code, results and information on how to submit  //
//  new results, is available at:                                          //
//                                                                         //
//           http://www.nas.nasa.gov/Software/NPB/                         //
//                                                                         //
//  Send comments or suggestions for this OpenMP C version to              //
//  cmp@aces.snu.ac.kr                                                     //
//                                                                         //
//          Center for Manycore Programming                                //
//          School of Computer Science and Engineering                     //
//          Seoul National University                                      //
//          Seoul 151-744, Korea                                           //
//                                                                         //
//          E-mail:  cmp@aces.snu.ac.kr                                    //
//                                                                         //
//-------------------------------------------------------------------------//

//-------------------------------------------------------------------------//
// Authors: Sangmin Seo, Jungwon Kim, Jun Lee, Jeongho Nah, Gangwon Jo,    //
//          and Jaejin Lee                                                 //
//-------------------------------------------------------------------------//

//---------------------------------------------------------------------
// program cg
//---------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "globals.h"
#include "randdp.h"
#include "timers.h"
#include "print_results.h"

#include "HTA.h"
#include "HTA_operations.h"
//---------------------------------------------------------------------
/* common / main_int_mem / */
static int NPROWS, NPCOLS, IsSquare;
static int NZZ;

//static HTA* x_HTA;
//static HTA* z_HTA;
//static HTA* p_HTA;
//static HTA* q_HTA;
//static HTA* r_HTA;
//static HTA* t_HTA;
//static HTA* hs;         // the sparse array
static int PROC = 1;
/* common /tinof/ */
// static int myid, num_threads, ilow, ihigh;
// #pragma omp threadprivate(myid, num_threads, ilow, ihigh)
#define max_threads 1024
//static int last_n[max_threads+1];

/* common / partit_size / */
static int naa;
//static int nzz;
//static int firstrow;
//static int lastrow;
//static int firstcol;
//static int lastcol;

/* common /urando/ */
//static double amult;
//static double tran;
//#pragma omp threadprivate (amult,tran)
static const double AMULT = 1220703125.0;
static double zeta_verify_value;

/* common /timers/ */
static logical timeron;

static double one = 1.0;
static double zero = 0.0;
//---------------------------------------------------------------------

//---------------------------------------------------------------------
static void conj_grad(double *rnorm, HTA* x_HTA, HTA* z_HTA, HTA* p_HTA, HTA* q_HTA, HTA* r_HTA, HTA* hs, double* tmp);
static void makea(int rank,
                  int n,
                  int nz,
                  double values[],
                  int colidx[],
                  int rowstr[],
                  int firstrow,
                  int lastrow,
                  int firstcol,
                  int lastcol);
//static void makea(int n,
//                  int nz,
//                  double a[],
//                  int colidx[],
//                  int rowstr[],
//                  int firstrow,
//                  int lastrow,
//                  int firstcol,
//                  int lastcol,
//                  int arow[],
//                  int acol[][NONZER+1],
//                  double aelt[][NONZER+1],
//                  // double v[],
//                  int iv[]);
static void makea_HTA(HTA *h);
//static void sparse(double a[],
//                   int colidx[],
//                   int rowstr[],
//                   int n,
//                   int nz,
//                   int nozer,
//                   int arow[],
//                   int acol[][NONZER+1],
//                   double aelt[][NONZER+1],
//                   int firstrow,
//                   int lastrow,
//                   // int last_n[],
//                   // double v[],
//                   // int iv[],
//                   int nzloc[],
//                   double rcond,
//                   double shift);
static void sprnvc(
		   int n,
		   int nz,
		   double v[],
		   int iv[],
		   int nzloc[],
		   int mark[],
                   double *tran);
static int icnvrt(double x, int ipwr2);
static void vecset(int n, double v[], int iv[], int *nzv, int i, double val);
//---------------------------------------------------------------------

void mul(HTA* h1, HTA* h2, void* scalar) {
    double val = *((double*)scalar);
    double *d1 = (double*) HTA_get_ptr_raw_data(h1);
    double *d2 = (double*) HTA_get_ptr_raw_data(h2);
    int num_scalars = h1->leaf.num_elem;
    for(int i = 0; i < num_scalars; i++)
    {
        *d1 = *d2 * val;
        d1++;
        d2++;
    }
}

void muladd(HTA* h1, HTA* h2, void* scalar) {
    double val = *((double*)scalar);
    double *d1 = (double*) HTA_get_ptr_raw_data(h1);
    double *d2 = (double*) HTA_get_ptr_raw_data(h2);
    int num_scalars = h1->leaf.num_elem;
    for(int i = 0; i < num_scalars; i++)
    {
        *d1 += *d2 * val;
        d1++;
        d2++;
    }
}

void muladd2(HTA* h1, HTA* h2, void* scalar) {
    double val = *((double*)scalar);
    double *d1 = (double*) HTA_get_ptr_raw_data(h1);
    double *d2 = (double*) HTA_get_ptr_raw_data(h2);
    int num_scalars = h1->leaf.num_elem;
    for(int i = 0; i < num_scalars; i++)
    {
        *d1 = *d2 + (val * *d1);
        d1++;
        d2++;
    }
}

void sum_sq_pwsub(HTA* h1, HTA* h2, void* s)
{
    int i;
    double sum = 0;
    double x;

    double* p1 = (double*) HTA_get_ptr_raw_data(h1);
    double* p2 = (double*) HTA_get_ptr_raw_data(h2);
    int num_scalars = h1->leaf.num_elem;
    for(i = 0; i < num_scalars; i++)
    {
        x = p1[i] - p2[i];
        sum += x * x;
    }

    *((double*)s) = sum;
}

void sum_pwmul(HTA* h1, HTA* h2, void* s)
{
    int i;
    double sum = 0;

    double* p1 = (double*) HTA_get_ptr_raw_data(h1);
    double* p2 = (double*) HTA_get_ptr_raw_data(h2);
    int num_scalars = h1->leaf.num_elem;
    for(i = 0; i < num_scalars; i++)
        sum += p1[i] * p2[i];

    *((double*)s) = sum;
}

void sum_sq(HTA* h1, HTA* dummy, void* s)
{
    int i;
    double sum = 0;
    double x;

    double* p1 = (double*) HTA_get_ptr_raw_data(h1);
    int num_scalars = h1->leaf.num_elem;
    for(i = 0; i < num_scalars; i++)
    {
        x = p1[i];
        sum += x * x;
    }

    *((double*)s) = sum;
}

//void merge_tiles(HTA* hr, HTA* hd)
//{
//    int offset = hr->nd_element_offset.values[0];
//    double* src = (double*)HTA_get_ptr_raw_data(hr);
//    double* dest = (double*)HTA_get_ptr_raw_data(hd);
//    memcpy(dest + offset, src, sizeof(double) * hr->leaf.num_elem);
//}

void sdmv(HTA* hr, HTA* hs, HTA* hd)
{
    ASSERT(hr->type == HTA_TYPE_DENSE
            && hs->type == HTA_TYPE_SPARSE
            && hd->type == HTA_TYPE_DENSE);
    ASSERT(hr->height == 1 && hs->height == 1 && hd->height == 2);
    ASSERT(hr->scalar_type == hs->scalar_type && hs->scalar_type == hd->scalar_type);

    int nnz = hs->leaf.num_elem;

    if(nnz == 0) // all zero tile
        return;

    int num_rows = hs->flat_size.values[0];
    void* raw = HTA_get_ptr_raw_data(hs);
    double *val_ptr = (double*) raw;
    int* col_ind = (int*) (raw + nnz * sizeof(double));
    int* row_ptr = (int*) (raw + nnz * sizeof(double) + nnz * sizeof(int));
    //int tile_idx = hs->nd_rank.values[1];
    HTA* hv = hd->tiles[0];
    double* hr_val = (double*) HTA_get_ptr_raw_data(hr);
    double* hv_val = (double*) HTA_get_ptr_raw_data(hv);
    for(int i = 0; i < num_rows; i++)
    {
        double sum = 0.0;
        for(int j = row_ptr[i]; j < row_ptr[i+1]; j++)
        {
            int col = col_ind[j];
            sum += val_ptr[j] * hv_val[col];
        }
        hr_val[i] = sum;
    }
}

void sdmv_h2s1(HTA* hr, HTA* hs, void* d)
{
    ASSERT(hr->type == HTA_TYPE_DENSE
            && hs->type == HTA_TYPE_SPARSE);
    ASSERT(hr->height == 1 && hs->height == 1);
    ASSERT(hr->scalar_type == hs->scalar_type);

    int nnz = hs->leaf.num_elem;

    if(nnz == 0) // all zero tile
        return;

    int num_rows = hs->flat_size.values[0];
    void* raw = HTA_get_ptr_raw_data(hs);
    double *val_ptr = (double*) raw;
    int* col_ind = (int*) (raw + nnz * sizeof(double));
    int* row_ptr = (int*) (raw + nnz * sizeof(double) + nnz * sizeof(int));
    //int tile_idx = hs->nd_rank.values[1];
    double* hr_val = (double*) HTA_get_ptr_raw_data(hr);
    double* hv_val = *(double**)d; // since the value is actually a pointer to double array, take the value first and then cast it to double array pointer
    for(int i = 0; i < num_rows; i++)
    {
        double sum = 0.0;
        for(int j = row_ptr[i]; j < row_ptr[i+1]; j++)
        {
            int col = col_ind[j];
            sum += val_ptr[j] * hv_val[col];
        }
        hr_val[i] = sum;
    }
}
// This operation is a special purpose repmat_transpose
// duplicate each column tile of h to multiple hr tiles of the same row
void repmat_transpose(HTA* hr, HTA* dummy, HTA* h) {
    int col_idx = hr->nd_rank.values[1];
    HTA* source_leaf = h->tiles[col_idx]; // copy from the vector the leaf idx == col idx
    double* p1 = (double*) HTA_get_ptr_raw_data(hr);
    double* p2 = (double*) HTA_get_ptr_raw_data(source_leaf);
    ASSERT(hr->leaf.num_elem == source_leaf->leaf.num_elem);
    memcpy(p1, p2, hr->leaf.num_elem * sizeof(double));
}
void even(HTA* hr, HTA* dummy, HTA* h) {
    int row_idx = hr->nd_rank.values[0];
    HTA* source_leaf = h->tiles[row_idx];
    double* p1 = (double*) HTA_get_ptr_raw_data(hr);
    double* p2 = (double*) HTA_get_ptr_raw_data(source_leaf);
    ASSERT(hr->leaf.num_elem == source_leaf->leaf.num_elem);
    for(int i = 0; i < hr->leaf.num_elem; i++)
        p1[i] = p2[i] / NPCOLS;
}

int hta_main(int argc, char *argv[], int pid)
{
  int i,  it;
  HTA* x_HTA;
  HTA* z_HTA;
  HTA* p_HTA;
  HTA* q_HTA;
  HTA* r_HTA;
  HTA* hs;         // the sparse array

  double zeta;
  double rnorm;
  double norm_temp1, norm_temp2;

  double t, mflops, tmax;
  char Class;
  logical verified;
  double epsilon, err;

  char *t_names[T_last];

  SET_SPMD_PID(pid)

  if (argc == 2)
  {
      PROC = atoi(argv[1]);
  }

  MASTER_EXEC_START
  for (i = 0; i < T_last; i++) {
    timer_clear(i);
  }

  FILE *fp;
  if ((fp = fopen("timer.flag", "r")) != NULL) {
    timeron = true;
    t_names[T_init] = "init";
    t_names[T_bench] = "benchmk";
    t_names[T_conj_grad] = "conjgd";
    t_names[T_sdmv] = "sdmv";
    t_names[T_flatten] = "flatten";
    fclose(fp);
  } else {
    timeron = false;
  }

  timer_start(T_init);

  if (NA == 1400 && NONZER == 7 && NITER == 15 && SHIFT == 10) {
    Class = 'S';
    zeta_verify_value = 8.5971775078648;
  } else if (NA == 7000 && NONZER == 8 && NITER == 15 && SHIFT == 12) {
    Class = 'W';
    zeta_verify_value = 10.362595087124;
  } else if (NA == 14000 && NONZER == 11 && NITER == 15 && SHIFT == 20) {
    Class = 'A';
    zeta_verify_value = 17.130235054029;
  } else if (NA == 75000 && NONZER == 13 && NITER == 75 && SHIFT == 60) {
    Class = 'B';
    zeta_verify_value = 22.712745482631;
  } else if (NA == 150000 && NONZER == 15 && NITER == 75 && SHIFT == 110) {
    Class = 'C';
    zeta_verify_value = 28.973605592845;
  } else if (NA == 1500000 && NONZER == 21 && NITER == 100 && SHIFT == 500) {
    Class = 'D';
    zeta_verify_value = 52.514532105794;
  } else if (NA == 9000000 && NONZER == 26 && NITER == 100 && SHIFT == 1500) {
    Class = 'E';
    zeta_verify_value = 77.522164599383;
  } else {
    Class = 'U';
  }

  printf("\n\n NAS Parallel Benchmarks (NPB3.3-HTA-C) - CG Benchmark\n\n");
  printf(" Size: %11d\n", NA);
  printf(" Iterations:                  %5d\n", NITER);
  printf(" Number of available threads: %5d\n", HTA_get_num_processes());
  printf("\n");

  // compute 2d tasks row/column number
  if(PROC & (PROC - 1)) // the expression will be 0 if x is a power of 2
  {
    printf("The number of processes has to be power of 2!\n");
    exit(1);
  }

  NPROWS=PROC;
  NPCOLS=1;

  printf("NPROWS=%d, NPCOLS=%d\n", NPROWS, NPCOLS);

  IsSquare = (NPROWS == NPCOLS);

  naa = NA;
  //nzz = NZ;
  NZZ = NA*(NONZER+1)*(NONZER+1)/(NPROWS*NPCOLS)
          + NA*(NONZER+2+(NPROWS*NPCOLS)/256)/NPCOLS;

  printf("NAA = %d, NZZ = %d\n", naa, NZZ);
  MASTER_EXEC_SYNC_END



  //---------------------------------------------------------------------
  //
  //---------------------------------------------------------------------
  //makea(naa, nzz, a, colidx, rowstr,
  //      firstrow, lastrow, firstcol, lastcol,
  //      arow,
  //      (int (*)[NONZER+1])(void*)acol,
  //      (double (*)[NONZER+1])(void*)aelt,
  //      iv);

  //---------------------------------------------------------------------
  // Note: as a result of the above call to makea:
  //      values of j used in indexing rowstr go from 0 --> lastrow-firstrow
  //      values of colidx which are col indexes go from firstcol --> lastcol
  //      So:
  //      Shift the col index vals from actual (firstcol --> lastcol )
  //      to local, i.e., (0 --> lastcol-firstcol)
  //---------------------------------------------------------------------
  //for (int j = 0; j < lastrow - firstrow + 1; j++) {
  //  for (int k = rowstr[j]; k < rowstr[j+1]; k++) {
  //    colidx[k] = colidx[k] - firstcol;
  //  }
  //}

  MASTER_PRINTF("HTA initialization starts\n");
  //---------------------------------------------------------------------
  // HTA definitions
  //---------------------------------------------------------------------
  Tuple tp0 = Tuple_create(1, NPROWS);
  Tuple fs0 = Tuple_create(1, NA);
  Dist dist0;
  Tuple mesh;
  Tuple_init(&mesh, 1, NPROWS);
  Dist_init(&dist0, DIST_BLOCK, &mesh);
  // initialize vectors
  int layout;
  if(CFG_get(CFG_ALLOW_SHARED_DATA)) {
      layout = ORDER_ROW;
  } else {
      layout = ORDER_TILE;
  }
  x_HTA = HTA_create_with_pid(pid, 1, 2, &fs0, layout, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, tp0);
  z_HTA = HTA_create_with_pid(pid, 1, 2, &fs0, layout, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, tp0);
  q_HTA = HTA_create_with_pid(pid, 1, 2, &fs0, layout, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, tp0);
  r_HTA = HTA_create_with_pid(pid, 1, 2, &fs0, layout, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, tp0);
  p_HTA = HTA_create_with_pid(pid, 1, 2, &fs0, layout, &dist0, HTA_SCALAR_TYPE_DOUBLE, 1, tp0);
  double *tmp = malloc(NA*sizeof(double));

  // sparse HTA initialization
  Tuple tp2 = Tuple_create(2, NPROWS, NPCOLS);
  Tuple fs2 = Tuple_create(2, NA, NA);
  Dist dist1;
  Tuple mesh1;
  Tuple_init(&mesh1, 2, NPROWS, NPCOLS); // NOTICE: This is actually 1D because NPCOLS == 1
  Dist_init(&dist1, DIST_BLOCK, &mesh1);
  // create an empty shell
  hs = HTA_sparse_create_with_pid(pid, 2, 2, &fs2, 0, &dist1, HTA_SCALAR_TYPE_DOUBLE, 1, tp2);
  HTA_map_h1(HTA_LEAF_LEVEL(hs), makea_HTA, hs);
  if(CFG_get(CFG_ALLOW_SHARED_DATA)) {
      HTA_make_shared_all_leaves(hs); // explicitly synchronize pointer to leaves
  }

  //---------------------------------------------------------------------
  // set starting vector to (1, 1, .... 1)
  //---------------------------------------------------------------------
  //for (i = 0; i < NA+1; i++) {
  //   x[i] = 1.0;
  //}

  HTA_map_h1s1(HTA_LEAF_LEVEL(x_HTA), H1S1_INIT, x_HTA, &one);
  HTA_map_h1s1(HTA_LEAF_LEVEL(z_HTA), H1S1_INIT, z_HTA, &zero);
  HTA_map_h1s1(HTA_LEAF_LEVEL(q_HTA), H1S1_INIT, q_HTA, &zero);
  HTA_map_h1s1(HTA_LEAF_LEVEL(r_HTA), H1S1_INIT, r_HTA, &zero);
  HTA_map_h1s1(HTA_LEAF_LEVEL(p_HTA), H1S1_INIT, p_HTA, &zero);

  MASTER_PRINTF("HTA initialization done\n");
  zeta = 0.0;
  //---------------------------------------------------------------------
  //---->
  // Do one iteration untimed to init all code and data page tables
  //---->                    (then reinit, start timing, to niter its)
  //---------------------------------------------------------------------
  for (it = 1; it <= 1; it++) {
    //---------------------------------------------------------------------
    // The call to the conjugate gradient routine:
    //---------------------------------------------------------------------
    //conj_grad(colidx, rowstr, x, z, a, p, q, r, &rnorm);
    logical timer_enabled;
    MASTER_EXEC_START
    timer_enabled = timeron;
    timeron = false;
    MASTER_EXEC_END

    conj_grad(&rnorm, x_HTA, z_HTA, p_HTA, q_HTA, r_HTA, hs, tmp);

    MASTER_EXEC_START
    timeron = timer_enabled;
    MASTER_EXEC_END
    //---------------------------------------------------------------------
    // zeta = shift + 1/(x.z)
    // So, first: (x.z)
    // Also, find norm of z
    // So, first: (z.z)
    //---------------------------------------------------------------------
    norm_temp1 = 0.0;
    norm_temp2 = 0.0;
    //#pragma omp parallel for default(shared) private(j)
    //                         reduction(+:norm_temp1,norm_temp2)
    //for (j = 0; j < lastcol - firstcol + 1; j++) {
    //  norm_temp1 = norm_temp1 + x[j] * z[j];
    //  norm_temp2 = norm_temp2 + z[j] * z[j];
    //}
    HTA_reduce_h2(REDUCE_SUM, sum_pwmul, &norm_temp1, x_HTA, z_HTA);
    HTA_reduce_h2(REDUCE_SUM, sum_sq, &norm_temp2, z_HTA, z_HTA);
    //norm_temp1 /= (double)NPROWS;
    //norm_temp2 /= (double)NPROWS;
    //printf("norm_temp1 = %.4f, norm_temp2 = %.4f\n", norm_temp1, norm_temp2);

    norm_temp2 = 1.0 / sqrt(norm_temp2);

    //---------------------------------------------------------------------
    // Normalize z to obtain x
    //---------------------------------------------------------------------
    //#pragma omp parallel for default(shared) private(j)
    //for (j = 0; j < lastcol - firstcol + 1; j++) {
    //  x[j] = norm_temp2 * z[j];
    //}
    HTA_map_h2s1(HTA_LEAF_LEVEL(x_HTA), mul, x_HTA, z_HTA, &norm_temp2);

  } // end of do one iteration untimed


  //printf("Run one iteration done\n");
  //---------------------------------------------------------------------
  // set starting vector to (1, 1, .... 1)
  //---------------------------------------------------------------------
  //#pragma omp parallel for default(shared) private(i)
  //for (i = 0; i < NA+1; i++) {
  //  x[i] = 1.0;
  //}
  HTA_map_h1s1(HTA_LEAF_LEVEL(x_HTA), H1S1_INIT, x_HTA, &one);

  zeta = 0.0;

  MASTER_EXEC_START
  timer_stop(T_init);
  printf(" Initialization time = %15.3f seconds\n", timer_read(T_init));
  timer_start(T_bench);
  MASTER_EXEC_END

  //---------------------------------------------------------------------
  //---->
  // Main Iteration for inverse power method
  //---->
  //---------------------------------------------------------------------
  for (it = 1; it <= NITER; it++) {
    //---------------------------------------------------------------------
    // The call to the conjugate gradient routine:
    //---------------------------------------------------------------------
    MASTER_EXEC_START
    if (timeron) timer_start(T_conj_grad);
    MASTER_EXEC_END
    //---------------------------------------------------------------------
    // Initialize the CG algorithm:
    //---------------------------------------------------------------------
    conj_grad(&rnorm, x_HTA, z_HTA, p_HTA, q_HTA, r_HTA, hs, tmp);
    MASTER_EXEC_START
    if (timeron) timer_stop(T_conj_grad);
    MASTER_EXEC_END

    //---------------------------------------------------------------------
    // zeta = shift + 1/(x.z)
    // So, first: (x.z)
    // Also, find norm of z
    // So, first: (z.z)
    //---------------------------------------------------------------------
    norm_temp1 = 0.0;
    norm_temp2 = 0.0;
    //#pragma omp parallel for default(shared) private(j)
    //                         reduction(+:norm_temp1,norm_temp2)
    //for (j = 0; j < lastcol - firstcol + 1; j++) {
    //  norm_temp1 = norm_temp1 + x[j]*z[j];
    //  norm_temp2 = norm_temp2 + z[j]*z[j];
    //}
    HTA_reduce_h2(REDUCE_SUM, sum_pwmul, &norm_temp1, x_HTA, z_HTA);
    HTA_reduce_h2(REDUCE_SUM, sum_sq, &norm_temp2, z_HTA, z_HTA);
    norm_temp1 /= (double)NPROWS;
    norm_temp2 /= (double)NPROWS;
    //printf("norm_temp1 = %.4f, norm_temp2 = %.4f\n", norm_temp1, norm_temp2);

    norm_temp2 = 1.0 / sqrt(norm_temp2);

    zeta = SHIFT + 1.0 / norm_temp1;
    MASTER_EXEC_START
    if (it == 1)
      printf("\n   iteration           ||r||                 zeta\n");
    printf("    %5d       %20.14E%20.13f\n", it, rnorm, zeta);
    MASTER_EXEC_END

    //---------------------------------------------------------------------
    // Normalize z to obtain x
    //---------------------------------------------------------------------
    //#pragma omp parallel for default(shared) private(j)
    //for (j = 0; j < lastcol - firstcol + 1; j++) {
    //  x[j] = norm_temp2 * z[j];
    //}
    HTA_map_h2s1(HTA_LEAF_LEVEL(x_HTA), mul, x_HTA, z_HTA, &norm_temp2);

  } // end of main iter inv pow meth
  MASTER_EXEC_START
  timer_stop(T_bench);
  MASTER_EXEC_END

  //---------------------------------------------------------------------
  // End of timed section
  //---------------------------------------------------------------------

  MASTER_EXEC_START
  t = timer_read(T_bench);
  printf(" Benchmark completed\n");
  MASTER_EXEC_END

  epsilon = 1.0e-10;
  if (Class != 'U') {
    err = fabs(zeta - zeta_verify_value) / zeta_verify_value;
    if (err <= epsilon) {
      verified = true;
      MASTER_PRINTF(" VERIFICATION SUCCESSFUL\n");
      MASTER_PRINTF(" Zeta is    %20.13E\n", zeta);
      MASTER_PRINTF(" Error is   %20.13E\n", err);
    } else {
      verified = false;
      MASTER_PRINTF(" VERIFICATION FAILED\n");
      MASTER_PRINTF(" Zeta                %20.13E\n", zeta);
      MASTER_PRINTF(" The correct zeta is %20.13E\n", zeta_verify_value);
    }
  } else {
    verified = false;
    MASTER_PRINTF(" Problem size unknown\n");
    MASTER_PRINTF(" NO VERIFICATION PERFORMED\n");
  }

  MASTER_EXEC_START
  if (t != 0.0) {
    mflops = (double)(2*NITER*NA)
                   * (3.0+(double)(NONZER*(NONZER+1))
                     + 25.0*(5.0+(double)(NONZER*(NONZER+1)))
                     + 3.0) / t / 1000000.0;
  } else {
    mflops = 0.0;
  }

  print_results("CG", Class, NA, 0, 0,
                NITER, t,
                mflops, "          floating point",
                verified, NPBVERSION, COMPILETIME,
                CS1, CS2, CS3, CS4, CS5, CS6, CS7);

  //---------------------------------------------------------------------
  // More timers
  //---------------------------------------------------------------------
  if (timeron) {
    tmax = timer_read(T_bench);
    if (tmax == 0.0) tmax = 1.0;
    printf("  SECTION   Time (secs)\n");

    char rec_name[256];
    sprintf(rec_name, "rec_spmd/cg.%c.%d.rec", Class, PROC);
    FILE* fp_rec = fopen(rec_name, "a");
    fprintf(fp_rec, "%9.4f ", tmax);

    for (i = 0; i < T_last; i++) {
      t = timer_read(i);
      fprintf(fp_rec, "%9.4f ", t);
      if (i == T_init) {
        printf("  %8s:%9.3f\n", t_names[i], t);
      } else {
        printf("  %8s:%9.3f  (%6.2f%%)\n", t_names[i], t, t*100.0/tmax);
        if (i == T_conj_grad) {
          t = tmax - t;
          printf("    --> %8s:%9.3f  (%6.2f%%)\n", "rest", t, t*100.0/tmax);
        }
      }
    }
    fprintf(fp_rec, "\n");
    fclose(fp_rec);
  }
  MASTER_EXEC_SYNC_END

  HTA_destroy(x_HTA);
  HTA_destroy(z_HTA);
  HTA_destroy(q_HTA);
  HTA_destroy(r_HTA);
  HTA_destroy(p_HTA);
  HTA_destroy(hs);
  free(tmp);

  assert(verified);
  return (verified)?0:1;
}

//---------------------------------------------------------------------
// Floaging point arrays here are named as in NPB1 spec discussion of
// CG algorithm
//---------------------------------------------------------------------
//static void conj_grad(int colidx[],
//                      int rowstr[],
//                      double x[],
//                      double z[],
//                      double a[],
//                      double p[],
//                      double q[],
//                      double r[],
//                      double *rnorm)
static void conj_grad(double *rnorm, HTA* x_HTA, HTA* z_HTA, HTA* p_HTA, HTA* q_HTA, HTA* r_HTA, HTA* hs, double *tmp)

{
  int pid = x_HTA->pid;
  int cgit, cgitmax = 25;
  double d, sum, rho, rho0, alpha, beta; //, suml;
  double temp;

  SET_SPMD_PID(pid)
  rho = 0.0;
  sum = 0.0;

  //#pragma omp parallel default(shared) private(j,k,cgit,suml,alpha,beta) shared(d,rho0,rho,sum)
  //{
  //---------------------------------------------------------------------
  // Initialize the CG algorithm:
  //---------------------------------------------------------------------
  //#pragma omp for
  //for (j = 0; j < naa+1; j++) {
  //  q[j] = 0.0;
  //  z[j] = 0.0;
  //  r[j] = x[j];
  //  p[j] = r[j];
  //}
  HTA_map_h1s1(HTA_LEAF_LEVEL(z_HTA), H1S1_INIT, z_HTA, &zero);
  HTA_map_h1s1(HTA_LEAF_LEVEL(q_HTA), H1S1_INIT, q_HTA, &zero);
  HTA_map_h2(HTA_LEAF_LEVEL(r_HTA), H2_COPY, r_HTA, x_HTA);
  HTA_map_h2(HTA_LEAF_LEVEL(p_HTA), H2_COPY, p_HTA, r_HTA);

  //---------------------------------------------------------------------
  // rho = r.r
  // Now, obtain the norm of r: First, sum squares of r elements locally...
  //---------------------------------------------------------------------
  //#pragma omp for reduction(+:rho)
  //for (j = 0; j < lastcol - firstcol + 1; j++) {
  //  rho = rho + r[j]*r[j];
  //}
  HTA_reduce_h2(REDUCE_SUM, sum_sq, &rho, r_HTA, r_HTA); // parallelism here is NPCOLS
  //printf("rho1 = %.4f\n", rho);

  //---------------------------------------------------------------------
  //---->
  // The conj grad iteration loop
  //---->
  //---------------------------------------------------------------------
  for (cgit = 1; cgit <= cgitmax; cgit++) {
    //#pragma omp master
    //{
      //---------------------------------------------------------------------
      // Save a temporary of rho and initialize reduction variables
      //---------------------------------------------------------------------
      rho0 = rho;
      d = 0.0;
      rho = 0.0;
    //}
    //#pragma omp barrier

    //---------------------------------------------------------------------
    // q = A.p
    // The partition submatrix-vector multiply: use workspace w
    //---------------------------------------------------------------------

    //#pragma omp for
    //for (j = 0; j < lastrow - firstrow + 1; j++) {
    //  suml = 0.0;
    //  for (k = rowstr[j]; k < rowstr[j+1]; k++) {
    //    suml = suml + a[k]*p[colidx[k]];
    //  }
    //  q[j] = suml;
    //}

    //HTA_tile_to_hta2(HTA_LEAF_LEVEL(p_HTA), merge_tiles, p_HTA, t_HTA);  // to avoid using extra operations to generate pr_HTA
    //HTA_tile_to_hta(HTA_LEAF_LEVEL(q_HTA), H3_SDMV, q_HTA, hs, t_HTA);  // to avoid using extra operations to generate pr_HTA
    if(CFG_get(CFG_ALLOW_SHARED_DATA)) {
      MASTER_EXEC_START
      if (timeron) timer_start(T_sdmv);
      MASTER_EXEC_END
      HTA_tile_to_hta(HTA_LEAF_LEVEL(q_HTA), sdmv, q_HTA, hs, p_HTA);  // to avoid using extra operations to generate pr_HTA
      MASTER_EXEC_START
      if (timeron) timer_stop(T_sdmv);
      MASTER_EXEC_END
    } else {
      // flatten p_HTA
      MASTER_EXEC_START
      if (timeron) timer_start(T_flatten);
      MASTER_EXEC_END
      HTA_flatten(tmp, NULL, NULL, p_HTA);
      MASTER_EXEC_START
      if (timeron) timer_stop(T_flatten);
      MASTER_EXEC_END
      // perform sdmv using map h2s1
      // since h2s1 only pass the value of scalar to the map operator and in this case the value of pointer address must be stored
      // in a double variable
      MASTER_EXEC_START
      if (timeron) timer_start(T_sdmv);
      MASTER_EXEC_END
      HTA_map_h2s1(HTA_LEAF_LEVEL(q_HTA), sdmv_h2s1, q_HTA, hs, &tmp);
      MASTER_EXEC_START
      if (timeron) timer_stop(T_sdmv);
      MASTER_EXEC_END
    }

    /*
    for (j = 0; j < lastrow - firstrow + 1; j++) {
      int i = rowstr[j];
      int iresidue = (rowstr[j+1] - i) % 2;
      double sum1 = 0.0;
      double sum2 = 0.0;
      if (iresidue == 1)
        sum1 = sum1 + a[i]*p[colidx[i]];
      for (k = i + iresidue; k <= rowstr[j+1] - 2; k += 2) {
        sum1 = sum1 + a[k]  *p[colidx[k]];
        sum2 = sum2 + a[k+1]*p[colidx[k+1]];
      }
      q[j] = sum1 + sum2;
    }

    for (j = 0; j < lastrow - firstrow + 1; j++) {
      int i = rowstr[j];
      int iresidue = (rowstr[j+1] - i) % 8;
      suml = 0.0;
      for (k = i; k <= i + iresidue - 1; k++) {
        suml = suml + a[k]*p[colidx[k]];
      }
      for (k = i + iresidue; k <= rowstr[j+1] - 8; k += 8) {
        suml = suml + a[k  ]*p[colidx[k  ]]
                  + a[k+1]*p[colidx[k+1]]
                  + a[k+2]*p[colidx[k+2]]
                  + a[k+3]*p[colidx[k+3]]
                  + a[k+4]*p[colidx[k+4]]
                  + a[k+5]*p[colidx[k+5]]
                  + a[k+6]*p[colidx[k+6]]
                  + a[k+7]*p[colidx[k+7]];
      }
      q[j] = suml;
    }
    */

    //---------------------------------------------------------------------
    // Obtain p.q
    //---------------------------------------------------------------------
    //#pragma omp for reduction(+:d)
    //for (j = 0; j < lastcol - firstcol + 1; j++) {
    //  d = d + p[j]*q[j];
    //}
    /*
#ifdef PREALLOC
    HTA_partial_reduce_with_preallocated(REDUCE_SUM, qr_HTA, 1, &zero, qt_HTA, q_HTA);
#else
    q_HTA = HTA_partial_reduce(REDUCE_SUM, qr_HTA, 1, &zero);
#endif
    */
    //for(int i = 0; i < q_HTA->num_tiles; i++) {
    //    printf("q_HTA tile %d\n", i);
    //    for(int j = 0; j < q_HTA->tiles[i]->leaf.num_elem; j++)
    //        printf("j = %d, val = %.4lf\n", j, ((double*)HTA_get_ptr_raw_data(q_HTA->tiles[i]))[j]);
    //}
    //checksum = 0.0;
    //HTA_full_reduce(REDUCE_SUM, &checksum, q_HTA);
    //printf("q_HTA_checksum = %lf\n", checksum);
    HTA_reduce_h2(REDUCE_SUM, sum_pwmul, &d, p_HTA, q_HTA);
    //printf("d = %.4f\n", d);

    //---------------------------------------------------------------------
    // Obtain alpha = rho / (p.q)
    //---------------------------------------------------------------------
    alpha = rho0 / d;
    //printf("alpha = %.4f\n", alpha);

    //---------------------------------------------------------------------
    // Obtain z = z + alpha*p
    // and    r = r - alpha*q
    //---------------------------------------------------------------------
    //#pragma omp for reduction(+:rho)
    //for (j = 0; j < lastcol - firstcol + 1; j++) {
    //  z[j] = z[j] + alpha*p[j];
    HTA_map_h2s1(HTA_LEAF_LEVEL(z_HTA), muladd, z_HTA, p_HTA, &alpha);
    //  r[j] = r[j] - alpha*q[j];
    temp = -alpha;
    HTA_map_h2s1(HTA_LEAF_LEVEL(r_HTA), muladd, r_HTA, q_HTA, &temp);

      //---------------------------------------------------------------------
      // rho = r.r
      // Now, obtain the norm of r: First, sum squares of r elements locally..
      //---------------------------------------------------------------------
    //  rho = rho + r[j]*r[j];
    //}
    HTA_reduce_h2(REDUCE_SUM, sum_sq, &rho, r_HTA, r_HTA);
    //printf("rho2 = %.4f\n", rho);

    //---------------------------------------------------------------------
    // Obtain beta:
    //---------------------------------------------------------------------
    beta = rho / rho0;

    //---------------------------------------------------------------------
    // p = r + beta*p
    //---------------------------------------------------------------------
    //#pragma omp for
    //for (j = 0; j < lastcol - firstcol + 1; j++) {
    //  p[j] = r[j] + beta*p[j];
    //}

    HTA_map_h2s1(HTA_LEAF_LEVEL(p_HTA), muladd2, p_HTA, r_HTA, &beta);
    if(CFG_get(CFG_ALLOW_SHARED_DATA)) {
      HTA_barrier(pid); // Barrier required before the next interation when SDMV accesses all tiles of p_HTA
    }
  } // end of do cgit=1,cgitmax

  //---------------------------------------------------------------------
  // Compute residual norm explicitly:  ||r|| = ||x - A.z||
  // First, form A.z
  // The partition submatrix-vector multiply
  //---------------------------------------------------------------------
  //#pragma omp for
  //for (j = 0; j < lastrow - firstrow + 1; j++) {
  //  suml = 0.0;
  //  for (k = rowstr[j]; k < rowstr[j+1]; k++) {
  //    suml = suml + a[k]*z[colidx[k]];
  //  }
  //  r[j] = suml;
  //}

  if(CFG_get(CFG_ALLOW_SHARED_DATA)) {
      MASTER_EXEC_START
      if (timeron) timer_start(T_sdmv);
      MASTER_EXEC_END
      HTA_tile_to_hta(HTA_LEAF_LEVEL(r_HTA), sdmv, r_HTA, hs, z_HTA);  // to avoid using extra operations to generate zr_HTA
      MASTER_EXEC_START
      if (timeron) timer_stop(T_sdmv);
      MASTER_EXEC_END
  } else {
    // flatten z_HTA
      MASTER_EXEC_START
      if (timeron) timer_start(T_flatten);
      MASTER_EXEC_END
      HTA_flatten(tmp, NULL, NULL, z_HTA);
      MASTER_EXEC_START
      if (timeron) timer_stop(T_flatten);
      MASTER_EXEC_END
    // perform sdmv using map h2s1
    // since h2s1 only pass the value of scalar to the map operator and in this case the value of pointer address must be stored
    // in a double variable
      MASTER_EXEC_START
      if (timeron) timer_start(T_sdmv);
      MASTER_EXEC_END
      HTA_map_h2s1(HTA_LEAF_LEVEL(q_HTA), sdmv_h2s1, r_HTA, hs, &tmp);
      MASTER_EXEC_START
      if (timeron) timer_stop(T_sdmv);
      MASTER_EXEC_END
  }

  //---------------------------------------------------------------------
  // At this point, r contains A.z
  //---------------------------------------------------------------------
  //#pragma omp for reduction(+:sum) nowait
  //for (j = 0; j < lastcol-firstcol+1; j++) {
  //  suml = x[j] - r[j];
  //  sum  = sum + suml*suml;
  //}
  HTA_reduce_h2(REDUCE_SUM, sum_sq_pwsub, &sum, x_HTA, r_HTA);
  //printf("sum = %.4f\n", sum);

  // } // End of parallel section

  *rnorm = sqrt(sum);
}

// this is a short cut but initialization is not timed so
// it should not matter. However, it only works for shared memory
static void makea_HTA(HTA *h)
{
  Tuple nd_rank = h->nd_rank;

  int firstrow = (nd_rank.values[0] * NA) / NPROWS;
  int lastrow = ((nd_rank.values[0] +1)* NA) / NPROWS - 1;
  int firstcol = (nd_rank.values[1] * NA) / NPCOLS;
  int lastcol = ((nd_rank.values[1] +1)* NA) / NPCOLS - 1;
  //printf("tile %d: firstrow = %d, lastrow = %d, firstcol = %d, lastcol = %d\n",
  //        h->rank, firstrow, lastrow, firstcol, lastcol);
  //printf("tile %d: NONZER = %d,  NZ = %d, NZZ = %d\n", h->rank, NONZER, NZ, NZZ);
  double *a = malloc(sizeof(double) * NZZ);
  int *colidx = malloc(sizeof(int) * NZZ);
  int *rowstr = malloc(sizeof(int) * (lastrow-firstrow+2));

  makea(h->rank, naa, NZZ, a, colidx, rowstr,
        firstrow, lastrow, firstcol, lastcol);

  //---------------------------------------------------------------------
  // Note: as a result of the above call to makea:
  //      values of j used in indexing rowstr go from 0 --> lastrow-firstrow
  //      values of colidx which are col indexes go from firstcol --> lastcol
  //      So:
  //      Shift the col index vals from actual (firstcol --> lastcol )
  //      to local, i.e., (0 --> lastcol-firstcol)
  //---------------------------------------------------------------------
  //for (int j = 0; j < lastrow - firstrow + 1; j++) {
  //  for (int k = rowstr[j]; k < rowstr[j+1]; k++) {
  //    //colidx[k] = colidx[k] - firstcol;
  //    printf("k = %d, colidx = %d\n", k, colidx[k]);
  //  }
  //}

  int nnz = rowstr[lastrow-firstrow+1] - rowstr[0];
  //printf("There are %d nonzeros in tile %d\n", nnz, h->rank);
  HTA_init_sparse_leaf(h, nnz, a, colidx, rowstr);

  free(a);
  free(colidx);
  free(rowstr);
}

//---------------------------------------------------------------------
// generate the test problem for benchmark 6
// makea generates a sparse matrix with a
// prescribed sparsity distribution
//
// parameter    type        usage
//
// input
//
// n            i           number of cols/rows of matrix
// nz           i           nonzeros as declared array size
// rcond        r*8         condition number
// shift        r*8         main diagonal shift
//
// output
//
// a            r*8         array for nonzeros
// colidx       i           col indices
// rowstr       i           row pointers
//
// workspace
//
// iv, arow, acol i
// aelt           r*8
//---------------------------------------------------------------------
static void makea(int rank,
                  int n,
                  int nz,
                  double values[],
                  int colidx[],
                  int rowstr[],
                  int firstrow,
                  int lastrow,
                  int firstcol,
                  int lastcol)
                  //int arow[],
                  //int acol[][NONZER+1],
                  //double aelt[][NONZER+1],
                  //int iv[])
{
  int iouter; //, ivelt, nzv, nn1;
  double *v = malloc(sizeof(double) * (NONZER+1));
  int* iv = malloc(sizeof(int) *(2*n+1));
  int* arow = malloc(sizeof(int) * (nz+1));
  int* acol = malloc(sizeof(int) * (nz+1));
  double* element = malloc(sizeof(double) * (nz + 1));
  int* rowidx = malloc(sizeof(int) *(2*n+1));
  // Code from HTA C++ CG implementation
  double size = 1.0;
  double ratio = pow(RCOND,  (size / (double)(n)));
  int  nnza = 0;

  //---------------------------------------------------------------------
  // Inialize random number generator
  //---------------------------------------------------------------------
  double tran    = 314159265.0;
  randlc(&tran, AMULT);

  //printf("makea (%d): n = %d, nz = %d, firstrow = %d, lastrow = %d, firstcol = %d, lastcol = %d\n",
  //        rank, n, nz, firstrow, lastrow, firstcol, lastcol);
  int* numelem = malloc(sizeof(int) * (lastrow - firstrow + 2));
  for (int i = 0; i < lastrow - firstrow + 2; i++) {
    numelem[i] = 0;
  }
  for (int i = 0; i < n; i++) {
    assert (n+i < 2*n+1);
    rowidx[n+i] = 0;
  }
  //printf("makea (%d): enter first loop\n", rank);
  for (iouter = 0; iouter < n; iouter++) {
    int nzv = NONZER;
    sprnvc (n, nzv, v, iv, &(rowidx[0]), &(rowidx[n]), &tran);
    vecset (n, v, iv, &nzv, iouter, (double) 0.50);
     for (int ivelt = 0; ivelt < nzv; ivelt++) {
      int jcol = iv[ivelt];
      if (jcol >= firstcol && jcol <= lastcol) {
	double scale = size * v[ivelt];
	for (int ivelt1 = 0; ivelt1 < nzv; ivelt1++) {
	  int irow = iv[ivelt1];
	  if (irow >= firstrow && irow <= lastrow) {
	    assert (nnza <= nz);
	    acol[nnza] = jcol - firstcol;
	    arow[nnza] = irow - firstrow;
	    element[nnza] = v[ivelt1] * scale;
	    numelem[irow - firstrow + 1]++;
	    nnza = nnza + 1;
	  }
	}
      }
    }
    size = size * ratio;
  }

  //printf("makea (%d): first loop result nnza = %d\n", rank, nnza);


  /**
   * ... add the identity * rcond to the generated matrix to bound
   *           the smallest eigenvalue from below by rcond
   * This step leads to duplicate value for a given (acol,arow).
   */
  for (int i = firstrow; i <= lastrow; i++){
    if (i >= firstcol && i <= lastcol) {
      iouter = n + i - 1;
      assert (nnza < nz+1);
      acol[nnza] = i - firstcol;
      arow[nnza] = i - firstrow;
      element[nnza] = RCOND - SHIFT;
      numelem[i - firstrow + 1]++;
      nnza = nnza + 1;
    }
  }
  //printf("second loop result nnza = %d\n", nnza);

  /**
   * Find the global starting positions
   * of each column in the values array
   */
  rowstr[0] = numelem[0];
  for (int i = 1; i < lastrow - firstrow + 2; i++) {
    numelem[i] += numelem[i-1];
    rowstr[i] = numelem[i];
  }

  /**
   * group all elements of row i together
   */
  for (int i = 0; i < nnza; i++) {
    int j = arow[i];
    int k = numelem[j];
    values[k] = element[i];
    colidx[k] = acol[i];
    numelem[j]++;
  }

  /**
   * Remove duplicates and generate
   * the final output
   */
  nnza = 0;
  //double *x = new double [n];
  //int *nzloc = new int[n];
  //bool *mark = new bool[n];
  double *x = malloc(sizeof(double) * n);
  int *nzloc = malloc(sizeof(int) * n);
  int *mark = malloc(sizeof(int) * n);

  for (int i = 0; i < n; i++) {
    x[i] = 0.0;
    nzloc[i] = 0;
    mark[i] = 0;
  }

  int jajpr = rowstr[0];
  for (int j = 0; j < lastrow - firstrow + 1; j++) {
    int nzrow = 0;
    for (int k = jajpr; k < rowstr[j+1]; k++) {
      int i = colidx[k];
      x[i] = x[i] + values[k];
      if (mark[i] == 0 && x[i] != 0.0){
	mark[i] = 1;
	nzloc[nzrow] = i;
	nzrow = nzrow + 1;
      }
    }

    for (int k = 0; k < nzrow; k++) {
      int i = nzloc[k];
      mark[i] = 0;
      /*
       * do not know why we need this temporary?
       * The FORTRAN version was written this way.
       * May be something to do with branch prediction..
      */
      double xi = x[i];
      x[i] = 0.0;
      if (xi != 0.0) {
	values[nnza] = xi;
	colidx[nnza] = i;
	nnza = nnza + 1;
      }
    }
    jajpr = rowstr[j+1];
    rowstr[j+1] = nnza + rowstr[0];
    //printf("nnza = %d, rowstr[%d] = %d\n", nnza, j+1, rowstr[j+1]);
  }

  free(v);
  free(iv);
  free(arow);
  free(acol);
  free(element);
  free(rowidx);
  free(numelem);
  free(x);
  free(nzloc);
  free(mark);
}

/*---------------------------------------------------------------------
  c       generate a sparse n-vector (v, iv)
  c       having nzv nonzeros
  c
  c       mark(i) is set to 1 if position i is nonzero.
  c       mark is all zero on entry and is reset to all zero before exit
  c       this corrects a performance bug found by John G. Lewis, caused by
  c       reinitialization of mark on every one of the n calls to sprnvc
  ---------------------------------------------------------------------*/
static void sprnvc(
		   int n,
		   int nz,
		   double v[],		/* v[1:*] */
		   int iv[],		/* iv[1:*] */
		   int nzloc[],	/* nzloc[1:n] */
		   int mark[],  	/* mark[1:n] */
                   double *tran)
{
  int nn1;
  int nzrow, nzv, ii, i;
  double vecelt, vecloc;

  nzv = 0;
  nzrow = 0;
  nn1 = 1;
  do {
    nn1 = 2 * nn1;
  } while (nn1 < n);

  /*--------------------------------------------------------------------
   *    nn1 is the smallest power of two not less than n
   *-------------------------------------------------------------------*/

  while (nzv < nz) {
    vecelt = randlc(tran, AMULT);

    /*--------------------------------------------------------------------
     *   generate an integer between 1 and n in a portable manner
     *-------------------------------------------------------------------*/
    vecloc =  randlc(tran, AMULT);
    i = icnvrt(vecloc, nn1) + 1;


    if (i > n) continue;

    /*--------------------------------------------------------------------
      c  was this integer generated already?
      c-------------------------------------------------------------------*/
    if (mark[i-1] == 0) {
      nzrow = nzrow + 1;
      nzv = nzv + 1;
      mark[i-1] = 1;
      nzloc[nzrow-1] = i-1;
      v[nzv-1] = vecelt;
      iv[nzv-1] = i-1;
    }
  }

  for (ii = 0; ii < nzrow; ii++) {
    i = nzloc[ii];
    mark[i] = 0;
  }
}

//---------------------------------------------------------------------
// scale a double precision number x in (0,1) by a power of 2 and chop it
//---------------------------------------------------------------------
static int icnvrt(double x, int ipwr2)
{
  return (int)(ipwr2 * x);
}


//---------------------------------------------------------------------
// set ith element of sparse vector (v, iv) with
// nzv nonzeros to val
//---------------------------------------------------------------------
static void vecset(int n, double v[], int iv[], int *nzv, int i, double val)
{
  int k;
  logical set;

  set = false;
  for (k = 0; k < *nzv; k++) {
    if (iv[k] == i) {
      v[k] = val;
      set  = true;
    }
  }
  if (set == false) {
    v[*nzv]  = val;
    iv[*nzv] = i;
    *nzv     = *nzv + 1;
  }
}

