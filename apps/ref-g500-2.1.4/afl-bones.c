#include "rmd_afl_all.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

//#define SINGLE_THREAD
extern void pthread_yield();
extern int g500main(int argc, char**argv);

void *thr_routine(void *arg);
rmd_env_param_t *env_param;

int main_codelet(int argc, char *argv[])
{
    int i,j,k;
    int err;
    int nthreads, nCEs, nXEs, nBlocks, nChips;
    pthread_t *thread;
    agentid_t *ids;

    /* you should use the -c <configfile> part of the commandline
     * to make sure you match what is done on memalyzer side of things
     */
    env_param = create_param_cmdline(argc, argv);
    if (env_param == NULL) {
        env_param = &default_param;
    }

    /* this needs to be called ONCE before any thread calls initmem
     * or tries to allocate memory from the special RMD memory pools
     */
    //    setup_rmd_memory_full_spec(env_param->nchips, env_param->block_perchip,
    //            env_param->xe_perblock, env_param->ce_perblock,
    //      env_param->dram_size, env_param->num_dram,
    //      env_param->block_size, env_param->num_block,
    //      env_param->local_mem_size, env_param->num_local_mem,
    //      env_param->local_stack_size);

    nthreads = env_param->nchips * env_param->block_perchip * (env_param->xe_perblock + env_param->ce_perblock);
    thread = (pthread_t*)malloc(sizeof(pthread_t)*nthreads);
    ids    = (agentid_t*)malloc(sizeof(agentid_t)*nthreads);

    nChips  = env_param->nchips;
    nBlocks = env_param->block_perchip;
    nCEs    = env_param->ce_perblock;
    nXEs    = env_param->xe_perblock;

    //srand(time(NULL)); /* seed the random number generator */
#ifndef SINGLE_THREAD
    for (i=0;i<nChips;i++)
        for (j=0;j<nBlocks;j++)
            for(k=0;k<nCEs+nXEs;k++)
                ids[k+j*(nCEs+nXEs)+i*nBlocks*(nCEs+nXEs)] = afl_gen_core_id(i,j,k);

    for (i=0;i<nthreads;i++) {
        err = pthread_create(&thread[i], NULL, thr_routine, (void*)&ids[i]);
        if (err != 0) {
            fprintf(stderr, "Unable to create thread, exiting\n");
            exit(42);
        }
    }

#else
    printf("foo!\n");
    ids[0] = afl_gen_core_id(0,0,0);
    thr_routine((void*)ids);
    g500main(1,NULL);
#endif

#ifndef SINGLE_THREAD
    for (i=0;i<nthreads;i++) {
        err = pthread_join(thread[i], NULL);
        if (err) {
            fprintf(stderr, "Unable to join thread, exiting\n");
            exit(42);
        }
    }
#endif
    teardown_rmd_memory_global();
    return 0;
}


void *thr_routine(void *arg)
{
    agentid_t myid = *(agentid_t*)arg;

    /* this needs to be called once for each thread, before
     * that thread tries to allocate memory
     */
    initmem_rmd_thread(myid);
    printf("I am thread 0x%lx\n", myid);
    return NULL;
}
