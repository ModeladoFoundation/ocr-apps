/*
Written by David S. Scott
Copyright Intel Corporation 2016

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/
/*
OCR version of the Parallel Research Kernel synch_p2p
See README file for more information

10/19/15 fixed assignment bug
10/20/15 added run time arg support
10/27/15 added wtime() usage
6/22/15 added channel events, deleted passing bufferOut (just create them)

*/
#define ENABLE_EXTENSION_LABELING
#define ENABLE_EXTENSION_AFFINITY

//if BLOCK is defined, ranks are place in sequential blocks into policy domains
//so that PD0 gets rank 0, 1, 2,...
//if BLOCK is commented out //the ranks are placed in a wrap map
//so PD0 gets rank0, PD1 gets rank1, and so on...
//Currently BLOCK gives better performance than WRAP
//
//If AFFINITY is commented out then NULL_HINT is used after the initial assignment
//which leads to bad performance regardless of the initial assignment
#define BLOCK
#define AFFINITY

#include "ocr.h"
#include "extensions/ocr-labeling.h"
#include "extensions/ocr-affinity.h"
#include "stdio.h"
#include "macros.h"
/* macros.h is the "macros" library
 * it provides 8 macros for using structures for parameters and dependencies
 */

//default values if no input args

#define P 10     //number of workers
#define M 100    //total number of columns
#define N 1000    //number of rows
#define T 100    //number of timesteps
#define GF 1     //group factor (should be 1)

#define ARRAY(i,j) dataPTR[(i)*privatePTR->k + (j)] //translation from 2d to 1d

typedef struct{
    ocrGuid_t event;
    } buffer_t;

typedef struct{
    u32 p;
    u32 m;
    u32 n;
    u32 t;
    u32 gf;
    ocrGuid_t rangeGuid;
    ocrGuid_t timerDBK;
    } shared_t;

typedef struct{
    u32 timestep;
    u32 myRank;
    u32 row;
    u32 p;
    u32 m;
    u32 n;
    u32 t;
    u32 gf; //grouping factor, how many rows are processed in a chunk
    u32 k;  //local number of columns
    u32 w;  //number of blocks of rows in vertical direction
    u32 phase; //phase goes from 0 to w-1 in each timestep
    ocrGuid_t p2pTML;
    ocrGuid_t sendRightEVT;
    ocrGuid_t recvLeftEVT;
    ocrGuid_t timerDBK;
    ocrHint_t myHNT; //location now, maybe priority later
    } private_t;

extern double wtime(void);

void bomb(char * s) {
    PRINTF("BOMB %s \n", s);
    ocrShutdown();
    return;
}

typedef struct {
    ocrEdtDep_t data;
    ocrEdtDep_t private;
    ocrEdtDep_t bufferIn;
//used only twice
//rank0 at the beginning of iteration 1 (the second)
//rank p-1 at the end
    ocrEdtDep_t timer;
} p2pDEPV_t;

ocrGuid_t p2pEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {
/*
Paramv none

Depv[3]:
0: My datablock
1: My private block
2: Leftin block (satisfied by my left neighbor)
3: timer block.  Only used twice,
    by Rank 0 just after the first iteration and once
    by Rank P-1 at the end to compute the elapsed time)
*/
    DEPVDEF(p2p);

    ocrGuid_t dataDBK = DEPV(p2p,data,guid);
    double * dataPTR = DEPV(p2p,data,ptr);

    ocrGuid_t privateDBK = DEPV(p2p,private,guid);
    private_t * privatePTR = DEPV(p2p,private,ptr);

    ocrGuid_t bufferInDBK = DEPV(p2p,bufferIn,guid);
    double * bufferInPTR = DEPV(p2p,bufferIn,ptr);

    ocrGuid_t timerDBK = DEPV(p2p,timer,guid);
    double * timerPTR = DEPV(p2p,timer,ptr);


    ocrGuid_t bufferOutDBK;
    double * bufferOutPTR;
    u32 myRank = privatePTR->myRank;
    u32 timestep = privatePTR->timestep; //current timestep
    u32 row = privatePTR->row; //last row computed
    u32 p = privatePTR->p;  //number of ranks
    u32 m = privatePTR->m;  //global number or columns
    u32 n = privatePTR->n;  //number of rows
    u32 t = privatePTR->t;  //number of timesteps
    u32 gf = privatePTR->gf; //grouping factor--number of rows computed before sending
    u32 k = privatePTR->k;  //my number of columns
    u32 w = privatePTR->w;  //number of phases in an iteration
    u32 phase = privatePTR->phase; //current phase


    u32 i, j;
    ocrGuid_t p2pEDT;

//record start time
    if(myRank == 0 && phase == 0 && timestep == 1){
        double * timerPTR = (double *) DEPV(p2p,timer,ptr);
        *timerPTR = wtime();
        ocrDbRelease(DEPV(p2p,timer,guid));
    }

//consume wrap around
    if(myRank == 0 && row == 0 && timestep !=0) {
        ARRAY(0,0) = -bufferInPTR[0];//wrap around value
    }

    u32 numrows = gf;   //number of rows to compute
    if(n-row <= gf) numrows = n - row - 1;

//compute first column
    if(myRank != 0) {
        for(i=row+1;i<=row+numrows;i++) ARRAY(i,0) = ARRAY(i-1,0) + bufferInPTR[i-row] - bufferInPTR[i-row-1];
        }

//destroy bufferIn if it exists
    if(!ocrGuidIsNull(bufferInDBK)) ocrDbDestroy(bufferInDBK);

//loop over the rest of the columns
    for(j=1;j<k;j++){
        for(i=row+1;i<=row+numrows;i++){ //loop over rows
            ARRAY(i,j) = ARRAY(i-1,j) + ARRAY(i,j-1) - ARRAY(i-1,j-1);
        }
    }

//send right

    if(myRank != p-1) {
        ocrDbCreate(&(bufferOutDBK), (void **) &bufferOutPTR, sizeof(double)*(numrows+1), 0, NULL_HINT, NO_ALLOC);
        for(i=0;i<numrows+1;i++) {
            bufferOutPTR[i] = ARRAY(row+i,privatePTR->k-1);
        }
        ocrDbRelease(bufferOutDBK);
        ocrEventSatisfy(privatePTR->sendRightEVT, bufferOutDBK);
    }


//check termination
    if(timestep == t && phase == w-1){
        if(myRank == p-1) {
            double * timerPTR = DEPV(p2p,timer,ptr);
            double time = wtime();
printf("start time %f end time %f diff %f \n", *timerPTR, time, time-*timerPTR);
            time = time - *timerPTR;

            double avgtime = time/t;
            if(ARRAY(n-1,k-1) == (t+1)*(n+m-2)) PRINTF("PASS checksum = %f   \n", ARRAY(n-1,k-1));
              else PRINTF("FAIL  checksum = %f  should be %d \n", ARRAY(n-1,k-1), (t+1)*(n+m-2));
            double flops = 1.0e-06*2*((double) (m-1))*(n-1)/avgtime;
            if(gf>1) flops = -flops;
            PRINTF("Rate (MFlops/s): %f Avg time (s): %f\n", flops, avgtime);
            ocrShutdown();
            }
        return NULL_GUID;
    }

//special wrap around send
    if(myRank == p-1 && phase == w-1) {
        ocrDbCreate(&(bufferOutDBK), (void **) &bufferOutPTR, sizeof(double), 0, NULL_HINT, NO_ALLOC);
        *bufferOutPTR = ARRAY(n-1,k-1);
        ocrDbRelease(bufferOutDBK);
        ocrEventSatisfy(privatePTR->sendRightEVT, bufferOutDBK);
    }

//create clone

    if((myRank == 0 && phase == w-1 && timestep == 0) || (myRank == p-1 && phase == w-2 && timestep == t) || (myRank == p-1 && w == 1 && timestep == t-1)){
#ifdef AFFINITY
        ocrEdtCreate(&p2pEDT, privatePTR->p2pTML, EDT_PARAM_DEF, NULL, 4, NULL, EDT_PROP_NONE, &(privatePTR->myHNT), NULL);
#else
        ocrEdtCreate(&p2pEDT, privatePTR->p2pTML, EDT_PARAM_DEF, NULL, 4, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif
        ocrAddDependence(privatePTR->timerDBK, p2pEDT, SLOT(p2p,timer), DB_MODE_RW);
    } else {
#ifdef AFFINITY
        ocrEdtCreate(&p2pEDT, privatePTR->p2pTML, EDT_PARAM_DEF, NULL, 3, NULL, EDT_PROP_NONE, &(privatePTR->myHNT), NULL);
#else
        ocrEdtCreate(&p2pEDT, privatePTR->p2pTML, EDT_PARAM_DEF, NULL, 3, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif
    }

//launch clone

    ocrDbRelease(dataDBK);
    ocrAddDependence(dataDBK, p2pEDT, SLOT(p2p,data), DB_MODE_RW);

    if(myRank != 0) {
        ocrAddDependence(privatePTR->recvLeftEVT, p2pEDT, SLOT(p2p,bufferIn), DB_MODE_RW);
    } else if(phase == w-1) {
            ocrAddDependence(privatePTR->recvLeftEVT, p2pEDT, SLOT(p2p,bufferIn), DB_MODE_RW); //special wrap around receive
        }
        else {
            ocrAddDependence(NULL_GUID, p2pEDT, SLOT(p2p,bufferIn), DB_MODE_RW); //no receive
        }


    if(phase == w-1){ //finish timestep
        privatePTR->row = 0;
        privatePTR->timestep++;
        privatePTR->phase = 0;
    }else{//next phase
        privatePTR->row += gf;
        privatePTR->phase++;
    }

    ocrDbRelease(privateDBK);
    ocrAddDependence(privateDBK, p2pEDT, SLOT(p2p,private), DB_MODE_RW);

    return NULL_GUID;
}

typedef struct {
    ocrGuid_t p2pEDT;
} initChannelPRM_t;

typedef struct {
    ocrEdtDep_t private;
    ocrEdtDep_t channel;
} initChannelDEPV_t;


ocrGuid_t initChannelEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv[1]
0: the GUID of the first instance of p2p, created by initp2p but launched here

DEPV[2]
0: private
1: channel containing the channel event I will receive on.
*/

    PRMDEF(initChannel);
    ocrGuid_t p2pEDT = PRM(initChannel,p2pEDT);

    DEPVDEF(initChannel);
    ocrGuid_t privateDBK = DEPV(initChannel,private,guid);
    ocrGuid_t channelDBK = DEPV(initChannel,channel,guid);
    private_t * privatePTR = (private_t *) DEPV(initChannel,private,ptr);
    ocrGuid_t * channelPTR = (ocrGuid_t *) DEPV(initChannel,channel,ptr);

    privatePTR->recvLeftEVT = *channelPTR;
    ocrDbDestroy(channelDBK);

    if(privatePTR->myRank != 0) ocrAddDependence(privatePTR->recvLeftEVT, p2pEDT, SLOT(p2p,bufferIn), DB_MODE_RO);
        else ocrAddDependence(NULL_GUID, p2pEDT, SLOT(p2p,bufferIn), DB_MODE_RO);

    ocrDbRelease(privateDBK);
    ocrAddDependence(privateDBK, p2pEDT, SLOT(p2p,private), DB_MODE_RW);

    return NULL_GUID;
}

typedef struct {
    u64 myRank;
} initp2pPRM_t;

typedef struct {
    ocrEdtDep_t shared;
    ocrEdtDep_t private;
    ocrEdtDep_t data;
    ocrEdtDep_t channel;
} initp2pDEPV_t;


ocrGuid_t initp2pEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv[1]:
0: myRank

depv[3];
0: shared block
1: private block
2: datablock
3: channel block

fill private block
create p2p
launch initChannel
*/


    PRMDEF(initp2p);
    u32 myRank = PRM(initp2p,myRank);

    DEPVDEF(initp2p);
    ocrGuid_t sharedDBK = DEPV(initp2p,shared,guid);
    shared_t * sharedPTR = DEPV(initp2p,shared,ptr);
    ocrGuid_t privateDBK = DEPV(initp2p,private,guid);
    private_t * privatePTR = DEPV(initp2p,private,ptr);
    ocrGuid_t dataDBK = DEPV(initp2p,data,guid);
    double * dataPTR = DEPV(initp2p,data,ptr);
    ocrGuid_t channelDBK = DEPV(initp2p,channel,guid);
    ocrGuid_t * channelPTR = DEPV(initp2p,channel,ptr);


    u32 i, j;

    privatePTR->timestep = 0;
    privatePTR->myRank = myRank;
    privatePTR->row = 0;
    privatePTR->p = sharedPTR->p;
    privatePTR->m = sharedPTR->m;
    privatePTR->n = sharedPTR->n;
    privatePTR->t = sharedPTR->t;
    privatePTR->gf = sharedPTR->gf;
    privatePTR->timerDBK = sharedPTR->timerDBK;
    privatePTR->k = privatePTR->m/privatePTR->p;
    u32 myfirst = privatePTR->k*myRank;
    if(myRank != 0 && myRank <= ((privatePTR->m)%privatePTR->p)) privatePTR->k++;
    for(i=1;i<myRank;i++) if(i-1 < ((privatePTR->m)%privatePTR->p)) myfirst++;
    privatePTR->w = (privatePTR->n-1 + privatePTR->gf -1)/privatePTR->gf;
    privatePTR->phase = 0;


#ifdef AFFINITY
    ocrHintInit(&(privatePTR->myHNT),OCR_HINT_EDT_T);
    ocrGuid_t myAffinity;
    ocrAffinityGetCurrent(&myAffinity);
    ocrSetHintValue(&(privatePTR->myHNT), OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));
#endif


//initialize values
    for(i=0;i<privatePTR->k;i++) ARRAY(0,i) = myfirst+i; //my part of bottom row

    for(j=1;j<privatePTR->n;j++)
        for(i=0;i<privatePTR->k;i++) ARRAY(j,i) = 0.0;

    if(myRank == 0) for(j=1;j<privatePTR->n;j++) ARRAY(j,0) = j;  //left most column



//create first p2pEDT

    ocrEdtTemplateCreate(&(privatePTR->p2pTML), p2pEdt, 0, EDT_PARAM_UNK);
    ocrGuid_t p2pEDT;
#ifdef AFFINITY
    ocrEdtCreate(&p2pEDT, privatePTR->p2pTML, EDT_PARAM_DEF, NULL, 3, NULL, EDT_PROP_NONE, &(privatePTR->myHNT), NULL);
#else
    ocrEdtCreate(&p2pEDT, privatePTR->p2pTML, EDT_PARAM_DEF, NULL, 3, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif

//create channelEDT

    ocrGuid_t initChannelTML, initChannelEDT;
    ocrEdtTemplateCreate(&(initChannelTML), initChannelEdt, PRMNUM(initChannel), DEPVNUM(initChannel));
    initChannelPRM_t initChannelParamv;
    initChannelPRM_t * initChannelPRM = &initChannelParamv;

    PRM(initChannel,p2pEDT) = p2pEDT;
#ifdef AFFINITY
    ocrEdtCreate(&initChannelEDT, initChannelTML, EDT_PARAM_DEF, (u64 *) initChannelPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &(privatePTR->myHNT), NULL);
#else
    ocrEdtCreate(&initChannelEDT, initChannelTML, EDT_PARAM_DEF, (u64 *) initChannelPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif

    ocrGuid_t sendRightEVT, recvLeftEVT;

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = (privatePTR->n+privatePTR->gf-1)/privatePTR->gf;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;
    ocrEventCreateParams(&(privatePTR->sendRightEVT), OCR_EVENT_CHANNEL_T, false, &params);



    ocrGuidFromIndex(&sendRightEVT, sharedPTR->rangeGuid, myRank);
    ocrEventCreate(&sendRightEVT, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK |EVT_PROP_TAKES_ARG);

    if(myRank != 0) {
        ocrGuidFromIndex(&recvLeftEVT, sharedPTR->rangeGuid, myRank-1);
    } else { //myRank == 0;
        ocrGuidFromIndex(&recvLeftEVT, sharedPTR->rangeGuid, privatePTR->p - 1);
    }
    ocrEventCreate(&recvLeftEVT, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK |EVT_PROP_TAKES_ARG);
    ocrAddDependence(recvLeftEVT, initChannelEDT, SLOT(initChannel,channel), DB_MODE_RO);





    *channelPTR = privatePTR->sendRightEVT;
    ocrDbRelease(channelDBK);
    ocrEventSatisfy(sendRightEVT, channelDBK);

    ocrDbRelease(dataDBK);
    ocrAddDependence(dataDBK, p2pEDT, SLOT(p2p,data), DB_MODE_RW);

    ocrDbRelease(privateDBK);
    ocrAddDependence(privateDBK, initChannelEDT, SLOT(initChannel,private), DB_MODE_RW);

    return NULL_GUID;

}

typedef struct {
    u64 myRank;
} initPRM_t;

typedef struct {
    ocrEdtDep_t shared;
} initDEPV_t;

ocrGuid_t initEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv[1]:
0: myRank

depv[1];
0: shared block

create private block
launch initp2p
*/
    u32 myRank = paramv[0];
    ocrGuid_t sharedGUID = depv[0].guid;
    shared_t * sharedPTR = depv[0].ptr;

    u32 p = sharedPTR->p;
    u32 m = sharedPTR->m;
    u32 n = sharedPTR->n;
    u32 gf = sharedPTR->gf;

    ocrGuid_t privateDBK, dataDBK, initp2pTML, initp2pEDT;

    ocrEdtTemplateCreate(&initp2pTML, initp2pEdt, PRMNUM(initp2p), DEPVNUM(initp2p));

    ocrHint_t myHNT;
    ocrHintInit(&myHNT,OCR_HINT_EDT_T);
    ocrGuid_t myAffinity;
    ocrAffinityGetCurrent(&myAffinity);
    ocrSetHintValue(&myHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));

#ifdef AFFINITY
    ocrEdtCreate(&initp2pEDT, initp2pTML, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &myHNT, NULL);
#else
    ocrEdtCreate(&initp2pEDT, initp2pTML, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif

    ocrDbRelease(sharedGUID);
    ocrAddDependence(sharedGUID, initp2pEDT, SLOT(initp2p,shared), DB_MODE_RO);

    u64 dummy;


    ocrDbCreate(&privateDBK, (void**) &dummy, sizeof(private_t), 0, NULL_HINT, NO_ALLOC);


    ocrAddDependence(privateDBK, initp2pEDT, SLOT(initp2p,private), DB_MODE_RW);

//round size up to be sure
    ocrDbCreate(&dataDBK, (void**) &dummy, n*(m/p+1)*sizeof(double), 0, NULL_HINT, NO_ALLOC);
    ocrAddDependence(dataDBK, initp2pEDT, SLOT(initp2p,data), DB_MODE_RW);

//used once to send channel events forward
    ocrGuid_t channelDBK;
    ocrDbCreate(&channelDBK, (void**) &dummy, sizeof(ocrGuid_t), 0, NULL_HINT, NO_ALLOC);
    ocrAddDependence(channelDBK, initp2pEDT, SLOT(initp2p,channel), DB_MODE_RW);


    return NULL_GUID;
}

typedef struct {
    u64 p;
    u64 m;
    u64 n;
    u64 t;
    u64 gf;
    ;
} realMainPRM_t;

typedef struct {
    ocrEdtDep_t shared;
    ocrEdtDep_t timer;
} realMainDEPV_t;

ocrGuid_t realMainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){

/*
paramv[5]:
0: p  the number of workers
1: m  the number of columns
2: n  the number of rows
3: t  the number of iterations
4: gf the grouping factor (should be 1)

depv[1]:
0: empty shared block
1: empty time block

realMain initializes the shared block and launches p initEdts
*/

    PRMDEF(realMain);
    DEPVDEF(realMain);

    ocrGuid_t sharedDBK = DEPV(realMain,shared,guid);
    shared_t * sharedPTR = DEPV(realMain,shared,ptr);

    ocrGuid_t initTML, initEDT;
    u64 i;

    sharedPTR->p = PRM(realMain,p);
    sharedPTR->m = PRM(realMain,m);
    sharedPTR->n = PRM(realMain,n);
    sharedPTR->t = PRM(realMain,t);
    sharedPTR->gf = PRM(realMain,gf);
    ocrGuidRangeCreate(&sharedPTR->rangeGuid, sharedPTR->p, GUID_USER_EVENT_STICKY);
    sharedPTR->timerDBK = DEPV(realMain,timer,guid);

    ocrEdtTemplateCreate(&initTML, initEdt, PRMNUM(init), DEPVNUM(init));

    ocrGuid_t myAffinity;
    ocrHint_t myHNT;
    u64 count, block, myPD;
    ocrAffinityCount(AFFINITY_PD, &count);
    block = (sharedPTR->p + count - 1)/count;


    for(i=0;i<sharedPTR->p;i++) {
//launch P p2p EDTs

//set affinity Hint

#ifdef BLOCK
        myPD = i/block;
#else
        myPD = i%count;
#endif
        ocrAffinityGetAt(AFFINITY_PD, myPD, &(myAffinity));
        ocrHintInit(&myHNT,OCR_HINT_EDT_T);
        ocrSetHintValue(&myHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));


#ifdef AFFINITY
        ocrEdtCreate(&(initEDT), initTML, EDT_PARAM_DEF, &i, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &myHNT, NULL);
#else
        ocrEdtCreate(&(initEDT), initTML, EDT_PARAM_DEF, &i, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif
        ocrDbRelease(sharedDBK);
        ocrAddDependence(sharedDBK, initEDT, SLOT(init,shared), DB_MODE_RO);
    }

    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
//mainEdt is executed first
//Parses Argv
//Creates a shared datablock
//creates realMain

    u64 *dummy;



    void * programArgv = depv[0].ptr;
    u32 argc = getArgc(programArgv);


PRINTF("argc %d \n", argc);
    if((argc != 1) && (argc != 5) && (argc != 6)) {
        bomb("arg count should be 0 (use defaults), 4 (p, m, n, iterations) or 5 (optional group factor) \n");
        return NULL_GUID;
    }

    u64 p = P;
    u64 m = M;
    u64 n = N;
    u64 t = T;
    u64 gf = GF;

    if(argc > 4) {
       p =  atoi(getArgv(programArgv, 1));
       if(p<=0) bomb("p must be positive");
       m =  atoi(getArgv(programArgv, 2));
       if(m<=0) bomb("m must be positive");
       n =  atoi(getArgv(programArgv, 3));
       if(n<=0) bomb("n must be positive");
       t =  atoi(getArgv(programArgv, 4));
       if(t<=0) bomb("t must be positive");
       if(argc == 6) gf =  atoi(getArgv(programArgv, 5));
       if(gf<=0) bomb("gf must be positive");
    }

    PRINTF("number of workers   %d \n", p);
    PRINTF("number of columns   %d \n", m);
    PRINTF("number of rows      %d \n", n);
    PRINTF("number of timesteps %d \n", t);
    PRINTF("grouping factor     %d", gf);
    if(gf == 1) PRINTF("\n"); else PRINTF(" (Cheating)\n");

    ocrGuid_t realMainTML, realMainEDT, sharedDBK, timerDBK;
//create realMain
    ocrEdtTemplateCreate(&realMainTML, realMainEdt, 5, 2);
    u64 paramvout[5] = {p, m, n, t, gf};
    ocrEdtCreate(&realMainEDT, realMainTML, EDT_PARAM_DEF, paramvout, EDT_PARAM_DEF, NULL,
          EDT_PROP_NONE, NULL_HINT, NULL);
//launch realMain

    ocrDbCreate(&sharedDBK, (void**) &dummy, sizeof(shared_t), 0, NULL_HINT, NO_ALLOC);
    ocrAddDependence(sharedDBK, realMainEDT, SLOT(realMain,shared), DB_MODE_RW);
    ocrDbCreate(&timerDBK, (void**) &dummy, sizeof(double), 0, NULL_HINT, NO_ALLOC);
    ocrAddDependence(timerDBK, realMainEDT, SLOT(realMain,timer), DB_MODE_RW);


    return NULL_GUID;
}
