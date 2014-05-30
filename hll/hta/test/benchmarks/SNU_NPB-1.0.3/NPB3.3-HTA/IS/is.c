/*************************************************************************
 *                                                                       * 
 *       N  A  S     P A R A L L E L     B E N C H M A R K S  3.3        *
 *                                                                       *
 *                      O p e n M P     V E R S I O N                    *
 *                                                                       * 
 *                                  I S                                  * 
 *                                                                       * 
 ************************************************************************* 
 *                                                                       * 
 *   This benchmark is an OpenMP version of the NPB IS code.             *
 *   It is described in NAS Technical Report 99-011.                     *
 *                                                                       *
 *   Permission to use, copy, distribute and modify this software        *
 *   for any purpose with or without fee is hereby granted.  We          *
 *   request, however, that all derived work reference the NAS           *
 *   Parallel Benchmarks 3.3. This software is provided "as is"          *
 *   without express or implied warranty.                                *
 *                                                                       *
 *   Information on NPB 3.3, including the technical report, the         *
 *   original specifications, source code, results and information       *
 *   on how to submit new results, is available at:                      *
 *                                                                       *
 *          http://www.nas.nasa.gov/Software/NPB/                        *
 *                                                                       *
 *   Send comments or suggestions to  npb@nas.nasa.gov                   *
 *                                                                       *
 *         NAS Parallel Benchmarks Group                                 *
 *         NASA Ames Research Center                                     *
 *         Mail Stop: T27A-1                                             *
 *         Moffett Field, CA   94035-1000                                *
 *                                                                       *
 *         E-mail:  npb@nas.nasa.gov                                     *
 *         Fax:     (650) 604-3957                                       *
 *                                                                       *
 ************************************************************************* 
 *                                                                       * 
 *   Author: M. Yarrow                                                   * 
 *           H. Jin                                                      * 
 *                                                                       * 
 *************************************************************************/

#include "npbparams.h"
#include <stdlib.h>
#include <stdio.h>
//#ifdef _OPENMP
//#include <omp.h>
//#endif

#include "HTA.h"
#include "HTA_operations.h"

/*****************************************************************/
/* For serial IS, buckets are not really req'd to solve NPB1 IS  */
/* spec, but their use on some machines improves performance, on */
/* other machines the use of buckets compromises performance,    */
/* probably because it is extra computation which is not req'd.  */
/* (Note: Mechanism not understood, probably cache related)      */
/* Example:  SP2-66MhzWN:  50% speedup with buckets              */
/* Example:  SGI Indy5000: 50% slowdown with buckets             */
/* Example:  SGI O2000:   400% slowdown with buckets (Wow!)      */
/*****************************************************************/
/* To disable the use of buckets, comment out the following line */
#define USE_BUCKETS

/* Uncomment below for cyclic schedule */
/*#define SCHED_CYCLIC*/


/******************/
/* default values */
/******************/
#ifndef CLASS
#define CLASS 'S'
#endif


/*************/
/*  CLASS S  */
/*************/
#if CLASS == 'S'
#define  TOTAL_KEYS_LOG_2    16
#define  MAX_KEY_LOG_2       11
#define  NUM_BUCKETS_LOG_2   9
#endif


/*************/
/*  CLASS W  */
/*************/
#if CLASS == 'W'
#define  TOTAL_KEYS_LOG_2    20
#define  MAX_KEY_LOG_2       16
#define  NUM_BUCKETS_LOG_2   10
#endif

/*************/
/*  CLASS A  */
/*************/
#if CLASS == 'A'
#define  TOTAL_KEYS_LOG_2    23
#define  MAX_KEY_LOG_2       19
#define  NUM_BUCKETS_LOG_2   10
#endif


/*************/
/*  CLASS B  */
/*************/
#if CLASS == 'B'
#define  TOTAL_KEYS_LOG_2    25
#define  MAX_KEY_LOG_2       21
#define  NUM_BUCKETS_LOG_2   10
#endif


/*************/
/*  CLASS C  */
/*************/
#if CLASS == 'C'
#define  TOTAL_KEYS_LOG_2    27
#define  MAX_KEY_LOG_2       23
#define  NUM_BUCKETS_LOG_2   10
#endif


/*************/
/*  CLASS D  */
/*************/
#if CLASS == 'D'
#define  TOTAL_KEYS_LOG_2    31
#define  MAX_KEY_LOG_2       27
#define  NUM_BUCKETS_LOG_2   10
#endif


#if CLASS == 'D'
#define  TOTAL_KEYS          (1L << TOTAL_KEYS_LOG_2)
#else
#define  TOTAL_KEYS          (1 << TOTAL_KEYS_LOG_2)
#endif
#define  MAX_KEY             (1 << MAX_KEY_LOG_2)
#define  NUM_BUCKETS         (1 << NUM_BUCKETS_LOG_2)
#define  NUM_KEYS            TOTAL_KEYS
#define  SIZE_OF_BUFFERS     NUM_KEYS  
                                           

#define  MAX_ITERATIONS      10
#define  TEST_ARRAY_SIZE     5


/*************************************/
/* Typedef: if necessary, change the */
/* size of int here by changing the  */
/* int type to, say, long            */
/*************************************/
#if CLASS == 'D'
typedef  long INT_TYPE;
#define SCALAR_TYPE HTA_SCALAR_TYPE_INT64
#else
typedef  int  INT_TYPE;
#define SCALAR_TYPE HTA_SCALAR_TYPE_INT32
#endif


/********************/
/* Some global info */
/********************/
INT_TYPE *key_buff_ptr_global;         /* used by full_verify to get */
                                       /* copies of rank info        */

int      passed_verification;
                                 

/************************************/
/* These are the three main arrays. */
/* See SIZE_OF_BUFFERS def above    */
/************************************/
INT_TYPE key_array[SIZE_OF_BUFFERS],
         partial_verify_vals[TEST_ARRAY_SIZE];

static HTA * key_array_HTA;
static HTA * key_buff1_HTA;
static HTA * key_buff2_HTA;
static HTA * local_sum_HTA;
static HTA * last_rank_HTA;

#ifdef USE_BUCKETS
//INT_TYPE **bucket_size, 
//         bucket_ptrs[NUM_BUCKETS];
//#pragma omp threadprivate(bucket_ptrs)
static HTA * bucket_size_HTA;
static HTA * bucket_ptrs_HTA;
#else
#error "Unimplemented" 
#endif

static int PROC=1;
static const int shift = MAX_KEY_LOG_2 - NUM_BUCKETS_LOG_2;

/**********************/
/* Partial verif info */
/**********************/
INT_TYPE test_index_array[TEST_ARRAY_SIZE],
         test_rank_array[TEST_ARRAY_SIZE],

         S_test_index_array[TEST_ARRAY_SIZE] = 
                             {48427,17148,23627,62548,4431},
         S_test_rank_array[TEST_ARRAY_SIZE] = 
                             {0,18,346,64917,65463},

         W_test_index_array[TEST_ARRAY_SIZE] = 
                             {357773,934767,875723,898999,404505},
         W_test_rank_array[TEST_ARRAY_SIZE] = 
                             {1249,11698,1039987,1043896,1048018},

         A_test_index_array[TEST_ARRAY_SIZE] = 
                             {2112377,662041,5336171,3642833,4250760},
         A_test_rank_array[TEST_ARRAY_SIZE] = 
                             {104,17523,123928,8288932,8388264},

         B_test_index_array[TEST_ARRAY_SIZE] = 
                             {41869,812306,5102857,18232239,26860214},
         B_test_rank_array[TEST_ARRAY_SIZE] = 
                             {33422937,10244,59149,33135281,99}, 

         C_test_index_array[TEST_ARRAY_SIZE] = 
                             {44172927,72999161,74326391,129606274,21736814},
         C_test_rank_array[TEST_ARRAY_SIZE] = 
                             {61147,882988,266290,133997595,133525895},

         D_test_index_array[TEST_ARRAY_SIZE] = 
                             {1317351170,995930646,1157283250,1503301535,1453734525},
         D_test_rank_array[TEST_ARRAY_SIZE] = 
                             {1,36538729,1978098519,2145192618,2147425337};


/***********************/
/* function prototypes */
/***********************/
double	randlc( double *X, double *A );

void full_verify( void );

void c_print_results( char   *name,
                      char   class,
                      int    n1, 
                      int    n2,
                      int    n3,
                      int    niter,
                      double t,
                      double mops,
		      char   *optype,
                      int    passed_verification,
                      char   *npbversion,
                      char   *compiletime,
                      char   *cc,
                      char   *clink,
                      char   *c_lib,
                      char   *c_inc,
                      char   *cflags,
                      char   *clinkflags );


void    timer_clear( int n );
void    timer_start( int n );
void    timer_stop( int n );
double  timer_read( int n );
static int timer_on;

/*
 *    FUNCTION RANDLC (X, A)
 *
 *  This routine returns a uniform pseudorandom double precision number in the
 *  range (0, 1) by using the linear congruential generator
 *
 *  x_{k+1} = a x_k  (mod 2^46)
 *
 *  where 0 < x_k < 2^46 and 0 < a < 2^46.  This scheme generates 2^44 numbers
 *  before repeating.  The argument A is the same as 'a' in the above formula,
 *  and X is the same as x_0.  A and X must be odd double precision integers
 *  in the range (1, 2^46).  The returned value RANDLC is normalized to be
 *  between 0 and 1, i.e. RANDLC = 2^(-46) * x_1.  X is updated to contain
 *  the new seed x_1, so that subsequent calls to RANDLC using the same
 *  arguments will generate a continuous sequence.
 *
 *  This routine should produce the same results on any computer with at least
 *  48 mantissa bits in double precision floating point data.  On Cray systems,
 *  double precision should be disabled.
 *
 *  David H. Bailey     October 26, 1990
 *
 *     IMPLICIT DOUBLE PRECISION (A-H, O-Z)
 *     SAVE KS, R23, R46, T23, T46
 *     DATA KS/0/
 *
 *  If this is the first call to RANDLC, compute R23 = 2 ^ -23, R46 = 2 ^ -46,
 *  T23 = 2 ^ 23, and T46 = 2 ^ 46.  These are computed in loops, rather than
 *  by merely using the ** operator, in order to insure that the results are
 *  exact on all systems.  This code assumes that 0.5D0 is represented exactly.
 */

/*****************************************************************/
/*************           R  A  N  D  L  C             ************/
/*************                                        ************/
/*************    portable random number generator    ************/
/*****************************************************************/

//static int      KS=0;
static double	R23, R46, T23, T46;

void init_constants()
{
    int i;
    R23 = 1.0;
    R46 = 1.0;
    T23 = 1.0;
    T46 = 1.0;

    for (i=1; i<=23; i++)
    {
      R23 = 0.50 * R23;
      T23 = 2.0 * T23;
    }
    for (i=1; i<=46; i++)
    {
      R46 = 0.50 * R46;
      T46 = 2.0 * T46;
    }
}

double	randlc( double *X, double *A )
{
      double		T1, T2, T3, T4;
      double		A1;
      double		A2;
      double		X1;
      double		X2;
      double		Z;
      int     		j;


/*  Break A into two parts such that A = 2^23 * A1 + A2 and set X = N.  */

      T1 = R23 * *A;
      j  = T1;
      A1 = j;
      A2 = *A - T23 * A1;

/*  Break X into two parts such that X = 2^23 * X1 + X2, compute
    Z = A1 * X2 + A2 * X1  (mod 2^23), and then
    X = 2^23 * Z + A2 * X2  (mod 2^46).                            */

      T1 = R23 * *X;
      j  = T1;
      X1 = j;
      X2 = *X - T23 * X1;
      T1 = A1 * X2 + A2 * X1;
      
      j  = R23 * T1;
      T2 = j;
      Z = T1 - T23 * T2;
      T3 = T23 * Z + A2 * X2;
      j  = R46 * T3;
      T4 = j;
      *X = T3 - T46 * T4;
      return(R46 * *X);
} 




/*****************************************************************/
/************   F  I  N  D  _  M  Y  _  S  E  E  D    ************/
/************                                         ************/
/************ returns parallel random number seq seed ************/
/*****************************************************************/

/*
 * Create a random number sequence of total length nn residing
 * on np number of processors.  Each processor will therefore have a
 * subsequence of length nn/np.  This routine returns that random
 * number which is the first random number for the subsequence belonging
 * to processor rank kn, and which is used as seed for proc kn ran # gen.
 */

double   tile_find_my_seed( int kn,        /* my processor rank, 0<=kn<=num procs */
                       int np,        /* np = num procs                      */
                       long nn,       /* total num of ran numbers, all procs */
                       double s,      /* Ran num seed, for ex.: 314159265.00 */
                       double a )     /* Ran num gen mult, try 1220703125.00 */
{

      double t1,t2;
      long   nq,kk,ik;

      if ( kn == 0 ) return s;

      // TODO: these 2 lines have to be modified for HTA block decomposition algorithm
      // mq = (nn/4 + np - 1) / np;
      // nq = mq * 4 * kn;               /* number of rans to be skipped */
      // TODO: HTA block decomposition dependent.. need to verify.. not sure why there's a /4 and *4 in the original implementation
      nq = ((kn * (nn/4)) / np) * 4;

      t1 = s;
      t2 = a;
      kk = nq;
      while ( kk > 1 ) {
      	 ik = kk / 2;
         if( 2 * ik ==  kk ) { // kk is even number
            (void)randlc( &t2, &t2 );
	    kk = ik;
	 }
	 else { // kk is odd number
            (void)randlc( &t1, &t2 );
	    kk = kk - 1;
	 }
      }
      // kk == 1
      (void)randlc( &t1, &t2 );

      return( t1 );

}



/*****************************************************************/
/*************      C  R  E  A  T  E  _  S  E  Q      ************/
/*****************************************************************/
static const double seed = 314159265.00;
static const double a = 1220703125.00;
void tile_create_seq(HTA * key_array_HTA)
{
  double x, s;
  INT_TYPE i, k;
  int* keys = HTA_get_ptr_raw_data(key_array_HTA);

  {
    INT_TYPE k1, k2;
    double an = a;
    int myid, num_procs;
    //INT_TYPE mq;

    //myid = omp_get_thread_num();
    myid = key_array_HTA->rank;
    //num_procs = omp_get_num_threads();
    num_procs = PROC;

    //mq = (NUM_KEYS + num_procs - 1) / num_procs;
    //k1 = mq * myid;
    //k2 = k1 + mq;
    //if ( k2 > NUM_KEYS ) k2 = NUM_KEYS;
    k1 = (myid * (long long) NUM_KEYS) / num_procs;
    k2 = ((myid+1) * (long long) NUM_KEYS) / num_procs;
    assert(k1 <= k2);
    //printf("k1 = %d, k2 = %d, myid = %d\n", k1, k2, myid);

    s = tile_find_my_seed( myid, num_procs,
        (long)4*NUM_KEYS, seed, an );

    k = MAX_KEY/4;

    for (i=k1; i<k2; i++)
    {
      x = randlc(&s, &an);
      x += randlc(&s, &an);
      x += randlc(&s, &an);
      x += randlc(&s, &an);  

      keys[i-k1] = k*x;
    }
  } /*omp parallel*/
}



/*****************************************************************/
/*****************    Allocate Working Buffer     ****************/
/*****************************************************************/
void *alloc_mem( size_t size )
{
    void *p;

    p = (void *)malloc(size);
    if (!p) {
        perror("Memory allocation error");
        exit(1);
    }
    return p;
}


// O(NK/P)
void cal_bucket_size(HTA * key_array_leaf, HTA * bucket_size_leaf) {
    INT_TYPE *key_array = (INT_TYPE*) HTA_get_ptr_raw_data(key_array_leaf);
    INT_TYPE *bucket_size = (INT_TYPE*) HTA_get_ptr_raw_data(bucket_size_leaf);

    int num_keys = key_array_leaf->flat_size.values[0];
    for(int i=0; i < num_keys; i++ )
        bucket_size[key_array[i] >> shift]++;
}

// O(NB/P)
void cal_bucket_ptrs(HTA * bucket_ptrs_leaf, HTA * bucket_size_leaf)
{
    INT_TYPE* bucket_ptrs = HTA_get_ptr_raw_data(bucket_ptrs_leaf);
    INT_TYPE* bucket_size = HTA_get_ptr_raw_data(bucket_size_leaf);
    INT_TYPE sum = 0;
    int i;
    for(i = 0; i< NUM_BUCKETS; i++ ) {     
        bucket_ptrs[i] = sum;
        sum += bucket_size[i];
    }
}

// O(NK/P)
// It's a local sort of num_keys in key_array_leaf[myid]
// The generated bucket_ptrs is local but the range of it covers all bucket value
void bucket_sort(HTA * key_buff2_leaf, HTA * key_array_leaf, HTA * bucket_ptrs_leaf)
{
    INT_TYPE* bucket_ptrs = HTA_get_ptr_raw_data(bucket_ptrs_leaf);
    INT_TYPE* key_buff2 = HTA_get_ptr_raw_data(key_buff2_leaf);
    INT_TYPE* key_array = HTA_get_ptr_raw_data(key_array_leaf);
    int i, k;

    int num_keys = key_array_leaf->flat_size.values[0];

    for(i=0; i<num_keys; i++ )  
    {
        k = key_array[i];
        key_buff2[bucket_ptrs[k >> shift]++] = k;
    }
}

void count_keys(HTA * key_buff1_leaf)
{
    int i, j, b, k;
    int myid = key_buff1_leaf->rank;
    INT_TYPE* key_counters = HTA_get_ptr_raw_data(key_buff1_leaf);
    int key_offset = myid * (MAX_KEY / PROC);

    assert(NUM_BUCKETS % PROC == 0);
    // calculate the bucket range for this processor
    int num = NUM_BUCKETS / PROC;
    int b1 = myid * num;
    int b2 = (myid+1) * num;


    for(i = 0; i < PROC; i++)
    {
        // FIXME: shared memory assume global view
        // Pointers to partial results of each processor
        INT_TYPE* key_buff2 = HTA_get_ptr_raw_data(key_buff2_HTA->tiles[i]);
        INT_TYPE* bucket_size = HTA_get_ptr_raw_data(bucket_size_HTA->tiles[i]);
        INT_TYPE* bucket_ptrs = HTA_get_ptr_raw_data(bucket_ptrs_HTA->tiles[i]);

        for(b = b1; b < b2; b++)
        {
            // re-evaluate bucket_ptrs
            // bucket_ptrs[b] -= bucket_size[b]; // move pointers back to the start of each bucket
            int start = bucket_ptrs[b] - bucket_size[b];
            int end = bucket_ptrs[b];
            for(j = start; j < end; j++)
            {
                k = key_buff2[j];
                key_counters[k - key_offset]++;
            }
        } 
    }

}

void get_last_rank(HTA * key_buff1_leaf, HTA* last_rank_leaf)
{
    INT_TYPE* key_buff_ptr = HTA_get_ptr_raw_data(key_buff1_leaf);
    INT_TYPE* last_rank = HTA_get_ptr_raw_data(last_rank_leaf);
    int last_key_idx = key_buff1_leaf->flat_size.values[0] - 1;
    *last_rank = key_buff_ptr[last_key_idx];
}

void final_sorting(HTA * key_buff1_leaf, HTA* last_rank_leaf)
{
    int i, j;
    int myid = key_buff1_leaf->rank;
    INT_TYPE* key_buff_ptr = HTA_get_ptr_raw_data(key_buff1_leaf);
    INT_TYPE* last_rank = HTA_get_ptr_raw_data(last_rank_leaf);
    int num_keys = MAX_KEY / PROC;                        
    int key_offset = myid * num_keys; // global key offset
    
    INT_TYPE prev_rank = (myid == 0) ? 0 : *last_rank;
    for(i = 0; i < num_keys; i++)
    {
        for(j = prev_rank; j < key_buff_ptr[i]; j++)
        {
            key_array[j] = i + key_offset; // FIXME: key_array should be HTA?
        }
        prev_rank = key_buff_ptr[i];
    }
}

void local_scan(HTA * key_buff1_leaf, HTA * local_sum_leaf)
{
    INT_TYPE* ptr = HTA_get_ptr_raw_data(key_buff1_leaf);
    INT_TYPE* sum = HTA_get_ptr_raw_data(local_sum_leaf);
    int tile_size = key_buff1_leaf->flat_size.values[0];
    for(int j = 1; j < tile_size; j++)
    {
        ptr[j] += ptr[j-1];
    }
    *sum = ptr[tile_size-1];
}

void scan_partial_and_shift(HTA * local_sum_HTA) // TODO: communication
{
    int num_tiles = local_sum_HTA->tiling->values[0];
    INT_TYPE sum = 0; 
    for(int i = 0; i < num_tiles; i++)
    {
        INT_TYPE* p = HTA_get_ptr_raw_data(local_sum_HTA->tiles[i]);
        INT_TYPE old_sum = sum;
        sum += *p;
        *p = old_sum;
    }
}

void final_scan(HTA * key_buff1_leaf, HTA * local_sum_leaf)
{
    INT_TYPE* ptr = HTA_get_ptr_raw_data(key_buff1_leaf);
    INT_TYPE sum = *(INT_TYPE*) HTA_get_ptr_raw_data(local_sum_leaf);
    int tile_size = key_buff1_leaf->flat_size.values[0];
    int myid = key_buff1_leaf->rank;
    if(myid != 0)
    {
        for(int j = 0; j < tile_size; j++)
        {
            ptr[j] += sum;
        }
    }
}

/*****************************************************************/
/*************    F  U  L  L  _  V  E  R  I  F  Y     ************/
/*****************************************************************/


void full_verify( void )
{
    INT_TYPE   j;

/*  Now, finally, sort the keys:  */

/*  Copy keys into work array; keys in key_array will be reassigned. */

    HTA_map_h2(HTA_LEAF_LEVEL(key_buff1_HTA), get_last_rank, key_buff1_HTA, last_rank_HTA);
    Tuple dir;
    Tuple_init(&dir, 1, 1); // shift +1
    HTA_circshift(last_rank_HTA, &dir);
    HTA_map_h2(HTA_LEAF_LEVEL(key_buff1_HTA), final_sorting, key_buff1_HTA, last_rank_HTA);

/*  Confirm keys correctly sorted: count incorrectly sorted keys, if any */

    j = 0;
    // FIXME: verification is sequential now.. need to make it parallel 
    for(int i=1; i<NUM_KEYS; i++ )
    {
        if( key_array[i-1] > key_array[i] )
            j++;
    }

    if( j != 0 )
        printf( "Full_verify: number of keys out of sort: %ld\n", (long)j );
    else
        passed_verification++;

}
/*****************************************************************/
/*************             R  A  N  K             ****************/
/*****************************************************************/

void rank( int iteration )
{

    INT_TYPE    i, k;
    INT_TYPE    *key_array_ptr;

    //key_array[iteration] = iteration;
    //key_array[iteration+MAX_ITERATIONS] = MAX_KEY - iteration;
    // FIXME: reduce overhead
    Tuple idx1 = Tuple_create(1, iteration);
    Tuple idx2 = Tuple_create(1, iteration + MAX_ITERATIONS);
    key_array_ptr = (INT_TYPE*) HTA_flat_access(key_array_HTA, &idx1);
    *key_array_ptr = iteration;
    key_array_ptr = (INT_TYPE*) HTA_flat_access(key_array_HTA, &idx2);
    *key_array_ptr = MAX_KEY - iteration;

/*  Determine where the partial verify test keys are, load into  */
/*  top of array bucket_size                                     */
    Tuple idx3;
    Tuple_init_zero(&idx3, 1);
    for( i=0; i<TEST_ARRAY_SIZE; i++ )
    {
        idx3.values[0] = test_index_array[i];
        partial_verify_vals[i] = *((INT_TYPE*) HTA_flat_access(key_array_HTA, &idx3));
    }

    if(timer_on) timer_start(4);
/*  Initialize */
    INT_TYPE initval_0 = 0;
    if(timer_on) timer_start(5);
    HTA_map_h1s1(HTA_LEAF_LEVEL(bucket_size_HTA), H1S1_INIT, bucket_size_HTA, &initval_0); // does not scale
    HTA_map_h1s1(HTA_LEAF_LEVEL(key_buff1_HTA), H1S1_INIT, key_buff1_HTA, &initval_0);
    if(timer_on) timer_stop(5);

    if(timer_on) timer_start(6);
    // determine local process bucket size
    HTA_map_h2(HTA_LEAF_LEVEL(key_array_HTA), cal_bucket_size, key_array_HTA, bucket_size_HTA);
    // determine local process bucket ptrs (local scan)
    HTA_map_h2(HTA_LEAF_LEVEL(bucket_ptrs_HTA), cal_bucket_ptrs, bucket_ptrs_HTA, bucket_size_HTA); // does not scale
    // sort keys locally
    HTA_map_h3(HTA_LEAF_LEVEL(key_buff2_HTA), bucket_sort, key_buff2_HTA, key_array_HTA, bucket_ptrs_HTA); 
    if(timer_on) timer_stop(6);
    // distribute buckets to the processors in charge and count population of the keys 
    // For now, it's not a generalized HTA operation
    if(timer_on) timer_start(7);
    HTA_map_h1(HTA_LEAF_LEVEL(key_buff1_HTA), count_keys, key_buff1_HTA);

///*  To obtain ranks of each key, successively add the individual key
//    population, not forgetting to add m, the total of lesser keys,
//    to the first key population                                          */
//        key_buff_ptr[k1] += m;
//        for ( k = k1+1; k < k2; k++ )
//            key_buff_ptr[k] += key_buff_ptr[k-1];
//
//    }
    HTA_map_h2(HTA_LEAF_LEVEL(key_buff1_HTA), local_scan, key_buff1_HTA, local_sum_HTA);
    scan_partial_and_shift(local_sum_HTA); // TODO: communication
    HTA_map_h2(HTA_LEAF_LEVEL(key_buff1_HTA), final_scan, key_buff1_HTA, local_sum_HTA);
    if(timer_on) timer_stop(7);
    if(timer_on) timer_stop(4);
    

    // At this point, all key ranks are determined

/* This is the partial verify test section */
/* Observe that test_rank_array vals are   */
/* shifted differently for different cases */
    for( i=0; i<TEST_ARRAY_SIZE; i++ )
    {                                             
        k = partial_verify_vals[i];          /* test vals were put here */
        if( 0 < k  &&  k <= NUM_KEYS-1 )
        {
            Tuple idx1 = Tuple_create(1, k-1);
            INT_TYPE key_rank = *((INT_TYPE*) HTA_flat_access(key_buff1_HTA, &idx1));

            int failed = 0;

            switch( CLASS )
            {
                case 'S':
                    if( i <= 2 )
                    {
                        if( key_rank != test_rank_array[i]+iteration )
                            failed = 1;
                        else
                            passed_verification++;
                    }
                    else
                    {
                        if( key_rank != test_rank_array[i]-iteration )
                            failed = 1;
                        else
                            passed_verification++;
                    }
                    break;
                case 'W':
                    if( i < 2 )
                    {
                        if( key_rank != test_rank_array[i]+(iteration-2) )
                            failed = 1;
                        else
                            passed_verification++;
                    }
                    else
                    {
                        if( key_rank != test_rank_array[i]-iteration )
                            failed = 1;
                        else
                            passed_verification++;
                    }
                    break;
                case 'A':
                    if( i <= 2 )
        	    {
                        if( key_rank != test_rank_array[i]+(iteration-1) )
                            failed = 1;
                        else
                            passed_verification++;
        	    }
                    else
                    {
                        if( key_rank != test_rank_array[i]-(iteration-1) )
                            failed = 1;
                        else
                            passed_verification++;
                    }
                    break;
                case 'B':
                    if( i == 1 || i == 2 || i == 4 )
        	    {
                        if( key_rank != test_rank_array[i]+iteration )
                            failed = 1;
                        else
                            passed_verification++;
        	    }
                    else
                    {
                        if( key_rank != test_rank_array[i]-iteration )
                            failed = 1;
                        else
                            passed_verification++;
                    }
                    break;
                case 'C':
                    if( i <= 2 )
        	    {
                        if( key_rank != test_rank_array[i]+iteration )
                            failed = 1;
                        else
                            passed_verification++;
        	    }
                    else
                    {
                        if( key_rank != test_rank_array[i]-iteration )
                            failed = 1;
                        else
                            passed_verification++;
                    }
                    break;
                case 'D':
                    if( i < 2 )
        	    {
                        if( key_rank != test_rank_array[i]+iteration )
                            failed = 1;
                        else
                            passed_verification++;
        	    }
                    else
                    {
                        if( key_rank != test_rank_array[i]-iteration )
                            failed = 1;
                        else
                            passed_verification++;
                    }
                    break;
            }
            if( failed == 1 )
                printf( "Failed partial verification: "
                        "iteration %d, test key %d, key_rank = %d\n", 
                         iteration, (int)i, key_rank );
        }
    }




/*  Make copies of rank info for use by full_verify: these variables
    in rank are local; making them global slows down the code, probably
    since they cannot be made register by compiler                        */

    //if( iteration == MAX_ITERATIONS ) 
    //    key_buff_ptr_global = key_buff_ptr;

}      


void alloc_and_init_HTAs()
{
    INT_TYPE initval_0 = 0;
    /* HTA initialization */
    Tuple tp0 = Tuple_create(1, PROC);
    Tuple fs0 = Tuple_create(1, SIZE_OF_BUFFERS); 
    Dist dist0;
    Dist_init(&dist0, 0);
    key_array_HTA = HTA_create(1, 2, &fs0, 0, &dist0, SCALAR_TYPE, 1, tp0);  // tile size = TOTAL_KEYS/PROC
    key_buff2_HTA = HTA_create(1, 2, &fs0, 0, &dist0, SCALAR_TYPE, 1, tp0);  // tile size = TOTAL_KEYS/PROC

    Tuple fs1 = Tuple_create(1, NUM_BUCKETS * PROC); 
    bucket_size_HTA = HTA_create(1, 2, &fs1, 0, &dist0, SCALAR_TYPE, 1, tp0);

    bucket_ptrs_HTA = HTA_create(1, 2, &fs1, 0, &dist0, SCALAR_TYPE, 1, tp0);

    Tuple fs2 = Tuple_create(1, MAX_KEY);
    assert( NUM_BUCKETS % PROC == 0  && MAX_KEY % NUM_BUCKETS == 0 );
    key_buff1_HTA = HTA_create(1, 2, &fs2, 0, &dist0, SCALAR_TYPE, 1, tp0); // tile size = MAX_KEY/PROC

    Tuple fs3 = Tuple_create(1, PROC);
    local_sum_HTA = HTA_create(1, 2, &fs3, 0, &dist0, SCALAR_TYPE, 1, tp0);

    last_rank_HTA = HTA_create(1, 2, &fs3, 0, &dist0, SCALAR_TYPE, 1, tp0);


    HTA_map_h1(HTA_LEAF_LEVEL(key_array_HTA), tile_create_seq, key_array_HTA);
    HTA_map_h1s1(HTA_LEAF_LEVEL(key_buff2_HTA), H1S1_INIT, key_buff2_HTA, &initval_0);
}
/*****************************************************************/
/*************             M  A  I  N             ****************/
/*****************************************************************/

int hta_main(int argc, char* argv[])
{

    int             i, iteration;

    double          timecounter;

    FILE            *fp;

  if (argc == 2)
  {
      PROC = atoi(argv[1]);
  }

/*  Initialize timers  */
    timer_on = 0;            
    if ((fp = fopen("timer.flag", "r")) != NULL) {
        fclose(fp);
        timer_on = 1;
    }
    timer_clear( 0 );
    if (timer_on) {
        timer_clear( 1 );
        timer_clear( 2 );
        timer_clear( 3 );
        timer_clear( 4 );
        timer_clear( 5 );
        timer_clear( 6 );
        timer_clear( 7 );
    }

    if (timer_on) timer_start( 3 );


/*  Initialize the verification arrays if a valid class */
    for( i=0; i<TEST_ARRAY_SIZE; i++ )
        switch( CLASS )
        {
            case 'S':
                test_index_array[i] = S_test_index_array[i];
                test_rank_array[i]  = S_test_rank_array[i];
                break;
            case 'A':
                test_index_array[i] = A_test_index_array[i];
                test_rank_array[i]  = A_test_rank_array[i];
                break;
            case 'W':
                test_index_array[i] = W_test_index_array[i];
                test_rank_array[i]  = W_test_rank_array[i];
                break;
            case 'B':
                test_index_array[i] = B_test_index_array[i];
                test_rank_array[i]  = B_test_rank_array[i];
                break;
            case 'C':
                test_index_array[i] = C_test_index_array[i];
                test_rank_array[i]  = C_test_rank_array[i];
                break;
            case 'D':
                test_index_array[i] = D_test_index_array[i];
                test_rank_array[i]  = D_test_rank_array[i];
                break;
        };

        

/*  Printout initial NPB info */
    printf
      ( "\n\n NAS Parallel Benchmarks (NPB3.3-OMP) - IS Benchmark\n\n" );
    printf( " Size:  %ld  (class %c)\n", (long)TOTAL_KEYS, CLASS );
    printf( " Iterations:  %d\n", MAX_ITERATIONS );
#ifdef _OPENMP
    printf( " Number of available threads:  %d\n", omp_get_max_threads() );
#endif
    printf( "\n" );

    if (timer_on) timer_start( 1 );

    init_constants();
    alloc_and_init_HTAs();


/*  Generate random number sequence and subsequent keys on all procs */
    //create_seq( 314159265.00,                    /* Random number gen seed */
    //            1220703125.00 );                 /* Random number gen mult */
    //INT_TYPE temp_array[SIZE_OF_BUFFERS];
    //HTA_to_array(key_array_HTA, key_array);

    // alloc_key_buff();
    
    if (timer_on) timer_stop( 1 );


/*  Do one interation for free (i.e., untimed) to guarantee initialization of  
    all data and code pages and respective tables */
    int timed = timer_on;
    timer_on = 0; // turn off timer for the first call
    rank( 1 );  
    timer_on = timed;

/*  Start verification counter */
    passed_verification = 0;

    if( CLASS != 'S' ) printf( "\n   iteration\n" );

/*  Start timer  */             
    timer_start( 0 );


/*  This is the main iteration */
    for( iteration=1; iteration<=MAX_ITERATIONS; iteration++ )
    {
        if( CLASS != 'S' ) printf( "        %d\n", iteration );
        rank( iteration );
    }


/*  End of timing, obtain maximum time of all processors */
    timer_stop( 0 );
    timecounter = timer_read( 0 );


/*  This tests that keys are in sequence: sorting of last ranked key seq
    occurs here, but is an untimed operation                             */
    if (timer_on) timer_start( 2 );
    full_verify();
    if (timer_on) timer_stop( 2 );

    if (timer_on) timer_stop( 3 );


/*  The final printout  */
    if( passed_verification != 5*MAX_ITERATIONS + 1 )
        passed_verification = 0;
    c_print_results( "IS",
                     CLASS,
                     (int)(TOTAL_KEYS/64),
                     64,
                     0,
                     MAX_ITERATIONS,
                     timecounter,
                     ((double) (MAX_ITERATIONS*TOTAL_KEYS))
                                                  /timecounter/1000000.,
                     "keys ranked", 
                     passed_verification,
                     NPBVERSION,
                     COMPILETIME,
                     CC,
                     CLINK,
                     C_LIB,
                     C_INC,
                     CFLAGS,
                     CLINKFLAGS );


/*  Print additional timers  */
    if (timer_on) {
       double t_total, t_percent;
       char rec_name[256];
       sprintf(rec_name, "rec/is.%c.%d.rec", CLASS, PROC);
       FILE* fp_rec = fopen(rec_name, "a");

       t_total = timer_read( 3 );
       printf("\nAdditional timers -\n");
       printf(" Total execution: %8.3f\n", t_total);
       fprintf(fp_rec, "%8.3f ", t_total);
       if (t_total == 0.0) t_total = 1.0;
       timecounter = timer_read(1);
       t_percent = timecounter/t_total * 100.;
       printf(" Initialization : %8.3f (%5.2f%%)\n", timecounter, t_percent);
       fprintf(fp_rec, "%8.3f ", timecounter);
       timecounter = timer_read(0);
       t_percent = timecounter/t_total * 100.;
       printf(" Benchmarking   : %8.3f (%5.2f%%)\n", timecounter, t_percent);
       fprintf(fp_rec, "%8.3f ", timecounter);
       timecounter = timer_read(2);
       t_percent = timecounter/t_total * 100.;
       printf(" Sorting        : %8.3f (%5.2f%%)\n", timecounter, t_percent);
       fprintf(fp_rec, "%8.3f ", timecounter);
       timecounter = timer_read(4);
       t_percent = timecounter/t_total * 100.;
       printf(" Rank Compute   : %8.3f (%5.2f%%)\n", timecounter, t_percent);
       fprintf(fp_rec, "%8.3f ", timecounter);
       timecounter = timer_read(5);
       t_percent = timecounter/t_total * 100.;
       printf(" Rank Init      : %8.3f (%5.2f%%)\n", timecounter, t_percent);
       fprintf(fp_rec, "%8.3f ", timecounter);
       timecounter = timer_read(6);
       t_percent = timecounter/t_total * 100.;
       printf(" Local compute  : %8.3f (%5.2f%%)\n", timecounter, t_percent);
       fprintf(fp_rec, "%8.3f ", timecounter);
       timecounter = timer_read(7);
       t_percent = timecounter/t_total * 100.;
       printf(" Global compute : %8.3f (%5.2f%%)\n", timecounter, t_percent);
       fprintf(fp_rec, "%8.3f\n", timecounter);
       fclose(fp_rec);
    }

    assert(passed_verification);
    return (passed_verification)?0:1;
         /**************************/
}        /*  E N D  P R O G R A M  */
         /**************************/




