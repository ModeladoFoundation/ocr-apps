#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ocr.h>
#include <assert.h>

#include "cg_ocr.h"
#include "la_ocr.h"

ocrGuid_t head_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t loop_top_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t loop_bottom_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t tail_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    u64 argc = getArgc(depv[0].ptr);

    classdb_t* class; ocrGuid_t classid;
    char* writeFile = NULL;
    char* readFile = NULL;
    if(argc>1) {
        if(argc!=3 && argc != 5 && argc !=7) {
            printf("cg [-t class] [-b blocking ] [-w writeFile] [-r readFile] (class=S|W|A|B|C|D|E; default: -t S, -b 1)\n");
            ocrShutdown();
            return NULL_GUID;
        }
        u32 blocking = 1;
        char classt = 'S';
        while(--argc) {
            if(strcmp(getArgv(depv[0].ptr,argc-1), "-t")==0)
	      classt = *getArgv(depv[0].ptr,argc--);
            else if(strcmp(getArgv(depv[0].ptr,argc-1), "-b")==0)
	      blocking = atoi(getArgv(depv[0].ptr,argc--));
	    else if(strcmp(getArgv(depv[0].ptr, argc-1), "-w")==0)
	      writeFile = getArgv(depv[0].ptr,argc--);
	    else if(strcmp(getArgv(depv[0].ptr, argc-1), "-r")==0)
	      readFile = getArgv(depv[0].ptr,argc--);
        }
        class_init(&class, &classid, classt, blocking);
        if(class->c == 'U') {
            printf("cg [-t] [S,W,A,B,C,D,E] (default: S -t)\n");
            ocrShutdown();
            return NULL_GUID;
        }
    }
    else
        class_init(&class, &classid, 'S', 1);

    timerdb_t* timer; ocrGuid_t timerid;
    timer_init(&timer, &timerid, class, class->on);
    printf("CG Benchmark: size=%d, iterations=%d", class->na, class->niter);
    if(writeFile != NULL) printf(", writeFile=%s", writeFile);
    if(readFile != NULL) printf(", readFile=%s", readFile);
    printf("\n");

    timer_start(timer);

    ocrGuid_t aid, xid;
    double* x;
    ocrDbCreate(&xid, (void**)&x, sizeof(double)*class->na, 0, currentAffinity, NO_ALLOC);

    if(makea(class, &aid, writeFile, readFile) == -1)
        return NULL_GUID;

    ocrDbRelease(aid);
    ocrDbRelease(xid);

    u32 i;
    for(i=0; i<class->na; ++i)
        x[i] = 1;

    ocrGuid_t tmp,edt;
    ocrEdtTemplateCreate(&tmp, head_edt, 0, 6);
    ocrEdtCreate(&edt, tmp, 0, NULL, 6, NULL, 0, currentAffinity, NULL);
    ocrAddDependence(classid, edt, 0, DB_MODE_RO);
    ocrAddDependence(timerid, edt, 1, DB_MODE_ITW);
    ocrAddDependence(aid, edt, 2, DB_MODE_RO);
    ocrAddDependence(xid, edt, 3, DB_MODE_RO);
    conj_grad(class->na, class->blk, aid, xid, edt, 4);
    ocrEdtTemplateDestroy(tmp);

    ocrDbDestroy(depv[0].guid);

    return NULL_GUID;
}

//     0        1  2  3  4  5
// class, timerdb, a, x, z, norm
ocrGuid_t head_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    classdb_t* class = (classdb_t*)depv[0].ptr;
    timerdb_t* tdb = (timerdb_t*)depv[1].ptr;
    timer_stop(tdb,0);
    ocrDbDestroy(depv[4].guid);
    ocrDbDestroy(depv[5].guid);

    printf("Iteration               ||r||               zeta\n");

    double* x = (double*)depv[3].ptr;
    u32 i;
    for(i=0; i<class->na; ++i)
        x[i] = 1;

    timer_start(tdb);

    u64 it = 1;

    ocrGuid_t tmp,edt;
    ocrEdtTemplateCreate(&tmp, loop_top_edt, 1, 4);
    ocrEdtCreate(&edt, tmp, 1, &it, 4, NULL, 0, currentAffinity, NULL);
    for(i=0; i<depc-2; ++i)
        ocrAddDependence(depv[i].guid, edt, i, DB_MODE_ITW);
    ocrEdtTemplateDestroy(tmp);

    return NULL_GUID;
}

//     0        1  2  3
// class, timerdb, a, x
ocrGuid_t loop_top_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    classdb_t* class = (classdb_t*)depv[0].ptr;
    timerdb_t* tdb = (timerdb_t*)depv[1].ptr;

    if(class->on) timer_start(tdb);

    ocrGuid_t tmp,edt;
    ocrEdtTemplateCreate(&tmp, loop_bottom_edt, 1, 6);
    ocrEdtCreate(&edt, tmp, 1, paramv, 6, NULL, 0, currentAffinity, NULL);
    int i;
    for(i=0; i<depc; ++i)
        ocrAddDependence(depv[i].guid, edt, i, DB_MODE_ITW);
    conj_grad(class->na, class->blk, depv[2].guid, depv[3].guid, edt, 4);
    ocrEdtTemplateDestroy(tmp);

    return NULL_GUID;
}

//     0        1  2  3  4  5
// class, timerdb, a, x, z, norm
ocrGuid_t loop_bottom_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    classdb_t* class = (classdb_t*)depv[0].ptr;
    timerdb_t* tdb = (timerdb_t*)depv[1].ptr;
    double* x = (double*)depv[3].ptr;
    double* z = (double*)depv[4].ptr;

    if(class->on) timer_stop(tdb,paramv[0]+1);

    double zeta = class->shift+1/_dot(class->na,x,z);
    double norm_temp2 = 1.0/sqrt(_dot(class->na,z,z));
    __scale(class->na,x,norm_temp2,z);
    printf("%9d, %20.13le %10.13lf\n", paramv[0], *(double*)depv[5].ptr, zeta);
    ocrDbDestroy(depv[4].guid);

    if(++paramv[0] > class->niter) {
        *(double*)depv[5].ptr = zeta;
        ocrGuid_t tmp,edt;
        ocrEdtTemplateCreate(&tmp, tail_edt, 0, 3);
        ocrEdtCreate(&edt, tmp, 0, NULL, 3, NULL, 0, currentAffinity, NULL);
        ocrAddDependence(depv[0].guid, edt, 0, DB_MODE_RO);
        ocrAddDependence(depv[1].guid, edt, 1, DB_MODE_RO);
        ocrAddDependence(depv[5].guid, edt, 2, DB_MODE_RO);
        ocrDbDestroy(depv[2].guid);
        ocrDbDestroy(depv[3].guid);
        ocrEdtTemplateDestroy(tmp);
    }
    else {
        ocrGuid_t tmp,edt;
        ocrEdtTemplateCreate(&tmp, loop_top_edt, 1, 4);
        ocrEdtCreate(&edt, tmp, 1, paramv, 4, NULL, 0, currentAffinity, NULL);
        int i;
        for(i=0; i<depc-2; ++i)
            ocrAddDependence(depv[i].guid, edt, i, DB_MODE_ITW);
        ocrDbDestroy(depv[5].guid);
        ocrEdtTemplateDestroy(tmp);
    }

    return NULL_GUID;
}

//     0        1     2
// class, timerdb, zeta
ocrGuid_t tail_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    classdb_t* class = (classdb_t*)depv[0].ptr;
    timerdb_t* tdb = (timerdb_t*)depv[1].ptr;
    double zeta = *(double*)depv[2].ptr;

    timer_stop(tdb,1);
    printf("Benchmark completed\n");

    double err = abs(zeta-class->zvv)/class->zvv;
    if(err<=1e-10) {
        double mflops;
        double t_bench = timer_read(tdb,1);
        if(t_bench)
            mflops = 2*class->niter*class->na*
                     (((double)3+class->nonzer*(class->nonzer+1))+((double)25*(5+class->nonzer*(class->nonzer+1))+3))/
                     t_bench/1000000;
        printf("Verification SUCCESSFUL (zeta=%.13lf, error=%.13lf\n", zeta, err);
        print_results(class, t_bench, mflops);

        if(class->on) {
            printf("Timing (sec,%%)\n");
            double t_cg = 0;
            int it;
            for(it = 1; it<=class->niter; ++it)
                t_cg += timer_read(tdb,it+1);
            double t_init = timer_read(tdb,0);
            double wall = t_init+timer_read(tdb,1);
            printf("init:      %10.3lf (%3.1lf%%)\n", t_init, t_init/wall*100);
            printf("cg:        %10.3lf (%3.1lf%%)\n", t_cg, t_cg/wall*100);

            u64 affinityCount = 0; ocrAffinityCount(AFFINITY_PD, &affinityCount); assert(affinityCount >= 1);
            printf("cg_format: \t %10.3lf \t %3.1lf% \t %f \t %f \t %c\n", t_cg, t_cg/wall*100, affinityCount, class->blk, class->c);

            printf("norm:      %10.3lf (%3.1lf%%)\n", (t_bench-t_cg), (t_bench-t_cg)/wall*100);
            printf("wall time: %10.3lf\n", wall);
        }
    }
    else
        printf("Verification FAILED (zeta=%.13lf, correct zeta=%.13lf\n", zeta, class->zvv);

    ocrDbDestroy(depv[0].guid);
    ocrDbDestroy(depv[1].guid);
    ocrDbDestroy(depv[2].guid);

    ocrShutdown();
    return NULL_GUID;
}
