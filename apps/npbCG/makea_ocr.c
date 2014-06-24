#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ocr.h>

#include "cg_ocr.h"

#define NEGATIVE 0xFFFFFFFF

// generate nz values for different indexes, which are stored in iv,
// and ensures that the index di is present or adds it with value dv,
// in which case there will be nz+1 values and nz is incremented accordingly
static void sprnvc(u32 n, u32* nz, u32 nn1, double* v, u32* iv, u32 di, double dv, randdb_t* randdb)
{
    int nzv = 0;
    while(nzv<*nz) {
        u32 i=n;
        while(i>=n)
            i = randi(randdb, nn1);
        u32 r = 0;
        while(r<nzv && iv[r]!=i) ++r;
        if(r==nzv) {
            v[nzv] = randd(randdb);
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

int makea(classdb_t* class, ocrGuid_t* a)
{
    u32 nn1 = 1;
    for(nn1=1; nn1<class->na; nn1<<=1);

    ocrGuid_t acolid, arowid, nzcountid, aeltid, randdbid, rowsid;
    u32* acol; // store generated indexes
    u32* arow; // count of generated value index pairs
    u32* nzcount;
    u32** rows;
    double* aelt; // store generated values
    ocrDbCreate(&acolid, (void**)&acol, sizeof(u32)*class->na*(class->nonzer+1),
                0, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&arowid, (void**)&arow, sizeof(u32)*class->na,
                0, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&nzcountid, (void**)&nzcount, sizeof(u32)*class->na,
                0, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&aeltid, (void**)&aelt, sizeof(double)*class->na*(class->nonzer+1),
                0, NULL_GUID, NO_ALLOC);
    ocrDbCreate(&rowsid, (void**)&rows, sizeof(u32*)*class->na,
                0, NULL_GUID, NO_ALLOC);
    bzero(nzcount, sizeof(u32)*class->na);

    randdb_t* randdb;
    rand_init(&randdb, &randdbid);

    double* ae = aelt;
    u32* ac = acol;
    int i;
    for(i=0; i<class->na; ++i) {
        arow[i] = class->nonzer;
        sprnvc(class->na, arow+i, nn1, ae, ac, i, 0.5, randdb);
        ac += arow[i];
        ae += arow[i];
    }

    ocrGuid_t* a_ptr;
    ocrDbCreate(a, (void**)&a_ptr, sizeof(ocrGuid_t)*class->na, 0, NULL_GUID, NO_ALLOC);
    for(i=0; i<class->na; ++i)
      nzcount[i] = 0;
    u64 nza = 0;
    ac = acol;
    for(i=0; i<class->na; ++i) {
      int j;
      for(j=0; j<arow[i]; ++j,++ac) {
         nzcount[*ac] += arow[i];
         nza += arow[i];
      }
    }
    for(i=0; i<class->na; ++i) {
      ocrDbCreate(a_ptr+i, (void**)rows+i, sizeof(u64)*(nzcount[i]+1)+sizeof(u32)*nzcount[i], 0, NULL_GUID, NO_ALLOC);
      rows[i][0] = nzcount[i];
    }
    for(i=0; i<class->na; ++i) {
      int j;
      for(j=(rows[i][0]+1)<<1; j<((rows[i][0]+1)<<1)+rows[i][0]; ++j)
        rows[i][j] = NEGATIVE;
    }

    if(nza>(class->na*(class->nonzer+1)*(class->nonzer+1))) {
        printf("Space for matrix elements exceeded in sparse %lu > %lu\n", nza, class->na*(u64)(class->nonzer+1)*(class->nonzer+1));
        ocrDbDestroy(acolid);
        ocrDbDestroy(aeltid);
        ocrDbDestroy(arowid);
        ocrDbDestroy(nzcountid);
        ocrDbDestroy(rowsid);
        for(i=0; i<class->na; ++i)
          ocrDbDestroy(a_ptr[i]);
        ocrDbDestroy(*a);
        ocrDbDestroy(randdbid);
        return -1;
    }

    double size = 1;
    double rcond = 0.1;
    double ratio = pow(rcond,1.0/class->na);

    ac = acol;
    ae = aelt;
    for(i=0; i<class->na; ++i) {
        int j;
        for(j=0; j<arow[i]; ++j) {
            u32 j0 = ac[j];
            double scale = size*ae[j];
            u32 jj;
            for(jj=0; jj<arow[i]; ++jj) {
                int j1 = ac[jj];
                double va = ae[jj]*scale;
                if(j1==j0 && j1 == i)
                   va += rcond-class->shift;

                u32* cols = rows[j0]+((rows[j0][0]+1)<<1);
                double* vals = (double*)(rows[j0]+2);
                u32 k;
                for(k=0; k<rows[j0][0]; ++k) {
                   if(cols[k] > j1 && cols[k]<NEGATIVE) {
                       int kk;
                       for(kk=rows[j0][0]-2; kk>=(int)k; --kk)
                           if(cols[kk]<NEGATIVE) {
                               vals[kk+1] = vals[kk];
                               cols[kk+1] = cols[kk];
                           }
                       cols[k] = j1;
                       vals[k] = 0;
                       break;
                   }
                   else if(cols[k] == NEGATIVE) {
                       cols[k] = j1;
                       break;
                   }
                   else if(cols[k] == j1) {
                       --nzcount[j0];
                       break;
                   }
                }
                vals[k] += va;
            }
        }
        ac += arow[i]; ae += arow[i];
        size *= ratio;
    }

    nza = 0;
    for(i=0; i<class->na; ++i) {
       u32 kk = (1+rows[i][0])<<1;
       rows[i][0] = nzcount[i];
       u32 j0 = (1+nzcount[i])<<1;
       u32 j1 = j0+nzcount[i];
       u32 k;
       for(k = j0; k<j1; ++k,++kk)
         rows[i][k] = rows[i][kk];
       nza += rows[i][0];
    }

    printf("number of nonzeros = %lu\n", nza);

    ocrDbDestroy(acolid);
    ocrDbDestroy(aeltid);
    ocrDbDestroy(arowid);
    ocrDbDestroy(nzcountid);
    ocrDbDestroy(randdbid);
    ocrDbDestroy(rowsid);

    return 0;
}
