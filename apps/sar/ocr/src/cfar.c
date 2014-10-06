#ifndef TG_ARCH
#include <stdlib.h>
#endif

#include "ocr.h"
#include "rag_ocr.h"
#include "common.h"

ocrGuid_t post_CFAR_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_2
PRINTF("//// enter post_CFAR_edt\n");RAG_FLUSH;
#endif
	assert(paramc==1);
#ifdef TG_ARCH
       void *pOutFile = (void *)paramv[0];
#else
       FILE *pOutFile = (FILE *)paramv[0];
#endif
#ifdef TRACE_LVL_2
PRINTF("//// pOutFile = %lx\n",pOutFile);RAG_FLUSH;
#endif
	assert(depc==3);
RAG_REF_MACRO_BSM( struct detects *,Y,NULL,NULL,Y_dbg,0);
RAG_REF_MACRO_BSM( int *,p_Nd,NULL,NULL,Nd_dbg,1);

	int Nd = *p_Nd;

#if RAG_QSORT_ON
	// Sorting the detections is not strictly necessary. This is only done so
	// that we can directly diff the output detections file for validation
	// purposes.  They can be out-of-order due to the OpenMP parallelization
	// of the above loop.

	qsort(Y, Nd, sizeof(struct detects), compare_detects);
#else
#ifdef TRACE_LVL_2
PRINTF("//// SKIPPING Sort of the detections\n");RAG_FLUSH;
#endif
#endif

#ifdef TRACE_LVL_2
PRINTF("//// Output to file %d detects\n",Nd);RAG_FLUSH;
#endif
	for(int m=0; m<Nd; m++) {
#ifndef TG_ARCH
		fprintf(pOutFile,"(x=%7.2f m, y=%7.2f m, p=%4.2f)\n", Y[m].x, Y[m].y, Y[m].p); // RAG -- Chaged to match latest code from Dan Campell
#else
		PRINTF("(x=0x%x m, y=0x%x m, p=0x%x)\n",
			 *(uint32_t *)&Y[m].x, *(uint32_t *)&Y[m].y, *(uint32_t *)&Y[m].p);
#endif
	} // for m

	bsm_free(p_Nd,Nd_dbg);
#ifdef TRACE_LVL_2
PRINTF("//// leave post_CFAR_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

ocrGuid_t CFAR_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_2
PRINTF("//// enter CFAR_edt\n");RAG_FLUSH;
#endif
	assert(paramc==1);
	ocrGuid_t post_CFAR_scg = (ocrGuid_t)paramv[0];
	assert(depc==5);
RAG_REF_MACRO_BSM( struct point **,corr_map,NULL,NULL,corr_map_dbg,0);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,1);
RAG_REF_MACRO_SPAD(struct CfarParams,cfar_params,cfar_params_ptr,cfar_params_lcl,cfar_params_dbg,2);
RAG_REF_MACRO_BSM( struct detects *,Y,NULL,NULL,Y_dbg,3);

	void CFAR(struct point **corr_map, ocrGuid_t corr_map_dbg,
		struct ImageParams *image_params, ocrGuid_t image_params_dbg,
		struct CfarParams *cfar_params, ocrGuid_t cfar_params_dbg,
		struct detects *Y, ocrGuid_t Y_dbg,
		ocrGuid_t post_CFAR_scg);

	CFAR(	corr_map, corr_map_dbg,
		image_params, image_params_dbg,
		cfar_params, cfar_params_dbg,
		Y, Y_dbg, post_CFAR_scg);

#ifdef TRACE_LVL_2
PRINTF("//// leave CFAR_edt\n");RAG_FLUSH;
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

ocrGuid_t cfar_async_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_3
PRINTF("////// enter cfar_async_edt\n");RAG_FLUSH;
#endif
	assert(paramc==((sizeof(struct corners_t) + sizeof(uint64_t) - 1)/sizeof(uint64_t)));
        struct corners_t *corners = (struct corners_t *)paramv;
	int m1   = corners->m1;
	int m2   = corners->m2;
	int n1   = corners->n1;
	int n2   = corners->n2;
	assert(depc==5);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,0);
RAG_REF_MACRO_SPAD(struct CfarParams,cfar_params,cfar_parms_ptr,cfar_parms_lcl,cfar_params_dbg,1);
RAG_REF_MACRO_BSM( struct point **,corr_map,NULL,NULL,corr_map_dbg,2);
RAG_REF_MACRO_BSM( struct detects *,Y,NULL,NULL,Y_dbg,3);
RAG_REF_MACRO_BSM( int *,p_Nd,NULL,NULL,Nd_dbg,4);

	int T,cnt;
	int mIndex, nIndex;
	float CUT;
	float **pLocal;

	// Correlation value threshold
	const float Tcorr = 0.8; // RAG -- Chaged to match latest code from Dan Campell
	// CUT's correlation value must be below Tcorr to be eligible for detection

#ifdef TRACE_LVL_3
PRINTF("////// cfar_async m1 m2 n1 n2 %d %d %d %d\n",m1,m2,n1,n2);RAG_FLUSH;
#endif

	ocrGuid_t pLocal_dbg;
        pLocal = (float **)spad_malloc(&pLocal_dbg,(cfar_params->Ncfar)*sizeof(float*)
						  +(cfar_params->Ncfar)*(cfar_params->Ncfar)*sizeof(float));
        if(pLocal == NULL) {
            PRINTF("Error allocating edge vector for local correlation map.\n");RAG_FLUSH;
            xe_exit(1);
        }

	float * pLocal_data_ptr = (float *)&pLocal[cfar_params->Ncfar];
        if (pLocal_data_ptr == NULL) {
            PRINTF("Error allocating data memory for local correlation map.\n");RAG_FLUSH;
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
                        pLocal[k][l] = corr_map[i][j].p;
                    } // for j
                } // for i

#ifdef DEBUG_LVL_2
PRINTF("////// Create guard window\n");RAG_FLUSH;
#endif
                for(int i=(cfar_params->Ncfar-1)/2-(cfar_params->Nguard-1)/2; i<=(cfar_params->Ncfar-1)/2+(cfar_params->Nguard-1)/2; i++) {
                    for(int j=(cfar_params->Ncfar-1)/2-(cfar_params->Nguard-1)/2; j<=(cfar_params->Ncfar-1)/2+(cfar_params->Nguard-1)/2; j++) {
                        pLocal[i][j] = -1;
                    } // for j
                } // for i

#ifdef DEBUG_LVL_3
PRINTF("////// Calculate threshold\n");RAG_FLUSH;
#endif
#ifdef RAG_PURE_FLOAT
                T = (int)floorf(cfar_params->Tcfar/100.0f*(cfar_params->Ncfar*cfar_params->Ncfar-cfar_params->Nguard*cfar_params->Nguard));
#else
                T = (int)floorf(cfar_params->Tcfar/100.0*(cfar_params->Ncfar*cfar_params->Ncfar-cfar_params->Nguard*cfar_params->Nguard));
#endif

                CUT = corr_map[mIndex][nIndex].p;

#ifdef DEBUG_LVL_2
PRINTF("////// Ensure CUT's correlation value is below the correlation threshold\n");RAG_FLUSH;
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
			    int nd = __sync_fetch_and_add(p_Nd,1);

			    Y[nd].x = image_params->xr[corr_map[mIndex][nIndex].x];
			    Y[nd].y = image_params->yr[corr_map[mIndex][nIndex].y];
			    Y[nd].p = CUT;

#ifdef DEBUG_LVL_2
PRINTF("detect %d x = 0x%x y = 0x%x p = 0x%x (%d,%d)\n",nd,*(uint32_t *)&Y[nd].x,*(uint32_t *)&Y[nd].y,*(uint32_t *)&Y[nd].p,*(int32_t *)&corr_map[mIndex][nIndex].x,*(int32_t *)&corr_map[mIndex][nIndex].y);RAG_FLUSH;
#endif
                    } // if cnt
                } // if CUT
            } // for n
        } // for m

        spad_free(pLocal,pLocal_dbg);

#ifdef TRACE_LVL_3
PRINTF("////// leave cfar_async_edt\n");RAG_FLUSH;
#endif
    return NULL_GUID;
}

void CFAR(
	struct point **corr_map, ocrGuid_t corr_map_dbg,
	struct ImageParams *image_params, ocrGuid_t image_params_dbg,
	struct CfarParams *cfar_params, ocrGuid_t cfar_params_dbg,
	struct detects *Y, ocrGuid_t Y_dbg, ocrGuid_t post_CFAR_scg) {
	int retval;
	int Mwins, Nwins;

#ifdef TRACE_LVL_2
PRINTF("//// enter CFAR\n");RAG_FLUSH;
#endif
	Mwins = image_params->Iy - image_params->Ncor - cfar_params->Ncfar + 2;
	Nwins = image_params->Ix - image_params->Ncor - cfar_params->Ncfar + 2;
#ifdef TRACE_LVL_2
PRINTF("//// Mwins == %d and Nwins == %d, Ncfar == %d, Ncor == %d\n",Mwins,Nwins,cfar_params->Ncfar,image_params->Ncor);RAG_FLUSH;
#endif

	ocrGuid_t Nd_dbg;
	int *Nd_ptr = bsm_malloc(&Nd_dbg,sizeof(int));
	*Nd_ptr = 0;
#ifdef RAG_NEW_BLK_SIZE
	int CFAR_ASYNC_BLOCK_SIZE_M = RAG_NEW_BLK_SIZE;
	int CFAR_ASYNC_BLOCK_SIZE_N = RAG_NEW_BLK_SIZE;
#else
	int CFAR_ASYNC_BLOCK_SIZE_M = blk_size(Mwins,32);
	int CFAR_ASYNC_BLOCK_SIZE_N = blk_size(Nwins,32);
	assert((Mwins%CFAR_ASYNC_BLOCK_SIZE_M)==0);
	assert((Nwins%CFAR_ASYNC_BLOCK_SIZE_N)==0);
#endif

#ifdef TRACE_LVL_2
PRINTF("//// satisfy non event guids for post_CFAR_scg\n");RAG_FLUSH;
#endif
RAG_DEF_MACRO_PASS(post_CFAR_scg,NULL,NULL,NULL,NULL,Y_dbg,0);
RAG_DEF_MACRO_PASS(post_CFAR_scg,NULL,NULL,NULL,NULL,Nd_dbg,1);

#ifdef TRACE_LVL_2
PRINTF("//// create a template for cfar_async function\n");RAG_FLUSH;
#endif
	ocrGuid_t cfar_async_clg;
	retval = ocrEdtTemplateCreate(
			&cfar_async_clg,	// ocrGuid_t *new_guid
			 cfar_async_edt,	// ocr_edt_ptr func_ptr
			(sizeof(struct corners_t) + sizeof(uint64_t) - 1)/sizeof(uint64_t), // paramc
			5);			// depc
	assert(retval==0);
	templateList[__sync_fetch_and_add(&templateIndex,1)] = cfar_async_clg;

#ifdef RAG_NEW_BLK_SIZE
	for(int m=0; m<Mwins; m+=CFAR_ASYNC_BLOCK_SIZE_M) {
		for(int n=0; n<Nwins; n+=CFAR_ASYNC_BLOCK_SIZE_N) {
			struct corners_t async_corners;
			async_corners.m1   = m;
			async_corners.m2   = (m+CFAR_ASYNC_BLOCK_SIZE_M)<Mwins?(m+CFAR_ASYNC_BLOCK_SIZE_M):Mwins;
			async_corners.n1   = n;
			async_corners.n2   = (n+CFAR_ASYNC_BLOCK_SIZE_N)<Nwins?(n+CFAR_ASYNC_BLOCK_SIZE_N):Nwins;
#else
	for(int m=0; m<Mwins; m+=CFAR_ASYNC_BLOCK_SIZE_M) {
		for(int n=0; n<Nwins; n+=CFAR_ASYNC_BLOCK_SIZE_N) {
			struct corners_t async_corners;
			async_corners.m1   = m;
			async_corners.m2   = m+CFAR_ASYNC_BLOCK_SIZE_M;
			async_corners.n1   = n;
			async_corners.n2   = n+CFAR_ASYNC_BLOCK_SIZE_N;
#endif
#ifdef TRACE_LVL_2
PRINTF("////// create an edt for cfar_async\n");RAG_FLUSH;
#endif
			ocrGuid_t cfar_async_scg;
			retval = ocrEdtCreate(
					&cfar_async_scg,	// *created_edt_guid
					 cfar_async_clg,	// edt_template_guid
					EDT_PARAM_DEF,		// paramc
					(uint64_t *)&async_corners,			// *paramv
					EDT_PARAM_DEF,		// depc
					NULL,			// *depv
					EDT_PROP_NONE,		// properties
					NULL_GUID,		// affinity
					NULL);			// *outputEvent
			assert(retval==0);

RAG_DEF_MACRO_PASS(cfar_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,0);
RAG_DEF_MACRO_PASS(cfar_async_scg,NULL,NULL,NULL,NULL,cfar_params_dbg,1);
RAG_DEF_MACRO_PASS(cfar_async_scg,NULL,NULL,NULL,NULL,corr_map_dbg,2);
RAG_DEF_MACRO_PASS(cfar_async_scg,NULL,NULL,NULL,NULL,Y_dbg,3);
RAG_DEF_MACRO_PASS(cfar_async_scg,NULL,NULL,NULL,NULL,Nd_dbg,4);
		} // for n
	} // for m

#ifdef TRACE_LVL_2
PRINTF("//// leave CFAR\n");RAG_FLUSH;
#endif
	return;
}
