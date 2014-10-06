#include <stdlib.h>

#include "common.h"

float Tcorr = 0.8;              // Correlation value threshold (CUT's correlation value must be below Tcorr to be eligible for detection)

// This function is used to sort the detections.  It returns -1 if _lhs < _rhs,
// 0 if they are equal, and 1 if _lhs > _rhs.  Ordering is performed primarily
// along the y axis and secondarily along the x axis.
static int compare_detects(const void * _lhs, const void * _rhs)
{
    const struct detects * const lhs = (const struct detects * const) _lhs;
    const struct detects * const rhs = (const struct detects * const) _rhs;

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


int CFAR(struct ImageParams *image_params, int Ncor, struct CfarParams *cfar_params, struct point **corr_map, struct detects *Y)
{
    int m, n, i, j, k, l;
    int T, cnt, Nd;
    int Mwins, Nwins;
    int mIndex, nIndex;
    float CUT;
    float **pLocal;

    Mwins = image_params->Iy - Ncor - cfar_params->Ncfar + 2;
    Nwins = image_params->Ix - Ncor - cfar_params->Ncfar + 2;

    Nd = 0;
    #pragma omp parallel private(pLocal, m, mIndex, n, nIndex, i, k, j, l, T, CUT, cnt)
    {
        pLocal = (float **)MALLOC_CHECKED(cfar_params->Ncfar*sizeof(float*));

        for(m=0; m<cfar_params->Ncfar; m++)
        {
            pLocal[m] = (float*)MALLOC_CHECKED(cfar_params->Ncfar*sizeof(float));
        }

        #pragma omp for
        for(m=0; m<Mwins; m++)
        {
            mIndex = (cfar_params->Ncfar-1)/2 + m;
            for(n=0; n<Nwins; n++)
            {
                nIndex = (cfar_params->Ncfar-1)/2 + n;

                for(i=mIndex-(cfar_params->Ncfar-1)/2, k=0; i<=mIndex+(cfar_params->Ncfar-1)/2; i++, k++)
                {
                    for(j=nIndex-(cfar_params->Ncfar-1)/2, l=0; j<=nIndex+(cfar_params->Ncfar-1)/2; j++, l++)
                    {
                        pLocal[k][l] = corr_map[i][j].p;
                    }
                }

                // Create guard window
                for(i=(cfar_params->Ncfar-1)/2-(cfar_params->Nguard-1)/2; i<=(cfar_params->Ncfar-1)/2+(cfar_params->Nguard-1)/2; i++)
                {
                    for(j=(cfar_params->Ncfar-1)/2-(cfar_params->Nguard-1)/2; j<=(cfar_params->Ncfar-1)/2+(cfar_params->Nguard-1)/2; j++)
                    {
                        pLocal[i][j] = -1;
                    }
                }

                // Calculate threshold
#ifdef RAG_PURE_FLOAT
                T = (int)floorf(cfar_params->Tcfar/100.0f*(cfar_params->Ncfar*cfar_params->Ncfar-cfar_params->Nguard*cfar_params->Nguard));
#else
                T = (int)floorf(cfar_params->Tcfar/100.0*(cfar_params->Ncfar*cfar_params->Ncfar-cfar_params->Nguard*cfar_params->Nguard));
#endif

                CUT = corr_map[mIndex][nIndex].p;

                // Ensure CUT's correlation value is below the correlation threshold
                if(CUT < Tcorr)
                {
                    for(i=cnt=0; i<cfar_params->Ncfar; i++)
                    {
                        for(j=0; j<cfar_params->Ncfar; j++)
                        {
                            if(CUT < pLocal[i][j]) {
                                cnt++;
                            }
                        }
                    }

                    if(cnt >= T)
                    {
                        #pragma omp critical
                        {
                            Y[Nd].x = image_params->xr[corr_map[mIndex][nIndex].x];
                            Y[Nd].y = image_params->yr[corr_map[mIndex][nIndex].y];
                            Y[Nd++].p = CUT;
                        }
                    }
                }
            }
        }

        for(m=0; m<cfar_params->Ncfar; m++)
        {
            if (pLocal && pLocal[m])
            {
                FREE_AND_NULL(pLocal[m]);
            }
        }
        FREE_AND_NULL(pLocal);
    }

    // Sorting the detections is not strictly necessary. This is only done so
    // that we can directly diff the output detections file for validation
    // purposes.  They can be out-of-order due to the OpenMP parallelization
    // of the above loop.
    qsort(Y, Nd, sizeof(struct detects), compare_detects);

    return Nd;
}
