/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

// OCR implementation of the Cooley-Tukey algorithm. Same as
// naive-parallel.c, but recursive creation of StartEDTs stops once
// the matrix size reaches serialBlockSize. For these small matrices ditfft2
// is called to compute the answer serially. This is meant to minimize the overhead
// of creating EDTs while still maximizing parallelism.
//
// EndEDTs are also changed to divide their work to a number of slave EDTs, such that
// each slave handles serialBlockSize elements.
//

#define _USE_MATH_DEFINES

#include <stack>

#include "options.h"
#include "verify.h"

#include "ocr.h"

#define SERIAL_BLOCK_SIZE_DEFAULT (1024*16)

// Performs one entire iteration of FFT.
// These are meant to be chained serially for timing and testing.
ocrGuid_t fftIterationEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    ocrGuid_t startTempGuid = paramv[0];
    ocrGuid_t endTempGuid = paramv[1];
    ocrGuid_t endSlaveTempGuid = paramv[2];
    u64 N = paramv[3];
    bool verbose = paramv[4];
    u64 serialBlockSize = paramv[5];
    if(verbose) {
        PRINTF("Creating iteration child\n");
    }

    ocrGuid_t dependencies[1] = { depv[0].guid };
    u64 edtParamv[9] = { startTempGuid, endTempGuid, endSlaveTempGuid, N,
                         1 /* step size */, 0 /* offset */, 0 /* x_in_offset */,
                         verbose, serialBlockSize };

    ocrGuid_t edtGuid;
    ocrEdtCreate(&edtGuid, startTempGuid, EDT_PARAM_DEF, edtParamv, EDT_PARAM_DEF,
                 dependencies, EDT_PROP_FINISH, NULL_GUID, NULL_GUID);

    return NULL_GUID;
}

ocrGuid_t fftStartEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    int i;
    ocrGuid_t startGuid = paramv[0];
    ocrGuid_t endGuid = paramv[1];
    ocrGuid_t endSlaveGuid = paramv[2];
    float *data = (float*)depv[0].ptr;
    ocrGuid_t dataGuid = depv[0].guid;
    u64 N = paramv[3];
    u64 step = paramv[4];
    u64 offset = paramv[5];
    u64 x_in_offset = paramv[6];
    bool verbose = paramv[7];
    u64 serialBlockSize = paramv[8];
    float *x_in = (float*)data;
    float *X_real = (float*)(data+offset + N*step);
    float *X_imag = (float*)(data+offset + 2*N*step);
    if(verbose) {
        PRINTF("Step %d offset: %d N*step: %d\n", step, offset, N*step);
    }

    if(N <= serialBlockSize) {
        ditfft2(X_real, X_imag, x_in+x_in_offset, N, step);
    } else {
        // DFT even side
        u64 childParamv[9] = { startGuid, endGuid, endSlaveGuid, N/2, 2 * step,
                               0 + offset, x_in_offset, verbose, serialBlockSize };
        u64 childParamv2[9] = { startGuid, endGuid, endSlaveGuid, N/2, 2 * step,
                                N/2 + offset, x_in_offset + step, verbose, serialBlockSize };

        if(verbose) {
            PRINTF("Creating children of size %d\n",N/2);
        }
        ocrGuid_t edtGuid, edtGuid2, endEdtGuid, finishEventGuid, finishEventGuid2;

        ocrEdtCreate(&edtGuid, startGuid, EDT_PARAM_DEF, childParamv,
                     EDT_PARAM_DEF, NULL_GUID, EDT_PROP_FINISH, NULL_GUID,
                     &finishEventGuid);
        ocrEdtCreate(&edtGuid2, startGuid, EDT_PARAM_DEF, childParamv2,
                     EDT_PARAM_DEF, NULL_GUID, EDT_PROP_FINISH, NULL_GUID,
                     &finishEventGuid2);
        if(verbose) {
            PRINTF("finishEventGuid after create: %lu\n",finishEventGuid);
        }

        ocrGuid_t endDependencies[3] = { dataGuid, finishEventGuid, finishEventGuid2 };
        // Do calculations after having divided and conquered
        ocrEdtCreate(&endEdtGuid, endGuid, EDT_PARAM_DEF, paramv, EDT_PARAM_DEF,
                     endDependencies, EDT_PROP_FINISH, NULL_GUID, NULL);

        ocrAddDependence(dataGuid, edtGuid, 0, DB_MODE_ITW);
        ocrAddDependence(dataGuid, edtGuid2, 0, DB_MODE_ITW);
    }

    if(verbose) {
        PRINTF("Task with size %d completed\n",N);
    }
    return NULL_GUID;
}

ocrGuid_t fftEndEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    int i;
    ocrGuid_t startGuid = paramv[0];
    ocrGuid_t endGuid = paramv[1];
    ocrGuid_t endSlaveGuid = paramv[2];
    float *data = (float*)depv[0].ptr;
    ocrGuid_t dataGuid = depv[0].guid;
    u64 N = paramv[3];
    u64 step = paramv[4];
    u64 offset = paramv[5];
    bool verbose = paramv[7];
    u64 serialBlockSize = paramv[8];
    float *x_in = (float*)data+offset;
    float *X_real = (float*)(data+offset + N*step);
    float *X_imag = (float*)(data+offset + 2*N*step);
    if(verbose) {
        PRINTF("Reached end phase for step %d\n",step);
    }
    u64 *slaveParamv;

    if(N/2 > serialBlockSize) {
        ocrGuid_t slaveGuids[(N/2)/serialBlockSize];
        u64 slaveParamv[5 * (N/2)/serialBlockSize];

        if(verbose) {
            PRINTF("Creating %d slaves for N=%d\n",(N/2)/serialBlockSize,N);
        }

        for(i=0;i<(N/2)/serialBlockSize;i++) {
            slaveParamv[i*5] = N;
            slaveParamv[i*5+1] = step;
            slaveParamv[i*5+2] = offset;
            slaveParamv[i*5+3] = i*serialBlockSize;
            slaveParamv[i*5+4] = (i+1)*serialBlockSize;

            ocrEdtCreate(slaveGuids+i, endSlaveGuid, EDT_PARAM_DEF,
                         slaveParamv+i*5, EDT_PARAM_DEF, &dataGuid,
                         EDT_PROP_NONE, NULL_GUID, NULL);
        }
    } else {
        ocrGuid_t slaveGuids[1];
        u64 slaveParamv[5];

        slaveParamv[0] = N;
        slaveParamv[1] = step;
        slaveParamv[2] = offset;
        slaveParamv[3] = 0;
        slaveParamv[4] = N/2;

        ocrEdtCreate(slaveGuids, endSlaveGuid, EDT_PARAM_DEF, slaveParamv,
                     EDT_PARAM_DEF, &dataGuid, EDT_PROP_NONE, NULL_GUID, NULL);
    }
    return NULL_GUID;
}
ocrGuid_t fftEndSlaveEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
    int i;
    float *data = (float*)depv[0].ptr;
    ocrGuid_t dataGuid = depv[0].guid;
    u64 N = paramv[0];
    u64 step = paramv[1];
    u64 offset = paramv[2];
    float *x_in = (float*)data+offset;
    float *X_real = (float*)(data+offset + N*step);
    float *X_imag = (float*)(data+offset + 2*N*step);
    u64 kStart = paramv[3];
    u64 kEnd = paramv[4];

    int k;
    for(k=kStart;k<kEnd;k++) {
        float t_real = X_real[k];
        float t_imag = X_imag[k];
        double twiddle_real;
        double twiddle_imag;
        twiddle_imag = sin(-2 * M_PI * k / N);
        twiddle_real = cos(-2 * M_PI * k / N);
        float xr = X_real[k+N/2];
        float xi = X_imag[k+N/2];

        // (a+bi)(c+di) = (ac - bd) + (bc + ad)i
        X_real[k] = t_real +
            (twiddle_real*xr - twiddle_imag*xi);
        X_imag[k] = t_imag +
            (twiddle_imag*xr + twiddle_real*xi);
        X_real[k+N/2] = t_real -
            (twiddle_real*xr - twiddle_imag*xi);
        X_imag[k+N/2] = t_imag -
            (twiddle_imag*xr + twiddle_real*xi);
    }

    return NULL_GUID;
}

ocrGuid_t finalPrintEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
    int i;
    float *data = (float*)depv[1].ptr;
    ocrGuid_t dataGuid = depv[1].guid;
    u64 N = paramv[0];
    bool verbose = paramv[1];
    bool printResults = paramv[2];
    float *x_in = (float*)data;
    float *X_real = (float*)(data + N);
    float *X_imag = (float*)(data + 2*N);

    if(verbose) {
        PRINTF("Final print EDT\n");
    }

    if(printResults) {
        PRINTF("Starting values:\n");
        for(i=0;i<N;i++) {
            PRINTF("%d [ %f ]\n",i,x_in[i]);
        }
        PRINTF("\n");

        PRINTF("Final result:\n");
        for(i=0;i<N;i++) {
            PRINTF("%d [%f + %fi]\n",i,X_real[i],X_imag[i]);
        }
    }

    ocrShutdown();
}

extern "C" ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 argc = getArgc(depv[0].ptr);
    int i;
    char *argv[argc];
    for(i=0;i<argc;i++) {
        argv[i] = getArgv(depv[0].ptr,i);
    }

    u64 N;
    u64 iterations;
    bool verify;
    bool verbose;
    bool printResults;
    u64 serialBlockSize = SERIAL_BLOCK_SIZE_DEFAULT;
    if(!parseOptions(argc, argv, &N, &verify, &iterations, &verbose, &printResults,
                     &serialBlockSize)) {
        printHelp(argv,true);
        ocrShutdown();
        return NULL_GUID;
    }
    if(verbose) {
        for(i=0;i<argc;i++) {
            PRINTF("argv[%d]: %s\n",i,argv[i]);
        }
        PRINTF("Running %d iterations\n",iterations);
    }

    ocrGuid_t iterationTempGuid,startTempGuid,endTempGuid,printTempGuid,endSlaveTempGuid;
    ocrEdtTemplateCreate(&iterationTempGuid, &fftIterationEdt, 6, 2);
    ocrEdtTemplateCreate(&startTempGuid, &fftStartEdt, 9, 1);
    ocrEdtTemplateCreate(&endTempGuid, &fftEndEdt, 9, 3);
    ocrEdtTemplateCreate(&endSlaveTempGuid, &fftEndSlaveEdt, 5, 1);
    ocrEdtTemplateCreate(&printTempGuid, &finalPrintEdt, 3, 2);

    // x_in, X_real, and X_imag in a contiguous block
    float *x;
    ocrGuid_t dataGuid;
    // TODO: OCR cannot handle large datablocks
    ocrDbCreate(&dataGuid, (void **) &x, sizeof(float) * N * 3, 0, NULL_GUID, NO_ALLOC);
    if(verbose) {
        PRINTF("Datablock of size %lu (N=%lu) created\n",sizeof(float)*N*3,N);
    }

    for(i=0;i<N;i++) {
        x[i] = 0;
    }
    x[1] = 1;
    //x[3] = -3;
    //x[4] = 8;
    //x[5] = 9;
    //x[6] = 1;


    std::stack<ocrGuid_t> edtStack;
    std::stack<ocrGuid_t> eventStack;
    u64 edtParamv[6] = { startTempGuid, endTempGuid, endSlaveTempGuid, N, verbose, serialBlockSize };

    ocrGuid_t edtGuid, printEdtGuid, edtEventGuid;

    for(i=1;i<=iterations;i++) {
        ocrEdtCreate(&edtGuid, iterationTempGuid, EDT_PARAM_DEF, edtParamv,
                     EDT_PARAM_DEF, NULL_GUID, EDT_PROP_FINISH, NULL_GUID,
                     &edtEventGuid);
        edtStack.push(edtGuid);
        eventStack.push(edtEventGuid);
    }

    edtEventGuid = eventStack.top();
    if(verify) {
        edtEventGuid = setUpVerify(dataGuid, NULL_GUID, NULL_GUID, N, edtEventGuid);
    }

    u64 printParamv[3] = { N, verbose, printResults };
    ocrGuid_t finishDependencies[2] = { edtEventGuid, dataGuid };
    ocrEdtCreate(&printEdtGuid, printTempGuid, EDT_PARAM_DEF, printParamv,
                 EDT_PARAM_DEF, finishDependencies, EDT_PROP_NONE, NULL_GUID, NULL);
    eventStack.pop();

    while(!edtStack.empty()) {
        edtGuid = edtStack.top();
        if(!eventStack.empty()) {
            edtEventGuid = eventStack.top();
        } else {
            edtEventGuid = NULL_GUID;
        }
        ocrAddDependence(dataGuid, edtGuid, 0, DB_MODE_ITW);
        ocrAddDependence(edtEventGuid, edtGuid, 1, DB_MODE_RO);
        edtStack.pop();
        eventStack.pop();
    }

    return NULL_GUID;
}
