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

#ifndef RAG_SIM
#define dram_free(addr,guid)
#define  bsm_free(addr,guid)
#define spad_free(addr,guid)
#endif

struct async_1_args_t {
	struct point ctrl_pt;
	ocrGuid_t output_dbg;
	ocrGuid_t curImage_dbg;
	ocrGuid_t refImage_dbg;
	ocrGuid_t post_Affine_scg;
};

struct async_2_args_t {
	float Wcx[6];
	float Wcy[6];
};

ocrGuid_t post_Affine_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_2
xe_printf("//// enter post_Affine_edt\n");RAG_FLUSH;
#endif
	assert(paramc==0);
	assert(depc==5);  // 5th is post_affine_async_2_evg
RAG_REF_MACRO_BSM( struct complexData **,output,NULL,NULL,output_dbg,0);
RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,1);
RAG_REF_MACRO_BSM( struct complexData **,refImage,NULL,NULL,refImage_dbg,2);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,3);

#ifdef TRACE_LVL_2
xe_printf("// Overwrite current image with registered image\n");RAG_FLUSH;
#endif
	for(int m=0; m<image_params->Iy; m++) {
		BSMtoBSM(curImage[m], output[m], image_params->Ix*sizeof(struct complexData));
	}

#ifdef RAG_DRAM
	dram_free(output,output_dbg);
#else
	 bsm_free(output,output_dbg);
#endif

#ifdef TRACE_LVL_2
xe_printf("//// leave post_Affine_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

ocrGuid_t Affine_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_2
xe_printf("//// enter Affine_edt\n");RAG_FLUSH;
#endif
	assert(paramc==2);
	ocrGuid_t post_Affine_scg		= (ocrGuid_t)paramv[0]; // post_Afine_scg
	ocrGuid_t post_affine_async_1_scg	= (ocrGuid_t)paramv[1]; // post_affine_async_1_scg
	assert(depc==4);
RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,0);
RAG_REF_MACRO_BSM( struct complexData **,refImage,NULL,NULL,refImage_dbg,1);
RAG_REF_MACRO_SPAD(struct AffineParams,affine_params,affine_params_ptr,affine_params_lcl,affine_params_dbg,2);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,3);


	ocrGuid_t Affine(
		struct complexData **curImage,		ocrGuid_t curImage_dbg,
		struct complexData **refImage,		ocrGuid_t refImage_dbg,
		struct AffineParams *affine_params,	ocrGuid_t affine_params_dbg, struct AffineParams *affine_params_ptr,
		struct ImageParams *image_params,	ocrGuid_t image_params_dbg,
		ocrGuid_t post_Affine_scg,		ocrGuid_t post_affine_async_1_scg);

	Affine(	curImage,	curImage_dbg,
		refImage,	refImage_dbg,
		affine_params,	affine_params_dbg, affine_params_ptr,
		image_params,	image_params_dbg,
		post_Affine_scg,post_affine_async_1_scg);

#ifdef TRACE_LVL_2
xe_printf("//// leave Affine_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

void gauss_elim(float *AA[], float *x, int N);

struct point corr2D(struct point ctrl_pt, int Nwin, int R,
    struct complexData**, struct complexData**, struct ImageParams*);

ocrGuid_t post_affine_async_1_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_3
xe_printf("////// enter post_affine_async_1_edt\n");RAG_FLUSH;
#endif
	assert(paramc==1);
	ocrGuid_t post_affine_async_2_scg = (ocrGuid_t)paramv[0]; // post_affine_async_2_scg
	assert(depc==9); // 9th is post_Affine_evg
	if(depc!=9) { xe_printf("// RAG // RAG // Need to handle N (affine_params->Nc) == 0 case // RAG // RAG// \n");RAG_FLUSH; }
RAG_REF_MACRO_SPAD(struct async_1_args_t,post_affine_async_1_args,post_affine_async_1_args_ptr,post_affine_async_1_args_lcl,post_affine_async_1_args_dbg,0);
RAG_REF_MACRO_SPAD(struct AffineParams,affine_params,affine_params_ptr,affine_params_lcl,affine_params_dbg,1);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,2);
RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,3);
RAG_REF_MACRO_BSM( struct complexData **,refImage,NULL,NULL,refImage_dbg,4);
RAG_REF_MACRO_BSM( int *,Fx,NULL,NULL,Fx_dbg,5);
RAG_REF_MACRO_BSM( int *,Fy,NULL,NULL,Fy_dbg,6);
RAG_REF_MACRO_BSM( int **,A,NULL,NULL,A_dbg,7);

	// b = 6 x 2
	float b[6][2];
	//
	// aug_mat = 6 x 7
	ocrGuid_t aug_mat_dbg;
	float **aug_mat;
	aug_mat = (float **)spad_malloc(&aug_mat_dbg,6*sizeof(float *)
						   +6*7*sizeof(float));
	if(aug_mat == NULL) {
		xe_printf("Unable to allocate memory for aug_mat.\n");RAG_FLUSH;
		xe_exit(1);
	}
	float *aug_mat_data_ptr = (float *)&aug_mat[6];
	if(aug_mat_data_ptr == NULL) {
		xe_printf("Unable to allocate memory for aug_mat.\n");RAG_FLUSH;
		xe_exit(1);
	}
	for(int n=0;n<6;n++) {
	        aug_mat[n] = aug_mat_data_ptr + n*7;
	        for(int i=0;i<7;i++) {
			aug_mat[n][i] = 0.0f;
		}
	}

	// Wcx[6];
	float Wcx[6];
	// Wcy[6];
	float Wcy[6];

#if defined(DEBUG) && !defined(RAG_SIM)
printf("Nc = %d\n",affine_params->Nc);RAG_FLUSH;
for(int i=0;i<affine_params->Nc;i++) {
	printf("A[%d][*] = %d %d %d %d %d %d\n",i, A[i][0], A[i][1], A[i][2], A[i][3], A[i][4], A[i][5]);RAG_FLUSH;
}
#endif

	// b = A'F
	// The computation of A'F and A'A requires better precision than would be obtained
	// by simply typecasting the A components to floats for the accumulation.  Here,
	// we use double precision accumulators, although 64-bit integer accumulations or
	// dynamic range rescaling provide other viable options to obtain additional precision.
	for(int m=0; m<6; m++) {
		double accum_x = 0.0, accum_y = 0.0;
		for(int n=0; n<affine_params->Nc; n++) {
			accum_x += (double)A[n][m]*(double)Fx[n];
			accum_y += (double)A[n][m]*(double)Fy[n];
		}
		b[m][0] = accum_x;
		b[m][1] = accum_y;
	}
#if defined(DEBUG) && !defined(RAG_SIM)
for(int i=0;i<6;i++) {
	printf("b[%d][*] = %f %f\n",i, b[i][0], b[i][1]);RAG_FLUSH;
}
#endif

	// aug_mat(1:6,1:6) = A'A
	for(int m=0; m<6; m++) {
		for(int n=0; n<6; n++) {
			double accum = 0.0;
			for(int k=0; k<affine_params->Nc; k++) {
				accum += (double)A[k][m] * (double) A[k][n];
			}
			aug_mat[m][n] = (float)accum;
		}
	}

	// aug_mat(1:6,7) = b(1:6,1)
	for(int m=0; m<6; m++) {
		aug_mat[m][6] = b[m][0];
	}
#if defined(DEBUG) && !defined(RAG_SIM)
for(int i=0;i<6;i++) {
	printf("aug[%d][*] = %f %f %f %f %f %f %f\n",i, aug_mat[i][0], aug_mat[i][1], aug_mat[i][2], aug_mat[i][3], aug_mat[i][4], aug_mat[i][5],aug_mat[i][6]);RAG_FLUSH;
}
#endif
#ifdef TRACE_LVL_3
xe_printf("////// Perform Gaussian elimination to find Wcx\n");RAG_FLUSH;
#endif
	gauss_elim(aug_mat, Wcx, 6);
#if defined(DEBUG) && !defined(RAG_SIM)
printf("Wcx = %f %f %f %f %f %f\n", Wcx[0], Wcx[1], Wcx[2], Wcx[3], Wcx[4], Wcx[5]);RAG_FLUSH;
#endif
	// aug_mat(1:6,7) = b(1:6,2)
	for(int m=0; m<6; m++) {
		aug_mat[m][6] = b[m][1];
	}
#if defined(DEBUG) && !defined(RAG_SIM)
for(int i=0;i<6;i++) {
	printf("aug[%d][*] = %f %f %f %f %f %f %f\n",i, aug_mat[i][0], aug_mat[i][1], aug_mat[i][2], aug_mat[i][3], aug_mat[i][4], aug_mat[i][5],aug_mat[i][6]);RAG_FLUSH;
}
#endif
#ifdef TRACE_LVL_3
xe_printf("////// Perform Gaussian elimination to find Wcy\n");RAG_FLUSH;
#endif
	gauss_elim(aug_mat, Wcy, 6);
#if defined(DEBUG) && !defined(RAG_SIM)
printf("Wcy = %f %f %f %f %f %f\n", Wcy[0], Wcy[1], Wcy[2], Wcy[3], Wcy[4], Wcy[5]);RAG_FLUSH;
#endif

	// Loop over the output pixel locations and interpolate the Target image
	// pixel values at these points. This is done by mapping the (rectangular)
	// Source coordinates into the Target coordinates and performing the
	// interpolation there.

	spad_free(aug_mat,aug_mat_dbg);

#ifdef RAG_NEW_BLK_SIZE
	int Xend = image_params->Ix;
	int Yend = image_params->Iy;
	int AFFINE_ASYNC_2_BLOCK_SIZE_X = 32;
	int AFFINE_ASYNC_2_BLOCK_SIZE_Y = 32;
#else
	int Xend = image_params->Ix;
	int Yend = image_params->Iy;
	int AFFINE_ASYNC_2_BLOCK_SIZE_X = blk_size(Xend,32);
	int AFFINE_ASYNC_2_BLOCK_SIZE_Y = blk_size(Yend,32);
	assert( (Xend%AFFINE_ASYNC_2_BLOCK_SIZE_X) == 0);
	assert( (Yend%AFFINE_ASYNC_2_BLOCK_SIZE_Y) == 0);
#endif

#ifdef TRACE_LVL_3
xe_printf("////// create a template for affine_async_2_edt function\n");RAG_FLUSH;
#endif
ocrGuid_t affine_async_2_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv);
	ocrGuid_t affine_async_2_clg;
	retval = ocrEdtTemplateCreate(
			&affine_async_2_clg,	// ocrGuid_t *new_guid
			 affine_async_2_edt,	// ocr_edt_ptr func_ptr
			(sizeof(struct corners_t) + sizeof(uint64_t) - 1)/sizeof(uint64_t), // paramc
			5);			// depc
	assert(retval==0);
	templateList[__sync_fetch_and_add(&templateIndex,1)] = affine_async_2_clg;

	struct async_2_args_t *async_2_args_ptr; ocrGuid_t async_2_args_dbg;
	async_2_args_ptr = bsm_malloc(&async_2_args_dbg,sizeof(struct async_2_args_t));
	SPADtoBSM(async_2_args_ptr->Wcx,Wcx,6*sizeof(float));
	SPADtoBSM(async_2_args_ptr->Wcy,Wcy,6*sizeof(float));

	RAG_DEF_MACRO_PASS(post_affine_async_2_scg,NULL,NULL,NULL,NULL,affine_params_dbg,0);
	RAG_DEF_MACRO_PASS(post_affine_async_2_scg,NULL,NULL,NULL,NULL,image_params_dbg,1);
	RAG_DEF_MACRO_PASS(post_affine_async_2_scg,NULL,NULL,NULL,NULL,curImage_dbg,2);
	RAG_DEF_MACRO_PASS(post_affine_async_2_scg,NULL,NULL,NULL,NULL,post_affine_async_1_args_dbg,3);
	RAG_DEF_MACRO_PASS(post_affine_async_2_scg,NULL,NULL,NULL,NULL,async_2_args_dbg,4);
	RAG_DEF_MACRO_PASS(post_affine_async_2_scg,NULL,NULL,NULL,NULL,Fx_dbg,5);
	RAG_DEF_MACRO_PASS(post_affine_async_2_scg,NULL,NULL,NULL,NULL,Fy_dbg,6);
	RAG_DEF_MACRO_PASS(post_affine_async_2_scg,NULL,NULL,NULL,NULL,A_dbg,7);

#ifdef RAG_NEW_BLK_SIZE
	for(int m=0; m<Yend; m+=AFFINE_ASYNC_2_BLOCK_SIZE_Y) {
		for(int n=0; n<Xend; n+=AFFINE_ASYNC_2_BLOCK_SIZE_X) {
			struct corners_t async_corners;
			async_corners.m1   = m;
			async_corners.m2   = (m+AFFINE_ASYNC_2_BLOCK_SIZE_X)<Xend?(m+AFFINE_ASYNC_2_BLOCK_SIZE_X):Xend;
			async_corners.n1   = n;
			async_corners.n2   = (n+AFFINE_ASYNC_2_BLOCK_SIZE_Y)<Yend?(n+AFFINE_ASYNC_2_BLOCK_SIZE_Y):Yend;
#else
	for(int m=0; m<Yend; m+=AFFINE_ASYNC_2_BLOCK_SIZE_Y) {
		for(int n=0; n<Xend; n+=AFFINE_ASYNC_2_BLOCK_SIZE_X) {
			struct corners_t async_corners;
			async_corners.m1   = m;
			async_corners.m2   = m+AFFINE_ASYNC_2_BLOCK_SIZE_X;
			async_corners.n1   = n;
			async_corners.n2   = n+AFFINE_ASYNC_2_BLOCK_SIZE_Y;
#endif
#ifdef TRACE_LVL_3
xe_printf("////// create an edt for affine_async_2\n");RAG_FLUSH;
#endif
			ocrGuid_t affine_async_2_scg;
			retval = ocrEdtCreate(
					&affine_async_2_scg,	// *created_edt_guid
					 affine_async_2_clg,	// edt_template_guid
					EDT_PARAM_DEF,		// paramc
					(uint64_t *)&async_corners, // *paramv
					EDT_PARAM_DEF,		// depc
					NULL,			// *depv
					EDT_PROP_NONE,		// properties
					NULL_GUID,		// affinity
					NULL);			// *outputEvent
			assert(retval==0);

RAG_DEF_MACRO_PASS(affine_async_2_scg,NULL,NULL,NULL,NULL,affine_params_dbg,0);
RAG_DEF_MACRO_PASS(affine_async_2_scg,NULL,NULL,NULL,NULL,image_params_dbg,1);
RAG_DEF_MACRO_PASS(affine_async_2_scg,NULL,NULL,NULL,NULL,curImage_dbg,2);
RAG_DEF_MACRO_PASS(affine_async_2_scg,NULL,NULL,NULL,NULL,post_affine_async_1_args->output_dbg,3);
RAG_DEF_MACRO_PASS(affine_async_2_scg,NULL,NULL,NULL,NULL,async_2_args_dbg,4);
		} // for n
	} // for m

#ifdef TRACE_LVL_3
xe_printf("////// leave post_affine_async_1_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

ocrGuid_t affine_async_1_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
	assert(paramc==0);
	assert(depc==8);
RAG_REF_MACRO_SPAD(struct async_1_args_t,async_1_args,async_1_args_ptr,async_1_args_lcl,async_1_args_dbg,0);
RAG_REF_MACRO_SPAD(struct AffineParams,affine_params,affine_params_ptr,affine_params_lcl,affine_params_dbg,1);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,2);
RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,3);
RAG_REF_MACRO_BSM( struct complexData **,refImage,NULL,NULL,refImage_dbg,4);
RAG_REF_MACRO_BSM( int *,Fx,NULL,NULL,Fx_dbg,5);
RAG_REF_MACRO_BSM( int *,Fy,NULL,NULL,Fy_dbg,6);
RAG_REF_MACRO_BSM( int **,A,NULL,NULL,A_dbg,7);

	struct point ctrl_pt = async_1_args->ctrl_pt;
#ifdef TRACE_LVL_3
xe_printf("////// enter affine_async_1_edt\n");RAG_FLUSH;
#endif
#if defined(TRACE_LVL_3) && !defined(RAG_SIM)
printf("////// enter affine_async_1_edt ctrl_pt %d %d %f\n",ctrl_pt.x,ctrl_pt.y,ctrl_pt.p);RAG_FLUSH;
#endif
#if defined(TRACE_LVL_3) && !defined(RAG_SIM)
printf("////// enter affine_async_1_edt Fx %16.16lx Fy %16.16lx A %16.16lx\n",Fx,Fy,A);RAG_FLUSH;
#endif
	// disp_vec
	// disp_vec
	struct point disp_vec;

#ifdef TRACE_LVL_3
xe_printf("////// Perform 2D correlation\n");RAG_FLUSH;
#endif
	disp_vec = corr2D(ctrl_pt, affine_params->Sc, affine_params->Rc,
				curImage, refImage, image_params);
#ifdef TRACE_LVL_3
xe_printf("////// Only retain control points that exceed the threshold\n");RAG_FLUSH;
#endif
	if(disp_vec.p >= affine_params->Tc) {
#ifdef TRACE_LVL_3
xe_printf("////// disp_vec.p >= affine_params->Tc\n");RAG_FLUSH;
#endif

        // Update Number of control points found
		const int k = __sync_fetch_and_add(
			((int *)(((char *)affine_params_ptr)
		                + offsetof(struct AffineParams,Nc))),1);

#ifdef TRACE_LVL_3
xe_printf("////// Form Fx, Fy (k=%d) and A\n",k);RAG_FLUSH;
#endif
		Fx[k] = ctrl_pt.x + disp_vec.x;
		Fy[k] = ctrl_pt.y + disp_vec.y;

		A[k][0] = 1;
		A[k][1] = ctrl_pt.x;
		A[k][2] = ctrl_pt.y;
		A[k][3] = ctrl_pt.x*ctrl_pt.x;
		A[k][4] = ctrl_pt.y*ctrl_pt.y;
		A[k][5] = ctrl_pt.x*ctrl_pt.y;

	} // if above threshold
	bsm_free(async_1_args,async_1_args_dbg);
#ifdef TRACE_LVL_3
xe_printf("////// leave affine_async_1_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

ocrGuid_t post_affine_async_2_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_3
xe_printf("////// enter post_affine_async_2_edt\n");RAG_FLUSH;
#endif
	assert(paramc==0);
	assert(depc==9); // 9th post_affine_async_1_evg
RAG_REF_MACRO_PASS(struct AffineParams,affine_params,affine_params_ptr,affine_params_lcl,affine_params_dbg,0);
RAG_REF_MACRO_PASS(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,1);
RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,2);
RAG_REF_MACRO_SPAD(struct async_1_args_t,post_affine_async_1_args,post_affine_async_1_args_ptr,post_affine_async_1_args_lcl,post_affine_async_1_args_dbg,3);
RAG_REF_MACRO_SPAD(struct async_2_args_t,async_2_args,async_2_args_ptr,async_2_args_lcl,async_2_args_dbg,4);
RAG_REF_MACRO_BSM( int *,Fx,NULL,NULL,Fx_dbg,5);
RAG_REF_MACRO_BSM( int *,Fy,NULL,NULL,Fy_dbg,6);
RAG_REF_MACRO_BSM( int **,A,NULL,NULL,A_dbg,7);

// RAG	RAG_DEF_MACRO_PASS(async_1_args->post_Affine_scg,NULL,NULL,NULL,NULL,async_1_args->output_dbg,0);

#ifdef TRACE_LVL_3
xe_printf("// Free data blocks\n");RAG_FLUSH;
#endif
	bsm_free(A, A_dbg);
	bsm_free(Fy,Fy_dbg);
	bsm_free(Fx,Fx_dbg);

	bsm_free(post_affine_async_1_args_ptr,post_affine_async_1_args_dbg);
	bsm_free(async_2_args_ptr,async_2_args_dbg);

#ifdef TRACE_LVL_3
xe_printf("////// leave post_affine_async_2_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

ocrGuid_t affine_async_2_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
	int retval;
#ifdef TRACE_LVL_3
xe_printf("////// enter affine_async_2_edt\n");RAG_FLUSH;
#endif
	assert(paramc==((sizeof(struct corners_t) + sizeof(uint64_t) - 1)/sizeof(uint64_t)));
	struct corners_t *corners = (struct corners_t *)paramv;
	int m1   = corners->m1;
	int m2   = corners->m2;
	int n1   = corners->n1;
	int n2   = corners->n2;
	assert(depc==5);
RAG_REF_MACRO_PASS(struct AffineParams,affine_params,affine_params_ptr,affine_params_lcl,affine_params_dbg,0);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,1);
RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,2);
RAG_REF_MACRO_BSM( struct complexData **,output,NULL,NULL,output_dbg,3);
RAG_REF_MACRO_SPAD(struct async_2_args_t,async_2_args,async_2_args_ptr,async_2_args_lcl,async_2_args_dbg,4);

	int aa, bb;
	float Px, Py, w, v;

	float Wcx[6];
	SPADtoSPAD(Wcx,async_2_args->Wcx,6*sizeof(float));
#if defined(DEBUG) && !defined(RAG_SIM)
printf("wCX = %f %f %f %f %f %f\n", Wcx[0], Wcx[1], Wcx[2], Wcx[3], Wcx[4], Wcx[5]);RAG_FLUSH;
#endif
	float Wcy[6];
	SPADtoSPAD(Wcy,async_2_args->Wcy,6*sizeof(float));
#if defined(DEBUG) && !defined(RAG_SIM)
printf("wCY = %f %f %f %f %f %f\n", Wcy[0], Wcy[1], Wcy[2], Wcy[3], Wcy[4], Wcy[5]);RAG_FLUSH;
#endif
	for(int m=m1; m<m2; m++) {
		for(int n=n1; n<n2; n++) {
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

			if( (aa >= 0) && (aa < image_params->Iy-1)
			&& (bb >= 0) && (bb < image_params->Ix-1) ) {
				output[m][n].real 	= (1-v)*(1-w)*curImage[aa  ][bb  ].real
							+ (  v)*(1-w)*curImage[aa  ][bb+1].real
							+ (1-v)*(  w)*curImage[aa+1][bb  ].real
							+ (  v)*(  w)*curImage[aa+1][bb+1].real;
				output[m][n].imag	= (1-v)*(1-w)*curImage[aa  ][bb  ].imag
							+ (  v)*(1-w)*curImage[aa  ][bb+1].imag
							+ (1-v)*(  w)*curImage[aa+1][bb  ].imag
							+ (  v)*(  w)*curImage[aa+1][bb+1].imag;
			} else {
				output[m][n].real = 0.0f;
				output[m][n].imag = 0.0f;
			}
		} // for n
	} // for m

#ifdef TRACE_LVL_3
xe_printf("////// leave affine_async_2_edt\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

ocrGuid_t Affine(
    struct complexData **curImage,	ocrGuid_t curImage_dbg,
    struct complexData **refImage,	ocrGuid_t refImage_dbg,
    struct AffineParams *affine_params,	ocrGuid_t affine_params_dbg, struct AffineParams *affine_params_ptr,
    struct ImageParams *image_params,	ocrGuid_t image_params_dbg,
    ocrGuid_t post_Affine_scg,		ocrGuid_t post_affine_async_1_scg)
{
	int retval;
#ifdef TRACE_LVL_2
xe_printf("//// enter Affine\n");RAG_FLUSH;
#endif
	int N, min;
	int dx, dy;
#ifdef TRACE_LVL_2
xe_printf("//// Affine registration dynamically allocated variables size = %d\n",affine_params->Nc);RAG_FLUSH;
#endif
	int **A;
	int *Fx, *Fy;
	struct complexData **output;

	// Fx = Nc x 1

	ocrGuid_t Fx_dbg;
	Fx = (int*)bsm_calloc(&Fx_dbg,affine_params->Nc,sizeof(int));
	if(Fx == NULL) {
		xe_printf("Unable to allocate memory for Fx.\n");RAG_FLUSH;
		xe_exit(1);
	}

	// Fy = Nc x 1

	ocrGuid_t Fy_dbg;
	Fy = (int*)bsm_calloc(&Fy_dbg,affine_params->Nc,sizeof(int));
	if(Fy == NULL) {
		xe_printf("Unable to allocate memory for Fy.\n");RAG_FLUSH;
		xe_exit(1);
	}

	// A = Nc x 6

	ocrGuid_t A_dbg;
	A = (int**)bsm_malloc(&A_dbg,(affine_params->Nc)*sizeof(int *)
				    +(affine_params->Nc)*6*sizeof(int));
	if(A == NULL) {
		xe_printf("Error allocating memory for A.\n");RAG_FLUSH;
		xe_exit(1);
	}
	int *A_data_ptr = (int *)&A[affine_params->Nc];
	if(A_data_ptr == NULL) {
		xe_printf("Unable to allocate memory for A.\n");RAG_FLUSH;
		xe_exit(1);
	}
	for(int m=0;m<affine_params->Nc;m++) {
	        A[m] = A_data_ptr + m*6;
	        for(int i=0;i<6;i++) {
			A[m][i] = 0;
		}
	}


#ifdef TRACE_LVL_2
xe_printf("//// Allocate memory for output image (%dx%d)\n",image_params->Iy,image_params->Ix);RAG_FLUSH;
#endif
	ocrGuid_t output_dbg;
#ifdef RAG_DRAM
	output = (struct complexData**)dram_malloc(&output_dbg,(image_params->Iy)*sizeof(struct complexData*)
					                      +(image_params->Iy)*(image_params->Ix)*sizeof(struct complexData));
#else
	output = (struct complexData**) bsm_malloc(&output_dbg,(image_params->Iy)*sizeof(struct complexData*)
							      +(image_params->Iy)*(image_params->Ix)*sizeof(struct complexData));
#endif
	if(output == NULL) {
		xe_printf("Error allocating memory for output.\n");RAG_FLUSH;
		xe_exit(1);
	}
	struct complexData *output_data_ptr = (struct complexData *)&output[image_params->Iy];
	if (output_data_ptr == NULL) {
		xe_printf("Error allocating memory for output.\n");RAG_FLUSH;
		xe_exit(1);
	}
	for(int n=0; n<image_params->Iy; n++) {
		output[n] = output_data_ptr + n*image_params->Ix;
	}

#ifdef TRACE_LVL_2
xe_printf("//// Calculate misc. parameters\n");RAG_FLUSH;
#endif
	N = (int)sqrtf_rn((float)affine_params->Nc);
#ifdef TRACE_LVL_2
xe_printf("//// N = %d (sqrtf_rn(%d))\n",N,affine_params->Nc);RAG_FLUSH;
#endif
	min = (affine_params->Sc-1)/2 + affine_params->Rc;

	dy = (image_params->Iy - affine_params->Sc + 1 - 2*affine_params->Rc)/N;
	dx = (image_params->Ix - affine_params->Sc + 1 - 2*affine_params->Rc)/N;

#ifdef TRACE
xe_printf("setting DRAM version of Nc to zero, will be updated with __sync_fetch_and_add() \n");RAG_FLUSH;
#endif
	affine_params->Nc = 0; 				// local copy of Nc
	affine_params_ptr->Nc = affine_params->Nc;	// global copy of Nc

#ifdef TRACE_LVL_2
xe_printf("//// create a template for affine_async_1_edt function (N=%d)\n",N);RAG_FLUSH;
#endif
	ocrGuid_t affine_async_1_clg;
	retval = ocrEdtTemplateCreate(
			&affine_async_1_clg,	// ocrGuid_t *new_guid
			 affine_async_1_edt,	// ocr_edt_ptr func_ptr
			0,			// paramc
			8);			// depc
	assert(retval==0);
	templateList[__sync_fetch_and_add(&templateIndex,1)] = affine_async_1_clg;

#ifdef TRACE_LVL_2
xe_printf("//// create a ctrl_pt affine_async_1_edt\n");RAG_FLUSH;
#endif
	struct point ctrl_pt;
	ctrl_pt.y =  0;
	ctrl_pt.x =  0;
	ctrl_pt.p = -1;
#ifdef TRACE_LVL_2
xe_printf("//// create a ctrl_pt post_affine_async_1_edt\n");RAG_FLUSH;
#endif
	struct async_1_args_t *post_affine_async_1_args_ptr; ocrGuid_t post_affine_async_1_args_dbg;
	post_affine_async_1_args_ptr = bsm_malloc(&post_affine_async_1_args_dbg,sizeof(struct async_1_args_t));
	post_affine_async_1_args_ptr->ctrl_pt = ctrl_pt;
	post_affine_async_1_args_ptr->output_dbg  = output_dbg;
	post_affine_async_1_args_ptr->curImage_dbg     = curImage_dbg;
	post_affine_async_1_args_ptr->refImage_dbg     = refImage_dbg;
	post_affine_async_1_args_ptr->post_Affine_scg  = post_Affine_scg;

#ifdef TRACE_LVL_2
xe_printf("//// statisy post_affine_async_1_edt\n");RAG_FLUSH;
#endif
	RAG_DEF_MACRO_PASS(post_affine_async_1_scg,NULL,NULL,NULL,NULL,post_affine_async_1_args_dbg,0);
	RAG_DEF_MACRO_PASS(post_affine_async_1_scg,NULL,NULL,NULL,NULL,affine_params_dbg,1);
	RAG_DEF_MACRO_PASS(post_affine_async_1_scg,NULL,NULL,NULL,NULL,image_params_dbg,2);
	RAG_DEF_MACRO_PASS(post_affine_async_1_scg,NULL,NULL,NULL,NULL,curImage_dbg,3);
	RAG_DEF_MACRO_PASS(post_affine_async_1_scg,NULL,NULL,NULL,NULL,output_dbg,4);
	RAG_DEF_MACRO_PASS(post_affine_async_1_scg,NULL,NULL,NULL,NULL,Fx_dbg,5);
	RAG_DEF_MACRO_PASS(post_affine_async_1_scg,NULL,NULL,NULL,NULL,Fy_dbg,6);
	RAG_DEF_MACRO_PASS(post_affine_async_1_scg,NULL,NULL,NULL,NULL,A_dbg,7);

	for(int m=0; m<N; m++) {
		for(int n=0; n<N; n++) {
#ifdef TRACE_LVL_2
xe_printf("//// create an edt for affine_async_1 m=%d n=%d\n",m,n);RAG_FLUSH;
#endif
			struct point ctrl_pt;
			ctrl_pt.y = m*dy + min;
			ctrl_pt.x = n*dx + min;
			struct async_1_args_t *async_1_args_ptr; ocrGuid_t async_1_args_dbg;
			async_1_args_ptr = bsm_malloc(&async_1_args_dbg,sizeof(struct async_1_args_t));
			async_1_args_ptr->ctrl_pt = ctrl_pt;
			async_1_args_ptr->output_dbg  = output_dbg;
			async_1_args_ptr->curImage_dbg     = curImage_dbg;
			async_1_args_ptr->refImage_dbg     = refImage_dbg;
			async_1_args_ptr->post_Affine_scg  = post_Affine_scg;
			ocrGuid_t affine_async_1_scg;
			retval = ocrEdtCreate(
					&affine_async_1_scg,	// *created_edt_guid
					 affine_async_1_clg,	// edt_template_guid
					EDT_PARAM_DEF,		// paramc
					NULL,			// *paramv
					EDT_PARAM_DEF,		// depc
					NULL,			// *depv
					EDT_PROP_NONE,		// properties
					NULL_GUID,		// affinity
					NULL);			// *outputEvent
			assert(retval==0);

RAG_DEF_MACRO_PASS(affine_async_1_scg,NULL,NULL,NULL,NULL,async_1_args_dbg,0);
RAG_DEF_MACRO_PASS(affine_async_1_scg,NULL,NULL,NULL,NULL,affine_params_dbg,1);
RAG_DEF_MACRO_PASS(affine_async_1_scg,NULL,NULL,NULL,NULL,image_params_dbg,2);
RAG_DEF_MACRO_PASS(affine_async_1_scg,NULL,NULL,NULL,NULL,curImage_dbg,3);
RAG_DEF_MACRO_PASS(affine_async_1_scg,NULL,NULL,NULL,NULL,refImage_dbg,4);
RAG_DEF_MACRO_PASS(affine_async_1_scg,NULL,NULL,NULL,NULL,Fx_dbg,5);
RAG_DEF_MACRO_PASS(affine_async_1_scg,NULL,NULL,NULL,NULL,Fy_dbg,6);
RAG_DEF_MACRO_PASS(affine_async_1_scg,NULL,NULL,NULL,NULL,A_dbg,7);
			OCR_DB_RELEASE(async_1_args_dbg);
		} // for n
	} // for m

RAG_DEF_MACRO_PASS(post_Affine_scg,NULL,NULL,NULL,NULL,output_dbg,0);
RAG_DEF_MACRO_PASS(post_Affine_scg,NULL,NULL,NULL,NULL,curImage_dbg,1);
RAG_DEF_MACRO_PASS(post_Affine_scg,NULL,NULL,NULL,NULL,refImage_dbg,2);
RAG_DEF_MACRO_PASS(post_Affine_scg,NULL,NULL,NULL,NULL,image_params_dbg,3);
#ifdef TRACE_LVL_2
xe_printf("//// leave Affine\n");RAG_FLUSH;
#endif
    return NULL_GUID;
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
#ifdef TRACE_LVL_4
        xe_printf("//////// corr2D ctrl_pt.x %d ctrl_pt.y %d\n",ctrl_pt.x,ctrl_pt.y);RAG_FLUSH;
#endif
    ocrGuid_t f_dbg;
    f = (struct complexData*)spad_malloc(&f_dbg,Nwin*Nwin*sizeof(struct complexData));
    ocrGuid_t g_dbg;
    g = (struct complexData*)spad_malloc(&g_dbg,Nwin*Nwin*sizeof(struct complexData));

    if( f == NULL || g == NULL ) {
        xe_printf("Unable to allocate memory for correlation windows.\n");RAG_FLUSH;
        xe_exit(1);
    }

    for(i=ctrl_pt.y-(Nwin-1)/2, k=mu_f.real=mu_f.imag=0; i<=ctrl_pt.y+(Nwin-1)/2; i++)
    {
        for(j=ctrl_pt.x-(Nwin-1)/2; j<=ctrl_pt.x+(Nwin-1)/2; j++, k++)
        {
            if( (i < 0) || (j < 0) || (i >= image_params->Iy) || (j >= image_params->Ix) ) {
                xe_printf("Warning: Index out of bounds in registration correlation.\n");RAG_FLUSH;
            }
            f[k].real  = curImage[i][j].real;
            f[k].imag  = curImage[i][j].imag;
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
                        xe_printf("Warning: Index out of bounds in registration correlation.\n");RAG_FLUSH;
                    }
                    g[k].real  = refImage[i][j].real;
                    g[k].imag  = refImage[i][j].imag;
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
                rho = sqrtf_rn( (num.real*num.real + num.imag*num.imag) / (den1*den2) );
            }
            else {
                rho = 0.0;
            }

/*          if(rho < 0 || rho > 1) {
                xe_printf("Correlation value out of range.\n");fflush(stderr); RAG_FLUSH;
                xe_exit(1);
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
#ifdef TRACE_LVL_4
        xe_printf("//////// gauss_elim\n");RAG_FLUSH;
#endif
	ocrGuid_t a_dbg;
	a = (float **)spad_malloc(&a_dbg,(N)*sizeof(float *)
				       +(N)*(N+1)*sizeof(float));
	if(a == NULL) {
		xe_printf("Unable to allocate memory for a.\n");RAG_FLUSH;
		xe_exit(1);
	}
        float *a_data_ptr = (float *)&a[N];
        if(a_data_ptr == NULL) {
            xe_printf("Unable to allocate memory for a.\n");RAG_FLUSH;
            xe_exit(1);
        }
	for(i=0; i<N; i++)
		a[i] = a_data_ptr + i*(N+1);

	for(i=0; i<N; i++) {
		for(j=0; j<N+1; j++) {
			a[i][j] = AA[i][j];
		}
	}

	for(i=0; i<N; i++) {
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
			if(a[j][i] != 0) {
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

	spad_free(a,a_dbg);
	return;
}

#ifdef RAG_THIN
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
    L = (float **)malloc((ts_params->Nf+3)*sizeof(float*));
    if(L == NULL) {
        xe_printf("Unable to allocate memory for L.\n");RAG_FLUSH;
        xe_exit(1);
    }
    for(m=0; m<ts_params->Nf+3; m++) {
        L[m] = (float*)malloc((ts_params->Nf+3+1)*sizeof(float));
        if(L[m] == NULL) {
            xe_printf("Unable to allocate memory for L.\n");RAG_FLUSH;
            xe_exit(1);
        }
    }

    // Yf = Nf+3 x 2
    Yf = (int**)malloc((ts_params->Nf+3)*sizeof(int*));
    if(Yf == NULL) {
        xe_printf("Unable to allocate memory for Yf.\n"); RAG_FLUSH;
        xe_exit(1);
    }
    for(m=0; m<ts_params->Nf+3; m++) {
        Yf[m] = (int*)malloc(2*sizeof(int));
        if(Yf[m] == NULL) {
            xe_printf("Unable to allocate memory for Yf.\n"); RAG_FLUSH;
            xe_exit(1);
        }
    }

    // P = Nf x 3
    P = (int**)malloc(ts_params->Nf*sizeof(int*));
    if(P == NULL) {
        xe_printf("Unable to allocate memory for P.\n"); RAG_FLUSH;
        xe_exit(1);
    }
    for(m=0; m<ts_params->Nf; m++) {
        P[m] = (int*)malloc(3*sizeof(int));
        if(P[m] == NULL) {
            xe_printf("Unable to allocate memory for P.\n"); RAG_FLUSH;
            xe_exit(1);
        }
    }

    // Wfx = Nf+3 x 1
    Wfx = (float*)malloc((ts_params->Nf+3)*sizeof(float));

    // Wfy = Nf+3 x 1
    Wfy = (float*)malloc((ts_params->Nf+3)*sizeof(float));

    // Allocate memory for output image
    output = (struct complexData**)malloc(image_params->Iy*sizeof(struct complexData*));
    if(output == NULL) {
        xe_printf("Error allocating memory for output.\n"); RAG_FLUSH;
        xe_exit(1);
    }
    for(n=0; n<image_params->Iy; n++) {
        output[n] = (struct complexData*)malloc(image_params->Ix*sizeof(struct complexData));
        if (output[n] == NULL) {
            xe_printf("Error allocating memory for output.\n"); RAG_FLUSH;
            xe_exit(1);
        }
    }

    // Yf(Nf+1:Nf+3,1:2) = 0
    for(m=ts_params->Nf; m<ts_params->Nf+3; m++) {
        spad_memset(&Yf[m][0], 0, 2*sizeof(int));
    }

    N = (int)sqrtf_rn((float)ts_params->Nf);
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
            r = sqrtf_rn( ((float)P[i][1]-(float)P[j][1])*((float)P[i][1]-(float)P[j][1]) + ((float)P[i][2]-(float)P[j][2])*((float)P[i][2]-(float)P[j][2]) );
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
                r = sqrtf_rn( (P[i][1]-n)*(P[i][1]-n) + (P[i][2]-m)*(P[i][2]-m) );
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
#ifdef RAG_DRAM
        SPADtoDRAM(&curImage[m][0], &output[m][0], image_params->Ix*sizeof(struct complexData));
#else
        BSMtoDRAM(&curImage[m][0], &output[m][0], image_params->Ix*sizeof(struct complexData));
#endif
    }

    free(L);
    free(P);
    free(Yf);
    free(Wfx);
    free(Wfy);
    free(output);
}
#endif // RAG_THIN
