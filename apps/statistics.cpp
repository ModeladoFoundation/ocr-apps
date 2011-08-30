
#define DEBUG_BINS 8192
unsigned long long int Timer4[MAX_THREADS][DEBUG_BINS];
unsigned long long int Timer5[MAX_THREADS][DEBUG_BINS];

void Report_Per_Phase_Performance(void)
{

#ifdef MULTI_SOCKET
    if (1)
    {
        long long int start_d, end_d;

        long long int Sum4[DEBUG_BINS] = {0};
        long long int Sum5[DEBUG_BINS] = {0};
        long long int Max4[DEBUG_BINS] = {0};
        long long int Max5[DEBUG_BINS] = {0};
    
        start_d = 0; end_d = DEBUG_BINS;
        for(long long int d=start_d; d<end_d; d++)
        {
            Max4[d] = Timer4[0][d];
            Max5[d] = Timer5[0][d];
            for(int th=0; th<nthreads; th++) 
            {
                Max4[d] = MAX(Timer4[th][d], Max4[d]);
                Max5[d] = MAX(Timer5[th][d], Max5[d]);
                Sum4[d] += Timer4[th][d];
                Sum5[d] += Timer5[th][d];
            }
        }

        long long int bfs_1 = 0, bfs_2 = 0;
        for(long long int d=start_d; d<end_d; d++) bfs_1 += Max4[d];
        for(long long int d=start_d; d<end_d; d++) bfs_2 += Max5[d];

        start_d = DEBUG_BINS/4; end_d = start_d + 16;
        start_d = 0; end_d = DEBUG_BINS;

        for(long long int d=start_d; d<end_d; d++) {if (!Sum4[d]) continue; printf ("d = %3lld", d); for(int th=0; th<nthreads; th++) printf("%10lld", Timer4[th][d]); printf("  [%10lld] ::: [%10lld] ::: %.2lfX\n", Max4[d], Sum4[d]/nthreads, (Max4[d]*nthreads*1.0)/Sum4[d]); }
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
        for(long long int d=start_d; d<end_d; d++) {if (!Sum5[d]) continue; printf ("d = %3lld", d); for(int th=0; th<nthreads; th++) printf("%10lld", Timer5[th][d]); printf("  [%10lld] ::: [%10lld] ::: %.2lfX\n", Max5[d], Sum5[d]/nthreads, (Max5[d]*nthreads*1.0)/Sum5[d]); }
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");




        printf("BFS_1() : %12lld (cpe: %5.2lf) (%5.1lf %%)\n", bfs_1, (bfs_1*1.0/global_number_of_traversed_edges), (bfs_1*100.0)/(bfs_1 + bfs_2));
        printf("BFS_2() : %12lld (cpe: %5.2lf) (%5.1lf %%)\n", bfs_2, (bfs_2*1.0/global_number_of_traversed_edges), (bfs_2*100.0)/(bfs_1 + bfs_2));
        printf("Total() : %12lld (cpe: %5.2lf) (%5.1lf %%)\n", bfs_1+bfs_2, ( (bfs_1+bfs_2)*1.0/global_number_of_traversed_edges), ((bfs_1+bfs_2)*100.0)/(bfs_1 + bfs_2));
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    
        // Computing stuff for the model
        global_number_of_assigned_vertices = 0;
        int depth = 0;

        for(int i=0; i<global_vertices_for_socket0; i++)
        {
            global_number_of_assigned_vertices += (global_DepthParent_Socket0[2*i] != NOT_ASSIGNED);
            if( (global_DepthParent_Socket0[2*i] != NOT_ASSIGNED) && (depth < global_DepthParent_Socket0[2*i]) ) depth = global_DepthParent_Socket0[2*i];
        }

        for(int i=global_vertices_for_socket0; i<global_number_of_vertices; i++)
        {
            global_number_of_assigned_vertices += (global_DepthParent_Socket1[2*i] != NOT_ASSIGNED);
            if( (global_DepthParent_Socket1[2*i] != NOT_ASSIGNED) && (depth < global_DepthParent_Socket1[2*i]) ) depth = global_DepthParent_Socket1[2*i];
        }

        printf("V = %d\n", global_number_of_vertices);
        printf("E = %lld\n", global_number_of_edges);
        printf("V' = %d\n", global_number_of_assigned_vertices);
        printf("E' = %lld\n", global_number_of_traversed_edges);
        printf("depth = %d\n", (depth+1));
    }

    if (0)
    {
        long long int start_d = DEBUG_BINS/4; long long int end_d = start_d + 16;
    
        for(long long int d=start_d; d<end_d; d++) {printf ("d = %10lld", d); for(int th=0; th<nthreads; th++) printf("%16lld", Timer4[th][d]); printf("\n");}
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
        for(long long int d=start_d; d<end_d; d++) {printf ("d = %10lld", d); for(int th=0; th<nthreads; th++) printf("%16lld", Timer5[th][d]); printf("\n");}
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

        long long int Sum4[MAX_THREADS] = {0};
        long long int Sum5[MAX_THREADS] = {0};

        for(int th=0; th<nthreads; th++) for(int d=start_d; d<end_d; d++) Sum4[th] += Timer4[th][d];
        for(int th=0; th<nthreads; th++) for(int d=start_d; d<end_d; d++) Sum5[th] += Timer5[th][d];
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");


        for(int th=0; th<nthreads; th++) printf("%10lld", Sum4[th]);  printf("\n");
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
        for(int th=0; th<nthreads; th++) printf("%10lld", Sum5[th]);  printf("\n");
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
        for(int th=0; th<nthreads; th++) printf("%10lld", (Sum4[th]+Sum5[th])); printf("\n");
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    }
#endif
}
