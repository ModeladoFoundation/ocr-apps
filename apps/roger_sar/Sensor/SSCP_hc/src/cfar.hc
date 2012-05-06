#include <stdlib.h>

#include "common.h"

void cfar_async_body(int *p_Nd,
		struct ImageParams *image_params,
		struct CfarParams *cfar_params,
		struct point **corr_map,
		struct detects *Y,
		struct corners_t *corners)
{
	int m1   = corners->m1;
	int m2   = corners->m2;
	int n1   = corners->n1;
	int n2   = corners->n2;
// Correlation value threshold 
// (CUT's correlation value must be below Tcorr to be eligible for detection)
	float Tcorr = 0.9;
	float CUT;
	float **pLocal;
	pLocal = (float **)malloc(cfar_params->Ncfar*sizeof(float*));
	if(pLocal == NULL) {
		fprintf(stderr,"Error allocating memory for local correlation map.\n");fflush(stderr);
		exit(1);
	}

	float *pLocal_data_ptr = (float*)malloc(cfar_params->Ncfar*cfar_params->Ncfar*sizeof(float));
	if (pLocal_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for local correlation map.\n");fflush(stderr);
		exit(1);
	}
	for(int m=0; m<cfar_params->Ncfar; m++) {
		pLocal[m] = pLocal_data_ptr + m * cfar_params->Ncfar;
        }

	for(int m=m1; m<m2; m++) {
		int mIndex = (cfar_params->Ncfar-1)/2 + m;
		for(int n=n1; n<n2; n++) {
			int nIndex = (cfar_params->Ncfar-1)/2 + n;
			for(int i=mIndex-(cfar_params->Ncfar-1)/2, k=0; i<=mIndex+(cfar_params->Ncfar-1)/2; i++, k++) {
				for(int j=nIndex-(cfar_params->Ncfar-1)/2, l=0; j<=nIndex+(cfar_params->Ncfar-1)/2; j++, l++) {
					pLocal[k][l] = corr_map[i][j].p;
				} // for j
			} // for i

		// Create guard window
			for(int i=(cfar_params->Ncfar-1)/2-(cfar_params->Nguard-1)/2; i<=(cfar_params->Ncfar-1)/2+(cfar_params->Nguard-1)/2; i++) {
				for(int j=(cfar_params->Ncfar-1)/2-(cfar_params->Nguard-1)/2; j<=(cfar_params->Ncfar-1)/2+(cfar_params->Nguard-1)/2; j++) {
					pLocal[i][j] = -1;
				} // for j
			} // for k

		// Calculate threshold
#ifdef RAG_PURE_FLOAT
			int T = (int)floorf(cfar_params->Tcfar/100.0f*(cfar_params->Ncfar*cfar_params->Ncfar-cfar_params->Nguard*cfar_params->Nguard));
#else
			int T = (int)floorf(cfar_params->Tcfar/100.0*(cfar_params->Ncfar*cfar_params->Ncfar-cfar_params->Nguard*cfar_params->Nguard));
#endif

			CUT = corr_map[mIndex][nIndex].p;

		// Ensure CUT's correlation value is below the correlation threshold
			if(CUT < Tcorr) {
				int cnt = 0;
				for(int i=0; i<cfar_params->Ncfar; i++) {
					for(int j=0; j<cfar_params->Ncfar; j++) {
						if(CUT < pLocal[i][j]) {
							cnt++;
						}
					} // for j
				} // for i
                
				if(cnt >= T) {
					int nd = RAG_fetch_and_add(p_Nd,1);
#ifdef DEBUG
fprintf(stderr,"Nd = %d\n",nd);fflush(stderr);
#endif
					Y[nd].x = image_params->xr[corr_map[mIndex][nIndex].x];
					Y[nd].y = image_params->yr[corr_map[mIndex][nIndex].y];
					Y[nd].p = CUT;
				} // if cnt
			} // if CUT
		} // for n
	} // for m

	if (pLocal_data_ptr) {
		free(pLocal_data_ptr);
	}
	if (pLocal) {
		free(pLocal);
	}
	free(corners);
	return;
} // cfar_async_body

#ifdef RAG_QSORT_ON
// This function is used to sort the detections.  It returns -1 if _lhs < _rhs,
// 0 if they are equal, and 1 if _lhs > _rhs.  Ordering is performed primarily
// along the y axis and secondarily along the x axis.
static int compare_detects(const void * _lhs, const void * _rhs)
{
	const struct detects * const lhs = (/*const*/ struct detects * /*const*/) _lhs;
	const struct detects * const rhs = (/*const*/ struct detects * /*const*/) _rhs;

	if (lhs->y < rhs->y) { return -1; }
	else if (lhs->y > rhs->y) { return 1; }
	else {
        // y coordinates are equal; revert to comparing x coordinates
		if (lhs->x < rhs->x) { return -1; }
		else if (lhs->x > rhs->x) { return 1; }
		else { return 0; } // equality
	}
}
#endif

void CFAR(struct ImageParams *image_params, int Ncor, struct CfarParams *cfar_params, struct point **corr_map, struct detects *Y, FILE *pOutFile)
{
	int *p_Nd;
	int Mwins, Nwins;

	Mwins = image_params->Iy - Ncor - cfar_params->Ncfar + 2;
	Nwins = image_params->Ix - Ncor - cfar_params->Ncfar + 2;
	int CFAR_ASYNC_BLOCK_SIZE_M = blk_size(Mwins,32);
	int CFAR_ASYNC_BLOCK_SIZE_N = blk_size(Nwins,32);
	assert((Mwins%CFAR_ASYNC_BLOCK_SIZE_M)==0);
	assert((Nwins%CFAR_ASYNC_BLOCK_SIZE_N)==0);
#ifdef DEBUG_RAG
fprintf(stderr,"Mwins == %d and Nwins == %d, Ncfar == %d, Ncor == %d\n",Mwins,Nwins,cfar_params->Ncfar,Ncor);fflush(stderr);
#endif

	p_Nd = (int *)malloc(sizeof(int));
	if(p_Nd == NULL) {
		fprintf(stderr,"Error allocating memory for variable holding the number of detects.\n");fflush(stderr);
		exit(1);
	}
	*p_Nd = 0;
finish{
	for(int m=0; m<Mwins; m+=CFAR_ASYNC_BLOCK_SIZE_M) {
		for(int n=0; n<Nwins; n+=CFAR_ASYNC_BLOCK_SIZE_N) {
			struct corners_t *corners;
			corners = malloc(sizeof(struct corners_t));
			corners->m1 = m;
			corners->m2 = m+CFAR_ASYNC_BLOCK_SIZE_M;
			corners->n1 = n;
			corners->n2 = n+CFAR_ASYNC_BLOCK_SIZE_N;
#if 1
			async        IN(p_Nd,image_params,cfar_params,corr_map,Y,corners)
#endif
			cfar_async_body(p_Nd,image_params,cfar_params,corr_map,Y,corners);
		} // for n
	} // for m
} // finish

#ifdef RAG_QSORT_ON
	// Sorting the detections is not strictly necessary. This is only done so
	// that we can directly diff the output detections file for validation
	// purposes.  They can be out-of-order due to the OpenMP parallelization
	// of the above loop.
	qsort(Y, *p_Nd, sizeof(struct detects), compare_detects);
#endif

#ifdef TRACE
fprintf(stderr,"Output to file\n");fflush(stderr);
#endif
	// Output to file
	for(int m=0; m<*p_Nd; m++) {
		fprintf(pOutFile, "x=%7.2fm y=%7.2fm p=%4.2f\n", Y[m].x, Y[m].y, Y[m].p);
	}
	free(p_Nd);
	return;
}
