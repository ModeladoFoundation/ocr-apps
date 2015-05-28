//------------------------------------------------------------------------------------------------------------------------------
// Copyright Notice
//------------------------------------------------------------------------------------------------------------------------------
// HPGMG, Copyright (c) 2014, The Regents of the University of
// California, through Lawrence Berkeley National Laboratory (subject to
// receipt of any required approvals from the U.S. Dept. of Energy).  All
// rights reserved.
//
// If you have questions about your rights to use or distribute this
// software, please contact Berkeley Lab's Technology Transfer Department
// at  TTD@lbl.gov.
//
// NOTICE.  This software is owned by the U.S. Department of Energy.  As
// such, the U.S. Government has been granted for itself and others
// acting on its behalf a paid-up, nonexclusive, irrevocable, worldwide
// license in the Software to reproduce, prepare derivative works, and
// perform publicly and display publicly.  Beginning five (5) years after
// the date permission to assert copyright is obtained from the U.S.
// Department of Energy, and subject to any subsequent five (5) year
// renewals, the U.S. Government is granted for itself and others acting
// on its behalf a paid-up, nonexclusive, irrevocable, worldwide license
// in the Software to reproduce, prepare derivative works, distribute
// copies to the public, perform publicly and display publicly, and to
// permit others to do so.
//------------------------------------------------------------------------------------------------------------------------------
// Samuel Williams
// SWWilliams@lbl.gov
// Lawrence Berkeley National Lab
//------------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
//------------------------------------------------------------------------------------------------------------------------------
#include <omp.h>
#ifdef USE_MPI
#include <mpi.h>
#endif
//------------------------------------------------------------------------------------------------------------------------------
#include "defines.h"
#include "level.h"
#include "mg.h"
#include "operators.h"
#include "solvers.h"
//------------------------------------------------------------------------------------------------------------------------------
#if (defined(RSTREAM_CHEBY) || defined(GSRB_R_STREAM))
#include <ocr.h>
#define ENABLE_EXTENSION_LEGACY
#include <ocr-legacy.h>
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
    // Bail if config file not found
    if (pPathPtr==NULL) {
	fprintf(stderr, "Please set and export the OCR_CONFIG environment variable to the path of a valid OCR configuration file.");
	exit(1);
    }
    printf("Using machine file  %s \n", pPathPtr);
    ocrConfig.iniFile = pPathPtr;
    ocrConfig.userArgc = 0;
    ocrConfig.userArgv = NULL;
    return ocrConfig;
}
#endif

int main(int argc, char **argv){
  int MPI_Rank=0;
  int MPI_Tasks=1;
  int OMP_Threads = 1;
  int OMP_Nested = 0;

  #pragma omp parallel
  {
    #pragma omp master
    {
      OMP_Threads = omp_get_num_threads();
      OMP_Nested  = omp_get_nested();
    }
  }
  //omp_set_nested(1);


  #ifdef USE_MPI
//#warning Compiling for MPI...
  //FIX... replace with ifdefs or env variables...
  int MPI_threadingModel          = -1;
//int MPI_threadingModelRequested = MPI_THREAD_SINGLE;
//int MPI_threadingModelRequested = MPI_THREAD_SERIALIZED;
  int MPI_threadingModelRequested = MPI_THREAD_FUNNELED;
//int MPI_threadingModelRequested = MPI_THREAD_MULTIPLE;
  MPI_Init_thread(&argc, &argv, MPI_threadingModelRequested, &MPI_threadingModel);
  MPI_Comm_size(MPI_COMM_WORLD, &MPI_Tasks);
  MPI_Comm_rank(MPI_COMM_WORLD, &MPI_Rank);
  #ifdef USE_HPM // IBM HPM counters for BGQ...
  HPM_Init();
  #endif

  if(MPI_threadingModel>MPI_threadingModelRequested)MPI_threadingModel=MPI_threadingModelRequested;
  if(MPI_Rank==0){
       if(MPI_threadingModelRequested == MPI_THREAD_MULTIPLE  )printf("Requested MPI_THREAD_MULTIPLE, ");
  else if(MPI_threadingModelRequested == MPI_THREAD_SINGLE    )printf("Requested MPI_THREAD_SINGLE, ");
  else if(MPI_threadingModelRequested == MPI_THREAD_FUNNELED  )printf("Requested MPI_THREAD_FUNNELED, ");
  else if(MPI_threadingModelRequested == MPI_THREAD_SERIALIZED)printf("Requested MPI_THREAD_SERIALIZED, ");
  else if(MPI_threadingModelRequested == MPI_THREAD_MULTIPLE  )printf("Requested MPI_THREAD_MULTIPLE, ");
  else                                                         printf("Requested Unknown MPI Threading Model (%d), ",MPI_threadingModelRequested);
       if(MPI_threadingModel == MPI_THREAD_MULTIPLE  )printf("got MPI_THREAD_MULTIPLE\n");
  else if(MPI_threadingModel == MPI_THREAD_SINGLE    )printf("got MPI_THREAD_SINGLE\n");
  else if(MPI_threadingModel == MPI_THREAD_FUNNELED  )printf("got MPI_THREAD_FUNNELED\n");
  else if(MPI_threadingModel == MPI_THREAD_SERIALIZED)printf("got MPI_THREAD_SERIALIZED\n");
  else if(MPI_threadingModel == MPI_THREAD_MULTIPLE  )printf("got MPI_THREAD_MULTIPLE\n");
  else                                                printf("got Unknown MPI Threading Model (%d)\n",MPI_threadingModel);
  fflush(stdout);  }
  #endif


  int log2_box_dim = 6;
  int target_boxes_per_rank = 1;

  if(argc==3){
           log2_box_dim=atoi(argv[1]);
     target_boxes_per_rank=atoi(argv[2]);
  }else{
    if(MPI_Rank==0){printf("usage: ./a.out  [log2_box_dim]  [target_boxes_per_rank]\n");}
    #ifdef USE_MPI
    MPI_Finalize();
    #endif
    exit(0);
  }

  if(log2_box_dim<4){
    if(MPI_Rank==0){printf("log2_box_dim must be at least 4\n");}
    #ifdef USE_MPI
    MPI_Finalize();
    #endif
    exit(0);
  }

  if(target_boxes_per_rank<1){
    if(MPI_Rank==0){printf("target_boxes_per_rank must be at least 1\n");}
    #ifdef USE_MPI
    MPI_Finalize();
    #endif
    exit(0);
  }

  if(MPI_Rank==0)if(OMP_Nested)printf("%d MPI Tasks of %d threads (OMP_NESTED=TRUE)\n\n" ,MPI_Tasks,OMP_Threads);
                          else printf("%d MPI Tasks of %d threads (OMP_NESTED=FALSE)\n\n",MPI_Tasks,OMP_Threads);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // calculate the problem size...
  int box_dim=1<<log2_box_dim;
  int target_boxes = target_boxes_per_rank*MPI_Tasks;
  int boxes_in_i = 1000; // FIX, int64_t?  could we really have >2e9 boxes?
  int total_boxes = boxes_in_i*boxes_in_i*boxes_in_i;
  while(total_boxes>target_boxes){
    boxes_in_i--;
    total_boxes = boxes_in_i*boxes_in_i*boxes_in_i;
  }

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // create the fine level...
  int ghosts=1;
  level_type fine_grid;
  //create_level(&fine_grid,boxes_in_i,box_dim,ghosts,VECTORS_RESERVED,BC_PERIODIC ,MPI_Rank,MPI_Tasks);double h0=1.0/( (double)boxes_in_i*(double)box_dim );double a=2.0;double b=1.0; // Helmholtz w/Periodic
  //create_level(&fine_grid,boxes_in_i,box_dim,ghosts,VECTORS_RESERVED,BC_PERIODIC ,MPI_Rank,MPI_Tasks);double h0=1.0/( (double)boxes_in_i*(double)box_dim );double a=0.0;double b=1.0; //   Poisson w/Periodic
  //create_level(&fine_grid,boxes_in_i,box_dim,ghosts,VECTORS_RESERVED,BC_DIRICHLET,MPI_Rank,MPI_Tasks);double h0=1.0/( (double)boxes_in_i*(double)box_dim );double a=2.0;double b=1.0; // Helmholtz w/Dirichlet
    create_level(&fine_grid,boxes_in_i,box_dim,ghosts,VECTORS_RESERVED,BC_DIRICHLET,MPI_Rank,MPI_Tasks);double h0=1.0/( (double)boxes_in_i*(double)box_dim );double a=0.0;double b=1.0; //   Poisson w/Dirichlet
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  initialize_problem(&fine_grid,h0,a,b);
  rebuild_operator(&fine_grid,NULL,a,b); // i.e. calculate Dinv and lambda_max
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  mg_type all_grids;
  int minCoarseDim = 1;
  MGBuild(&all_grids,&fine_grid,a,b,minCoarseDim);
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  int doTiming;for(doTiming=0;doTiming<=1;doTiming++){ // first pass warms up, second times
  MGResetTimers(&all_grids);
  #ifdef USE_HPM // IBM performance counters for BGQ...
  if(doTiming)HPM_Start("FMGSolve()");
  #endif
  #ifdef USE_FCYCLES
  int trial;for(trial=0;trial<1;trial++){
        #ifdef RSTREAM_CHEBY
        ocrGuid_t ocrLegCtx;

        //printf("OCR CONFIG START\n");
        ocrConfig_t ocrConf = ocrConfig();
	ocrLegacyInit(&ocrLegCtx, &ocrConf);
        //printf("OCR CONFIG FINISH\n");
        #endif

        zero_vector(all_grids.levels[0],VECTOR_U);
        FMGSolve(&all_grids,VECTOR_U,VECTOR_F,a,b,1e-15);

        #ifdef RSTREAM_CHEBY
        ocrShutdown();
        //printf("FINISH OCR SHUTDOWN\n");
        ocrLegacyFinalize(ocrLegCtx, false);
        //printf("FINISH OCR FINALIZE\n");
        #endif
  }
  #else
  int trial;for(trial=0;trial< 1;trial++){
        #ifdef GSRB_R_STREAM
        ocrGuid_t ocrLegCtx;

        //printf("OCR CONFIG START\n");
        ocrConfig_t ocrConf = ocrConfig();
	ocrLegacyInit(&ocrLegCtx, &ocrConf);
        //printf("OCR CONFIG FINISH\n");
        #endif
        zero_vector(all_grids.levels[0],VECTOR_U);
        MGSolve(&all_grids,VECTOR_U,VECTOR_F,a,b,1e-15);
        #ifdef GSRB_R_STREAM
        ocrShutdown();
        //printf("FINISH OCR SHUTDOWN\n");
        ocrLegacyFinalize(ocrLegCtx, false);
        //printf("FINISH OCR FINALIZE\n");
        #endif
  }
  #endif
  #ifdef USE_HPM // IBM performance counters for BGQ...
  if(doTiming)HPM_Stop("FMGSolve()");
  #endif
  }
  MGPrintTiming(&all_grids); // don't include the error check in the timing results
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  if(MPI_Rank==0){printf("calculating error...\n");}
  double fine_error = error(&fine_grid,VECTOR_U,VECTOR_UTRUE);if(MPI_Rank==0){printf(" h = %22.15e  ||error|| = %22.15e\n\n",h0,fine_error);fflush(stdout);}
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // MGDestroy()
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  #ifdef USE_MPI
  #ifdef USE_HPM // IBM performance counters for BGQ...
  HPM_Print();
  #endif
  MPI_Finalize();
  #endif
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  return(0);
}
