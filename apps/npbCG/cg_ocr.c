#include <ocr.h>

#include "cg_ocr.h"

void conj_grad(u32 nn, ocrGuid_t n, ocrGuid_t a, ocrGuid_t x, ocrGuid_t cont, u32 dep)
{
    ocrGuid_t p,pp,r; void* ptr;
    ocrDbCreate(&r,&ptr,sizeof(double)*nn,DB_PROP_NO_ACQUIRE,NULL_GUID,NO_ALLOC);
    ocrDbCreate(&p,&ptr,sizeof(double)*nn,DB_PROP_NO_ACQUIRE,NULL_GUID,NO_ALLOC);

    ocrGuid_t rho,q,z,al,nal;
    ocrGuid_t first=square(x,&rho);                    // rho=x^2
    ocrGuid_t second = fspMv(a,x,&q);                  // q=Ap
    alphas(n,rho,x,q,&al,&nal);                        // alpha=rho/pq,nalpha=-alpha
    scale(n,al,x,&z);                                  // z=alpha x
    update(n,nal,p,q,r,x,x,rho,&p,&pp);                // r=r-alpha q,rho0=r^2,beta=rho0/rho,rho=rho0,p=r+beta p

    uint32_t it;
    for(it=1; it<24; ++it) {
      spMv(n,a,p,&q);                                  // q=Ap
      alphas(n,rho,p,q,&al,&nal);                      // alpha=rho/pq,nalpha=-alpha
      daxpy(n,z,al,pp,&z);                             // z=z+alpha p
      update(n,nal,p,q,r,r,p,rho,&p,&pp);              // r=r-alpha q,rho0=r^2,beta=rho0/rho,rho=rho0,p=r+beta p
    }

    spMv(n,a,p,&q);                                    // q=Ap
    alpha(n,rho,p,q,&al);                              // alpha=rho/pq
    daxpy(n,z,al,pp,&z);                               // z=z+alpha p
    spMv(n,a,z,&q);                                    // r=Az
    distance(n,q,x,r,p,&p);                            // rnorm=||x-r||

    ocrAddDependence(z, cont, dep, DB_MODE_RO);
    ocrAddDependence(p, cont, dep+1, DB_MODE_RO);

    ocrAddDependence(n, first, 0, DB_MODE_RO);
    ocrAddDependence(n, second, 0, DB_MODE_RO);
}
