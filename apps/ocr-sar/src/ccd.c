#ifndef RAG_SIM
#include "ocr.h"
#include "rag_ocr.h"
#include "common.h"
#else
#include "xe-edt.h"
#include "xe-memory.h"
#include "xe-console.h"
#include "xe-global.h"
#include "rag_ocr.h"
#include "common.h"
#endif

#define MAX_Ncor (10)
#define MAX_Ncor_sqr (MAX_Ncor*MAX_Ncor)

ocrGuid_t CCD_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_2
xe_printf("//// enter CCD_edt\n");RAG_FLUSH;
#endif
	assert(paramc==0);
	assert(depc==5); // 5th is post_Affine_evg
RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,0);
RAG_REF_MACRO_BSM( struct complexData **,refImage,NULL,NULL,refImage_dbg,1);
RAG_REF_MACRO_BSM( struct point **,corr_map,NULL,NULL,corr_map_dbg,2);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,3);

	void CCD(
		struct complexData **curImage,    ocrGuid_t curImage_dbg,
		struct complexData **refImage,    ocrGuid_t refImage_dbg,
		struct point **corr_map,          ocrGuid_t corr_map_dbg,
		struct ImageParams *image_params, ocrGuid_t image_params_dbg);

	CCD(	curImage,	curImage_dbg,
		refImage,	refImage_dbg,
		corr_map,	corr_map_dbg,
		image_params,	image_params_dbg);

#ifdef TRACE_LVL_2
xe_printf("//// leave CCD_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

ocrGuid_t ccd_async_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
	assert(paramc==0);
	assert(depc==5);
#ifdef TRACE_LVL_3
xe_printf("////// enter ccd_async_edt\n");RAG_FLUSH;
#endif
//	ocrGuid_t arg_scg = (ocrGuid_t)paramv[0];
RAG_REF_MACRO_SPAD(struct corners_t,corners,corners_ptr,corners_lcl,corners_dbg,0);
RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,1);
RAG_REF_MACRO_BSM( struct complexData **,refImage,NULL,NULL,refImage_dbg,2);
RAG_REF_MACRO_BSM( struct points **,corr_map,NULL,NULL,corr_map_dbg,3);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,4);
	int Ncor = image_params->Ncor;
	int m1   = corners->m1;
	int m2   = corners->m2;
	int n1   = corners->n1;
	int n2   = corners->n2;
        int Ncor_sqr = Ncor * Ncor;
#ifdef DEBUG
xe_printf("////// ccd_async m1 m2 n1 n2 s# %d %d %d %d\n",m1,m2,n1,m2);RAG_FLUSH;
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
                    corr_map_m_n.p = sqrtf_rn( (num.real*num.real + num.imag*num.imag) / (den1*den2) );
                } else {
                    corr_map_m_n.p = 0.0f;
                }
                REM_STX_ADDR(corr_map_m+n, corr_map_m_n,struct point);
            } // for n
	} // for n

	bsm_free(corners,corners_dbg);
#ifdef TRACE_LVL_3
xe_printf("////// leave ccd_async_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

void
CCD(	struct complexData **curImage,    ocrGuid_t curImage_dbg,
	struct complexData **refImage,    ocrGuid_t refImage_dbg,
	struct point **corr_map,          ocrGuid_t corr_map_dbg,
	struct ImageParams *image_params, ocrGuid_t image_params_dbg)
{
	int retval;
#ifdef TRACE_LVL_2
xe_printf("//// enter CCD\n");RAG_FLUSH;
#endif
	int Mwins = image_params->Iy - image_params->Ncor + 1;
	int Nwins = image_params->Ix - image_params->Ncor + 1;
	assert(image_params->Ncor < MAX_Ncor);
#ifdef TRACE_LVL_2
xe_printf("//// Mwins == %d and Nwins == %d, Ncor == %d\n",Mwins,Nwins,image_params->Ncor);RAG_FLUSH;
#endif
	int CCD_ASYNC_BLOCK_SIZE_M = blk_size(Mwins,32);
	int CCD_ASYNC_BLOCK_SIZE_N = blk_size(Nwins,32);
	assert((Mwins%CCD_ASYNC_BLOCK_SIZE_M)==0);
	assert((Nwins%CCD_ASYNC_BLOCK_SIZE_N)==0);
///////////// create async
#ifdef TRACE_LVL_2
xe_printf("//// create a template for ccd_async function\n");RAG_FLUSH;
#endif
	ocrGuid_t ccd_async_clg;
	retval = ocrEdtTemplateCreate(
			&ccd_async_clg,		// ocrGuid_t *new_guid
			 ccd_async_edt, 	// ocr_edt_ptr func_ptr
			0,			// paramc
			5);			// depc
	assert(retval==0);

	for(int m=0; m<Mwins; m+=CCD_ASYNC_BLOCK_SIZE_M) {
		for(int n=0; n<Nwins; n+=CCD_ASYNC_BLOCK_SIZE_N) {
#ifdef TRACE_LVL_2
xe_printf("//// create an edt for ccd_async\n");RAG_FLUSH;
#endif
			ocrGuid_t ccd_async_scg;
			retval = ocrEdtCreate(
					&ccd_async_scg,		// *created_edt_guid
					 ccd_async_clg,		// edt_template_guid
					EDT_PARAM_DEF,		// paramc
					NULL,			// *paramv
					EDT_PARAM_DEF,		// depc
					NULL,			// *depv
					EDT_PROP_NONE,		// properties
					NULL_GUID,		// affinity
					NULL);			// *outputEvent
			assert(retval==0);
			struct corners_t *async_corners, *async_corners_ptr, async_corners_lcl; ocrGuid_t async_corners_dbg;
			async_corners = &async_corners_lcl;
			async_corners_ptr = bsm_malloc(&async_corners_dbg,sizeof(struct corners_t));
			async_corners->m1   = m;
			async_corners->m2   = m+CCD_ASYNC_BLOCK_SIZE_M;
			async_corners->n1   = n;
			async_corners->n2   = n+CCD_ASYNC_BLOCK_SIZE_N;
			REM_STX_ADDR(async_corners_ptr,async_corners_lcl,struct corners_t);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,async_corners_dbg,0);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,curImage_dbg,1);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,refImage_dbg,2);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,corr_map_dbg,3);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,4);
			OCR_DB_RELEASE(async_corners_dbg);
		} /* for n */
	} /* for m */

	OCR_DB_RELEASE(curImage_dbg);
	OCR_DB_RELEASE(refImage_dbg);
	OCR_DB_RELEASE(corr_map_dbg);
	OCR_DB_RELEASE(image_params_dbg);
#ifdef TRACE_LVL_2
xe_printf("//// leave CCD\n");RAG_FLUSH;
#endif
	return;
}
