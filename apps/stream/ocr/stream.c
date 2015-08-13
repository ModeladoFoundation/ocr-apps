/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */
/* Based on stream benchmark at https://www.cs.virginia.edu/stream/FTP/Code/stream.c */

#define _GNU_SOURCE
#include "ocr.h"
#include <stdlib.h>
#include <stdio.h>

# include <math.h>
# include <float.h>
#include "pthread.h"

#define STREAM_ARRAY_SIZE 45000//12000000
#define NUM_THREADS  6//12

#define FLAGS DB_PROP_NONE
#define PROPERTIES EDT_PROP_NONE

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif

#ifndef STREAM_TYPE
#define STREAM_TYPE double
#endif

#define MOD NUM_THREADS
#define NTIMES 40
//#include "numaif.h" //Used for next touch policy

STREAM_TYPE scalar = 3.0;
static double   avgtime[4] = {0}, maxtime[4] = {0},
        mintime[4] = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};

static u64 lastSeen[4] = {0, 0, 0, 0};
static double  t_d_copy, t_d_scale, t_d_add, t_d_triad;

static double times[4];

static double   bytes[4] = {
    2 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
    2 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
    3 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
    3 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE
    };

static char *label[4] = {"Copy:      ", "Scale:     ",
    "Add:       ", "Triad:     "};

#include <sys/time.h>

static int iteration = 0;

double mysecond()
{
        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

static ocrGuid_t templateInit, templateCopy, templateScale, templateAdd, templateTriad, templateWrapInit, templateWrapCopy, templateWrapScale, templateWrapAdd, templateWrapTriad;
static ocrGuid_t template_copy_prescriber, template_scale_prescriber, template_add_prescriber, template_triad_prescriber;
static  ocrGuid_t db_guid_a, db_guid_b, db_guid_c;

ocrGuid_t event_guid_iteration[NTIMES+1];

ocrGuid_t event_guid_wrap_init[NUM_THREADS];
ocrGuid_t event_guid_wrap_copy[NUM_THREADS];
ocrGuid_t event_guid_wrap_scale[NUM_THREADS];
ocrGuid_t event_guid_wrap_add[NUM_THREADS];
ocrGuid_t event_guid_wrap_triad[NUM_THREADS];

ocrGuid_t event_guid_copy_prescriber[NTIMES];
ocrGuid_t event_guid_scale_prescriber[NTIMES];
ocrGuid_t event_guid_add_prescriber[NTIMES];
ocrGuid_t event_guid_triad_prescriber[NTIMES];

ocrGuid_t wrap_init_task( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){

    ocrGuid_t db_guid;
    void * temp;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                         FLAGS, NULL_GUID, NO_ALLOC);
    ocrEventSatisfy(event_guid_iteration[0], db_guid);
}

ocrGuid_t wrap_copy_task( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
    if(iteration>=0){
        times[0] = mysecond() - times[0]; 
        mintime[0] = MIN(mintime[0], times[0]);
    }

    ocrGuid_t db_guid;
    void * temp;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                         FLAGS, NULL_GUID, NO_ALLOC);
    ocrEventSatisfy(event_guid_scale_prescriber[iteration], db_guid);
}


ocrGuid_t wrap_scale_task( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
    if(iteration>=0){
        times[1] = mysecond() - times[1];
        mintime[1] = MIN(mintime[1], times[1]);
    }

    ocrGuid_t db_guid;
    void * temp;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                         FLAGS, NULL_GUID, NO_ALLOC);
    ocrEventSatisfy(event_guid_add_prescriber[iteration], db_guid);

}


ocrGuid_t wrap_add_task(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
    if(iteration>=0){
        times[2] = mysecond() - times[2];
        mintime[2] = MIN(mintime[2], times[2]);
    }

    ocrGuid_t db_guid;
    void * temp;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                         FLAGS, NULL_GUID, NO_ALLOC);
    ocrEventSatisfy(event_guid_triad_prescriber[iteration], db_guid);

}

ocrGuid_t wrap_triad_task( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
    if(iteration>=0){
        times[3] = mysecond() - times[3];
        mintime[3] = MIN(mintime[3], times[3]);
    }
    if(iteration%500==0)PRINTF("\nDone with Triad (iteration=%d)",iteration);

    int j;
   if(iteration>=0)
    {
    for (j=0; j<4; j++)
        {
        avgtime[j] = avgtime[j] + times[j];
        mintime[j] = MIN(mintime[j], times[j]);
        maxtime[j] = MAX(maxtime[j], times[j]);
        }
    }

    if(iteration==(NTIMES-1)) {
        printf("Function    Best Rate MB/s  Avg time     Min time     Max time\n");
        for (j=0; j<4; j++) {
            avgtime[j] = avgtime[j]/(double)(NTIMES);

            printf("%s%12.1f  %11.6f  %11.6f  %11.6f\n", label[j],
               1.0E-06 * bytes[j]/mintime[j],
               avgtime[j],
               mintime[j],
              maxtime[j]);

        }
        PRINTF("Time for %d chunk of stream_copy=%11.6f", NUM_THREADS, (NUM_THREADS*t_d_copy)/(NTIMES));
        PRINTF("\nTime for %d chunk of stream_scale=%11.6f", NUM_THREADS, (NUM_THREADS*t_d_scale)/(NTIMES));
        PRINTF("\nTime for %d chunk of stream_add=%11.6f", NUM_THREADS, (NUM_THREADS*t_d_add)/(NTIMES));
        PRINTF("\nTime for %d chunk of stream_triad=%11.6f", NUM_THREADS, (NUM_THREADS*t_d_triad)/(NTIMES));
        ocrShutdown();
    }
    else{
        ocrGuid_t db_guid;
        void * temp;
        ocrDbCreate(&db_guid, &temp, sizeof(double),
                    FLAGS, NULL_GUID, NO_ALLOC);
        ocrEventSatisfy(event_guid_iteration[++iteration], db_guid);
        //usleep(15*1000);
    }
}


int SIZE = STREAM_ARRAY_SIZE/NUM_THREADS;


ocrGuid_t stream_init( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){

    u64 *func_args = paramv;
    u64 startIndex = (u64) func_args[0];
    u64 endIndex = (u64) func_args[1];

    int satisfy = (int)(startIndex/SIZE);

    double* a = (double*) depv[0].ptr;
    double* b = (double*) depv[1].ptr;
    double* c = (double*) depv[2].ptr;

    int j;
    for (j=startIndex; j<endIndex; j++)
    {
        a[j] = 1.0;
        b[j] = 2.0;
        c[j] = 0.0;
    }

    ocrGuid_t db_guid;
    void * temp;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                         FLAGS, NULL_GUID, NO_ALLOC);
/*    if(satisfy!=sched_getcpu())
        PRINTF("\nViolation!");
*/
    //PRINTF("\nSatisfy(copy)=%d on CPU #%d", satisfy, sched_getcpu());
    ocrEventSatisfy(event_guid_wrap_init[satisfy], db_guid);
}

ocrGuid_t stream_copy( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){

    u64 *func_args = paramv;
    u64 startIndex = (u64) func_args[0];
    u64 endIndex = (u64) func_args[1];

    int satisfy = (int)(startIndex/SIZE);
    
    double* a = (double*) depv[0].ptr;
    double* b = (double*) depv[1].ptr;
    double* c = (double*) depv[2].ptr;
/*
    int node =0;
    int cpu = sched_getcpu();
    if((cpu>5&&cpu<12)||(cpu>17&&cpu<23))
        node = 1;
    long count = 1;
    int nodes[count];
    void* addr[count];
    int ii;
    int status[count];

    void * ptr_to_check = &a[startIndex];
    addr[0] = ptr_to_check;
    long ret = move_pages(0,count, (void**)addr,NULL,status,0);

    if(ret==0 && status[0]!=node)
        PRINTF("\nNot local, %d, %d", status[0], node);
*/
    int j;
    double time_d = mysecond();
    for (j=startIndex; j<endIndex; j++){
        c[j] = a[j]*scalar;
        c[j] += a[j];
    }
//    PRINTF("Time for 1 chunk of stream_copy=%11.6f", (mysecond()-time_d));
    if(lastSeen[0]!=iteration){
        lastSeen[0] = iteration;
        t_d_copy+=(mysecond()-time_d);
    }

    ocrGuid_t db_guid;    
    void * temp;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                         FLAGS, NULL_GUID, NO_ALLOC);
    temp = &c[0];
/*    if(satisfy!=sched_getcpu())
        PRINTF("\nViolation!");
*/
    //PRINTF("\nSatisfy(copy)=%d on CPU #%d", satisfy, sched_getcpu());
    ocrEventSatisfy(event_guid_wrap_copy[satisfy], db_guid);
}


ocrGuid_t stream_scale( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
    u64 *func_args = paramv;
    u64 startIndex = (u64) func_args[0];
    u64 endIndex = (u64) func_args[1];
    int satisfy = (int)(startIndex/SIZE);

    double* a = (double*) depv[0].ptr;
    double* b = (double*) depv[1].ptr;
    double* c = (double*) depv[2].ptr;
    int j;
    
    double time_d = mysecond();

    for (j=startIndex; j<endIndex; j++)
        b[j] = scalar*c[j];

  
     if(lastSeen[1]!=iteration){
        lastSeen[1] = iteration;
        t_d_scale+=(mysecond()-time_d);
    }


    ocrGuid_t db_guid;
    void * temp;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                         FLAGS, NULL_GUID, NO_ALLOC);
    ocrEventSatisfy(event_guid_wrap_scale[satisfy], db_guid);

}

ocrGuid_t stream_add( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
    u64 *func_args = paramv;
    u64 startIndex = (u64) func_args[0];
    u64 endIndex = (u64) func_args[1];
    int satisfy = (int)(startIndex/SIZE);

    double* a = (double*) depv[0].ptr;
    double* b = (double*) depv[1].ptr;
    double* c = (double*) depv[2].ptr;

    double time_d = mysecond();
    int j;
    for (j=startIndex;j<endIndex; j++)
        c[j] = a[j]+b[j];

  if(lastSeen[2]!=iteration){
        lastSeen[2] = iteration;
        t_d_add+=(mysecond()-time_d);
    }

    ocrGuid_t db_guid;
    void * temp;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                         FLAGS, NULL_GUID, NO_ALLOC);
    ocrEventSatisfy(event_guid_wrap_add[satisfy], db_guid);

}

ocrGuid_t stream_triad( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
    u64 *func_args = paramv;
    u64 startIndex = (u64) func_args[0];
    u64 endIndex = (u64) func_args[1];
    int satisfy = (int)(startIndex/SIZE);

    double* a = (double*) depv[0].ptr;
    double* b = (double*) depv[1].ptr;
    double* c = (double*) depv[2].ptr;

    double time_d = mysecond();
    int j;
    for (j=startIndex; j<endIndex; j++)
        a[j] = b[j]+scalar*c[j];

      if(lastSeen[3]!=iteration){
        lastSeen[3] = iteration;
        t_d_triad+=(mysecond()-time_d);
    }

    ocrGuid_t db_guid;

    void * temp;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                         FLAGS, NULL_GUID, NO_ALLOC);
    ocrEventSatisfy(event_guid_wrap_triad[satisfy], db_guid);
}

ocrGuid_t init_prescriber(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
    u64 i, startIndex, endIndex;
    ocrGuid_t affinity = NULL_GUID;
    times[0] = mysecond();
    for(i=0; i<STREAM_ARRAY_SIZE;i+=SIZE){
        startIndex = i;
        endIndex = i+SIZE;
        int satisfy = (int)(startIndex/SIZE);
        u64 func_args[2];
        func_args[0] = startIndex;
        func_args[1] = endIndex;

        ocrGuid_t init_guid;
        ocrGuid_t event_guid_a, event_guid_b, event_guid_c;
        u64 satisfy64 = (startIndex/SIZE);
        ocrEdtCreate/*Aff*/(&init_guid, templateInit, 2, func_args, 3, NULL, PROPERTIES, affinity, NULL);//, satisfy64%MOD);

        ocrEventCreate(&event_guid_a, OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&event_guid_b, OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&event_guid_c, OCR_EVENT_STICKY_T, TRUE);

        ocrAddDependence(event_guid_a, init_guid, 0, DB_MODE_ITW);
        ocrAddDependence(event_guid_b, init_guid, 1, DB_MODE_ITW);
        ocrAddDependence(event_guid_c, init_guid, 2, DB_MODE_ITW);

        //satisfy event
        ocrEventSatisfy(event_guid_a, db_guid_a);
        ocrEventSatisfy(event_guid_b, db_guid_b);
        ocrEventSatisfy(event_guid_c, db_guid_c);
    }
}

ocrGuid_t copy_prescriber(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
    u64 i, startIndex, endIndex;
    ocrGuid_t affinity = NULL_GUID;
    times[0] = mysecond();
    for(i=0; i<STREAM_ARRAY_SIZE;i+=SIZE){
        startIndex = i;
        endIndex = i+SIZE;
        int satisfy = (int)(startIndex/SIZE);
        u64 func_args[2];
        func_args[0] = startIndex;
        func_args[1] = endIndex;

        ocrGuid_t copy_guid;
        ocrGuid_t event_guid_a, event_guid_b, event_guid_c;
        u64 satisfy64 = (startIndex/SIZE);
        ocrEdtCreate/*Aff*/(&copy_guid, templateCopy, 2, func_args, 3, NULL, PROPERTIES, affinity, NULL);//, satisfy64%MOD);

        ocrEventCreate(&event_guid_a, OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&event_guid_b, OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&event_guid_c, OCR_EVENT_STICKY_T, TRUE);

        ocrAddDependence(event_guid_a, copy_guid, 0, DB_MODE_ITW);
        ocrAddDependence(event_guid_b, copy_guid, 1, DB_MODE_ITW);
        ocrAddDependence(event_guid_c, copy_guid, 2, DB_MODE_ITW);
    
        //satisfy event
        ocrEventSatisfy(event_guid_a, db_guid_a);
        ocrEventSatisfy(event_guid_b, db_guid_b);
        ocrEventSatisfy(event_guid_c, db_guid_c);
    }
}

#define PUSH_FACTOR 0 
ocrGuid_t scale_prescriber(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
    u64 i, startIndex, endIndex;
    ocrGuid_t affinity = NULL_GUID;
    times[1] = mysecond();
    for(i=0; i<STREAM_ARRAY_SIZE;i+=SIZE){
        startIndex = i;//%STREAM_ARRAY_SIZE;
        endIndex = (i+SIZE);//%STREAM_ARRAY_SIZE;
        int satisfy = (int)(startIndex/SIZE);
        u64 func_args[2];
        func_args[0] = startIndex;
        func_args[1] = endIndex;

        ocrGuid_t scale_guid;
        ocrGuid_t event_guid_a, event_guid_b, event_guid_c;
        u64 satisfy64 = (startIndex/SIZE);
        ocrEdtCreate/*Aff*/(&scale_guid, templateScale, 2, func_args, 3, NULL, PROPERTIES, affinity, NULL);//, satisfy64%MOD);

        ocrEventCreate(&event_guid_a, OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&event_guid_b, OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&event_guid_c, OCR_EVENT_STICKY_T, TRUE);

        ocrAddDependence(event_guid_a, scale_guid, 0, DB_MODE_ITW);
        ocrAddDependence(event_guid_b, scale_guid, 1, DB_MODE_ITW);
        ocrAddDependence(event_guid_c, scale_guid, 2, DB_MODE_ITW);
        
        //satisfy event
        ocrEventSatisfy(event_guid_a, db_guid_a);
        ocrEventSatisfy(event_guid_b, db_guid_b);
        ocrEventSatisfy(event_guid_c, db_guid_c);
    }


}

ocrGuid_t add_prescriber(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
    u64 i, startIndex, endIndex;
    ocrGuid_t affinity = NULL_GUID;
    times[2] = mysecond();
    for(i=0; i<STREAM_ARRAY_SIZE;i+=SIZE){
        startIndex = i;
        endIndex = (i+SIZE);
        int satisfy = (int)(startIndex/SIZE);
        u64 func_args[2];
        func_args[0] = startIndex;
        func_args[1] = endIndex;

        ocrGuid_t add_guid;
        ocrGuid_t event_guid_a, event_guid_b, event_guid_c;
        u64 satisfy64 = (startIndex/SIZE);
        ocrEdtCreate/*Aff*/(&add_guid, templateAdd, 2, func_args, 3, NULL, PROPERTIES, affinity, NULL);//, satisfy64%MOD);

        ocrEventCreate(&event_guid_a, OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&event_guid_b, OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&event_guid_c, OCR_EVENT_STICKY_T, TRUE);

        ocrAddDependence(event_guid_a, add_guid, 0, DB_MODE_ITW);
        ocrAddDependence(event_guid_b, add_guid, 1, DB_MODE_ITW);
        ocrAddDependence(event_guid_c, add_guid, 2, DB_MODE_ITW);
        
        //satisfy event
        ocrEventSatisfy(event_guid_a, db_guid_a);
        ocrEventSatisfy(event_guid_b, db_guid_b);
        ocrEventSatisfy(event_guid_c, db_guid_c);
    }

}

ocrGuid_t triad_prescriber(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
    u64 i, startIndex, endIndex;
    ocrGuid_t affinity = NULL_GUID;
    times[3] = mysecond();
    for(i=0; i<STREAM_ARRAY_SIZE;i+=SIZE){
        startIndex = i;
        endIndex = (i+SIZE);
        int satisfy = (int)(startIndex/SIZE);
        u64 func_args[2];
        func_args[0] = startIndex;
        func_args[1] = endIndex;

        ocrGuid_t triad_guid;
        ocrGuid_t event_guid_a, event_guid_b, event_guid_c;
        u64 satisfy64 = (startIndex/SIZE);
        ocrEdtCreate/*Aff*/(&triad_guid, templateTriad, 2, func_args, 3, NULL, PROPERTIES, affinity, NULL);//, satisfy64%MOD);

        ocrEventCreate(&event_guid_a, OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&event_guid_b, OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&event_guid_c, OCR_EVENT_STICKY_T, TRUE);

        ocrAddDependence(event_guid_a, triad_guid, 0, DB_MODE_ITW);
        ocrAddDependence(event_guid_b, triad_guid, 1, DB_MODE_ITW);
        ocrAddDependence(event_guid_c, triad_guid, 2, DB_MODE_ITW);
    
        //satisfy event
        ocrEventSatisfy(event_guid_a, db_guid_a);
        ocrEventSatisfy(event_guid_b, db_guid_b);
        ocrEventSatisfy(event_guid_c, db_guid_c);
    }

}

ocrGuid_t start_iteration(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){

    int i;

            ocrGuid_t affinity = NULL_GUID;

            ocrGuid_t wrap_copy_guid, wrap_scale_guid, wrap_add_guid, wrap_triad_guid;
            ocrEdtCreate(&wrap_copy_guid, templateWrapCopy, 0, NULL, NUM_THREADS, NULL, PROPERTIES, affinity, NULL);
            ocrEdtCreate(&wrap_scale_guid, templateWrapScale, 0, NULL, NUM_THREADS, NULL, PROPERTIES, affinity, NULL);
            ocrEdtCreate(&wrap_add_guid, templateWrapAdd, 0, NULL, NUM_THREADS, NULL, PROPERTIES, affinity, NULL);
            ocrEdtCreate(&wrap_triad_guid, templateWrapTriad, 0, NULL, NUM_THREADS, NULL, PROPERTIES, affinity, NULL);
            for(i=0;i<NUM_THREADS;i++){

                event_guid_wrap_copy[i] = NULL_GUID;
                ocrEventCreate(&event_guid_wrap_copy[i], OCR_EVENT_STICKY_T, TRUE);
                ocrAddDependence(event_guid_wrap_copy[i], wrap_copy_guid, i, DB_MODE_ITW);
    
                event_guid_wrap_scale[i] = NULL_GUID;
                ocrEventCreate(&event_guid_wrap_scale[i], OCR_EVENT_STICKY_T, TRUE);
                ocrAddDependence(event_guid_wrap_scale[i], wrap_scale_guid, i, DB_MODE_ITW);
                
                event_guid_wrap_add[i] = NULL_GUID;
                ocrEventCreate(&event_guid_wrap_add[i], OCR_EVENT_STICKY_T, TRUE);
                ocrAddDependence(event_guid_wrap_add[i], wrap_add_guid, i, DB_MODE_ITW);
                    
                event_guid_wrap_triad[i] = NULL_GUID;
                ocrEventCreate(&event_guid_wrap_triad[i], OCR_EVENT_STICKY_T, TRUE);
                ocrAddDependence(event_guid_wrap_triad[i], wrap_triad_guid, i, DB_MODE_ITW);
                
        }
 
//COPY
//copy_prescriber
    ocrGuid_t copy_prescriber_guid;
    ocrEdtTemplateCreate(&template_copy_prescriber, copy_prescriber, 0, 1);
    ocrEdtCreate(&copy_prescriber_guid, template_copy_prescriber, 0, NULL, 1, NULL, PROPERTIES, affinity, NULL);

    ocrAddDependence(event_guid_copy_prescriber[iteration], copy_prescriber_guid, 0, DB_MODE_ITW);


    ocrGuid_t db_guid;
    void * temp;
    ocrDbCreate(&db_guid, &temp, sizeof(double),
                                         FLAGS, NULL_GUID, NO_ALLOC);
    ocrEventSatisfy(event_guid_copy_prescriber[iteration], db_guid);

//SCALE

    ocrGuid_t scale_prescriber_guid;
    ocrEdtTemplateCreate(&template_scale_prescriber, scale_prescriber, 0, 1);
    ocrEdtCreate(&scale_prescriber_guid, template_scale_prescriber, 0, NULL, 1, NULL, PROPERTIES, affinity, NULL);

    ocrAddDependence(event_guid_scale_prescriber[iteration], scale_prescriber_guid, 0, DB_MODE_ITW);

//ADD
    ocrGuid_t add_prescriber_guid;
    ocrEdtTemplateCreate(&template_add_prescriber, add_prescriber, 0, 1);
    ocrEdtCreate(&add_prescriber_guid, template_add_prescriber, 0, NULL, 1, NULL, PROPERTIES, affinity, NULL);

    ocrAddDependence(event_guid_add_prescriber[iteration], add_prescriber_guid, 0, DB_MODE_ITW);


//TRIAD
    ocrGuid_t triad_prescriber_guid;
    ocrEdtTemplateCreate(&template_triad_prescriber, triad_prescriber, 0, 1);
    ocrEdtCreate(&triad_prescriber_guid, template_triad_prescriber, 0, NULL, 1, NULL, PROPERTIES, affinity, NULL);

    ocrAddDependence(event_guid_triad_prescriber[iteration], triad_prescriber_guid, 0, DB_MODE_ITW);

}


ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {

    double start_main = mysecond();
    
    u64 i;
    u64 argc;

    void *programArg = depv[0].ptr;
    argc = getArgc(programArg);

    char *nparamv[argc];

    ocrGuid_t template_start_iteration, start_iteration_guid;

    ocrEdtTemplateCreate(&template_start_iteration, start_iteration, 0, EDT_PARAM_UNK);

    ocrEdtTemplateCreate(&templateInit, stream_init, 2, 3);
    ocrEdtTemplateCreate(&templateCopy, stream_copy, 2, 3); 
    ocrEdtTemplateCreate(&templateScale, stream_scale, 2, 3);
    ocrEdtTemplateCreate(&templateAdd, stream_add, 2, 3);
    ocrEdtTemplateCreate(&templateTriad, stream_triad, 2, 3);

    ocrEdtTemplateCreate(&templateWrapInit, wrap_init_task, 0, NUM_THREADS);
    ocrEdtTemplateCreate(&templateWrapCopy, wrap_copy_task, 0, NUM_THREADS);
    ocrEdtTemplateCreate(&templateWrapScale, wrap_scale_task, 0, NUM_THREADS);
    ocrEdtTemplateCreate(&templateWrapAdd, wrap_add_task, 0, NUM_THREADS);
    ocrEdtTemplateCreate(&templateWrapTriad, wrap_triad_task, 0, NUM_THREADS);

    void *a;
    void *b;
    void *c;

    ocrGuid_t db_affinity;
    ocrDbCreate(&db_guid_a, &a, sizeof(double)*STREAM_ARRAY_SIZE,
                             FLAGS, db_affinity, NO_ALLOC);

    ocrDbCreate(&db_guid_b, &b, sizeof(double)*STREAM_ARRAY_SIZE,
                             FLAGS, db_affinity, NO_ALLOC);

    ocrDbCreate(&db_guid_c, &c, sizeof(double)*STREAM_ARRAY_SIZE,
                             FLAGS, db_affinity, NO_ALLOC);

    int k;
    ocrGuid_t affinity = NULL_GUID;
//Init
    for (k=0; k<NTIMES; k++){
        ocrEventCreate(&(event_guid_copy_prescriber[k]), OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&(event_guid_scale_prescriber[k]), OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&(event_guid_add_prescriber[k]), OCR_EVENT_STICKY_T, TRUE);
        ocrEventCreate(&(event_guid_triad_prescriber[k]), OCR_EVENT_STICKY_T, TRUE);
   }

    k = 0;
    for (k=0; k<NTIMES; k++){
        ocrEventCreate(&event_guid_iteration[k], OCR_EVENT_STICKY_T, TRUE);
        ocrEdtCreate(&start_iteration_guid, template_start_iteration, 0, NULL, k+1, NULL, PROPERTIES, affinity, NULL);
        int j;
        for(j=0;j<k+1;j++){
             ocrAddDependence(event_guid_iteration[j], start_iteration_guid, j, DB_MODE_ITW);
        }

    }


    ocrGuid_t wrap_init_guid;
    ocrEdtCreate(&wrap_init_guid, templateWrapInit, 0, NULL, NUM_THREADS, NULL, PROPERTIES, affinity, NULL);
    for(i=0;i<NUM_THREADS;i++){
        event_guid_wrap_init[i] = NULL_GUID;
        ocrEventCreate(&event_guid_wrap_init[i], OCR_EVENT_STICKY_T, TRUE);
        ocrAddDependence(event_guid_wrap_init[i], wrap_init_guid, i, DB_MODE_ITW);
    }

    ocrGuid_t init_prescriber_guid, template_init_prescriber;
    ocrEdtTemplateCreate(&template_init_prescriber, init_prescriber, 0, 0);
    ocrEdtCreate(&init_prescriber_guid, template_init_prescriber, 0, NULL, 0, NULL, PROPERTIES, affinity, NULL);


//Start
    PRINTF("\nStarting Parallel portion %11.6f\n", mysecond()-start_main);

    return NULL_GUID;
}

