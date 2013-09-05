#include <assert.h>

#include "common.h"

#define MAX_Ncor (10)
#define MAX_Ncor_sqr (MAX_Ncor*MAX_Ncor)

void
ccd_async_body(
	int Ncor,
	struct ImageParams *image_params,
	struct complexData **curImage,
	struct complexData **refImage,
	struct hcPoint **corr_map,
	struct corners_t *corners
) {

	int m1   = corners->m1;
	int m2   = corners->m2;
	int n1   = corners->n1;
	int n2   = corners->n2;
        int Ncor_sqr = Ncor * Ncor;
#ifdef TRACE_CCD
fprintf(stderr,"cdd m = %5d, end = %5d\n",m1,m2);fflush(stderr);
#endif
	for(int m=m1; m<m2; m++) {
        	int mIndex = (Ncor-1)/2 + m;
		for(int n=n1; n<n2; n++) {
			struct complexData f[MAX_Ncor_sqr];
			struct complexData g[MAX_Ncor_sqr];
			const int nIndex = (Ncor-1)/2 + n;
			float     den1, den2;
			struct    complexData mu_f, mu_g, num;
#ifdef TRACE_CCD
fprintf(stderr,"cdd     n = %5d, end= %5d\n",n1,n2);fflush(stderr);
#endif
			mu_f.real=0.0f;
			mu_f.imag=0.0f;
			mu_g.real=0.0f;
			mu_g.imag=0.0f;
			for(int k=0,i=mIndex-(Ncor-1)/2; i<=mIndex+(Ncor-1)/2; i++) {
#ifdef TRACE_CCD
fprintf(stderr,"cdd         i,k = %5d,%5d, end= %5d\n",i,k,(mIndex+(Ncor-1)/2));fflush(stderr);
#endif
				for(int j=nIndex-(Ncor-1)/2; j<=nIndex+(Ncor-1)/2; j++, k++) {
#ifdef TRACE_CCD
fprintf(stderr,"cdd             j,k = %5d,%5d end = %5d\n",j,k,(nIndex+(Ncor-1)/2));fflush(stderr);
#endif
#ifdef TRACE_CCD
fprintf(stderr,"curImage %16.16llx, f %16.16llx\n",curImage,f);fflush(stderr);
#endif
					f[k].real  = curImage[i][j].real;
					f[k].imag  = curImage[i][j].imag;
#ifdef TRACE_CCD
fprintf(stderr,"refImage %16.16llx, g %16.16llx\n",refImage,g);fflush(stderr);
#endif
					g[k].real  = refImage[i][j].real;
					g[k].imag  = refImage[i][j].imag;
					mu_f.real += curImage[i][j].real;
					mu_f.imag += curImage[i][j].imag;
					mu_g.real += refImage[i][j].real;
					mu_g.imag += refImage[i][j].imag;                   
#ifdef TRACE_CCD
fprintf(stderr,"cdd             k:=j,k = %5d,%5d,%5d\n",k,i,j);fflush(stderr);
#endif
				} // for j
			} // for k
			mu_f.real /= Ncor_sqr;
			mu_f.imag /= Ncor_sqr;
			mu_g.real /= Ncor_sqr;
			mu_g.imag /= Ncor_sqr;

			num.real = 0.0f;
			num.imag = 0.0f;
			den1 = 0.0f;
			den2 = 0.0f;
			for(int k=0; k<Ncor_sqr; k++) {
#ifdef TRACE_CCD
fprintf(stderr,"cdd         K = %5d, end = %5d\n",k,Ncor_sqr);fflush(stderr);
#endif
				f[k].real -= mu_f.real;
				f[k].imag -= mu_f.imag;
				g[k].real -= mu_g.real;
				g[k].imag -= mu_g.imag;
                    
				num.real += f[k].real*g[k].real + f[k].imag*g[k].imag;
				num.imag += f[k].real*g[k].imag - f[k].imag*g[k].real;

				den1 += f[k].real*f[k].real + f[k].imag*f[k].imag;
				den2 += g[k].real*g[k].real + g[k].imag*g[k].imag;
			} // for k

			corr_map[m][n].x = nIndex;
			corr_map[m][n].y = mIndex;
                
			if( (den1 != 0.0f) && (den2 != 0.0f) ) {
				corr_map[m][n].p = sqrtf( (num.real*num.real + num.imag*num.imag) / (den1*den2) );
			} else {
				corr_map[m][n].p = 0.0f;
			}
		} // for n
	} // for m
	free(corners);
} // async_body_ccd

void
CCD(	int    Ncor,
	struct hcPoint **corr_map,
	struct ImageParams *image_params,
	struct complexData **curImage,
	struct complexData **refImage)
{
	int Mwins = image_params->Iy - Ncor + 1;
	int Nwins = image_params->Ix - Ncor + 1;
	assert(Ncor < MAX_Ncor);
#ifdef TRACE_CCD
fprintf(stderr,"cdd -- begin Ncor = %d\n",Ncor);fflush(stderr);
fprintf(stderr,"corr_map %16.16llx, ImageParams %16.16llx\n",corr_map,image_params);fflush(stderr);
fprintf(stderr,"curImage %16.16llx, refImage    %16.16llx\n",curImage,refImage);fflush(stderr);
#endif
	int CCD_ASYNC_BLOCK_SIZE_M = blk_size(Mwins,32);
	int CCD_ASYNC_BLOCK_SIZE_N = blk_size(Nwins,32);
	assert((Mwins%CCD_ASYNC_BLOCK_SIZE_M)==0);
	assert((Nwins%CCD_ASYNC_BLOCK_SIZE_N)==0);
finish {
	for(int m=0; m<Mwins; m+=CCD_ASYNC_BLOCK_SIZE_M) {
		for(int n=0; n<Nwins; n+=CCD_ASYNC_BLOCK_SIZE_N) {
			struct corners_t *corners;
			corners = malloc(sizeof(struct corners_t));
			corners->m1 = m;
			corners->m2 = m+CCD_ASYNC_BLOCK_SIZE_M;
			corners->n1 = n;
			corners->n2 = n+CCD_ASYNC_BLOCK_SIZE_N;
#if 1
			async       IN(Ncor, image_params, curImage, refImage, corr_map, corners) 
#endif
			ccd_async_body(Ncor, image_params, curImage, refImage, corr_map, corners);
		} // for n
	} // for m
} // finish

#ifdef TRACE_CCD
fprintf(stderr,"cdd -- finish\n");fflush(stderr);
#endif
	return;
}
