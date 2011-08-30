#include <stdio.h>
#include <emmintrin.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#define MAX_TASKS_IN_TASKQ 1024
#include "taskQ.h"
#include "color.h"

#define CORE_FREQUENCY (3.2*1000*1000.0*1000.0)
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

//#define NOT_ASSIGNED (123123123)
#define NOT_ASSIGNED (2147483647) //Basically 2^31-1

#define COMPUTE_SOCKET_ID(thr)  ( (nsockets == 1) ? 0 : (thr >= (nthreads>>1)) )
#define COMPUTE_SOCKET_ID_FOR_TASK(taskidddd)  ( (nsockets == 1) ? 0 : (taskidddd >= (ntasks>>1)) )
#define COMPUTE_BIN_ID(vertex_id) ((nsockets == 1) ? 0 : (vertex_id >= global_vertices_for_socket0))

#define ERROR_PRINT() {printf("Error on (%d) line in (%s) file\n", __LINE__, __FILE__); exit(123);}
extern "C" unsigned long long int read_tsc ();

//#define USE_LARGE_PAGE //<-- Can be set or unset independently of the other parameters...
//#define NUMA_AWARENESS //<-- Can be set or unset independently of other paramters...
//#define MULTI_SOCKET //<-- Can be set or unset -- Setting it runs the socket subdivision code...

#define CORES_PER_SOCKET 6
int global_cores_per_socket = CORES_PER_SOCKET;

#define MAX_THREADS 32

#define LOG_HIST_BINS 10  //13 bits
#define HIST_BINS (1<<LOG_HIST_BINS)
#define RIGHT_SHIFT (LOG_V - LOG_HIST_BINS) //<-- This parameter needs to be changed based on number of vertices (log(V) - 13), LOG_V should be defined somewhere...

#define LOG_ANOTHER_HIST_BINS 1 //1bit
//#define ANOTHER_HIST_BINS 12
#define ANOTHER_HIST_BINS (1<<LOG_ANOTHER_HIST_BINS) 
#define ANOTHER_RIGHT_SHIFT (LOG_V - LOG_ANOTHER_HIST_BINS) 

//#define EVENLY_DIVIDE_BS //<-- This ensures Boundary States division  is load_balanced (i.e. a socket can read global_Adjacency_0/1 across sockets...
#if !defined MULTI_SOCKET && !defined EVENLY_DIVIDE_BS
    #error "EVENLY_DIVIDE_BS should be defined in chaos mode - else only one socket is active"
#endif


#ifdef MULTI_SOCKET
//#define EVENLY_DIVIDE_PBS //<-- This ensures PBS is load_balanced (i.e. a asocket can read global_Assigned_0/1 across sockets...
#define PERFORMING_BFS_I  Perform_BFS_I
#define PERFORMING_BFS_II  Perform_BFS_II
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef NUMA_AWARENESS
#include <numa.h>
#endif

////////////////// Option 0:
#define USE_TASKQ

////////////////// Option 1:
//#define USE_BS_BINNING

////////////////// Option 2:
#define SIMPLE_SCALAR
//#define UNROLLED_SCALAR
//#define SIMD

////////////////// Option 3:
#define TLB_BLOCKING

////////////////// Option 4:
//#define BLOOM_FILTER

////////////////////////////////////////  BLOOM_FILTER  //////////////////////////////////////////////////////////////////////
#ifdef BLOOM_FILTER
  //#define BYTE_LEVEL
  //#define BIT_LEVEL

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef MULTI_SOCKET
int global_vertices_for_socket0 = -95123;
int log_vertices_socket0 = -95123;
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef TLB_BLOCKING
#define UPDATE_HISTOGRAM(Hist, rstate_id, right_shift, BASE)  Hist[(rstate_id>>right_shift)]++
#else
#define UPDATE_HISTOGRAM(Hist, rstate_id, right_shift, BASE)  
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define UPDATE_BS(BBSS, counterr, new_state_idd) BBSS[counterr++] = new_state_idd;

#ifdef USE_BS_BINNING
#define BS_BINS 6
int **global_Binned_Boundary_States;
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/// CMOV version

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef BLOOM_FILTER

int bloom_indices = -95123;
#ifdef MULTI_SOCKET
int bloom_indices_socket0 = -95123;
int bloom_indices_socket1 = -95123;
#endif

////////////////////////////////////////////////////////////////////////
#ifdef BYTE_LEVEL
unsigned char *global_Assigned;
#ifdef MULTI_SOCKET
unsigned char *global_Assigned_Socket0;
unsigned char *global_Assigned_Socket1;
#endif
#endif
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
#ifdef BIT_LEVEL
unsigned int *global_Assigned;
#ifdef MULTI_SOCKET
unsigned int *global_Assigned_Socket0;
unsigned int *global_Assigned_Socket1;
#endif
#endif
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef BLOOM_FILTER

/// /// /// /// /// /// /// ///
#if (PER_BIT == 2)
#define LOG_PER_BIT 1
#define BIT_MASK 0xFE
#define TWO_POWER_BITS 3
#endif
/// /// /// /// /// /// /// ///
#if (PER_BIT == 4)
#define LOG_PER_BIT 2
#define BIT_MASK 0xFC
#define TWO_POWER_BITS 15
#endif
/// /// /// /// /// /// /// ///
#if (PER_BIT == 8)
#define LOG_PER_BIT 3
#define BIT_MASK 0xF8
#define TWO_POWER_BITS 255
#endif
/// /// /// /// /// /// /// ///
#if (PER_BIT == 16)
#define LOG_PER_BIT 4
#define BIT_MASK 0xF0
#define TWO_POWER_BITS 65535
#endif
/// /// /// /// /// /// /// ///

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef BLOOM_FILTER

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///
#ifdef BYTE_LEVEL
#define UPDATE_DEPTHPARENT_AND_BS(Depth, new_state_id, equality, depth, BS, counter, parent) \
{\
    if (global_Assigned[new_state_id] == 0) \
    {\
        global_Assigned[new_state_id] = 1;  \
        if (Depth[2*new_state_id]  > depth) \
        { \
            Depth[2*new_state_id+0] = depth;        \
            Depth[2*new_state_id+1] = parent;        \
            UPDATE_BS(BS, counter, new_state_id); \
            UPDATE_HISTOGRAM(Hist, new_state_id, RIGHT_SHIFT, (HIST_BINS-1)); \
        }\
    }  \
}
#endif
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///
#ifdef BIT_LEVEL
#define UPDATE_DEPTHPARENT_AND_BS(Depth, new_state_id, equality, depth, BS, counter, parent) \
{\
    int dword = new_state_id >> 5;                                   \
    int bit = new_state_id & 0x1F;                                   \
    unsigned int current_value = global_Assigned[dword];            \
    if ( (current_value & (1<<bit)) == 0)                           \
    {                                                               \
        global_Assigned[dword] = current_value | (1<<bit);           \
        /*BS[counter++] = new_state_id;                           \
        Hist[(new_state_id >> RIGHT_SHIFT) & (HIST_BINS-1)]++; */\
        if (Depth[2*new_state_id] > depth)                            \
        {                                                           \
            Depth[2*new_state_id+0] = depth;                            \
            Depth[2*new_state_id+1] = parent;                            \
            UPDATE_BS(BS, counter, new_state_id); \
            UPDATE_HISTOGRAM(Hist, new_state_id, RIGHT_SHIFT, (HIST_BINS-1)); \
        }                                                           \
    }                                                               \
}
#endif
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///

#ifdef MULTI_SOCKET

#ifdef BYTE_LEVEL
#define UPDATE_DEPTHPARENT_AND_BS_Socket(Depth, new_state_id, equality, depth, BS, counter, local_Assigned_param, parent) \
{\
    if (local_Assigned_param[new_state_id] == 0) \
    {\
        local_Assigned_param[new_state_id] = 1;  \
        if (Depth[2*new_state_id]  > depth) \
        { \
            Depth[2*new_state_id+0] = depth;        \
            Depth[2*new_state_id+1] = parent;       \
            UPDATE_BS(BS, counter, new_state_id); \
            UPDATE_HISTOGRAM(Hist, new_state_id, RIGHT_SHIFT, (HIST_BINS-1)); \
        }\
    }  \
}
#endif

#ifdef BIT_LEVEL
#define UPDATE_DEPTHPARENT_AND_BS_Socket(Depth, new_state_id, equality, depth, BS, counter, local_Assigned_param, parent) \
{\
    int dword = new_state_id >> 5;                                   \
    int bit = new_state_id & 0x1F;                                   \
    unsigned int current_value = local_Assigned_param[dword];            \
    if ( (current_value & (1<<bit)) == 0)                           \
    {                                                               \
        local_Assigned_param[dword] = current_value | (1<<bit);           \
        /*BS[counter++] = new_state_id;                           \
        Hist[(new_state_id >> RIGHT_SHIFT) & (HIST_BINS-1)]++; */\
        if (Depth[2*new_state_id] > depth)                            \
        {                                                           \
            Depth[2*new_state_id+0] = depth;                            \
            Depth[2*new_state_id+1] = parent;                           \
            UPDATE_BS(BS, counter, new_state_id); \
            UPDATE_HISTOGRAM(Hist, new_state_id, RIGHT_SHIFT, (HIST_BINS-1)); \
        }                                                           \
    }                                                               \
}
#endif

#endif
/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef BLOOM_FILTER

/// Branch version
#define UPDATEPARENT_DEPTH_AND_BS(Depth, new_state_id, equality, depth, BS, counter, parent) \
{\
    if (Depth[2*new_state_id] == equality) \
    { \
        Depth[2*new_state_id+0] = depth;  \
        Depth[2*new_state_id+1] = parent; \
        UPDATE_BS(BS, counter, new_state_id); \
        UPDATE_HISTOGRAM(Hist, new_state_id, RIGHT_SHIFT, (HIST_BINS-1)); \
    }\
}

#define UPDATE_DEPTHPARENT_AND_BS_Socket(Depth, new_state_id, equality, depth, BS, counter, local_Assigned_param, parent) \
    UPDATE_DEPTHPARENT_AND_BS(Depth, new_state_id, equality, depth, BS, counter, parent)



/// CMOV version
#if 0 
#define UPDATE_DEPTH_AND_BS(Depth, new_state_id, equality, depth, BS, counter) \
                        int value = (Depth[new_state_id] == equality); \
                        Depth[new_state_id] = MIN(Depth[new_state_id], depth); \
                        BS[counter] = new_state_id;\
                        counter += value;
#endif

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


unsigned long long int Timer[MAX_THREADS];
unsigned long long int Overhead_Timer[MAX_THREADS];
pthread_t threads[MAX_THREADS];
int nthreads;
int nsockets;
pthread_attr_t attr;
pthread_barrier_t mybarrier;

int ntasks_per_thread;
int ntasks;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__declspec(align(64)) int Count_Boundary_States0[MAX_THREADS];
__declspec(align(64)) int Count_Boundary_States1[MAX_THREADS];
int **global_BS_Count_per_bin0;
int **global_BS_Count_per_bin1;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#ifdef TLB_BLOCKING
int **global_Histogram;
#endif

#ifdef MULTI_SOCKET
int **global_Potential_Boundary_States;
//int **global_Other_Potential_Boundary_States;
#endif

int Quantiles[ANOTHER_HIST_BINS+4];
int **Temporary_Array;
int **Boundary_States0;
int **Boundary_States1;
int **Boundary_Neighbors0;

#ifndef USE_TASKQ
int **Number_of_Neighbors0;
int **Number_of_Neighbors1;
#endif

int *Serial_Code_DepthParent;
int global_starting_vertex;
int global_number_of_vertices;
long long int global_number_of_edges;
int global_max_neighbors, global_min_neighbors;
int global_number_of_assigned_vertices = 0;
long long int global_number_of_traversed_edges = 0;

#ifndef MULTI_SOCKET
int **global_Adjacency;
int *global_DepthParent;
#else
int **global_Adjacency_Socket0;
int **global_Adjacency_Socket1;
int *global_DepthParent_Socket0;
int *global_DepthParent_Socket1;
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef MULTI_SOCKET
#define GET_GLOBAL_ADJACENCY(i) ( ((i) < global_vertices_for_socket0) ? (global_Adjacency_Socket0[i]) : (global_Adjacency_Socket1[i]) )
#define GET_GLOBAL_ADJACENCY_Addr(i) ( ((i) < global_vertices_for_socket0) ? (global_Adjacency_Socket0+i) : (global_Adjacency_Socket1+i) )
#else
#define GET_GLOBAL_ADJACENCY(i) ( global_Adjacency[i] )
#define GET_GLOBAL_ADJACENCY_Addr(i) ( global_Adjacency+i )
#endif

#define GET_GLOBAL_ADJACENCY_2(i, local_Adjacency) ( GET_GLOBAL_ADJACENCY(i) )
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__declspec (align(64)) unsigned char Remaining[5][16] = {
                                                            {  0,0,0,0,  0,0,0,0,   0,0,0,0,   0,0,0,0},
                                                            {  255,255,255,255,  0,0,0,0,   0,0,0,0,   0,0,0,0},
                                                            {  255,255,255,255,  255,255,255,255,   0,0,0,0,   0,0,0,0},
                                                            {  255,255,255,255,  255,255,255,255,   255,255,255,255,   0,0,0,0},
                                                            {  255,255,255,255,  255,255,255,255,   255,255,255,255,   255,255,255,255}};
                                                        

__declspec (align(64)) int Number_of_Ones[16] = {0, 1, 1, 2,   1, 2, 2, 3,     1, 2, 2, 3,     2, 3, 3, 4};

__declspec (align(64)) unsigned char TTT[16][16] = {    
                                     { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128}, //Corresponds to 0000
                                     {   0,   1,   2,   3, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128}, //Corresponds to 0001
                                     {   4,   5,   6,   7, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128}, //Corresponds to 0010
                                     {   0,   1,   2,   3,   4,   5,   6,   7, 128, 128, 128, 128, 128, 128, 128, 128}, //Corresponds to 0011
                                     {   8,   9,  10,  11, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128}, //Corresponds to 0100
                                     {   0,   1,   2,   3,   8,   9,  10,  11, 128, 128, 128, 128, 128, 128, 128, 128}, //Corresponds to 0101
                                     {   4,   5,   6,   7,   8,   9,  10,  11, 128, 128, 128, 128, 128, 128, 128, 128}, //Corresponds to 0110
                                     {   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11, 128, 128, 128, 128}, //Corresponds to 0111
                                     {  12,  13,  14,  15, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128}, //Corresponds to 1000
                                     {   0,   1,   2,   3,  12,  13,  14,  15, 128, 128, 128, 128, 128, 128, 128, 128}, //Corresponds to 1001
                                     {   4,   5,   6,   7,  12,  13,  14,  15, 128, 128, 128, 128, 128, 128, 128, 128}, //Corresponds to 1010
                                     {   0,   1,   2,   3,   4,   5,   6,   7,  12,  13,  14,  15, 128, 128, 128, 128}, //Corresponds to 1011
                                     {   8,   9,  10,  11,  12,  13,  14,  15, 128, 128, 128, 128, 128, 128, 128, 128}, //Corresponds to 1100
                                     {   0,   1,   2,   3,   8,   9,  10,  11,  12,  13,  14,  15, 128, 128, 128, 128}, //Corresponds to 1101
                                     {   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15, 128, 128, 128, 128}, //Corresponds to 1110
                                     {   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15}};//Corresponds to 1111


__declspec (align(64)) __m128i Shuffle_Pattern[16] = { _mm_load_si128((__m128i *)(TTT[0])), _mm_load_si128((__m128i *)(TTT[1])), _mm_load_si128((__m128i *)(TTT[2])), _mm_load_si128((__m128i *)(TTT[3])), _mm_load_si128((__m128i *)(TTT[4])), _mm_load_si128((__m128i *)(TTT[5])), _mm_load_si128((__m128i *)(TTT[6])), _mm_load_si128((__m128i *)(TTT[7])), _mm_load_si128((__m128i *)(TTT[8])), _mm_load_si128((__m128i *)(TTT[9])), _mm_load_si128((__m128i *)(TTT[10])), _mm_load_si128((__m128i *)(TTT[11])), _mm_load_si128((__m128i *)(TTT[12])), _mm_load_si128((__m128i *)(TTT[13])), _mm_load_si128((__m128i *)(TTT[14])), _mm_load_si128((__m128i *)(TTT[15])) };


//#define NEW_BARRIER
#include "barrier.c"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef NEW_BARRIER
#include "barrier.24.h"
TREE_BARRIER_TYPE b1;
#define BARRIER(threadid) TREE_BARRIER(&b1, threadid, 24)
#else
#define BARRIER(threadid) barrier(threadid)
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define MINIMUM_DEPTH 0
#define MAX_ITERATIONS 2

int Debug[4096];
int debug_summ = 0;
unsigned long long int global_wasted_time = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "util_new.h"
#include "simd.cpp"
#include "statistics.cpp"
#include "etc.cpp"
#include "low_degree.cpp"
#include "affinity.cpp"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       
#ifdef TLB_BLOCKING
__inline
void Rearrange_for_Reduced_TLB_Misses(int threadid, int *X, int start_index, int end_index)
{
    //printf("fdsfsdf\n");
    unsigned long long int o_start_time = read_tsc();
    //Locally rearrange the  BS1 values...
    int *Histgram = global_Histogram[threadid];
    int sum = 0;

    //int min, max; min = max = Histgram[0]; for(int k=0; k<HIST_BINS; k++) { if (min > Histgram[k]) min = Histgram[k]; if (max < Histgram[k]) max = Histgram[k]; }
    for(int k=0; k<HIST_BINS; k++)
    {
        int prev_sum = sum;
        sum += Histgram[k];
        Histgram[k] = prev_sum;
    }

    //printf("max = %d ::: sum = %d\n", max, sum);

    Histgram[HIST_BINS] = sum;
    if ( (sum) != (end_index - start_index)) ERROR_PRINT();

    int *T_Array = Temporary_Array[threadid];
    for(int k = start_index; k<end_index; k++)
    {
        int index = X[k];
        int value = (index >> RIGHT_SHIFT); // & (HIST_BINS-1);
        T_Array[ Histgram[value]++] = index;
    }

    for(int k=start_index; k<end_index; k++)
    {
        X[k] = T_Array[k-start_index];
    }
        
    unsigned long long int o_end_time = read_tsc();
    Overhead_Timer[threadid] += (o_end_time - o_start_time);
}
#endif

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Function 0:
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#ifdef USE_BS_BINNING
#endif


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Function 1:
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void Perform_BFS(int taskid, int threadid, int depth, int *BS0, int start_index, int end_index, int *BS1, int *next_counter, int *DepthParent)
{
    //ERROR_PRINT();
    unsigned long long int task_start_time = read_tsc();

#ifdef TLB_BLOCKING
    int *Hist = global_Histogram[threadid];
#endif

#ifndef USE_TASKQ
    int *NN = Number_of_Neighbors1[threadid];
#endif
    int next_counter_0 = *next_counter;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
#ifdef SIMPLE_SCALAR

#define UNROLL  4
    //if ((end_index - start_index)  >= 0)
    if ((end_index - start_index) <= UNROLL)
    {
        for(int j = start_index; j < end_index; j++)
        {
            //_mm_prefetch((char *)(BS0 + j + 8+16), _MM_HINT_T0);
	  
            _mm_prefetch((char *)(GET_GLOBAL_ADJACENCY(BS0[j+8])),  _MM_HINT_T0);

            int state_id = BS0[j];
            int *temp_address = GET_GLOBAL_ADJACENCY(state_id);
            int neighbors = temp_address[0];
                
            for(int k=1; k<=neighbors; k+=1)
            {
                int new_state_id_0 = temp_address[k+0];
                UPDATE_DEPTHPARENT_AND_BS(DepthParent, new_state_id_0, NOT_ASSIGNED, depth, BS1, next_counter_0, state_id);
            }
        }
    }
    else
    {
#ifdef BIT_LEVEL
        for(int j = start_index; j<(start_index+UNROLL); j++)
        {
            _mm_prefetch((char *)(GET_GLOBAL_ADJACENCY(BS0[j+8])),  _MM_HINT_T0);
            int state_id = BS0[j];
            int *temp_address =  GET_GLOBAL_ADJACENCY(state_id);
            int neighbors = temp_address[0];
            for(int k=1; k<=neighbors; k+=1) _mm_prefetch((char *)(global_Assigned + (temp_address[k]>>5)), _MM_HINT_T0);
        }
#endif

        for(int j = start_index; j < (end_index-UNROLL); j++)
        {
            _mm_prefetch((char *)(GET_GLOBAL_ADJACENCY(BS0[j+UNROLL+8])),  _MM_HINT_T0);
#ifdef BIT_LEVEL
            {
                int state_id = BS0[j+UNROLL];
                int *temp_address = GET_GLOBAL_ADJACENCY(state_id);
                int neighbors = temp_address[0];
                for(int k=1; k<=neighbors; k+=1) _mm_prefetch((char *)(global_Assigned + (temp_address[k]>>5)), _MM_HINT_T0);
            }
#endif

            {
                int state_id = BS0[j];
                int *temp_address = GET_GLOBAL_ADJACENCY(state_id);
                int neighbors = temp_address[0];
                
                for(int k=1; k<=neighbors; k+=1)
                {
                    int new_state_id_0 = temp_address[k+0];
                    UPDATE_DEPTHPARENT_AND_BS(DepthParent, new_state_id_0, NOT_ASSIGNED, depth, BS1, next_counter_0, state_id);
                }
            }
        }

        for(int j = end_index-UNROLL; j<end_index; j++)
        {
            int state_id = BS0[j];
            int *temp_address = GET_GLOBAL_ADJACENCY(state_id);
            int neighbors = temp_address[0];

            for(int k=1; k<=neighbors; k+=1)
            {
                int new_state_id_0 = temp_address[k+0];
                UPDATE_DEPTHPARENT_AND_BS(DepthParent, new_state_id_0, NOT_ASSIGNED, depth, BS1, next_counter_0, state_id);
            }
        }
    }
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    

    *next_counter = next_counter_0;

    if (next_counter_0 >= MAX_BOUNDARY_STATES)
    {
        printf("MAX_BOUNDARY_STATES = %d ::: boundary states = %d\n", MAX_BOUNDARY_STATES, next_counter_0);
        ERROR_PRINT();
    }

    unsigned long long int task_end_time = read_tsc();
    Timer[threadid] += (task_end_time - task_start_time);
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Function 2:
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
__attribute__((noinline)) 
void ComputeBFS_serial(void *arg1, int *DepthParent)
{
    int max_counter = 0;
    int sum_counter = 0;
    int depth;
    unsigned long long int total_time = 0;

    int threadid = (int)((size_t)(arg1));
    if (threadid) ERROR_PRINT();

    int taskid = threadid;

    long long int overall_counter1 = 0;
    long long int overall_counter2 = 0;

#ifndef USE_TASKQ
    int *NN = Number_of_Neighbors1[threadid];
#endif

    for(int iter=0; iter<MAX_ITERATIONS; iter++)
    {
        for(int i=0; i<global_number_of_vertices; i++) DepthParent[2*i] = DepthParent[2*i+1] = NOT_ASSIGNED;
        RESET_GLOBAL_ASSIGNED();
    
        unsigned long long int start_time = read_tsc();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

        int *BS0_0 = Boundary_States0[0];
        int *BS1_0 = Boundary_States1[0];

#ifdef TLB_BLOCKING
        int  *Hist = global_Histogram[0];
#endif
        sum_counter = 1;
        max_counter = 1;
        int zero = 0;

        UPDATE_DEPTHPARENT_AND_BS(DepthParent, global_starting_vertex, NOT_ASSIGNED, MINIMUM_DEPTH, BS0_0, zero, global_starting_vertex);

        Count_Boundary_States0[0] = 1;

        for(depth=(1+MINIMUM_DEPTH); depth <= global_number_of_vertices; depth++)
        {
            //printf("Y\n");
            int next_counter = 0;
            Perform_BFS(taskid, threadid, depth, BS0_0, 0, Count_Boundary_States0[0], BS1_0, &next_counter, DepthParent); 

            Count_Boundary_States0[0] = next_counter;
            if (!Count_Boundary_States0[0]) break;

            unsigned long long int sstime = read_tsc();
            //qsort(BS1_0, Count_Boundary_States0[0], sizeof(int), Compare_Ints);
            unsigned long long int eetime = read_tsc();
            global_wasted_time += (eetime - sstime);

            int *BS_temp_0 = BS0_0; BS0_0 = BS1_0; BS1_0 = BS_temp_0;
            sum_counter += Count_Boundary_States0[0];
            if (max_counter < Count_Boundary_States0[0] ) max_counter = Count_Boundary_States0[0];
        }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
        unsigned long long int end_time = read_tsc();
        total_time += end_time - start_time;
    }


    
    double total_time_seconds = (total_time*1.0)/CORE_FREQUENCY;
    long long int actual_edges_traversed = 0; for(int i=0; i<global_number_of_vertices; i++) if (DepthParent[2*i] != NOT_ASSIGNED) actual_edges_traversed += GET_GLOBAL_ADJACENCY(i)[0]; 
    printf("Actual Edges Traversed = %lld\n", actual_edges_traversed);
    double edges_per_second = (((double)(actual_edges_traversed)) * ((double)(MAX_ITERATIONS)))/total_time_seconds;

    printf("\n*********************************************************************************************************\n");
    printf("Maximum Depth = %d ::: Maximum Boundary = %d ::: Sum Boundary = %d ::: Average Boundary = %.4lf\n", depth, max_counter, sum_counter, (sum_counter*1.0)/depth);
    printf("Scalar Code with %d iterations -->\n", MAX_ITERATIONS);
    printf("Time Taken = %lld cycles (%.3lf seconds) ::: Time Per Vertex = %.3lf cycles ::: Timer Per Edges = %.3lf cycles ::: Average Degree of a Vertex = %.2lf\n", 
            total_time, total_time_seconds, (total_time*1.0)/global_number_of_vertices/MAX_ITERATIONS,  (total_time*1.0)/global_number_of_edges/MAX_ITERATIONS,
            (global_number_of_edges*1.0)/global_number_of_vertices);
PRINT_RED    
    printf("Edges Per Second = %.3lf Million/sec\n", edges_per_second/1000.0/1000.0);
PRINT_BLACK
    printf("*********************************************************************************************************\n");

    printf("overall_counter1 = %lld ::: overall_counter2 = %lld\n", overall_counter1, overall_counter2);
}
 
int global_boundary_states;
int global_depth;

#ifdef MULTI_SOCKET

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Function 3:
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void Compute_Start_And_End_Offsets_for_Potential_Boundary_States(int taskid, int threadid, int socket_to_work_on, int *starting_thread_index, int *ending_thread_index, int *starting_index_offset, int *ending_index_offset)
{
    //THIS FUNCTION ASSUMES THAT MULTI_SOCKET IS ENABLED...

    if (ANOTHER_HIST_BINS != 2) ERROR_PRINT();
    unsigned long long int start_time = read_tsc();


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef EVENLY_DIVIDE_PBS

#if 0
if (ntasks == nthreads)
{
    *starting_index_offset = 0;
    int hist_index = my_socket_id;

    if (socket_to_work_on == my_socket_id)
    {
        *starting_thread_index = taskid;
        *ending_index_offset = global_Potential_Boundary_States[taskid*ANOTHER_HIST_BINS + hist_index][0];
    }
    else
    {
        *starting_thread_index = (taskid + nthreads/2) % nthreads;
        *ending_index_offset = global_Potential_Boundary_States[(*starting_thread_index)*ANOTHER_HIST_BINS + hist_index][0];
    }

    *ending_thread_index = *starting_thread_index;

    if (taskid == 7)
    {
        //printf("taskid = %d ::: socket_to_work_on = %d ::: starting_thread_index = %d ::: ending_thread_index = %d ::: starting_index_offset = %d ::: ending_index_offset = %d\n", taskid, socket_to_work_on, *starting_thread_index, *ending_thread_index, *starting_index_offset, *ending_index_offset);
    }

    return;
}
else 
{
    ERROR_PRINT();
}
#endif

    int sum_hist_0 = 0, sum_hist_1 = 0;

    for(int i=0; i<nthreads; i++)
    {
        sum_hist_0 += global_Potential_Boundary_States[i*ANOTHER_HIST_BINS + 0][0];
        sum_hist_1 += global_Potential_Boundary_States[i*ANOTHER_HIST_BINS + 1][0];
    }

    int sum = sum_hist_0 + sum_hist_1;

    int done = 0;
    int boundary_states = sum;

    int starting_index, ending_index;
        
    int local_ntasks = ntasks;
    int vertices_per_task = (boundary_states % local_ntasks) ? (boundary_states / local_ntasks + 1) : (boundary_states / local_ntasks);
            
    starting_index = taskid*vertices_per_task;
    ending_index = starting_index + vertices_per_task;
        
    if (starting_index >= boundary_states) starting_index = boundary_states;
    if (ending_index > boundary_states) ending_index = boundary_states;

        
    if (socket_to_work_on == 0)
    {
        if (starting_index >= sum_hist_0) done = 1;
        if (ending_index > sum_hist_0) ending_index = sum_hist_0;
        boundary_states = sum_hist_0;
    }
    else if (socket_to_work_on == 1)
    {
        if (ending_index <= sum_hist_0) done = 1;
        starting_index -= sum_hist_0; if (starting_index < 0) starting_index = 0;
        ending_index -= sum_hist_0;
        boundary_states = sum_hist_1;
    }

    //if (!done) printf("taskid = %d ::: socket_to_wor_on = %d ::: starting_index = %d :::  ending_index = %d ::: bs = %d\n", taskid, socket_to_work_on, starting_index, ending_index, boundary_states);

    int hist_index = socket_to_work_on;
    int starting_thread = 0;
    int ending_thread = nthreads;

    int starting_thread_itr = -1;

    if ((!done) && (starting_index < boundary_states))
    {
        int vertices_so_far = 0;

        for(int thread_itr = starting_thread; thread_itr < ending_thread; thread_itr++)
        {
            int indexx = thread_itr * ANOTHER_HIST_BINS + hist_index;

            if ( (vertices_so_far + global_Potential_Boundary_States[indexx][0]) > starting_index) 
            {
                starting_thread_itr = thread_itr;
                *starting_thread_index = thread_itr;
                *starting_index_offset = starting_index - vertices_so_far;
                break;
            }

            vertices_so_far += global_Potential_Boundary_States[indexx][0];
        }

        if ((*starting_thread_index) == -1) 
            ERROR_PRINT();
        if (starting_thread_itr == -1) 
            ERROR_PRINT();

        for(int thread_itr = starting_thread_itr; thread_itr < ending_thread; thread_itr++)
        {
            int indexx = thread_itr * ANOTHER_HIST_BINS + hist_index;
            if ( (vertices_so_far + global_Potential_Boundary_States[indexx][0]) >= ending_index)
            {
                *ending_thread_index = thread_itr;
                *ending_index_offset = ending_index - vertices_so_far;
                break;
            }

            vertices_so_far += global_Potential_Boundary_States[indexx][0];
        }
    }

#endif 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#if 1

    if (taskid == 0)
    {
    #if 0
        int ss_0 = 0; 
        int ss_1 = 0; 

        for(int i=0; i<nthreads; i++) { int indexx = i*ANOTHER_HIST_BINS + 0; ss_0 += global_Potential_Boundary_States[indexx][0]; }
        for(int i=0; i<nthreads; i++) { int indexx = i*ANOTHER_HIST_BINS + 1; ss_1 += global_Potential_Boundary_States[indexx][0]; }
        printf("ss_0 = %d ::: ss_1 = %d\n", ss_0, ss_1);

            
        for(int i=0; i<nthreads; i++)
        {
            int sum = 0;
            for(int hist_index = 0; hist_index < ANOTHER_HIST_BINS; hist_index++)
            {
                sum += global_Potential_Boundary_States[i*ANOTHER_HIST_BINS + hist_index][0];
            }
            printf("i = %d ::: sum = %d\n", i, sum);
        }
    #endif
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef EVENLY_DIVIDE_PBS // defined MULTI_SOCKET  && !(defined EVENLY_DIVIDE_PBS)

//So each socket will pick up vertices that only belong to their socket...

    int my_socket_id = COMPUTE_SOCKET_ID_FOR_TASK(taskid);
    int starting_thread, ending_thread, hist_index, local_ntasks;

    if (nsockets == 2)
    {
        local_ntasks = ntasks/2;
        hist_index = my_socket_id;
        starting_thread = socket_to_work_on * nthreads/2;
        ending_thread = starting_thread + nthreads/2;

        if (taskid >= ntasks/2) taskid -= ntasks/2;
    }
    else if (nsockets == 1)
    {
        //There is actually only 1 socket with (socket_id == 0)...
        local_ntasks = ntasks;
        starting_thread = 0;
        ending_thread = nthreads;

        if (my_socket_id  == socket_to_work_on) hist_index = 0; else hist_index = 1;
    }
    else
    {
        ERROR_PRINT();
    }

    int boundary_states = 0;
    for(int i=starting_thread; i<ending_thread; i++)
    {
        int indexx = i*ANOTHER_HIST_BINS + hist_index;
        boundary_states += global_Potential_Boundary_States[indexx][0];
    }

    int sum = boundary_states;

    int vertices_per_task = (boundary_states % local_ntasks) ? (boundary_states / local_ntasks + 1) : (boundary_states / local_ntasks);

    //printf("threadid = %d ::: depth = %d ::: boundary_states = %d\n", threadid, global_depth, boundary_states);
                //printf("states_for_socket_0: %d states_for_socket_1: %d\n", states_for_socket0, states_for_socket1);

    int starting_index = taskid*vertices_per_task;
    int ending_index = starting_index + vertices_per_task;

    if (starting_index >= boundary_states) starting_index = boundary_states;
    if (ending_index > boundary_states) ending_index = boundary_states;

    int starting_thread_itr = -1;

    if (starting_index < boundary_states)
    {
        int vertices_so_far = 0;

        for(int thread_itr = starting_thread; thread_itr < ending_thread; thread_itr++)
        {
            int indexx = thread_itr * ANOTHER_HIST_BINS + hist_index;

            if ( (vertices_so_far + global_Potential_Boundary_States[indexx][0]) > starting_index) 
            {
                starting_thread_itr = thread_itr;
                *starting_thread_index = thread_itr;
                *starting_index_offset = starting_index - vertices_so_far;
                break;
            }

            vertices_so_far += global_Potential_Boundary_States[indexx][0];
        }

        if ((*starting_thread_index) == -1) 
            ERROR_PRINT();
        if (starting_thread_itr == -1) 
            ERROR_PRINT();

        for(int thread_itr = starting_thread_itr; thread_itr < ending_thread; thread_itr++)
        {
            int indexx = thread_itr * ANOTHER_HIST_BINS + hist_index;
            if ( (vertices_so_far + global_Potential_Boundary_States[indexx][0]) >= ending_index)
            {
                *ending_thread_index = thread_itr;
                *ending_index_offset = ending_index - vertices_so_far;
                break;
            }

            vertices_so_far += global_Potential_Boundary_States[indexx][0];
        }
    }
#endif


#endif

    unsigned long long int end_time = read_tsc();
    Overhead_Timer[threadid] += (end_time - start_time);
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Function 4:
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#if 0
__attribute__((noinline)) 
void Perform_BFS_III(int taskid, int threadid, int depth, int *BS0, int start_index, int end_index, int *BS1, int *Depth)
{
    if (ANOTHER_HIST_BINS == 2) ERROR_PRINT();

    //THIS FUNCTION ASSUMES THAT MULTI_SOCKET IS ENABLED...
    unsigned long long int stime = read_tsc();

    int **PBS = global_Potential_Boundary_States + (threadid * ANOTHER_HIST_BINS);

    int socketid = COMPUTE_SOCKET_ID(threadid);

    int ** local_Adjacency = (socketid)?(global_Adjacency_Socket1):(global_Adjacency_Socket0);

#define ANOTHER_PREF_DIST2 8

    for(int j = start_index; j < end_index; j++)
    {
        _mm_prefetch((char *)(BS0 + j + 8+16), _MM_HINT_T0);
        _mm_prefetch((char *)(GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST2+0], local_Adjacency)), _MM_HINT_T0);
        _mm_prefetch((char *)(GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST2+1], local_Adjacency)), _MM_HINT_T0);
        _mm_prefetch((char *)(GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST2+2], local_Adjacency)), _MM_HINT_T0);
        _mm_prefetch((char *)(GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST2+3], local_Adjacency)), _MM_HINT_T0);
        _mm_prefetch((char *)(GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST2+4], local_Adjacency)), _MM_HINT_T0);

        int state_id = BS0[j];
        int *temp_address = GET_GLOBAL_ADJACENCY_2(state_id, local_Adjacency );
        int neighbors = temp_address[0];

        for(int k=1; k<=neighbors; k+=1)
        {
            int new_state_id_0 = temp_address[k+0];
            int bin_id = 0;
            for(int l=0; l<ANOTHER_HIST_BINS; l++) bin_id += (new_state_id_0 >= Quantiles[l]);
            bin_id--;
            //int bin_id = (new_state_id_0 >> ANOTHER_RIGHT_SHIFT);
            PBS[bin_id][++(PBS[bin_id][0])] = new_state_id_0;

#ifndef USE_LARGE_PAGE
            if (PBS[bin_id][0] > global_max_pbs_entries) printf("PBS[bin_id][0] = %d ::: global_max_pbs_entries = %lld\n", PBS[bin_id][0], global_max_pbs_entries);
#endif
        }
        

        //if(PBS[1][0] != 0){ printf("%d \n", PBS[1][0]); }
    }

#if 0
    for(int bin_id = 0; bin_id<ANOTHER_HIST_BINS; bin_id++)
    {
        for(int k=1; k<=PBS[bin_id][0]; k++)
        {
            int value = PBS[bin_id][k];
            //if (!( (value >= Quantiles[bin_id]) && (value < Quantiles[bin_id+1])) ) ERROR_PRINT();
        }
    }
#endif

    unsigned long long int etime = read_tsc();
    Timer[threadid] += (etime - stime);
    Timer4[threadid][depth] += (etime - stime);
}
#endif


//__attribute__((noinline)) 
void Perform_BFS_I(int taskid, int threadid, int depth, int *BS0, int start_index, int end_index, int *BS1, int *Depth)
{
    //THIS FUNCTION ASSUMES THAT MULTI_SOCKET IS ENABLED...
    //It is possible to have taskid != threadid...
    
    if (ANOTHER_HIST_BINS != 2) ERROR_PRINT();

    unsigned long long int stime = read_tsc();

    int **PBS = global_Potential_Boundary_States + (threadid * ANOTHER_HIST_BINS);

    if(ANOTHER_HIST_BINS != 2) { printf("The code was expecting ANOTHER_HIST_BINS == 2\n"); ERROR_PRINT(); }

    int *PBS0 = PBS[0];
    int *PBS1 = PBS[1];
    int index_0 = PBS0[0]+1;
    int index_1 = PBS1[0]+1;

    int socketid = COMPUTE_SOCKET_ID(threadid);

    unsigned int all_ones = 0xFFFFFFFF;
    __m128i xmm_all_ones = _MM_SET1(all_ones);
    __m128i xmm_mean = _MM_SET1(global_vertices_for_socket0);

    int ** local_Adjacency = (socketid)?(global_Adjacency_Socket1):(global_Adjacency_Socket0);

    int old_index_0 = index_0;
    int old_index_1 = index_1;

#define ANOTHER_PREF_DIST3 8
//if (threadid < CORES_PER_SOCKET)
//{
    for(int j = start_index; j < end_index; j++)
    {
        _mm_prefetch((char *)(BS0 + j + 8 + 16), _MM_HINT_T0);
        int **addr_temp_address0 = GET_GLOBAL_ADJACENCY_Addr(BS0[j + ANOTHER_PREF_DIST3+0]);
        int *temp_address0 = GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST3+0], NULL);

        int **addr_temp_address1 = GET_GLOBAL_ADJACENCY_Addr(BS0[j + ANOTHER_PREF_DIST3+1]);
        int *temp_address1 = GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST3+1], NULL);

        //int **addr_temp_address2 = GET_GLOBAL_ADJACENCY_Addr(BS0[j + ANOTHER_PREF_DIST3+2]);
        //int *temp_address2 = GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST3+2], NULL);

        _mm_prefetch((char *)(addr_temp_address0), _MM_HINT_T1);
        _mm_prefetch((char *)(temp_address0), _MM_HINT_T1);

        _mm_prefetch((char *)(addr_temp_address1), _MM_HINT_T1);
        _mm_prefetch((char *)(temp_address1), _MM_HINT_T1);

        //_mm_prefetch((char *)(addr_temp_address2), _MM_HINT_T1);
        //_mm_prefetch((char *)(temp_address2), _MM_HINT_T1);
        //_mm_prefetch((char *)(*temp_address0), _MM_HINT_T0);
        //_mm_prefetch((char *)(*temp_address1), _MM_HINT_T0);
        //_mm_prefetch((char *)(GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST3+0], local_Adjacency)), _MM_HINT_T0);
        //_mm_prefetch((char *)(GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST3+1], local_Adjacency)), _MM_HINT_T0);
        
        
        _mm_prefetch((char *)(GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST3+0], local_Adjacency)), _MM_HINT_T0);
        _mm_prefetch((char *)(GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST3+1], local_Adjacency)), _MM_HINT_T0);
        _mm_prefetch((char *)(GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST3+2], local_Adjacency)), _MM_HINT_T0);
        //_mm_prefetch((char *)(GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST3+3], local_Adjacency)), _MM_HINT_T0);
        //_mm_prefetch((char *)(GET_GLOBAL_ADJACENCY_2(BS0[j + ANOTHER_PREF_DIST3+4], local_Adjacency)), _MM_HINT_T0);

        int state_id = BS0[j];
        //if( (threadid == 0) && (j< 30)) printf("j: %d ::: state_id : %d\n", j, state_id);
#if 0
	if( (state_id >= global_vertices_for_socket0) && (socketid == 0)){
	  printf("stateid: %d global_vertices_for_socket0: %d Assumption failure 1\n", state_id, global_vertices_for_socket0 );
	  ERROR_PRINT();
	} 
	if( (state_id < global_vertices_for_socket0) && (socketid == 1)){
	  printf("tid: %d stateid: %d global_vertices_for_socket0: %d Assumption failure 2\n", threadid, state_id, global_vertices_for_socket0 );
	  ERROR_PRINT();
	} 
#endif

        int *temp_address = GET_GLOBAL_ADJACENCY_2(state_id, local_Adjacency );
        int neighbors = temp_address[0];
        //int neighbors = 2;
        int remaining = neighbors;

            
        *(PBS0+index_0) = -(state_id+1); index_0++;
        if (nsockets == 2)
        {
            *(PBS1+index_1) = -(state_id+1); index_1++;
        }

#if 0
        for(int k=1; k<=neighbors; k+=1)
        {
            int new_state_id_0 = temp_address[k+0];
            int bin_id = (new_state_id_0 >> ANOTHER_RIGHT_SHIFT);
            PBS[bin_id][++(PBS[bin_id][0])] = new_state_id_0;
        }
        
        //if(PBS[1][0] != 0){ printf("%d \n", PBS[1][0]); }
#else

        for(int k=0; k<neighbors; k+=4, remaining -= 4)
        {
            __m128i xmm_anding = _MM_LOAD(Remaining[(remaining>4)?4:remaining]);
            __m128i xmm_val = _MM_LOADU(temp_address + k + 1);

            __m128i mask_0 = _MM_CMP_LT(xmm_val, xmm_mean);
            __m128i mask_1 = _MM_NOT(mask_0);

            int mask_value_0 = _mm_movemask_ps(_mm_castsi128_ps(_MM_AND(mask_0, xmm_anding))); //mask_value is [0..15]
            int number_of_ones_0 = Number_of_Ones[mask_value_0];
            _MM_STOREU((PBS0 + index_0), _MM_SHUFFLE_EPI8(xmm_val, Shuffle_Pattern[mask_value_0]));
            index_0 += number_of_ones_0;

            int mask_value_1 = _mm_movemask_ps(_mm_castsi128_ps(_MM_AND(mask_1, xmm_anding))); //mask_value is [0..15]
            int number_of_ones_1 = Number_of_Ones[mask_value_1];
            _MM_STOREU((PBS1 + index_1), _MM_SHUFFLE_EPI8(xmm_val, Shuffle_Pattern[mask_value_1]));
            index_1 += number_of_ones_1;
        }
#endif

#ifndef USE_LARGE_PAGE
        if (index_0 > global_max_pbs_entries) printf("index_0 = %d ::: global_max_pbs_entries = %lld\n", index_0, global_max_pbs_entries);
        if (index_1 > global_max_pbs_entries) printf("index_1 = %d ::: global_max_pbs_entries = %lld\n", index_1, global_max_pbs_entries);
        //printf("index_0 = %d ::: index_1 = %d\n", index_0, index_1);
#endif


#if 0
        long long int MAX_INDEX = 2*global_number_of_edges/nthreads + 121;
        if(index_0 > MAX_INDEX) {
            printf("Index 0 bound crossed! index_0: %d MAX_INDEX: %lld\n", index_0, MAX_INDEX); ERROR_PRINT();
        }
        if(index_1 > MAX_INDEX) {
            printf("Index 1 bound crossed!\n"); ERROR_PRINT();
        }
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#define THRESHOLD (1<<21)
        if (((index_0 - old_index_0) > THRESHOLD))
        {
            Further_Rearrange_PBS(threadid, (PBS0 + old_index_0), (index_0 - old_index_0));
            old_index_0 = index_0;
        }

        if (((index_1 - old_index_1) > THRESHOLD))
        {
            Further_Rearrange_PBS(threadid, (PBS1 + old_index_1), (index_1 - old_index_1));
            old_index_1 = index_1;
        }
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    }

    PBS0[0] = index_0-1;
    PBS1[0] = index_1-1;
            
#if 0
    Further_Rearrange_PBS(threadid, (PBS0 + old_index_0), (index_0 - old_index_0));
    Further_Rearrange_PBS(threadid, (PBS1 + old_index_1), (index_1 - old_index_1));
#endif

//}
#if 0
else
{
    //theadid - 6 is my SMT thread...
        {
            if ((index_0 - old_index_0) > 10000)
            {
                Trigger_Rearrangement();
            }

            if ((index_1 - old_index_1) > 10000)
            {
            Trigger_Rearrangement();
            }
        }
}
#endif


        
    //unsigned long long int eetime = read_tsc();
    //int local_count = (PBS0[0] + PBS1[0]);
    //printf("Time for (%d) elements = %.2lf cycles/element \n", local_count, (eetime - stime)/1.00/local_count);
#if 0
    {
        //unsigned long long int etime = read_tsc();
        //int local_count = (index_0 + index_1 - PBS0[0] - PBS1[0]);
        //printf("Time for (%d) elements = %.2lf cycles/element \n", local_count, (etime - stime)/1.00/local_count);
    }

#endif


    unsigned long long int etime = read_tsc();
    Timer[threadid] += (etime - stime);
    Timer4[threadid][depth] += (etime - stime);
}

int BookKeeping[64];

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Function 5:
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//__attribute__((noinline)) 

#if 0
void Perform_BFS_IV(int taskid, int threadid, int depth, int *BS1, int *next_counter, int *Depth)
{
    //It is possible to have taskid != threadid...
    //THIS FUNCTION ASSUMES THAT MULTI_SOCKET IS ENABLED...

    unsigned long long int task_start_time = read_tsc();

    if (ANOTHER_HIST_BINS == 2) ERROR_PRINT();

#if 0
    if (threadid == 0)
    {
            
        printf("depth = %d :::", depth);
        for(int j=0; j<ANOTHER_HIST_BINS; j++)
        {
            int sum = 0;
            for(int k=0; k<nthreads; k++)
            {
                sum += global_Potential_Boundary_States[ k * ANOTHER_HIST_BINS + j][0];
            }

            printf(" %8d ", sum);
        }
        printf("\n");
    }
#endif

    int next_counter_0 = *next_counter;

#ifdef TLB_BLOCKING
    int *Hist = global_Histogram[threadid];
#endif

    int sum_total = 0;
    int max_so_far = 0;
    int min_so_far = global_number_of_edges;

    for(int thr=0; thr < nthreads; thr++)
    {
        int **PBS = global_Potential_Boundary_States + (thr * ANOTHER_HIST_BINS);
        for(int j=0; j<ANOTHER_HIST_BINS; j++) 
        {
            sum_total += PBS[j][0];
            max_so_far = MAX(max_so_far, PBS[j][0]);
            min_so_far = MIN(min_so_far, PBS[j][0]);
        }
    }


#ifndef USE_LARGE_PAGE
    if (threadid == 0) BookKeeping[depth] = sum_total;
    if (threadid == 0)
    {
        printf("depth = %d ::: min_so_far = %d ::: max_so_far = %d ::: sum_total = %d\n", depth, min_so_far, max_so_far, sum_total);
    }
#endif

    int vertices_per_task = (sum_total % ntasks) ? (sum_total/ntasks +1) : (sum_total/ntasks);
    int starting_index = vertices_per_task * taskid;
    int ending_index = starting_index + vertices_per_task;

    if (starting_index > sum_total) starting_index = sum_total;
    if (ending_index   > sum_total) ending_index   = sum_total;

    sum_total = 0;

        
#ifdef BLOOM_FILTER
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+- //+-+-+-+-+-+-+-+-+-+-+-+-+-+- //+-+-+-+-+-+-+-+-+-+-+-+-+-+-
    #ifdef BYTE_LEVEL
        unsigned char *local_Assigned;
    #else
        unsigned int *local_Assigned;
    #endif
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+- //+-+-+-+-+-+-+-+-+-+-+-+-+-+- //+-+-+-+-+-+-+-+-+-+-+-+-+-+-
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+- //+-+-+-+-+-+-+-+-+-+-+-+-+-+- //+-+-+-+-+-+-+-+-+-+-+-+-+-+-
    #ifdef SUB_BIT_LEVEL
        unsigned int *local_Assigned_sub_bit;
    #endif
    //+-+-+-+-+-+-+-+-+-+-+-+-+-+- //+-+-+-+-+-+-+-+-+-+-+-+-+-+- //+-+-+-+-+-+-+-+-+-+-+-+-+-+-
        
#else
        unsigned int *local_Assigned;
#endif
        
#if 0
    for(int bin=threadid; bin<(threadid+1); bin++) 
    {
        int **PBS = global_Potential_Boundary_States + bin;
        for(int thr=0; thr < nthreads; thr++)
        {
                //Do the work...
            int starting_offset = 0;
            int   ending_offset = PBS[0][0];

            for (int k=1+starting_offset; k<=ending_offset; k++)
            {
                _mm_prefetch((char *)(PBS[0] + k + 16), _MM_HINT_T0);
                int new_state_id_0 = PBS[0][k];

             #ifdef BLOOM_FILTER
                local_Assigned = (new_state_id_0 < global_vertices_for_socket0) ? global_Assigned_Socket0 : global_Assigned_Socket1;
                #ifdef SUB_BIT_LEVEL
                  local_Assigned_sub_bit = (new_state_id_0 < global_vertices_for_socket0) ? global_Assigned_sub_bit_Socket0 : global_Assigned_sub_bit_Socket1;
                #endif
             #endif

                  UPDATE_DEPTH_AND_BS_Socket(Depth, new_state_id_0, NOT_ASSIGNED, depth, BS1, next_counter_0, local_Assigned);
                    
            }
            PBS += ANOTHER_HIST_BINS;
        }

    }
#endif

#if 1
    for(int bin=0; bin<ANOTHER_HIST_BINS; bin++) 
    {
        int **PBS = global_Potential_Boundary_States + bin;
        for(int thr=0; thr < nthreads; thr++)
        {
            if ( (sum_total + PBS[0][0]) > starting_index)
            {
                //Do the work...

                int starting_offset = starting_index - sum_total;
                int   ending_offset =   ending_index - sum_total;

                starting_offset = MIN( MAX(starting_offset, 0), PBS[0][0]);
                  ending_offset = MIN( MAX(  ending_offset, 0), PBS[0][0]);

                  for (int k=1+starting_offset; k<=ending_offset; k++)
                  {
                      _mm_prefetch((char *)(PBS[0] + k + 16), _MM_HINT_T0);
                      int new_state_id_0 = PBS[0][k];

                    #ifdef BLOOM_FILTER
                      local_Assigned = (new_state_id_0 < global_vertices_for_socket0) ? global_Assigned_Socket0 : global_Assigned_Socket1;
                      #ifdef SUB_BIT_LEVEL
                            local_Assigned_sub_bit = (new_state_id_0 < global_vertices_for_socket0) ? global_Assigned_sub_bit_Socket0 : global_Assigned_sub_bit_Socket1;
                      #endif
                    #endif

                      UPDATE_DEPTH_AND_BS_Socket(Depth, new_state_id_0, NOT_ASSIGNED, depth, BS1, next_counter_0, local_Assigned);
                    
                  }
            }

            sum_total += PBS[0][0];
            PBS += ANOTHER_HIST_BINS;
        }

        if (sum_total > ending_index) break;
    }
#endif

    ///////////////////////////////////////////// ///////////////////////////////////////////// /////////////////////////////////////////////
    /////////////////////////////////////////////STEP 3: ///////////////////////////////
    ///////////////////////////////////////////// ///////////////////////////////////////////// /////////////////////////////////////////////

    *next_counter = next_counter_0;

    if (next_counter_0 >= MAX_BOUNDARY_STATES)
    {
        printf("MAX_BOUNDARY_STATES = %d ::: boundary states = %d\n", MAX_BOUNDARY_STATES, next_counter_0);
        ERROR_PRINT();
    }

    unsigned long long int task_end_time = read_tsc();
    Timer[threadid] += (task_end_time - task_start_time);
    Timer5[threadid][depth] += (task_end_time - task_start_time);
}
#endif


void Perform_BFS_II(int taskid, int threadid, int depth, int *BS1, int *next_counter, int **BS_Count_per_bin1)
{
    //THIS FUNCTION ASSUMES THAT MULTI_SOCKET IS ENABLED...
    //It is possible to have taskid != threadid...

    unsigned long long int task_start_time = read_tsc();

    int socket_id = COMPUTE_SOCKET_ID(threadid);

    int next_counter_0 = *next_counter;

    int starting_thread_index, ending_thread_index, starting_index_offset, ending_index_offset;
    int socket_to_work_on;
    ///////////////////////////////////////////// ///////////////////////////////////////////// /////////////////////////////////////////////
    /////////////////////////////////////////////STEP 1: ///////////////////////////////
    ///////////////////////////////////////////// ///////////////////////////////////////////// /////////////////////////////////////////////
    
    //Compute the [start,end] vertices for my socket_id within my socket...
   
    socket_to_work_on = socket_id;
    starting_thread_index = -1; ending_thread_index = -1; starting_index_offset = -1; ending_index_offset = -1;
    Compute_Start_And_End_Offsets_for_Potential_Boundary_States(taskid, threadid, socket_to_work_on, &starting_thread_index, &ending_thread_index, &starting_index_offset, &ending_index_offset);


    if ((starting_thread_index != -1) && (ending_thread_index != -1) && (starting_index_offset != -1) && (ending_index_offset != -1))
    {
        int hist_index = socket_id;
        int old_boundary_states = next_counter_0;
    #ifdef TLB_BLOCKING
        int *Hist = global_Histogram[threadid];
        RESET_HISTOGRAM(Hist);
    #endif

#ifdef BLOOM_FILTER
    #ifdef BYTE_LEVEL
        unsigned char *local_Assigned;
    #else
        unsigned int *local_Assigned;
    #endif

        if (hist_index == 0) local_Assigned = global_Assigned_Socket0;
        else local_Assigned = global_Assigned_Socket1;
#else
        unsigned int *local_Assigned;
#endif

        int *Depth = (hist_index == 0) ? global_DepthParent_Socket0 : global_DepthParent_Socket1;

        for(int thread_itr = starting_thread_index; thread_itr <= ending_thread_index; thread_itr++)
        {
            int local_starting_index, local_ending_index;

            if (thread_itr == starting_thread_index) local_starting_index = starting_index_offset;
            else local_starting_index = 0;

            if (thread_itr == ending_thread_index) local_ending_index = ending_index_offset;
            else local_ending_index = global_Potential_Boundary_States[thread_itr * ANOTHER_HIST_BINS + hist_index][0];

            int *PBS_prime = global_Potential_Boundary_States[thread_itr * ANOTHER_HIST_BINS + hist_index];

            int number_of_elements = local_ending_index - local_starting_index;
            //int starting_point = (1+local_starting_index) + (threadid * number_of_elements)/nthreads;
            int starting_point = (1+local_starting_index) + 0;

            int parent_id = PBS_prime[starting_point];
            if (parent_id >= 0) { for(int k=(starting_point-1); k>0; k--) { parent_id = PBS_prime[k]; if (parent_id < 0) break; } }
            parent_id = -parent_id-1;
            if (parent_id < 0) ERROR_PRINT();
            

            for(int j=starting_point; j<=local_ending_index; j++)
            {
                _mm_prefetch((char *)(PBS_prime + j + 16), _MM_HINT_T0);
                int new_state_id_0 = PBS_prime[j];
                if (new_state_id_0 < 0) { parent_id = -new_state_id_0-1; continue;}
                        
                UPDATE_DEPTHPARENT_AND_BS_Socket(Depth, new_state_id_0, NOT_ASSIGNED, depth, BS1, next_counter_0, local_Assigned, parent_id);
            }

            for(int j=(1+local_starting_index); j<starting_point; j++)
            {
                ERROR_PRINT();
                _mm_prefetch((char *)(PBS_prime + j + 16), _MM_HINT_T0);
                int new_state_id_0 = PBS_prime[j];
                UPDATE_DEPTHPARENT_AND_BS_Socket(Depth, new_state_id_0, NOT_ASSIGNED, depth, BS1, next_counter_0, local_Assigned, new_state_id_0);
            }
        }

        BS_Count_per_bin1[threadid][hist_index] += (next_counter_0 - old_boundary_states);
      #ifdef TLB_BLOCKING
        Rearrange_for_Reduced_TLB_Misses(threadid, BS1, old_boundary_states, next_counter_0);
      #endif
    }

    ///////////////////////////////////////////// ///////////////////////////////////////////// /////////////////////////////////////////////
    /////////////////////////////////////////////STEP 2: ///////////////////////////////
    ///////////////////////////////////////////// ///////////////////////////////////////////// /////////////////////////////////////////////

    //Compute the [start,end] vertices for other socket_id within my socket...

    socket_to_work_on = (1-socket_id);
    starting_thread_index = -1; ending_thread_index = -1; starting_index_offset = -1; ending_index_offset = -1;
    Compute_Start_And_End_Offsets_for_Potential_Boundary_States(taskid, threadid, socket_to_work_on, &starting_thread_index, &ending_thread_index, &starting_index_offset, &ending_index_offset);


    if ((starting_thread_index != -1) && (ending_thread_index != -1) && (starting_index_offset != -1) && (ending_index_offset != -1))
    {
    
    #ifdef EVENLY_DIVIDE_PBS
        int hist_index =  (nsockets == 1) ? 1 : socket_to_work_on;
    #else
        int hist_index =  (nsockets == 1) ? 1 : socket_id;
    #endif

#ifdef BLOOM_FILTER
    #ifdef BYTE_LEVEL
        unsigned char *local_Assigned;
    #else
        unsigned int *local_Assigned;
    #endif

        if (hist_index == 0) local_Assigned = global_Assigned_Socket0;
        else local_Assigned = global_Assigned_Socket1;
#else
        unsigned int *local_Assigned;
#endif

        int *Depth = (hist_index == 0) ? global_DepthParent_Socket0 : global_DepthParent_Socket1;
        int old_boundary_states = next_counter_0;
    #ifdef TLB_BLOCKING
        int *Hist = global_Histogram[threadid];
        RESET_HISTOGRAM(Hist);
    #endif

        for(int thread_itr = starting_thread_index; thread_itr <= ending_thread_index; thread_itr++)
        {
            int local_starting_index, local_ending_index;

            if (thread_itr == starting_thread_index) local_starting_index = starting_index_offset;
            else local_starting_index = 0;

            if (thread_itr == ending_thread_index) local_ending_index = ending_index_offset;
            else local_ending_index = global_Potential_Boundary_States[thread_itr * ANOTHER_HIST_BINS + hist_index][0];

            int *PBS_prime = global_Potential_Boundary_States[thread_itr * ANOTHER_HIST_BINS + hist_index];

            int number_of_elements = local_ending_index - local_starting_index;
            //int starting_point = (1+local_starting_index) + (threadid * number_of_elements)/nthreads;
            int starting_point = (1+local_starting_index) + 0;

            int parent_id = PBS_prime[starting_point];
            if (parent_id >= 0) { for(int k=(starting_point-1); k>0; k--) { parent_id = PBS_prime[k]; if (parent_id < 0) break; } }
            parent_id = -parent_id-1;
            if (parent_id < 0) ERROR_PRINT();

            for(int j=starting_point; j<=local_ending_index; j++)
            {
                _mm_prefetch((char *)(PBS_prime + j + 16), _MM_HINT_T0);
                int new_state_id_0 = PBS_prime[j];
                if (new_state_id_0 < 0) { parent_id = -new_state_id_0-1; continue;}

                UPDATE_DEPTHPARENT_AND_BS_Socket(Depth, new_state_id_0, NOT_ASSIGNED, depth, BS1, next_counter_0, local_Assigned, parent_id);
            }

            for(int j=(1+local_starting_index); j<starting_point; j++)
            {
                ERROR_PRINT();
                _mm_prefetch((char *)(PBS_prime + j + 16), _MM_HINT_T0);
                int new_state_id_0 = PBS_prime[j];
                UPDATE_DEPTHPARENT_AND_BS_Socket(Depth, new_state_id_0, NOT_ASSIGNED, depth, BS1, next_counter_0, local_Assigned, new_state_id_0);
            }
        }

        BS_Count_per_bin1[threadid][hist_index] += (next_counter_0 - old_boundary_states);
      #ifdef TLB_BLOCKING
        Rearrange_for_Reduced_TLB_Misses(threadid, BS1, old_boundary_states, next_counter_0);
      #endif
    }


    ///////////////////////////////////////////// ///////////////////////////////////////////// /////////////////////////////////////////////
    /////////////////////////////////////////////STEP 3: ///////////////////////////////
    ///////////////////////////////////////////// ///////////////////////////////////////////// /////////////////////////////////////////////

    *next_counter = next_counter_0;

    if (next_counter_0 >= MAX_BOUNDARY_STATES)
    {
        printf("MAX_BOUNDARY_STATES = %d ::: boundary states = %d\n", MAX_BOUNDARY_STATES, next_counter_0);
        ERROR_PRINT();
    }

    unsigned long long int task_end_time = read_tsc();
    Timer[threadid] += (task_end_time - task_start_time);
    Timer5[threadid][depth] += (task_end_time - task_start_time);
}

#endif //MULTI_SOCKET


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Function 6:
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

__inline 
void Compute_Start_And_End_Offsets_New(int taskid, int *Count_B0, int local_ntasks, int boundary_states, int *starting_thread_index, int *ending_thread_index, int *starting_index_offset, int *ending_index_offset, 
        int *starting_bin_id, int *ending_bin_id, int **BS_Count_per_bin0)
{
    //Note added 04/25/2011: Using the new division scheme, the work
    //is divided on a per-bin basis... 

    for(int k=0; k<ANOTHER_HIST_BINS; k++) starting_thread_index[k] = -1;
    for(int k=0; k<ANOTHER_HIST_BINS; k++) ending_thread_index[k] = -1;
    for(int k=0; k<ANOTHER_HIST_BINS; k++) starting_index_offset[k] = -1;
    for(int k=0; k<ANOTHER_HIST_BINS; k++) ending_index_offset[k] = -1;
    *starting_bin_id = -1;
    *ending_bin_id = -1;

#ifdef USE_TASKQ

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        int starting_bin  = 0;
        int ending_bin = ANOTHER_HIST_BINS;

        // Enabling this line means a socket can only pick work from its own socket... So NO load-balancing between sockets...
        #ifndef EVENLY_DIVIDE_BS //<-- NOT DEFINING ensures BFS_I is "NOT LOAD BALANCED" (i.e. a socket CANNOT read global_Adjacency_0/1 across sockets...
        //Satish: For MULTI_SOCKET, the condition below should be made true, so that each socket finds work within its socket...
        {
            if (nsockets == 2)
            {
            // Each socket picks up works within its socket... So NO inter-socket load-balancing will happen :)

                int states_for_socket0 = 0;
                int states_for_socket1 = 0;
                int socket_id = COMPUTE_SOCKET_ID_FOR_TASK(taskid);

                if (socket_id == 0)
                {
                    for(int bin_id=0; bin_id < (ANOTHER_HIST_BINS/2); bin_id++)
                    {
                        for(int i=0; i<nthreads; i++) states_for_socket0 += BS_Count_per_bin0[i][bin_id];
                    }

                    boundary_states = states_for_socket0; 
                    ending_bin  = ANOTHER_HIST_BINS/2;
                }
                else if (socket_id == 1)
                {
                    for(int bin_id=(ANOTHER_HIST_BINS/2); bin_id < (ANOTHER_HIST_BINS); bin_id++)
                    {
                        for(int i=0; i<nthreads; i++) states_for_socket1 += BS_Count_per_bin0[i][bin_id];
                    }

                    boundary_states = states_for_socket1;
                    starting_bin = ANOTHER_HIST_BINS/2;
                    taskid -= (local_ntasks/2);
                }
                else ERROR_PRINT();

    	        if (taskid < 0) ERROR_PRINT();
    	        if (taskid >= (local_ntasks/2)) ERROR_PRINT();

                local_ntasks = local_ntasks>>1;
                //printf("states_for_socket_0: %d states_for_socket_1: %d\n", states_for_socket0, states_for_socket1);
            }
        }
        #endif

        int sum = boundary_states;
        int vertices_per_task = (boundary_states % local_ntasks) ? (boundary_states / local_ntasks + 1) : (boundary_states / local_ntasks);

        //printf("threadid = %d ::: depth = %d ::: vertices_per_thread = %d\n", threadid, depth, vertices_per_thread);
    
        int starting_index = taskid*vertices_per_task;
        int ending_index = starting_index + vertices_per_task;

        if (starting_index > boundary_states) starting_index = boundary_states;
        if (ending_index > boundary_states) ending_index = boundary_states;

        int done_and_dusted = 0;
        int already_started = 0;

        if (starting_index < boundary_states)
        {
            int vertices_so_far = 0;

            for(int bin_id = starting_bin; bin_id < ending_bin; bin_id++)
            {
                int thread_itr = 0;
                if (already_started) 
                {
                    starting_thread_index[bin_id] = 0;
                    starting_index_offset[bin_id] = 0;
                }
                else
                {
                    for(; thread_itr < nthreads; thread_itr++)
                    {
                        if ( (vertices_so_far + BS_Count_per_bin0[thread_itr][bin_id]) > starting_index) 
                        {
                            starting_thread_index[bin_id] = thread_itr;
                            starting_index_offset[bin_id] = starting_index - vertices_so_far;
                            *starting_bin_id = bin_id;
                            already_started = 1;
                            break;
                        }
                        vertices_so_far += BS_Count_per_bin0[thread_itr][bin_id];
                    }
                }

                if (!done_and_dusted)
                {
                    for(; thread_itr < nthreads; thread_itr++)
                    {
                        if ( (vertices_so_far + BS_Count_per_bin0[thread_itr][bin_id]) >= ending_index)
                        {
                            *ending_bin_id = bin_id;
                            ending_thread_index[bin_id] = thread_itr;
                            ending_index_offset[bin_id] = ending_index - vertices_so_far;
                            done_and_dusted = 1;
                            break;
                        }
                        vertices_so_far += BS_Count_per_bin0[thread_itr][bin_id];
                    }
                }

                if (done_and_dusted)
                {
                    break;
                }
                else
                {
                    if (already_started)
                    {
                        ending_thread_index[bin_id] = nthreads-1;
                        ending_index_offset[bin_id] = BS_Count_per_bin0[nthreads-1][bin_id];
                    }
                }
            }
        
            if ( (!done_and_dusted) || (!already_started)) ERROR_PRINT();
        }
        else
        {
            //ERROR_PRINT();
        }

    }

#else //NOT_USING_TASKQ
    //The following code uses NN to perform load-balancing... This is probably going to be our last bet to perform well :)

    {
        printf("This still has to be implemented --- looking at neighbor count and such :)\n");
        ERROR_PRINT();
    }
#endif
    //if(global_depth < 2) printf("task: %d starting_bin_id: %d ending_bin_id :%d\n", taskid, *starting_bin_id, *ending_bin_id);
}



__inline 
void Compute_Start_And_End_Offsets(int taskid, int *Count_B0, int local_ntasks, int boundary_states, int *starting_thread_index, int *ending_thread_index, int *starting_index_offset, int *ending_index_offset)
{

#ifdef USE_TASKQ

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        int starting_thread  = 0;
        int ending_thread = nthreads;

        // Enabling this line means a socket can only pick work from its own socket... So NO load-balancing between sockets...
        #ifndef EVENLY_DIVIDE_BS //<-- NOT DEFINING ensures BFS_I is "NOT LOAD BALANCED" (i.e. a socket CANNOT read global_Adjacency_0/1 across sockets...
        //Satish: For MULTI_SOCKET, the condition below should be made true, so that each socket finds work within its socket...
        {
            if (nsockets == 2)
            {
            // Each socket picks up works within its socket... So NO inter-socket load-balancing will happen :)

                int states_for_socket0 = 0;
                int states_for_socket1 = 0;
                int socket_id = COMPUTE_SOCKET_ID_FOR_TASK(taskid);

                for(int i=0; i<(nthreads/2); i++) states_for_socket0 += Count_B0[i];
                for(int i=0; i<(nthreads/2); i++) states_for_socket1 += Count_B0[i + (nthreads/2)];

                if (socket_id == 0) 
                {
                    boundary_states = states_for_socket0; 
                    ending_thread  = nthreads/2;
                }
                else if (socket_id == 1) 
                {
                    boundary_states = states_for_socket1;
                    taskid -= (local_ntasks/2);
                    starting_thread = nthreads/2;
                }
                else ERROR_PRINT();

    	        if (taskid < 0) ERROR_PRINT();
    	        if (taskid >= (local_ntasks/2)) ERROR_PRINT();

                local_ntasks = local_ntasks>>1;
                //printf("states_for_socket_0: %d states_for_socket_1: %d\n", states_for_socket0, states_for_socket1);
            }
        }
        #endif


        int sum = boundary_states;
        int vertices_per_task = (boundary_states % local_ntasks) ? (boundary_states / local_ntasks + 1) : (boundary_states / local_ntasks);

        //printf("threadid = %d ::: depth = %d ::: vertices_per_thread = %d\n", threadid, depth, vertices_per_thread);
    
        int starting_index = taskid*vertices_per_task;
        int ending_index = starting_index + vertices_per_task;

        if (starting_index >= boundary_states) starting_index = boundary_states;
        if (ending_index > boundary_states) ending_index = boundary_states;

        int starting_thread_itr = -1;

        if (starting_index < boundary_states)
        {
            int vertices_so_far = 0;

            for(int thread_itr = starting_thread; thread_itr < ending_thread; thread_itr++)
            {
                if ( (vertices_so_far + Count_B0[thread_itr]) > starting_index) 
                {
                    starting_thread_itr = thread_itr;
                    *starting_thread_index = thread_itr;
                    *starting_index_offset = starting_index - vertices_so_far;
                    break;
                }

                vertices_so_far += Count_B0[thread_itr];
            }

            if ((*starting_thread_index) == -1) ERROR_PRINT();
            if (starting_thread_itr == -1) ERROR_PRINT();

            for(int thread_itr = starting_thread_itr; thread_itr < ending_thread; thread_itr++)
            {
                if ( (vertices_so_far + Count_B0[thread_itr]) >= ending_index)
                {
                    *ending_thread_index = thread_itr;
                    *ending_index_offset = ending_index - vertices_so_far;
                    break;
                }

                vertices_so_far += Count_B0[thread_itr];
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#else //NOT_USING_TASKQ
    //The following code uses NN to perform load-balancing... This is probably going to be our last bet to perform well :)

    {
        int **NN = Number_of_Neighbors0;
        int sum = 0;
        //Divide work based on the number of neighbors...
        for(int i=0; i<nthreads; i++) sum += NN[i][Count_B0[i]];
        int total_number_of_neighbors = sum;

        int starting_thread  = 0;
        int ending_thread = nthreads;

        // Enabling this line means a socket can only pick work from its own socket... So NO load-balancing between sockets...
        #ifndef EVENLY_DIVIDE_BS //<-- NOT DEFINING ensures BFS_I is "NOT LOAD BALANCED" (i.e. a socket CANNOT read global_Adjacency_0/1 across sockets...
        //Satish: For MULTI_SOCKET, the condition below should be made true, so that each socket finds work within its socket...
        {
            if (nsockets == 2)
            {
            // Each socket picks up works within its socket... So NO inter-socket load-balancing will happen :)

                int states_for_socket0 = 0;
                int states_for_socket1 = 0;
                int socket_id = COMPUTE_SOCKET_ID_FOR_TASK(taskid);

                for(int i=0; i<(nthreads/2); i++)        states_for_socket0 += NN[i][Count_B0[i]];
                for(int i=nthreads/2; i<(nthreads); i++) states_for_socket1 += NN[i][Count_B0[i]];

                if (socket_id == 0) 
                {
                    total_number_of_neighbors = states_for_socket0; 
                    ending_thread  = nthreads/2;
                }
                else if (socket_id == 1) 
                {
                    total_number_of_neighbors = states_for_socket1;
                    taskid -= (local_ntasks/2);
                    starting_thread = nthreads/2;
                }
                else ERROR_PRINT();

    	        if (taskid < 0) ERROR_PRINT();
    	        if (taskid >= (local_ntasks/2)) 
                {
                    printf("taskid = %d ::: local_ntasks = %d\n", taskid, local_ntasks);
                    ERROR_PRINT();
                }

                local_ntasks = local_ntasks>>1;
            }
        }
        #endif

    
        int vertices_per_task = (total_number_of_neighbors % local_ntasks) ? (total_number_of_neighbors / local_ntasks + 1) : (total_number_of_neighbors / local_ntasks);
        
        int starting_index = taskid*vertices_per_task;
        int ending_index = starting_index + vertices_per_task;

        if (starting_index >= total_number_of_neighbors) starting_index = total_number_of_neighbors;
        if (ending_index > total_number_of_neighbors) ending_index = total_number_of_neighbors;

        if (starting_index < total_number_of_neighbors)
        {
            int vertices_so_far = 0;

            for(int thread_itr = starting_thread; thread_itr < ending_thread; thread_itr++)
            {
                if ( (vertices_so_far + NN[thread_itr][Count_B0[thread_itr]]) > starting_index) 
                {
                    *starting_thread_index = thread_itr;
                    if (vertices_so_far == starting_index)  { *starting_index_offset = 0; break;}
                    else
                    {
                        int j;
                        int running_sum = vertices_so_far;
                        for(j=0; j<Count_B0[thread_itr]; j++)
                        {
                            if ((running_sum + NN[thread_itr][j]) >= starting_index)
                            {
                                *starting_index_offset = j+1;
                                break;
                            }

                            running_sum += NN[thread_itr][j];
                        }
                        if (j == Count_B0[thread_itr]) ERROR_PRINT();
                        break;
                    }
                }

                vertices_so_far += NN[thread_itr][Count_B0[thread_itr]];
            }

            if ((*starting_thread_index) == -1) ERROR_PRINT();

            for(int thread_itr = (*starting_thread_index); thread_itr < ending_thread; thread_itr++)
            {
                if ( (vertices_so_far + NN[thread_itr][Count_B0[thread_itr]]) >= ending_index)
                {
                    *ending_thread_index = thread_itr;
                    int j;
                    int running_sum = vertices_so_far;
                    for(j=0; j<Count_B0[thread_itr]; j++)
                    {
                        if ((running_sum + NN[thread_itr][j]) > ending_index)
                        {
                            *ending_index_offset = j+1;
                            break;
                        }
                        
                        running_sum += NN[thread_itr][j];
                    }

                    if (j==Count_B0[thread_itr]) *ending_index_offset = Count_B0[thread_itr];
                    break;
                }

                vertices_so_far += NN[thread_itr][Count_B0[thread_itr]];
            }
        }
    }

#endif
#if 0
    *starting_thread_index = taskid;
    *ending_thread_index = taskid;
    *starting_index_offset = 0;
    *ending_index_offset = Count_B0[taskid];
    //printf("Taskid: %d ::::::::: starting_thread_index = %d ::: starting_index_offset = %d :::::: ending_thread_index = %d ::: ending_index_offset = %d\n", 
    printf("Taskid: %d :: Sum = %d :::: (%d,%d) --> (%d,%d)\n", 
    taskid, sum, *starting_thread_index, *starting_index_offset, *ending_thread_index, *ending_index_offset);
#endif

}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Function 7:
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

#ifdef MULTI_SOCKET

#if 0


void BFS_Parallel_I(long threadid, void *arg)
{
    int taskid = (int)((size_t)(arg));
    //if (threadid == 3) { printf("taskid = %d ::: threadid = %lld\n", taskid, threadid); }

    unsigned long long int task_start_time = read_tsc();
        
    int *BS1 = BS11[threadid];

    unsigned long long int task_end_time = read_tsc();
    //Timer[threadid] += (task_end_time - task_start_time);

    int socket_id = COMPUTE_SOCKET_ID(threadid);
    // We operate on [starting_index..ending_index)

    int starting_thread_index = -1;
    int starting_index_offset = -1;
    int ending_thread_index = -1;
    int ending_index_offset = -1;

    unsigned long long int overhead_start_time = read_tsc();

    Compute_Start_And_End_Offsets(taskid, Count_B0, ntasks, global_boundary_states, &starting_thread_index, &ending_thread_index, &starting_index_offset, &ending_index_offset);

    unsigned long long int overhead_end_time = read_tsc();
    Overhead_Timer[threadid] += (overhead_end_time - overhead_start_time);

    if ((starting_thread_index != -1) && (starting_index_offset != -1) && (ending_thread_index != -1) && (ending_index_offset != -1))
    {
        /////////////////////////////////////////////////////////////////
        //Step B: Now go over the neighbors and lookup neighbors and
        //compute depth...
        /////////////////////////////////////////////////////////////////

        for(int thread_itr = starting_thread_index; thread_itr <= ending_thread_index; thread_itr++)
        {
            int local_starting_index, local_ending_index;

            if (thread_itr == starting_thread_index) local_starting_index = starting_index_offset;
            else local_starting_index = 0;

            if (thread_itr == ending_thread_index) local_ending_index = ending_index_offset;
            else local_ending_index = Count_B0[thread_itr];

            int *BS0 = BS00[thread_itr];

            PERFORMING_BFS_I(taskid, threadid, global_depth, BS0, local_starting_index, local_ending_index, BS1, global_Depth); 
        }
    }
}
#endif

#if 0

void BFS_Parallel_II(long threadid, void *arg)
{
    int taskid = (int)((size_t)(arg));
    int *BS1 = BS11[threadid];

    int old_boundary_states = Count_B1[threadid];
    int new_boundary_states = Count_B1[threadid];

#ifdef TLB_BLOCKING
    int *Histgram = global_Histogram[threadid];
    for(int k=0; k<HIST_BINS; k++) Histgram[k] = 0;
#endif

    PERFORMING_BFS_II(taskid, threadid, global_depth, BS1, &new_boundary_states, global_Depth); 

    Count_B1[threadid] = new_boundary_states;

#ifdef TLB_BLOCKING
    Rearrange_for_Reduced_TLB_Misses(threadid, BS1, old_boundary_states, new_boundary_states);
#endif

}
#endif
#endif //MULTI_SOCKET is now defined...

                
int Please_Add_Offset(int **Values, int tid, int bin_id)
{
    int sum = 0;
    for(int k=0; k<(bin_id); k++) sum += Values[tid][k];
    return (sum);
}

void BFS_Parallel(long threadid, void *arg)
{
    int taskid = (int)((size_t)(arg));
    //if (threadid == 3) { printf("taskid = %d ::: threadid = %lld\n", taskid, threadid); }

    //if (taskid != threadid) ERROR_PRINT();
    unsigned long long int task_start_time = read_tsc();

    int **BS00, **BS11, *Count_B0, *Count_B1, **BS_Count_per_bin0, **BS_Count_per_bin1;

    if ((global_depth - MINIMUM_DEPTH)%2)
    {
        BS00 = Boundary_States0;
        BS11 = Boundary_States1;
        Count_B0 = Count_Boundary_States0;
        Count_B1 = Count_Boundary_States1;
        BS_Count_per_bin0 = global_BS_Count_per_bin0;
        BS_Count_per_bin1 = global_BS_Count_per_bin1;
    }
    else
    {
        BS00 = Boundary_States1;
        BS11 = Boundary_States0;
        Count_B0 = Count_Boundary_States1;
        Count_B1 = Count_Boundary_States0;
        BS_Count_per_bin0 = global_BS_Count_per_bin1;
        BS_Count_per_bin1 = global_BS_Count_per_bin0;
    }


    //if (threadid == 0) printf("---------------------------------------------------------------------- %d\n", __LINE__);
        
    int *BS1 = BS11[threadid];
    {
    #ifdef MULTI_SOCKET
        for(int k=0; k<ANOTHER_HIST_BINS; k++) global_Potential_Boundary_States[threadid*ANOTHER_HIST_BINS + k][0] = 0;
    #endif
    }

    unsigned long long int task_end_time = read_tsc();
    Timer[threadid] += (task_end_time - task_start_time);


#ifdef MULTI_SOCKET
    int socket_id = COMPUTE_SOCKET_ID(threadid);
#endif

   int starting_thread_index[ANOTHER_HIST_BINS]; //=-1
   int starting_index_offset[ANOTHER_HIST_BINS]; // = -1;
   int ending_thread_index[ANOTHER_HIST_BINS]; // = -1;
   int ending_index_offset[ANOTHER_HIST_BINS]; // = -1;

   int starting_bin_id;
   int ending_bin_id;
        
    unsigned long long int overhead_start_time = read_tsc();

    /////////////////////////////////////////////////////////////////
    //Step A: Compute the Bounds for each thread... All threads do
    //the execution below...
    /////////////////////////////////////////////////////////////////
    //Compute_Start_And_End_Offsets(taskid, Count_B0, ntasks, global_boundary_states, &starting_thread_index, &ending_thread_index, &starting_index_offset, &ending_index_offset);
    Compute_Start_And_End_Offsets_New(taskid, Count_B0, ntasks, global_boundary_states, starting_thread_index, ending_thread_index, starting_index_offset, ending_index_offset, &starting_bin_id, &ending_bin_id, BS_Count_per_bin0);

    unsigned long long int overhead_end_time = read_tsc();
    Overhead_Timer[threadid] += (overhead_end_time - overhead_start_time);

    //if(global_depth < 2) printf("Depth: %d thread: %d starting_bin_id: %d ending_bin_id: %d\n", global_depth, threadid, starting_bin_id, ending_bin_id);
    for(int bin_id=starting_bin_id; bin_id <= ending_bin_id; bin_id++)
    {
        // We operate on [starting_index..ending_index)

#if 0
    starting_thread_index = threadid; starting_index_offset = 0; ending_thread_index = threadid; ending_index_offset = local_Count_Boundary_States[threadid];
#endif
        if (threadid == 0)
        {
            //for(int i=0; i<nthreads; i++) printf("%d ", Count_B0[i]); printf("\n");
        }

        if (1)
        {
            /////////////////////////////////////////////////////////////////
            //Step B: Now go over the neighbors and lookup neighbors and
            //compute depth...
            /////////////////////////////////////////////////////////////////

        #ifndef MULTI_SOCKET
            int old_boundary_states = Count_B1[threadid];
            int new_boundary_states = old_boundary_states;
          #ifdef TLB_BLOCKING
            int *Hist = global_Histogram[threadid];
            RESET_HISTOGRAM(Hist);
          #endif
        #endif
            
        #ifdef MULTI_SOCKET
            int *DepthParent = (bin_id < (ANOTHER_HIST_BINS/2)) ? global_DepthParent_Socket0 : global_DepthParent_Socket1;
        #endif


            for(int thread_itr = starting_thread_index[bin_id]; thread_itr <= ending_thread_index[bin_id]; thread_itr++)
            {
            
                int local_starting_index, local_ending_index;

                if (thread_itr == starting_thread_index[bin_id]) local_starting_index = starting_index_offset[bin_id];
                else local_starting_index = 0;
                
                local_starting_index += Please_Add_Offset(BS_Count_per_bin0, thread_itr, bin_id);

                if (thread_itr == ending_thread_index[bin_id]) local_ending_index = ending_index_offset[bin_id];
                else local_ending_index = BS_Count_per_bin0[thread_itr][bin_id];

                local_ending_index += Please_Add_Offset(BS_Count_per_bin0, thread_itr, bin_id);

                int *BS0 = BS00[thread_itr];

            #ifdef MULTI_SOCKET
                PERFORMING_BFS_I(taskid, threadid, global_depth, BS0, local_starting_index, local_ending_index, BS1, DepthParent);
            #else
                Perform_BFS(taskid, threadid, global_depth, BS0, local_starting_index, local_ending_index, BS1, &new_boundary_states, global_Depth); 
            #endif
            }

       #ifndef MULTI_SOCKET
            Count_B1[threadid] = new_boundary_states;
            BS_Count_per_bin1[threadid][bin_id] = (new_boundary_states - old_boundary_states);

          #ifdef TLB_BLOCKING
            Rearrange_for_Reduced_TLB_Misses(threadid, BS1, old_boundary_states, new_boundary_states);
          #endif
      #endif
       ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }
    }

#ifdef MULTI_SOCKET


    if (0)
    {
        int **PBS = global_Potential_Boundary_States + (threadid * ANOTHER_HIST_BINS);
        //Further_Rearrange_PBS(threadid, PBS[0]+1, PBS[0][0]);
        //Further_Rearrange_PBS(threadid, PBS[1]+1, PBS[1][0]);
        //printf("Before: "); for(int i = 1; i <= PBS[0][0]; i++) printf("%d ", PBS[0][i]); printf("\n");
        qsort(PBS[0]+1, PBS[0][0], sizeof(int), Compare_Ints);
        //long long int uniq_count = 0;
        //for(int i = 1; i <= PBS[0][0]; i++) if(PBS[0][i] != PBS[0][i-1]) uniq_count++;
        //printf("uniq: %d out of %d\n", uniq_count, PBS[0][0]); 
        //total_uniq_count[threadid] += uniq_count; total_PBS_count_0[threadid] += PBS[0][0]; 
        //total_PBS_count_1[threadid] += PBS[1][0]; 
        qsort(PBS[1]+1, PBS[1][0], sizeof(int), Compare_Ints);
    }

    BARRIER(threadid);

#ifndef USE_LARGE_PAGE
    int sum_total = 0;
    int max_so_far = 0;
    int min_so_far = global_number_of_edges;

    for(int thr=0; thr < nthreads; thr++)
    {
        int **PBS = global_Potential_Boundary_States + (thr * ANOTHER_HIST_BINS);
        for(int j=0; j<ANOTHER_HIST_BINS; j++) 
        {
            sum_total += PBS[j][0];
            max_so_far = MAX(max_so_far, PBS[j][0]);
            min_so_far = MIN(min_so_far, PBS[j][0]);
        }
    }


    if (threadid == 0)
    {
        printf("min_so_far = %d ::: max_so_far = %d ::: sum_total = %d\n", min_so_far, max_so_far, sum_total);
    }
#endif

                
    int new_boundary_states = Count_B1[threadid];
    PERFORMING_BFS_II(taskid, threadid, global_depth, BS1, &new_boundary_states, BS_Count_per_bin1);  
    Count_B1[threadid] = new_boundary_states;

#endif
}


#ifdef USE_TASKQ
 

long long int total_uniq_count[MAX_THREADS] = {0};
long long int total_PBS_count_0[MAX_THREADS] = {0};
long long int total_PBS_count_1[MAX_THREADS] = {0};

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Function 8:
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#if 0
void ComputeBFS_TaskQ_Parallel(void)
{
     long dimensionSize[3], tileSize[3];
     dimensionSize[0] = ntasks;
     tileSize[0] = 1;

     unsigned long long int thread_start_time, thread_end_time;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// START OF COMPUTATION...
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    BS00 = Boundary_States0;
    BS11 = Boundary_States1;
    Count_B0 = Count_Boundary_States0;
    Count_B1 = Count_Boundary_States1;

    ////////////////////////////////////////////////////////////////////////////////////////
    //Step1: INITIALIZE... To be done by 1 thread...
    ////////////////////////////////////////////////////////////////////////////////////////

    for(int i=0; i<nthreads; i++)
    {
        Count_B0[i] = 0; // The other threads have no states in the boundary state...
        Count_B1[i] = 0; // The other threads have no states in the boundary state...
    }

        
#ifdef TLB_BLOCKING
    int *Hist = global_Histogram[0];
#endif

    Count_B0[0] = 1;
    int zero = 0;
    int threadid = 0;
    UPDATE_DEPTH_AND_BS(global_Depth, global_starting_vertex, NOT_ASSIGNED, MINIMUM_DEPTH, BS00[0], zero);

    ////////////////////////////////////////////////////////////////////////////////////////
    //Step2: Iterate over the boundary vertices until all vertices
    //have been assigned a depth.
    ////////////////////////////////////////////////////////////////////////////////////////
    
    global_boundary_states = 1;
    thread_start_time = read_tsc();

    for(global_depth=(1+MINIMUM_DEPTH); global_depth <= global_number_of_vertices; global_depth++)
    {

    #ifndef MULTI_SOCKET
        taskQEnqueueGrid((TaskQTask)(BFS_Parallel), 0, 1, dimensionSize, tileSize);
        taskQWait();
    #else 
        //MULTI_SOCKET is defined...
        for(int tid = 0; tid < nthreads; tid++) for(int k=0; k<ANOTHER_HIST_BINS; k++) global_Potential_Boundary_States[ tid * ANOTHER_HIST_BINS + k][0] = 0;

        taskQEnqueueGrid((TaskQTask)(BFS_Parallel_I), 0, 1, dimensionSize, tileSize);
        taskQWait();
#ifndef USE_LARGE_PAGE
    int sum_total = 0;
    int max_so_far = 0;
    int min_so_far = global_number_of_edges;

    for(int thr=0; thr < nthreads; thr++)
    {
        int **PBS = global_Potential_Boundary_States + (thr * ANOTHER_HIST_BINS);
        for(int j=0; j<ANOTHER_HIST_BINS; j++) 
        {
            sum_total += PBS[j][0];
            max_so_far = MAX(max_so_far, PBS[j][0]);
            min_so_far = MIN(min_so_far, PBS[j][0]);
        }
    }


    if (threadid == 0)
    {
        printf("min_so_far = %d ::: max_so_far = %d ::: sum_total = %d\n", min_so_far, max_so_far, sum_total);
    }
#endif
        taskQEnqueueGrid((TaskQTask)(BFS_Parallel_II), 0, 1, dimensionSize, tileSize);
        taskQWait();
    #endif

        //printf("\n");

#if 0
        //for(int thread_itr=0; thread_itr<nthreads; thread_itr++) for(int k=0; k<HIST_BINS; k++) global_Histogram[thread_itr][k] = 0;
        for(int thread_itr=0; thread_itr<nthreads; thread_itr++) { for(int k=0; k<=HIST_BINS; k++) printf(" %d ", global_Histogram[thread_itr][k]); printf("\n"); }
        for(int thread_itr=0; thread_itr<nthreads; thread_itr++) printf("%d %d\n", Count_B1[thread_itr], global_Histogram[thread_itr][HIST_BINS]);
        for(int thread_itr=0; thread_itr<nthreads; thread_itr++) 
        {
            if (Count_B1[thread_itr] != global_Histogram[thread_itr][HIST_BINS]) ERROR_PRINT();
        }
#endif

        global_boundary_states = 0;
        for(int thread_itr=0; thread_itr<nthreads; thread_itr++) 
        {
            global_boundary_states += Count_B1[thread_itr];
        }

        if (global_boundary_states == 0)
        {
            break;
        }

        ///////////////////////////////////////////////////
        //Swap BS00 and BS11
        //Swap Count_B0 and Count_B1
        ///////////////////////////////////////////////////

        int **BS_temp = BS00; BS00 = BS11; BS11 = BS_temp;
        int *Count_Btemp = Count_B0; Count_B0 = Count_B1; Count_B1 = Count_Btemp;

        for(int i=0; i<nthreads; i++) Count_B1[i] = 0; // The other threads have no states in the boundary state...
    }

    thread_end_time = read_tsc();
    {
        //printf("CK: For reference Timer[0] = %lld\n", Timer[0]);
        unsigned long long int total_time = thread_end_time - thread_start_time;
        double total_time_seconds = (total_time*1.0)/CORE_FREQUENCY;

        printf("\n*********************************************************************************************************\n");
        printf("Parallel Code (TaskQ)\n");
        printf("Time Taken = %lld cycles (%.3lf seconds) ::: Time Per Vertex = %.3lf cycles ::: Time Per Edge = %.3lf cycles ::: Average Degree of a Vertex = %.2lf\n", 
            total_time, total_time_seconds, (total_time*1.0)/global_number_of_vertices/1, (total_time*1.0)/global_number_of_edges, 
            (global_number_of_edges*1.0)/global_number_of_vertices);
        long long int actual_edges_traversed = 0; for(int i=0; i<global_number_of_vertices; i++) if (global_Depth[i] != NOT_ASSIGNED) actual_edges_traversed += GET_GLOBAL_ADJACENCY(i)[0]; 
        printf("Actual Edges Traversed = %lld\n", actual_edges_traversed);
PRINT_RED
        printf("Edges Per Second = %.3lf Million/sec\n", (actual_edges_traversed*1)/(total_time_seconds*1000*1000));
PRINT_BLACK
        printf("*********************************************************************************************************\n");
    }
}
#endif

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Function 9:
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void *ComputeBFS_TaskQ_Pthread_Parallel(void *arg1)
{
    int threadid = (int)((size_t)(arg1));
    unsigned long long int thread_start_time, thread_end_time;
    long long int taskid = threadid;

    Set_Affinity(threadid);
    //int local_Count_Boundary_States[MAX_THREADS];

    BARRIER(threadid);

    thread_start_time = read_tsc();

    int **BS00, **BS11, *Count_B0, *Count_B1, **BS_Count_per_bin0, **BS_Count_per_bin1;
    BS00 = Boundary_States0;
    BS11 = Boundary_States1;
    Count_B0 = Count_Boundary_States0;
    Count_B1 = Count_Boundary_States1;
    BS_Count_per_bin0 = global_BS_Count_per_bin0;
    BS_Count_per_bin1 = global_BS_Count_per_bin1;

            
    Count_B0[threadid] = 0; // Resetting the number of boundary states...
    Count_B1[threadid] = 0; // Resetting the number of boundary states...
    for(int j=0; j<ANOTHER_HIST_BINS; j++) BS_Count_per_bin0[threadid][j] = 0;
    for(int j=0; j<ANOTHER_HIST_BINS; j++) BS_Count_per_bin1[threadid][j] = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// START OF COMPUTATION...
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////
    //Step1: INITIALIZE... To be done by 1 thread...
    ////////////////////////////////////////////////////////////////////////////////////////

    if (threadid == 0)
    {

#ifdef TLB_BLOCKING
        int *Hist = global_Histogram[0];
#endif

        int bin_for_vertex = COMPUTE_BIN_ID(global_starting_vertex);

        BS_Count_per_bin0[0][bin_for_vertex]= 1;  
        Count_B0[0] = 1;
        global_boundary_states = 1;
        int zero = 0;

#ifdef MULTI_SOCKET
    #ifdef BYTE_LEVEL
        unsigned char *local_Assigned;
    #else
        unsigned  int *local_Assigned;
    #endif


    #ifdef BLOOM_FILTER
        if (global_starting_vertex < global_vertices_for_socket0)   local_Assigned = global_Assigned_Socket0;
        else                                                        local_Assigned = global_Assigned_Socket1;
    #endif

        int *DepthParent = (bin_for_vertex == 0) ? global_DepthParent_Socket0 : global_DepthParent_Socket1;
        UPDATE_DEPTHPARENT_AND_BS_Socket(DepthParent, global_starting_vertex, NOT_ASSIGNED, MINIMUM_DEPTH, BS00[0], zero, local_Assigned, global_starting_vertex);
#else
        UPDATE_DEPTHPARENT_AND_BS(global_Depth, global_starting_vertex, NOT_ASSIGNED, MINIMUM_DEPTH, BS00[0], zero, global_starting_vertex);
#endif
        global_depth = MINIMUM_DEPTH+1;
    }

    BARRIER(threadid);

    ////////////////////////////////////////////////////////////////////////////////////////
    //Step2: Iterate over the boundary vertices until all vertices
    //have been assigned a depth.
    ////////////////////////////////////////////////////////////////////////////////////////
    

    for(int depth=(1+MINIMUM_DEPTH); depth <= global_number_of_vertices; depth++)
    {
       //if(threadid == 0) printf("******************************************\n");
       //if(threadid == 0) printf("Beginning of depth: %d\n", global_depth);

        BFS_Parallel(threadid, (void *)(taskid));

        BARRIER(threadid);

        // Book keeping for the next step...

        unsigned long long int barrier_start_time, barrier_end_time;
        barrier_start_time = read_tsc();

        int local_boundary_states = 0; 
            
        for(int thread_itr=0; thread_itr<nthreads; thread_itr++)  local_boundary_states +=  Count_B1[thread_itr];
        global_boundary_states = local_boundary_states;
        //if(threadid == 0) printf("End of depth: %d ::: boundary_states: %lld\n", global_depth, global_boundary_states);
        //if(threadid == 0) printf("******************************************\n");
        if (0 && (threadid == 0))
        {
            int *Ptr = GET_GLOBAL_ADJACENCY(global_starting_vertex);
            FILE *gp = fopen("rt.txt", "w");
            for(int j=1; j<=Ptr[0]; j++)
            {
                fprintf(gp, "%d\n", Ptr[j]);
            }
            fclose(gp);
            exit(123);
        }

        if (0 && (threadid == 0))
        {
            FILE *gp = fopen("rrr.txt", "a");
            int lines_written = 0;
            for(int thread_itr=0; thread_itr<nthreads; thread_itr++)  
            {
                for(int j=0; j< Count_B1[thread_itr]; j++)
                {
                    lines_written++;
                    fprintf(gp, "AAA %d ::: %d\n", depth,  BS11[thread_itr][j]);
                }
            }
            if(threadid == 0) printf("depth: %d lines_written: %d\n", global_depth, lines_written);
            fclose(gp);
        }

        if (0 && (threadid == 0))
        {
            int found_error = 0;
            for(int thread_itr=0; thread_itr<nthreads; thread_itr++)
            {
                if ( (BS_Count_per_bin1[thread_itr][0] + BS_Count_per_bin1[thread_itr][1]) != Count_B1[thread_itr]) 
                {
                    found_error++;
                    printf("tid: %d ::: ", thread_itr); 
                    printf("%d + %d != %d\n", BS_Count_per_bin1[thread_itr][0], BS_Count_per_bin1[thread_itr][1], Count_B1[thread_itr]);
                }
            }

            if (found_error) ERROR_PRINT();
        }
        //printf("threadid: %d ::: local_boundary_states = %d\n", threadid, local_boundary_states);

        global_depth = (depth+1);

        if (!local_boundary_states) break;

            
        int **BS_temp = BS00; BS00 = BS11; BS11 = BS_temp;
        int *Count_Btemp = Count_B0; Count_B0 = Count_B1; Count_B1 = Count_Btemp;
        int **BS_Count_temp = BS_Count_per_bin0; BS_Count_per_bin0 = BS_Count_per_bin1; BS_Count_per_bin1 = BS_Count_temp;

        Count_B1[threadid] = 0; //Resetting the boundary states for the next 'step'...
        for(int j=0; j<ANOTHER_HIST_BINS; j++) BS_Count_per_bin1[threadid][j] = 0;

        barrier_end_time = read_tsc();
        Timer[threadid] += (barrier_end_time - barrier_start_time);
    }

#if 0
        if (threadid == 0)
        {
            global_boundary_states = 0;
            for(int thread_itr=0; thread_itr<nthreads; thread_itr++) 
            {
                global_boundary_states += Count_B1[thread_itr];
            }

            global_depth++;

            ///////////////////////////////////////////////////
            //Swap BS00 and BS11
            //Swap Count_B0 and Count_B1
            ///////////////////////////////////////////////////

            for(int i=0; i<nthreads; i++) Count_B1[i] = 0; // The other threads have no states in the boundary state...
        }

        BARRIER(threadid);
        barrier_end_time = read_tsc();
        Timer[threadid] += (barrier_end_time - barrier_start_time);

        if (!global_boundary_states) break;
#endif

    BARRIER(threadid);

    thread_end_time = read_tsc();

#if 1
    if (threadid == 0)
    {
        unsigned long long int total_time = thread_end_time - thread_start_time;
        double total_time_seconds = (total_time*1.0)/CORE_FREQUENCY;

        printf("\n*********************************************************************************************************\n");
        printf("Parallel Code (TaskQ but actually Pthread)\n");
        printf("Time Taken = %lld cycles (%.3lf seconds) ::: Time Per Vertex = %.3lf cycles ::: Time Per Edge = %.3lf cycles ::: Average Degree of a Vertex = %.2lf\n", 
            total_time, total_time_seconds, (total_time*1.0)/global_number_of_vertices/1, (total_time*1.0)/global_number_of_edges, 
            (global_number_of_edges*1.0)/global_number_of_vertices);

        long long int actual_edges_traversed = 0; 
     #ifndef MULTI_SOCKET
        for(int i=0; i<global_number_of_vertices; i++) if (global_DepthParent[2*i] != NOT_ASSIGNED) actual_edges_traversed += GET_GLOBAL_ADJACENCY(i)[0]; 
     #else
        for(int i=0; i<global_vertices_for_socket0; i++) if (global_DepthParent_Socket0[2*i] != NOT_ASSIGNED) actual_edges_traversed += GET_GLOBAL_ADJACENCY(i)[0]; 
        for(int i=global_vertices_for_socket0; i<global_number_of_vertices; i++) if (global_DepthParent_Socket1[2*i] != NOT_ASSIGNED) actual_edges_traversed += GET_GLOBAL_ADJACENCY(i)[0]; 
     #endif

        printf("Actual Edges Traversed = %lld\n", actual_edges_traversed);
PRINT_RED    
        printf("Edges Per Second = %.3lf Million/sec\n", (actual_edges_traversed*1)/(total_time_seconds*1000*1000));
PRINT_BLACK   
        printf("*********************************************************************************************************\n");

        global_number_of_traversed_edges = actual_edges_traversed;
        long long int tot_uniq = 0, tot_PBS_0 = 0, tot_PBS_1 = 0;
        for(int i= 0; i < MAX_THREADS; i++) { tot_uniq += total_uniq_count[i]; tot_PBS_0 +=  (total_PBS_count_0[i] ); tot_PBS_1 +=  (total_PBS_count_1[i] ); }
        printf("DEBUG: total_uniq_count for PBS[0]: %lld out of %lld , %lld\n", tot_uniq, tot_PBS_0, tot_PBS_1); 
    }
#endif

    return NULL;
}
#endif //USE_TASKQ


#ifndef USE_TASKQ

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Function 10:
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#if 0
void *ComputeBFS_Pthread_Parallel(void *arg1)
{
    int threadid = (int)((size_t)(arg1));
    unsigned long long int thread_start_time, thread_end_time;
    long long int taskid = threadid;

    //int local_Count_Boundary_States[MAX_THREADS];

    Set_Affinity(threadid);

    BARRIER(threadid);

    thread_start_time = read_tsc();


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// START OF COMPUTATION...
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////
    //Step1: INITIALIZE... To be done by 1 thread...
    ////////////////////////////////////////////////////////////////////////////////////////

    if (threadid == 0)
    {
        BS00 = Boundary_States0;
        BS11 = Boundary_States1;
        Count_B0 = Count_Boundary_States0;
        Count_B1 = Count_Boundary_States1;
    
        for(int i=0; i<nthreads; i++)
        {
            Count_B0[i] = 0; // The other threads have no states in the boundary state...
            Count_B1[i] = 0; // The other threads have no states in the boundary state...
            Number_of_Neighbors0[i][0] = 0;
        }

    #ifdef TLB_BLOCKING
        int *Hist = global_Histogram[0];
    #endif

        Count_B0[0] = 1;
        global_boundary_states = 1;
        int zero = 0;
        int *NN = Number_of_Neighbors0[threadid];
        NN[1] = NN[0] = GET_GLOBAL_ADJACENCY(global_starting_vertex)[0];
        UPDATE_DEPTHPARENT_AND_BS(global_Depth, global_starting_vertex, NOT_ASSIGNED, MINIMUM_DEPTH, BS00[0], zero, global_starting_vertex);
        global_depth = MINIMUM_DEPTH+1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    //Step2: Iterate over the boundary vertices until all vertices
    //have been assigned a depth.
    ////////////////////////////////////////////////////////////////////////////////////////
    
    BARRIER(threadid);

    for(int depth=(1+MINIMUM_DEPTH); depth <= global_number_of_vertices; depth++)
    {
        BFS_Parallel(threadid, (void *)(taskid));
        int *NN = Number_of_Neighbors1[threadid];
        int *BS1 = BS11[threadid];
        int sum = 0;

        //int sum = 0; for(int i=0; i<Count_B1[threadid]; i++) sum += Number_of_Neighbors1[threadid][i]; Number_of_Neighbors1[threadid][Count_B1[threadid]] = sum;

        int number_of_elements = Count_B1[threadid];
        for(int i=0; i<number_of_elements; i++)
        {
            _mm_prefetch((char *)(GET_GLOBAL_ADJACENCY(BS1[i+2])),  _MM_HINT_T0);
	        int value =  GET_GLOBAL_ADJACENCY(BS1[i])[0];
            sum += value;
            NN[i] = value;
        }

        NN[number_of_elements] = sum;

    
        BARRIER(threadid);

        if (threadid == 0)
        {
            global_boundary_states = 0;
            for(int thread_itr=0; thread_itr<nthreads; thread_itr++) 
            {
                global_boundary_states += Count_B1[thread_itr];
            }

            global_depth++;

            ///////////////////////////////////////////////////
            //Swap BS00 and BS11
            //Swap Count_B0 and Count_B1
            ///////////////////////////////////////////////////

            int **BS_temp = BS00; BS00 = BS11; BS11 = BS_temp;
            int **NN_temp = Number_of_Neighbors0; Number_of_Neighbors0 = Number_of_Neighbors1; Number_of_Neighbors1 = NN_temp;
            int *Count_Btemp = Count_B0; Count_B0 = Count_B1; Count_B1 = Count_Btemp;
            for(int i=0; i<nthreads; i++) Count_B1[i] = 0; // The other threads have no states in the boundary state...
            for(int i=0; i<nthreads; i++) Number_of_Neighbors1[i][0] = 0; 
        }

        BARRIER(threadid);
        if (!global_boundary_states) break;
    }

        
    BARRIER(threadid);
    thread_end_time = read_tsc();

    if (threadid == 0)
    {
        unsigned long long int total_time = thread_end_time - thread_start_time;
        double total_time_seconds = (total_time*1.0)/CORE_FREQUENCY;

        printf("\n*********************************************************************************************************\n");
        printf("Parallel Code (Pthread)\n");
        printf("Time Taken = %lld cycles (%.3lf seconds) ::: Time Per Vertex = %.3lf cycles ::: Time Per Edge = %.3lf cycles ::: Average Degree of a Vertex = %.2lf\n", 
            total_time, total_time_seconds, (total_time*1.0)/global_number_of_vertices/1, (total_time*1.0)/global_number_of_edges, 
            (global_number_of_edges*1.0)/global_number_of_vertices);
        long long int actual_edges_traversed = 0; for(int i=0; i<global_number_of_vertices; i++) if (global_Depth[i] != NOT_ASSIGNED) actual_edges_traversed += GET_GLOBAL_ADJACENCY(i)[0]; 
        printf("Actual Edges Traversed = %lld\n", actual_edges_traversed);
PRINT_RED
        printf("Edges Per Second = %.3lf Million/sec\n", (actual_edges_traversed*1)/(total_time_seconds*1000*1000));
PRINT_BLACK
        printf("*********************************************************************************************************\n");

    }

    return NULL;
}
#endif
#endif //NOT_USING_TASKQ


int main(int argc, char **argv)
{
    if (argc != 6)
    {
        printf("Usage ./a.out <input_graph_file> <starting_vertex> <nthreads> <nsockets> <ntasks_per_thread>\n");
        exit(123);
    }

    ParseParams(argv);

    Initialize_BFS(argv);

    ComputeBFS_serial(0, Serial_Code_DepthParent);
    RESET_GLOBAL_ASSIGNED();
    for(int i=0; i<nthreads; i++) Timer[i] = 0;

    //Spawn the Parallel Code...

#if 1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef USE_TASKQ

#if 0
     pthread_barrier_init(&mybarrier, NULL, nthreads);
     for(int i=1; i<nthreads; i++) pthread_create(&threads[i], &attr, ComputeBFS_Pthread_Parallel, (void *)i);
     ComputeBFS_Pthread_Parallel(0);
     for(int i=1; i<nthreads; i++) pthread_join(threads[i], NULL);
#endif

#else
     if (ntasks == nthreads)
     {
         pthread_barrier_init(&mybarrier, NULL, nthreads);
         for(int i=1; i<nthreads; i++) pthread_create(&threads[i], &attr, ComputeBFS_TaskQ_Pthread_Parallel, (void *)i);
         ComputeBFS_TaskQ_Pthread_Parallel(0);
         for(int i=1; i<nthreads; i++) pthread_join(threads[i], NULL);
     }
     else
     {
         //ComputeBFS_TaskQ_Parallel();
     }
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
    Report_Per_Phase_Performance();

    Checking(Serial_Code_DepthParent);

    Finalize();


#ifdef MULTI_SOCKET
    //int another_sum = 0; for(int i=0; i<64; i++) another_sum += BookKeeping[i]; printf("another_sum = %d\n", another_sum);
#endif

}
