#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ocr.h>

#include "cg_ocr.h"
#include "la_ocr.h"

ocrGuid_t head_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t loop_top_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t loop_bottom_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t tail_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    u64 argc = getArgc(depv[0].ptr);

    classdb_t* class; ocrGuid_t classid;
    if(argc>1) {
        if(argc>3) {
            printf("cg [-t] [S,W,A,B,C,D,E] (default: S -t)\n");
            ocrShutdown();
            return NULL_GUID;
        }
        int on = 0;
        while(--argc) {
            if(strcmp(getArgv(depv[0].ptr,argc), "-t")==0)
                on = 1;
            else
                class_init(&class, &classid, *getArgv(depv[0].ptr,argc));
        }
        if(class->c == 'U') {
            printf("cg [-t] [S,W,A,B,C,D,E] (default: S -t)\n");
            ocrShutdown();
            return NULL_GUID;
        }
        class->on = on;
    }
    else
        class_init(&class, &classid, 'S');

    timerdb_t* timer; ocrGuid_t timerid;
    timer_init(&timer, &timerid, class, class->on);
    printf("CG Benchmark: size=%d, iterations=%d\n", class->na, class->niter);

    timer_start(timer);

    ocrGuid_t nid, aid, xid;
    double* x; u32* n;
    ocrDbCreate(&xid, (void**)&x, sizeof(double)*class->na, 0, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&nid, (void**)&n, sizeof(u32), 0, NULL_GUID, NO_ALLOC);
    *n = class->na;

    if(makea(class, &aid) == -1)
        return NULL_GUID;

    u32 i;
    for(i=0; i<class->na; ++i)
        x[i] = 1;

    ocrGuid_t tmp,edt;
    ocrEdtTemplateCreate(&tmp, head_edt, 0, 7);
    ocrEdtCreate(&edt, tmp, 0, NULL, 7, NULL, 0, NULL_GUID, NULL);
    ocrAddDependence(classid, edt, 0, DB_MODE_RO);
    ocrAddDependence(timerid, edt, 1, DB_MODE_ITW);
    ocrAddDependence(nid, edt, 2, DB_MODE_RO);
    ocrAddDependence(aid, edt, 3, DB_MODE_RO);
    ocrAddDependence(xid, edt, 4, DB_MODE_RO);
    conj_grad(class->na, nid, aid, xid, edt, 5);
    ocrEdtTemplateDestroy(tmp);

    ocrDbDestroy(depv[0].guid);

    return NULL_GUID;
}

//     0        1  2  3  4  5     6
// class, timerdb, n, a, x, z, norm
ocrGuid_t head_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    classdb_t* class = (classdb_t*)depv[0].ptr;
    timerdb_t* tdb = (timerdb_t*)depv[1].ptr;
    timer_stop(tdb,0);
    ocrDbDestroy(depv[5].guid);
    ocrDbDestroy(depv[6].guid);

    printf("Iteration               ||r||               zeta\n");

    double* x = (double*)depv[4].ptr;
    u32 i;
    for(i=0; i<class->na; ++i)
        x[i] = 1;

    timer_start(tdb);

    ocrGuid_t itid; u32* it;
    ocrDbCreate(&itid, (void**)&it, sizeof(u32), 0, NULL_GUID, NO_ALLOC);
    *it = 1;

    ocrGuid_t tmp,edt;
    ocrEdtTemplateCreate(&tmp, loop_top_edt, 0, 6);
    ocrEdtCreate(&edt, tmp, 0, NULL, 6, NULL, 0, NULL_GUID, NULL);
    for(i=0; i<depc-2; ++i)
        ocrAddDependence(depv[i].guid, edt, i, DB_MODE_ITW);
    ocrAddDependence(itid, edt, i, DB_MODE_ITW);
    ocrEdtTemplateDestroy(tmp);

    return NULL_GUID;
}

//     0        1  2  3  4   5
// class, timerdb, n, a, x, it
ocrGuid_t loop_top_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    classdb_t* class = (classdb_t*)depv[0].ptr;
    timerdb_t* tdb = (timerdb_t*)depv[1].ptr;

    if(class->on) timer_start(tdb);

    ocrGuid_t tmp,edt;
    ocrEdtTemplateCreate(&tmp, loop_bottom_edt, 0, 8);
    ocrEdtCreate(&edt, tmp, 0, NULL, 8, NULL, 0, NULL_GUID, NULL);
    int i;
    for(i=0; i<depc; ++i)
        ocrAddDependence(depv[i].guid, edt, i, DB_MODE_ITW);
    conj_grad(class->na, depv[2].guid, depv[3].guid, depv[4].guid, edt, depc);
    ocrEdtTemplateDestroy(tmp);

    return NULL_GUID;
}

//     0        1  2  3  4   5  6     7
// class, timerdb, n, a, x, it, z, norm
ocrGuid_t loop_bottom_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    classdb_t* class = (classdb_t*)depv[0].ptr;
    timerdb_t* tdb = (timerdb_t*)depv[1].ptr;
    double* x = (double*)depv[4].ptr;
    double* z = (double*)depv[6].ptr;
    u32* it = (u32*)depv[5].ptr;

    if(class->on) timer_stop(tdb,*it+1);

    double zeta = class->shift+1/_dot(class->na,x,z);
    double norm_temp2 = 1.0/sqrt(_dot(class->na,z,z));
    __scale(class->na,x,norm_temp2,z);
    printf("%9d, %20.13le %10.13lf\n", *it, *(double*)depv[7].ptr, zeta);
    ocrDbDestroy(depv[6].guid);

    ++*it;
    if(*it > class->niter) {
        *(double*)depv[7].ptr = zeta;
        ocrGuid_t tmp,edt;
        ocrEdtTemplateCreate(&tmp, tail_edt, 0, 3);
        ocrEdtCreate(&edt, tmp, 0, NULL, 3, NULL, 0, NULL_GUID, NULL);
        ocrAddDependence(depv[0].guid, edt, 0, DB_MODE_RO);
        ocrAddDependence(depv[1].guid, edt, 1, DB_MODE_RO);
        ocrAddDependence(depv[7].guid, edt, 2, DB_MODE_RO);
        ocrDbDestroy(depv[2].guid);
        ocrDbDestroy(depv[3].guid);
        ocrDbDestroy(depv[4].guid);
        ocrDbDestroy(depv[5].guid);
        ocrEdtTemplateDestroy(tmp);
    }
    else {
        ocrGuid_t tmp,edt;
        ocrEdtTemplateCreate(&tmp, loop_top_edt, 0, 6);
        ocrEdtCreate(&edt, tmp, 0, NULL, 6, NULL, 0, NULL_GUID, NULL);
        int i;
        for(i=0; i<depc-2; ++i)
            ocrAddDependence(depv[i].guid, edt, i, DB_MODE_ITW);
        ocrDbDestroy(depv[7].guid);
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
