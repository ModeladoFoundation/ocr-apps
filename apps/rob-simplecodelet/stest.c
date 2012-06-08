#define NUMDUMB 512

//#define rckprintf(a) printf(a)
//#define rckprintf(a) fprintf(stderr,a)
#define rckprintf(a)

//#define rckprintf1(a) printf(a)
//#define rckprintf1(a) fprintf(stderr,a)
#define rckprintf1(a)

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>


#include "codelet.h"
#include "rmd_afl_all.h"

rmd_guid_t simple(uint64_t arg, int n_db, void *db_ptr[],
		      rmd_guid_t *db) {
  rmd_guid_t emptyguid;
  //printf("in simple, b=%d xe=%d.\n", afl_get_block_id(get_currentid()),
  //afl_get_core_id(get_currentid()));

  //  int i,j;
  //  for (i=0; i<1000000; i++) 
  //    for (j=0; j<100000; j++) ;

  usleep(1);
  //sched_yield();

  return(emptyguid);
}

rmd_guid_t end_codelet(uint64_t arg, int n_db, void *db_ptr[],
			rmd_guid_t *db) {

  //printf("RCK: program ends here.\n"); // what core? :)
  rmd_complete();

  rmd_guid_t nullret;
  return nullret; // not for any reason other than it's handy
}

rmd_guid_t main_codelet(uint64_t arg, int n_db, void *db_ptr[],
			rmd_guid_t *db) {
  
  //rmd_cmd_line_t *ptr = db_ptr[0];
  rmd_guid_t ret;
  rmd_guid_t simplecdlt_t;
  rmd_guid_t simplecdlt_i[NUMDUMB];
  rmd_guid_t endcodelet_t, endcodelet_i;
  int i;

  // create the one that ends everything
  rmd_codelet_create(&endcodelet_t, end_codelet, 0,0,1,0,false,0);
  rmd_codelet_sched(&endcodelet_i, 0, endcodelet_t);
  rmd_dep_add(rmd_env_idle,endcodelet_i, 0);

  //RCK initialize_core_logging("log_output.txt");
  //printf("Starting.\n");
  //printf("in main, b=%d xe=%d.\n", afl_get_block_id(get_currentid()),
  //afl_get_core_id(get_currentid()));
  
  rmd_codelet_create(&simplecdlt_t,simple,0,0,1,0,false,0);  
  for (i=0; i<NUMDUMB; i++) {
    rmd_codelet_sched(&simplecdlt_i[i], 0,simplecdlt_t);
  }  

  rmd_guid_t emptydb;

  for (i=0; i<NUMDUMB; i++) {
    rmd_codelet_satisfy(simplecdlt_i[i], emptydb, 0);
  }  

  printf("main_codelet done.\n");
  ret.data = EXIT_SUCCESS;

  return ret;
}
