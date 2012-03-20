#ifdef RAG_SIM
#include <string.h>

#include "xe-codelet.h"
#include "xe_memory.h"
#include "xe_console.h"
#include "xe_global.h"
#endif

#ifdef RAG_AFL
#include <string.h>

#include "codelet.h"
#include "rmd_afl_all.h"

#define xe_printf printf
#endif

#ifndef RMD_DB_MEM
#define RMD_DB_MEM(x,y) \
do { \
	u64 __retval__; \
	int __err__; \
	__retval__ = rmd_db_acquire((y),(0)); \
	__err__ = GET_STATUS(__retval__);        \
	if (__err__ != 0) { \
		xe_printf("db_acquire ERROR arg=%ld (%s) %s:%d\n", y.data, strerror(__err__), __FILE__, __LINE__); \
		exit(__err__); \
	} \
	__err__ = GET_ACCESS_MODE(__retval__); \
	*(x) = (void*)GET_ADDRESS(__retval__);  \
} while(0)
#endif

rmd_guid_t NULL_GUID = { .data = 0 };
#ifdef RAG_BLOCK
rmd_location_t    allocLoc = {	.type                     = RMD_LOC_TYPE_RELATIVE,
        			.data.relative.identifier = 0,
				.data.relative.level      = RMD_LOCATION_BLOCK   };
#else
rmd_location_t    allocLoc = {	.type                     = RMD_LOC_TYPE_RELATIVE,
        			.data.relative.identifier = 0,
				.data.relative.level      = RMD_LOCATION_CORE    };
#endif
#ifndef LOCAL
const int LOCAL = 0;
#endif

#define ALLOC_LOC (&allocLoc)


#ifdef RAG_SIM
#ifdef assert
#undef assert
#endif
#ifndef RAG_DEBUG
#define assert(x)
#else
#define assert(x) if(!(x)) {xe_printf("ASSERT () failed %s:%d\n",__FILE__, __LINE__); exit(x); }
#endif
#else
#include <assert.h>
#endif

// _clg => codelet guid
// _scg => scheduled codelet guid
// _dbg => data block guid
// _dbp => data block pointer
// _lcl => on stack data

#ifdef RAG_AFL
#define REM_LDX_ADDR(out_var,in_ptr,type) memcpy(&out_var,in_ptr,sizeof(type))
#define REM_STX_ADDR(out_ptr,in_var,type) memcpy(out_ptr,&in_var,sizeof(type))
#define REM_ST64_ADDR(out_ptr,in_var) memcpy(out_ptr,&in_var,8*sizeof(char))
#endif
//
//
// Reservoir proprietary, government purpose rights (UHPC program), "Foreground IP"
// vasilache@reservoir.com 11/24/2010
//

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <string.h>
#include <assert.h>

#include <fftw3.h>

#if defined(RAG_AFL) || defined(RAG_SIM)

// main codelet for rmd's runtime to start

rmd_guid_t main_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)
{
xe_printf("main_codelet arg = %ld, n_db = %d, db_ptr = %p, db %p\n",arg,n_db,db_ptr,db);
#else
int main()
{
#endif
  int Pof2 = 3;
  int SIZE  = 1<<Pof2;
  int size  = 8;

  fftwf_complex *z_in  = (fftwf_complex *)fftwf_malloc(size*sizeof(fftwf_complex));
  fftwf_complex *z_out = (fftwf_complex *)fftwf_malloc(size*sizeof(fftwf_complex));
  fftwf_complex *z_chk = (fftwf_complex *)fftwf_malloc(size*sizeof(fftwf_complex));

  fftwf_plan forward_plan  = fftwf_plan_dft_1d(size,z_in, z_out,FFTW_FORWARD, FFTW_ESTIMATE); 
  fftwf_plan backward_plan = fftwf_plan_dft_1d(size,z_out,z_chk,FFTW_BACKWARD,FFTW_ESTIMATE); 

  assert(z_in  != NULL);
  assert(z_out != NULL);
  assert(z_chk != NULL);

  // Modify this if you want different initializations  

  for (int i=0; i<size;i++) {
    z_in [i][0] = (float)(i+1);
    z_in [i][1] = (float)(i+1)/size;
    z_out[i][0] = -1;
    z_out[i][1] = -1;
    z_chk[i][0] = -1;
    z_chk[i][1] = -1;
  }
  for (int i=size; i<SIZE;i++) {
    z_in [i][0] =  0;
    z_in [i][1] =  0;
    z_out[i][0] = -1;
    z_out[i][1] = -1;
    z_chk[i][0] = -1;
    z_chk[i][1] = -1;
  }

#ifdef TRACE
  printf("Zr[] = ");
  for (int i=0;i<SIZE;i++) {
    printf("%lf ", z_in[i][0]);
    if(size > 8)printf("\n");
  }
  printf("\nZi[] = ");
  for (int i=0;i<SIZE;i++) {
    printf("%lf ", z_in[i][1]);
    if(size > 8)printf("\n");
  }
  printf("\n");
  printf("\n");
#endif

  fftwf_execute(forward_plan);
  
#ifdef TRACE
  printf("ZR[] = ");
  for (int i=0;i<SIZE;i++) {
    printf("%lf ", z_out[i][0]);
    if(size > 8)printf("\n");
  }
  printf("\nZI[] = ");
  for (int i=0;i<SIZE;i++) {
    printf("%lf ", z_out[i][1]);
    if(size > 8)printf("\n");
  }
  printf("\n");
  printf("\n");
#endif

   double SIZE_inv = 1.0/SIZE;
   for (int i=0;i<SIZE;i++) {
     z_out[i][0] *= SIZE_inv;
     z_out[i][1] *= SIZE_inv;
  }

  fftwf_execute(backward_plan);

#ifdef TRACE
  printf("zr[] = ");
  for (int i=0;i<SIZE;i++) {
    printf("%f ", z_chk[i][0]);
    if(size > 8)printf("\n");
  }
  printf("\nzi[] = ");
  for (int i=0;i<SIZE;i++) {
    printf("%f ", z_chk[i][1]);
    if(size > 8)printf("\n");
  }
  printf("\n");
  printf("\n");
#endif

  double err = 0;
  for(int i=0;i<size;i++) {
	double tr = z_chk[i][0] - z_in[i][0];
	double ti = z_chk[i][1] - z_in[i][1];
	tr *= tr;
	ti *= ti;
	err += sqrt( tr+ti );
  }
  printf("sum of cabs of error = %lf\n",err);

  fftwf_destroy_plan(forward_plan);
  fftwf_destroy_plan(backward_plan);

  fftwf_free(z_in);
  fftwf_free(z_out);
  fftwf_free(z_chk);

#if defined(RAG_AFL) || defined(RAG_SIM)
  rmd_complete();
  return NULL_GUID;
#else
  return 0;
#endif
}
