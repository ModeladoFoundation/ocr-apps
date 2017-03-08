/*
 *  This file is subject to the license agreement located in the file LICENSE
 *  and cannot be distributed without it. This notice cannot be
 *  removed or modified.
 */

/* Example of a "fork-join" pattern in OCR
 *
 * Implements the following dependence graph:
 *
 *             mainEdt
 *          /           \
 *         /             \
 *        /               \
 *    resEdt00           resEdt01
 *       /|\                /|\
 *locEdt1 | locEdt2  locEdt3 | locEdt4
 *       \|/                \|/
 *    resEdt10           resEdt11
 *       /|\                /|\
 *locEdt5 | locEdt6  locEdt7 | locEdt8
 *       \|/                \|/
 *        \                 /
 *         \               /
 *          \             /
 *            shutdownEdt
 *
 */

#include "ocr.h"

ocrGuid_t localFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("Hello from local EDT (%d, %d, %d)\n",paramv[0], paramv[1], paramv[2]);
    return NULL_GUID;
}

ocrGuid_t resilientFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("Hello from resilient EDT (%d, %d)\n",paramv[0], paramv[1]);
    ocrGuid_t localEdt_template;
    ocrGuid_t localEdt1, localEdt2;
    ocrEdtTemplateCreate(&localEdt_template, localFunc, 3, 0);
    u64 params[3];
    params[0] = paramv[0];
    params[1] = paramv[1];
    params[2] = 0;
    ocrEdtCreate(&localEdt1, localEdt_template, 3, params, 0, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
    params[2] = 1;
    ocrEdtCreate(&localEdt2, localEdt_template, 3, params, 0, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
    return NULL_GUID;
}

ocrGuid_t shutdownFunc(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("Hello from shutdownEdt\n");
    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("Starting mainEdt\n");
    ocrGuid_t resEdt_template, shutdown_template;
    ocrGuid_t predEdt1, predEdt2, succEdt1, succEdt2, shutdownEdt;
    ocrGuid_t outputResEvent1, outputResEvent2, outputSuccEvent1, outputSuccEvent2;

    //Create templates for the EDTs
    ocrEdtTemplateCreate(&resEdt_template, resilientFunc, 2, 1);
    ocrEdtTemplateCreate(&shutdown_template, shutdownFunc, 0, 2);


    //Create the EDTs
    u64 params[2];
    params[0] = 0; params[1] = 0;
    ocrEdtCreate(&predEdt1, resEdt_template, 2, params, EDT_PARAM_DEF, NULL, EDT_PROP_RESILIENT, NULL_HINT, &outputResEvent1);

    params[0] = 0; params[1] = 1;
    ocrEdtCreate(&predEdt2, resEdt_template, 2, params, EDT_PARAM_DEF, NULL, EDT_PROP_RESILIENT, NULL_HINT, &outputResEvent2);

    params[0] = 1; params[1] = 0;
    ocrEdtCreate(&succEdt1, resEdt_template, 2, params, EDT_PARAM_DEF, NULL, EDT_PROP_RESILIENT, NULL_HINT, &outputSuccEvent1);

    params[0] = 1; params[1] = 1;
    ocrEdtCreate(&succEdt2, resEdt_template, 2, params, EDT_PARAM_DEF, NULL, EDT_PROP_RESILIENT, NULL_HINT, &outputSuccEvent2);

    ocrEdtCreate(&shutdownEdt, shutdown_template, 0, NULL, 2, NULL, EDT_PROP_NONE, NULL_HINT, NULL);

    //Setup dependences for the shutdown EDT
    ocrAddDependence(outputSuccEvent1, shutdownEdt, 0, DB_MODE_CONST);
    ocrAddDependence(outputSuccEvent2, shutdownEdt, 1, DB_MODE_CONST);

    //Setup dependences for the successor EDTs
    ocrAddDependence(outputResEvent1, succEdt1, 0, DB_MODE_CONST);
    ocrAddDependence(outputResEvent2, succEdt2, 0, DB_MODE_CONST);

    //Start execution of the parallel resilient EDTs
    ocrAddDependence(NULL_GUID, predEdt1, 0, DB_DEFAULT_MODE);
    ocrAddDependence(NULL_GUID, predEdt2, 0, DB_DEFAULT_MODE);
    return NULL_GUID;
}
