#include <stdlib.h>
#include <stdio.h>
#include <ocr.h>

#include "cg_ocr.h"

ocrGuid_t square(ocrGuid_t x, ocrGuid_t* rho)
{
    ocrGuid_t se,tmp;
    ocrEdtTemplateCreate(&tmp,square_edt,0,2);
    ocrEdtCreate(&se,tmp,0,NULL,2,NULL,0,NULL_GUID,rho);
    ocrAddDependence(x,se,1,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
    return se;
}

void alphas(ocrGuid_t n, ocrGuid_t rho, ocrGuid_t p, ocrGuid_t q, ocrGuid_t* al, ocrGuid_t* nal)
{
    ocrGuid_t ae,tmp;
    ocrEventCreate(nal,OCR_EVENT_ONCE_T,true);
    ocrGuid_t edb, *eptr;
    ocrDbCreate(&edb, (void**)&eptr, sizeof(ocrGuid_t),0,NULL_GUID, NO_ALLOC);
    *eptr = *nal;
    ocrEdtTemplateCreate(&tmp,alphas_edt,0,5);
    ocrEdtCreate(&ae,tmp,0,NULL,5,NULL,0,NULL_GUID,al);
    ocrAddDependence(n,ae,0,DB_MODE_RO);
    ocrAddDependence(rho,ae,1,DB_MODE_RO);
    ocrAddDependence(p,ae,2,DB_MODE_RO);
    ocrAddDependence(q,ae,3,DB_MODE_RO);
    ocrAddDependence(edb,ae,4,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}

void alpha(ocrGuid_t n, ocrGuid_t rho, ocrGuid_t p, ocrGuid_t q, ocrGuid_t* al)
{
    ocrGuid_t ae,tmp;
    ocrEdtTemplateCreate(&tmp,alpha_edt,0,4);
    ocrEdtCreate(&ae,tmp,0,NULL,4,NULL,0,NULL_GUID,al);
    ocrAddDependence(n,ae,0,DB_MODE_RO);
    ocrAddDependence(rho,ae,1,DB_MODE_RO);
    ocrAddDependence(p,ae,2,DB_MODE_RO);
    ocrAddDependence(q,ae,3,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}

ocrGuid_t fspMv(ocrGuid_t a, ocrGuid_t p, ocrGuid_t* q)
{
    ocrGuid_t spmv,tmp;
    ocrEventCreate(q,OCR_EVENT_ONCE_T,true);
    ocrGuid_t edb, *eptr;
    ocrDbCreate(&edb, (void**)&eptr, sizeof(ocrGuid_t),0,NULL_GUID, NO_ALLOC);
    *eptr = *q;
    ocrEdtTemplateCreate(&tmp,spmv_edt,0,4);
    ocrEdtCreate(&spmv,tmp,0,NULL,4,NULL,0,NULL_GUID,NULL);
    ocrAddDependence(a,spmv,1,DB_MODE_RO);
    ocrAddDependence(p,spmv,2,DB_MODE_RO);
    ocrAddDependence(edb,spmv,3,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);

    return spmv;
}

void spMv(ocrGuid_t n, ocrGuid_t a, ocrGuid_t p, ocrGuid_t* q)
{
    ocrGuid_t spmv,tmp;
    ocrEventCreate(q,OCR_EVENT_ONCE_T,true);
    ocrGuid_t edb, *eptr;
    ocrDbCreate(&edb, (void**)&eptr, sizeof(ocrGuid_t),0,NULL_GUID, NO_ALLOC);
    *eptr = *q;
    ocrEdtTemplateCreate(&tmp,spmv_edt,0,4);
    ocrEdtCreate(&spmv,tmp,0,NULL,4,NULL,0,NULL_GUID,NULL);
    ocrAddDependence(n,spmv,0,DB_MODE_RO);
    ocrAddDependence(a,spmv,1,DB_MODE_RO);
    ocrAddDependence(p,spmv,2,DB_MODE_RO);
    ocrAddDependence(edb,spmv,3,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}

void scale(ocrGuid_t n, ocrGuid_t a, ocrGuid_t x, ocrGuid_t* z)
{
    ocrGuid_t se,tmp;
    ocrEdtTemplateCreate(&tmp,scale_edt,0,3);
    ocrEdtCreate(&se,tmp,0,NULL,3,NULL,0,NULL_GUID,z);
    ocrAddDependence(n,se,0,DB_MODE_RO);
    ocrAddDependence(a,se,1,DB_MODE_RO);
    ocrAddDependence(x,se,2,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}

void daxpy(ocrGuid_t n, ocrGuid_t z, ocrGuid_t a, ocrGuid_t p, ocrGuid_t* zo)
{
    ocrGuid_t de,tmp;
    ocrEdtTemplateCreate(&tmp,daxpy_edt,0,4);
    ocrEdtCreate(&de,tmp,0,NULL,4,NULL,0,NULL_GUID,zo);
    ocrAddDependence(n,de,0,DB_MODE_RO);
    ocrAddDependence(z,de,1,DB_MODE_ITW);
    ocrAddDependence(a,de,2,DB_MODE_RO);
    ocrAddDependence(p,de,3,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}

void daxpyl(ocrGuid_t n, ocrGuid_t z, ocrGuid_t a, ocrGuid_t p, ocrGuid_t* zo)
{
    ocrGuid_t de,tmp;
    ocrEdtTemplateCreate(&tmp,daxpyl_edt,0,4);
    ocrEdtCreate(&de,tmp,0,NULL,4,NULL,0,NULL_GUID,zo);
    ocrAddDependence(n,de,0,DB_MODE_RO);
    ocrAddDependence(z,de,1,DB_MODE_ITW);
    ocrAddDependence(a,de,2,DB_MODE_RO);
    ocrAddDependence(p,de,3,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}

void distance(ocrGuid_t n, ocrGuid_t r, ocrGuid_t x, ocrGuid_t rr, ocrGuid_t pp, ocrGuid_t* d)
{
    ocrGuid_t de,tmp;
    ocrEdtTemplateCreate(&tmp,dist_edt,0,5);
    ocrEdtCreate(&de,tmp,0,NULL,5,NULL,0,NULL_GUID,d);
    ocrAddDependence(n,de,0,DB_MODE_RO);
    ocrAddDependence(r,de,1,DB_MODE_RO);
    ocrAddDependence(x,de,2,DB_MODE_RO);
    ocrAddDependence(rr,de,3,DB_MODE_RO);
    ocrAddDependence(pp,de,4,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}

void update(ocrGuid_t n, ocrGuid_t na, ocrGuid_t p, ocrGuid_t q, ocrGuid_t r, ocrGuid_t rr, ocrGuid_t pp, ocrGuid_t rho, ocrGuid_t* po, ocrGuid_t* ppo)
{
    ocrEventCreate(ppo,OCR_EVENT_ONCE_T,true);
    ocrGuid_t edb, *eptr;
    ocrDbCreate(&edb, (void**)&eptr, sizeof(ocrGuid_t),0,NULL_GUID, NO_ALLOC);
    *eptr = *ppo;
    ocrGuid_t de,tmp;
    ocrEdtTemplateCreate(&tmp,update_edt,0,9);
    ocrEdtCreate(&de,tmp,0,NULL,9,NULL,0,NULL_GUID,po);
    ocrAddDependence(n,de,0,DB_MODE_RO);
    ocrAddDependence(na,de,1,DB_MODE_RO);
    ocrAddDependence(p,de,2,DB_MODE_RO);
    ocrAddDependence(q,de,3,DB_MODE_RO);
    ocrAddDependence(r,de,4,DB_MODE_ITW);
    ocrAddDependence(rr,de,5,DB_MODE_ITW);
    ocrAddDependence(pp,de,6,DB_MODE_ITW);
    ocrAddDependence(rho,de,7,DB_MODE_ITW);
    ocrAddDependence(edb,de,8,DB_MODE_ITW);
    ocrEdtTemplateDestroy(tmp);
}
