/*
 *Author: Bryan Pawlowski
 *Copywrite Intel Corporation 2015
 *
 *This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 *and cannot be distributed without it. This notice cannot be removed or modified.
 */

/*
 * Stencil1D code with once events. Parallelized Init.
 */

#define ENABLE_EXTENSION_LABELING

#include <ocr.h>
#include <stdio.h>
#include <ocr-std.h>
#include "stencil1D.h"
#include <extensions/ocr-labeling.h>

#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG

typedef struct{
    ocrGuid_t wrapup;
    ocrGuid_t ranges[2];
    u64 n, m, t;
}shared_t;

typedef struct{
    u64 timestep;
    u64 mynode;
    u64 n, m, t;
    ocrGuid_t wrapup;
    ocrGuid_t template;
    ocrGuid_t mychild;
    ocrGuid_t toDestroy[2];
} private_t;

ocrGuid_t stencilEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    /*
     * paramv[0]:
     *
     * depv[4]:
     *  0: datablock for node.
     *  1: node's private block.
     *  2: leftin block/leftsend block.
     *  3: rightin block/rightsend block.
     *
     *
     * math for the stencil is done here
     * create grandchild, if needed.
     * Pass along grandchild's guid, if needed.
     * if on the final timestep, add dependence to
     * wrapup for this node.
     *
     */
    double * a = depv[0].ptr;
    private_t * private = depv[1].ptr;
    buffer_t * leftin = depv[2].ptr;
    buffer_t * rightin = depv[3].ptr;
    ocrGuid_t grandchild, child, leftctrl, rightctrl, leftinctrl, rightinctrl;
    ocrGuid_t wrapup = private->wrapup;
    u64 n = private->n;
    u64 m = private->m;
    u64 t = private->t;
    u64 node = private->mynode;
    u64 timestep = private->timestep;
    double * atemp = a + m;
    u64 i;




    if( timestep < t ){

        /*do the maths here.*/
        if(timestep == 0){
            //initialize!
        if( private->toDestroy[0] != NULL_GUID )
            ocrEventDestroy( private->toDestroy[0] );
        if( private->toDestroy[1] != NULL_GUID )
            ocrEventDestroy( private->toDestroy[1] );

            if( m == 1 ){
                if( node == 0 || node == (n - 1)) a[0] = 1.0;
                else a[0] = 0.0;
            }else{


            if(node == 0) a[0] = 1.0;
            else a[0] = 0.0;

            for(i = 1; i < m - 1; i++) a[i] = 0.0;
                                                                //setrightmost
            if(node == (n-1)) a[m-1] = 1.0;


            }
            if( node != 0 )leftin->buffer = a[0];
            if( node != (n - 1) )rightin->buffer = a[m-1];
        }else{                                   //math happens here.
            if( m == 1 ) {
                if(node != 0 || node != (n - 1)) atemp[0] = a[0];
                else atemp[0] = 0.5*a[0] + 0.25*((leftin->buffer) + (rightin->buffer));
            }else{
                if(node != 0) atemp[0] = 0.5*a[0] + 0.25*(a[1] + (leftin->buffer));
                else atemp[0] = a[0];

                for(i = 1; i < m - 1; i++) atemp[i] = 0.5*a[i] + 0.25*(a[i+1] + a[i-1]);

                if(node != (n-1)) atemp[m-1] = 0.5* a[m-1] + 0.25*( (rightin->buffer) + a[m-2]);
                else atemp[m - 1] = a[m-1];
            }
            for(i = 0; i < m; i++) a[i] = atemp[i];
            if(node != 0) leftin->buffer = a[0];
            if(node != (n - 1)) rightin->buffer = a[m-1];
        }
        private->timestep++;



        /*end doing maths.*/

        /*create grandchild*/
        ocrEdtCreate(   &grandchild,
                        private->template,
                        EDT_PARAM_DEF,
                        NULL,
                        EDT_PARAM_DEF,
                        NULL,
                        EDT_PROP_NONE,
                        NULL_GUID,
                        NULL_GUID );
        /*create and paste events*/


        /*update datablocks*/

        /*end datablock update*/
        if( node != 0 ){
            ocrEventCreate( &leftctrl,
                            OCR_EVENT_ONCE_T,
                            true );
            ocrAddDependence(   leftctrl,
                                grandchild,
                                2,
                                DB_MODE_RW );
        }else{
            leftctrl = NULL_GUID;
            ocrAddDependence(   NULL_GUID,
                                grandchild,
                                2,
                                DB_MODE_RW );
        }

        if( node != (n - 1) ){
            ocrEventCreate( &rightctrl,
                            OCR_EVENT_ONCE_T,
                            true );
            ocrAddDependence(   rightctrl,
                                grandchild,
                                3,
                                DB_MODE_RW );
        }else{
            rightctrl = NULL_GUID;
            ocrAddDependence(   rightctrl,
                                grandchild,
                                3,
                                DB_MODE_RW );
        }
        /* events created and pasted as needed. */
        /*update private block and other data blocks.*/
        child = private->mychild;       //this is all that needs to be done for private at this point.
        private->mychild = grandchild;
        /*Launch current child.*/
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


        if( node != 0 ){
            leftinctrl = leftin->control;
            leftin->control = leftctrl;
        }
        if( node != (n - 1) ){
            rightinctrl = rightin->control;
            rightin->control = rightctrl;
        }

        if( node !=  0 ){
            ocrDbRelease( depv[2].guid );
            ocrEventSatisfy(    leftinctrl,
                                depv[2].guid );
        }
        if( node != (n - 1) ){
            ocrDbRelease( depv[3].guid );
            ocrEventSatisfy(    rightinctrl,
                                depv[3].guid );
        }
        /*end current child launch.*/

    }else{
        ocrAddDependence(   depv[0].guid,
                            private->wrapup,
                            private->mynode,
                            DB_MODE_RW );
    }
    return NULL_GUID;
}

ocrGuid_t stencilInitEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    /*
     * paramv[1]:
     *  0: nodeNum
     *
     * depv[4]:
     *  0: shared block
     *  1: empty private block
     *  2: left buffer_t
     *  3: right buffer_t
     *
     *  -   private block setup
     *  -   create left and right datablocks (index permitting)
     *  -   create grandchild and child.
     *  -   get grandchild set up to receive events.
     */

    shared_t * shared = depv[0].ptr;
    private_t * private = depv[1].ptr;
    buffer_t * leftbuf = depv[2].ptr;
    buffer_t * rightbuf = depv[3].ptr;

    double* dummy;
    ocrGuid_t stencil;
    ocrGuid_t data;
    ocrGuid_t leftsend, leftrcv;
    ocrGuid_t rightsend, rightrcv;

    private->timestep = 0;                          //populate private block
    private->mynode = paramv[0];
    private->wrapup = shared->wrapup;
    private->n = shared->n;
    private->m = shared->m;
    private->t = shared->t;

    u64 node = paramv[0];
    u64 n = shared->n;
    u64 m = shared->m;
    u64 t = shared->t;

    ocrEdtTemplateCreate(   &(private->template),   //template for private block.
                            stencilEdt,
                            0,
                            4 );

    ocrDbCreate(    &data,
                    (void **)&dummy,
                    2 * private->m * sizeof(double),
                    0,
                    NULL_GUID,
                    NO_ALLOC );

    ocrEdtCreate(   &stencil,
                    private->template,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PROP_NONE,
                    NULL_GUID,
                    NULL_GUID );

    ocrEdtCreate(   &(private->mychild),            //grandchild created here.
                    private->template,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PROP_NONE,
                    NULL_GUID,
                    NULL_GUID );

    /*set up grandchild deps*/
    ocrGuid_t once;
    if( leftbuf != NULL ){
        ocrEventCreate( &once,
                        OCR_EVENT_ONCE_T,
                        true );

        ocrAddDependence(   once,
                            private->mychild,
                            2,
                            DB_MODE_RW );
        leftbuf->control = once;
    }else{
        ocrAddDependence(   NULL_GUID,
                            private->mychild,
                            2,
                            DB_MODE_RW );
    }
    if( rightbuf != NULL ){
        ocrEventCreate( &once,
                        OCR_EVENT_ONCE_T,
                        true );
        ocrAddDependence(   once,
                            private->mychild,
                            3,
                            DB_MODE_RW );
        rightbuf->control = once;
    }else{
        ocrAddDependence(   NULL_GUID,
                            private->mychild,
                            3,
                            DB_MODE_RW );
    }
    /*end grandchild dep setup*/

    if( private->mynode != 0 ){

        ocrGuidFromIndex(   &leftsend,              //set labeled guids for start of steady state.
                            shared->ranges[0],
                            private->mynode );
        ocrGuidFromIndex(   &leftrcv,
                            shared->ranges[1],
                            private->mynode );

        ocrEventCreate( &leftrcv,
                        OCR_EVENT_STICKY_T,
                        DEFAULT_LG_PROPS );

    }else{
        leftsend = NULL_GUID;
        leftrcv = NULL_GUID;
    }
    if( private->mynode < (private->n - 1)){
        ocrGuidFromIndex(   &rightsend,
                            shared->ranges[1],
                            private->mynode + 1);
        ocrGuidFromIndex(   &rightrcv,
                            shared->ranges[0],
                            private->mynode + 1);

        ocrEventCreate( &rightrcv,
                        OCR_EVENT_STICKY_T,
                        DEFAULT_LG_PROPS );
    }else{
        rightsend = NULL_GUID;
        rightrcv = NULL_GUID;
    }

    /*set child for launch*/
    private->toDestroy[0] = leftrcv;
    private->toDestroy[1] = rightrcv;

    ocrDbRelease( data );
    ocrAddDependence(   data,
                        stencil,
                        0,
                        DB_MODE_RW );
    ocrDbRelease( depv[1].guid );
    ocrAddDependence(   depv[1].guid,
                        stencil,
                        1,
                        DB_MODE_RW );
    ocrAddDependence(   leftrcv,
                        stencil,
                        2,
                        DB_MODE_RW );
    ocrAddDependence(   rightrcv,
                        stencil,
                        3,
                        DB_MODE_RW );

    if( leftsend != NULL_GUID ){
        ocrEventCreate( &leftsend,
                        OCR_EVENT_STICKY_T,
                        DEFAULT_LG_PROPS );
        ocrDbRelease( depv[2].guid );
        ocrEventSatisfy(    leftsend,
                            depv[2].guid );
    }

    if( rightsend != NULL_GUID ){
        ocrEventCreate( &rightsend,
                        OCR_EVENT_STICKY_T,
                        DEFAULT_LG_PROPS );
        ocrDbRelease( depv[3].guid );
        ocrEventSatisfy(    rightsend,
                            depv[3].guid );
    }
    /*end child launch*/

    return NULL_GUID;
}

ocrGuid_t initEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    /*
     * paramv[1]:
     *  0: nodeNum
     *
     * depv[1]:
     *  0: shared block
     *
     * create private block.
     * create edt for stencilInit
     * pass nodeNum through.
     */
    shared_t * shared = depv[0].ptr;
    ocrGuid_t privateDb, leftDb, rightDb;
    ocrGuid_t stencilInit, stencilInitTemplate;
    double * dummy;
    buffer_t *bDummy;

    ocrDbCreate(    &privateDb,
                    (void **)&dummy,
                    sizeof(private_t),
                    0,
                    NULL_GUID,
                    NO_ALLOC );
    if( paramv[0] != 0){
        ocrDbCreate(    &leftDb,
                        (void **)&bDummy,
                        sizeof(buffer_t),
                        0,
                        NULL_GUID,
                        NO_ALLOC );
    }else{
        leftDb = NULL_GUID;
    }
    if(paramv[0] < (shared->n - 1)){
        ocrDbCreate(    &rightDb,
                        (void **)&bDummy,
                        sizeof(buffer_t),
                        0,
                        NULL_GUID,
                        NO_ALLOC );
    }else{
        rightDb = NULL_GUID;
    }

    ocrEdtTemplateCreate(   &stencilInitTemplate,
                            stencilInitEdt,
                            1,
                            4 );

    ocrEdtCreate(   &stencilInit,
                    stencilInitTemplate,
                    EDT_PARAM_DEF,
                    paramv,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PROP_NONE,
                    NULL_GUID,
                    NULL_GUID );
    ocrDbRelease( depv[0].guid );
    ocrAddDependence(   depv[0].guid,
                        stencilInit,
                        0,
                        DB_MODE_RO );
    ocrDbRelease( privateDb );
    ocrAddDependence(   privateDb,
                        stencilInit,
                        1,
                        DB_MODE_RW );
    if( leftDb != NULL_GUID ) ocrDbRelease( leftDb );
    ocrAddDependence(   leftDb,
                        stencilInit,
                        2,
                        DB_MODE_RW );
    if( rightDb != NULL_GUID )ocrDbRelease( rightDb );
    ocrAddDependence(   rightDb,
                        stencilInit,
                        3,
                        DB_MODE_RW );
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

ocrGuid_t realmainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    /*
     * paramv[3]:
     *  0: N
     *  1: M
     *  2: T
     *
     * depv[1]:
     *  0: empty shared block
     *
     *
     *  create guid ranges
     *  create wrapup task
     *  populate shared block
     *  create i init tasks.
     */


    ocrGuid_t wrapup, wrapupTemplate, init, initTemplate;
    ocrGuid_t ranges[2];
    shared_t * shared = depv[0].ptr;
    u64 i;
    shared->n = paramv[0];
    shared->m = paramv[1];
    shared->t = paramv[2];

    PRINTF("N = %u\nM = %u\nT = %u\n", paramv[0], paramv[1], paramv[2]);

    ocrEdtTemplateCreate(   &wrapupTemplate,
                            wrapupEdt,
                            3,
                            shared->n );

    ocrEdtTemplateCreate(   &initTemplate,
                            initEdt,
                            1,
                            1 );

    for( i = 0; i < 2; i++ ){
        ocrGuidRangeCreate( &(shared->ranges[i]),           //we need these sticky ranges to start the alg.
                            shared->n,
                            GUID_USER_EVENT_STICKY );
    }

    ocrEdtCreate(   &wrapup,
                    wrapupTemplate,
                    EDT_PARAM_DEF,
                    paramv,
                    EDT_PARAM_DEF,
                    NULL_GUID,
                    EDT_PROP_NONE,
                    NULL_GUID,
                    NULL_GUID );

    shared->wrapup = wrapup;

    ocrDbRelease( depv[0].guid );

    for( i = 0; i < shared->n; i++ ){

        ocrEdtCreate(   &init,
                        initTemplate,
                        EDT_PARAM_DEF,
                        &i,
                        EDT_PARAM_DEF,
                        NULL,
                        EDT_PROP_NONE,
                        NULL_GUID,
                        NULL_GUID );

        ocrAddDependence(   depv[0].guid,
                            init,
                            0,
                            DB_MODE_RW );
    }

    return NULL_GUID;
}

ocrGuid_t mainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    /*
     * paramv[0]:
     *
     * depv[1]:
     *  0: cmd line args
     *
     * create empty shared block to initialize in realmainEdt, and
     * take in cmd line args.
     */

    void * programArgv = depv[0].ptr;
    u64 * dummy;
    u64 cmdArgs[3];

    ocrGuid_t shared, realMain, realMainTemplate;
    u32 argc = getArgc( programArgv );

    PRINTF("STENCIL 1D PARALLEL INIT WITH ONCE EVENTS.\n");

    if( argc != 4){
        PRINTF( "INCORRECT NUMBER OF ARGS. USING DEFAULT PARAMS. %s\n", getArgv(programArgv, 0) );
        cmdArgs[0] = N;
        cmdArgs[1] = M;
        cmdArgs[2] = T;
    }else{
        cmdArgs[0] = (u64)atoi(getArgv(programArgv, 1));
        cmdArgs[1] = (u64)atoi(getArgv(programArgv, 2));
        cmdArgs[2] = (u64)atoi(getArgv(programArgv, 3));
    }

    ocrDbCreate(    &shared,
                    (void **)&dummy,
                    sizeof( shared_t ),
                    0,
                    NULL_GUID,
                    NO_ALLOC );


    ocrEdtTemplateCreate(   &realMainTemplate,
                            realmainEdt,
                            3,
                            1 );

    ocrEdtCreate(   &realMain,
                    realMainTemplate,
                    EDT_PARAM_DEF,
                    cmdArgs,
                    EDT_PARAM_DEF,
                    NULL_GUID,
                    EDT_PROP_NONE,
                    NULL_GUID,
                    NULL_GUID );

    ocrDbRelease( shared );

    ocrAddDependence(   shared,
                        realMain,
                        0,
                        DB_MODE_RW );

    return NULL_GUID;
}
