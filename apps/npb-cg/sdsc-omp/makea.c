#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "cg.h"

#define NEGATIVE 0xFFFFFFFF

void sprnvc(uint32_t n, uint32_t* nz, uint32_t nn1, double* v, uint32_t* iv, uint32_t di, double dv)
{
    int nzv = 0;
    while(nzv<*nz) {
        uint32_t i=n;
        while(i>=n)
            i = randi(nn1);
        uint32_t r = 0;
        while(r<nzv && iv[r]!=i) ++r;
        if(r==nzv) {
            v[nzv] = randd();
            iv[nzv] = i;
            ++nzv;
            if(i==di)
                di=NEGATIVE;
        }
    }
    if(di<NEGATIVE) {
        v[*nz] = dv;
        iv[*nz] = di;
        ++*nz;
    }
}

int makea(class_t* class, double** a, uint32_t** colidx, uint64_t* rowstr)
{
    uint32_t nn1 = 1;
    for(nn1=1; nn1<class->na; nn1<<=1);

    uint32_t* acol = (uint32_t*)malloc(sizeof(uint32_t)*class->na*(class->nonzer+1));
    uint32_t* arow = (uint32_t*)malloc(sizeof(uint32_t)*class->na);
    uint64_t* nzcount = (uint64_t*)malloc(sizeof(uint64_t)*class->na);
    double* aelt = (double*)malloc(sizeof(double)*class->na*(class->nonzer+1));

    double* ae = aelt;
    uint32_t* ac = acol;
    int i;
    for(i=0; i<class->na; ++i) {
        arow[i] = class->nonzer;
        sprnvc(class->na, arow+i, nn1, ae, ac, i, 0.5);
        ac += arow[i];
        ae += arow[i];
    }

    bzero(rowstr, sizeof(uint64_t)*(class->na+1)); bzero(nzcount, sizeof(uint64_t)*class->na);

    ac = acol;
    rowstr[0] = 0;
    for(i=0; i<class->na; ++i) {
        int j;
        for(j=0; j<arow[i]; ++j,++ac)
            rowstr[*ac+1] += arow[i];
    }

    for(i=1; i<=class->na; ++i)
        rowstr[i] = rowstr[i]+rowstr[i-1];
    uint64_t nza = rowstr[class->na];

    uint64_t nz = (class->nonzer+1)*(class->nonzer+1);
    nz *= class->na;
    if(nza>nz) {
       printf("Space for matrix elements exceeded in sparse %lu > %lu\n", nza, nz);
       free(acol); free(aelt); free(arow); free(nzcount);
       return -1;
    }
    *a = (double*) malloc(sizeof(double)*nza); bzero(*a, sizeof(double)*nza);
    *colidx = (uint32_t*) malloc(sizeof(uint32_t)*nza);
    for(i=0; i<nza; ++i)
        (*colidx)[i] = NEGATIVE;

    double size = 1;
    double rcond = 0.1;
    double ratio = pow(rcond,1.0/class->na);

    ac = acol;
    ae = aelt;
    for(i=0; i<class->na; ++i) {
        int j;
        for(j=0; j<arow[i]; ++j) {
            uint32_t j0 = ac[j];
            double scale = size*ae[j];
            uint32_t jj;
            for(jj=0; jj<arow[i]; ++jj) {
                int j1 = ac[jj];
                double va = ae[jj]*scale;
                if(j1==j0 && j1 == i)
                   va += rcond-class->shift;

                uint32_t k;
                for(k=rowstr[j0]; k<rowstr[j0+1]; ++k) {
                   if((*colidx)[k] > j1 && (*colidx)[k]<NEGATIVE) {
                       int kk;
                       for(kk=rowstr[j0+1]-2; kk>=(int)k; --kk)
                           if((*colidx)[kk]<NEGATIVE) {
                               (*a)[kk+1] = (*a)[kk];
                               (*colidx)[kk+1] = (*colidx)[kk];
                           }
                       (*colidx)[k] = j1;
                       (*a)[k] = 0;
                       break;
                   }
                   else if((*colidx)[k] == NEGATIVE) {
                       (*colidx)[k] = j1;
                       break;
                   }
                   else if((*colidx)[k] == j1) {
                       ++nzcount[j0];
                       break;
                   }
                }
                (*a)[k] += va;
            }
        }
        ac += arow[i]; ae += arow[i];
        size *= ratio;
    }

    for(i=1; i<class->na; ++i)
        nzcount[i] = nzcount[i] + nzcount[i-1];

    for(i=0; i<class->na; ++i) {
        uint32_t j0 = i>0 ? rowstr[i]-nzcount[i-1] : 0;
        uint32_t j1 = rowstr[i+1]-nzcount[i];
        nza = rowstr[i];
        uint32_t k;
        for(k = j0; k<j1; ++k, ++nza) {
            (*a)[k] = (*a)[nza];
            (*colidx)[k] = (*colidx)[nza];
        }
        rowstr[i] = j0;
    }
    rowstr[class->na] = rowstr[class->na]-nzcount[class->na-1];

    printf("number of nonzeros = %lu\n", rowstr[class->na]);

    free(acol);
    free(aelt);
    free(arow);
    free(nzcount);

    return 0;
}
