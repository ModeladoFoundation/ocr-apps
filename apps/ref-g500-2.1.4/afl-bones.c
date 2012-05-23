#include "rmd_afl_all.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

//#define SINGLE_THREAD
extern void pthread_yield();
extern int g500main(int argc, char**argv);

void *thr_routine(void *arg);
rmd_env_param_t *env_param;

rmd_guid_t main_codelet(uint64_t arg, int n_in, void *db_ptr[],
			rmd_guid_t *in) {

  rmd_guid_t ret;

  printf("foo!\n");
  g500main(1,NULL);
  printf("\n\n\n----------------------------------done!\n");

  rmd_complete();
  ret.data=0;
  return ret;
}

