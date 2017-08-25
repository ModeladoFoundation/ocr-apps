#include <math.h>
#include <stdlib.h>

#include "ocr.h"

#include "macros.h"

#define DEFAULTnrank 4
#define DEFAULTtolerance 0.01
#define DEFAULTdelta 0.5
#define DEFAULTmaxX 100000

#define EPSILON 0.000001

//double less than compare
bool lt(double x1, double x2){
    return x1 < (x2 - EPSILON);
}

//double greater than compare
bool gt(double x1, double x2){
    return x1 > (x2 - EPSILON);
}

//double equality compare
bool eq(double x1, double x2){
    return fabs(x1 - x2) < EPSILON;
}

//Compute y-intercept for linear segment
double b(double m, double x, double y){
    return y - (m*x);
}

//Arbitrary curve fit function
double f(double x){
    return 2*(sin(x)/2)*x;
}

//Compute predicted value for fitted linear function
double predict(double m, double b, double x){
    return (m*x)+b;
}

//Compute RMS error for linear segment
double compute_error(double x1, double x2, double delta) {
    double error = 0.0;
    double idx = x1;
    //Get slope and y-intercept for linear segment
    double slope = ( f(x2) - f(x1) ) / (x2 - x1);
    double y_int = b(slope, x1, f(x1));

    u32 numVals = 0;

    while(lt(idx, x2)) {
        numVals++;
        //Compute diff of predicted value from linear segment, and actual value
        error += pow( ( (predict(slope, y_int, idx)) - f(idx) ),  2);
        idx += delta;
    }
    //return RMS Error
    return sqrt((error/numVals));
}

void errorOut(char *s) {
    ocrPrintf("Error: %s \n", s);
    ocrShutdown();
}


typedef struct {
    u64 nrank;
    double x1;  //L
    double x2;  //H
    double tolerance;
    double delta;
    ocrGuid_t curveFitTemplate;
} curveFitPRM_t;

ocrGuid_t curveFitEdt( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {

    PRMDEF(curveFit);

    u64 nrank = PRM(curveFit, nrank);
    double x1 = PRM(curveFit, x1); //subsection low
    double x2 = PRM(curveFit, x2); //subsection high
    double tolerance = PRM(curveFit, tolerance);
    double delta = PRM(curveFit, delta);
    ocrGuid_t curveFitTemplate = PRM(curveFit, curveFitTemplate);

    curveFitPRM_t curveFitParamv;
    ocrGuid_t curveFitEdt;

    curveFitParamv.nrank = nrank;
    curveFitParamv.curveFitTemplate = curveFitTemplate;
    curveFitParamv.tolerance = tolerance;
    curveFitParamv.delta = delta;

    double error = compute_error(x1, x2, delta);

    //curve fit not found for current section
    if(gt(error, tolerance)){

        double splitDiff = x2 - x1;

        u32 i;
        //Subdivide current section and spawn nrank EDTs for refined curve fitting.
        for(i = 0; i < nrank; i++){
            curveFitParamv.x1 = x1+(splitDiff/nrank)*i;
            curveFitParamv.x2 = x1+(splitDiff/nrank)*(i+1);
            ocrEdtCreate(&curveFitEdt, curveFitTemplate, EDT_PARAM_DEF, (u64 *)&curveFitParamv,
                EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
        }

    }
    //else curve fit found for current section
    //TODO report x1, x2, equation for linear segment??
    return NULL_GUID;
}


typedef struct {
} wrapupPRM_t;

ocrGuid_t wrapupEdt( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
    ocrPrintf("SUCCESS - Shutting down OCR\n");
    ocrShutdown();
}

typedef struct {
    u64 nrank;
    double tolerance;
    double delta;
    u64 maxX;
} realMainPRM_t;

ocrGuid_t realMainEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRMDEF(realMain);

    //Gather params
    u64 nrank = PRM(realMain, nrank);
    double tolerance = PRM(realMain, tolerance);
    double delta = PRM(realMain, delta);
    u64 maxX = PRM(realMain, maxX);

    ocrPrintf("ARGS: nrank - %lu | tolerance - %f | delta - %f | max X value - %lu\n", nrank, tolerance, delta, maxX);

    curveFitPRM_t curveFitParamv;
    ocrGuid_t curveFitTemplate;
    ocrGuid_t curveFitEDT;

    ocrEdtTemplateCreate(&curveFitTemplate, curveFitEdt, PRMNUM(curveFit), 0);

    curveFitParamv.nrank = nrank;
    curveFitParamv.x1 = 0;
    curveFitParamv.x2 = maxX;
    curveFitParamv.tolerance = tolerance;
    curveFitParamv.delta = delta;
    curveFitParamv.curveFitTemplate = curveFitTemplate;

    ocrGuid_t outEvt;

    //Launch initial curve fit computation
    ocrEdtCreate(&curveFitEDT, curveFitTemplate, EDT_PARAM_DEF, (u64 *)&curveFitParamv,
        EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_HINT, &outEvt);

    ocrGuid_t wrapupTemplate;
    ocrGuid_t wrapupEDT;

    ocrGuid_t wrapupDepv[1] = {outEvt};
    ocrEdtTemplateCreate(&wrapupTemplate, wrapupEdt, 0, 1);
    ocrEdtCreate(&wrapupEDT, wrapupTemplate, EDT_PARAM_DEF, NULL,
        EDT_PARAM_DEF, wrapupDepv, EDT_PROP_FINISH, NULL_HINT, NULL);



    return NULL_GUID;

}

ocrGuid_t mainEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    realMainPRM_t paramvout;
    realMainPRM_t * realMainPRM = &paramvout;

    void *mainArgv = depv[0].ptr;
    u32 argc = ocrGetArgc(mainArgv);

    PRM(realMain, nrank) = DEFAULTnrank;
    PRM(realMain, tolerance) = DEFAULTtolerance;
    PRM(realMain, delta) = DEFAULTdelta;
    PRM(realMain, maxX) = DEFAULTmaxX;

    if(argc != 1 && argc != 5)
        errorOut("Usage: [numRanks, Error Tolerance, Delta, MaxX]");

    if(argc == 5){
        u32 i = 1;
        PRM(realMain, nrank) = (u64) atoi(ocrGetArgv(mainArgv, i++));
        PRM(realMain, tolerance) = (double) atof(ocrGetArgv(mainArgv, i++));
        PRM(realMain, delta) = (double) atof(ocrGetArgv(mainArgv, i++));
        PRM(realMain, maxX) = (u64) atoi(ocrGetArgv(mainArgv, i++));

        if(PRM(realMain, nrank) <= 0)
            errorOut("numRanks must be positive");
        if(PRM(realMain, tolerance) <= 0)
            errorOut("Error Tolerance must be positive");
        if(PRM(realMain, delta) <= 0)
            errorOut("Delta value must be positive");
        if(PRM(realMain, maxX) <= 0)
            errorOut("Maximum X value must be positive");
    }

    ocrGuid_t realMainEDT, realMainTemplate;

    ocrEdtTemplateCreate(&realMainTemplate, realMainEdt, PRMNUM(realMain), 0);
    ocrEdtCreate(&realMainEDT, realMainTemplate, EDT_PARAM_DEF, (u64 *) realMainPRM, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL);
    return NULL_GUID;

}
