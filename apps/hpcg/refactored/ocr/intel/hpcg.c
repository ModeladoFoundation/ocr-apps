/*
Author David S. Scott
Copywrite Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.

*/

/*
This code implements HPCG in OCR.
It uses labelled GUIDs that are currently an extension
More information is in README
October 25 2015: added runtime arguments
October 27 2015: fixed "RO" to "RW" for the private block

*/

#define COMPUTE

#define ENABLE_EXTENSION_LABELING
#include "ocr.h"
#include "extensions/ocr-labeling.h" //currently needed for labeled guids
#include "string.h" //if memcpy is needed
#include "stdio.h"  //needed for PRINTF debugging
#include "math.h" //for integer abs

#include "reduction.h"

#define PRECONDITIONER  //undefine if you want to run without the precondition

#define NPX 3  //number of workers is NPX x NPY x NPZ
#define NPY 4
#define NPZ 5
#define M 16   // size of local block
#define T 50  //number of time steps
#define DEBUG 0 //debug print level 0: none 1: some 2: LOTS
#define Z 0 //4*ht
#define R 1 //4*mt
#define AP 2 //1*mt (reused)
#define X 3 //1*mt
#define P 4 //1*ht
#define B 5 //1*mt

typedef struct{
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
    u32 t;
    u32 debug;
    ocrGuid_t wrapup;
    ocrGuid_t haloRangeGuid;
} sharedBlock_t;


typedef struct{
    u32 numnodes;
    u32 mynode;
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
    u32 hePhase;
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
    ocrGuid_t haloRangeGuid;
    ocrGuid_t hpcgTemplate;
    ocrGuid_t spmvTemplate;
    ocrGuid_t mgTemplate;
    ocrGuid_t haloTemplate;
    ocrGuid_t smoothTemplate;
    ocrGuid_t packTemplate;
    ocrGuid_t unpackTemplate;
    ocrGuid_t wrapup;
    ocrGuid_t haloSendBlock[26];
    ocrGuid_t haloSendGuid[2][26];
    ocrGuid_t haloRecvGuid[2][26];
    } privateBlock_t;

//indices of vectors (arbitrary order)


void bomb(char * s) {
PRINTF("ERROR %s TERMINATING\n", s);
ocrShutdown();
return;
}

void gatherfill(gather_t * g, u32 m1){
u32 m2 = m1*m1;
u32 m3 = m1*m1*m1;

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

            printf("%d \n", k);
            for(j=0;j<size;j++) {
                for(i=0;i<size;i++) printf("%5d ", ind[c++]);
                printf("\n");
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

void matrixfill(u32 npx, u32 npy, u32 npz, u32 m, u32 mynode, double * a, u32 * diag, u32 * ind, s32 * column_index) {
//m is one of m[]
//a and ind  are 27*m*m*m
//diag is length m*m*m
//column index is (m+2)^3 which holds column offsets

if(DEBUG > 0) PRINTF("MF%d m %d \n", mynode, m);
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
//locate mynode
    u32 pz = mynode/(npx*npy);
    u32 py = (mynode/npx)%npy;
    u32 px = mynode%(npx);

    u32 counter = 0;
    double * asave;
    u32 * diagsave;

    asave = a;
    diagsave = diag;


if(DEBUG > 0) PRINTF("MF%d px %d py %d pz %d \n", mynode, px, py, pz);

    p = offset;

    for(k=-1;k<2;k++)
        for(j=-1;j<2;j++)
            for(i=-1;i<2;i++) {
                *(p++) = k*(m+2)*(m+2) + j*(m+2) + i;
    }
fflush(stdout);

if(DEBUG > 1) for(i=0;i<27;i++) PRINTF("MF%d offset %d value %d \n", mynode, i, offset[i]);

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
if(DEBUG > 0) PRINTF("MF%d finish %d \n", mynode, m);
fflush(stdout);
}





ocrGuid_t haloExchangeEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {
/*
paramv
0: level
1: vector index
2: return event for unpack
3: return event for pack

depv
0: private block

launches packANDsatisfy
launches unpack

*/



    u64 level = paramv[0];
    u64 vectorIndex = paramv[1];
    ocrGuid_t unpackEvent = paramv[2];
    ocrGuid_t packEvent = paramv[3];

    ocrGuid_t PBguid = depv[0].guid;
    privateBlock_t * PB = depv[0].ptr;

    PB->hePhase ^= 1;      //toggle between 0 and 1

    u32 mynode = PB->mynode;
    u32 phase = PB->hePhase;

    u32 i, errno;
    ocrGuid_t packEdt, unpackEdt;


if(DEBUG != 0) PRINTF("HE%d phase %d \n", mynode, PB->hePhase);


if(DEBUG != 0) PRINTF("HE%d start\n", mynode);
if(DEBUG != 0) PRINTF("HE%d start PBguid %lx \n", mynode, PBguid);
if(DEBUG != 0) PRINTF("HE%d start level %d \n", mynode, level);
if(DEBUG != 0) PRINTF("HE%d start vectorIndex %d \n", mynode, vectorIndex);
if(DEBUG != 0) PRINTF("HE%lx start unpackEvent %lx \n", mynode, unpackEvent);
if(DEBUG != 0) PRINTF("HE%lx start packEvent %lx \n", mynode, packEvent);

if(DEBUG > 1) for(i=0;i<26;i++) PRINTF("HE%d i %d sendevent %lx \n", mynode, i, PB->haloSendGuid[phase][i]);
if(DEBUG > 1) for(i=0;i<26;i++) PRINTF("HE%d i %d recvevent %lx \n", mynode, i, PB->haloRecvGuid[phase][i]);
if(DEBUG > 1) for(i=0;i<26;i++) PRINTF("HE%d i %d sendblock %lx \n", mynode, i, PB->haloSendBlock[i]);

fflush(stdout);

    u64 paramvPack[3] = {level, vectorIndex, packEvent};
    ocrEdtCreate(&packEdt, PB->packTemplate, EDT_PARAM_DEF, paramvPack, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    u64 paramvUnpack[3] = {level, vectorIndex, unpackEvent};
    ocrEdtCreate(&unpackEdt, PB->unpackTemplate, EDT_PARAM_DEF, paramvUnpack, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

if(DEBUG > 1) PRINTF("HE%d after create \n", mynode);

if(DEBUG > 1) PRINTF("HE%d after 0-1\n", mynode);
    for(i=0;i<26;i++) {
if(DEBUG > 1) PRINTF("HE%d attach sendblock %lx in slot(+1) %d  \n", mynode, PB->haloSendBlock[i], i );
        ocrAddDependence(PB->haloSendBlock[i], packEdt, i+1, DB_MODE_RW);
    }
if(DEBUG != 0) PRINTF("HE%d after sendblocks\n", mynode);

    for(i=0;i<26;i++) {

        if(PB->haloRecvGuid[phase][i] != NULL_GUID){
            errno = ocrEventCreate(&(PB->haloRecvGuid[phase][i]), OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);

            ocrAddDependence(PB->haloRecvGuid[phase][i], unpackEdt, i+1, DB_MODE_RW);
        } else {
            ocrAddDependence(NULL_GUID, unpackEdt, i+1, DB_MODE_RW);
        }
    }
    ocrDbRelease(PBguid);
    ocrAddDependence(PBguid, packEdt, 0, DB_MODE_RW);
    ocrAddDependence(PBguid, unpackEdt, 0, DB_MODE_RW);

if(DEBUG != 0) PRINTF("HE%d finish\n", mynode);
fflush(stdout);

    return(NULL_GUID);

}

ocrGuid_t packANDsatisfyEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
param
0: level
1: vector index
2: return event

depv
0: private block
1-26: datablocks to send (if on boundary will be NULL)

pack extracts the boundary values from the the source vector
copies them to the send buffers
send buffers are in lexicographic order

*/

    u64 level = paramv[0];
    u64 vector_index = paramv[1];
    ocrGuid_t returnEvent = paramv[2];




    ocrGuid_t PBguid = depv[0].guid;
    void * ptr = depv[0].ptr;
    privateBlock_t * PB = (privateBlock_t *) ptr;


    double * sendBlockPtr[26];
    ocrGuid_t sendBlockGuid[26];

    u32 j;

    for(j=0;j<26;j++) {
        sendBlockGuid[j] = depv[j+1].guid;
        sendBlockPtr[j] = (double *) depv[j+1].ptr;
    }

    double * vector = (double *) (((u64)ptr) + PB->vector_offset[level][vector_index]);

    u32 phase = PB->hePhase;
    u32 mynode = PB->mynode;
    u32 debug = PB->debug;
    gather_t * gather = &(PB->gather[level]);

    u32 i, i1, i2;

if(debug != 0) PRINTF("PK%d L%d start P%d vi %d PBguid %lx \n", mynode, level, phase, vector_index, PBguid);
if(debug > 1) for(i=0;i<27;i++) PRINTF("PK%d sendBlockGuid[i] %lx \n", mynode, i, sendBlockGuid[i]);

    u32 errno;

    u32 * start = gather->start;
    u32 * l1 = gather->l1;
    u32 * l2 = gather->l2;
    u32 * p1 = gather->p1;
    u32 * p2 = gather->p2;

    double *src, * s, * d;

    src = vector;


    for(i=0;i<26;i++) {
if(debug > 1) PRINTF("PK%d i %d start %d l1 %d l2 %d p1 %d p2 %d len %d block %lx\n", mynode, i, *start, *l1, *l2, *p1, *p2, *l1*(*l2), depv[i+1].guid);
        if(depv[i+1].guid != NULL_GUID) {
#ifdef COMPUTE

            d = sendBlockPtr[i];

            s = src + *start;
            for(i2=0;i2<*l2;i2++) {
                for(i1=0;i1<*l1;i1++) {
                    *(d++) = *s;
                    s += *p1;
                }
                s += *p2;
            }
if(debug > 0) PRINTF("PK%d DIR%d satisfy %lx with %lx\n", mynode, i, PB->haloSendGuid[phase][i], sendBlockGuid[i]);
fflush(stdout);
#endif
            errno = ocrEventCreate(&(PB->haloSendGuid[phase][i]), OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            ocrDbRelease(sendBlockGuid[i]);
            ocrEventSatisfy(PB->haloSendGuid[phase][i], sendBlockGuid[i]);
         }

         start++;
         l1++;
         l2++;
         p1++;
         p2++;

    }
if(debug != 0) PRINTF("PK%d finish\n", mynode);
fflush(stdout);

ocrEventSatisfy(returnEvent, NULL_GUID);

return(NULL_GUID);

}

ocrGuid_t unpackEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
param
0: level
1: vector index
2: returnEvent


depv
0: private block
1-26: datablocks(from Pack)

copies the values from the received data blocks into the destination vector
sends the private block (to either spmv or smooth)
*/
    u64 level = paramv[0];
    u64 vectorIndex = paramv[1];
    ocrGuid_t returnEvent = paramv[2];

    void * ptr = depv[0].ptr;
    ocrGuid_t PBguid = depv[0].guid;
    privateBlock_t * PB = depv[0].ptr;
    u32 mynode = PB->mynode;
    u32 phase = PB->hePhase;
    u32 debug = PB->debug;
    gather_t * gather = &(PB->gather[level]);

    ocrGuid_t recvBlockGuid[26];
    double * recvBlockPtr[26];

if(debug != 0) PRINTF("UN%d L%d start P%d PBguid %lx \n", mynode, level, phase, PBguid);

    u32 i;
    for(i=0;i<26;i++) {
        recvBlockGuid[i] = depv[i+1].guid;
        recvBlockPtr[i] = (double *) depv[i+1].ptr;
    }
    double * dest = (double *) (((u64) ptr) + PB->vector_offset[level][vectorIndex]);

if(debug > 1) for(i=0;i<27;i++) PRINTF("UN%d depv[%d] %lx \n", mynode, i, depv[i].guid);




    ocrGuid_t sticky;


    if(debug > 0) PRINTF("UN%d phase %d \n", mynode, phase);
    double * b;
if(debug >1 ) for(i=0;i<PB->mt[level];i++) PRINTF("UN%d i %d v %f \n", mynode, i, *(dest+i));

    dest += PB->mt[level];  //skip over existing vector




    for(i=0;i<26;i++) {
        if(PB->haloRecvGuid[phase][i] != NULL_GUID) {
if(debug > 0) PRINTF("UN%d L%d i %d destroy %lx \n", mynode, level,  i, PB->haloRecvGuid[phase][i]);
            ocrEventDestroy(PB->haloRecvGuid[phase][i]);
            PB->haloSendBlock[i] = recvBlockGuid[i];
        }
    }


    u32 j, len;
    double * a;

#ifdef COMPUTE
    for(i=0;i<26;i++) {
if(debug > 1) PRINTF("UN%d start i %d\n", mynode, i);
        b = recvBlockPtr[i];
        len = gather->l1[i]*gather->l2[i];
        if(b == NULL) for(j=0;j<len;j++) *dest++ = 0.0;
          else {

            for(j=0;j<len;j++) {
if(debug >1) PRINTF("UN%d i %d v %f\n", mynode, i, *b);
               *dest++ = *b++;
            }

        }
    }
#endif

    ocrDbRelease(PBguid);
    ocrEventSatisfy(returnEvent, PBguid);
    return(NULL_GUID);
}





ocrGuid_t smoothEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv
0: level
1: vector index
2: rhs index
3: return event

depv
0: private block from unpack through unpackEvent
1: packEvent (control)

smooth does a forward and backward Gauss-Seidel sweep
then sends the private block back

*/
    u64 level = paramv[0];
    u64 vector_index = paramv[1];
    u64 rhs_index = paramv[2];
    ocrGuid_t returnEvent = paramv[3];

    ocrGuid_t PBguid = depv[0].guid;
    void * ptr = depv[0].ptr;
    privateBlock_t * PB = ptr;


    double * abase = (double *) (((u64) ptr) + PB->matrix_offset[level]);
    u32 * indbase = (u32 *) (((u64) ptr) + PB->column_index_offset[level]);
    u32 * diagbase = (u32 *) (((u64) ptr) + PB->diagonal_index_offset[level]);
    double * vectorbase = (double *) (((u64) ptr) + PB->vector_offset[level][vector_index]);
    double * rhsbase = (double *) (((u64) ptr) + PB->vector_offset[level][rhs_index]);

    u32 length = PB->mt[level];
    u32 mynode = PB->mynode;
    u32 debug = PB->debug;


    u32 i, j;
    double sum;



if(debug >= 1) PRINTF("SM%d L%d V%d rhs%d start \n", mynode, level, vector_index, rhs_index);

    u32 * ind = indbase;
    u32 * diag = diagbase;
    double * a = abase;
    double * vector = vectorbase;
    double * rhs = rhsbase;

#ifdef COMPUTE
//forward sweep
    for(i=0;i<length;i++){
if(debug >= 2) PRINTF("SM%d i %d rhs %f \n", mynode, i, rhs[i]);
        sum = rhs[i];
        for(j=0;j<27;j++){
if(debug >= 2) PRINTF("SM%d i %d j %d sum %f a %f ind %d z %f \n", mynode, i, j, sum, a[j], ind[j], vectorbase[ind[j]]);
           sum -= a[j]*vectorbase[ind[j]];
        }
if(debug >= 2) PRINTF("SM%d diag index %d diag value %f\n", mynode, *diag, abase[*diag]);

        sum += abase[*diag]*vectorbase[indbase[*diag]];
        *(vector++) = sum/abase[*(diag++)];
if(debug >= 2) PRINTF("SM%d diag index %d diag value %f sum %f vector %f \n", mynode, *(diag-1), abase[*(diag-1)], sum, *(vector-1));
        a += 27;
        ind += 27;
    }
if(debug > 2){
    vector = vectorbase;
    for(i=0;i<PB->mt[level];i++)
       PRINTF("SM%d L%d i %d val %f \n", mynode, level, i, vector[i]);
}

if(debug >= 1) PRINTF("SM%d L%d finish forward\n", mynode, level);
fflush(stdout);

//backward sweep
    a = abase + 27*(length - 1);
    ind = indbase + 27*(length - 1);
    diag = diagbase + length - 1;
    vector = vectorbase + length-1;
    rhs = rhsbase+length-1;;


    for(i=length;i>0;i--){
        sum = rhsbase[i-1];
if(DEBUG > 1) PRINTF("SM%d i %d sum %f \n", mynode, i, sum);
        for(j=0;j<27;j++){
if(DEBUG > 2) PRINTF("SM%d i %d j %d sum %f a %f ind %d z %f \n", mynode, i, j, sum, a[j], ind[j], vectorbase[ind[j]]);
           sum -= a[j]*vectorbase[ind[j]];
        }
        sum += abase[*diag]*vectorbase[indbase[*diag]];
        *(vector--) = sum/abase[*(diag--)];
        a -= 27;
        ind -= 27;
    }
#endif


    ocrDbRelease(PBguid);
    ocrEventSatisfy(returnEvent, PBguid);
    return NULL_GUID;
}

ocrGuid_t spmvEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv
0: level
1: source index
2: result index
3: return event

depv
0: private block (from pack through packEvent)
1: unpackEvent (just control)


computes the local sparse matrix-vector product
sends the private block back
*/
    u64 level = paramv[0];
    u64 source_index = paramv[1];
    u64 result_index = paramv[2];
    ocrGuid_t returnEvent = paramv[3];

    ocrGuid_t PBguid = depv[0].guid;
    void * ptr = depv[0].ptr;
    privateBlock_t * PB = ptr;

#ifdef COMPUTE

    double * srcbase = (double *) ((u64) (ptr + PB->vector_offset[level][source_index]));
    double * destbase = (double *) ((u64) (ptr + PB->vector_offset[level][result_index]));
    double * abase = (double *) ((u64) (ptr + PB->matrix_offset[level]));
    u32 * indbase = (u32 *) ((u64) (ptr + PB->column_index_offset[level]));

    u32 length = PB->mt[level];

    u32 mynode = PB->mynode;
    u32 debug = PB->debug;


    u32 i, j;
    double sum;
    double * src = srcbase;
    double * dest = destbase;
    double * a = abase;
    u32 * ind = indbase;

    for(i=0;i<length;i++){
        sum = 0;
        for(j=0;j<27;j++){
if(debug >= 2) PRINTF("SPMV%d i %d j %d dest %f a %f ind %d src %f \n", mynode, i, j, sum, a[j], ind[j], src[ind[j]]);
           sum += a[j]*src[ind[j]];
        }
        *(dest++) = sum;
        a += 27;
        ind += 27;
    }

if(debug > 1) for(i=0;i<PB->mt[level];i++)
   PRINTF("SPMV%d i %d val %f \n", mynode, i, destbase[i]);

#endif

    ocrDbRelease(PBguid);
    ocrEventSatisfy(returnEvent, PBguid);
    return NULL_GUID;
}


ocrGuid_t mgEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
paramv
0: mgStep (0 to 6)
1: return event

depv
0: privateBlock  //returned by smooth or SPMV

this is the driver for the multigrid steps.

*/

    u64 mgStep = paramv[0];
    ocrGuid_t returnEvent = paramv[1];

    u32 level = 3 - abs(mgStep-3);


    ocrGuid_t PBguid = depv[0].guid;
    void * ptr = depv[0].ptr;
    privateBlock_t * PB = ptr;


    u32 m = PB->m[level];
    u32 phase = PB->mgPhase[level];
    u32 mynode = PB->mynode;
    u32 debug = PB->debug;


    u32 i;

    double * z, *ap, *zold, *znew, *r, *rold, *rnew;

    ocrGuid_t halo, packEvent, unpackEvent, smooth, smoothEvent, spmv, spmvEvent, mg, mgevent;

if(debug > 0) PRINTF("MG%d S%d P%d L%d start return event %lx\n", PB->mynode, mgStep, phase, level, returnEvent);

    switch(phase) {

        case 0: //smoothing

#ifdef COMPUTE
        z = (double *) (((u64) ptr) + PB->vector_offset[level][Z]);
        for(i=0;i<PB->mt[level];i++) z[i] = 0.0;
#endif

if(debug > 0) PRINTF("MG%d S%d P%d L%d  create clone\n", PB->mynode, mgStep, phase, level);

//create clone, smooth, and halo
//TODO phase vs step??

        PB->mgPhase[level] = 1;

        ocrEdtCreate(&mg, PB->mgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrEventCreate(&smoothEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);

        u64 paramvsmooth0[4] = {(u64) level, (u64) Z, (u64) R, (u64) smoothEvent};
        ocrEdtCreate(&smooth, PB->smoothTemplate, EDT_PARAM_DEF, paramvsmooth0, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrAddDependence(smoothEvent, mg, 0, DB_MODE_RW);

        ocrEventCreate(&unpackEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrAddDependence(unpackEvent, smooth, 0, DB_MODE_RW);

        ocrEventCreate(&packEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrAddDependence(packEvent, smooth, 1, DB_MODE_RW);

        u64 paramvhalo0[4] = {(u64) level, (u64) Z, unpackEvent, packEvent};
        ocrEdtCreate(&halo, PB->haloTemplate, EDT_PARAM_DEF, paramvhalo0, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);


        ocrDbRelease(PBguid);
        ocrAddDependence(PBguid, halo, 0, DB_MODE_RW);

if(debug > 0) PRINTF("MG%d S%d P%d L%d finish\n", PB->mynode, mgStep, phase, level);
fflush(stdout);

        return NULL_GUID;

        case 1:  //return from smooth, start SPMV

//return if at the bottom
        if(paramv[0] == 3) {
            ocrDbRelease(PBguid);
            ocrEventSatisfy(returnEvent, PBguid);
            return NULL_GUID;
        }

//create clone
        PB->mgPhase[level] = 2;
        ocrEdtCreate(&mg, PB->mgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);


        ocrEventCreate(&spmvEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        u64 paramvspmv1[4] = {(u64) level, (u64) Z, (u64) AP, spmvEvent};
        ocrEdtCreate(&spmv, PB->spmvTemplate, EDT_PARAM_DEF, paramvspmv1, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrEventCreate(&unpackEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrEventCreate(&packEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);

        u64 paramvhalo1[4] = {(u64) level, (u64) Z, unpackEvent, packEvent};
        ocrEdtCreate(&halo, PB->haloTemplate, EDT_PARAM_DEF, paramvhalo1, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

if(debug > 0) PRINTF("MG%d S%d P%d L%d finish\n", PB->mynode, mgStep, phase, level);
fflush(stdout);

        ocrAddDependence(spmvEvent, mg, 0, DB_MODE_RW);
        ocrAddDependence(unpackEvent, spmv, 0, DB_MODE_RW);
        ocrAddDependence(packEvent, spmv, 1, DB_MODE_RW);
        ocrDbRelease(PBguid);
        ocrAddDependence(PBguid, halo, 0, DB_MODE_RW);


            return NULL_GUID;
        case 2:  //return from SPMV, launch recursive call

//create clone

        PB->mgPhase[level] = 3;
        u64 paramvout[2] = {6-paramv[0], paramv[1]};
//needed because the template is used after the release of the shared block
        ocrGuid_t sbMgTemplate = PB->mgTemplate;
        ocrEdtCreate(&mg, sbMgTemplate, EDT_PARAM_DEF, paramvout, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        ocrEventCreate(&returnEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrAddDependence(returnEvent, mg, 0, DB_MODE_RW);

#ifdef COMPUTE
//create restriction
        ap  = (double *) (((u64) ptr) + PB->vector_offset[level][AP]);
        rold  = (double *) (((u64) ptr) + PB->vector_offset[level][R]);
        rnew  = (double *) (((u64) ptr) + PB->vector_offset[level+1][R]);

        for(i=0;i<PB->mt[level+1];i++) {
            *(rnew++) = *rold - *ap;
            ap += 2;
            rold += 2;
        }
#endif

//create recursive call
        mgStep++;
        paramvout[0] = mgStep;
        paramvout[1] = returnEvent;
        level = 3 - abs(mgStep-3);
        PB->mgPhase[level] = 0;
        ocrEdtCreate(&mg, sbMgTemplate, EDT_PARAM_DEF, paramvout, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
if(debug > 0) PRINTF("MG%d S%d P%d L%d finishing\n", PB->mynode, mgStep, phase, level);
fflush(stdout);
        ocrDbRelease(PBguid);
        ocrAddDependence(PBguid, mg, 0, DB_MODE_RW);

        return NULL_GUID;

        case 3:  //return from recursive call

//do prolongation (add returned half Z to full Z)


#ifdef COMPUTE

           z  = (double *) (((u64) ptr) + PB->vector_offset[level][Z]);
           znew  = (double *) (((u64) ptr) + PB->vector_offset[level+1][Z]);

           for(i=0;i<PB->mt[level+1];i++) {
if(debug > 1) PRINTF("MG%d S%d P%d L%d i %d z %f update %f\n", PB->mynode, mgStep, phase, level, i, *z, *znew);
               *z += *(znew++);
               z += 2;
           }
#endif

//create clone, smooth, and halo
//
if(debug > 0) PRINTF("MG%d S%d P%d  launch halo\n", PB->mynode, mgStep, phase, level);

        PB->mgPhase[level] = 4;
        ocrEdtCreate(&mg, PB->mgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrEventCreate(&smoothEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        u64 paramvsmooth3[4] = {(u64) level, (u64) Z, (u64) R, smoothEvent};
        ocrEdtCreate(&smooth, PB->smoothTemplate, EDT_PARAM_DEF, paramvsmooth3, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrEventCreate(&unpackEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrEventCreate(&packEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);

        u64 paramvhalo3[4] = {(u64) level, (u64) R, unpackEvent, packEvent};
        ocrEdtCreate(&halo, PB->haloTemplate, EDT_PARAM_DEF, paramvhalo3, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);


        ocrAddDependence(smoothEvent, mg, 0, DB_MODE_RW);

        ocrAddDependence(unpackEvent, smooth, 0, DB_MODE_RW);
        ocrAddDependence(packEvent, smooth, 1, DB_MODE_RW);

        ocrDbRelease(PBguid);
        ocrAddDependence(PBguid, halo, 0, DB_MODE_RW);

        return NULL_GUID;

        case 4:  //return from final smooth
        ocrDbRelease(PBguid);
        ocrEventSatisfy(returnEvent, PBguid);
if(debug > 0) PRINTF("MG%d S%d P%d L%d finish\n", PB->mynode, mgStep, phase, level);
fflush(stdout);
        return NULL_GUID;
    }

}

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

    ocrGuid_t PBguid = depv[0].guid;
    void * ptr = depv[0].ptr;
    privateBlock_t * PB = (privateBlock_t *) ptr;
    ocrGuid_t reductionPrivateGuid = depv[1].guid;
    void * reductionPrivatePtr = depv[1].ptr;
    ocrGuid_t myDataBlockGuid = depv[2].guid;
    double * myDataBlockPtr = (double *) depv[2].ptr;
    ocrGuid_t returnBlockGuid = depv[3].guid;
    double * returnBlockPtr = (double *) depv[3].ptr;

    u32 mynode = PB->mynode;
    u32 timestep = PB->timestep;
    u32 phase = PB->hpcgPhase;


if(PB->debug > 0) PRINTF("CG%d T%d P%d begin private %lx reduceprivate %lx mydata %lx return block %lx \n", mynode, timestep, phase, PBguid, reductionPrivateGuid, myDataBlockGuid, returnBlockGuid);


    ocrGuid_t onceEvent;
    double *a, *p, *x, *ap, *r, *z, *b, sum, pap, rtz, alpha, beta;
    u32 i, j, ind, errno;
    ocrGuid_t mg, halo, spmv, unpackEvent, packEvent, spmvEvent, hpcg, hpcgEvent;

    switch (phase) {
        case 0:
//Initial call only

        x = (double *) (((u64) ptr) + PB->vector_offset[0][X]);

        for(i=0;i<PB->mt[0];i++) *x++ = 0.0;

        a = (double *) (((u64) ptr) + PB->matrix_offset[0]);;
        r = (double *) (((u64) ptr) + PB->vector_offset[0][R]);;
        b = (double *) (((u64) ptr) + PB->vector_offset[0][B]);;
        for(i=0;i<PB->mt[0];i++){
            r[i] = 0.0;
            for(j=0;j<27;j++) r[i] += *(a++);
            b[i] = r[i];
        }




//local sum
        sum = 0;
#ifdef COMPUTE
        double * r = (double *) (((u64) ptr) + PB->vector_offset[0][R]);
        for(i=0;i<PB->mt[0];i++) sum += r[i]*r[i];
#endif
        *myDataBlockPtr = sum;  //local sum

if(PB->debug > 0) PRINTF("CG%d T%d P%d rtr %f\n", mynode, timestep, phase, *myDataBlockPtr);

//create clone

        ocrEdtCreate(&hpcg, PB->hpcgTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

//get return event from reduction

        onceEvent = reductionGetOnceEvent(reductionPrivatePtr, reductionPrivateGuid);
        ocrAddDependence(onceEvent, hpcg, 3, DB_MODE_RO);

if(PB->debug > 0) PRINTF("CG%d T%d P%d finishing \n", mynode, timestep, phase);
fflush(stdout);

        PB->hpcgPhase = 1;
        ocrDbRelease(PBguid);
        ocrAddDependence(PBguid, hpcg, 0, DB_MODE_RW);
        ocrAddDependence(reductionPrivateGuid, hpcg, 1, DB_MODE_RW);
        ocrAddDependence(myDataBlockGuid, hpcg, 2, DB_MODE_RW);

//launch reduction

        reductionLaunch(reductionPrivatePtr, reductionPrivateGuid, myDataBlockGuid);

        return NULL_GUID;


    case 1:

//consume rtr   CONVERGENCE test
if(PB->debug > 0) PRINTF("CG%d T%d P%d global rtr %f \n", mynode, timestep, phase, *returnBlockPtr);
        if(mynode==0) PRINTF("time %d rtr %f \n", timestep, *returnBlockPtr);
        if(timestep==0) PB->rtr0 = *returnBlockPtr;
           else if(*returnBlockPtr/PB->rtr0 < 1e-13 || timestep == PB->maxIter) {
if(PB->debug > 0) PRINTF("CG%d T%d P%d finishing \n", mynode, timestep, phase);
fflush(stdout);
ocrGuid_t wrapup=PB->wrapup;
             ocrDbRelease(PBguid);
             ocrAddDependence(PBguid, wrapup, mynode, DB_MODE_RO);
             return NULL_GUID;
        }

#ifdef PRECONDITIONER

//preconditioning...launch mg

//create clone
        PB->hpcgPhase = 2;
        PB->mgPhase[0] = 0;

        ocrEdtCreate(&hpcg, PB->hpcgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrEventCreate(&hpcgEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        u64 paramvMG[2] = {(u64) 0, hpcgEvent};
        ocrEdtCreate(&mg, PB->mgTemplate, EDT_PARAM_DEF, paramvMG, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

if(PB->debug > 0) PRINTF("CG%d T%d P%d return from mg event %lx \n", mynode, timestep, phase, hpcgEvent);

        ocrAddDependence(hpcgEvent, hpcg, 0, DB_MODE_RW);
        ocrAddDependence(reductionPrivateGuid, hpcg, 1, DB_MODE_RW);
        ocrAddDependence(myDataBlockGuid, hpcg, 2, DB_MODE_RW);
        ocrAddDependence(NULL_GUID, hpcg, 3, DB_MODE_RW);

        ocrDbRelease(PBguid);
        ocrAddDependence(PBguid, mg, 0, DB_MODE_RW);

fflush(stdout);
        return NULL_GUID;

#else
//preconditioning NOT, just copy R to Z
        r = (double *) (((u64) ptr) + PB->vector_offset[0][R]);
        z = (double *) (((u64) ptr) + PB->vector_offset[0][Z]);
        for(i=0;i<PB->mt[0];i++) z[i] = r[i];
#endif
    case 2:

//consume Z
//compute local rtz
        sum = 0;
#ifdef COMPUTE
        z = (double *) (((u64) ptr) + PB->vector_offset[0][Z]);
        r = (double *) (((u64) ptr) + PB->vector_offset[0][R]);
if(PB->debug > 1) for(i=0;i<PB->mt[0];i++) PRINTF("CG%d T%d P%d i %d Z %f R %f \n", mynode, timestep, phase, i, z[i], r[i]);
        for(i=0;i<PB->mt[0];i++) {
            sum += z[i]*r[i];
        }

if(PB->debug > 0) PRINTF("CG%d T%d P%d local rtz %f \n", mynode, timestep, phase, sum);
#endif
        *myDataBlockPtr = sum;


//Create clone
        PB->hpcgPhase = 3;

        ocrEdtCreate(&hpcg, PB->hpcgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        onceEvent = reductionGetOnceEvent(reductionPrivatePtr, reductionPrivateGuid);
        ocrAddDependence(onceEvent, hpcg, 3, DB_MODE_RO);

        ocrAddDependence(PBguid, hpcg, 0, DB_MODE_RW);
        ocrAddDependence(reductionPrivateGuid, hpcg, 1, DB_MODE_RW);
        ocrAddDependence(myDataBlockGuid, hpcg, 2, DB_MODE_RW);

//launch reduction

        reductionLaunch(reductionPrivatePtr, reductionPrivateGuid, myDataBlockGuid);

        return NULL_GUID;

    case 3:
//consume rtz
        if(mynode==0) PRINTF("time %d rtz %f \n", timestep, *returnBlockPtr);
        PB->rtz = *returnBlockPtr;
#ifdef COMPUTE
//compute beta
        if(timestep == 0) beta = 0;
               else beta = *returnBlockPtr/PB->rtzold;
//update p
        z = (double *) (((u64) ptr) + PB->vector_offset[0][Z]);
        p = (double *) (((u64) ptr) + PB->vector_offset[0][P]);
        for(i=0;i<PB->mt[0];i++) p[i] = z[i] + beta*p[i];
#endif

//compute Ap

ocrEventCreate(&packEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
ocrEventCreate(&unpackEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
ocrEventCreate(&spmvEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);

//create clone
        PB->hpcgPhase = 4;
        ocrEdtCreate(&hpcg, PB->hpcgTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        u64 paramvspmv[4] = {(u64) 0, (u64) P, (u64) AP, spmvEvent};
        ocrEdtCreate(&spmv, PB->spmvTemplate, EDT_PARAM_DEF, paramvspmv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        u64 paramvhalo[4] = {(u64) 0, (u64) P, unpackEvent, packEvent};
        ocrEdtCreate(&halo, PB->haloTemplate, EDT_PARAM_DEF, paramvhalo, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrAddDependence(spmvEvent, hpcg, 0, DB_MODE_RW);
        ocrAddDependence(reductionPrivateGuid, hpcg, 1, DB_MODE_RW);
        ocrAddDependence(myDataBlockGuid, hpcg, 2, DB_MODE_RW);
        ocrAddDependence(NULL_GUID, hpcg, 3, DB_MODE_RW);


        ocrAddDependence(unpackEvent, spmv, 0, DB_MODE_RW);
        ocrAddDependence(packEvent, spmv, 1, DB_MODE_RW);


        ocrDbRelease(PBguid);
        ocrAddDependence(PBguid, halo, 0, DB_MODE_RW);

if(PB->debug > 0) PRINTF("CG%d T%d P%d finish \n", mynode, timestep, phase);
fflush(stdout);
        return NULL_GUID;



//compute pAp (consuming Ap)

    case 4:
        ap = (double *) (((u64) ptr) + PB->vector_offset[0][AP]);
        p = (double *) (((u64) ptr) + PB->vector_offset[0][P]);
        sum = 0;
#ifdef COMPUTE
        for(i=0;i<PB->mt[0];i++) sum += p[i]*ap[i];
#endif
        *myDataBlockPtr = sum;
if(PB->debug > 0) PRINTF("CG%d T%d P%d pap %f \n", mynode, timestep, phase, sum);
//Create clone
        PB->hpcgPhase=5;
        ocrEdtCreate(&hpcg, PB->hpcgTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        onceEvent = reductionGetOnceEvent(reductionPrivatePtr, reductionPrivateGuid);
        ocrAddDependence(onceEvent, hpcg, 3, DB_MODE_RO);

        ocrAddDependence(PBguid, hpcg, 0, DB_MODE_RW);
        ocrAddDependence(reductionPrivateGuid, hpcg, 1, DB_MODE_RW);
//note that depv[2] doesn't need to be released here, just before calling reductionLaunch
        ocrAddDependence(myDataBlockGuid, hpcg, 2, DB_MODE_RW);

//launch reduction

        reductionLaunch(reductionPrivatePtr, reductionPrivateGuid, myDataBlockGuid);

if(PB->debug > 0) PRINTF("CG%d T%d P%d finish \n", mynode, timestep, phase);
fflush(stdout);
        return NULL_GUID;

    case 5:
//consume pAp
        pap = *returnBlockPtr;
#ifdef COMPUTE
        alpha = PB->rtz/pap;
if(PB->debug > 0) PRINTF("CG%d T%d P%d pap %f rtz %f\n", mynode, timestep, phase, pap, *returnBlockPtr);
        PB->rtzold = PB->rtz;  //safe time to move it
//update x and r
        ap = (double *) (((u64) ptr) + PB->vector_offset[0][AP]);
        p = (double *) (((u64) ptr) + PB->vector_offset[0][P]);
        r = (double *) (((u64) ptr) + PB->vector_offset[0][R]);
        x = (double *) (((u64) ptr) + PB->vector_offset[0][X]);
        for(i=0;i<PB->mt[0];i++) {
            x[i] += alpha*p[i];
            r[i] -= alpha*ap[i];
if(PB->debug > 1) PRINTF("CG%d T%d P%d i %d x %f r %f p %f ap %f \n ", mynode, timestep, phase, i, x[i], r[i], p[i], ap[i]);

        }
//Prepare for the summation of rtr
        r = (double *) (((u64) ptr) + PB->vector_offset[0][R]);
#endif
        sum = 0;
#ifdef COMPUTE
        for(i=0;i<PB->mt[0];i++) sum += r[i]*r[i];
#endif
        *myDataBlockPtr = sum;  //local sum
if(PB->debug > 0) PRINTF("CG%d T%d P%d rtr %f \n", mynode, timestep, phase, sum);
//create clone
        PB->hpcgPhase=1;
        PB->timestep++;

        ocrEdtCreate(&hpcg, PB->hpcgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        onceEvent = reductionGetOnceEvent(depv[1].ptr, depv[1].guid);
        ocrAddDependence(onceEvent, hpcg, 3, DB_MODE_RO);

        ocrDbRelease(PBguid);
        ocrAddDependence(PBguid, hpcg, 0, DB_MODE_RW);
        ocrAddDependence(reductionPrivateGuid, hpcg, 1, DB_MODE_RW);
//note that depv[2] doesn't need to be released here, just before calling rductionLaunch
        ocrAddDependence(myDataBlockGuid, hpcg, 2, DB_MODE_RW);

//launch reduction

        ocrDbRelease(myDataBlockGuid);
        reductionLaunch(reductionPrivatePtr, reductionPrivateGuid, myDataBlockGuid);

if(PB->debug > 0) PRINTF("CG%d T%d P%d finish \n", mynode, timestep, phase);
fflush(stdout);
        return NULL_GUID;
        }
}

ocrGuid_t hpcgInitEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
params
0:  mynode

depv
0: hpcg shared block  //not used after this EDT
1: hpcg private block
2: reduction shared block //not used after this EDT
3: reduction private block

Initialize private blocks
launch hpcgEdt
*/


    u32 mynode = paramv[0];
    sharedBlock_t * SB = depv[0].ptr;
    void * ptr = depv[1].ptr;
    ocrGuid_t PBguid = depv[1].guid;
    privateBlock_t * PB = (privateBlock_t *) depv[1].ptr;



    PB->numnodes = SB->npx * SB->npy * SB->npz;
    PB->mynode = paramv[0];
    PB->timestep = 0;
    PB->npx = SB->npx;
    PB->npy = SB->npy;
    PB->npz = SB->npz;
    PB->maxIter = SB->t;
    PB->debug = SB->debug;
    PB->wrapup = SB->wrapup;
    PB->m[0] = SB->m;
    PB->m[1] = (PB->m[0]+1)/2;
    PB->m[2] = (PB->m[1]+1)/2;
    PB->m[3] = (PB->m[2]+1)/2;
    u32 m0 = PB->m[0];
    u32 m1 = PB->m[1];
    u32 m2 = PB->m[2];
    u32 m3 = PB->m[3];
    PB->mt[0] = m0*m0*m0;
    PB->mt[1] = m1*m1*m1;
    PB->mt[2] = m2*m2*m2;
    PB->mt[3] = m3*m3*m3;
    PB->ht[0] = (m0+2)*(m0+2)*(m0+2);
    PB->ht[1] = (m1+2)*(m1+2)*(m1+2);
    PB->ht[2] = (m2+2)*(m2+2)*(m2+2);
    PB->ht[3] = (m3+2)*(m3+2)*(m3+2);
    PB->hpcgPhase = 0;
    PB->hePhase = 0;


    PB->matrix_offset[0] = sizeof(privateBlock_t);
    PB->matrix_offset[1] = PB->matrix_offset[0] + 27*m0*m0*m0*sizeof(double);
    PB->matrix_offset[2] = PB->matrix_offset[1] + 27*m1*m1*m1*sizeof(double);
    PB->matrix_offset[3] = PB->matrix_offset[2] + 27*m2*m2*m2*sizeof(double);

    PB->column_index_offset[0] = PB->matrix_offset[3] + 27*m3*m3*m3*sizeof(double);
    PB->column_index_offset[1] = PB->column_index_offset[0] + 27*m0*m0*m0*sizeof(u32);
    PB->column_index_offset[2] = PB->column_index_offset[1] + 27*m1*m1*m1*sizeof(u32);
    PB->column_index_offset[3] = PB->column_index_offset[2] + 27*m2*m2*m2*sizeof(u32);

    PB->diagonal_index_offset[0] = PB->column_index_offset[3] + 27*m3*m3*m3*sizeof(u32);

    PB->diagonal_index_offset[1] = PB->diagonal_index_offset[0] + m0*m0*m0*sizeof(u32);
    PB->diagonal_index_offset[2] = PB->diagonal_index_offset[1] + m1*m1*m1*sizeof(u32);
    PB->diagonal_index_offset[3] = PB->diagonal_index_offset[2] + m2*m2*m2*sizeof(u32);

    PB->vector_offset[0][Z] = PB->diagonal_index_offset[3] + m3*m3*m3*sizeof(u32);
    PB->vector_offset[1][Z] = PB->vector_offset[0][Z] + (m0+2)*(m0+2)*(m0+2)*sizeof(double);
    PB->vector_offset[2][Z] = PB->vector_offset[1][Z] + (m1+2)*(m1+2)*(m1+2)*sizeof(double);
    PB->vector_offset[3][Z] = PB->vector_offset[2][Z] + (m2+2)*(m2+2)*(m2+2)*sizeof(double);
    PB->vector_offset[0][R] = PB->vector_offset[3][Z] + (m3+2)*(m3+2)*(m3+2)*sizeof(double);
    PB->vector_offset[1][R] = PB->vector_offset[0][R] + m0*m0*m0*sizeof(double);
    PB->vector_offset[2][R] = PB->vector_offset[1][R] + m1*m1*m1*sizeof(double);
    PB->vector_offset[3][R] = PB->vector_offset[2][R] + m2*m2*m2*sizeof(double);
    PB->vector_offset[0][AP] = PB->vector_offset[3][R] + m3*m3*m3*sizeof(double);
    PB->vector_offset[1][AP] = PB->vector_offset[0][AP];
    PB->vector_offset[2][AP] = PB->vector_offset[0][AP];
    PB->vector_offset[3][AP] = PB->vector_offset[0][AP];
    PB->vector_offset[0][X] = PB->vector_offset[0][AP] + m0*m0*m0*sizeof(double);
    PB->vector_offset[1][X] = 1000000000;
    PB->vector_offset[2][X] = 1000000000;
    PB->vector_offset[3][X] = 1000000000;

    PB->vector_offset[0][P] = PB->vector_offset[0][X] + m0*m0*m0*sizeof(double);
    PB->vector_offset[1][P] = 1000000000;
    PB->vector_offset[2][P] = 1000000000;
    PB->vector_offset[3][P] = 1000000000;

    PB->vector_offset[0][B] = PB->vector_offset[0][P] + (m0+2)*(m0+2)*(m0+2)*sizeof(double);
    PB->vector_offset[1][B] = 1000000000;
    PB->vector_offset[2][B] = 1000000000;
    PB->vector_offset[3][B] = 1000000000;


//printf("matrixoff %lx %lx %lx %lx\n", PB->matrix_offset[0], PB->matrix_offset[1], PB->matrix_offset[2], PB->matrix_offset[3]);
//printf("column_indexoff %lx %lx %lx %lx\n", PB->column_index_offset[0], PB->column_index_offset[1], PB->column_index_offset[2], PB->column_index_offset[3]);
//printf("diagonal_indexoff %lx %lx %lx %lx\n", PB->diagonal_index_offset[0], PB->diagonal_index_offset[1], PB->diagonal_index_offset[2], PB->diagonal_index_offset[3]);
//printf("vectorZoff %lx %lx %lx %lx\n", PB->vector_offset[0][Z], PB->vector_offset[1][Z], PB->vector_offset[2][Z], PB->vector_offset[3][Z]);
//printf("vectorRoff %lx %lx %lx %lx\n", PB->vector_offset[0][R], PB->vector_offset[1][R], PB->vector_offset[2][R], PB->vector_offset[3][R]);
//printf("vectorAPoff %lx %lx %lx %lx\n", PB->vector_offset[0][AP], PB->vector_offset[1][AP], PB->vector_offset[2][AP], PB->vector_offset[3][AP]);
//printf("vectorXoff %lx %lx %lx %lx\n", PB->vector_offset[0][X], PB->vector_offset[1][X], PB->vector_offset[2][X], PB->vector_offset[3][X]);
//printf("vectorPoff %lx %lx %lx %lx\n", PB->vector_offset[0][P], PB->vector_offset[1][P], PB->vector_offset[2][P], PB->vector_offset[3][P]);
//printf("vectorBoff %lx %lx %lx %lx\n", PB->vector_offset[0][B], PB->vector_offset[1][B], PB->vector_offset[2][B], PB->vector_offset[3][B]);


    ocrEdtTemplateCreate(&(PB->hpcgTemplate), hpcgEdt, 0, 4);
    ocrEdtTemplateCreate(&(PB->mgTemplate), mgEdt, 2, 1);
    ocrEdtTemplateCreate(&(PB->haloTemplate), haloExchangeEdt, 4, 1);
    ocrEdtTemplateCreate(&(PB->packTemplate), packANDsatisfyEdt, 3, 27);
    ocrEdtTemplateCreate(&(PB->unpackTemplate), unpackEdt, 3, 27);
    ocrEdtTemplateCreate(&(PB->spmvTemplate), spmvEdt, 4, 2);
    ocrEdtTemplateCreate(&(PB->smoothTemplate), smoothEdt, 4, 2);

    u64 dummy;

if(PB->debug > 0) PRINTF("HI%d\n", mynode);

    u32 numnodes = PB->numnodes;

    u32 pz = mynode/(PB->npx*PB->npy);
    u32 py = (mynode/PB->npx)%PB->npy;
    u32 px = mynode%(PB->npx);
    s32 i, j, k;

//initialize sendBlock, sendEvent, and recvEvent

    for(i=0;i<26;i++) {
        PB->haloSendBlock[i] = 1;
    }

    if(pz == 0) {
        PB->haloSendBlock[0] = NULL_GUID;
        PB->haloSendBlock[1] = NULL_GUID;
        PB->haloSendBlock[2] = NULL_GUID;
        PB->haloSendBlock[3] = NULL_GUID;
        PB->haloSendBlock[4] = NULL_GUID;
        PB->haloSendBlock[5] = NULL_GUID;
        PB->haloSendBlock[6] = NULL_GUID;
        PB->haloSendBlock[7] = NULL_GUID;
        PB->haloSendBlock[8] = NULL_GUID;
    }

    if(pz == PB->npz-1) {
        PB->haloSendBlock[17] = NULL_GUID;
        PB->haloSendBlock[18] = NULL_GUID;
        PB->haloSendBlock[19] = NULL_GUID;
        PB->haloSendBlock[20] = NULL_GUID;
        PB->haloSendBlock[21] = NULL_GUID;
        PB->haloSendBlock[22] = NULL_GUID;
        PB->haloSendBlock[23] = NULL_GUID;
        PB->haloSendBlock[24] = NULL_GUID;
        PB->haloSendBlock[25] = NULL_GUID;
    }


    if(py == 0) {
        PB->haloSendBlock[0] = NULL_GUID;
        PB->haloSendBlock[1] = NULL_GUID;
        PB->haloSendBlock[2] = NULL_GUID;
        PB->haloSendBlock[9] = NULL_GUID;
        PB->haloSendBlock[10] = NULL_GUID;
        PB->haloSendBlock[11] = NULL_GUID;
        PB->haloSendBlock[17] = NULL_GUID;
        PB->haloSendBlock[18] = NULL_GUID;
        PB->haloSendBlock[19] = NULL_GUID;
    }

    if(py == PB->npy-1) {
        PB->haloSendBlock[6] = NULL_GUID;
        PB->haloSendBlock[7] = NULL_GUID;
        PB->haloSendBlock[8] = NULL_GUID;
        PB->haloSendBlock[14] = NULL_GUID;
        PB->haloSendBlock[15] = NULL_GUID;
        PB->haloSendBlock[16] = NULL_GUID;
        PB->haloSendBlock[23] = NULL_GUID;
        PB->haloSendBlock[24] = NULL_GUID;
        PB->haloSendBlock[25] = NULL_GUID;
    }


    if(px == 0) {
        PB->haloSendBlock[0] = NULL_GUID;
        PB->haloSendBlock[3] = NULL_GUID;
        PB->haloSendBlock[6] = NULL_GUID;
        PB->haloSendBlock[9] = NULL_GUID;
        PB->haloSendBlock[12] = NULL_GUID;
        PB->haloSendBlock[14] = NULL_GUID;
        PB->haloSendBlock[17] = NULL_GUID;
        PB->haloSendBlock[20] = NULL_GUID;
        PB->haloSendBlock[23] = NULL_GUID;
    }

    if(px == PB->npx-1) {
        PB->haloSendBlock[2] = NULL_GUID;
        PB->haloSendBlock[5] = NULL_GUID;
        PB->haloSendBlock[8] = NULL_GUID;
        PB->haloSendBlock[11] = NULL_GUID;
        PB->haloSendBlock[13] = NULL_GUID;
        PB->haloSendBlock[16] = NULL_GUID;
        PB->haloSendBlock[19] = NULL_GUID;
        PB->haloSendBlock[22] = NULL_GUID;
        PB->haloSendBlock[25] = NULL_GUID;
    }


    u32 m = PB->m[0];
    u32 size[26] = {1, m, 1, m, m*m, m, 1, m, 1, m, m*m, m, m*m, m*m, m, m*m, m, 1, m, 1, m, m*m, m, 1, m, 1};
    u32 ind = 0;
    u32 partner;
    for(k=-1;k<2;k++)
        for(j=-1;j<2;j++)
            for(i=-1;i<2;i++) {
                if(i==0 && j==0 && k==0) continue; //skip "center" of cube
if(PB->debug > 1) PRINTF("ind %d \n", ind);
                if(PB->haloSendBlock[ind] == NULL_GUID) {
                    PB->haloRecvGuid[0][ind] = NULL_GUID;
                    PB->haloRecvGuid[1][ind] = NULL_GUID;
                    PB->haloSendGuid[0][ind] = NULL_GUID;
                    PB->haloSendGuid[1][ind++] = NULL_GUID;
                } else {
                    ocrDbCreate(&(PB->haloSendBlock[ind]), (void**) &dummy, size[ind]*sizeof(double), 0, NULL_GUID, NO_ALLOC);
                    ocrGuidFromIndex(&(PB->haloSendGuid[0][ind]), SB->haloRangeGuid, 52*mynode + ind);
                    ocrGuidFromIndex(&(PB->haloSendGuid[1][ind]), SB->haloRangeGuid, 52*mynode + 26 + ind);
                    ocrGuidFromIndex(&(PB->haloRecvGuid[0][ind]), SB->haloRangeGuid, 52*(mynode + k*PB->npx*PB->npy + j*PB->npx + i) + 25-ind);
                    ocrGuidFromIndex(&(PB->haloRecvGuid[1][ind++]), SB->haloRangeGuid, 52*(mynode + k*PB->npx*PB->npy + j*PB->npx + i) + 26 + 25-ind);
if(PB->debug > 1) PRINTF("mynode %d mysource %d ind %d Send %lx  Recv %lx\n", mynode, mynode+k*PB->npx*PB->npy+j*PB->npx + i, ind-1,PB->haloSendGuid[ind-1], PB->haloRecvGuid[ind-1]);
                }


        }
if(PB->debug > 0) for(i=0;i<26;i++) PRINTF("HI%d i %d sendGuid %lx %lx recvGuid %lx %lx \n", mynode, i, PB->haloSendGuid[0][i], PB->haloSendGuid[1][i], PB->haloRecvGuid[0][i], PB->haloRecvGuid[1][i]);

//initialize four matrices
//P vector used as scratch space
//



//printf("void* %lx, void %d char %d mo %lx dio %lx cio %lx ZO %lx\n", sizeof(void *), sizeof(void), sizeof(char), (u64) PB->matrix_offset[0], (u64) PB->diagonal_index_offset[0], (u64) PB->column_index_offset[0], (u64) PB->vector_offset[0][Z]);
//printf("ptr %lx mo %lx dio %lx cio %lx ZO %lx\n", ptr, (u64) (ptr + (u64) PB->matrix_offset[0]), (u64) (ptr + (u64) PB->diagonal_index_offset[0]), (u64) (ptr + (u64) PB->column_index_offset[0]), (u64) (((u64) ptr) + PB->vector_offset[0][Z]));



    matrixfill(PB->npx, PB->npy, PB->npz, PB->m[0], mynode, (double *) (((u64) ptr) + (u64) PB->matrix_offset[0]), (u32 *) (((u64) ptr) + (u64) PB->diagonal_index_offset[0]), (u32 *)(((u64) ptr) + (u64) PB->column_index_offset[0]), (u32 *) (((u64) ptr) + (u64) PB->vector_offset[0][Z]));
    matrixfill(PB->npx, PB->npy, PB->npz, PB->m[1], mynode, (double *) (((u64) ptr) + PB->matrix_offset[1]), (u32 *) (((u64) ptr) + PB->diagonal_index_offset[1]), (u32 *)(((u64) ptr) + PB->column_index_offset[1]), (u32 *) (((u64) ptr) + PB->vector_offset[0][Z]));
    matrixfill(PB->npx, PB->npy, PB->npz, PB->m[2], mynode, (double *) (((u64) ptr) + PB->matrix_offset[2]), (u32 *) (((u64) ptr) + PB->diagonal_index_offset[2]), (u32 *)(((u64) ptr) + PB->column_index_offset[2]), (u32 *) (((u64) ptr) + PB->vector_offset[0][Z]));
    matrixfill(PB->npx, PB->npy, PB->npz, PB->m[3], mynode, (double *) (((u64) ptr) + PB->matrix_offset[3]), (u32 *) (((u64) ptr) + PB->diagonal_index_offset[3]), (u32 *)(((u64) ptr) + PB->column_index_offset[3]), (u32 *) (((u64) ptr) + PB->vector_offset[0][Z]));

//initialize b vector

    vectorfill(PB->mt[0], (double *) (((u64) ptr) + PB->vector_offset[0][B]), (double *) (((u64) ptr) + PB->matrix_offset[0]));

    gatherfill(&PB->gather[0], PB->m[0]);
    gatherfill(&PB->gather[1], PB->m[1]);
    gatherfill(&PB->gather[2], PB->m[2]);
    gatherfill(&PB->gather[3], PB->m[3]);



//launch hpcg
    ocrGuid_t hpcg;
    ocrEdtCreate(&hpcg, PB->hpcgTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
      EDT_PROP_NONE, NULL_GUID, NULL);

if(PB->debug > 0) PRINTF("HI%d finish \n", mynode);
fflush(stdout);

    ocrDbRelease(PBguid);
    ocrAddDependence(PBguid, hpcg, 0, DB_MODE_RW);


    reductionParallelInit(mynode, depv[2].ptr, depv[3].ptr, depv[3].guid);

    ocrAddDependence(depv[3].guid, hpcg, 1, DB_MODE_RW);

    ocrGuid_t dataBlock;
    ocrDbCreate(&dataBlock, (void**) &dummy, sizeof(double), 0, NULL_GUID, NO_ALLOC);
    ocrAddDependence(dataBlock, hpcg, 2, DB_MODE_RW);
    ocrAddDependence(NULL_GUID, hpcg, 3, DB_MODE_RW);



return NULL_GUID;
}


ocrGuid_t InitEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
params:
0:  mynode

depv:
0: hpcg shared block
1: reduction shared block

create privateblock
create reduction private block

launches hpcgInitEdt with shared blocks, private blocks, mynode as param

*/
    ocrGuid_t SBguid = depv[0].guid;
    sharedBlock_t * SB = (sharedBlock_t *) depv[0].ptr;

    ocrGuid_t hpcgInit, hpcgInitTemplate;

if(SB->debug > 0) PRINTF("I%d\n", paramv[0]);

    ocrEdtTemplateCreate(&hpcgInitTemplate, hpcgInitEdt, 1, 4);

    ocrEdtCreate(&hpcgInit, hpcgInitTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
      EDT_PROP_NONE, NULL_GUID, NULL);


    ocrAddDependence(SBguid, hpcgInit, 0, DB_MODE_RO);

    ocrGuid_t tempDb;
    u64 dummy;
    u64 size;
    u64 m0 = SB->m;
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


    ocrDbCreate(&tempDb, (void**) &dummy, size, 0, NULL_GUID, NO_ALLOC);
    ocrAddDependence(tempDb, hpcgInit, 1, DB_MODE_RW);

//reduction blocks
    ocrAddDependence(depv[1].guid, hpcgInit, 2, DB_MODE_RO);
    ocrAddDependence(reductionCreatePrivate(), hpcgInit, 3, DB_MODE_RW);

    return NULL_GUID;
}

ocrGuid_t realmainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv
0:  npx
1:  npy
2:  npz
3:  m
4:  t
5:  debug
6:  wrapup

depv
0: hpcg Shared Block
1: reduction Shared Block

Initializes reduction shared block
Creates and launches NPZ*NPY*NPX InitEdt with shared blocks and mynode as a param

*/


    ocrGuid_t SBguid = depv[0].guid;
    sharedBlock_t * SB = depv[0].ptr;

    u64 i;
    u64 n = paramv[0]*paramv[1]*paramv[2];


    SB->npx     = paramv[0];
    SB->npy     = paramv[1];
    SB->npz     = paramv[2];
    SB->m       = paramv[3];
    SB->t       = paramv[4];
    SB->debug   = paramv[5];
    SB->wrapup  = paramv[6];
if(SB->debug != 0) PRINTF("RM start n %d npx %d npy %d npz %d m %d t %d debug %d wrapup %lx \n", n, SB->npx, SB->npy, SB->npz, SB->m, SB->t, SB->debug, SB->wrapup);

    ocrGuidRangeCreate(&(SB->haloRangeGuid), 2*26*n, GUID_USER_EVENT_STICKY);

printf("n %d rangeGuid %lx \n", n, SB->haloRangeGuid);
    ocrDbRelease(SBguid);

    reductionSerialInit(n, 1, REDUCTION_F8_ADD, depv[1].ptr, depv[1].guid);

    ocrGuid_t initTemplate, init;
    ocrEdtTemplateCreate(&initTemplate, InitEdt, 1, 2);

    for(i=0;i<n;i++){
        ocrEdtCreate(&init, initTemplate, EDT_PARAM_DEF, &i, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);
        ocrAddDependence(SBguid, init, 0, DB_MODE_RO);
        ocrAddDependence(depv[1].guid, init, 1, DB_MODE_RO);
    }

if(paramv[5] != 0) PRINTF("RM finish\n");
fflush(stdout);

return NULL_GUID;
}

ocrGuid_t wrapupEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
//prints out the deviations from all 1s (which could be done by each rank but this gets them in order



    u32 i, j;
    privateBlock_t * PB = depv[0].ptr;
    double * x = (double *) (((u64)depv[0].ptr) + PB->vector_offset[0][X]);
    double sum;
    for(i=0;i<PB->numnodes;i++) {
      PB = depv[i].ptr;
      sum = 0.0;
      for(j=0;j<PB->mt[0];j++) sum += fabs(1.0-x[j]);
      PRINTF("average deviation for node %d is %f \n", i, sum/PB->mt[0]);
    }

    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {


/*
mainEdt is executed first
Creates the shared datablock
Creates the reduction shared datablock
Creates wrapup
Creates realmain passing in wrapupEdtGuid
Passes the shared blocks to realmain
*/

u32 i;
u64 npx, npy, npz, m, t, debug;

u32 _paramc, _depc, _idep;

void * programArgv = depv[0].ptr;
u32 argc = getArgc(programArgv);

npx = NPX;
npy = NPY;
npz = NPZ;
m = M;
t = T;
debug = DEBUG;

printf("argc %d \n", argc);
//if(argc ==2 || argc ==3) bomb("number of run time parameters cannot be 1 or 2");

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
    t = (u64) atoi(getArgv(programArgv, 5));
    }

if(argc>6) {
    debug = (u64) atoi(getArgv(programArgv, 6));
    }



PRINTF("NPX = %d \nNPY= %d \nNPZ= %d \nM = %d\nt == %d\ndebug = %d\n", npx, npy, npz, m, t, debug);


u64 *dummy;

ocrGuid_t realmain, realmainTemplate, sharedDb, reductionSharedBlock;


ocrGuid_t wrapupTemplate;
ocrGuid_t wrapup;


ocrEdtTemplateCreate(&wrapupTemplate, wrapupEdt, 0, npx*npy*npz);
ocrEdtCreate(&wrapup, wrapupTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

ocrEdtTemplateCreate(&realmainTemplate, realmainEdt, 7, 2);

u64 paramvout[7] = {npx, npy, npz, m, t, debug, wrapup};

ocrEdtCreate(&realmain, realmainTemplate, EDT_PARAM_DEF, paramvout, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

ocrDbCreate(&sharedDb, (void**) &dummy, sizeof(sharedBlock_t), 0, NULL_GUID, NO_ALLOC);
ocrAddDependence(sharedDb, realmain, 0, DB_MODE_RW);

//initializing reduction library
reductionSharedBlock = reductionCreateShared();
ocrAddDependence(reductionSharedBlock, realmain, 1, DB_MODE_RW);

return NULL_GUID;

}


