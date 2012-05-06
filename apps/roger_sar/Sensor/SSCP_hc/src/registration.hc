#include "common.h"

void gauss_elim(float *AA[], float *x, int N);

struct point corr2D(struct point ctrl_pt, int Nwin, int R,
	struct complexData**, struct complexData**, struct ImageParams*);

//struct reg_map *regmap;

void Affine_async_1_body(
		struct point *ctrl_pt_ptr,
		int **A, int *Fx, int *Fy,
		struct ImageParams *image_params,
		struct AffineParams *affine_params,
		struct complexData **curImage,
		struct complexData **refImage)
{
#ifdef DEBUG_RAG
fprintf(stderr,"// Perform 2D correlation\n");fflush(stderr);
#endif
	struct point ctrl_pt = *ctrl_pt_ptr;
#ifdef DEBUG_RAG
fprintf(stderr,"ctrl_pt %d %d %f\n",ctrl_pt.x,ctrl_pt.y,ctrl_pt.p);fflush(stderr);
#endif
	struct point disp_vec = corr2D(ctrl_pt,
				affine_params->Sc, affine_params->Rc,
				curImage, refImage, image_params);

	// Only retain control points that exceed the threshold
	if(disp_vec.p >= affine_params->Tc) {
#ifdef DEBUG_RAG
fprintf(stderr,"// Number of control points after pruning\n");fflush(stderr);
#endif
		int k = RAG_fetch_and_add(&affine_params->Nc,1);

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

/*		regmap[k].x = ctrl_pt.x;
 *		regmap[k].y = ctrl_pt.y;
 *		regmap[k].u = disp_vec.x;
 *		regmap[k].v = disp_vec.y;
 *		regmap[k].p = disp_vec.p;
 */
	} // if threshold

	free(ctrl_pt_ptr);
	return;
}

void Affine_async_2_body(
	struct corners_t *corners,
	float *Wcx, float *Wcy,
	struct ImageParams *image_params,
	struct complexData ** output, 
	struct complexData **curImage)
{
	int aa, bb;
	float v, w;
	float Px, Py;
	int m1   = corners->m1;
	int m2   = corners->m2;
	int n1   = corners->n1;
	int n2   = corners->n2;
	for(int m=m1; m<m2; m++) {
		for(int n=n1; n<n2; n++) {
			Px = Wcx[0] + Wcx[1]*(float)n + Wcx[2]*(float)m
			   + Wcx[3]*(float)n*(float)n + Wcx[4]*(float)m*(float)m + Wcx[5]*(float)n*(float)m;
			Py = Wcy[0] + Wcy[1]*(float)n + Wcy[2]*(float)m 
			   + Wcy[3]*(float)n*(float)n + Wcy[4]*(float)m*(float)m + Wcy[5]*(float)n*(float)m;

			aa = (int)floorf(Py);
			bb = (int)floorf(Px);
			w = Py - (float)aa;
			v = Px - (float)bb; 

			if( (aa >= 0) && (aa < image_params->Iy-1)
			&&  (bb >= 0) && (bb < image_params->Ix-1) ) {
				output[m][n].real = (1-v)*(1-w)*curImage[aa][bb].real + v*(1-w)*curImage[aa][bb+1].real + (1-v)*w*curImage[aa+1][bb].real + v*w*curImage[aa+1][bb+1].real;
				output[m][n].imag = (1-v)*(1-w)*curImage[aa][bb].imag + v*(1-w)*curImage[aa][bb+1].imag + (1-v)*w*curImage[aa+1][bb].imag + v*w*curImage[aa+1][bb+1].imag;
			} else {
				output[m][n].real = 0.0;
				output[m][n].imag = 0.0;
			} // if aa and bb
		} // for n
	} // for m
	free(corners);
	return;
}

void Affine(
	struct AffineParams *affine_params,
	struct ImageParams  *image_params,
	struct complexData **curImage,
	struct complexData **refImage)
{
	int N, min;
	int dx, dy;
#if 1 // H-C bug with two dimensional arrays on stack
#else
	float b[6][2];
	float Wcx[6], Wcy[6];
#endif

//	regmap = (struct reg_map*)malloc(Nf*sizeof(struct reg_map));

	// Fx = Nc x 1

	int *Fx = (int*)calloc(affine_params->Nc,sizeof(int));
	if(Fx == NULL) {
		fprintf(stderr,"Unable to allocate memory for Fx.\n");fflush(stderr);
		exit(1);
	}

	// Fy = Nc x 1

	int *Fy = (int*)calloc(affine_params->Nc,sizeof(int));
	if(Fy == NULL) {
		fprintf(stderr,"Unable to allocate memory for Fy.\n");fflush(stderr);
		exit(1);
	}

	// A = Nc x 6

	int **A = (int**)calloc(affine_params->Nc,sizeof(int*));
	if(A == NULL) {
		fprintf(stderr,"Error allocating memory for A.\n");
		exit(1);
	}
	int *A_data_ptr	= (int*)calloc(affine_params->Nc*6,sizeof(int));
	if(A_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for A.\n");
		exit(1);
	}
	for(int m=0; m<affine_params->Nc; m++) {
		A[m] = A_data_ptr + m*6;
	}

	// aug_mat = 6 x 7

	float **aug_mat = (float**)calloc(6,sizeof(float*));
	if(aug_mat == NULL) {
		fprintf(stderr,"Unable to allocate memory for aug_mat.\n");
		exit(1);
	}
	float *aug_mat_data_ptr = (float*)calloc(6*7,sizeof(float));
	if(aug_mat_data_ptr == NULL) {
		fprintf(stderr,"Unable to allocate memory for aug_mat.\n");
		exit(1);
	}
	for(int n=0; n<6; n++) {
		aug_mat[n] = aug_mat_data_ptr + n*7;
	}

#if 1 // H-C bug with two dimensional arrays on stack
	// b = 6 x 2

	float **b = (float**)calloc(6,sizeof(float*));
	if(aug_mat == NULL) {
		fprintf(stderr,"Unable to allocate memory for b.\n");
		exit(1);
	}
	float *b_data_ptr = (float*)calloc(6*2,sizeof(float));
	if(b_data_ptr == NULL) {
		fprintf(stderr,"Unable to allocate memory for b.\n");
		exit(1);
	}
	for(int n=0; n<6; n++) {
		b[n] = b_data_ptr + n*2;
	}

	// Wcx = 6 x 1

	float *Wcx = (float*)calloc(6,sizeof(float));
	if(Wcx == NULL) {
		fprintf(stderr,"Unable to allocate memory for Wcx.\n");
		exit(1);
	}

	// Wcy = 6 x 1

	float *Wcy = (float*)calloc(6,sizeof(float));
	if(Wcx == NULL) {
		fprintf(stderr,"Unable to allocate memory for Wcy.\n");
		exit(1);
	}
#endif

#ifdef DEBUG_RAG
fprintf(stderr,"// Allocate memory for output image\n");fflush(stderr);
#endif
	struct complexData **output = (struct complexData**)malloc(image_params->Iy*sizeof(struct complexData*));
	if(output == NULL) {
		fprintf(stderr,"Error allocating memory for output.\n");
		exit(1);
	}
	struct complexData *output_data_ptr = (struct complexData*)malloc(image_params->Iy*image_params->Ix*sizeof(struct complexData));
	if (output_data_ptr == NULL) {
		fprintf(stderr,"Error allocating memory for output data.\n");
		exit(1);
	}
	for(int n=0; n<image_params->Iy; n++) {
		output[n] = output_data_ptr + n*image_params->Ix;
	}

	N = (int)sqrtf((float)affine_params->Nc);
	min = (affine_params->Sc-1)/2 + affine_params->Rc;

	dy = (image_params->Iy - affine_params->Sc + 1 - 2*affine_params->Rc)/N;
	dx = (image_params->Ix - affine_params->Sc + 1 - 2*affine_params->Rc)/N;
                
	affine_params->Nc = 0;  // RAG will get updated atomically 
#ifdef DEBUG_RAG
fprintf(stderr,"// form A, Fx and Fy\n");fflush(stderr);
#endif
finish {
	for(int m=0; m<N; m++) {
		for(int n=0; n<N; n++) {
			struct point *ctrl_pt_ptr;
			ctrl_pt_ptr = malloc(sizeof(struct point));
			ctrl_pt_ptr->y = m*dy + min;
			ctrl_pt_ptr->x = n*dx + min;
			ctrl_pt_ptr->p = 0.0f;
#if 1
			async            IN(ctrl_pt_ptr, A, Fx, Fy, image_params, affine_params, curImage, refImage)
#endif
			Affine_async_1_body(ctrl_pt_ptr, A, Fx, Fy, image_params, affine_params, curImage, refImage);
		} // for n
	} // for m
} // finish
#ifdef DEBUG_RAG
fprintf(stderr,"// b = A'F\n");fflush(stderr);
#endif
	// The computation of A'F and A'A requires better precision than would be obtained
	// by simply typecasting the A components to floats for the accumulation.  Here,
	// we use double precision accumulators, although 64-bit integer accumulations or
	// dynamic range rescaling provide other viable options to obtain additional precision.

#ifdef DEBUG
fprintf(stderr,"// b = accurate_accum(A,Fx,Fy) Nc = %d\n",affine_params->Nc);fflush(stderr);
#endif
	for(int m=0; m<6; m++) {
		double accum_x = 0.0, accum_y = 0.0;
		for(int n=0; n<affine_params->Nc; n++) {
			accum_x += (double)A[n][m]*(double)Fx[n];
			accum_y += (double)A[n][m]*(double)Fy[n];
		} // for n
		b[m][0] = accum_x;
		b[m][1] = accum_y;
	} // for m
#ifdef DEBUG_RAG
fprintf(stderr,"// aug_mat[0:5][0:5] = A'A\n");fflush(stderr);
#endif
	for(int m=0; m<6; m++) {
		for(int n=0; n<6; n++) {
			double accum = 0.0;
			for(int k=0; k<affine_params->Nc; k++) {
				accum += (double) A[k][m] * (double) A[k][n];
			} // for k
			aug_mat[m][n] = (float)accum;
		} // for n
	} // for m
#ifdef DEBUG_RAG
fprintf(stderr,"// aug_mat[0:5][6] = b[0:5][0]\n");fflush(stderr);
#endif
	for(int m=0; m<6; m++) {
		aug_mat[m][6] = b[m][0];
	} // for m
#ifdef DEBUG_RAG
fprintf(stderr,"// Perform Gaussian elimination to find Wcx\n");fflush(stderr);
#endif
	gauss_elim(aug_mat, Wcx, 6);
#ifdef DEBUG
fprintf(stderr,"Wcx %f %f %f %f %f %f\n" ,Wcx[0] ,Wcx[1] ,Wcx[2] ,Wcx[3] ,Wcx[4] ,Wcx[5]);fflush(stderr);
#endif
#ifdef DEBUG_RAG
fprintf(stderr,"// aug_mat[0:5][6] = b[0:5][1]\n");fflush(stderr);
#endif
	for(int m=0; m<6; m++) {
		aug_mat[m][6] = b[m][1];
	} // for m
#ifdef DEBUG_RAG
fprintf(stderr,"// Perform Gaussian elimination to find Wcy\n");fflush(stderr);
#endif
	gauss_elim(aug_mat, Wcy, 6);
#ifdef DEBUG
fprintf(stderr,"Wcy %f %f %f %f %f %f\n" ,Wcy[0] ,Wcy[1] ,Wcy[2] ,Wcy[3] ,Wcy[4] ,Wcy[5]);fflush(stderr);
#endif
	// Loop over the output pixel locations and interpolate the Target image
	// pixel values at these points. This is done by mapping the (rectangular)
	// Source coordinates into the Target coordinates and performing the
	// interpolation there.
	int AFFINE_ASYNC_2_BLOCK_SIZE_M = blk_size(image_params->Iy,32);
	int AFFINE_ASYNC_2_BLOCK_SIZE_N = blk_size(image_params->Ix,32);
	assert(((image_params->Iy)%AFFINE_ASYNC_2_BLOCK_SIZE_M)==0);
	assert(((image_params->Ix)%AFFINE_ASYNC_2_BLOCK_SIZE_N)==0);
finish {
	for(int m=0; m<image_params->Iy; m+=AFFINE_ASYNC_2_BLOCK_SIZE_M) {
		for(int n=0; n<image_params->Ix; n+=AFFINE_ASYNC_2_BLOCK_SIZE_N) {
			struct corners_t *corners;
			corners = malloc(sizeof(struct corners_t));
			corners->m1 = m;
			corners->m2 = m+AFFINE_ASYNC_2_BLOCK_SIZE_M;
			corners->n1 = n;
			corners->n2 = n+AFFINE_ASYNC_2_BLOCK_SIZE_N;
#if 1
                        async            IN(corners, Wcx, Wcy, image_params, output, curImage)
#endif
			Affine_async_2_body(corners, Wcx, Wcy, image_params, output, curImage);
		} // for n
	} // for m
} // finish

	// Overwrite current image with registered image

	for(int m=0; m<image_params->Iy; m++) {
		memcpy(&curImage[m][0], &output[m][0], image_params->Ix*sizeof(struct complexData));
	} // for m

	free(output_data_ptr);
	free(output);
#if 1 // H-C bug with two dimensional arrays on stack
	free(Wcy);
	free(Wcx);
	free(b_data_ptr);
	free(b);
#endif
	free(aug_mat_data_ptr);
	free(aug_mat);
	free(A_data_ptr);
	free(A);
	free(Fy);
	free(Fx);
}

#ifdef RAG_THIN_ON
// Thin spline dynamically allocated variables
int **Yf, **P;
float **L;
float *Wfx, *Wfy;

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
#endif

struct point corr2D(struct point ctrl_pt, int Nwin, int R, struct complexData **curImage, struct complexData **refImage, struct ImageParams *image_params)
{
	float den1, den2;
	float rho;
	struct point pt;
	struct complexData num;
	struct complexData *f, *g;
	struct complexData mu_f, mu_g;
	pt.x = 0;
	pt.y = 0;
	pt.p = -1.0f;
	f = (struct complexData*)malloc(Nwin*Nwin*sizeof(struct complexData));
	g = (struct complexData*)malloc(Nwin*Nwin*sizeof(struct complexData));

	if( f == NULL || g == NULL ) {
		fprintf(stderr,"Unable to allocate memory for correlation windows.\n");
		exit(1);
	}

	for(int i=ctrl_pt.y-(Nwin-1)/2, k=mu_f.real=mu_f.imag=0; i<=ctrl_pt.y+(Nwin-1)/2; i++) {
		for(int j=ctrl_pt.x-(Nwin-1)/2; j<=ctrl_pt.x+(Nwin-1)/2; j++, k++) {
			if( (i < 0) || (i >= image_params->Iy) 
			 || (j < 0) || (j >= image_params->Ix) ) {
				fprintf(stderr,"Warning: Index out of bounds in registration correlation.\n");fflush(stderr);
			}
			f[k].real = curImage[i][j].real;
			f[k].imag = curImage[i][j].imag;
			mu_f.real += curImage[i][j].real;
			mu_f.imag += curImage[i][j].imag;               
		} // for j
	} // for i

	mu_f.real /= Nwin*Nwin;
	mu_f.imag /= Nwin*Nwin;

	for(int k=0; k<Nwin*Nwin; k++) {
		f[k].real -= mu_f.real;
		f[k].imag -= mu_f.imag;
	} // for k

	for(int m=ctrl_pt.y-R; m<=ctrl_pt.y+R; m++) {
		for(int n=ctrl_pt.x-R; n<=ctrl_pt.x+R; n++) {
			int k=0;
			mu_g.real=mu_g.imag=0;
			for(int i=m-(Nwin-1)/2; i<=m+(Nwin-1)/2; i++) {
				for(int j=n-(Nwin-1)/2; j<=n+(Nwin-1)/2; j++, k++) {
					if( (i < 0) || (i >= image_params->Iy)
					 || (j < 0) || (j >= image_params->Ix) ) {
						fprintf(stderr,"Warning: Index out of bounds in registration correlation.\n");fflush(stderr);
					}
					g[k].real =  refImage[i][j].real;
					g[k].imag =  refImage[i][j].imag;
					mu_g.real += refImage[i][j].real;
					mu_g.imag += refImage[i][j].imag;                   
				} // for j
            		} // for i
            
			mu_g.real /= Nwin*Nwin;
			mu_g.imag /= Nwin*Nwin;

			num.real=num.imag=0.0f;
			den1=den2=0.0f;
			for(int k=0; k<Nwin*Nwin; k++) {       
				g[k].real -= mu_g.real;
				g[k].imag -= mu_g.imag;
                
				num.real += f[k].real*g[k].real + f[k].imag*g[k].imag;
				num.imag += f[k].real*g[k].imag - f[k].imag*g[k].real;

				den1 += f[k].real*f[k].real + f[k].imag*f[k].imag;
				den2 += g[k].real*g[k].real + g[k].imag*g[k].imag;
			} // for k

			if(den1 != 0.0f && den2 != 0.0f) {
				rho = sqrtf( (num.real*num.real + num.imag*num.imag) / (den1*den2) );
			} else {
				rho = 0.0f;
			}
            
/*			if(rho < 0 || rho > 1) {
 *				fprintf(stderr,"Correlation value out of range.\n");fflush(stderr);
 *				exit(-1);
 *			}
 */

			if(rho > pt.p) {
				pt.x = ctrl_pt.x-n;
				pt.y = ctrl_pt.y-m;
				pt.p = rho;
			}
		} // for n
	} // for m
/*
 *	regmap[cnt].x = ctrl_pt.x;
 *	regmap[cnt].y = ctrl_pt.y;
 *	regmap[cnt].u = pt.x;
 *	regmap[cnt].v = pt.y;
 *	regmap[cnt].p = pt.p;
 *	cnt++;
 */
	free(g);
	free(f);

	return pt;
}

void gauss_elim(float *AA[], float *x, int N)
{
	float **a = (float**)malloc(N*sizeof(float*));
	if(a == NULL) {
		fprintf(stderr,"Unable to allocate memory for a.\n");fflush(stderr);
		exit(-1);
	}
	float *a_data_ptr = (float*)malloc(N*(N+1)*sizeof(float));
	if(a_data_ptr == NULL) {
		fprintf(stderr,"Unable to allocate memory for a.\n");fflush(stderr);
		exit(-1);
	}
	for(int i=0; i<N; i++) {
		a[i] = a_data_ptr + i*(N+1);
	} // for i

	for(int i=0; i<N; i++) {
		for(int j=0; j<N+1; j++) {
			a[i][j] = AA[i][j];
		} // for j
	} // for i
    
	for(int i=0; i<N; i++) {
		// Find the largest value row
		int max = i;
		for(int j=i+1; j<N; j++) {
			if(fabsf(a[j][i]) > fabsf(a[max][i])) {
				max = j;
			}
		} // for j

		// Swap the largest row with the ith row
		for(int k=i; k<N+1; k++) {
			float temp = a[i][k];
			a[i][k]    = a[max][k];
			a[max][k]  = temp;
		}

		// Check to see if this is a singular matrix
		if(fabsf(a[i][i]) == 0.0) {
			fprintf(stderr,"Warning: Encountered a singular matrix in registration correlation.\n");fflush(stderr);
			a[i][i] = 1;
		}
    
		// Starting from row i+1, eliminate the elements of the ith column
		for(int j=i+1; j<N; j++) {
			if(a[j][i] != 0) {
				for(int k=N; k>=i; k--) {
					a[j][k] -= a[i][k] * a[j][i] / a[i][i];
				} // for k
			}
		} // for j
	} // for i

	// Perform the back substitution
	for(int j=N-1; j>=0; j--) {
		float temp = 0;
		for(int k=j+1; k<N; k++) {
			temp += a[j][k] * x[k];
		} // for k
		x[j] = (a[j][N] - temp) / a[j][j];
	} // for j

	free(a_data_ptr);
	free(a);

	return;
}
