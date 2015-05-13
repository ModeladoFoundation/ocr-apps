// hack to turn off any omp functions
#define omp_get_num_procs()    1
#define omp_get_num_threads()  1
#define omp_set_num_threads(a)    1
#define omp_get_wtime()        MPI_Wtime()
#define omp_get_thread_num()   0
