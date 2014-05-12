#include <stdlib.h>

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

ocrGuid_t post_CFAR_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_2
xe_printf("//// enter post_CFAR_edt\n");RAG_FLUSH;
#endif
	assert(paramc==1);
	assert(depc==2);
RAG_REF_MACRO_BSM( struct detects *,Y,NULL,NULL,Y_dbg,0);
RAG_REF_MACRO_BSM( int *,p_Nd,NULL,NULL,Nd_dbg,1);
	int Nd = RAG_GET_INT(p_Nd);

#if RAG_QSORT_ON
	// Sorting the detections is not strictly necessary. This is only done so
	// that we can directly diff the output detections file for validation
	// purposes.  They can be out-of-order due to the OpenMP parallelization
	// of the above loop.

	qsort(Y, Nd, sizeof(struct detects), compare_detects);
#else
#ifdef TRACE_LVL_2
xe_printf("//// SKIPPING Sort of the detections\n");RAG_FLUSH;
#endif
#endif

#ifdef TRACE_LVL_2
xe_printf("//// Output to file %d detects\n",Nd);RAG_FLUSH;
#endif
	for(int m=0; m<Nd; m++) {
#ifndef RAG_SIM
		printf("x=%7.2fm y=%7.2fm p=%4.2f\n", Y[m].x, Y[m].y, Y[m].p);
#else
		struct detects Y_m;
		REM_LDX_ADDR(Y_m,&Y[m],struct detects);
		xe_printf("x=0x%x m y=0x%x m p=0x%x\n",
			 *(uint32_t *)&Y_m.x, *(uint32_t *)&Y_m.y, *(uint32_t *)&Y_m.p);
#endif
	} // for m
	ocrGuid_t arg_scg = (ocrGuid_t)paramv[0];
RAG_DEF_MACRO_PASS(arg_scg,NULL,NULL,NULL,NULL,Y_dbg,0);
	OCR_DB_RELEASE(Y_dbg);
	bsm_free(p_Nd,Nd_dbg);
#ifdef TRACE_LVL_2
xe_printf("//// leave post_CFAR_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

ocrGuid_t CFAR_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_2
xe_printf("//// enter CFAR_edt\n");RAG_FLUSH;
#endif
	assert(paramc==1);
	assert(depc==4);
	ocrGuid_t arg_scg = (ocrGuid_t)paramv[0];

RAG_REF_MACRO_BSM( struct point **,corr_map,NULL,NULL,corr_map_dbg,0);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,1);
RAG_REF_MACRO_SPAD(struct CfarParams,cfar_params,cfar_params_ptr,cfar_params_lcl,cfar_params_dbg,2);
RAG_REF_MACRO_BSM( struct detects *,Y,NULL,NULL,Y_dbg,3);

#ifdef TRACE_LVL_2
xe_printf("//// create a template for post_CFAR function\n");RAG_FLUSH;
#endif
	ocrGuid_t post_CFAR_clg;
	retval = ocrEdtTemplateCreate(
			&post_CFAR_clg,		// ocrGuid_t *new_guid
			 post_CFAR_edt,		// ocr_edt_ptr func_ptr
			1,			// paramc
			2);			// depc
	assert(retval==0);

#ifdef TRACE_LVL_2
xe_printf("//// create an edt for post_CFAR\n");RAG_FLUSH;
#endif
	ocrGuid_t post_CFAR_scg;
{	uint64_t paramv[1] = { GUID_VALUE(arg_scg) };
	retval = ocrEdtCreate(
			&post_CFAR_scg,		// *created_edt_guid
			 post_CFAR_clg,		// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			paramv,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_NONE,		// properties
			NULL_GUID,		// affinity
			NULL);			// *outputEvent
}
	assert(retval==0);

	void CFAR(struct point **corr_map, ocrGuid_t corr_map_dbg,
		struct ImageParams *image_params, ocrGuid_t image_params_dbg,
		struct CfarParams *cfar_params, ocrGuid_t cfar_params_dbg,
		struct detects *Y, ocrGuid_t Y_dbg,
		ocrGuid_t post_CFAR_scg);

	CFAR(	corr_map, corr_map_dbg,
		image_params, image_params_dbg,
		cfar_params, cfar_params_dbg,
		Y, Y_dbg,
		post_CFAR_scg);

#ifdef TRACE_LVL_2
xe_printf("//// leave CFAR_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

#if RAG_QSORT_ON
// This function is used to sort the detections.  It returns -1 if _lhs < _rhs,
// 0 if they are equal, and 1 if _lhs > _rhs.  Ordering is performed primarily
// along the y axis and secondarily along the x axis.
static int compare_detects(const void * _lhs, const void * _rhs)
{
    const struct detects * const lhs = (/*const*/ struct detects * /*const*/) _lhs;
    const struct detects * const rhs = (/*const*/ struct detects * /*const*/) _rhs;

    if (lhs->y < rhs->y) { return -1; }
    else if (lhs->y > rhs->y) { return 1; }
    else
    {
        // y coordinates are equal; revert to comparing x coordinates
        if (lhs->x < rhs->x) { return -1; }
        else if (lhs->x > rhs->x) { return 1; }
        else { return 0; } // equality
    }
}
#endif

ocrGuid_t cfar_finish_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_3
xe_printf("////// enter cfar_finish_edt %d\n",depc);RAG_FLUSH;
#endif
	assert(paramc==1);
	assert(depc>1);
	RAG_REF_MACRO_BSM( struct detects *,Y,NULL,NULL,Y_dbg,0);
	RAG_REF_MACRO_BSM( int *,p_Nd,NULL,NULL,Nd_dbg,1);
	ocrGuid_t arg_scg = (ocrGuid_t)paramv[0];
	RAG_DEF_MACRO_PASS(arg_scg,NULL,NULL,NULL,NULL,Y_dbg,0);
	RAG_DEF_MACRO_PASS(arg_scg,NULL,NULL,NULL,NULL,Nd_dbg,1);
	OCR_DB_RELEASE(Y_dbg);
	OCR_DB_RELEASE(Nd_dbg);
#ifdef TRACE_LVL_3
xe_printf("////// leave cfar_finish_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

ocrGuid_t cfar_async_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_3
xe_printf("////// enter cfar_async_edt\n");RAG_FLUSH;
#endif
	assert(paramc==1);
	assert(depc==6);

RAG_REF_MACRO_SPAD(struct corners_t,corners,corners_ptr,corners_lcl,corners_dbg,0);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,1);
RAG_REF_MACRO_SPAD(struct CfarParams,cfar_params,cfar_parms_ptr,cfar_parms_lcl,cfar_params_dbg,2);
RAG_REF_MACRO_BSM( struct point **,corr_map,NULL,NULL,corr_map_dbg,3);
RAG_REF_MACRO_BSM( struct detects *,Y,NULL,NULL,Y_dbg,4);
RAG_REF_MACRO_BSM( int *,p_Nd,NULL,NULL,Nd_dbg,5);

	int m1   = corners->m1;
	int m2   = corners->m2;
	int n1   = corners->n1;
	int n2   = corners->n2;
	int slot = corners->slot;

	int T,cnt;
	int mIndex, nIndex;
	float CUT;
	float **pLocal;

	// Correlation value threshold
	const float Tcorr = 0.9;
	// CUT's correlation value must be below Tcorr to be eligible for detection

#ifdef TRACE_LVL_3
xe_printf("////// cfar_async m1 m2 n1 n2 s# %d %d %d %d (%d)\n",m1,m2,n1,m2,slot);RAG_FLUSH;
#endif

	ocrGuid_t pLocal_dbg;
        pLocal = (float **)spad_malloc(&pLocal_dbg,cfar_params->Ncfar*sizeof(float*));
        if(pLocal == NULL) {
            xe_printf("Error allocating edge vector for local correlation map.\n");RAG_FLUSH;
            xe_exit(1);
        }

	float * pLocal_data_ptr; ocrGuid_t pLocal_data_dbg;
        pLocal_data_ptr = (float*)spad_malloc(&pLocal_data_dbg,
		cfar_params->Ncfar*(cfar_params->Ncfar*sizeof(float)));
        if (pLocal_data_ptr == NULL) {
            xe_printf("Error allocating data memory for local correlation map.\n");RAG_FLUSH;
            xe_exit(1);
        }
        for(int m=0; m<cfar_params->Ncfar; m++)
        {
            pLocal[m] = pLocal_data_ptr + m*cfar_params->Ncfar;
        }

        for(int m=m1; m<m2; m++) {
            mIndex = (cfar_params->Ncfar-1)/2 + m;
            for(int n=n1; n<n2; n++) {
                nIndex = (cfar_params->Ncfar-1)/2 + n;

                for(int i=mIndex-(cfar_params->Ncfar-1)/2, k=0; i<=mIndex+(cfar_params->Ncfar-1)/2; i++, k++) {
                    for(int j=nIndex-(cfar_params->Ncfar-1)/2, l=0; j<=nIndex+(cfar_params->Ncfar-1)/2; j++, l++) {
			struct point *corr_map_i;
			corr_map_i = (struct point *)RAG_GET_PTR(corr_map+i);
			struct point *corr_map_i_j = corr_map_i+j;
			float *corr_map_i_j_p = (float *)(((char *)corr_map_i_j)+offsetof(struct point,p));

assert(corr_map_i_j_p == &corr_map[i][j].p);

                        pLocal[k][l] = RAG_GET_FLT(corr_map_i_j_p);

assert(pLocal[k][l] == corr_map[i][j].p);

                    } // for j
                } // for i

#ifdef DEBUG_LVL_2
xe_printf("////// Create guard window\n");RAG_FLUSH;
#endif
                for(int i=(cfar_params->Ncfar-1)/2-(cfar_params->Nguard-1)/2; i<=(cfar_params->Ncfar-1)/2+(cfar_params->Nguard-1)/2; i++) {
                    for(int j=(cfar_params->Ncfar-1)/2-(cfar_params->Nguard-1)/2; j<=(cfar_params->Ncfar-1)/2+(cfar_params->Nguard-1)/2; j++) {
                        pLocal[i][j] = -1;
                    } // for j
                } // for i

#ifdef TRACE_LVL_3
xe_printf("////// Calculate threshold\n");RAG_FLUSH;
#endif
#ifdef RAG_PURE_FLOAT
                T = (int)floorf(cfar_params->Tcfar/100.0f*(cfar_params->Ncfar*cfar_params->Ncfar-cfar_params->Nguard*cfar_params->Nguard));
#else
                T = (int)floorf(cfar_params->Tcfar/100.0*(cfar_params->Ncfar*cfar_params->Ncfar-cfar_params->Nguard*cfar_params->Nguard));
#endif

		struct point *corr_map_m;
		corr_map_m = (struct point *)RAG_GET_PTR(corr_map+mIndex);
		struct point *corr_map_m_n = corr_map_m+nIndex;
		float *corr_map_m_n_p = (float *)(((char *)corr_map_m_n)+offsetof(struct point,p));

assert(corr_map_m_n_p == &corr_map[mIndex][nIndex].p);

                CUT = RAG_GET_FLT(corr_map_m_n_p);

#ifdef DEBUG_LVL_2
xe_printf("////// Ensure CUT's correlation value is below the correlation threshold\n");RAG_FLUSH;
#endif
                if(CUT < Tcorr) {
                    for(int i=cnt=0; i<cfar_params->Ncfar; i++) {
                        for(int j=0; j<cfar_params->Ncfar; j++) {
                            if(CUT < pLocal[i][j]) {
                                cnt++;
                            } // if CUT
                        } // for j
                    } // for j

                    if(cnt >= T) {
			    int *corr_map_m_n_x =
				(int *)(((char *)corr_map_m_n)+offsetof(struct point,x));
			    int *corr_map_m_n_y =
				(int *)(((char *)corr_map_m_n)+offsetof(struct point,y));
			    int nd = __sync_fetch_and_add(p_Nd,1);
			    struct detects Y_nd;

assert(&Y[nd] == (Y+nd));

                            Y_nd.x = RAG_GET_FLT(image_params->xr+RAG_GET_INT((int32_t *)corr_map_m_n_x));
                            Y_nd.y = RAG_GET_FLT(image_params->yr+RAG_GET_INT((int32_t *)corr_map_m_n_y));
			    Y_nd.p = CUT;

assert(Y_nd.x == image_params->xr[corr_map[mIndex][nIndex].x]);
assert(Y_nd.y == image_params->yr[corr_map[mIndex][nIndex].y]);

#ifdef DEBUG_LVL_2
xe_printf("detect %d x = 0x%x y = 0x%x p = 0x%x (%d,%d)\n",nd,*(uint32_t *)&Y_nd.x,*(uint32_t *)&Y_nd.y,*(uint32_t *)&Y_nd.p,RAG_GET_INT((int32_t *)corr_map_m_n_x),RAG_GET_INT((int32_t *)corr_map_m_n_y));RAG_FLUSH;
#endif
			    REM_STX_ADDR(Y+nd,Y_nd,struct detects);
                    } // if cnt
                } // if CUT
            } // for n
        } // for m

        if (pLocal[0])
            spad_free(pLocal_data_ptr,pLocal_data_dbg);
        if (pLocal) {
            spad_free(pLocal,pLocal_dbg);
            pLocal = NULL;
        }

	ocrGuid_t arg_scg = (ocrGuid_t)paramv[0];
#ifdef RAG_SIM_NULL_GUID_WORKAROUND
RAG_DEF_MACRO_PASS(arg_scg,NULL,NULL,NULL,NULL,corr_map_dbg,slot);
#else
RAG_DEF_MACRO_PASS(arg_scg,NULL,NULL,NULL,NULL,NULL_GUID,slot);
#endif
	bsm_free(corners_ptr,corners_dbg);
#ifdef TRACE_LVL_3
xe_printf("////// leave cfar_async_cedt\n");RAG_FLUSH;
#endif
    return NULL_GUID;
}

void CFAR(
	struct point **corr_map, ocrGuid_t corr_map_dbg,
	struct ImageParams *image_params, ocrGuid_t image_params_dbg,
	struct CfarParams *cfar_params, ocrGuid_t cfar_params_dbg,
	struct detects *Y, ocrGuid_t Y_dbg,
	ocrGuid_t post_CFAR_scg) {
	int retval;
	int Mwins, Nwins;

#ifdef TRACE_LVL_2
xe_printf("//// enter CFAR\n");RAG_FLUSH;
#endif
	Mwins = image_params->Iy - image_params->Ncor - cfar_params->Ncfar + 2;
	Nwins = image_params->Ix - image_params->Ncor - cfar_params->Ncfar + 2;
#ifdef TRACE_LVL_2
xe_printf("//// Mwins == %d and Nwins == %d, Ncfar == %d, Ncor == %d\n",Mwins,Nwins,cfar_params->Ncfar,image_params->Ncor);RAG_FLUSH;
#endif

	ocrGuid_t Nd_dbg;
	int *Nd_ptr = bsm_malloc(&Nd_dbg,sizeof(int));
	RAG_PUT_INT(Nd_ptr,0);
	int CFAR_ASYNC_BLOCK_SIZE_M = blk_size(Mwins,32);
	int CFAR_ASYNC_BLOCK_SIZE_N = blk_size(Nwins,32);
	assert((Mwins%CFAR_ASYNC_BLOCK_SIZE_M)==0);
	assert((Nwins%CFAR_ASYNC_BLOCK_SIZE_N)==0);
///////////// create async and finish, instance of finish
#ifdef TRACE_LVL_2
xe_printf("//// create a template for cfar_async function\n");RAG_FLUSH;
#endif
	ocrGuid_t cfar_async_clg;
	retval = ocrEdtTemplateCreate(
			&cfar_async_clg,	// ocrGuid_t *new_guid
			 cfar_async_edt,	// ocr_edt_ptr func_ptr
			1,			// paramc
			6);			// depc
	assert(retval==0);

#ifdef TRACE_LVL_2
xe_printf("//// create a template for cfar_finish function\n");RAG_FLUSH;
#endif
	ocrGuid_t cfar_finish_clg;
	retval = ocrEdtTemplateCreate(
			&cfar_finish_clg,	// ocrGuid_t *new_guid
			 cfar_finish_edt,	// ocr_edt_ptr func_ptr
			1,			// paramc
			((Mwins-0)/CFAR_ASYNC_BLOCK_SIZE_M)*((Nwins-0)/CFAR_ASYNC_BLOCK_SIZE_N)+2); // depc
	assert(retval==0);
#ifdef TRACE_LVL_2
xe_printf("//// create an edt for cfar_finish\n");RAG_FLUSH;
#endif
	ocrGuid_t cfar_finish_scg;
{	uint64_t paramv[1] = { GUID_VALUE(post_CFAR_scg) };
	retval = ocrEdtCreate(
			&cfar_finish_scg,	// *created_edt_guid
			 cfar_finish_clg,	// edt_template_guid
			EDT_PARAM_DEF,		// paramc
			paramv,			// *paramv
			EDT_PARAM_DEF,		// depc
			NULL,			// *depv
			EDT_PROP_NONE,		// properties
			NULL_GUID,		// affinity
			NULL);			// *outputEvent
}
	assert(retval==0);
	int slot = 0;
RAG_DEF_MACRO_PASS(cfar_finish_scg,NULL,NULL,NULL,NULL,Y_dbg, slot++);
RAG_DEF_MACRO_PASS(cfar_finish_scg,NULL,NULL,NULL,NULL,Nd_dbg,slot++);

	for(int m=0; m<Mwins; m+=CFAR_ASYNC_BLOCK_SIZE_M) {
		for(int n=0; n<Nwins; n+=CFAR_ASYNC_BLOCK_SIZE_N) {
#ifdef TRACE_LVL_2
xe_printf("////// create an edt for cfar_async slot %d\n",slot);RAG_FLUSH;
#endif
			ocrGuid_t cfar_async_scg;
{			uint64_t paramv[1] = { GUID_VALUE(cfar_finish_scg) };
			retval = ocrEdtCreate(
					&cfar_async_scg,	// *created_edt_guid
					 cfar_async_clg,	// edt_template_guid
					EDT_PARAM_DEF,		// paramc
					paramv,			// *paramv
					EDT_PARAM_DEF,		// depc
					NULL,			// *depv
					EDT_PROP_NONE,		// properties
					NULL_GUID,		// affinity
					NULL);			// *outputEvent
}
			assert(retval==0);
			struct corners_t *async_corners, *async_corners_ptr, async_corners_lcl; ocrGuid_t async_corners_dbg;
			async_corners = &async_corners_lcl;
			async_corners_ptr = bsm_malloc(&async_corners_dbg,sizeof(struct corners_t));
			async_corners->m1   = m;
			async_corners->m2   = m+CFAR_ASYNC_BLOCK_SIZE_M;
			async_corners->n1   = n;
			async_corners->n2   = n+CFAR_ASYNC_BLOCK_SIZE_N;
			async_corners->slot = slot++;
			REM_STX_ADDR(async_corners_ptr,async_corners_lcl,struct corners_t);
RAG_DEF_MACRO_PASS(cfar_async_scg,NULL,NULL,NULL,NULL,async_corners_dbg,0);
RAG_DEF_MACRO_PASS(cfar_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,1);
RAG_DEF_MACRO_PASS(cfar_async_scg,NULL,NULL,NULL,NULL,cfar_params_dbg,2);
RAG_DEF_MACRO_PASS(cfar_async_scg,NULL,NULL,NULL,NULL,corr_map_dbg,3);
RAG_DEF_MACRO_PASS(cfar_async_scg,NULL,NULL,NULL,NULL,Y_dbg,4);
RAG_DEF_MACRO_PASS(cfar_async_scg,NULL,NULL,NULL,NULL,Nd_dbg,5);
			OCR_DB_RELEASE(async_corners_dbg);
		} // for n
	} // for m

	OCR_DB_RELEASE(image_params_dbg);
	OCR_DB_RELEASE(cfar_params_dbg);
	OCR_DB_RELEASE(corr_map_dbg);
	OCR_DB_RELEASE(Y_dbg);
	OCR_DB_RELEASE(Nd_dbg);
#ifdef TRACE_LVL_2
xe_printf("//// leave CFAR\n");RAG_FLUSH;
#endif
	return;
}
