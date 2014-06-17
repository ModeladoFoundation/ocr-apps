#include <string.h>
#include <ocr.h>

#include "la_ocr.h"

// depv: n x
ocrGuid_t square_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t rho; double* rho_ptr;
    ocrDbCreate(&rho,(void**)&rho_ptr,sizeof(double)*(*(u32*)depv[0].ptr),0,NULL_GUID,NO_ALLOC);
    *rho_ptr = _dot(*(u32*)depv[0].ptr,depv[1].ptr,depv[1].ptr);

    return rho;
}

// depv: n rho p q -> nalpha
ocrGuid_t alphas_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t alpha,nalpha; double* alpha_ptr,* nalpha_ptr;
    ocrDbCreate(&alpha,(void**)&alpha_ptr,sizeof(double),0,NULL_GUID,NO_ALLOC);
    ocrDbCreate(&nalpha,(void**)&nalpha_ptr,sizeof(double),0,NULL_GUID,NO_ALLOC);
    *alpha_ptr = *(double*)depv[1].ptr/_dot(*(u32*)depv[0].ptr,depv[2].ptr,depv[3].ptr);
    *nalpha_ptr = -*alpha_ptr;
    ocrEventSatisfy(*(ocrGuid_t*)depv[4].ptr,nalpha);
    ocrDbDestroy(depv[4].guid);

    return alpha;
}

// depv: n rho p q
ocrGuid_t alpha_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t alpha; double* alpha_ptr;
    ocrDbCreate(&alpha,(void**)&alpha_ptr,sizeof(double),0,NULL_GUID,NO_ALLOC);
    *alpha_ptr = *(double*)depv[1].ptr/_dot(*(u32*)depv[0].ptr,depv[2].ptr,depv[3].ptr);
    ocrDbDestroy(depv[1].guid);
    ocrDbDestroy(depv[3].guid);

    return alpha;
}

// depv: n a x
ocrGuid_t scale_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t z; double* z_ptr;
    ocrDbCreate(&z,(void**)&z_ptr,sizeof(double)*(*(u32*)depv[0].ptr),0,NULL_GUID,NO_ALLOC);
    __scale(*(u32*)depv[0].ptr,z_ptr,*(double*)depv[1].ptr,depv[2].ptr);
    ocrDbDestroy(depv[1].guid);
    return z;
}

// depv: n y a x
ocrGuid_t daxpy_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
     __daxpy(*(u32*)depv[0].ptr,depv[1].ptr,*(double*)depv[2].ptr,depv[3].ptr,depv[1].ptr);
    ocrDbDestroy(depv[2].guid);
    ocrDbDestroy(depv[3].guid);
    return depv[1].guid;
}

// depv: n y a x
ocrGuid_t daxpyl_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
     __daxpy(*(u32*)depv[0].ptr,depv[1].ptr,*(double*)depv[2].ptr,depv[3].ptr,depv[1].ptr);
    ocrDbDestroy(depv[2].guid);
    ocrDbDestroy(depv[3].guid);
    return depv[1].guid;
}

// depv: n na p q r rr pp rho out
ocrGuid_t update_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    __daxpy(*(u32*)depv[0].ptr,depv[4].ptr,*(double*)depv[1].ptr,depv[3].ptr,depv[5].ptr);
    double rhotmp = *(double*)depv[7].ptr;
    *(double*)depv[7].ptr = _dot(*(u32*)depv[0].ptr,depv[4].ptr,depv[4].ptr);
    double beta = *(double*)depv[7].ptr/rhotmp;
    __daxpy(*(u32*)depv[0].ptr,depv[2].ptr,beta,depv[6].ptr,depv[4].ptr);
    ocrDbDestroy(depv[1].guid);
    ocrDbDestroy(depv[3].guid);
    ocrGuid_t pp; double* pp_ptr;
    ocrDbCreate(&pp,(void**)&pp_ptr,sizeof(double)*(*(u32*)depv[0].ptr),0,NULL_GUID,NO_ALLOC);
    _copy(*(u32*)depv[0].ptr,pp_ptr,depv[2].ptr);
    ocrEventSatisfy(*(ocrGuid_t*)depv[depc-1].ptr,pp);
    ocrDbDestroy(depv[depc-1].guid);
    return depv[2].guid;
}

// depv: x1 x2 ... xn out
ocrGuid_t assign_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t r;
    double* ptr;
    ocrDbCreate(&r,(void**)&ptr,sizeof(double)*(depc-1),0,NULL_GUID,NO_ALLOC);

    u64 i;
    for(i = 0; i<depc-1; ++i,++ptr) {
        *ptr = *(double*)depv[i].ptr;
        ocrDbDestroy(depv[i].guid);
    }
    ocrEventSatisfy(*(ocrGuid_t*)depv[depc-1].ptr,r);
    ocrDbDestroy(depv[depc-1].guid);

    return NULL_GUID;
}

// depv: c x
ocrGuid_t rowvec_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t r; double* r_ptr;
    ocrDbCreate(&r,(void**)&r_ptr,sizeof(double),0,NULL_GUID,NO_ALLOC);
    *r_ptr = _dotg(*(u32*)depv[0].ptr,((double*)depv[0].ptr)+1,
                  ((u32*)depv[0].ptr)+((1+((u32*)depv[0].ptr)[0])<<1),depv[1].ptr);
    return r;
}

// depv: n A x out
ocrGuid_t spmv_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    u32 n = *(u32*)depv[0].ptr;

    ocrGuid_t assgnt,assgn;
    ocrEdtTemplateCreate(&assgnt,assign_edt,0,n+1);
    ocrEdtCreate(&assgn,assgnt,0,NULL,n+1,NULL,0,NULL_GUID,NULL);
    ocrAddDependence(depv[3].guid,assgn,n,DB_MODE_RO);
    ocrEdtTemplateDestroy(assgnt);

    ocrGuid_t rowvect;
    ocrEdtTemplateCreate(&rowvect,rowvec_edt,0,2);

    u32 e;
    for(e = 0; e < n; ++e) {
        ocrGuid_t rv,rve;
        ocrEdtCreate(&rv,rowvect,0,NULL,2,NULL,0,NULL_GUID,&rve);
        ocrAddDependence(rve,assgn,e,DB_MODE_RO);
        ocrAddDependence(((ocrGuid_t*)depv[1].ptr)[e],rv,0,DB_MODE_RO);
        ocrAddDependence(depv[2].guid,rv,1,DB_MODE_RO);
    }

    ocrEdtTemplateDestroy(rowvect);

    return NULL_GUID;
}

// depv: n r x rr pp
ocrGuid_t dist_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t d; double* d_ptr;
    ocrDbCreate(&d,(void**)&d_ptr,sizeof(double),0,NULL_GUID,NO_ALLOC);
    *d_ptr = _dist(*(u32*)depv[0].ptr,(double*)depv[1].ptr,(double*)depv[2].ptr);
    ocrDbDestroy(depv[1].guid);
    ocrDbDestroy(depv[3].guid);
    ocrDbDestroy(depv[4].guid);

    return d;
}
