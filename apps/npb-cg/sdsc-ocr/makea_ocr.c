#include <stdlib.h>
#include <string.h>
#include <strings.h>
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
    ocrDbCreate(a, (void**)&a_ptr, sizeof(ocrGuid_t)*class->na/class->blk, 0, NULL_GUID, NO_ALLOC);
    for(i=0; i<class->na; ++i)
      nzcount[i] = 0;
    u64 nza = 0;
    ac = acol;
    for(i=0; i<class->na; ++i) {
      u32 j;
      for(j=0; j<arow[i]; ++j,++ac) {
        nzcount[*ac] += arow[i];
        nza += arow[i];
      }
    }
    u32 even = class->blk+1+(1-class->blk&1);
    for(i=0; i<class->na/class->blk; ++i) {
      u32 b; u32 counts = 0;
      for(b=0; b<class->blk; ++b)
        counts += nzcount[i*class->blk+b];
      ocrDbCreate(a_ptr+i, (void**)rows+i, sizeof(double)*counts+sizeof(u32)*(counts+even), 0, NULL_GUID, NO_ALLOC);
      rows[i][class->blk] = counts<<1;
      for(b=0; b<class->blk; ++b)
        rows[i][b] = nzcount[i*class->blk+b];
      rows[i][class->blk] += even;
    }
    for(i=0; i<class->na/class->blk; ++i) {
      u32 b, j, offset=0;
      for(b=0; b<class->blk; ++b)
        offset += rows[i][b];
      offset += rows[i][class->blk];
      for(j=rows[i][class->blk]; j<offset; ++j)
        rows[i][j] = NEGATIVE;
    }

    if(nza>(class->na*(class->nonzer+1)*(class->nonzer+1))) {
        PRINTF("Space for matrix elements exceeded in sparse %lu > %lu\n", nza, class->na*(u64)(class->nonzer+1)*(class->nonzer+1));
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

                u32 jj0 = j0/class->blk;
                u32* cols = rows[jj0]+rows[jj0][class->blk];
                double* vals = (double*)(rows[jj0]+even);
                u32 b;
                for(b=0; b<j0%class->blk; ++b) {
                   cols += rows[jj0][b];
                   vals += rows[jj0][b];
                }
                u32 vn = rows[jj0][j0%class->blk];
                u32 k;
                for(k=0; k<vn; ++k) {
                   if(cols[k] > j1 && cols[k]<NEGATIVE) {
                       int kk;
                       for(kk=vn-2; kk>=(int)k; --kk)
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
    for(i=0; i<class->na/class->blk; ++i) {
      u32 b, j0=even, j1=even;
      for(b=0; b<class->blk-1; ++b) {
        j0 += nzcount[i*class->blk+b]<<1;
        j1 += rows[i][b]<<1;
        u32 kk = nzcount[i*class->blk+b+1];
        double* dst = (double*)(rows[i]+j0);
        double* src = (double*)(rows[i]+j1);
        u32 k;
        for(k = 0; k<kk; ++k)
          dst[k] = src[k];
      }
      j0 += nzcount[i*class->blk+class->blk-1]<<1;
      j1 = rows[i][class->blk];
      rows[i][class->blk] = j0;
      for(b=0; b<class->blk; ++b) {
        u32 kk = j1+nzcount[i*class->blk+b];
        u32 k;
        for(k = j1; k<kk; ++k, ++j0)
          rows[i][j0] = rows[i][k];
        j1 += rows[i][b];
        rows[i][b] = nzcount[i*class->blk+b];
        nza += nzcount[i*class->blk+b];
      }
    }

    PRINTF("number of nonzeros = %lu\n", nza);

    ocrDbDestroy(acolid);
    ocrDbDestroy(aeltid);
    ocrDbDestroy(arowid);
    ocrDbDestroy(nzcountid);
    ocrDbDestroy(randdbid);
    ocrDbDestroy(rowsid);

    return 0;
}
