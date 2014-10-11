//
// Roger A. Golliver Intel 2012 for UHPC project
//
// Simple wrappers to change SAR's fftw calls to fft based on Reservoir's fft
//

#include "ocr.h"
#include "rag_ocr.h"
#include "fftw3.h"

static int Ceiling_Power_Of_Two(int n) {
	int Pof2 = 0;
	while( (1<<(Pof2++)) < n)
		;
	return (Pof2-1);
}

fftwf_plan fftwf_plan_dft_1d(int size, fftwf_complex *in, fftwf_complex *out, int dir, int est)
{
	fftwf_plan plan;
	plan.dir  = dir;
	plan.size = size;
	plan.Pof2 = Ceiling_Power_Of_Two(size);
	plan.SIZE = 1<<plan.Pof2;
	plan.in   = in;
	plan.out  = out;
	assert( (plan.dir == FFTW_FORWARD) || (plan.dir == FFTW_BACKWARD) );
	assert( plan.SIZE == plan.size );
	assert( plan.in   != NULL );
	assert( plan.out  != NULL );
	return plan;
};

void fftwf_destroy_plan(fftwf_plan plan)
{
	plan.dir  = 0;
	plan.size = 0;
	plan.Pof2 = 0;
	plan.SIZE = 0;
	plan.in   = NULL;
	plan.out  = NULL;
}

fftwf_complex *fftwf_malloc(ocrGuid_t *dbg, size_t size)
{
	size = 1<<Ceiling_Power_Of_Two(size);
	fftwf_complex *retval;
	retval = (fftwf_complex *)spad_malloc(dbg,size);
	assert(retval != NULL);
	return retval;
};

void fftwf_free(fftwf_complex *ptr, ocrGuid_t dbg)
{
	spad_free(ptr,dbg);
	return;
};

//
// Reservoir proprietary, government purpose rights (UHPC program), "Foreground IP"
// vasilache@reservoir.com 11/24/2010
//


void fftwf_execute(fftwf_plan plan)
{
  int i1,i2,l1,l2;
  double cR,cI,tR,tI,uR,uI,zt;

  const int dir  = plan.dir;
  const int Pof2 = plan.Pof2;
  const int SIZE = plan.SIZE;
  fftwf_complex * restrict z_in  = plan.in;
  fftwf_complex * restrict z_out = plan.out;

  /* Copy input to output and do inplace in output */

  SPADtoSPAD(z_out,z_in, SIZE*sizeof(fftwf_complex));

  /* Do the bit reversal */

#if defined(DEBUG) && !defined(TG_ARCH)
  printf("zr[] = ");
  for (int i=0;i<SIZE;i++) {
    printf("%lf ", z_out[i][0]);
  }
  printf("\nzi[] = ");
  for (int i=0;i<SIZE;i++) {
    printf("%lf ", z_out[i][1]);
  }
  printf("\n");
  printf("\n");
#endif

  i2 = SIZE >> 1;
  for (int i=0,j=0,k=0;i<SIZE-1;i++) {
    if (i < j) {
      fftwf_complex t;
      t[0] = z_out[i][0];
      t[1] = z_out[i][1];
      z_out[i][0] = z_out[j][0];
      z_out[i][1] = z_out[j][1];
      z_out[j][0] = t[0];
      z_out[j][1] = t[1];
    }
    k = i2;
    while (k <= j) {
      j -= k;
      k >>= 1;
    }
    j += k;
  }

#if defined(DEBUG) && !defined(TG_ARCH)
  printf("zR[] = ");
  for (int i=0;i<SIZE;i++) {
    printf("%lf ", z_out[i][0]);
  }
  printf("\nzI[] = ");
  for (int i=0;i<SIZE;i++) {
    printf("%lf ", z_out[i][1]);
  }
  printf("\n");
  printf("\n");
#endif

  /* Compute the FFT */
  cR = -1.0;
  cI = 0.0;
  l2 = 1;
  for (int l=0;l<Pof2;l++) {
    // Compared to Cormen/Leiserson/Rivest, l goes from 0 to log - 1 instead
    // of 1 to log.
    // l1 represents m/2 and l2 represents m in their terminology
    // therefore cosinuses and sinuses are indexed by l and have the values:
    //  e(i2PI/l2) = e(i2PI/(1<<(l+1)))
    // cos[0] = -1.0
    // cos[1] = 0.0
    // cos[2] = sqrt(2)/2
    // cos[3] = cos(PI/16)
    l1 = l2;
    l2 <<= 1;
    uR = 1.0;
    uI = 0.0;
    for (int j=0;j<l1;j++) {
      for (int i=j;i<SIZE;i+=l2) {
	i1 = i + l1;
	tR = uR * z_out[i1][0] - uI * z_out[i1][1];
	tI = uR * z_out[i1][1] + uI * z_out[i1][0];
	z_out[i1][0] = z_out[i][0] - tR;
	z_out[i1][1] = z_out[i][1] - tI;
	z_out[i][0]  = z_out[i][0] + tR;
	z_out[i][1]  = z_out[i][1] + tI;
      }
      zt = uR * cR - uI * cI;
      uI = uR * cI + uI * cR;
      uR = zt;
    }
    cI = sqrt((1.0 - cR) / 2.0);
    if (dir == 1)
      cI = -cI;
    cR = sqrt((1.0 + cR) / 2.0);
  }

  return;
}
