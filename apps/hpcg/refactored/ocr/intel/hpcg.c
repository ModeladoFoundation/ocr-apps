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
*/

#define ENABLE_EXTENSION_LABELING
#include "ocr.h"
#include "extensions/ocr-labeling.h" //currently needed for labeled guids
#include "string.h" //if memcpy is needed
#include "stdio.h"  //needed for printf debugging
#include "math.h" //for integer abs

#include "reduction.h"

#define PRECONDITIONER  //undefine if you want to run without the precondition

#define NPX 3  //number of workers is NPX x NPY x NPZ
#define NPY 4
#define NPZ 5
#define N (NPX*NPY*NPZ)
#define M0 16   // size of local block
#define M02 (M0*M0)
#define M03 (M0*M0*M0)
#define M1 ((M0+1)/2)
#define M12 (M1*M1)
#define M13 (M1*M1*M1)
#define M2 ((M1+1)/2)
#define M22 (M2*M2)
#define M23 (M2*M2*M2)
#define M3 ((M2+1)/2)
#define M32 (M3*M3)
#define M33 (M3*M3*M3)
#define MT0 (M0*M0*M0)
#define MT1 (M1*M1*M1)
#define MT2 (M2*M2*M2)
#define MT3 (M3*M3*M3)
#define HT0 ((M0+2)*(M0+2)*(M0+2))
#define HT1 ((M1+2)*(M1+2)*(M1+2))
#define HT2 ((M2+2)*(M2+2)*(M2+2))
#define HT3 ((M3+2)*(M3+2)*(M3+2))
#define T 50  //number of time steps

typedef struct{
    u64 start[4][26];
    u64 l1[4][26];
    u64 l2[4][26];
    u64 p1[4][26];
    u64 p2[4][26];
    } gather_t;

typedef struct{
    u64 numnodes;
    u64 size[4];
    gather_t gather;
    ocrGuid_t haloRangeGuid;
    ocrGuid_t hpcgTemplate;
    ocrGuid_t spmvTemplate;
    ocrGuid_t mgTemplate;
    ocrGuid_t haloTemplate;
    ocrGuid_t smoothTemplate;
    ocrGuid_t packTemplate;
    ocrGuid_t unpackTemplate;
    ocrGuid_t hpcgInitTemplate;
    ocrGuid_t wrapup;
    } sharedBlock_t;


typedef struct{
    u64 mynode;
    u64 timestep;
    ocrGuid_t haloSendBlock[26];
    ocrGuid_t haloSendGuid[2][26];
    ocrGuid_t haloRecvGuid[2][26];
    u64 hePhase;
    u64 hpcgPhase;
    u64 mgPhase[4];
    double rtr;
    double rtr0;
    double rtz;
    double rtzold;
    double mySum;
//some vectors may not need to use HT, MT might be enough but can't take advantage
    double vector[6][HT0+HT1+HT2+HT3];
    double matrix[27*(MT0+MT1+MT2+MT3)];
    u64 col_ind[27*(MT0+MT1+MT2+MT3)];
    u64 diag[MT0+MT1+MT2+MT3];
    } privateBlock_t;

//indices of vectors (arbitrary order)
#define X 0
#define R 1
#define B 2
#define P 3
#define AP 4
#define Z 5


u64 debug = 0; //controls debug printing..1 is some and 2 is LOTS




void compute_column_indices(u64 size, u64 * ind) {
u64 i, j, k, c;
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


    if(debug > 0) {
        c=0;
        for(k=0;k<size;k++) {

            printf("%d \n", k);
            for(j=0;j<size;j++) {
                for(i=0;i<size;i++) printf("%5d", ind[c++]);
                printf("\n");
            }
        }
    }

return;
}

void vectorfill(u64 size, double * b, double * a) {
//size is M*
//computes vector b as the row sums of A
    u64 i, j;
    u64 s3 = size*size*size;

    for(i=0;i<s3;i++) {
        *b = 0;
        for(j=0;j<27;j++) *b += *(a++);
        b++;
    }
    if(debug > 1) {
        b -= s3;
        for(i=0;i<s3;i++) printf("b ind %d val %f s3 %d \n", i, b[i], s3);
    }
    return;
}

void matrixfill(u64 size, u64 mynode, double * a, u64 * diag, u64 * ind, u64 * column_index) {
//size is M*
//a and ind  are 27*size^3
//diag is length size^3
//column index is (size+2)^3 used to compute offsets once

if(debug > 0) printf("MF%d size %d \n", mynode, size);
    u64 s3 = size*size*size;
    s64 i, j, k, m;
    s64 offset[27], *p;
    u64 index;

//used to zero out elements that are outside the global grid
    s64 znoff[9] = {-27,-26,-25,-24,-23,-22,-21,-20,-19};
    s64 ynoff[9] = {-27,-26,-25,-18,-17,-16,-9,-8,-7};
    s64 xnoff[9] = { -27,-24,-21,-18,-15,-12,-9,-6,-3};
    s64 zpoff[9] = {-9,-8,-7,-6,-5,-4,-3,-2,-1};
    s64 ypoff[9] = {-21,-20,-19,-12,-11,-10,-3,-2,-1};
    s64 xpoff[9] = {-25,-22,-19,-16,-13,-10,-7,-4,-1};
//locate mynode
    u64 pz = mynode/(NPX*NPY);
    u64 py = (mynode/NPX)%NPY;
    u64 px = mynode%(NPX);

    u64 counter = 0;
    double * asave;
    u64 * diagsave;

    asave = a;
    diagsave = diag;


if(debug > 0) printf("MF%d px %d py %d pz %d \n", mynode, px, py, pz);

    p = offset;

    for(k=-1;k<2;k++)
        for(j=-1;j<2;j++)
            for(i=-1;i<2;i++)
                *(p++) = k*(size+2)*(size+2) + j*(size+2) + i;

if(debug > 1) for(i=0;i<27;i++) printf("MF%d offset %d value %d \n", mynode, i, offset[i]);

    compute_column_indices(size+2, column_index);

    for(k=1;k<size+1;k++)
        for(j=1;j<size+1;j++)
            for(i=1;i<size+1;i++) {
                index = i + j*(size+2) + k*(size+2)*(size+2);

                for(m=0;m<13;m++) {
                    counter++;
                    *a++ = -1;
                    *ind++ = column_index[index + offset[m]];
                }
                *diag++ = counter++;
                *a++ = 26;
                *ind++ = column_index[index]; //offset[13] = 0

                for(m=14;m<27;m++) {
                     counter++;
                    *a++ = -1;
                    *ind++ = column_index[index + offset[m]];
                }
//zero out elements of A that are outside the global boundary (legal to leave them as zeros)
    if(px == 0 && i == 1) for(m=0;m<9;m++) a[xnoff[m]] = 0.0;
    if(py == 0 && j == 1) for(m=0;m<9;m++) a[ynoff[m]] = 0.0;
    if(pz == 0 && k == 1) for(m=0;m<9;m++) a[znoff[m]] = 0.0;
    if(px == NPX-1 && i == size) for(m=0;m<9;m++) a[xpoff[m]] = 0.0;
    if(py == NPY-1 && j == size) for(m=0;m<9;m++) a[ypoff[m]] = 0.0;
    if(pz == NPZ-1 && k == size) for(m=0;m<9;m++) a[zpoff[m]] = 0.0;
    }

if(debug > 1) for(i=0;i<size*size*size;i++) printf("MF i %d diag %d a %f\n", i, diagsave[i], asave[diagsave[i]]);
if(debug > 0) printf("MF%d finish %d \n", mynode, size);
fflush(stdout);
}





ocrGuid_t haloExchangeEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {
/*
paramv
0: matrixID
1: vector index
2: return event for unpack
3: return event for pack

depv
0: shared block
1: private block

launches packANDsatisfy
launches unpack

*/


    u64 i, errno;
    ocrGuid_t packEdt, unpackEdt;

    sharedBlock_t * SB = depv[0].ptr;
    privateBlock_t * PB = depv[1].ptr;
    u64 mynode = PB->mynode;
    u64 phase = PB->hePhase;
    PB->hePhase ^= 1;      //toggle between 0 and 1

if(debug != 0) printf("HE%d start\n", mynode);
if(debug != 0) printf("HE%d start depv0 %lx \n", mynode, depv[0].guid);
if(debug != 0) printf("HE%d start depv1 %lx \n", mynode, depv[1].guid);
if(debug != 0) printf("HE%d start param0 %d \n", mynode, paramv[0]);
if(debug != 0) printf("HE%d start param1 %d \n", mynode, paramv[1]);
if(debug != 0) printf("HE%d start param2 %lx \n", mynode, paramv[2]);
if(debug > 1) for(i=0;i<26;i++) printf("HE%d i %d sendevent %lx \n", mynode, i, PB->haloSendGuid[phase][i]);
if(debug > 1) for(i=0;i<26;i++) printf("HE%d i %d recvevent %lx \n", mynode, i, PB->haloRecvGuid[phase][i]);
if(debug > 1) for(i=0;i<26;i++) printf("HE%d i %d sendblock %lx \n", mynode, i, PB->haloSendBlock[i]);

fflush(stdout);

    u64 paramvPack[4] = {paramv[0], paramv[1], paramv[3], phase};
    ocrEdtCreate(&packEdt, SB->packTemplate, EDT_PARAM_DEF, paramvPack, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    u64 paramvUnpack[4] = {paramv[0], paramv[1], paramv[2], phase};
    ocrEdtCreate(&unpackEdt, SB->unpackTemplate, EDT_PARAM_DEF, paramvUnpack, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

if(debug > 1) printf("HE%d after create \n", mynode);

    ocrAddDependence(depv[0].guid, packEdt, 0, DB_MODE_RO);
    ocrAddDependence(depv[1].guid, packEdt, 1, DB_MODE_RW);
if(debug > 1) printf("HE%d after 0-1\n", mynode);
    for(i=0;i<26;i++) {
if(debug > 1) printf("HE%d attach sendblock %lx in slot(+2) %d  \n", mynode, PB->haloSendBlock[i], i );
        ocrAddDependence(PB->haloSendBlock[i], packEdt, i+2, DB_MODE_RW);
    }
if(debug != 0) printf("HE%d after sendblocks\n", mynode);

    ocrAddDependence(depv[0].guid, unpackEdt, 0, DB_MODE_RO);
    ocrAddDependence(depv[1].guid, unpackEdt, 1, DB_MODE_RW);

if(debug != 0) printf("HE%d after 0-2 \n", mynode);

    for(i=0;i<26;i++) {

        if(PB->haloRecvGuid[phase][i] != NULL_GUID){
            errno = ocrEventCreate(&(PB->haloRecvGuid[phase][i]), OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            ocrAddDependence(PB->haloRecvGuid[phase][i], unpackEdt, i+2, DB_MODE_RO);
            }
          else ocrAddDependence(NULL_GUID, unpackEdt, i+2, DB_MODE_RO);
}

if(debug != 0) printf("HE%d finish\n", mynode);
fflush(stdout);

    return(NULL_GUID);

}

ocrGuid_t packANDsatisfyEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
param
0: matrixID
1: vector index
2: return event
3: hePhase   //choose between two sets of GUIDs

depv
0: sharedBlock
1: privateBlock
2-27: datablocks to send (if on boundary will be NULL)

pack extracts the boundary values from the the source vector
copies them to the send buffers
send buffers are in lexicographic order

*/
    u64 matrixID = paramv[0];
    u64 vi = paramv[1];
    ocrGuid_t event = paramv[2];
    u64 phase = paramv[3];

    sharedBlock_t * SB = depv[0].ptr;

    privateBlock_t * PB = depv[1].ptr;
    u64 mynode = PB->mynode;


if(debug != 0) printf("PK%d ID%d start P%d vi %d \n", mynode, matrixID, phase, vi);
    u64 errno;

    gather_t * g = &(SB->gather);
    u64 * start = (g->start[matrixID]);
    u64 * l1 = (g->l1[matrixID]);
    u64 * l2 = (g->l2[matrixID]);
    u64 * p1 = (g->p1[matrixID]);
    u64 * p2 = (g->p2[matrixID]);

    u64 i, j, i1, i2;
    double *src, * s, * d;

    src = PB->vector[vi];
    if(matrixID > 0) src += HT0;
    if(matrixID > 1) src += HT1;
    if(matrixID > 2) src += HT2;

if(debug != 0) printf("PK%d  ID%d start vi %d \n", mynode, matrixID, vi);
if(debug > 1) for(i=0;i<28;i++) printf("PK%d ID%d depv[%d] %lx \n", mynode, matrixID, i, depv[i].guid);

    for(i=0;i<26;i++) {
if(debug > 1) printf("PK%d i %d start %d l1 %d l2 %d p1 %d p2 %d len %d block %lx\n", mynode, i, *start, *l1, *l2, *p1, *p2, *l1*(*l2), depv[i+2].guid);
        if(depv[i+2].guid != NULL_GUID) {

            d = depv[i+2].ptr;
            s = src;


            s = src + *start;
            for(i2=0;i2<*l2;i2++) {
                for(i1=0;i1<*l1;i1++) {
                    *(d++) = *s;
                    s += *p1;
                }
                s += *p2;
            }
if(debug > 0) printf("PK%d DIR%d satisfy %lx with %lx\n", mynode, i, PB->haloSendGuid[phase][i], depv[i+2].guid);
fflush(stdout);
            errno = ocrEventCreate(&(PB->haloSendGuid[phase][i]), OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            ocrDbRelease(depv[i+2].guid);
            ocrEventSatisfy(PB->haloSendGuid[phase][i], depv[i+2].guid);
         }
         start++;
         l1++;
         l2++;
         p1++;
         p2++;

    }
if(debug != 0) printf("PK%d finish\n", mynode);
fflush(stdout);

ocrEventSatisfy(event, NULL_GUID);

return(NULL_GUID);

}

ocrGuid_t unpackEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
param
0: matrixID
1: vector index
2: returnEvent
3: hePhase

depv
0: sharedBlock
1: privateBlock
2-27: datablocks to put into dest

copies the values from the received data blocks into the destination vector
sends the private block (to either spmv or smooth)
*/
    u64 matrixID = paramv[0];
    u64 vi = paramv[1];
    ocrGuid_t returnEvent = paramv[2];
    u64 phase = paramv[3];

    ocrGuid_t sticky;

    sharedBlock_t * SB = depv[0].ptr;


    privateBlock_t * PB = depv[1].ptr;
    u64 mynode = PB->mynode;


    u64 i;

    if(debug > 0) printf("UN%d matrixID %d  phase %d vi %d \n", mynode, matrixID, phase, vi);
    double * b;
    double * dest = PB->vector[vi];
if(debug >1 ) for(i=0;i<MT0;i++) printf("UN%d i %d v %f \n", mynode, i, *(dest+i));
    if(matrixID == 0) dest += MT0;
    if(matrixID == 1) dest += HT0 + MT1;
    if(matrixID == 2) dest += HT0 + HT1 + MT2;
    if(matrixID == 3) dest += HT0 + HT1 + HT2 + MT3;


if(debug > 1) for(i=0;i<29;i++) printf("UN%d ID%d depv[%d] %lx \n", mynode, matrixID, i, depv[i].guid);


    for(i=0;i<26;i++) {
        if(PB->haloRecvGuid[phase][i] != NULL_GUID) {
if(debug > 0) printf("UN%d ID%d i %d destroy %lx \n", mynode, matrixID,  i, PB->haloRecvGuid[phase][i]);
            ocrEventDestroy(PB->haloRecvGuid[phase][i]);
            PB->haloSendBlock[i] = depv[i+2].guid;
        }
    }


    u64 j, len;
    double * a;

    for(i=0;i<26;i++) {
if(debug > 1) printf("UN%d start i %d\n", mynode, i);
        b = depv[i+2].ptr;
        len = SB->gather.l1[matrixID][i]*SB->gather.l2[matrixID][i];

        if(b == NULL) for(j=0;j<len;j++) *dest++ = 0.0;
          else {
if(debug >1) printf("UN%d i %d v %f\n", mynode, i, *b);

            for(j=0;j<len;j++) {
               *dest++ = *b++;
            }

        }
    }

    dest = PB->vector[vi];
    if(debug > 1) for(j=0;j<HT0;j++)printf("UN%d %d %f \n", mynode, j, *(dest++));

    ocrDbRelease(depv[1].guid);
    ocrEventSatisfy(returnEvent, depv[1].guid);
    return(NULL_GUID);
}





ocrGuid_t smoothEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv
0: matrixID
1: rhs vector index
2: solution vector index
3: return event

depv
0: shared block
1: private block

smooth does a forward and backward Gauss-Seidel sweep
then sends the private block

*/
    u64 matrixID = paramv[0];
    u64 rhsind = paramv[1];
    u64 solnind = paramv[2];

    sharedBlock_t * SB = depv[0].ptr;

    privateBlock_t * PB = depv[1].ptr;


    u64 mynode = PB->mynode;
if(debug >= 1) printf("SM%d MID%d start \n", mynode, matrixID);

    double * rhs = PB->vector[rhsind];
    if(matrixID == 1) rhs += HT0;
    if(matrixID == 2) rhs += HT0 + HT1;
    if(matrixID == 3) rhs += HT0 + HT1 + HT2;

    double * soln = PB->vector[solnind];
    if(matrixID == 1) soln += HT0;
    if(matrixID == 2) soln += HT0 + HT1;
    if(matrixID == 3) soln += HT0 + HT1 + HT2;

    double * a = PB->matrix;
    if(matrixID == 1) a += 27*MT0;
    if(matrixID == 2) a += 27*(MT0 + MT1);
    if(matrixID == 3) a += 27*(MT0 + MT1 + MT2);

    u64 * ind = PB->col_ind;
    if(matrixID == 1) ind += 27*MT0;
    if(matrixID == 2) ind += 27*(MT0 + MT1);
    if(matrixID == 3) ind += 27*(MT0 + MT1 + MT2);

    u64 * diag = PB->diag;
    if(matrixID == 1) diag += MT0;
    if(matrixID == 2) diag += MT0 + MT1;
    if(matrixID == 3) diag += MT0 + MT1 + MT2;


    u64 size = SB->size[matrixID];
    u64 hsize = (size+2)*(size+2)*(size+2);
    size = size*size*size;



if(debug >= 1) printf("SM%d MID%d size %d \n", mynode, matrixID, size);

    double * abase = a;
    u64 * indbase = ind;
    double * solnbase = soln;
    u64 * diagbase = diag;



    u64 i, j;
    double sum;

soln = solnbase;

//forward sweep
    for(i=0;i<size;i++){
if(debug >= 2) printf("SM%d i %d rhs %f \n", mynode, i, rhs[i]);
        sum = rhs[i];
        for(j=0;j<27;j++){
if(debug >= 2) printf("SM%d i %d j %d sum %f a %f ind %d z %f \n", mynode, i, j, sum, a[j], ind[j], solnbase[ind[j]]);
           sum -= a[j]*solnbase[ind[j]];
        }
if(debug >= 2) printf("SM%d diag index %d diag value %f\n", mynode, *diag, abase[*diag]);

        sum += abase[*diag]*solnbase[indbase[*diag]];
        *(soln++) = sum/abase[*(diag++)];
if(debug >= 2) printf("SM%d diag index %d diag value %f sum %f soln %f \n", mynode, *(diag-1), abase[*(diag-1)], sum, *(soln-1));
        a += 27;
        ind += 27;
    }
if(debug > 2){
    soln = solnbase;
    for(i=0;i<size;i++)
       printf("SM%d MID%d i %d val %f \n", mynode, matrixID, i, soln[i]);
}

if(debug >= 1) printf("SM%d MID%d finish forward\n", mynode, matrixID);
fflush(stdout);
//backward sweep
    a = abase;
    a += 27*(size-1);
    ind = indbase;
    ind += 27*(size-1);
    soln = solnbase;
    soln += size-1;
    diag = diagbase;
    diag += size-1;

    for(i=size;i>0;i--){
        sum = rhs[i-1];
if(debug > 1) printf("SM%d i %d sum %f \n", mynode, i, sum);
        for(j=0;j<27;j++){
if(debug >= 2) printf("SM%d i %d j %d sum %f a %f ind %d z %f \n", mynode, i-1, j, sum, a[j], ind[j], solnbase[ind[j]]);
           sum -= a[j]*solnbase[ind[j]];
        }
        sum += abase[*diag]*solnbase[indbase[*diag]];
        *(soln--) = sum/abase[*(diag--)];
if(debug >= 2) printf("SM%d diag index %d diag value %f sum %f soln %f \n", mynode, *(diag+1), abase[*(diag+1)], sum, *(soln+1));
        a -= 27;
        ind -= 27;
    }


sum = 0;
soln = solnbase;
for(i=0;i<size;i++) sum += *soln++;

    ocrDbRelease(depv[1].guid);
    ocrEventSatisfy(paramv[3], depv[1].guid);
    return NULL_GUID;
}

ocrGuid_t spmvEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv
0: matrixID
1: vector src
2: vector dst
3: return event

depv
0: shared block
1: private block

computes the local sparse matrix-vector product
sends private block
*/
    u64 matrixID = paramv[0];
    u64 srcind = paramv[1];
    u64 destind = paramv[2];

    sharedBlock_t * SB = depv[0].ptr;

    privateBlock_t * PB = depv[1].ptr;
    u64 mynode = PB->mynode;


    double * src = PB->vector[srcind];
    if(matrixID == 1) src += HT0;
    if(matrixID == 2) src += HT0 + HT1;
    if(matrixID == 3) src += HT0 + HT1 + HT2;

    double * dest = PB->vector[destind];
    if(matrixID == 1) dest += HT0;
    if(matrixID == 2) dest += HT0 + HT1;
    if(matrixID == 3) dest += HT0 + HT1 + HT2;

    double * a = PB->matrix;
    if(matrixID == 1) a += 27*MT0;
    if(matrixID == 2) a += 27*(MT0 + MT1);
    if(matrixID == 3) a += 27*(MT0 + MT1 + MT2);

    u64 * ind = PB->col_ind;
    if(matrixID == 1) ind += 27*MT0;
    if(matrixID == 2) ind += 27*(MT0 + MT1);
    if(matrixID == 3) ind += 27*(MT0 + MT1 + MT2);

    u64 size = SB->size[matrixID];
    size = size*size*size;

    u64 i, j;
    double sum;
    for(i=0;i<size;i++){
        sum = 0;
        for(j=0;j<27;j++){
if(debug >= 2) printf("SPMV%d i %d j %d dest %f a %f ind %d src %f \n", mynode, i, j, sum, a[j], ind[j], src[ind[j]]);
           sum += a[j]*src[ind[j]];
        }
        *(dest++) = sum;
        a += 27;
        ind += 27;
    }

    dest = PB->vector[destind];
    if(matrixID == 1) dest += HT0;
    if(matrixID == 2) dest += HT0 + HT1;
    if(matrixID == 3) dest += HT0 + HT1 + HT2;
if(debug > 1) for(i=0;i<size;i++)
   printf("SPMV%d i %d val %f \n", mynode, i, dest[i]);

    ocrDbRelease(depv[1].guid);
    ocrEventSatisfy(paramv[3], depv[1].guid);
    return NULL_GUID;
}


ocrGuid_t mgEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
paramv
0: mgStep (0 to 6)
1: return event

depv
0: sharedBlock
1: privateBlock

this is the driver for the multigrid steps.

*/

    u64 mgStep = paramv[0];
    ocrGuid_t returnEvent = paramv[1];

    u64 matrixID = 3 - abs(mgStep-3);

    sharedBlock_t * SB = depv[0].ptr;
    u64 m = SB->size[matrixID];
    u64 i;
    double * z;
    privateBlock_t * PB = depv[1].ptr;
    u64 phase = PB->mgPhase[matrixID];

    ocrGuid_t halo, packEvent, unpackEvent, smooth, smoothEvent, spmv, spmvEvent, mg, mgevent;

if(debug > 0) printf("MG%d S%d P%d ID%d start return event %lx\n", PB->mynode, mgStep, phase, matrixID, paramv[1]);


    switch(phase) {

        case 0:  //smoothing
//zero out z vector
        z = PB->vector[Z];
        switch(matrixID) {
            case 0:
                for(i=0;i<MT0;i++) *(z++) = 0.0;
                break;
            case 1:
                z += HT0;
                for(i=0;i<MT1;i++) *(z++) = 0.0;
                break;
            case 2:
                z += HT0+ HT1;
                for(i=0;i<MT2;i++) *(z++) = 0.0;
                break;
            case 3:
                z += HT0+ HT1 + HT2;
                for(i=0;i<MT3;i++) *(z++) = 0.0;
                break;
        }

if(debug > 0) printf("MG%d S%d P%d  create clone\n", PB->mynode, mgStep, phase, matrixID);
//create clone, smooth, and halo

        PB->mgPhase[matrixID] = 1;
        ocrEdtCreate(&mg, SB->mgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrEventCreate(&smoothEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        u64 paramvsmooth0[4] = {matrixID, R, Z, smoothEvent};
        ocrEdtCreate(&smooth, SB->smoothTemplate, EDT_PARAM_DEF, paramvsmooth0, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrAddDependence(smoothEvent, mg, 1, DB_MODE_RW);
        ocrEventCreate(&unpackEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrAddDependence(unpackEvent, smooth, 1, DB_MODE_RW);
        ocrEventCreate(&packEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrAddDependence(packEvent, smooth, 2, DB_MODE_RW);

        u64 paramvhalo0[4] = {matrixID, Z, unpackEvent, packEvent};
        ocrEdtCreate(&halo, SB->haloTemplate, EDT_PARAM_DEF, paramvhalo0, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);


        ocrDbRelease(depv[0].guid);
        ocrAddDependence(depv[0].guid, mg, 0, DB_MODE_RO);
        ocrAddDependence(depv[0].guid, smooth, 0, DB_MODE_RO);
        ocrAddDependence(depv[0].guid, halo, 0, DB_MODE_RO);

        ocrDbRelease(depv[1].guid);
        ocrAddDependence(depv[1].guid, halo, 1, DB_MODE_RW);

if(debug > 0) printf("MG%d S%d P%d ID%d finish\n", PB->mynode, mgStep, phase, matrixID);
fflush(stdout);

        return NULL_GUID;

        case 1:  //return from smooth, start SPMV

//return if at the bottom
        if(paramv[0] == 3) {
            ocrEventSatisfy(returnEvent, depv[1].guid);
            return NULL_GUID;
        }

//create clone
        PB->mgPhase[matrixID] = 2;
        ocrEdtCreate(&mg, SB->mgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);


        ocrEventCreate(&spmvEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        u64 paramvspmv1[4] = {matrixID, Z, AP, spmvEvent};
        ocrEdtCreate(&spmv, SB->spmvTemplate, EDT_PARAM_DEF, paramvspmv1, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrEventCreate(&unpackEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrEventCreate(&packEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        u64 paramvhalo1[4] = {matrixID, Z, unpackEvent, packEvent};
        ocrEdtCreate(&halo, SB->haloTemplate, EDT_PARAM_DEF, paramvhalo1, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrAddDependence(depv[0].guid, mg, 0, DB_MODE_RO);
        ocrAddDependence(depv[0].guid, spmv, 0, DB_MODE_RO);
        ocrAddDependence(depv[0].guid, halo, 0, DB_MODE_RO);

        ocrDbRelease(depv[1].guid);
        ocrAddDependence(spmvEvent, mg, 1, DB_MODE_RW);
        ocrAddDependence(unpackEvent, spmv, 1, DB_MODE_RW);
        ocrAddDependence(packEvent, spmv, 2, DB_MODE_RW);
        ocrAddDependence(depv[1].guid, halo, 1, DB_MODE_RW);
if(debug > 0) printf("MG%d S%d P%d ID%d finish\n", PB->mynode, mgStep, phase, matrixID);
fflush(stdout);

            return NULL_GUID;
        case 2:  //return from SPMV, launch recursive call

//create clone

        PB->mgPhase[matrixID] = 3;
        u64 paramvout[2] = {6-paramv[0], paramv[1]};
//needed because the template is used after the release of the shared block
        ocrGuid_t sbMgTemplate = SB->mgTemplate;
        ocrEdtCreate(&mg, sbMgTemplate, EDT_PARAM_DEF, paramvout, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        ocrDbRelease(depv[0].guid);
        ocrAddDependence(depv[0].guid, mg, 0, DB_MODE_RO);
        ocrEventCreate(&returnEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrAddDependence(returnEvent, mg, 1, DB_MODE_RW);

//create restriction
        double * ap  = PB->vector[AP];
        double * rold  = PB->vector[R];
        double * rnew  = PB->vector[R];
        rnew += HT0;
        switch(matrixID) {
        case 0:
           for(i=0;i<MT1;i++){
              *(rnew++) = *rold - *ap;
if(debug > 1)printf("i %d ap %f rold %f rnew %f \n", i, *ap, *rold, *(rnew-1));
              ap += 2;
              rold += 2;
           }
           break;
        case 1:
           ap += HT0;
           rold += HT0;
           rnew += HT1;
           for(i=0;i<MT2;i++){
              *(rnew++) = *rold - *ap;
if(debug > 1) printf("i %d ap %f rold %f rnew %f \n", i, *ap, *rold, *(rnew-1));
              ap += 2;
              rold += 2;
           }
           break;

        case 2:
           ap += HT0+HT1;
           rold += HT0+HT1;
           rnew += HT1+HT2;
           for(i=0;i<MT3;i++){
              *(rnew++) = *rold - *ap;
if(debug > 1) printf("i %d ap %f rold %f rnew %f \n", i, *ap, *rold, *(rnew-1));
              ap += 2;
              rold += 2;
           }
           break;
        case 3:
PRINTF("MG%d S%d P%d ID%d hit case 3 in restriction\n", PB->mynode, mgStep, phase, matrixID);

        }

//create recursive call
        mgStep++;
        paramvout[0] = mgStep;
        paramvout[1] = returnEvent;
        matrixID = 3 - abs(mgStep-3);
        PB->mgPhase[matrixID] = 0;
        ocrEdtCreate(&mg, sbMgTemplate, EDT_PARAM_DEF, paramvout, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        ocrAddDependence(depv[0].guid, mg, 0, DB_MODE_RO);
if(debug > 0) printf("MG%d S%d P%d ID%d finish\n", PB->mynode, mgStep, phase, matrixID);
fflush(stdout);
        ocrDbRelease(depv[1].guid);
        ocrAddDependence(depv[1].guid, mg, 1, DB_MODE_RW);
        return NULL_GUID;

        case 3:  //return from recursive call

//do prolongation (add returned half Z to full Z)


        z  = PB->vector[Z];
        double * znew  = PB->vector[Z];
        znew += HT0;
        switch(matrixID) {
        case 0:
           for(i=0;i<MT1;i++){
if(debug > 1) printf("MG%d S%d P%d ID%d i %d z %f update %f\n", PB->mynode, mgStep, phase, matrixID, i, *z, *znew);
              *z += *(znew++);
              z += 2;
           }
           break;
        case 1:
           z += HT0;
           znew += HT1;
           for(i=0;i<MT2;i++){
if(debug > 1) printf("MG%d S%d P%d ID%d i %d z %f update %f\n", PB->mynode, mgStep, phase, matrixID, i, *z, *znew);
              *z += *(znew++);
              z += 2;
           }
           break;

        case 2:
           z += HT0 + HT1;
           znew += HT1 + HT2;

           for(i=0;i<MT3;i++){
if(debug > 0) printf("MG%d S%d P%d ID%d i %d z %f update %f\n", PB->mynode, mgStep, phase, matrixID, i, *z, *znew);
              *z += *(znew++);
              z += 2;
           }
           break;
        case 3:
printf("MG%d S%d P%d ID%d hit case 3 in prolong\n", PB->mynode, mgStep, phase, matrixID);
        ocrShutdown();

        }
//create clone, smooth, and halo
//
if(debug > 0) printf("MG%d S%d P%d  launch halo\n", PB->mynode, mgStep, phase, matrixID);

        PB->mgPhase[matrixID] = 4;
        ocrEdtCreate(&mg, SB->mgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrEventCreate(&smoothEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        u64 paramvsmooth3[4] = {matrixID, R, Z, smoothEvent};
        ocrEdtCreate(&smooth, SB->smoothTemplate, EDT_PARAM_DEF, paramvsmooth3, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrEventCreate(&unpackEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        ocrEventCreate(&packEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);

        u64 paramvhalo3[4] = {matrixID, R, unpackEvent, packEvent};
        ocrEdtCreate(&halo, SB->haloTemplate, EDT_PARAM_DEF, paramvhalo3, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrDbRelease(depv[0].guid);
        ocrAddDependence(depv[0].guid, mg, 0, DB_MODE_RO);
        ocrAddDependence(depv[0].guid, smooth, 0, DB_MODE_RO);
        ocrAddDependence(depv[0].guid, halo, 0, DB_MODE_RO);

        ocrAddDependence(smoothEvent, mg, 1, DB_MODE_RW);
        ocrAddDependence(unpackEvent, smooth, 1, DB_MODE_RW);
        ocrDbRelease(depv[1].guid);

        ocrAddDependence(packEvent, smooth, 2, DB_MODE_RW);
        ocrAddDependence(depv[1].guid, halo, 1, DB_MODE_RW);
        return NULL_GUID;

        case 4:  //return from final smooth
        ocrEventSatisfy(paramv[1], depv[1].guid);
if(debug > 0) printf("MG%d S%d P%d ID%d finish\n", PB->mynode, mgStep, phase, matrixID);
fflush(stdout);
        return NULL_GUID;
    }

}

ocrGuid_t hpcgEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){

/*
depv
0: sharedBlock
1: privateBlock
2: reduction private block
3: my datablock
3: return block from reduction (or NULL_GUID)

hpcg driver.
only work done is local linear algebra
*/
    sharedBlock_t * SB = depv[0].ptr;
    privateBlock_t * PB = depv[1].ptr;
//reduction private block is depv[2]
    double * myDataBlock = depv[3].ptr;
    double * returnBlock = depv[4].ptr;

    u64 mynode = PB->mynode;
    u64 timestep = PB->timestep;
    u64 phase = PB->hpcgPhase;


if(debug > 0) printf("PCG%d T%d P%d begin shared %lx private %lx returnblock %lx \n", mynode, timestep, phase, depv[0].guid, depv[1].guid, depv[2].guid);


    ocrGuid_t onceEvent;
    double *a, sum, pap, rtz, alpha, beta;
    u64 i, j, ind, errno;
    ocrGuid_t mg, halo, spmv, unpackEvent, packEvent, spmvEvent, hpcg, hpcgEvent;

    switch (phase) {
        case 0:
//Initial call only

        for(i=0;i<MT0;i++) PB->vector[X][i] = 0.0;
        a = PB->matrix;;
        for(i=0;i<MT0;i++){
            PB->vector[R][i] = 0.0;
            for(j=0;j<27;j++) PB->vector[R][i] += *(a++);
            PB->vector[B][i] = PB->vector[R][i];
        }



//local sum
        sum = 0;
        for(i=0;i<MT0;i++) sum += PB->vector[R][i]*PB->vector[R][i];
        *myDataBlock = sum;  //local sum

if(debug > 0) printf("PCG%d T%d P%d rtr %f\n", mynode, timestep, phase, PB->mySum);
//fprintf(stderr,"PCG%d T%d P%d rtr %f\n", mynode, timestep, phase, PB->mySum);

//create clone

        PB->hpcgPhase = 1;
        ocrEdtCreate(&hpcg, SB->hpcgTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

//get return event from reduction

        onceEvent = reductionGetOnceEvent(depv[2].ptr, depv[2].guid);
        ocrAddDependence(onceEvent, hpcg, 4, DB_MODE_RO);

        ocrAddDependence(depv[0].guid, hpcg, 0, DB_MODE_RO);
        ocrDbRelease(depv[1].guid);
        ocrAddDependence(depv[1].guid, hpcg, 1, DB_MODE_RW);
        ocrAddDependence(depv[2].guid, hpcg, 2, DB_MODE_RW);
        ocrAddDependence(depv[3].guid, hpcg, 3, DB_MODE_RW);

//launch reduction

        ocrDbRelease(depv[3].guid);
        reductionLaunch(depv[2].ptr, depv[2].guid, depv[3].guid);

if(debug > 0) printf("PCG%d T%d P%d finish \n", mynode, timestep, phase);
fflush(stdout);
        return NULL_GUID;


    case 1:

//consume rtr   CONVERGENCE test
if(debug > 0) printf("PCG%d T%d P%d global rtr %f \n", mynode, timestep, phase, *returnBlock);
        if(mynode==0) PRINTF("time %d rtr %f \n", timestep, *returnBlock);
        if(timestep==0) PB->rtr0 = *returnBlock;
           //else if(*returnBlock/PB->rtr0 < 1e-13 || timestep == T) {
           else if(*returnBlock/PB->rtr0 < 1e-13 || timestep == 50) {
if(debug > 0) printf("PCG%d T%d P%d finishing \n", mynode, timestep, phase);
fflush(stdout);
             ocrAddDependence(depv[1].guid, SB->wrapup, mynode, DB_MODE_RO);
             return NULL_GUID;
        }

#ifdef PRECONDITIONER

//preconditioning...launch mg

//create clone
        PB->hpcgPhase = 2;
        PB->mgPhase[0] = 0;

        ocrEdtCreate(&hpcg, SB->hpcgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrEventCreate(&hpcgEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
        u64 paramvMG[2] = {0, hpcgEvent};
        ocrEdtCreate(&mg, SB->mgTemplate, EDT_PARAM_DEF, paramvMG, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

if(debug > 0) printf("PCG%d T%d P%d return from mg event %lx \n", mynode, timestep, phase, hpcgEvent);
        ocrAddDependence(depv[0].guid, hpcg, 0, DB_MODE_RO);

        ocrAddDependence(hpcgEvent, hpcg, 1, DB_MODE_RW);
        ocrAddDependence(depv[2].guid, hpcg, 2, DB_MODE_RW);
        ocrAddDependence(depv[3].guid, hpcg, 3, DB_MODE_RW);
        ocrAddDependence(NULL_GUID, hpcg, 4, DB_MODE_RW);

        ocrAddDependence(depv[0].guid, mg, 0, DB_MODE_RO);
        ocrDbRelease(depv[1].guid);
        ocrAddDependence(depv[1].guid, mg, 1, DB_MODE_RW);


if(debug > 0) printf("PCG%d T%d P%d finish \n", mynode, timestep, phase);
fflush(stdout);
        return NULL_GUID;

#else
//preconditioning NOT, just copy R to Z
        for(i=0;i<MT0;i++) PB->vector[Z][i] =  PB->vector[R][i];
#endif
    case 2:

//consume Z
//compute local rtz
        sum = 0;
        for(i=0;i<MT0;i++) {
            sum += PB->vector[Z][i]*PB->vector[R][i];
        }

        *myDataBlock = sum;
if(debug > 0) printf("PCG%d T%d P%d local rtz %f \n", mynode, timestep, phase, sum);
if(debug > 1) for(i=0;i<MT0;i++) printf("PCG%d T%d P%d i %d Z %f R %f \n", mynode, timestep, phase, i, PB->vector[Z][i], PB->vector[R][i]);


//Create clone
        PB->hpcgPhase = 3;

        ocrEdtCreate(&hpcg, SB->hpcgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        onceEvent = reductionGetOnceEvent(depv[2].ptr, depv[2].guid);
        ocrAddDependence(onceEvent, hpcg, 4, DB_MODE_RO);

        ocrAddDependence(depv[0].guid, hpcg, 0, DB_MODE_RO);
        ocrDbRelease(depv[1].guid);
        ocrAddDependence(depv[1].guid, hpcg, 1, DB_MODE_RW);
        ocrAddDependence(depv[2].guid, hpcg, 2, DB_MODE_RW);
//note that depv[3] doesn't need to be released here, just before calling rductionLaunch
        ocrAddDependence(depv[3].guid, hpcg, 3, DB_MODE_RW);

//launch reduction

        ocrDbRelease(depv[3].guid);
        reductionLaunch(depv[2].ptr, depv[2].guid, depv[3].guid);


        return NULL_GUID;

    case 3:
//consume rtz
        if(mynode==0) PRINTF("time %d rtz %f \n", timestep, *returnBlock);
        PB->rtz = *returnBlock;
//compute beta
        if(timestep == 0) beta = 0;
               else beta = *returnBlock/PB->rtzold;
//update p
        for(i=0;i<MT0;i++) PB->vector[P][i] = PB->vector[Z][i] + beta*PB->vector[P][i];

//compute Ap

ocrEventCreate(&packEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
ocrEventCreate(&unpackEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
ocrEventCreate(&spmvEvent, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);

//create clone
        PB->hpcgPhase = 4;
        ocrEdtCreate(&hpcg, SB->hpcgTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        u64 paramvspmv[4] = {0, P, AP, spmvEvent};
        ocrEdtCreate(&spmv, SB->spmvTemplate, EDT_PARAM_DEF, paramvspmv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        u64 paramvhalo[4] = {0, P, unpackEvent, packEvent};
        ocrEdtCreate(&halo, SB->haloTemplate, EDT_PARAM_DEF, paramvhalo, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        ocrDbRelease(depv[0].guid);
        ocrAddDependence(depv[0].guid, hpcg, 0, DB_MODE_RO);
        ocrAddDependence(depv[0].guid, spmv, 0, DB_MODE_RO);
        ocrAddDependence(depv[0].guid, halo, 0, DB_MODE_RO);

        ocrDbRelease(depv[1].guid);
        ocrAddDependence(spmvEvent, hpcg, 1, DB_MODE_RW);
        ocrAddDependence(unpackEvent, spmv, 1, DB_MODE_RW);

        ocrAddDependence(packEvent, spmv, 2, DB_MODE_RW);
        ocrAddDependence(depv[2].guid, hpcg, 2, DB_MODE_RW);
        ocrAddDependence(depv[3].guid, hpcg, 3, DB_MODE_RW);
        ocrAddDependence(NULL_GUID, hpcg, 4, DB_MODE_RW);
        ocrAddDependence(depv[1].guid, halo, 1, DB_MODE_RW);


if(debug > 0) printf("PCG%d T%d P%d finish \n", mynode, timestep, phase);
fflush(stdout);
        return NULL_GUID;



//compute pAp (consuming Ap)

    case 4:
        sum = 0;
        for(i=0;i<MT0;i++) sum += PB->vector[P][i]*PB->vector[AP][i];
        *myDataBlock= sum;
if(debug > 0) printf("PCG%d T%d P%d pap %f \n", mynode, timestep, phase, sum);
//Create clone
        PB->hpcgPhase=5;
        ocrEdtCreate(&hpcg, SB->hpcgTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        onceEvent = reductionGetOnceEvent(depv[2].ptr, depv[2].guid);
        ocrAddDependence(onceEvent, hpcg, 4, DB_MODE_RO);

        ocrAddDependence(depv[0].guid, hpcg, 0, DB_MODE_RO);
        ocrDbRelease(depv[1].guid);
        ocrAddDependence(depv[1].guid, hpcg, 1, DB_MODE_RW);
        ocrAddDependence(depv[2].guid, hpcg, 2, DB_MODE_RW);
//note that depv[3] doesn't need to be released here, just before calling rductionLaunch
        ocrAddDependence(depv[3].guid, hpcg, 3, DB_MODE_RW);

//launch reduction

        ocrDbRelease(depv[3].guid);
        reductionLaunch(depv[2].ptr, depv[2].guid, depv[3].guid);

if(debug > 0) printf("PCG%d T%d P%d finish \n", mynode, timestep, phase);
fflush(stdout);
        return NULL_GUID;

    case 5:
//consume pAp
        pap = *returnBlock;
        alpha = PB->rtz/pap;
if(debug > 0) printf("PCG%d T%d P%d pap %f rtz %f\n", mynode, timestep, phase, pap, *returnBlock);
        PB->rtzold = PB->rtz;  //safe time to move it
//update x and r
        for(i=0;i<MT0;i++) {
            PB->vector[X][i] += alpha*PB->vector[P][i];
            PB->vector[R][i] -= alpha*PB->vector[AP][i];
if(debug > 1) printf("PCG%d T%d P%d i %d x %f r %f p %f ap %f \n ", mynode, timestep, phase, i, PB->vector[X][i], PB->vector[R][i], PB->vector[P][i], PB->vector[AP][i]);

        }
//Prepare for the summation of rtr
        sum = 0;
        for(i=0;i<MT0;i++) sum += PB->vector[R][i]*PB->vector[R][i];
        *myDataBlock = sum;  //local sum
if(debug > 0) printf("PCG%d T%d P%d rtr %f \n", mynode, timestep, phase, sum);
//create clone
        PB->hpcgPhase=1;
        PB->timestep++;

        ocrEdtCreate(&hpcg, SB->hpcgTemplate, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);

        onceEvent = reductionGetOnceEvent(depv[2].ptr, depv[2].guid);
        ocrAddDependence(onceEvent, hpcg, 4, DB_MODE_RO);

        ocrAddDependence(depv[0].guid, hpcg, 0, DB_MODE_RO);
        ocrDbRelease(depv[1].guid);
        ocrAddDependence(depv[1].guid, hpcg, 1, DB_MODE_RW);
        ocrAddDependence(depv[2].guid, hpcg, 2, DB_MODE_RW);
//note that depv[3] doesn't need to be released here, just before calling rductionLaunch
        ocrAddDependence(depv[3].guid, hpcg, 3, DB_MODE_RW);

//launch reduction

        ocrDbRelease(depv[3].guid);
        reductionLaunch(depv[2].ptr, depv[2].guid, depv[3].guid);

if(debug > 0) printf("PCG%d T%d P%d finish \n", mynode, timestep, phase);
fflush(stdout);
        return NULL_GUID;
        }
    }


ocrGuid_t hpcgInitEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
params
0: mynode

depv
0: sharedBlock
1: privateBlock
2: reduction shared block
3: reduction private block

Initialize private block
launch hpcgEdt
*/
    u64 mynode = paramv[0];
    sharedBlock_t * SB = depv[0].ptr;
    privateBlock_t * PB = depv[1].ptr;

if(debug > 0) printf("HI%d\n", mynode);
    u64 dummy;
    u64 numnodes = SB->numnodes;

    u64 pz = mynode/(NPX*NPY);
    u64 py = (mynode/NPX)%NPY;
    u64 px = mynode%(NPX);
    s64 i, j, k;

    PB->timestep = 0;
    PB->mynode = mynode;
    PB->hePhase = 0;
    PB->hpcgPhase = 0;

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

    if(pz == NPZ-1) {
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

    if(py == NPY-1) {
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

    if(px == NPX-1) {
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


    u64 size[26] = {1, M0, 1, M0, M02, M0, 1, M0, 1, M0, M02, M0, M02, M02, M0, M02, M0, 1, M0, 1, M0, M02, M0, 1, M0, 1};
    u64 ind = 0;
    u64 partner;
    for(k=-1;k<2;k++)
        for(j=-1;j<2;j++)
            for(i=-1;i<2;i++) {
                if(i==0 && j==0 && k==0) continue; //skip "center" of cube
if(debug > 1) printf("ind %d \n", ind);
                if(PB->haloSendBlock[ind] == NULL_GUID) {
                    PB->haloRecvGuid[0][ind] = NULL_GUID;
                    PB->haloRecvGuid[1][ind] = NULL_GUID;
                    PB->haloSendGuid[0][ind] = NULL_GUID;
                    PB->haloSendGuid[1][ind++] = NULL_GUID;
                } else {
                    ocrDbCreate(&(PB->haloSendBlock[ind]), (void**) &dummy, size[ind]*sizeof(double), 0, NULL_GUID, NO_ALLOC);
                    ocrGuidFromIndex(&(PB->haloSendGuid[0][ind]), SB->haloRangeGuid, 52*mynode + ind);
                    ocrGuidFromIndex(&(PB->haloSendGuid[1][ind]), SB->haloRangeGuid, 52*mynode + 26 + ind);
                    ocrGuidFromIndex(&(PB->haloRecvGuid[0][ind]), SB->haloRangeGuid, 52*(mynode + k*NPX*NPY + j*NPX + i) + 25-ind);
                    ocrGuidFromIndex(&(PB->haloRecvGuid[1][ind++]), SB->haloRangeGuid, 52*(mynode + k*NPX*NPY + j*NPX + i) + 26 + 25-ind);
if(debug > 1) printf("mynode %d mysource %d ind %d Send %lx  Recv %lx\n", mynode, mynode+k*NPX*NPY+j*NPX + i, ind-1,PB->haloSendGuid[ind-1], PB->haloRecvGuid[ind-1]);
                }


        }
if(debug > 0) for(i=0;i<26;i++) printf("HI%d i %d sendGuid %lx %lx recvGuid %lx %lx \n", mynode, i, PB->haloSendGuid[0][i], PB->haloSendGuid[1][i], PB->haloRecvGuid[0][i], PB->haloRecvGuid[1][i]);

//initialize four matrices
//P vector used as scratch space

    matrixfill(M0, mynode, PB->matrix, PB->diag, PB->col_ind, (u64 *) PB->vector[P]);
    matrixfill(M1, mynode, &(PB->matrix[27*MT0]), &(PB->diag[MT0]), &(PB->col_ind[27*MT0]), (u64 *) PB->vector[P]);
    matrixfill(M2, mynode, &(PB->matrix[27*(MT0+MT1)]), &(PB->diag[MT0+MT1]), &(PB->col_ind[27*(MT0+MT1)]), (u64 *) PB->vector[P]);
    matrixfill(M3, mynode, &(PB->matrix[27*(MT0+MT1+MT2)]), &(PB->diag[MT0+MT1+MT2]), &(PB->col_ind[27*(MT0+MT1+MT2)]), (u64 *) PB->vector[P]);

//initialize b vectors

    vectorfill(M0, PB->vector[B], PB->matrix);
    vectorfill(M1, &(PB->vector[B][HT0]),&(PB->matrix[27*MT0]));
    vectorfill(M2, &(PB->vector[B][HT0+HT1]), &(PB->matrix[27*(MT0+MT1)]));
    vectorfill(M3, &(PB->vector[B][HT0+HT1+HT2]), &(PB->matrix[27*(MT0+MT1+MT2)]));

if(debug > 1) {
u64 i, j, *ind, *diag;
double * a;

a = PB->matrix;
ind = PB->col_ind;
diag = PB->diag;
for(i=0;i<MT0;i++) {
   for(j=0;j<27;j++) printf("HI%d i %d j%d a %f \n", mynode, i, j, *a++);
   printf("HI%d i %d diag %d \n", mynode, i, *diag++);
}

a = PB->vector[B];
for(i=0;i<MT0;i++) printf("HI%d i %d B %f \n", mynode, i, *a++);

a = PB->matrix;
a += 27*MT0;
ind = PB->col_ind;
ind += 27*MT0;
diag = PB->diag;
diag += MT0;
for(i=0;i<MT1;i++) {
   for(j=0;j<27;j++) printf("HI%d i %d j%d a %f \n", mynode, i, j, *a++);
   printf("HI%d i %d diag %d \n", mynode, i, *diag++);
}

a = PB->vector[B];
a += HT0;
for(i=0;i<MT1;i++) printf("HI%d i %d B %f \n", mynode, i, *a++);


a = PB->matrix;
a += 27*(MT0+MT1);
ind = PB->col_ind;
ind += 27*(MT0+MT1);
diag = PB->diag;
diag += MT0+MT1;
for(i=0;i<MT2;i++) {
   for(j=0;j<27;j++) printf("HI%d i %d j%d a %f \n", mynode, i, j, *a++);
   printf("HI%d i %d diag %d \n", mynode, i, *diag++);
}

a = PB->vector[B];
a += HT0+HT1;
for(i=0;i<MT2;i++) printf("HI%d i %d B %f \n", mynode, i, *a++);

a = PB->matrix;
a += 27*(MT0+MT1+MT2);
ind = PB->col_ind;
ind += 27*(MT0+MT1+MT2);
diag = PB->diag;
diag += MT0+MT1+MT2;
for(i=0;i<MT3;i++) {
   for(j=0;j<27;j++) printf("HI%d i %d j%d a %f \n", mynode, i, j, *a++);
   printf("HI%d i %d diag %d \n", mynode, i, *diag++);
}
a = PB->vector[B];
a += HT0+HT1+HT2;
for(i=0;i<MT3;i++) printf("HI%d i %d B %f \n", mynode, i, *a++);

}

//launch hpcg
    ocrGuid_t hpcg;
    ocrEdtCreate(&hpcg, SB->hpcgTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
      EDT_PROP_NONE, NULL_GUID, NULL);


    ocrAddDependence(depv[0].guid, hpcg, 0, DB_MODE_RO);
    ocrDbRelease(depv[1].guid);
    ocrAddDependence(depv[1].guid, hpcg, 1, DB_MODE_RW);

    reductionParallelInit(mynode, depv[2].ptr, depv[3].ptr, depv[3].guid);
    ocrAddDependence(depv[3].guid, hpcg, 2, DB_MODE_RW);

    ocrGuid_t dataBlock;
    ocrDbCreate(&dataBlock, (void**) &dummy, sizeof(double), 0, NULL_GUID, NO_ALLOC);
    ocrAddDependence(dataBlock, hpcg, 3, DB_MODE_RW);
    ocrAddDependence(NULL_GUID, hpcg, 4, DB_MODE_RW);

if(debug > 0) printf("HI%d finish \n", mynode);
fflush(stdout);

return NULL_GUID;
}


ocrGuid_t InitEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
params:
0: mynode

depv:
0: sharedblock
1: reduction shared block

create privateblock
create reduction private block
launches hpcgInitEdt with shared blocks, private blocks, mynode as param

*/

    u64 mynode = paramv[0];
    sharedBlock_t * SB = depv[0].ptr;
    ocrGuid_t hpcgInit;

if(debug > 0) printf("I%d\n", mynode);

    ocrEdtCreate(&hpcgInit, SB->hpcgInitTemplate, EDT_PARAM_DEF, &mynode, EDT_PARAM_DEF, NULL,
      EDT_PROP_NONE, NULL_GUID, NULL);


    ocrAddDependence(depv[0].guid, hpcgInit, 0, DB_MODE_RO);

    ocrGuid_t tempDb;
    u64 dummy;
    ocrDbCreate(&tempDb, (void**) &dummy, sizeof(privateBlock_t), 0, NULL_GUID, NO_ALLOC);
    ocrAddDependence(tempDb, hpcgInit, 1, DB_MODE_RW);

//reduction blocks
    ocrAddDependence(depv[1].guid, hpcgInit, 2, DB_MODE_RO);
    ocrAddDependence(reductionCreatePrivate(), hpcgInit, 3, DB_MODE_RW);

    return NULL_GUID;
}

ocrGuid_t realmainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv
0:  wrapupEdtGuid

depv
0: shared block
1: reduction Shared Block

Initializes shared block
Initializes reduction shared block
Creates and launches NPZ*NPY*NPX InitEdt with shared blocks and mynode as param
*/

    u64 i, j, k, m;
    privateBlock_t * PB[N];
    double * vector[N];
    double * a;


if(debug != 0) printf("RM start\n");


    sharedBlock_t * SB = depv[0].ptr;

    reductionSerialInit(N, 1, REDUCTION_F8_ADD, depv[1].ptr, depv[1].guid);

    SB->wrapup = paramv[0];
    SB->numnodes = N;
    SB->size[0] = M0;
    SB->size[1] = M1;
    SB->size[2] = M2;
    SB->size[3] = M3;

    ocrGuid_t tempDb;
    u64 dummy;


    ocrGuidRangeCreate(&(SB->haloRangeGuid),(u64) 2*26*N, GUID_USER_EVENT_STICKY);


    ocrEdtTemplateCreate(&(SB->packTemplate), packANDsatisfyEdt, 4, 28);
    ocrEdtTemplateCreate(&(SB->unpackTemplate), unpackEdt, 4, 28);
    ocrEdtTemplateCreate(&(SB->hpcgTemplate), hpcgEdt, 0, 5);
    ocrEdtTemplateCreate(&(SB->spmvTemplate), spmvEdt, 4, 3);
    ocrEdtTemplateCreate(&(SB->mgTemplate), mgEdt, 2, 2);
    ocrEdtTemplateCreate(&(SB->haloTemplate), haloExchangeEdt, 4, 2);
    ocrEdtTemplateCreate(&(SB->smoothTemplate), smoothEdt, 4, 3);
    ocrEdtTemplateCreate(&(SB->hpcgInitTemplate), hpcgInitEdt, 1, 4);

//table for packing communication blocks
u64 start[4][26]= {{0, 0,  M0-1, 0,  0,   M0-1,M02-M0, M02-M0, M02-1, 0,    0,      M0-1, 0,   M0-1,  M02-M0, M02-M0,  M02-1, (M0-1)*M02,(M0-1)*M02,  M03-M02+M0-1, (M0-1)*M02, (M0-1)*M02, M03-M02+M0-1, M03-M0, M03-M0,  M03-1},
                   {0, 0,  M1-1, 0,  0,   M1-1,M12-M1, M12-M1, M12-1, 0,    0,      M1-1, 0,   M1-1,  M12-M1, M12-M1,  M12-1, (M1-1)*M12,(M1-1)*M12,  M13-M12+M1-1, (M1-1)*M12, (M1-1)*M12, M13-M12+M1-1, M13-M1, M13-M1,  M13-1},
                   {0, 0,  M2-1, 0,  0,   M2-1,M22-M2, M22-M2, M22-1, 0,    0,      M2-1, 0,   M2-1,  M22-M2, M22-M2,  M22-1, (M2-1)*M22,(M2-1)*M22,  M23-M22+M2-1, (M2-1)*M22, (M2-1)*M22, M23-M22+M2-1, M23-M2, M23-M2,  M23-1},
                   {0, 0,  M3-1, 0,  0,   M3-1,M32-M3, M32-M3, M32-1, 0,    0,      M3-1, 0,   M3-1,  M32-M3, M32-M3,  M32-1, (M3-1)*M32,(M3-1)*M32,  M33-M32+M3-1, (M3-1)*M32, (M3-1)*M32, M33-M32+M3-1, M33-M3, M33-M3,  M33-1}};
u64 l1[4][26]=    {{1, M0, 1,    M0, M02, M0,  1,       M0,    1,     M0,   M0,    M0,    M02, M02,   M0,     M0,      M0,    1,          M0,         1,             M0,         M02,       M0,           1,      M0,      1},
                   {1, M1, 1,    M1, M12, M1,  1,       M1,    1,     M1,   M1,     M1,   M12, M12,   M1,     M1,      M1,    1,          M1,         1,             M1,         M12,       M1,           1,      M1,      1},
                   {1, M2, 1,    M2, M22, M2,  1,       M2,    1,     M2,   M2,     M2,   M22, M22,   M2,     M2,      M2,    1,          M2,         1,             M2,         M22,       M2,           1,      M2,      1},
                   {1, M3, 1,    M3, M32, M3,  1,       M3,    1,     M3,   M3,     M3,   M32, M32,   M3,     M3,      M3,    1,          M3,         1,             M3,         M32,       M3,           1,      M3,      1}};
u64 l2[4][26]=    {{1, 1,  1,    1,  1,   1,   1,       1,     1,     1,    M0,     1,    1,   1,     1,      M0,      1,     1,          1,          1,             1,          1,         1,            1,      1,       1},
                   {1, 1,  1,    1,  1,   1,   1,       1,     1,     1,    M1,     1,    1,   1,     1,      M1,      1,     1,          1,          1,             1,          1,         1,            1,      1,       1},
                   {1, 1,  1,    1,  1,   1,   1,       1,     1,     1,    M2,     1,    1,   1,     1,      M2,      1,     1,          1,          1,             1,          1,         1,            1,      1,       1},
                   {1, 1,  1,    1,  1,   1,   1,       1,     1,     1,    M3,     1,    1,   1,     1,      M3,      1,     1,          1,          1,             1,          1,         1,            1,      1,       1}};
u64 p1[4][26]=    {{1, 1,  1,    M0, 1,   M0,  1,       1,     1,     M02,  1,      M02,  M0,  M0,    M02,    1,       M02,   1,          1,          1,             M0,         1,         M0,           1,      1,       1},
                   {1, 1,  1,    M1, 1,   M1,  1,       1,     1,     M12,  1,      M12,  M1,  M1,    M12,    1,       M12,   1,          1,          1,             M1,         1,         M1,           1,      1,       1},
                   {1, 1,  1,    M2, 1,   M2,  1,       1,     1,     M22,  1,      M22,  M2,  M2,    M22,    1,       M22,   1,          1,          1,             M2,         1,         M2,            1,      1,       1},
                   {1, 1,  1,    M3, 1,   M3,  1,       1,     1,     M32,  1,      M32,  M3,  M3,    M32,    1,       M32,   1,          1,          1,             M3,         1,         M3,           1,      1,       1}};
u64 p2[4][26]=    {{1, 1,  1,    1,  1,   1,   1,       1,     1,     1,    M02-M0, 1,    1,   1,     1,      M02-M0, 1,      1,          1,          1,             1,          1,         1,            1,      1,       1},
                   {1, 1,  1,    1,  1,   1,   1,       1,     1,     1,    M12-M1, 1,    1,   1,     1,      M12-M1, 1,      1,          1,          1,             1,          1,         1,            1,      1,       1},
                   {1, 1,  1,    1,  1,   1,   1,       1,     1,     1,    M22-M2, 1,    1,   1,     1,      M22-M2, 1,      1,          1,          1,             1,          1,         1,            1,      1,       1},
                   {1, 1,  1,    1,  1,   1,   1,       1,     1,     1,    M32-M3, 1,    1,   1,     1,      M32-M3, 1,      1,          1,          1,             1,          1,         1,            1,      1,       1}};


for(i=0;i<4;i++)
   for(j=0;j<26;j++){
     SB->gather.start[i][j] = start[i][j];
     SB->gather.l1[i][j] = l1[i][j];
     SB->gather.l2[i][j] = l2[i][j];
     SB->gather.p1[i][j] = p1[i][j];
     SB->gather.p2[i][j] = p2[i][j];
     }

    ocrGuid_t initTemplate, init;
    ocrEdtTemplateCreate(&initTemplate, InitEdt, 1, 2);
    ocrDbRelease(depv[0].guid);

    for(i=0;i<N;i++){
        ocrEdtCreate(&init, initTemplate, EDT_PARAM_DEF, &i, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);
        ocrAddDependence(depv[0].guid, init, 0, DB_MODE_RO);
        ocrAddDependence(depv[1].guid, init, 1, DB_MODE_RO);
    }
if(debug != 0) printf("RM finish\n");
fflush(stdout);

return NULL_GUID;
}

ocrGuid_t wrapupEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
//prints out the deviations from all 1s (which could be done by each rank but this gets them in order


if(debug != 0) printf("in wrapup\n");

    u64 i, j;
    privateBlock_t * PB;
    double sum;
    for(i=0;i<N;i++) {
      PB = depv[i].ptr;
      sum = 0.0;
      for(j=0;j<MT0;j++) sum += fabs(1.0-PB->vector[X][j]);
      printf("average deviation for node %d is %f \n", i, sum/MT0);
    }

    ocrShutdown();
    return NULL_GUID;
}


ocrGuid_t mainEdt(){

/*
mainEdt is executed first
Creates the shared datablock
Creates the reduction shared datablock
Creates wrapup
Creates realmain passing in wrapupEdtGuid
Passes the shared blocks to realmain
*/

u64 i;


PRINTF("NPX = %d \nNPY= %d \nNPZ= %d \nM0 = %d\n", NPX, NPY, NPZ, M0);

u64 *dummy;

ocrGuid_t realmain, realmainTemplate, realmainOutputEvent, sharedDb, privateDb[N], vectorDb[N], matrix0Db[N], matrix1Db[N], matrix2Db[N], matrix3Db[N];

ocrDbCreate(&sharedDb, (void**) &dummy, sizeof(sharedBlock_t), 0, NULL_GUID, NO_ALLOC);

ocrGuid_t wrapupTemplate;
ocrGuid_t wrapup;

//initializing reduction library
ocrGuid_t reductionSharedBlock = reductionCreateShared();

ocrEdtTemplateCreate(&wrapupTemplate, wrapupEdt, 0, N);

ocrEdtCreate(&wrapup, wrapupTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);

ocrEdtTemplateCreate(&realmainTemplate, realmainEdt, 1, 2);

ocrEdtCreate(&realmain, realmainTemplate, EDT_PARAM_DEF, &wrapup , EDT_PARAM_DEF, NULL,
  EDT_PROP_NONE, NULL_GUID, NULL);

ocrAddDependence(sharedDb, realmain, 0, DB_MODE_RW);
ocrAddDependence(reductionSharedBlock, realmain, 1, DB_MODE_RW);

return NULL_GUID;

}


