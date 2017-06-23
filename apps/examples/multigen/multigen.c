/* multigen.c
 *  *
 *   for testing OCR_HINT_EDT_SPAWNING
 *
 *   Each spawning_fn creates two EDTs: leaf1 and leaf2
 *   leaf1 does more work than leaf2
 *
 *   Graph looks like:
 *    mainEdt -> spawning_fn (1)            -> leaf1
 *                                          -> leaf2
 *
 *            -> spawning_fn (2)            -> leaf1
 *                                          -> leaf2
 *            ...
 *            -> spawning_fn (NUM_CHILDREN) -> leaf1
 *                                          -> leaf2
 *
 * shutdownEdt triggered when all of the spawning_fn's finish
 * spawning_fn's are created with EDT_PROP_FINISH which means
 * they have to wait for leaf EDTs created within their scope to finish
 *
 *                                                                                                              */

#include "ocr.h"
#define NUM_CHILDREN 32
// larger numbers for FIBN require XE stack be in IPM (xe_ipm_stack  = 128 in config file)
#define FIBN 25

/**
 *  *  * Initial code: simple Fibonacci
 *   *   */
int fib(int n) {
    if(n > 2) {
        return fib(n-1) + fib(n-2);
    } else {
        return n;
    }
}


ocrGuid_t leaf1(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
   //leaf1 does more work
   PRINTF("Hello from leaf1\n");
   int i,sum = 0;
   int result = fib(FIBN );
   for(i = 2; i < result+2; i++){
      if(i > result/2) {
         if(i%2 == 0) {
            sum+= i;
         }
      } else {
        sum += i*(i-1);
      }
   }
   PRINTF("End leaf1, result =%"PRId32" sum = %"PRId32"\n",result, sum);
   return NULL_GUID;
}

ocrGuid_t leaf2(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
   //leaf2 does less work
   PRINTF("Hello from leaf2\n");
   int result = fib(FIBN - 1);
   PRINTF("End leaf2, result =%"PRId32"\n",result);
   return NULL_GUID;
}

ocrGuid_t spawning_fn(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
   const u64 num = paramv[0];
   int* k;
   u64 _hintVal = 1;
   PRINTF("Start spawning_fn(%"PRId64")\n", num);
   ocrGuid_t db_guid, leaf1_template, leaf2_template, leaf1_edt, leaf2_edt;
   //Create two leaf EDTs
   //Create templates for the EDTs
   /**/
   ocrEdtTemplateCreate(&leaf1_template, leaf1, 0, 1);
   ocrEdtTemplateCreate(&leaf2_template, leaf2, 0, 1);
   ocrEdtCreate(&leaf1_edt, leaf1_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);
   ocrEdtCreate(&leaf2_edt, leaf2_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);

   //Start execution of the parallel EDTs
   ocrAddDependence(NULL_GUID, leaf1_edt, 0, DB_DEFAULT_MODE);
   ocrAddDependence(NULL_GUID, leaf2_edt, 0, DB_DEFAULT_MODE);

   ocrDbCreate(&db_guid, (void**) &k, sizeof(int), 0, NULL_HINT, NO_ALLOC);
   k[0] = fib(FIBN);

   PRINTF("End spawning_fn(%"PRId64"), sending k=%"PRId32"\n",num,*k);
   return db_guid;
   //return NULL_GUID;
}


ocrGuid_t shutdownEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
   PRINTF("Hello from shutdownEdt\n");
   PRINTF("--->  depc is: %"PRId32"\n",depc);
   PRINTF("--->  paramc is: %"PRId32"\n",paramc);
   PRINTF("--->  NUM_CHILDREN is: %"PRId32"\n",NUM_CHILDREN);
   int* data;
   int i;
/** //TODO: Not sure why this doesn't work:
   for(i=0; i < depc; i++){
      data = (int*) depv[i].ptr;
      PRINTF("Received data = %"PRId32"\n", *data);
   }
**/

   for(i=0; i < depc; i++){
      ocrDbDestroy(depv[i].guid);
   }
   ocrShutdown();
   return NULL_GUID;
}


ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
   int i;
   PRINTF("Starting mainEdt\n");
   ocrGuid_t edtJoin, edt_template, edtJoin_template;
   ocrGuid_t edts[NUM_CHILDREN],  outputEvents[NUM_CHILDREN];
   u64 _hintVal = 1;
#ifdef ENABLE_SPAWNING_HINT
   ocrHint_t _spawnHints;
   ocrHintInit(&_spawnHints, OCR_HINT_EDT_T);
   ocrSetHintValue(&_spawnHints, OCR_HINT_EDT_SPAWNING, _hintVal);
#endif

   //Create templates for the EDTs
   ocrEdtTemplateCreate(&edt_template, spawning_fn, 1, 1);
   ocrEdtTemplateCreate(&edtJoin_template, shutdownEdt, 0, NUM_CHILDREN);

   ocrEdtCreate(&edtJoin, edtJoin_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
         EDT_PROP_FINISH, NULL_HINT, NULL);

   for(i = 0; i < NUM_CHILDREN; i++) {
      u64 ii = i;
      PRINTF("EdtCreate: i is: %"PRId32"\n",i);
   //Create the EDTs
#ifdef ENABLE_SPAWNING_HINT
      ocrEdtCreate(&edts[i], edt_template, EDT_PARAM_DEF, &ii, EDT_PARAM_DEF, NULL,
           EDT_PROP_FINISH, &_spawnHints, &outputEvents[i]);
#else
      ocrEdtCreate(&edts[i], edt_template, EDT_PARAM_DEF, &ii, EDT_PARAM_DEF, NULL,
           EDT_PROP_FINISH, NULL_HINT, &outputEvents[i]);
#endif
      ocrAddDependence(outputEvents[i], edtJoin, i, DB_MODE_CONST);
   }


   for(i = 0; i < NUM_CHILDREN; i++) {
      PRINTF("AddDep: i is: %"PRId32"\n",i);
      //Start execution of the parallel EDTs
      ocrAddDependence(NULL_GUID, edts[i], 0, DB_DEFAULT_MODE);
   }

   return NULL_GUID;
}
