
//Comments
//1. BYTE_LEVEL and BIT_LEVEL both use global_Assigned to store the bloom filter...
//2. SUB_BIT_LEVEL uses global_Assigned and global_sub_bit...

#define large_malloc      my_malloc
#define large_malloc_mmap my_malloc_mmap

size_t global_small_memory_alloted = 0;
size_t global_small_memory_alloted_0 = 0;
size_t global_small_memory_alloted_1 = 0;
size_t global_max_pbs_entries;
size_t global_max_tmp_entries;

void *small_malloc(size_t whatever)
{
    global_small_memory_alloted += whatever;
    return malloc(whatever);
}

void small_free(void *R, size_t size_in_bytes)
{
    global_small_memory_alloted -= size_in_bytes;
    free(R);
}


void *my_socket_large_malloc(size_t whatever, int socket_id)
{
#ifdef NUMA_AWARENESS
    return (large_malloc_mmap(whatever, socket_id));
#else
    return (large_malloc(whatever));
#endif
}

void *my_socket_small_malloc(size_t whatever, int socket_id)
{
#ifdef NUMA_AWARENESS
    if (socket_id == 1) global_small_memory_alloted_1 += whatever;
    if (socket_id == 0) global_small_memory_alloted_0 += whatever;
    return(numa_alloc_onnode(whatever, socket_id));
#else
    return (small_malloc(whatever));
#endif
}


#define COMPUTE_PAGES(x) ( ((x) % page_size) ? ((x)/ page_size)+1 : ((x)/page_size))
#define SMALL_MALLOC 123
#define LARGE_MALLOC 124
#define SMALL_SOCKET_MALLOC 125
#define LARGE_SOCKET_MALLOC 126

int global_number_of_comments = 0;
typedef struct COMMEN
{
    char comm[64];
    size_t sz;
    int type;
}COMMENT;

COMMENT Comments[48];

int Compare_Comments(const void *a, const void *b)
{
    COMMENT *ia = (COMMENT *)(a);
    COMMENT *ib = (COMMENT *)(b);

    if (ia->type != ib->type) return (ia->type - ib->type);
    return (ia->sz - ib->sz);
}

void Comment_Add(char *tcl_string, size_t sz, int type)
{

    Comments[global_number_of_comments].sz = sz;
    Comments[global_number_of_comments].type = type;
    sprintf(Comments[global_number_of_comments].comm, "%s", tcl_string);

    global_number_of_comments++;
}


void Comments_Print(void)
{
    qsort(Comments, global_number_of_comments, sizeof(COMMENT), Compare_Comments);
    printf("\n##### Memory Allocation Patterns ##########################################################################\n");
    size_t page_size = 2*1024*1024;
    for(int i=0; i<global_number_of_comments; i++)
    {
        printf("[%35s] :: ", Comments[i].comm);
        if (Comments[i].type == SMALL_MALLOC)           printf("Small Malloc       ");
        if (Comments[i].type == LARGE_MALLOC)           printf("Large Malloc       ");
        if (Comments[i].type == SMALL_SOCKET_MALLOC)    printf("Small Socket Malloc");
        if (Comments[i].type == LARGE_SOCKET_MALLOC)    printf("Large Socket Malloc");

        printf(" of size (%8.2lf MB) ::: %4lld Pages\n", Comments[i].sz/1024.0/1024.0, COMPUTE_PAGES(Comments[i].sz));
    }
    printf("#############################################################################################################\n\n");
}


int Compare_Ints(const void *a, const void *b)
{
    int *ia = (int *)(a);
    int *ib = (int *)(b);

    return ((*ia)-(*ib));
}

void RESET_HISTOGRAM(int *Histo)
{
    for(int k=0; k<HIST_BINS; k++) Histo[k] = 0;
}

void RESET_GLOBAL_ASSIGNED(void)
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef BYTE_LEVEL
    for(int i=0; i<bloom_indices; i++) global_Assigned[i] = 0;
#ifdef MULTI_SOCKET
    for(int i=0; i<bloom_indices_socket0; i++) global_Assigned_Socket0[i] = 0;
    for(int i=bloom_indices_socket0; i<(bloom_indices_socket1+bloom_indices_socket0); i++) global_Assigned_Socket1[i] = 0;
#endif
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef BIT_LEVEL
    for(int i=0; i<bloom_indices; i++) global_Assigned[i] = 0;
#ifdef MULTI_SOCKET
    for(int i=0; i<bloom_indices_socket0; i++) global_Assigned_Socket0[i] = 0;
    for(int i=bloom_indices_socket0; i<(bloom_indices_socket1+bloom_indices_socket0); i++) global_Assigned_Socket1[i] = 0;
#endif
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

void ParseParams (char **argv)
{
#ifdef NUMA_AWARENESS
    if (numa_available()==-1) 
    {
      printf("[-] numa_available() failed!\n");
      ERROR_PRINT();
    }
#endif

/////////////////////////////////////////////////////////////////////////////////////
    sscanf(argv[3], "%d", &nthreads);
    if (nthreads < 1) nthreads = 1;
    if (nthreads > MAX_THREADS) ERROR_PRINT();
    printf("nthreads = %d\n", nthreads);

/////////////////////////////////////////////////////////////////////////////////////

    sscanf(argv[4], "%d", &nsockets);
    if (nsockets < 1) nsockets = 1;
    if (nsockets > 2) nsockets = 2;
    printf("nsockets = %d\n", nsockets);

#ifdef MULTI_SOCKET
    if (nthreads <= CORES_PER_SOCKET)  { if (nsockets != 1) { printf("nthreads (%d) is <= (%d), and nsockets > 1\n", nthreads, CORES_PER_SOCKET); ERROR_PRINT();}}
    if (nthreads > 2*CORES_PER_SOCKET) { if (nsockets != 2) { printf("nthreads (%d) is > (%d), and nsockets ==  1\n", nthreads, 2*CORES_PER_SOCKET); ERROR_PRINT();}}
#else
    //if (nsockets != 1) ERROR_PRINT();
#endif
/////////////////////////////////////////////////////////////////////////////////////

    sscanf(argv[5], "%d", &ntasks_per_thread);
#ifndef USE_TASKQ
    ntasks_per_thread = 1;
#ifdef SIMD
    //printf("SIMD is not supported when TASKQ is not enabled\n");
    //ERROR_PRINT();
#endif
#endif
    ntasks = ntasks_per_thread * nthreads;

    printf("ntasks_per_thread = %d ::: ntasks = %d\n", ntasks_per_thread, ntasks);

    if (ntasks > MAX_TASKS_IN_TASKQ)
    {
        printf("ntasks = %d ::: MAX_TASKS_IN_TASKQ = %d\n", ntasks, MAX_TASKS_IN_TASKQ);
        ERROR_PRINT();
    }
}

void Sort_Neighbors(void)
{
    return;
    
    printf("Sorting Neighbors..."); fflush(stdout);
    for(int i=0; i<global_number_of_vertices; i++)
    {
        if (GET_GLOBAL_ADJACENCY(i)[0])
        {
            qsort(GET_GLOBAL_ADJACENCY(i)+1, GET_GLOBAL_ADJACENCY(i)[0], sizeof(int), Compare_Ints);
        }
    }
    printf("...\n");
}


void Assign_global_vertices_for_socket0(void)
{
#ifdef MULTI_SOCKET
    if (nsockets == 1)
    {
        global_vertices_for_socket0 = global_number_of_vertices;
    }
    else
    {
        if (nsockets != 2) ERROR_PRINT();
        global_vertices_for_socket0 = global_number_of_vertices/nsockets;
        //global_vertices_for_socket0 *= 0.76;

        if (0)
        {
            size_t TT[64];
            TT[0] = 1; TT[1] = 2; TT[2] = 4; TT[3] = 8; TT[4] = 16;
            for(int k=5; k<64; k++) TT[k] = TT[k-1] * (size_t)(2);

            for(int k=0; k<63; k++) 
            {
                if ( (TT[k] < global_vertices_for_socket0) && (global_vertices_for_socket0 <= TT[k+1])) 
                { 
                    global_vertices_for_socket0 = TT[k+1]; 
                    log_vertices_socket0 = (k+1);
                    printf("log_vertices_socket0 = %d\n", log_vertices_socket0);
                    break;
                }
            }
        }

        if (global_vertices_for_socket0 % 512) 
        {
            //ERROR_PRINT();
            global_vertices_for_socket0 = (global_vertices_for_socket0/512+1)*512;
        }
    }
#endif
}
    
void Allocate_global_Assigned(void)
{

#ifdef SIMD
    printf("SIMD should not be defined with BLOOM_FILTER being defined...\n");
    ERROR_PRINT();
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef BYTE_LEVEL
    bloom_indices = global_number_of_vertices;
    printf("bloom_indices = %d ::: global_number_of_vertices = %d\n", bloom_indices, global_number_of_vertices);
    printf("Bloom Filter Size (Bytes) = %d ::: global_number_of_vertices = %d\n", bloom_indices, global_number_of_vertices);
    if (nsockets == 1)
    {
        global_Assigned = (unsigned char *)my_socket_large_malloc(bloom_indices * sizeof(unsigned char), 0);
        Comment_Add("global_Assigned", (bloom_indices * sizeof(unsigned char)), LARGE_SOCKET_MALLOC);
    }
    else
    {
        global_Assigned = (unsigned char *)large_malloc(bloom_indices * sizeof(unsigned char));
        Comment_Add("global_Assigned", (bloom_indices * sizeof(unsigned char)), LARGE_MALLOC);
    }
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef BIT_LEVEL
    bloom_indices = (global_number_of_vertices % 32) ? (global_number_of_vertices/32 + 1) : global_number_of_vertices/32;
    printf("bloom_indices = %d ::: global_number_of_vertices = %d\n", bloom_indices, global_number_of_vertices);
    printf("Bloom Filter Size = %.2lf MB ::: global_number_of_vertices = %d\n", (bloom_indices*(int)(sizeof(unsigned int)))/1000.0/1000.0, global_number_of_vertices);
    if (nsockets == 1)
    {
        global_Assigned = (unsigned int *)my_socket_large_malloc((bloom_indices) * sizeof(unsigned int), 0);
        Comment_Add("global_Assigned", (bloom_indices * sizeof(unsigned int)), LARGE_SOCKET_MALLOC);
    }
    else
    {
        global_Assigned = (unsigned int *)large_malloc((bloom_indices) * sizeof(unsigned int));
        Comment_Add("global_Assigned", (bloom_indices * sizeof(unsigned int)), LARGE_MALLOC);
    }
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef MULTI_SOCKET

    #ifdef BYTE_LEVEL
            
        bloom_indices_socket0 =  global_vertices_for_socket0;
        bloom_indices_socket1 = bloom_indices - bloom_indices_socket0;

        global_Assigned_Socket0 = (unsigned char *)my_socket_large_malloc((bloom_indices_socket0 * sizeof(unsigned char)), 0);
        global_Assigned_Socket1 = (unsigned char *)my_socket_large_malloc((bloom_indices_socket1 * sizeof(unsigned char)), 1);
        global_Assigned_Socket1 -= bloom_indices_socket0;

        printf("bloom_indices_socket0 = %d ::: bloom_indices_socket1 = %d\n",  bloom_indices_socket0, bloom_indices_socket1);
        printf("global_vertices_for_socket0 = %d ::: global_number_of_vertices = %d ::: Percentage on Socket0 = %.2lf%%\n", 
                global_vertices_for_socket0, global_number_of_vertices, (global_vertices_for_socket0*100.0)/global_number_of_vertices);

    
        Comment_Add("global_Assigned_Socket0", ((bloom_indices_socket0 * sizeof(unsigned char))), LARGE_SOCKET_MALLOC);
        Comment_Add("global_Assigned_Socket1", ((bloom_indices_socket1 * sizeof(unsigned char))), LARGE_SOCKET_MALLOC);
    #endif

    #ifdef BIT_LEVEL
            
        bloom_indices_socket0 =  global_vertices_for_socket0/32;
        bloom_indices_socket1 = bloom_indices - bloom_indices_socket0;

        global_Assigned_Socket0 = (unsigned int *)my_socket_large_malloc((bloom_indices_socket0 * sizeof(unsigned int)), 0);
        global_Assigned_Socket1 = (unsigned int *)my_socket_large_malloc((bloom_indices_socket1 * sizeof(unsigned int)), 1);
        global_Assigned_Socket1 -= bloom_indices_socket0;

        printf("bloom_indices_socket0 = %d ::: bloom_indices_socket1 = %d\n",  bloom_indices_socket0, bloom_indices_socket1);
        printf("global_vertices_for_socket0 = %d ::: global_number_of_vertices = %d ::: Percentage on Socket0 = %.2lf%%\n", 
                global_vertices_for_socket0, global_number_of_vertices, (global_vertices_for_socket0*100.0)/global_number_of_vertices);

    
        Comment_Add("global_Assigned_Socket0", ((bloom_indices_socket0 * sizeof(unsigned int))), LARGE_SOCKET_MALLOC);
        Comment_Add("global_Assigned_Socket1", ((bloom_indices_socket1 * sizeof(unsigned int))), LARGE_SOCKET_MALLOC);
    #endif

#endif

    RESET_GLOBAL_ASSIGNED();
}

void Assign_Quantiles(void)
{
#ifdef MULTI_SOCKET
    int global_vertices_for_socket1 = global_number_of_vertices - global_vertices_for_socket0;
    int vertices_per_bucket0 = (global_vertices_for_socket0 % (ANOTHER_HIST_BINS/2)) ? (global_vertices_for_socket0/(ANOTHER_HIST_BINS/2) + 1) : (global_vertices_for_socket0/(ANOTHER_HIST_BINS/2));
    int vertices_per_bucket1 = (global_vertices_for_socket1 % (ANOTHER_HIST_BINS/2)) ? (global_vertices_for_socket1/(ANOTHER_HIST_BINS/2) + 1) : (global_vertices_for_socket1/(ANOTHER_HIST_BINS/2));

    for(int i=0; i<(ANOTHER_HIST_BINS/2); i++) Quantiles[i] = i*vertices_per_bucket0; 
    for(int i=0; i<(ANOTHER_HIST_BINS/2); i++) Quantiles[i + (ANOTHER_HIST_BINS/2)] = global_vertices_for_socket0  + i*vertices_per_bucket1; 

    //if (threadid == 0) { for(int i=0; i<ANOTHER_HIST_BINS; i++) printf(" %10d ", Quantiles[i]); printf("\n"); } 

#else
    int vertices_per_bucket = (global_number_of_vertices % ANOTHER_HIST_BINS) ? (global_number_of_vertices/ANOTHER_HIST_BINS+1) : global_number_of_vertices/ANOTHER_HIST_BINS;
    for(int i=0; i<ANOTHER_HIST_BINS; i++) Quantiles[i] = i*vertices_per_bucket; 
#endif

    Quantiles[ANOTHER_HIST_BINS+0] = global_number_of_vertices;
    Quantiles[ANOTHER_HIST_BINS+1] = global_number_of_vertices;
    Quantiles[ANOTHER_HIST_BINS+2] = global_number_of_vertices;
    Quantiles[ANOTHER_HIST_BINS+3] = global_number_of_vertices;

    printf("Assigned Quantiles\n");
}


void Allocate_Boundary_States(void)
{

//#define MAX_BOUNDARY_STATES (2*1024*1024)
//#define MAX_BOUNDARY_STATES (55*1023*1024 + 64)
#define MAX_BOUNDARY_STATES (150*1023*1024 + 64)

#if 0
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Boundary_States0 = (int **)small_malloc((GROUPS+2) * nthreads * sizeof(int *));
    Boundary_States1 = (int **)small_malloc((GROUPS+2) * nthreads * sizeof(int *));
    for(int i=0; i<((GROUPS+2) * nthreads); i++)
    {
        Boundary_States0[i] = (int *)small_malloc(MAX_BOUNDARY_STATES * sizeof(int));
        Boundary_States1[i] = (int *)small_malloc(MAX_BOUNDARY_STATES * sizeof(int));

        if (!Boundary_States0[i]) ERROR_PRINT();
        if (!Boundary_States1[i]) ERROR_PRINT();
    }

    Boundary_Neighbors0 = (int **)small_malloc(nthreads * sizeof(int *));
    for(int i=0; i<(nthreads); i++)
    {
        Boundary_Neighbors0[i] = (int *)small_malloc(MAX_BOUNDARY_STATES * sizeof(int));
        if (!Boundary_Neighbors0[i]) ERROR_PRINT();
    }

#ifndef USE_TASKQ
    Number_of_Neighbors0 = (int **)small_malloc((GROUPS+2) * nthreads * sizeof(int *));
    Number_of_Neighbors1 = (int **)small_malloc((GROUPS+2) * nthreads * sizeof(int *));
    for(int i=0; i<((GROUPS+2) * nthreads); i++)
    {
        Number_of_Neighbors0[i] = (int *)small_malloc(MAX_BOUNDARY_STATES * sizeof(int));
        Number_of_Neighbors1[i] = (int *)small_malloc(MAX_BOUNDARY_STATES * sizeof(int));
        if (!Number_of_Neighbors0[i]) ERROR_PRINT();
        if (!Number_of_Neighbors1[i]) ERROR_PRINT();
    }
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#else //if 1
    size_t total_malloced_size_0 = 0;
    size_t total_malloced_size_1 = 0;

    int Maximum_Size[MAX_THREADS]; 
    Maximum_Size[0] = MAX_BOUNDARY_STATES; 
    for(int i=1; i<nthreads; i++) Maximum_Size[i] = (MAX_BOUNDARY_STATES/nthreads) + 16;

    int bins_per_thread = 1;

    Boundary_States0 = (int **)small_malloc(nthreads * bins_per_thread * sizeof(int *));
    Boundary_States1 = (int **)small_malloc(nthreads * bins_per_thread * sizeof(int *));

    int indexxx   = 0;

    for(int i=0; i<(nthreads); i++)
    {
        size_t size_to_malloc = (size_t)(Maximum_Size[i]) * (size_t)(sizeof(int));
        int socket_id = COMPUTE_SOCKET_ID(i);

        for(int j=0; j<bins_per_thread; j++, indexxx++)
        {
            Boundary_States0[indexxx] = (int *)my_socket_small_malloc(size_to_malloc, socket_id);
            Boundary_States1[indexxx] = (int *)my_socket_small_malloc(size_to_malloc, socket_id);
            if (!Boundary_States0[indexxx]) ERROR_PRINT();
            if (!Boundary_States1[indexxx]) ERROR_PRINT();
            total_malloced_size_0 += size_to_malloc;
            total_malloced_size_1 += size_to_malloc;
        }
    }

    Comment_Add("Boundary_States0", total_malloced_size_0, SMALL_SOCKET_MALLOC);
    Comment_Add("Boundary_States1", total_malloced_size_1, SMALL_SOCKET_MALLOC);

#ifdef USE_BS_BINNING

#define MAX_BINNED_BOUNDARY_STATES (100 * 1001)

    global_Binned_Boundary_States = (int **)small_malloc(nthreads * BS_BINS * sizeof(int *));
    int odd_threads = 0;
    int even_threads = 0;
    for(int i=0; i<nthreads; i++)
    {
        int socket_id = COMPUTE_SOCKET_ID(i);
        if (socket_id % 2) odd_threads ++;
        else even_threads++;
    }

    //if (even_threads != odd_threads) ERROR_PRINT();
        
    size_t size_to_malloc_socket0 = (size_t)(MAX_BINNED_BOUNDARY_STATES) * (size_t)(sizeof(int) * (size_t)(even_threads) * (size_t)(BS_BINS));
    size_t size_to_malloc_socket1 = (size_t)(MAX_BINNED_BOUNDARY_STATES) * (size_t)(sizeof(int) * (size_t)(odd_threads)  * (size_t)(BS_BINS));

    int *t0 = (int *)my_socket_large_malloc(size_to_malloc_socket0, 0);
    int *t1 = (int *)my_socket_large_malloc(size_to_malloc_socket1, 1);


    for(int i=0; i<nthreads; i++)
    {
        int socket_id = COMPUTE_SOCKET_ID(i);
        if (socket_id % 2)
        {
            //odd thread
            for(int j=0; j<BS_BINS; j++)
            {
                global_Binned_Boundary_States[i*BS_BINS + j] = t1;
                t1 += MAX_BINNED_BOUNDARY_STATES;
            }
        }
        else
        {
            //even thread
            for(int j=0; j<BS_BINS; j++)
            {
                global_Binned_Boundary_States[i*BS_BINS + j] = t0;
                t0 += MAX_BINNED_BOUNDARY_STATES;
            }
        }
    }

    Comment_Add("Binned_Boundary_States0", size_to_malloc_socket0, LARGE_SOCKET_MALLOC);
    Comment_Add("Binned_Boundary_States1", size_to_malloc_socket1, LARGE_SOCKET_MALLOC);

#endif


#ifndef USE_TASKQ
    total_malloced_size_0 = 0;
    total_malloced_size_1 = 0;

    //Comment: Even if BS_BINS > 1, we only need one Number_of_Neighbors0/1 since only the last BIN's neighbors is not known :)

    Number_of_Neighbors0 = (int **)small_malloc(nthreads * sizeof(int *));
    Number_of_Neighbors1 = (int **)small_malloc(nthreads * sizeof(int *));

    for(int i=0; i<nthreads; i++)
    {
        size_t size_to_malloc = (size_t)(Maximum_Size[i]) * (size_t)(sizeof(int));
        int socket_id = COMPUTE_SOCKET_ID(i);

        Number_of_Neighbors0[i] = (int *)my_socket_small_malloc(size_to_malloc, socket_id);
        Number_of_Neighbors1[i] = (int *)my_socket_small_malloc(size_to_malloc, socket_id);

        if (!Number_of_Neighbors0[i]) ERROR_PRINT();
        if (!Number_of_Neighbors1[i]) ERROR_PRINT();

        total_malloced_size_0 += size_to_malloc;
        total_malloced_size_1 += size_to_malloc;
    }

    Comment_Add("Number_of_Neighbors0", total_malloced_size_0, SMALL_SOCKET_MALLOC);
    Comment_Add("Number_of_Neighbors1", total_malloced_size_1, SMALL_SOCKET_MALLOC);
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif

}

#ifdef TLB_BLOCKING
void Allocate_Histogram(void)
{
    size_t sz = 0;
    global_Histogram = (int **)small_malloc(nthreads * sizeof(int *));
    for(int k=0; k<nthreads; k++)
    {
        size_t local_size = (HIST_BINS+16)*sizeof(int);
        int socket_id = COMPUTE_SOCKET_ID(k);
        global_Histogram[k] = (int *)my_socket_small_malloc(local_size, socket_id);
        sz += local_size;
    }

    Comment_Add("global_Histogram", sz, SMALL_SOCKET_MALLOC);
}

    
void Allocate_Temporary_Array(void)
{
    size_t sz = 0;
    Temporary_Array = (int **)small_malloc(nthreads * sizeof(int *));
    for(int k=0; k<nthreads; k++)
    {
        //size_t max_entries_per_thread = global_number_of_vertices/nthreads + 121; //200000000; //MAX_BOUNDARY_STATES/1 + 16;
        size_t max_entries_per_thread = global_number_of_edges/nthreads + 121; //200000000; //MAX_BOUNDARY_STATES/1 + 16;
        global_max_tmp_entries = max_entries_per_thread;
        int socket_id = COMPUTE_SOCKET_ID(k);
        size_t local_size = (size_t)(max_entries_per_thread) * size_t(sizeof(int));

        Temporary_Array[k] = (int *)my_socket_large_malloc(local_size, socket_id);

        sz += local_size;
    }

    Comment_Add("Temporary_Array", sz, LARGE_SOCKET_MALLOC);
}
#endif

#ifdef MULTI_SOCKET
void Allocate_PBS(void)
{
    long long int max_entries_per_array;
    global_Potential_Boundary_States = (int **)small_malloc(nthreads * ANOTHER_HIST_BINS * sizeof(int *));

    max_entries_per_array = 44.9 * 1000 * 1000;
    max_entries_per_array = 9.9 * 1000 * 1000;
    max_entries_per_array = 2 * 1000 * 1000;
    max_entries_per_array = 12*1000*1000;
    max_entries_per_array = (1.7*global_number_of_edges/nthreads)/3 + 121;
    max_entries_per_array = (1.7*global_number_of_edges/nthreads)/1 + 121;
    max_entries_per_array = (1.5*global_number_of_edges/nthreads)/1 + 121;

#if ANOTHER_HIST_BINS!=2
    max_entries_per_array = (2.1*global_number_of_edges/nthreads)/4 + 121;
#endif

    global_max_pbs_entries  = max_entries_per_array;
    printf("global_max_pbs_entries = %lld\n", global_max_pbs_entries);

    size_t sz = max_entries_per_array * (long long int)(sizeof(int));
    size_t total_sz = 0;

    for(int k1=0; k1<nthreads; k1++)
    {
        int socket_id = COMPUTE_SOCKET_ID(k1);
        for(int k2=0; k2<ANOTHER_HIST_BINS; k2++)
        {
            //global_Potential_Boundary_States[k1*ANOTHER_HIST_BINS+k2] = (int *)my_socket_small_malloc(sz, socket_id);
            global_Potential_Boundary_States[k1*ANOTHER_HIST_BINS+k2] = (int *)my_socket_large_malloc(sz, socket_id);
            total_sz += sz;
        }
    }

    Comment_Add("global_Potential_Boundary_States", total_sz, LARGE_SOCKET_MALLOC);
}
#endif

void Allocate_DepthParent_Arrays(void)
{
    size_t sz = (size_t)(global_number_of_vertices) * (size_t)(2*sizeof(int));

    Serial_Code_DepthParent = (int *)my_socket_small_malloc(sz, 0);
    Comment_Add("Serial_Code_DepthParent", sz, SMALL_SOCKET_MALLOC);


#ifndef MULTI_SOCKET
    if (nsockets == 1)
    {
        global_DepthParent =      (int *)my_socket_large_malloc(sz, 0);
        Comment_Add("global_DepthParent", sz, LARGE_SOCKET_MALLOC);
    }
    else
    {
        global_DepthParent =      (int *)large_malloc(sz);
        Comment_Add("global_DepthParent", sz, LARGE_MALLOC);
    }

    for(int i=0; i<global_number_of_vertices; i++) { global_DepthParent[2*i] = NOT_ASSIGNED; global_DepthParent[2*i+1] = NOT_ASSIGNED;}
#else

    int global_vertices_for_socket1 = global_number_of_vertices - global_vertices_for_socket0;

    size_t size_to_malloc_socket0 = (size_t)(global_vertices_for_socket0) * (size_t)(sizeof(int) * (size_t)(2));
    size_t size_to_malloc_socket1 = (size_t)(global_vertices_for_socket1) * (size_t)(sizeof(int) * (size_t)(2));

    global_DepthParent_Socket0   = (int *)my_socket_large_malloc(size_to_malloc_socket0, 0);
    global_DepthParent_Socket1   = (int *)my_socket_large_malloc(size_to_malloc_socket1, 1);
    global_DepthParent_Socket1  -= (2*global_vertices_for_socket0);

    Comment_Add("global_DepthParent_Socket0", size_to_malloc_socket0, LARGE_SOCKET_MALLOC);
    Comment_Add("global_DepthParent_Socket1", size_to_malloc_socket1, LARGE_SOCKET_MALLOC);

    for(int i=0; i<global_vertices_for_socket0; i++)                         { global_DepthParent_Socket0[2*i] = NOT_ASSIGNED; global_DepthParent_Socket0[2*i+1] = NOT_ASSIGNED;}
    for(int i=global_vertices_for_socket0; i<global_number_of_vertices; i++) { global_DepthParent_Socket1[2*i] = NOT_ASSIGNED; global_DepthParent_Socket1[2*i+1] = NOT_ASSIGNED;}
#endif
}

void Allocate_BS_Count_per_bin(void)
{
    global_BS_Count_per_bin0 = (int **)small_malloc(nthreads * sizeof(int *));
    global_BS_Count_per_bin1 = (int **)small_malloc(nthreads * sizeof(int *));

    int mem_required_socket0 = 0;
    int mem_required_socket1 = 0;

    for(int i=0; i<nthreads; i++)
    {
        int socket_id = COMPUTE_SOCKET_ID(i);
        if (socket_id == 0) mem_required_socket0 += (2*ANOTHER_HIST_BINS * sizeof(int));
        else mem_required_socket1 += (2*ANOTHER_HIST_BINS * sizeof(int));
    }

    int *temp_memory0 = (int *)my_socket_small_malloc(mem_required_socket0, 0);
    int *temp_memory1 = (int *)my_socket_small_malloc(mem_required_socket1, 1);

    for(int i=0; i<nthreads; i++)
    {
        int socket_id = COMPUTE_SOCKET_ID(i);
        if (socket_id == 0) 
        {
            global_BS_Count_per_bin0[i] = temp_memory0; temp_memory0 += (ANOTHER_HIST_BINS);
            global_BS_Count_per_bin1[i] = temp_memory0; temp_memory0 += (ANOTHER_HIST_BINS);
        }
        else
        {
            global_BS_Count_per_bin0[i] = temp_memory1; temp_memory1 += (ANOTHER_HIST_BINS);
            global_BS_Count_per_bin1[i] = temp_memory1; temp_memory1 += (ANOTHER_HIST_BINS);
        }
    }

    int sz = mem_required_socket0 + mem_required_socket0;
        
    Comment_Add("global_BS_Count_per_bin", sz, LARGE_SOCKET_MALLOC);

}

void Init_BFS(void)
{
    printf("NOT_ASSIGNED = %d\n", NOT_ASSIGNED);

//#ifdef NUMA_AWARENESS
//#ifndef BIT_LEVEL
//    printf("NUMA_AWARENESS is only supported when BIT_LEVEL is ENABLED\n");
//    ERROR_PRINT();
//#endif
//#endif

#ifdef MULTI_SOCKET
    Assign_global_vertices_for_socket0();
    Allocate_PBS();
#endif

    Allocate_BS_Count_per_bin();

    Allocate_DepthParent_Arrays();

    Allocate_Boundary_States();


#ifdef BLOOM_FILTER
    Allocate_global_Assigned();
#endif

#ifdef TLB_BLOCKING
    Allocate_Histogram();
    Allocate_Temporary_Array();
#endif

    Assign_Quantiles();
}

void ParseGraph(char *filename)
{
#define TYPE double
    int *Count;
    long long int edges_found = 0;
    TYPE pos_x, pos_y;
    int id, id1, id2;
    int max_number_of_neighbors = 0;
    int min_number_of_neighbors = 999999999;

    int binary_file = 1;

    char X;
    FILE *fp;
    fp = fopen(filename, "rb");
    fread(&X, sizeof(char), 1, fp); if (X != 'P') binary_file = 0;
    fread(&X, sizeof(char), 1, fp); if (X != 'C') binary_file = 0;
    fread(&X, sizeof(char), 1, fp); if (X != 'L') binary_file = 0;

    fread(&X, sizeof(char), 1, fp); if (X != '9') binary_file = 0;
    fread(&X, sizeof(char), 1, fp); if (X != '5') binary_file = 0;
    fread(&X, sizeof(char), 1, fp); if (X != '1') binary_file = 0;
    fread(&X, sizeof(char), 1, fp); if (X != '2') binary_file = 0;
    fread(&X, sizeof(char), 1, fp); if (X != '3') binary_file = 0;

    if (binary_file)
    {
        long long int number_of_integers = 0;
        long long int number_of_integers_for_socket0 = 0;
        long long int number_of_integers_for_socket1 = 0;

        fread(&global_number_of_vertices, sizeof(int), 1, fp);
        fread(&global_number_of_edges, sizeof(long long int), 1, fp);
        fread(&number_of_integers, sizeof(long long int), 1, fp);

        printf("global_number_of_vertices = %d ::: global_number_of_edges = %lld\n", global_number_of_vertices, global_number_of_edges);

	    // call Init BFS now to ensure that global_vertices_for_socket0 is set correctly
	    Init_BFS();

        Count = (int *)small_malloc(global_number_of_vertices * sizeof(int));
        for(int i=0; i<global_number_of_vertices; i++) Count[i] = 0;


    #ifdef MULTI_SOCKET
        int *temp_memory_2 = (int *)small_malloc(sizeof(int)*global_number_of_vertices);
        for(size_t i=0; i<global_vertices_for_socket0; i++)
        {
            int neighbors;
	        fread(&neighbors, 1, sizeof(int), fp);
	        number_of_integers_for_socket0 += (neighbors+1); // 1 for the number of neighbors
	        // dummy read to advance file pointer
	        fread(temp_memory_2, neighbors, sizeof(int), fp);	    
        }
	    small_free(temp_memory_2, sizeof(int)*global_number_of_vertices);

        printf("number_of_integers for socket0 = %lld ::: number_of_integers = %lld :: Percentage on socket0 = %.2lf%%\n", 
	       number_of_integers_for_socket0, number_of_integers, (number_of_integers_for_socket0*100.0/number_of_integers) );

	    number_of_integers_for_socket1 = (number_of_integers - number_of_integers_for_socket0);

        fseek(fp, 28, SEEK_SET); // reset fp to the point where the non NUMA_AWARE code would be

        global_Adjacency_Socket0 = (int **) my_socket_large_malloc(global_vertices_for_socket0 * sizeof(int *), 0);
        global_Adjacency_Socket1 = (int **) my_socket_large_malloc((global_number_of_vertices - global_vertices_for_socket0) * sizeof(int *), 1);
        global_Adjacency_Socket1 -= global_vertices_for_socket0;


	    size_t total_memory_required_for_socket0 = (size_t)(number_of_integers_for_socket0) * (size_t)(sizeof(int));
	    size_t total_memory_required_for_socket1 = (size_t)(number_of_integers_for_socket1) * (size_t)(sizeof(int));

        int *temp_memory_socket0 = (int *)my_socket_large_malloc(total_memory_required_for_socket0, 0);
        int *temp_memory_socket1 = (int *)my_socket_large_malloc(total_memory_required_for_socket1, 1);

        Comment_Add("global_Adjacency_Socket0", total_memory_required_for_socket0 + global_vertices_for_socket0 * sizeof(int *), LARGE_SOCKET_MALLOC);
        Comment_Add("global_Adjacency_Socket1", total_memory_required_for_socket1 + (global_number_of_vertices - global_vertices_for_socket0) * sizeof(int *), LARGE_SOCKET_MALLOC);

        fread(temp_memory_socket0, number_of_integers_for_socket0, sizeof(int), fp);	
        fread(temp_memory_socket1, number_of_integers_for_socket1, sizeof(int), fp);	


        long long int sum_of_neighbors = 0;
        long long int sum_of_neighbors_socket0 = 0;
        long long int sum_of_neighbors_socket1 = 0;

        for(size_t i=0; i<global_vertices_for_socket0; i++)
        {
            global_Adjacency_Socket0[i] = temp_memory_socket0;
            temp_memory_socket0 += (1+global_Adjacency_Socket0[i][0]);
            sum_of_neighbors_socket0 += global_Adjacency_Socket0[i][0];
            sum_of_neighbors += global_Adjacency_Socket0[i][0];
            Count[global_Adjacency_Socket0[i][0]]++;
            if (global_Adjacency_Socket0[i][0] > max_number_of_neighbors) max_number_of_neighbors = global_Adjacency_Socket0[i][0];
            if (global_Adjacency_Socket0[i][0] < min_number_of_neighbors) min_number_of_neighbors = global_Adjacency_Socket0[i][0];
        }
        if ( (temp_memory_socket0 - global_Adjacency_Socket0[0]) != number_of_integers_for_socket0) ERROR_PRINT();

        for(size_t i=global_vertices_for_socket0; i< global_number_of_vertices; i++)
        {
            global_Adjacency_Socket1[i] = temp_memory_socket1;
            temp_memory_socket1 += (1+global_Adjacency_Socket1[i][0]);
            sum_of_neighbors_socket1 += global_Adjacency_Socket1[i][0];
            sum_of_neighbors += global_Adjacency_Socket1[i][0];
            Count[global_Adjacency_Socket1[i][0]]++;
            if (global_Adjacency_Socket1[i][0] > max_number_of_neighbors) max_number_of_neighbors = global_Adjacency_Socket1[i][0];
            if (global_Adjacency_Socket1[i][0] < min_number_of_neighbors) min_number_of_neighbors = global_Adjacency_Socket1[i][0];
        }

        if ( sum_of_neighbors != global_number_of_edges) 
        {
            printf("sum_of_neighbors = %lld ::: global_number_of_edges = %lld\n", sum_of_neighbors, global_number_of_edges);
            ERROR_PRINT();
        }

        if(number_of_integers_for_socket1) if ( (temp_memory_socket1 - global_Adjacency_Socket1[global_vertices_for_socket0]) != number_of_integers_for_socket1) ERROR_PRINT();

    #else	

        size_t sz = (size_t)(global_number_of_vertices) * (size_t)(sizeof(int *));
        size_t total_amount_of_memory_required = (size_t)(number_of_integers) * sizeof(int);
       
        int *temp_memory;
        if( nsockets == 1)
        {
            global_Adjacency = (int **)my_socket_large_malloc(sz,0);
            temp_memory = (int *)my_socket_large_malloc(total_amount_of_memory_required, 0);
            Comment_Add("global_Adjacency", total_amount_of_memory_required + sz, LARGE_SOCKET_MALLOC);
        }
        else
        {
            global_Adjacency = (int **)large_malloc(sz);
            temp_memory = (int *)large_malloc(total_amount_of_memory_required);
            Comment_Add("global_Adjacency", total_amount_of_memory_required + sz, LARGE_MALLOC);
        }
        fread(temp_memory, number_of_integers, sizeof(int), fp);


        long long int sum_of_neighbors = 0;
        for(size_t i=0; i<global_number_of_vertices; i++)
        {
            global_Adjacency[i] = temp_memory;
            temp_memory += (1+global_Adjacency[i][0]);
            sum_of_neighbors += global_Adjacency[i][0];
            Count[global_Adjacency[i][0]]++;
            if (global_Adjacency[i][0] > max_number_of_neighbors) max_number_of_neighbors = global_Adjacency[i][0];
            if (global_Adjacency[i][0] < min_number_of_neighbors) min_number_of_neighbors = global_Adjacency[i][0];
        }

        if (sum_of_neighbors != global_number_of_edges) 
        {
            printf("sum_of_neighbors = %lld ::: global_number_of_edges = %lld\n", sum_of_neighbors, global_number_of_edges);
            ERROR_PRINT();
        }
        if ( (temp_memory - global_Adjacency[0]) != number_of_integers) ERROR_PRINT();
    #endif
        fclose(fp);
    }

    else
    {
        //ASCII format...
        printf("Not supported for now...\n"); 
        fclose(fp);
        ERROR_PRINT();
    }

    global_max_neighbors = max_number_of_neighbors;
    global_min_neighbors = min_number_of_neighbors;

    int vertex_with_max_neighbors = 0;

#ifdef MULTI_SOCKET
        
    size_t i;
    for(i=0; i<global_vertices_for_socket0; i++)
    {
        if (global_Adjacency_Socket0[i][0] == global_max_neighbors)   
        {
            vertex_with_max_neighbors = i;
            break;
        }
    }

    if (i == global_vertices_for_socket0)
    {
        for(size_t i=global_vertices_for_socket0; i<global_number_of_vertices; i++)
        {
            if (global_Adjacency_Socket1[i][0] == global_max_neighbors)   
            {
                vertex_with_max_neighbors = i;
                break;
            }
        }
    }

#else
    for(size_t i=0; i<global_number_of_vertices; i++)
    {
        if (global_Adjacency[i][0] == global_max_neighbors)   
        {
            vertex_with_max_neighbors = i;
            break;
        }
    }
#endif
    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    printf("Parsed (%s) successfully \nglobal_number_of_vertices = %d (%.2lf K) ::: global_number_of_edges = %lld (%.2lf K)\n\n", 
            filename, global_number_of_vertices, global_number_of_vertices/1000.0, global_number_of_edges, global_number_of_edges/1000.0);
    int k;
    int total_count = 0;
    long long int cummulative_count_of_edges = 0;

#ifdef USE_BS_BINNING
    int groups = BS_BINS;
#else
    int groups = 5;
#endif

    for(k=0; k<groups; k++)
    {
        total_count += Count[k];
        cummulative_count_of_edges += k*Count[k];
        printf("Number of vertices with %d neighbors = %d (%.2lf %%) [%.2lf %%]\n",  
                k, Count[k], (Count[k]*100.0)/global_number_of_vertices, (cummulative_count_of_edges*100.0)/global_number_of_edges);
    }
    printf("Number of vertices with < %d neighbors = %d (%.2lf %%)\n",  k, total_count, (total_count*100.0)/global_number_of_vertices);
    printf("Maximum Number of Neighbors for any (%d) vertex = %d\n", vertex_with_max_neighbors, max_number_of_neighbors);

    printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    small_free(Count, (global_number_of_vertices * sizeof(int)));
    Comments_Print();
}

void Initialize_BFS(char **argv)
{

#ifdef USE_TASKQ
    taskQInit(nthreads, MAX_TASKS_IN_TASKQ);
#endif

#ifdef USE_LARGE_PAGE
    Init_Large_Memory();
#ifdef NUMA_AWARENESS
    Init_Large_Memory_mmap();
#endif
#endif

/////////////////////////////////////////////////////////////////////////////////////
    ParseGraph(argv[1]); // <-- call Init_BFS() in here 
/////////////////////////////////////////////////////////////////////////////////////

    sscanf(argv[2], "%d", &global_starting_vertex);
    printf("global_starting_vertex = %d ::: After Changing ", global_starting_vertex);
    if (global_starting_vertex < 0) global_starting_vertex = -global_starting_vertex;
    global_starting_vertex = global_starting_vertex%global_number_of_vertices;
    printf("global_starting_vertex = %d\n", global_starting_vertex);
/////////////////////////////////////////////////////////////////////////////////////


    int page_size = 2*1024*1024; //2MB page...
    
    printf("-------------------------------------------------------------\n");
    printf("NUMA small memory (onnode) ::: socket 0 = %lld bytes (%.3lf MB) ::: socket1 = %lld bytes (%.3lf MB)\n", 
            global_small_memory_alloted_0, global_small_memory_alloted_0/1024.0/1024.0, global_small_memory_alloted_1, global_small_memory_alloted_1/1024.0/1024.0);
    printf("Amount of SMALL MEMORY USED = %lld bytes (%.3lf MB)\n",  global_small_memory_alloted, global_small_memory_alloted/1024.0/1024.0);
    printf("NUMA large memory (mmap) ::: socket 0 = %lld pages ::: socket 1 = %lld pages\n", COMPUTE_PAGES(global_large_memory_alloted_0), COMPUTE_PAGES(global_large_memory_alloted_1));
    printf("Regular large memory (shmget)  = %lld pages\n", COMPUTE_PAGES(global_large_memory_alloted));
    printf("-------------------------------------------------------------\n");
    printf("HIST_BINS = %d ::: RIGHT_SHIFT = %d\n", HIST_BINS, RIGHT_SHIFT);

    {
        int log_hist_bins = 1; int hist_bins = 2; while (hist_bins < HIST_BINS) { log_hist_bins ++; hist_bins *= 2;}
        int bits_for_vertices = 10; int vertices = 1024; while (vertices < global_number_of_vertices) { vertices *= 2; bits_for_vertices++; }
        //int log_number_of_pages = 2; int pages = 4; while (pages < number_of_large_pages) { pages *= 2; log_number_of_pages++; }
        //int right_shift_should_be =  log_number_of_pages - log_hist_bins; 

        int right_shift_should_be =  bits_for_vertices - log_hist_bins; 
        if (RIGHT_SHIFT != right_shift_should_be)
        {
            printf("PLEASE CHANGE RIGHT_SHIFT (%d --> %d)\n", RIGHT_SHIFT, right_shift_should_be);
            ERROR_PRINT();
        }
    }

    printf("ANOTHER_HIST_BINS = %d ::: ANOTHER_RIGHT_SHIFT = %d\n", ANOTHER_HIST_BINS, ANOTHER_RIGHT_SHIFT);
    if (ANOTHER_HIST_BINS == 2)
    {
        int log_hist_bins = 1; int hist_bins = 2; while (hist_bins < ANOTHER_HIST_BINS) { log_hist_bins ++; hist_bins *= 2;}
        int bits_for_vertices = 10; int vertices = 1024; while (vertices < global_number_of_vertices) { vertices *= 2; bits_for_vertices++; }

        int right_shift_should_be =  bits_for_vertices - log_hist_bins; 
        if (ANOTHER_RIGHT_SHIFT != right_shift_should_be)
        {
            printf("PLEASE CHANGE ANOTHER_RIGHT_SHIFT (%d --> %d)\n", ANOTHER_RIGHT_SHIFT, right_shift_should_be);
            ERROR_PRINT();
        }
    }


    for(int i=0; i<nthreads; i++) for(int j=0; j<DEBUG_BINS; j++) Timer4[i][j] = 0;
    for(int i=0; i<nthreads; i++) for(int j=0; j<DEBUG_BINS; j++) Timer5[i][j] = 0;
    for(int i=0; i<nthreads; i++) Timer[i] = 0;
    for(int i=0; i<nthreads; i++) Overhead_Timer[i] = 0;
    Sort_Neighbors();
}

void Finalize(void)
{
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("CORE_FREQUENCY = %.2lf GHz\n", CORE_FREQUENCY/1000.0/1000.0/1000.0);
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EVENLY_DIVIDE_BS
    printf("BS is LOAD BALANCED\n");
#else
    printf("BS is NOT LOAD BALANCED\n");
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EVENLY_DIVIDE_PBS
    printf("PBS is LOAD BALANCED\n");
#else
    printf("PBS is NOT LOAD BALANCED\n");
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef TLB_BLOCKING
    printf("USING TLB_BLOCKING\n");
#else
    printf("NOT USING TLB_BLOCKING\n");
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_BS_BINNING
    printf("USING BS_BINNING (with %d bins)\n", BS_BINS);
#else
    printf("NOT USING BS_BINNING\n");
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_LARGE_PAGE
    printf("USING LARGE PAGES\n");
#else
    printf("NOT USING LARGE PAGES\n");
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_TASKQ
    printf("Using TaskQ's\n");
#else
    printf("NOT Using TaskQ's\n");
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_LARGE_PAGE
    my_free_large();
#ifdef NUMA_AWARENESS
    Release_Large_Memory_mmap();
#endif
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef BLOOM_FILTER

#ifdef BYTE_LEVEL
    printf("BLOOM_FILTER is ENABLED... BYTE_LEVEL is ENABLED\n");
#endif
#ifdef BIT_LEVEL
    printf("BLOOM_FILTER is ENABLED... BIT_LEVEL is ENABLED\n");
#endif
#ifdef SUB_BIT_LEVEL
    printf("BLOOM_FILTER is ENABLED... SUB_BIT_LEVEL is ENABLED\n");
    printf("PER_BIT = %d ::: LOG_PER_BIT = %d ::: BIT_MASK = %d ::: TWO_POWER_BITS = %d\n", PER_BIT, LOG_PER_BIT, BIT_MASK, TWO_POWER_BITS);
#endif
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef BLOOM_FILTER
    printf("NOT USING ANY BLOOM_FILTER\n");
#endif
////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SIMPLE_SCALAR
    printf("Using SIMPLE_SCALAR\n");
#endif
#ifdef UNROLLED_SCALAR
    printf("Using UNROLLED_SCALAR\n");
#endif
#ifdef SIMD
    printf("Using SIMD\n");
#endif

#ifdef MULTI_SOCKET
    printf("MULTI_SOCKET code is ENABLED, with CORES_PER_SOCKET = %d\n", CORES_PER_SOCKET);
#else
    printf("MULTI_SOCKET code is NOT ENABLED\n");
#endif

#ifdef NUMA_AWARENESS
    printf("NUMA_AWARENESS is ENABLED\n");
#else
    printf("NUMA_AWARENESS is NOT ENABLED\n");
#endif

    printf("global_number_of_vertices = %.2lf M ::: global_number_of_edges = %.2lf M ::: Average Degree = %.2lf\n", 
            (global_number_of_vertices/1000.0/1000.0), (global_number_of_edges/1000.0/1000.0), (global_number_of_edges*1.0)/global_number_of_vertices);
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}


void Checking(int *Local_DepthParent) 
{
    printf("global_wasted_time = %lld cycles (%.2lf seconds)\n", global_wasted_time, (global_wasted_time*1.0)/CORE_FREQUENCY);
    int erroneous_vertices = 0;
    int erroneous_parents = 0;
    int not_assigned_vertices = 0;
    long long int sum = 0;
    int prev_not_assigned = -1;
    int max_diff_between_not_assigned = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MULTI_SOCKET

    for(int i=0; i<global_number_of_vertices; i++)
    {
        sum += global_Depth[2*i];
        if (global_DepthParent[2*i] != Local_DepthParent[2*i]) 
        {
            erroneous_vertices++;
            //printf("Vertex Id: %d ::: global_Depth = %d ::: Local_Depth = %d\n", i, global_Depth[i],  Local_Depth[i]);
        }
        if (global_DepthParent[2*i+1] != Local_DepthParent[2*i+1])  erroneous_parents++;

        if (global_DepthParent[2*i] == NOT_ASSIGNED) 
        {
            not_assigned_vertices++;
            if ( (i-prev_not_assigned) > max_diff_between_not_assigned)  max_diff_between_not_assigned = i-prev_not_assigned;
            prev_not_assigned = i;
        }
    }
#else

    for(int i=0; i<global_vertices_for_socket0; i++)
    {
        sum += global_DepthParent_Socket0[2*i];
        if (global_DepthParent_Socket0[2*i] != Local_DepthParent[2*i]) 
        {
            erroneous_vertices++;
            //printf("Vertex Id: %d ::: global_Depth = %d ::: Local_Depth = %d\n", i, global_Depth[i],  Local_Depth[i]);
        }

        if (global_DepthParent_Socket0[2*i+1] != Local_DepthParent[2*i+1])  
        {
            if (global_DepthParent_Socket0[2*i+1] < 0) ERROR_PRINT();
            int parent = global_DepthParent_Socket0[2*i+1];
            int depth_of_parent = (parent < global_vertices_for_socket0) ? global_DepthParent_Socket0[2*parent] : global_DepthParent_Socket1[2*parent];
            int depth_right = Local_DepthParent[2*(Local_DepthParent[2*i+1])];

            if (depth_right != depth_of_parent) erroneous_parents++;

            int j; int *Adj = GET_GLOBAL_ADJACENCY(parent); for(j=1; j<=Adj[0]; j++) if (Adj[j] == i) break; if (j > Adj[0]) ERROR_PRINT();
        }

        if (global_DepthParent_Socket0[2*i] == NOT_ASSIGNED) 
        {
            not_assigned_vertices++;
            if ( (i-prev_not_assigned) > max_diff_between_not_assigned)  max_diff_between_not_assigned = i-prev_not_assigned;
            prev_not_assigned = i;
        }
    }

    for(int i=global_vertices_for_socket0; i<global_number_of_vertices; i++)
    {
        sum += global_DepthParent_Socket1[2*i];
        if (global_DepthParent_Socket1[2*i] != Local_DepthParent[2*i]) 
        {
            erroneous_vertices++;
            //printf("Vertex Id: %d ::: global_Depth = %d ::: Local_Depth = %d\n", i, global_Depth[i],  Local_Depth[i]);
        }

        if (global_DepthParent_Socket1[2*i+1] != Local_DepthParent[2*i+1])  
        {
            if (global_DepthParent_Socket1[2*i+1] < 0) ERROR_PRINT();
            int parent = global_DepthParent_Socket1[2*i+1];
            int depth_of_parent = (parent < global_vertices_for_socket0) ? global_DepthParent_Socket0[2*parent] : global_DepthParent_Socket1[2*parent];
            int depth_right = Local_DepthParent[2*(Local_DepthParent[2*i+1])];

            if (depth_right != depth_of_parent) erroneous_parents++;
            int j; int *Adj = GET_GLOBAL_ADJACENCY(parent); for(j=1; j<=Adj[0]; j++) if (Adj[j] == i) break; if (j > Adj[0]) ERROR_PRINT();
        }


        if (global_DepthParent_Socket1[2*i] == NOT_ASSIGNED) 
        {
            not_assigned_vertices++;
            if ( (i-prev_not_assigned) > max_diff_between_not_assigned)  max_diff_between_not_assigned = i-prev_not_assigned;
            prev_not_assigned = i;
        }
    }

#if 1
    for(int i=0; i<global_vertices_for_socket0; i++)
    {
        int *Adj = GET_GLOBAL_ADJACENCY(i);
        int current_depth = global_DepthParent_Socket0[2*i];

        for(int j=1; j<=Adj[0]; j++)
        {
            int nghbr_id = Adj[j];
            int depth_of_nghbr = (nghbr_id < global_vertices_for_socket0) ? global_DepthParent_Socket0[2*nghbr_id] : global_DepthParent_Socket1[2*nghbr_id];

            if  ((current_depth == NOT_ASSIGNED) && depth_of_nghbr == NOT_ASSIGNED) continue;
            if  ((depth_of_nghbr - current_depth) > 1) ERROR_PRINT();
        }
    }

    for(int i=global_vertices_for_socket0; i < global_number_of_vertices; i++)
    {
        int *Adj = GET_GLOBAL_ADJACENCY(i);
        int current_depth = global_DepthParent_Socket1[2*i];

        for(int j=1; j<=Adj[0]; j++)
        {
            int nghbr_id = Adj[j];
            int depth_of_nghbr = (nghbr_id < global_vertices_for_socket0) ? global_DepthParent_Socket0[2*nghbr_id] : global_DepthParent_Socket1[2*nghbr_id];

            if  ((current_depth == NOT_ASSIGNED) && depth_of_nghbr == NOT_ASSIGNED) continue;
            if  ((depth_of_nghbr - current_depth) > 1) ERROR_PRINT();
        }
    }


#endif

#endif

    printf("Total Sum of Depths = %lld ::: Average Depth = %.3lf\n", sum, (sum*1.0)/global_number_of_vertices);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
    int total_size_of_depth_array = global_number_of_vertices*sizeof(int);
    int number_of_cache_lines = total_size_of_depth_array;

    int *CL = (int *)small_malloc(number_of_cache_lines*sizeof(int));

    int total_count = 0;

    for(int i=0; i<global_number_of_vertices; i++)
    {
        int  count = 0;
        for(int j=1; j<=global_Adjacency[i][0]; j++)
        {
            int id = global_Adjacency[i][j];
            int cache_line_address = id & 0xFFFFFFF0;

            int k;
            for(k=0; k<count; k++) if (CL[k] == cache_line_address) break;
            if (k==count) CL[count++] = cache_line_address;
        }

        total_count += count;
    }

    printf("Total number of CacheLines = %d ::: Average Per Vertex = %.2lf\n", total_count, (1.0*total_count)/global_number_of_vertices);
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    {
        unsigned long long total_time = 0;
        for(int i=0; i<nthreads; i++) printf("%d thread took %lld cycles \t\t\t Overhead was %lld cycles (%.2lf %%)\n", i, Timer[i], Overhead_Timer[i], (Overhead_Timer[i]*100.0)/Timer[i]);
        for(int i=0; i<nthreads; i++) total_time += Timer[i];
PRINT_RED
        printf("Average Time = %lld cycles\n", total_time/nthreads);
PRINT_BLACK
        //printf("OVERHEAD: "); for(int i=0; i<nthreads; i++) printf("%d thread took %lld cycles ::: ", i, Overhead_Timer[i]); printf("\n");
    }




    if (erroneous_vertices  /*|| not_assigned_vertices*/ || erroneous_parents)
    {
    PRINT_BLUE
        printf("erroneous_parents = %d ::: global_number_of_vertices = %d \n", erroneous_parents, global_number_of_vertices);
        printf("erroneous_vertices = %d ::: global_number_of_vertices = %d \n", erroneous_vertices, global_number_of_vertices);
        printf("not_assigned_vertices  = %d (%.2lf %%) ::: global_number_of_vertices = %d \n", not_assigned_vertices,  (100.0*not_assigned_vertices)/global_number_of_vertices, global_number_of_vertices);
        printf("max_diff_between_not_assigned = %d\n", max_diff_between_not_assigned);
        //ERROR_PRINT();
        printf("******************** ERROR: \n\n\n");
    PRINT_BLACK
    }
    else
    {
        printf("Checking was SUCCESSFUL\n");
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
