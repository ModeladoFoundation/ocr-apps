#include "common.h"

void CCD(int Ncor, struct point **corr_map, struct ImageParams *image_params, struct complexData **curImage, struct complexData **refImage)
{
    int m, n, i, j, k;
    int Mwins, Nwins;
    int mIndex, nIndex;
    float den1, den2;
    struct complexData num;
    struct complexData *f, *g;
    struct complexData mu_f, mu_g;

    Mwins = image_params->Iy - Ncor + 1;
    Nwins = image_params->Ix - Ncor + 1;

    #pragma omp parallel private(f, g, m, mIndex, n, nIndex, i, mu_f, mu_g, j, k, num, den1, den2)
    {
        f = (struct complexData*)MALLOC_CHECKED(Ncor*Ncor*sizeof(struct complexData));
        g = (struct complexData*)MALLOC_CHECKED(Ncor*Ncor*sizeof(struct complexData));

        #pragma omp for
        for(m=0; m<Mwins; m++)
        {
            mIndex = (Ncor-1)/2 + m;
            for(n=0; n<Nwins; n++)
            {
                nIndex = (Ncor-1)/2 + n;

                for(i=mIndex-(Ncor-1)/2, k=mu_f.real=mu_f.imag=mu_g.real=mu_g.imag=0; i<=mIndex+(Ncor-1)/2; i++)
                {
                    for(j=nIndex-(Ncor-1)/2; j<=nIndex+(Ncor-1)/2; j++, k++)
                    {
                        f[k].real = curImage[i][j].real;
                        f[k].imag = curImage[i][j].imag;
                        g[k].real = refImage[i][j].real;
                        g[k].imag = refImage[i][j].imag;
                        mu_f.real += curImage[i][j].real;
                        mu_f.imag += curImage[i][j].imag;
                        mu_g.real += refImage[i][j].real;
                        mu_g.imag += refImage[i][j].imag;
                    }
                }

                mu_f.real /= Ncor*Ncor;
                mu_f.imag /= Ncor*Ncor;
                mu_g.real /= Ncor*Ncor;
                mu_g.imag /= Ncor*Ncor;

                for(k=num.real=num.imag=den1=den2=0; k<Ncor*Ncor; k++)
                {
                    f[k].real -= mu_f.real;
                    f[k].imag -= mu_f.imag;
                    g[k].real -= mu_g.real;
                    g[k].imag -= mu_g.imag;

                    num.real += f[k].real*g[k].real + f[k].imag*g[k].imag;
                    num.imag += f[k].real*g[k].imag - f[k].imag*g[k].real;

                    den1 += f[k].real*f[k].real + f[k].imag*f[k].imag;
                    den2 += g[k].real*g[k].real + g[k].imag*g[k].imag;
                }

                corr_map[m][n].x = nIndex;
                corr_map[m][n].y = mIndex;

#ifdef RAG_PURE_FLOAT
                if(den1 != 0.0f && den2 != 0.0f) {
#else
                if(den1 != 0.0 && den2 != 0.0) {
#endif
                    corr_map[m][n].p = sqrtf( (num.real*num.real + num.imag*num.imag) / (den1*den2) );
                }
                else {
                    corr_map[m][n].p = 0.0;
                }
            }
        }

        FREE_AND_NULL(f);
        FREE_AND_NULL(g);
    }
}
