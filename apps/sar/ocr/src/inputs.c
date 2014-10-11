#include <stdint.h>

#include "ocr.h"
#include "rag_ocr.h"
#include "common.h"

int ReadParams(struct RadarParams *radar_params, struct ImageParams *image_params, struct AffineParams *affine_params, struct CfarParams *cfar_params)
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
  image_params->Fbp       = RAG_Fbp;
  image_params->P1        = RAG_P1;
  image_params->S1        = RAG_S1;
  radar_params->r0        = RAG_r0;
  radar_params->R0        = RAG_R0;
  affine_params->Nc       = RAG_Nc;
  affine_params->Rc       = RAG_Rc;
  affine_params->Sc       = RAG_Sc;
  affine_params->Tc       = RAG_Tc;
  image_params->Ncor      = RAG_Ncor;
  cfar_params->Ncfar      = RAG_Ncfar;
  cfar_params->Tcfar      = RAG_Tcfar;
  cfar_params->Nguard     = RAG_Nguard;
  image_params->numImages = RAG_NumberImages;
  image_params->imageNumber=0; // RAG
  radar_params->fs        = RAG_Fs;
#else // RAG_IMPLICIT_INPUTS
  FILE *pFile;
  char param[80];

  pFile = fopen("Parameters.txt", "r");

  if(pFile == NULL) {
    return 1;
  }

  fscanf(pFile, "%s %f", param, &radar_params->fs);
  if(strcmp(param, "Fs") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Fs\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %f", param, &radar_params->fc);
  if(strcmp(param, "Fc") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Fc\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %f", param, &radar_params->PRF);
  if(strcmp(param, "PRF") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"PRF\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &image_params->F);
  if(strcmp(param, "F") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"F\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &image_params->Ix);
  if(strcmp(param, "Ix") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Ix\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &image_params->Iy);
  if(strcmp(param, "Iy") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Iy\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &image_params->Sx);
  if(strcmp(param, "Sx") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Sx\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &image_params->Sy);
  if(strcmp(param, "Sy") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Sy\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &image_params->Fbp);
  if(strcmp(param, "Fbp") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Fbp\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &image_params->P1);
  if(strcmp(param, "P1") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"P1\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &image_params->S1);
  if(strcmp(param, "S1") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"S1\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %f", param, &radar_params->r0);
  if(strcmp(param, "r0") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"r0\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %f", param, &radar_params->R0);
  if(strcmp(param, "R0") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"R0\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &affine_params->Nc);
  if(strcmp(param, "Nc") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Nc\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &affine_params->Rc);
  if(strcmp(param, "Rc") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Rc\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &affine_params->Sc);
  if(strcmp(param, "Sc") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Sc\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %f", param, &affine_params->Tc);
  if(strcmp(param, "Tc") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Tc\n");
#endif
    return 2;
  }

  fscanf(pFile, "%s %d", param, &image_params->Ncor);
  if(strcmp(param, "Ncor") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Ncor\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &cfar_params->Ncfar);
  if(strcmp(param, "Ncfar") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Ncfar\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %f", param, &cfar_params->Tcfar);
  if(strcmp(param, "Tcfar") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Tcfar\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &cfar_params->Nguard);
  if(strcmp(param, "Nguard") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"Nguard\n");
#endif
    return 2;
  }
  fscanf(pFile, "%s %d", param, &image_params->numImages);
  if(strcmp(param, "NumberImages") != 0) {
#ifndef TG_ARCH
    fprintf(stderr,"NumberImages\n");
#endif
    return 2;
  }

  fclose(pFile);

#endif // RAG_IMPLICIT_INPUTS

  image_params->imageNumber=0; // RAG

  return 0;
}

// ReadData edt

ocrGuid_t ReadData_edt(uint32_t paramc, uint64_t *paramv, uint32_t depc, ocrEdtDep_t *depv) {
  int retval;
#ifdef TRACE_LVL_2
  PRINTF("//// enter ReadData_edt\n");RAG_FLUSH;
#endif
  assert(paramc==1);
  ocrGuid_t arg_scg = (ocrGuid_t)paramv[0]; // FormImage_scg or refFormImage_scg
  assert(depc==5);
  RAG_REF_MACRO_SPAD(struct ImageParams,image_params,image_params_ptr,image_params_lcl,image_params_dbg,0);
  RAG_REF_MACRO_SPAD(struct file_args_t,file_args,file_args_ptr,file_args_lcl,file_args_dbg,1);
  RAG_REF_MACRO_BSM( struct complexData **,X,NULL,NULL,X_dbg,2);
  RAG_REF_MACRO_BSM( float **,Pt,NULL,NULL,Pt_dbg,3);
  RAG_REF_MACRO_BSM( float *,Tp,NULL,NULL,Tp_dbg,4);
  assert((void *)&X[0][0] == (void *)&X[image_params_lcl.P1]);   // check to see if Datablocks are getting relocated
  assert((void *)&Pt[0][0] == (void *)&Pt[image_params_lcl.P1]); // if they are then need to remap 2D array
#ifdef TG_ARCH
  void *pInFile  = file_args_lcl.pInFile;
  void *pInFile2 = file_args_lcl.pInFile2;
  void *pInFile3 = file_args_lcl.pInFile3;
#else
  FILE *pInFile  = file_args_lcl.pInFile;
  FILE *pInFile2 = file_args_lcl.pInFile2;
  FILE *pInFile3 = file_args_lcl.pInFile3;
#endif
#ifdef DEBUG
  PRINTF("file descripters are %ld %ld %ld\n",(uint64_t)pInFile, (uint64_t)pInFile2, (uint64_t) pInFile3);RAG_FLUSH;
#endif
#ifndef TG_ARCH
  void ReadData(struct ImageParams *image_params, FILE *pFile1, FILE *pFile2, FILE *pFile3,
                struct complexData **X, float **Pt, float *Tp);
#else
  void ReadData(struct ImageParams *image_params, void *pFile1, void *pFile2, void *pFile3,
                struct complexData **X, float **Pt, float *Tp);
#endif

  ReadData(image_params, pInFile, pInFile2, pInFile3, X, Pt, Tp);

#ifdef TG_ARCH
  SPADtoBSM(image_params_ptr,image_params,sizeof(struct ImageParams));
#else
  memcpy(image_params_ptr,image_params,sizeof(struct ImageParams));
#endif

  RAG_DEF_MACRO_SPAD(arg_scg,NULL,NULL,NULL,NULL,X_dbg,4);
  RAG_DEF_MACRO_SPAD(arg_scg,NULL,NULL,NULL,NULL,Pt_dbg,5);
  RAG_DEF_MACRO_SPAD(arg_scg,NULL,NULL,NULL,NULL,Tp_dbg,6);

#ifdef TRACE_LVL_2
  PRINTF("//// leave ReadData_edt\n");RAG_FLUSH;
#endif
  return NULL_GUID;
}

// ReadData

#ifndef TG_ARCH
void ReadData(struct ImageParams *image_params, FILE *pFile1, FILE *pFile2, FILE *pFile3,
              struct complexData **X, float **Pt, float *Tp)
#else
void ReadData(struct ImageParams *image_params, void *pFile1, void *pFile2, void *pFile3,
              struct complexData **X, float **Pt, float *Tp)
#endif
{
#ifdef TRACE_LVL_2
  PRINTF("//// enter ReadData()\n");RAG_FLUSH;
#endif

#ifdef RAG_IMPLICIT_INPUTS
  extern SHARED struct {int32_t real; int32_t imag;} image_0[RAG_P1][RAG_S1];
  extern SHARED struct {int32_t real; int32_t imag;} image_1[RAG_P1][RAG_S1];
  extern SHARED struct {int32_t x; int32_t y; int32_t z;} platform_0[RAG_P1][1];
  extern SHARED struct {int32_t x; int32_t y; int32_t z;} platform_1[RAG_P1][1];
  extern SHARED struct {int32_t t; } pulse_0[RAG_P1][1];
  extern SHARED struct {int32_t t; } pulse_1[RAG_P1][1];
#ifdef TRACE_LVL_2
  PRINTF("//// RAG_IMPLICIT_INPUTS P1 = %d S1 = %d\n",image_params->P1,image_params->S1);RAG_FLUSH;
#endif
#endif

  for(int m=0; m<image_params->P1; m++) {
#ifdef TRACE_LVL_2
    if(!m)PRINTF("//// Read complex SAR data\n");RAG_FLUSH;
#endif
#ifndef RAG_IMPLICIT_INPUTS
    fread(&X[m][0], sizeof(struct complexData), image_params->S1, pFile1);
#else
    if ( image_params->imageNumber == 0 ) {
      // &X[m][0]
      GlobalPtrToDataBlock(X[m],&image_0[m][0], sizeof(struct complexData)*image_params->S1);
    } else if ( image_params->imageNumber == 1 ) {
      // &X[m][0]
      GlobalPtrToDataBlock(X[m],&image_1[m][0], sizeof(struct complexData)*image_params->S1);
    } // endif image
#endif
#ifdef TRACE_LVL_2
    if(!m){PRINTF("//// Read platform positions\n");RAG_FLUSH;}
#endif
#ifndef RAG_IMPLICIT_INPUTS
    fread(&Pt[m][0], sizeof(float), 3, pFile2);
#else
    if ( image_params->imageNumber == 0 ) {
      // &Pt[m][0]
      GlobalPtrToDataBlock(Pt[m],&platform_0[m][0], sizeof(float)*3);
    } else if ( image_params->imageNumber == 1 ) {
      // &Pt[m][0]
      GlobalPtrToDataBlock(Pt[m],&platform_1[m][0], sizeof(float)*3);
    } // endif image
#endif
  } // for m (P1)
#ifdef TRACE_LVL_2
  PRINTF("//// Read pulse transmission timestamps\n");RAG_FLUSH;
#endif
#ifndef RAG_IMPLICIT_INPUTS
  fread(Tp, sizeof(float), image_params->P1, pFile3);
#else
  for(int m=0; m<image_params->P1; m++) {
    if ( image_params->imageNumber == 0 ) {
      // &Tp[m]
      GlobalPtrToDataBlock(Tp+m,&pulse_0[m][0].t, sizeof(float)*1);
    } else if ( image_params->imageNumber == 1 ) {
      // &Tp[m]
      GlobalPtrToDataBlock(Tp+m,&pulse_1[m][0].t, sizeof(float)*1);
    } // endif image
  } // for m (P1)
#endif
#ifdef RAG_IMPLICIT_INPUTS
  image_params->imageNumber++;
#endif
#ifdef TRACE_LVL_2
  PRINTF("//// leave ReadData()\n");RAG_FLUSH;
#endif
}
