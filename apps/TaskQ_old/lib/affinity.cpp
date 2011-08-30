
extern int nthreads;
extern int nsockets;
extern int global_cores_per_socket;


#define ERROR_PRINT_TASKQ() {printf("Error on line (%d) in file (%s)\n", __LINE__, __FILE__); exit(95123);}


void Set_Affinity (int threadid)
{
    if (global_cores_per_socket != CORES_PER_SOCKET) ERROR_PRINT_TASKQ();

#ifdef NUMA_AWARENESS

#if 0
    int socket_id = COMPUTE_SOCKET_ID(threadid);
    if (numa_run_on_node(socket_id)!=0) {
      printf("numa_run_on_node() failed!\n");
      pthread_exit(NULL);
    } else {
      //printf("Run t%d on node %d\n", threadid, socket_id);
    }
    numa_set_preferred(socket_id);
#endif

    cpu_set_t mask;
    CPU_ZERO(&mask);
     // Real thread assignment to sockets:
	// Socket 0: [ t0, t2, t4, t6, t8, t10, t12, t14, t16, t18, t20, t22 ]
	// Socket 1: [ t1, t3, t5, t7, t9, t11, t13, t15, t17, t19, t21, t23 ]
    
    // Enable if Code assumption: threadid 0-5 : socket 0, threadid 6-11 : socket 1, threadid 12-17 : socket 0 (SMT), threadid 18-23 : socket 1 (SMT)
    //int threadid_prime = 2*(threadid%6) + ( (threadid%12)/6) + (threadid/12)*12; 


     // Else do the following
    int threadid_prime;
    if(nthreads <= CORES_PER_SOCKET) { // 1 socket, no SMT
	//code assumption: threads 0-5: socket 0
	threadid_prime = 2*threadid;	
    }else if ( (nthreads <= (CORES_PER_SOCKET*2)) && (nsockets == 1) ){ // 1 socket, SMT
	// code assumption: threads 0-11: socket 0
	threadid_prime = 2*threadid;
    }else if ( (nthreads <= (CORES_PER_SOCKET*2)) && (nsockets == 2) ){ // 2 sockets, no SMT
	// code assumption: threads 0-5: socket 0, threads 6-11: socket 1
	threadid_prime = 2*(threadid%CORES_PER_SOCKET) + (threadid/CORES_PER_SOCKET);
    }else if ( nthreads == (CORES_PER_SOCKET*2*2) ){ // 2 sockets, SMT
	// code assumption: threads 0-11: socket 0, threads 12-23: socket 1
	threadid_prime = 2*(threadid%(CORES_PER_SOCKET*2)) + (threadid/(CORES_PER_SOCKET*2));
    }else{
	printf("Error: nthreads: %d nsockets: %d\n", nthreads, nsockets);
	ERROR_PRINT_TASKQ();
    }
    //printf("threadid: %d threadid_prime: %d\n",threadid, threadid_prime);
    CPU_SET(threadid_prime, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#else
////////////////////////////////////////////////////////////////////////////////////////////////
#define AFFINITY
#ifdef AFFINITY
    cpu_set_t mask;
    CPU_ZERO(&mask);
     // Real thread assignment to sockets:
	// Socket 0: [ t0, t2, t4, t6, t8, t10, t12, t14, t16, t18, t20, t22 ]
	// Socket 1: [ t1, t3, t5, t7, t9, t11, t13, t15, t17, t19, t21, t23 ]
    
    // Enable if Code assumption: threadid 0-5 : socket 0, threadid 6-11 : socket 1, threadid 12-17 : socket 0 (SMT), threadid 18-23 : socket 1 (SMT)
    //int threadid_prime = 2*(threadid%6) + ( (threadid%12)/6) + (threadid/12)*12; 


     // Else do the following
    int threadid_prime;
    if(nthreads <= CORES_PER_SOCKET) { // 1 socket, no SMT
	//code assumption: threads 0-5: socket 0
	threadid_prime = 2*threadid;	
    }else if ( (nthreads <= (CORES_PER_SOCKET*2)) && (nsockets == 1) ){ // 1 socket, SMT
	// code assumption: threads 0-11: socket 0
	threadid_prime = 2*threadid;
    }else if ( (nthreads <= (CORES_PER_SOCKET*2)) && (nsockets == 2) ){ // 2 sockets, no SMT
	// code assumption: threads 0-5: socket 0, threads 6-11: socket 1
	threadid_prime = 2*(threadid%CORES_PER_SOCKET) + (threadid/CORES_PER_SOCKET);
    }else if ( nthreads == (CORES_PER_SOCKET*2*2) ){ // 2 sockets, SMT
	// code assumption: threads 0-11: socket 0, threads 12-23: socket 1
	threadid_prime = 2*(threadid%(CORES_PER_SOCKET*2)) + (threadid/(CORES_PER_SOCKET*2));
    }else{
	printf("Error: nthreads: %d nsockets: %d\n", nthreads, nsockets);
	ERROR_PRINT_TASKQ();
    }
    //printf("threadid: %d threadid_prime: %d\n",threadid, threadid_prime);
    //
    //For sngle socket systems, enable the following
    //threadid_prime = threadid;
    //threadid_prime = (threadid%2)*CORES_PER_SOCKET + (threadid/2);
    CPU_SET(threadid_prime, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
        

#endif
////////////////////////////////////////////////////////////////////////////////////////////////
#endif


}
