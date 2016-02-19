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

#include <mpi.h>

#include "block.h"
#include "comm.h"
#include "proto.h"
#include "timer.h"

// This file contains routines that modify the number of blocks so that the
// number is close (+- 3) to the target number of blocks for the problem.
int reduce_blocks(Globals_t * const glbl)
{
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_parents
   typedef struct {
      Frame_Header_t myFrame;
      int l, i, j, p, c, num_comb, comb, numb_parents, nm_t;
      double t1, t2, t3, tp, tm, tu;
      struct {
         Parent_t *pp;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__reduce_blocks_t;

#define l                 (lcl->l)
#define i                 (lcl->i)
#define j                 (lcl->j)
#define p                 (lcl->p)
#define c                 (lcl->c)
#define num_comb          (lcl->num_comb)
#define comb              (lcl->comb)
#define numb_parents      (lcl->numb_parents)
#define nm_t              (lcl->nm_t)
#define t1                (lcl->t1)
#define t2                (lcl->t2)
#define t3                (lcl->t3)
#define tp                (lcl->tp)
#define tm                (lcl->tm)
#define tu                (lcl->tu)
#define pp                (lcl->pointers.pp)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__reduce_blocks_t)
   TRACE

   nm_t = 0;
   tp = tm = tu = t3 = 0.0;
   t1 = timer();

   zero_refine(glbl);
   if (target_active)
      num_comb = (global_active - num_pes*target_active + 3)/7;
   else
      num_comb = (global_active - num_pes*target_active)/7;

   for (comb = 0, l = num_refine-1; comb < num_comb; l--) {
      for (i = 0; i < num_pes; i++)
         bin[i] = 0;
      for (p = 0; p < max_active_parent; p++)
         if ((pp = &parents[p])->number >= 0)
            if (pp->level == l)
               bin[my_pe]++;
      CALL_SUSPENDABLE_CALLEE(0)
      gasket__mpi_Allreduce(glbl, bin, gbin, num_pes, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      for (numb_parents = i = 0; i < num_pes; i++)
         numb_parents += gbin[i];

      if ((num_comb-comb) < numb_parents) {
         while (comb < num_comb)
            for (i = 0; i < num_pes; i++)
               if (gbin[i] > 0) {
                  gbin[i]--;
                  comb++;
                  if (comb == num_comb)
                     break;
               }
         j = bin[my_pe] - gbin[my_pe];
         for (i = p = 0; i < j; p++)
            if ((pp = &parents[p])->number >= 0)
               if (pp->level == l) {
                  pp->refine = -1;
                  i++;
                  for (c = 0; c < 8; c++)
                     if (pp->child_node[c] == my_pe && pp->child[c] >= 0)
                        blocks[pp->child[c]].refine = -1;
               }
      } else {
         comb += numb_parents;
         for (p = 0; p < max_active_parent; p++)
            if ((pp = &parents[p])->number >= 0)
               if (pp->level == l) {
                  pp->refine = -1;
                  for (c = 0; c < 8; c++)
                     if (pp->child_node[c] == my_pe && pp->child[c] >= 0)
                        blocks[pp->child[c]].refine = -1;
               }
      }

      CALL_SUSPENDABLE_CALLEE(0)
      comm_parent_unrefine(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
      CALL_SUSPENDABLE_CALLEE(0)
      comm_refine_unrefine(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
      CALL_SUSPENDABLE_CALLEE(0)
      redistribute_blocks(glbl, &tp, &tm, &tu, &t2, &nm_t, 0);
      DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
      t2 = timer() - t2;
      CALL_SUSPENDABLE_CALLEE(0)
      consolidate_blocks(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
      t3 += timer() - t2;
   }
   timer_target_rb += timer() - t1;
   timer_target_dc += timer() - t1 - t3 - tp - tm - tu;
   timer_target_cb += t3;
   timer_target_pa += tp;
   timer_target_mv += tm;
   timer_target_un += tu;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(nm_t)
   SUSPENDABLE_FUNCTION_EPILOGUE

#undef  l
#undef  i
#undef  j
#undef  p
#undef  c
#undef  num_comb
#undef  comb
#undef  num_parents
#undef  nm_t
#undef  t1
#undef  t2
#undef  t3
#undef  tp
#undef  tm
#undef  tu
#undef  pp
}

void add_blocks(Globals_t * const glbl)
{
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_sorted_list

   typedef struct {
      Frame_Header_t myFrame;
      int l, i, j, n, in, num_split, split;
      double t1, t2, t3;
      struct {
         Block_t *bp;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__add_blocks_t;

#define l                 (lcl->l)
#define i                 (lcl->i)
#define j                 (lcl->j)
// #define n                 (lcl->n)  Name overload ("n" appears as a member of a struct.  Just use the fully qualified name lcl->n instead.)
#define in                (lcl->in)
#define num_split         (lcl->num_split)
#define split             (lcl->split)
#define t1                (lcl->t1)
#define t2                (lcl->t2)
#define t3                (lcl->t3)
#define bp                (lcl->pointers.bp)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__add_blocks_t)
   TRACE

   t3 = 0.0;
   t1 = timer();

   if (target_active)
      num_split = (num_pes*target_active + 3 - global_active)/7;
   else
      num_split = (num_pes*target_active - global_active)/7;

   for (split = l = 0; split < num_split; l++) {
      zero_refine(glbl);
      for (j = num_refine; j >= 0; j--)
         if (num_blocks[j]) {
            cur_max_level = j;
            break;
      }
      if ((num_split-split) < num_blocks[l]) {
         for (i = 0; i < num_pes; i++)
            bin[i] = 0;
         bin[my_pe] = local_num_blocks[l];
         CALL_SUSPENDABLE_CALLEE(0)
         gasket__mpi_Allreduce(glbl, bin, gbin, num_pes, MPI_INTEGER, MPI_SUM,
                       MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)

         while (split < num_split)
            for (i = 0; i < num_pes; i++)
               if (gbin[i] > 0) {
                  gbin[i]--;
                  split++;
                  if (split == num_split)
                     break;
               }
         j = bin[my_pe] - gbin[my_pe];
         for (i = in = 0; i < j && in < sorted_index[num_refine+1]; in++) {
            lcl->n = sorted_list[in].n;
            if ((bp = &blocks[lcl->n])->number >= 0)
               if (bp->level == l) {
                  bp->refine = 1;
                  i++;
               }
         }
      } else {  // Mark all blocks in level l to be refined.
         split += num_blocks[l];
         for (in = 0; in < sorted_index[num_refine+1]; in++) {
            lcl->n = sorted_list[in].n;
            if ((bp = &blocks[lcl->n])->number >= 0)
               if (bp->level == l)
                  bp->refine = 1;
         }
      }

      CALL_SUSPENDABLE_CALLEE(0)
      comm_refine_unrefine(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      t2 = timer();
      CALL_SUSPENDABLE_CALLEE(0)
      split_blocks(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      t3 += timer() - t2;
      CALL_SUSPENDABLE_CALLEE(0)
      gasket__mpi_Allreduce(glbl, local_num_blocks, num_blocks, (num_refine+1), MPI_INTEGER,
                    MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
   }
   timer_target_ab += timer() - t1;
   timer_target_da += timer() - t1 - t3;
   timer_target_sb += t3;
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  l
#undef  i
#undef  j
#undef  n
#undef  in
#undef  num_split
#undef  split
#undef  t1
#undef  t2
#undef  t3
#undef  bp
}

void zero_refine(Globals_t * const glbl)
{
   int n, c, in;
   Block_t *bp;
   Parent_t *pp;
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_parents
   OBTAIN_ACCESS_TO_sorted_list
   TRACE

   for (in = 0; in < sorted_index[num_refine+1]; in++) {
      n = sorted_list[in].n;
      if ((bp= &blocks[n])->number >= 0) {
         bp->refine = 0;
         for (c = 0; c < 6; c++)
            if (bp->nei_level[c] >= 0)
               bp->nei_refine[c] = 0;
      }
   }

   for (n = 0; n < max_active_parent; n++)
      if ((pp = &parents[n])->number >= 0)
         pp->refine = 0;
}
