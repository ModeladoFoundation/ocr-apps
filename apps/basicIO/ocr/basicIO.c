#include "ocr.h"
#include <stdlib.h>
#include <stdio.h>

ocrGuid_t add_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

    u32 offset = paramv[0];
    u32 size = paramv[1];
    u64 * data = depv[0].ptr;
    u64 i;
    for(i=0;i<size;i++)
        data[i]+=offset;
    FILE *out = fopen("basicIO_output.txt","w");

#ifndef TG_ARCH
    for(i=0;i<size;i++)
        fprintf(out,"%llu\n",data[i]);
#else
    fwrite(data,10,sizeof(u64),out);
#endif
    fclose(out);

    ocrShutdown(); // This is the last EDT to execute
    return NULL_GUID;
}


ocrGuid_t mainEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 size = -1;
    u64 offset = -1;
    u32 i = 0;
    u64 argc;

    void *programArg = depv[0].ptr;
    argc = getArgc(programArg);
    if ( argc !=  4 ) {
        PRINTF("Usage: ./basicIO offset size fileName \n");
        ocrShutdown();
        return 1;
    }

    offset = atoi(getArgv(programArg, 1));
    size = atoi(getArgv(programArg, 2));
    u64 nparamv[2];
    nparamv[0] = offset;
    nparamv[1] = size;
    FILE *in;
    in = fopen(getArgv(programArg, 3), "r");
    if( !in ) {
        PRINTF("Cannot find file: %s\n", getArgv(programArg, 3));
        ocrShutdown();
        return NULL_GUID;
    }
    ocrGuid_t dataGuid;
    // Data can be passed as parameter also , there was no
    // necessary need of creating data block  in this example.
    // Its has been created for demo purpose
    //Create datablock to hold a block of 'size' elements
    u64 *inputarr;
    ocrDbCreate(&dataGuid, (void**)&inputarr, sizeof(u64)*size,0,NULL_GUID, NO_ALLOC);
#ifndef TG_ARCH
    while(fscanf(in,"%llu\n",&inputarr[i++])!=EOF);
#else
    fread(inputarr, sizeof(u64),size , in);

#endif
    fclose(in);

    ocrGuid_t addEdtTemplateGuid;
    ocrEdtTemplateCreate(&addEdtTemplateGuid, add_edt, 2 /*paramc*/, 1 /*depc*/);
    ocrGuid_t add_edt_guid;
    // Create the EDT not specifying the dependence vector at creation
    ocrEdtCreate(&add_edt_guid, addEdtTemplateGuid, EDT_PARAM_DEF, nparamv,1,NULL ,EDT_PROP_FINISH , NULL_GUID, NULL);

    ocrGuid_t triggerEventGuid;
    ocrEventCreate(&triggerEventGuid, OCR_EVENT_STICKY_T, true);

    ocrAddDependence(triggerEventGuid, add_edt_guid, 0, DB_MODE_EW);
    ocrEventSatisfy(triggerEventGuid, dataGuid);
    return NULL_GUID;
}
