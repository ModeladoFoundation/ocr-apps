#include <stdlib.h>

#include "common.h"

#include "rag_rmd.h"

float Tcorr = 0.9;              // Correlation value threshold (CUT's correlation value must be below Tcorr to be eligible for detection)                                                                                       

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

rmd_guid_t cfar_async_body( int m1, int m2, int n1, int n2,
	struct ImageParams *image_params,
	int Ncor,
	struct CfarParams *cfar_params,
	struct point **corr_map,
	struct detects *Y,
	int *Nd)
{
    int T,cnt;
    int mIndex, nIndex;
    float CUT;
    float **pLocal;

#ifdef TRACE
    xe_printf("cfar_async_body\n");RAG_FLUSH;
#endif

	rmd_guid_t pLocal_dbg;
        pLocal = (float **)spad_malloc(&pLocal_dbg,cfar_params->Ncfar*sizeof(float*));
        if(pLocal == NULL) {
            xe_printf("Error allocating edge vector for local correlation map.\n");RAG_FLUSH;
            exit(1);
        }

	float * pLocal_data_ptr; rmd_guid_t pLocal_data_dbg;
        pLocal_data_ptr = (float*)spad_malloc(&pLocal_data_dbg,
		cfar_params->Ncfar*(cfar_params->Ncfar*sizeof(float)));
        if (pLocal_data_ptr == NULL) {
            xe_printf("Error allocating data memory for local correlation map.\n");RAG_FLUSH;
            exit(1);
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
#ifdef DEBUG
xe_printf("m %d n %d i %d j %d\n",m,n,i,j);RAG_FLUSH;
#endif
#if defined(RAG_SIM) || defined(RAG_AFL)
			struct point *corr_map_i;
			corr_map_i = (struct point *)RAG_GET_PTR(corr_map+i);
			struct point *corr_map_i_j = corr_map_i+j;
			float *corr_map_i_j_p = (float *)(((char *)corr_map_i_j)+offsetof(struct point,p));
#ifdef RAG_AFL
assert(corr_map_i_j_p == &corr_map[i][j].p);
#endif
                        REM_LD32_ADDR(pLocal[k][l],corr_map_i_j_p);
#ifdef RAG_AFL
assert(pLocal[k][l] == corr_map[i][j].p);
#endif
#else
                        pLocal[k][l] = corr_map[i][j].p;
#endif
                    }
                }

#ifdef DEBUG
xe_printf("// Create guard window\n");RAG_FLUSH;
#endif
                for(int i=(cfar_params->Ncfar-1)/2-(cfar_params->Nguard-1)/2; i<=(cfar_params->Ncfar-1)/2+(cfar_params->Nguard-1)/2; i++) {
                    for(int j=(cfar_params->Ncfar-1)/2-(cfar_params->Nguard-1)/2; j<=(cfar_params->Ncfar-1)/2+(cfar_params->Nguard-1)/2; j++) {
                        pLocal[i][j] = -1;
                    }
                }

#ifdef DEBUG
xe_printf("// Calculate threshold\n");RAG_FLUSH;
#endif
#if RAG_PURE_FLOAT
                T = (int)floorf(cfar_params->Tcfar/100.0f*(cfar_params->Ncfar*cfar_params->Ncfar-cfar_params->Nguard*cfar_params->Nguard));
#else
                T = (int)floorf(cfar_params->Tcfar/100.0*(cfar_params->Ncfar*cfar_params->Ncfar-cfar_params->Nguard*cfar_params->Nguard));
#endif

#if defined(RAG_SIM) || defined(RAG_AFL)
		struct point *corr_map_m;
		corr_map_m = (struct point *)RAG_GET_PTR(corr_map+mIndex);
		struct point *corr_map_m_n = corr_map_m+nIndex;
		float *corr_map_m_n_p = (float *)(((char *)corr_map_m_n)+offsetof(struct point,p));
#ifdef RAG_AFL
assert(corr_map_m_n_p == &corr_map[mIndex][nIndex].p);
#endif
                REM_LD32_ADDR(CUT,corr_map_m_n_p);
#else
                CUT = corr_map[mIndex][nIndex].p;
#endif

#ifdef DEBUG
xe_printf("// Ensure CUT's correlation value is below the correlation threshold\n");RAG_FLUSH;
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
			    int nd = __sync_fetch_and_add(Nd,1);
			    struct detects Y_nd;
#ifdef RAG_AFL
assert(&Y[nd] == (Y+nd));
#endif
                            Y_nd.x = RAG_GET_FLT(image_params->xr+RAG_GET_INT((int32_t *)corr_map_m_n_x));
                            Y_nd.y = RAG_GET_FLT(image_params->yr+RAG_GET_INT((int32_t *)corr_map_m_n_y));
			    Y_nd.p = CUT;
#ifdef RAG_AFL
assert(Y_nd.x == image_params->xr[corr_map[mIndex][nIndex].x]);
assert(Y_nd.y == image_params->yr[corr_map[mIndex][nIndex].y]);
#endif
#ifdef TRACE
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

    return NULL_GUID;
}
int CFAR(struct ImageParams *image_params, int Ncor, struct CfarParams *cfar_params, struct point **corr_map, struct detects *Y)
{
    int Nd;
    int Mwins, Nwins;

    Mwins = image_params->Iy - Ncor - cfar_params->Ncfar + 2;
    Nwins = image_params->Ix - Ncor - cfar_params->Ncfar + 2;
#ifdef TRACE
    xe_printf("Mwins == %d and Nwins == %d, Ncfar == %d, Ncor == %d\n",Mwins,Nwins,cfar_params->Ncfar,Ncor);RAG_FLUSH;
#endif

    Nd = 0;
#define BLOCK_SIZE 4
    assert((Mwins%BLOCK_SIZE)==0);
    assert((Nwins%BLOCK_SIZE)==0);
    for(int m=0; m<Mwins; m+=BLOCK_SIZE) {
        for(int n=0; n<Nwins; n+=BLOCK_SIZE) {
	    cfar_async_body( m, m+BLOCK_SIZE, n, n+BLOCK_SIZE,
		image_params, Ncor, cfar_params, corr_map, Y, &Nd);
        } // for n
    } // for m

#if RAG_QSORT_ON
    // Sorting the detections is not strictly necessary. This is only done so
    // that we can directly diff the output detections file for validation
    // purposes.  They can be out-of-order due to the OpenMP parallelization
    // of the above loop.

    qsort(Y, Nd, sizeof(struct detects), compare_detects);
#else
#ifdef TRACE
xe_printf("// SKIPPING Sort of the detections\n");RAG_FLUSH;
#endif
#endif

    return Nd;
}
