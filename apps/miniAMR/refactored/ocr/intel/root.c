// ************************************************************************
//
// miniAMR: stencil computations with boundary exchange and AMR.
//
// Copyright (2014) Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
// retains certain rights in this software.
//
// Portions Copyright (2016) Intel Corporation.
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
#include <math.h>

#include <ocr.h>
#include <ocr-guid-functions.h>
#include <ocr-std.h>
#include <extensions/ocr-labeling.h>
#include "ocrmacs.h"

#include "control.h"
#include "object.h"
#include "chksum.h"
#include "block.h"
#include "root.h"
//PROFILE:#include "profile.h"
#include "proto.h"
#include "clone.h"

#ifdef NANNY_FUNC_NAMES
#line __LINE__ "root   "
#endif

// root.c          This is the "root progenitor", i.e. the "Adam"-level parent of the entire mesh.
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// mainEdt
//
// This is the EDT that the OCR startup instantiates.  It creates only ONE instance of this EDT.  The name "mainEdt" is essentially just a generic name made necessary by the fact that
// the OCR startup doesn't know the context of what will be done at the very start.  To make this more clear, we will simply make this function call a function whose name reflects what we
// need to do first:  rootLaunch_Func.

ocrGuid_t mainEdt (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {
   return rootLaunch_Func(paramc, paramv, depc, depv);
} // mainEdt

// ***************************************************************************************************************************************************************************************
// rootLaunch_Func  -- "Thunked" to by mainEdt, i.e. called as a function directly from mainEdt, NOT created as an EDT therefrom.
// -- Topology:  mainEdt --> Launch(create datablocks) --> Init(initialize contents of datablocks, create children) --> Clone(steady state: perform service requests of children; clone again)
//
// rootLaunch_Func does the following:
//   * Process the command-line arguments only sufficiently to figure out how many objects will be modeled to move through the mesh.  Creates a "scratch" datablock for same.
//   * Creates a "control" datablock for an instance of the shared struct of parsed command line arguments and derived control variables.
//   * Passes control to the EDT that starts at rootInit_Func.

ocrGuid_t rootLaunch_Func ( u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   rootLaunch_Deps_t * myDeps = (rootLaunch_Deps_t *) depv;
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
            *((int *) 123) = 456;
            ocrShutdown();
         }
         if (i == argc) {
            printf ("Number of objects must appear after the --num_objects keyword on the command-line.\n"); fflush(stdout);
            *((int *) 123) = 456;
            ocrShutdown();
         }
         num_objects = atoi(getArgv(argv, ++i));
      }
      if (!strcmp(getArgv(argv, i), "--npx") || !strcmp(getArgv(argv, i), "--npy") || !strcmp(getArgv(argv, i), "--npz")) {
         num_blocks *= atoi(getArgv(argv, ++i));
      }
   }
   if (num_objects == -9999) num_objects = 1;
   if (num_blocks > MAX_NUM_UNREFINED_BLOCKS) {
      printf ("Number of totally unrefined blocks, i.e. npx*npy*npz, is excessive.  Recommendation:\n");
      printf ("reduce, and plan to do more refinement.  Otherwise, modify limits and try again.\n");
      printf ("Note, though, that the rootClone function has to have an input dependency\n");
      printf ("for each and every unrefined block of the mesh (plus a few more), and that might hit up\n");
      printf ("against limits in OCR itself.  Those limits can be increased too, but it means rebuilding OCR.\n"); fflush(stdout);
      *((int *) 123) = 456;
      ocrShutdown();
   }

// Create the datablock into which the command-line arguments will be parsed.  Also create the datablock that will record the initial position, shape, and motion characterstics of the object(s) to model.

   ocrGuid_t meta_dblk;
   ocrGuid_t control_dblk;
   ocrGuid_t scratchAllObjects_dblk;
   void * dummy = NULL;

#define control (&hack)   // In all other contexts, the control_t datablock (which contains npx, npy, npz, and num_objects) will be available, but here, we need to hack a look-alike so that
                          // the sizeof_AllObjects_t and sizeof_RootMeta_t macros will work.
   struct {
      int num_objects;
      int npx, npy, npz;
   } hack;
   hack.num_objects = num_objects;
   hack.npx = num_blocks;
   hack.npy = 1;
   hack.npz = 1;
   gasket__ocrDbCreate(&meta_dblk,              &dummy, sizeof_RootMeta_t,   __FILE__, __func__, __LINE__, "rootLaunch", "meta");
   gasket__ocrDbCreate(&scratchAllObjects_dblk, &dummy, sizeof_AllObjects_t, __FILE__, __func__, __LINE__, "rootLaunch", "allObjects");
#undef control            // Clean up the above hack, so that it's effects will not propagate to any code below.
   gasket__ocrDbCreate(&control_dblk,           &dummy, sizeof_Control_t,    __FILE__, __func__, __LINE__, "rootLaunch", "control");

// Create the rootInit object as successor to this EDT.

   ocrGuid_t rootInit_Template;
   ocrEdtTemplateCreate(&rootInit_Template,                 // Guid of template
                        rootInit_Func,                      // Top level function of EDT
                        countof_rootInit_Params_t,          // Number of params
                        countof_rootInit_Deps_t);           // Number of dependencies

   ocrGuid_t rootInit_Edt;
   rootInit_Params_t rootInit_Params;
   rootInit_Params.dummy_Prm = 9999;                        // No parameters to pass to rootInit_Func.
   gasket__ocrEdtCreate(&rootInit_Edt,                              // Guid of the EDT created to start at function realMainEdt.
                        SLOT(rootInit_Deps_t, whoAmI_Dep),
                        rootInit_Template,                          // Template for the EDT we are creating.
                        EDT_PARAM_DEF,
                        (u64 *) &rootInit_Params,
                        EDT_PARAM_DEF,
                        NULL,
                        EDT_PROP_NONE,
                        NULL_HINT,
                        NULL,
                        __FILE__,
                        __func__,
                        __LINE__,
                        " ",
                        "rootInit");
   ocrEdtTemplateDestroy(rootInit_Template);

// Plumb the dependencies to the rootInit object.

   gasket__ocrDbRelease  (argv_dblk, __FILE__, __func__, __LINE__, " ", "argv -- NOTE: STARTUP DID ocrDbCreate, so IGNORE AUDIT MESSAGES REGARDING THIS."); argv = NULL;
   ADD_DEPENDENCE(argv_dblk,              rootInit_Edt, rootInit_Deps_t, argv_Dep,              DB_MODE_RO, "rootLaunch", "argv -- NOTE: STARTUP DID ocrDbCreate, so IGNORE AUDIT MESSAGES REGARDING THIS."); // Provide datablock containing command line argument list.
   ADD_DEPENDENCE(meta_dblk,              rootInit_Edt, rootInit_Deps_t, meta_Dep,              DB_MODE_RW, "rootLaunch", "meta"); // Provide datablock into which the continuatio cloning stack resides.
   ADD_DEPENDENCE(control_dblk,           rootInit_Edt, rootInit_Deps_t, control_Dep,           DB_MODE_RW, "rootLaunch", "control"); // Provide datablock into which it is to parse the command line.
   ADD_DEPENDENCE(scratchAllObjects_dblk, rootInit_Edt, rootInit_Deps_t, scratchAllObjects_Dep, DB_MODE_RW, "rootLaunch", "scratchAllObjects"); // Provide db into which it is to describe the objects to model.

// This EDT is done, and should die.  rootInit_Edt should fire now.

   return NULL_GUID;
} // rootLaunch


// **************************************************************************************************************************************************************************************************************
// rootInit_Func
// -- Topology:  mainEdt --> Launch(create datablocks) --> Init(initialize contents of datablocks, create children) --> Clone(steady state: perform service requests of children; clone again)
//
// This EDT does the following:
//   * Parses the command line into control_t and allScratchObjects_t.
//   * Creates one instance of blockLaunch for each unrefined block in the problem mesh, of which there are npx*npy*npz of them.  These are the "children" of the root.  Identify position of each by
//     parameters thereto.
//   * Creates the first in the series of rootClone clones, to await the first service request from the children.

ocrGuid_t rootInit_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   int i;

//printf ("Function %36s, File %30s, line %4d\n", __func__, __FILE__, __LINE__); fflush(stdout);
   rootInit_Deps_t  * myDeps = (rootInit_Deps_t  *) depv;
   char         ** argv                   = ((char **)        (myDeps->argv_Dep.ptr));
   ocrGuid_t       meta_dblk              =                   (myDeps->meta_Dep.guid);
   RootMeta_t    * meta                   = ((RootMeta_t *)   (myDeps->meta_Dep.ptr));
   Control_t     * control                = ((Control_t *)    (myDeps->control_Dep.ptr));
   ocrGuid_t       control_dblk           =                   (myDeps->control_Dep.guid);
   AllObjects_t  * scratchAllObjects      = ((AllObjects_t *) (myDeps->scratchAllObjects_Dep.ptr));
   ocrGuid_t       scratchAllObjects_dblk =                   (myDeps->scratchAllObjects_Dep.guid);
   u32 argc = getArgc(argv);

   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->argv_Dep,              rootInit_Deps_t, argv_Dep,              " ", "argv");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->meta_Dep,              rootInit_Deps_t, meta_Dep,              " ", "meta");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->control_Dep,           rootInit_Deps_t, control_Dep,           " ", "control");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->scratchAllObjects_Dep, rootInit_Deps_t, scratchAllObjects_Dep, " ", "scratchAllObjects");

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
      } else if (!strcmp(getArgv(argv, i), "--code")) {
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
            *((int *) 123) = 456;
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
         *((int *) 123) = 456;
         ocrShutdown();
      } else {
         printf("** Error ** Unknown input parameter %s\n", getArgv(argv, i)); fflush(stdout);
         print_help_message();
         *((int *) 123) = 456;
         ocrShutdown();
      }
   }

   if (check_input(control)) {
      *((int *) 123) = 456;
      ocrShutdown();
   }

   if (!control->block_change) {
      control->block_change = control->num_refine;
   }


// Init some derived control information

   control->tol = pow(10.0, ((double) -control->error_tol));

   control->p2[0] = /* p8 not used.  commented out!  p8[0] = */ 1;
   for (i = 0; i < (control->num_refine+1); i++) {
      // Not used:  p8[i+1] = p8[i]*8;
      control->p2[i+1] = control->p2[i]*2;
      // sorted_index[i] = 0;
   }
   int size = control->p2[control->num_refine+1];  // block size is p2[num_refine+1-level] smallest block is size p2[1], so can find its center
   control->mesh_size[0] = control->npx*size;
   control->mesh_size[1] = control->npy*size;
   control->mesh_size[2] = control->npz*size;
#if 0
TODO:
   control->max_mesh_size = mesh_size[0];
   if (mesh_size[1] > max_mesh_size)
      max_mesh_size = mesh_size[1];
   if (mesh_size[2] > max_mesh_size)
      max_mesh_size = mesh_size[2];
   if ((num_pes+1) > max_mesh_size)
      max_mesh_size = num_pes + 1;
#endif

   rootClone_Deps_t cloneDeps;
   depv = &cloneDeps.firstDependenceSlot;
   DbSize_t * dbSize = meta->dbSize;

   DbSize(cloneDeps.whoAmI_Dep)               = sizeof(ocrGuid_t);
   DbSize(cloneDeps.meta_Dep)                 = sizeof_RootMeta_t;
   DbSize(cloneDeps.control_Dep)              = sizeof_Control_t;
   DbSize(cloneDeps.goldenChecksum_Dep)       = sizeof_Checksum_t;
   DbSize(cloneDeps.scratchChecksum_Dep)      = sizeof_Checksum_t;
   for(i = 0; i < countof_rootClone_AnnexDeps_t; i++) {
      DbSize(cloneDeps.serviceRequest_Dep[i]) = -9999;
   }

// Init our meta data.
// First, init the Cloning State.

   Frame_Header_t * topOfStack  = (Frame_Header_t *) meta->cloningState.stack;
   topOfStack[0].resumption_case_num                  = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[0].my_size                              = sizeof_Frame_Header_t;  // Distance to first callee's frame.
   topOfStack[0].caller_size                          = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[0].validate_callers_prep_for_suspension = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[1].resumption_case_num                  = 0;                      // Prepare for first callee.
   topOfStack[1].my_size                              = -9999;                  // Size of callee's frame is not known by caller.
   topOfStack[1].caller_size                          = sizeof_Frame_Header_t;  // To tell callee how much to pop when it returns.
   meta->cloningState.topPtrAdjRecOffset              = -9999;
   meta->cloningState.numberOfDatablocks              = countof_rootClone_Deps_t;
   meta->cloningState.cloneNum                        = 0;

//   * Create the first in the series of root clones, to await the first service request from the children.

   rootClone_Params_t rootClone_Params;
   rootClone_Params.isFirstChecksum = 1;                               // Init to TRUE
   rootClone_Params.scratchAllObjects_dblk = scratchAllObjects_dblk;   // Pass in as a param, so first instance of rootClone can clean up.
   ocrEdtTemplateCreate (&rootClone_Params.rootClone_Template,         // Guid of template
                         rootClone_Func,                               // Top level function of EDT
                         countof_rootClone_Params_t,                   // Number of params
                         countof_rootClone_Deps_t);                    // Number of dependencies
   ocrGuid_t     rootClone_Edt;
#ifdef NANNY_ON_STEROIDS
#define NANNYLEN 200
           char nanny[NANNYLEN];
   sprintf(nanny, "clone=%5d", meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrEdtCreate (&rootClone_Edt,
                         SLOT(rootClone_Deps_t, whoAmI_Dep),
                         rootClone_Params.rootClone_Template,
                         EDT_PARAM_DEF,
                         (u64 *) &rootClone_Params,
                         EDT_PARAM_DEF,
                         NULL,
                         EDT_PROP_NONE,
                         NULL_HINT,
                         NULL,
                        __FILE__,
                        __func__,
                        __LINE__,
                        nanny,
                        "rootClone");
   ocrGuid_t     goldenChecksum_dblk;
   void * dummy = NULL;
   gasket__ocrDbCreate(&goldenChecksum_dblk,  &dummy, sizeof_Checksum_t, __FILE__, __func__, __LINE__,      "rootInit", "goldenChecksum");
   ADD_DEPENDENCE(goldenChecksum_dblk,  rootClone_Edt, rootClone_Deps_t, goldenChecksum_Dep,  DB_MODE_RW,   "rootInit", "goldenChecksum");
   ADD_DEPENDENCE(NULL_GUID,            rootClone_Edt, rootClone_Deps_t, scratchChecksum_Dep, DB_MODE_NULL, "rootInit", "scratchChecksum");
   ADD_DEPENDENCE(control_dblk,         rootClone_Edt, rootClone_Deps_t, control_Dep,         DB_MODE_RO,   "rootInit", "control");

//   * Create one instance of blockLaunch for each unrefined block in the problem mesh, of which there are npx*npy*npz of them.  These are the "children".  Identify position of each by parameters thereto.

   blockLaunch_Params_t blockLaunch_Params;

   ocrEdtTemplateCreate (&blockLaunch_Params.template.blockLaunch_Template,   // Guid of template
                         blockLaunch_Func,                                    // Top level function of EDT
                         countof_blockLaunch_Params_t,                        // Number of params
                         countof_blockLaunch_Deps_t);                         // Number of dependencies
   ocrEdtTemplateCreate (&blockLaunch_Params.template.blockInit_Template,     // Guid of template
                         blockInit_Func,                                      // Top level function of EDT
                         countof_blockInit_Params_t,                          // Number of params
                         countof_blockInit_Deps_t);                           // Number of dependencies
   ocrEdtTemplateCreate (&blockLaunch_Params.template.blockClone_Template,    // Guid of template
                         blockClone_Func,                                     // Top level function of EDT
                         countof_blockClone_Params_t,                         // Number of params
                         EDT_PARAM_UNK);                                      // Number of dependencies
   ocrEdtTemplateCreate (&blockLaunch_Params.template.parentInit_Template,    // Guid of template
                         parentInit_Func,                                     // Top level function of EDT
                         countof_parentInit_Params_t,                         // Number of params
                         countof_parentInit_Deps_t);                          // Number of dependencies
   ocrEdtTemplateCreate (&blockLaunch_Params.template.parentClone_Template,   // Guid of template
                         parentClone_Func,                                    // Top level function of EDT
                         countof_parentClone_Params_t,                        // Number of params
                         countof_parentClone_Deps_t);                         // Number of dependencies
// Create a labeled guid range, providing one index for each potential face exchange (36 per block -- 6 towards coarser neighbors, 6 towards same-grained neighbors, 24 toward finer neighbors) times the
// number of blocks possible at ALL refinement levels.
   unsigned long long numBlksAllLevels = (011111111111111111LL & ((1LL << ((control->num_refine+1) * 3LL)) - 1LL)) * control->npx * control->npy * control->npz;
   unsigned long long roundRobinSpan   = 1; // TODO:  if we decide we need more than 1 "robin in the nest", modify both here and at usage site.  Usage site presently just assumes ONE, as I think this is enough.
   unsigned long long numCommBatches   = 1; // TODO:  if we decide to fan-out and perform the comm_vars-sized batches in openMP-style parallelism, increase to ceiling(num_vars/comm_vars).  Propagate change.
   ocrGuidRangeCreate (&blockLaunch_Params.labeledGuidRangeForHaloExchange, numBlksAllLevels * 36 * roundRobinSpan * numCommBatches, GUID_USER_EVENT_STICKY);

   int xLim, yLim, zLim, idep;
   xLim     = control->npx;
   yLim     = control->npy;
   zLim     = control->npz;
   gasket__ocrDbRelease  (control_dblk,           __FILE__, __func__, __LINE__, " ", "control");           control = NULL;
   gasket__ocrDbRelease  (scratchAllObjects_dblk, __FILE__, __func__, __LINE__, " ", "scratchAllObjects"); scratchAllObjects = NULL;

   idep = 0;
   for (      blockLaunch_Params.xPos = 0; blockLaunch_Params.xPos < xLim; blockLaunch_Params.xPos++) {
      for (   blockLaunch_Params.yPos = 0; blockLaunch_Params.yPos < yLim; blockLaunch_Params.yPos++) {
         for (blockLaunch_Params.zPos = 0; blockLaunch_Params.zPos < zLim; blockLaunch_Params.zPos++) {
            gasket__ocrEventCreate(&blockLaunch_Params.conveyServiceRequestToParent_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyServiceRequestToParent");  // Create the On Deck Event; record it in our meta.

#ifdef NANNY_ON_STEROIDS
           char nanny[NANNYLEN];
           sprintf(nanny, "serviceRequest[%12d], for unrefined block[%4d][%4d][%4d]", idep, blockLaunch_Params.xPos, blockLaunch_Params.yPos, blockLaunch_Params.zPos);
#else
#define nanny NULL
#endif
            EVT_DEPENDENCE(blockLaunch_Params.conveyServiceRequestToParent_Event, rootClone_Edt, rootClone_Deps_t, serviceRequest_Dep[idep], DB_MODE_RO, "rootInit", nanny);

            ocrGuid_t   blockLaunch_Edt;
#ifdef NANNY_ON_STEROIDS
            sprintf(nanny, "creating: xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", blockLaunch_Params.xPos, blockLaunch_Params.yPos, blockLaunch_Params.zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
            gasket__ocrEdtCreate(&blockLaunch_Edt,
                                 SLOT(blockLaunch_Deps_t, whoAmI_Dep),
                                 blockLaunch_Params.template.blockLaunch_Template,
                                 EDT_PARAM_DEF,
                                 (u64 *) &blockLaunch_Params,
                                 EDT_PARAM_DEF,
                                 NULL,
                                 EDT_PROP_NONE,
                                 NULL_HINT,
                                 NULL,
                                __FILE__,
                                __func__,
                                __LINE__,
                                nanny,
                                "blockLaunch");

#ifdef NANNY_ON_STEROIDS
            sprintf(nanny, "xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", blockLaunch_Params.xPos, blockLaunch_Params.yPos, blockLaunch_Params.zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
            ADD_DEPENDENCE(control_dblk,           blockLaunch_Edt, blockLaunch_Deps_t, control_Dep,    DB_MODE_RO,    nanny, "control");
            ADD_DEPENDENCE(scratchAllObjects_dblk, blockLaunch_Edt, blockLaunch_Deps_t, allObjects_Dep, DB_MODE_RO,    nanny, "scratchAllObjects");

            idep++;
         }
      }
   }

#ifdef NANNY_ON_STEROIDS
   sprintf(nanny, "xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", blockLaunch_Params.xPos, blockLaunch_Params.yPos, blockLaunch_Params.zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrDbRelease  (meta_dblk, __FILE__, __func__, __LINE__, " ", "meta"); meta = NULL;
   ADD_DEPENDENCE(meta_dblk,  rootClone_Edt, rootClone_Deps_t, meta_Dep,     DB_MODE_RW, nanny, "meta");
   REPORT_EDT_DEMISE(myDeps->whoAmI_Dep);
   // Defeat OCR's habit of releasing the input ocrEdtDep_t's, because we have changed many of them to other guids, including some events, which simply must not be released.  This is just a debugging expedient.
   {
      myDeps->whoAmI_Dep.guid              = NULL_GUID;
      myDeps->argv_Dep.guid                = NULL_GUID;
      myDeps->meta_Dep.guid                = NULL_GUID;
      myDeps->control_Dep.guid             = NULL_GUID;
      myDeps->scratchAllObjects_Dep.guid   = NULL_GUID;
   }

   return NULL_GUID;

} // rootInit_Func


// **************************************************************************************************************************************************************************************************************
// rootClone_Func
// -- Topology:  mainEdt --> rPLaunch(create datablocks) --> rPInit(initialize contents of datablocks, create children) --> rPClone(steady state: perform service requests of children; clone again)
//
// Provide services to the top-level blocks (i.e. the blocks that are totally unrefined, or the parents of refined blocks descended from them.)

ocrGuid_t rootClone_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   int i;

   rootClone_Deps_t   * myDeps   = (rootClone_Deps_t   *) depv;
   rootClone_Params_t * myParams = (rootClone_Params_t *) paramv;

   RootMeta_t * meta    = (RootMeta_t *) myDeps->meta_Dep.ptr;
   Control_t  * control = (Control_t *)  myDeps->control_Dep.ptr;

#ifdef NANNY_ON_STEROIDS
   char nanny[NANNYLEN];
   sprintf(nanny, "clone=%5d", meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif

   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->meta_Dep,              rootClone_Deps_t, meta_Dep,             nanny, "meta");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->control_Dep,           rootClone_Deps_t, control_Dep,          nanny, "control");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->goldenChecksum_Dep,    rootClone_Deps_t, goldenChecksum_Dep,   nanny, "goldenChecksum");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->scratchChecksum_Dep,   rootClone_Deps_t, scratchChecksum_Dep,  nanny, "scratchChecksum");
   for (i = 0; i < countof_rootClone_AnnexDeps_t; i++) {
#ifdef NANNY_ON_STEROIDS
      char name[NANNYLEN];
      sprintf(name, "serviceRequest[%8d]", i);
#else
#define name NULL
#endif
      INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[i], rootClone_Deps_t, serviceRequest_Dep[i], nanny, name);
   }

   // If this is the first exectuion of the rootClone, we can clean up some initialization datablocks.

   if (!ocrGuidIsNull(myParams->scratchAllObjects_dblk)) {
      void * dummy = NULL;
      gasket__ocrDbDestroy(&myParams->scratchAllObjects_dblk, &dummy, __FILE__, __func__, __LINE__, "rootClone", "scratchAllObjects");
   }

   meta    = (RootMeta_t *)  myDeps->meta_Dep.ptr;
   control = (Control_t *)   myDeps->control_Dep.ptr;

//printf ("Function %36s, File %30s, line %4d\n", __func__, __FILE__, __LINE__); fflush(stdout);


   meta->cloningState.tos          = ((char *)                  (((unsigned long long) meta->cloningState.stack)));
   meta->cloningState.topPtrAdjRec = ((PtrAdjustmentRecord_t *) (((unsigned long long) meta->cloningState.stack) + meta->cloningState.topPtrAdjRecOffset));
   meta->cloningState.cloneNum++;
//printf ("Function %36s, File %30s, line %4d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->cloningState.cloneNum); fflush(stdout);
   meta->cloningState.cloningOpcode = OpcodeUnspecified;
   Frame_Header_t * topOfStack  = (Frame_Header_t *) meta->cloningState.stack;

   topOfStack[1].validate_callers_prep_for_suspension = 1;     // Set "canary trap" for first callee.
   meta->cloningState.returnCanary                    = 1;     // Init the "return canary trap" to "live", to detect a return from a suspendable function via a return stmt instead of the macro.
   rootClone_SoupToNuts(myParams, depc, depv, meta->dbSize);   // Resume (or start, the first time) the algorithm.  When we get back, it is either done or needing to clone.
   if (meta->cloningState.returnCanary == 1) {                 // If the return canary is "live" the callee must have used a return statement.  We need it to use the macro!
      printf ("ERROR: %s at line %d: Canary trap on RETURN from the callee.  Change callee to return through the NORMAL_RETURN_SEQUENCE macro.\n", __FILE__, __LINE__); fflush(stdout); \
      printf ("ERROR: Other possibility is that callee is not (yet) a SUSPENDABLE function, but it is being identified as such by the caller.\n"); fflush(stdout); \
      *((int *) 123) = 456; \
      ocrShutdown();
   }

   if (topOfStack[1].resumption_case_num != 0) {               // Create a clone EDT, cause it to fire, and cause this EDT has to terminate.

      meta->cloningState.topPtrAdjRecOffset = ((int) ((unsigned long long) meta->cloningState.topPtrAdjRec) - ((unsigned long long) meta->cloningState.stack));

      ocrGuid_t rootClone_Edt;
#ifdef NANNY_ON_STEROIDS
      char nanny[NANNYLEN];
      sprintf(nanny, "clone=%5d", meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
      gasket__ocrEdtCreate(&rootClone_Edt,                             // Guid of the EDT created to continue at function blockContinuaiton_Func.
                           SLOT(rootClone_Deps_t, whoAmI_Dep),
                           myParams->rootClone_Template,               // Template for the EDT we are creating.
                           EDT_PARAM_DEF,
                           (u64 *) myParams,
                           EDT_PARAM_DEF,
                           NULL,
                           EDT_PROP_NONE,
                           NULL_HINT,
                           NULL,
                           __FILE__,
                           __func__,
                           __LINE__,
                           nanny,
                           "rootClone");

      switch (meta->cloningState.cloningOpcode) {
      case OpcodeUnspecified:
         printf ("In Root, Cloning opcode unspecified\n"); fflush(stdout);
         *((int *) 0) = 123;
         ocrShutdown();
         break;
      case SeasoningOneOrMoreDbCreates:
      case ReceivingACommunication:
         // The site that sent us this opcode has put the guid of the events for the remote newResources into the catalog already.  Just drop out of this switch statement and add dependencies of our clone.
         break;
      default:
         printf ("In Root, Cloning opcode not yet handled,  opcode = %d / 0x%x\n", meta->cloningState.cloningOpcode, meta->cloningState.cloningOpcode); fflush(stdout);
         *((int *) 0) = 123;
         ocrShutdown();
         break;
      }

      gasket__ocrDbRelease(   myDeps->control_Dep.guid,                                                                               __FILE__, __func__, __LINE__, nanny, "control");
      ADD_DEPENDENCE(         myDeps->control_Dep.guid,                 rootClone_Edt, rootClone_Deps_t, control_Dep,                 DB_MODE_RW,                   nanny, "control")
      gasket__ocrDbRelease(   myDeps->goldenChecksum_Dep.guid,                                                                        __FILE__, __func__, __LINE__, nanny, "goldenChecksum");
      ADD_DEPENDENCE(         myDeps->goldenChecksum_Dep.guid,          rootClone_Edt, rootClone_Deps_t, goldenChecksum_Dep,          DB_MODE_RO,                   nanny, "goldenChecksum")
      if (ocrGuidIsNull(myDeps->scratchChecksum_Dep.guid)) {
         ADD_DEPENDENCE(      NULL_GUID,                                rootClone_Edt, rootClone_Deps_t, scratchChecksum_Dep,         DB_MODE_NULL,                 nanny, "scratchChecksum")
      } else {
         gasket__ocrDbRelease(myDeps->scratchChecksum_Dep.guid,                                                                       __FILE__, __func__, __LINE__, nanny, "scratchChecksum");
         ADD_DEPENDENCE(      myDeps->scratchChecksum_Dep.guid,         rootClone_Edt, rootClone_Deps_t, scratchChecksum_Dep,         DB_MODE_RW,                   nanny, "scratchChecksum")
      }
      for (i = 0; i < countof_rootClone_AnnexDeps_t; i++) {
#ifdef NANNY_ON_STEROIDS
         char name[NANNYLEN];
         sprintf(name, "serviceRequest[%8d]", i);
#else
#define nanny NULL
#endif
         if (meta->cloningState.cloningOpcode == SeasoningOneOrMoreDbCreates) {
            ADD_DEPENDENCE(      myDeps->serviceRequest_Dep[i].guid, rootClone_Edt, rootClone_Deps_t, serviceRequest_Dep[i], DB_MODE_RO,                   nanny, name);
         } else {
            EVT_DEPENDENCE(      myDeps->serviceRequest_Dep[i].guid, rootClone_Edt, rootClone_Deps_t, serviceRequest_Dep[i], DB_MODE_RO,                   nanny, name);
            myDeps->serviceRequest_Dep[i].guid = NULL_GUID;
         }
      }

      gasket__ocrDbRelease(   myDeps->meta_Dep.guid,                                                                                  __FILE__, __func__, __LINE__, nanny, "meta");
      ADD_DEPENDENCE(         myDeps->meta_Dep.guid,                    rootClone_Edt, rootClone_Deps_t, meta_Dep,                    DB_MODE_RW,                   nanny, "meta")

   } else {  // (topOfStack[1].resumption_case_num != 0)    If that was the very last iteration, clean up and shut down.
      gasket__ocrDbDestroy(&myDeps->goldenChecksum_Dep.guid, &myDeps->goldenChecksum_Dep.ptr, __FILE__, __func__, __LINE__, "rootClone", "goldenChecksum");
      gasket__ocrDbDestroy(&myDeps->control_Dep.guid,        &myDeps->control_Dep.ptr,        __FILE__, __func__, __LINE__, "rootClone", "control");
      gasket__ocrDbDestroy(&myDeps->meta_Dep.guid,           &myDeps->meta_Dep.ptr,           __FILE__, __func__, __LINE__, "rootClone", "meta");
      printf ("Success!  miniAMR shutting down normally.\n"); fflush(stdout);
      ocrShutdown();
   }

   // Defeat OCR's habit of releasing the input ocrEdtDep_t's, because we have changed many of them to other guids, including some events, which simply must not be released.  This is just a debugging expedient.
   REPORT_EDT_DEMISE(myDeps->whoAmI_Dep);
   {
      myDeps->meta_Dep.guid                = NULL_GUID;
      myDeps->whoAmI_Dep.guid              = NULL_GUID;
      myDeps->control_Dep.guid             = NULL_GUID;
      myDeps->goldenChecksum_Dep.guid      = NULL_GUID;
      myDeps->scratchChecksum_Dep.guid     = NULL_GUID;
      int i;
      for (i=0; i < countof_rootClone_AnnexDeps_t; i++) myDeps->serviceRequest_Dep[i].guid = NULL_GUID;
   }
   return NULL_GUID;

} // rootClone_Func


// **************************************************************************************************************************************************************************************************************
// rootClone_SoupToNuts
//
// This is the top-level function in the calling topology of the several functions that do the actual processing of the root's service duties.
//
void rootClone_SoupToNuts(rootClone_Params_t * myParams, u32 depc, ocrEdtDep_t depv[], DbSize_t * dbSize) {

   typedef struct {
      Frame_Header_t myFrame;
      //double t1, t2;
      DataExchangeOpcode_t opcode;
      struct {
         Control_t * control;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__rootClone_SoupToNuts_t;

#define control (lcl->pointers.control)
#define opcode  (lcl->opcode)

   int idep;
   rootClone_Deps_t * myDeps = (rootClone_Deps_t *) depv;
   RootMeta_t       * meta   = myDeps->meta_Dep.ptr;

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__rootClone_SoupToNuts_t)

//printf ("Function %36s, File %30s, line %4d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->cloningState.cloneNum); fflush(stdout);

   control = myDeps->control_Dep.ptr;

// Looping, perform the operations requested by the children.  Keep doing so until the final shutdown request arrives.

   do {

      // Process the service requested from children.

      opcode = (((DbCommHeader_t *) (myDeps->serviceRequest_Dep[0].ptr)))->serviceOpcode;
      for (idep = 1; idep < countof_rootClone_AnnexDeps_t; idep++) {
         if (((DbCommHeader_t *) (myDeps->serviceRequest_Dep[idep].ptr))->serviceOpcode != opcode) {
            *((int *) 123) = 456;   // Service opcodes from children don't match!
         }
      }

      if (opcode == Operation_Checksum) {
         CALL_SUSPENDABLE_CALLEE
         checksum_RootFinalAggregation (depc, depv, myParams);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      } else if (opcode == Operation_Plot) {
//printf ("Function %36s, File %30s, line %4d, clone=%4d    calling propagatePlot)\n", __func__, __FILE__, __LINE__, meta->cloningState.cloneNum); fflush(stdout);
         plot_RootFinalAggregation (depc, depv);
//PROFILE:      } else if (opcode == Operation_Profile) {
//PROFILE:         //CALL_SUSPENDABLE_CALLEE
//PROFILE:         reportProfileResults (meta);
//PROFILE:         //DEBRIEF_SUSPENDABLE_FUNCTION(;)
      } else if (opcode != Operation_Shutdown) {
         printf ("Unrecognized opcode received by rootClone, opcode = %d\n", opcode); fflush(stdout);
         *((int *) 123) = 456;
         ocrShutdown();
      } // opcode

      // Now for each block, clean up event that brought THIS operand to us, and put "on deck" event into the catalog so that it will be used to plumb operand dependence next time a service request is sought.
      for (idep = 0; idep < countof_rootClone_AnnexDeps_t; idep++) {
         DbCommHeader_t * operand = ((DbCommHeader_t *) (myDeps->serviceRequest_Dep[idep].ptr));
#ifdef NANNY_ON_STEROIDS
         char nanny[NANNYLEN];
         sprintf(nanny, "clone=%5d  index=[%d]", meta->cloningState.cloneNum, idep);
#else
#define nanny NULL
#endif
         gasket__ocrEventDestroy(&operand->atBat_Event, __FILE__, __func__, __LINE__, nanny, "serviceRequest[...]");     // Destroy event that brought us our operand
         ocrGuid_t onDeck = operand->onDeck_Event;
         gasket__ocrDbDestroy(&myDeps->serviceRequest_Dep[idep].guid, &myDeps->serviceRequest_Dep[idep].ptr, __FILE__, __func__, __LINE__, nanny, "serviceRequest[...]");
         myDeps->serviceRequest_Dep[idep].guid = onDeck;
      }

      // Start a cloning request, to obtain the next service request from root's children.

      if (opcode == Operation_Shutdown) {
         SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)    // Top-level will sense normal completion, and just end this EDT (without creating any more clones).
      }
      meta->cloningState.cloningOpcode = ReceivingACommunication;
      SUSPEND__RESUME_IN_CLONE_EDT(;)

   } while (1);

   SUSPENDABLE_FUNCTION_EPILOGUE

#undef  control
#undef  opcode

} // rootClone_SoupToNuts



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
   printf("--num_refine - (>= 0, <= 15) number of levels of refinement\n");
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
   printf("--code - closely mimic communication of different applications (for which this is merely a proxy-app)\n");
   printf("         0 minimal sends, 1 send ghosts, 2 send ghosts and process on send\n");
   printf("--permute - altenates directions in communication\n");
   printf("--refine_ghost - use full extent of block (including ghosts) to determine if block is refined\n");
   printf("--num_objects - (>= 0) number of objects to cause refinement\n");
   printf("--object - type, position, movement, size, size rate of change\n");

   printf("All associated settings are integers except for objects\n"); fflush(stdout);
} // print_help_message


int check_input(Control_t * control) {
   int error = 0;

   if (control->x_block_size < 1 || control->y_block_size < 1 || control->z_block_size < 1) {
      printf("block size must be positive\n"); fflush(stdout);
      error = 1;
   }
   if (((control->x_block_size/2)*2) != control->x_block_size) {
      printf("block size in x direction must be even\n"); fflush(stdout);
      error = 1;
   }
   if (((control->y_block_size/2)*2) != control->y_block_size) {
      printf("block size in y direction must be even\n"); fflush(stdout);
      error = 1;
   }
   if (((control->z_block_size/2)*2) != control->z_block_size) {
      printf("block size in z direction must be even\n"); fflush(stdout);
      error = 1;
   }
   if (control->num_refine < 0) {
      printf("number of refinement levels must be non-negative\n"); fflush(stdout);
      error = 1;
   }
   if (control->num_refine > 15) {
      printf("number of refinement levels must not exceed fifteen\n"); fflush(stdout);
      error = 1;
   }
   if (control->block_change < 0) {
      printf("number of refinement levels must be non-negative\n"); fflush(stdout);
      error = 1;
   }
   if (control->num_vars < 1) {
      printf("number of variables must be positive\n"); fflush(stdout);
      error = 1;
   }
   if (control->stencil != 7 && control->stencil != 27) {
      printf("illegal value for stencil\n"); fflush(stdout);
      error = 1;
   }
   if (control->stencil == 27 && control->num_refine && !control->uniform_refine)
      printf("WARNING: 27 point stencil with non-uniform refinement: answers may diverge\n"); fflush(stdout);
   if (control->comm_vars == 0 || control->comm_vars > control->num_vars)
      control->comm_vars = control->num_vars;
   if (control->code < 0 || control->code > 2) {
      printf("code must be 0, 1, or 2\n"); fflush(stdout);
      error = 1;
   }

   return (error);
}
