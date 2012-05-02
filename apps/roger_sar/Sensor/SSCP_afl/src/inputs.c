#include <stdint.h>

#include "common.h"

#include "rag_rmd.h"

int ReadParams(struct RadarParams *radar_params, struct ImageParams *image_params, struct AffineParams *affine_params, struct ThinSplineParams *ts_params, struct CfarParams *cfar_params)
{
#ifdef RAG_IMPLICIT_INPUTS
#include "Parameters.h"
	radar_params->fs        = RAG_Fs;
	radar_params->fc        = RAG_Fc;
	radar_params->PRF       = RAG_PRF;
	image_params->F         = RAG_F;
	image_params->Ix        = RAG_Ix;
	image_params->Iy        = RAG_Iy;
	image_params->Sx        = RAG_Sx;
	image_params->Sy        = RAG_Sy;
	image_params->P1        = RAG_P1;
	image_params->S1        = RAG_S1;
	radar_params->r0        = RAG_r0;
	radar_params->R0        = RAG_R0;
	affine_params->Nc       = RAG_Nc;
	affine_params->Rc       = RAG_Rc;
	affine_params->Sc       = RAG_Sc;
	affine_params->Tc       = RAG_Tc;
	ts_params->Nf           = RAG_Nf;
	ts_params->Rf           = RAG_Rf;
	ts_params->Scf          = RAG_Scf;
	ts_params->Tf           = RAG_Tf;
	image_params->Ncor      = RAG_Ncor;
	cfar_params->Ncfar      = RAG_Ncfar;
	cfar_params->Tcfar      = RAG_Tcfar;
	cfar_params->Nguard     = RAG_Nguard;
	image_params->numImages = RAG_NumberImages;
	image_params->imageNumber=0; // RAG
	radar_params->fs        = RAG_Fs;
#else
	char param[80];
	FILE *pFile;

	pFile = fopen("Parameters.txt", "r");

	if(pFile == NULL) {
		return 1;
	}

	fscanf(pFile, "%s %f", param, &radar_params->fs);
	if(strcmp(param, "Fs") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &radar_params->fc);
	if(strcmp(param, "Fc") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &radar_params->PRF);
	if(strcmp(param, "PRF") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &image_params->F);
	if(strcmp(param, "F") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &image_params->Ix);
	if(strcmp(param, "Ix") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &image_params->Iy);
	if(strcmp(param, "Iy") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &image_params->Sx);
	if(strcmp(param, "Sx") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &image_params->Sy);
	if(strcmp(param, "Sy") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &image_params->P1);
	if(strcmp(param, "P1") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &image_params->S1);
	if(strcmp(param, "S1") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &radar_params->r0);
	if(strcmp(param, "r0") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &radar_params->R0);
	if(strcmp(param, "R0") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &affine_params->Nc);
	if(strcmp(param, "Nc") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &affine_params->Rc);
	if(strcmp(param, "Rc") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &affine_params->Sc);
	if(strcmp(param, "Sc") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &affine_params->Tc);
	if(strcmp(param, "Tc") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &ts_params->Nf);
	if(strcmp(param, "Nf") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &ts_params->Rf);
	if(strcmp(param, "Rf") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &ts_params->Scf);
	if(strcmp(param, "Scf") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &ts_params->Tf);
	if(strcmp(param, "Tf") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &image_params->Ncor);
	if(strcmp(param, "Ncor") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &cfar_params->Ncfar);
	if(strcmp(param, "Ncfar") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &cfar_params->Tcfar);
	if(strcmp(param, "Tcfar") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &cfar_params->Nguard);
	if(strcmp(param, "Nguard") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &image_params->numImages);
	if(strcmp(param, "NumberImages") != 0) {
		return 2;
	}

	fclose(pFile);
#endif
	image_params->imageNumber=0; // RAG

	return 0;
}

#if defined(RAG_AFL) || defined(RAG_SIM)
// ReadData codelet

rmd_guid_t ReadData_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
	int retval;
	assert(n_db == 3);
	rmd_guid_t arg_scg;
RAG_REF_MACRO_SPAD(struct Inputs,in,in_ptr,in_lcl,in_dbg,0);
RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,1);
RAG_REF_MACRO_SPAD(struct file_args_t,file_args,file_args_ptr,file_args_lcl,file_args_dbg,2);
#ifdef TRACE_LVL_2
xe_printf("//// enter ReadData_codelet\n");RAG_FLUSH;
#endif
	FILE *pInFile  = file_args_lcl.pInFile;
	FILE *pInFile2 = file_args_lcl.pInFile2;
	FILE *pInFile3 = file_args_lcl.pInFile3;
#ifdef DEBUG
xe_printf("%ld %ld %ld\n",(uint64_t)pInFile, (uint64_t)pInFile2, (uint64_t) pInFile3);RAG_FLUSH;
#endif
	void ReadData(struct Inputs *in, struct ImageParams *image_params,
			FILE *pFile1, FILE *pFile2, FILE *pFile3);

	ReadData(in, image_params, pInFile, pInFile2, pInFile3);

REM_STX_ADDR(image_params_ptr,image_params_lcl,struct ImageParams);

	arg_scg.data = arg;
RAG_DEF_MACRO_SPAD(arg_scg,struct Inputs *,in,in_ptr,in_lcl,in_dbg,0); 

	RMD_DB_RELEASE(file_args_dbg);
	RMD_DB_RELEASE(in_dbg);
	RMD_DB_RELEASE(image_params_dbg);
#ifdef TRACE_LVL_2
xe_printf("//// leave ReadData_codelet\n");RAG_FLUSH;
#endif
	return NULL_GUID;
}
#endif // RAG_AFL or RAG_SIM

// ReadData

void ReadData(struct Inputs *in, struct ImageParams *image_params,
	FILE *pFile1, FILE *pFile2, FILE *pFile3)
{
#ifdef RAG_IMPLICIT_INPUTS
extern struct {int32_t real; int32_t imag;} image_0[RAG_P1][RAG_S1];
extern struct {int32_t real; int32_t imag;} image_1[RAG_P1][RAG_S1];
extern struct {int32_t x; int32_t y; int32_t z;} platform_0[RAG_P1][1];
extern struct {int32_t x; int32_t y; int32_t z;} platform_1[RAG_P1][1];
extern struct {int32_t t; } pulse_0[RAG_P1][1];
extern struct {int32_t t; } pulse_1[RAG_P1][1];
#ifdef TRACE_LVL_2
xe_printf("//// RAG_IMPLICIT_INPUTS P1 = %d S1 = %d\n",image_params->P1,image_params->S1);RAG_FLUSH;
#endif
#endif

	for(int m=0; m<image_params->P1; m++) {
#ifdef TRACE_LVL_2
if(!m){xe_printf("//// Read complex SAR data\n");RAG_FLUSH;}
#endif
#ifndef RAG_IMPLICIT_INPUTS
		fread(&in->X[m][0], sizeof(struct complexData), image_params->S1, pFile1);
#else
		if ( image_params->imageNumber == 0 ) {
// &in->X[m][0]
		SPADtoBSM(RAG_GET_PTR(in->X+m),&image_0[m][0], sizeof(struct complexData)*image_params->S1);
		} else if ( image_params->imageNumber == 1 ) {
// &in->X[m][0]
		SPADtoBSM(RAG_GET_PTR(in->X+m),&image_1[m][0], sizeof(struct complexData)*image_params->S1);
		} // endif image
#endif
#ifdef TRACE_LVL_2
if(!m){xe_printf("//// Read platform positions\n");RAG_FLUSH;}
#endif
#ifndef RAG_IMPLICIT_INPUTS
		fread(&in->Pt[m][0], sizeof(float), 3, pFile2);
#else
		if ( image_params->imageNumber == 0 ) {
// &in->Pt[m][0]
		SPADtoBSM(RAG_GET_PTR(in->Pt+m),&platform_0[m][0], sizeof(float)*3);
		} else if ( image_params->imageNumber == 1 ) {
// &in->Pt[m][0]
		SPADtoBSM(RAG_GET_PTR(in->Pt+m),&platform_1[m][0], sizeof(float)*3);
		} // endif image
#endif
	} // for m (P1)
#ifdef TRACE_LVL_2
xe_printf("//// Read pulse transmission timestamps\n");RAG_FLUSH;
#endif
#ifndef RAG_IMPLICIT_INPUTS
	fread(in->Tp, sizeof(float), image_params->P1, pFile3);
#else
	for(int m=0; m<image_params->P1; m++) {
		if ( image_params->imageNumber == 0 ) {
// &in->Tp[m]
		SPADtoBSM(in->Tp+m,&pulse_0[m][0].t, sizeof(float)*1);
		} else if ( image_params->imageNumber == 1 ) {
// &in->Tp[m]
		SPADtoBSM(in->Tp+m,&pulse_1[m][0].t, sizeof(float)*1);
		} // endif image
	} // for m (P1)
#endif
#ifdef RAG_IMPLICIT_INPUTS
	image_params->imageNumber++;
#endif
}
