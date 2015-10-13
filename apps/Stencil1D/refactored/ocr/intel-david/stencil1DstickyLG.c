/*
 *Author: Bryan Pawlowski
 *Copywrite Intel Corporation 2015
 *
 *This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 *and cannot be distributed without it. This notice cannot be removed or modified.
 */

/*
 * Stencil1D code with labeled sticky events and parallel init.
 */

#define ENABLE_EXTENSION_LABELING

#include <ocr.h>
#include <stdio.h>
#include <ocr-std.h>
#include "stencil1D.h"
#include <extensions/ocr-labeling.h>

#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG

//dss: we need to switch from compile time constants to run time parameters.  Chandra's Stencil codes do that...


typedef struct{
    ocrGuid_t wrapup;
    u64 n, m, t;
    ocrGuid_t ranges[4];
}shared_t;

typedef struct{
    u64 timestep;
    u64 mynode;
    u64 toggle;
    u64 n, m, t;
    ocrGuid_t wrapup;
    ocrGuid_t template;
    ocrGuid_t left[2], right[2];
    ocrGuid_t leftrcv[2], rightrcv[2];
} private_t;


ocrGuid_t stencilEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    /*
     *  paramv[1]:
     *      0: toggle
     *
     *  depv[4]:
     *      0: this node's data (of size M)
     *      1: this node's private block
     *      2: leftin data (single double)
     *      3: rightin data (single double)
     *
     *  1: create child
     *  2: set child dependences
     *  3: do mathematics for that timestep
     *  4: send finished work to left and right.
     */
    double * a = depv[0].ptr;

    private_t * private = depv[1].ptr;
    double * leftin = depv[2].ptr;
    double * rightin = depv[3].ptr;
    u64 i, toggle;
    toggle = 1 ^ private->toggle;
    u64 node = private->mynode;
    u64 timestep = private->timestep;
    u64 n = private->n;
    u64 m = private->m;
    u64 t = private->t;
    double * atemp = a + m;

    ocrGuid_t child;

    if(timestep > 0){                                  //event cleanup.
        if( node != 0 ) ocrEventDestroy( private->leftrcv[private->toggle] );
        if( node != (n-1) ) ocrEventDestroy( private->rightrcv[private->toggle] );
    }


    if(timestep == 0){
        //initialize step
        if(m==1) {                                              //special single datapoint code.
            if(leftin == NULL || rightin == NULL) a[0] = 1.0;       //we are right/leftmost.
                else a[0] = 0.0;
        }else{
                                                                //set leftmost
            if(node == 0) a[0] = 1.0;
            else a[0] = 0.0;

            for(i = 1; i < m - 1; i++) a[i] = 0.0;
                                                                //setrightmost
            if(node == (n-1)) a[m-1] = 1.0;


        }

        if(leftin != NULL)*leftin = a[0];
        if(rightin != NULL)*rightin = a[m-1];
    }
    if( t==0 ) return NULL_GUID;

    if(timestep < t ){
        if( timestep != 0 ){                                            //math happens here.
            if( m == 1 ) {
                if(leftin == NULL || rightin == NULL) atemp[0] = a[0];
                else atemp[0] = 0.5*a[0] + 0.25*((*leftin) + (*rightin));
            }else{
                if(leftin != NULL) atemp[0] = 0.5*a[0] + 0.25*(a[1] + (*leftin));
                else atemp[0] = a[0];

                for(i = 1; i < m - 1; i++) atemp[i] = 0.5*a[i] + 0.25*(a[i+1] + a[i-1]);

                if(rightin != NULL) atemp[m-1] = 0.5* a[m-1] + 0.25*( (*rightin) + a[m-2]);
                else atemp[m - 1] = a[m-1];
            }
            for(i = 0; i < m; i++) a[i] = atemp[i];
            if(leftin != NULL) *leftin = a[0];
            if(rightin != NULL) *rightin = a[m-1];
        }

        private->timestep++;

        if(depv[0].ptr == NULL) PRINTF("node %u problem.\n", private->mynode);

        private->toggle = toggle;

        ocrEdtCreate(   &child,
                        private->template,
                        EDT_PARAM_DEF,
                        NULL,
                        EDT_PARAM_DEF,
                        NULL,
                        EDT_PROP_NONE,
                        NULL_GUID,
                        NULL_GUID );



        if( private->mynode != 0 )//set left event stuffs.
            ocrEventCreate( &private->leftrcv[toggle],
                            OCR_EVENT_STICKY_T,
                            DEFAULT_LG_PROPS );

        ocrAddDependence(   private->leftrcv[toggle],
                            child,
                            2,
                            DB_MODE_RW );




        if( private->mynode != (n - 1) )
            ocrEventCreate( &private->rightrcv[toggle],
                            OCR_EVENT_STICKY_T,
                            DEFAULT_LG_PROPS );

        ocrAddDependence(   private->rightrcv[toggle],
                            child,
                            3,
                            DB_MODE_RW );


        if( private->mynode != 0 ){                             //send info to the left.
            ocrEventCreate( &private->left[toggle],
                            OCR_EVENT_STICKY_T,
                            DEFAULT_LG_PROPS );
            ocrDbRelease( depv[2].guid );
            ocrEventSatisfy(    private->left[toggle],
                                depv[2].guid );
        }


        if( private->mynode != n - 1 ){                         //send info to the right.

            ocrEventCreate( &private->right[toggle],
                            OCR_EVENT_STICKY_T,
                            DEFAULT_LG_PROPS );

            ocrDbRelease( depv[3].guid );

            ocrEventSatisfy(    private->right[toggle],
                                 depv[3].guid);
        }


        ocrDbRelease( depv[0].guid );
        ocrAddDependence(   depv[0].guid,
                            child,
                            0,
                            DB_MODE_RW );

        ocrDbRelease( depv[1].guid );
        ocrAddDependence(   depv[1].guid,
                            child,
                            1,
                            DB_MODE_RW );

    }else{
        ocrAddDependence(   depv[0].guid,               //add dep to wrapup.
                            private->wrapup,
                            private->mynode,
                            DB_MODE_RW );
    }
    return NULL_GUID;
}



ocrGuid_t wrapupEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    u64 i, j;
    u64 n = paramv[0];
    u64 m = paramv[1];
    u64 t = paramv[2];
    double * data[paramv[0]];

    for(i=0;i<n;i++) {
        data[i] = depv[i].ptr;
        for(j=0;j<m;j++) PRINTF("%lld %lld %f \n", i, j, data[i][j]);
        }
    if(m==50&&n==10&&t==10000) //default values for Jenkins
        if(data[4][49] - .000836 < 1e-5) PRINTF("PASS\n");
           else PRINTF("fail by %f\n", data[4][49]-.000836);

    ocrShutdown();

    return NULL_GUID;

}

ocrGuid_t stencilInitEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    /*
     * paramv[1]:
     *  0: nodeNum
     *
     * depv[2]:
     *  0: sharedBlock (this is its last initializing Edt)
     *  1: empty privateBlock
     */


    ocrGuid_t dataDb, sendleftDb, sendrightDb, temp;
    shared_t * shared = depv[0].ptr;
    private_t * private = depv[1].ptr;


    private->mynode = paramv[0];
    u64 params[2];
    u64 nodenum = paramv[0];
    u64 * dummy;
    u64 toggle = 0;


    private->wrapup = shared->wrapup;
    private->toggle = toggle;
    private->n = shared->n;
    private->m = shared->m;
    private->t = shared->t;

    ocrDbCreate(&dataDb, (void **) &dummy, 2 * shared->m * sizeof(double), 0, NULL_GUID, NO_ALLOC );    //slot 0
        ocrDbCreate(    &sendleftDb,            //slot 2
                        (void **) &dummy,
                        sizeof(double),
                        0,
                        NULL_GUID,
                        NO_ALLOC );
        ocrDbCreate(    &sendrightDb,
                        (void **)&dummy,
                        sizeof(double),
                        0,
                        NULL_GUID,
                        NO_ALLOC );

    ocrEdtTemplateCreate(   &(private->template),
                            stencilEdt,
                            0,
                            4   );

    ocrEdtCreate(   &temp,
                    private->template,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PROP_NONE,
                    NULL_GUID,
                    NULL_GUID );

    if( private->mynode < (private->n - 1)){

        ocrGuidFromIndex(   &private->right[0], //set the right-bound send.
                            shared->ranges[0],
                            nodenum );
        ocrGuidFromIndex(   &private->right[1],
                            shared->ranges[1],
                            nodenum );

        ocrGuidFromIndex(   &private->rightrcv[0],  //set right-inbound receive.
                            shared->ranges[2],
                            nodenum );

        ocrGuidFromIndex(   &private->rightrcv[1],
                            shared->ranges[3],
                            nodenum );
    }else{
        private->right[0] = NULL_GUID;
        private->right[1] = NULL_GUID;
        private->rightrcv[0] = NULL_GUID;
        private->rightrcv[1] = NULL_GUID;
    }
    if( private->mynode > 0 ){                      //if I am not the leftmost

        ocrGuidFromIndex(   &private->left[0],      //set the left-bound send.
                            shared->ranges[2],
                            nodenum - 1);
        ocrGuidFromIndex(   &private->left[1],
                            shared->ranges[3],
                            nodenum - 1);
        ocrGuidFromIndex(   &private->leftrcv[0],   //set the left-inbound receive.
                            shared->ranges[0],
                            nodenum - 1);
        ocrGuidFromIndex(   &private->leftrcv[1],
                            shared->ranges[1],
                            nodenum - 1);

    }else{
        private->left[0] = NULL_GUID;
        private->left[1] = NULL_GUID;
        private->leftrcv[0] = NULL_GUID;
        private->leftrcv[1] = NULL_GUID;
    }

    private->timestep = 0;

    ocrDbRelease( dataDb );
    ocrAddDependence(   dataDb,
                        temp,
                        0,
                        DB_MODE_RW );

    ocrDbRelease( depv[1].guid );
    ocrAddDependence(   depv[1].guid,
                        temp,
                        1,
                        DB_MODE_RW );

    ocrDbRelease( sendleftDb );
    ocrAddDependence(   sendleftDb,
                        temp,
                        2,
                        DB_MODE_RW );

    ocrDbRelease( sendrightDb );
    ocrAddDependence(   sendrightDb,
                        temp,
                        3,
                        DB_MODE_RW );


    return NULL_GUID;
}

ocrGuid_t initEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    /*
     * paramv[0]:
     *
     *
     * depv[1]:
     *  0: shared block.
     *
     *
     * Create private Db
     * wake up stencilInitEdt when done.
     */

    u64 * dummy;
    ocrGuid_t temp, SITemplate;
    shared_t * shared = depv[0].ptr;
    u64 i = paramv[0];

    ocrGuid_t privateDb;

    ocrDbCreate(    &privateDb,
                    (void **)&dummy,
                    sizeof(private_t),
                    0,
                    NULL_GUID,
                    NO_ALLOC );

    ocrEdtTemplateCreate(   &SITemplate,
                            stencilInitEdt,
                            1,
                            2 );

    ocrEdtCreate(   &temp,
                    SITemplate,
                    EDT_PARAM_DEF,
                    &paramv[0],
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PROP_NONE,
                    NULL_GUID,
                    NULL_GUID );

    ocrAddDependence(   depv[0].guid,
                        temp,
                        0,
                        DB_MODE_RW );

    ocrAddDependence(   privateDb,
                        temp,
                        1,
                        DB_MODE_RW );


    return NULL_GUID;
}

ocrGuid_t realmainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{


    /*
     * paramv[3]:
     *  0: numTasks
     *  1: chunkSize
     *  2: timesteps
     *
     * depv[1]:
     *  0: empty shared block
     *
     * initializes shared block
     * pastes shared block into each init subtask
     */


    u64 i;

    ocrGuid_t initTemplate, init;
    ocrGuid_t ranges[4];
    ocrGuid_t wrapupTemplate, wrapup;
    shared_t * shared = depv[0].ptr;

    shared->n = paramv[0];
    shared->m = paramv[1];
    shared->t = paramv[2];

    for(i = 0; i < 4; i++){

        ocrGuidRangeCreate( &(shared->ranges[i]),
                            shared->n - 1,
                            GUID_USER_EVENT_STICKY );
    }


    ocrEdtTemplateCreate(   &wrapupTemplate,            //params/deps for wrapup will be zero for now;
                            wrapupEdt,                  //this will change.
                            3,
                            shared->n );

    ocrEdtCreate(   &shared->wrapup,
                    wrapupTemplate,
                    EDT_PARAM_DEF,
                    paramv,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PROP_NONE,
                    NULL_GUID,
                    NULL_GUID );

    ocrEdtTemplateCreate(   &initTemplate,
                            initEdt,
                            1,
                            1 );

    for( i = 0; i < shared->n; i++ ){


        ocrEdtCreate(   &init,
                        initTemplate,
                        EDT_PARAM_DEF,
                        &i,
                        EDT_PARAM_DEF,
                        &depv[0].guid,
                        EDT_PROP_NONE,
                        NULL_GUID,
                        NULL_GUID );

    }



    return NULL_GUID;
}


ocrGuid_t mainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{


    /*
     * Create the shared datablock
     * create realmain
     * launch realmain
     */


    u64 i, *dummy;

    ocrGuid_t realMainTemplate, realMain;
    ocrGuid_t sharedDb;
    u64 numNodes, chunkSize, timesteps;
    u64 params[3];

    void* programArgv = depv[0].ptr;
    u32 argc = getArgc( programArgv );

    PRINTF("STENCIL 1D PARALLEL INIT WITH LABELED STICKIES.\n");

    if(argc != 4){
        PRINTF( "INCORRECT NUMBER OF ARGS. USING DEFAULT PARAMS. %s\n", getArgv(programArgv, 0) );
        numNodes = N;
        chunkSize = M;
        timesteps = T;
    }else{
        numNodes = (u64)atoi(getArgv(programArgv, 1));
        chunkSize = (u64)atoi(getArgv(programArgv, 2));
        timesteps = (u64)atoi(getArgv(programArgv, 3));
    }
    PRINTF("N = %u\nM = %u\nT = %u\n", numNodes, chunkSize, timesteps);

    params[0] = numNodes;
    params[1] = chunkSize;
    params[2] = timesteps;

    ocrDbCreate( &sharedDb, (void **)&dummy, sizeof(shared_t), 0, NULL_GUID, NO_ALLOC );
    ocrEdtTemplateCreate(   &realMainTemplate,
                            realmainEdt,
                            3,
                            1 );

    ocrEdtCreate(   &realMain,
                    realMainTemplate,
                    EDT_PARAM_DEF,
                    params,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PROP_NONE,
                    NULL_GUID,
                    NULL_GUID );

    ocrAddDependence(   sharedDb,
                        realMain,
                        0,
                        DB_MODE_RW );

    return NULL_GUID;

}
