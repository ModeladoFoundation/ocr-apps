// IRAD rights

/***---**--*
 *
 *  Copyright (C) 2007 Reservoir Labs. All rights reserved.
 *
 *--**---***/

/*
 * $Id$
 */

#ifndef __MICRO_KERNELS_H__
#define __MICRO_KERNELS_H__

#ifdef MC_DOUBLE_PRECISION
typedef double real_d;
typedef double real_t;
#define MKL_Complex_t MKL_Complex16
#define MKL_CGEMM     cblas_zgemm
#define MKL_CGEMV     cblas_zgemv
#define MKL_CNRM2     cblas_dznrm2
#define MKL_DOTHX     cblas_zdotc_sub
#define MKL_CSCAL     cblas_zdscal
#define MKL_CCSCAL    cblas_zscal
#define MKL_OUTER     cblas_zgeru
#define MKL_CQR       zgeqrf
#define MKL_CLU       zgetrf
#define MKL_CINV      zgetri
#define MKL_BKFAC     zhetrf
#define MKL_HSOLV     zhetrs
#define MKL_CAXPY     cblas_zaxpy

#define SQRT(x) sqrt(x)
#define COS(x)  cos(x)
#define SIN(x)  sin(x)
#define CST(x)  x
#define POW(x)  pow(x)
#define FMT     "%lf"
#else
typedef double real_d;
typedef float real_t;
#define MKL_Complex_t MKL_Complex8
#define MKL_CGEMM     cblas_cgemm
#define MKL_CGEMV     cblas_cgemv
#define MKL_CNRM2     cblas_scnrm2
#define MKL_DOTHX     cblas_cdotc_sub
#define MKL_CSCAL     cblas_csscal
#define MKL_CCSCAL    cblas_cscal
#define MKL_OUTER     cblas_cgeru
#define MKL_CQR       cgeqrf
#define MKL_CLU       cgetrf
#define MKL_CINV      cgetri
#define MKL_BKFAC     chetrf
#define MKL_HSOLV     chetrs
#define MKL_CAXPY     cblas_caxpy

#define SQRT(x) sqrtf(x)
#define COS(x)  cosf(x)
#define SIN(x)  sinf(x)
#define CST(x)  x ## f
#define POW(x)  powf
#define FMT     "%f"
#endif

extern int check_results; // check the answers (default 0)
extern int full_check_results; // fully check the answers (default 0)
extern int max_trials;    // number of trials to run (default 1)
extern int verbose;       // show extra messages during execution (default 0)
extern int init_only;     // only perform initialization (default 0)
extern int show_results;  // show the results (default 0)
extern double epsilon;    // tolerable errors.
extern int max_errors;

// flop count per operation type. Those are used in the total
// flop count estimation. They are defined in files called X-flops.c,
// where X is the architecture.
extern int add_flop;
extern int add_flop_f;
extern int add_flop_d;
extern int sub_flop;
extern int sub_flop_f;
extern int sub_flop_d;
extern int mul_flop;
extern int mul_flop_f;
extern int mul_flop_d;
extern int div_flop;
extern int div_flop_f;
extern int div_flop_d;
extern int sqrt_flop;
extern int sqrt_flop_f;
extern int sqrt_flop_d;
extern int exp_flop;
extern int exp_flop_f;
extern int exp_flop_d;
extern int cos_flop;

extern double fft1d_flop(long n);

// Must be defined by the client
extern double flops_per_trial(void); // Number of flops per iteration of the kernel
// number of samples. "sample" is defined in accordance with the kernel
extern int nb_samples;
// name of the kernel
extern char const * function_name;

// Initialization (run only once)
extern void initialize_once(void);
// Initialization (run before each trial)
extern void initialize(void);
// The kernel
extern void kernel(void);
// Show results
extern void show(void);
// Checking routine
extern int check(void);

// Prints the contents of a m*n matrix in rows [i0-i1) and columns [j0-j1)
extern void print_submatrix(const char * name,
                            const void * A, int m, int n,
                            int i0, int i1,
                            int j0, int j1);

// Prints the contents of a m*n matrix of complex data (matrix is
// defined like A[m][n][2]) in rows [i0-i1) and columns [j0-j1).
extern void print_complex_submatrix(const char * name,
                                    const void * A, int m, int n,
                                    int i0, int i1,
                                    int j0, int j1);

// Prints the contents of a m*n matrix in rows [i0-i1) and columns [j0-j1)
extern void print_int_submatrix(const char * name,
                                const void * A, int m, int n,
                                int i0, int i1,
                                int j0, int j1);

// Compare the contents of two m*n matrices.
// Returns 1 if they are different
extern int check_matrices(const char * name1,
                          const char * name2,
                          const void *A, const void *B, int m, int n);

// Compare the contents of subportions of two m*n matrices.
// Returns 1 if they are different
extern int check_matrices_portions(const char * name1,
				   const char * name2,
				   const void *A, const void *B,
				   int m, int n, int size);

// Compare the contents of two m*n matrices of complex data (matrix is
// defined like A[m][n][2]).
// Returns 1 if they are different
extern int check_complex_matrices(const char * name1,
                                  const char * name2,
                                  const void *A, const void *B, int m, int n);


/*
   Checks for a complex submatrix embedded in a bigger matrix at (0,0).
   This version must be used in parametric codes when the initialization and
   the check are not performed on the full static array.
   This usually happens when we conservatively allocate a big chunk of memory
   but we do not want to initialize it all and compare it all for performance
   issues.
             ____________________
            |                    |
            |_____________       |
            |            |       |
            |            |       |
            |____________|_______|
*/
extern int check_complex_matrices_portions(const char * name1, const char * name2,
					   const void * A, const void * B,
					   int m, int n, int Ndim);

// Compare the contents of two m vectors.
// Returns 1 if they are different
extern int check_vectors(const char * name1,
			 const char * name2,
			 const void *SX, const void *SY, int m);

extern int check_int_vectors(const char * name1, const char * name2,
			     const void * SX, const void * SY,
			     int m);

// Compares two doubles
// Returns 1 if they are different
extern int fequal(double x, double y);

#ifdef GPU_TARGET
extern int *host_state, *device_state;
extern void GPU_init_device();
extern void GPU_init_sync_variables();
extern void GPU_finalize_sync_variables();
extern void CUDAMEMCPY(void *A, void * B, unsigned long C, int D);
extern int CUDAMALLOC(void **ptr, unsigned long n);
extern int CUDAFREE(void *ptr);
extern void GPU_initialize_once();
extern void GPU_initialize();
extern void GPU_finalize_once();
#endif

#ifdef MONITOR_EXECUTION
extern int count_full_tiles;
extern int count_non_full_tiles;
#define COUNT_FULL_TILES() count_full_tiles++
#define COUNT_NON_FULL_TILES() count_non_full_tiles++
#define PRINT_MONITORING_STATS() printf("Full tiles=%d,\tNon-Full tiles=%d\n", \
					count_full_tiles,count_non_full_tiles)
#else
#define COUNT_FULL_TILES() ;
#define COUNT_NON_FULL_TILES() ;
#define PRINT_MONITORING_STATS() ;
#endif

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// End:
