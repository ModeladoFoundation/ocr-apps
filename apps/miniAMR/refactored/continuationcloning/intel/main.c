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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>

#ifdef BUILD_OCR_VERSION
#include <ocr.h>
#include <ocr-std.h>
#include <extensions/ocr-labeling.h>
#endif

#include "block.h"
#include "comm.h"
#include "timer.h"
#include "proto.h"

#ifdef BUILD_OCR_VERSION

#define MAKE_DB_MODE_X(mode) DB_MODE_ ## mode
#define ADD_DEPENDENCE(source,dest,depList,depSlotName,mode) ocrAddDependence(source,dest,offsetof(depList,depSlotName)/sizeof(ocrEdtDep_t),MAKE_DB_MODE_X(mode));


ocrGuid_t mainEdt ( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] );
// This is the EDT that the OCR startup instantiates.  It creates only ONE instance of this EDT.
// The EDT does the following:
//   * Process the command-line arguments to figure out how many "ranks" to instantiate, which is numberOfPes=npx*npy*npz.
//   * Process the command-line arguments to figure out how many objects will be modeled to move through the mesh.
//   * Creates as "sharedDb" datablock for an instance of the shared_t struct.

// mainEdt receives NO parameters.

typedef struct {
   ocrEdtDep_t dep_argv;                    // OCR startup sends us a datablock comprised of the argc argument count followed by the arguments themselves.
} main_Deps_t;


ocrGuid_t realMainEdt ( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] );
// This EDT completes the work started by mainEdt.  With the shared datablock now "seasoned", this EDT can write to it.
// It creates a labeled-guid "range" array in the shared datablock, large enough to host (numberOfPes**2) event guids.
//   * Creates the labeled-guid range arrays
//   * Creates a datablock large enough to host the descriptions of those objects.
//   * Instantiates numberOfPes copies of "realMainEdt".

typedef struct {
   u64 prm_numberOfPes;                     // Number of "ranks".
   u64 prm_numberOfObjects;                 // Number of objects that the command-line provides, which will be modeled as they move through the mesh.
} realMain_Params_t;

typedef struct {
   ocrEdtDep_t dep_shared;
   ocrEdtDep_t dep_argv;
} realMain_Deps_t;


ocrGuid_t referenceVersionsMainEdt ( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] );

typedef struct {
   u64 prm_continuationTemplate;     // Guid of template to create an instance of the referenceVersionMain EDT.
   u64 prm_my_pe;                    // Index of my "rank".
   u64 prm_isFirstInstance;          // Non-zero for the top-level instance of referenceVersionMain (on each rank);  Zero for all other instances, which are continuations.
} referenceVersionsMain_Params_t;

typedef struct {
   ocrEdtDep_t dep_shared;           // The datablock that is shared among all ranks.  It contains the labeled guid range.
   union {
      ocrEdtDep_t dep_argv;          // For the top-level instance, the command-line arguments will be received by this dependency.
      ocrEdtDep_t dep_newResource;   // For all continuation instances, a "new resource" might be received,  e.g. when the continuation is caused by needing to recieve the results of a cross-rank communication.
   };
   ocrEdtDep_t dep_glbl;             // The datablock for what in the reference version were all the relatively small global variables.
   ocrEdtDep_t dep_scratch_objects;  // Space needed while the command line arguments are being processed, to capture details about the objects that will be moving through the mesh.

   // ***NOTE***:  For the top-level instance, only the above dependencies are passed in.  For all continuation instances, all the remaining dependencies are also passed in.

   ocrEdtDep_t dep_bulk;             // The datablock for what in the reference version were all the relatively large global variables.
   ocrEdtDep_t dep_tmpDb1;
   ocrEdtDep_t dep_tmpDb2;
   ocrEdtDep_t dep_tmpDb3;
   ocrEdtDep_t dep_tmpDb4;
   ocrEdtDep_t dep_tmpDb5;
   ocrEdtDep_t dep_tmpDb6;
   ocrEdtDep_t dep_tmpDb7;
   ocrEdtDep_t dep_bin;
   ocrEdtDep_t dep_comms;
   ocrEdtDep_t dep_me;
   ocrEdtDep_t dep_np;
   ocrEdtDep_t dep_request;
   ocrEdtDep_t dep_send_buff;
   ocrEdtDep_t dep_s_req;
   ocrEdtDep_t dep_par_b__comm_b;
   ocrEdtDep_t dep_par_b__comm_c;
   ocrEdtDep_t dep_par_b__comm_part;
   ocrEdtDep_t dep_par_b__comm_p;
   ocrEdtDep_t dep_par_b__index;
   ocrEdtDep_t dep_par_b__par_comm_num;
   ocrEdtDep_t dep_par_p1__comm_b;
   ocrEdtDep_t dep_par_p1__comm_c;
   ocrEdtDep_t dep_par_p1__comm_part;
   ocrEdtDep_t dep_par_p1__comm_p;
   ocrEdtDep_t dep_par_p1__index;
   ocrEdtDep_t dep_par_p1__par_comm_num;
   ocrEdtDep_t dep_par_p__comm_b;
   ocrEdtDep_t dep_par_p__comm_c;
   ocrEdtDep_t dep_par_p__comm_part;
   ocrEdtDep_t dep_par_p__comm_p;
   ocrEdtDep_t dep_par_p__index;
   ocrEdtDep_t dep_par_p__par_comm_num;
   ocrEdtDep_t dep_comm_block[3];
   ocrEdtDep_t dep_comm_face_case[3];
   ocrEdtDep_t dep_comm_index[3];
   ocrEdtDep_t dep_comm_num[3];
   ocrEdtDep_t dep_comm_partner[3];
   ocrEdtDep_t dep_comm_pos1[3];
   ocrEdtDep_t dep_comm_pos[3];
   ocrEdtDep_t dep_comm_recv_off[3];
   ocrEdtDep_t dep_comm_send_off[3];
   ocrEdtDep_t dep_recv_size[3];
   ocrEdtDep_t dep_send_size[3];
   ocrEdtDep_t dep_gbin;
   ocrEdtDep_t dep_recv_buff;
} referenceVersionsMain_Deps_t;

ocrGuid_t wrapupEdt      ( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] );

// wrapupEdt receives NO parameters.

typedef struct {
   u64 prm_dummy;                           // wrapup doesn't really need any params, but give it one anyway.
} wrapup_Params_t;

typedef struct {
   ocrEdtDep_t dep_DoneEvent;
} wrapup_Deps_t;


ocrGuid_t mainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
// This is the EDT that the OCR startup instantiates.  It creates only ONE instance of this EDT.
// The EDT does the following:
//   * Process the command-line arguments to figure out how many "ranks" to instantiate, which is numberOfPes=npx*npy*npz.
//   * Process the command-line arguments to figure out how many objects will be modeled to move through the mesh.
//   * Creates as "sharedDb" datablock for an instance of the shared_t struct.

// mainEdt receives NO parameters.

   main_Deps_t * myDeps = (main_Deps_t *) depv;

   ocrGuid_t realMainTemplate, realMain, realMainOutputEvent;
   ocrGuid_t wrapupTemplate,   wrapup,   wrapupOutputEvent;
   ocrGuid_t sharedDb;
   void * dummy;

   char ** argv__mainEdt = ((char **) (myDeps->dep_argv.ptr));
   u32 argc = getArgc( argv__mainEdt );

   u64 numberOfPes = 1;
   int npxSeen = 0;
   int npySeen = 0;
   int npzSeen = 0;
   int numberOfObjects = 1;
   int i;

// Process the command line only to the extent necessary to figure out how many "ranks" will be utilized, and how many objects to model.

   for (i = 1; i < argc; i++) {
      if (!strcmp(getArgv(argv__mainEdt, i), "--npx")) {
         i++;
         numberOfPes *= atoi(getArgv(argv__mainEdt, i));
         if (npxSeen++) {
            printf ("--npx command-line argument should only appear once (or not at all, in which case it will default to 1.)\n");fflush(stdout);
            ocrShutdown();
         }
      } else if (!strcmp(getArgv(argv__mainEdt, i), "--npy")) {
         i++;
         numberOfPes *= atoi(getArgv(argv__mainEdt, i));
         if (npySeen++) {
            printf ("--npy command-line argument should only appear once (or not at all, in which case it will default to 1.)\n");fflush(stdout);
            ocrShutdown();
         }
      } else if (!strcmp(getArgv(argv__mainEdt, i), "--npz")) {
         i++;
         numberOfPes *= atoi(getArgv(argv__mainEdt, i));
         if (npzSeen++) {
            printf ("--npz command-line argument should only appear once (or not at all, in which case it will default to 1.)\n");fflush(stdout);
            ocrShutdown();
         }
      } else if (!strcmp(getArgv(argv__mainEdt, i), "--num_objects")) {
         numberOfObjects = atoi(getArgv(argv__mainEdt, ++i));
      }
   }

   if (numberOfPes > MAX_RANKS) {
      printf ("numberOfPes = %d, but MAX_RANKS symbolic constant is only %d.  Enlarge the latter and recompile.\n", numberOfPes, MAX_RANKS);fflush(stdout);
      ocrShutdown();
   }

   ocrDbCreate(&sharedDb, (void **)&dummy, sizeof(shared_t), 0, NULL_GUID, NO_ALLOC);    // Create the datablock to share with all "ranks".

   ocrEdtTemplateCreate(&realMainTemplate,                                    // Guid of template
                        realMainEdt,                                          // Top level function of EDT
                        sizeof(realMain_Params_t) / sizeof(u64),              // Number of params
                        sizeof(realMain_Deps_t)   / sizeof(ocrEdtDep_t));     // Number of dependencies

   ocrEdtTemplateCreate(&wrapupTemplate,                                      // Guid of template
                        wrapupEdt,                                            // Top level function of EDT
                        sizeof(wrapup_Params_t) / sizeof(u64),                // Number of params
                        sizeof(wrapup_Deps_t) / sizeof(ocrEdtDep_t));         // Number of dependencies

   wrapup_Params_t paramsToWrapup;
   paramsToWrapup.prm_dummy = 9999;

   ocrEdtCreate(&wrapup,                        // Guid of the EDT created to start at function wrapup.
                wrapupTemplate,                 // Template for the EDT we are creating.
                EDT_PARAM_DEF,
                (u64 *) &paramsToWrapup,        // It doesn't receive any params
                EDT_PARAM_DEF,
                NULL,
                EDT_PROP_FINISH,
                NULL_GUID,
                &wrapupOutputEvent);


   realMain_Params_t paramsToRealMain;
   paramsToRealMain.prm_numberOfPes     = numberOfPes;
   paramsToRealMain.prm_numberOfObjects = numberOfObjects;

   ocrEdtCreate(&realMain,                      // Guid of the EDT created to start at function realMainEdt.
                realMainTemplate,               // Template for the EDT we are creating.
                EDT_PARAM_DEF,
                (u64 *) &paramsToRealMain,
                EDT_PARAM_DEF,
                NULL,
                EDT_PROP_FINISH,
                NULL_GUID,
                &realMainOutputEvent);

   ADD_DEPENDENCE(realMainOutputEvent, wrapup, wrapup_Deps_t, dep_DoneEvent, RO);        // Provide the wrapup EDT with the finish event out of realMain.

   ocrDbRelease  (myDeps->dep_argv.guid);
   ADD_DEPENDENCE(myDeps->dep_argv.guid, realMain, realMain_Deps_t, dep_argv, RO);       // Provide the realMain EDT with the guid for the datablock containing command line argument list.
   ADD_DEPENDENCE(sharedDb,              realMain, realMain_Deps_t, dep_shared, RW);     // Provide the realMain EDT with the guid for the shared datablock.

   return wrapupOutputEvent;                       // This EDT is done.  It has turned control over to the readmainEdt.

}

ocrGuid_t wrapupEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
printf ("wrapupEdt about to call ocrShutdown\n"); fflush(stdout);
    ocrShutdown();
    return NULL_GUID;
}



ocrGuid_t realMainEdt( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )
{
// This EDT completes the work started by mainEdt.  With the shared datablock now "seasoned", this EDT can write to it.
// It creates a labeled-guid "range" array in the shared datablock, large enough to host (numberOfPes**2) event guids.
//   * Creates the labeled-guid range arrays
//   * Creates a datablock large enough to host the descriptions of those objects.
//   * Instantiates numberOfPes copies of "realMainEdt".

   realMain_Params_t * myParams = (realMain_Params_t *) paramv;
   realMain_Deps_t   * myDeps   = (realMain_Deps_t   *) depv;

   u64 numberOfPes       = myParams->prm_numberOfPes;
   u64 numberOfObjects   = myParams->prm_numberOfObjects;
   shared_t * lcl_shared = myDeps->dep_shared.ptr;
   ocrGuid_t  sharedDb   = myDeps->dep_shared.guid;
   ocrGuid_t  argvDb     = myDeps->dep_argv.guid;

   u64 i;

   ocrGuid_t referenceVersionsMainTemplate, continuationTemplate, referenceVersionsMain, glblDb, scratchObjectsDb;
   void * dummy;

   lcl_shared->numberOfPes = numberOfPes;
   ocrGuidRangeCreate( &(lcl_shared->labeledGuidRange),
                       numberOfPes * numberOfPes * GUID_ROUND_ROBIN_SPAN,   // Reserve guids for every rank to talk to every other rank.
                       GUID_USER_EVENT_STICKY );

   ocrDbCreate ( &scratchObjectsDb, (void **) &dummy, numberOfObjects*sizeof(Object_t), 0, NULL_GUID, NO_ALLOC);

   ocrEdtTemplateCreate(&referenceVersionsMainTemplate,                                // Guid of template
                        referenceVersionsMainEdt,                                      // Top level function of EDT
                        sizeof(referenceVersionsMain_Params_t) / sizeof(u64),          // Number of parameters.
                        4);                                                            // Number of dependencies *** IS ONLY THE FIRST FOUR for the top-level EDT (of each rank).

   ocrEdtTemplateCreate(&continuationTemplate,                                         // Guid of template
                        referenceVersionsMainEdt,                                      // Top level function of EDT
                        sizeof(referenceVersionsMain_Params_t) / sizeof(u64),          // Number of parameters.
                        sizeof(referenceVersionsMain_Deps_t)   / sizeof(ocrEdtDep_t)); // ***DEFAULT*** number of dependencies. (Applicable for continuation instances; overridden for top-level instances).


   referenceVersionsMain_Params_t paramsForReferenceVersionsMain;
   paramsForReferenceVersionsMain.prm_continuationTemplate = continuationTemplate;
   paramsForReferenceVersionsMain.prm_isFirstInstance      = 1;

   ocrDbRelease (sharedDb);

   for( i = 0; i < numberOfPes; i++ ){

      paramsForReferenceVersionsMain.prm_my_pe = i;

      ocrEdtCreate(&referenceVersionsMain,         // Guid of the EDT created to start at function refMainEdt (i.e. the OCR-ified reference version of its "main" function.
                   referenceVersionsMainTemplate,  // Template for the EDT we are creating.
                   EDT_PARAM_DEF,
                   (u64 *) &paramsForReferenceVersionsMain,
                   EDT_PARAM_DEF,
                   NULL,
                   EDT_PROP_FINISH,
                   NULL_GUID,
                   NULL_GUID);

      ocrDbCreate( &glblDb, (void **)&dummy, sizeof_Globals_t, 0, NULL_GUID, NO_ALLOC );    // Create the datablock into which the instance of refMain will host its copy of glbl.

      ADD_DEPENDENCE(sharedDb,         referenceVersionsMain, referenceVersionsMain_Deps_t, dep_shared,          RO);
      ADD_DEPENDENCE(argvDb,           referenceVersionsMain, referenceVersionsMain_Deps_t, dep_argv,            RO);
      ADD_DEPENDENCE(glblDb,           referenceVersionsMain, referenceVersionsMain_Deps_t, dep_glbl,            RW);
      ADD_DEPENDENCE(scratchObjectsDb, referenceVersionsMain, referenceVersionsMain_Deps_t, dep_scratch_objects, RW);
      scratchObjectsDb = NULL_GUID;    // Only rank 0 gets this datablock;  all others just get NULL_GUID for it.

   }

   return NULL_GUID;

}

ocrGuid_t referenceVersionsMainEdt ( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] )    // For the OCR-ified version, the reference version's "main" function becomes this EDT.
{
   Globals_t * glbl = NULL;
   Dbmeta_t    dbmeta__glbl;
   int i, j;

   referenceVersionsMain_Params_t * myParams = (referenceVersionsMain_Params_t *) paramv;
   referenceVersionsMain_Deps_t   * myDeps   = (referenceVersionsMain_Deps_t   *) depv;

   glbl                = myDeps->dep_glbl.ptr;
   dbmeta__glbl.guid   = myDeps->dep_glbl.guid;
   dbmeta__glbl.size   = sizeof_Globals_t;
   if (myParams->prm_isFirstInstance) {
      memset (glbl, 0, sizeof_Globals_t);
   }

   ocrGuid_t continuationTemplate = myParams->prm_continuationTemplate;
   my_pe                               = myParams->prm_my_pe;

   shared             = myDeps->dep_shared.ptr;
   ocrGuid_t dbShared = myDeps->dep_shared.guid;
   num_pes            = shared->numberOfPes;

   Frame_Header_t * topOfStack = (Frame_Header_t *) glbl->stack;

   if (myParams->prm_isFirstInstance) {

      ocrGuid_t dbProgramArgv      = myDeps->dep_argv.guid;
      programArgv                  = myDeps->dep_argv.ptr;
      programArgc                  = getArgc(programArgv);

      scratch_objects              = myDeps->dep_scratch_objects.ptr;
      dbmeta__scratch_objects.guid = myDeps->dep_scratch_objects.guid;

      // Init pointers (other than glbl, programArgv, and scratch_objects, which are valid and received from on high) to NULL and guids to NULL_GUID for each datablock.

      dbmeta__tmpDb1              .guid = NULL_GUID; tmpDb1              = NULL;
      dbmeta__tmpDb2              .guid = NULL_GUID; tmpDb2              = NULL;
      dbmeta__tmpDb3              .guid = NULL_GUID; tmpDb3              = NULL;
      dbmeta__tmpDb4              .guid = NULL_GUID; tmpDb4              = NULL;
      dbmeta__tmpDb5              .guid = NULL_GUID; tmpDb5              = NULL;
      dbmeta__tmpDb6              .guid = NULL_GUID; tmpDb6              = NULL;
      dbmeta__tmpDb7              .guid = NULL_GUID; tmpDb7              = NULL;
      dbmeta__newResource         .guid = NULL_GUID; newResource         = NULL;
      dbmeta__bin                 .guid = NULL_GUID; bin                 = NULL;
      dbmeta__comms               .guid = NULL_GUID; comms               = NULL;
      dbmeta__me                  .guid = NULL_GUID; me                  = NULL;
      dbmeta__np                  .guid = NULL_GUID; np                  = NULL;
      dbmeta__request             .guid = NULL_GUID; request             = NULL;
      dbmeta__send_buff           .guid = NULL_GUID; send_buff           = NULL;
      dbmeta__s_req               .guid = NULL_GUID; s_req               = NULL;
      par_b.dbmeta__comm_b        .guid = NULL_GUID; par_b.comm_b        = NULL;
      par_b.dbmeta__comm_c        .guid = NULL_GUID; par_b.comm_c        = NULL;
      par_b.dbmeta__comm_part     .guid = NULL_GUID; par_b.comm_part     = NULL;
      par_b.dbmeta__comm_p        .guid = NULL_GUID; par_b.comm_p        = NULL;
      par_b.dbmeta__index         .guid = NULL_GUID; par_b.index         = NULL;
      par_b.dbmeta__par_comm_num  .guid = NULL_GUID; par_b.par_comm_num  = NULL;
      par_p1.dbmeta__comm_b       .guid = NULL_GUID; par_p1.comm_b       = NULL;
      par_p1.dbmeta__comm_c       .guid = NULL_GUID; par_p1.comm_c       = NULL;
      par_p1.dbmeta__comm_part    .guid = NULL_GUID; par_p1.comm_part    = NULL;
      par_p1.dbmeta__comm_p       .guid = NULL_GUID; par_p1.comm_p       = NULL;
      par_p1.dbmeta__index        .guid = NULL_GUID; par_p1.index        = NULL;
      par_p1.dbmeta__par_comm_num .guid = NULL_GUID; par_p1.par_comm_num = NULL;
      par_p.dbmeta__comm_b        .guid = NULL_GUID; par_p.comm_b        = NULL;
      par_p.dbmeta__comm_c        .guid = NULL_GUID; par_p.comm_c        = NULL;
      par_p.dbmeta__comm_part     .guid = NULL_GUID; par_p.comm_part     = NULL;
      par_p.dbmeta__comm_p        .guid = NULL_GUID; par_p.comm_p        = NULL;
      par_p.dbmeta__index         .guid = NULL_GUID; par_p.index         = NULL;
      par_p.dbmeta__par_comm_num  .guid = NULL_GUID; par_p.par_comm_num  = NULL;

      for (i = 0; i < 3; i++) {
         dbmeta__comm_block[i]       .guid = NULL_GUID; comm_block[i]       = NULL;
         dbmeta__comm_face_case[i]   .guid = NULL_GUID; comm_face_case[i]   = NULL;
         dbmeta__comm_index[i]       .guid = NULL_GUID; comm_index[i]       = NULL;
         dbmeta__comm_num[i]         .guid = NULL_GUID; comm_num[i]         = NULL;
         dbmeta__comm_partner[i]     .guid = NULL_GUID; comm_partner[i]     = NULL;
         dbmeta__comm_pos1[i]        .guid = NULL_GUID; comm_pos1[i]        = NULL;
         dbmeta__comm_pos[i]         .guid = NULL_GUID; comm_pos[i]         = NULL;
         dbmeta__comm_recv_off[i]    .guid = NULL_GUID; comm_recv_off[i]    = NULL;
         dbmeta__comm_send_off[i]    .guid = NULL_GUID; comm_send_off[i]    = NULL;
         dbmeta__recv_size[i]        .guid = NULL_GUID; recv_size[i]        = NULL;
         dbmeta__send_size[i]        .guid = NULL_GUID; send_size[i]        = NULL;
      }

      dbmeta__gbin                .guid = NULL_GUID; gbin                = NULL;
      dbmeta__recv_buff           .guid = NULL_GUID; recv_buff           = NULL;

      dbmeta__bulk                .guid = NULL_GUID; bulk                = NULL;

      counter_malloc = 0;
      size_malloc = 0.0;

      tos = (char *) glbl->stack;
      callingDepth = 0;
      topOfStack[0].resumption_case_num                  = -9999;                  // Irrelevant for topmost activation record.
      topOfStack[0].my_size                              = sizeof(Frame_Header_t); // Distance to first callee's frame.
      topOfStack[0].caller_size                          = -9999;                  // Irrelevant for topmost activation record.
      topOfStack[0].validate_callers_prep_for_suspension = -9999;                  // Irrelevant for topmost activation record.
      topOfStack[1].resumption_case_num                  = 0;                      // Prepare for first callee.
      topOfStack[1].my_size                              = -9999;                  // Size of callee's frame is not known by caller.
      topOfStack[1].caller_size                          = sizeof(Frame_Header_t); // To tell callee how much to pop when it returns.

   } else { // if (myParams->isFirstInstance)

contin_cnt++;
//printf ("pe = %d, %s line %d, Started continuation EDT %d\n", my_pe, __FILE__, __LINE__, contin_cnt); fflush(stdout);

      dbmeta__scratch_objects.guid          = myDeps->dep_scratch_objects.guid;           scratch_objects          = myDeps->dep_scratch_objects.ptr;
      dbmeta__bulk.guid                     = myDeps->dep_bulk.guid;                      bulk                     = myDeps->dep_bulk.ptr;
      dbmeta__tmpDb1.guid                   = myDeps->dep_tmpDb1.guid;                    tmpDb1                   = myDeps->dep_tmpDb1.ptr;
      dbmeta__tmpDb2.guid                   = myDeps->dep_tmpDb2.guid;                    tmpDb2                   = myDeps->dep_tmpDb2.ptr;
      dbmeta__tmpDb3.guid                   = myDeps->dep_tmpDb3.guid;                    tmpDb3                   = myDeps->dep_tmpDb3.ptr;
      dbmeta__tmpDb4.guid                   = myDeps->dep_tmpDb4.guid;                    tmpDb4                   = myDeps->dep_tmpDb4.ptr;
      dbmeta__tmpDb5.guid                   = myDeps->dep_tmpDb5.guid;                    tmpDb5                   = myDeps->dep_tmpDb5.ptr;
      dbmeta__tmpDb6.guid                   = myDeps->dep_tmpDb6.guid;                    tmpDb6                   = myDeps->dep_tmpDb6.ptr;
      dbmeta__tmpDb7.guid                   = myDeps->dep_tmpDb7.guid;                    tmpDb7                   = myDeps->dep_tmpDb7.ptr;
      dbmeta__newResource.guid              = myDeps->dep_newResource.guid;               newResource              = myDeps->dep_newResource.ptr;
      dbmeta__bin.guid                      = myDeps->dep_bin.guid;                       bin                      = myDeps->dep_bin.ptr;
      dbmeta__comms.guid                    = myDeps->dep_comms.guid;                     comms                    = myDeps->dep_comms.ptr;
      dbmeta__me.guid                       = myDeps->dep_me.guid;                        me                       = myDeps->dep_me.ptr;
      dbmeta__np.guid                       = myDeps->dep_np.guid;                        np                       = myDeps->dep_np.ptr;
      dbmeta__request.guid                  = myDeps->dep_request.guid;                   request                  = myDeps->dep_request.ptr;
      dbmeta__send_buff.guid                = myDeps->dep_send_buff.guid;                 send_buff                = myDeps->dep_send_buff.ptr;
      dbmeta__s_req.guid                    = myDeps->dep_s_req.guid;                     s_req                    = myDeps->dep_s_req.ptr;
      par_b.dbmeta__comm_b.guid             = myDeps->dep_par_b__comm_b.guid;             par_b.comm_b             = myDeps->dep_par_b__comm_b.ptr;
      par_b.dbmeta__comm_c.guid             = myDeps->dep_par_b__comm_c.guid;             par_b.comm_c             = myDeps->dep_par_b__comm_c.ptr;
      par_b.dbmeta__comm_part.guid          = myDeps->dep_par_b__comm_part.guid;          par_b.comm_part          = myDeps->dep_par_b__comm_part.ptr;
      par_b.dbmeta__comm_p.guid             = myDeps->dep_par_b__comm_p.guid;             par_b.comm_p             = myDeps->dep_par_b__comm_p.ptr;
      par_b.dbmeta__index.guid              = myDeps->dep_par_b__index.guid;              par_b.index              = myDeps->dep_par_b__index.ptr;
      par_b.dbmeta__par_comm_num.guid       = myDeps->dep_par_b__par_comm_num.guid;       par_b.par_comm_num       = myDeps->dep_par_b__par_comm_num.ptr;
      par_p1.dbmeta__comm_b.guid            = myDeps->dep_par_p1__comm_b.guid;            par_p1.comm_b            = myDeps->dep_par_p1__comm_b.ptr;
      par_p1.dbmeta__comm_c.guid            = myDeps->dep_par_p1__comm_c.guid;            par_p1.comm_c            = myDeps->dep_par_p1__comm_c.ptr;
      par_p1.dbmeta__comm_part.guid         = myDeps->dep_par_p1__comm_part.guid;         par_p1.comm_part         = myDeps->dep_par_p1__comm_part.ptr;
      par_p1.dbmeta__comm_p.guid            = myDeps->dep_par_p1__comm_p.guid;            par_p1.comm_p            = myDeps->dep_par_p1__comm_p.ptr;
      par_p1.dbmeta__index.guid             = myDeps->dep_par_p1__index.guid;             par_p1.index             = myDeps->dep_par_p1__index.ptr;
      par_p1.dbmeta__par_comm_num.guid      = myDeps->dep_par_p1__par_comm_num.guid;      par_p1.par_comm_num      = myDeps->dep_par_p1__par_comm_num.ptr;
      par_p.dbmeta__comm_b.guid             = myDeps->dep_par_p__comm_b.guid;             par_p.comm_b             = myDeps->dep_par_p__comm_b.ptr;
      par_p.dbmeta__comm_c.guid             = myDeps->dep_par_p__comm_c.guid;             par_p.comm_c             = myDeps->dep_par_p__comm_c.ptr;
      par_p.dbmeta__comm_part.guid          = myDeps->dep_par_p__comm_part.guid;          par_p.comm_part          = myDeps->dep_par_p__comm_part.ptr;
      par_p.dbmeta__comm_p.guid             = myDeps->dep_par_p__comm_p.guid;             par_p.comm_p             = myDeps->dep_par_p__comm_p.ptr;
      par_p.dbmeta__index.guid              = myDeps->dep_par_p__index.guid;              par_p.index              = myDeps->dep_par_p__index.ptr;
      par_p.dbmeta__par_comm_num.guid       = myDeps->dep_par_p__par_comm_num.guid;       par_p.par_comm_num       = myDeps->dep_par_p__par_comm_num.ptr;
      {
         int i;
         for (i = 0; i < 3; i++) {
            dbmeta__comm_block[i].guid      = myDeps->dep_comm_block[i].guid;             comm_block[i]            = myDeps->dep_comm_block[i].ptr;
            dbmeta__comm_face_case[i].guid  = myDeps->dep_comm_face_case[i].guid;         comm_face_case[i]        = myDeps->dep_comm_face_case[i].ptr;
            dbmeta__comm_index[i].guid      = myDeps->dep_comm_index[i].guid;             comm_index[i]            = myDeps->dep_comm_index[i].ptr;
            dbmeta__comm_num[i].guid        = myDeps->dep_comm_num[i].guid;               comm_num[i]              = myDeps->dep_comm_num[i].ptr;
            dbmeta__comm_partner[i].guid    = myDeps->dep_comm_partner[i].guid;           comm_partner[i]          = myDeps->dep_comm_partner[i].ptr;
            dbmeta__comm_pos1[i].guid       = myDeps->dep_comm_pos1[i].guid;              comm_pos1[i]             = myDeps->dep_comm_pos1[i].ptr;
            dbmeta__comm_pos[i].guid        = myDeps->dep_comm_pos[i].guid;               comm_pos[i]              = myDeps->dep_comm_pos[i].ptr;
            dbmeta__comm_recv_off[i].guid   = myDeps->dep_comm_recv_off[i].guid;          comm_recv_off[i]         = myDeps->dep_comm_recv_off[i].ptr;
            dbmeta__comm_send_off[i].guid   = myDeps->dep_comm_send_off[i].guid;          comm_send_off[i]         = myDeps->dep_comm_send_off[i].ptr;
            dbmeta__recv_size[i].guid       = myDeps->dep_recv_size[i].guid;              recv_size[i]             = myDeps->dep_recv_size[i].ptr;
            dbmeta__send_size[i].guid       = myDeps->dep_send_size[i].guid;              send_size[i]             = myDeps->dep_send_size[i].ptr;
         }
      }
      dbmeta__gbin.guid                     = myDeps->dep_gbin.guid;                      gbin                     = myDeps->dep_gbin.ptr;
      dbmeta__recv_buff.guid                = myDeps->dep_recv_buff.guid;                 recv_buff                = myDeps->dep_recv_buff.ptr;
   } // if (myParams->isFirstInstance)

   continuationOpcode = ContinuationOpcodeUnspecified;

   topOfStack[1].validate_callers_prep_for_suspension = 1;                  // Set "canary trap" for first callee.
   driver__soup_to_nuts(glbl, my_pe);
   if (topOfStack[1].resumption_case_num != 0) { // Create a continuation EDT, cause it to fire, and cause this EDT has to terminate.

      referenceVersionsMain_Params_t paramsForContinuation;
      paramsForContinuation.prm_continuationTemplate = continuationTemplate;
      paramsForContinuation.prm_isFirstInstance      = 0;
      paramsForContinuation.prm_my_pe = my_pe;

      ocrGuid_t continuation;
      ocrEdtCreate(&continuation,                  // Guid of the EDT created to continue at function refMainEdt.
                   continuationTemplate,           // Template for the EDT we are creating.
                   EDT_PARAM_DEF,
                   (u64 *) &paramsForContinuation,
                   EDT_PARAM_DEF,
                   NULL,
                   EDT_PROP_NONE,
                   NULL_GUID,
                   NULL_GUID);

//printf ("pe = %d, continuationOpcode = %d, continuationDetail = %d season_cnt=%d, recv_cnt=%d, send_cnt=%d, isend_cnt=%d\n", my_pe, continuationOpcode, continuationDetail, season_cnt, recv_cnt, send_cnt, isend_cnt); fflush(stdout);
      switch (continuationOpcode) {
      case SeasoningOneOrMoreDbCreates:
season_cnt++;
//printf ("pe = %d, %s line %d, Cloning continuation EDT %d (%d) to Season one or more datablocks\n", my_pe, __FILE__, __LINE__, season_cnt, recv_cnt+season_cnt); fflush(stdout);
         ADD_DEPENDENCE(NULL_GUID, continuation, referenceVersionsMain_Deps_t, dep_newResource, RO);
         break;
      case ReceivingACommunication:
recv_cnt++;
//printf ("pe = %d, %s line %d, Cloning continuation EDT to Receive communication %d (%d) from sender pe %d (GUID index is %d)\n", my_pe, __FILE__, __LINE__, recv_cnt, recv_cnt+season_cnt, continuationDetail, (((my_pe*num_pes+continuationDetail)*GUID_ROUND_ROBIN_SPAN)+guid_toggle__recv[continuationDetail])); fflush(stdout);
         ocrGuidFromIndex(&resource, shared->labeledGuidRange, (((my_pe*num_pes+continuationDetail)*GUID_ROUND_ROBIN_SPAN)+guid_toggle__recv[continuationDetail]));
         guid_toggle__recv[continuationDetail] = (guid_toggle__recv[continuationDetail] + 1) & (GUID_ROUND_ROBIN_SPAN - 1);
#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG
         ocrEventCreate(&resource, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS);
         ADD_DEPENDENCE(resource, continuation, referenceVersionsMain_Deps_t, dep_newResource, RO);
         break;
      case MassParallelismDone:
         printf ("pe %d is regaining control after utilizing massive OpenMP-like parallelism, such as for the stencil operation\n", my_pe); fflush(stdout);
         *((int *) 0) = 123;
         ADD_DEPENDENCE(NULL_GUID, continuation, referenceVersionsMain_Deps_t, dep_newResource, RO);
         break;
      case ContinuationOpcodeUnspecified :
         printf ("pe %d, Continuation opcode unspecified\n", my_pe); fflush(stdout);
         ocrShutdown();
         *((int *) 0) = 123;
         break;
      default:
         printf ("pe %d, Continuation opcode not yet handled\n", my_pe); fflush(stdout);
         ocrShutdown();
         *((int *) 0) = 123;
         break;
      }

      if (dbmeta__scratch_objects    .guid  != NULL_GUID) ocrDbRelease(dbmeta__scratch_objects    .guid);
      if (dbmeta__tmpDb1             .guid  != NULL_GUID) ocrDbRelease(dbmeta__tmpDb1             .guid);
      if (dbmeta__tmpDb2             .guid  != NULL_GUID) ocrDbRelease(dbmeta__tmpDb2             .guid);
      if (dbmeta__tmpDb3             .guid  != NULL_GUID) ocrDbRelease(dbmeta__tmpDb3             .guid);
      if (dbmeta__tmpDb4             .guid  != NULL_GUID) ocrDbRelease(dbmeta__tmpDb4             .guid);
      if (dbmeta__tmpDb5             .guid  != NULL_GUID) ocrDbRelease(dbmeta__tmpDb5             .guid);
      if (dbmeta__tmpDb6             .guid  != NULL_GUID) ocrDbRelease(dbmeta__tmpDb6             .guid);
      if (dbmeta__tmpDb7             .guid  != NULL_GUID) ocrDbRelease(dbmeta__tmpDb7             .guid);
      if (dbmeta__bin                .guid  != NULL_GUID) ocrDbRelease(dbmeta__bin                .guid);
      if (dbmeta__comms              .guid  != NULL_GUID) ocrDbRelease(dbmeta__comms              .guid);
      if (dbmeta__me                 .guid  != NULL_GUID) ocrDbRelease(dbmeta__me                 .guid);
      if (dbmeta__np                 .guid  != NULL_GUID) ocrDbRelease(dbmeta__np                 .guid);
      if (dbmeta__request            .guid  != NULL_GUID) ocrDbRelease(dbmeta__request            .guid);
      if (dbmeta__send_buff          .guid  != NULL_GUID) ocrDbRelease(dbmeta__send_buff          .guid);
      if (dbmeta__s_req              .guid  != NULL_GUID) ocrDbRelease(dbmeta__s_req              .guid);
      if (par_b.dbmeta__comm_b       .guid  != NULL_GUID) ocrDbRelease(par_b.dbmeta__comm_b       .guid);
      if (par_b.dbmeta__comm_c       .guid  != NULL_GUID) ocrDbRelease(par_b.dbmeta__comm_c       .guid);
      if (par_b.dbmeta__comm_part    .guid  != NULL_GUID) ocrDbRelease(par_b.dbmeta__comm_part    .guid);
      if (par_b.dbmeta__comm_p       .guid  != NULL_GUID) ocrDbRelease(par_b.dbmeta__comm_p       .guid);
      if (par_b.dbmeta__index        .guid  != NULL_GUID) ocrDbRelease(par_b.dbmeta__index        .guid);
      if (par_b.dbmeta__par_comm_num .guid  != NULL_GUID) ocrDbRelease(par_b.dbmeta__par_comm_num .guid);
      if (par_p1.dbmeta__comm_b      .guid  != NULL_GUID) ocrDbRelease(par_p1.dbmeta__comm_b      .guid);
      if (par_p1.dbmeta__comm_c      .guid  != NULL_GUID) ocrDbRelease(par_p1.dbmeta__comm_c      .guid);
      if (par_p1.dbmeta__comm_part   .guid  != NULL_GUID) ocrDbRelease(par_p1.dbmeta__comm_part   .guid);
      if (par_p1.dbmeta__comm_p      .guid  != NULL_GUID) ocrDbRelease(par_p1.dbmeta__comm_p      .guid);
      if (par_p1.dbmeta__index       .guid  != NULL_GUID) ocrDbRelease(par_p1.dbmeta__index       .guid);
      if (par_p1.dbmeta__par_comm_num.guid  != NULL_GUID) ocrDbRelease(par_p1.dbmeta__par_comm_num.guid);
      if (par_p.dbmeta__comm_b       .guid  != NULL_GUID) ocrDbRelease(par_p.dbmeta__comm_b       .guid);
      if (par_p.dbmeta__comm_c       .guid  != NULL_GUID) ocrDbRelease(par_p.dbmeta__comm_c       .guid);
      if (par_p.dbmeta__comm_part    .guid  != NULL_GUID) ocrDbRelease(par_p.dbmeta__comm_part    .guid);
      if (par_p.dbmeta__comm_p       .guid  != NULL_GUID) ocrDbRelease(par_p.dbmeta__comm_p       .guid);
      if (par_p.dbmeta__index        .guid  != NULL_GUID) ocrDbRelease(par_p.dbmeta__index        .guid);
      if (par_p.dbmeta__par_comm_num .guid  != NULL_GUID) ocrDbRelease(par_p.dbmeta__par_comm_num .guid);
      {
         int i;
         for (i = 0; i < 3; i++) {
            if (dbmeta__comm_block[i]    .guid  != NULL_GUID) ocrDbRelease(dbmeta__comm_block[i]    .guid);
            if (dbmeta__comm_face_case[i].guid  != NULL_GUID) ocrDbRelease(dbmeta__comm_face_case[i].guid);
            if (dbmeta__comm_index[i]    .guid  != NULL_GUID) ocrDbRelease(dbmeta__comm_index[i]    .guid);
            if (dbmeta__comm_num[i]      .guid  != NULL_GUID) ocrDbRelease(dbmeta__comm_num[i]      .guid);
            if (dbmeta__comm_partner[i]  .guid  != NULL_GUID) ocrDbRelease(dbmeta__comm_partner[i]  .guid);
            if (dbmeta__comm_pos1[i]     .guid  != NULL_GUID) ocrDbRelease(dbmeta__comm_pos1[i]     .guid);
            if (dbmeta__comm_pos[i]      .guid  != NULL_GUID) ocrDbRelease(dbmeta__comm_pos[i]      .guid);
            if (dbmeta__comm_recv_off[i] .guid  != NULL_GUID) ocrDbRelease(dbmeta__comm_recv_off[i] .guid);
            if (dbmeta__comm_send_off[i] .guid  != NULL_GUID) ocrDbRelease(dbmeta__comm_send_off[i] .guid);
            if (dbmeta__recv_size[i]     .guid  != NULL_GUID) ocrDbRelease(dbmeta__recv_size[i]     .guid);
            if (dbmeta__send_size[i]     .guid  != NULL_GUID) ocrDbRelease(dbmeta__send_size[i]     .guid);
         }
      }

      if (dbmeta__gbin               .guid  != NULL_GUID) ocrDbRelease(dbmeta__gbin               .guid);
      if (dbmeta__recv_buff          .guid  != NULL_GUID) ocrDbRelease(dbmeta__recv_buff          .guid);

      ADD_DEPENDENCE(dbmeta__scratch_objects    .guid, continuation, referenceVersionsMain_Deps_t, dep_scratch_objects     , RO);
      ADD_DEPENDENCE(dbmeta__tmpDb1             .guid, continuation, referenceVersionsMain_Deps_t, dep_tmpDb1              , RW);
      ADD_DEPENDENCE(dbmeta__tmpDb2             .guid, continuation, referenceVersionsMain_Deps_t, dep_tmpDb2              , RW);
      ADD_DEPENDENCE(dbmeta__tmpDb3             .guid, continuation, referenceVersionsMain_Deps_t, dep_tmpDb3              , RW);
      ADD_DEPENDENCE(dbmeta__tmpDb4             .guid, continuation, referenceVersionsMain_Deps_t, dep_tmpDb4              , RW);
      ADD_DEPENDENCE(dbmeta__tmpDb5             .guid, continuation, referenceVersionsMain_Deps_t, dep_tmpDb5              , RW);
      ADD_DEPENDENCE(dbmeta__tmpDb6             .guid, continuation, referenceVersionsMain_Deps_t, dep_tmpDb6              , RW);
      ADD_DEPENDENCE(dbmeta__tmpDb7             .guid, continuation, referenceVersionsMain_Deps_t, dep_tmpDb7              , RW);
      ADD_DEPENDENCE(dbmeta__bin                .guid, continuation, referenceVersionsMain_Deps_t, dep_bin                 , RW);
      ADD_DEPENDENCE(dbmeta__comms              .guid, continuation, referenceVersionsMain_Deps_t, dep_comms               , RW);
      ADD_DEPENDENCE(dbmeta__me                 .guid, continuation, referenceVersionsMain_Deps_t, dep_me                  , RW);
      ADD_DEPENDENCE(dbmeta__np                 .guid, continuation, referenceVersionsMain_Deps_t, dep_np                  , RW);
      ADD_DEPENDENCE(dbmeta__request            .guid, continuation, referenceVersionsMain_Deps_t, dep_request             , RW);
      ADD_DEPENDENCE(dbmeta__send_buff          .guid, continuation, referenceVersionsMain_Deps_t, dep_send_buff           , RW);
      ADD_DEPENDENCE(dbmeta__s_req              .guid, continuation, referenceVersionsMain_Deps_t, dep_s_req               , RW);
      ADD_DEPENDENCE(par_b.dbmeta__comm_b       .guid, continuation, referenceVersionsMain_Deps_t, dep_par_b__comm_b       , RW);
      ADD_DEPENDENCE(par_b.dbmeta__comm_c       .guid, continuation, referenceVersionsMain_Deps_t, dep_par_b__comm_c       , RW);
      ADD_DEPENDENCE(par_b.dbmeta__comm_part    .guid, continuation, referenceVersionsMain_Deps_t, dep_par_b__comm_part    , RW);
      ADD_DEPENDENCE(par_b.dbmeta__comm_p       .guid, continuation, referenceVersionsMain_Deps_t, dep_par_b__comm_p       , RW);
      ADD_DEPENDENCE(par_b.dbmeta__index        .guid, continuation, referenceVersionsMain_Deps_t, dep_par_b__index        , RW);
      ADD_DEPENDENCE(par_b.dbmeta__par_comm_num .guid, continuation, referenceVersionsMain_Deps_t, dep_par_b__par_comm_num , RW);
      ADD_DEPENDENCE(par_p1.dbmeta__comm_b      .guid, continuation, referenceVersionsMain_Deps_t, dep_par_p1__comm_b      , RW);
      ADD_DEPENDENCE(par_p1.dbmeta__comm_c      .guid, continuation, referenceVersionsMain_Deps_t, dep_par_p1__comm_c      , RW);
      ADD_DEPENDENCE(par_p1.dbmeta__comm_part   .guid, continuation, referenceVersionsMain_Deps_t, dep_par_p1__comm_part   , RW);
      ADD_DEPENDENCE(par_p1.dbmeta__comm_p      .guid, continuation, referenceVersionsMain_Deps_t, dep_par_p1__comm_p      , RW);
      ADD_DEPENDENCE(par_p1.dbmeta__index       .guid, continuation, referenceVersionsMain_Deps_t, dep_par_p1__index       , RW);
      ADD_DEPENDENCE(par_p1.dbmeta__par_comm_num.guid, continuation, referenceVersionsMain_Deps_t, dep_par_p1__par_comm_num, RW);
      ADD_DEPENDENCE(par_p.dbmeta__comm_b       .guid, continuation, referenceVersionsMain_Deps_t, dep_par_p__comm_b       , RW);
      ADD_DEPENDENCE(par_p.dbmeta__comm_c       .guid, continuation, referenceVersionsMain_Deps_t, dep_par_p__comm_c       , RW);
      ADD_DEPENDENCE(par_p.dbmeta__comm_part    .guid, continuation, referenceVersionsMain_Deps_t, dep_par_p__comm_part    , RW);
      ADD_DEPENDENCE(par_p.dbmeta__comm_p       .guid, continuation, referenceVersionsMain_Deps_t, dep_par_p__comm_p       , RW);
      ADD_DEPENDENCE(par_p.dbmeta__index        .guid, continuation, referenceVersionsMain_Deps_t, dep_par_p__index        , RW);
      ADD_DEPENDENCE(par_p.dbmeta__par_comm_num .guid, continuation, referenceVersionsMain_Deps_t, dep_par_p__par_comm_num , RW);
      {
         int i;
         for (i = 0; i < 3; i++) {
            ADD_DEPENDENCE(dbmeta__comm_block[i]    .guid, continuation, referenceVersionsMain_Deps_t, dep_comm_block[i]    , RW);
            ADD_DEPENDENCE(dbmeta__comm_face_case[i].guid, continuation, referenceVersionsMain_Deps_t, dep_comm_face_case[i], RW);
            ADD_DEPENDENCE(dbmeta__comm_index[i]    .guid, continuation, referenceVersionsMain_Deps_t, dep_comm_index[i]    , RW);
            ADD_DEPENDENCE(dbmeta__comm_num[i]      .guid, continuation, referenceVersionsMain_Deps_t, dep_comm_num[i]      , RW);
            ADD_DEPENDENCE(dbmeta__comm_partner[i]  .guid, continuation, referenceVersionsMain_Deps_t, dep_comm_partner[i]  , RW);
            ADD_DEPENDENCE(dbmeta__comm_pos1[i]     .guid, continuation, referenceVersionsMain_Deps_t, dep_comm_pos1[i]     , RW);
            ADD_DEPENDENCE(dbmeta__comm_pos[i]      .guid, continuation, referenceVersionsMain_Deps_t, dep_comm_pos[i]      , RW);
            ADD_DEPENDENCE(dbmeta__comm_recv_off[i] .guid, continuation, referenceVersionsMain_Deps_t, dep_comm_recv_off[i] , RW);
            ADD_DEPENDENCE(dbmeta__comm_send_off[i] .guid, continuation, referenceVersionsMain_Deps_t, dep_comm_send_off[i] , RW);
            ADD_DEPENDENCE(dbmeta__recv_size[i]     .guid, continuation, referenceVersionsMain_Deps_t, dep_recv_size[i]     , RW);
            ADD_DEPENDENCE(dbmeta__send_size[i]     .guid, continuation, referenceVersionsMain_Deps_t, dep_send_size[i]     , RW);
         }
      }

      ADD_DEPENDENCE(dbmeta__gbin               .guid, continuation, referenceVersionsMain_Deps_t, dep_gbin                , RW);
      ADD_DEPENDENCE(dbmeta__recv_buff          .guid, continuation, referenceVersionsMain_Deps_t, dep_recv_buff           , RW);

      dbmeta__tmpDb1             .guid = NULL_GUID; tmpDb1              = NULL;
      dbmeta__tmpDb2             .guid = NULL_GUID; tmpDb2              = NULL;
      dbmeta__tmpDb3             .guid = NULL_GUID; tmpDb3              = NULL;
      dbmeta__tmpDb4             .guid = NULL_GUID; tmpDb4              = NULL;
      dbmeta__tmpDb5             .guid = NULL_GUID; tmpDb5              = NULL;
      dbmeta__tmpDb6             .guid = NULL_GUID; tmpDb6              = NULL;
      dbmeta__tmpDb7             .guid = NULL_GUID; tmpDb7              = NULL;
      dbmeta__newResource        .guid = NULL_GUID; newResource         = NULL;
      dbmeta__bin                .guid = NULL_GUID; bin                 = NULL;
      dbmeta__comms              .guid = NULL_GUID; comms               = NULL;
      dbmeta__me                 .guid = NULL_GUID; me                  = NULL;
      dbmeta__np                 .guid = NULL_GUID; np                  = NULL;
      dbmeta__request            .guid = NULL_GUID; request             = NULL;
      dbmeta__send_buff          .guid = NULL_GUID; send_buff           = NULL;
      dbmeta__s_req              .guid = NULL_GUID; s_req               = NULL;
      par_b.dbmeta__comm_b       .guid = NULL_GUID; par_b.comm_b        = NULL;
      par_b.dbmeta__comm_c       .guid = NULL_GUID; par_b.comm_c        = NULL;
      par_b.dbmeta__comm_part    .guid = NULL_GUID; par_b.comm_part     = NULL;
      par_b.dbmeta__comm_p       .guid = NULL_GUID; par_b.comm_p        = NULL;
      par_b.dbmeta__index        .guid = NULL_GUID; par_b.index         = NULL;
      par_b.dbmeta__par_comm_num .guid = NULL_GUID; par_b.par_comm_num  = NULL;
      par_p1.dbmeta__comm_b      .guid = NULL_GUID; par_p1.comm_b       = NULL;
      par_p1.dbmeta__comm_c      .guid = NULL_GUID; par_p1.comm_c       = NULL;
      par_p1.dbmeta__comm_part   .guid = NULL_GUID; par_p1.comm_part    = NULL;
      par_p1.dbmeta__comm_p      .guid = NULL_GUID; par_p1.comm_p       = NULL;
      par_p1.dbmeta__index       .guid = NULL_GUID; par_p1.index        = NULL;
      par_p1.dbmeta__par_comm_num.guid = NULL_GUID; par_p1.par_comm_num = NULL;
      par_p.dbmeta__comm_b       .guid = NULL_GUID; par_p.comm_b        = NULL;
      par_p.dbmeta__comm_c       .guid = NULL_GUID; par_p.comm_c        = NULL;
      par_p.dbmeta__comm_part    .guid = NULL_GUID; par_p.comm_part     = NULL;
      par_p.dbmeta__comm_p       .guid = NULL_GUID; par_p.comm_p        = NULL;
      par_p.dbmeta__index        .guid = NULL_GUID; par_p.index         = NULL;
      par_p.dbmeta__par_comm_num .guid = NULL_GUID; par_p.par_comm_num  = NULL;
      {
         int i;
         for (i = 0; i < 3; i++) {
            dbmeta__comm_block[i]      .guid = NULL_GUID; comm_block[i]       = NULL;
            dbmeta__comm_face_case[i]  .guid = NULL_GUID; comm_face_case[i]   = NULL;
            dbmeta__comm_index[i]      .guid = NULL_GUID; comm_index[i]       = NULL;
            dbmeta__comm_num[i]        .guid = NULL_GUID; comm_num[i]         = NULL;
            dbmeta__comm_partner[i]    .guid = NULL_GUID; comm_partner[i]     = NULL;
            dbmeta__comm_pos1[i]       .guid = NULL_GUID; comm_pos1[i]        = NULL;
            dbmeta__comm_pos[i]        .guid = NULL_GUID; comm_pos[i]         = NULL;
            dbmeta__comm_recv_off[i]   .guid = NULL_GUID; comm_recv_off[i]    = NULL;
            dbmeta__comm_send_off[i]   .guid = NULL_GUID; comm_send_off[i]    = NULL;
            dbmeta__recv_size[i]       .guid = NULL_GUID; recv_size[i]        = NULL;
            dbmeta__send_size[i]       .guid = NULL_GUID; send_size[0]        = NULL;
         }
      }
      dbmeta__gbin               .guid = NULL_GUID; gbin                = NULL;
      dbmeta__recv_buff          .guid = NULL_GUID; recv_buff           = NULL;

      if (dbmeta__bulk                .guid  != NULL_GUID) ocrDbRelease(dbmeta__bulk               .guid);
      ADD_DEPENDENCE(dbmeta__bulk               .guid, continuation, referenceVersionsMain_Deps_t, dep_bulk               , RW);
      dbmeta__bulk.guid = NULL_GUID;
      bulk = NULL;

      ADD_DEPENDENCE(dbShared,         continuation, referenceVersionsMain_Deps_t, dep_shared,          RO);

      if (dbmeta__glbl                .guid  != NULL_GUID) ocrDbRelease(dbmeta__glbl               .guid);
      ADD_DEPENDENCE(dbmeta__glbl               .guid, continuation, referenceVersionsMain_Deps_t, dep_glbl               , RW);
      dbmeta__glbl.guid = NULL_GUID;
      glbl = NULL;


      // Just to test to make sure that all pointers to base address of datablocks are re-initialized from guids, clobber thos pointers here.
      // Tell the new EDT that it is NOT the initial instance (but rather, is a continuation) so that it will know to receive the new resource.

   } else {  // (topOfStack[1].resumption_case_num != 0)    If that was the very last iteration, clean up and shut down.
//int peNum = my_pe;
//printf ("pe = %d, %s line %d, Terminating.\n", peNum, __FILE__, __LINE__); fflush(stdout);
      ocrDbDestroy(dbmeta__glbl.guid);
//printf ("pe = %d, %s line %d, Terminating.\n", peNum, __FILE__, __LINE__); fflush(stdout);
   }
   return NULL_GUID;
}

#else

int main(int argc, char** argv_arg)
{

   Globals_t * glbl = NULL;
   Dbmeta_t    dbmeta__glbl;
   int tmp_my_pe, tmp_num_pes;
   int i, j;
   #undef  getArgv
   #define getArgv(junk, index)(programArgv[index])
   int ierr;
   ierr = MPI_Init(&argc, &argv_arg);
   ierr = MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_ARE_FATAL);
   ierr = MPI_Comm_rank(MPI_COMM_WORLD, &tmp_my_pe);
   ierr = MPI_Comm_size(MPI_COMM_WORLD, &tmp_num_pes);

   glbl = (Globals_t *) ma_malloc(glbl, sizeof_Globals_t, __FILE__, __LINE__);
   dbmeta__glbl.guid = 9999;
   dbmeta__glbl.size = sizeof_Globals_t;
   //dbmeta__programArgv.guid = 9999;
   memset (glbl, 0, sizeof_Globals_t);
   programArgv              = argv_arg;
   programArgc              = argc;

   counter_malloc = 0;
   size_malloc = 0.0;
   my_pe   = tmp_my_pe;
   num_pes = tmp_num_pes;

   tos = (char *) glbl->stack;
   callingDepth = 0;
   Frame_Header_t * topOfStack = (Frame_Header_t *) glbl->stack;
   topOfStack[0].resumption_case_num                  = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[0].my_size                              = sizeof(Frame_Header_t); // Distance to first callee's frame.
   topOfStack[0].caller_size                          = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[0].validate_callers_prep_for_suspension = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[1].resumption_case_num                  = 0;                      // Prepare for first callee.
   topOfStack[1].my_size                              = -9999;                  // Size of callee's frame is not known by caller.
   topOfStack[1].caller_size                          = sizeof(Frame_Header_t); // To tell callee how much to pop when it returns.

   do {  // Call the soup-to-nuts driver first time (when resumption_case_num == 0), and then repetitively until resumption_case_num comes back as zero again, signaling being done
      //printf ("PE %d calling driver, resumption case %d\n", my_pe, topOfStack[1].resumption_case_num); fflush(stdout);
      topOfStack[1].validate_callers_prep_for_suspension = 1;                  // Set "canary trap" for first callee.
      driver__soup_to_nuts(glbl, tmp_my_pe);
//printf ("pe = %d, continuationOpcode = %d, continuationDetail = %d season_cnt=%d, recv_cnt=%d, send_cnt=%d, isend_cnt=%d\n", my_pe, continuationOpcode, continuationDetail, season_cnt, recv_cnt, send_cnt, isend_cnt); fflush(stdout);

      const unsigned long long BAD  = 0xBAD0000000000001LL;
      typedef unsigned long long ULL;
      if(((ULL) tmpDb1)               & 1) *((ULL *) &tmpDb1)               ^= BAD;
      if(((ULL) tmpDb2)               & 1) *((ULL *) &tmpDb2)               ^= BAD;
      if(((ULL) tmpDb3)               & 1) *((ULL *) &tmpDb3)               ^= BAD;
      if(((ULL) tmpDb4)               & 1) *((ULL *) &tmpDb4)               ^= BAD;
      if(((ULL) tmpDb5)               & 1) *((ULL *) &tmpDb5)               ^= BAD;
      if(((ULL) tmpDb6)               & 1) *((ULL *) &tmpDb6)               ^= BAD;
      if(((ULL) tmpDb7)               & 1) *((ULL *) &tmpDb7)               ^= BAD;
      if(((ULL) newResource)          & 1) *((ULL *) &newResource)          ^= BAD;
      if(((ULL) bin)                  & 1) *((ULL *) &bin)                  ^= BAD;
      if(((ULL) comms)                & 1) *((ULL *) &comms)                ^= BAD;
      if(((ULL) me)                   & 1) *((ULL *) &me)                   ^= BAD;
      if(((ULL) np)                   & 1) *((ULL *) &np)                   ^= BAD;
      if(((ULL) request)              & 1) *((ULL *) &request)              ^= BAD;
      if(((ULL) scratch_objects)      & 1) *((ULL *) &scratch_objects)      ^= BAD;
      if(((ULL) send_buff)            & 1) *((ULL *) &send_buff)            ^= BAD;
      if(((ULL) s_req)                & 1) *((ULL *) &s_req)                ^= BAD;
      if(((ULL) programArgv)          & 1) *((ULL *) &programArgv)          ^= BAD;
      if(((ULL) par_b.comm_b)         & 1) *((ULL *) &par_b.comm_b)         ^= BAD;
      if(((ULL) par_b.comm_c)         & 1) *((ULL *) &par_b.comm_c)         ^= BAD;
      if(((ULL) par_b.comm_part)      & 1) *((ULL *) &par_b.comm_part)      ^= BAD;
      if(((ULL) par_b.comm_p)         & 1) *((ULL *) &par_b.comm_p)         ^= BAD;
      if(((ULL) par_b.index)          & 1) *((ULL *) &par_b.index)          ^= BAD;
      if(((ULL) par_b.par_comm_num)   & 1) *((ULL *) &par_b.par_comm_num)   ^= BAD;
      if(((ULL) par_p1.comm_b)        & 1) *((ULL *) &par_p1.comm_b)        ^= BAD;
      if(((ULL) par_p1.comm_c)        & 1) *((ULL *) &par_p1.comm_c)        ^= BAD;
      if(((ULL) par_p1.comm_part)     & 1) *((ULL *) &par_p1.comm_part)     ^= BAD;
      if(((ULL) par_p1.comm_p)        & 1) *((ULL *) &par_p1.comm_p)        ^= BAD;
      if(((ULL) par_p1.index)         & 1) *((ULL *) &par_p1.index)         ^= BAD;
      if(((ULL) par_p1.par_comm_num)  & 1) *((ULL *) &par_p1.par_comm_num)  ^= BAD;
      if(((ULL) par_p.comm_b)         & 1) *((ULL *) &par_p.comm_b)         ^= BAD;
      if(((ULL) par_p.comm_c)         & 1) *((ULL *) &par_p.comm_c)         ^= BAD;
      if(((ULL) par_p.comm_part)      & 1) *((ULL *) &par_p.comm_part)      ^= BAD;
      if(((ULL) par_p.comm_p)         & 1) *((ULL *) &par_p.comm_p)         ^= BAD;
      if(((ULL) par_p.index)          & 1) *((ULL *) &par_p.index)          ^= BAD;
      if(((ULL) par_p.par_comm_num)   & 1) *((ULL *) &par_p.par_comm_num)   ^= BAD;

      for (i = 0; i < 3; i++) {
         if(((ULL) comm_block[i])        & 1) *((ULL *) &comm_block[i])        ^= BAD;
         if(((ULL) comm_face_case[i])    & 1) *((ULL *) &comm_face_case[i])    ^= BAD;
         if(((ULL) comm_index[i])        & 1) *((ULL *) &comm_index[i])        ^= BAD;
         if(((ULL) comm_num[i])          & 1) *((ULL *) &comm_num[i])          ^= BAD;
         if(((ULL) comm_partner[i])      & 1) *((ULL *) &comm_partner[i])      ^= BAD;
         if(((ULL) comm_pos1[i])         & 1) *((ULL *) &comm_pos1[i])         ^= BAD;
         if(((ULL) comm_pos[i])          & 1) *((ULL *) &comm_pos[i])          ^= BAD;
         if(((ULL) comm_recv_off[i])     & 1) *((ULL *) &comm_recv_off[i])     ^= BAD;
         if(((ULL) comm_send_off[i])     & 1) *((ULL *) &comm_send_off[i])     ^= BAD;
         if(((ULL) recv_size[i])         & 1) *((ULL *) &recv_size[i])         ^= BAD;
         if(((ULL) send_size[i])         & 1) *((ULL *) &send_size[i])         ^= BAD;
      }

      //  NOT Perturbing any of the DBs that receive results of asynchronous MPI communications, because if the perturbation
      //  happens in the interval between gasket__mpi_iRecv and ...Wait..., the code will crash.   Crosstalk between these
      //  two mpi functions is a bit opaque, and moving (perturbing) the storage causes the wait function to fail.  It isn't
      //  necessary to model this anyway; OCR version will handle iRecv / Wait as receiving a datablock dependency on a
      //  continuation EDT.  The general ability/proclivity of OCR to move datablocks from one continuation EDT to the next
      //  (necessitating restoring pointers from one to the next) is adequately unit tested just by perturbing the other DBs.
      if(((ULL) gbin)                 & 1) *((ULL *) &gbin)                 ^= BAD;
      if(((ULL) recv_buff)            & 1) *((ULL *) &recv_buff)            ^= BAD;

      // Okay, now do bulk and glbl.
      if(((ULL) bulk)                 & 1) *((ULL *) &bulk)                 ^= BAD;
      if(((ULL) glbl)                 & 1) *((ULL *) &glbl)                 ^= BAD;

   } while (topOfStack[1].resumption_case_num != 0);

   free(glbl);
   MPI_Finalize();
   exit(0);
}
#endif


void driver__soup_to_nuts(Globals_t * const glbl, int const tmp_my_pe)
{
   typedef struct {
      Frame_Header_t myFrame;
      int i, object_num;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__driver__soup_to_nuts_t;

#define i                       (lcl->i)
#define object_num              (lcl->object_num)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__driver__soup_to_nuts_t)

#include "param.h"
   /* set initial values */
   if (!my_pe) {
      for (i = 1; i < programArgc; i++) {
         if (!strcmp(getArgv(programArgv, i), "--max_blocks"))
            max_num_blocks = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--target_active"))
            target_active = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--target_max"))
            target_max = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--target_min"))
            target_min = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--num_refine"))
            num_refine = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--block_change"))
            block_change = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--uniform_refine"))
            uniform_refine = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--nx"))
            x_block_size = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--ny"))
            y_block_size = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--nz"))
            z_block_size = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--num_vars"))
            num_vars = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--comm_vars"))
            comm_vars = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--init_x"))
            init_block_x = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--init_y"))
            init_block_y = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--init_z"))
            init_block_z = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--reorder"))
            reorder = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--npx"))
            npx = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--npy"))
            npy = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--npz"))
            npz = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--inbalance"))
            inbalance = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--lb_opt"))
            lb_opt = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--refine_freq"))
            refine_freq = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--report_diffusion"))
            report_diffusion = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--error_tol"))
            error_tol = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--num_tsteps"))
            num_tsteps = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--stages_per_ts"))
            stages_per_ts = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--checksum_freq"))
            checksum_freq = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--stencil"))
            stencil = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--permute"))
            permute = 1;
         else if (!strcmp(getArgv(programArgv, i), "--report_perf"))
            report_perf = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--plot_freq"))
            plot_freq = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--code"))
            code = atoi(getArgv(programArgv, ++i));
         else if (!strcmp(getArgv(programArgv, i), "--blocking_send"))
            nonblocking = 0;
         else if (!strcmp(getArgv(programArgv, i), "--refine_ghost"))
            refine_ghost = 1;
         else if (!strcmp(getArgv(programArgv, i), "--num_objects")) {
            num_objects = atoi(getArgv(programArgv, ++i));
#ifndef BUILD_OCR_VERSION
            gasket__ma_malloc(glbl, &dbmeta__scratch_objects, (void *) &scratch_objects, num_objects*sizeof(Object_t), __FILE__, __LINE__);
            SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.
#endif
            object_num = 0;
         } else if (!strcmp(getArgv(programArgv, i), "--object")) {
            if (object_num >= num_objects) {
               printf("object number greater than num_objects\n"); fflush(stdout);
               exit(-1);
            }
            scratch_objects[object_num].type = atoi(getArgv(programArgv, ++i));
            scratch_objects[object_num].bounce = atoi(getArgv(programArgv, ++i));
            scratch_objects[object_num].cen[0] = atof(getArgv(programArgv, ++i));
            scratch_objects[object_num].cen[1] = atof(getArgv(programArgv, ++i));
            scratch_objects[object_num].cen[2] = atof(getArgv(programArgv, ++i));
            scratch_objects[object_num].move[0] = atof(getArgv(programArgv, ++i));
            scratch_objects[object_num].move[1] = atof(getArgv(programArgv, ++i));
            scratch_objects[object_num].move[2] = atof(getArgv(programArgv, ++i));
            scratch_objects[object_num].size[0] = atof(getArgv(programArgv, ++i));
            scratch_objects[object_num].size[1] = atof(getArgv(programArgv, ++i));
            scratch_objects[object_num].size[2] = atof(getArgv(programArgv, ++i));
            scratch_objects[object_num].inc[0] = atof(getArgv(programArgv, ++i));
            scratch_objects[object_num].inc[1] = atof(getArgv(programArgv, ++i));
            scratch_objects[object_num].inc[2] = atof(getArgv(programArgv, ++i));
            object_num++;
         } else if (!strcmp(getArgv(programArgv, i), "--help")) {
            print_help_message(glbl);
#ifdef BUILD_OCR_VERSION
            ocrShutdown();
#else
            MPI_Abort(MPI_COMM_WORLD, -1);
#endif
         } else {
            printf("** Error ** Unknown input parameter %s\n", getArgv(programArgv, i)); fflush(stdout);
            print_help_message(glbl);
#ifdef BUILD_OCR_VERSION
            ocrShutdown();
#else
            MPI_Abort(MPI_COMM_WORLD, -1);
#endif
         }
      }
      if (check_input(glbl))
         exit(-1);

      if (!block_change)
         block_change = num_refine;

      max_num_parents = max_num_blocks;  // Guess at number needed
      max_num_dots = 2*max_num_blocks;   // Guess at number needed

      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Bcast(glbl, glbl, sizeof_glbl_to_broadcast, MPI_BYTE, 0, MPI_COMM_WORLD, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)

      gasket__ma_malloc(glbl, &dbmeta__bulk, (void *) &bulk, sizeof_Bulk_t, __FILE__, __LINE__);
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.

      OBTAIN_ACCESS_TO_objects

      for (object_num = 0; object_num < num_objects; object_num++) {
         objects[object_num] = scratch_objects[object_num];
         for (i = 0; i < 3; i++) {
            objects[object_num].orig_cen[i]  = objects[object_num].cen[i];
            objects[object_num].orig_move[i] = objects[object_num].move[i];
            objects[object_num].orig_size[i] = objects[object_num].size[i];
         }
      }
      gasket__free(&dbmeta__scratch_objects, (void **) &scratch_objects, __FILE__, __LINE__);

      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Bcast(glbl, bulk, sizeof_Bulk_t, MPI_BYTE, 0, MPI_COMM_WORLD, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)



   } else {
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Bcast(glbl, glbl, sizeof_glbl_to_broadcast, MPI_BYTE, 0, MPI_COMM_WORLD, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      my_pe = tmp_my_pe;

      gasket__ma_malloc(glbl, &dbmeta__bulk, (void *) &bulk, sizeof_Bulk_t, __FILE__, __LINE__);
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Bcast(glbl, bulk, sizeof_Bulk_t, MPI_BYTE, 0, MPI_COMM_WORLD, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)

   }


   CALL_SUSPENDABLE_CALLEE(1)
   allocate(glbl);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)

   CALL_SUSPENDABLE_CALLEE(1)
   driver(glbl);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)

   CALL_SUSPENDABLE_CALLEE(1)
   profile(glbl);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)

   deallocate(glbl);

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

#undef  i
#undef  object_num
#undef  scratch_objects
#undef  dbmeta__scratch_objects
}


// =================================== print_help_message ====================

void print_help_message(Globals_t * const glbl)
{
   printf("(Optional) command line input is of the form: \n\n");

   printf("--nx - block size x (even && > 0)\n");
   printf("--ny - block size y (even && > 0)\n");
   printf("--nz - block size z (even && > 0)\n");
   printf("--init_x - initial blocks in x (> 0)\n");
   printf("--init_y - initial blocks in y (> 0)\n");
   printf("--init_z - initial blocks in z (> 0)\n");
   printf("--reorder - ordering of blocks if initial number > 1\n");
   printf("--npx - (0 < npx <= num_pes)\n");
   printf("--npy - (0 < npy <= num_pes)\n");
   printf("--npz - (0 < npz <= num_pes)\n");
   printf("--max_blocks - maximun number of blocks per core\n");
   printf("--num_refine - (>= 0) number of levels of refinement\n");
   printf("--block_change - (>= 0) number of levels a block can change in a timestep\n");
   printf("--uniform_refine - if 1, then grid is uniformly refined\n");
   printf("--refine_freq - frequency (in timesteps) of checking for refinement\n");
   printf("--target_active - (>= 0) target number of blocks per core, none if 0\n");
   printf("--target_max - (>= 0) max number of blocks per core, none if 0\n");
   printf("--target_min - (>= 0) min number of blocks per core, none if 0\n");
   printf("--inbalance - percentage inbalance to trigger inbalance\n");
   printf("--lb_opt - load balancing - 0 = none, 1 = each refine, 2 = each refine phase\n");
   printf("--num_vars - number of variables (> 0)\n");
   printf("--comm_vars - number of vars to communicate together\n");
   printf("--num_tsteps - number of timesteps (> 0)\n");
   printf("--stages_per_ts - number of comm/calc stages per timestep\n");
   printf("--checksum_freq - number of stages between checksums\n");
   printf("--stencil - 7 or 27 point (27 will not work with refinement (except uniform))\n");
   printf("--error_tol - (e^{-error_tol} ; >= 0) \n");
   printf("--report_diffusion - (>= 0) none if 0\n");
   printf("--report_perf - 0, 1, 2\n");
   printf("--plot_freq - frequency (timesteps) of plotting (0 for none)\n");
   printf("--code - closely minic communication of different codes\n");
   printf("         0 minimal sends, 1 send ghosts, 2 send ghosts and process on send\n");
   printf("--permute - altenates directions in communication\n");
   printf("--blocking_send - use blocking sends instead of nonblocking\n");
   printf("--refine_ghost - use full extent of block (including ghosts) to determine if block is refined\n");
   printf("--num_objects - (>= 0) number of objects to cause refinement\n");
   printf("--object - type, position, movement, size, size rate of change\n");

   printf("All associated settings are integers except for objects\n"); fflush(stdout);
}

// =================================== allocate ==============================

void allocate(Globals_t * const glbl)
{
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_dots

   typedef struct {
      Frame_Header_t myFrame;
      int i, j, k, m, n;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__allocate_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define k                 (lcl->k)
#define m                 (lcl->m)
#define n                 (lcl->n)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__allocate_t)

   // Now part of global: num_blocks = (int *) ma_malloc(glbl, (num_refine+1)*sizeof(int), __FILE__, __LINE__);
   num_blocks[0] = num_pes*init_block_x*init_block_y*init_block_z;
   // Now part of global: local_num_blocks = (int *) ma_malloc(glbl, (num_refine+1)*sizeof(int), __FILE__, __LINE__);
   local_num_blocks[0] = init_block_x*init_block_y*init_block_z;

   // Now part of Bulk_t:  blocks = (Block_t *) ma_malloc(glbl, max_num_blocks*sizeof(Block_t), __FILE__, __LINE__);

   for (n = 0; n < max_num_blocks; n++) {
      blocks[n].number = -1;
#if 0
Cell space is now part of Bulk_t
      blocks[n].array = (double ****) ma_malloc(glbl, num_vars*sizeof(double ***),
                                                __FILE__, __LINE__);
      for (m = 0; m < num_vars; m++) {
         blocks[n].array[m] = (double ***)
                              ma_malloc(glbl, (x_block_size+2)*sizeof(double **),
                                        __FILE__, __LINE__);
         for (i = 0; i < x_block_size+2; i++) {
            blocks[n].array[m][i] = (double **)
                                   ma_malloc(glbl, (y_block_size+2)*sizeof(double *),
                                             __FILE__, __LINE__);
            for (j = 0; j < y_block_size+2; j++)
               blocks[n].array[m][i][j] = (double *)
                                     ma_malloc(glbl, (z_block_size+2)*sizeof(double),
                                               __FILE__, __LINE__);
         }
      }
#endif
   }

   // Now part of Bulk_t:  sorted_list = (sorted_block *)ma_malloc(glbl, max_num_blocks*sizeof(sorted_block), __FILE__, __LINE__);
   // Now part of global: sorted_index = (int *) ma_malloc(glbl, (num_refine+2)*sizeof(int), __FILE__, __LINE__);

   // Now part of Bulk_t:  parents = (Parent_t *) ma_malloc(glbl, max_num_parents*sizeof(Parent_t), __FILE__, __LINE__);
   {
      OBTAIN_ACCESS_TO_parents
      for (n = 0; n < max_num_parents; n++)
         parents[n].number = -1;
   }

   // Now part of Bulk_t:  dots = (Dot_t *) ma_malloc(glbl, max_num_dots*sizeof(Dot_t), __FILE__, __LINE__);
   for (n = 0; n < max_num_dots; n++)
      dots[n].number = -1;

   // Now part of Bulk_t:  grid_sum = (double *)ma_malloc(glbl, num_vars*sizeof(double), __FILE__, __LINE__);

   // Not used: p8 = (int *) ma_malloc(glbl, (num_refine+2)*sizeof(int), __FILE__, __LINE__);
   // Now part of global: p2 = (int *) ma_malloc(glbl, (num_refine+2)*sizeof(int), __FILE__, __LINE__);
   // Now part of global: block_start = (int *) ma_malloc(glbl, (num_refine+1)*sizeof(int), __FILE__, __LINE__);

   // Now part of Bulk_t: from = (int *) ma_malloc(glbl, num_pes*sizeof(int), __FILE__, __LINE__);
   // Now part of Bulk_t: to   = (int *) ma_malloc(glbl, num_pes*sizeof(int), __FILE__, __LINE__);

   // first try at allocating comm arrays
   for (i = 0; i < 3; i++) {
      if (num_refine)
         max_comm_part[i] = 20;
      else
         max_comm_part[i] = 2;
      gasket__ma_malloc(glbl, &dbmeta__comm_partner[i], (void *) &comm_partner[i], max_comm_part[i]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__send_size[i], (void *) &send_size[i], max_comm_part[i]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__recv_size[i], (void *) &recv_size[i], max_comm_part[i]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__comm_index[i], (void *) &comm_index[i], max_comm_part[i]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__comm_num[i], (void *) &comm_num[i], max_comm_part[i]*sizeof(int), __FILE__, __LINE__);
      if (num_refine)
         max_num_cases[i] = 100;
      else if (i == 0)
         max_num_cases[i] = 2*init_block_y*init_block_z;
      else if (i == 1)
         max_num_cases[i] = 2*init_block_x*init_block_z;
      else
         max_num_cases[i] = 2*init_block_x*init_block_y;
      gasket__ma_malloc(glbl, &dbmeta__comm_block[i], (void *) &comm_block[i], max_num_cases[i]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__comm_face_case[i], (void *) &comm_face_case[i], max_num_cases[i]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__comm_pos[i], (void *) &comm_pos[i], max_num_cases[i]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__comm_pos1[i], (void *) &comm_pos1[i], max_num_cases[i]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__comm_send_off[i], (void *) &comm_send_off[i], max_num_cases[i]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__comm_recv_off[i], (void *) &comm_recv_off[i], max_num_cases[i]*sizeof(int), __FILE__, __LINE__);
   }

   if (num_refine) {
      par_b.max_part = 10;
      par_b.max_cases = 100;
      par_p.max_part = 10;
      par_p.max_cases = 100;
      par_p1.max_part = 10;
      par_p1.max_cases = 100;
   } else {
      par_b.max_part = 1;
      par_b.max_cases = 1;
      par_p.max_part = 1;
      par_p.max_cases = 1;
      par_p1.max_part = 1;
      par_p1.max_cases = 1;
   }
   gasket__ma_malloc(glbl, &par_b.dbmeta__comm_part, (void *) &par_b.comm_part, par_b.max_part*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_b.dbmeta__par_comm_num, (void *) &par_b.par_comm_num, par_b.max_part*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_b.dbmeta__index, (void *) &par_b.index, par_b.max_part*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_b.dbmeta__comm_b, (void *) &par_b.comm_b, par_b.max_cases*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_b.dbmeta__comm_p, (void *) &par_b.comm_p, par_b.max_cases*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_b.dbmeta__comm_c, (void *) &par_b.comm_c, par_b.max_cases*sizeof(int), __FILE__, __LINE__);

   gasket__ma_malloc(glbl, &par_p.dbmeta__comm_part, (void *) &par_p.comm_part, par_b.max_part*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_p.dbmeta__par_comm_num, (void *) &par_p.par_comm_num, par_b.max_part*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_p.dbmeta__index, (void *) &par_p.index, par_b.max_part*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_p.dbmeta__comm_b, (void *) &par_p.comm_b, par_b.max_cases*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_p.dbmeta__comm_p, (void *) &par_p.comm_p, par_b.max_cases*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_p.dbmeta__comm_c, (void *) &par_p.comm_c, par_b.max_cases*sizeof(int), __FILE__, __LINE__);

   gasket__ma_malloc(glbl, &par_p1.dbmeta__comm_part, (void *) &par_p1.comm_part, par_b.max_part*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_p1.dbmeta__par_comm_num, (void *) &par_p1.par_comm_num, par_b.max_part*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_p1.dbmeta__index, (void *) &par_p1.index, par_b.max_part*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_p1.dbmeta__comm_b, (void *) &par_p1.comm_b, par_b.max_cases*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_p1.dbmeta__comm_p, (void *) &par_p1.comm_p, par_b.max_cases*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &par_p1.dbmeta__comm_c, (void *) &par_p1.comm_c, par_b.max_cases*sizeof(int), __FILE__, __LINE__);

   if (num_refine) {
      s_buf_size = (int) (0.10*((double)max_num_blocks))*comm_vars*
                   (x_block_size+2)*(y_block_size+2)*(z_block_size+2);
      if (s_buf_size < (num_vars*x_block_size*y_block_size*z_block_size + 45))
         s_buf_size = num_vars*x_block_size*y_block_size*z_block_size + 45;
      r_buf_size = 5*s_buf_size;
   } else {
      i = init_block_x*(x_block_size+2);
      j = init_block_y*(y_block_size+2);
      k = init_block_z*(z_block_size+2);
      if (i > j)         // do not need ordering just two largest
         if (j > k)      // i > j > k
            s_buf_size = i*j;
         else            // i > j && k > j
            s_buf_size = i*k;
      else if (i > k)    // j > i > k
            s_buf_size = i*j;
         else            // j > i && k > i
            s_buf_size = j*k;
      r_buf_size = 2*s_buf_size;
   }
   gasket__ma_malloc(glbl, &dbmeta__send_buff, (void *) &send_buff, s_buf_size*sizeof(double), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &dbmeta__recv_buff, (void *) &recv_buff, r_buf_size*sizeof(double), __FILE__, __LINE__);

   SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  k
#undef  m
#undef  n
}

// =================================== deallocate ============================

void deallocate(Globals_t * const glbl)
{
   int i, j, m, n;

#if 0
Cell space is now part of Bulk_t
   for (n = 0; n < max_num_blocks; n++) {
      for (m = 0; m < num_vars; m++) {
         for (i = 0; i < x_block_size+2; i++) {
            for (j = 0; j < y_block_size+2; j++)
               free(blocks[n].array[m][i][j]);
            free(blocks[n].array[m][i]);
         }
         free(blocks[n].array[m]);
      }
      free(blocks[n].array);
   }
#endif
   // Now part of Bulk_t:  free(blocks);

   // Now part of Bulk_t:  free(sorted_list);
   // Now part of global:  free(sorted_index);

   // Now part of Bulk_t:  free(objects);

   // Now part of Bulk_t:  free(grid_sum);

   // Not used: free(p8);
   // Now part of global:  free(p2);

   // Now part of Bulk_t:  free(from);
   // Now part of Bulk_t:  free(to);

   for (i = 0; i < 3; i++) {
      gasket__free(&dbmeta__comm_partner[i], (void **) &comm_partner[i], __FILE__, __LINE__);
      gasket__free(&dbmeta__send_size[i], (void **) &send_size[i], __FILE__, __LINE__);
      gasket__free(&dbmeta__recv_size[i], (void **) &recv_size[i], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_index[i], (void **) &comm_index[i], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_num[i], (void **) &comm_num[i], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_block[i], (void **) &comm_block[i], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_face_case[i], (void **) &comm_face_case[i], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_pos[i], (void **) &comm_pos[i], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_pos1[i], (void **) &comm_pos1[i], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_send_off[i], (void **) &comm_send_off[i], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_recv_off[i], (void **) &comm_recv_off[i], __FILE__, __LINE__);
   }

   gasket__free(&dbmeta__send_buff, (void **) &send_buff, __FILE__, __LINE__);
   gasket__free(&dbmeta__recv_buff, (void **) &recv_buff, __FILE__, __LINE__);

   gasket__free(&dbmeta__bulk, (void **) &bulk, __FILE__, __LINE__);
}

int check_input(Globals_t * const glbl)
{
   int error = 0;

   if (init_block_x < 1 || init_block_y < 1 || init_block_z < 1) {
      printf("initial blocks on processor must be positive\n"); fflush(stdout);
      error = 1;
   }
#ifdef BUILD_FOR_ATOMIC_OCR
   if (init_block_x > 1 || init_block_y > 1 || init_block_z > 1) {
      printf("Command-line arguments init_block_x, init_block_y, and init_block_z\n");
      printf("have no relevance for atomic version.  npx, npy, and npz being\n");
      printf("increased correspondingl\n");
      npx *= init_block_x;
      npy *= init_block_y;
      npz *= init_block_z;
      init_block_x = 1;
      init_block_y = 1;
      init_block_x = 1;
   }
#endif
   if (max_num_blocks < init_block_x*init_block_y*init_block_z) {
      printf("max_num_blocks not large enough\n"); fflush(stdout);
      error = 1;
   }
   if (x_block_size < 1 || y_block_size < 1 || z_block_size < 1) {
      printf("block size must be positive\n"); fflush(stdout);
      error = 1;
   }
   if (((x_block_size/2)*2) != x_block_size) {
      printf("block size in x direction must be even\n"); fflush(stdout);
      error = 1;
   }
   if (((y_block_size/2)*2) != y_block_size) {
      printf("block size in y direction must be even\n"); fflush(stdout);
      error = 1;
   }
   if (((z_block_size/2)*2) != z_block_size) {
      printf("block size in z direction must be even\n"); fflush(stdout);
      error = 1;
   }
   if (target_active && target_max) {
      printf("Only one of target_active and target_max can be used\n"); fflush(stdout);
      error = 1;
   }
   if (target_active && target_min) {
      printf("Only one of target_active and target_min can be used\n"); fflush(stdout);
      error = 1;
   }
   if (target_active < 0 || target_active > max_num_blocks) {
      printf("illegal value for target_active\n"); fflush(stdout);
      error = 1;
   }
   if (target_max < 0 || target_max > max_num_blocks ||
       target_max < target_active) {
      printf("illegal value for target_max\n"); fflush(stdout);
      error = 1;
   }
   if (target_min < 0 || target_min > max_num_blocks ||
       target_min > target_active || target_min > target_max) {
      printf("illegal value for target_min\n"); fflush(stdout);
      error = 1;
   }
   if (num_refine < 0) {
      printf("number of refinement levels must be non-negative\n"); fflush(stdout);
      error = 1;
   }
   if (num_refine > 10) {
      printf("number of refinement levels must not exceed ten\n"); fflush(stdout);
      error = 1;
   }
   if (block_change < 0) {
      printf("number of refinement levels must be non-negative\n"); fflush(stdout);
      error = 1;
   }
   if (num_vars < 1) {
      printf("number of variables must be positive\n"); fflush(stdout);
      error = 1;
   }
   if (num_pes != npx*npy*npz) {
      printf("number of processors used does not match number allocated\n"); fflush(stdout);
      error = 1;
   }
   if (stencil != 7 && stencil != 27) {
      printf("illegal value for stencil\n"); fflush(stdout);
      error = 1;
   }
   if (stencil == 27 && num_refine && !uniform_refine)
      printf("WARNING: 27 point stencil with non-uniform refinement: answers may diverge\n"); fflush(stdout);
   if (comm_vars == 0 || comm_vars > num_vars)
      comm_vars = num_vars;
   if (code < 0 || code > 2) {
      printf("code must be 0, 1, or 2\n"); fflush(stdout);
      error = 1;
   }
   if (lb_opt < 0 || lb_opt > 2) {
      printf("lb_opt must be 0, 1, or 2\n"); fflush(stdout);
      error = 1;
   }

   return (error);
}
