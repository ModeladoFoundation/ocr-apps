/*
 *Author: Bryan Pawlowski
 *Copywrite Intel Corporation 2015
 *
 *This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 *and cannot be distributed without it. This notice cannot be removed or modified.
 */

/*
 * Stencil1D code with no events
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
    u64 n, m, t;
    ocrGuid_t ranges[2];
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

    u64 node = private->mynode;
    u64 n = private->n;
    u64 m = private->m;
    u64 t = private->t;
    ocrGuid_t leftctrl, rightctrl, grandchild;
    u64 i;

    double * atemp = a + m;

    if( node != 0 ) leftctrl = leftin->control;
    else leftctrl = NULL_GUID;

    if( node != (n - 1) ) rightctrl = rightin->control;
    else rightctrl = NULL_GUID;


    ocrGuid_t wrapup = private->wrapup;
    ocrGuid_t mychild = private->mychild;

    if( private->timestep == 0 ){
        if( private->toDestroy[0] != NULL_GUID ){
            ocrEventDestroy( private->toDestroy[0] );
            private->toDestroy[0] = NULL_GUID;
        }
        if( private->toDestroy[1] != NULL_GUID ){
            ocrEventDestroy( private->toDestroy[1] );
            private->toDestroy[1] = NULL_GUID;
        }

        if( m == 1 ){
            if( leftin == NULL || rightin == NULL) a[0] = 1.0;
            else a[0] = 0.0;
        }else{


        if(private->mynode == 0) a[0] = 1.0;
        else a[0] = 0.0;

            for(i = 1; i < m - 1; i++) a[i] = 0.0;
                                                                //setrightmost
            if(private->mynode == (n-1)) a[m-1] = 1.0;


            }
            if( leftin != NULL )leftin->buffer = a[0];
            if( rightin != NULL )rightin->buffer = a[m-1];
    }


    if( private->timestep < t ){

        if( private->timestep != 0){
            if( m == 1 ) {
                if(leftin == NULL || rightin == NULL) atemp[0] = a[0];
                else atemp[0] = 0.5*a[0] + 0.25*((leftin->buffer) + (rightin->buffer));
            }else{
                if(leftin != NULL) atemp[0] = 0.5*a[0] + 0.25*(a[1] + (leftin->buffer));
                else atemp[0] = a[0];

                for(i = 1; i < m - 1; i++) atemp[i] = 0.5*a[i] + 0.25*(a[i+1] + a[i-1]);

                if(rightin != NULL) atemp[m-1] = 0.5* a[m-1] + 0.25*( (rightin->buffer) + a[m-2]);
                else atemp[m - 1] = a[m-1];
            }
            for(i = 0; i < m; i++) a[i] = atemp[i];
            if(leftin != NULL) leftin->buffer = a[0];
            if(rightin != NULL) rightin->buffer = a[m-1];
        }
        if( private->timestep < (t - 1) ){
            ocrEdtCreate(   &grandchild,
                            private->template,
                            EDT_PARAM_DEF,
                            NULL,
                            EDT_PARAM_DEF,
                            NULL,
                            EDT_PROP_NONE,
                            NULL_GUID,
                            NULL_GUID );

            if( private->mynode != 0){
                leftin->control = grandchild;
            }else{
                ocrAddDependence(   NULL_GUID,
                                    grandchild,
                                    2,
                                    DB_MODE_RW );
            }
            if( private->mynode != (n - 1) ){
                rightin->control = grandchild;
            }else{
                ocrAddDependence(   NULL_GUID,
                                    grandchild,
                                    3,
                                    DB_MODE_RW );
            }

            private->mychild = grandchild;
        }

        ocrDbRelease( depv[0].guid );
        ocrAddDependence(   depv[0].guid,
                            mychild,
                            0,
                            DB_MODE_RW );
        private->timestep++;
        ocrDbRelease( depv[1].guid );
        ocrAddDependence(   depv[1].guid,
                            mychild,
                            1,
                            DB_MODE_RW );
        if( node != 0){
            ocrDbRelease( depv[2].guid );
            ocrAddDependence(   depv[2].guid,
                                leftctrl,
                                3,                  //when you send left, it must be plugged into the
                                DB_MODE_RW );       //right rcv of the receiver.
        }
        if( node != (n - 1) ){
            ocrDbRelease( depv[3].guid );
            ocrAddDependence(   depv[3].guid,
                                rightctrl,
                                2,
                                DB_MODE_RW );
        }
    }else{


        ocrAddDependence(   depv[0].guid,
                            wrapup,
                            node,
                            DB_MODE_RO );
    }


    return NULL_GUID;
}

ocrGuid_t stencilInitEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
    /*
     * paramv[1]:
     *  0: node number
     * depv[4]:
     *  0: shared block
     *  1: private block
     *  2: left datablock //if needed
     *  3: right datablock //if needed
     *
     *  populate private block.
     *  set ranges for start events. no more events after this start.
     *  create child
     *  create grandchild.
     *  trigger events to get things started.
     */

    ocrGuid_t stencil;
    ocrGuid_t dataDb;
    ocrGuid_t leftsend, leftrcv;
    ocrGuid_t rightsend, rightrcv;

    shared_t * shared = depv[0].ptr;
    private_t * private = depv[1].ptr;
    buffer_t * lbuf = depv[2].ptr;
    buffer_t * rbuf = depv[3].ptr;
    double * dummy;

    private->timestep = 0;
    private->mynode = paramv[0];
    private->n = shared->n;
    private->m = shared->m;
    private->t = shared->t;
    private->wrapup = shared->wrapup;

    ocrEdtTemplateCreate(   &(private->template),
                            stencilEdt,
                            0,
                            4 );

    ocrEdtCreate(   &stencil,
                    private->template,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PROP_NONE,
                    NULL_GUID,
                    NULL_GUID );

    ocrEdtCreate(   &(private->mychild),
                    private->template,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PROP_NONE,
                    NULL_GUID,
                    NULL_GUID );

    if( private->mynode == 0 ){
        ocrAddDependence(   NULL_GUID,
                            private->mychild,
                            2,
                            DB_MODE_RW );
    }
    if( private->mynode == (private->n - 1 )){
        ocrAddDependence(   NULL_GUID,
                            private->mychild,
                            3,
                            DB_MODE_RW );
    }

    if(lbuf != NULL) lbuf->control = private->mychild;
    if(rbuf != NULL) rbuf->control = private->mychild;

    /*set labeled events.*/

    if( private->mynode != 0 ){
        ocrGuidFromIndex(   &leftsend,
                            shared->ranges[0],
                            private->mynode );

        ocrGuidFromIndex(   &leftrcv,
                            shared->ranges[1],
                            private->mynode );
    }else{
        leftsend = NULL_GUID;
        leftrcv = NULL_GUID;
    }

    if( private->mynode != ( private->n - 1 ) ){
        ocrGuidFromIndex(   &rightsend,
                            shared->ranges[1],
                            private->mynode + 1 );
        ocrGuidFromIndex(   &rightrcv,
                            shared->ranges[0],
                            private->mynode + 1 );
    }else{
        rightsend = NULL_GUID;
        rightrcv = NULL_GUID;
    }
    private->toDestroy[0] = leftrcv;
    private->toDestroy[1] = rightrcv;


    //create datadb.
    //

    ocrDbCreate(    &dataDb,
                    (void **)&dummy,
                    2 * private->m * sizeof(double),
                    0,
                    NULL_GUID,
                    NO_ALLOC );

    ocrAddDependence(   dataDb,
                        stencil,
                        0,
                        DB_MODE_RW );

    ocrDbRelease( depv[1].guid );
    ocrAddDependence(   depv[1].guid,
                        stencil,
                        1,
                        DB_MODE_RW );

    if( leftrcv != NULL_GUID  )
    {
        ocrEventCreate( &leftrcv,
                        OCR_EVENT_STICKY_T,
                        DEFAULT_LG_PROPS );
    }

    ocrAddDependence(   leftrcv,
                        stencil,
                        2,
                        DB_MODE_RW );

    if( rightrcv != NULL_GUID )
    {
        ocrEventCreate( &rightrcv,
                        OCR_EVENT_STICKY_T,
                        DEFAULT_LG_PROPS );
    }

    ocrAddDependence(   rightrcv,
                        stencil,
                        3,
                        DB_MODE_RW );

    //launch first wave.

    if( leftsend != NULL_GUID )
    {
        ocrEventCreate( &leftsend,
                        OCR_EVENT_STICKY_T,
                        DEFAULT_LG_PROPS );

        ocrEventSatisfy(    leftsend,
                            depv[2].guid );

    }
    if( rightsend != NULL_GUID )
    {
        ocrEventCreate( &rightsend,
                        OCR_EVENT_STICKY_T,
                        DEFAULT_LG_PROPS );
        ocrEventSatisfy(    rightsend,
                            depv[3].guid );
    }

    return NULL_GUID;
}

ocrGuid_t initEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{

    /*
     * paramv[1]:
     *  0: node number
     *
     * depv[1]:
     *  0: shared block
     *
     *
     * Create our private block and our left and right buffer_t data blocks.
     *
     * Call stencil Init and pass everything down.
     */
    shared_t * shared = depv[0].ptr;
    private_t * dummy;
    buffer_t * btDummy;

    ocrGuid_t private, leftDb, rightDb;
    ocrGuid_t stencilInit, stencilInitTemplate;


    ocrDbCreate(    &private,
                    (void **)&dummy,
                    sizeof( private_t ),
                    0,
                    NULL_GUID,
                    NO_ALLOC );

    ocrDbCreate(    &leftDb,
                    (void **)&btDummy,
                    sizeof( buffer_t ),
                    0,
                    NULL_GUID,
                    NO_ALLOC );
    ocrDbCreate(    &rightDb,
                    (void **)&btDummy,
                    sizeof( buffer_t ),
                    0,
                    NULL_GUID,
                    NO_ALLOC );

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

    ocrAddDependence(   depv[0].guid,
                        stencilInit,
                        0,
                        DB_MODE_RO );

    ocrAddDependence(   private,
                        stencilInit,
                        1,
                        DB_MODE_RW );

    ocrAddDependence(   leftDb,
                        stencilInit,
                        2,
                        DB_MODE_RW );

    ocrAddDependence(   rightDb,
                        stencilInit,
                        3,
                        DB_MODE_RW );

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
     *  0: empty shared block.
     *
     *  populate shared block.
     *  send it on to parallel init.
     */
    shared_t * shared = depv[0].ptr;
    shared->n = paramv[0];
    shared->m = paramv[1];
    shared->t = paramv[2];

    u64 i;

    PRINTF("N = %u\nM = %u\nT = %u\n", paramv[0], paramv[1], paramv[2]);

    for( i = 0; i < 2; i++ ){
        ocrGuidRangeCreate( &(shared->ranges[i]),
                            shared->n,
                            GUID_USER_EVENT_STICKY );
    }

    ocrGuid_t wrapup, wrapupTemplate;
    ocrGuid_t init, initTemplate;

    ocrEdtTemplateCreate(   &wrapupTemplate,
                            wrapupEdt,
                            3,
                            shared->n );

    ocrEdtCreate(   &wrapup,
                    wrapupTemplate,
                    EDT_PARAM_DEF,
                    paramv,
                    EDT_PARAM_DEF,
                    NULL,
                    EDT_PROP_NONE,
                    NULL_GUID,
                    NULL_GUID );

    shared->wrapup = wrapup;

    ocrEdtTemplateCreate(   &initTemplate,
                            initEdt,
                            1,
                            1 );

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
                            DB_MODE_RO );
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

    PRINTF("STENCIL 1D PARALLEL INIT WITH NO EVENTS\n");

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
