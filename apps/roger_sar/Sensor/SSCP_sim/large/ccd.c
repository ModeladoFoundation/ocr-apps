#include <assert.h>

#include "common.h"

#include "rag_rmd.h"

#define MAX_Ncor (10)
#define MAX_Ncor_sqr (MAX_Ncor*MAX_Ncor)

rmd_guid_t post_CCD_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
	int retval;
#ifdef TRACE
xe_printf("//// enter post_CCD_codelet\n");RAG_FLUSH;
#endif
	assert(n_db == 1);
RAG_REF_MACRO_BSM( struct points *,corr_map,NULL,NULL,corr_map_dbg,0);

	rmd_guid_t arg_scg = { .data = arg };
RAG_DEF_MACRO_PASS(arg_scg,NULL,NULL,NULL,NULL,corr_map_dbg,0);
	RMD_DB_RELEASE(corr_map_dbg);
#ifdef TRACE
xe_printf("//// leave post_CCD_codelet\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

rmd_guid_t CCD_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
	int retval;
#ifdef TRACE
xe_printf("//// enter CCD_codelet\n");RAG_FLUSH;
#endif
	assert(n_db==4);
RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,0);
RAG_REF_MACRO_BSM( struct complexData **,refImage,NULL,NULL,refImage_dbg,1);
RAG_REF_MACRO_BSM( struct point **,corr_map,NULL,NULL,corr_map_dbg,2);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,3);
	rmd_guid_t arg_scg;
	arg_scg.data = arg;

#ifdef TRACE
xe_printf("//// create a codelet for post_CCD function\n");RAG_FLUSH;
#endif
	rmd_guid_t post_CCD_clg;
	retval = rmd_codelet_create(
		&post_CCD_clg,		// rmd_guid_t *new_guid
		 post_CCD_codelet,	// rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		1,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);

#ifdef TRACE
xe_printf("//// create an instance for post_CCD\n");RAG_FLUSH;
#endif
	rmd_guid_t post_CCD_scg;
	retval = rmd_codelet_sched(
		&post_CCD_scg,		// rmd_guid_t* scheduled codelet's guid
		arg_scg.data,		// uint64_t arg
		post_CCD_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);

	void CCD(
		struct complexData **curImage,    rmd_guid_t curImage_dbg,
		struct complexData **refImage,    rmd_guid_t refImage_dbg,
		struct point **corr_map,          rmd_guid_t corr_map_dbg,
		struct ImageParams *image_params, rmd_guid_t image_params_dbg,
		rmd_guid_t post_CCD_scg);

	CCD(	curImage,	curImage_dbg,
		refImage,	refImage_dbg,
		corr_map,	corr_map_dbg,
		image_params,	image_params_dbg,
		post_CCD_scg);

	return NULL_GUID;
}

rmd_guid_t ccd_final_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
	int retval;
#ifdef TRACE
xe_printf("////// enter ccd_final_codelet %d\n",n_db);RAG_FLUSH;
#endif
	assert(n_db>1);
	RAG_REF_MACRO_BSM( struct poinsts *,corr_map,NULL,NULL,corr_map_dbg,0);
	rmd_guid_t arg_scg = { .data = arg };
	RAG_DEF_MACRO_PASS(arg_scg,NULL,NULL,NULL,NULL,corr_map_dbg,0);
#ifdef TRACE
xe_printf("////// leave ccd_final_codelet\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

rmd_guid_t ccd_async_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
	int retval;
	assert(n_db==5);
#ifdef TRACE
xe_printf("////// enter ccd_async_codelet\n");RAG_FLUSH;
#endif
	rmd_guid_t arg_scg = { .data = arg };
RAG_REF_MACRO_SPAD(struct Corners_t,corners,corners_ptr,corners_lcl,corners_dbg,0);
RAG_REF_MACRO_BSM( struct complexData **,curImage,NULL,NULL,curImage_dbg,1);
RAG_REF_MACRO_BSM( struct complexData **,refImage,NULL,NULL,refImage_dbg,2);
RAG_REF_MACRO_BSM( struct points **,corr_map,NULL,NULL,corr_map_dbg,3);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,4);
	int Ncor = image_params->Ncor;
	int m1   = corners->x1;
	int m2   = corners->x2;
	int n1   = corners->y1;
	int n2   = corners->y2;
	int slot = corners->slot;
        int Ncor_sqr = Ncor * Ncor;
#ifdef TRACE
xe_printf("////// ccd_async m1 m2 n1 n2 s# %d %d %d %d (%d)\n",m1,m2,n1,m2,slot);RAG_FLUSH;
#endif
        for(int m=m1; m<m2; m++) {
            int mIndex = (Ncor-1)/2 + m;
            for(int n=n1; n<n2; n++) {
                struct complexData f[MAX_Ncor_sqr];
                struct complexData g[MAX_Ncor_sqr];
                const int nIndex = (Ncor-1)/2 + n;
	        float     den1, den2;
        	struct    complexData mu_f, mu_g, num;
                mu_f.real=0.0f;
                mu_f.imag=0.0f;
                mu_g.real=0.0f;
                mu_g.imag=0.0f;
                for(int k=0,i=mIndex-(Ncor-1)/2; i<=mIndex+(Ncor-1)/2; i++) {
                    struct complexData *cur_i;
                    cur_i = (struct complexData *)RAG_GET_PTR(curImage+i);
                    struct complexData *ref_i;
                    ref_i = (struct complexData *)RAG_GET_PTR(refImage+i);
                    for(int j=nIndex-(Ncor-1)/2; j<=nIndex+(Ncor-1)/2; j++, k++) {
                        struct complexData cur_i_j;
                        struct complexData ref_i_j;
			REM_LDX_ADDR(cur_i_j,cur_i+j,struct complexData);
			REM_LDX_ADDR(ref_i_j,ref_i+j,struct complexData);
                        f[k].real  = cur_i_j.real;
                        f[k].imag  = cur_i_j.imag;
                        g[k].real  = ref_i_j.real;
                        g[k].imag  = ref_i_j.imag;
                        mu_f.real += cur_i_j.real;
                        mu_f.imag += cur_i_j.imag;
                        mu_g.real += ref_i_j.real;
                        mu_g.imag += ref_i_j.imag;
                    } // for
                } // for k
                mu_f.real /= Ncor_sqr;
                mu_f.imag /= Ncor_sqr;
                mu_g.real /= Ncor_sqr;
                mu_g.imag /= Ncor_sqr;

		num.real = 0.0f;
		num.imag = 0.0f;
		den1 = 0.0f;
		den2 = 0.0f;
                for(int k=0; k<Ncor_sqr; k++) {
                    f[k].real -= mu_f.real;
                    f[k].imag -= mu_f.imag;
                    g[k].real -= mu_g.real;
                    g[k].imag -= mu_g.imag;
                    
                    num.real += f[k].real*g[k].real + f[k].imag*g[k].imag;
                    num.imag += f[k].real*g[k].imag - f[k].imag*g[k].real;

                    den1 += f[k].real*f[k].real + f[k].imag*f[k].imag;
                    den2 += g[k].real*g[k].real + g[k].imag*g[k].imag;
                } // for k

                struct point *corr_map_m;
                corr_map_m = (struct point *)RAG_GET_PTR(corr_map+m);
		struct point corr_map_m_n;
                corr_map_m_n.x = nIndex;
                corr_map_m_n.y = mIndex;
                if( (den1 != 0.0f) && (den2 != 0.0f) ) {
                    corr_map_m_n.p = sqrtf( (num.real*num.real + num.imag*num.imag) / (den1*den2) );
                } else {
                    corr_map_m_n.p = 0.0f;
                }
                REM_STX_ADDR(corr_map_m+n, corr_map_m_n,struct point);
            } // for n
	} // for n

RAG_DEF_MACRO_PASS(arg_scg,NULL,NULL,NULL,NULL,NULL_GUID,slot);
	bsm_free(corners,corners_dbg);
#ifdef TRACE
xe_printf("////// leave ccd_async_codelet\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}

void
CCD(	struct complexData **curImage,    rmd_guid_t curImage_dbg,
	struct complexData **refImage,    rmd_guid_t refImage_dbg,
	struct point **corr_map,          rmd_guid_t corr_map_dbg,
	struct ImageParams *image_params, rmd_guid_t image_params_dbg,
	rmd_guid_t post_CCD_scg)
{
	int retval;
#ifdef TRACE
xe_printf("//// enter CCD\n");RAG_FLUSH;
#endif
	int Mwins = image_params->Iy - image_params->Ncor + 1;
	int Nwins = image_params->Ix - image_params->Ncor + 1;
	assert(image_params->Ncor < MAX_Ncor);
#ifdef TRACE
xe_printf("//// Mwins == %d and Nwins == %d, Ncor == %d\n",Mwins,Nwins,image_params->Ncor);RAG_FLUSH;
#endif
#define BLOCK_SIZE 255
	assert((Mwins%BLOCK_SIZE)==0);
	assert((Nwins%BLOCK_SIZE)==0);
///////////// create async and final, instance of final
#ifdef TRACE
xe_printf("//// create a codelet for ccd_async function\n");RAG_FLUSH;
#endif
	rmd_guid_t ccd_async_clg;
	retval = rmd_codelet_create(
		&ccd_async_clg,     // rmd_guid_t *new_guid
		 ccd_async_codelet, // rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		5,			// int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);

#ifdef TRACE
xe_printf("//// create a codelet for ccd_final function\n");RAG_FLUSH;
#endif
	rmd_guid_t ccd_final_clg;
	retval = rmd_codelet_create(
		&ccd_final_clg,     // rmd_guid_t *new_guid
		 ccd_final_codelet, // rmd_codelet_ptr func_ptr
		0,			// size_t code_size
		0,			// uinit64_t default_arg
		((Mwins-0)/BLOCK_SIZE)*((Nwins-0)/BLOCK_SIZE)+1, // int n_dep
		1,			// int buffer_in
		false,			// bool gen_out
		0);			// uint64_t prop
	assert(retval==0);
#ifdef TRACE
// create an instance for ccd_final
#endif
	rmd_guid_t ccd_final_scg;
	retval = rmd_codelet_sched(
		&ccd_final_scg,		// rmd_guid_t* scheduled codelet's guid
		post_CCD_scg.data,		// uint64_t arg
		ccd_final_clg);		// rmd_guid_t created codelet's guid
	assert(retval==0);
	int slot = 0;
RAG_DEF_MACRO_PASS(ccd_final_scg,NULL,NULL,NULL,NULL,corr_map_dbg, slot++);

	for(int m=0; m<Mwins; m+=BLOCK_SIZE) {
		for(int n=0; n<Nwins; n+=BLOCK_SIZE) {
#ifdef TRACE
xe_printf("//// create an instance for ccd_async slot %d\n",slot);RAG_FLUSH;
#endif
			rmd_guid_t ccd_async_scg;
			retval = rmd_codelet_sched(
		&ccd_async_scg,		// rmd_guid_t* scheduled codelet's guid
		ccd_final_scg.data,	// uint64_t arg
		ccd_async_clg);		// rmd_guid_t created codelet's guid
			assert(retval==0);
			struct Corners_t *async_corners, *async_corners_ptr, async_corners_lcl; rmd_guid_t async_corners_dbg;
			async_corners = &async_corners_lcl;
			async_corners_ptr = bsm_malloc(&async_corners_dbg,sizeof(struct Corners_t));
			async_corners->x1   = m;
			async_corners->x2   = m+BLOCK_SIZE;
			async_corners->y1   = n;
			async_corners->y2   = n+BLOCK_SIZE;
			async_corners->slot = slot++;
			REM_STX_ADDR(async_corners_ptr,async_corners_lcl,struct Corners_t);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,async_corners_dbg,0);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,curImage_dbg,1);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,refImage_dbg,2);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,corr_map_dbg,3);
RAG_DEF_MACRO_PASS(ccd_async_scg,NULL,NULL,NULL,NULL,image_params_dbg,4);
			RMD_DB_RELEASE(async_corners_dbg);
		} /* for n */
	} /* for m */

	RMD_DB_RELEASE(curImage_dbg);
	RMD_DB_RELEASE(refImage_dbg);
	RMD_DB_RELEASE(corr_map_dbg);
	RMD_DB_RELEASE(image_params_dbg);
#ifdef TRACE
xe_printf("//// leave CCD\n");RAG_FLUSH;
#endif
	return;
}
