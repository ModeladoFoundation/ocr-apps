// ************************************************************************
//
// miniAMR: stencil computations with boundary exchange and AMR.
//
// Copyright (2014) Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
// retains certain rights in this software.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// Questions? Contact Courtenay T. Vaughan (ctvaugh@sandia.gov)
//                    Richard F. Barrett (rfbarre@sandia.gov)
//
// ************************************************************************

#define ENABLE_EXTENSION_LABELING

#include "ocr.h"
#include "extensions/ocr-labeling.h"
#include "extensions/ocr-affinity.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "block.h"
#include "proto.h"

ocrGuid_t mainEdt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;

    void * PTR_cmdLineArgs = depv[0].ptr;
    u32 argc = getArgc( PTR_cmdLineArgs );

    //Pack the PTR_cmdLineArgs into the "cannonical" char** argv
    ocrGuid_t DBK_argv;
    char** argv;
    ocrDbCreate( &DBK_argv, (void**)&argv, sizeof(char*)*argc, DB_PROP_NONE, NULL_HINT, NO_ALLOC );
    for( u32 a = 0; a < argc; ++a )
    {
       argv[a] = getArgv( PTR_cmdLineArgs, a );
       DEBUG_PRINTF(("argv[%d] %s\n", a, argv[a]));
    }

    Command cmd = parseCommandLine(argc, argv);

    ocrDbDestroy(DBK_argv);

    u64 nRanks = (cmd.npx)*(cmd.npy)*(cmd.npz);
    DEBUG_PRINTF(("nRanks %d\n", nRanks ));

    //Create OCR objects to pass down to the child/SPMD EDTs
    //for collective operatations among the child/SPMD EDTs
    globalOcrParamH_t ocrParamH;
    initGlobalOcrParamH( &ocrParamH, nRanks, &cmd );

    ocrGuid_t wrapUpTML, wrapUpEDT;
    ocrEdtTemplateCreate( &wrapUpTML, wrapUpEdt, 0, 1 );
    ocrEdtCreate( &wrapUpEDT, wrapUpTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_HINT, NULL );
    ocrEdtTemplateDestroy( wrapUpTML );

    ocrAddDependence(ocrParamH.EVT_OUT_spmdJoin_reduction, wrapUpEDT, 0, DB_MODE_RO);

    //A datablock to store the commandline and the OCR objectes created above
    ocrGuid_t DBK_globalParamH;
    globalParamH_t* PTR_globalParamH;
    ocrDbCreate( &DBK_globalParamH, (void**) &PTR_globalParamH, sizeof(globalParamH_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    memcpy(&(PTR_globalParamH->cmdParamH), &cmd, sizeof(Command) );
    memcpy(&(PTR_globalParamH->ocrParamH), &ocrParamH, sizeof(globalOcrParamH_t) );

    //Release the changes so they are visible for any dependent EDTs below
    ocrDbRelease(DBK_globalParamH);

    u64 edtGridDims[3] = { cmd.npx, cmd.npy, cmd.npz };

    //All SPMD EDTs depend on the following dependencies
    ocrGuid_t spmdDepv[2] = {DBK_cmdLineArgs, DBK_globalParamH};

    //3-D Cartesian grid of SPMD EDTs get mapped to a 3-D Cartesian grid of PDs
#ifdef USE_STATIC_SCHEDULER
    PRINTF("Using STATIC scheduler\n");
    forkSpmdEdts_staticScheduler_Cart3D( initEdt, edtGridDims, spmdDepv );
#else
    PRINTF("NOT Using STATIC scheduler\n");
    forkSpmdEdts_Cart3D( initEdt, edtGridDims, spmdDepv );
#endif

    return NULL_GUID;
}

ocrGuid_t wrapUpEdt( u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[] )
{
    PRINTF("Shutting down\n");

#ifdef SHUTDOWN_LAG
    sleep(SHUTDOWN_LAG); //Added this in caes there are any pending EDTs.
#endif

    ocrShutdown();
    PRINTF("Done\n");
    return NULL_GUID;
}

void initGlobalOcrParamH( globalOcrParamH_t* ocrParamH, u64 nRanks, Command* cmd )
{
    ocrEventCreate( &(ocrParamH->EVT_OUT_spmdJoin_reduction), OCR_EVENT_IDEM_T, EVT_PROP_TAKES_ARG );

    int r;
    for( r=0; r<MAX_REDUCTION_HANDLES; r++ ) {
    ocrGuidRangeCreate(&(ocrParamH->redUpRangeGUID[r]), nRanks, GUID_USER_EVENT_STICKY);
    }

    //TODO: Either have 6 nbrs or 27 nbrs depending on the halo-exchange implementation
    int coarseBlocksPerRank =  cmd->init_block_x*cmd->init_block_y*cmd->init_block_z;
    int p8 = 1;
    int i;

    s64 totalBlocks = 0;

    for(i = 0; i <= cmd->num_refine; i++)
    {
         totalBlocks += nRanks*coarseBlocksPerRank*p8;
         ocrGuidRangeCreate(&(ocrParamH->haloRangeGUID[i]), 6*(nRanks*coarseBlocksPerRank)*p8, GUID_USER_EVENT_STICKY);
         p8 *= 8;
    }

    DEBUG_PRINTF(("INIT PARTICIPANTS 1 %d\n", totalBlocks ));
    ocrGuidRangeCreate(&(ocrParamH->initRedRangeGUID),totalBlocks, GUID_USER_EVENT_STICKY);

}

void initCommand(Command* cmd)
{
#include "param.h"

    cmd->max_num_blocks     = MAX_NUM_BLOCKS;
    cmd->target_active      = TARGET_ACTIVE;
    cmd->num_refine         = NUM_REFINE;
    cmd->uniform_refine     = UNIFORM_REFINE;
    cmd->x_block_size       = X_BLOCK_SIZE;
    cmd->y_block_size       = Y_BLOCK_SIZE;
    cmd->z_block_size       = Z_BLOCK_SIZE;
    cmd->num_vars           = NUM_VARS;
    cmd->comm_vars          = COMM_VARS;
    cmd->init_block_x       = INIT_BLOCK_X;
    cmd->init_block_y       = INIT_BLOCK_Y;
    cmd->init_block_z       = INIT_BLOCK_Z;
    cmd->reorder            = REORDER;
    cmd->npx                = NPX;
    cmd->npy                = NPY;
    cmd->npz                = NPZ;
    cmd->inbalance          = INBALANCE;
    cmd->refine_freq        = REFINE_FREQ;
    cmd->report_diffusion   = REPORT_DIFFUSION;
    cmd->error_tol          = ERROR_TOL;
    cmd->num_tsteps         = NUM_TSTEPS;
    cmd->stencil            = STENCIL;
    cmd->report_perf        = REPORT_PERF;
    cmd->plot_freq          = PLOT_FREQ;
    cmd->num_objects        = NUM_OBJECTS;
    cmd->checksum_freq      = CHECKSUM_FREQ;
    cmd->target_max         = TARGET_MAX;
    cmd->target_min         = TARGET_MIN;
    cmd->stages_per_ts      = STAGES_PER_TS;
    cmd->lb_opt             = LB_OPT;
    cmd->block_change       = BLOCK_CHANGE;
    cmd->code               = CODE;
    cmd->permute            = PERMUTE;
    cmd->nonblocking        = NONBLOCKING;
    cmd->refine_ghost       = REFINE_GHOST;
}

Command parseCommandLine(int argc, char** argv)
{
   Command cmd;

   int i, object_num;

   initCommand( &cmd );

   /* set initial values */
   for (i = 1; i < argc; i++)
   {
      if (!strcmp(argv[i], "--max_blocks"))
         cmd.max_num_blocks = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--target_active"))
         cmd.target_active = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--target_max"))
         cmd.target_max = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--target_min"))
         cmd.target_min = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--num_refine"))
         cmd.num_refine = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--block_change"))
         cmd.block_change = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--uniform_refine"))
         cmd.uniform_refine = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--nx"))
         cmd.x_block_size = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--ny"))
         cmd.y_block_size = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--nz"))
         cmd.z_block_size = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--num_vars"))
         cmd.num_vars = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--comm_vars"))
         cmd.comm_vars = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--init_x"))
         cmd.init_block_x = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--init_y"))
         cmd.init_block_y = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--init_z"))
         cmd.init_block_z = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--reorder"))
         cmd.reorder = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--npx"))
         cmd.npx = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--npy"))
         cmd.npy = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--npz"))
         cmd.npz = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--inbalance"))
         cmd.inbalance = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--lb_opt"))
         cmd.lb_opt = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--refine_freq"))
         cmd.refine_freq = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--report_diffusion"))
         cmd.report_diffusion = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--error_tol"))
         cmd.error_tol = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--num_tsteps"))
         cmd.num_tsteps = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--stages_per_ts"))
         cmd.stages_per_ts = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--checksum_freq"))
         cmd.checksum_freq = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--stencil"))
         cmd.stencil = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--permute"))
         cmd.permute = 1;
      else if (!strcmp(argv[i], "--report_perf"))
         cmd.report_perf = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--plot_freq"))
         cmd.plot_freq = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--code"))
         cmd.code = atoi(argv[++i]);
      else if (!strcmp(argv[i], "--blocking_send"))
         cmd.nonblocking = 0;
      else if (!strcmp(argv[i], "--refine_ghost"))
         cmd.refine_ghost = 1;
      else if (!strcmp(argv[i], "--num_objects"))
      {
         cmd.num_objects = atoi(argv[++i]);
         object_num = 0;
      }
      else if (!strcmp(argv[i], "--object"))
      {
         if (object_num >= cmd.num_objects)
         {
            PRINTF("object number greater than num_objects\n");
            exit(-1);
         }
         cmd.objects[object_num].type = atoi(argv[++i]);
         cmd.objects[object_num].bounce = atoi(argv[++i]);
         cmd.objects[object_num].cen[0] = atof(argv[++i]);
         cmd.objects[object_num].cen[1] = atof(argv[++i]);
         cmd.objects[object_num].cen[2] = atof(argv[++i]);
         cmd.objects[object_num].move[0] = atof(argv[++i]);
         cmd.objects[object_num].move[1] = atof(argv[++i]);
         cmd.objects[object_num].move[2] = atof(argv[++i]);
         cmd.objects[object_num].size[0] = atof(argv[++i]);
         cmd.objects[object_num].size[1] = atof(argv[++i]);
         cmd.objects[object_num].size[2] = atof(argv[++i]);
         cmd.objects[object_num].inc[0] = atof(argv[++i]);
         cmd.objects[object_num].inc[1] = atof(argv[++i]);
         cmd.objects[object_num].inc[2] = atof(argv[++i]);
         object_num++;
      }
      else if (!strcmp(argv[i], "--help"))
      {
         print_help_message();
         ocrShutdown();
      }
      else
      {
         PRINTF("** Error ** Unknown input parameter %s\n", argv[i]);
         print_help_message();
         ocrShutdown();
      }
   }

   if (check_input(cmd))
      exit(-1);

   if (cmd.comm_vars == 0 || cmd.comm_vars > cmd.num_vars) {
      cmd.comm_vars = cmd.num_vars;
   }

   if (!cmd.block_change)
      cmd.block_change = cmd.num_refine;

   for (object_num = 0; object_num < cmd.num_objects; object_num++)
      for (i = 0; i < 3; i++)
      {
         cmd.objects[object_num].orig_cen[i] = cmd.objects[object_num].cen[i];
         cmd.objects[object_num].orig_move[i] = cmd.objects[object_num].move[i];
         cmd.objects[object_num].orig_size[i] = cmd.objects[object_num].size[i];
      }

   return cmd;
}

_OCR_TASK_FNC_( FNC_driver )
{
    s32 _idep, _paramc, _depc;
    u64 id = paramv[0];

    ocrGuid_t DBK_rankH = depv[0].guid;

    rankH_t* PTR_rankH = depv[0].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    int ilevel = PTR_rankH->ilevel;
    DEBUG_PRINTF(( "%s ilevel %d id_l %d\n", __func__, ilevel, PTR_rankH->myRank ));

    int irefine = 0;
    int ts = 0;

    int r = BLOCKCOUNT_RED_HANDLE_LB + (irefine%2); //reserved for block counts
    redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];
    ocrDBK_t DBK_in = PTR_redObjects->DBK_in;

    ocrTML_t TML_refineLoop = PTR_rankTemplateH->TML_refineLoop;
    ocrTML_t TML_timestepLoop = PTR_rankTemplateH->TML_timestepLoop;

    MyOcrTaskStruct_t TS_refineLoop; _paramc = 1; _depc = 1;

    //At ts=0; All rankH handles are active blocks
    //Create a refine task --> the current block may become inactive due to refinement or coarsening

    PTR_rankH->tBegin = timer();

    int num_refine = PTR_cmd->num_refine;
    int uniform_refine = PTR_cmd->uniform_refine;

    ocrDbRelease(DBK_rankH);

    if( num_refine || (uniform_refine && num_refine) ) {
        refineLoopPRM_t refineLoopPRM = {irefine, ts};
        ocrEdtCreate( &TS_refineLoop.EDT, TML_refineLoop, //FNC_refineLoop
                      EDT_PARAM_DEF, (u64*)&refineLoopPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, TS_refineLoop.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_in, TS_refineLoop.EDT, _idep++, DB_MODE_RW );
        ocrAddDependence( NULL_GUID, TS_refineLoop.EDT, _idep++, DB_MODE_NULL );

        //the timestepLoop task is launched by the FNC_refineLoop since there may be
        //new active blocks or the current block may have coarsened
    }
    else {
        ts += 1; //First timestep
        timestepLoopPRM_t timestepLoopPRM = {ts};
        ocrGuid_t timestepLoopEDT;
        ocrEdtCreate( &timestepLoopEDT, TML_timestepLoop,
                      EDT_PARAM_DEF, (u64*)&timestepLoopPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, NULL ); //FNC_timestepLoop
        _idep = 0;
        ocrAddDependence( DBK_rankH, timestepLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( NULL_GUID, timestepLoopEDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_miniamrMain )
{
    DEBUG_PRINTF(( "%s\n", __func__ ));
#if 1
    s32 _idep, _paramc, _depc;

    u64 id = paramv[0];

    ocrGuid_t DBK_rankH = depv[0].guid;

    rankH_t* PTR_rankH = depv[0].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    MyOcrTaskStruct_t TS_driver; _paramc = 1; _depc = 1;

    TS_driver.FNC = FNC_driver;
    ocrEdtTemplateCreate( &TS_driver.TML, TS_driver.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_driver.EDT, TS_driver.TML,
                  EDT_PARAM_DEF, &id, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

    ocrEdtTemplateDestroy( TS_driver.TML );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_driver.EDT, _idep++, DB_MODE_RW );
#endif

    return NULL_GUID;
}

typedef struct
{
    u64 id;
    int nNbrs;

} PRM_channelSetupEdt_t;

ocrGuid_t channelSetupEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    s32 _idep, _paramc, _depc;

    PRM_channelSetupEdt_t* PRM_channelSetupEdt = (PRM_channelSetupEdt_t*) paramv;

    u64 id = PRM_channelSetupEdt->id;
    int nNbrs = PRM_channelSetupEdt->nNbrs;

    ocrGuid_t DBK_rankH = depv[6*nNbrs+9+8].guid;

    rankH_t* PTR_rankH = depv[6*nNbrs+9+8].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    u64 nbr, i, j, k, r, ichild, isibling;
    int nbrImage;

    u64 gz = PTR_rankH->gz; //global blocks in z at this level
    u64 gy = PTR_rankH->gy;
    u64 gx = PTR_rankH->gx;

    u64 iblock_g = PTR_rankH->iblock_g;
    u64 jblock_g = PTR_rankH->jblock_g;
    u64 kblock_g = PTR_rankH->kblock_g;

    int ilevel = PTR_rankH->ilevel;
    DEBUG_PRINTF(( "%s ilevel %d id_l %d\n", __func__, ilevel, PTR_rankH->myRank ));

    int num_refine = PTR_cmd->num_refine;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    //Capture all the events needed for inter-rank depedencies
    for( nbr = 0; nbr < nNbrs; nbr++) {
        sharedOcrObj_t* PTR_sharedOcrObjH_nbr = depv[nbr].ptr; //peer
        sharedOcrObj_t* PTR_sharedOcrObjH_nbr_c = depv[nNbrs+nbr].ptr; //coarser neighbor
        sharedOcrObj_t* PTR_sharedOcrObjH_nbr_r0 = depv[2*nNbrs+4*nbr+0].ptr; //finer neighbor 1
        sharedOcrObj_t* PTR_sharedOcrObjH_nbr_r1 = depv[2*nNbrs+4*nbr+1].ptr;
        sharedOcrObj_t* PTR_sharedOcrObjH_nbr_r2 = depv[2*nNbrs+4*nbr+2].ptr;
        sharedOcrObj_t* PTR_sharedOcrObjH_nbr_r3 = depv[2*nNbrs+4*nbr+3].ptr;

        doubleBufferedOcrObj_t* PTR_dBufH1_nbr = &(PTR_sharedOcrObjH_nbr->doubleBufferedOcrObjH[0]);
        doubleBufferedOcrObj_t* PTR_dBufH1_nbr_c = &(PTR_sharedOcrObjH_nbr_c->doubleBufferedOcrObjH[0]);
        doubleBufferedOcrObj_t* PTR_dBufH1_nbr_r0 = &(PTR_sharedOcrObjH_nbr_r0->doubleBufferedOcrObjH[0]);
        doubleBufferedOcrObj_t* PTR_dBufH1_nbr_r1 = &(PTR_sharedOcrObjH_nbr_r1->doubleBufferedOcrObjH[0]);
        doubleBufferedOcrObj_t* PTR_dBufH1_nbr_r2 = &(PTR_sharedOcrObjH_nbr_r2->doubleBufferedOcrObjH[0]);
        doubleBufferedOcrObj_t* PTR_dBufH1_nbr_r3 = &(PTR_sharedOcrObjH_nbr_r3->doubleBufferedOcrObjH[0]);

        doubleBufferedOcrObj_t* PTR_dBufH2_nbr = &(PTR_sharedOcrObjH_nbr->doubleBufferedOcrObjH[1]);
        doubleBufferedOcrObj_t* PTR_dBufH2_nbr_c = &(PTR_sharedOcrObjH_nbr_c->doubleBufferedOcrObjH[1]);
        doubleBufferedOcrObj_t* PTR_dBufH2_nbr_r0 = &(PTR_sharedOcrObjH_nbr_r0->doubleBufferedOcrObjH[1]);
        doubleBufferedOcrObj_t* PTR_dBufH2_nbr_r1 = &(PTR_sharedOcrObjH_nbr_r1->doubleBufferedOcrObjH[1]);
        doubleBufferedOcrObj_t* PTR_dBufH2_nbr_r2 = &(PTR_sharedOcrObjH_nbr_r2->doubleBufferedOcrObjH[1]);
        doubleBufferedOcrObj_t* PTR_dBufH2_nbr_r3 = &(PTR_sharedOcrObjH_nbr_r3->doubleBufferedOcrObjH[1]);

        int i_r = iblock_g%2;
        int j_r = jblock_g%2;
        int k_r = kblock_g%2;

        switch(nbr) {
            case 0:
                nbrImage = 1;           // this is the tag the same nbr uses to me
                i_r = 0; j = k_r*2 + j_r;
                break;
            case 1:
                nbrImage = 0;
                i_r = 0; j = k_r*2 + j_r;
                break;
            case 2:
                nbrImage = 3;
                j_r = 0; j = k_r*2 + i_r;
                break;
            case 3:
                nbrImage = 2;
                j_r = 0; j = k_r*2 + i_r;
                break;
            case 4:
                nbrImage = 5;
                k_r = 0; j = j_r*2 + i_r;
                break;
            case 5:
                nbrImage = 4;
                k_r = 0; j = j_r*2 + i_r;
                break;
        }

        PTR_sharedOcrObjH->currRankDBKs[nbr] = PTR_sharedOcrObjH_nbr->DBK_rankH;
        PTR_sharedOcrObjH->coarRankDBKs[nbr] = (ilevel!=0) ? PTR_sharedOcrObjH_nbr_c->DBK_rankH : NULL_GUID;
        PTR_sharedOcrObjH->refnRankDBKs[nbr][0] = (ilevel!=num_refine) ? PTR_sharedOcrObjH_nbr_r0->DBK_rankH : NULL_GUID;
        PTR_sharedOcrObjH->refnRankDBKs[nbr][1] = (ilevel!=num_refine) ? PTR_sharedOcrObjH_nbr_r1->DBK_rankH : NULL_GUID;
        PTR_sharedOcrObjH->refnRankDBKs[nbr][2] = (ilevel!=num_refine) ? PTR_sharedOcrObjH_nbr_r2->DBK_rankH : NULL_GUID;
        PTR_sharedOcrObjH->refnRankDBKs[nbr][3] = (ilevel!=num_refine) ? PTR_sharedOcrObjH_nbr_r3->DBK_rankH : NULL_GUID;

#ifdef CHANNEL_EVENTS_AT_RECEIVER
        PTR_dBufH1->haloCurrSendEVTs[nbr] = PTR_dBufH1_nbr->haloCurrRecvEVTs[nbrImage]; //peer
        PTR_dBufH1->haloCoarSendEVTs[nbr] = (ilevel!=0) ? PTR_dBufH1_nbr_c->haloRefnRecvEVTs[nbrImage][j] : NULL_GUID; //coarser neighbor's recv event for refined neighbor
        PTR_dBufH1->haloRefnSendEVTs[nbr][0] = (ilevel!=num_refine) ? PTR_dBufH1_nbr_r0->haloCoarRecvEVTs[nbrImage] : NULL_GUID; //refined neighbor's recv event for coarser neighbor
        PTR_dBufH1->haloRefnSendEVTs[nbr][1] = (ilevel!=num_refine) ? PTR_dBufH1_nbr_r1->haloCoarRecvEVTs[nbrImage] : NULL_GUID; //refined neighbor's recv event for coarser neighbor
        PTR_dBufH1->haloRefnSendEVTs[nbr][2] = (ilevel!=num_refine) ? PTR_dBufH1_nbr_r2->haloCoarRecvEVTs[nbrImage] : NULL_GUID; //refined neighbor's recv event for coarser neighbor
        PTR_dBufH1->haloRefnSendEVTs[nbr][3] = (ilevel!=num_refine) ? PTR_dBufH1_nbr_r3->haloCoarRecvEVTs[nbrImage] : NULL_GUID; //refined neighbor's recv event for coarser neighbor

        DEBUG_PRINTF(("Send: ilevel %d id_g %d id_l %d nbr %d rankH "GUIDF" peer "GUIDF" c "GUIDF" ref "GUIDF" "GUIDF" "GUIDF" "GUIDF" \n", ilevel, PTR_rankH->myRank_g, PTR_rankH->myRank, nbr, PTR_sharedOcrObjH->currRankDBKs[nbr], PTR_dBufH1->haloCurrSendEVTs[nbr], PTR_dBufH1->haloCoarSendEVTs[nbr], PTR_dBufH1->haloRefnSendEVTs[nbr][0], PTR_dBufH1->haloRefnSendEVTs[nbr][1], PTR_dBufH1->haloRefnSendEVTs[nbr][2], PTR_dBufH1->haloRefnSendEVTs[nbr][3]));
#else
        PTR_dBufH1->haloCurrRecvEVTs[nbr] = PTR_dBufH1_nbr->haloCurrSendEVTs[nbrImage]; //peer
        PTR_dBufH1->haloCoarRecvEVTs[nbr] = (ilevel!=0) ? PTR_dBufH1_nbr_c->haloRefnSendEVTs[nbrImage][j] : NULL_GUID; //coarser neighbor's send event for refined neighbor
        PTR_dBufH1->haloRefnRecvEVTs[nbr][0] = (ilevel!=num_refine) ? PTR_dBufH1_nbr_r0->haloCoarSendEVTs[nbrImage] : NULL_GUID; //refined neighbor's send event for coarser neighbor
        PTR_dBufH1->haloRefnRecvEVTs[nbr][1] = (ilevel!=num_refine) ? PTR_dBufH1_nbr_r1->haloCoarSendEVTs[nbrImage] : NULL_GUID; //refined neighbor's send event for coarser neighbor
        PTR_dBufH1->haloRefnRecvEVTs[nbr][2] = (ilevel!=num_refine) ? PTR_dBufH1_nbr_r2->haloCoarSendEVTs[nbrImage] : NULL_GUID; //refined neighbor's send event for coarser neighbor
        PTR_dBufH1->haloRefnRecvEVTs[nbr][3] = (ilevel!=num_refine) ? PTR_dBufH1_nbr_r3->haloCoarSendEVTs[nbrImage] : NULL_GUID; //refined neighbor's send event for coarser neighbor

        DEBUG_PRINTF(("Recv: ilevel %d id_g %d id_l %d nbr %d rankH "GUIDF" peer "GUIDF" c "GUIDF" ref "GUIDF" "GUIDF" "GUIDF" "GUIDF" \n", ilevel, PTR_rankH->myRank_g, PTR_rankH->myRank, nbr, PTR_sharedOcrObjH->currRankDBKs[nbr], PTR_dBufH1->haloCurrRecvEVTs[nbr], PTR_dBufH1->haloCoarRecvEVTs[nbr], PTR_dBufH1->haloRefnRecvEVTs[nbr][0], PTR_dBufH1->haloRefnRecvEVTs[nbr][1], PTR_dBufH1->haloRefnRecvEVTs[nbr][2], PTR_dBufH1->haloRefnRecvEVTs[nbr][3]));
#endif

    }

    sharedOcrObj_t* PTR_sharedOcrObjH_parent = depv[6*nNbrs].ptr;
    PTR_sharedOcrObjH->parentRankDBK = (ilevel != 0) ? PTR_sharedOcrObjH_parent->DBK_rankH : NULL_GUID;

     for( r=0; r<MAX_REDUCTION_HANDLES; r++ ) {
         if(ilevel!=0) memcpy( &PTR_sharedOcrObjH->parentRedObjects[r], &PTR_sharedOcrObjH_parent->blockRedObjects[r], sizeof(redObjects_t) );
         else memset(&PTR_sharedOcrObjH->parentRedObjects[r], 0, sizeof(redObjects_t));
      }

    PTR_sharedOcrObjH->siblingintentionRedRangeGUID = (ilevel != 0) ? PTR_sharedOcrObjH_parent->childrenintentionRedRangeGUID : NULL_GUID;
    PTR_sharedOcrObjH->siblingchkSumRedRangeGUID = (ilevel != 0) ? PTR_sharedOcrObjH_parent->childrenchkSumRedRangeGUID : NULL_GUID;
    PTR_sharedOcrObjH->siblingredRangeGUID = (ilevel != 0) ? PTR_sharedOcrObjH_parent->childrenredRangeGUID : NULL_GUID;

    //eight children of the oct-tree
    for( k=0; k<2; k++)
    for( j=0; j<2; j++)
    for( i=0; i<2; i++)
    {
        ichild = i+2*j+4*k;

        sharedOcrObj_t* PTR_sharedOcrObjH_child = depv[6*nNbrs+1+ichild].ptr;
        doubleBufferedOcrObj_t* PTR_dBufH1_child = &(PTR_sharedOcrObjH_child->doubleBufferedOcrObjH[0]);
        doubleBufferedOcrObj_t* PTR_dBufH2_child = &(PTR_sharedOcrObjH_child->doubleBufferedOcrObjH[1]);

        PTR_sharedOcrObjH->childrenRankDBKs[ichild] = (ilevel!=num_refine) ? PTR_sharedOcrObjH_child->DBK_rankH : NULL_GUID;

        for( r=0; r<MAX_REDUCTION_HANDLES; r++ ) {
            if(ilevel!=num_refine) memcpy( &PTR_sharedOcrObjH->childrenRedObjects[ichild][r], &PTR_sharedOcrObjH_child->blockRedObjects[r], sizeof(redObjects_t) );
            else memset(&PTR_sharedOcrObjH->childrenRedObjects[ichild][r], 0, sizeof(redObjects_t));
         }

    #ifdef CHANNEL_EVENTS_AT_RECEIVER
        PTR_dBufH1->haloChildrenSendEVTs[ichild] = (ilevel!=num_refine) ? PTR_dBufH1_child->haloParentRecvEVTs[0] : NULL_GUID;
    #else
        PTR_dBufH1->haloChildrenRecvEVTs[ichild] = (ilevel!=num_refine) ? PTR_dBufH1_child->haloParentSendEVTs[0] : NULL_GUID;
    #endif

    }

    //eight siblings of the oct-tree
    for( k=0; k<2; k++)
    for( j=0; j<2; j++)
    for( i=0; i<2; i++)
    {
        isibling = i+2*j+4*k;
        sharedOcrObj_t* PTR_sharedOcrObjH_sibling = depv[6*nNbrs+9+isibling].ptr;
        doubleBufferedOcrObj_t* PTR_dBufH1_sibling = &(PTR_sharedOcrObjH_sibling->doubleBufferedOcrObjH[0]);
        doubleBufferedOcrObj_t* PTR_dBufH2_sibling = &(PTR_sharedOcrObjH_sibling->doubleBufferedOcrObjH[1]);

        PTR_sharedOcrObjH->siblingsRankDBKs[isibling] = (ilevel!=0) ? PTR_sharedOcrObjH_sibling->DBK_rankH : NULL_GUID;

        for( r=0; r<MAX_REDUCTION_HANDLES; r++ ) {
            if(ilevel!=0) memcpy( &PTR_sharedOcrObjH->siblingsRedObjects[isibling][r], &PTR_sharedOcrObjH_sibling->blockRedObjects[r], sizeof(redObjects_t) );
            else memset(&PTR_sharedOcrObjH->siblingsRedObjects[isibling][r], 0, sizeof(redObjects_t));
         }

    #ifdef CHANNEL_EVENTS_AT_RECEIVER
        PTR_dBufH1->haloSiblingsSendEVTs[isibling] = (ilevel!=0) ? PTR_dBufH1_sibling->haloSiblingsRecvEVTs[PTR_rankH->isibling] : NULL_GUID;
    #else
        PTR_dBufH1->haloSiblingsRecvEVTs[isibling] = (ilevel!=0) ? PTR_dBufH1_sibling->haloSiblingsSendEVTs[PTR_rankH->isibling] : NULL_GUID;
    #endif
    }

    DEBUG_PRINTF(("current "GUIDF" parent "GUIDF" children "GUIDF" "GUIDF" "GUIDF" "GUIDF" "GUIDF" "GUIDF" "GUIDF" "GUIDF" \nsibling "GUIDF" "GUIDF" "GUIDF" "GUIDF" "GUIDF" "GUIDF" "GUIDF" "GUIDF"\n",
        DBK_rankH, PTR_sharedOcrObjH->parentRankDBK,
        PTR_sharedOcrObjH->childrenRankDBKs[0], PTR_sharedOcrObjH->childrenRankDBKs[1], PTR_sharedOcrObjH->childrenRankDBKs[2], PTR_sharedOcrObjH->childrenRankDBKs[3], PTR_sharedOcrObjH->childrenRankDBKs[4], PTR_sharedOcrObjH->childrenRankDBKs[5], PTR_sharedOcrObjH->childrenRankDBKs[6], PTR_sharedOcrObjH->childrenRankDBKs[7],
        PTR_sharedOcrObjH->siblingsRankDBKs[0], PTR_sharedOcrObjH->siblingsRankDBKs[1], PTR_sharedOcrObjH->siblingsRankDBKs[2], PTR_sharedOcrObjH->siblingsRankDBKs[3], PTR_sharedOcrObjH->siblingsRankDBKs[4], PTR_sharedOcrObjH->siblingsRankDBKs[5], PTR_sharedOcrObjH->siblingsRankDBKs[6], PTR_sharedOcrObjH->siblingsRankDBKs[7] ));


    init(PTR_rankH);

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    ocrEVT_t initRedOEVT = PTR_rankH->sharedOcrObjH.initRedOEVT;

    int PTR_dummy[1] = { 1 };
    reductionLaunch(PTR_rankH->PTR_initRedH, PTR_rankH->DBK_initRedH, PTR_dummy);

    ocrDbRelease(DBK_rankH);

    #if 1

    //Main DAG starts at the 'Coarse'-grain level (i.e., at level 0)
    if( ilevel == 0 )
    {
        ocrGuid_t checkSumLoopEDT, checkSumLoopOEVT, checkSumLoopOEVTS;

        int istart = 0; int iend = PTR_cmd->num_vars-1;
        int istage = 0; int ts = 0;
        checkSumLoopPRM_t checkSumLoopPRM = {istart, iend, istage, ts};
        ocrEdtCreate( &checkSumLoopEDT, PTR_rankTemplateH->TML_checkSumLoop, //FNC_checkSumLoop
                      EDT_PARAM_DEF, (u64*)&checkSumLoopPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &myEdtAffinityHNT, &checkSumLoopOEVT );

        createEventHelper(&checkSumLoopOEVTS, 1);
        ocrAddDependence( checkSumLoopOEVT, checkSumLoopOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, checkSumLoopEDT, _idep++, DB_MODE_RO );
        ocrAddDependence( initRedOEVT, checkSumLoopEDT, _idep++, DB_MODE_NULL );

        MyOcrTaskStruct_t TS_miniamrMain; _paramc = 1; _depc = 2;

        TS_miniamrMain.FNC = FNC_miniamrMain;
        ocrEdtTemplateCreate( &TS_miniamrMain.TML, TS_miniamrMain.FNC, _paramc, _depc );

        ocrEdtCreate( &TS_miniamrMain.EDT, TS_miniamrMain.TML,
                      EDT_PARAM_DEF, &id, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, TS_miniamrMain.EDT, _idep++, DB_MODE_RO );
        ocrAddDependence( checkSumLoopOEVTS, TS_miniamrMain.EDT, _idep++, DB_MODE_RO );

        ocrEdtTemplateDestroy( TS_miniamrMain.TML );

    }

    #endif

    return NULL_GUID;
}

void sharedOcrObjCreate_doubleBufferedObjs( doubleBufferedOcrObj_t* PTR_dBufH, rankH_t* PTR_rankH )
{
    DEBUG_PRINTF(("%s ID %d\n", __func__, PTR_rankH->myRank_g ));

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 2;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;

    int ichild, isibling, nbr;
    int nNbrs = 6;

#ifdef CHANNEL_EVENTS_AT_RECEIVER
    ocrEventCreateParams( &(PTR_dBufH->haloParentRecvEVTs[0]), OCR_EVENT_CHANNEL_T, false, &params );
    for( ichild = 0; ichild < 8; ichild++ )
        ocrEventCreateParams( &(PTR_dBufH->haloChildrenRecvEVTs[ichild]), OCR_EVENT_CHANNEL_T, false, &params );
    for( isibling = 0; isibling < 8; isibling++ )
        ocrEventCreateParams( &(PTR_dBufH->haloSiblingsRecvEVTs[isibling]), OCR_EVENT_CHANNEL_T, false, &params );
    //
    //6 neighbors: -x, +x, -y, +y, -z, +z
    for( nbr = 0; nbr < nNbrs; nbr++ ) {
        ocrEventCreateParams( &(PTR_dBufH->haloCurrRecvEVTs[nbr]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_dBufH->haloCoarRecvEVTs[nbr]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_dBufH->haloRefnRecvEVTs[nbr][0]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_dBufH->haloRefnRecvEVTs[nbr][1]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_dBufH->haloRefnRecvEVTs[nbr][2]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_dBufH->haloRefnRecvEVTs[nbr][3]), OCR_EVENT_CHANNEL_T, false, &params );

        DEBUG_PRINTF(("Recv: ilevel %d id_g %d id_l %d nbr %d rankH "GUIDF" peer "GUIDF" c "GUIDF" ref "GUIDF" "GUIDF" "GUIDF" "GUIDF" \n", PTR_rankH->ilevel, PTR_rankH->myRank_g, PTR_rankH->myRank, nbr, PTR_rankH->sharedOcrObjH.DBK_rankH, PTR_dBufH->haloCurrRecvEVTs[nbr], PTR_dBufH->haloCoarRecvEVTs[nbr], PTR_dBufH->haloRefnRecvEVTs[nbr][0], PTR_dBufH->haloRefnRecvEVTs[nbr][1], PTR_dBufH->haloRefnRecvEVTs[nbr][2], PTR_dBufH->haloRefnRecvEVTs[nbr][3]));


    }

#else
    ocrEventCreateParams( &(PTR_dBufH->haloParentSendEVTs[0]), OCR_EVENT_CHANNEL_T, false, &params );
    for( ichild = 0; ichild < 8; ichild++ )
        ocrEventCreateParams( &(PTR_dBufH->haloChildrenSendEVTs[ichild]), OCR_EVENT_CHANNEL_T, false, &params );
    for( isibling = 0; isibling < 8; isibling++ )
        ocrEventCreateParams( &(PTR_dBufH->haloSiblingsSendEVTs[isibling]), OCR_EVENT_CHANNEL_T, false, &params );

    //
    //6 neighbors: -x, +x, -y, +y, -z, +z
    for( nbr = 0; nbr < nNbrs; nbr++ ) {

        ocrEventCreateParams( &(PTR_dBufH->haloCurrSendEVTs[nbr]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_dBufH->haloCoarSendEVTs[nbr]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_dBufH->haloRefnSendEVTs[nbr][0]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_dBufH->haloRefnSendEVTs[nbr][1]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_dBufH->haloRefnSendEVTs[nbr][2]), OCR_EVENT_CHANNEL_T, false, &params );
        ocrEventCreateParams( &(PTR_dBufH->haloRefnSendEVTs[nbr][3]), OCR_EVENT_CHANNEL_T, false, &params );

        DEBUG_PRINTF(("Send: ilevel %d id_g %d id_l %d nbr %d rankH "GUIDF" peer "GUIDF" c "GUIDF" ref "GUIDF" "GUIDF" "GUIDF" "GUIDF" \n", PTR_rankH->ilevel, PTR_rankH->myRank_g, PTR_rankH->myRank, nbr, PTR_rankH->sharedOcrObjH.DBK_rankH, PTR_dBufH->haloCurrSendEVTs[nbr], PTR_dBufH->haloCoarSendEVTs[nbr], PTR_dBufH->haloRefnSendEVTs[nbr][0], PTR_dBufH->haloRefnSendEVTs[nbr][1], PTR_dBufH->haloRefnSendEVTs[nbr][2], PTR_dBufH->haloRefnSendEVTs[nbr][3]));

    }

#endif
}

void sharedOcrObjCreate( rankH_t* PTR_rankH )
{
    DEBUG_PRINTF(("%s ID %d\n", __func__, PTR_rankH->myRank_g ));

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);

    ocrGuidRangeCreate(&(PTR_sharedOcrObjH->childrenintentionRedRangeGUID), 8, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(PTR_sharedOcrObjH->childrenchkSumRedRangeGUID), 8, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(PTR_sharedOcrObjH->childrenredRangeGUID), 8, GUID_USER_EVENT_STICKY);

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 2;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;

    int *PTR_in, *PTR_out;
    int r;
    size_t size;
    for( r=0; r<MAX_REDUCTION_HANDLES; r++ ) {
        redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

        ocrEventCreateParams(&(PTR_redObjects->upIEVT), OCR_EVENT_CHANNEL_T, true, &params);
        ocrEventCreateParams(&(PTR_redObjects->upOEVT), OCR_EVENT_CHANNEL_T, true, &params);
        ocrEventCreateParams(&(PTR_redObjects->downIEVT), OCR_EVENT_CHANNEL_T, true, &params);
        ocrEventCreateParams(&(PTR_redObjects->downOEVT), OCR_EVENT_CHANNEL_T, true, &params);

        size = reductionDBKsize( r, PTR_rankH );

        ocrDbCreate( &PTR_redObjects->DBK_in, (void**) &PTR_in, size, 0, NULL_HINT, NO_ALLOC );
        ocrDbCreate( &PTR_redObjects->DBK_out, (void**) &PTR_out, size, 0, NULL_HINT, NO_ALLOC );

        ocrDbRelease( PTR_redObjects->DBK_in );
        ocrDbRelease( PTR_redObjects->DBK_out );

    }

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    sharedOcrObjCreate_doubleBufferedObjs( PTR_dBufH1, PTR_rankH );
#ifdef DOUBLE_BUFFERED_EVTS
    sharedOcrObjCreate_doubleBufferedObjs( PTR_dBufH2, PTR_rankH );
#endif

}

ocrGuid_t initEdt( EDT_ARGS )
{
    PRM_init3dEdt_t* PTR_PRM_initEdt = (PRM_init3dEdt_t*) paramv;

    u64 id = PTR_PRM_initEdt->id;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    ocrGuid_t DBK_globalParamH = depv[1].guid;

    void *PTR_cmdLineArgs = depv[0].ptr;
    globalParamH_t *PTR_globalParamH = (globalParamH_t *) depv[1].ptr;

    Command* PTR_cmd = &(PTR_globalParamH->cmdParamH);

    DEBUG_PRINTF(("%s ID %d\n", __func__, id ));

    ocrHint_t myEdtAffinityHNT, myDbkAffinityHNT;
    getAffinityHintsForDBandEdt( &myDbkAffinityHNT, &myEdtAffinityHNT );

    s32 npx = PTR_PRM_initEdt->edtGridDims[0];
    s32 npy = PTR_PRM_initEdt->edtGridDims[1];
    s32 npz = PTR_PRM_initEdt->edtGridDims[2];

    int ix0, iy0, iz0, ix, iy, iz;

    globalCoordsFromRank_Cart3D( id, npx, npy, npz, &ix0, &iy0, &iz0 );

#if 1
   int ilevel, iblock, jblock, kblock;
   int iblock_g, jblock_g, kblock_g;
   int p2 = 1;
   int p8 = 1;
   int offset = 0;
   int num_refine = PTR_cmd->num_refine;
   int init_block_x = PTR_cmd->init_block_x;
   int init_block_y = PTR_cmd->init_block_y;
   int init_block_z = PTR_cmd->init_block_z;
   int gx, gy, gz;
   int id_l, id_g;
   int i_r[4], j_r[4], k_r[4];
   int ix_r[4], iy_r[4], iz_r[4], nbrRank_r[4];
   int ix_c, iy_c, iz_c;

   int ichild;
   int ix_child[2][2][2], iy_child[2][2][2], iz_child[2][2][2], childRanks[2][2][2];
   int ix_parent, iy_parent, iz_parent, parentRank;

   int isibling;
   int ix_sibling[2][2][2], iy_sibling[2][2][2], iz_sibling[2][2][2], siblingRanks[2][2][2];

   int ievent;

   for (ilevel = 0; ilevel <= num_refine; ilevel++)
   {
        gz = p2*npz*init_block_z; //global blocks in z at this level
        gy = p2*npy*init_block_y;
        gx = p2*npx*init_block_x;

        for (kblock = 0; kblock < p2*init_block_z; kblock++)
        for (jblock = 0; jblock < p2*init_block_y; jblock++)
        for (iblock = 0; iblock < p2*init_block_x; iblock++) {
            kblock_g = p2*iz0*init_block_z + kblock;
            jblock_g = p2*iy0*init_block_y + jblock;
            iblock_g = p2*ix0*init_block_x + iblock;

            id_l = globalRankFromCoords_Cart3D(iblock_g, jblock_g, kblock_g, gx, gy, gz);

            id_g = offset + id_l;

            ocrGuid_t DBK_rankH;
            rankH_t *PTR_rankH;

            //DB creation calls
            ocrDbCreate( &DBK_rankH, (void**) &PTR_rankH, sizeof(rankH_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
            memcpy( &(PTR_rankH->globalParamH), PTR_globalParamH, sizeof(globalParamH_t) );

            PTR_rankH->myDbkAffinityHNT = myDbkAffinityHNT;
            PTR_rankH->myEdtAffinityHNT = myEdtAffinityHNT;

            PTR_rankH->my_pe = id;
            PTR_rankH->num_pes = npx*npy*npz;

            PTR_rankH->ilevel = ilevel;
            PTR_rankH->myRank = id_l;
            PTR_rankH->nRanks = gx*gy*gz; //at this level
            PTR_rankH->gx = gx;
            PTR_rankH->gy = gy;
            PTR_rankH->gz = gz;
            PTR_rankH->iblock = iblock;
            PTR_rankH->jblock = jblock;
            PTR_rankH->kblock = kblock;

            PTR_rankH->iblock_g = iblock_g;
            PTR_rankH->jblock_g = jblock_g;
            PTR_rankH->kblock_g = kblock_g;
            PTR_rankH->myRank_g = id_g;

            PTR_rankH->isibling = (ilevel!=0)?(iblock_g%2)+2*(jblock_g%2)+4*(kblock_g%2):-1;

            sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
            PTR_sharedOcrObjH->DBK_rankH = DBK_rankH;

            sharedOcrObjCreate( PTR_rankH );

            int nNbrs = 6;

            ocrGuid_t channelSetupTML, stickyEVT;
            ocrEDT_t channelSetupEDT;

            PRM_channelSetupEdt_t PRM_channelSetupEdt;
            PRM_channelSetupEdt.id = id_l;
            PRM_channelSetupEdt.nNbrs = nNbrs;

            ocrEdtTemplateCreate( &channelSetupTML, channelSetupEdt, PARAMC_U64(PRM_channelSetupEdt_t), 6*nNbrs+1+8+8+1 ); // (1 peer, 1 coarser, 4 refined) nbrs per dir + 1 parent + 8 children + 8 siblings + 1 self-rankH
            ocrEdtCreate( &channelSetupEDT, channelSetupTML, EDT_PARAM_DEF, (u64*)&PRM_channelSetupEdt, EDT_PARAM_DEF, NULL, EDT_PROP_NONE,
                            &myEdtAffinityHNT, NULL );
            ocrEdtTemplateDestroy( channelSetupTML );

            int nbr, i, j, k;
            int nbrImage;

            //6 neighbors: -x, +x, -y, +y, -z, +z
            for( nbr = 0; nbr < nNbrs; nbr++ ) {
                i = j = k = 0;
                i_r[0] = j_r[0] = k_r[0] = 0;
                i_r[1] = j_r[1] = k_r[1] = 0;
                i_r[2] = j_r[2] = k_r[2] = 0;
                i_r[3] = j_r[3] = k_r[3] = 0;

                switch(nbr) {
                    case 0:
                        i = -1;                 // neighbor offset
                        nbrImage = 1;           // this is the tag the same nbr uses to me
                        i_r[0] = 1; j_r[0] = 0; k_r[0] = 0;
                        i_r[1] = 1; j_r[1] = 1; k_r[1] = 0;
                        i_r[2] = 1; j_r[2] = 0; k_r[2] = 1;
                        i_r[3] = 1; j_r[3] = 1; k_r[3] = 1;
                        break;

                    case 1:
                        i = 1;
                        nbrImage = 0;
                        i_r[0] = 0; j_r[0] = 0; k_r[0] = 0;
                        i_r[1] = 0; j_r[1] = 1; k_r[1] = 0;
                        i_r[2] = 0; j_r[2] = 0; k_r[2] = 1;
                        i_r[3] = 0; j_r[3] = 1; k_r[3] = 1;
                        break;

                    case 2:
                        j = -1;
                        nbrImage = 3;
                        i_r[0] = 0; j_r[0] = 1; k_r[0] = 0;
                        i_r[1] = 1; j_r[1] = 1; k_r[1] = 0;
                        i_r[2] = 0; j_r[2] = 1; k_r[2] = 1;
                        i_r[3] = 1; j_r[3] = 1; k_r[3] = 1;
                        break;
                    case 3:
                        j = 1;
                        nbrImage = 2;
                        i_r[0] = 0; j_r[0] = 0; k_r[0] = 0;
                        i_r[1] = 1; j_r[1] = 0; k_r[1] = 0;
                        i_r[2] = 0; j_r[2] = 0; k_r[2] = 1;
                        i_r[3] = 1; j_r[3] = 0; k_r[3] = 1;
                        break;
                    case 4:
                        k = -1;
                        nbrImage = 5;
                        i_r[0] = 0; j_r[0] = 0; k_r[0] = 1;
                        i_r[1] = 1; j_r[1] = 0; k_r[1] = 1;
                        i_r[2] = 0; j_r[2] = 1; k_r[2] = 1;
                        i_r[3] = 1; j_r[3] = 1; k_r[3] = 1;
                        break;
                    case 5:
                        k = 1;
                        nbrImage = 4;
                        i_r[0] = 0; j_r[0] = 0; k_r[0] = 0;
                        i_r[1] = 1; j_r[1] = 0; k_r[1] = 0;
                        i_r[2] = 0; j_r[2] = 1; k_r[2] = 0;
                        i_r[3] = 1; j_r[3] = 1; k_r[3] = 0;
                        break;
                }

                ix = MOD(iblock_g+i, gx); //periodic index
                iy = MOD(jblock_g+j, gy);
                iz = MOD(kblock_g+k, gz);
                u32 nbrRank = globalRankFromCoords_Cart3D(ix, iy, iz, gx, gy, gz);

                u32 nbrRank_c;

                //Collective event create for sends
                ocrGuidFromIndex(&(stickyEVT), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID[ilevel], nNbrs*id_l + nbr);//send
                ocrEventCreate( &stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );

                sharedOcrObj_t* PTR_sharedOcrObjH_copy;
                ocrGuid_t eventsDBK;
                ocrDbCreate( &eventsDBK, (void**) &PTR_sharedOcrObjH_copy, sizeof(sharedOcrObj_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );
                ievent = 0;

                memcpy( PTR_sharedOcrObjH_copy, PTR_sharedOcrObjH, sizeof(sharedOcrObj_t) );

                ocrDbRelease( eventsDBK );

                ocrEventSatisfy( stickyEVT, eventsDBK );

                ocrGuid_t stkyEVT_nbrImg;
                ocrGuid_t stkyEVT_nbrImg_c;
                ocrGuid_t stkyEVT_nbrImg_r[4];

                //Map the sends to receive events
                //receive: (id, nbr) will be the send event from the (nbrRank,nbrImage)
                ocrGuidFromIndex( &(stkyEVT_nbrImg), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID[ilevel], nNbrs*nbrRank + nbrImage );
                ocrEventCreate( &stkyEVT_nbrImg, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );

                if( ilevel != 0 ) {
                    ix_c = MOD((iblock_g)/2+i, gx/2); //periodic index
                    iy_c = MOD((jblock_g)/2+j, gy/2);
                    iz_c = MOD((kblock_g)/2+k, gz/2);
                    nbrRank_c = globalRankFromCoords_Cart3D(ix_c, iy_c, iz_c, gx/2, gy/2, gz/2);

                    ocrGuidFromIndex( &(stkyEVT_nbrImg_c), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID[ilevel-1], nNbrs*nbrRank_c + nbrImage );
                    ocrEventCreate( &stkyEVT_nbrImg_c, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );
                }
                else {
                    createEventHelper(&stkyEVT_nbrImg_c, 1);
                    ocrEventSatisfy( stkyEVT_nbrImg_c, NULL_GUID );
                }

                if( ilevel != num_refine ) {
                    ix_r[0] = MOD(ix*2+i_r[0], gx*2); //periodic index
                    iy_r[0] = MOD(iy*2+j_r[0], gy*2);
                    iz_r[0] = MOD(iz*2+k_r[0], gz*2);
                    nbrRank_r[0] = globalRankFromCoords_Cart3D(ix_r[0], iy_r[0], iz_r[0], gx*2, gy*2, gz*2);

                    ix_r[1] = MOD(ix*2+i_r[1], gx*2); //periodic index
                    iy_r[1] = MOD(iy*2+j_r[1], gy*2);
                    iz_r[1] = MOD(iz*2+k_r[1], gz*2);
                    nbrRank_r[1] = globalRankFromCoords_Cart3D(ix_r[1], iy_r[1], iz_r[1], gx*2, gy*2, gz*2);

                    ix_r[2] = MOD(ix*2+i_r[2], gx*2); //periodic index
                    iy_r[2] = MOD(iy*2+j_r[2], gy*2);
                    iz_r[2] = MOD(iz*2+k_r[2], gz*2);
                    nbrRank_r[2] = globalRankFromCoords_Cart3D(ix_r[2], iy_r[2], iz_r[2], gx*2, gy*2, gz*2);

                    ix_r[3] = MOD(ix*2+i_r[3], gx*2); //periodic index
                    iy_r[3] = MOD(iy*2+j_r[3], gy*2);
                    iz_r[3] = MOD(iz*2+k_r[3], gz*2);
                    nbrRank_r[3] = globalRankFromCoords_Cart3D(ix_r[3], iy_r[3], iz_r[3], gx*2, gy*2, gz*2);

                    ocrGuidFromIndex( &(stkyEVT_nbrImg_r[0]), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID[ilevel+1], nNbrs*nbrRank_r[0] + nbrImage );
                    ocrEventCreate( &stkyEVT_nbrImg_r[0], OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );
                    ocrGuidFromIndex( &(stkyEVT_nbrImg_r[1]), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID[ilevel+1], nNbrs*nbrRank_r[1] + nbrImage );
                    ocrEventCreate( &stkyEVT_nbrImg_r[1], OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );
                    ocrGuidFromIndex( &(stkyEVT_nbrImg_r[2]), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID[ilevel+1], nNbrs*nbrRank_r[2] + nbrImage );
                    ocrEventCreate( &stkyEVT_nbrImg_r[2], OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );
                    ocrGuidFromIndex( &(stkyEVT_nbrImg_r[3]), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID[ilevel+1], nNbrs*nbrRank_r[3] + nbrImage );
                    ocrEventCreate( &stkyEVT_nbrImg_r[3], OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );
                }
                else {
                    createEventHelper(&stkyEVT_nbrImg_r[0], 1);
                    createEventHelper(&stkyEVT_nbrImg_r[1], 1);
                    createEventHelper(&stkyEVT_nbrImg_r[2], 1);
                    createEventHelper(&stkyEVT_nbrImg_r[3], 1);
                    ocrEventSatisfy( stkyEVT_nbrImg_r[0], NULL_GUID );
                    ocrEventSatisfy( stkyEVT_nbrImg_r[1], NULL_GUID );
                    ocrEventSatisfy( stkyEVT_nbrImg_r[2], NULL_GUID );
                    ocrEventSatisfy( stkyEVT_nbrImg_r[3], NULL_GUID );
                }

                ocrAddDependence( stkyEVT_nbrImg, channelSetupEDT, nbr, DB_MODE_RO );
                ocrAddDependence( stkyEVT_nbrImg_c, channelSetupEDT, nNbrs+nbr, DB_MODE_RO );
                ocrAddDependence( stkyEVT_nbrImg_r[0], channelSetupEDT, 2*nNbrs+4*nbr+0, DB_MODE_RO );
                ocrAddDependence( stkyEVT_nbrImg_r[1], channelSetupEDT, 2*nNbrs+4*nbr+1, DB_MODE_RO );
                ocrAddDependence( stkyEVT_nbrImg_r[2], channelSetupEDT, 2*nNbrs+4*nbr+2, DB_MODE_RO );
                ocrAddDependence( stkyEVT_nbrImg_r[3], channelSetupEDT, 2*nNbrs+4*nbr+3, DB_MODE_RO );

            } //end of for loop over nbrs

            ocrGuid_t stkyEVT_parent;
            ocrGuid_t stkyEVT_child[8];
            ocrGuid_t stkyEVT_sibling[8];

            //one parent in the oct-tree
            u32 parentRank = 0;
            if( ilevel != 0 ) {
                ix_parent = MOD((iblock_g)/2, gx/2); //periodic index
                iy_parent = MOD((jblock_g)/2, gy/2);
                iz_parent = MOD((kblock_g)/2, gz/2);
                parentRank = globalRankFromCoords_Cart3D(ix_parent, iy_parent, iz_parent, gx/2, gy/2, gz/2);

                ocrGuidFromIndex( &(stkyEVT_parent), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID[ilevel-1], nNbrs*parentRank + 0 );
                ocrEventCreate( &stkyEVT_parent, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );
            }
            else {
                createEventHelper(&stkyEVT_parent, 1);
                ocrEventSatisfy( stkyEVT_parent, NULL_GUID );
            }

            ocrAddDependence( stkyEVT_parent, channelSetupEDT, 6*nNbrs, DB_MODE_RO );

            //eight children of the oct-tree
            for( k=0; k<2; k++)
            for( j=0; j<2; j++)
            for( i=0; i<2; i++) {
                ichild = i+2*j+4*k;

                if( ilevel != num_refine ) {
                    ix_child[i][j][k] = MOD((iblock_g)*2+i, gx*2); //periodic index
                    iy_child[i][j][k] = MOD((jblock_g)*2+j, gy*2);
                    iz_child[i][j][k] = MOD((kblock_g)*2+k, gz*2);
                    childRanks[i][j][k] = globalRankFromCoords_Cart3D(ix_child[i][j][k], iy_child[i][j][k], iz_child[i][j][k], gx*2, gy*2, gz*2);
                    ocrGuidFromIndex( &(stkyEVT_child[ichild]), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID[ilevel+1], nNbrs*childRanks[i][j][k] + 0 ); //pick nbr '0'
                    ocrEventCreate( &stkyEVT_child[ichild], OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );
                }
                else {
                    createEventHelper(&stkyEVT_child[ichild], 1);
                    ocrEventSatisfy( stkyEVT_child[ichild], NULL_GUID );
                }

                ocrAddDependence( stkyEVT_child[ichild], channelSetupEDT, 6*nNbrs+1+ichild, DB_MODE_RO );
            }

            //eight siblings of the oct-tree
            for( k=0; k<2; k++)
            for( j=0; j<2; j++)
            for( i=0; i<2; i++)
            {
                isibling = i+2*j+4*k;

                if( ilevel != 0 ) {
                    ix_sibling[i][j][k] = MOD((iblock_g/2)*2+i, gx); //periodic index
                    iy_sibling[i][j][k] = MOD((jblock_g/2)*2+j, gy);
                    iz_sibling[i][j][k] = MOD((kblock_g/2)*2+k, gz);
                    siblingRanks[i][j][k] = globalRankFromCoords_Cart3D(ix_sibling[i][j][k], iy_sibling[i][j][k], iz_sibling[i][j][k], gx, gy, gz);
                    ocrGuidFromIndex( &(stkyEVT_sibling[isibling]), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID[ilevel], nNbrs*siblingRanks[i][j][k] + 0 ); //pick nbr '0'
                    ocrEventCreate( &stkyEVT_sibling[isibling], OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );
                }
                else {
                    createEventHelper(&stkyEVT_sibling[isibling], 1);
                    ocrEventSatisfy( stkyEVT_sibling[isibling], NULL_GUID );
                }

                ocrAddDependence( stkyEVT_sibling[isibling], channelSetupEDT, 6*nNbrs+9+isibling, DB_MODE_RO );
            }

            ocrDbRelease(DBK_rankH);
            ocrAddDependence( DBK_rankH, channelSetupEDT, 6*nNbrs+9+8, DB_MODE_RW );
        }

        offset += p8*npx*npy*npz*init_block_x*init_block_y*init_block_z;

        p2 *= 2;
        p8 *= 8;
    }

#endif

    return NULL_GUID;
}


// =================================== print_help_message ====================

void print_help_message(void)
{
   PRINTF("(Optional) command line input is of the form: \n\n");

   PRINTF("--nx - block size x (even && > 0)\n");
   PRINTF("--ny - block size y (even && > 0)\n");
   PRINTF("--nz - block size z (even && > 0)\n");
   PRINTF("--init_x - initial blocks in x (> 0)\n");
   PRINTF("--init_y - initial blocks in y (> 0)\n");
   PRINTF("--init_z - initial blocks in z (> 0)\n");
   PRINTF("--reorder - ordering of blocks if initial number > 1\n");
   PRINTF("--npx - (0 < npx <= num_pes)\n");
   PRINTF("--npy - (0 < npy <= num_pes)\n");
   PRINTF("--npz - (0 < npz <= num_pes)\n");
   PRINTF("--max_blocks - maximun number of blocks per core\n");
   PRINTF("--num_refine - (>= 0) number of levels of refinement\n");
   PRINTF("--block_change - (>= 0) number of levels a block can change in a timestep\n");
   PRINTF("--uniform_refine - if 1, then grid is uniformly refined\n");
   PRINTF("--refine_freq - frequency (in timesteps) of checking for refinement\n");
   PRINTF("--target_active - (>= 0) target number of blocks per core, none if 0\n");
   PRINTF("--target_max - (>= 0) max number of blocks per core, none if 0\n");
   PRINTF("--target_min - (>= 0) min number of blocks per core, none if 0\n");
   PRINTF("--inbalance - percentage inbalance to trigger inbalance\n");
   PRINTF("--lb_opt - load balancing - 0 = none, 1 = each refine, 2 = each refine phase\n");
   PRINTF("--num_vars - number of variables (> 0)\n");
   PRINTF("--comm_vars - number of vars to communicate together\n");
   PRINTF("--num_tsteps - number of timesteps (> 0)\n");
   PRINTF("--stages_per_ts - number of comm/calc stages per timestep\n");
   PRINTF("--checksum_freq - number of stages between checksums\n");
   PRINTF("--stencil - 7 or 27 point (27 will not work with refinement (except uniform))\n");
   PRINTF("--error_tol - (e^{-error_tol} ; >= 0) \n");
   PRINTF("--report_diffusion - (>= 0) none if 0\n");
   PRINTF("--report_perf - 0, 1, 2\n");
   PRINTF("--refine_freq - frequency (timesteps) of plotting (0 for none)\n");
   PRINTF("--code - closely minic communication of different codes\n");
   PRINTF("         0 minimal sends, 1 send ghosts, 2 send ghosts and process on send\n");
   PRINTF("--permute - altenates directions in communication\n");
   PRINTF("--blocking_send - use blocking sends instead of nonblocking\n");
   PRINTF("--refine_ghost - use full extent of block (including ghosts) to determine if block is refined\n");
   PRINTF("--num_objects - (>= 0) number of objects to cause refinement\n");
   PRINTF("--object - type, position, movement, size, size rate of change\n");

   PRINTF("All associated settings are integers except for objects\n");
}

int check_input(Command cmd)
{
   int error = 0;

   if (cmd.init_block_x < 1 || cmd.init_block_y < 1 || cmd.init_block_z < 1) {
      PRINTF("initial blocks on processor must be positive\n");
      error = 1;
   }
   if (cmd.max_num_blocks < cmd.init_block_x*cmd.init_block_y*cmd.init_block_z) {
      PRINTF("max_num_blocks not large enough\n");
      error = 1;
   }
   if (cmd.x_block_size < 1 || cmd.y_block_size < 1 || cmd.z_block_size < 1) {
      PRINTF("block size must be positive\n");
      error = 1;
   }
   if (((cmd.x_block_size/2)*2) != cmd.x_block_size) {
      PRINTF("block size in x direction must be even\n");
      error = 1;
   }
   if (((cmd.y_block_size/2)*2) != cmd.y_block_size) {
      PRINTF("block size in y direction must be even\n");
      error = 1;
   }
   if (((cmd.z_block_size/2)*2) != cmd.z_block_size) {
      PRINTF("block size in z direction must be even\n");
      error = 1;
   }
   if (cmd.target_active && cmd.target_max) {
      PRINTF("Only one of target_active and target_max can be used\n");
      error = 1;
   }
   if (cmd.target_active && cmd.target_min) {
      PRINTF("Only one of target_active and target_min can be used\n");
      error = 1;
   }
   if (cmd.target_active < 0 || cmd.target_active > cmd.max_num_blocks) {
      PRINTF("illegal value for target_active\n");
      error = 1;
   }
   if (cmd.target_max < 0 || cmd.target_max > cmd.max_num_blocks ||
       cmd.target_max < cmd.target_active) {
      PRINTF("illegal value for target_max\n");
      error = 1;
   }
   if (cmd.target_min < 0 || cmd.target_min > cmd.max_num_blocks ||
       cmd.target_min > cmd.target_active || cmd.target_min > cmd.target_max) {
      PRINTF("illegal value for target_min\n");
      error = 1;
   }
   if (cmd.num_refine < 0) {
      PRINTF("number of refinement levels must be non-negative\n");
      error = 1;
   }
   if (cmd.block_change < 0) {
      PRINTF("number of refinement levels must be non-negative\n");
      error = 1;
   }
   if (cmd.num_vars < 1) {
      PRINTF("number of variables must be positive\n");
      error = 1;
   }
   //if (num_pes != npx*npy*npz) {
   //   PRINTF("number of processors used does not match number allocated\n");
   //   error = 1;
   //}
   if (cmd.stencil != 7 && cmd.stencil != 27) {
      PRINTF("illegal value for stencil\n");
      error = 1;
   }
   if (cmd.stencil == 27 && cmd.num_refine && !cmd.uniform_refine)
      PRINTF("WARNING: 27 point stencil with non-uniform refinement: answers may diverge\n");
   if (cmd.code < 0 || cmd.code > 2) {
      PRINTF("code must be 0, 1, or 2\n");
      error = 1;
   }
   if (cmd.lb_opt < 0 || cmd.lb_opt > 2) {
      PRINTF("lb_opt must be 0, 1, or 2\n");
      error = 1;
   }

   return (error);
}
