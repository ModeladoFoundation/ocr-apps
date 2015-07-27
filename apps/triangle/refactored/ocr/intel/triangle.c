/*
Author: David Scott
Copywrite Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.

*/

/*
This code implements a recursive search of the game tree of the "14 peg puzzle" in OCR
to count the number of solutions.

See the README file for more information.

*/

#include <ocr.h>
#define BOARDSIZE 15
#define MOVESIZE 36

#define BOTTOM 13



/*
void printboard(u64 board[15]) {
    PRINTF("board\n");
    PRINTF("          %3d \n", board[0]);
    PRINTF("         %3d %3d \n", board[1], board[2]);
    PRINTF("        %3d %3d %3d \n", board[3], board[4], board[5]);
    PRINTF("       %3d %3d %3d %3d \n", board[6], board[7], board[8], board[9]);
    PRINTF("      %3d %3d %3d %3d %3d\n", board[10], board[11], board[12], board[13], board[14]);
    return ;
}
*/

ocrGuid_t incrementTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 * counter = depv[0].ptr;
//PRINTF("in increment with %d\n", *counter);
//    if(*counter == 0) PRINTF("found first solution...wait for final count\n");
    (*counter)++;
//if(*counter %100 == 1) PRINTF("in increment with %d\n", *counter);
//PRINTF("in increment with %d\n", *counter);
    return NULL_GUID;
}
ocrGuid_t triangleTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
/*
paramv
0: nummoves
1: oldmove
2: triangleTemplate
3: incrementTemplate
4: counterDBguid
depv
0: oldboard
1: board
2: moveblock
make move
check for bottom
look for legal moves
*/

    u64 nummoves = paramv[0];
    u64 oldmove = paramv[1];
    ocrGuid_t triangleTemplate = paramv[2];
    ocrGuid_t incrementTemplate = paramv[3];
    ocrGuid_t counterDb = paramv[4];
    u64 * oldboard = depv[0].ptr;
    u64 * board = depv[1].ptr;
    u64 * pmoves = depv[2].ptr;
    ocrGuid_t newboardDb;
    ocrGuid_t triangleEdt, once;
    u64 i, j;
    u64 *newboard;
//PRINTF("starting Triangle with nummoves %d oldmove %d \n", nummoves, oldmove);
    for(i=0;i<BOARDSIZE;i++) board[i] = oldboard[i];
    if(oldmove != -1){
        nummoves++;
        board[pmoves[3*oldmove]] = 0;
        board[pmoves[3*oldmove+1]] = 0;
        board[pmoves[3*oldmove+2]] = 1;
    }
//printboard(board);
    if(nummoves == BOTTOM){
//PRINTF("nummoves == 13 !!\n");
/*
        if(*count==0) {
            PRINTF("board numbering\n");
            PRINTF("  0  1  2  3  4\n");
            PRINTF("  5  6  7  8\n");
            PRINTF("  9 10  11\n");
            PRINTF(" 12 13\n");
            PRINTF(" 14\n");

            PRINTF("1st solution\n");
            for(i=0;i<13;i++){
                PRINTF("%2d %2d %2d\n", rmoves[i][0], rmoves[i][1], rmoves[i][2]);
            }
            printboard(board);
        }
*/
        ocrGuid_t incrementEdt;
        ocrEdtCreate(&incrementEdt, incrementTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);
        ocrAddDependence(counterDb, incrementEdt, 0, DB_MODE_EW);
        return NULL_GUID;
    } else {
        u64 triangleParamv[5] = {nummoves, 0, triangleTemplate, incrementTemplate, counterDb};
        ocrEventCreate(&once, OCR_EVENT_ONCE_T, true);
        for(i=0;i<MOVESIZE;i++) {
            if(board[pmoves[3*i]] && board[pmoves[3*i+1]] && (!board[pmoves[3*i+2]])) { //legal move
                ocrDbCreate(&newboardDb, (void**) &newboard, sizeof(u64)*BOARDSIZE, 0, NULL_GUID, NO_ALLOC);
                triangleParamv[1] = i;
                ocrEdtCreate(&triangleEdt, triangleTemplate, EDT_PARAM_DEF, triangleParamv, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);
                ocrAddDependence(once, triangleEdt, 0, DB_MODE_CONST);
                ocrAddDependence(newboardDb, triangleEdt, 1, DB_MODE_RW);
                ocrAddDependence(depv[2].guid, triangleEdt, 2, DB_MODE_CONST);
            }
        }
    }
    ocrDbRelease(depv[1].guid);
    ocrEventSatisfy(once, depv[1].guid);
    return NULL_GUID;
}
//print final count
ocrGuid_t wrapupTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 * count = depv[0].ptr;
    double starttime = paramv[0];
    if(*count == 29760) PRINTF("PASS  final count %d \n", *count);
        else PRINTF("FAIL final count %d should be 29760 \n", *count);

    ocrShutdown();
    return NULL_GUID;
}
ocrGuid_t realmainTask(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]){
/*
params: none
depv
0: counter
1: oldboard
2: board
3: move block
initialize datablocks
create triangleEdt
create and launch wrapup
launch triangleEdt
*/

    u64 count = 0;
    u64 nummoves = 0;
    u64 i, j;
    ocrGuid_t incrementTemplate, triangleTemplate, triangleEdt, triangleOutputEvent;
    ocrGuid_t outEvt;
    u64 oldmove;
//initialize counter
    u64 *counter = depv[0].ptr;
    *counter = 0;
    u64 *oldboard = depv[1].ptr;
    u64 *pmoves = depv[3].ptr;
//initialize pmoves
    u64 ptemp[MOVESIZE][3] ={
        {0,1,3},
        {3,1,0},
        {0,2,5},
        {5,2,0},
        {3,4,5},
        {5,4,3},
        {1,3,6},
        {6,3,1},
        {1,4,8},
        {8,4,1},
        {2,4,7},
        {7,4,2},
        {2,5,9},
        {9,5,2},
        {6,7,8},
        {8,7,6},
        {7,8,9},
        {9,8,7},
        {3,6,10},
        {10,6,3},
        {3,7,12},
        {12,7,3},
        {4,7,11},
        {11,7,4},
        {4,8,13},
        {13,8,4},
        {5,8,12},
        {12,8,5},
        {5,9,14},
        {14,9,5},
        {10,11,12},
        {12,11,10},
        {11,12,13},
        {13,12,11},
        {12,13,14},
        {14,13,12}
        };
    for(i=0;i<MOVESIZE;i++)for(j=0;j<3;j++)  pmoves[3*i+j] = ptemp[i][j];
//initialize oldboard
    u64 btemp[BOARDSIZE] = {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    for(i=0;i<BOARDSIZE;i++) oldboard[i] = btemp[i];
    ocrEdtTemplateCreate(&triangleTemplate, triangleTask, 5, 3);
    ocrEdtTemplateCreate(&incrementTemplate, incrementTask, 0, 1);
    oldmove = -1;
    u64 triangleParamv[5] = {nummoves, oldmove, triangleTemplate, incrementTemplate, depv[0].guid};
//create triangleEdt as a FINISH Edt
    ocrEdtCreate(&triangleEdt, triangleTemplate, EDT_PARAM_DEF, triangleParamv, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &triangleOutputEvent);
//create and launch wrapup
    ocrGuid_t wrapupTemplate;
    ocrGuid_t wrapupEdt;
    ocrEdtTemplateCreate(&wrapupTemplate, wrapupTask, 0, 2);
    ocrEdtCreate(&wrapupEdt, wrapupTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);
    ocrDbRelease(depv[0].guid);
    ocrAddDependence(depv[0].guid, wrapupEdt, 0, DB_MODE_CONST);
    ocrAddDependence(triangleOutputEvent, wrapupEdt, 1, DB_MODE_RW);
//launch triangleEdt
    ocrDbRelease(depv[1].guid);
    ocrAddDependence(depv[1].guid, triangleEdt, 0, DB_MODE_CONST);
    ocrAddDependence(depv[2].guid, triangleEdt, 1, DB_MODE_RW);
    ocrDbRelease(depv[3].guid);
    ocrAddDependence(depv[3].guid, triangleEdt, 2, DB_MODE_CONST);
    return NULL_GUID;
}
ocrGuid_t mainEdt(){
    u64 *counter;
    ocrGuid_t realmain, realmainTemplate, counterDb, boardDb, oldboardDb, pmovesDb;
    u64 *oldboard, *board, *pmoves;
PRINTF("triangle puzzle BOTTOM %d \n", BOTTOM);

    ocrDbCreate(&counterDb, (void**) &counter, sizeof(u64), 0, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&oldboardDb, (void**) &oldboard, sizeof(u64)*BOARDSIZE, 0, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&boardDb, (void**) &board, sizeof(u64)*BOARDSIZE, 0, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&pmovesDb, (void**) &pmoves, sizeof(u64)*MOVESIZE*3, 0, NULL_GUID, NO_ALLOC);
    ocrEdtTemplateCreate(&realmainTemplate, realmainTask, 0, 4);
    ocrEdtCreate(&realmain, realmainTemplate, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, NULL);
    ocrAddDependence(counterDb, realmain, 0, DB_MODE_RW);
    ocrAddDependence(oldboardDb, realmain, 1, DB_MODE_RW);
    ocrAddDependence(boardDb, realmain, 2, DB_MODE_RW);
    ocrAddDependence(pmovesDb, realmain, 3, DB_MODE_CONST);
    return NULL_GUID;
}
