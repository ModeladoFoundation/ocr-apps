#include <stdlib.h>
#include <stdio.h>
#include <ocr.h>

#include "cg_ocr.h"

ocrGuid_t square(u64 n, ocrGuid_t* rho)
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t se,tmp;
    ocrEdtTemplateCreate(&tmp,square_edt,1,1);
    ocrEdtCreate(&se,tmp,1,&n,1,NULL,0,currentAffinity,rho);
    ocrEdtTemplateDestroy(tmp);
    return se;
}

void alphas(u64 n, ocrGuid_t rho, ocrGuid_t p, ocrGuid_t q, ocrGuid_t* al, ocrGuid_t* nal)
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t ae,tmp;
    ocrEventCreate(nal,OCR_EVENT_ONCE_T,true);
    u64 paramv[2] = {n, *nal};
    ocrEdtTemplateCreate(&tmp,alphas_edt,2,3);
    ocrEdtCreate(&ae,tmp,2,paramv,3,NULL,0,currentAffinity,al);
    ocrAddDependence(rho,ae,0,DB_MODE_RO);
    ocrAddDependence(p,ae,1,DB_MODE_RO);
    ocrAddDependence(q,ae,2,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}

void alpha(u64 n, ocrGuid_t rho, ocrGuid_t p, ocrGuid_t q, ocrGuid_t* al)
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t ae,tmp;
    ocrEdtTemplateCreate(&tmp,alpha_edt,1,3);
    ocrEdtCreate(&ae,tmp,1,&n,3,NULL,0,currentAffinity,al);
    ocrAddDependence(rho,ae,0,DB_MODE_RO);
    ocrAddDependence(p,ae,1,DB_MODE_RO);
    ocrAddDependence(q,ae,2,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}

ocrGuid_t fspMv(u64 n, u64 blk, ocrGuid_t p, ocrGuid_t* q)
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t spmv,tmp;
    ocrEventCreate(q,OCR_EVENT_STICKY_T,true);
    u64 paramv[3] = {n, blk, *q};
    ocrEdtTemplateCreate(&tmp,spmv_edt,3,2);
    ocrEdtCreate(&spmv,tmp,3,paramv,2,NULL,0,currentAffinity,NULL);
    ocrAddDependence(p,spmv,1,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);

    return spmv;
}

void spMv(u64 n, u64 blk, ocrGuid_t a, ocrGuid_t p, ocrGuid_t* q)
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t spmv,tmp;
    ocrEventCreate(q,OCR_EVENT_STICKY_T,true);
    u64 paramv[3] = {n, blk, *q};
    ocrEdtTemplateCreate(&tmp,spmv_edt,3,2);
    ocrEdtCreate(&spmv,tmp,3,paramv,2,NULL,0,currentAffinity,NULL);
    ocrAddDependence(a,spmv,0,DB_MODE_RO);
    ocrAddDependence(p,spmv,1,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}

void scale(u64 n, ocrGuid_t a, ocrGuid_t x, ocrGuid_t* z)
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t se,tmp;
    ocrEdtTemplateCreate(&tmp,scale_edt,1,2);
    ocrEdtCreate(&se,tmp,1,&n,2,NULL,0,currentAffinity,z);
    ocrAddDependence(a,se,0,DB_MODE_RO);
    ocrAddDependence(x,se,1,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}

void daxpy(u64 n, ocrGuid_t z, ocrGuid_t a, ocrGuid_t p, ocrGuid_t* zo)
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t de,tmp;
    ocrEdtTemplateCreate(&tmp,daxpy_edt,1,3);
    ocrEdtCreate(&de,tmp,1,&n,3,NULL,0,currentAffinity,zo);
    ocrAddDependence(z,de,0,DB_MODE_ITW);
    ocrAddDependence(a,de,1,DB_MODE_RO);
    ocrAddDependence(p,de,2,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}
// distance(n,q=r,x,r=rr,p=pp,&p=d);                          // rnorm=||x-r||
void distance(u64 n, ocrGuid_t r, ocrGuid_t x, ocrGuid_t rr, ocrGuid_t pp, ocrGuid_t* d)
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrGuid_t de,tmp;
    ocrEdtTemplateCreate(&tmp,dist_edt,1,4);
    ocrEdtCreate(&de,tmp,1,&n,4,NULL,0,currentAffinity,d);
    ocrAddDependence(r,de,0,DB_MODE_RO);
    ocrAddDependence(x,de,1,DB_MODE_RO);
    ocrAddDependence(rr,de,2,DB_MODE_RO);
    ocrAddDependence(pp,de,3,DB_MODE_RO);
    ocrEdtTemplateDestroy(tmp);
}
// (n,nal,p,q,r,x,x,rho,&p,&pp);              // r=r-alpha q,rho0=r^2,beta=rho0/rho,rho=rho0,p=r+beta p
void update(u64 n, ocrGuid_t na, ocrGuid_t p, ocrGuid_t q, ocrGuid_t r, ocrGuid_t rr, ocrGuid_t pp, ocrGuid_t rho, ocrGuid_t* po, ocrGuid_t* ppo)
{
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
    ocrEventCreate(ppo,OCR_EVENT_ONCE_T,true);
    u64 paramv[2] = {n, *ppo};
    ocrGuid_t de,tmp;
    ocrEdtTemplateCreate(&tmp,update_edt,2,7);
    ocrEdtCreate(&de,tmp,2,paramv,7,NULL,0,currentAffinity,po);
    ocrAddDependence(na,de,0,DB_MODE_RO);
    ocrAddDependence(p,de,1,DB_MODE_RO);
    ocrAddDependence(q,de,2,DB_MODE_RO);
    ocrAddDependence(r,de,3,DB_MODE_ITW);
    ocrAddDependence(rr,de,4,DB_MODE_ITW);
    ocrAddDependence(pp,de,5,DB_MODE_ITW);
    ocrAddDependence(rho,de,6,DB_MODE_ITW);
    ocrEdtTemplateDestroy(tmp);
}
