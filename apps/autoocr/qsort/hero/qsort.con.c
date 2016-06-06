// Raw program with the __data keyword
// and __attribute__ that specifies where the datablock can be placed
// Similarly quickSort is decorated with __task keyword
//
// hand converted to non-blocking
#include <stdio.h>
#include <stdlib.h>

#define LEN 128

#define __task __attribute__((ocrtask))
#define __data __attribute__((ocrdata))

#include <stdlib.h>
#include "ocr.h"
#include "alloca.h"

/*
 * forward declarations
 */
ocrGuid_t done_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]);

ocrGuid_t finish_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]);
ocrGuid_t quickSort_launch(ocrGuid_t a, int l, int r );
/*
 * The "continuation" EDT for whatever follows the task call
 * in mainEdt() that depends on that task.
 */
ocrGuid_t done_template() {
    static ocrGuid_t doneGuid;
    ocrEdtTemplateCreate(&doneGuid, &done_edt, 0, 2);
    return doneGuid;
}
ocrGuid_t done_launch(ocrGuid_t returnEvent, ocrGuid_t dbGuid) {
    ocrGuid_t done_edt;
    ocrGuid_t * ocrdeps = (ocrGuid_t *)alloca(sizeof(ocrGuid_t)*2);
    *(ocrGuid_t *) (ocrdeps) = returnEvent;
    *(ocrGuid_t *) (ocrdeps + 1) = dbGuid;
    ocrEdtCreate( &done_edt, done_template(), EDT_PARAM_DEF,
                  NULL, EDT_PARAM_DEF, ocrdeps, EDT_PROP_NONE,
                  NULL_HINT, NULL);

    return NULL_GUID;
}
ocrGuid_t done_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]) {
    int *a = (int *)(ocrdepv[1].ptr);
    printf("After\n");
    int i;
    for (i = 0; i < 128; i++)
        printf("%02x ", a[i]);
    printf("\n");
    ocrShutdown();
    return NULL_GUID;
}
 /*
 * triggers when the pair of qsort edts have finished and triggered
 * thier postslots (returnEvent). Those events are connected to the preslots
 * here
 */
ocrGuid_t finish_template() {
    static ocrGuid_t templateGuid;

    ocrEdtTemplateCreate(&templateGuid, &finish_edt, 1, 2);
    return templateGuid;
}

ocrGuid_t finish_launch(ocrGuid_t evt1, ocrGuid_t evt2, ocrGuid_t returnEvent) {
    //
    // Create a parameter and dependency array
    //
    u64 *ocrparams = (u64 *) alloca(sizeof(u64)*1);
    ocrGuid_t *ocrdeps = (ocrGuid_t *) alloca(sizeof(ocrGuid_t)*2);
    //
    // Marshall parameters
    //
    *(ocrGuid_t *) (ocrparams) = returnEvent;
    *ocrdeps = evt1;;
    *(ocrdeps+1) = evt2;;


    ocrGuid_t finish_edt;

    //
    // Create the EDT task
    //
    ocrEdtCreate( &finish_edt, finish_template(), EDT_PARAM_DEF,
                  ocrparams, EDT_PARAM_DEF, ocrdeps, EDT_PROP_NONE,
                  NULL_HINT, NULL);

    return NULL_GUID;
}
/*
 * triggered by post-slots from two quickSort_edt()'s.
 * satisfy the return event of the parent quickSort_edt
 */
ocrGuid_t finish_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]) {
    /* Get event to satisfy */
    ocrGuid_t outEvt = *(ocrGuid_t*)(ocrparams);
    ocrEventSatisfy(outEvt, NULL_GUID);
    return NULL_GUID;
}



int partition(int * a, int l, int r) {
    int pivot, i, j, t;
    pivot = a[l];
    i = l;
    j = r + 1;
    while (1)
        {
            do
                ++i;
            while (a[i] <= pivot && i <= r);
            do
                --j;
            while (a[j] > pivot);
            if (i >= j)
                break;
            t = a[i];
            a[i] = a[j];
            a[j] = t;
        }
    t = a[l];
    a[l] = a[j];
    a[j] = t;
    return j;
 }

ocrGuid_t quickSort_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]);

ocrGuid_t quickSort_template() {
    static ocrGuid_t templateGuid;

    ocrEdtTemplateCreate(&templateGuid, &quickSort_edt, 3, 1);
    return templateGuid;
}

ocrGuid_t quickSort_launch(ocrGuid_t a, int l, int r ) {
    //
    // Create a parameter and dependency array
    //
    u64 *ocrparams = (u64 *) alloca(sizeof(u64)*3);
    ocrGuid_t *ocrdeps = (ocrGuid_t *) alloca(sizeof(ocrGuid_t)*1);
    //
    // Create an event to synchronize completion with
    //
    ocrGuid_t returnEvent;
    ocrEventCreate(&returnEvent, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
    *(ocrGuid_t *) ocrparams = returnEvent;
    //
    // Marshall parameters
    //
    ocrdeps[0] = a;
    *(int *)(ocrparams + 1) = l;
    *(int *)(ocrparams + 2) = r;


    ocrGuid_t edtGuid;

    //
    // Create the EDT task
    //
    ocrEdtCreate( &edtGuid, quickSort_template(), EDT_PARAM_DEF,
                  ocrparams, EDT_PARAM_DEF, ocrdeps, EDT_PROP_NONE,
                  NULL_HINT, NULL);

    return returnEvent;
}

ocrGuid_t quickSort_edt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]) {
    /* Get event to satisfy */
    ocrGuid_t ocrOutEvt = *(ocrGuid_t *)ocrparams;
    int * a = (int *)ocrdepv[0].ptr;
    ocrGuid_t a_dbGuid = ocrdepv[0].guid;
    int l = *(int *)(ocrparams + 1);
    int r = *(int *)(ocrparams + 2);
    int j;
    if (l < r) {
        j = partition(a, l, r);
        ocrGuid_t ocrCallEvt0 = quickSort_launch(a_dbGuid, l, j - 1);
        ocrGuid_t ocrCallEvt1 = quickSort_launch(a_dbGuid, j + 1, r);

       // nothing happens until finish_edt fires ...
       // then satisfy the ocrOutEvt.. triggering the next quickSort_edt..

       ocrDbRelease(a_dbGuid);
       finish_launch(ocrCallEvt0, ocrCallEvt1, ocrOutEvt);
       return NULL_GUID;
    } else {

// nothing to launch

        ocrDbRelease(a_dbGuid);
        ocrEventSatisfy(ocrOutEvt, NULL_GUID);
    }

    return NULL_GUID;
 }

int * main_a_dballoc( ocrGuid_t * guid  ) {
    int * retVal;
    ocrDbCreate( guid, (void **)&retVal, 512,
                 DB_PROP_NONE, NULL_HINT, NO_ALLOC);

    return retVal;
}

ocrGuid_t mainEdt(u32 ocrparamc, u64* ocrparams,
                      u32 ocrdepc, ocrEdtDep_t ocrdepv[]) {

    ocrGuid_t a_dbGuid;
    int *a = main_a_dballoc(&a_dbGuid);
    int i;
    for (i = 0; i < 128; i++)
        a[i] = rand();
    printf("Before\n");
    for (i = 0; i < 128; i++)
        printf("%02x ", a[i]);
    printf("\n");
    ocrGuid_t ocrCallEvt0 = quickSort_launch(a_dbGuid, 0, 128 - 1 );
    // essentially a continuation of a non-task ..?? cant do that
    // here I convert whatever follows the task call into
    // another task whos pre slot is linked to the main task's postslot
    done_launch(ocrCallEvt0, a_dbGuid);
    return NULL_GUID;
 }
