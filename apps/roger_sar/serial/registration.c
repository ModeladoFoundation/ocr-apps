#include "common.h"

#include "rag_rmd.h"

#define BLOCK_SIZE 32

void gauss_elim(float *AA[], float *x, int N);

struct point corr2D(struct point ctrl_pt, int Nwin, int R,
    struct complexData**, struct complexData**, struct ImageParams*);

#ifdef RAG_WORKAROUND
#else
struct body_1 {
	int *Fx;
	int *Fy;
	int **A;
	struct point ctrl_pt;
};
#endif

rmd_guid_t Affine_async_body_1(
#ifdef RAG_WORKAROUND
    int *Fx, int *Fy, int **A, struct point ctrl_pt,
#else
    struct body_1 *body_1_args,
#endif
    rmd_guid_t affine_params_dbg,
    rmd_guid_t image_params_dbg,
    rmd_guid_t curImage_dbg,
    rmd_guid_t refImage_dbg);

#ifdef RAG_WORKAROUND
#else
struct body_2 {
	int x1;
	int x2;
	int y1;
	int y2;
	float Wcx[6];
	float Wcy[6];
};
#endif

rmd_guid_t Affine_async_body_2(
#ifdef RAG_WORKAROUND
    int x1, int x2,int y1, int y2, float Wcx[6], float Wcy[6],
#else
    struct body_2 *body_2_args,
#endif
    rmd_guid_t image_params_dbg,
    rmd_guid_t curImage_dbg,
    rmd_guid_t output_dbg);

rmd_guid_t Affine(
    rmd_guid_t affine_params_dbg,
    rmd_guid_t image_params_dbg,
    rmd_guid_t curImage_dbg,
    rmd_guid_t refImage_dbg)
{
    int N, min;
    int dx, dy;
    float b[6][2];
    float **aug_mat;
    float Wcx[6], Wcy[6];

#ifdef TRACE
xe_printf("// enter affine registration\n");RAG_FLUSH;
#endif
#ifdef TRACE
xe_printf("enter Affine registration affine_params_dbg is at %ld\n",(uint64_t)&affine_params_dbg);RAG_FLUSH;
xe_printf("enter Affine registration image_params_dbg  is at %ld\n",(uint64_t)&image_params_dbg);RAG_FLUSH;
xe_printf("enter Affine registration curImage_dbg      is at %ld\n",(uint64_t)&curImage_dbg);RAG_FLUSH;
xe_printf("enter Affine registration refImage_dbg      is at %ld\n",(uint64_t)&refImage_dbg);RAG_FLUSH;
#endif
#ifdef TRACE
xe_printf("affine_params\n");RAG_FLUSH;
xe_printf("affine_params_dbg is at %ld\n",(uint64_t)&affine_params_dbg);RAG_FLUSH;
xe_printf("and holds %ld)\n",affine_params_dbg.data);RAG_FLUSH;
xe_printf("end\n");RAG_FLUSH;
#endif
    struct AffineParams *affine_params, *affine_params_ptr, affine_params_lcl;
    RMD_DB_MEM(&affine_params_ptr,  affine_params_dbg);
    REM_LDX_ADDR(affine_params_lcl, affine_params_ptr, struct AffineParams);
    affine_params = &affine_params_lcl;
#ifdef TRACE
xe_printf("image_params\n");RAG_FLUSH;
xe_printf("image_params_dbg is at %ld\n",(uint64_t)&image_params_dbg);RAG_FLUSH;
xe_printf("and holds %ld)\n",image_params_dbg.data);RAG_FLUSH;
xe_printf("end\n");RAG_FLUSH;
#endif
    struct ImageParams *image_params, *image_params_ptr, image_params_lcl;
    RMD_DB_MEM(&image_params_ptr,  image_params_dbg);
    REM_LDX_ADDR(image_params_lcl, image_params_ptr, struct ImageParams);
    image_params = &image_params_lcl;
#ifdef TRACE
xe_printf("curImage\n");RAG_FLUSH;
xe_printf("curImage_dbg is at %ld\n",(uint64_t)&curImage_dbg);RAG_FLUSH;
xe_printf("and holds %ld)\n",curImage_dbg.data);RAG_FLUSH;
xe_printf("end\n");RAG_FLUSH;
#endif
    struct complexData **curImage;
    RMD_DB_MEM(&curImage,      curImage_dbg);
#ifdef TRACE
xe_printf("curImage is at %ld and holds %ld\n",(uint64_t)&curImage,(uint64_t)curImage);RAG_FLUSH;
#endif
#ifdef TRACE
xe_printf("refImage\n");RAG_FLUSH;
xe_printf("refImage_dbg is at %ld\n",(uint64_t)&refImage_dbg);RAG_FLUSH;
xe_printf("and holds %ld)\n",refImage_dbg.data);RAG_FLUSH;
xe_printf("end\n");RAG_FLUSH;
#endif
    struct complexData **refImage;
    RMD_DB_MEM(&refImage,      refImage_dbg);
#ifdef TRACE
xe_printf("refImage is at %ld and holds %ld\n",(uint64_t)&refImage,(uint64_t)refImage);RAG_FLUSH;
#endif
#ifdef TRACE
xe_printf("// Affine registration dynamically allocated variables\n");RAG_FLUSH;
#endif
    int **A;
    int *Fx, *Fy;
    struct complexData **output;

    // Fx = Nc x 1

    rmd_guid_t Fx_dbg;
    Fx = (int*)bsm_calloc(&Fx_dbg,affine_params->Nc,sizeof(int));
    if(Fx == NULL) {
        xe_printf("Unable to allocate memory for Fx.\n");RAG_FLUSH;
	exit(1);
    }
 
    // Fy = Nc x 1

    rmd_guid_t Fy_dbg;
    Fy = (int*)bsm_calloc(&Fy_dbg,affine_params->Nc,sizeof(int));
    if(Fy == NULL) {
        xe_printf("Unable to allocate memory for Fy.\n");RAG_FLUSH;
        exit(1);
    }

    // A = Nc x 6

    rmd_guid_t A_dbg;
    A = (int**)spad_malloc(&A_dbg,affine_params->Nc*sizeof(int*));
    if(A == NULL) {
        xe_printf("Error allocating memory for A.\n");RAG_FLUSH;
        exit(1);
    }
    int * A_data_ptr; rmd_guid_t A_data_dbg;
    A_data_ptr = (int*)spad_calloc(&A_data_dbg,affine_params->Nc*6,sizeof(int));
    if(A_data_ptr == NULL) {
        xe_printf("Error allocating memory for A.\n");RAG_FLUSH;
        exit(1);
    }
    for(int m=0; m<affine_params->Nc; m++) {
        A[m] = A_data_ptr + m*6;
    }

    // aug_mat = 6 x 7

    rmd_guid_t aug_mat_dbg;
    aug_mat = (float**)spad_malloc(&aug_mat_dbg,6*sizeof(float*));
    if(aug_mat == NULL) {
        xe_printf("Unable to allocate memory for aug_mat.\n");RAG_FLUSH;
        exit(1);
    }
    float *aug_mat_data_ptr; rmd_guid_t aug_mat_data_dbg;
    aug_mat_data_ptr = (float*)spad_calloc(&aug_mat_data_dbg,6*7,sizeof(float));
    if(aug_mat_data_ptr == NULL) {
        xe_printf("Unable to allocate memory for aug_mat.\n");RAG_FLUSH;
        exit(1);
    }
    for(int n=0; n<6; n++) {
        aug_mat[n] = aug_mat_data_ptr + n*7;
    }

    // Allocate memory for output image

    rmd_guid_t output_dbg;
    output = (struct complexData**)bsm_malloc(&output_dbg,image_params->Iy*sizeof(struct complexData*));
    if(output == NULL) {
        xe_printf("Error allocating memory for output.\n");RAG_FLUSH;
        exit(1);
    }
    struct complexData *output_data_ptr; rmd_guid_t output_data_dbg;
    output_data_ptr = (struct complexData*)bsm_malloc(&output_data_dbg,
	image_params->Iy*(image_params->Ix*sizeof(struct complexData)));
    if (output_data_ptr == NULL) {
        xe_printf("Error allocating memory for output.\n");RAG_FLUSH;
        exit(1);
    }
    for(int n=0; n<image_params->Iy; n++) {
        RAG_PUT_PTR(&output[n] , output_data_ptr + n*image_params->Ix);
    }

    //  Calculate misc. parameters

    N = (int)sqrtf((float)affine_params->Nc);
    min = (affine_params->Sc-1)/2 + affine_params->Rc;

    dy = (image_params->Iy - affine_params->Sc + 1 - 2*affine_params->Rc)/N;
    dx = (image_params->Ix - affine_params->Sc + 1 - 2*affine_params->Rc)/N;
                
    affine_params->Nc = 0;  
    REM_ST32_ADDR(((int *)(((char *)affine_params_ptr)+offsetof(struct AffineParams,Nc))),affine_params->Nc);

    for(int m=0; m<N; m++) {
        for(int n=0; n<N; n++) {
xe_printf("(m,n) = (%d,%d)\n",m,n);RAG_FLUSH;
            struct point ctrl_pt;
            ctrl_pt.y = m*dy + min;
            ctrl_pt.x = n*dx + min;
#ifdef RAG_WORKAROUND
            Affine_async_body_1(Fx, Fy, A, ctrl_pt,
		affine_params_dbg, image_params_dbg, curImage_dbg, refImage_dbg);
#else
	    struct body_1 body_1_args;
	    body_1_args.Fx = Fx;
	    body_1_args.Fy = Fy;
	    body_1_args.A  = A;
	    body_1_args.ctrl_pt  = ctrl_pt;
            Affine_async_body_1(&body_1_args,
		affine_params_dbg, image_params_dbg, curImage_dbg, refImage_dbg);
#endif
        } // for n
    } // for m

    REM_LD32_ADDR(affine_params->Nc,
	 ((int *)(((char *)affine_params_ptr)+offsetof(struct AffineParams,Nc))));

    // b = A'F
    // The computation of A'F and A'A requires better precision than would be obtained
    // by simply typecasting the A components to floats for the accumulation.  Here,
    // we use double precision accumulators, although 64-bit integer accumulations or
    // dynamic range rescaling provide other viable options to obtain additional precision.
    for(int m=0; m<6; m++) {
        double accum_x = 0.0, accum_y = 0.0;
        for(int n=0; n<affine_params->Nc; n++) {
            accum_x += (double)A[n][m]*(double)(RAG_GET_INT(Fx+n));
            accum_y += (double)A[n][m]*(double)(RAG_GET_INT(Fy+n));
        }
        b[m][0] = accum_x;
        b[m][1] = accum_y;
    }

    // aug_mat(1:6,1:6) = A'A
    for(int m=0; m<6; m++) {
        for(int n=0; n<6; n++) {
            double accum = 0.0;
            for(int k=0; k<affine_params->Nc; k++) {
                accum += (double) A[k][m] * (double) A[k][n];
            }
            aug_mat[m][n] = (float)accum;
        }
    }

    // aug_mat(1:6,7) = b(1:6,1)
    for(int m=0; m<6; m++) {
        aug_mat[m][6] = b[m][0];
    }

#ifdef TRACE
xe_printf("// Perform Gaussian elimination to find Wcx\n");RAG_FLUSH;
#endif
    gauss_elim(aug_mat, Wcx, 6);

    // aug_mat(1:6,7) = b(1:6,2)
    for(int m=0; m<6; m++) {
        aug_mat[m][6] = b[m][1];
    }

#ifdef TRACE
xe_printf("// Perform Gaussian elimination to find Wcy\n");RAG_FLUSH;
#endif
    gauss_elim(aug_mat, Wcy, 6);

    // Loop over the output pixel locations and interpolate the Target image
    // pixel values at these points. This is done by mapping the (rectangular)
    // Source coordinates into the Target coordinates and performing the
    // interpolation there.

#ifdef RAG_AFL
    assert( (image_params->Iy%BLOCK_SIZE) == 0);
    assert( (image_params->Ix%BLOCK_SIZE) == 0);
#endif
    for(int m=0; m<image_params->Iy; m+=BLOCK_SIZE) {
        for(int n=0; n<image_params->Ix; n+=BLOCK_SIZE) {
xe_printf("(M,N) = (%d,%d)\n",m,n);RAG_FLUSH;
#ifdef RAG_WORKAROUND
#else
	    struct body_2 body_2_args;
	    body_2_args.x1 = m;
	    body_2_args.x2 = m+BLOCK_SIZE;
	    body_2_args.y1 = n;
	    body_2_args.y2 = n+BLOCK_SIZE;
	    memcpy(body_2_args.Wcx,Wcx,6*sizeof(float)); 
	    memcpy(body_2_args.Wcy,Wcy,6*sizeof(float)); 
#endif
#ifdef RAG_WORKAROUND
            Affine_async_body_2(m,m+BLOCK_SIZE,n,n+BLOCK_SIZE,Wcx,Wcy,
#else
            Affine_async_body_2(&body_2_args,
#endif
		image_params_dbg,curImage_dbg,output_dbg);
        } // for n
    } // for m

#ifdef RAG_TRACE
xe_printf("// Overwrite current image with registered image\n");RAG_FLUSH;
#endif
    for(int m=0; m<image_params->Iy; m++) {
        struct complexData *cur_m;
        cur_m = (struct complexData *)RAG_GET_PTR(curImage+m);
        struct complexData *out_m;
        out_m = (struct complexData *)RAG_GET_PTR(output+m);
        BSMtoBSM(cur_m, out_m, image_params->Ix*sizeof(struct complexData));
    }

#ifdef RAG_TRACE
xe_printf("// Free data blocks\n");RAG_FLUSH;
#endif

    spad_free(output_data_ptr,output_data_dbg);
    spad_free(output,output_dbg);
    spad_free(aug_mat_data_ptr,aug_mat_data_dbg);
    spad_free(aug_mat,aug_mat_dbg);
    spad_free(A_data_ptr,A_data_dbg);
    spad_free(A,A_dbg);
    bsm_free(Fy,Fy_dbg);
    bsm_free(Fx,Fx_dbg);
#ifdef RAG_TRACE
xe_printf("// leave affine registration\n");RAG_FLUSH;
#endif
    return NULL_GUID;
}

#if RAG_THIN_ON
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

    // Thin spline dynamically allocated variables

    int **Yf, **P;
    float **L;
    float *Wfx, *Wfy;
    struct complexData **output;

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
#endif // RAG_THIN_ON

struct point corr2D(struct point ctrl_pt, int Nwin, int R, struct complexData **curImage, struct complexData **refImage, struct ImageParams *image_params)
{
    int m, n, i, j, k;
    float den1, den2;
    float rho;
    struct point pt;
    struct complexData num;
    struct complexData *f, *g;
    struct complexData mu_f, mu_g;

#ifdef TRACE
        xe_printf("corr2D ctrl_pt.x %d ctrl_pt.y %d\n",ctrl_pt.x,ctrl_pt.y);RAG_FLUSH;
#endif
    rmd_guid_t f_dbg;
    f = (struct complexData*)spad_malloc(&f_dbg,Nwin*Nwin*sizeof(struct complexData));
    rmd_guid_t g_dbg;
    g = (struct complexData*)spad_malloc(&g_dbg,Nwin*Nwin*sizeof(struct complexData));

    if( f == NULL || g == NULL ) {
        xe_printf("Unable to allocate memory for correlation windows.\n");RAG_FLUSH;
        exit(1);
    }

    for(i=ctrl_pt.y-(Nwin-1)/2, k=mu_f.real=mu_f.imag=0; i<=ctrl_pt.y+(Nwin-1)/2; i++)
    {
        struct complexData *cur_i;
        cur_i = (struct complexData *)RAG_GET_PTR(curImage+i);
        for(j=ctrl_pt.x-(Nwin-1)/2; j<=ctrl_pt.x+(Nwin-1)/2; j++, k++)
        {
            if( (i < 0) || (j < 0) || (i >= image_params->Iy) || (j >= image_params->Ix) ) {
                xe_printf("Warning: Index out of bounds in registration correlation.\n");RAG_FLUSH;
            }
            struct complexData *cur_i_j = cur_i + j;
            struct complexData cur;
            REM_LDX_ADDR(cur,cur_i_j,struct complexData);
            f[k].real  = cur.real;
            f[k].imag  = cur.imag;
            mu_f.real += cur.real;
            mu_f.imag += cur.imag;               
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
                struct complexData *ref_i;
                ref_i = (struct complexData *)RAG_GET_PTR(refImage+i);
                for(j=n-(Nwin-1)/2; j<=n+(Nwin-1)/2; j++, k++)
                {
                    if( (i < 0) || (j < 0) || (i >= image_params->Iy) || (j >= image_params->Ix) ) {
                        xe_printf("Warning: Index out of bounds in registration correlation.\n");RAG_FLUSH;
                    }
                    struct complexData *ref_i_j = ref_i + j;
                    struct complexData ref;
                    REM_LDX_ADDR(ref,ref_i_j,struct complexData);
                    g[k].real  = ref.real;
                    g[k].imag  = ref.imag;
                    mu_g.real += ref.real;
                    mu_g.imag += ref.imag;                   
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

    spad_free(g,g_dbg);
    spad_free(f,f_dbg);

    return pt;
}

void gauss_elim(float *AA[], float *x, int N)
{
    int i, j, k, max;
    float **a, temp;

    a = (float**)malloc(N*sizeof(float*));
    if(a == NULL) {
        xe_printf("Unable to allocate memory for a.\n");RAG_FLUSH;
        exit(-1);
    }
    for(i=0; i<N; i++) {
        a[i] = (float*)malloc((N+1)*sizeof(float));
        if(a[i] == NULL) {
            xe_printf("Unable to allocate memory for a.\n");RAG_FLUSH;
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
            xe_printf("Warning: Encountered a singular matrix in registration correlation.\n");RAG_FLUSH;
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

#ifdef RAG_WORKAROUND
rmd_guid_t Affine_async_body_1(
    int *Fx, int *Fy, int **A, struct point ctrl_pt,
    rmd_guid_t  affine_params_dbg,
    rmd_guid_t  image_params_dbg,
    rmd_guid_t  curImage_dbg,
    rmd_guid_t  refImage_dbg)
{
#else
rmd_guid_t Affine_async_body_1(
    struct body_1 *body_1_args,
    rmd_guid_t  affine_params_dbg,
    rmd_guid_t  image_params_dbg,
    rmd_guid_t  curImage_dbg,
    rmd_guid_t  refImage_dbg)
{
    int *Fx = body_1_args->Fx;
    int *Fy = body_1_args->Fy;
    int **A = body_1_args->A;
    struct point ctrl_pt = body_1_args->ctrl_pt;
#endif
#ifdef TRACE
xe_printf("enter Affine_async_body_1 affine_params_dbg is at %ld\n",(uint64_t)&affine_params_dbg);RAG_FLUSH;
xe_printf("enter Affine_async_body_1 image_params_dbg  is at %ld\n",(uint64_t)&image_params_dbg);RAG_FLUSH;
xe_printf("enter Affine_async_body_1 curImage_dbg      is at %ld\n",(uint64_t)&curImage_dbg);RAG_FLUSH;
xe_printf("enter Affine_async_body_1 refImage_dbg      is at %ld\n",(uint64_t)&refImage_dbg);RAG_FLUSH;
#endif
#ifdef TRACE
xe_printf("affine_params\n");RAG_FLUSH;
xe_printf("affine_params_dbg is at %ld\n",(uint64_t)&affine_params_dbg);RAG_FLUSH;
xe_printf("and holds %ld)\n",affine_params_dbg.data);RAG_FLUSH;
xe_printf("end\n");RAG_FLUSH;
#endif
    struct AffineParams *affine_params, *affine_params_ptr, affine_params_lcl;
    RMD_DB_MEM(&affine_params_ptr,  affine_params_dbg);
    REM_LDX_ADDR(affine_params_lcl, affine_params_ptr, struct AffineParams);
    affine_params = &affine_params_lcl;
#ifdef TRACE
xe_printf("image_params\n");RAG_FLUSH;
xe_printf("image_params_dbg is at %ld\n",(uint64_t)&image_params_dbg);RAG_FLUSH;
xe_printf("and holds %ld)\n",image_params_dbg.data);RAG_FLUSH;
xe_printf("end\n");RAG_FLUSH;
#endif
    struct ImageParams *image_params, *image_params_ptr, image_params_lcl;
    RMD_DB_MEM(&image_params_ptr,  image_params_dbg);
    REM_LDX_ADDR(image_params_lcl, image_params_ptr, struct ImageParams);
    image_params = &image_params_lcl;
#ifdef TRACE
xe_printf("curImage\n");RAG_FLUSH;
xe_printf("curImage_dbg is at %ld\n",(uint64_t)&curImage_dbg);RAG_FLUSH;
xe_printf("and holds %ld)\n",curImage_dbg.data);RAG_FLUSH;
xe_printf("end\n");RAG_FLUSH;
#endif
    struct complexData **curImage;
    RMD_DB_MEM(&curImage,      curImage_dbg);
#ifdef TRACE
xe_printf("curImage is at %ld and holds %ld\n",(uint64_t)&curImage,(uint64_t)curImage);RAG_FLUSH;
#endif
#ifdef TRACE
//xe_printf("refImage (dbg=%ld)\n",refImage_dbg.data);RAG_FLUSH;
xe_printf("refImage\n");RAG_FLUSH;
xe_printf("refImage_dbg is at %ld\n",(uint64_t)&refImage_dbg);RAG_FLUSH;
xe_printf("and holds %ld)\n",refImage_dbg.data);RAG_FLUSH;
xe_printf("end\n");RAG_FLUSH;
#endif
    struct complexData **refImage;
    RMD_DB_MEM(&refImage,      refImage_dbg);
#ifdef TRACE
xe_printf("refImage is at %ld and holds %ld\n",(uint64_t)&refImage,(uint64_t)refImage);RAG_FLUSH;
#endif
#ifdef TRACE
xe_printf("disp_vec\n");RAG_FLUSH;
#endif
    struct point disp_vec;
            
#ifdef TRACE
xe_printf("// Perform 2D correlation\n");RAG_FLUSH;
#endif
    disp_vec = corr2D(ctrl_pt, affine_params->Sc, affine_params->Rc,
			 curImage, refImage, image_params);
#ifdef TRACE
xe_printf("// Only retain control points that exceed the threshold\n");RAG_FLUSH;
#endif
    if(disp_vec.p >= affine_params->Tc) {
xe_printf("disp_vec.p\n");RAG_FLUSH;

        // Allocate slot and update Number of control points found
xe_printf("fetchadd %d\n",affine_params->Nc);RAG_FLUSH;
#ifdef RAG_SIM
        int k = 1;
	REM_XADD32_ADDR( ((int *)(((char *)affine_params_ptr)
		                + offsetof(struct AffineParams,Nc))),k);
#else
        const int k = __sync_fetch_and_add(
		 ((int *)(((char *)affine_params_ptr)
		                + offsetof(struct AffineParams,Nc))),1);
#endif
#ifdef TRACE
xe_printf("// Form Fx, Fy (k=%d)\n",k);RAG_FLUSH;
#endif

xe_printf("Fx\n");RAG_FLUSH;
        RAG_PUT_INT(Fx+k, ctrl_pt.x + disp_vec.x);
xe_printf("Fy\n");RAG_FLUSH;
        RAG_PUT_INT(Fy+k, ctrl_pt.y + disp_vec.y);

xe_printf("A\n");RAG_FLUSH;
        // Form A
        A[k][0] = 1;
        A[k][1] = ctrl_pt.x;
        A[k][2] = ctrl_pt.y;
        A[k][3] = ctrl_pt.x*ctrl_pt.x;
        A[k][4] = ctrl_pt.y*ctrl_pt.y;
        A[k][5] = ctrl_pt.x*ctrl_pt.y;
xe_printf("a\n");RAG_FLUSH;

    } // if above threshold
xe_printf("ret\n");RAG_FLUSH;
    return NULL_GUID;
}

rmd_guid_t Affine_async_body_2(
#ifdef RAG_WORKAROUND
    int x1, int x2, int y1, int y2, float Wcx[6], float Wcy[6],
#else
    struct body_2 *body_2_args,
#endif
    rmd_guid_t image_params_dbg,
    rmd_guid_t curImage_dbg,
    rmd_guid_t output_dbg)
{
#ifdef RAG_WORKAROUND
#else
        int x1; int x2; int y1; int y2; float Wcx[6]; float Wcy[6];
	x1 = body_2_args->x1;
	x2 = body_2_args->x2;
	y1 = body_2_args->y1;
	y2 = body_2_args->y2;
	memcpy(Wcx,body_2_args->Wcx,6*sizeof(float)); 
	memcpy(Wcy,body_2_args->Wcy,6*sizeof(float)); 
#endif
    int aa, bb;
    float Px, Py, w, v;

    struct ImageParams *image_params, *image_params_ptr, image_params_lcl;
    RMD_DB_MEM(&image_params_ptr,  image_params_dbg);
    REM_LDX_ADDR(image_params_lcl, image_params_ptr, struct ImageParams);
    image_params = &image_params_lcl;

    struct complexData **curImage;
    RMD_DB_MEM(&curImage,      curImage_dbg);

    struct complexData **output;
    RMD_DB_MEM(&output,        output_dbg);
            
#ifdef RAG_AFL
    assert( ((x2-x1)%BLOCK_SIZE) == 0);
    assert( ((y2-y1)%BLOCK_SIZE) == 0);
#endif
    for(int m=x1; m<x2; m++) {
        struct complexData *out_m;
        out_m = (struct complexData *)RAG_GET_PTR(output+m);
        for(int n=y1; n<y2; n++) {
            const float m_flt = (float)m;
            const float n_flt = (float)n;
            Px = Wcx[0] + Wcx[1]*n_flt + Wcx[2]*m_flt
               + Wcx[3]*n_flt*n_flt + Wcx[4]*m_flt*m_flt + Wcx[5]*n_flt*m_flt;
            Py = Wcy[0] + Wcy[1]*(float)n + Wcy[2]*(float)m
               + Wcy[3]*n_flt*n_flt + Wcy[4]*m_flt*m_flt + Wcy[5]*n_flt*m_flt;

            aa = (int)floorf(Py);
            bb = (int)floorf(Px);
            w = Py - (float)aa;
            v = Px - (float)bb; 

            if( (aa >= 0) && (aa < image_params->Iy-1) && (bb >= 0) && (bb < image_params->Ix-1) ) {
                struct complexData *cur_aa0;
                struct complexData *cur_aa1;
                struct complexData cur_aa0_bb0;
                struct complexData cur_aa0_bb1;
                struct complexData cur_aa1_bb0;
                struct complexData cur_aa1_bb1;
                struct complexData out_m_n;
                cur_aa0 = (struct complexData *)RAG_GET_PTR(curImage+aa  );
                cur_aa1 = (struct complexData *)RAG_GET_PTR(curImage+aa+1);
                REM_LDX_ADDR(cur_aa0_bb0,cur_aa0+bb  ,struct complexData);
                REM_LDX_ADDR(cur_aa0_bb1,cur_aa0+bb+1,struct complexData);
                REM_LDX_ADDR(cur_aa1_bb0,cur_aa1+bb  ,struct complexData);
                REM_LDX_ADDR(cur_aa1_bb1,cur_aa1+bb+1,struct complexData);
                out_m_n.real 	= (1-v)*(1-w)*cur_aa0_bb0.real
				+ (  v)*(1-w)*cur_aa0_bb1.real
				+ (1-v)*(  w)*cur_aa1_bb0.real
				+ (  v)*(  w)*cur_aa1_bb1.real;
                out_m_n.imag	= (1-v)*(1-w)*cur_aa0_bb0.imag
				+ (  v)*(1-w)*cur_aa0_bb1.imag
				+ (1-v)*(  w)*cur_aa1_bb0.imag
				+ (  v)*(  w)*cur_aa1_bb1.imag;
		REM_STX_ADDR(out_m+n,out_m_n,struct complexData);
            } else {
                struct complexData out_m_n = {0.0f,0.0f};
		REM_STX_ADDR(out_m+n,out_m_n,struct complexData);
            }
        } // for n
    } // for m
    return NULL_GUID;
}
