/*
 Author: David S Scott
 Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
an OCR "library" for computing global sums, including a parallel initialization
see README for more documentation
*/

#include <ocr.h>
#include <stdio.h>
#include "ocrGS.h"
ocrGuid_t GSxEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {
/*
note: ARITY is define as 2 in ocrGS.h but can be changed to another value and recompile
paramv[0]
mynode

depv[2+ARITY]
0: GSsharedBlock
1: mydata: myGSblock
2 to ARITY+1: receive blocks (NULL_GUID to start with by real blocks if cloned)
This routine computes a global sum using a ARITY tree
The "user" routines paste their local value into mydata and then launch their copy of GSxEDT with
GSsharedblock, mydata, and ARITY NULL_GUIDs
GSxEDT then determines whether to "send" immediately or clone itself to receive.
Node 0 returns the global value in GSsharedBlock by satisfying rootEvent
*/
    ocrGuid_t gsEdt;
    ocrGuid_t once, sticky, mydest;
    u64 i;
    u64 mynode = paramv[0];
    GSsharedBlock_t * SB = depv[0].ptr;
    gsBlock_t * mydata = depv[1].ptr;
    gsBlock_t * yourdata;
//check whether need to receive by cloning
    if((mydata->event[1] != NULL_GUID) && (depv[2].guid == NULL_GUID)) {
        ocrEdtCreate(&gsEdt, SB->GSxTemplate, EDT_PARAM_DEF, &mynode, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        ocrAddDependence(depv[0].guid, gsEdt, 0 , DB_MODE_RW);
        ocrAddDependence(depv[1].guid, gsEdt, 1 , DB_MODE_RW);
        for(i=0;i<ARITY;i++) {
            ocrAddDependence(mydata->event[i+1], gsEdt, i+2 , DB_MODE_RW);
        }
        return NULL_GUID;
    }
//I have received
    for(i=0;i<ARITY;i++) {
        if(mydata->event[i+1] == NULL_GUID) break;
        ocrEventDestroy(mydata->event[i+1]);
        yourdata = depv[i+2].ptr;
        mydata->event[i+1] = yourdata->event[0];
        mydata->data += yourdata->data;
    }
//send
    if(mynode == 0) {
        mydest = SB->rootEvent;
        ocrEventCreate(&once, OCR_EVENT_ONCE_T, true);
        SB->rootEvent = once;
        SB->sum = mydata->data;
        ocrDbRelease(depv[0].guid);
        ocrEventSatisfy(mydest, depv[0].guid);
        return NULL_GUID;
    }
    mydest = mydata->event[0];
    ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, true);
    mydata->event[0] = sticky;
    ocrDbRelease(depv[1].guid);
    ocrEventSatisfy(mydest, depv[1].guid);
    return NULL_GUID;
}

ocrGuid_t GSiEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
paramv:
0: mynode
1: mysend event
2: Event to satisfy to provide the shared block to the computeInitEdts
depv:
0: GSsharedBlock
1: my gsBlock
Implements an ARITY tree creating and initializing gsBlocks for later use by GSxEdts
The user launches a single instance of GSiEDT with paramv 1 and 2 set to NULL_GUID
GSsharedBlock must have fields GSiTemplate and numnodes initialized
GSsharedblock field userBlock must to set to the guid of a datablock that the user will use
in ComputeInitEdt (or NULL_GUID)
gsBlock can be uninitialized
GSiEDT creates a tree of numnodes GSiEDTs each with its own "mynode" and initialized gsBlock
It then returns to the user by launching numnodes copies of computeInitEdt each with userBlock,
GSsharedBlock and private (initialized) gsBlock and rank number as a parameter.
*/
    u64 mynode = paramv[0];
    ocrGuid_t mysend = paramv[1];
    ocrGuid_t rootEvent = paramv[2];
    GSsharedBlock_t * SB = depv[0].ptr;
    gsBlock_t * myGSblock = depv[1].ptr;
    u64 numnodes = SB->numnodes;
    ocrGuid_t gsBlock, GSi, compute, mydest, once, sticky, dummy;
    ocrGuid_t GSiTemplate = SB->GSiTemplate;
    u64 yournode, paramvout[3], i;
    myGSblock->data = 0.0;
    for(i=1;i<ARITY+1;i++) myGSblock->event[i] = NULL_GUID;
//special send event for node 0
    if(mynode == 0){
        ocrEdtTemplateCreate(&(SB->GSxTemplate), GSxEdt, 1, ARITY+2);
        ocrEdtTemplateCreate(&(SB->computeInitTemplate), computeInitEdt, 1, 3);
        ocrEventCreate(&rootEvent, OCR_EVENT_STICKY_T, true);
        paramv[2] = rootEvent;
        ocrEventCreate(&once, OCR_EVENT_ONCE_T, true);
        SB->rootEvent = once;
    }
//create my compute event
    ocrEdtCreate(&compute, SB->computeInitTemplate, EDT_PARAM_DEF, &mynode, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrAddDependence(SB->userBlock, compute, 0, DB_MODE_RW);
    ocrDbRelease(depv[0].guid);
    ocrAddDependence(depv[0].guid, compute, 1, DB_MODE_RW);
//create and launch my children
    myGSblock->event[0] = mysend;
    paramvout[2] = paramv[2];
    for(i=1;i<=ARITY;i++){
        yournode = ARITY*mynode+i;
        if(yournode  >= numnodes) break; //done
        paramvout[0] = yournode;
        ocrEventCreate(&sticky, OCR_EVENT_STICKY_T, true);
        myGSblock->event[i] = sticky;
        paramvout[1] = sticky;
        ocrDbCreate(&(gsBlock), (void**) &dummy, sizeof(gsBlock_t), 0, NULL_GUID, NO_ALLOC);
        ocrEdtCreate(&GSi, GSiTemplate, EDT_PARAM_DEF, paramvout, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        ocrAddDependence(depv[0].guid, GSi, 0, DB_MODE_RW);
        ocrAddDependence(gsBlock, GSi, 1, DB_MODE_RW);
    }
//launch my compute event
    ocrDbRelease(depv[1].guid);
    ocrAddDependence(depv[1].guid, compute, 2, DB_MODE_RW);
    return NULL_GUID;
}
