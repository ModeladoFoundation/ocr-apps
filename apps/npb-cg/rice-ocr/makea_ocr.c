#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ocr.h>
#include <assert.h>

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

void indices(int* index, u64 affinityCount, u64 n, u32 blk){
    u64 numberOfBlocks = n / blk;
    u64 blocksPerNode = numberOfBlocks / affinityCount;
    u64 res = numberOfBlocks - blocksPerNode * affinityCount;
    int i = 0;
    for(i=0;i<affinityCount;i++){
        if(i<res){
            index[i] = blocksPerNode + 1;
        }
        else{
            index[i] = blocksPerNode;
        }

        // Cumulative
        if(i==0)
          index[i] = index[i];
        else
          index[i] = index[i-1]+index[i];
    }
}

int makea(classdb_t* class, ocrGuid_t* a, char* writeFile, char* readFile)
{
	u64 affinityCount = 0; ocrAffinityCount(AFFINITY_PD, &affinityCount); assert(affinityCount >= 1);
  ocrGuid_t affinities[affinityCount]; ocrAffinityGet(AFFINITY_PD, &affinityCount, affinities);
	ocrGuid_t currentAffinity; ocrAffinityGetCurrent(&currentAffinity);
	printf("Affinity Count: %d\n", affinityCount);

    u32 nn1 = 1;
    for(nn1=1; nn1<class->na; nn1<<=1);

    ocrGuid_t acolid, arowid, nzcountid, aeltid, randdbid, rowsid;
    u32* acol; // store generated indexes
    u32* arow; // count of generated value index pairs
    u32* nzcount;
    u32** rows;
    double* aelt; // store generated values
    ocrDbCreate(&acolid, (void**)&acol, sizeof(u32)*class->na*(class->nonzer+1),
                0, currentAffinity, NO_ALLOC);
    ocrDbCreate(&arowid, (void**)&arow, sizeof(u32)*class->na,
                0, currentAffinity, NO_ALLOC);
    ocrDbCreate(&nzcountid, (void**)&nzcount, sizeof(u32)*class->na,
                0, currentAffinity, NO_ALLOC);
    ocrDbCreate(&aeltid, (void**)&aelt, sizeof(double)*class->na*(class->nonzer+1),
                0, currentAffinity, NO_ALLOC);
    ocrDbCreate(&rowsid, (void**)&rows, sizeof(u32*)*class->na,
                0, currentAffinity, NO_ALLOC);
    bzero(nzcount, sizeof(u32)*class->na);

    int index[affinityCount];
    indices(index, affinityCount, class->na, class->blk);

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

    int lb = 0;
    for(i=0;i<affinityCount;i++) {
      printf("Affinity [%d] gets DBs [%d:%d]\n", i, lb, index[i]);
      lb = index[i];
    }

    printf("Populating data: begin (takes a while)\n");

    ocrGuid_t* a_ptr;
    ocrDbCreate(a, (void**)&a_ptr, sizeof(ocrGuid_t)*class->na/class->blk, 0, currentAffinity, NO_ALLOC);
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
    int affinityDB   = 0;
    int indexCounter = 0;

    for(i=0; i<class->na/class->blk; ++i) {
      if(i>=index[indexCounter]){
        indexCounter++;
        affinityDB++;
      }
      // printf("DB index [%d] :  Affinity %d \n", i, affinityDB);
      u32 b; u32 counts = 0;
      for(b=0; b<class->blk; ++b)
        counts += nzcount[i*class->blk+b];
      ocrDbCreate(a_ptr+i, (void**)rows+i, sizeof(double)*counts+sizeof(u32)*(counts+even), 0, affinities[affinityDB], NO_ALLOC);
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

    /* ac = acol; */
    /* for(i=0; i<class->na; ++i) { */
    /*   u32 j; */
    /*   for(j=0; j<arow[i]; ++j,++ac) { */
    /*     printf("nzcount[%d] = %d\n", *ac, nzcount[*ac]); */
    /*   } */
    /* } */


    if(readFile != NULL){
      FILE *ptr_read_file;
      ptr_read_file = fopen(readFile, "rb");
      if(!ptr_read_file){
	printf("Unable to open the input file for reading!\n");
	return (-1);
      }
      printf("Reading data from a file %s\n", readFile);
      fread(nzcount, sizeof(u32)*class->na, 1, ptr_read_file);
      for(i=0; i<class->na/class->blk; ++i) {
	u32 b; u32 counts = 0;
	for(b=0; b<class->blk; ++b){
	  counts += nzcount[i*class->blk+b];
	  //printf ("nzcount[%d*%d+%d] = %d\n", i, class->blk, b, nzcount[i*class->blk+b]);
	}
	size_t size = sizeof(double)*counts+sizeof(u32)*(counts+even);
	//printf("Reading %d bytes from file for row %d, counts = %d, even = %d, class->blk = %d, class->na = %d\n", size, i, counts, even, class->blk, class->na);
	size_t size_read = fread(*((void**)rows+i), size, 1, ptr_read_file);
      }
      fclose(ptr_read_file);
      printf("Done reading from a file %s\n", readFile);
    }else{

      printf("Generation beginning\n");

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

      printf("Generation finished\n");

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

      for(i=0; i<class->na/class->blk; ++i) {
	ocrDbRelease(a_ptr[i]);
	// printf("[makea] out GUID[%d]: %#017x\n", i, a_ptr[i]);
      }

      printf("Populating data: done\n");
      printf("number of nonzeros = %lu\n", nza);

    }

    if(writeFile != NULL){
      FILE *ptr_write_file;
      ptr_write_file = fopen(writeFile, "wb");
      if(!ptr_write_file){
	printf("Unable to open the output file for writing!\n");
	return (-1);
      }
      printf("Writing data into file %s\n", writeFile);
      fwrite(nzcount, sizeof(u32)*class->na, 1, ptr_write_file);
      for(i=0; i<class->na/class->blk; ++i) {
	u32 b; u32 counts = 0;
	for(b=0; b<class->blk; ++b){
	  counts += nzcount[i*class->blk+b];
	  //printf ("nzcount[%d*%d+%d] = %d\n", i, class->blk, b, nzcount[i*class->blk+b]);
	}
	size_t size = sizeof(double)*counts+sizeof(u32)*(counts+even);
	//printf("Writing %d bytes into file for row %d, counts = %d, even = %d, class->blk = %d, class->na = %d\n", size, i, counts, even, class->blk, class->na);
	size_t size_written = fwrite(*((void**)rows+i), size, 1, ptr_write_file);

	
      }
      fclose(ptr_write_file);
      printf("Done writing into file %s\n", writeFile);
    }

    ocrDbDestroy(acolid);
    ocrDbDestroy(aeltid);
    ocrDbDestroy(arowid);
    ocrDbDestroy(nzcountid);
    ocrDbDestroy(randdbid);
    ocrDbDestroy(rowsid);

    return 0;
}
