#include <string.h>
#include <ocr.h>
#include <assert.h>
#include "extensions/ocr-affinity.h"

#include "la_ocr.h"

// depv: x
ocrGuid_t square_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t rho; double* rho_ptr;
    ocrDbCreate(&rho,(void**)&rho_ptr,sizeof(double)*paramv[0],0,currentAffinity,NO_ALLOC);
    *rho_ptr = _dot(paramv[0],depv[0].ptr,depv[0].ptr);

    return rho;
}

// depv: rho p q -> param:nalpha
ocrGuid_t alphas_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  //  printf("[alphas] Start \n");
    ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t alpha,nalpha; double* alpha_ptr,* nalpha_ptr;
    ocrDbCreate(&alpha,(void**)&alpha_ptr,sizeof(double),0,currentAffinity,NO_ALLOC);
    ocrDbCreate(&nalpha,(void**)&nalpha_ptr,sizeof(double),0,currentAffinity,NO_ALLOC);
    *alpha_ptr = *(double*)depv[0].ptr/_dot(paramv[0],depv[1].ptr,depv[2].ptr);
    *nalpha_ptr = -*alpha_ptr;
    ocrEventSatisfy(paramv[1],nalpha);

    return alpha;
}

// depv: rho p q
ocrGuid_t alpha_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  // printf("[alpha] Start \n");
    ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t alpha; double* alpha_ptr;
    ocrDbCreate(&alpha,(void**)&alpha_ptr,sizeof(double),0,currentAffinity,NO_ALLOC);
    *alpha_ptr = *(double*)depv[0].ptr/_dot(paramv[0],depv[1].ptr,depv[2].ptr);
    ocrDbDestroy(depv[0].guid);
    ocrDbDestroy(depv[2].guid);

    return alpha;
}

// depv: a x
ocrGuid_t scale_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  //printf("[scale] Start \n");
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t z; double* z_ptr;
    ocrDbCreate(&z,(void**)&z_ptr,sizeof(double)*paramv[0],0,currentAffinity,NO_ALLOC);
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

// depv: nalpha p q r rr=x pp=x rho, args {n ppo=norm}
ocrGuid_t update_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    __daxpy(paramv[0],depv[3].ptr,*(double*)depv[0].ptr,depv[2].ptr,depv[4].ptr); // r = nalpha*q+x
    double rhotmp = *(double*)depv[6].ptr;
    *(double*)depv[6].ptr = _dot(paramv[0],depv[3].ptr,depv[3].ptr); // rho = r*r
    double beta = *(double*)depv[6].ptr/rhotmp;						 //beta = rho_new/rho_old
    __daxpy(paramv[0],depv[1].ptr,beta,depv[5].ptr,depv[3].ptr);     // p = beta*x=p+r
    ocrDbDestroy(depv[0].guid);
    ocrDbDestroy(depv[2].guid);
    ocrGuid_t pp; double* pp_ptr;
    ocrDbCreate(&pp,(void**)&pp_ptr,sizeof(double)*paramv[0],0,currentAffinity,NO_ALLOC);
    _copy(paramv[0],pp_ptr,depv[1].ptr); // pp = copy(p)
    ocrEventSatisfy(paramv[1],pp);		 // dep for next iteration
    return depv[1].guid;
}

// depv: param:n x1 x2 ... xn param:out // argc {n out_q} deps {r1 r2 come from rowvec_edt
ocrGuid_t assign_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{

  //    printf("[assign] Start \n");
    u64 affinityCount = 0; ocrAffinityCount(AFFINITY_PD, &affinityCount); assert(affinityCount >= 1);
    ocrGuid_t affinities[affinityCount]; ocrAffinityGet(AFFINITY_PD, &affinityCount, affinities);
    ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);

    // result vector
    ocrGuid_t r;
    double* ptr;
    ocrDbCreate(&r,(void**)&ptr,sizeof(double)*paramv[0],0,currentAffinity,NO_ALLOC);

    int index[affinityCount];
    indices(index, affinityCount, paramv[0], paramv[1]);

    //printf("[assign_edt] out GUID: %#017x\n", r);
    // Delete the output DB from rowvec_edt
    u64 i = 0;
    u64 j;
    int numberElemFromAffinity = ( (i==0)?index[i]:index[i]-index[i-1] ) * paramv[1];
    for(i = 0; i<depc; ++i, ptr+=numberElemFromAffinity) {
      numberElemFromAffinity = ( (i==0)?index[i]:index[i]-index[i-1] ) * paramv[1];

      //    printf("[assign] numberElemFromAffinity %d :  startIndex %d \n", numberElemFromAffinity, ptr);
      //      for(j = 0; j<numberElemFromAffinity; ++j,++ptr)
      //  *ptr = ((double*)depv[i].ptr)[j];
      memcpy(ptr,depv[i].ptr,sizeof(double)*numberElemFromAffinity);
      //printf("[assign] endIndex %d \n", j);
      //ocrDbDestroy(depv[i].guid);
    }

    //    ocrDbRelease(r);
    //    printf("[assign] DB Release \n");
    ocrEventSatisfy(paramv[2],r);
    //printf("[assign] Event Satisfied \n");
    return NULL_GUID;
}

// depv: c x // args blk, deps a[e],p
ocrGuid_t rowvec_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    u64 affinityCount = 0; ocrAffinityCount(AFFINITY_PD, &affinityCount); assert(affinityCount >= 1);
    ocrGuid_t affinities[affinityCount]; ocrAffinityGet(AFFINITY_PD, &affinityCount, affinities);
    ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);

    ocrGuid_t r; void* dbPtr;
    ocrDbCreate(&r,(void**)&dbPtr,sizeof(double)*paramv[0],0,currentAffinity,NO_ALLOC);
    u32* rows = (u32*)depv[0].ptr;
    //printf("[rowvec_edt] in0 GUID: %#017x\n", depv[0].guid);
    //printf("[rowvec_edt] in1 GUID: %#017x\n", depv[1].guid);
    //printf("[rowvec_edt] out GUID: %#017x\n", r);
    double* values = ((double*)depv[0].ptr)+((paramv[0]+2)>>1);
    u32* indexes = rows+rows[paramv[0]];
    u64 i;
    double* r_ptr = (double*) dbPtr;
    for(i = 0; i<paramv[0]; ++i) {
      r_ptr[i] = _dotg(rows[i],values,indexes,depv[1].ptr);
      values += rows[i]; indexes += rows[i];
    }
    //ocrDbRelease(r);
    return r;
}

// depv: param:n,blk A x param:out //  args {n, blk, aff_index, assgn_guid,} dep a,p
ocrGuid_t join_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    u64 affinityCount = 0; ocrAffinityCount(AFFINITY_PD, &affinityCount); assert(affinityCount >= 1);
    ocrGuid_t affinities[affinityCount]; ocrAffinityGet(AFFINITY_PD, &affinityCount, affinities);
    ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);

    int index[affinityCount];
    indices(index, affinityCount, paramv[0], paramv[1]);
    ocrGuid_t affinityIndex = paramv[2];
    int numerBlocks = (affinityIndex==0)?index[affinityIndex]:index[affinityIndex]-index[affinityIndex-1];

    // result vector
    ocrGuid_t r;
    double* ptr;
    ocrDbCreate(&r,(void**)&ptr,sizeof(double)*numerBlocks*paramv[1],0,affinities[0],NO_ALLOC);

    //printf("[assign_edt] out GUID: %#017x\n", r);
    // Delete the output DB from rowvec_edt
    u64 i,j;
    for(i = 0; i<depc; ++i, ptr+=paramv[1]) {
      //printf("[join]  Affinity [%d] :  event_guid %d \n", paramv[2], paramv[4]);
      memcpy(ptr,depv[i].ptr,sizeof(double)*paramv[1]);
      //      ocrDbDestroy(depv[i].guid);
    }
    ocrDbRelease(r);

    ocrEventSatisfy(paramv[4],r);

    //printf("[join]  Finished\n");

    return NULL_GUID;
}

// depv: param:n,blk A x param:out //  args {n, blk, aff_index, event_guid} dep a,p
ocrGuid_t spawn_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    u64 affinityCount = 0; ocrAffinityCount(AFFINITY_PD, &affinityCount); assert(affinityCount >= 1);
    ocrGuid_t affinities[affinityCount]; ocrAffinityGet(AFFINITY_PD, &affinityCount, affinities);
    ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);

    // Parameter reading
    u64 nb                  = paramv[0];
    u32 blk                 = paramv[1];
    ocrGuid_t affinityIndex = paramv[2];
    ocrGuid_t assgn         = paramv[3];
    int index[affinityCount];
    indices(index, affinityCount, nb, blk);
    int numerBlocks = (affinityIndex==0)?index[affinityIndex]:index[affinityIndex]-index[affinityIndex-1];
    int offset      = (affinityIndex==0)?0:index[affinityIndex-1];
    //printf("[spawn] Affinity [%d] :  offset %d : numerBlocks %d \n", affinityIndex, offset, numerBlocks);

    // Join EDT and dependence to assng
    ocrGuid_t joint, join;
    ocrEdtTemplateCreate(&joint,join_edt,5,numerBlocks);
    ocrEdtCreate(&join,joint,5,paramv,numerBlocks,NULL,0,currentAffinity,NULL_GUID);
    ocrEdtTemplateDestroy(joint);

    //ocrAddDependence(joine,event,0,DB_MODE_RO);

    // Vector copy on remote node
    ocrGuid_t p; double* dbPtrRemote;
    ocrDbCreate(&p,(void**)&dbPtrRemote,sizeof(double)*nb,0,currentAffinity,NO_ALLOC);
    memcpy(dbPtrRemote,depv[1].ptr,sizeof(double)*nb);
    //_copy(nb, dbPtrRemote, depv[1].ptr);

    // Creating rowvec_edts
    ocrGuid_t rowvect;
    ocrEdtTemplateCreate(&rowvect,rowvec_edt,1,2);

    int e = 0;
    for(e = 0; e < numerBlocks; ++e) {
      //        printf("[spawn] DB index [%d] :  Affinity %d \n", e+offset, affinityIndex);
        ocrGuid_t rv,rve;
        ocrEdtCreate(&rv,rowvect,1,paramv+1,2,NULL,0,currentAffinity,&rve);
        ocrAddDependence(rve,join,e,DB_MODE_RO);
        ocrAddDependence(((ocrGuid_t*)depv[0].ptr)[e+offset],rv,0,DB_MODE_RO);
        ocrAddDependence(p,rv,1,DB_MODE_RO);
    }

    //ocrDbRelease(depv[1].guid);
    ocrEdtTemplateDestroy(rowvect);

    return NULL_GUID;
}

// depv: param:n,blk A x param:out //  args {n, blk, out_q*} dep a,p
ocrGuid_t spmv_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	u64 affinityCount = 0; ocrAffinityCount(AFFINITY_PD, &affinityCount); assert(affinityCount >= 1);
  	ocrGuid_t affinities[affinityCount]; ocrAffinityGet(AFFINITY_PD, &affinityCount, affinities);
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);

    u64 e;

    ocrGuid_t assgnt,assgn;
    ocrEdtTemplateCreate(&assgnt,assign_edt,3,affinityCount);
    ocrEdtCreate(&assgn,assgnt,3,paramv,affinityCount,NULL,0,currentAffinity,NULL);
    ocrEdtTemplateDestroy(assgnt);

    ocrGuid_t spawnt;
    ocrEdtTemplateCreate(&spawnt,spawn_edt,5,2);

    ocrGuid_t spawnEv[affinityCount];
    for(e = 0; e < affinityCount; ++e) {
        ocrEventCreate(&spawnEv[e], OCR_EVENT_STICKY_T, 1);
        u64 paramv3[5] = {paramv[0],paramv[1], e, assgn, spawnEv[e]};

        ocrGuid_t spawn;
        ocrEdtCreate(&spawn,spawnt,5,paramv3,2,NULL,0,affinities[e],NULL_GUID);
        ocrAddDependence(spawnEv[e],  assgn,e,DB_MODE_RO);
        ocrAddDependence(depv[0].guid,spawn,0,DB_MODE_RO);
        ocrAddDependence(depv[1].guid,spawn,1,DB_MODE_RO);
    }

    ocrEdtTemplateDestroy(spawnt);
    ocrDbRelease(depv[1].guid);

    return NULL_GUID;
}

// depv: r x rr pp, out dist
ocrGuid_t dist_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t d; double* d_ptr;
    ocrDbCreate(&d,(void**)&d_ptr,sizeof(double),0,currentAffinity,NO_ALLOC);
    *d_ptr = _dist(paramv[0],(double*)depv[0].ptr,(double*)depv[1].ptr);
    ocrDbDestroy(depv[0].guid);
    ocrDbDestroy(depv[2].guid);
    ocrDbDestroy(depv[3].guid);

    return d;
}
