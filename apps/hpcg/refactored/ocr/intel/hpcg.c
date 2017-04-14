/*
Author David S. Scott
Copywrite Intel Corporation 2016

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.

*/

/*
This code implements HPCG in OCR.
It uses labelled GUIDs and channel events that are currently extensions

More information is in README

October 25 2015: added runtime arguments
October 27 2015: fixed "RO" to "RW" for the private block
November 15 2015: changed to the newer reduction library
November 30 2015: added AFFINITY extensions
December 8 2015: added macros "library" and structures for params and deps
April 2016: added channel events, brought up to release 1.1.0 including affinity HINTS
June 2016: added "AFFINITY" definition to allow it to be turned off easily
Sept 2016: eliminated realMainEdt and hpcgInitEdt and packEdt (modifying created datablocks)
Sept 2016: changed timer function
Sept 2016: fixed 3 bugs: f2c, wrong update coming up in multigrid, wrong vector in one call to Halo Exchange
Sept 2016: added "NO_*" controls
Sept 2016: added timing each phase separately on each node
Oct  2016: modified to support reduction library change (ALLREDUCE, REDUCE, BROADCAST)

*/

//#define TIMER//undefine to suppress timing of haloexchange and unpack

#include "timers.h"
#define TICK (1.0e-6)

#ifndef NO_COMPUTE
#define COMPUTE  //undefine to suppress compute
#endif

#ifndef NO_AFFINITY
#define AFFINITY
#endif

#ifndef NO_PRECONDITIONER
#define PRECONDITIONER  //undefine if you want to run without the precondition
#endif
//#define RECEIVER_OWNS_CHANNEL_EVENT //undefine to make the sender own it

#ifndef ENABLE_EXTENSION_LABELING
#define ENABLE_EXTENSION_LABELING
#endif
#ifndef ENABLE_EXTENSION_AFFINITY
#define ENABLE_EXTENSION_AFFINITY
#endif

#include "ocr.h"
#include "stdlib.h"
#include "extensions/ocr-labeling.h" //currently needed for labeled guids
#include "extensions/ocr-affinity.h" //needed for affinity


#ifdef USE_PROFILER
#include "extensions/ocr-profiler.h"
#endif

#include "string.h" //if memcpy is needed
#include "stdio.h"  //needed for PRINTF debugging
#include "math.h" //for integer abs

#include "reduction.h"
#include "macros.h"


#define PHASES 54
#define NPX 3  //number of workers is NPX x NPY x NPZ
#define NPY 4
#define NPZ 5
#define M 16   // size of local block
#define HPCGMAXITER 50
#define T 50  //number of time steps
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
    u32 npx;
    u32 npy;
    u32 npz;
    u32 m;
    u32 maxIter;
    u32 debug;
    ocrGuid_t haloRangeGUID;
    ocrGuid_t reductionRangeGUID;
    ocrGuid_t finalOnceEVT;
} sharedBlock_t;


typedef struct{
    u32 nrank;
    u32 myrank;
    u32 timestep;
    u32 npx;
    u32 npy;
    u32 npz;
    u32 maxIter;
    u32 debug;
    u32 m[4];
    u32 mt[4];
    u32 ht[4];
    u32 hpcgPhase;
    u32 mgPhase[4];
//offsets in bytes for the various structures in the datablock beyond the private structure
    u32 matrix_offset[4];
    u32 column_index_offset[4];
    u32 diagonal_index_offset[4];
    u32 vector_offset[4][6];
    double rtr;
    double rtr0;
    double rtz;
    double rtzold;
    gather_t gather[4];
    ocrGuid_t reductionReturnEVT;
    ocrHint_t myAffinityHNT;
    ocrGuid_t haloRangeGUID;
    ocrGuid_t hpcgTML;
    ocrGuid_t spmvTML;
    ocrGuid_t mgTML;
    ocrGuid_t haloExchangeTML;
    ocrGuid_t smoothTML;
    ocrGuid_t packTML;
    ocrGuid_t unpackTML;
    ocrGuid_t haloSendEVT[26];
    ocrGuid_t haloRecvEVT[26];
    ocrGuid_t finalOnceEVT;
//WARNING: time only works for <=30 iterations

#ifdef TIMER
    u64 start[HPCGMAXITER+1][PHASES];
    u64 end[HPCGMAXITER+1][PHASES];
    u64 timePhase;
#endif
    } privateBlock_t;

//indices of vectors (arbitrary order)


void bomb(char * s) {
PRINTF("ERROR %s TERMINATING\n", s);
ocrShutdown();
return;
}


u64 getMyPDc(u64 PDstart, u64 PDend, u32 myx, u32 myy, u32 myz, u32 npx, u32 npy, u32 npz) {

//printf("pdstart %d pdend %d myx %d myy %d myz %d npx %d npy %d npz %d \n", PDstart, PDend, myx, myy, myz, npx, npy, npz);

if(PDstart == PDend || (npx == 1 && npy == 1 && npz == 1)) return(PDstart);

if(npx >= npy && npx >= npz) {  // divide x dir

    if(myx < npx/2) return(getMyPDc(PDstart, (PDstart+PDend-1)/2, myx, myy, myz, npx/2, npy, npz));
     else return(getMyPDc((PDstart+PDend-1)/2 + 1, PDend, myx-npx/2, myy, myz, (npx+1)/2, npy, npz));

}

if(npy >= npz) { //divide in y dir

    if(myy < npy/2) return(getMyPDc(PDstart, (PDstart+PDend-1)/2, myx, myy, myz, npx, npy/2, npz));
     else return(getMyPDc((PDstart+PDend-1)/2 + 1, PDend, myx, myy-npy/2, myz, npx, (npy+1)/2, npz));

}
//divide in z dir

    if(myz < npz/2) return(getMyPDc(PDstart, (PDstart+PDend-1)/2, myx, myy, myz, npx, npy, npz/2));
     else return(getMyPDc((PDstart+PDend-1)/2 + 1, PDend, myx, myy, myz-npz/2, npx, npy, (npz+1)/2));

}

u64 getMyPD(u64 PDstart, u64 PDend, u64 myrank, u32 npx, u32 npy, u32 npz) {


//compute the "best" PD for this myrank

u32   myx = myrank%npx;
u32   myy = (myrank/npx)%npy;
u32   myz = myrank/(npx*npy);

return(getMyPDc(PDstart, PDend, myx, myy, myz, npx, npy, npz));
}




u64 f2c(u64 index, u64 size){
//converts index into a cube of linear dimension size into
//the correct index for blocing up and embedding in a cube twice the size

u64 z = index/(size*size);
u64 y = (index/size)%size;
u64 x = index%size;
u64 result = ((z*4*size) + 2*y)*2*size + 2*x;


return result;
}


void gatherfill(gather_t * g, u32 m1){
u32 m2 = m1*m1;
u32 m3 = m1*m1*m1;

g->length[0] = 1;
g->length[1] = m1;
g->length[2] = 1;
g->length[3] = m1;
g->length[4] = m2;
g->length[5] = m1;
g->length[6] = 1;
g->length[7] = m1;
g->length[8] = 1;
g->length[9] = m1;
g->length[10] = m2;
g->length[11] = m1;
g->length[12] = m2;
g->length[13] = m2;
g->length[14] = m1;
g->length[15] = m2;
g->length[16] = m1;
g->length[17] = 1;
g->length[18] = m1;
g->length[19] = 1;
g->length[20] = m1;
g->length[21] = m2;
g->length[22] = m1;
g->length[23] = 1;
g->length[24] = m1;
g->length[25] = 1;

g->start[0] = 0;
g->start[1] = 0;
g->start[2] = m1-1;
g->start[3] = 0;
g->start[4] = 0;
g->start[5] = m1-1;
g->start[6] = m2-m1;
g->start[7] = m2-m1;
g->start[8] = m2-1;
g->start[9] = 0;
g->start[10] = 0;
g->start[11] = m1-1;
g->start[12] = 0;
g->start[13] = m1-1;
g->start[14] = m2-m1;
g->start[15] = m2-m1;
g->start[16] = m2-1;
g->start[17] = (m1-1)*m2;
g->start[18] = (m1-1)*m2;
g->start[19] = m3-m2+m1-1;
g->start[20] = (m1-1)*m2;
g->start[21] = (m1-1)*m2;
g->start[22] = m3-m2+m1-1;
g->start[23] = m3-m1;
g->start[24] = m3-m1;
g->start[25] = m3-1;

g->l1[0] = 1;
g->l1[1] = m1;
g->l1[2] = 1;
g->l1[3] = m1;
g->l1[4] = m2;
g->l1[5] = m1;
g->l1[6] = 1;
g->l1[7] = m1;
g->l1[8] = 1;
g->l1[9] = m1;
g->l1[10] = m1;
g->l1[11] = m1;
g->l1[12] = m2;
g->l1[13] = m2;
g->l1[14] = m1;
g->l1[15] = m1;
g->l1[16] = m1;
g->l1[17] = 1;
g->l1[18] = m1;
g->l1[19] = 1;
g->l1[20] = m1;
g->l1[21] = m2;
g->l1[22] = m1;
g->l1[23] = 1;
g->l1[24] = m1;
g->l1[25] = 1;

g->l2[0] = 1;
g->l2[1] = 1;
g->l2[2] = 1;
g->l2[3] = 1;
g->l2[4] = 1;
g->l2[5] = 1;
g->l2[6] = 1;
g->l2[7] = 1;
g->l2[8] = 1;
g->l2[9] = 1;
g->l2[10] = m1;
g->l2[11] = 1;
g->l2[12] = 1;
g->l2[13] = 1;
g->l2[14] = 1;
g->l2[15] = m1;
g->l2[16] = 1;
g->l2[17] = 1;
g->l2[18] = 1;
g->l2[19] = 1;
g->l2[20] = 1;
g->l2[21] = 1;
g->l2[22] = 1;
g->l2[23] = 1;
g->l2[24] = 1;
g->l2[25] = 1;

g->p1[0] = 1;
g->p1[1] = 1;
g->p1[2] = 1;
g->p1[3] = m1;
g->p1[4] = 1;
g->p1[5] = m1;
g->p1[6] = 1;
g->p1[7] = 1;
g->p1[8] = 1;
g->p1[9] = m2;
g->p1[10] = 1;
g->p1[11] = m2;
g->p1[12] = m1;
g->p1[13] = m1;
g->p1[14] = m2;
g->p1[15] = 1;
g->p1[16] = m2;
g->p1[17] = 1;
g->p1[18] = 1;
g->p1[19] = 1;
g->p1[20] = m1;
g->p1[21] = 1;
g->p1[22] = m1;
g->p1[23] = 1;
g->p1[24] = 1;
g->p1[25] = 1;

g->p2[0] = 1;
g->p2[1] = 1;
g->p2[2] = 1;
g->p2[3] = 1;
g->p2[4] = 1;
g->p2[5] = 1;
g->p2[6] = 1;
g->p2[7] = 1;
g->p2[8] = 1;
g->p2[9] = 1;
g->p2[10] = m2-m1;
g->p2[11] = 1;
g->p2[12] = 1;
g->p2[13] = 1;
g->p2[14] = 1;
g->p2[15] = m2-m1;
g->p2[16] = 1;
g->p2[17] = 1;
g->p2[18] = 1;
g->p2[19] = 1;
g->p2[20] = 1;
g->p2[21] = 1;
g->p2[22] = 1;
g->p2[23] = 1;
g->p2[24] = 1;
g->p2[25] = 1;
return;
}



void compute_column_indices(u32 size, s32 * ind) {
s32 i, j, k, c;
//size is m[]
//creates an HT* x HT* x HT* brick of indices for the column numbers in matrixfill


    c=0;

//interior
    for(k=1;k<size-1;k++)
        for(j=1;j<size-1;j++)
            for(i=1;i<size-1;i++)
               ind[i + j*size + k*size*size] = c++;

//fill in halo
//26 pieces, some fused

//0, 1, 2
for(i=0;i<size;i++) ind[i] = c++;
//3
for(j=1;j<size-1;j++) ind[j*size] = c++;
//4
for(j=1;j<size-1;j++) for(i=1;i<size-1;i++)  ind[j*size+i] = c++;
//5
for(j=1;j<size-1;j++) ind[(size-1) + j*size] = c++;
//6, 7, 8
for(i=0;i<size;i++) ind[i + (size-1)*size] = c++;
//9
for(k=1;k<size-1;k++) ind[k*size*size] = c++;
//10
for(k=1;k<size-1;k++) for(i=1;i<size-1;i++)  ind[k*size*size + i] = c++;
//11
for(k=1;k<size-1;k++) ind[size-1 + k*size*size] = c++;
//12
for(k=1;k<size-1;k++) for(j=1;j<size-1;j++) ind[k*size*size + j*size] = c++;
//13
for(k=1;k<size-1;k++) for(j=1;j<size-1;j++) ind[size-1 + k*size*size + j*size] = c++;
//14
for(k=1;k<size-1;k++) ind[(size-1)*size + k*size*size] = c++;
//15
for(k=1;k<size-1;k++) for(i=1;i<size-1;i++) ind[size*(size-1) + k*size*size + i] = c++;
//16
for(k=1;k<size-1;k++) ind[size*size-1 + k*size*size] = c++;
//17, 18, 19
for(i=0;i<size;i++) ind[size*size*(size-1) + i] = c++;
//20
for(j=1;j<size-1;j++) ind[size*size*(size-1) + j*size] = c++;
//21
for(j=1;j<size-1;j++) for(i=1;i<size-1;i++)  ind[size*size*(size-1) + j*size + i] = c++;
//22
for(j=1;j<size-1;j++) ind[size*size*(size-1)+size-1 + j*size] = c++;
//23, 24, 25
for(i=0;i<size;i++) ind[size*size*size - size + i] = c++;


    if(DEBUG > 0) {
        c=0;
        for(k=0;k<size;k++) {

            PRINTF("%d \n", k);
            for(j=0;j<size;j++) {
                for(i=0;i<size;i++) PRINTF("%5d ", ind[c++]);
                PRINTF("\n");
            }
        }
    }

return;
}

void vectorfill(u32 length, double * v, double * a) {
//length is mt*
//computes vector b as the row sums of A
    u32 i, j;


    for(i=0;i<length;i++) {
        *v = 0;
        for(j=0;j<27;j++) *v += *(a++);
        v++;
    }
    if(DEBUG > 1) {
        v -= length;
        for(i=0;i<length;i++) PRINTF("v ind %d val %f \n", i, v[i]);
    }
    return;
}

void matrixfill(u32 npx, u32 npy, u32 npz, u32 m, u32 myrank, double * a, u32 * diag, u32 * ind, s32 * column_index) {
//m is one of m[]
//a and ind  are 27*m*m*m
//diag is length m*m*m
//column index is (m+2)^3 which holds column offsets

if(DEBUG > 0) PRINTF("MF%d m %d \n", myrank, m);
    u32 n = m*m*m;
    s32 i, j, k, ml;
    s64 offset[27], *p;
    u32 index;

//used to zero out elements that are outside the global grid
    s64 znoff[9] = {-27,-26,-25,-24,-23,-22,-21,-20,-19};
    s64 ynoff[9] = {-27,-26,-25,-18,-17,-16,-9,-8,-7};
    s64 xnoff[9] = { -27,-24,-21,-18,-15,-12,-9,-6,-3};
    s64 zpoff[9] = {-9,-8,-7,-6,-5,-4,-3,-2,-1};
    s64 ypoff[9] = {-21,-20,-19,-12,-11,-10,-3,-2,-1};
    s64 xpoff[9] = {-25,-22,-19,-16,-13,-10,-7,-4,-1};
//locate myrank
    u32 pz = myrank/(npx*npy);
    u32 py = (myrank/npx)%npy;
    u32 px = myrank%(npx);

    u32 counter = 0;
    double * asave;
    u32 * diagsave;

    asave = a;
    diagsave = diag;


if(DEBUG > 0) PRINTF("MF%d px %d py %d pz %d \n", myrank, px, py, pz);

    p = offset;

    for(k=-1;k<2;k++)
        for(j=-1;j<2;j++)
            for(i=-1;i<2;i++) {
                *(p++) = k*(m+2)*(m+2) + j*(m+2) + i;
    }

if(DEBUG > 1) for(i=0;i<27;i++) PRINTF("MF%d offset %d value %d \n", myrank, i, offset[i]);

    compute_column_indices(m+2, column_index);

    for(k=1;k<m+1;k++)
        for(j=1;j<m+1;j++)
            for(i=1;i<m+1;i++) {
                index = i + j*(m+2) + k*(m+2)*(m+2);

                for(ml=0;ml<13;ml++) {
                    counter++;
                    *a++ = -1;
                    *ind++ = column_index[(int) (index + offset[ml])];
                }
                *diag++ = counter++;
                *a++ = 26;
                *ind++ = column_index[index]; //offset[13] = 0

                for(ml=14;ml<27;ml++) {
                     counter++;
                    *a++ = -1;
                    *ind++ = column_index[index + offset[ml]];
                }
//zero out elements of A that are outside the global boundary (legal to store them as zeros)
    if(px == 0 && i == 1) for(ml=0;ml<9;ml++) a[xnoff[ml]] = 0.0;
    if(py == 0 && j == 1) for(ml=0;ml<9;ml++) a[ynoff[ml]] = 0.0;
    if(pz == 0 && k == 1) for(ml=0;ml<9;ml++) a[znoff[ml]] = 0.0;
    if(px == npx-1 && i == m) for(ml=0;ml<9;ml++) a[xpoff[ml]] = 0.0;
    if(py == npy-1 && j == m) for(ml=0;ml<9;ml++) a[ypoff[ml]] = 0.0;
    if(pz == npz-1 && k == m) for(ml=0;ml<9;ml++) a[zpoff[ml]] = 0.0;
    }

if(DEBUG > 1) for(i=0;i<m*m*m;i++) PRINTF("MF i %d diag %d a %f\n", i, diagsave[i], asave[diagsave[i]]);
if(DEBUG > 0) PRINTF("MF%d finish %d \n", myrank, m);
}



typedef struct{
    u64 level;
    u64 vectorIndex;
    ocrGuid_t returnEVT;
    } unpackPRM_t;

typedef struct{
    ocrEdtDep_t privateBlock;
    ocrEdtDep_t block[26];
    } unpackDEPV_t;


ocrGuid_t unpackEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
param
0: level
1: vector index
2: returnEVT


depv
0: private block
1-26: datablocks(from Pack)

copies the values from the received data blocks into the destination vector
sends the private block (to either spmv or smooth)
*/
#ifdef USE_PROFILER
    START_PROFILE(app_unpack);
#endif
    PRMDEF(unpack);
    u64 level = PRM(unpack,level);
    u64 vectorIndex = PRM(unpack,vectorIndex);
    ocrGuid_t returnEVT = PRM(unpack,returnEVT);


    DEPVDEF(unpack);
    ocrGuid_t pbDBK = DEPV(unpack,privateBlock,guid);
    privateBlock_t * pbPTR = DEPV(unpack,privateBlock,ptr);
    void * ptr = (void *) pbPTR;

#ifdef TIMER
    u32 timePhase = pbPTR->timePhase;
    pbPTR->start[pbPTR->timestep][pbPTR->timePhase++] = getTime();
#endif


    double * recvPTR[26];
    ocrGuid_t recvDBK[26];

    u32 i;

    for(i=0;i<26;i++) {
        recvPTR[i] = DEPVARRAY(unpack,block,i,ptr);
        }

    u32 myrank = pbPTR->myrank;
    u32 debug = pbPTR->debug;
    gather_t * gather = &(pbPTR->gather[level]);


    double * dest = (double *) (((u64) ptr) + pbPTR->vector_offset[level][vectorIndex]);

if(debug > 1) for(i=0;i<27;i++) PRINTF("UN%d depv[%d] "GUIDF" \n", myrank, i, GUIDA(depv[i].guid));




    ocrGuid_t sticky;


    if(debug > 0) PRINTF("UN%d L%d \n", myrank, level);
    double * b;
if(debug >1 ) for(i=0;i<pbPTR->mt[level];i++) PRINTF("UN%d i %d v %f \n", myrank, i, *(dest+i));
//for(i=0;i<pbPTR->mt[level];i++) PRINTF("UN%d i %d v %f \n", myrank, i, *(dest+i));

    dest += pbPTR->mt[level];  //skip over existing vector






    u32 j, len;
    double * a;

#ifdef COMPUTE
    for(i=0;i<26;i++) {
if(debug > 1) PRINTF("UN%d start i %d\n", myrank, i);
        b = recvPTR[i];
        len = gather->l1[i]*gather->l2[i];
        if(b == NULL) for(j=0;j<len;j++) *dest++ = 0.0;
          else {

            for(j=0;j<len;j++) {
if(debug >1) PRINTF("UN%d i %d v %f\n", myrank, i, *b);
//PRINTF("rank%d UN i %d v %f\n", myrank, j, *b);
               *dest++ = *b++;
            }

        }
    }
#endif

    for(i=0;i<26;i++) {
        if(!ocrGuidIsNull(pbPTR->haloRecvEVT[i])) {
            ocrDbDestroy(DEPVARRAY(unpack,block,i,guid));
        }
    }

#ifdef TIMER
    pbPTR->end[pbPTR->timestep][timePhase] = getTime();
PRINTF("UN timePhase %d R%d timestep %d time %ld \n", timePhase, pbPTR->myrank, pbPTR->timestep, pbPTR->end[pbPTR->timestep][timePhase]-pbPTR->start[pbPTR->timestep][timePhase]);
#endif
    ocrDbRelease(pbDBK);
    ocrEventSatisfy(returnEVT, pbDBK);

#ifdef USE_PROFILER
    RETURN_PROFILE(NULL_GUID);
#endif
    return(NULL_GUID);
}


typedef struct{
    u64 level;
    u64 vectorIndex;
    ocrGuid_t unpackEVT;
    ocrGuid_t packEVT;
    } haloExchangePRM_t;

typedef struct{
    ocrEdtDep_t privateBlock;
    } haloExchangeDEPV_t;

ocrGuid_t haloExchangeEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {
/*
paramv
0: level
1: vector index
2: return event for unpack
3: return event for pack

depv
0: private block

launches unpack
packs data and sends

*/

#ifdef USE_PROFILER
    START_PROFILE(app_haloExchange);
#endif
    PRMDEF(haloExchange);
    u64 level = PRM(haloExchange,level);
    u64 vectorIndex = PRM(haloExchange,vectorIndex);
    ocrGuid_t unpackEVT = PRM(haloExchange,unpackEVT);
    ocrGuid_t packEVT = PRM(haloExchange,packEVT);

    DEPVDEF(haloExchange);
    ocrGuid_t pbDBK = DEPV(haloExchange,privateBlock,guid);
    privateBlock_t * pbPTR = DEPV(haloExchange,privateBlock,ptr);

    u64 time[5];
#ifdef TIMER
    u32 timePhase = pbPTR->timePhase;
    pbPTR->start[pbPTR->timestep][pbPTR->timePhase++] = getTime();
#endif

    time[0] = getTime();

    u32 debug = pbPTR->debug;


    u32 myrank = pbPTR->myrank;

    u32 i, errno;
    ocrGuid_t unpackEDT;



if(debug != 0) PRINTF("HE%d start\n", myrank);
if(debug != 0) PRINTF("HE%d start pbDBK "GUIDF" \n", myrank, GUIDA(pbDBK));
if(debug != 0) PRINTF("HE%d start level %d \n", myrank, level);
if(debug != 0) PRINTF("HE%d start vectorIndex %d \n", myrank, vectorIndex);
if(debug != 0) PRINTF("HE%d start unpackEVT "GUIDF" \n", myrank, GUIDA(unpackEVT));
if(debug != 0) PRINTF("HE%d start packEVT "GUIDF" \n", myrank, GUIDA(packEVT));

if(debug > 1) for(i=0;i<26;i++) PRINTF("HE%d i %d sendevent "GUIDF" \n", myrank, i, GUIDA(pbPTR->haloSendEVT[i]));
if(debug > 1) for(i=0;i<26;i++) PRINTF("HE%d i %d recvevent "GUIDF" \n", myrank, i, GUIDA(pbPTR->haloRecvEVT[i]));


    unpackPRM_t paramUnpack;
    unpackPRM_t * unpackPRM = &paramUnpack;
    PRM(unpack,level) = level;
    PRM(unpack,vectorIndex) = vectorIndex;
    PRM(unpack,returnEVT) = unpackEVT;
    ocrEdtCreate(&unpackEDT, pbPTR->unpackTML, EDT_PARAM_DEF, (u64 *) unpackPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
    for(i=0;i<26;i++) {
        ocrAddDependence(pbPTR->haloRecvEVT[i], unpackEDT, SLOTARRAY(unpack,block,i), DB_MODE_RO);
    }
//    ocrDbRelease(pbDBK);  don't need to release because nothing has changed
        time[1] = getTime();

if(debug > 1) PRINTF("HE%d after launch unpack\n", myrank);

    double * vector = (double *) (((u64)pbPTR) + pbPTR->vector_offset[level][vectorIndex]);
    ocrGuid_t haloDBK;
    gather_t * gather = &(pbPTR->gather[level]);
    double * sendPTR;
    u32 i1, i2;
    u32 * start = gather->start;
    u32 * l1 = gather->l1;
    u32 * l2 = gather->l2;
    u32 * p1 = gather->p1;
    u32 * p2 = gather->p2;
    double *src, * s, * d;

    time[2] = getTime();

    for(i=0;i<26;i++) {
        if(!ocrGuidIsNull(pbPTR->haloSendEVT[i])) {
            ocrDbCreate(&haloDBK, (void **) &sendPTR, (pbPTR->gather[level]).length[i]*sizeof(double), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
if(debug != 0) PRINTF("PK%d L%d \n", myrank, level);


if(debug != 0) PRINTF("PK%d L%d start vi %d pbDBK "GUIDF" \n", myrank, level, vectorIndex, GUIDA(pbDBK));
if(debug > 1) for(i=0;i<26;i++) PRINTF("PK%d i%d haloDBK "GUIDF" \n", myrank, i, GUIDA(haloDBK));


    src = vector;


#ifdef COMPUTE

        d = sendPTR;

        s = src + *start;
        for(i2=0;i2<*l2;i2++) {
            for(i1=0;i1<*l1;i1++) {
                *(d++) = *s;
                s += *p1;
            }
            s += *p2;
        }
#endif
if(debug > 0) PRINTF("PK%d L%d  DIR%d satisfy "GUIDF" with "GUIDF" \n", myrank, level, i, GUIDA(pbPTR->haloSendEVT[i]), GUIDA(haloDBK));
            ocrDbRelease(haloDBK);
            ocrEventSatisfy(pbPTR->haloSendEVT[i], haloDBK);
         }

         start++;
         l1++;
         l2++;
         p1++;
         p2++;

    }

#ifdef TIMER
    pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
//PRINTF("HE timePhase %d R%d timestep %d time %ld \n", timePhase, pbPTR->myrank, pbPTR->timestep, pbPTR->end[pbPTR->timestep][timePhase]-pbPTR->start[pbPTR->timestep][timePhase]);

    time[3] = getTime();
    ocrDbRelease(pbDBK);
    ocrAddDependence(pbDBK, unpackEDT, SLOT(unpack,privateBlock), DB_MODE_RW);
    ocrEventSatisfy(packEVT, NULL_GUID);

if(debug != 0) PRINTF("HE%d finish\n", myrank);
    time[4] = getTime();
#ifdef TIMER
    PRINTF("HE timePhase %d R%d timestep %d addD %ld create %ld satisfy %ld finish %ld\n", timePhase, pbPTR->myrank, pbPTR->timestep, time[1]-time[0], time[2]-time[1], time[3]-time[2], time[4]-time[3]);
#endif
#ifdef USE_PROFILER
    RETURN_PROFILE(NULL_GUID);
#endif
    return(NULL_GUID);

}



typedef struct{
    u64 level;
    u64 vectorIndex;
    u64 rhsIndex;
    ocrGuid_t returnEVT;
    } smoothPRM_t;

typedef struct{
    ocrEdtDep_t privateBlock;
    ocrEdtDep_t packEVT;
    } smoothDEPV_t;


ocrGuid_t smoothEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv
0: level
1: vector index
2: rhs index
3: return event

depv
0: private block from unpack through unpackEVT
1: packEVT (control)

smooth does a forward and backward Gauss-Seidel sweep
then sends the private block back

*/

#ifdef USE_PROFILER
    START_PROFILE(app_smooth);
#endif
    PRMDEF(smooth);
    u64 level = PRM(smooth,level);
    u64 vectorIndex = PRM(smooth,vectorIndex);
    u64 rhsIndex = PRM(smooth,rhsIndex);
    ocrGuid_t returnEVT = PRM(smooth,returnEVT);

    DEPVDEF(smooth);
    ocrGuid_t pbDBK = DEPV(smooth,privateBlock,guid);
    privateBlock_t * pbPTR = DEPV(smooth,privateBlock,ptr);
    void * ptr = pbPTR;

#ifdef TIMER
    u32 timePhase = pbPTR->timePhase;
    pbPTR->start[pbPTR->timestep][pbPTR->timePhase++] = getTime();
#endif
//if(pbPTR->myrank==0) PRINTF("start smooth level %d timetimePhase %d\n", level, pbPTR->timePhase);


    double * abase = (double *) (((u64) ptr) + pbPTR->matrix_offset[level]);
    u32 * indbase = (u32 *) (((u64) ptr) + pbPTR->column_index_offset[level]);
    u32 * diagbase = (u32 *) (((u64) ptr) + pbPTR->diagonal_index_offset[level]);
    double * vectorbase = (double *) (((u64) ptr) + pbPTR->vector_offset[level][vectorIndex]);
    double * rhsbase = (double *) (((u64) ptr) + pbPTR->vector_offset[level][rhsIndex]);

    u32 length = pbPTR->mt[level];
    u32 myrank = pbPTR->myrank;
    u32 debug = pbPTR->debug;


    s32 i, j;
    double sum;


//for(i=0;i<length;i++) PRINTF("sm before i%d v %f \n", i, vectorbase[i]);

if(debug >= 1) PRINTF("SM%d L%d V%d rhs%d start \n", myrank, level, vectorIndex, rhsIndex);
//for(i=0;i<pbPTR->ht[level];i++) PRINTF("rank%d SM before i%d v %f \n", myrank, i, vectorbase[i]);

    u32 * ind = indbase;
    u32 * diag = diagbase;
    double * a = abase;
    double * vector = vectorbase;
    double * rhs = rhsbase;

#ifdef COMPUTE
//forward sweep
    for(i=0;i<length;i++){
if(debug >= 2) PRINTF("SM%d i %d rhs %f \n", myrank, i, rhs[i]);
        sum = rhs[i];
        for(j=0;j<27;j++){
if(debug >= 2) PRINTF("SM%d i %d j %d sum %f a %f ind %d z %f \n", myrank, i, j, sum, a[j], ind[j], vectorbase[ind[j]]);
           sum -= a[j]*vectorbase[ind[j]];
        }
if(debug >= 2) PRINTF("SM%d diag index %d diag value %f\n", myrank, *diag, abase[*diag]);

        sum += abase[*diag]*vectorbase[indbase[*diag]];
        *(vector++) = sum/abase[*(diag++)];
if(debug >= 2) PRINTF("SM%d diag index %d diag value %f sum %f vector %f \n", myrank, *(diag-1), abase[*(diag-1)], sum, *(vector-1));
        a += 27;
        ind += 27;
    }
if(debug > 2){
    vector = vectorbase;
    for(i=0;i<pbPTR->mt[level];i++)
       PRINTF("SM%d L%d i %d val %f \n", myrank, level, i, vector[i]);
}

//for(i=0;i<length;i++) PRINTF("rank%d SM middle i %d v %f \n", myrank, i, vectorbase[i]);

if(debug >= 1) PRINTF("SM%d L%d finish forward\n", myrank, level);

//backward sweep
    a = abase + 27*(length - 1);
    ind = indbase + 27*(length - 1);
    diag = diagbase + length - 1;
    vector = vectorbase + length-1;
    rhs = rhsbase+length-1;;


    for(i=length-1;i>=0;i--){
        sum = rhsbase[i];
if(debug > 1) PRINTF("SM%d i %d sum %f \n", myrank, i, sum);
        for(j=26;j>=0;j--){  //backwards to help prefetching
if(debug > 2) PRINTF("SM%d i %d j %d sum %f a %f ind %d z %f \n", myrank, i, j, sum, a[j], ind[j], vectorbase[ind[j]]);
           sum -= a[j]*vectorbase[ind[j]];
        }
        sum += abase[*diag]*vectorbase[indbase[*diag]];
        *(vector--) = sum/abase[*(diag--)];
        a -= 27;
        ind -= 27;
if(debug > 1) PRINTF("SM%d after i%d v %f \n", myrank, i, vectorbase[i]);
    }

//for(i=0;i<length;i++) PRINTF("rank%d SM after i %d v %f \n", myrank, i, vectorbase[i]);
#endif


#ifdef TIMER
    pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
    ocrDbRelease(pbDBK);
    ocrEventSatisfy(returnEVT, pbDBK);

#ifdef USE_PROFILER
    RETURN_PROFILE(NULL_GUID);
#endif
    return NULL_GUID;
}

typedef struct{
    u64 level;
    u64 sourceIndex;
    u64 resultIndex;
    ocrGuid_t returnEVT;
    } spmvPRM_t;

typedef struct{
    ocrEdtDep_t privateBlock;
    ocrEdtDep_t packEVT;
    } spmvDEPV_t;

ocrGuid_t spmvEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv
0: level
1: source index
2: result index
3: return event

depv
0: private block (from unpack through unpackEVT)
1: packEVT (just control)


computes the local sparse matrix-vector product
sends the private block back
*/

#ifdef USE_PROFILER
    START_PROFILE(app_spmv);
#endif
    PRMDEF(spmv);
    u64 level = PRM(spmv,level);
    u64 sourceIndex = PRM(spmv,sourceIndex);
    u64 resultIndex = PRM(spmv,resultIndex);
    ocrGuid_t returnEVT = PRM(spmv,returnEVT);

    DEPVDEF(spmv);

    ocrGuid_t pbDBK = DEPV(spmv,privateBlock,guid);
    privateBlock_t * pbPTR = DEPV(spmv,privateBlock,ptr);
    void * ptr = pbPTR;

#ifdef TIMER
    u32 timePhase = pbPTR->timePhase;
    pbPTR->start[pbPTR->timestep][pbPTR->timePhase++] = getTime();
#endif
//if(pbPTR->myrank==0) PRINTF("start spmv level %d timetimePhase %d\n", level, pbPTR->timePhase);

#ifdef COMPUTE

    double * srcbase = (double *) ((u64) (ptr + pbPTR->vector_offset[level][sourceIndex]));
    double * destbase = (double *) ((u64) (ptr + pbPTR->vector_offset[level][resultIndex]));
    double * abase = (double *) ((u64) (ptr + pbPTR->matrix_offset[level]));
    u32 * indbase = (u32 *) ((u64) (ptr + pbPTR->column_index_offset[level]));

    u32 length = pbPTR->mt[level];

    u32 myrank = pbPTR->myrank;
    u32 debug = pbPTR->debug;


    u32 i, j;
    double sum;
    double * src = srcbase;
    double * dest = destbase;
    double * a = abase;
    u32 * ind = indbase;


    for(i=0;i<length;i++){
        sum = 0;
        for(j=0;j<27;j++){
if(debug >= 2) PRINTF("SPMV%d i %d j %d dest %f a %f ind %d src %f \n", myrank, i, j, sum, a[j], ind[j], src[ind[j]]);
           sum += a[j]*src[ind[j]];
        }
        *(dest++) = sum;
        a += 27;
        ind += 27;
    }
#endif

if(debug > 1) for(i=0;i<pbPTR->mt[level];i++)
for(i=0;i<length;i++) PRINTF("i %d dst %f \n", i, destbase[i]);

#ifdef TIMER
    pbPTR->end[pbPTR->timestep][timePhase] = getTime();
//if(pbPTR->myrank==0) PRINTF("end spmv level %d timephase %d\n", level, pbPTR->timePhase);
#endif

    ocrDbRelease(pbDBK);
    ocrEventSatisfy(returnEVT, pbDBK);
#ifdef USE_PROFILER
    RETURN_PROFILE(NULL_GUID);
#endif
    return NULL_GUID;
}


typedef struct{
    u64 mgStep;
    ocrGuid_t returnEVT;
    } mgPRM_t;

typedef struct{
    ocrEdtDep_t privateBlock;
    } mgDEPV_t;

ocrGuid_t mgEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
paramv
0: mgStep (0 to 6)
1: return event

depv
0: privateBlock  //returned by smooth or SPMV

this is the driver for the multigrid steps.

*/
#ifdef USE_PROFILER
    START_PROFILE(app_mg);
#endif
    PRMDEF(mg);
    u64 mgStep = PRM(mg,mgStep);;
    ocrGuid_t returnEVT = PRM(mg,returnEVT);

    u32 level = 3 - abs(mgStep-3);

    DEPVDEF(mg);
    ocrGuid_t pbDBK = DEPV(mg,privateBlock,guid);
    privateBlock_t * pbPTR = DEPV(mg,privateBlock,ptr);
    void * ptr = (void *) pbPTR;


    u64 m = pbPTR->m[level];
    u64 phase = pbPTR->mgPhase[level];
    u64 myrank = pbPTR->myrank;
    u64 debug = pbPTR->debug;

#ifdef TIMER
    u32 timePhase = pbPTR->timePhase;
    pbPTR->start[pbPTR->timestep][pbPTR->timePhase++] = getTime();
//if(pbPTR->myrank==0) PRINTF("start mgEdt level %d phase %d timephase %d\n", level, phase, pbPTR->timePhase);
#endif



    double * z, *ap, *zold, *znew, *r, *rold, *rnew;
    u64 i, size, time;

    ocrGuid_t haloExchangeEDT, packEVT, unpackEVT, smoothEDT, smoothEVT, spmvEDT, spmvEVT, mgEDT;
    haloExchangePRM_t haloExchangeParamv;
    haloExchangePRM_t * haloExchangePRM = &haloExchangeParamv;
    smoothPRM_t smoothParamv;
    smoothPRM_t * smoothPRM = &smoothParamv;
    spmvPRM_t spmvParamv;
    spmvPRM_t * spmvPRM = &spmvParamv;

    if(debug > 0) PRINTF("MG%d S%d P%d L%d start return event "GUIDF"\n", &pbPTR->myrank, mgStep, phase, level, GUIDA(returnEVT));

    switch(phase) {

        case 0: //smoothing

#ifdef COMPUTE
        z = (double *) (((u64) ptr) + pbPTR->vector_offset[level][Z]);
        for(i=0;i<pbPTR->mt[level];i++) z[i] = 0.0;
#endif

        if(debug > 0) PRINTF("MG%d S%d P%d L%d  create clone\n", &pbPTR->myrank, mgStep, phase, level);

//create clone, smooth, and halo
//TODO phase vs step??

        pbPTR->mgPhase[level] = 1;

//create clone which depends on smooth
        ocrEdtCreate(&mgEDT, pbPTR->mgTML, EDT_PARAM_DEF, (u64 *) mgPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
        ocrEventCreate(&smoothEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrAddDependence(smoothEVT, mgEDT, SLOT(mg,privateBlock), DB_MODE_RW);

//make smooth wait for both Pack and Unpack
        PRM(smooth,level) = level;
        PRM(smooth,vectorIndex) = Z;
        PRM(smooth,rhsIndex) = R;
        PRM(smooth,returnEVT) = smoothEVT;
        ocrEdtCreate(&smoothEDT, pbPTR->smoothTML, EDT_PARAM_DEF, (u64 *) smoothPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
        ocrEventCreate(&unpackEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrAddDependence(unpackEVT, smoothEDT, SLOT(smooth,privateBlock), DB_MODE_RW);
        ocrEventCreate(&packEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrAddDependence(packEVT, smoothEDT, SLOT(smooth,packEVT), DB_MODE_RW);

//launch halo
        PRM(haloExchange,level) = level;
        PRM(haloExchange,vectorIndex) = Z;
        PRM(haloExchange,unpackEVT) = unpackEVT;
        PRM(haloExchange,packEVT) = packEVT;

        ocrEdtCreate(&haloExchangeEDT, pbPTR->haloExchangeTML, EDT_PARAM_DEF, (u64 *) haloExchangePRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
#ifdef TIMER
        pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
        ocrDbRelease(pbDBK);
        ocrAddDependence(pbDBK, haloExchangeEDT, SLOT(haloExchange,privateBlock), DB_MODE_RW);

        if(debug > 0) PRINTF("MG%d S%d P%d L%d finish\n", &pbPTR->myrank, mgStep, phase, level);

#ifdef USE_PROFILER
        RETURN_PROFILE(NULL_GUID);
#endif
        return NULL_GUID;

        case 1:  //return from smooth, start SPMV

//return if at the bottom
        if(paramv[0] == 3) {
#ifdef TIMER
            pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
            ocrDbRelease(pbDBK);
            ocrEventSatisfy(returnEVT, pbDBK);
#ifdef USE_PROFILER
            RETURN_PROFILE(NULL_GUID);
#endif
            return NULL_GUID;
        }

//create clone and have it depend on spmv
        pbPTR->mgPhase[level] = 2;
        ocrEdtCreate(&mgEDT, pbPTR->mgTML, EDT_PARAM_DEF, (u64 *) mgPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
        ocrEventCreate(&spmvEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrAddDependence(spmvEVT, mgEDT, SLOT(mg,privateBlock), DB_MODE_RW);

//make spmv depend on pack and unpack
        PRM(spmv,level) = level;
        PRM(spmv,sourceIndex) = Z;
        PRM(spmv,resultIndex) = AP;
        PRM(spmv,returnEVT) = spmvEVT;
        ocrEdtCreate(&spmvEDT, pbPTR->spmvTML, EDT_PARAM_DEF, (u64 *) spmvPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
        ocrEventCreate(&unpackEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrEventCreate(&packEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrAddDependence(unpackEVT, spmvEDT, SLOT(spmv,privateBlock), DB_MODE_RW);
        ocrAddDependence(packEVT, spmvEDT, SLOT(spmv,packEVT), DB_MODE_RW);

        PRM(haloExchange,level) = level;
        PRM(haloExchange,vectorIndex) = Z;
        PRM(haloExchange,packEVT) = packEVT;
        PRM(haloExchange,unpackEVT) = unpackEVT;
        ocrEdtCreate(&haloExchangeEDT, pbPTR->haloExchangeTML, EDT_PARAM_DEF, (u64 *) haloExchangePRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);

        if(debug > 0) PRINTF("MG%d S%d P%d L%d finish\n", &pbPTR->myrank, mgStep, phase, level);

        //launch halo
#ifdef TIMER
        pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
        ocrDbRelease(pbDBK);
        ocrAddDependence(pbDBK, haloExchangeEDT, SLOT(haloExchange,privateBlock), DB_MODE_RW);

#ifdef USE_PROFILER
        RETURN_PROFILE(NULL_GUID);
#endif
        return NULL_GUID;


        case 2:  //return from SPMV, launch recursive call

//create clone

        pbPTR->mgPhase[level] = 3;
        PRM(mg,mgStep) = 6-mgStep;
        PRM(mg,returnEVT) = returnEVT;
//needed because the template is used after the release of the shared block
        ocrGuid_t sbMgTML = pbPTR->mgTML;
        ocrEdtCreate(&mgEDT, sbMgTML, EDT_PARAM_DEF, (u64 *) mgPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
        ocrEventCreate(&returnEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrAddDependence(returnEVT, mgEDT, SLOT(mg,privateBlock), DB_MODE_RW);

#ifdef COMPUTE
        //create restriction
        ap  = (double *) (((u64) ptr) + pbPTR->vector_offset[level][AP]);
        rold  = (double *) (((u64) ptr) + pbPTR->vector_offset[level][R]);
        rnew  = (double *) (((u64) ptr) + pbPTR->vector_offset[level+1][R]);

        size = pbPTR->m[level+1];
        u64 oldindex;
        for(i=0;i<pbPTR->mt[level+1];i++) {
            oldindex = f2c(i,size);
            rnew[i] = rold[oldindex] - ap[oldindex];
        //PRINTF("rest %d oldindex %d size %d res %f fine %f A %f \n", i, oldindex, size, rnew[i], rold[oldindex], ap[oldindex]);
        }
#endif

//create recursive call
        mgStep++;

        PRM(mg,mgStep) = mgStep;
        PRM(mg,returnEVT) = returnEVT;
        level = 3 - abs(mgStep-3);
        pbPTR->mgPhase[level] = 0;
        ocrEdtCreate(&mgEDT, sbMgTML, EDT_PARAM_DEF, (u64 *) mgPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
        if(debug > 0) PRINTF("MG%d S%d P%d L%d finishing\n", &pbPTR->myrank, mgStep, phase, level);

#ifdef TIMER
        pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
        ocrDbRelease(pbDBK);
        ocrAddDependence(pbDBK, mgEDT, SLOT(mg,privateBlock), DB_MODE_RW);
#ifdef USE_PROFILER
        RETURN_PROFILE(NULL_GUID);
#endif
        return NULL_GUID;

        case 3:  //return from recursive call

//do prolongation (add returned half Z to full Z)


#ifdef COMPUTE

           z  = (double *) (((u64) ptr) + pbPTR->vector_offset[level][Z]);
           znew  = (double *) (((u64) ptr) + pbPTR->vector_offset[level+1][Z]);
           u64 size = pbPTR->m[level+1];

           for(i=0;i<pbPTR->mt[level+1];i++) {
                if(debug > 1) PRINTF("MG%d S%d P%d L%d i %d source %f update %f result %f \n", pbPTR->myrank, mgStep, phase, level, i, z[f2c(i,size)], znew[i], z[f2c(i,size)] + znew[i]);
                //PRINTF("rank%d pro S%d P%d L%d i %d f2c %d source %f update %f result %f \n", pbPTR->myrank, mgStep, phase, level, i, f2c(i,size), z[f2c(i,size)], znew[i], z[f2c(i,size)] + znew[i]);
                //PRINTF("MG%d S%d P%d L%d i %d f2c %d source %f update %f result %f \n", pbPTR->myrank, mgStep, phase, level, i, f2c(i,size), z[f2c(i,size)], znew[i], z[f2c(i,size)] + znew[i]);
                    z[f2c(i,size)] += znew[i];
           }
#endif

        //create clone, smooth, and halo

        if(debug > 0) PRINTF("MG%d S%d P%d  launch halo\n", &pbPTR->myrank, mgStep, phase, level);

        pbPTR->mgPhase[level] = 4;
        ocrEdtCreate(&mgEDT, pbPTR->mgTML, EDT_PARAM_DEF, (u64 *) mgPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);

        ocrEventCreate(&smoothEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        PRM(smooth,level) = level;
        PRM(smooth,vectorIndex) = Z;
        PRM(smooth,rhsIndex) = R;
        PRM(smooth,returnEVT) = smoothEVT;
        ocrEdtCreate(&smoothEDT, pbPTR->smoothTML, EDT_PARAM_DEF, (u64 *) smoothPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);

        ocrEventCreate(&unpackEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrEventCreate(&packEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);


        PRM(haloExchange,level) = level;
        PRM(haloExchange,vectorIndex) = Z;
        PRM(haloExchange,unpackEVT) = unpackEVT;
        PRM(haloExchange,packEVT) = packEVT;
        ocrEdtCreate(&haloExchangeEDT, pbPTR->haloExchangeTML, EDT_PARAM_DEF, (u64 *) haloExchangePRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);


        ocrAddDependence(smoothEVT, mgEDT, SLOT(mg,privateBlock), DB_MODE_RW);

        ocrAddDependence(unpackEVT, smoothEDT, SLOT(smooth,privateBlock), DB_MODE_RW);
        ocrAddDependence(packEVT, smoothEDT, SLOT(smooth,packEVT), DB_MODE_RW);

#ifdef TIMER
    pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
        ocrDbRelease(pbDBK);
        ocrAddDependence(pbDBK, haloExchangeEDT, SLOT(haloExchange,privateBlock), DB_MODE_RW);
#ifdef USE_PROFILER
        RETURN_PROFILE(NULL_GUID);
#endif
        return NULL_GUID;

        case 4:  //return from final smooth

#ifdef TIMER
    pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
        ocrDbRelease(pbDBK);
        ocrEventSatisfy(returnEVT, pbDBK);
        if(debug > 0) PRINTF("MG%d S%d P%d L%d finish\n", &pbPTR->myrank, mgStep, phase, level);
#ifdef USE_PROFILER
        RETURN_PROFILE(NULL_GUID);
#endif
        return NULL_GUID;
    }
#ifdef USE_PROFILER
    RETURN_PROFILE(NULL_GUID);
#endif
    return NULL_GUID;
}


typedef struct{
    ocrEdtDep_t privateBlock;
    ocrEdtDep_t reductionPrivateBlock;
    ocrEdtDep_t myDataBlock;
    ocrEdtDep_t returnBlock;
    } hpcgDEPV_t;


ocrGuid_t hpcgEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){

/*
depv
0: privateBlock
1: reduction private block
2: my datablock (for reduction)
3: return block from reduction (or NULL_GUID)

hpcg driver.
only work done is local linear algebra
*/
#ifdef USE_PROFILER
    START_PROFILE(app_hpcg);
#endif
    DEPVDEF(hpcg);
    ocrGuid_t pbDBK = DEPV(hpcg,privateBlock,guid);
    privateBlock_t * pbPTR = (privateBlock_t *) DEPV(hpcg,privateBlock,ptr);
    void * ptr = pbPTR;
    ocrGuid_t rpDBK = DEPV(hpcg,reductionPrivateBlock,guid);
    reductionPrivate_t * rpPTR = (reductionPrivate_t *) DEPV(hpcg,reductionPrivateBlock,ptr);
    ocrGuid_t myDataDBK = DEPV(hpcg,myDataBlock,guid);
    double * myDataPTR = (double *) DEPV(hpcg,myDataBlock,ptr);
    ocrGuid_t returnDBK = DEPV(hpcg,returnBlock,guid);
    double * returnPTR = DEPV(hpcg,returnBlock,ptr);

    u32 myrank = pbPTR->myrank;
    u32 timestep = pbPTR->timestep;
    u32 phase = pbPTR->hpcgPhase;

#ifdef TIMER
    if(phase==0) pbPTR->timePhase = 0;
    u32 timePhase = pbPTR->timePhase;
#endif

#ifdef TIMER
    pbPTR->start[pbPTR->timestep][pbPTR->timePhase++] = getTime();
#endif
//if(pbPTR->myrank==0) PRINTF("start hpcg phase %d timephase %d\n", phase, pbPTR->timePhase);


if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d begin private "GUIDF" reduceprivate "GUIDF" mydata "GUIDF" \n", myrank, timestep, phase, GUIDA(pbDBK), GUIDA(rpDBK), GUIDA(myDataDBK));


    ocrGuid_t onceEVT;
    double *a, *p, *x, *ap, *r, *z, *b, sum, pap, rtz, alpha, beta;
    u32 i, j, ind, errno;
    ocrGuid_t mgEDT, haloExchangeEDT, spmvEDT, unpackEVT, packEVT, spmvEVT, hpcgEDT, hpcgEVT;
    u64 time;

    haloExchangePRM_t haloExchangeParamv;
    haloExchangePRM_t * haloExchangePRM = &haloExchangeParamv;
    spmvPRM_t spmvParamv;
    spmvPRM_t * spmvPRM = &spmvParamv;
    mgPRM_t mgParamv;
    mgPRM_t * mgPRM = &mgParamv;

    switch (phase) {
        case 0: //Initial call only


        x = (double *) (((u64) ptr) + pbPTR->vector_offset[0][X]);

        for(i=0;i<pbPTR->mt[0];i++) *x++ = 0.0;

        a = (double *) (((u64) ptr) + pbPTR->matrix_offset[0]);;
        r = (double *) (((u64) ptr) + pbPTR->vector_offset[0][R]);;
        b = (double *) (((u64) ptr) + pbPTR->vector_offset[0][B]);;
        for(i=0;i<pbPTR->mt[0];i++){
            r[i] = 0.0;
            for(j=0;j<27;j++) r[i] += *(a++);
            b[i] = r[i];
        }

//local sum
        sum = 0;
#ifdef COMPUTE
        double * r = (double *) (((u64) ptr) + pbPTR->vector_offset[0][R]);
        for(i=0;i<pbPTR->mt[0];i++) sum += r[i]*r[i];
#endif
        *myDataPTR = sum;  //local sum

if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d rtr %e\n", myrank, timestep, phase, *myDataPTR);

//create clone

#ifdef AFFINITY
        ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
#else
        ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif



if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d finishing \n", myrank, timestep, phase);

        pbPTR->hpcgPhase = 1;
#ifdef TIMER
    pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
        ocrDbRelease(pbDBK);
        ocrAddDependence(pbDBK, hpcgEDT, SLOT(hpcg,privateBlock), DB_MODE_RW);
        ocrAddDependence(rpDBK, hpcgEDT, SLOT(hpcg,reductionPrivateBlock), DB_MODE_RW);

//get return event from reduction
//PRINTF("CG%d T%d P%d local "GUIDF" \n", myrank, timestep, phase, GUIDA(rpPTR->returnEVT));
        ocrAddDependence(rpPTR->returnEVT, hpcgEDT, SLOT(hpcg,returnBlock), DB_MODE_RO);

//launch reduction

        reductionLaunch(rpPTR, rpDBK, myDataPTR);

        ocrAddDependence(myDataDBK, hpcgEDT, SLOT(hpcg,myDataBlock), DB_MODE_RW);
#ifdef USE_PROFILER
        RETURN_PROFILE(NULL_GUID);
#endif
        return NULL_GUID;


    case 1:

//consume rtr   CONVERGENCE test
if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d global rtr %f \n", myrank, timestep, phase, *returnPTR);
        if(myrank==0) PRINTF("time %d rtr %f \n", timestep, *returnPTR);
        if(timestep==0) pbPTR->rtr0 = *returnPTR;
           else if(*returnPTR/pbPTR->rtr0 < 1e-13 || timestep == pbPTR->maxIter) {
if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d finishing \n", myrank, timestep, phase);

for(i=0;i<pbPTR->maxIter;i++)
    for(j=0;j<PHASES;j++)
        //PRINTF("%d %d %d %ld %ld\n", myrank, i, j, pbPTR->start[i][j], pbPTR->end[i][j]);

            x = (double *) (((u64) ptr) + pbPTR->vector_offset[0][X]);
            sum = 0;
            for(i=0;i<pbPTR->mt[0];i++) sum += (1-x[i])*(1-x[i]);
            *myDataPTR = sum;
            rpPTR->type = REDUCE;
        rpPTR->returnEVT = pbPTR->finalOnceEVT;
            reductionLaunch(rpPTR, rpDBK, myDataPTR);
            ocrDbDestroy(returnDBK);
#ifdef USE_PROFILER
            RETURN_PROFILE(NULL_GUID);
#endif
            return NULL_GUID;
        }

#ifdef PRECONDITIONER

//preconditioning...launch mg

//create clone
        pbPTR->hpcgPhase = 2;
        pbPTR->mgPhase[0] = 0;

#ifdef AFFINITY
        ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
#else
        ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif

        ocrEventCreate(&hpcgEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        PRM(mg,mgStep) = 0;
        PRM(mg,returnEVT) = hpcgEVT;


        ocrEdtCreate(&mgEDT, pbPTR->mgTML, EDT_PARAM_DEF, (u64 *) mgPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);

if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d return from mg event "GUIDF" \n", myrank, timestep, phase, GUIDA(hpcgEVT));

        ocrAddDependence(hpcgEVT, hpcgEDT, SLOT(hpcg,privateBlock), DB_MODE_RW);
        ocrAddDependence(rpDBK, hpcgEDT, SLOT(hpcg,reductionPrivateBlock), DB_MODE_RW);
        ocrAddDependence(myDataDBK, hpcgEDT, SLOT(hpcg,myDataBlock), DB_MODE_RW);
        ocrAddDependence(NULL_GUID, hpcgEDT, SLOT(hpcg,returnBlock), DB_MODE_RW);

#ifdef TIMER
    pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
        ocrDbRelease(pbDBK);
        ocrAddDependence(pbDBK, mgEDT, SLOT(mg,privateBlock), DB_MODE_RW);

#ifdef USE_PROFILER
        RETURN_PROFILE(NULL_GUID);
#endif
        return NULL_GUID;

#else
//preconditioning NOT, just copy R to Z AND THEN FALL THROUGH TO PHASE 3!!
        r = (double *) (((u64) ptr) + pbPTR->vector_offset[0][R]);
        z = (double *) (((u64) ptr) + pbPTR->vector_offset[0][Z]);
        for(i=0;i<pbPTR->mt[0];i++) z[i] = r[i];
#endif
    case 2:

//consume Z
//compute local rtz
        sum = 0;
#ifdef COMPUTE
        z = (double *) (((u64) ptr) + pbPTR->vector_offset[0][Z]);
        r = (double *) (((u64) ptr) + pbPTR->vector_offset[0][R]);
if(pbPTR->debug > 1) for(i=0;i<pbPTR->mt[0];i++) PRINTF("CG%d T%d P%d i %d Z %f R %f \n", myrank, timestep, phase, i, z[i], r[i]);
        for(i=0;i<pbPTR->mt[0];i++) {
            sum += z[i]*r[i];
        }

if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d local rtz %e \n", myrank, timestep, phase, sum);
#endif
        *myDataPTR = sum;


//Create clone
        pbPTR->hpcgPhase = 3;


#ifdef AFFINITY
        ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
#else
        ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif

//PRINTF("CG%d T%d P%d local "GUIDF" \n", myrank, timestep, phase, GUIDA(rpPTR->returnEVT));
        ocrAddDependence(rpPTR->returnEVT, hpcgEDT, SLOT(hpcg,returnBlock), DB_MODE_RO);
#ifdef TIMER
    pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
        ocrDbRelease(pbDBK);
        ocrAddDependence(pbDBK, hpcgEDT, SLOT(hpcg,privateBlock), DB_MODE_RW);
        ocrAddDependence(rpDBK, hpcgEDT, SLOT(hpcg,reductionPrivateBlock), DB_MODE_RW);
        ocrAddDependence(myDataDBK, hpcgEDT, SLOT(hpcg,myDataBlock), DB_MODE_RW);

//launch reduction

        reductionLaunch(rpPTR, rpDBK, myDataPTR);
#ifdef USE_PROFILER
        RETURN_PROFILE(NULL_GUID);
#endif
        return NULL_GUID;

    case 3:
//consume rtz
        if(myrank==0) PRINTF("time %d rtz %f \n", timestep, *returnPTR);
        pbPTR->rtz = *returnPTR;
#ifdef COMPUTE
//compute beta
        if(timestep == 0) beta = 0;
               else beta = *returnPTR/pbPTR->rtzold;
        ocrDbDestroy(returnDBK);
//update p
        z = (double *) (((u64) ptr) + pbPTR->vector_offset[0][Z]);
        p = (double *) (((u64) ptr) + pbPTR->vector_offset[0][P]);
        for(i=0;i<pbPTR->mt[0];i++) p[i] = z[i] + beta*p[i];
#endif

//compute Ap

        ocrEventCreate(&packEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrEventCreate(&unpackEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrEventCreate(&spmvEVT, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);

//create clone
        pbPTR->hpcgPhase = 4;
#ifdef AFFINITY
        ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
#else
        ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif

        PRM(spmv,level) = 0;
        PRM(spmv,sourceIndex) = P;
        PRM(spmv,resultIndex) = AP;
        PRM(spmv,returnEVT) = spmvEVT;
        ocrEdtCreate(&spmvEDT, pbPTR->spmvTML, EDT_PARAM_DEF, (u64 *) spmvPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);

        PRM(haloExchange,level) = 0;
        PRM(haloExchange,vectorIndex) = P;
        PRM(haloExchange,unpackEVT) = unpackEVT;
        PRM(haloExchange,packEVT) = packEVT;
        ocrEdtCreate(&haloExchangeEDT, pbPTR->haloExchangeTML, EDT_PARAM_DEF, (u64 *) haloExchangePRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);

        ocrAddDependence(spmvEVT, hpcgEDT, SLOT(hpcg,privateBlock), DB_MODE_RW);
        ocrAddDependence(rpDBK, hpcgEDT, SLOT(hpcg,reductionPrivateBlock), DB_MODE_RW);
        ocrAddDependence(myDataDBK, hpcgEDT, SLOT(hpcg,myDataBlock), DB_MODE_RW);
        ocrAddDependence(NULL_GUID, hpcgEDT, SLOT(hpcg,returnBlock), DB_MODE_RO);


        ocrAddDependence(unpackEVT, spmvEDT, SLOT(spmv,privateBlock), DB_MODE_RW);
        ocrAddDependence(packEVT, spmvEDT, SLOT(spmv,packEVT), DB_MODE_RW);


if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d finish \n", myrank, timestep, phase);
#ifdef TIMER
    pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
        ocrDbRelease(pbDBK);
        ocrAddDependence(pbDBK, haloExchangeEDT, SLOT(haloExchange,privateBlock), DB_MODE_RW);
#ifdef USE_PROFILER
        RETURN_PROFILE(NULL_GUID);
#endif
        return NULL_GUID;



//compute pAp (consuming Ap)

    case 4:
        ap = (double *) (((u64) ptr) + pbPTR->vector_offset[0][AP]);
        p = (double *) (((u64) ptr) + pbPTR->vector_offset[0][P]);
        sum = 0;
#ifdef COMPUTE
        for(i=0;i<pbPTR->mt[0];i++) sum += p[i]*ap[i];
#endif
        *myDataPTR = sum;
if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d pap %f \n", myrank, timestep, phase, sum);
//Create clone
        pbPTR->hpcgPhase=5;
#ifdef AFFINITY
        ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
#else
        ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif

//PRINTF("CG%d T%d P%d local "GUIDF" \n", myrank, timestep, phase, GUIDA(rpPTR->returnEVT));
        ocrAddDependence(rpPTR->returnEVT, hpcgEDT, SLOT(hpcg,returnBlock), DB_MODE_RO);
if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d finish \n", myrank, timestep, phase);
#ifdef TIMER
    pbPTR->end[pbPTR->timestep][timePhase] = getTime();
        pbPTR->timePhase=0;
#endif
        ocrDbRelease(pbDBK);
        ocrAddDependence(pbDBK, hpcgEDT, SLOT(hpcg,privateBlock), DB_MODE_RW);
        ocrAddDependence(rpDBK, hpcgEDT, SLOT(hpcg,reductionPrivateBlock), DB_MODE_RW);
        ocrAddDependence(myDataDBK, hpcgEDT, SLOT(hpcg,myDataBlock), DB_MODE_RW);

//launch reduction

        reductionLaunch(rpPTR, rpDBK, myDataPTR);
#ifdef USE_PROFILER
        RETURN_PROFILE(NULL_GUID);
#endif
        return NULL_GUID;

    case 5:
//consume pAp
        pap = *returnPTR;
#ifdef COMPUTE
        alpha = pbPTR->rtz/pap;
if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d pap %f rtz %f\n", myrank, timestep, phase, pap, *returnPTR);
        pbPTR->rtzold = pbPTR->rtz;  //safe time to move it
        ocrDbDestroy(returnDBK);
//update x and r
        ap = (double *) (((u64) ptr) + pbPTR->vector_offset[0][AP]);
        p = (double *) (((u64) ptr) + pbPTR->vector_offset[0][P]);
        r = (double *) (((u64) ptr) + pbPTR->vector_offset[0][R]);
        x = (double *) (((u64) ptr) + pbPTR->vector_offset[0][X]);
        for(i=0;i<pbPTR->mt[0];i++) {
            x[i] += alpha*p[i];
            r[i] -= alpha*ap[i];
if(pbPTR->debug > 1) PRINTF("CG%d T%d P%d i %d x %f r %f p %f ap %f \n ", myrank, timestep, phase, i, x[i], r[i], p[i], ap[i]);

        }
//Prepare for the summation of rtr
        r = (double *) (((u64) ptr) + pbPTR->vector_offset[0][R]);
        sum = 0;
        for(i=0;i<pbPTR->mt[0];i++) sum += r[i]*r[i];
#endif
        *myDataPTR = sum;  //local sum
if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d rtr %f \n", myrank, timestep, phase, sum);
//create clone
        pbPTR->hpcgPhase=1;
        pbPTR->timestep++;

#ifdef AFFINITY
        ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
#else
        ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif

//PRINTF("CG%d T%d P%d local "GUIDF" \n", myrank, timestep, phase, GUIDA(rpPTR->returnEVT));
        ocrAddDependence(rpPTR->returnEVT, hpcgEDT, SLOT(hpcg,returnBlock), DB_MODE_RO);

if(pbPTR->debug > 0) PRINTF("CG%d T%d P%d finish \n", myrank, timestep, phase);
#ifdef TIMER
    pbPTR->end[pbPTR->timestep][timePhase] = getTime();
#endif
        ocrDbRelease(pbDBK);
        ocrAddDependence(pbDBK, hpcgEDT, SLOT(hpcg,privateBlock), DB_MODE_RW);
        ocrAddDependence(rpDBK, hpcgEDT, SLOT(hpcg,reductionPrivateBlock), DB_MODE_RW);
        ocrAddDependence(myDataDBK, hpcgEDT, SLOT(hpcg,myDataBlock), DB_MODE_RW);

//launch reduction

        reductionLaunch(rpPTR, rpDBK, myDataPTR);

#ifdef USE_PROFILER
        RETURN_PROFILE(NULL_GUID);
#endif
        return NULL_GUID;
    }
#ifdef USE_PROFILER
        RETURN_PROFILE(NULL_GUID);
#endif
    return NULL_GUID;
}


typedef struct{
    ocrEdtDep_t privateBlock;
    ocrEdtDep_t reductionPrivateBlock;
    ocrEdtDep_t channel[26];
    } channelInitDEPV_t;

ocrGuid_t channelInitEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){

    DEPVDEF(channelInit);
    ocrGuid_t pbDBK = DEPV(channelInit,privateBlock,guid);
    ocrGuid_t rpDBK = DEPV(channelInit,reductionPrivateBlock,guid);

    privateBlock_t * pbPTR = (privateBlock_t *) DEPV(channelInit,privateBlock,ptr);

if(pbPTR->debug > 0) PRINTF("CI%d start \n", &pbPTR->myrank);

    u64 i, dummy;

#ifdef RECEIVER_OWNS_CHANNEL_EVENT
    ocrGuid_t temp;
#endif

//record received channel events

    for(i=0;i<26;i++) {
        if(!ocrGuidIsNull(DEPVARRAY(channelInit,channel,i,guid))) {
            pbPTR->haloRecvEVT[i] = (ocrGuid_t) * (ocrGuid_t *)DEPVARRAY(channelInit,channel,i,ptr);
            ocrDbDestroy(DEPVARRAY(channelInit,channel,i,guid));
          } else
            pbPTR->haloRecvEVT[i] = NULL_GUID;
#ifdef RECEIVER_OWNS_CHANNEL_EVENT
        temp = pbPTR->haloRecvEVT[i];
        pbPTR->haloRecvEVT[i] = pbPTR->haloSendEVT[i];
        pbPTR->haloSendEVT[i] = temp;

#endif
    }

//launch hpcg

    ocrGuid_t hpcgEDT;
    ocrEdtCreate(&hpcgEDT, pbPTR->hpcgTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
      EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);

    ocrGuid_t dataBlock;
    ocrDbCreate(&dataBlock, (void**) &dummy, sizeof(double), 0, NULL_HINT, NO_ALLOC);

if(pbPTR->debug > 0) PRINTF("CI%d end \n", &pbPTR->myrank);

    ocrDbRelease(pbDBK);
    ocrAddDependence(pbDBK, hpcgEDT, SLOT(hpcg,privateBlock), DB_MODE_RW);
    ocrDbRelease(rpDBK);  //not needed?
    ocrAddDependence(rpDBK, hpcgEDT, SLOT(hpcg,reductionPrivateBlock), DB_MODE_RW);
    ocrAddDependence(dataBlock, hpcgEDT, SLOT(hpcg,myDataBlock), DB_MODE_RW);
    ocrAddDependence(NULL_GUID, hpcgEDT, SLOT(hpcg,returnBlock), DB_MODE_RO);



return NULL_GUID;
}

typedef struct{
    u64 myrank;
    } initPRM_t;

typedef struct{
    ocrEdtDep_t sharedBlock;
    } initDEPV_t;

ocrGuid_t initEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
params:
0:  myrank

depv:
0: hpcg shared block

create privateblock
create reduction private block

launches hpcgInitEDT with shared blocks, private blocks, myrank as param

*/

    PRMDEF(init);
    u64 myrank = PRM(init,myrank);
    DEPVDEF(init);
    ocrGuid_t sbDBK = DEPV(init,sharedBlock,guid);
    sharedBlock_t * sbPTR = (sharedBlock_t *) DEPV(init,sharedBlock,ptr);

if(sbPTR->debug > 0) PRINTF("I%d\n", myrank);



    ocrGuid_t tempDBK;
    u64 dummy;
    u64 size;
    u64 m0 = sbPTR->m;
    u64 m1 = m0/2;
    u64 m2 = m1/2;
    u64 m3 = m2/2;

    size = sizeof(privateBlock_t)
      + 27*m0*m0*m0*sizeof(double)         //level 0 matrix
      + 27*m1*m1*m1*sizeof(double)         //level 1 matrix
      + 27*m2*m2*m2*sizeof(double)         //level 2 matrix
      + 27*m3*m3*m3*sizeof(double)         //level 3 matrix
      + 27*m0*m0*m0*sizeof(u32)            //level 0 column indices
      + 27*m1*m1*m1*sizeof(u32)            //level 1 column indices
      + 27*m2*m2*m2*sizeof(u32)            //level 2 column indices
      + 27*m3*m3*m3*sizeof(u32)            //level 3 column indices
      + m0*m0*m0*sizeof(u32)               //level 0 diag indices
      + m1*m1*m1*sizeof(u32)               //level 1 diag indices
      + m2*m2*m2*sizeof(u32)               //level 2 diag indices
      + m3*m3*m3*sizeof(u32)               //level 3 diag indices
      + (m0+2)*(m0+2)*(m0+2)*sizeof(double)//level 0 Z vector
      + (m1+2)*(m1+2)*(m1+2)*sizeof(double)//level 1 Z vector
      + (m2+2)*(m2+2)*(m2+2)*sizeof(double)//level 2 Z vector
      + (m3+2)*(m3+2)*(m3+2)*sizeof(double)//level 3 Z vector
      + m0*m0*m0*sizeof(double)            //level 0 R vector
      + m1*m1*m1*sizeof(double)            //level 1 R vector
      + m2*m2*m2*sizeof(double)            //level 2 R vector
      + m3*m3*m3*sizeof(double)            //level 3 R vector
      + m0*m0*m0*sizeof(double)            //level 0 AP vector (used at all levels
      + m0*m0*m0*sizeof(double)            //level 0 X vector
      + (m0+2)*(m0+2)*(m0+2)*sizeof(double)//level 0 P vector
      + m0*m0*m0*sizeof(double);           //level 0 B vector

    ocrGuid_t pbDBK, rpDBK;
    privateBlock_t * pbPTR;
    reductionPrivate_t * rpPTR;


    ocrDbCreate(&pbDBK, (void**) &pbPTR, size, DB_PROP_NONE, NULL_HINT, NO_ALLOC);

//reduction block
    ocrDbCreate(&rpDBK, (void**) &rpPTR, sizeof(reductionPrivate_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC);

    s64 i;





    rpPTR->nrank = sbPTR->npx * sbPTR->npy * sbPTR->npz;
    rpPTR->myrank = myrank;
    rpPTR->ndata = 1;
    rpPTR->reductionOperator = REDUCTION_F8_ADD;
    rpPTR->rangeGUID = sbPTR->reductionRangeGUID;
    rpPTR->reductionTML = NULL_GUID;
    rpPTR->new = 1;  //first time
    rpPTR->type = ALLREDUCE;  //go up and down (ALL_REDUCE)

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 2;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;

    ocrEventCreateParams(&(rpPTR->returnEVT), OCR_EVENT_CHANNEL_T, false, &params);
//PRINTF("HI%d returnEVT "GUIDF" \n", paramv[0], GUIDA(rpPTR->returnEVT));

    pbPTR->nrank = sbPTR->npx * sbPTR->npy * sbPTR->npz;
    pbPTR->myrank = paramv[0];
    pbPTR->timestep = 0;
    pbPTR->npx = sbPTR->npx;
    pbPTR->npy = sbPTR->npy;
    pbPTR->npz = sbPTR->npz;
    pbPTR->maxIter = sbPTR->maxIter;
    pbPTR->debug = sbPTR->debug;



    pbPTR->finalOnceEVT = sbPTR->finalOnceEVT;
    pbPTR->m[0] = m0;
    pbPTR->m[1] = m1;
    pbPTR->m[2] = m2;
    pbPTR->m[3] = m3;

    pbPTR->mt[0] = m0*m0*m0;
    pbPTR->mt[1] = m1*m1*m1;
    pbPTR->mt[2] = m2*m2*m2;
    pbPTR->mt[3] = m3*m3*m3;
    pbPTR->ht[0] = (m0+2)*(m0+2)*(m0+2);
    pbPTR->ht[1] = (m1+2)*(m1+2)*(m1+2);
    pbPTR->ht[2] = (m2+2)*(m2+2)*(m2+2);
    pbPTR->ht[3] = (m3+2)*(m3+2)*(m3+2);
    pbPTR->hpcgPhase = 0;


    pbPTR->matrix_offset[0] = sizeof(privateBlock_t);
    pbPTR->matrix_offset[1] = pbPTR->matrix_offset[0] + 27*m0*m0*m0*sizeof(double);
    pbPTR->matrix_offset[2] = pbPTR->matrix_offset[1] + 27*m1*m1*m1*sizeof(double);
    pbPTR->matrix_offset[3] = pbPTR->matrix_offset[2] + 27*m2*m2*m2*sizeof(double);

    pbPTR->column_index_offset[0] = pbPTR->matrix_offset[3] + 27*m3*m3*m3*sizeof(double);
    pbPTR->column_index_offset[1] = pbPTR->column_index_offset[0] + 27*m0*m0*m0*sizeof(u32);
    pbPTR->column_index_offset[2] = pbPTR->column_index_offset[1] + 27*m1*m1*m1*sizeof(u32);
    pbPTR->column_index_offset[3] = pbPTR->column_index_offset[2] + 27*m2*m2*m2*sizeof(u32);

    pbPTR->diagonal_index_offset[0] = pbPTR->column_index_offset[3] + 27*m3*m3*m3*sizeof(u32);

    pbPTR->diagonal_index_offset[1] = pbPTR->diagonal_index_offset[0] + m0*m0*m0*sizeof(u32);
    pbPTR->diagonal_index_offset[2] = pbPTR->diagonal_index_offset[1] + m1*m1*m1*sizeof(u32);
    pbPTR->diagonal_index_offset[3] = pbPTR->diagonal_index_offset[2] + m2*m2*m2*sizeof(u32);

    pbPTR->vector_offset[0][Z] = pbPTR->diagonal_index_offset[3] + m3*m3*m3*sizeof(u32);
    pbPTR->vector_offset[1][Z] = pbPTR->vector_offset[0][Z] + (m0+2)*(m0+2)*(m0+2)*sizeof(double);
    pbPTR->vector_offset[2][Z] = pbPTR->vector_offset[1][Z] + (m1+2)*(m1+2)*(m1+2)*sizeof(double);
    pbPTR->vector_offset[3][Z] = pbPTR->vector_offset[2][Z] + (m2+2)*(m2+2)*(m2+2)*sizeof(double);
    pbPTR->vector_offset[0][R] = pbPTR->vector_offset[3][Z] + (m3+2)*(m3+2)*(m3+2)*sizeof(double);
    pbPTR->vector_offset[1][R] = pbPTR->vector_offset[0][R] + m0*m0*m0*sizeof(double);
    pbPTR->vector_offset[2][R] = pbPTR->vector_offset[1][R] + m1*m1*m1*sizeof(double);
    pbPTR->vector_offset[3][R] = pbPTR->vector_offset[2][R] + m2*m2*m2*sizeof(double);
    pbPTR->vector_offset[0][AP] = pbPTR->vector_offset[3][R] + m3*m3*m3*sizeof(double);
    pbPTR->vector_offset[1][AP] = pbPTR->vector_offset[0][AP];
    pbPTR->vector_offset[2][AP] = pbPTR->vector_offset[0][AP];
    pbPTR->vector_offset[3][AP] = pbPTR->vector_offset[0][AP];
    pbPTR->vector_offset[0][X] = pbPTR->vector_offset[0][AP] + m0*m0*m0*sizeof(double);
    pbPTR->vector_offset[1][X] = 1000000000;
    pbPTR->vector_offset[2][X] = 1000000000;
    pbPTR->vector_offset[3][X] = 1000000000;

    pbPTR->vector_offset[0][P] = pbPTR->vector_offset[0][X] + m0*m0*m0*sizeof(double);
    pbPTR->vector_offset[1][P] = 1000000000;
    pbPTR->vector_offset[2][P] = 1000000000;
    pbPTR->vector_offset[3][P] = 1000000000;

    pbPTR->vector_offset[0][B] = pbPTR->vector_offset[0][P] + (m0+2)*(m0+2)*(m0+2)*sizeof(double);
    pbPTR->vector_offset[1][B] = 1000000000;
    pbPTR->vector_offset[2][B] = 1000000000;
    pbPTR->vector_offset[3][B] = 1000000000;

    ocrEdtTemplateCreate(&(pbPTR->hpcgTML), hpcgEdt, 0, DEPVNUM(hpcg));
    ocrEdtTemplateCreate(&(pbPTR->mgTML), mgEdt, PRMNUM(mg), DEPVNUM(mg));
    ocrEdtTemplateCreate(&(pbPTR->haloExchangeTML), haloExchangeEdt, PRMNUM(haloExchange), DEPVNUM(haloExchange));
    ocrEdtTemplateCreate(&(pbPTR->unpackTML), unpackEdt, PRMNUM(unpack), DEPVNUM(unpack));
    ocrEdtTemplateCreate(&(pbPTR->spmvTML), spmvEdt, PRMNUM(spmv), DEPVNUM(spmv));
    ocrEdtTemplateCreate(&(pbPTR->smoothTML), smoothEdt, PRMNUM(smooth), DEPVNUM(smooth));


if(pbPTR->debug > 0) PRINTF("HI%d start\n", myrank);

    u32 nrank = pbPTR->nrank;

    u32 pz = myrank/(pbPTR->npx*pbPTR->npy);
    u32 py = (myrank/pbPTR->npx)%pbPTR->npy;
    u32 px = myrank%(pbPTR->npx);
    s32 j, k;

//initialize sendBlock, sendEVT, and recvEVT


    for(i=0;i<26;i++) {
        pbPTR->haloSendEVT[i] = UNINITIALIZED_GUID;
    }


    if(pz == 0) {
        pbPTR->haloSendEVT[0] = NULL_GUID;
        pbPTR->haloSendEVT[1] = NULL_GUID;
        pbPTR->haloSendEVT[2] = NULL_GUID;
        pbPTR->haloSendEVT[3] = NULL_GUID;
        pbPTR->haloSendEVT[4] = NULL_GUID;
        pbPTR->haloSendEVT[5] = NULL_GUID;
        pbPTR->haloSendEVT[6] = NULL_GUID;
        pbPTR->haloSendEVT[7] = NULL_GUID;
        pbPTR->haloSendEVT[8] = NULL_GUID;
    }

    if(pz == pbPTR->npz-1) {
        pbPTR->haloSendEVT[17] = NULL_GUID;
        pbPTR->haloSendEVT[18] = NULL_GUID;
        pbPTR->haloSendEVT[19] = NULL_GUID;
        pbPTR->haloSendEVT[20] = NULL_GUID;
        pbPTR->haloSendEVT[21] = NULL_GUID;
        pbPTR->haloSendEVT[22] = NULL_GUID;
        pbPTR->haloSendEVT[23] = NULL_GUID;
        pbPTR->haloSendEVT[24] = NULL_GUID;
        pbPTR->haloSendEVT[25] = NULL_GUID;
    }


    if(py == 0) {
        pbPTR->haloSendEVT[0] = NULL_GUID;
        pbPTR->haloSendEVT[1] = NULL_GUID;
        pbPTR->haloSendEVT[2] = NULL_GUID;
        pbPTR->haloSendEVT[9] = NULL_GUID;
        pbPTR->haloSendEVT[10] = NULL_GUID;
        pbPTR->haloSendEVT[11] = NULL_GUID;
        pbPTR->haloSendEVT[17] = NULL_GUID;
        pbPTR->haloSendEVT[18] = NULL_GUID;
        pbPTR->haloSendEVT[19] = NULL_GUID;
    }

    if(py == pbPTR->npy-1) {
        pbPTR->haloSendEVT[6] = NULL_GUID;
        pbPTR->haloSendEVT[7] = NULL_GUID;
        pbPTR->haloSendEVT[8] = NULL_GUID;
        pbPTR->haloSendEVT[14] = NULL_GUID;
        pbPTR->haloSendEVT[15] = NULL_GUID;
        pbPTR->haloSendEVT[16] = NULL_GUID;
        pbPTR->haloSendEVT[23] = NULL_GUID;
        pbPTR->haloSendEVT[24] = NULL_GUID;
        pbPTR->haloSendEVT[25] = NULL_GUID;
    }


    if(px == 0) {
        pbPTR->haloSendEVT[0] = NULL_GUID;
        pbPTR->haloSendEVT[3] = NULL_GUID;
        pbPTR->haloSendEVT[6] = NULL_GUID;
        pbPTR->haloSendEVT[9] = NULL_GUID;
        pbPTR->haloSendEVT[12] = NULL_GUID;
        pbPTR->haloSendEVT[14] = NULL_GUID;
        pbPTR->haloSendEVT[17] = NULL_GUID;
        pbPTR->haloSendEVT[20] = NULL_GUID;
        pbPTR->haloSendEVT[23] = NULL_GUID;
    }

    if(px == pbPTR->npx-1) {
        pbPTR->haloSendEVT[2] = NULL_GUID;
        pbPTR->haloSendEVT[5] = NULL_GUID;
        pbPTR->haloSendEVT[8] = NULL_GUID;
        pbPTR->haloSendEVT[11] = NULL_GUID;
        pbPTR->haloSendEVT[13] = NULL_GUID;
        pbPTR->haloSendEVT[16] = NULL_GUID;
        pbPTR->haloSendEVT[19] = NULL_GUID;
        pbPTR->haloSendEVT[22] = NULL_GUID;
        pbPTR->haloSendEVT[25] = NULL_GUID;
    }


    ocrGuid_t hpcgEDT;
    ocrGuid_t cbDBK;
    ocrGuid_t cbPTR;
    ocrGuid_t channelEVT;
    ocrGuid_t channelInitTML;
    ocrGuid_t channelInitEDT;
    ocrGuid_t stickyEVT;

    ocrEdtTemplateCreate(&channelInitTML, channelInitEdt, 0, DEPVNUM(channelInit));

    ocrHintInit(&pbPTR->myAffinityHNT,OCR_HINT_EDT_T);

#ifdef AFFINITY
    ocrGuid_t myAffinity;
    ocrAffinityGetCurrent(&myAffinity);
    ocrSetHintValue(&pbPTR->myAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));
    ocrEdtCreate(&channelInitEDT, channelInitTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &pbPTR->myAffinityHNT, NULL);
#else
    ocrEdtCreate(&channelInitEDT, channelInitTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
#endif



    ocrGuid_t * blockPTR;

    u32 errno;
    ocrGuid_t channelDBK, * channelPTR;
    u32 ind = 0;
    u32 index = 0;
    for(k=-1;k<2;k++)
        for(j=-1;j<2;j++)
            for(i=-1;i<2;i++) {
                if(i==0 && j==0 && k==0) continue; //skip "center" of cube
if(pbPTR->debug > 1) PRINTF("ind %d \n", ind);
                if(ocrGuidIsNull(pbPTR->haloSendEVT[ind])) {
                    pbPTR->haloRecvEVT[ind] = NULL_GUID;
                    ocrAddDependence(NULL_GUID, channelInitEDT, SLOTARRAY(channelInit,channel,ind), DB_MODE_RW);
                    ind++;
                } else {

//send
                    ocrDbCreate(&channelDBK, (void**) &channelPTR, sizeof(ocrGuid_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
//PRINTF("I%d i%d j%d k%d ind%d index out %d \n", myrank, i, j, k, ind, 26*myrank+ind);
                    ocrGuidFromIndex(&(stickyEVT), sbPTR->haloRangeGUID, 26*myrank + ind);
                    ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
                    ocrEventCreateParams(&(pbPTR->haloSendEVT[ind]), OCR_EVENT_CHANNEL_T, false, &params);
                    *channelPTR = pbPTR->haloSendEVT[ind];
                    ocrDbRelease(channelDBK);
                    ocrEventSatisfy(stickyEVT, channelDBK);

//receive
                    index = 26*(myrank + k*pbPTR->npx*pbPTR->npy + j*pbPTR->npx + i) + 25-ind;
//PRINTF("I%d i%d j%d k%d ind%d index in %d \n", myrank, i, j, k, ind, index);
                    ocrGuidFromIndex(&(stickyEVT), sbPTR->haloRangeGUID, index);
                    ocrEventCreate(&stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
                    ocrAddDependence(stickyEVT, channelInitEDT, SLOTARRAY(channelInit,channel,ind), DB_MODE_RW);


                    ind++;
                }


        }
if(pbPTR->debug > 0) for(i=0;i<26;i++) PRINTF("HI%d i %d sendEVT "GUIDF" recvEVT "GUIDF" \n", myrank, i, GUIDA(pbPTR->haloSendEVT[i]), GUIDA(pbPTR->haloRecvEVT[i]));

//initialize four matrices
//P vector used as scratch space
//


   void * ptr = (void *) pbPTR;

    matrixfill(pbPTR->npx, pbPTR->npy, pbPTR->npz, pbPTR->m[0], myrank, (double *) (((u64) ptr) + (u64) pbPTR->matrix_offset[0]), (u32 *) (((u64) ptr) + (u64) pbPTR->diagonal_index_offset[0]), (u32 *)(((u64) ptr) + (u64) pbPTR->column_index_offset[0]), (s32 *) (((u64) ptr) + (u64) pbPTR->vector_offset[0][Z]));
    matrixfill(pbPTR->npx, pbPTR->npy, pbPTR->npz, pbPTR->m[1], myrank, (double *) (((u64) ptr) + pbPTR->matrix_offset[1]), (u32 *) (((u64) ptr) + pbPTR->diagonal_index_offset[1]), (u32 *)(((u64) ptr) + pbPTR->column_index_offset[1]), (s32 *) (((u64) ptr) + pbPTR->vector_offset[0][Z]));
    matrixfill(pbPTR->npx, pbPTR->npy, pbPTR->npz, pbPTR->m[2], myrank, (double *) (((u64) ptr) + pbPTR->matrix_offset[2]), (u32 *) (((u64) ptr) + pbPTR->diagonal_index_offset[2]), (u32 *)(((u64) ptr) + pbPTR->column_index_offset[2]), (s32 *) (((u64) ptr) + pbPTR->vector_offset[0][Z]));
    matrixfill(pbPTR->npx, pbPTR->npy, pbPTR->npz, pbPTR->m[3], myrank, (double *) (((u64) ptr) + pbPTR->matrix_offset[3]), (u32 *) (((u64) ptr) + pbPTR->diagonal_index_offset[3]), (u32 *)(((u64) ptr) + pbPTR->column_index_offset[3]), (s32 *) (((u64) ptr) + pbPTR->vector_offset[0][Z]));

//initialize b vector

    vectorfill(pbPTR->mt[0], (double *) (((u64) ptr) + pbPTR->vector_offset[0][B]), (double *) (((u64) ptr) + pbPTR->matrix_offset[0]));

    gatherfill(&pbPTR->gather[0], pbPTR->m[0]);
    gatherfill(&pbPTR->gather[1], pbPTR->m[1]);
    gatherfill(&pbPTR->gather[2], pbPTR->m[2]);
    gatherfill(&pbPTR->gather[3], pbPTR->m[3]);



//launch


    ocrDbRelease(pbDBK);
    ocrAddDependence(pbDBK, channelInitEDT, SLOT(hpcg,privateBlock), DB_MODE_RW);
    ocrDbRelease(rpDBK);
    ocrAddDependence(rpDBK, channelInitEDT, SLOT(hpcg,reductionPrivateBlock), DB_MODE_RW);





    return NULL_GUID;
}

typedef struct {
    u64 startTime;
} wrapUpPRM_t;

typedef struct {
    ocrEdtDep_t returnBlock;
} wrapUpDEPV_t;

ocrGuid_t wrapUpEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
//prints out the sum of deviations from all 1s

    DEPVDEF(wrapUp);
    PRMDEF(wrapUp);
    double * returnPTR = (double *) DEPV(wrapUp,returnBlock,ptr);
    PRINTF("final deviation: %f \n", returnPTR[0]);
    u64 stop = getTime();
    double elapsed = TICK*(stop - PRM(wrapUp,startTime));
    PRINTF("elapsed time: %f \n", elapsed);
    ocrShutdown();
    return NULL_GUID;
}





ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {


/*
mainEDT is executed first
Creates the shared datablock
Creates the reduction shared datablock
Passes the shared blocks to realMain
*/

    u64 npx, npy, npz, m, maxIter, debug;

    u32 _paramc, _depc, _idep;

    void * programArgv = depv[0].ptr;
    u32 argc = getArgc(programArgv);

    u64 startTime = getTime();
    npx = NPX;
    npy = NPY;
    npz = NPZ;
    m = M;
    maxIter = T;
    debug = DEBUG;

    if(argc ==2 || argc ==3) bomb("number of run time parameters cannot be 1 or 2");

    if(argc > 3) {
        u32 k = 1;
        npx = (s64) atoi(getArgv(programArgv, k++));
        npy = (s64) atoi(getArgv(programArgv, k++));
        npz = (s64) atoi(getArgv(programArgv, k++));
        if(npx <= 0) bomb("npx must be positive");
        if(npy <= 0) bomb("npy must be positive");
        if(npz <= 0) bomb("npz must be positive");
        }

    if(argc > 4) {
        m = (u64) atoi(getArgv(programArgv, 4));
        if(m == 0) bomb("m must be positive");
        m = 16*((m-1)/16 +1);
        }

    if(argc > 5) {
        maxIter = (u64) atoi(getArgv(programArgv, 5));
        if(maxIter > HPCGMAXITER) bomb("maxIter must be <= HPCGMAXITER\n");
        }

    if(argc>6) {
        debug = (u64) atoi(getArgv(programArgv, 6));
        }


    PRINTF("NPX = %d \n", npx);
    PRINTF("NPY= %d \n", npy);
    PRINTF("NPZ= %d \n", npz);
    PRINTF("M = %d\n", m);
    PRINTF("maxIter = %d\n", maxIter);
    PRINTF("debug = %d\n", debug);
    PRINTF("startTime %u \n", startTime);


    ocrGuid_t sbDBK;
    sharedBlock_t * sbPTR;
    ocrDbCreate(&sbDBK, (void**) &sbPTR, sizeof(sharedBlock_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC);


    sbPTR->npx     = npx;
    sbPTR->npy     = npy;
    sbPTR->npz     = npz;
    sbPTR->m       = m;
    sbPTR->maxIter = maxIter;
    sbPTR->debug   = debug;
    u64 nrank = npx * npy * npz;

    ocrGuidRangeCreate(&(sbPTR->haloRangeGUID), 26*nrank, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(sbPTR->reductionRangeGUID), nrank, GUID_USER_EVENT_STICKY);


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
        myPD = getMyPD(0L, PDcount-1, myrank, npx, npy, npz);
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

if(debug != 0) PRINTF("M finish\n");




    return NULL_GUID;

}


