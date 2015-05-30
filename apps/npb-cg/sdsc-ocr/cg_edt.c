#include <ocr.h>

#include "la_ocr.h"

// depv: x
ocrGuid_t square_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t rho; double* rho_ptr;
    ocrDbCreate(&rho,(void**)&rho_ptr,sizeof(double)*paramv[0],0,NULL_GUID,NO_ALLOC);
    *rho_ptr = _dot(paramv[0],depv[0].ptr,depv[0].ptr);

    return rho;
}

// depv: rho p q -> param:nalpha
ocrGuid_t alphas_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t alpha,nalpha; double* alpha_ptr,* nalpha_ptr;
    ocrDbCreate(&alpha,(void**)&alpha_ptr,sizeof(double),0,NULL_GUID,NO_ALLOC);
    ocrDbCreate(&nalpha,(void**)&nalpha_ptr,sizeof(double),0,NULL_GUID,NO_ALLOC);
    *alpha_ptr = *(double*)depv[0].ptr/_dot(paramv[0],depv[1].ptr,depv[2].ptr);
    *nalpha_ptr = -*alpha_ptr;
    ocrEventSatisfy(paramv[1],nalpha);

    return alpha;
}

// depv: rho p q
ocrGuid_t alpha_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t alpha; double* alpha_ptr;
    ocrDbCreate(&alpha,(void**)&alpha_ptr,sizeof(double),0,NULL_GUID,NO_ALLOC);
    *alpha_ptr = *(double*)depv[0].ptr/_dot(paramv[0],depv[1].ptr,depv[2].ptr);
    ocrDbDestroy(depv[0].guid);
    ocrDbDestroy(depv[2].guid);

    return alpha;
}

// depv: a x
ocrGuid_t scale_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t z; double* z_ptr;
    ocrDbCreate(&z,(void**)&z_ptr,sizeof(double)*paramv[0],0,NULL_GUID,NO_ALLOC);
    __scale(paramv[0],z_ptr,*(double*)depv[0].ptr,depv[1].ptr);
    ocrDbDestroy(depv[0].guid);
    return z;
}

// depv: y a x
ocrGuid_t daxpy_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
     __daxpy(paramv[0],depv[0].ptr,*(double*)depv[1].ptr,depv[2].ptr,depv[0].ptr);
    ocrDbDestroy(depv[1].guid);
    ocrDbDestroy(depv[2].guid);
    return depv[0].guid;
}

// depv: na p q r rr pp rho param:out
ocrGuid_t update_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    __daxpy(paramv[0],depv[3].ptr,*(double*)depv[0].ptr,depv[2].ptr,depv[4].ptr);
    double rhotmp = *(double*)depv[6].ptr;
    *(double*)depv[6].ptr = _dot(paramv[0],depv[3].ptr,depv[3].ptr);
    double beta = *(double*)depv[6].ptr/rhotmp;
    __daxpy(paramv[0],depv[1].ptr,beta,depv[5].ptr,depv[3].ptr);
    ocrDbDestroy(depv[0].guid);
    ocrDbDestroy(depv[2].guid);
    ocrGuid_t pp; double* pp_ptr;
    ocrDbCreate(&pp,(void**)&pp_ptr,sizeof(double)*paramv[0],0,NULL_GUID,NO_ALLOC);
    _copy(paramv[0],pp_ptr,depv[1].ptr);
    ocrEventSatisfy(paramv[1],pp);
    return depv[1].guid;
}

// depv: param:n x1 x2 ... xn param:out
ocrGuid_t assign_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t r;
    double* ptr;
    ocrDbCreate(&r,(void**)&ptr,sizeof(double)*paramv[0],0,NULL_GUID,NO_ALLOC);

    u64 i,j;
    for(i = 0; i<depc; ++i) {
      for(j = 0; j<paramv[0]/depc; ++j,++ptr)
        *ptr = ((double*)depv[i].ptr)[j];
      ocrDbDestroy(depv[i].guid);
    }
    ocrEventSatisfy(paramv[1],r);

    return NULL_GUID;
}

// depv: c x
ocrGuid_t rowvec_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t r; double* r_ptr;
    ocrDbCreate(&r,(void**)&r_ptr,sizeof(double)*paramv[0],0,NULL_GUID,NO_ALLOC);
    u32* rows = (u32*)depv[0].ptr;
    double* values = ((double*)depv[0].ptr)+((paramv[0]+2)>>1);
    u32* indexes = rows+rows[paramv[0]];
    u64 i;
    for(i = 0; i<paramv[0]; ++i) {
      r_ptr[i] = _dotg(rows[i],values,indexes,depv[1].ptr);
      values += rows[i]; indexes += rows[i];
    }
    return r;
}

// depv: param:n,blk A x param:out
ocrGuid_t spmv_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    u64 paramv2[2] = {paramv[0], paramv[2]};
    ocrGuid_t assgnt,assgn;
    ocrEdtTemplateCreate(&assgnt,assign_edt,2,paramv[0]/paramv[1]);
    ocrEdtCreate(&assgn,assgnt,2,paramv2,paramv[0]/paramv[1],NULL,0,NULL_GUID,NULL);
    ocrEdtTemplateDestroy(assgnt);

    ocrGuid_t rowvect;
    ocrEdtTemplateCreate(&rowvect,rowvec_edt,1,2);
    u32 e;
    for(e = 0; e < paramv[0]/paramv[1]; ++e) {
        ocrGuid_t rv,rve;
        ocrEdtCreate(&rv,rowvect,1,paramv+1,2,NULL,0,NULL_GUID,&rve);
        ocrAddDependence(rve,assgn,e,DB_MODE_CONST);
        ocrAddDependence(((ocrGuid_t*)depv[0].ptr)[e],rv,0,DB_MODE_CONST);
        ocrAddDependence(depv[1].guid,rv,1,DB_MODE_CONST);
    }
    ocrEdtTemplateDestroy(rowvect);

    return NULL_GUID;
}

// depv: r x rr pp
ocrGuid_t dist_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t d; double* d_ptr;
    ocrDbCreate(&d,(void**)&d_ptr,sizeof(double),0,NULL_GUID,NO_ALLOC);
    *d_ptr = _dist(paramv[0],(double*)depv[0].ptr,(double*)depv[1].ptr);
    ocrDbDestroy(depv[0].guid);
    ocrDbDestroy(depv[2].guid);
    ocrDbDestroy(depv[3].guid);

    return d;
}
