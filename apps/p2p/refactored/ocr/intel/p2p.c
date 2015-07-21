/*
Written by David S. Scott
Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/
/*
OCR version of the Parallel Research Kernel synch_p2p
See README file for more information

*/
#include <ocr.h>
#include <stdio.h>
#define T 300    //number of timesteps
#define P 20     //number of workers
#define K 20     //columns per worker
#define M (P*K) //global number of columns
#define B 5     //blocking factor
#define W 20     //number of blocks in vertical direction
#define N (B*W+1) //number of rows
#define ARRAY(i,j) vector[(i)*K + (j)] //translation from 2d to 1d

typedef struct{
    double data[B+1];
    ocrGuid_t event;
    } buffer_t;

typedef struct{
    u64 timestep;
    u64 mynode;
    u64 phase;
    ocrGuid_t template;
    ocrGuid_t sendrightevent;
    ocrGuid_t leftold;
    ocrGuid_t leftrecvevent; //used by node 0 to catch data from P-1 at the end of a timestep
    u64 next; //next block for node0 to send out
    ocrGuid_t block[W];//list of guids for the buffer blocks (could be smaller if W<P);
    } private0_t;
typedef struct{
    u64 timestep;
    u64 mynode;
    u64 phase;
    ocrGuid_t template;
    ocrGuid_t sendrightevent;
    ocrGuid_t leftold;
    } private_t;

ocrGuid_t p2pTask(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {
/*
Paramv none
Depv:
0: My datablock
1: My private block (parameters)
2: Leftin block (satisfied by my right neighbor, will be sent on, destroyed by P-1)
*/
    u64 i, j, dummy;
    ocrGuid_t bufferDb, sticky, leftinevent, sendrightevent, p2pEdt;
    double * vector = depv[0].ptr;
    private_t * private = depv[1].ptr;
    private0_t * private0 = depv[1].ptr;


    buffer_t * leftin = depv[2].ptr;
    u64 mynode = private->mynode;
    u64 timestep = private->timestep;
    u64 phase = private->phase;
//PRINTF("N%d T%d P%d guids %lx %lx %lx\n", mynode, timestep, phase, depv[0].guid, depv[1].guid, depv[2].guid);
//PRINTF("N%d T%d P%d left old %lx \n", mynode, timestep, phase, private->leftold);
    if(private->leftold != NULL_GUID) ocrEventDestroy(private->leftold); //destroy old event
    if(phase == 0 && timestep == 0){ //initialize array
        for(i=0;i<N;i++) for(j=0;j<K;j++) ARRAY(i,j) = 0.0;
        for(j=0;j<K;j++) ARRAY(0,j) = (double) mynode*K+j; //first (distributed) row
        if(mynode == 0) {
            for(i=0;i<N;i++) ARRAY(i,0) = (double) i;  //first column
//create list of buffer blocks
            private0->block[0] = depv[2].guid;
            for(i=1;i<W;i++) {
                ocrDbCreate(&bufferDb, (void**) &dummy, sizeof(buffer_t), 0, NULL_GUID, NO_ALLOC);
                private0->block[i] = bufferDb;
            }
            private0->next = 1;
        }
    }
    if(mynode == 0 && phase == 0 && timestep !=0) ARRAY(0,0) = -leftin->data[B];//wrap around value
//PRINTF("N%d T%d P%d in event %d\n", mynode, timestep, phase, leftin->event);
//for(i=0;i<B+1;i++)
//PRINTF("N%d T%d P%d datain %d %g\n", mynode, timestep, phase, i, leftin->data[i]);
//compute first column
    if(mynode != 0) for(i=phase*B+1;i<(phase+1)*B+1;i++)
            ARRAY(i,0) = ARRAY(i-1,0) + leftin->data[i-phase*B] - leftin->data[i-phase*B-1];
    for(j=1;j<K;j++){//loop over columns
        for(i=phase*B+1;i<(phase+1)*B+1;i++)//loop over rows
              ARRAY(i,j) = ARRAY(i-1,j) + ARRAY(i,j-1) - ARRAY(i-1,j-1);
        }
//PRINTF("N%d T%d P%d finish computing \n", mynode, timestep, phase);
//send right (including node P-1 who is just recycling the block except on the last phase)

    if(mynode != 0) {
        leftinevent = leftin->event;
        private->leftold = leftinevent;
    } else if (phase == 0 && timestep != 0) {
        private0->leftrecvevent = leftin->event; //save for next iteration
        private0->leftold = NULL_GUID;
    }
    if(mynode != P-1 || phase == W-1) {
        for(i=0;i<B+1;i++) leftin->data[i] = ARRAY(i+phase*B,K-1);
        sendrightevent = private->sendrightevent;
//for(i=0;i<B+1;i++)
//PRINTF("N%d T%d P%d dataout %d %g\n", mynode, timestep, phase, i, leftin->data[i]);
//PRINTF("N%d T%d P%d sending to %d \n", mynode, timestep, phase, sendrightevent);
        ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, true);
//PRINTF("N%d T%d P%d create sticky %lx \n", mynode, timestep, phase, sticky);
//PRINTF("N%d T%d P%d leftin->event %d \n", mynode, timestep, phase, leftinevent);
        leftin->event = sticky;
        private->sendrightevent = sticky;
        ocrDbRelease(depv[2].guid);
//PRINTF("N%d T%d P%d satisfy event %lx \n", mynode, timestep, phase, sendrightevent);
        ocrEventSatisfy(sendrightevent, depv[2].guid);
    }
//PRINTF("N%d T%d P%d destroy Db %d \n", mynode, timestep, phase, depv[2].guid);


//PRINTF("N%d T%d P%d finish sending\n", mynode, timestep, phase);
    private->phase++;
//PRINTF("N%d T%d P%d newphase %d \n", mynode, timestep, phase, private->phase);
    if(private->phase == W){ //finish timestep
        private->phase = 0;
        private->timestep++;
//PRINTF("N%d T%d P%d newtimestep %d newphase %d \n", mynode, timestep, phase, private->timestep, private->phase);
        if(timestep == T-1){
            if(mynode == P-1) {
                if(ARRAY(N-1,K-1) == T*(N+M-2)) PRINTF("PASS checksum = %f   \n", ARRAY(N-1,K-1));
                  else PRINTF("FAIL  checksum = %f  should be %d \n", ARRAY(N-1,K-1), T*(N+M-2));
                ocrShutdown();
                }
            return NULL_GUID;
        }
    }
//create and launch clone

    ocrEdtCreate(&p2pEdt, private->template, EDT_PARAM_DEF, NULL,
          EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
//PRINTF("N%d T%d P%d guids %d %d %d\n", mynode, timestep, phase, depv[0].guid, depv[1].guid, depv[2].guid);
//fflush(stdout);
    ocrDbRelease(depv[0].guid);
    ocrAddDependence(depv[0].guid, p2pEdt, 0 , DB_MODE_RW);
//PRINTF("N%d T%d P%d after depv0\n", mynode, timestep, phase);

//PRINTF("N%d T%d P%d after depv1\n", mynode, timestep, phase);
    if(mynode != 0) {
//PRINTF("N%d T%d P%d launching clone with event %lx \n", mynode, timestep, phase, leftinevent);
//fflush(stdout);
        ocrAddDependence(leftinevent, p2pEdt, 2 , DB_MODE_RW);
    } else {
        if(phase == W-1) {
            ocrAddDependence(private0->leftrecvevent, p2pEdt, 2 , DB_MODE_RW);
//PRINTF("N%d T%d P%d launching clone with event %lx \n", mynode, timestep, phase, ((private0_t *) private)->leftrecvevent);
//fflush(stdout);
        } else {
//PRINTF("N%d T%d P%d launching clone with db %lx \n", mynode, timestep, phase, private0->block[private0->next]);
            ocrAddDependence(private0->block[private0->next], p2pEdt, 2 , DB_MODE_RW);
            private0->next = (private0->next + 1) % W;
        }
    }
    ocrDbRelease(depv[1].guid);
    ocrAddDependence(depv[1].guid, p2pEdt, 1 , DB_MODE_RW);
    return NULL_GUID;
}


ocrGuid_t realmainTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
//params: none
    ocrGuid_t p2pTemplate, Template0, p2pEdt[P];
    u64 i, j, *dummy;
    ocrGuid_t sticky;

    double * vector[P];
    private_t * private[P];

    ocrEdtTemplateCreate(&p2pTemplate, p2pTask, 0, 3);

//PRINTF("RM start \n");
//for(i=0;i<P;i++) PRINTF("RM i %d data %d private %d \n", i, depv[i].guid, depv[i+P].guid);
//PRINTF("RM db %d \n", depv[2*P].guid);

    for(i=0;i<P;i++){
        vector[i] = depv[i].ptr;
        private[i] = depv[P+i].ptr;
    }

    for(i=0;i<P;i++) {
//launch P p2p EDTs
        ocrEdtCreate(&(p2pEdt[i]), p2pTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        private[i]->mynode = i;
        private[i]->timestep = 0;
        private[i]->phase = 0;
        private[i]->template = p2pTemplate;
        ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, true);
//PRINTF("RM i %d sticky %d \n", i, sticky);
        private[i]->sendrightevent = sticky;
    }
        ((private0_t *) private[0])->leftrecvevent = private[P-1]->sendrightevent;

    for(i=0;i<P;i++) {
        j = P-i-1;
        ocrAddDependence(depv[j].guid, p2pEdt[j], 0, DB_MODE_RW);
        if(j != 0) {
            ocrAddDependence(private[j-1]->sendrightevent, p2pEdt[j], 2, DB_MODE_RW);
            private[j]->leftold = private[j-1]->sendrightevent;
          }else{
            private[0]->leftold == NULL_GUID;
            ocrAddDependence(depv[2*P].guid, p2pEdt[0], 2, DB_MODE_RW);
        }
        ocrDbRelease(depv[P+j].guid);
        ocrAddDependence(depv[P+j].guid, p2pEdt[j], 1, DB_MODE_RW);
    }
    return NULL_GUID;
}

ocrGuid_t mainEdt(){
//mainEdt is executed first
//Creates the datablocks
//creates realmain
//launches realmian
    u64 i;
    u64 *dummy;
    PRINTF("p2p \nnumber of workers = %d \nrows on each worker = %d \ncolumns on each worker %d\nrow blocking %d\nnumber of timesteps = %d \n", P, N, K, B, T);
    ocrGuid_t realmain, realmainTemplate, dataDb[P], privateDb[P], bufferDb;
    ocrDbCreate(&(dataDb[0]), (void**) &dummy, K*N*sizeof(double), 0, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&(privateDb[0]), (void**) &dummy, sizeof(private0_t), 0, NULL_GUID, NO_ALLOC);
    for(i=1;i<P;i++) {
        ocrDbCreate(&(dataDb[i]), (void**) &dummy, K*N*sizeof(double), 0, NULL_GUID, NO_ALLOC);
        ocrDbCreate(&(privateDb[i]), (void**) &dummy, sizeof(private_t), 0, NULL_GUID, NO_ALLOC);
    }
    ocrDbCreate(&bufferDb, (void**) &dummy, sizeof(buffer_t), 0, NULL_GUID, NO_ALLOC);
//for(i=0;i<P;i++) PRINTF("M i %d DATA %d private %d \n", i, dataDb[i], privateDb[i]);
//PRINTF("M buffer %d \n", bufferDb);
//create realmain
    ocrEdtTemplateCreate(&realmainTemplate, realmainTask, 0, 2*P+1);
    ocrEdtCreate(&realmain, realmainTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
          EDT_PROP_FINISH, NULL_GUID, NULL_GUID);
//launch realmain
    for(i=0;i<P;i++) ocrAddDependence(dataDb[i], realmain, i, DB_MODE_RW);
    for(i=0;i<P;i++) ocrAddDependence(privateDb[i], realmain, P+i, DB_MODE_RW);
    ocrAddDependence(bufferDb, realmain, 2*P, DB_MODE_RW);
//PRINTF("done in main \n");
    return NULL_GUID;
}
