
static long long int total_time_without_fixed_neighbors = 0;
static long long int total_time_binning = 0;
            
#ifdef USE_GROUPS
__inline void Perform_BFS_without_fixed_neighbors(int threadid, int taskid, int depth, int *BS0, int start_index, int end_index, int **BS1, int *Count1, int *Depth)
{
    unsigned long long int stime = read_tsc();
#define NUM3 4

    float *Depth_float = (float *)(Depth);
    int *local_BS1 = BS1[GROUPS];
    int next_counter = 0;
    __m128i xmm_not_assigned = _MM_SET1(NOT_ASSIGNED);

    int counter = (end_index - start_index);
    int counter_prime = NUM3*(counter/NUM3);
    int end_index_prime = start_index + counter_prime;

    //end_index_prime = start_index;

    for(int j=start_index; j<end_index_prime; j+=NUM3)
    {
        int prev_counter = next_counter;
        for(int k=0; k<NUM3; k++) 
        {
            int state_id = BS0[j+k];
            _mm_prefetch((char *)(global_Adjacency[BS0[j+k+8]]), _MM_HINT_T1);
            int *temp_address = global_Adjacency[state_id];
            int neighbors = temp_address[0];

        
            int neighbors_prime = neighbors & 0xFFFFFFFC;
            for(int k=1; k < (neighbors_prime+1); k+=SIMD_WIDTH)
            {
            
                __m128i xmm_idx0 = _MM_LOADU(temp_address + k);
                __m128i mask_0 = _MM_CMP_EQ(xmm_not_assigned, _mm_gather_epi32(Depth_float, xmm_idx0));
                int mask_value_0 = _mm_movemask_ps(_mm_castsi128_ps(mask_0)); //mask_value is [0..15]
                int number_of_ones_0 = Number_of_Ones[mask_value_0];

                _MM_STOREU((local_BS1 + next_counter), _MM_SHUFFLE_EPI8(xmm_idx0, Shuffle_Pattern[mask_value_0]));
                next_counter += number_of_ones_0;
            }
        
            for(int k=(neighbors_prime+1); k<=neighbors; k++)
            {
                int new_state_id_0 = temp_address[k];
                UPDATE_DEPTH_AND_BS(Depth, new_state_id_0, NOT_ASSIGNED, depth, local_BS1, next_counter);
            }
        }


        _MM_STOREU(local_BS1 + next_counter, _MM_SET1(global_starting_vertex));

        ////////////////////////////////////////////////////////////////////////////////////////////////
        //Step D: Scatter the "depth" value to these boundary //states...
        ////////////////////////////////////////////////////////////////////////////////////////////////

        for(int k=prev_counter; k<next_counter; k+=SIMD_WIDTH) _MM_SCATTER((local_BS1 + k), depth, Depth);
        Depth[global_starting_vertex] = MINIMUM_DEPTH;
    }
    
    for(int j = end_index_prime; j < end_index; j++)
    {
        _mm_prefetch((char *)(global_Adjacency[BS0[j+8]]), _MM_HINT_T0);

        int state_id = BS0[j];
        int *temp_address = global_Adjacency[state_id];
        int neighbors = temp_address[0];
        for(int k=1; k<=neighbors; k+=1)
        {
            int new_state_id_0 = temp_address[k+0];
			UPDATE_DEPTH_AND_BS(Depth, new_state_id_0, NOT_ASSIGNED, depth, local_BS1, next_counter);
        }
    }

    for(int i=0; i<next_counter; i++)
    {
        _mm_prefetch((char *)(global_Adjacency[local_BS1[i+8]]), _MM_HINT_T0);
        int new_state_id = local_BS1[i];
        int neighbors = global_Adjacency[new_state_id][0]; 

        int binned_vertex = (neighbors > GROUPS_1) ?  GROUPS_1 : neighbors;
        int counter = Count1[binned_vertex]; 

        Count1[binned_vertex]++;  
        BS1[binned_vertex][counter] = new_state_id; 
    }

    unsigned long long int etime = read_tsc();
    total_time_without_fixed_neighbors += (etime-stime);
}


__inline void Perform_BFS_without_fixed_neighbors_not_used(int threadid, int taskid, int depth, int *BS0, int start_index, int end_index, int **BS1, int *Count1, int *Depth)
{
#define NUM2 8
    unsigned long long int stime = read_tsc();

#if 0
    for(int j = start_index; j < end_index; j++)
    {
        _mm_prefetch((char *)(global_Adjacency[BS0[j+8]]), _MM_HINT_T0);

        int state_id = BS0[j];
        int *temp_address = global_Adjacency[state_id];
        int neighbors = temp_address[0];
        for(int k=1; k<=neighbors; k+=1)
        {
            int new_state_id_0 = temp_address[k+0];
            UPDATE_DEPTH_AND_BS_NEW(Depth, new_state_id_0, NOT_ASSIGNED, depth, BS1, Count1);
        }
    }
#else

    float *Depth_float = (float *)(Depth);
    int *local_BS1 = BS1[GROUPS];
    int next_counter_0 = 0;
    __m128i xmm_not_assigned = _MM_SET1(NOT_ASSIGNED);

#if 1
    for(int j = start_index; j < end_index; j++)
    {
        _mm_prefetch((char *)(global_Adjacency[BS0[j+8]]), _MM_HINT_T0);

        int state_id = BS0[j];
        int *temp_address = global_Adjacency[state_id];
        int neighbors = temp_address[0];
                
        int neighbors_prime = neighbors & 0xFFFFFFFC;
        for(int k=1; k < (neighbors_prime+1); k+=SIMD_WIDTH)
        {
                
            __m128i xmm_idx0 = _MM_LOADU(temp_address + k);
            __m128i mask_0 = _MM_CMP_EQ(xmm_not_assigned, _mm_gather_epi32(Depth_float, xmm_idx0));
            int mask_value_0 = _mm_movemask_ps(_mm_castsi128_ps(mask_0)); //mask_value is [0..15]
            int number_of_ones_0 = Number_of_Ones[mask_value_0];
            //if (number_of_ones_0) 
            {
                _MM_STOREU((local_BS1 + next_counter_0), _MM_SHUFFLE_EPI8(xmm_idx0, Shuffle_Pattern[mask_value_0]));
                next_counter_0 += number_of_ones_0;
            }
        }
        for(int k=(neighbors_prime+1); k<=neighbors; k++)
        {
            int new_state_id_0 = temp_address[k];
            UPDATE_DEPTH_AND_BS(Depth, new_state_id_0, NOT_ASSIGNED, depth, local_BS1, next_counter_0);
        }
    }

            
    _MM_STOREU(local_BS1 + next_counter_0, _MM_SET1(global_starting_vertex));

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //Step D: Scatter the "depth" value to these boundary //states...
    ////////////////////////////////////////////////////////////////////////////////////////////////

    for(int k=0; k<next_counter_0; k+=SIMD_WIDTH) _MM_SCATTER((local_BS1 + k), depth, Depth);
    Depth[global_starting_vertex] = MINIMUM_DEPTH;
    
    for(int i=0; i<next_counter_0; i++)
    {
        _mm_prefetch((char *)(global_Adjacency[local_BS1[i+8]]), _MM_HINT_T0);
        int new_state_id = local_BS1[i];
        int neighbors = global_Adjacency[new_state_id][0]; 

        int binned_vertex = (neighbors > GROUPS_1) ?  GROUPS_1 : neighbors;
        int counter = Count1[binned_vertex]; 

        Count1[binned_vertex]++;  
        BS1[binned_vertex][counter] = new_state_id; 
    }

#else

    int *BN_0 = Boundary_Neighbors0[threadid];
    int counter = (end_index - start_index);
    int counter_prime = NUM2*(counter/NUM2);
    int end_index_prime = start_index + counter_prime;

    for(int j=start_index; j<end_index_prime; j+=NUM2)
    {
        int collected_neighbors = 0;
        //Collect all the neighbors into 1 BN_0 array :)
                
        for(int k=0; k<NUM2; k++) 
        {
            int state_id = BS0[j+k];
            _mm_prefetch((char *)(global_Adjacency[BS0[j+k+8]]), _MM_HINT_T1);
            int *temp_address = global_Adjacency[state_id];
            int neighbors = temp_address[0];
            for(int l=0; l<neighbors; l += SIMD_WIDTH)
            {
                _MM_STOREU((BN_0 + collected_neighbors + l), _MM_LOADU(temp_address + l + 1));
            }
                    
            collected_neighbors += neighbors;
        }

        _MM_STOREU((BN_0 + collected_neighbors), _MM_SET1(global_starting_vertex));
                
        int prev_next_counter_0 = next_counter_0;
        int *temp_address = BN_0;
        int neighbors = collected_neighbors;

        for(int k=0; k<neighbors; k += SIMD_WIDTH)
        {
            __m128i xmm_idx0 = _MM_LOADU(temp_address + k);
            __m128i mask_0 = _MM_CMP_EQ(xmm_not_assigned, _mm_gather_epi32(Depth_float, xmm_idx0)); 
            int mask_value_0 = _mm_movemask_ps(_mm_castsi128_ps(mask_0)); //mask_value is [0..15]
            int number_of_ones_0 = Number_of_Ones[mask_value_0];

            //if (number_of_ones_0) //( || number_of_ones_1)
            {
                _MM_STOREU((local_BS1 + next_counter_0), _MM_SHUFFLE_EPI8(xmm_idx0, Shuffle_Pattern[mask_value_0]));
                next_counter_0 += number_of_ones_0;
            }
        }

        _MM_STOREU(local_BS1 + next_counter_0, _MM_SET1(global_starting_vertex));
        
        ////////////////////////////////////////////////////////////////////////////////////////////////
        //Step D: Scatter the "depth" value to these boundary //states...
        ////////////////////////////////////////////////////////////////////////////////////////////////

        for(int k=prev_next_counter_0; k<next_counter_0; k+=SIMD_WIDTH) _MM_SCATTER((local_BS1 + k), depth, Depth);
        Depth[global_starting_vertex] = MINIMUM_DEPTH;

    }
            
    for(int j=end_index_prime; j<end_index; j++)
    {
    
        _mm_prefetch((char *)(global_Adjacency[BS0[j+1]]), _MM_HINT_T1);
        _mm_prefetch((char *)(global_Adjacency[BS0[j+2]]), _MM_HINT_T1);

        int prev_next_counter_0 = next_counter_0;
        int state_id = BS0[j];
        int *temp_address = global_Adjacency[state_id];
        int neighbors = temp_address[0];
        int neighbors_prime = neighbors & 0xFFFFFFFC;

        for(int k=1; k<=neighbors_prime; k+=SIMD_WIDTH)
        {
            _mm_prefetch((char *)(temp_address + k + 16), _MM_HINT_T0);
            __m128i mask_0 = _MM_CMP_EQ(xmm_not_assigned, _mm_gather_epi32(Depth_float, _MM_LOADU(temp_address + k)));
            int mask_value_0 = _mm_movemask_ps(_mm_castsi128_ps(mask_0)); //mask_value is [0..15]
            int number_of_ones_0 = Number_of_Ones[mask_value_0];
                    
            //if (number_of_ones_0) //( || number_of_ones_1)
            {
                _MM_STOREU((local_BS1 + next_counter_0), _MM_SHUFFLE_EPI8(_MM_LOADU(temp_address + k), Shuffle_Pattern[mask_value_0]));
                next_counter_0 += number_of_ones_0;
            }
        }

        _MM_STOREU(local_BS1 + next_counter_0, _MM_SET1(global_starting_vertex));

        ////////////////////////////////////////////////////////////////////////////////////////////////
        //Step D: Scatter the "depth" value to these boundary //states...
        ////////////////////////////////////////////////////////////////////////////////////////////////

        for(int k=prev_next_counter_0; k<next_counter_0; k+=SIMD_WIDTH) _MM_SCATTER((local_BS1 + k), depth, Depth);
        Depth[global_starting_vertex] = MINIMUM_DEPTH;

        for(int k=(neighbors_prime+1); k<=neighbors; k++)
        {
            int new_state_id_0 = temp_address[k];
            UPDATE_DEPTH_AND_BS(Depth, new_state_id_0, NOT_ASSIGNED, depth, local_BS1, next_counter_0);
        }
    }

    //unsigned long long int etime = read_tsc();
    //total_time_without_fixed_neighbors += (etime-stime);

    for(int i=0; i<next_counter_0; i++)
    {
        _mm_prefetch((char *)(global_Adjacency[local_BS1[i+8]]), _MM_HINT_T0);
        int new_state_id = local_BS1[i];
        int neighbors = global_Adjacency[new_state_id][0]; 

        int binned_vertex = (neighbors > GROUPS_1) ?  GROUPS_1 : neighbors;
        int counter = Count1[binned_vertex]; 

        Count1[binned_vertex]++;  
        BS1[binned_vertex][counter] = new_state_id; 
    }


#endif
#endif
    unsigned long long int etime = read_tsc();
    total_time_without_fixed_neighbors += (etime-stime);
    //printf("Total Time Taken inside without_fixed_neighbors = %lld ::: %lld\n", (etime-stime), total_time);

}
#endif

#ifdef USE_GROUPS
#include "Generator/cases_with_fixed_neighbors.cpp"
#endif
