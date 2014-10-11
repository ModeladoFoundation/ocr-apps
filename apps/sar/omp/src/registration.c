#include "common.h"

int **A;
int *Fx, *Fy;
struct complexData **output;

#define GAUSS_ELIM_SUCCESS 0
#define GAUSS_ELIM_SINGULAR_MATRIX 1

int gauss_elim(float *AA[], float *x, int N);
static struct point corr2D(struct point ctrl_pt, int Nwin, int R, struct complexData**, struct complexData**, struct ImageParams*, struct complexData *, struct complexData *);

void Registration(struct RegistrationParams *registration_params, struct ImageParams *image_params, struct complexData **curImage, struct complexData **refImage)
{
    int m, n, k;
    int N, min;
    int dx, dy;
    int aa, bb;
    int rc;
    float v, w;
    float Px, Py;
    float b[6][2];
    float **aug_mat;
    float Wcx[6], Wcy[6];
    struct point disp_vec, ctrl_pt;

    // Fx = Nc x 1
    Fx = (int*)CALLOC_CHECKED(registration_params->Nc,sizeof(int));

    // Fy = Nc x 1
    Fy = (int*)CALLOC_CHECKED(registration_params->Nc,sizeof(int));

    // A = Nc x 6
    A = (int**)CALLOC_CHECKED(registration_params->Nc,sizeof(int*));
    for(m=0; m<registration_params->Nc; m++) {
        A[m] = (int*)CALLOC_CHECKED(6,sizeof(int));
    }

    // aug_mat = 6 x 7
    aug_mat = (float**)CALLOC_CHECKED(6,sizeof(float*));
    for(n=0; n<6; n++) {
        aug_mat[n] = (float*)CALLOC_CHECKED(7,sizeof(float));
    }

    // Allocate memory for output image
    output = (struct complexData**)MALLOC_CHECKED(image_params->Iy2*sizeof(struct complexData*));
    for(n=0; n<image_params->Iy2; n++) {
        output[n] = (struct complexData*)MALLOC_CHECKED(image_params->Ix2*sizeof(struct complexData));
    }

    N = (int)sqrtf((float)registration_params->Nc);
    min = (registration_params->Sc-1)/2 + registration_params->Rc;

    dy = (image_params->Iy2 - registration_params->Sc + 1 - 2*registration_params->Rc)/N;
    dx = (image_params->Ix2 - registration_params->Sc + 1 - 2*registration_params->Rc)/N;

    k = 0;
#if 0
fprintf(stderr,"RAG Nc = %d\n",registration_params->Nc);fflush(stderr);
#endif
    registration_params->Nc = 0;
    #pragma omp parallel
    {
        const int Sc = registration_params->Sc;
        struct complexData *f = (struct complexData*)MALLOC_CHECKED(Sc*Sc*sizeof(struct complexData));
        struct complexData *g = (struct complexData*)MALLOC_CHECKED(Sc*Sc*sizeof(struct complexData));

        #pragma omp for private(n, ctrl_pt, disp_vec)
        for(m=0; m<N; m++)
        {
            ctrl_pt.y = m*dy + min;
            for(n=0; n<N; n++)
            {
                ctrl_pt.x = n*dx + min;

                // Perform 2D correlation
                disp_vec = corr2D(ctrl_pt, registration_params->Sc, registration_params->Rc, curImage, refImage, image_params, f, g);

                // Only retain control points that exceed the threshold
                if(disp_vec.p >= registration_params->Tc)
                {
                    #pragma omp critical
                    {
                        // Form Fx, Fy
                        Fx[k] = ctrl_pt.x + disp_vec.x;
                        Fy[k] = ctrl_pt.y + disp_vec.y;

                        // Form A
                        A[k][0] = 1;
                        A[k][1] = ctrl_pt.x;
                        A[k][2] = ctrl_pt.y;
                        A[k][3] = ctrl_pt.x*ctrl_pt.x;
                        A[k][4] = ctrl_pt.y*ctrl_pt.y;
                        A[k][5] = ctrl_pt.x*ctrl_pt.y;

                        k++;

                        // Number of control points after pruning
                        registration_params->Nc = k;
                    }
                }
            }
        }

        FREE_AND_NULL(f);
        FREE_AND_NULL(g);
    }
#if 0
fprintf(stderr,"RAG Nc = %d\n",registration_params->Nc);fflush(stderr)
#endif

    // b = A'F
    // The computation of A'F and A'A requires better precision than would be obtained
    // by simply typecasting the A components to floats for the accumulation.  Here,
    // we use double precision accumulators, although 64-bit integer accumulations or
    // dynamic range rescaling provide other viable options to obtain additional precision.
    for(m=0; m<6; m++) {
        double accum_x = 0.0, accum_y = 0.0;
        for(n=0; n<registration_params->Nc; n++) {
            accum_x += (double)A[n][m]*(double)Fx[n];
            accum_y += (double)A[n][m]*(double)Fy[n];
        }
        b[m][0] = accum_x;
        b[m][1] = accum_y;
    }

    // aug_mat(1:6,1:6) = A'A
    for(m=0; m<6; m++) {
        for(n=0; n<6; n++) {
            double accum = 0.0;
            for(k=0; k<registration_params->Nc; k++) {
                accum += (double) A[k][m] * (double) A[k][n];
            }
            aug_mat[m][n] = (float)accum;
        }
    }

    // aug_mat(1:6,7) = b(1:6,1)
    for(m=0; m<6; m++) {
        aug_mat[m][6] = b[m][0];
    }

    // Perform Gaussian elimination to find Wcx
    rc = gauss_elim(aug_mat, Wcx, 6);
    if (rc != GAUSS_ELIM_SUCCESS)
    {
        // Default to the identity if Gaussian elimination fails
        memset(Wcx, 0, sizeof(float) * 6);
        Wcx[1] = 1.0f;
    }
#if 1
printf("Wcx %f %f %f %f %f %f\n",Wcx[0],Wcx[1],Wcx[2],Wcx[3],Wcx[4],Wcx[5]);fflush(stdout);
#endif

    // aug_mat(1:6,7) = b(1:6,2)
    for(m=0; m<6; m++) {
        aug_mat[m][6] = b[m][1];
    }

    // Perform Gaussian elimination to find Wcy
    rc = gauss_elim(aug_mat, Wcy, 6);
    if (rc != GAUSS_ELIM_SUCCESS)
    {
        // Default to the identity if Gaussian elimination fails
        memset(Wcy, 0, sizeof(float) * 6);
        Wcy[2] = 1.0f;
    }
#if 1
printf("Wcy %f %f %f %f %f %f\n",Wcy[0],Wcy[1],Wcy[2],Wcy[3],Wcy[4],Wcy[5]);fflush(stdout);
#endif

    // Loop over the output pixel locations and interpolate the Target image
    // pixel values at these points. This is done by mapping the (rectangular)
    // Source coordinates into the Target coordinates and performing the
    // interpolation there.
    #pragma omp parallel for private(n, Px, Py, aa, bb, w, v)
    for(m=0; m<image_params->Iy2; m++)
    {
        for(n=0; n<image_params->Ix2; n++)
        {
            Px = Wcx[0] + Wcx[1]*(float)n + Wcx[2]*(float)m + Wcx[3]*(float)n*(float)n + Wcx[4]*(float)m*(float)m + Wcx[5]*(float)n*(float)m;
            Py = Wcy[0] + Wcy[1]*(float)n + Wcy[2]*(float)m + Wcy[3]*(float)n*(float)n + Wcy[4]*(float)m*(float)m + Wcy[5]*(float)n*(float)m;

            aa = (int)floorf(Py);
            bb = (int)floorf(Px);

            w = Py - (float)aa;
            v = Px - (float)bb;
#if 0
static int rag_first = 1;
if(rag_first)fprintf(stderr,"RAG (%d,%d) -> <%d,%d>\n",aa,bb,m,n);
rag_first=0;
#endif
            if( (aa >= 0) && (aa < image_params->Iy2-1) && (bb >= 0) && (bb < image_params->Ix2-1) ) {
                output[m][n].real = (1-v)*(1-w)*curImage[aa][bb].real + v*(1-w)*curImage[aa][bb+1].real + (1-v)*w*curImage[aa+1][bb].real + v*w*curImage[aa+1][bb+1].real;
                output[m][n].imag = (1-v)*(1-w)*curImage[aa][bb].imag + v*(1-w)*curImage[aa][bb+1].imag + (1-v)*w*curImage[aa+1][bb].imag + v*w*curImage[aa+1][bb+1].imag;
            }
            else {
                output[m][n].real = 0.0;
                output[m][n].imag = 0.0;
            }
        }
    }

    // Overwrite current image with registered image
    for(m=0; m<image_params->Iy2; m++) {
        memcpy(&curImage[m][0], &output[m][0], image_params->Ix2*sizeof(struct complexData));
    }

    for(n=0; n<6; n++) {
        FREE_AND_NULL(aug_mat[n]);
    }
    FREE_AND_NULL(aug_mat);

    for(m=0; m<registration_params->Nc; m++) {
        FREE_AND_NULL(A[m]);
    }
    FREE_AND_NULL(A);

    FREE_AND_NULL(Fx);
    FREE_AND_NULL(Fy);
    for(n=0; n<image_params->Iy2; n++) {
        FREE_AND_NULL(output[n]);
    }
    FREE_AND_NULL(output);
}

struct point corr2D(
    struct point ctrl_pt,
    int Nwin,
    int R,
    struct complexData **curImage,
    struct complexData **refImage,
    struct ImageParams *image_params,
    struct complexData *f, // work buffer
    struct complexData *g) // work buffer
{
    int m, n, i, j, k;
    float den1, den2;
    float rho;
    struct point pt;
    struct complexData num;
    struct complexData mu_f, mu_g;

    for(i=ctrl_pt.y-(Nwin-1)/2, k=mu_f.real=mu_f.imag=0; i<=ctrl_pt.y+(Nwin-1)/2; i++)
    {
        for(j=ctrl_pt.x-(Nwin-1)/2; j<=ctrl_pt.x+(Nwin-1)/2; j++, k++)
        {
            if( (i < 0) || (j < 0) || (i >= image_params->Iy2) || (j >= image_params->Ix2) ) {
                printf("Warning: Index out of bounds in registration correlation.\n");
            }
            f[k].real = curImage[i][j].real;
            f[k].imag = curImage[i][j].imag;
            mu_f.real += curImage[i][j].real;
            mu_f.imag += curImage[i][j].imag;
        }
    }

    mu_f.real /= Nwin*Nwin;
    mu_f.imag /= Nwin*Nwin;

    for(k=0; k<Nwin*Nwin; k++)
    {
        f[k].real -= mu_f.real;
        f[k].imag -= mu_f.imag;
    }

    pt.x = pt.y = -1;

    for(m=ctrl_pt.y-R, pt.p=0; m<=ctrl_pt.y+R; m++)
    {
        for(n=ctrl_pt.x-R; n<=ctrl_pt.x+R; n++)
        {
            for(i=m-(Nwin-1)/2, k=mu_g.real=mu_g.imag=0; i<=m+(Nwin-1)/2; i++)
            {
                for(j=n-(Nwin-1)/2; j<=n+(Nwin-1)/2; j++, k++)
                {
                    if( (i < 0) || (j < 0) || (i >= image_params->Iy2) || (j >= image_params->Ix2) ) {
                        printf("Warning: Index out of bounds in registration correlation.\n");
                    }
                    g[k].real = refImage[i][j].real;
                    g[k].imag = refImage[i][j].imag;
                    mu_g.real += refImage[i][j].real;
                    mu_g.imag += refImage[i][j].imag;
                }
            }

            mu_g.real /= Nwin*Nwin;
            mu_g.imag /= Nwin*Nwin;

            for(k=num.real=num.imag=den1=den2=0; k<Nwin*Nwin; k++)
            {
                g[k].real -= mu_g.real;
                g[k].imag -= mu_g.imag;

                num.real += f[k].real*g[k].real + f[k].imag*g[k].imag;
                num.imag += f[k].real*g[k].imag - f[k].imag*g[k].real;

                den1 += f[k].real*f[k].real + f[k].imag*f[k].imag;
                den2 += g[k].real*g[k].real + g[k].imag*g[k].imag;
            }

            if(den1 != 0.0 && den2 != 0.0) {
                rho = sqrtf( (num.real*num.real + num.imag*num.imag) / (den1*den2) );
            }
            else {
                rho = 0.0;
            }

/*          if(rho < 0 || rho > 1) {
                printf("Correlation value out of range.\n");
                exit(-1);
            }*/

            if(rho > pt.p) {
                pt.x = ctrl_pt.x-n;
                pt.y = ctrl_pt.y-m;
                pt.p = rho;
            }
        }
    }

    return pt;
}

// Returns 0 on success and non-zero on failure
int gauss_elim(float *AA[], float *x, int N)
{
    int i, j, k, max;
    float **a, temp;

    a = (float**)MALLOC_CHECKED(N*sizeof(float*));
    for(i=0; i<N; i++) {
        a[i] = (float*)MALLOC_CHECKED((N+1)*sizeof(float));
    }

    for(i=0; i<N; i++) {
        for(j=0; j<N+1; j++) {
            a[i][j] = AA[i][j];
        }
    }

    for(i=0; i<N; i++)
    {
        // Find the largest value row
        max = i;
        for(j=i+1; j<N; j++) {
            if(fabsf(a[j][i]) > fabsf(a[max][i])) {
                max = j;
            }
        }

        // Swap the largest row with the ith row
        for(k=i; k<N+1; k++) {
            temp = a[i][k];
            a[i][k] = a[max][k];
            a[max][k] = temp;
        }

        // Check to see if this is a singular matrix
        if(fabsf(a[i][i]) == 0.0) {
            printf("Warning: Encountered a singular matrix in registration correlation.\n");
            for(i=0; i<N; i++) { if (a[i]) { free(a[i]); a[i] = NULL; } }
            if (a) { free(a); a = NULL; }
            return GAUSS_ELIM_SINGULAR_MATRIX;
        }

        // Starting from row i+1, eliminate the elements of the ith column
        for(j=i+1; j<N; j++) {
            if(a[j][i] != 0)
            {
                for(k=N; k>=i; k--) {
                    a[j][k] -= a[i][k] * a[j][i] / a[i][i];
                }
            }
        }
    }

    // Perform the back substitution
    for(j=N-1; j>=0; j--) {
        temp = 0;
        for(k=j+1; k<N; k++) {
            temp += a[j][k] * x[k];
        }
        x[j] = (a[j][N] - temp) / a[j][j];
    }

    for(i=0; i<N; i++) { if (a[i]) { free(a[i]); a[i] = NULL; } }
    if (a) { free(a); a = NULL; }

    return 0;
}
