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
#include <mpi.h>

#include "block.h"
#include "comm.h"
#include "proto.h"

// These routines are concerned with communicating information between
// parents and their children.  This includes information about refinement
// level and also includes a routine to keep track to where children are
// being moved to.  For on node children, the parent has the index of the
// child block and for off node children has the block number.
void comm_parent(Globals_t * const glbl)
{
   OBTAIN_ACCESS_TO_parents
   OBTAIN_ACCESS_TO_blocks

   typedef struct {
      Frame_Header_t myFrame;
      int i, j, b, which, type, offset;
      MPI_Status status;
      struct {
         int *send_int;
         int *recv_int;
         Parent_t *pp;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__comm_parent_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define b                 (lcl->b)
#define which             (lcl->which)
#define type              (lcl->type)
#define offset            (lcl->offset)
#define send_int          (lcl->pointers.send_int)
#define recv_int          (lcl->pointers.recv_int)
#define pp                (lcl->pointers.pp)
#define status            (lcl->status)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__comm_parent_t)
   TRACE

   send_int = (int *) send_buff;
   recv_int = (int *) recv_buff;

   type = 20;
   for (i = 0; i < par_p.num_comm_part; i++)
      gasket__mpi_Irecv(glbl, &recv_int[par_p.index[i]], par_p.par_comm_num[i], MPI_INTEGER,
                 par_p.comm_part[i], type, MPI_COMM_WORLD, -9999, &request[i], __FILE__, __LINE__);

   for (i = 0; i < par_b.num_comm_part; i++) {
      if (nonblocking)
         offset = par_b.index[i];
      else
         offset = 0;
      for (j = 0; j < par_b.par_comm_num[i]; j++)
         if (par_b.comm_b[par_b.index[i]+j] < 0)
            // parent, so send 0 (its parent can not refine)
            send_int[offset+j] = 0;
         else
            send_int[offset+j] = blocks[par_b.comm_b[par_b.index[i]+j]].refine;
      if (nonblocking) {
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Isend(glbl, &send_int[par_b.index[i]], par_b.par_comm_num[i], MPI_INTEGER,
                   par_b.comm_part[i], type, MPI_COMM_WORLD, -9999, &s_req[i], __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      } else {
         CALL_SUSPENDABLE_CALLEE(0)
         gasket__mpi_Send(glbl, &send_int[0], par_b.par_comm_num[i], MPI_INTEGER,
                  par_b.comm_part[i], type, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      }
   }

   for (i = 0; i < par_p.num_comm_part; i++) {
#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this:
      MPI_Waitany(par_p.num_comm_part, request, &which, &status);
#else   // OCR doesn't (presently) support the concept of an EDT triggering when ANY of a list of dependences is satisfied.  Rework
        // to wait specifically for each IRecv.  Also provide stuff OCR version needs to be able to receive the result buffer.
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Wait__for_Irecv(glbl, &recv_int[par_p.index[i]], par_p.par_comm_num[i], MPI_INTEGER, par_p.comm_part[i], MPI_COMM_WORLD, -9999, &request[i], &status, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      which = i;
#endif
      for (j = 0; j < par_p.par_comm_num[which]; j++)
         if (recv_int[par_p.index[which]+j] > -1) {
            pp = &parents[par_p.comm_p[par_p.index[which]+j]];
            pp->refine = 0;
            for (b = 0; b < 8; b++)
               if (pp->child_node[b] == my_pe && pp->child[b] >= 0 &&
                   blocks[pp->child[b]].refine == -1)
                  blocks[pp->child[b]].refine = 0;
         }
   }

   if (nonblocking)
      for (i = 0; i < par_b.num_comm_part; i++) {
#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this:
         MPI_Waitany(par_b.num_comm_part, s_req, &which, &status);
#else   // OCR doesn't (presently) support the concept of an EDT triggering when ANY of a list of dependences is satisfied.  Rework
        // to wait specifically for each ISend.
         gasket__mpi_Wait__for_Isend(glbl, &s_req[i], &status, __FILE__, __LINE__);
#endif
      }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  b
#undef  which
#undef  type
#undef  offset
#undef  send_int
#undef  recv_int
#undef  pp
#undef  status
}

void comm_parent_reverse(Globals_t * const glbl)
{
   OBTAIN_ACCESS_TO_parents
   OBTAIN_ACCESS_TO_blocks

   typedef struct {
      Frame_Header_t myFrame;
      int i, j, which, type, offset;
      MPI_Status status;
      struct {
         int *send_int;
         int *recv_int;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__comm_parent_reverse_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define which             (lcl->which)
#define type              (lcl->type)
#define offset            (lcl->offset)
#define send_int          (lcl->pointers.send_int)
#define recv_int          (lcl->pointers.recv_int)
#define status            (lcl->status)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__comm_parent_reverse_t)
   TRACE

   send_int = (int *) send_buff;
   recv_int = (int *) recv_buff;

   type = 21;
   for (i = 0; i < par_b.num_comm_part; i++) {
      gasket__mpi_Irecv(glbl, &recv_int[par_b.index[i]], par_b.par_comm_num[i], MPI_INTEGER,
                 par_b.comm_part[i], type, MPI_COMM_WORLD, -9999, &request[i], __FILE__, __LINE__);
   }

   for (i = 0; i < par_p.num_comm_part; i++) {
      if (nonblocking)
         offset = par_p.index[i];
      else
         offset = 0;
      for (j = 0; j < par_p.par_comm_num[i]; j++)
         send_int[offset+j] = parents[par_p.comm_p[par_p.index[i]+j]].refine;
      if (nonblocking) {
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Isend(glbl, &send_int[par_p.index[i]], par_p.par_comm_num[i], MPI_INTEGER,
                   par_p.comm_part[i], type, MPI_COMM_WORLD, -9999, &s_req[i], __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      } else {
         CALL_SUSPENDABLE_CALLEE(0)
         gasket__mpi_Send(glbl, &send_int[0], par_p.par_comm_num[i], MPI_INTEGER,
                  par_p.comm_part[i], type, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      }
   }

   for (i = 0; i < par_b.num_comm_part; i++) {
#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this:
      MPI_Waitany(par_b.num_comm_part, request, &which, &status);
#else   // OCR doesn't (presently) support the concept of an EDT triggering when ANY of a list of dependences is satisfied.  Rework
        // to wait specifically for each IRecv.  Also provide stuff OCR version needs to be able to receive the result buffer.
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Wait__for_Irecv(glbl, &recv_int[par_b.index[i]], par_b.par_comm_num[i], MPI_INTEGER, par_b.comm_part[i], MPI_COMM_WORLD, -9999, &request[i], &status, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      which = i;
#endif
      for (j = 0; j < par_b.par_comm_num[which]; j++)
         if (recv_int[par_b.index[which]+j] > -1 &&
             par_b.comm_b[par_b.index[which]+j] >= 0)
            if (blocks[par_b.comm_b[par_b.index[which]+j]].refine == -1)
               blocks[par_b.comm_b[par_b.index[which]+j]].refine = 0;
   }

   if (nonblocking)
      for (i = 0; i < par_p.num_comm_part; i++) {
#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this:
         MPI_Waitany(par_p.num_comm_part, s_req, &which, &status);
#else   // OCR doesn't (presently) support the concept of an EDT triggering when ANY of a list of dependences is satisfied.  Rework
        // to wait specifically for each ISend.
         gasket__mpi_Wait__for_Isend(glbl, &s_req[i], &status, __FILE__, __LINE__);
#endif
      }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  which
#undef  type
#undef  offset
#undef  send_int
#undef  recv_int
#undef  status
}

void comm_parent_unrefine(Globals_t * const glbl)
{
   OBTAIN_ACCESS_TO_parents
   OBTAIN_ACCESS_TO_blocks
   TRACE

   typedef struct {
      Frame_Header_t myFrame;
      int i, j, which, type, offset;
      MPI_Status status;
      struct {
         int *send_int;
         int *recv_int;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__parent_unrefine_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define which             (lcl->which)
#define type              (lcl->type)
#define offset            (lcl->offset)
#define send_int          (lcl->pointers.send_int)
#define recv_int          (lcl->pointers.recv_int)
#define status            (lcl->status)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__parent_unrefine_t)

   send_int = (int *) send_buff;
   recv_int = (int *) recv_buff;

   type = 22;
   for (i = 0; i < par_b.num_comm_part; i++)
      gasket__mpi_Irecv(glbl, &recv_int[par_b.index[i]], par_b.par_comm_num[i], MPI_INTEGER,
                 par_b.comm_part[i], type, MPI_COMM_WORLD, -9999, &request[i], __FILE__, __LINE__);

   for (i = 0; i < par_p.num_comm_part; i++) {
      if (nonblocking)
         offset = par_p.index[i];
      else
         offset = 0;
      for (j = 0; j < par_p.par_comm_num[i]; j++)
         send_int[offset+j] = parents[par_p.comm_p[par_p.index[i]+j]].refine;
      if (nonblocking) {
         CALL_SUSPENDABLE_CALLEE(0)
         gasket__mpi_Isend(glbl, &send_int[par_p.index[i]], par_p.par_comm_num[i], MPI_INTEGER,
                   par_p.comm_part[i], type, MPI_COMM_WORLD, -9999, &s_req[i], __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      } else {
         CALL_SUSPENDABLE_CALLEE(0)
         gasket__mpi_Send(glbl, &send_int[0], par_p.par_comm_num[i], MPI_INTEGER,
                  par_p.comm_part[i], type, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      }
   }

   for (i = 0; i < par_b.num_comm_part; i++) {
#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this:
      MPI_Waitany(par_b.num_comm_part, request, &which, &status);
#else   // OCR doesn't (presently) support the concept of an EDT triggering when ANY of a list of dependences is satisfied.  Rework
        // to wait specifically for each IRecv.  Also provide stuff OCR version needs to be able to receive the result buffer.
      CALL_SUSPENDABLE_CALLEE(0)
      gasket__mpi_Wait__for_Irecv(glbl, &recv_int[par_b.index[i]], par_b.par_comm_num[i], MPI_INTEGER, par_b.comm_part[i], MPI_COMM_WORLD, -9999, &request[i], &status, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      which = i;
#endif
      for (j = 0; j < par_b.par_comm_num[which]; j++)
         if (par_b.comm_b[par_b.index[which]+j] >= 0)
            blocks[par_b.comm_b[par_b.index[which]+j]].refine =
                  recv_int[par_b.index[which]+j];
   }

   if (nonblocking)
      for (i = 0; i < par_p.num_comm_part; i++) {
#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this:
         MPI_Waitany(par_b.num_comm_part, s_req, &which, &status);
#else   // OCR doesn't (presently) support the concept of an EDT triggering when ANY of a list of dependences is satisfied.  Rework
        // to wait specifically for each ISend.
         gasket__mpi_Wait__for_Isend(glbl, &s_req[i], &status, __FILE__, __LINE__);
#endif
      }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  which
#undef  type
#undef  offset
#undef  send_int
#undef  recv_int
#undef  status
}

// Communicate new proc to parents - coordinate properly
// As new proc numbers come in, del the current and add the new
void comm_parent_proc(Globals_t * const glbl)
{
   OBTAIN_ACCESS_TO_parents
   OBTAIN_ACCESS_TO_blocks

   typedef struct {
      Frame_Header_t myFrame;
      int i, j, which, type, offset, season;
      MPI_Status status;
      struct {
         int *send_int;
         int *recv_int;
         Parent_t *pp;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__comm_parent_proc_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define which             (lcl->which)
#define type              (lcl->type)
#define offset            (lcl->offset)
#define season            (lcl->season)
#define send_int          (lcl->pointers.send_int)
#define recv_int          (lcl->pointers.recv_int)
#define pp                (lcl->pointers.pp)
#define status            (lcl->status)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__comm_parent_proc_t)
   TRACE

   send_int = (int *) send_buff;
   recv_int = (int *) recv_buff;

   season = 0;
   // duplicate par_p to par_p1
   if (par_p.num_comm_part > par_p1.max_part) {
      season = 1;
      gasket__free(&par_p1.dbmeta__comm_part,    (void **) &par_p1.comm_part, __FILE__, __LINE__);
      gasket__free(&par_p1.dbmeta__par_comm_num, (void **) &par_p1.par_comm_num, __FILE__, __LINE__);
      gasket__free(&par_p1.dbmeta__index,        (void **) &par_p1.index, __FILE__, __LINE__);
      par_p1.max_part = par_p.max_part;
      gasket__ma_malloc(glbl, &par_p1.dbmeta__comm_part, (void *) &par_p1.comm_part, par_p.max_part*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &par_p1.dbmeta__par_comm_num, (void *) &par_p1.par_comm_num, par_p.max_part*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &par_p1.dbmeta__index, (void *) &par_p1.index, par_p.max_part*sizeof(int), __FILE__, __LINE__);
   }
   if (par_p.par_num_cases > par_p1.max_cases) {
      season = 1;
      gasket__free(&par_p1.dbmeta__comm_b, (void **) &par_p1.comm_b, __FILE__, __LINE__);
      gasket__free(&par_p1.dbmeta__comm_p, (void **) &par_p1.comm_p, __FILE__, __LINE__);
      gasket__free(&par_p1.dbmeta__comm_c, (void **) &par_p1.comm_c, __FILE__, __LINE__);
      par_p1.max_cases = par_p.max_cases;
      gasket__ma_malloc(glbl, &par_p1.dbmeta__comm_b, (void *) &par_p1.comm_b, par_p.max_cases*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &par_p1.dbmeta__comm_p, (void *) &par_p1.comm_p, par_p.max_cases*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &par_p1.dbmeta__comm_c, (void *) &par_p1.comm_c, par_p.max_cases*sizeof(int), __FILE__, __LINE__);
   }
   if (season) {
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.
   }

   par_p1.num_comm_part = par_p.num_comm_part;
   for (i = 0; i < par_p.num_comm_part; i++) {
      par_p1.comm_part[i] = par_p.comm_part[i];
      par_p1.par_comm_num[i] = par_p.par_comm_num[i];
      par_p1.index[i] = par_p.index[i];
   }
   par_p1.par_num_cases = par_p.par_num_cases;
   for (i = 0; i < par_p.par_num_cases; i++) {
      par_p1.comm_b[i] = par_p.comm_b[i];
      par_p1.comm_p[i] = par_p.comm_p[i];
      par_p1.comm_c[i] = par_p.comm_c[i];
   }

   type = 23;
   for (i = 0; i < par_p.num_comm_part; i++)
      gasket__mpi_Irecv(glbl, &recv_int[par_p.index[i]], par_p.par_comm_num[i], MPI_INTEGER,
                 par_p.comm_part[i], type, MPI_COMM_WORLD, -9999, &request[i], __FILE__, __LINE__);

   for (i = 0; i < par_b.num_comm_part; i++) {
      if (nonblocking)
         offset = par_b.index[i];
      else
         offset = 0;
      for (j = 0; j < par_b.par_comm_num[i]; j++)
         if (par_b.comm_b[par_b.index[i]+j] < 0)
            // parent and will not move, so send current processor
            send_int[offset+j] = my_pe;
         else
            send_int[offset+j] =
                               blocks[par_b.comm_b[par_b.index[i]+j]].new_proc;
      if (nonblocking) {
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Isend(glbl, &send_int[par_b.index[i]], par_b.par_comm_num[i], MPI_INTEGER,
                   par_b.comm_part[i], type, MPI_COMM_WORLD, -9999, &s_req[i], __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      } else {
         CALL_SUSPENDABLE_CALLEE(0)
         gasket__mpi_Send(glbl, &send_int[0], par_b.par_comm_num[i], MPI_INTEGER,
                  par_b.comm_part[i], type, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      }
   }

   for (i = 0; i < par_p1.num_comm_part; i++) {
#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this:
      MPI_Waitany(par_p1.num_comm_part, request, &which, &status);
#else   // OCR doesn't (presently) support the concept of an EDT triggering when ANY of a list of dependences is satisfied.  Rework
        // to wait specifically for each IRecv.  Also provide stuff OCR version needs to be able to receive the result buffer.
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Wait__for_Irecv(glbl, &recv_int[par_p1.index[i]], par_p1.par_comm_num[i], MPI_INTEGER, par_p1.comm_part[i], MPI_COMM_WORLD, -9999, &request[i], &status, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      which = i;
#endif
      for (j = 0; j < par_p1.par_comm_num[which]; j++)
         if (recv_int[par_p1.index[which]+j] > -1) {
            pp = &parents[par_p1.comm_p[par_p1.index[which]+j]];
            if (pp->child_node[par_p1.comm_c[par_p1.index[which]+j]] !=
                  recv_int[par_p1.index[which]+j]) {
               del_par_list(glbl, &par_p, par_p1.comm_p[par_p1.index[which]+j],
                            par_p1.comm_b[par_p1.index[which]+j],
                            par_p1.comm_c[par_p1.index[which]+j],
                            par_p1.comm_part[which]);
               if (recv_int[par_p1.index[which]+j] != my_pe) {
                  CALL_SUSPENDABLE_CALLEE(1)
                  add_par_list(glbl, &par_p, par_p1.comm_p[par_p1.index[which]+j],
                               par_p1.comm_b[par_p1.index[which]+j],
                               par_p1.comm_c[par_p1.index[which]+j],
                               recv_int[par_p1.index[which]+j], 1);
                  DEBRIEF_SUSPENDABLE_FUNCTION(;)
                  pp->child_node[par_p1.comm_c[par_p1.index[which]+j]] =
                        recv_int[par_p1.index[which]+j];
               } else
                  pp->child_node[par_p1.comm_c[par_p1.index[which]+j]] = my_pe;
            }
         }
   }

   if (nonblocking)
      for (i = 0; i < par_b.num_comm_part; i++) {
#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this:
         MPI_Waitany(par_b.num_comm_part, s_req, &which, &status);
#else   // OCR doesn't (presently) support the concept of an EDT triggering when ANY of a list of dependences is satisfied.  Rework
        // to wait specifically for each ISend.
         gasket__mpi_Wait__for_Isend(glbl, &s_req[i], &status, __FILE__, __LINE__);
#endif
      }

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  which
#undef  type
#undef  offset
#undef  send_int
#undef  recv_int
#undef  pp
#undef  status
}

// Below are routines for adding and deleting from arrays used above

void add_par_list(Globals_t * const glbl, Par_Comm_t * const pc, int const nparent, int const block, int const child, int const pe, int const sort)
{
   OBTAIN_ACCESS_TO_parents
   TRACE

   typedef struct {
      Frame_Header_t myFrame;
      int i, j;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__add_par_list_t;

#define i                 (lcl->i)
#define j                 (lcl->j)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__add_par_list_t)

   // first add information into comm_part, par_comm_num, and index
   // i is being used as an index to where the info goes in the arrays
   for (i = 0; i < pc->num_comm_part; i++)
      if (pc->comm_part[i] >= pe)
         break;

   if (i < pc->num_comm_part && pc->comm_part[i] == pe) {
      for (j = pc->num_comm_part-1; j > i; j--)
         pc->index[j]++;
      pc->par_comm_num[i]++;
   } else {
      // adding new pe, make sure arrays are large enough
      if (pc->num_comm_part == pc->max_part) {
         pc->max_part = (int)(2.0*((double) (pc->num_comm_part + 1)));

         gasket__ma_malloc(glbl, &dbmeta__tmpDb1, (void *) &tmpDb1, pc->max_part*sizeof(int), __FILE__, __LINE__);
         gasket__ma_malloc(glbl, &dbmeta__tmpDb2, (void *) &tmpDb2, pc->max_part*sizeof(int), __FILE__, __LINE__);
         gasket__ma_malloc(glbl, &dbmeta__tmpDb3, (void *) &tmpDb3, pc->max_part*sizeof(int), __FILE__, __LINE__);
         SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.

         for (j = 0; j < i; j++)
            tmpDb1[j] = pc->comm_part[j];
         for (j = i; j < pc->num_comm_part; j++)
            tmpDb1[j+1] = pc->comm_part[j];
         for (j = 0; j < i; j++)
            tmpDb2[j] = pc->par_comm_num[j];
         for (j = i; j < pc->num_comm_part; j++)
            tmpDb2[j+1] = pc->par_comm_num[j];
         for (j = 0; j <= i; j++)
            tmpDb3[j] = pc->index[j];
         for (j = i; j < pc->num_comm_part; j++)
            tmpDb3[j+1] = pc->index[j] + 1;

         gasket__free(&pc->dbmeta__comm_part, (void **) &pc->comm_part, __FILE__, __LINE__);
         gasket__free(&pc->dbmeta__par_comm_num, (void **) &pc->par_comm_num, __FILE__, __LINE__);
         gasket__free(&pc->dbmeta__index, (void **) &pc->index, __FILE__, __LINE__);

         pc->comm_part = tmpDb1;
         pc->dbmeta__comm_part = dbmeta__tmpDb1;
         tmpDb1 = NULL;
         dbmeta__tmpDb1.guid = NULL_GUID;
         dbmeta__tmpDb1.size = 0;

         pc->par_comm_num = tmpDb2;
         pc->dbmeta__par_comm_num = dbmeta__tmpDb2;
         tmpDb2 = NULL;
         dbmeta__tmpDb2.guid = NULL_GUID;
         dbmeta__tmpDb2.size = 0;

         pc->index = tmpDb3;
         pc->dbmeta__index = dbmeta__tmpDb3;
         tmpDb3 = NULL;
         dbmeta__tmpDb3.guid = NULL_GUID;
         dbmeta__tmpDb3.size = 0;
      } else {
         for (j = pc->num_comm_part; j > i; j--) {
            pc->comm_part[j] = pc->comm_part[j-1];
            pc->par_comm_num[j] = pc->par_comm_num[j-1];
            pc->index[j] = pc->index[j-1] + 1;
         }
      }
      if (i == pc->num_comm_part)
         pc->index[i] = pc->par_num_cases;
      pc->num_comm_part++;
      pc->comm_part[i] = pe;
      pc->par_comm_num[i] = 1;
   }

   // now add into to comm_p and comm_b according to index
   // first check if there is room in the arrays
   if (pc->par_num_cases == pc->max_cases) {
      pc->max_cases = (int)(2.0*((double) (pc->par_num_cases+1)));

      gasket__ma_malloc(glbl, &dbmeta__tmpDb1, (void *) &tmpDb1, pc->max_cases*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__tmpDb2, (void *) &tmpDb2, pc->max_cases*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__tmpDb3, (void *) &tmpDb3, pc->max_cases*sizeof(int), __FILE__, __LINE__);
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.

      for (j = 0; j < pc->par_num_cases; j++)
         tmpDb1[j] = pc->comm_p[j];
      for (j = 0; j < pc->par_num_cases; j++)
         tmpDb2[j] = pc->comm_b[j];
      for (j = 0; j < pc->par_num_cases; j++)
         tmpDb3[j] = pc->comm_c[j];

      gasket__free(&pc->dbmeta__comm_p, (void **) &pc->comm_p, __FILE__, __LINE__);
      gasket__free(&pc->dbmeta__comm_b, (void **) &pc->comm_b, __FILE__, __LINE__);
      gasket__free(&pc->dbmeta__comm_c, (void **) &pc->comm_c, __FILE__, __LINE__);

      pc->comm_p = tmpDb1;
      pc->dbmeta__comm_p = dbmeta__tmpDb1;
      tmpDb1 = NULL;
      dbmeta__tmpDb1.guid = NULL_GUID;
      dbmeta__tmpDb1.size = 0;

      pc->comm_b = tmpDb2;
      pc->dbmeta__comm_b = dbmeta__tmpDb2;
      tmpDb2 = NULL;
      dbmeta__tmpDb2.guid = NULL_GUID;
      dbmeta__tmpDb2.size = 0;

      pc->comm_c = tmpDb3;
      pc->dbmeta__comm_c = dbmeta__tmpDb3;
      tmpDb3 = NULL;
      dbmeta__tmpDb3.guid = NULL_GUID;
      dbmeta__tmpDb3.size = 0;
   }
   if (pc->index[i] == pc->par_num_cases) {
      // at end of arrays
      pc->comm_p[pc->par_num_cases] = nparent;
      pc->comm_b[pc->par_num_cases] = block;
      pc->comm_c[pc->par_num_cases] = child;
   } else {
      for (j = pc->par_num_cases; j >= pc->index[i]+pc->par_comm_num[i]; j--) {
         pc->comm_p[j] = pc->comm_p[j-1];
         pc->comm_b[j] = pc->comm_b[j-1];
         pc->comm_c[j] = pc->comm_c[j-1];
      }
      for (j = pc->index[i]+pc->par_comm_num[i]-1; j >= pc->index[i]; j--) {
         if (j == pc->index[i] ||
             (sort && (parents[pc->comm_p[j-1]].number < parents[nparent].number
                || (pc->comm_p[j-1] == nparent && pc->comm_c[j-1] < child))) ||
             (!sort && (pc->comm_p[j-1] < nparent
                || (pc->comm_p[j-1] == nparent && pc->comm_c[j-1] < child)))) {
            pc->comm_p[j] = nparent;
            pc->comm_b[j] = block;
            pc->comm_c[j] = child;
            break;
         } else {
            pc->comm_p[j] = pc->comm_p[j-1];
            pc->comm_b[j] = pc->comm_b[j-1];
            pc->comm_c[j] = pc->comm_c[j-1];
         }
      }
   }
   pc->par_num_cases++;
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
}

void del_par_list(Globals_t * const glbl, Par_Comm_t * const pc, int const nparent, int const block, int const child, int const pe)
{
   int i, j, k;
   TRACE

   // find core number in index list and use i below
   for (i = 0; i < pc->num_comm_part; i++)
      if (pc->comm_part[i] == pe)
         break;

   // find and delete case in comm_p, comm_b, and comm_c
   pc->par_num_cases--;
   for (j = pc->index[i]; j < pc->index[i]+pc->par_comm_num[i]; j++)
      if (pc->comm_p[j] == nparent && pc->comm_c[j] == child) {
         for (k = j; k < pc->par_num_cases; k++) {
            pc->comm_p[k] = pc->comm_p[k+1];
            pc->comm_b[k] = pc->comm_b[k+1];
            pc->comm_c[k] = pc->comm_c[k+1];
         }
         break;
      }
   // fix index and adjust comm_part and par_comm_num
   pc->par_comm_num[i]--;
   if (pc->par_comm_num[i])
      for (j = i+1; j < pc->num_comm_part; j++)
         pc->index[j]--;
   else {
      pc->num_comm_part--;
      for (j = i; j < pc->num_comm_part; j++) {
         pc->comm_part[j] = pc->comm_part[j+1];
         pc->par_comm_num[j] = pc->par_comm_num[j+1];
         pc->index[j] = pc->index[j+1] - 1;
      }
   }
}
