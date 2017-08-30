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
#include "time.h"
#ifdef ENABLE_SPAWNING_HINT
#include "priority.h"
#endif

#define NUM_CHILDREN 64
// larger numbers for FIBN require XE stack be in IPM (xe_ipm_stack  = 128 in config file)
#define FIBN 32

static clock_t start_time, diff;

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
   ocrPrintf("Hello from leaf1\n");
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
   ocrPrintf("End leaf1, result =%"PRId32" sum = %"PRId32"\n",result, sum);
   return NULL_GUID;
}

ocrGuid_t leaf2(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
   //leaf2 does less work
   ocrPrintf("Hello from leaf2\n");
   int result = fib(FIBN - 1);
   ocrPrintf("End leaf2, result =%"PRId32"\n",result);
   return NULL_GUID;
}


ocrGuid_t spawning_fn2(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
   const u64 num = paramv[0];
   int* k;
   u64 _hintVal = 1;
   ocrPrintf("Start spawning_fn2(%"PRId64")\n", num);
   ocrGuid_t db_guid, leaf1_template, leaf2_template;
   ocrGuid_t leaf1_edt, leaf2_edt;
   //Create two leaf EDTs
   //Create templates for the EDTs
   /**/
   ocrEdtTemplateCreate(&leaf1_template, leaf1, 0, 1);
   ocrEdtTemplateCreate(&leaf2_template, leaf2, 0, 1);
   int i;
   ocrEdtCreate(&leaf1_edt, leaf1_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);
   ocrEdtCreate(&leaf2_edt, leaf2_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);

   //Start execution of the parallel EDTs
   ocrAddDependence(NULL_GUID, leaf1_edt, 0, DB_DEFAULT_MODE);
   ocrAddDependence(NULL_GUID, leaf2_edt, 0, DB_DEFAULT_MODE);

   ocrDbCreate(&db_guid, (void**) &k, sizeof(int), 0, NULL_HINT, NO_ALLOC);
   k[0] = fib(FIBN);

   ocrPrintf("End spawning_fn2(%"PRId64"), sending k=%"PRId32"\n",num,*k);
   return db_guid;
}

ocrGuid_t spawning_fn3(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
   const u64 num = paramv[0];
   int* k;
   u64 _hintVal = 1;
   ocrPrintf("Start spawning_fn3(%"PRId64")\n", num);
   ocrGuid_t db_guid, leaf1_template, leaf2_template;
   ocrGuid_t leaf1_edt, leaf2_edt, leaf2_2_edt;
   //Create two leaf EDTs
   //Create templates for the EDTs
   /**/
   ocrEdtTemplateCreate(&leaf1_template, leaf1, 0, 1);
   ocrEdtTemplateCreate(&leaf2_template, leaf2, 0, 1);
   int i;
   ocrEdtCreate(&leaf1_edt, leaf1_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);
   ocrEdtCreate(&leaf2_edt, leaf2_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);
   ocrEdtCreate(&leaf2_2_edt, leaf2_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);

   //Start execution of the parallel EDTs
   ocrAddDependence(NULL_GUID, leaf1_edt, 0, DB_DEFAULT_MODE);
   ocrAddDependence(NULL_GUID, leaf2_edt, 0, DB_DEFAULT_MODE);
   ocrAddDependence(NULL_GUID, leaf2_2_edt, 0, DB_DEFAULT_MODE);

   ocrDbCreate(&db_guid, (void**) &k, sizeof(int), 0, NULL_HINT, NO_ALLOC);
   k[0] = fib(FIBN);

   ocrPrintf("End spawning_fn3(%"PRId64"), sending k=%"PRId32"\n",num,*k);
   return db_guid;
}

ocrGuid_t spawning_fn6(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
   const u64 num = paramv[0];
   int* k;
   u64 _hintVal = 1;
   ocrPrintf("Start spawning_fn6(%"PRId64")\n", num);
   ocrGuid_t db_guid, leaf1_template, leaf2_template;
   ocrGuid_t leaf1_edt, leaf1_1_edt, leaf1_2_edt, leaf2_edt, leaf2_1_edt, leaf2_2_edt;
   //Create two leaf EDTs
   //Create templates for the EDTs
   /**/
   ocrEdtTemplateCreate(&leaf1_template, leaf1, 0, 1);
   ocrEdtTemplateCreate(&leaf2_template, leaf2, 0, 1);
   int i;
   ocrEdtCreate(&leaf1_edt, leaf1_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);
   ocrEdtCreate(&leaf2_edt, leaf2_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);
   ocrEdtCreate(&leaf2_1_edt, leaf2_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);
   ocrEdtCreate(&leaf1_1_edt, leaf1_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);
   ocrEdtCreate(&leaf2_2_edt, leaf2_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);
   ocrEdtCreate(&leaf1_2_edt, leaf1_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                EDT_PROP_FINISH, NULL_HINT, NULL);

   //Start execution of the parallel EDTs
   ocrAddDependence(NULL_GUID, leaf1_edt, 0, DB_DEFAULT_MODE);
   ocrAddDependence(NULL_GUID, leaf2_edt, 0, DB_DEFAULT_MODE);
   ocrAddDependence(NULL_GUID, leaf1_1_edt, 0, DB_DEFAULT_MODE);
   ocrAddDependence(NULL_GUID, leaf1_2_edt, 0, DB_DEFAULT_MODE);
   ocrAddDependence(NULL_GUID, leaf2_1_edt, 0, DB_DEFAULT_MODE);
   ocrAddDependence(NULL_GUID, leaf2_2_edt, 0, DB_DEFAULT_MODE);

   ocrDbCreate(&db_guid, (void**) &k, sizeof(int), 0, NULL_HINT, NO_ALLOC);
   k[0] = fib(FIBN);

   ocrPrintf("End spawning_fn6(%"PRId64"), sending k=%"PRId32"\n",num,*k);
   return db_guid;
}

ocrGuid_t spawning_fn(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
   const u64 num = paramv[0];
   int* k;
#ifdef ENABLE_SPAWNING_HINT
   ocrHint_t fn6_hint;
   ocrHint_t fn3_hint;
   ocrHint_t fn2_hint;
   u64 fn6_hintVal = (u64)Spawning_fn6_PRIORITY;
   u64 fn3_hintVal = (u64)Spawning_fn3_PRIORITY;
   u64 fn2_hintVal = (u64)Spawning_fn2_PRIORITY;
   ocrHintInit(&fn6_hint, OCR_HINT_EDT_T);
   ocrSetHintValue(&fn6_hint, OCR_HINT_EDT_SPAWNING, fn6_hintVal);
   ocrHintInit(&fn3_hint, OCR_HINT_EDT_T);
   ocrSetHintValue(&fn3_hint, OCR_HINT_EDT_SPAWNING, fn3_hintVal);
   ocrHintInit(&fn2_hint, OCR_HINT_EDT_T);
   ocrSetHintValue(&fn2_hint, OCR_HINT_EDT_SPAWNING, fn2_hintVal);
#endif
   ocrPrintf("Start spawning_fn(%"PRId64")\n", num);
   ocrGuid_t db_guid, leaf1_template, leaf2_template;
   ocrGuid_t edt2_template, edt3_template, edt6_template;
   ocrGuid_t edts[num];
   ocrGuid_t leaf1_edt[num], leaf2_edt[num];
   //Create two leaf EDTs
   //Create templates for the EDTs
   ocrEdtTemplateCreate(&leaf1_template, leaf1, 0, 1);
   ocrEdtTemplateCreate(&leaf2_template, leaf2, 0, 1);
   ocrEdtTemplateCreate(&edt2_template, spawning_fn2, 1, 1);
   ocrEdtTemplateCreate(&edt3_template, spawning_fn3, 1, 1);
   ocrEdtTemplateCreate(&edt6_template, spawning_fn6, 1, 1);
   u64 i;
   for(i=0; i < num; i++){
      if       (i%6 == 0) {
#ifdef ENABLE_SPAWNING_HINT
         ocrEdtCreate(&edts[i], edt6_template, EDT_PARAM_DEF, &i, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &fn6_hint, NULL);
#else
         ocrEdtCreate(&edts[i], edt6_template, EDT_PARAM_DEF, &i, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, NULL_HINT, NULL);
#endif
         ocrAddDependence(NULL_GUID, edts[i], 0, DB_DEFAULT_MODE);
      } else if(i%3 == 0) {
#ifdef ENABLE_SPAWNING_HINT
         ocrEdtCreate(&edts[i], edt3_template, EDT_PARAM_DEF, &i, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &fn3_hint, NULL);
#else
         ocrEdtCreate(&edts[i], edt3_template, EDT_PARAM_DEF, &i, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, NULL_HINT, NULL);
#endif
         ocrAddDependence(NULL_GUID, edts[i], 0, DB_DEFAULT_MODE);
      } else if(i%2 == 0) {
#ifdef ENABLE_SPAWNING_HINT
         ocrEdtCreate(&edts[i], edt2_template, EDT_PARAM_DEF, &i, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &fn2_hint, NULL);
#else
         ocrEdtCreate(&edts[i], edt2_template, EDT_PARAM_DEF, &i, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, NULL_HINT, NULL);
#endif
         ocrAddDependence(NULL_GUID, edts[i], 0, DB_DEFAULT_MODE);
      } else {
         ocrEdtCreate(&leaf1_edt[i], leaf1_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, NULL_HINT, NULL);
         ocrEdtCreate(&leaf2_edt[i], leaf2_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, NULL_HINT, NULL);

         //Start execution of the parallel EDTs
         ocrAddDependence(NULL_GUID, leaf1_edt[i], 0, DB_DEFAULT_MODE);
         ocrAddDependence(NULL_GUID, leaf2_edt[i], 0, DB_DEFAULT_MODE);
      }
   }

   ocrDbCreate(&db_guid, (void**) &k, sizeof(int), 0, NULL_HINT, NO_ALLOC);
   k[0] = fib(FIBN);

   ocrPrintf("End spawning_fn(%"PRId64"), sending k=%"PRId32"\n",num,*k);
   return db_guid;
   //return NULL_GUID;
}

ocrGuid_t shutdownEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
   ocrPrintf("Hello from shutdownEdt\n");
   ocrPrintf("--->  depc is: %"PRId32"\n",depc);
   ocrPrintf("--->  paramc is: %"PRId32"\n",paramc);
   ocrPrintf("--->  NUM_CHILDREN is: %"PRId32"\n",NUM_CHILDREN);
   int* data ;
   int i;

   for(i=0; i < depc; i++){
      ocrDbDestroy(depv[i].guid);
   }
   diff = clock() - start_time;
   int msec = diff*1000 / CLOCKS_PER_SEC;
   int sec  = diff / CLOCKS_PER_SEC;
   ocrPrintf("Time taken %d seconds, %d ms, diff is: %d\n", sec, msec, diff);
   ocrShutdown();
   return NULL_GUID;
}


ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
   start_time = clock();
   int i;
   ocrPrintf("Starting mainEdt\n");
   ocrGuid_t edtJoin, edt_template, edtJoin_template;
   ocrGuid_t edts[NUM_CHILDREN],  outputEvents[NUM_CHILDREN];
   u64 hintVal = 0;
#ifdef ENABLE_SPAWNING_HINT
   ocrPrintf(">>>SPAWNING_HINT enabled<<<\n");
   hintVal = Spawning_fn_PRIORITY;
   ocrHint_t spawnHints;
   ocrHintInit(&spawnHints, OCR_HINT_EDT_T);
   ocrSetHintValue(&spawnHints, OCR_HINT_EDT_SPAWNING, hintVal);
#endif

   //Create templates for the EDTs
   ocrEdtTemplateCreate(&edt_template, spawning_fn, 1, 1);
   ocrEdtTemplateCreate(&edtJoin_template, shutdownEdt, 0, NUM_CHILDREN);

   ocrEdtCreate(&edtJoin, edtJoin_template, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
         EDT_PROP_FINISH, NULL_HINT, NULL);

   for(i = 0; i < NUM_CHILDREN; i++) {
      u64 ii = i;
      ocrPrintf("EdtCreate: i is: %"PRId32"\n",i);
   //Create the EDTs
#ifdef ENABLE_SPAWNING_HINT
      ocrEdtCreate(&edts[i], edt_template, EDT_PARAM_DEF, &ii, EDT_PARAM_DEF, NULL,
           EDT_PROP_FINISH, &spawnHints, &outputEvents[i]);
#else
      ocrEdtCreate(&edts[i], edt_template, EDT_PARAM_DEF, &ii, EDT_PARAM_DEF, NULL,
           EDT_PROP_FINISH, NULL_HINT, &outputEvents[i]);
#endif
      ocrAddDependence(outputEvents[i], edtJoin, i, DB_MODE_CONST);
   }


   for(i = 0; i < NUM_CHILDREN; i++) {
      ocrPrintf("AddDep: i is: %"PRId32"\n",i);
      //Start execution of the parallel EDTs
      ocrAddDependence(NULL_GUID, edts[i], 0, DB_DEFAULT_MODE);
   }

   return NULL_GUID;
}
