#include "common.h"

// Affine registration dynamically allocated variables
int **A;
int *Fx, *Fy;
struct complexData **output;

// Thin spline dynamically allocated variables
int **Yf, **P;
float **L;
float *Wfx, *Wfy;

void gauss_elim(float *AA[], float *x, int N);
struct point corr2D(struct point ctrl_pt, int Nwin, int R, struct complexData**, struct complexData**, struct ImageParams*);

//struct reg_map *regmap;

void Affine(struct AffineParams *affine_params, struct ImageParams *image_params, struct complexData **curImage, struct complexData **refImage)
{
    int m, n, k;
    int N, min;
    int dx, dy;
    int aa, bb;
    float v, w;
    float Px, Py;
    float b[6][2];
    float **aug_mat;
    float Wcx[6], Wcy[6];
    struct point disp_vec, ctrl_pt;

//  regmap = (struct reg_map*)malloc(Nf*sizeof(struct reg_map));

    // Fx = Nc x 1
    Fx = (int*)calloc(affine_params->Nc,sizeof(int));
    if(Fx == NULL) {
        fprintf(stderr,"Unable to allocate memory for Fx.\n");fflush(stderr);
    }

    // Fy = Nc x 1
    Fy = (int*)calloc(affine_params->Nc,sizeof(int));
    if(Fy == NULL) {
        fprintf(stderr,"Unable to allocate memory for Fy.\n");fflush(stderr);
    }

    // A = Nc x 6
    A = (int**)calloc(affine_params->Nc,sizeof(int*));
    if(A == NULL) {
        fprintf(stderr,"Error allocating memory for A.\n");
        exit(1);
    }
    for(m=0; m<affine_params->Nc; m++) {
        A[m] = (int*)calloc(6,sizeof(int));
        if(A[m] == NULL) {
            fprintf(stderr,"Error allocating memory for A.\n");
            exit(1);
        }
    }

    // aug_mat = 6 x 7
    aug_mat = (float**)calloc(6,sizeof(float*));
    if(aug_mat == NULL) {
        fprintf(stderr,"Unable to allocate memory for aug_mat.\n");
        exit(1);
    }
    for(n=0; n<6; n++) {
        aug_mat[n] = (float*)calloc(7,sizeof(float));
        if(aug_mat[n] == NULL) {
            fprintf(stderr,"Unable to allocate memory for aug_mat.\n");
            exit(1);
        }
    }

    // Allocate memory for output image
    output = (struct complexData**)malloc(image_params->Iy*sizeof(struct complexData*));
    if(output == NULL) {
        fprintf(stderr,"Error allocating memory for output.\n");
        exit(1);
    }
    for(n=0; n<image_params->Iy; n++) {
        output[n] = (struct complexData*)malloc(image_params->Ix*sizeof(struct complexData));
        if (output[n] == NULL) {
            fprintf(stderr,"Error allocating memory for output.\n");
            exit(1);
        }
    }

    N = (int)sqrtf((float)affine_params->Nc);
    min = (affine_params->Sc-1)/2 + affine_params->Rc;

    dy = (image_params->Iy - affine_params->Sc + 1 - 2*affine_params->Rc)/N;
    dx = (image_params->Ix - affine_params->Sc + 1 - 2*affine_params->Rc)/N;
                
    k = 0;
    affine_params->Nc = 0;  
#ifdef _OPENMP
    #pragma omp parallel for private(n, ctrl_pt, disp_vec)
#endif
    for(m=0; m<N; m++)
    {
        ctrl_pt.y = m*dy + min;
        for(n=0; n<N; n++)
        {
            ctrl_pt.x = n*dx + min;
            
            // Perform 2D correlation
            disp_vec = corr2D(ctrl_pt, affine_params->Sc, affine_params->Rc, curImage, refImage, image_params);

            // Only retain control points that exceed the threshold
            if(disp_vec.p >= affine_params->Tc) 
            {
#ifdef _OPENMP
                #pragma omp critical
#endif
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

                    /*              regmap[k].x = ctrl_pt.x;
                    regmap[k].y = ctrl_pt.y;
                    regmap[k].u = disp_vec.x;
                    regmap[k].v = disp_vec.y;
                    regmap[k].p = disp_vec.p;*/

                    k++;

                    // Number of control points after pruning
                    affine_params->Nc = k;  
                }
            }
        }
    }

    // b = A'F
    // The computation of A'F and A'A requires better precision than would be obtained
    // by simply typecasting the A components to floats for the accumulation.  Here,
    // we use double precision accumulators, although 64-bit integer accumulations or
    // dynamic range rescaling provide other viable options to obtain additional precision.
    for(m=0; m<6; m++) {
        double accum_x = 0.0, accum_y = 0.0;
        for(n=0; n<affine_params->Nc; n++) {
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
            for(k=0; k<affine_params->Nc; k++) {
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
    gauss_elim(aug_mat, Wcx, 6);

    // aug_mat(1:6,7) = b(1:6,2)
    for(m=0; m<6; m++) {
        aug_mat[m][6] = b[m][1];
    }

    // Perform Gaussian elimination to find Wcy
    gauss_elim(aug_mat, Wcy, 6);

    // Loop over the output pixel locations and interpolate the Target image
    // pixel values at these points. This is done by mapping the (rectangular)
    // Source coordinates into the Target coordinates and performing the
    // interpolation there.
#ifdef _OPENMP
    #pragma omp parallel for private(n, Px, Py, aa, bb, w, v)
#endif
    for(m=0; m<image_params->Iy; m++)
    {
        for(n=0; n<image_params->Ix; n++)
        {
            Px = Wcx[0] + Wcx[1]*(float)n + Wcx[2]*(float)m + Wcx[3]*(float)n*(float)n + Wcx[4]*(float)m*(float)m + Wcx[5]*(float)n*(float)m;
            Py = Wcy[0] + Wcy[1]*(float)n + Wcy[2]*(float)m + Wcy[3]*(float)n*(float)n + Wcy[4]*(float)m*(float)m + Wcy[5]*(float)n*(float)m;

            aa = (int)floorf(Py);
            bb = (int)floorf(Px);
#if 0
//RAG DEBUG PRINT
printf("<%4d,%4d> <%4d,%4d>\n",m,n,aa,bb);
#endif
            w = Py - (float)aa;
            v = Px - (float)bb; 

            if( (aa >= 0) && (aa < image_params->Iy-1) && (bb >= 0) && (bb < image_params->Ix-1) ) {
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
    for(m=0; m<image_params->Iy; m++) {
        memcpy(&curImage[m][0], &output[m][0], image_params->Ix*sizeof(struct complexData));
    }

    free(A);
    free(Fx);
    free(Fy);
    free(output);
    free(aug_mat);
}

void ThinSpline(struct ThinSplineParams *ts_params, struct ImageParams *image_params, struct complexData **curImage, struct complexData **refImage)
{
    int i, j, m, n, k;
    int N, min;
    int dx, dy;
    int aa, bb;
    float w, v;
    float warped_pt[2];
    float r, sum_x, sum_y;
    struct point disp_vec, ctrl_pt;

//  regmap = (struct reg_map*)malloc(Nf*sizeof(struct reg_map));

    // L = Nf+3 x Nf+3+1 (+1 because L is used as augmented matrix)
    L = (float**)malloc((ts_params->Nf+3)*sizeof(float*));
    if(L == NULL) {
        fprintf(stderr,"Unable to allocate memory for L.\n");fflush(stderr);
        exit(-1);
    }
    for(m=0; m<ts_params->Nf+3; m++) {
        L[m] = (float*)malloc((ts_params->Nf+3+1)*sizeof(float));
        if(L[m] == NULL) {
            fprintf(stderr,"Unable to allocate memory for L.\n");fflush(stderr);
            exit(-1);
        }
    }

    // Yf = Nf+3 x 2
    Yf = (int**)malloc((ts_params->Nf+3)*sizeof(int*));
    if(Yf == NULL) {
        fprintf(stderr,"Unable to allocate memory for Yf.\n"); fflush(stderr);
        exit(-1);
    }
    for(m=0; m<ts_params->Nf+3; m++) {
        Yf[m] = (int*)malloc(2*sizeof(int));
        if(Yf[m] == NULL) {
            fprintf(stderr,"Unable to allocate memory for Yf.\n"); fflush(stderr);
            exit(-1);
        }
    }

    // P = Nf x 3
    P = (int**)malloc(ts_params->Nf*sizeof(int*));
    if(P == NULL) {
        fprintf(stderr,"Unable to allocate memory for P.\n"); fflush(stderr);
        exit(-1);
    }
    for(m=0; m<ts_params->Nf; m++) {
        P[m] = (int*)malloc(3*sizeof(int));
        if(P[m] == NULL) {
            fprintf(stderr,"Unable to allocate memory for P.\n"); fflush(stderr);
            exit(-1);
        }
    }

    // Wfx = Nf+3 x 1
    Wfx = (float*)malloc((ts_params->Nf+3)*sizeof(float));

    // Wfy = Nf+3 x 1
    Wfy = (float*)malloc((ts_params->Nf+3)*sizeof(float));

    // Allocate memory for output image
    output = (struct complexData**)malloc(image_params->Iy*sizeof(struct complexData*));
    if(output == NULL) {
        fprintf(stderr,"Error allocating memory for output.\n");
        exit(1);
    }
    for(n=0; n<image_params->Iy; n++) {
        output[n] = (struct complexData*)malloc(image_params->Ix*sizeof(struct complexData));
        if (output[n] == NULL) {
            fprintf(stderr,"Error allocating memory for output.\n");
            exit(1);
        }
    }

    // Yf(Nf+1:Nf+3,1:2) = 0
    for(m=ts_params->Nf; m<ts_params->Nf+3; m++) {
        memset(&Yf[m][0], 0, 2*sizeof(int));
    }

    N = (int)sqrtf((float)ts_params->Nf);
    min = (ts_params->Scf-1)/2 + ts_params->Rf;

    dy = (image_params->Iy - ts_params->Scf + 1 - 2*ts_params->Rf)/N;
    dx = (image_params->Ix - ts_params->Scf + 1 - 2*ts_params->Rf)/N;

    // L(Nf+1:Nf+3,Nf+1:Nf+3) = 0
    for(m=ts_params->Nf; m<ts_params->Nf+3; m++) {
        for(n=ts_params->Nf; n<ts_params->Nf+3; n++) {
            L[m][n] = 0.0;
        }
    }

    k = 0;
    ts_params->Nf = 0;

#ifdef _OPENMP
    #pragma omp parallel for private(ctrl_pt, n, disp_vec)
#endif
    for(m=0; m<N; m++)
    {
        ctrl_pt.y = m*dy + min;
        for(n=0; n<N; n++)
        {
            ctrl_pt.x = n*dx + min;

            // Perform 2D correlation
            disp_vec = corr2D(ctrl_pt, ts_params->Scf, ts_params->Rf, curImage, refImage, image_params);

            // Only retain control points that exceed the threshold
            if(disp_vec.p >= ts_params->Tf)
            {
#ifdef _OPENMP
                #pragma omp critical
#endif
                {
                    // Form Yf
                    Yf[k][0] = ctrl_pt.x + disp_vec.x;
                    Yf[k][1] = ctrl_pt.y + disp_vec.y;

                    // Form P
                    P[k][0] = 1;
                    P[k][1] = ctrl_pt.x;
                    P[k][2] = ctrl_pt.y;

                    k++;

                    // Number of control points after pruning
                    ts_params->Nf = k;
                } 
            }
        }
    }

    // Construct K portion of L
    for(i=0; i<ts_params->Nf; i++) {
        for(j=0; j<ts_params->Nf; j++) {
            r = sqrtf( ((float)P[i][1]-(float)P[j][1])*((float)P[i][1]-(float)P[j][1]) + ((float)P[i][2]-(float)P[j][2])*((float)P[i][2]-(float)P[j][2]) );
            if(r == 0.0) {
                L[i][j] = 0;
            }
            else {
                L[i][j] = r*r*logf(r*r);
            }
        }
    }

    // Construct P and P' portions of L
    for(m=0; m<ts_params->Nf; m++) {
        L[m][ts_params->Nf] = (float)P[m][0];
        L[m][ts_params->Nf+1] = (float)P[m][1];
        L[m][ts_params->Nf+2] = (float)P[m][2];

        L[ts_params->Nf][m] = (float)P[m][0];
        L[ts_params->Nf+1][m] = (float)P[m][1];
        L[ts_params->Nf+2][m] = (float)P[m][2];
    }

    // L(:,Nf+3+1) = Y(:,1) -- x vector
    for(m=0; m<ts_params->Nf+3; m++) {
        L[m][ts_params->Nf+3] = (float)Yf[m][0];
    }

    // Perform Gaussian elimination to find Wfx
    gauss_elim(L, Wfx, ts_params->Nf+3);

    // L(:,Nf+3+1) = Y(:,2) -- y vector
    for(m=0; m<ts_params->Nf+3; m++) {
        L[m][ts_params->Nf+3] = (float)Yf[m][1];
    }

    // Perform Gaussian elimination to find Wfy
    gauss_elim(L, Wfy, ts_params->Nf+3);

    // Loop over the output pixel locations and interpolate the Target image
    // pixel values at these points. This is done by mapping the (rectangular)
    // Source coordinates into the Target coordinates and performing the
    // interpolation there.
#ifdef _OPENMP
    #pragma omp parallel for private(n, i, sum_x, sum_y, r, warped_pt, aa, bb, w, v)
#endif
    for(m=0; m<image_params->Iy; m++)
    {
        for(n=0; n<image_params->Ix; n++)
        {
            for(i=sum_x=sum_y=0; i<ts_params->Nf; i++)
            {
                r = sqrtf( (P[i][1]-n)*(P[i][1]-n) + (P[i][2]-m)*(P[i][2]-m) );
                if(r != 0.0) {
                    sum_x += Wfx[i]*r*r*logf(r*r);
                    sum_y += Wfy[i]*r*r*logf(r*r);
                }
            }

            warped_pt[0] = Wfx[ts_params->Nf] + Wfx[ts_params->Nf+1]*n + Wfx[ts_params->Nf+2]*m + sum_x;
            warped_pt[1] = Wfy[ts_params->Nf] + Wfy[ts_params->Nf+1]*n + Wfy[ts_params->Nf+2]*m + sum_y;

            aa = (int)floorf(warped_pt[1]);
            bb = (int)floorf(warped_pt[0]);

            w = warped_pt[1] - (float)aa;
            v = warped_pt[0] - (float)bb;   

            if( (aa >= 0) && (aa < image_params->Iy-1) && (bb >= 0) && (bb < image_params->Ix-1) ) {
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
    for(m=0; m<image_params->Iy; m++) {
        memcpy(&curImage[m][0], &output[m][0], image_params->Ix*sizeof(struct complexData));
    }

    free(L);
    free(P);
    free(Yf);
    free(Wfx);
    free(Wfy);
    free(output);
}

struct point corr2D(struct point ctrl_pt, int Nwin, int R, struct complexData **curImage, struct complexData **refImage, struct ImageParams *image_params)
{
    int m, n, i, j, k;
    float den1, den2;
    float rho;
    struct point pt;
    struct complexData num;
    struct complexData *f, *g;
    struct complexData mu_f, mu_g;

    f = (struct complexData*)malloc(Nwin*Nwin*sizeof(struct complexData));
    g = (struct complexData*)malloc(Nwin*Nwin*sizeof(struct complexData));

    if( f == NULL || g == NULL ) {
        fprintf(stderr,"Unable to allocate memory for correlation windows.\n");
        exit(1);
    }

    for(i=ctrl_pt.y-(Nwin-1)/2, k=mu_f.real=mu_f.imag=0; i<=ctrl_pt.y+(Nwin-1)/2; i++)
    {
        for(j=ctrl_pt.x-(Nwin-1)/2; j<=ctrl_pt.x+(Nwin-1)/2; j++, k++)
        {
            if( (i < 0) || (j < 0) || (i >= image_params->Iy) || (j >= image_params->Ix) ) {
                fprintf(stderr,"Warning: Index out of bounds in registration correlation.\n");fflush(stderr);
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

    for(m=ctrl_pt.y-R, pt.p=0; m<=ctrl_pt.y+R; m++)
    {
        for(n=ctrl_pt.x-R; n<=ctrl_pt.x+R; n++)
        {
            for(i=m-(Nwin-1)/2, k=mu_g.real=mu_g.imag=0; i<=m+(Nwin-1)/2; i++)
            {
                for(j=n-(Nwin-1)/2; j<=n+(Nwin-1)/2; j++, k++)
                {
                    if( (i < 0) || (j < 0) || (i >= image_params->Iy) || (j >= image_params->Ix) ) {
                        fprintf(stderr,"Warning: Index out of bounds in registration correlation.\n");fflush(stderr);
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
                fprintf(stderr,"Correlation value out of range.\n");fflush(stderr);
                exit(-1);
            }*/

            if(rho > pt.p) {
                pt.x = ctrl_pt.x-n;
                pt.y = ctrl_pt.y-m;
                pt.p = rho;
            }
        }
    }
/*
    regmap[cnt].x = ctrl_pt.x;
    regmap[cnt].y = ctrl_pt.y;
    regmap[cnt].u = pt.x;
    regmap[cnt].v = pt.y;
    regmap[cnt].p = pt.p;
    cnt++;
*/
    free(f);
    free(g);

    return pt;
}

void gauss_elim(float *AA[], float *x, int N)
{
    int i, j, k, max;
    float **a, temp;

    a = (float**)malloc(N*sizeof(float*));
    if(a == NULL) {
        fprintf(stderr,"Unable to allocate memory for a.\n");fflush(stderr);
        exit(-1);
    }
    for(i=0; i<N; i++) {
        a[i] = (float*)malloc((N+1)*sizeof(float));
        if(a[i] == NULL) {
            fprintf(stderr,"Unable to allocate memory for a.\n");fflush(stderr);
            exit(-1);
        }
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
            fprintf(stderr,"Warning: Encountered a singular matrix in registration correlation.\n");fflush(stderr);
            a[i][i] = 1;
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
}
