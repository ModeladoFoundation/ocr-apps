#include "ocr.h"
#include "rag_ocr.h"
#include "common.h"

#define MAX_Ncor (10)
#define MAX_Ncor_sqr (MAX_Ncor*MAX_Ncor)

ocrGuid_t CCD_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_2
PRINTF("//// enter CCD_edt\n");RAG_FLUSH;
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
PRINTF("//// leave CCD_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

ocrGuid_t ccd_async_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_3
PRINTF("////// enter ccd_async_edt\n");RAG_FLUSH;
#endif
	assert(paramc==((sizeof(struct corners_t) + sizeof(uint64_t) - 1)/sizeof(uint64_t)));
	struct corners_t *corners = (struct corners_t *)paramv;
	int m1   = corners->m1;
	int m2   = corners->m2;
	int n1   = corners->n1;
	int n2   = corners->n2;
	assert(depc==4);
RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,0);
RAG_REF_MACRO_BSM( struct complexData **,refImage,NULL,NULL,refImage_dbg,1);
RAG_REF_MACRO_BSM( struct point **,corr_map,NULL,NULL,corr_map_dbg,2);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,3);
	int Ncor = image_params->Ncor;
        int Ncor_sqr = Ncor * Ncor;
#ifdef DEBUG
PRINTF("////// ccd_async m1 m2 n1 n2 %d %d %d %d\n",m1,m2,n1,n2);RAG_FLUSH;
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
                    for(int j=nIndex-(Ncor-1)/2; j<=nIndex+(Ncor-1)/2; j++, k++) {
                        f[k].real  = curImage[i][j].real;
                        f[k].imag  = curImage[i][j].imag;
                        g[k].real  = refImage[i][j].real;
                        g[k].imag  = refImage[i][j].imag;
                        mu_f.real += curImage[i][j].real;
                        mu_f.imag += curImage[i][j].imag;
                        mu_g.real += refImage[i][j].real;
                        mu_g.imag += refImage[i][j].imag;
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

                corr_map[m][n].x = nIndex;
                corr_map[m][n].y = mIndex;
                if( (den1 != 0.0f) && (den2 != 0.0f) ) {
                    corr_map[m][n].p = sqrtf( (num.real*num.real + num.imag*num.imag) / (den1*den2) );
                } else {
                    corr_map[m][n].p = 0.0f;
                }
            } // for n
	} // for n

#ifdef TRACE_LVL_3
PRINTF("////// leave ccd_async_edt\n");RAG_FLUSH;
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
PRINTF("//// enter CCD\n");RAG_FLUSH;
#endif
	int Mwins = image_params->Iy - image_params->Ncor + 1;
	int Nwins = image_params->Ix - image_params->Ncor + 1;
	assert(image_params->Ncor < MAX_Ncor);
#ifdef TRACE_LVL_2
PRINTF("//// Mwins == %d and Nwins == %d, Ncor == %d\n",Mwins,Nwins,image_params->Ncor);RAG_FLUSH;
#endif
#ifdef RAG_NEW_BLK_SIZE
	int CCD_ASYNC_BLOCK_SIZE_M = RAG_NEW_BLK_SIZE;
	int CCD_ASYNC_BLOCK_SIZE_N = RAG_NEW_BLK_SIZE;
#else
	int CCD_ASYNC_BLOCK_SIZE_M = blk_size(Mwins,32);
	int CCD_ASYNC_BLOCK_SIZE_N = blk_size(Nwins,32);
	assert((Mwins%CCD_ASYNC_BLOCK_SIZE_M)==0);
	assert((Nwins%CCD_ASYNC_BLOCK_SIZE_N)==0);
#endif
///////////// create async
#ifdef TRACE_LVL_2
PRINTF("//// create a template for ccd_async function\n");RAG_FLUSH;
#endif
	ocrGuid_t ccd_async_clg;
	retval = ocrEdtTemplateCreate(
			&ccd_async_clg,		// ocrGuid_t *new_guid
			 ccd_async_edt, 	// ocr_edt_ptr func_ptr
			(sizeof(struct corners_t) + sizeof(uint64_t) - 1)/sizeof(uint64_t), // paramc
			4);			// depc
	assert(retval==0);
	templateList[__sync_fetch_and_add(&templateIndex,1)] = ccd_async_clg;

#ifdef RAG_NEW_BLK_SIZE
	for(int m=0; m<Mwins; m+=CCD_ASYNC_BLOCK_SIZE_M) {
		for(int n=0; n<Nwins; n+=CCD_ASYNC_BLOCK_SIZE_N) {
			struct corners_t async_corners;
			async_corners.m1   = m;
			async_corners.m2   = (m+CCD_ASYNC_BLOCK_SIZE_M)<Mwins?(m+CCD_ASYNC_BLOCK_SIZE_M):Mwins;
			async_corners.n1   = n;
			async_corners.n2   = (n+CCD_ASYNC_BLOCK_SIZE_N)<Nwins?(n+CCD_ASYNC_BLOCK_SIZE_N):Nwins;
#else
	for(int m=0; m<Mwins; m+=CCD_ASYNC_BLOCK_SIZE_M) {
		for(int n=0; n<Nwins; n+=CCD_ASYNC_BLOCK_SIZE_N) {
			struct corners_t async_corners;
			async_corners.m1   = m;
			async_corners.m2   = m+CCD_ASYNC_BLOCK_SIZE_M;
			async_corners.n1   = n;
			async_corners.n2   = n+CCD_ASYNC_BLOCK_SIZE_N;
#endif
#ifdef TRACE_LVL_2
PRINTF("//// create an edt for ccd_async\n");RAG_FLUSH;
#endif
			ocrGuid_t ccd_async_scg;
			retval = ocrEdtCreate(
					&ccd_async_scg,		// *created_edt_guid
					 ccd_async_clg,		// edt_template_guid
					EDT_PARAM_DEF,		// paramc
					(uint64_t*)&async_corners, // *paramv
					EDT_PARAM_DEF,		// depc
					NULL,			// *depv
					EDT_PROP_NONE,		// properties
					NULL_GUID,		// affinity
					NULL);			// *outputEvent
			assert(retval==0);

RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,curImage_dbg,0);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,refImage_dbg,1);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,corr_map_dbg,2);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,3);

		} /* for n */
	} /* for m */

#ifdef TRACE_LVL_2
PRINTF("//// leave CCD\n");RAG_FLUSH;
#endif
	return;
}
