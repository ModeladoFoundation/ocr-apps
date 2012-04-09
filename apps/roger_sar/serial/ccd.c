#include <assert.h>

#include "common.h"

#include "rag_rmd.h"

#define MAX_Ncor (10)
#define MAX_Ncor_sqr (MAX_Ncor*MAX_Ncor)

void
async_body_ccd(
	int Ncor,
	struct complexData **curImage,
	struct complexData **refImage,
	struct point **corr_map,
	int m1, int m2, int n1, int n2
) {
        int Ncor_sqr = Ncor * Ncor;
#ifdef TRACE
xe_printf("cdd (m1,m2) = (%d,%d) (n1,n2) = (%d,%d)\n",m1,m2,n1,n2);RAG_FLUSH;
#endif
        for(int m=m1; m<m2; m++) {
            int mIndex = (Ncor-1)/2 + m;
            for(int n=n1; n<n2; n++) {
                struct complexData f[MAX_Ncor_sqr];
                struct complexData g[MAX_Ncor_sqr];
                const int nIndex = (Ncor-1)/2 + n;
	        float     den1, den2;
        	struct    complexData mu_f, mu_g, num;
                mu_f.real=0.0f;
                mu_f.imag=0.0f;
                mu_g.real=0.0f;
                mu_g.imag=0.0f;
                for(int k=0,i=mIndex-(Ncor-1)/2; i<=mIndex+(Ncor-1)/2; i++) {
                    struct complexData *cur_i;
                    cur_i = (struct complexData *)RAG_GET_PTR(curImage+i);
                    struct complexData *ref_i;
                    ref_i = (struct complexData *)RAG_GET_PTR(refImage+i);
                    for(int j=nIndex-(Ncor-1)/2; j<=nIndex+(Ncor-1)/2; j++, k++) {
                        struct complexData cur_i_j;
                        struct complexData ref_i_j;
			REM_LDX_ADDR(cur_i_j,cur_i+j,struct complexData);
			REM_LDX_ADDR(ref_i_j,ref_i+j,struct complexData);
                        f[k].real  = cur_i_j.real;
                        f[k].imag  = cur_i_j.imag;
                        g[k].real  = ref_i_j.real;
                        g[k].imag  = ref_i_j.imag;
                        mu_f.real += cur_i_j.real;
                        mu_f.imag += cur_i_j.imag;
                        mu_g.real += ref_i_j.real;
                        mu_g.imag += ref_i_j.imag;
                    } // for
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
                    f[k].real -= mu_f.real;
                    f[k].imag -= mu_f.imag;
                    g[k].real -= mu_g.real;
                    g[k].imag -= mu_g.imag;
                    
                    num.real += f[k].real*g[k].real + f[k].imag*g[k].imag;
                    num.imag += f[k].real*g[k].imag - f[k].imag*g[k].real;

                    den1 += f[k].real*f[k].real + f[k].imag*f[k].imag;
                    den2 += g[k].real*g[k].real + g[k].imag*g[k].imag;
                } // for k

                struct point *corr_map_m;
                corr_map_m = (struct point *)RAG_GET_PTR(corr_map+m);
		struct point corr_map_m_n;
                corr_map_m_n.x = nIndex;
                corr_map_m_n.y = mIndex;
                if( (den1 != 0.0f) && (den2 != 0.0f) ) {
                    corr_map_m_n.p = sqrtf( (num.real*num.real + num.imag*num.imag) / (den1*den2) );
                } else {
                    corr_map_m_n.p = 0.0f;
                }
                REM_STX_ADDR(corr_map_m+n, corr_map_m_n,struct point);
            } // for n
	} // for n
} // async_body_ccd

void
CCD(	int    Ncor,
	struct point **corr_map,
	struct ImageParams *image_params,
	struct complexData **curImage,
	struct complexData **refImage)
{
    int Mwins = image_params->Iy - Ncor + 1;
    int Nwins = image_params->Ix - Ncor + 1;
    assert(Ncor < MAX_Ncor);
#ifdef TRACE_CCD
xe_printf("cdd -- begin Ncor = %d\n",Ncor);RAG_FLUSH;
xe_printf("corr_map %1x, ImageParams %lx\n",(uint64_t)corr_map,(uint64_t)image_params);RAG_FLUSH;
xe_printf("curImage %lx, refImage    %lx\n",(uint64_t)curImage,(uint64_t)refImage);RAG_FLUSH;
#endif
#define BLOCK_SIZE 4
    assert((Mwins%BLOCK_SIZE)==0);
    assert((Nwins%BLOCK_SIZE)==0);
    for(int m=0; m<Mwins; m+=BLOCK_SIZE) {
        for(int n=0; n<Nwins; n+=BLOCK_SIZE) {
            async_body_ccd(Ncor, curImage, refImage, corr_map,
		 m,m+BLOCK_SIZE,n,n+BLOCK_SIZE);
        } /* for n */
    } /* for m */
#ifdef TRACE_CCD
xe_printf("cdd -- finish\n");RAG_FLUSH;
#endif
    return;
}
