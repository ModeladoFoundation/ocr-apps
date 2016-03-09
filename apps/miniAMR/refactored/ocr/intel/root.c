// TODO: FIXME:  UPDATE Copyright notice!
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
//#include <mpi.h> TODO

#include <ocr.h>
#include <ocr-std.h>
//#include <extensions/ocr-labeling.h>
#include "ocr-macros_brn.h"

#include "control.h"
#include "meta.h"
#include "object.h"
#include "checksum.h"
#include "block.h"
#include "profile.h"
#include "proto.h"
#include "continuationcloner.h"

// root.c          This is the "root progenitor", i.e. the "Adam"-level parent of the entire mesh.
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// mainEdt
//
// This is the EDT that the OCR startup instantiates.  It creates only ONE instance of this EDT.  The name "mainEdt" is essentially just a generic name made necessary by the fact that
// the OCR startup doesn't know the context of what will be done at the very start.  To make this more clear, we will simply make this function call a function whose name reflects what we
// need to do first:  rootProgenitorLaunch_Func.

ocrGuid_t mainEdt (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {
   return rootProgenitorLaunch_Func(paramc, paramv, depc, depv);
} // mainEdt

// ***************************************************************************************************************************************************************************************
// rootProgenitorLaunch_Func  -- "Thunked" to by mainEdt, i.e. called as a function directly from mainEdt, NOT created as an EDT therefrom.
// -- Topology:  mainEdt --> Launch(create datablocks) --> Init(initialize contents of datablocks, create children) --> Clone(steady state: perform service requests of children; clone again)
//
// rootProgenitorLaunch_Func does the following:
//   * Process the command-line arguments only sufficiently to figure out how many objects will be modeled to move through the mesh.  Creates a "scratch" datablock for same.
//   * Creates a "control" datablock for an instance of the shared struct of parsed command line arguments and derived control variables.
//   * Creates a "wrapup" EDT, which will gain control when the parseCmdLine finish EDT terminates.  It will shut down the application.
//   * Passes control to the EDT that starts at rootProgenitorInit_Func.

ocrGuid_t rootProgenitorLaunch_Func ( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   rootProgenitorLaunch_Deps_t * myDeps = (rootProgenitorLaunch_Deps_t *) depv;
   char      ** argv      = ((char **) (myDeps->argv_Dep.ptr));
   ocrGuid_t    argv_dblk =            (myDeps->argv_Dep.guid);
   u32 argc = getArgc(argv);

   int i;
   int num_objects = -9999;
   int num_blocks  = 1;

// Process the command line only to the extent necessary to figure out how many objects to model, and how many totally-unrefined blocks there are in the mesh..

   for (i = 1; i < argc; i++) {
      if (!strcmp(getArgv(argv, i), "--num_objects")) {
         if (num_objects != -9999) {
            printf ("--num_objects should only appear once on the command line.\n"); fflush(stdout);
            ocrShutdown();
         }
         if (i == argc) {
            printf ("Number of objects must appear after the --num_objects keyword on the command-line.\n"); fflush(stdout);
            ocrShutdown();
         }
         num_objects = atoi(getArgv(argv, ++i));
      }
      if (!strcmp(getArgv(argv, i), "--npx") || !strcmp(getArgv(argv, i), "--npy") || !strcmp(getArgv(argv, i), "--npz")) {
         num_blocks *= atoi(getArgv(argv, ++i));
      }
   }
   if (num_objects == -9999) num_objects = 1;
   if (num_blocks > 1000) {
      printf ("Number of totally unrefined blocks, i.e. npx*npy*npz, is excessive.  Recommendation:\n");
      printf ("reduce, and plan to do more refinement.  Otherwise, modify limits and try again.\n");
      printf ("Note, though, that the rootProgenitorContinuatin function has to have an input dependency\n");
      printf ("for each and every unrefined block of the mesh (plus a few more), and that might hit up\n");
      printf ("against limits in OCR itself.  Those limits can be increased too, but it means rebuilding OCR.\n"); fflush(stdout);
      *((int *) 123) = 456;
      ocrShutdown();
   }

// Create the datablock into which the command-line arguments will be parsed.  Also create the datablock that will record the initial position, shape, and motion characterstics of the object(s) to model.

   ocrGuid_t meta_dblk;
   ocrGuid_t control_dblk;
   ocrGuid_t scratchAllObjects_dblk;
   void * dummy;

#define control (&hack)   // In all other contexts, the control_t datablock (which contains npx, npy, npz, and num_objects) will be available, but here, we need to hack a look-alike so that
                          // the sizeof_AllObjects_t and sizeof_RootProgenitorMeta_t macros will work.
   struct {
      int num_objects;
      int npx, npy, npz;
   } hack;
   hack.num_objects = num_objects;
   hack.npx = num_blocks;
   hack.npy = 1;
   hack.npz = 1;
   gasket__ocrDbCreate(&meta_dblk,              &dummy, sizeof_RootProgenitorMeta_t, __FILE__, __func__, __LINE__);
   gasket__ocrDbCreate(&scratchAllObjects_dblk, &dummy, sizeof_AllObjects_t,         __FILE__, __func__, __LINE__);
#undef control            // Clean up the above hack, so that it's effects will not propagate to any code below.
   gasket__ocrDbCreate(&control_dblk,           &dummy, sizeof_Control_t,            __FILE__, __func__, __LINE__);

// Create the rootProgenitorInit object as successor to this EDT. Postpone plumbing its dependencies to it until after setting up the wrapup object.

   ocrGuid_t rootProgenitorInit_Template;
   ocrEdtTemplateCreate(&rootProgenitorInit_Template,                 // Guid of template
                        rootProgenitorInit_Func,                      // Top level function of EDT
                        countof_rootProgenitorInit_Params_t,          // Number of params
                        countof_rootProgenitorInit_Deps_t);           // Number of dependencies

   ocrGuid_t rootProgenitorInit_Edt;
   ocrGuid_t rootProgenitorInit_DoneEvent;
   rootProgenitorInit_Params_t rootProgenitorInit_Params;
   rootProgenitorInit_Params.dummy_Prm = 9999;                        // No parameters to pass to rootProgenitorInit_Func.
   ocrEdtCreate(&rootProgenitorInit_Edt,                              // Guid of the EDT created to start at function realMainEdt.
                rootProgenitorInit_Template,                          // Template for the EDT we are creating.
                EDT_PARAM_DEF,
                (u64 *) &rootProgenitorInit_Params,
                EDT_PARAM_DEF,
                NULL,
                EDT_PROP_FINISH,
                NULL_GUID,
                &rootProgenitorInit_DoneEvent);
   ocrEdtTemplateDestroy(rootProgenitorInit_Template);

// Create the wrapup object, and plumb its dependency.

   ocrGuid_t wrapup_Template;
   ocrEdtTemplateCreate(&wrapup_Template,                       // Guid of template
                        wrapup_Func,                            // Top level function of EDT
                        countof_wrapup_Params_t,                // Number of params
                        countof_wrapup_Deps_t);                 // Number of dependencies

   ocrGuid_t wrapup_Edt;
   wrapup_Params_t wrapup_Params;
   wrapup_Params.dummy_Prm = 9999;                              // No parameters to pass to wrapup_Func.
   ocrEdtCreate(&wrapup_Edt,
                wrapup_Template,
                EDT_PARAM_DEF,
                (u64 *) &wrapup_Params,
                EDT_PARAM_DEF,
                NULL,
                EDT_PROP_NONE,
                NULL_GUID,
                NULL_GUID);
   ocrEdtTemplateDestroy(wrapup_Template);

   ADD_DEPENDENCE(rootProgenitorInit_DoneEvent, wrapup_Edt, wrapup_Deps_t, triggerEvent_Dep, DB_MODE_RO);

// Plumb the dependencies to the rootProgenitorInit object.

   ocrDbRelease  (argv_dblk); argv = NULL;
   ADD_DEPENDENCE(argv_dblk,              rootProgenitorInit_Edt, rootProgenitorInit_Deps_t, argv_Dep,              DB_MODE_RO); // Provide datablock containing command line argument list.
   ADD_DEPENDENCE(meta_dblk,              rootProgenitorInit_Edt, rootProgenitorInit_Deps_t, meta_Dep,              DB_MODE_RW); // Provide datablock into which the continuatio cloning stack resides.
   ADD_DEPENDENCE(control_dblk,           rootProgenitorInit_Edt, rootProgenitorInit_Deps_t, control_Dep,           DB_MODE_RW); // Provide datablock into which it is to parse the command line.
   ADD_DEPENDENCE(scratchAllObjects_dblk, rootProgenitorInit_Edt, rootProgenitorInit_Deps_t, scratchAllObjects_Dep, DB_MODE_RW); // Provide datablock into which it is to describe the objects to model.

// This EDT is done, and should die.  rootProgenitorInit_Edt should fire now, and wrapup_Edt should wait for it as a finish EDT.

   return NULL_GUID;
} // rootProgenitorLaunch


// **************************************************************************************************************************************************************************************************************
// rootProgenitorInit_Func
// -- Topology:  mainEdt --> Launch(create datablocks) --> Init(initialize contents of datablocks, create children) --> Clone(steady state: perform service requests of children; clone again)
//
// This EDT does the following:
//   * Parses the command line into control_t and allScratchObjects_t.
//   * Creates one instance of blockLaunch for each unrefined block in the problem mesh, of which there are npx*npy*npz of them.  These are the "children".  Identify position of each by parameters thereto.
//   * Creates the first in the series of rootProgenitorContinuation clones, to await the first service request from the children.

ocrGuid_t rootProgenitorInit_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   int i;

printf ("Function %36s, File %30s, line %4d\n", __func__, __FILE__, __LINE__); fflush(stdout);
   rootProgenitorInit_Deps_t  * myDeps = (rootProgenitorInit_Deps_t  *) depv;
   char                 ** argv                   = ((char **)                (myDeps->argv_Dep.ptr));
   ocrGuid_t               argv_dblk              =                           (myDeps->argv_Dep.guid);
   ocrGuid_t               meta_dblk              =                           (myDeps->meta_Dep.guid);
   RootProgenitorMeta_t  * meta                   = ((RootProgenitorMeta_t *) (myDeps->meta_Dep.ptr));
   Control_t             * control                = ((Control_t *)            (myDeps->control_Dep.ptr));
   ocrGuid_t               control_dblk           =                           (myDeps->control_Dep.guid);
   AllObjects_t          * scratchAllObjects      = ((AllObjects_t *)         (myDeps->scratchAllObjects_Dep.ptr));
   ocrGuid_t               scratchAllObjects_dblk =                           (myDeps->scratchAllObjects_Dep.guid);
   u32 argc = getArgc(argv);

// Set defaults for command line arguments

   control->x_block_size = 10;
   control->y_block_size = 10;
   control->z_block_size = 10;
   control->npx = 1;
   control->npy = 1;
   control->npz = 1;
   control->num_refine = 5;
   control->block_change = 0;
   control->uniform_refine = 0;
   control->refine_freq = 5;
   control->num_vars = 40;
   control->comm_vars = 0;
   control->num_tsteps = 20;
   control->stages_per_ts = 20;
   control->checksum_freq = 5;
   control->stencil = 7;
   control->error_tol = 8;
   control->report_diffusion = 0;
   control->report_perf = 4;
   control->plot_freq = 0;
   control->code = 0;
   control->permute = 0;
   control->refine_ghost = 0;
   control->num_objects = -1;

//   * Parse the command line into control_t and allScratchObjects_t.

   int object_num = 0;

   for (i = 1; i < argc; i++) {
      if (!strcmp(getArgv(argv, i), "--nx")) {
         control->x_block_size = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--ny")) {
         control->y_block_size = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--nz")) {
         control->z_block_size = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--npx")) {
         control->npx = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--npy")) {
         control->npy = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--npz")) {
         control->npz = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--num_refine")) {
         control->num_refine = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--block_change")) {
         control->block_change = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--uniform_refine")) {
         control->uniform_refine = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--refine_freq")) {
         control->refine_freq = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--num_vars")) {
         control->num_vars = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--comm_vars")) {
         control->comm_vars = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--num_tsteps")) {
         control->num_tsteps = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--stages_per_ts")) {
         control->stages_per_ts = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--checksum_freq")) {
         control->checksum_freq = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--stencil")) {
         control->stencil = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--error_tol")) {
         control->error_tol = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--report_diffusion")) {
         control->report_diffusion = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--report_perf")) {
         control->report_perf = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--plot_freq")) {
         control->plot_freq = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--code")) {                // TODO:  Determine if this is really still needed.  (Note: help message should be changed accordingly.
         control->code = atoi(getArgv(argv, ++i));
      } else if (!strcmp(getArgv(argv, i), "--permute")) {
         control->permute = 1;
      } else if (!strcmp(getArgv(argv, i), "--refine_ghost")) {
         control->refine_ghost = 1;
      } else if (!strcmp(getArgv(argv, i), "--num_objects")) {
         control->num_objects = atoi(getArgv(argv, ++i));
         object_num = 0;
      } else if (!strcmp(getArgv(argv, i), "--object")) {
         if (object_num >= control->num_objects) {
            printf("object number greater than num_objects\n"); fflush(stdout);
            ocrShutdown();
         }
         scratchAllObjects->object[object_num].type    = atoi(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].bounce  = atoi(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].cen[0]  = atof(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].cen[1]  = atof(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].cen[2]  = atof(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].move[0] = atof(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].move[1] = atof(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].move[2] = atof(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].size[0] = atof(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].size[1] = atof(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].size[2] = atof(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].inc[0]  = atof(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].inc[1]  = atof(getArgv(argv, ++i));
         scratchAllObjects->object[object_num].inc[2]  = atof(getArgv(argv, ++i));
         object_num++;
      } else if (!strcmp(getArgv(argv, i), "--help")) {
         print_help_message();
#ifdef BUILD_OCR_VERSION
         ocrShutdown();
#else
         MPI_Abort(MPI_COMM_WORLD, -1);
#endif
      } else {
         printf("** Error ** Unknown input parameter %s\n", getArgv(argv, i)); fflush(stdout);
         print_help_message();
#ifdef BUILD_OCR_VERSION
         ocrShutdown();
#else
         MPI_Abort(MPI_COMM_WORLD, -1);
#endif
      }
   }

// Init out meta data.
// First, init the Continuation Cloning State.

   Frame_Header_t * topOfStack  = (Frame_Header_t *) meta->cloningState.stack;
   topOfStack[0].resumption_case_num                  = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[0].my_size                              = sizeof_Frame_Header_t;  // Distance to first callee's frame.
   topOfStack[0].caller_size                          = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[0].validate_callers_prep_for_suspension = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[1].resumption_case_num                  = 0;                      // Prepare for first callee.
   topOfStack[1].my_size                              = -9999;                  // Size of callee's frame is not known by caller.
   topOfStack[1].caller_size                          = sizeof_Frame_Header_t;  // To tell callee how much to pop when it returns.
   meta->cloningState.topPtrAdjRecOffset              = -9999;
   meta->cloningState.numberOfDatablocks              = numDatablocksInRootProgenitorMetaCatalog;
   meta->cloningState.offsetToCatalogOfDatablocks     = ((unsigned long long) (&(meta->dbCatalog[0]))) - ((unsigned long long) (&(meta->cloningState)));
   meta->cloningState.cloneNum                        = 0;

// First, init all the datablocks in the catalog to null.

   for (i = 0; i < meta->cloningState.numberOfDatablocks; i++) {
      meta->dbCatalog[i].dblk = NULL_GUID;      // Default to the datablock NOT existing yet.
      meta->dbCatalog[i].base = NULL;           // Default to its pointer also indicating it does not exist yet.
      meta->dbCatalog[i].size = -9999;          // Default that, when it ultimately is created, its size will have to be filled in at that time, as it is unknowable in advance.
      meta->dbCatalog[i].acMd = DB_MODE_NULL;   // Default to it having no access rights yet.
  }

// Now flesh out more details about the datablocks that we know more about.

   meta->metaDb.dblk              = meta_dblk;                   // If this looks a bit "self-referential", it is because it unions with an array that holds the meta info about ALL the other
   meta->metaDb.base              = meta;                        // datablocks too, and we use that array to convert between pointers(used in the body of EDTs) and datablock-index-and-offset
   meta->metaDb.size              = sizeof_RootProgenitorMeta_t; // (used to communicate the pointers from one EDT to another, wherein OCR might move the datablock to a new address).
   meta->metaDb.acMd              = DB_MODE_UPDATE;              // We are writing to this datablock here, in this very instance of this EDT, but it will be updated by our successor EDT.

   meta->controlDb.dblk           = control_dblk;                // This datablock is read-only, and shared among all children below this point.
   meta->controlDb.base           = control;
   meta->controlDb.size           = sizeof_Control_t;
   meta->controlDb.acMd           = DB_MODE_RO;                  // We are writing to this databock in this very EDT (we did so just above), but hereafter, this one will be read-only to our successor lineage.

   void * dummy;
#ifndef ALLOW_DATABLOCK_REWRITES
   // The first successor will surely need to clone meta.  Create its datablock for doing same.
   gasket__ocrDbCreate(&meta->metaCloneDb.dblk, (void **) &dummy, sizeof_RootProgenitorMeta_t,    __FILE__, __func__, __LINE__);
   meta->metaCloneDb.size         = sizeof_RootProgenitorMeta_t;
   meta->metaCloneDb.acMd         = DB_MODE_RW;                  // Our successor EDT will surely update this datablock.
#endif

   meta->goldenChecksumDb.size    = sizeof_Checksum_t;
   meta->goldenChecksumDb.acMd    = DB_MODE_RO;                  // Actually, we make this one DB_MODE_RW explicitly, just below, where we pass it to the FIRST continuation clone.  After THAT, it will be RO.
   meta->scratchChecksumDb.size   = sizeof_Checksum_t;

//   * Create the first in the series of rootProgenitorContinuation clones, to await the first service request from the children.

   rootProgenitorContinuation_Params_t rootProgenitorContinuation_Params;
   rootProgenitorContinuation_Params.isFirstChecksum = 1;                                           // Init to TRUE
   ocrEdtTemplateCreate (&rootProgenitorContinuation_Params.rootProgenitorContinuation_Template,    // Guid of template
                         rootProgenitorContinuation_Func,                                           // Top level function of EDT
                         countof_rootProgenitorContinuation_Params_t,                               // Number of params
                         countof_rootProgenitorContinuation_Deps_t);                                // Number of dependencies
   ocrGuid_t     rootProgenitorContinuation_Edt;
   ocrEdtCreate (&rootProgenitorContinuation_Edt,
                 rootProgenitorContinuation_Params.rootProgenitorContinuation_Template,
                 EDT_PARAM_DEF,
                 (u64 *) &rootProgenitorContinuation_Params,
                 EDT_PARAM_DEF,
                 NULL,
                 EDT_PROP_NONE,
                 NULL_GUID,
                 NULL_GUID);
   ocrGuid_t     goldenChecksum_dblk;
   ocrGuid_t     scratchChecksum_dblk;
   gasket__ocrDbCreate(&goldenChecksum_dblk,  &dummy, sizeof_Checksum_t, __FILE__, __func__, __LINE__);
   ADD_DEPENDENCE(goldenChecksum_dblk,  rootProgenitorContinuation_Edt, rootProgenitorContinuation_Deps_t, goldenChecksum_Dep,  DB_MODE_RW);
   ADD_DEPENDENCE(NULL_GUID,            rootProgenitorContinuation_Edt, rootProgenitorContinuation_Deps_t, scratchChecksum_Dep, DB_MODE_NULL);
   ADD_DEPENDENCE(control_dblk,         rootProgenitorContinuation_Edt, rootProgenitorContinuation_Deps_t, control_Dep,         DB_MODE_RO);

//   * Create one instance of blockLaunch for each unrefined block in the problem mesh, of which there are npx*npy*npz of them.  These are the "children".  Identify position of each by parameters thereto.

   blockLaunch_Params_t blockLaunch_Params;

   ocrEdtTemplateCreate (&blockLaunch_Params.template.blockLaunch_Template,        // Guid of template
                         blockLaunch_Func,                                         // Top level function of EDT
                         countof_blockLaunch_Params_t,                             // Number of params
                         countof_blockLaunch_Deps_t);                              // Number of dependencies
   ocrEdtTemplateCreate (&blockLaunch_Params.template.blockInit_Template,          // Guid of template
                         blockInit_Func,                                           // Top level function of EDT
                         countof_blockInit_Params_t,                               // Number of params
                         countof_blockInit_Deps_t);                                // Number of dependencies
   ocrEdtTemplateCreate (&blockLaunch_Params.template.blockContinuation_Template,  // Guid of template
                         blockContinuation_Func,                                   // Top level function of EDT
                         countof_blockContinuation_Params_t,                       // Number of params
                         countof_blockContinuation_Deps_t);                        // Number of dependencies
   blockLaunch_Params.refinementLevel = 0;

   int xLim, yLim, zLim, idep;
   xLim     = control->npx;
   yLim     = control->npy;
   zLim     = control->npz;
   ocrDbRelease(control_dblk);            control           = NULL;
   ocrDbRelease(scratchAllObjects_dblk);  scratchAllObjects = NULL;

   idep = 0;
   for (      blockLaunch_Params.xPos = 0; blockLaunch_Params.xPos < xLim; blockLaunch_Params.xPos++) {
      for (   blockLaunch_Params.yPos = 0; blockLaunch_Params.yPos < yLim; blockLaunch_Params.yPos++) {
         for (blockLaunch_Params.zPos = 0; blockLaunch_Params.zPos < zLim; blockLaunch_Params.zPos++) {
            ocrEventCreate(&blockLaunch_Params.conveyOperand_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

            ADD_DEPENDENCE(blockLaunch_Params.conveyOperand_Event, rootProgenitorContinuation_Edt, rootProgenitorContinuation_Deps_t, annex[idep].serviceRequestOperand_Dep, DB_MODE_RO);

            ocrGuid_t   blockLaunch_Edt;
            ocrEdtCreate(&blockLaunch_Edt,
                         blockLaunch_Params.template.blockLaunch_Template,
                         EDT_PARAM_DEF,
                         (u64 *) &blockLaunch_Params,
                         EDT_PARAM_DEF,
                         NULL,
                         EDT_PROP_NONE,
                         NULL_GUID,
                         NULL_GUID);

            ADD_DEPENDENCE(control_dblk,           blockLaunch_Edt, blockLaunch_Deps_t, control_Dep,    DB_MODE_RO);
            ADD_DEPENDENCE(scratchAllObjects_dblk, blockLaunch_Edt, blockLaunch_Deps_t, allObjects_Dep, DB_MODE_RO);
            ADD_DEPENDENCE(NULL_GUID,              blockLaunch_Edt, blockLaunch_Deps_t, block_Dep,      DB_MODE_NULL);

            idep++;
         }
      }
   }

#ifndef ALLOW_DATABLOCK_REWRITES
   ADD_DEPENDENCE(meta->metaCloneDb.dblk, rootProgenitorContinuation_Edt, rootProgenitorContinuation_Deps_t, metaClone_Dep, meta->metaCloneDb.acMd);
#endif
   ocrDbAccessMode_t acMd = meta->metaDb.acMd;
   ocrDbRelease  (meta_dblk); meta = NULL;
   ADD_DEPENDENCE(meta_dblk,            rootProgenitorContinuation_Edt, rootProgenitorContinuation_Deps_t, meta_Dep,      acMd);

   return NULL_GUID;

} // rootProgenitorInit_Func


// **************************************************************************************************************************************************************************************************************
// rootProgenitorContinuation_Func
// -- Topology:  mainEdt --> rPLaunch(create datablocks) --> rPInit(initialize contents of datablocks, create children) --> rPClone(steady state: perform service requests of children; clone again)
//
// Provide services to the top-level blocks (i.e. the blocks that are totally unrefined, or the parents of refined blocks descended from them.)

ocrGuid_t rootProgenitorContinuation_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   int i;

printf ("Function %36s, File %30s, line %4d\n", __func__, __FILE__, __LINE__); fflush(stdout);
   rootProgenitorContinuation_Deps_t   * myDeps   = (rootProgenitorContinuation_Deps_t   *) depv;
   rootProgenitorContinuation_Params_t * myParams = (rootProgenitorContinuation_Params_t *) paramv;

   // First prepare meta datablock.   To do so, we also need the control datablock.

   ocrGuid_t                    control_dblk         =                           (myDeps->control_Dep.guid);
   Control_t *                  control              = ((Control_t *)            (myDeps->control_Dep.ptr));
#ifdef ALLOW_DATABLOCK_REWRITES
   ocrGuid_t                    meta_dblk            =                           (myDeps->meta_Dep.guid);
   RootProgenitorMeta_t       * meta                 = ((RootProgenitorMeta_t *) (myDeps->meta_Dep.ptr));
#else
   ocrGuid_t                    oldMeta_dblk         =                           (myDeps->meta_Dep.guid);
   RootProgenitorMeta_t const * oldMeta              = ((RootProgenitorMeta_t *) (myDeps->meta_Dep.ptr));
   ocrGuid_t                    meta_dblk            =                           (myDeps->metaClone_Dep.guid);
   RootProgenitorMeta_t       * meta                 = ((RootProgenitorMeta_t *) (myDeps->metaClone_Dep.ptr));
   memcpy (meta, oldMeta, sizeof_RootProgenitorMeta_t);
   ocrDbDestroy(oldMeta_dblk);  oldMeta_dblk = NULL_GUID; oldMeta = NULL;
   meta->metaCloneDb.dblk                            = NULL_GUID;
   meta->metaCloneDb.base                            = NULL;
#endif

   meta->metaDb.dblk                                 = meta_dblk;
   meta->metaDb.base                                 = meta;
   meta->cloningState.tos                            = ((char *)                  (((unsigned long long) meta->cloningState.stack)));
   meta->cloningState.topPtrAdjRec                   = ((PtrAdjustmentRecord_t *) (((unsigned long long) meta->cloningState.stack) + meta->cloningState.topPtrAdjRecOffset));

// Now prepare all other datablocks.

   for (i = 0; i < meta->cloningState.numberOfDatablocks; i++) {
      if (&meta->dbCatalog[i].dblk == &meta->metaDb.dblk)         continue;    // meta already prepared.
#ifndef ALLOW_DATABLOCK_REWRITES
      if (&meta->dbCatalog[i].dblk == &meta->metaCloneDb.dblk)    continue;    // already done with metaClone, above.
#endif
      meta->dbCatalog[i].dblk = (myDeps->dependence[i].guid);
      meta->dbCatalog[i].base = (myDeps->dependence[i].ptr);
   }

   meta->cloningState.cloneNum++;
printf ("Function %36s, File %30s, line %4d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->cloningState.cloneNum); fflush(stdout);

   meta->cloningState.continuationOpcode = ContinuationOpcodeUnspecified;
   Frame_Header_t * topOfStack  = (Frame_Header_t *) meta->cloningState.stack;

   topOfStack[1].validate_callers_prep_for_suspension = 1;                    // Set "canary trap" for first callee.
   meta->cloningState.returnCanary                    = 1;                    // Init the "return canary trap" to "live", to detect a return from a suspendable function via a return stmt instead of the macro.
   rootProgenitorContinuation_SoupToNuts(meta, myParams);                     // Resume (or start, the first time) the algorithm.  When we get back, it is either done or needing to clone.
   if (meta->cloningState.returnCanary == 1) {  // If the return canary is "live" the callee must have used a return statement.  We need it to use the macro!
      printf ("ERROR: %s at line %d: Canary trap on RETURN from the callee.  Change callee to return through the NORMAL_RETURN_SEQUENCE macro.\n", __FILE__, __LINE__); fflush(stdout); \
      printf ("ERROR: Other possibility is that callee is not (yet) a SUSPENDABLE function, but it is being identified as such by the caller.\n"); fflush(stdout); \
      *((int *) 123) = 456; \
      ocrShutdown();
   }

   if (topOfStack[1].resumption_case_num != 0) {                              // Create a continuation EDT, cause it to fire, and cause this EDT has to terminate.

      meta->cloningState.topPtrAdjRecOffset = ((int) ((unsigned long long) meta->cloningState.topPtrAdjRec) - ((unsigned long long) meta->cloningState.stack));

      ocrGuid_t rootProgenitorContinuation_Edt;
      ocrEdtCreate(&rootProgenitorContinuation_Edt,                           // Guid of the EDT created to continue at function blockContinuaiton_Func.
                   myParams->rootProgenitorContinuation_Template,             // Template for the EDT we are creating.
                   EDT_PARAM_DEF,
                   (u64 *) myParams,
                   EDT_PARAM_DEF,
                   NULL,
                   EDT_PROP_NONE,
                   NULL_GUID,
                   NULL_GUID);

#ifndef ALLOW_DATABLOCK_REWRITES
      gasket__ocrDbCreate(&meta->metaCloneDb.dblk,    (void **) &meta->metaCloneDb.base,    sizeof_RootProgenitorMeta_t,    __FILE__, __func__, __LINE__);
      meta->metaCloneDb.base       = NULL;
#endif

      switch (meta->cloningState.continuationOpcode) {
      case ContinuationOpcodeUnspecified:
         printf ("In Root Progenitor, Continuation opcode unspecified\n"); fflush(stdout);
         ocrShutdown();
         *((int *) 0) = 123;
         break;
      case SeasoningOneOrMoreDbCreates:
         // TODO:  If a newResource SLOT exists, we will need to satisfy it with NULL_GUID for this opcode:  ADD_DEPENDENCE(NULL_GUID, blockContinuation_Edt, blockContinuation_Deps_t, dep_newResource, RO);
         break;
      case ReceivingACommunication:
         // The site that sent us this opcode has put the guid of the events for the remote newResources into the catalog already.  Just drop out of this switch statement and add dependencies of our clone.
         break;
      default:
         printf ("In Root Progenitor, Continuation opcode not yet handled,  opcode = %d / 0x%x\n", meta->cloningState.continuationOpcode, meta->cloningState.continuationOpcode); fflush(stdout);
         ocrShutdown();
         *((int *) 0) = 123;
         break;
      }

      for (i = meta->cloningState.numberOfDatablocks-1; i >= 0; i--) {
         if (meta->dbCatalog[i].dblk == NULL_GUID) {
            ADD_DEPENDENCE(NULL_GUID, rootProgenitorContinuation_Edt, rootProgenitorContinuation_Deps_t, dependence[i], DB_MODE_NULL);
         } else {
            ocrGuid_t         dblk = meta->dbCatalog[i].dblk;
            ocrDbAccessMode_t acMd = meta->dbCatalog[i].acMd;
            meta->dbCatalog[i].dblk = NULL_GUID;
            meta->dbCatalog[i].base = NULL;
            //ocrDbRelease(dblk);     // Release the guid.  Warning:  in the last loop iteration, this releases meta, which is why we copied dblk and acMd to temps above.
            ADD_DEPENDENCE(dblk, rootProgenitorContinuation_Edt, rootProgenitorContinuation_Deps_t, dependence[i], acMd);
         }
      }

   } else {  // (topOfStack[1].resumption_case_num != 0)    If that was the very last iteration, clean up and shut down.
      int i;
      for (i = 1; i < meta->cloningState.numberOfDatablocks; i++) {     // Skip over meta_t (until after this loop).
         if (meta->dbCatalog[i].dblk != NULL_GUID) {
            ocrDbDestroy(meta->dbCatalog[i].dblk);
            meta->dbCatalog[i].dblk = NULL_GUID;
            meta->dbCatalog[i].base = NULL;
         }
      }
      ocrDbDestroy(meta_dblk);
   }
   return NULL_GUID;

} // rootProgenitorContinuation_Func


// **************************************************************************************************************************************************************************************************************
// rootProgenitorContinuation_SoupToNuts
//
// This is the top-level function in the calling topology of the several functions that do the actual processing of the root progenitor's service duties.
//
void rootProgenitorContinuation_SoupToNuts(RootProgenitorMeta_t * meta, rootProgenitorContinuation_Params_t * myParams) {

   typedef struct {
      Frame_Header_t myFrame;
      //double t1, t2;
      int idep;
      DataExchangeOpcode_t opcode;
      struct {
         Control_t * control;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__rootProgenitorContinuation_SoupToNuts_t;

#define idep    (lcl->idep)
#define control (lcl->pointers.control)
#define opcode  (lcl->opcode)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__rootProgenitorContinuation_SoupToNuts_t)

printf ("Function %36s, File %30s, line %4d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->cloningState.cloneNum); fflush(stdout);

   control = meta->controlDb.base;

// Looping, perform the operations requested by the children.  Keep doing so until the final shutdown request arrives.

   do {

      if (meta->annexDb[0].serviceRequestOperand.dblk == NULL_GUID) {  // NULL_GUID on the Operand dependency signals final shutdown.
         for (idep = 0; idep < countof_rootProgenitorContinuation_AnnexDeps_t; idep++) {
            if (meta->annexDb[0].serviceRequestOperand.dblk != NULL_GUID) {
               printf ("Error!  Inconsistency in shutdown signal provided to continuationRootProgenitor\n"); fflush(stdout);
               *((int *) 123) = 456;
               ocrShutdown();
            }
         }
         SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)    // Top-level will sense normal completion, and just end this EDT (without creating any more clones).
      }

// Service requested from children.

      opcode = (((DbCommHeader_t *) (meta->annexDb[0].serviceRequestOperand.base)))->serviceOpcode;

      if (opcode == Operation_Checksum) {
         // Perform the final accumulation of partial contributions to the checksum.  If this is the first time, we record this as the "golden" checksum;  otherwise we compare it against the "golden"
         // values and complain if they differ.  Note that this particular service does NOT return anything to the children, nor do they expect anything back.  If the checksum is bad, this EDT will
         // simply report the mismatch and shutdown the run.


         int isFirstChecksum = myParams->isFirstChecksum;     // Note whether this is the first time we are calculating a checksum (i.e. the "golden" one) ...
         myParams->isFirstChecksum = 0;                       // ... but tell the continuation EDT that the "golden" one is now in hand.

         // If this is the golden one, we want to accumulate into the golden checksum.  Otherwise, we accumulate into child[0][0][0]'s contribution.

         Checksum_t * accumulator = meta->goldenChecksumDb.base;  // Speculate this being the first time.
         if (!isFirstChecksum) {
            gasket__ocrDbCreate(&meta->scratchChecksumDb.dblk,  (void **) &meta->scratchChecksumDb.base, sizeof_Checksum_t, __FILE__, __func__, __LINE__);
            meta->scratchChecksumDb.acMd = DB_MODE_RW;
            meta->cloningState.continuationOpcode = SeasoningOneOrMoreDbCreates;
            SUSPEND__RESUME_IN_CONTINUATION_EDT(;)
            accumulator = meta->scratchChecksumDb.base;
         }

         int i;
         for (i = 0; i < control->num_vars; i++) {
             accumulator->sum[i] = 0.0;
         }

         for (idep = 0; idep < countof_rootProgenitorContinuation_AnnexDeps_t; idep++) {

            // The operand to Operation_Checksum is the child's contribution to the overall checksum.  Add it into our accumulator.

            ocrGuid_t    operand_dblk =                 (meta->annexDb[idep].serviceRequestOperand.dblk);
            Checksum_t * operand      = ((Checksum_t *) (meta->annexDb[idep].serviceRequestOperand.base));

            if (operand->dbCommHeader.serviceOpcode != opcode) {
               printf ("Error!  Inconsistency in opcodes provided to continuationRootProgenitor\n"); fflush(stdout);
               *((int *) 123) = 456;
               ocrShutdown();
            }

            for (i = 0; i < control->num_vars; i++) {
                accumulator->sum[i] = accumulator->sum[i] + operand->sum[i];
            }
         }

         double total = 0.0;
         for (i = 0; i < control->num_vars; i++) {
            total += accumulator->sum[i];
            if (accumulator->sum[i] != meta->goldenChecksumDb.base->sum[i]) {
               printf ("Checksum failure! var index = %ld, acc->sum[i] = %lf/0x%lx, golden->sum[i] = %lf/0x%lx\n", ((unsigned long long) i),
                       accumulator->sum[i], (*((unsigned long long *) &(accumulator->sum[i]))),
                       meta->goldenChecksumDb.base->sum[i], (*((unsigned long long *) &(meta->goldenChecksumDb.base->sum[i])))); fflush(stdout);   // TODO:  Probably needs to allow some wee epsilon
               * ((int *) 123) = 456;
               ocrShutdown();
            } else {
            }
         }
printf ("Grand Total Checksum == %lf\n", total); fflush(stdout);

         if (meta->scratchChecksumDb.dblk != NULL_GUID) {
            ocrDbDestroy(meta->scratchChecksumDb.dblk);  meta->scratchChecksumDb.dblk = NULL_GUID; meta->scratchChecksumDb.base = NULL;
            meta->scratchChecksumDb.acMd = DB_MODE_NULL;
         }

      } else if (opcode == Operation_Profile) {
         // Aggregate the performance profile contributions of dying children to the aggregate.

         Profile_t profile;  // NOTE:  I assume that the stack can accomodate this structure.  If, on some platform, it throws stack overflow exceptions, we could "strip-mine" the aggregation and reporting.
         memset (&profile, 0, sizeof_Profile_t);

         for (idep = 0; idep < countof_rootProgenitorContinuation_AnnexDeps_t; idep++) {

            // The operand to Operation_Checksum is the child's contribution to the overall checksum.  Add it into our accumulator.

            ocrGuid_t   operand_dblk =                (meta->annexDb[idep].serviceRequestOperand.dblk);
            Profile_t * operand      = ((Profile_t *) (meta->annexDb[idep].serviceRequestOperand.base));

            if (operand->dbCommHeader.serviceOpcode != opcode) {
               printf ("Error!  Inconsistency in opcodes provided to continuationRootProgenitor\n"); fflush(stdout);
               *((int *) 123) = 456;
               ocrShutdown();
            }

            int i;
            for (i = 0; i < NUM_PROFILE_DOUBLES; i++) {
               profile.allDoubles[i] += operand->allDoubles[i];
            }
            for (i = 0; i < NUM_PROFILE_INTS; i++) {
               profile.allInts[i] += operand->allInts[i];
            }

         }

         // Report out the results of the profiling information

         report_profile_results(control, &profile);

      } else {
         printf ("Unrecognized opcode received by rootProgenitiorContinuation\n"); fflush(stdout);
         ocrShutdown();
      } // opcode

      // Now for each block, clean up event that brought THIS operand to us, and put "on deck" event into the catalog so that it will be used to plumb operand dependence next time a service request is sought.
      for (idep = 0; idep < countof_rootProgenitorContinuation_AnnexDeps_t; idep++) {
         ocrGuid_t        operand_dblk =                (meta->annexDb[idep].serviceRequestOperand.dblk);
         DbCommHeader_t * operand = ((DbCommHeader_t *) (meta->annexDb[idep].serviceRequestOperand.base));
         ocrEventDestroy(operand->atBat_Event);     // Destroy event that brought us our operand
         meta->annexDb[idep].serviceRequestOperand.dblk = operand->onDeck_Event;
         meta->annexDb[idep].serviceRequestOperand.acMd = DB_MODE_RO;
         ocrDbDestroy   (operand_dblk); operand = NULL;
      }

      // Start a cloning request, to obtain the next service request from rootProgenitor's children.

      meta->cloningState.continuationOpcode = ReceivingACommunication;
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;)

   } while (1);

   SUSPENDABLE_FUNCTION_EPILOGUE

#undef  idep
#undef  control
#undef  opcode

} // rootProgenitorContinuation_Func


// ***************************************************************************************************************************************************************************************
// wrapup_Func

ocrGuid_t wrapup_Func( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[] ) {
// One instance of this EDT is created by mainEdt, and gains control after the finish EDT parseCmdLine and ALL the topology below it terminates.

printf ("Function %36s, File %30s, line %4d\n", __func__, __FILE__, __LINE__); fflush(stdout);
   printf ("wrapupEdt about to call ocrShutdown\n"); fflush(stdout);
   ocrShutdown();
   return NULL_GUID;
} // wrapup_Func


// ***************************************************************************************************************************************************************************************
// print_help_message

void print_help_message() {
   printf("(Optional) command line input is of the form: \n\n");

   printf("--nx - block size x (even && > 0)\n");
   printf("--ny - block size y (even && > 0)\n");
   printf("--nz - block size z (even && > 0)\n");
   printf("--npx - number of blocks in the x direction, before any refinement\n");
   printf("--npy - number of blocks in the y direction, before any refinement\n");
   printf("--npz - number of blocks in the z direction, before any refinement\n");
   printf("--num_refine - (>= 0) number of levels of refinement\n");
   printf("--block_change - (>= 0) number of levels a block can change in a timestep\n");
   printf("--uniform_refine - if 1, then grid is uniformly refined\n");
   printf("--refine_freq - frequency (in timesteps) of checking for refinement\n");
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
   printf("??????   --code - closely minic communication of different codes\n");
   printf("         0 minimal sends, 1 send ghosts, 2 send ghosts and process on send\n");
   printf("--permute - altenates directions in communication\n");
   printf("--refine_ghost - use full extent of block (including ghosts) to determine if block is refined\n");
   printf("--num_objects - (>= 0) number of objects to cause refinement\n");
   printf("--object - type, position, movement, size, size rate of change\n");

   printf("All associated settings are integers except for objects\n"); fflush(stdout);
} // print_help_message
