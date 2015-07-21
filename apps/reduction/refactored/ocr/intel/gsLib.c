/*
 Author: David S Scott
 Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
an OCR "library" for computing global reductions using labeled GUIDs.

This library can be called by a set of "numnodes" compute EDTs to compute a global reduction and return
result to the clones of the compute EDTs.
The EDTs must be numbered from 0 to numnodes-1

Before launching reduceEdt the user MUST do the following (see the test.c driver for an example):

at the beginning of time:

create a reductionFunction(u64 n, void * a, void *b) which computes a = a op b for the n
   elements in each vector
create numnodes-1 labeled GUIDs for STICKY events (used internally by reduceEDT)
create 2 label GUIDs for ONCE events
create the template for reduceEDT
create numnodes+1 datablocks which hold n elements.  Each EDT has a private copy to hold its data
and the last one is used to return the results of the reduction to all of the clones


to launch reduceEdt, each compute EDT does:

create the local data to be reduced in its private datablock
create a clone
create a once event using one of the two reserved once GUIDs (alternate between iterations)
make the clone depend on the once Event
attach the private datablock to the clone

create a copy of reduceEDT with the correct parameter set
attach your private datablock
if(node 0) attach the special block to return the answer

There are 8 parameters for reduceEdt:
0: mynode
1: numnodes
2: template
3: number of elements in the vector
4: size of each element
5: stickyGuidRange
6: onceEvent
7: phase     (always zero when calling)

There are 1 or 2 dependencies:
0: my local block
1: return block  (only node 0)

ReduceEDts work together to compute the reduction using an ARITY tree.  ARITY is set to 2
(below) implementing a binary tree but it should work correctly with other positive values.
The larger the value of ARITY, the less parallelism but the fewer number of EDTs created.

The stickyGuids are used to create STICKY events for sending datablocks from one EDT to
another.  The STICKY events are destroyed by the receiver so that the GUIDs can be
reused in a later iteration.


*/

#define ARITY 2   //could be changed and recompiled, no visible change

#define ENABLE_EXTENSION_LABELING
#include "ocr.h"
#include "extensions/ocr-labeling.h"
#include "string.h"
#include "stdio.h"
//#include "ocr-std.h"
//#include "stdlib.h"

void reductionFunction(u64 n, void * a, void * b);

ocrGuid_t reduceEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {
/*
paramv[6]
0: mynode
1: numnodes
2: template
3: n   number of elements in reduction vector (often 1)
4: size of element of array (for copying);
5: stickyGuidRange
6: onceEvent (used by node 0 to return to compute)
7: phase  (0 for user call, 1 to mark clones)


depv[1] or 2
0: my datablock (of length n)
1: datablock to return (node 0 only)
*/

    u64 mynode = paramv[0];
    u64 numnodes = paramv[1];
    ocrGuid_t template = (ocrGuid_t) paramv[2];
    u64 n = paramv[3];
    u64 size = paramv[4];
    ocrGuid_t rangeGuid = (ocrGuid_t) paramv[5];
    ocrGuid_t onceEvent = paramv[6];
    u64 phase = paramv[7];

    void * mydata = (void *) depv[0].ptr;

//printf("M%d P%d start depv0 %lx \n", mynode, phase, depv[0].guid);
//if(depc > 1) printf("M%d P%d start depv1 %lx \n", mynode, phase, depv[1].guid);
//if(depc > 2) printf("M%d P%d start depv2 %lx \n", mynode, phase, depv[2].guid);
//if(depc > 3) printf("M%d P%d start depv3 %lx \n", mynode, phase, depv[3].guid);


    void * yourdata;
    ocrGuid_t reduceEdt;
    ocrGuid_t srcEvent, destEvent, tempEvent;
    u64 errno, i, src, dest, nrecv, ndep;

//check whether need to receive by cloning
    if((phase == 0) && (mynode*ARITY+1 < numnodes)) {
        paramv[7] = 1; //change phase
        nrecv = ARITY;
        if(nrecv > numnodes - mynode*ARITY-1) nrecv = numnodes - mynode*ARITY -1;
        ndep = nrecv+1;
        if(mynode == 0) ndep++;
//printf("M%d P%d clone with ndep %d \n", mynode, phase, ndep);
        ocrEdtCreate(&reduceEdt, template, EDT_PARAM_DEF, paramv, ndep, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        ocrAddDependence(depv[0].guid, reduceEdt, 0 , DB_MODE_RW);
        for(i=0;i<ARITY;i++) {
            src = mynode*ARITY+i+1;
            if(src >= numnodes) break;
            ocrGuidFromIndex(&srcEvent, rangeGuid, src-1);
            tempEvent = srcEvent;
            errno = ocrEventCreate(&tempEvent, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | EVT_PROP_TAKES_ARG);
printf("M%d P%d event guid %lx errno %d \n", mynode, phase, srcEvent, errno);
            ocrAddDependence(srcEvent, reduceEdt, i+1, DB_MODE_RO);
        }
        if(mynode == 0) ocrAddDependence(depv[1].guid, reduceEdt, ndep-1, DB_MODE_RW);
        return NULL_GUID;
    }

//I have received
    if(phase == 1) {
//printf("M%d P%d received \n", mynode, phase);
        for(i=0;i<ARITY;i++) {
            src = mynode*ARITY+i+1;
            if(src >= numnodes) break;
            ocrGuidFromIndex(&srcEvent, rangeGuid, src-1);
//printf("M%d P%d destroy %lx\n", mynode, phase, srcEvent);

            ocrEventDestroy(srcEvent);
            yourdata = (void *) depv[i+1].ptr;
//printf("M%d P%d before function mine %lx yours %lx\n", mynode, phase, mydata, yourdata);
            reductionFunction(n, mydata, yourdata);
        }
    }

//send
    if(mynode == 0) {
        nrecv = ARITY;
        if(nrecv > numnodes - mynode*ARITY-1) nrecv = numnodes - mynode*ARITY -1;
//printf("M0 P%d nrecv %d\n", phase, nrecv);
        memcpy(depv[nrecv+1].ptr, mydata, n*size);
        ocrDbRelease(depv[nrecv+1].guid);

//printf("M%d P%d satisfy Guid %lx with block %lx\n", mynode, phase, onceEvent, depv[nrecv+1].guid);
        ocrEventSatisfy(onceEvent, depv[nrecv+1].guid);
        return NULL_GUID;
    }
    ocrGuidFromIndex(&destEvent, rangeGuid, mynode-1);
//printf("M%d P%d send\n",mynode, phase);
    tempEvent = destEvent;
    errno = ocrEventCreate(&tempEvent, OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | EVT_PROP_TAKES_ARG);
printf("M%d P%d event guid %lx errno %d \n", mynode, phase, destEvent, errno);
    ocrDbRelease(depv[0].guid);
//printf("M%d P%d satisfy Guid %lx with %lx \n", mynode, phase, destEvent, depv[0].guid);
    ocrEventSatisfy(destEvent, depv[0].guid);
    return NULL_GUID;
}

