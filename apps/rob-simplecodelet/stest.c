#define NUMCODELETS 128

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


#ifdef AFL
#include "codelet.h"
#include "rmd_afl_all.h"
#else
#include "xe-codelet.h"  //note hyphen, not underscore :(
#include "xe_memory.h"
#include "xe_console.h"
#include "xe_global.h"
#endif

rmd_guid_t simple(uint64_t arg, int n_db, void *db_ptr[],
		      rmd_guid_t *db) {
  rmd_guid_t emptyguid;

  rmd_agent_core_t me;
  me.data=__self.data;
  //  xe_printf("in simple \n");
  xe_printf("in simple #%ld on b=%d/x=%d.\n",
  	    arg, get_block_id(me), get_core_id(me));
  //  printf("in simple, b=%d xe=%d.\n", afl_get_block_id(get_currentid()),
  //	 afl_get_core_id(get_currentid()));

#ifndef __ALWAYS_UNDEFINED
  int i,j,k,l;
  for (i=0; i<1000; i++) 
    for (j=0; j<10000; j++) 
      k++;

  l=k; // use it so doesn't get compiled away
#endif

  //usleep(1);
  //sched_yield();

  return(emptyguid);
}

rmd_guid_t end_codelet(uint64_t arg, int n_db, void *db_ptr[],
			rmd_guid_t *db) {

  xe_printf("RCK: program ends here.\n"); // what core? :)
  rmd_complete();

  rmd_guid_t nullret;
  return nullret; // not for any reason other than it's handy
}

rmd_guid_t main_codelet(uint64_t arg, int n_db, void *db_ptr[],
			rmd_guid_t *db) {
  
  //rmd_cmd_line_t *ptr = db_ptr[0];
  rmd_guid_t ret;
  rmd_guid_t simplecdlt_t;
  rmd_guid_t simplecdlt_i[NUMCODELETS];
  rmd_guid_t endcodelet_t, endcodelet_i;
  rmd_guid_t endtrigger;
  rmd_guid_t emptydb=INVAL_GUID;
  int i;

  xe_printf("Starting main_codelet, agent stuff: b=%d/x=%d.\n",
	    get_block_id(__self), get_core_id(__self));


  // create the one that ends everything
  rmd_codelet_create(&endcodelet_t, end_codelet, 0,0,1,0,false,0);
  rmd_codelet_sched(&endcodelet_i, 0, endcodelet_t);
  rmd_dep_add(endtrigger,endcodelet_i, 0);

  //RCK initialize_core_logging("log_output.txt");
  xe_printf("Starting.\n");
  //printf("in main, b=%d xe=%d.\n", afl_get_block_id(get_currentid()),
  //afl_get_core_id(get_currentid()));
  
  rmd_codelet_create(&simplecdlt_t,simple,0,0,1,0,false,0);  
  for (i=0; i<NUMCODELETS; i++) {
    xe_printf("scheduling %d\n",i);
    rmd_codelet_sched(&simplecdlt_i[i], i,simplecdlt_t);
  }  

  for (i=0; i<NUMCODELETS; i++) {
    xe_printf("satisfying %d\n",i);
    rmd_codelet_satisfy(simplecdlt_i[i], emptydb, 0);
  }  

  //rmd_codelet_satisfy(endcodelet_i, emptydb, 0);
  xe_printf("main_codelet done.\n");
  ret.data = EXIT_SUCCESS;

  return ret;
}
