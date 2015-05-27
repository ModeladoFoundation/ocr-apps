//------------------------------------------------------------------------------------------------------------------------------
// Samuel Williams
// SWWilliams@lbl.gov
// Lawrence Berkeley National Lab
//------------------------------------------------------------------------------------------------------------------------------
// Based on Yousef Saad's Iterative Methods for Sparse Linear Algebra, Algorithm 12.1, page 399
//------------------------------------------------------------------------------------------------------------------------------

#include "rstream_chebyshev.h"
#ifdef RSTREAM_CHEBY
#include "cheby2/mcheby2.h"
#else
#include "rstream_chebyshev_mappable.c" // Reference
#endif

#ifdef XXXRSTREAM_CHEBY
#define ENABLE_EXTENSION_LEGACY
#include "ocr-legacy.h"
static char * concat(char const * c1, char const * c2) {
    size_t plen;
    char * buf;
    plen = strlen(c1);
    buf = (char *) malloc(sizeof(char)*(plen+strlen(c2)+1));
    strcpy(buf, c1);
    strcpy(buf+plen, c2);
    return buf;
}

ocrConfig_t ocrConfig() {
    char* pPathPtr;
    ocrConfig_t ocrConfig;
    char * buf = NULL;

    pPathPtr = getenv ("OCR_CONFIG");
    // if config file not found, default to 12-thread shared mem configuration
    if (pPathPtr==NULL) {
        char * ocrHomePath = getenv("OCR_HOME");
        const char * rstDftMach = "/runtime/codelet/ocr/ocr-hc/machine-configs/mach-hc-48w.cfg";
        const char * dftMach    = "/machine-configs/mach-hc-48w.cfg";
        size_t plen;
        char * buf;

        if (ocrHomePath==NULL) {
            ocrHomePath = getenv("RSTREAM_HOME");
            if (ocrHomePath==NULL) {
                //printf("Please define $OCR_CONFIG (machine description file) or $OCR_HOME\n");
                exit(1);
            } else {
                pPathPtr = concat(ocrHomePath, rstDftMach);
            }
        } else {
            pPathPtr = concat(ocrHomePath, dftMach);
        }
    }
    //printf("Using machine file  %s \n", pPathPtr);
    ocrConfig.iniFile = pPathPtr;
    ocrConfig.userArgc = 0;
    ocrConfig.userArgv = NULL;
    return ocrConfig;
}
#endif

void smooth(level_type * level, int x_id, int rhs_id, double a, double b){
  if( (level->dominant_eigenvalue_of_DinvA<=0.0) && (level->my_rank==0) )//printf/gc("dominant_eigenvalue_of_DinvA <= 0.0 !\n");
  if((CHEBYSHEV_DEGREE*NUM_SMOOTHS)&1){
    //printf("error... CHEBYSHEV_DEGREE*NUM_SMOOTHS must be even for the chebyshev smoother...\n");
    exit(0);
  }
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  int box,s;
  int ghosts = level->box_ghosts;
  int radius     = STENCIL_RADIUS;
  int communicationAvoiding = ghosts > radius;
#ifdef XXXRSTREAM_CHEBY
  ocrGuid_t ocrLegCtx;

  //printf("OCR CONFIG START\n");
  ocrConfig_t ocrConf = ocrConfig();
  ocrLegacyInit(&ocrLegCtx, &ocrConf);
  //printf("OCR CONFIG FINISH\n");
#endif


  // compute the Chebyshev coefficients...
  double beta     = 1.000*level->dominant_eigenvalue_of_DinvA;
//double alpha    = 0.300000*beta;
//double alpha    = 0.250000*beta;
//double alpha    = 0.166666*beta;
  double alpha    = 0.125000*beta;
  double theta    = 0.5*(beta+alpha);		// center of the spectral ellipse
  double delta    = 0.5*(beta-alpha);		// major axis?
  double sigma = theta/delta;
  double rho_n = 1/sigma;			// rho_0
  double chebyshev_c1[CHEBYSHEV_DEGREE];	// + c1*(x_n-x_nm1) == rho_n*rho_nm1
  double chebyshev_c2[CHEBYSHEV_DEGREE];	// + c2*(b-Ax_n)
  chebyshev_c1[0] = 0.0;
  chebyshev_c2[0] = 1/theta;
  for(s=1;s<CHEBYSHEV_DEGREE;s++){
    double rho_nm1 = rho_n;
    rho_n = 1.0/(2.0*sigma - rho_nm1);
    chebyshev_c1[s] = rho_n*rho_nm1;
    chebyshev_c2[s] = rho_n*2.0/delta;
  }


  // if communication-avoiding, need updated RHS for stencils in ghost zones
  if(communicationAvoiding)exchange_boundary(level,rhs_id,0);

  for(s=0;s<CHEBYSHEV_DEGREE*NUM_SMOOTHS;s+=ghosts){
    //printf("START CHEBY S LOOP %d\n", s);
    // Chebyshev ping pongs between x_id and VECTOR_TEMP
    if((s&1)==0){exchange_boundary(level,       x_id,STENCIL_IS_STAR_SHAPED && !communicationAvoiding);apply_BCs(level,       x_id);}
            else{exchange_boundary(level,VECTOR_TEMP,STENCIL_IS_STAR_SHAPED && !communicationAvoiding);apply_BCs(level,VECTOR_TEMP);}

    // now do ghosts communication-avoiding smooths on each box...
    uint64_t _timeStart = CycleTime();
    //#pragma omp parallel for private(box) OMP_THREAD_ACROSS_BOXES(level->concurrent_boxes)
    for(box=0;box<level->num_my_boxes;box++){
      //printf("START CHEBY BOX LOOP %d\n", box);
      int i,j,k,ss;
      const int jStride = level->my_boxes[box].jStride;
      const int kStride = level->my_boxes[box].kStride;
      const int     dim = level->my_boxes[box].dim;
      const double h2inv = 1.0/(level->h*level->h);
      const double * __restrict__ rhs      = level->my_boxes[box].vectors[       rhs_id] + ghosts*(1+jStride+kStride);
      const double * __restrict__ alpha    = level->my_boxes[box].vectors[VECTOR_ALPHA ] + ghosts*(1+jStride+kStride);
      const double * __restrict__ beta_i   = level->my_boxes[box].vectors[VECTOR_BETA_I] + ghosts*(1+jStride+kStride);
      const double * __restrict__ beta_j   = level->my_boxes[box].vectors[VECTOR_BETA_J] + ghosts*(1+jStride+kStride);
      const double * __restrict__ beta_k   = level->my_boxes[box].vectors[VECTOR_BETA_K] + ghosts*(1+jStride+kStride);
      const double * __restrict__ Dinv     = level->my_boxes[box].vectors[VECTOR_DINV  ] + ghosts*(1+jStride+kStride);
      const double * __restrict__ valid    = level->my_boxes[box].vectors[VECTOR_VALID ] + ghosts*(1+jStride+kStride); // cell is inside the domain


      const double * __restrict__ x_n_0   = level->my_boxes[box].vectors[       x_id] + ghosts*(1+jStride+kStride);
            double * __restrict__ x_np1_0 = level->my_boxes[box].vectors[VECTOR_TEMP] + ghosts*(1+jStride+kStride);
      const double * __restrict__ x_nm1_0 = level->my_boxes[box].vectors[VECTOR_TEMP] + ghosts*(1+jStride+kStride);

      const double * __restrict__ x_n_1   = level->my_boxes[box].vectors[VECTOR_TEMP] + ghosts*(1+jStride+kStride);
            double * __restrict__ x_np1_1 = level->my_boxes[box].vectors[       x_id] + ghosts*(1+jStride+kStride);
      const double * __restrict__ x_nm1_1 = level->my_boxes[box].vectors[       x_id] + ghosts*(1+jStride+kStride);

      int ghostsToOperateOn=ghosts-1;
      //printf("CALL CHEBY MAPPED\n");
      RSTREAM_CHEBY_EXEC(7);
      //printf("FINISH CHEBY BOX LOOP\n");
    } // box-loop

    level->cycles.smooth += (uint64_t)(CycleTime()-_timeStart);
  //printf("FINISH CHEBY S LOOP\n");
  } // s-loop
#ifdef XXXRSTREAM_CHEBY
  ocrShutdown();
  //printf("FINISH OCR SHUTDOWN\n");
  ocrLegacyFinalize(ocrLegCtx, false);
  //printf("FINISH OCR FINALIZE\n");
#endif
}
