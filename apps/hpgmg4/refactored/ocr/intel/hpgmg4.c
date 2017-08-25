/*
Author David S. Scott
Copywrite Intel Corporation 2016

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.

*/

/*
This code started to implement fourth order HPGMG in OCR.
It started with the hpcg code since it was going to need
a similar halo exchange.

Some modifications were made.

More information is in README

October 2016  started
November 2016 abandoned to concentrate on hpcg performance

*/

#ifndef TG_ARCH
#include "time.h"
#else
#define NO_AFFINITY
#endif


#ifndef NO_COMPUTE
#define COMPUTE  //undefine to suppress compute
#endif

#ifndef NO_AFFINITY
#define AFFINITY
#endif

//#define RECEIVER_OWNS_CHANNEL_EVENT //undefine to make the sender own it

#define ENABLE_EXTENSION_LABELING
#define ENABLE_EXTENSION_AFFINITY

#include "ocr.h"
#include "stdlib.h"
#include "extensions/ocr-labeling.h" //currently needed for labeled guids
#include "extensions/ocr-affinity.h" //needed for affinity


#include "timers.h"
#define TICK (1.0e-6)


#include "string.h" //if memcpy is needed
#include "stdio.h"  //needed for ocrPrintf debugging

#include "reduction.h"
#include "macros.h"

#define MAXLEVEL 10
#define AGGLOMERATION_SIZE 8

#define DEBUG 0 //debug print level 0: none 1: some 2: LOTS

//           sizes of the vectors
#define Z 0 //4 levels of ht
#define R 1 //4 levels of mt
#define AP 2 //1*mt (reused)
#define X 3 //1*mt
#define P 4 //1*ht
#define B 5 //1*mt

typedef struct{
    u32 length[26];
    u32 start[26];
    u32 l1[26];
    u32 l2[26];
    u32 p1[26];
    u32 p2[26];
    } gather_t;

typedef struct{
    u32 nrank; //must be a cube of a power of 2
    u32 nlevels; //
    u32 npx;
    u32 boxsize;   //size of box
    u32 debug;
    ocrGuid_t haloRangeGUID;
    ocrGuid_t coarsenRangeGUID;
    ocrGuid_t refineRangeGUID;
    ocrGuid_t reductionRangeGUID[MAXLEVEL];
    ocrGuid_t finalOnceEVT;
} sharedBlock_t;


typedef struct{
    u32 nrank;
    s32 myrank[MAXLEVEL]; //use negative rank numbers to facilitate agglomeration
    u32 boxsize[MAXLEVEL];
    u32 nlevels;
    u32 npx[MAXLEVEL];
    u32 level;
    u32 phase;
    u32 debug;
    u32 m[MAXLEVEL];
    u32 mt[MAXLEVEL];
    u32 ht[MAXLEVEL];
    u32 mgPhase[MAXLEVEL];
//offsets in bytes for the various structures in the datablock beyond the private structure
    u32 vector_offset[MAXLEVEL][6];
    gather_t gather[4];
    ocrGuid_t reductionReturnEVT;
    ocrHint_t myAffinityHNT;
    ocrGuid_t haloRangeGUID;
    ocrGuid_t coarsenRangeGUID;
    ocrGuid_t refineRangeGUID;
    ocrGuid_t reductionRangeGUID[MAXLEVEL];
    ocrGuid_t spmvTML;
    ocrGuid_t mgTML;
    ocrGuid_t haloExchangeTML;
    ocrGuid_t smoothTML;
    ocrGuid_t unpackTML;
    ocrGuid_t haloSendEVT[MAXLEVEL][26];
    ocrGuid_t haloRecvEVT[MAXLEVEL][26];
    ocrGuid_t refineSendEVT[MAXLEVEL][7];
    ocrGuid_t refineRecvEVT[MAXLEVEL];
    ocrGuid_t coarsenSendEVT[MAXLEVEL];
    ocrGuid_t coarsenRecvEVT[MAXLEVEL][7];
    ocrGuid_t finalOnceEVT;
    } privateBlock_t;


void bomb(char * s) {
ocrPrintf("ERROR %s TERMINATING\n", s);
ocrShutdown();
return;
}

void initializeEvents(u64 level, privateBlock_t * pbPTR){

    s64 nlevels = pbPTR->nlevels;

printf("nlevels %d level %d \n", nlevels, level);
    if(level < nlevels-1) { //not top level, use old values to set new
printf("myOLDrank %d \n", pbPTR->myrank[level+1]);
        if(pbPTR->myrank[level+1] == -1 || !ocrGuidIsNull(pbPTR->coarsenSendEVT[level+1])) {
            pbPTR->myrank[level] = -1;
            return;
        }
        if(pbPTR->boxsize[level+1] > AGGLOMERATION_SIZE) { //local shrink
            pbPTR->boxsize[level] = pbPTR->boxsize[level+1] >>= 1;
            pbPTR->npx[level] = pbPTR->npx[level+1];
            pbPTR->myrank[level] = pbPTR->myrank[level+1];
        } else
            if(pbPTR->npx[level+1] > 2){ //agglomerate
                pbPTR->boxsize[level] = pbPTR->boxsize[level+1];
                pbPTR->npx[level] = pbPTR->npx[level+1] >> 1;
            } else { //down to rank 0
                pbPTR->boxsize[level] = pbPTR->boxsize[level+1]/2;
                pbPTR->npx[level] = 1;
                pbPTR->myrank[level] = 0;
                return;
            }
    }

    s64 boxsize = pbPTR->boxsize[level];
    s64 npx = pbPTR->npx[level];
    s64 expansion = pbPTR->npx[nlevels-1]/pbPTR->npx[level];
    s64 myrank = pbPTR->myrank[level];
printf("myNEWrank %d \n", pbPTR->myrank[level]);


printf("start level %d boxsize %d npx %d expansion %d myrank %d \n", level, boxsize, npx, expansion, myrank);



    s64 nrank = npx*npx*npx;
    s64 myx = myrank%npx;
    s64 myy = (myrank/npx)%npx;
    s64 myz = myrank/(npx*npx);

    s64 i, j, k;
    s64 lx = myx;
    s64 ly = myy;
    s64 lz = myz;
    s64 lp = npx;



ocrPrintf("myrank %d level %d boxsize %d exp %d lp %d myx %d myy %d myz %d npx %d\n",pbPTR->myrank[level], level, boxsize, expansion, lp, myx, myy, myz, npx);

    for(j=0;j<26;j++) {
        pbPTR->haloSendEVT[level][j] = UNINITIALIZED_GUID;
        pbPTR->haloRecvEVT[level][j] = UNINITIALIZED_GUID;
    }

    //lx = myx/expansion;
    //ly = myy/expansion;
    //lz = myz/expansion;

printf("myrank %d level %d exp %d lp %d lx %d ly %d lz %d \n", myrank, level, expansion, lp, lx, ly, lz);

        if(lz == 0) {
            pbPTR->haloSendEVT[level][0] = NULL_GUID;
            pbPTR->haloSendEVT[level][1] = NULL_GUID;
            pbPTR->haloSendEVT[level][2] = NULL_GUID;
            pbPTR->haloSendEVT[level][3] = NULL_GUID;
            pbPTR->haloSendEVT[level][4] = NULL_GUID;
            pbPTR->haloSendEVT[level][5] = NULL_GUID;
            pbPTR->haloSendEVT[level][6] = NULL_GUID;
            pbPTR->haloSendEVT[level][7] = NULL_GUID;
            pbPTR->haloSendEVT[level][8] = NULL_GUID;
        }

        if(lz == lp-1) {
            pbPTR->haloSendEVT[level][17] = NULL_GUID;
            pbPTR->haloSendEVT[level][18] = NULL_GUID;
            pbPTR->haloSendEVT[level][19] = NULL_GUID;
            pbPTR->haloSendEVT[level][20] = NULL_GUID;
            pbPTR->haloSendEVT[level][21] = NULL_GUID;
            pbPTR->haloSendEVT[level][22] = NULL_GUID;
            pbPTR->haloSendEVT[level][23] = NULL_GUID;
            pbPTR->haloSendEVT[level][24] = NULL_GUID;
            pbPTR->haloSendEVT[level][25] = NULL_GUID;
        }


        if(ly == 0) {
            pbPTR->haloSendEVT[level][0] = NULL_GUID;
            pbPTR->haloSendEVT[level][1] = NULL_GUID;
            pbPTR->haloSendEVT[level][2] = NULL_GUID;
            pbPTR->haloSendEVT[level][9] = NULL_GUID;
            pbPTR->haloSendEVT[level][10] = NULL_GUID;
            pbPTR->haloSendEVT[level][11] = NULL_GUID;
            pbPTR->haloSendEVT[level][17] = NULL_GUID;
            pbPTR->haloSendEVT[level][18] = NULL_GUID;
            pbPTR->haloSendEVT[level][19] = NULL_GUID;
        }

        if(ly == lp-1) {
            pbPTR->haloSendEVT[level][6] = NULL_GUID;
            pbPTR->haloSendEVT[level][7] = NULL_GUID;
            pbPTR->haloSendEVT[level][8] = NULL_GUID;
            pbPTR->haloSendEVT[level][14] = NULL_GUID;
            pbPTR->haloSendEVT[level][15] = NULL_GUID;
            pbPTR->haloSendEVT[level][16] = NULL_GUID;
            pbPTR->haloSendEVT[level][23] = NULL_GUID;
            pbPTR->haloSendEVT[level][24] = NULL_GUID;
            pbPTR->haloSendEVT[level][25] = NULL_GUID;
        }


        if(lx == 0) {
            pbPTR->haloSendEVT[level][0] = NULL_GUID;
            pbPTR->haloSendEVT[level][3] = NULL_GUID;
            pbPTR->haloSendEVT[level][6] = NULL_GUID;
            pbPTR->haloSendEVT[level][9] = NULL_GUID;
            pbPTR->haloSendEVT[level][12] = NULL_GUID;
            pbPTR->haloSendEVT[level][14] = NULL_GUID;
            pbPTR->haloSendEVT[level][17] = NULL_GUID;
            pbPTR->haloSendEVT[level][20] = NULL_GUID;
            pbPTR->haloSendEVT[level][23] = NULL_GUID;
        }

        if(lx == lp-1) {
            pbPTR->haloSendEVT[level][2] = NULL_GUID;
            pbPTR->haloSendEVT[level][5] = NULL_GUID;
            pbPTR->haloSendEVT[level][8] = NULL_GUID;
            pbPTR->haloSendEVT[level][11] = NULL_GUID;
            pbPTR->haloSendEVT[level][13] = NULL_GUID;
            pbPTR->haloSendEVT[level][16] = NULL_GUID;
            pbPTR->haloSendEVT[level][19] = NULL_GUID;
            pbPTR->haloSendEVT[level][22] = NULL_GUID;
            pbPTR->haloSendEVT[level][25] = NULL_GUID;
        }



    u32 ind = 0;
    u32 index = 0;
    ocrGuid_t stickyEVT;

        for(k=-1;k<2;k++)
            for(j=-1;j<2;j++)
                for(i=-1;i<2;i++) {
                    if(i==0 && j==0 && k==0) continue; //skip "center" of cube
//ocrPrintf("ind %d "GUIDF" "GUIDF" "GUIDF"\n", ind,GUIDA(pbPTR->haloSendEVT[level][ind]), GUIDA(NULL_GUID), GUIDA(UNINITIALIZED_GUID));
                    if(ocrGuidIsNull(pbPTR->haloSendEVT[level][ind])) {
                        pbPTR->haloRecvEVT[level][ind] = NULL_GUID;
                    } else {

//send
                    index = 26*nrank*(nlevels-level) + 26*myrank + ind;
                    ocrGuidFromIndex(&(stickyEVT), pbPTR->haloRangeGUID, index);
                    ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
                    pbPTR->haloSendEVT[level][ind] = stickyEVT;
ocrPrintf("I%d i%d j%d k%d send ind %d index in %d guid "GUIDF" \n", myrank, i, j, k, ind, index, GUIDA(stickyEVT));

//receive
                    index = 26*nrank*(nlevels-level) + 26*(myrank + k*npx*npx + j*npx + i) + 25-ind;
                    ocrGuidFromIndex(&(stickyEVT), pbPTR->haloRangeGUID, index);
                    ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
                    pbPTR->haloRecvEVT[level][ind] = stickyEVT;
ocrPrintf("I%d i%d j%d k%d recv ind %d index in %d guid "GUIDF" "GUIDF" \n", myrank, i, j, k, ind, index, GUIDA(stickyEVT), GUIDA(pbPTR->haloRecvEVT[level][ind]));

                }
                ind++;
        }

for(i=0;i<26;i++) ocrPrintf("HI%d i %d sendEVT "GUIDF" recvEVT "GUIDF" \n", myrank, i, GUIDA(pbPTR->haloSendEVT[level][i]), GUIDA(pbPTR->haloRecvEVT[level][i]));

        if(boxsize != AGGLOMERATION_SIZE) return;


    for(j=0;j<7;j++) {
        pbPTR->coarsenRecvEVT[level][j] = NULL_GUID;
        pbPTR->refineSendEVT[level][j] = NULL_GUID;
    }
    pbPTR->coarsenSendEVT[level] = NULL_GUID;
    pbPTR->refineRecvEVT[level] = NULL_GUID;


    //if((myx%expansion) || (myy%expansion) || (myz%expansion)) return; //I'm not part of this level


        u32 target;

        if((lx%2) || (ly%2) || (lz%2)) {  //my last level, set coarsenSend and refineRecv
            ind = 4*(lz%2) + 2*(ly%2) + (lx%2)-1;
            ocrGuidFromIndex(&(stickyEVT), pbPTR->coarsenRangeGUID, 7*nrank*(nlevels - level) + 7*myrank + ind);
            ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            pbPTR->coarsenSendEVT[level] = stickyEVT;

            target = 2*(lx/2) + 2*(ly/2)*npx + 2*(lz/2)*npx*npx;
printf("target %d ind %d \n");
            ocrGuidFromIndex(&(stickyEVT), pbPTR->refineRangeGUID, 7*nrank*(nlevels - level) + 7*target + ind);
            ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            pbPTR->refineRecvEVT[level] = stickyEVT;
            pbPTR->myrank[level-1] = -1;
        } else { //set 7 coarsen receives and 7 refine sends

        ind = 0;

        for(k=0;k<2;k++)
            for(j=0;j<2;j++)
                for(i=0;i<2;i++) {
                    if(i==0 && j==0 && k==0) continue; //don't pair with myself
            ocrGuidFromIndex(&(stickyEVT), pbPTR->refineRangeGUID, 7*nrank*(nlevels - level) + 7*myrank + ind);
            ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            pbPTR->refineSendEVT[level][ind] = stickyEVT;

            target = myrank + i+ j*npx+ k*npx*npx;
printf("npx %d target %d ind %d \n", npx, target, ind);
            ocrGuidFromIndex(&(stickyEVT), pbPTR->coarsenRangeGUID, 7*nrank*(nlevels - level) + 7*target + ind);
            ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            pbPTR->coarsenRecvEVT[level][ind] = stickyEVT;

            ind++;
            }

       pbPTR->myrank[level-1] = lx/2 + (ly/2)*npx/2 + (lz/2)*npx*npx/4;
printf("lx %d ly %d lz %d npx %d myrank %d\n",lx, ly, lz, npx, pbPTR->myrank[level-1]);
    }
for(i=0;i<7;i++) ocrPrintf("HI%d i %d coarseRecvEVT "GUIDF" refineSendEVT "GUIDF" \n", myrank, i, GUIDA(pbPTR->coarsenRecvEVT[level][i]), GUIDA(pbPTR->refineSendEVT[level][i]));
ocrPrintf("HI%d i %d coarseSendEVT "GUIDF" refineRecvEVT "GUIDF" \n", myrank, i, GUIDA(pbPTR->coarsenSendEVT[level]), GUIDA(pbPTR->refineRecvEVT[level]));

//create reductionPrivate block



    ocrDbCreate(&(pbPTR->reductionPrivatePTR[level]), (void**) &rpPTR, sizeof(reductionPrivate_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    rpPTR->new = 1;
    rpPTR->nrank = npx*npx*npx;
    rpPTR->myrank = myrank;
    rpPTR->ndata = 1;
    rpPTR->reductionType = ALLREDUCE;
    rpPTR->reductionOperator = REDUCTION_F8_ADD;
    rpPTR->rangeGuid = pbPTR->reductionRangeGuid[level];
    rpPTR->returnEVT = pbPTR->returnEVT[level];






64 getMyPDc(u64 PDstart, u64 PDend, u32 myx, u32 myy, u32 myz, u32 npx, u32 npy, u32 npz) {

/printf("pdstart %d pdend %d myx %d myy %d myz %d npx %d npy %d npz %d \n", PDstart, PDend, myx, myy, myz, npx, npy, npz);

f(PDstart == PDend || (npx == 1 && npy == 1 && npz == 1)) return(PDstart);

f(npx >= npy && npx >= npz) {  // divide x dir

   if(myx < npx/2) return(getMyPDc(PDstart, (PDstart+PDend-1)/2, myx, myy, myz, npx/2, npy, npz));
    else return(getMyPDc((PDstart+PDend-1)/2 + 1, PDend, myx-npx/2, myy, myz, (npx+1)/2, npy, npz));



f(npy >= npz) { //divide in y dir

   if(myy < npy/2) return(getMyPDc(PDstart, (PDstart+PDend-1)/2, myx, myy, myz, npx, npy/2, npz));
    else return(getMyPDc((PDstart+PDend-1)/2 + 1, PDend, myx, myy-npy/2, myz, npx, (npy+1)/2, npz));


/divide in z dir

   if(myz < npz/2) return(getMyPDc(PDstart, (PDstart+PDend-1)/2, myx, myy, myz, npx, npy, npz/2));
    else return(getMyPDc((PDstart+PDend-1)/2 + 1, PDend, myx, myy, myz-npz/2, npx, npy, (npz+1)/2));



64 getMyPD(u64 PDstart, u64 PDend, u64 myrank, u32 npx, u32 npy, u32 npz) {


/compute the "best" PD for this myrank

32   myx = myrank%npx;
32   myy = (myrank/npx)%npy;
32   myz = myrank/(npx*npy);

eturn(getMyPDc(PDstart, PDend, myx, myy, myz, npx, npy, npz));





64 f2c(u64 index, u64 size){
/converts index into a cube of linear dimension size into
/the correct index for blocing up and embedding in a cube twice the size

64 z = index/(size*size);
64 y = (index/size)%size;
64 x = index%size;
64 result = ((z*4*size) + 2*y)*2*size + 2*x;


eturn result;















ypedef struct {
   u64 startTime;
 wrapUpPRM_t;

ypedef struct {
   ocrEdtDep_t returnBlock;
 wrapUpDEPV_t;

ocrGuid_t wrapUpEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
//prints out the sum of deviations from all 1s

    DEPVDEF(wrapUp);
    PRMDEF(wrapUp);
    double * returnPTR = (double *) DEPV(wrapUp,returnBlock,ptr);
    ocrPrintf("final deviation: %f \n", returnPTR[0]);
    u64 stop = getTime();
    double elapsed = TICK*(stop - PRM(wrapUp,startTime));
    ocrPrintf("elapsed time: %f \n", elapsed);
    ocrShutdown();
    return NULL_GUID;
}





ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {


/*
mainEDT is executed first
Creates and fills the shared datablock
Creates and fills the reduction shared datablock
*/

    u64 npx, boxsize, debug, levels;

    u32 _paramc, _depc, _idep;

    void * programArgv = depv[0].ptr;
    u32 argc = ocrGetArgc(programArgv);

    u64 startTime = getTime();

    if(argc < 3) bomb("must input log2(box size) and log2(cuberoot(ranks))");
    if(argc > 4) bomb("no fourth parameter, third parameter is optional debug level");

    boxsize = (u64) atoi(ocrGetArgv(programArgv, 1));
    if(boxsize < 4) bomb("log2(boxsize) must be at least 4");
    if(boxsize > 10) bomb("log2(boxsize) must be at most 10");

    npx = (u64) atoi(ocrGetArgv(programArgv, 2));
    if(npx < 0) bomb("log2(cuberoot(ranks)) must be nonnegative");
    if(npx > 4) bomb("log2(cuberoot(ranks)) must 4 or less");


    debug = 0;
    if(argc == 4) debug = atoi(ocrGetArgv(programArgv, 3));


    ocrPrintf("HPGMG fourth order FV\n");
    ocrPrintf("log2 box size = %d \n", boxsize);
    ocrPrintf("log2(cuberoot(ranks)) = %d \n", npx);
    ocrPrintf("debug = %d\n", debug);
    ocrPrintf("startTime %u \n", startTime);

    u64 nlevels = npx + boxsize;


    boxsize = 1 << boxsize;
    npx = 1 << npx;

    printf("nlevels %d boxsize %d npx %d \n", nlevels, boxsize, npx);

    ocrGuid_t sbDBK;
    sharedBlock_t * sbPTR;
    ocrDbCreate(&sbDBK, (void**) &sbPTR, sizeof(sharedBlock_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC);

    sbPTR->npx     = npx;
    sbPTR->boxsize = boxsize;
    sbPTR->debug   = debug;
    u64 nrank = npx * npx * npx;
    sbPTR->nrank = nrank;


    ocrGuidRangeCreate(&(sbPTR->haloRangeGUID), 26*nrank*nlevels, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(sbPTR->refineRangeGUID), 7*nrank*nlevels, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(sbPTR->coarsenRangeGUID), 7*nrank*nlevels, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(sbPTR->reductionRangeGUID), nrank*nlevels, GUID_USER_EVENT_STICKY);

    ocrGuid_t pbDBK;
    privateBlock_t * pbPTR;
    ocrDbCreate(&pbDBK, (void**) &pbPTR, sizeof(privateBlock_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC);

    pbPTR->nrank = nrank;
    pbPTR->myrank[nlevels-1] = debug;
    pbPTR->npx[nlevels-1] = npx;
    pbPTR->nlevels = nlevels;
    pbPTR->boxsize[nlevels-1] = boxsize;
    pbPTR->haloRangeGUID = sbPTR->haloRangeGUID;
    pbPTR->refineRangeGUID = sbPTR->refineRangeGUID;
    pbPTR->coarsenRangeGUID = sbPTR->coarsenRangeGUID;


    u64 level;
    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 2;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;
    for(level=nlevels;level>=1;level--)
        ocrEventCreateParams(&(pbPTR->returnEVT[level]), OCR_EVENT_CHANNE    L_T, false, &params);

    printf("myrank %d nlevels %d boxsize %d\n", debug, nlevels, pbPTR->boxsize[nlevels-1]);

    for(level=nlevels;level>=1;level--) initializeEvents(level-1, pbPTR);

    ocrShutdown();

/*
    ocrGuid_t wrapUpTML, wrapUpEDT;
    ocrEdtTemplateCreate(&wrapUpTML, wrapUpEdt, PRMNUM(wrapUp), DEPVNUM(wrapUp));
    ocrEdtCreate(&wrapUpEDT, wrapUpTML, EDT_PARAM_DEF, (u64 *) &(startTime), EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
    ocrEventCreate(&(sbPTR->finalOnceEVT), OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
    ocrEdtTemplateDestroy(wrapUpTML);

    ocrAddDependence(sbPTR->finalOnceEVT, wrapUpEDT, SLOT(wrapUp,returnBlock), DB_MODE_RO);

    ocrDbRelease(sbDBK);

    ocrGuid_t initTML, initEDT;
    ocrEdtTemplateCreate(&initTML, initEdt, PRMNUM(init), DEPVNUM(init));

    u64 myrank;
    ocrGuid_t myAffinity;
#ifdef AFFINITY

    u64 PDcount, myPD;
    ocrAffinityCount(AFFINITY_PD, &PDcount);
#endif

    for(myrank=0;myrank<nrank;myrank++) {
#ifdef AFFINITY
        myPD = getMyPD(0L, PDcount-1, myrank, npx, npx, npx);
        ocrAffinityGetAt(AFFINITY_PD, myPD, &(myAffinity));
        ocrHint_t myHNT;
        ocrHintInit(&myHNT,OCR_HINT_EDT_T);
        ocrSetHintValue(&myHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));

        ocrEdtCreate(&initEDT, initTML, EDT_PARAM_DEF, &myrank, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &myHNT, NULL);
#else
        ocrEdtCreate(&initEDT, initTML, EDT_PARAM_DEF, &myrank, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif
        ocrAddDependence(sbDBK, initEDT, SLOT(init,sharedBlock), DB_MODE_RO);
    }

    ocrEdtTemplateDestroy(initTML);

    if(debug != 0) ocrPrintf("M finish\n");

*/
    return NULL_GUID;

}



