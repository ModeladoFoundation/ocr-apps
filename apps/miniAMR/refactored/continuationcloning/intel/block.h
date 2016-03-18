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

#ifndef __BLOCK_H__
#define __BLOCK_H__

#define MAX_RANKS 1000

#ifdef BUILD_OCR_VERSION
#include <ocr.h>
typedef struct {    // Meta information for description of a datablock.
   ocrGuid_t guid;
   unsigned long long size;
} Dbmeta_t;
#define GUID_ROUND_ROBIN_SPAN 4    // **** POWER OF TWO!!!
typedef struct{
   u64 numberOfPes;
   ocrGuid_t labeledGuidRange;     // Size is (num_pes * num_pes * GUID_ROUND_ROBIN_SPAN) guids
}shared_t;

#else
#define NULL_GUID 0
typedef struct {
   void *    ptr;
   int       guid;  // Junk for non-OCR versions.
} ocrEdtDep_t;
typedef struct {    // Meta information for description of a datablock.
   unsigned long long guid;
   unsigned long long size;
} Dbmeta_t;
#endif

#include "comm.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>

#define STRINGIFY_X(a) #a
#define STRINGIFY(a) STRINGIFY_X(a)

typedef struct {
   int number;
   int level;
   int refine;
   int new_proc;
   int parent;       // if original block -1,
                     // else if on node, number in structure
                     // else (-2 - parent->number)
   int parent_node;
   int child_number;
   int nei_refine[6];
   int nei_level[6];  /* 0 to 5 = W, E, S, N, D, U; use -2 for boundary */
   int nei[6][2][2];  /* negative if off processor (-1 - proc) */
   int cen[3];
   //double ****array;   // This is now split out as Cells_t
} Block_t;

typedef struct {
   int number;
   int level;
   int parent;      // -1 if original block
   int parent_node;
   int child_number;
   int refine;
   int child[8];    // n if on node, number if not
                    // if negative, then onnode child is a parent (-1 - n)
   int child_node[8];
   int cen[3];
} Parent_t;

typedef struct {
   int number;     // number of block
   int n;          // position in block array
} Sorted_Block_t;

typedef struct {
   int type;
   int bounce;
   double cen[3];
   double orig_cen[3];
   double move[3];
   double orig_move[3];
   double size[3];
   double orig_size[3];
   double inc[3];
} Object_t;

typedef struct {
   int cen[3];
   int number;
   int n;
   int proc;
   int new_proc;
} Dot_t;

// These are the large-sized global constructs.  (Small globals are in the Globals_t struct.)
// All large constructs are here except Blocks_t.  It is intended that this struct should be
// stored far from the processor, but only as far as necessary.  For instance, if there is
// going to be one Lead-EDT (akin to the MPI rank processor) per chip, placing the Bulk_t at
// chip-level memory would be excellent.  This struct is read/write for all activities at the
// Lead-EDT level, but it is read-only for the stencil operation, which is the only place
// (presently) where we will go massively parallel (akin to OPENMP under each MPI rank).  Note,
// though, that the volume of reading of data from Bulk_t at the stencil level is very small
// relative to the rest of the activity, and is limited to the Sorted_Block_t construct.  If that
// construct is "postured" so that it is a bit closer to the processor doing the stencil, so
// much the better, but that is not of great importance.

typedef void Bulk_t;

#define sizeof_array_of_grid_sum      (num_vars * sizeof(double))
#define sizeof_array_of_from          (num_pes * sizeof(int))
#define sizeof_array_of_to            (num_pes * sizeof(int))
#define sizeof_array_of_dots          (max_num_dots * sizeof(Dot_t))
#define sizeof_array_of_sorted_list   (max_num_blocks * sizeof(Sorted_Block_t))
#define sizeof_array_of_objects       (num_objects * sizeof(Object_t))
#define sizeof_array_of_blocks        (max_num_blocks * sizeof(Block_t))
#define sizeof_array_of_cells         (max_num_blocks * num_vars * (x_block_size + 2) * (y_block_size + 2) * (z_block_size + 2) * sizeof(double))
#define sizeof_array_of_parents       (max_num_parents * sizeof(Parent_t))

#define OBTAIN_ACCESS_TO_grid_sum \
   double       * grid_sum      = ((double *)        (((char *) bulk)));
#define OBTAIN_ACCESS_TO_from \
   int          * from          = ((int *)           (((char *) bulk) + \
                                                      sizeof_array_of_grid_sum));
#define OBTAIN_ACCESS_TO_to \
   int          * to            = ((int *)           (((char *) bulk) + \
                                                      sizeof_array_of_grid_sum + \
                                                      sizeof_array_of_from));
#define OBTAIN_ACCESS_TO_dots \
   Dot_t          * dots          = ((Dot_t *)       (((char *) bulk) + \
                                                      sizeof_array_of_grid_sum + \
                                                      sizeof_array_of_from + \
                                                      sizeof_array_of_to));
#define OBTAIN_ACCESS_TO_sorted_list \
   Sorted_Block_t * sorted_list = ((Sorted_Block_t *)(((char *) bulk) + \
                                                      sizeof_array_of_grid_sum + \
                                                      sizeof_array_of_from + \
                                                      sizeof_array_of_to + \
                                                      sizeof_array_of_dots));
#define OBTAIN_ACCESS_TO_objects \
   Object_t       * objects     = ((Object_t *)      (((char *) bulk) + \
                                                      sizeof_array_of_grid_sum + \
                                                      sizeof_array_of_from + \
                                                      sizeof_array_of_to + \
                                                      sizeof_array_of_dots + \
                                                      sizeof_array_of_sorted_list));
#define OBTAIN_ACCESS_TO_parents \
   Parent_t     * parents       = ((Parent_t *)      (((char *) bulk) + \
                                                      sizeof_array_of_grid_sum + \
                                                      sizeof_array_of_from + \
                                                      sizeof_array_of_to + \
                                                      sizeof_array_of_dots + \
                                                      sizeof_array_of_sorted_list + \
                                                      sizeof_array_of_objects));

#define OBTAIN_ACCESS_TO_cells \
   double       * cellstorage = ((double *)          (((char *) bulk) + \
                                                      sizeof_array_of_grid_sum + \
                                                      sizeof_array_of_from + \
                                                      sizeof_array_of_to + \
                                                      sizeof_array_of_dots + \
                                                      sizeof_array_of_sorted_list + \
                                                      sizeof_array_of_objects + \
                                                      sizeof_array_of_parents)); \
   typedef double Cell_t [num_vars][x_block_size+2][y_block_size+2][z_block_size+2]; \
   Cell_t  *cells = ((Cell_t *) (cellstorage));

#define OBTAIN_ACCESS_TO_blocks \
   Block_t      * blocks      = ((Block_t *)         (((char *) bulk) + \
                                                      sizeof_array_of_grid_sum + \
                                                      sizeof_array_of_from + \
                                                      sizeof_array_of_to + \
                                                      sizeof_array_of_dots + \
                                                      sizeof_array_of_sorted_list + \
                                                      sizeof_array_of_objects + \
                                                      sizeof_array_of_parents + \
                                                      sizeof_array_of_cells));

#define sizeof_Bulk_t \
                                                     (sizeof_array_of_grid_sum + \
                                                      sizeof_array_of_from + \
                                                      sizeof_array_of_to + \
                                                      sizeof_array_of_dots + \
                                                      sizeof_array_of_sorted_list + \
                                                      sizeof_array_of_objects + \
                                                      sizeof_array_of_parents + \
                                                      sizeof_array_of_cells + \
                                                      sizeof_array_of_blocks)


typedef struct {
   union {
      void * junk;                                     // This is merely a totally portable way to assure that the Frame_Header_t is aligned to an 8-byte boundary.
      struct {
         short resumption_case_num;                    // Zero on initial entry to a function, and upon its final return.
         short my_size;                                // Includes "my" frame header, but NOT my callee's frame header.
         short caller_size;                            // "my_size" of my caller, (so I can pop my frame back to his).
         short validate_callers_prep_for_suspension;   // Caller sets this to one to indicate it is prepared for callee's suspension.
      };
   };
} Frame_Header_t;

typedef struct {
   int  serialNum;          // Serial number of the datablock into which this pointer points.
   int  ptrOffset;          // Offset within the datablock.
} PtrAdjustmentRecord_t;

// These are the small-sized global variables.  (Larger globals are in the Bulk_t struct.)
// It is intended that this struct should be stored "close" to the processor, wheras "Bulk_t"
// is far away.  This struct is read/write for all activities at the Lead-EDT level, but it
// is read-only for the stencil operation, which is the only place (presently) where we
// will go massively parallel (akin to OPENMP under each MPI rank).

typedef struct {
//   int                                       global__just_junk;
//#define       just_junk               (glbl->global__just_junk)
   int                                       global__max_num_blocks;
#define       max_num_blocks          (glbl->global__max_num_blocks)
   int                                       global__target_active;
#define       target_active           (glbl->global__target_active)
   int                                       global__target_max;
#define       target_max              (glbl->global__target_max)
   int                                       global__target_min;
#define       target_min              (glbl->global__target_min)
   int                                       global__num_refine;
#define       num_refine              (glbl->global__num_refine)
   int                                       global__uniform_refine;
#define       uniform_refine          (glbl->global__uniform_refine)
   int                                       global__x_block_size;
#define       x_block_size            (glbl->global__x_block_size)
   int                                       global__y_block_size;
#define       y_block_size            (glbl->global__y_block_size)
   int                                       global__z_block_size;
#define       z_block_size            (glbl->global__z_block_size)
   int                                       global__num_vars;
#define       num_vars                (glbl->global__num_vars)
   int                                       global__comm_vars;
#define       comm_vars               (glbl->global__comm_vars)
   int                                       global__init_block_x;
#define       init_block_x            (glbl->global__init_block_x)
   int                                       global__init_block_y;
#define       init_block_y            (glbl->global__init_block_y)
   int                                       global__init_block_z;
#define       init_block_z            (glbl->global__init_block_z)
   int                                       global__reorder;
#define       reorder                 (glbl->global__reorder)
   int                                       global__npx;
#define       npx                     (glbl->global__npx)
   int                                       global__npy;
#define       npy                     (glbl->global__npy)
   int                                       global__npz;
#define       npz                     (glbl->global__npz)
   int                                       global__inbalance;
#define       inbalance               (glbl->global__inbalance)
   int                                       global__refine_freq;
#define       refine_freq             (glbl->global__refine_freq)
   int                                       global__report_diffusion;
#define       report_diffusion        (glbl->global__report_diffusion)
   int                                       global__checksum_freq;
#define       checksum_freq           (glbl->global__checksum_freq)
   int                                       global__stages_per_ts;
#define       stages_per_ts           (glbl->global__stages_per_ts)
   int                                       global__error_tol;
#define       error_tol               (glbl->global__error_tol)
   int                                       global__num_tsteps;
#define       num_tsteps              (glbl->global__num_tsteps)
   int                                       global__stencil;
#define       stencil                 (glbl->global__stencil)
   int                                       global__report_perf;
#define       report_perf             (glbl->global__report_perf)
   int                                       global__plot_freq;
#define       plot_freq               (glbl->global__plot_freq)
   int                                       global__lb_opt;
#define       lb_opt                  (glbl->global__lb_opt)
   int                                       global__block_change;
#define       block_change            (glbl->global__block_change)
   int                                       global__code;
#define       code                    (glbl->global__code)
   int                                       global__permute;
#define       permute                 (glbl->global__permute)
   int                                       global__nonblocking;
#define       nonblocking             (glbl->global__nonblocking)
   int                                       global__refine_ghost;
#define       refine_ghost            (glbl->global__refine_ghost)
   int                                       global__num_objects;
#define       num_objects             (glbl->global__num_objects)
   int                                       global__x_block_half;
#define       x_block_half            (glbl->global__x_block_half)
   int                                       global__y_block_half;
#define       y_block_half            (glbl->global__y_block_half)
   int                                       global__z_block_half;
#define       z_block_half            (glbl->global__z_block_half)
   int                                       global__num_pes;
#define       num_pes                 (glbl->global__num_pes)
   int                                       global__mesh_size[3];
#define       mesh_size               (glbl->global__mesh_size)
   int                                       global__msg_len[3][4];
#define       msg_len                 (glbl->global__msg_len)
   int                                       global__max_num_dots;
#define       max_num_dots            (glbl->global__max_num_dots)
   //int max_mesh_size;   // Comment out; made a mere local to the single function that uses it.
   int                                       global__max_num_parents;
#define       max_num_parents         (glbl->global__max_num_parents)
   int                                       global__num_parents;
#define       num_parents             (glbl->global__num_parents)
   int                                       global__max_active_parent;
#define       max_active_parent       (glbl->global__max_active_parent)
   int                                       global__cur_max_level;
#define       cur_max_level           (glbl->global__cur_max_level)
   int                                       global__num_active;
#define       num_active              (glbl->global__num_active)
   int                                       global__max_active_block;
#define       max_active_block        (glbl->global__max_active_block)
   int                                       global__global_active;
#define       global_active           (glbl->global__global_active)
   int                                       global__global_max_b;
#define       global_max_b            (glbl->global__global_max_b)
   int                                       global__num_blocks[11];      // Number of elements is max number of refinement levels, plus one.
#define       num_blocks              (glbl->global__num_blocks)
   int                                       global__block_start[11];      // Number of elements is max number of refinement levels, plus one.
#define       block_start             (glbl->global__block_start)
   int                                       global__local_num_blocks[11]; // Number of elements is max number of refinement levels, plus one.
#define       local_num_blocks        (glbl->global__local_num_blocks)
   int                                       global__p2[12];               // Number of elements is max number of refinement levels, plus two.
#define       p2                      (glbl->global__p2)
//   int                                       global__p8[12];             // Deleted.  Not actually used, other than just initialized.
//#define       p8                      (glbl->global__p8)
   int                                       global__sorted_index[12];     // Number of elements is max number of refinement levels, plus two.
#define       sorted_index            (glbl->global__sorted_index)
   int                                       global__num_dots;
#define       num_dots                (glbl->global__num_dots)
   int                                       global__max_active_dot;
#define       max_active_dot          (glbl->global__max_active_dot)

// WARNING:  ALL Globals that need to be copied from PE0 to other PEs must occur ABOVE this cut line.  "bulk" must be the first thing to appear hereafter.

#define sizeof_glbl_to_broadcast (offsetof(Globals_t, global__bulk))
   Bulk_t *                                  global__bulk;    // Storage for larger and command-line-size-determined arrays.
#define       bulk                    (glbl->global__bulk)
   Dbmeta_t                                  global__dbmeta__bulk;
#define       dbmeta__bulk            (glbl->global__dbmeta__bulk)

   int        global__send_cnt;
#define       send_cnt (glbl->global__send_cnt)
   int        global__isend_cnt;
#define       isend_cnt (glbl->global__isend_cnt)
   int        global__recv_cnt;
#define       recv_cnt (glbl->global__recv_cnt)
   int        global__season_cnt;
#define       season_cnt (glbl->global__season_cnt)
   int        global__contin_cnt;
#define       contin_cnt (glbl->global__contin_cnt)
   enum  {ContinuationOpcodeUnspecified, SeasoningOneOrMoreDbCreates, ReceivingACommunication, MassParallelismDone} global__continuationOpcode;
#define       continuationOpcode      (glbl->global__continuationOpcode)
   int                                       global__continuationDetail;
#define       continuationDetail      (glbl->global__continuationDetail)
   double                                    global__tol;
#define       tol                     (glbl->global__tol)
   Comm_t                                    global__comm;           // See comm.h
   Timer_t                                   global__timer;          // See timer.h
   int *                                     global__tmpDb1;
#define       tmpDb1                  (glbl->global__tmpDb1)
   Dbmeta_t                                  global__dbmeta__tmpDb1;
#define       dbmeta__tmpDb1          (glbl->global__dbmeta__tmpDb1)
   int *                                     global__tmpDb2;
#define       tmpDb2                  (glbl->global__tmpDb2)
   Dbmeta_t                                  global__dbmeta__tmpDb2;
#define       dbmeta__tmpDb2          (glbl->global__dbmeta__tmpDb2)
   int *                                     global__tmpDb3;
#define       tmpDb3                  (glbl->global__tmpDb3)
   Dbmeta_t                                  global__dbmeta__tmpDb3;
#define       dbmeta__tmpDb3          (glbl->global__dbmeta__tmpDb3)
   int *                                     global__tmpDb4;
#define       tmpDb4                  (glbl->global__tmpDb4)
   Dbmeta_t                                  global__dbmeta__tmpDb4;
#define       dbmeta__tmpDb4          (glbl->global__dbmeta__tmpDb4)
   int *                                     global__tmpDb5;
#define       tmpDb5                  (glbl->global__tmpDb5)
   Dbmeta_t                                  global__dbmeta__tmpDb5;
#define       dbmeta__tmpDb5          (glbl->global__dbmeta__tmpDb5)
   int *                                     global__tmpDb6;
#define       tmpDb6                  (glbl->global__tmpDb6)
   Dbmeta_t                                  global__dbmeta__tmpDb6;
#define       dbmeta__tmpDb6          (glbl->global__dbmeta__tmpDb6)
   int *                                     global__tmpDb7;
#define       tmpDb7                  (glbl->global__tmpDb7)
   Dbmeta_t                                  global__dbmeta__tmpDb7;
#define       dbmeta__tmpDb7          (glbl->global__dbmeta__tmpDb7)
   char **                                   global__programArgv;
#define       programArgv             (glbl->global__programArgv)
   int                                       global__programArgc;
#define       programArgc             (glbl->global__programArgc)
   int                                       global__callingDepth;
#define       callingDepth            (glbl->global__callingDepth)
#ifdef BUILD_OCR_VERSION
   shared_t *                                global__shared;
#define       shared                  (glbl->global__shared)
#endif
   char *                                    global__tos;                  // Pointer to top of stack.  This is the stack of activation records.
#define       tos                     (glbl->global__tos)
   PtrAdjustmentRecord_t *                   global__topPtrAdjRec;
#define       topPtrAdjRec            (glbl->global__topPtrAdjRec)
   int                                       global__topPtrAdjRecOffset;
#define       topPtrAdjRecOffset      (glbl->global__topPtrAdjRecOffset)
   int                                       global__my_pe;
#define       my_pe                   (glbl->global__my_pe)
   void *                                    global__newResource;
#define       newResource             (glbl->global__newResource)
#ifdef BUILD_OCR_VERSION
   Dbmeta_t                                  global__dbmeta__newResource;
#define       dbmeta__newResource     (glbl->global__dbmeta__newResource)
   ocrGuid_t                                 resource;
#define       resource                (glbl->resource)
   char                                      global__guid_toggle__send[MAX_RANKS];
#define       guid_toggle__send       (glbl->global__guid_toggle__send)
   char                                      global__guid_toggle__recv[MAX_RANKS];
#define       guid_toggle__recv       (glbl->global__guid_toggle__recv)
#endif
   Object_t *                                global__scratch_objects;
#define       scratch_objects         (glbl->global__scratch_objects)
   Dbmeta_t                                  global__dbmeta__scratch_objects;
#define       dbmeta__scratch_objects (glbl->global__dbmeta__scratch_objects)

#define                                       SIZEOFSTACK 8192
   char                                 stack[SIZEOFSTACK];                // Stack space
   Frame_Header_t                       extraStackSpaceSlop;

// WARNING:  Before adding any more variables, see the WARNING above.

} Globals_t;

#define sizeof_Globals_t (sizeof(Globals_t))


#ifdef BUILD_REFERENCE_VERSION
#define SUSPENDABLE_FUNCTION_PROLOGUE(frameName) \
   do { /* Create an opening bracket that is closed by the Eplilogue.  It is of the form do { ... } while (false); so only executes the do-loop once, but the form makes it more clear if there is a failure to match the prologue with the epilogue. */ \
      frameName lclVars; \
      frameName * lcl = &lclVars;
#define CALL_SUSPENDABLE_CALLEE(coverage) \
   do { /* Create an opening bracket that is closed by the Debrief.  It is of the form do { ... } while (false); so only executes the do-loop once, but the form makes it more clear if there is a failure to match the Calling macro with the debriefing macro. */
#define DEBRIEF_SUSPENDABLE_FUNCTION(suspensionReturnValue) \
   } while (0);  /* This is the closing bracket to match up with the opening bracket expanded in the CALL_SUSPENDABLE_CALLEE macro */
#define SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(normalReturnValue) \
   return normalReturnValue;
#define SUSPEND__RESUME_IN_CONTINUATION_EDT(suspensionReturnValue)
#define SUSPENDABLE_FUNCTION_EPILOGUE \
   } while (0);  /* This is the closing bracket to match up with the opening bracket expanded in the SUSPENDABLE_FUNCTION_PROLOGUE macro */

#else

// A suspendable function starts with this macro, which obtains (or re-obtains) the stack frame, generates the switch statement boilerplate, and the case 0 label (initial call)
#define SUSPENDABLE_FUNCTION_PROLOGUE(frameName) \
   char StackDumpSpaces[] = {"                                                            "}; \
   callingDepth++; \
   if (my_pe == -999) printf ("PE=%3d, %s%s in %s line %d:  PROLOGUE:  my caller's frame is at 0x%p -> my frame is at 0x%p\n", my_pe, StackDumpSpaces+sizeof(StackDumpSpaces)-callingDepth*2, __func__, __FILE__, __LINE__, tos, tos + ((Frame_Header_t *) tos)->my_size); \
   tos += ((Frame_Header_t *) tos)->my_size;                /* Here, "my_size" is actually my caller's frame size.  Advance TOS to MY frame header. */ \
   if ((((unsigned long long) tos) + sizeof(frameName)) >= ((unsigned long long) &glbl->stack[SIZEOFSTACK])) { \
      printf ("Stack overflow in Global_t block, detected in SUSPENDABLE_FUNCTION_PROLOGUE.  Increase SIZEOFSTACK and try again.\n"); \
      exit(-1); \
   } \
   frameName * lcl = (frameName *) tos;                     /* Carve out my frame. */ \
   if (my_pe == -999) printf ("PE=%3d, %s%s in %s line %d:  PROLOGUE:  my frame name is %s.  Resumption case is %d which is at 0x%p\n", my_pe, StackDumpSpaces+sizeof(StackDumpSpaces)-callingDepth*2, __func__, __FILE__, __LINE__, STRINGIFY(frameName), lcl->myFrame.resumption_case_num, &lcl->myFrame.resumption_case_num); \
   if (lcl->myFrame.validate_callers_prep_for_suspension == 0) { /* If caller didn't prepare, we cannot suspend correctly. */ \
      printf ("PE=%3d, CANARY TRAP!!!  Caller to %s (in %s) is NOT prepared to propagate suspension back to the root.\n", my_pe, __func__, __FILE__); \
      exit(-1); \
   } \
   lcl->myFrame.validate_callers_prep_for_suspension = 0;   /* Done with this consistency check.  Prep for next time */ \
   if (lcl->myFrame.resumption_case_num == 0) {             /* Initial activation of this function instance (i.e. NOT a resumption). */ \
      lcl->myFrame.my_size = offsetof(frameName, calleeFrame); /* Initialize size of my frame. */ \
      lcl->calleeFrame.resumption_case_num = 0;             /* Initialize for my callee's initial activation. */ \
      lcl->calleeFrame.caller_size = lcl->myFrame.my_size;  /* Initialize for my callee's initial activation. */ \
      lcl->calleeFrame.validate_callers_prep_for_suspension = 0; /* Set "canary trap" for possibility that we are NOT prepared for calling a callee that might suspend activation back to me */ \
      { \
         int pointerCounter; \
         for (pointerCounter = 0; pointerCounter < sizeof(lcl->pointers)/sizeof(void *); pointerCounter ++) { \
            ((void **) (&(lcl->pointers)))[pointerCounter] = NULL; \
         } \
      } \
   } else { \
      if (lcl->myFrame.my_size != offsetof(frameName, calleeFrame)) { \
         printf ("PE=%3d, %s at line %d, My stack frame size is wrong upon resumption of a previous activation.\n", my_pe, __FILE__, __LINE__); \
         exit(-1); \
      } \
      int pointerCounter; \
      for (pointerCounter = sizeof(lcl->pointers)/sizeof(void *); --pointerCounter >= 0; ) { \
         RestorePointerBaseAndOffset(glbl, &(((void **) (&(lcl->pointers)))[pointerCounter]), __FILE__, __func__, __LINE__, pointerCounter); \
      } \
   } \
   __COUNTER__; /* Assure that the counter is NOT zero */ \
   do { /* Create an opening bracket that is closed by the Eplilogue.  It is of the form do { ... } while (false); so only executes the do-loop once, but the form makes it more clear if there is a failure to match the prologue with the epilogue. */ \
   switch (lcl->myFrame.resumption_case_num) { \
   case 0:


// Caller has to precede the call to a suspendable function with this macro.  Otherwise a "canary trap" is triggered in the SUSPENDABLE_FUNCTION_ENTRY_SEQUENCE of
// the callee, to report that the call site probably has not been modified yet to convey a possible suspension of the callee back up the calling chain to the root.
#define CALL_SUSPENDABLE_CALLEE(coverage) \
   do { /* Create an opening bracket that is closed by the Debrief.  It is of the form do { ... } while (false); so only executes the do-loop once, but the form makes it more clear if there is a failure to match the Calling macro with the debriefing macro. */ \
      if (coverage == 0) { \
         static int coverageReported = 0; \
         if (coverageReported == 0) { \
            coverageReported = 1; \
            /*if (my_pe == 1)*/ printf ("Coverage at %s line %04d, PE = %07d\n", __FILE__, __LINE__, my_pe); \
         } \
      } \
         if (my_pe == -999) printf ("PE=%3d, %s%s in %s line %d:  CALLING:   frame is at 0x%p\n", my_pe, StackDumpSpaces+sizeof(StackDumpSpaces)-callingDepth*2, __func__, __FILE__, __LINE__, tos); \
      case __COUNTER__: \
         lcl->calleeFrame.validate_callers_prep_for_suspension = 1;  /* Assure callee that it can suspend activation back to me and I will unwind. */


// Caller has to immediately follow the call to a suspendable function with this macro, to effect unwinding when a suspension operation needs to be perpetuated back to the root.
// If the function is of type void, use a semi-colon for the argument to the macro.
#define DEBRIEF_SUSPENDABLE_FUNCTION(suspensionReturnValue) \
      if (my_pe == -999) printf ("PE=%3d, %s%s in %s line %d:  DEBRIEF:   frame is at 0x%p, resumption_case_num of callee is %d which is at 0x%p\n", my_pe, StackDumpSpaces+sizeof(StackDumpSpaces)-callingDepth*2, __func__, __FILE__, __LINE__, tos, lcl->calleeFrame.resumption_case_num, &lcl->calleeFrame.resumption_case_num); \
      if(lcl->calleeFrame.resumption_case_num != 0) {      /* If the callee suspended, we have to do likewise, so that activations unwind all the way up to the root. Prep for eventual reactivation. */ \
         lcl->myFrame.resumption_case_num = __COUNTER__-1; /* Inform my caller that I am NOT done, and when it resumes me, this is the case number I need to return to. */ \
         tos -= ((Frame_Header_t *) tos)->caller_size;     /* Retreat TOS to my caller's frame header. */  \
         { \
            int pointerCounter; \
            for (pointerCounter = 0; pointerCounter < sizeof(lcl->pointers)/sizeof(void *); pointerCounter ++) { \
               CapturePointerBaseAndOffset(glbl, ((void **) (&(lcl->pointers)))[pointerCounter], __FILE__, __func__, __LINE__, pointerCounter); \
               ((void **) (&(lcl->pointers)))[pointerCounter] = NULL; \
            } \
         } \
         callingDepth--; \
         return suspensionReturnValue; \
      } \
   } while (0);  /* This is the closing bracket to match up with the opening bracket expanded in the CALL_SUSPENDABLE_CALLEE macro */

// A suspendable function must change all normal return instructions to this macro instead.  It pops the stack frame, tells the caller its a normal return, and preps for the next initial activation of
// the caller's next callee.
// If the function is of type void, use a semi-colon for the argument to the macro.
#define SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(normalReturnValue) \
   if (my_pe == -999) printf ("PE=%3d, %s%s in %s line %d:  NRML RTN:  frame is at 0x%p\n", my_pe, StackDumpSpaces+sizeof(StackDumpSpaces)-callingDepth*2, __func__, __FILE__, __LINE__, tos); \
   lcl->myFrame.resumption_case_num = 0;             /* Inform my caller that I am done, and set up for caller's next callee. */ \
   tos -= ((Frame_Header_t *) tos)->caller_size;     /* Retreat TOS to my caller's frame header. */ \
   callingDepth--; \
   return normalReturnValue;

// Trigger suspension of the current EDT, unwinding the stack back to the root EDT function (so that it can "return" and thus terminate the EDT).  The current EDT should have created a continuation EDT,
// and when that EDT's events are satisfied, it will crawl back through the stack to the current point, and carry-on herefrom.
//
// This is like an expansion of CALL_SUSPENDABLE_CALLEE immediately followed by DEBRIEF..., but without a function call.  We are NOT trying to call a SUSPENDABLE function, but rather, we called something
// that requires subsequent code only to proceed after we've received a dependency.  Examples are:
//
// 1) the reference code did a malloc, and then used the new block.  We instead do an ocrDbCreate.  Good form is for the creator of a datablock to NOT write to it, but rather to feed it to a child EDT
// for first access.  (The reason has to do with the future implementation of resiliency.)  So we need to follow the ocrDbCreate with an ocrEdtCreate of an EDT that will receive that datablock.
//
// 2) the reference code did an MPI_recv.  We have to do that by receiving that message block as an EDT.  The call gets replaced by this SUSPEND macro, and when the continuation EDT RESUMEs at this point,
// we need to take the datablock that satisfied our EDT's dependency and stitch it into where the reference version's MPI_recv call received its message.
// If the function is of type void, use a semi-colon for the argument to the macro.
#define SUSPEND__RESUME_IN_CONTINUATION_EDT(suspensionReturnValue) \
   topPtrAdjRec = (PtrAdjustmentRecord_t *) (((unsigned long long) &(lcl->calleeFrame)) + sizeof(Frame_Header_t));;  /* Use this as a cursor for adding Pointer Adjustment Records as we crawl out. */ \
   lcl->myFrame.resumption_case_num = __COUNTER__;   /* Inform my caller that I am NOT done, and when it resumes me, this is the case number I need to return to. */ \
   { \
      void * tmptos = tos; \
      tos -= ((Frame_Header_t *) tos)->caller_size;     /* Retreat TOS to my caller's frame header. */  \
      if (my_pe == -999) printf ("PE=%3d, %s%s in %s line %d:  SUSPEND:   frame is at 0x%p->0x%p, resumption_case_num back to here is %d which is at 0x%p\n", my_pe, StackDumpSpaces+sizeof(StackDumpSpaces)-callingDepth*2, __func__, __FILE__, __LINE__, tmptos, tos, lcl->myFrame.resumption_case_num, &lcl->calleeFrame.resumption_case_num); \
   } \
   { \
      int pointerCounter; \
      for (pointerCounter = 0; pointerCounter < sizeof(lcl->pointers)/sizeof(void *); pointerCounter ++) { \
         CapturePointerBaseAndOffset(glbl, ((void **) (&(lcl->pointers)))[pointerCounter], __FILE__, __func__, __LINE__, pointerCounter); \
         ((void **) (&(lcl->pointers)))[pointerCounter] = NULL; \
      } \
   } \
   callingDepth--; \
   return suspensionReturnValue; \
   case __COUNTER__-1: ;

// A suspendable function has to finish (lexically) with the following boilerplate.
#define SUSPENDABLE_FUNCTION_EPILOGUE \
   default: \
      printf ("ERROR, resumption_case_num = %d is NOT defined for function %s in file %s\n", lcl->myFrame.resumption_case_num, __func__, __FILE__); \
      exit(-1); \
   } /* switch */ \
   } while (0);  /* This is the closing bracket to match up with the opening bracket expanded in the SUSPENDABLE_FUNCTION_PROLOGUE macro */
#endif

#endif
