#include <stdint.h>
#include <assert.h>

#include "common.h"

int ReadParams(struct RadarParams *radar_params, struct ImageParams *image_params, struct AffineParams *affine_params, struct ThinSplineParams *ts_params, int *Ncor, struct CfarParams *cfar_params)
{
	char param[80];
	FILE *pFile;

#ifdef RAG_IMPLICIT_INPUTS
#define RAG_Fs 600000000
#define RAG_Fc 10000000000
#define RAG_PRF 700
#define RAG_F 1
#define RAG_Ix 1024
#define RAG_Iy 1024
#define RAG_Sx 1024
#define RAG_Sy 1024
#define RAG_P1 4200
#define RAG_S1 4096
#define RAG_r0 10000
#define RAG_R0 9488
#define RAG_Nc 3629
#define RAG_Rc 16
#define RAG_Sc 31
#define RAG_Tc 0.9
#define RAG_Nf 400
#define RAG_Rf 4
#define RAG_Scf 9
#define RAG_Tf 0
#define RAG_Ncor 5
#define RAG_Ncfar 25
#define RAG_Tcfar 90
#define RAG_Nguard 17
#define RAG_NumberImages 2
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
	*Ncor                   = RAG_Ncor;
	cfar_params->Ncfar      = RAG_Ncfar;
	cfar_params->Tcfar      = RAG_Tcfar;
	cfar_params->Nguard     = RAG_Nguard;
	image_params->numImages = RAG_NumberImages;
	radar_params->fs        = RAG_Fs;
#else
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
	fscanf(pFile, "%s %d", param, Ncor);
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
	return 0;
}

void ReadData(FILE *pFile1, FILE *pFile2, FILE *pFile3, struct Inputs *in, struct ImageParams *image_params)
{
	int m;
#ifdef RAG_IMPLICIT_INPUTS
static int image=0;
extern struct {int32_t real; int32_t imag;} image_0[RAG_P1][RAG_S1];
extern struct {int32_t real; int32_t imag;} image_1[RAG_P1][RAG_S1];
extern struct {int32_t x; int32_t y; int32_t z;} platform_0[RAG_P1][1];
extern struct {int32_t x; int32_t y; int32_t z;} platform_1[RAG_P1][1];
extern struct {int32_t t; } pulse_0[RAG_P1][1];
extern struct {int32_t t; } pulse_1[RAG_P1][1];
fprintf(stderr,"RAG_IMPLICIT_INPUTS P1 = %d S1 = %d\n",image_params->P1,image_params->S1);fflush(stderr);
#endif
#ifdef RAG_IMPLICIT_INPUTS_TEST
fprintf(stderr,"RAG_IMPLICIT_INPUTS_TEST\n");fflush(stderr);
#endif

	for(m=0; m<image_params->P1; m++) {
#if !defined(RAG_IMPLICIT_INPUTS) || (defined(RAG_IMPLICIT_INPUTS) && defined(RAG_IMPLICIT_INPUTS_TEST))
		fread(&in->X[m][0], sizeof(struct complexData), image_params->S1, pFile1);	// Read complex SAR data
#endif
#ifdef RAG_IMPLICIT_INPUTS
		if ( image == 0 ) {
#ifdef RAG_IMPLICIT_INPUTS_TEST
for(int i=0;i<image_params->S1;i++) {
#ifdef DEBUG
if(in->X[m][i].real != *(float *)&image_0[m][i].real) {
fprintf(stderr,"image_0[%d][%d].real %f (%8.8x),%f(%8.8x)\n",m,i,
in->X[m][i].real,*(uint32_t *)&in->X[m][i].real,
*(float *)&image_0[m][i].real,image_0[m][i].real); fflush(stderr); }
if(in->X[m][i].imag != *(float *)&image_0[m][i].imag) {
fprintf(stderr,"image_0[%d][%d].imag %f (%8.8x),%f(%8.8x)\n",m,i,
in->X[m][i].imag,*(uint32_t *)&in->X[m][i].imag,
*(float *)&image_0[m][i].imag,image_0[m][i].imag); fflush(stderr); }
#endif
assert(in->X[m][i].real == *(float *)&image_0[m][i].real);
assert(in->X[m][i].imag == *(float *)&image_0[m][i].imag);
} // for i (S1)
#endif
		memcpy(&in->X[m][0],&image_0[m][0], sizeof(struct complexData)*image_params->S1);
		} else if ( image == 1 ) {
#ifdef RAG_IMPLICIT_INPUTS_TEST
for(int i=0;i<image_params->S1;i++) {
#ifdef DEBUG
if(in->X[m][i].real != *(float *)&image_1[m][i].real) {
fprintf(stderr,"image_1[%d][%d].real %f (%8.8x),%f(%8.8x)\n",m,i,
in->X[m][i].real,*(uint32_t *)&in->X[m][i].real,
*(float *)&image_1[m][i].real,image_1[m][i].real); fflush(stderr); }
if(in->X[m][i].imag != *(float *)&image_1[m][i].imag) {
fprintf(stderr,"image_1[%d][%d].imag %f (%8.8x),%f(%8.8x)\n",m,i,
in->X[m][i].imag,*(uint32_t *)&in->X[m][i].imag,
*(float *)&image_1[m][i].imag,image_1[m][i].imag); fflush(stderr); }
#endif
assert(in->X[m][i].real == *(float *)&image_1[m][i].real);
assert(in->X[m][i].imag == *(float *)&image_1[m][i].imag);
} // for i (S1)
#endif
		memcpy(&in->X[m][0],&image_1[m][0], sizeof(struct complexData)*image_params->S1);
		} // endif image
#endif
#if !defined(RAG_IMPLICIT_INPUTS) || (defined(RAG_IMPLICIT_INPUTS) && defined(RAG_IMPLICIT_INPUTS_TEST))
		fread(&in->Pt[m][0], sizeof(float), 3, pFile2);	// Read platform positions
#endif
#ifdef RAG_IMPLICIT_INPUTS
		if ( image == 0 ) {
#ifdef RAG_IMPLICIT_INPUTS_TEST
#ifdef DEBUG
if(in->Pt[m][0] != *(float *)&platform_0[m][0].x) {
fprintf(stderr,"platform_0[%d].x %f (%8.8x),%f(%8.8x)\n",m,
in->Pt[m][0],*(uint32_t *)&in->Pt[m][0],
*(float *)&platform_0[m][0].x,platform_0[m][0].x); fflush(stderr); }
if(in->Pt[m][1] != *(float *)&platform_0[m][0].y) {
fprintf(stderr,"platform_0[%d].y %f (%8.8x),%f(%8.8x)\n",m,
in->Pt[m][1],*(uint32_t *)&in->Pt[m][1],
*(float *)&platform_0[m][0].y,platform_0[m][0].y); fflush(stderr); }
if(in->Pt[m][2] != *(float *)&platform_0[m][0].z) {
fprintf(stderr,"platform_0[%d].z %f (%8.8x),%f(%8.8x)\n",m,
in->Pt[m][2],*(uint32_t *)&in->Pt[m][2],
*(float *)&platform_0[m][0].z,platform_0[m][0].z); fflush(stderr); }
#endif
assert(in->Pt[m][0] == *(float *)&platform_0[m][0].x);
assert(in->Pt[m][1] == *(float *)&platform_0[m][0].y);
assert(in->Pt[m][2] == *(float *)&platform_0[m][0].z);
#endif
		memcpy(&in->Pt[m][0],&platform_0[m][0], sizeof(float)*3);
		} else if ( image == 1 ) {
#ifdef RAG_IMPLICIT_INPUTS_TEST
#ifdef DEBUG
if(in->Pt[m][0] != *(float *)&platform_1[m][0].x) {
fprintf(stderr,"platform_1[%d].x %f (%8.8x),%f(%8.8x)\n",m,
in->Pt[m][0],*(uint32_t *)&in->Pt[m][0],
*(float *)&platform_1[m][0].x,platform_1[m][0].x); fflush(stderr); }
if(in->Pt[m][1] != *(float *)&platform_1[m][0].y) {
fprintf(stderr,"platform_1[%d].y %f (%8.8x),%f(%8.8x)\n",m,
in->Pt[m][1],*(uint32_t *)&in->Pt[m][1],
*(float *)&platform_1[m][0].y,platform_1[m][0].y); fflush(stderr); }
if(in->Pt[m][2] != *(float *)&platform_1[m][0].z) {
fprintf(stderr,"platform_1[%d].z %f (%8.8x),%f(%8.8x)\n",m,
in->Pt[m][2],*(uint32_t *)&in->Pt[m][2],
*(float *)&platform_1[m][0].z,platform_1[m][0].z); fflush(stderr); }
#endif
assert(in->Pt[m][0] == *(float *)&platform_1[m][0].x);
assert(in->Pt[m][1] == *(float *)&platform_1[m][0].y);
assert(in->Pt[m][2] == *(float *)&platform_1[m][0].z);
#endif
		memcpy(&in->Pt[m][0],&platform_1[m][0], sizeof(float)*3);
		} // endif image
#endif
	} // for m (P1)
#if !defined(RAG_IMPLICIT_INPUTS) || (defined(RAG_IMPLICIT_INPUTS) && defined(RAG_IMPLICIT_INPUTS_TEST))
	fread(in->Tp, sizeof(float), image_params->P1, pFile3);	// Read pulse transmission timestamps
#endif
#ifdef RAG_IMPLICIT_INPUTS
	for(m=0; m<image_params->P1; m++) {
		if ( image == 0 ) {
#ifdef RAG_IMPLICIT_INPUTS_TEST
#ifdef DEBUG
if(in->Tp[m] != *(float *)&pulse_0[m][0].t) {
fprintf(stderr,"pulse_0[%d].t %f (%8.8x),%f(%8.8x)\n",m,
in->Tp[m],*(uint32_t *)&in->Tp[m],
*(float *)&pulse_0[m][0].t,pulse_0[m][0].t); fflush(stderr); }
#endif
assert(in->Tp[m] == *(float *)&pulse_0[m][0].t);
#endif
		memcpy(&in->Tp[m],&pulse_0[m][0].t, sizeof(float)*1);
		} else if ( image == 1 ) {
#ifdef RAG_IMPLICIT_INPUTS_TEST
#ifdef DEBUG
if(in->Tp[m] != *(float *)&pulse_1[m][0].t) {
fprintf(stderr,"pulse_1[%d].t %f (%8.8x),%f(%8.8x)\n",m,
in->Tp[m],*(uint32_t *)&in->Tp[m],
*(float *)&pulse_1[m][0].t,pulse_1[m][0].t); fflush(stderr); }
#endif
assert(in->Tp[m] == *(float *)&pulse_1[m][0].t);
#endif
		memcpy(&in->Tp[m],&pulse_1[m][0].t, sizeof(float)*1);
		} // endif image
	} // for m (P1)
#endif
#ifdef RAG_IMPLICIT_INPUTS
	image++;
#endif
}
