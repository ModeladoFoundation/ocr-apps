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

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "block.h"
#include "comm.h"
#include "timer.h"
#include "proto.h"

// This file includes routines needed for load balancing.  Load balancing is
// based on RCB.  At each stage, a direction and factor is chosen (factor is
// based on the prime factorization of the number of processors) and the
// blocks in that group are sorted in that direction and divided into factor
// subgroups.  Then dots (corresponding to blocks) are moved into the proper
// subgroup and the process is repeated with the subgroups until each group
// represents a processor.  The dots are then moved back to the originating
// processor, at which point we know where the blocks need to be moved and
// then the blocks are moved.  Some of these routines are also used when
// blocks need to be coarsened - the coarsening routine determines which
// blocks need to be coarsened and those blocks are moved to the processor
// where their parent is.
void load_balance(Globals_t * const glbl)
{
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_to
   OBTAIN_ACCESS_TO_from
   OBTAIN_ACCESS_TO_dots
   OBTAIN_ACCESS_TO_sorted_list

   typedef struct {
      Frame_Header_t myFrame;
      int npx1, npy1, npz1, nfac, fac[25], fact;
      int i, j, m, n, dir, in;
      double t1, t2, t3, t4, t5, tp, tm, tu;
      struct {
         Block_t *bp;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__load_balance_t;

#define npx1              (lcl->npx1)
#define npy1              (lcl->npy1)
#define npz1              (lcl->npz1)
#define nfac              (lcl->nfac)
#define fac               (lcl->fac)
#define fact              (lcl->fact)
#define i                 (lcl->i)
#define j                 (lcl->j)
#define m                 (lcl->m)
// #define n                 (lcl->n)  // Overloaded name ("n" appears as a member of a struct elsewhere.  Just use the fully qualified name, lcl->n).
#define dir               (lcl->dir)
#define in                (lcl->in)
#define t1                (lcl->t1)
#define t2                (lcl->t2)
#define t3                (lcl->t3)
#define t4                (lcl->t4)
#define t5                (lcl->t5)
#define tp                (lcl->tp)
#define tm                (lcl->tm)
#define tu                (lcl->tu)
#define bp                (lcl->pointers.bp)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__load_balance_t)
   //TRACErcb
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, At entry\n", my_pe, __func__, __FILE__); fflush(stdout); }

   tp = tm = tu = 0.0;

   t3 = t4 = t5 = 0.0;
   t1 = timer();
   for (in = 0, num_dots = 0; in < sorted_index[num_refine+1]; in++) {
      lcl->n = sorted_list[in].n;
      if ((bp = &blocks[lcl->n])->number >= 0) {
         bp->new_proc = my_pe;
         if ((num_dots+1) > max_num_dots) {
            printf("%d ERROR: need more dots\n", my_pe);
            exit(-1);
         }
         dots[num_dots].cen[0] = bp->cen[0];
         dots[num_dots].cen[1] = bp->cen[1];
         dots[num_dots].cen[2] = bp->cen[2];
         dots[num_dots].number = bp->number;
         dots[num_dots].n = lcl->n;
         dots[num_dots].proc = my_pe;
         dots[num_dots++].new_proc = 0;
      }
   }
//???{ printf("pe = %3d, %30s %23s, At point 1, num_dots = %d\n", my_pe, __func__, __FILE__, num_dots); fflush(stdout); }
   max_active_dot = num_dots;
   for (lcl->n = num_dots; lcl->n < max_num_dots; lcl->n++)
      dots[lcl->n].number = -1;

   npx1 = npx;
   npy1 = npy;
   npz1 = npz;
   nfac = factor(glbl, fac);
   for (i = nfac, j = 0; i > 0; i--, j++) {
      fact = fac[i-1];
      dir = find_dir(glbl, fact, npx1, npy1, npz1);
      if (dir == 0)
         npx1 /= fact;
      else if (dir == 1)
         npy1 /= fact;
      else
         npz1 /= fact;
      CALL_SUSPENDABLE_CALLEE(1)
      sort(glbl, j, fact, dir);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      move_dots(glbl, j, fact);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
   }
   // first have to move information from dots back to original core,
   // then will update processor block is moving to, and then its neighbors
   for (lcl->n = 0; lcl->n < num_pes; lcl->n++)
      to[lcl->n] = 0;
   for (m = i = 0; i < max_active_dot; i++)
      if (dots[i].number >= 0 && dots[i].proc != my_pe) {
         to[dots[i].proc]++;
         m++;
      }

   num_moved_lb += m;
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Allreduce\n", my_pe, __func__, __FILE__); fflush(stdout); }
   CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
   gasket__mpi_Allreduce(glbl, &m, &lcl->n, 1, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
   t4 = timer();
   t2 = t4 - t1;
   if (lcl->n) {  // Only move dots and blocks if there is something to move
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Alltoall\n", my_pe, __func__, __FILE__); fflush(stdout); }
      CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
      gasket__mpi_Alltoall(glbl, to, 1, MPI_INTEGER, from, 1, MPI_INTEGER, MPI_COMM_WORLD, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }

      CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
      move_dots_back(glbl);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
      t5 = timer();
      t3 = t5 - t4;
      t4 = t5;

//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
      CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
      move_blocks(glbl, &tp, &tm, &tu);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
   }
   t5 = timer() - t4;
   timer_lb_misc += timer() - t1 - t2 - t3 - tp - tm - tu;
   timer_lb_sort += t2;
   timer_lb_pa += tp;
   timer_lb_mv += tm;
   timer_lb_un += tu;
   timer_lb_mb += t3;
   timer_lb_ma += t5;

//???{ printf("pe = %3d, %30s %23s, At exit\n", my_pe, __func__, __FILE__); fflush(stdout); }

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

#undef  npx1
#undef  npy1
#undef  npz1
#undef  nfac
#undef  fac
#undef  fact
#undef  i
#undef  j
#undef  m
// #undef  n
#undef  dir
#undef  in
#undef  t1
#undef  t2
#undef  t3
#undef  t4
#undef  t5
#undef  tp
#undef  tm
#undef  tu
#undef  bp
}

void exchange(Globals_t * const glbl, double * const tp, double * const tm, double * const tu)
{
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_to
   OBTAIN_ACCESS_TO_from
   OBTAIN_ACCESS_TO_dots

   typedef struct {
      Frame_Header_t myFrame;
      int f, s, sp, fp, i, j[25], l, rb, lev, block_size, type, type1,
          par[25], start[25];
      double t1, t2, t3, t4;
      MPI_Status status;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__exchange_t;

#define f                 (lcl->f)
#define s                 (lcl->s)
#define sp                (lcl->sp)
#define fp                (lcl->fp)
#define i                 (lcl->i)
#define j                 (lcl->j)
#define l                 (lcl->l)
#define rb                (lcl->rb)
#define lev               (lcl->lev)
#define block_size        (lcl->block_size)
#define type              (lcl->type)
#define type1             (lcl->type1)
#define par               (lcl->par)
#define start             (lcl->start)
#define t1                (lcl->t1)
#define t2                (lcl->t2)
#define t3                (lcl->t3)
#define t4                (lcl->t4)
#define status            (lcl->status)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__exchange_t)
   //TRACErcb
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, At entry\n", my_pe, __func__, __FILE__); fflush(stdout); }

   block_size = 45 + num_vars*x_block_size*y_block_size*z_block_size;
   type = 40;
   type1 = 41;

   par[0] = 1;
   lev = 0;
   while (par[lev] < num_pes) {
      par[lev+1] = 2*par[lev];
      lev++;
   }
   j[l = 0] = 0;
   start[0] = 0;
   while(j[0] < 2) {
      if (l == lev) {
         t3 = t4 = 0.0;
         t1 = timer();
         sp = fp = s = f = 0;
         // The sense of to and from are reversed in this routine.  They are
         // related to moving the dots from where they ended up back to the
         // processors that they came from and blocks are moved in reverse.
         while (s < from[start[l]] || f < to[start[l]]) {
            if (f < to[start[l]]) {
               if (num_active < max_num_blocks) {
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Irecv\n", my_pe, __func__, __FILE__); fflush(stdout); }
                  gasket__mpi_Irecv(glbl, recv_buff, block_size, MPI_DOUBLE, start[l], type,
                            MPI_COMM_WORLD, -9999, request, __FILE__, __LINE__);
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
                  rb = 1;
               } else
                  rb = 0;
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Send 1\n", my_pe, __func__, __FILE__); fflush(stdout); }
               CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
               gasket__mpi_Send(glbl, &rb, 1, MPI_INTEGER, start[l], type1, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
               DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
            }
            if (s < from[start[l]]) {
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Recv\n", my_pe, __func__, __FILE__); fflush(stdout); }
               CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
               gasket__mpi_Recv(glbl, &i, 1, MPI_INTEGER, start[l], type1,
                        MPI_COMM_WORLD, -9999, &status, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
               DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
               if (i) {
                  while (sp < max_active_block && blocks[sp].number < 0 ||
                         (blocks[sp].number >= 0 &&
                            (blocks[sp].new_proc != start[l] ||
                             blocks[sp].new_proc == my_pe)))
                     sp++;
                  t2 = timer();
                  pack_block(glbl, sp);
                  t3 += timer() - t2;
                  num_active--;
                  local_num_blocks[blocks[sp].level]--;
                  del_sorted_list(glbl, blocks[sp].number, blocks[sp].level);
                  blocks[sp].number = -1;
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Send 2\n", my_pe, __func__, __FILE__); fflush(stdout); }
                  CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
                  gasket__mpi_Send(glbl, send_buff, block_size, MPI_DOUBLE, start[l], type,
                           MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
                  DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
                  if (fp > sp)
                     fp = sp;
                  sp++;
                  from[start[l]]--;
               } else
                  s = from[start[l]];
            }
            if (f < to[start[l]]) {
               if (rb) {
                  while (fp < max_num_blocks && blocks[fp].number >= 0)
                     fp++;
                  if (fp == max_num_blocks)
                     if (num_active == max_num_blocks) {
                        printf("ERROR: exchange - need more blocks\n");
                        exit(-1);
                     } else {  // there is at least one free block
                        fp = 0;
                        while (blocks[fp].number >= 0)
                           fp++;
                     }
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Wait\n", my_pe, __func__, __FILE__); fflush(stdout); }
                  CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
                  gasket__mpi_Wait__for_Irecv(glbl, recv_buff, block_size, MPI_DOUBLE, start[l], MPI_COMM_WORLD, -9999, request, &status, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
                  DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
                  t2 = timer();
                  unpack_block(glbl, fp);
                  t4 += timer() - t2;
                  if ((fp+1) > max_active_block)
                     max_active_block = fp + 1;
                  num_active++;
                  local_num_blocks[blocks[fp].level]++;
                  add_sorted_list(glbl, fp, blocks[fp].number, blocks[fp].level);
                  fp++;
                  to[start[l]]--;
               } else
                  f = to[start[l]];
            }
         }
         *tm += timer() - t1 - t3 - t4;
         *tp += t3;
         *tu += t4;

         l--;
         j[l]++;
      } else if (j[l] == 0) {
         j[l+1] = 0;
         if (my_pe & par[l])
            if (start[l]+par[l] < num_pes)
               start[l+1] = start[l] + par[l];
            else {
               start[l+1] = start[l];
               j[l] = 1;
            }
         else
            start[l+1] = start[l];
         l++;
      } else if (j[l] == 1) {
         j[l+1] = 0;
         if (my_pe & par[l]) {
            start[l+1] = start[l];
            l++;
         } else
            if (start[l]+par[l] < num_pes) {
               start[l+1] = start[l] + par[l];
               l++;
            } else {
               l--;
               j[l]++;
            }
      } else {
         l--;
         j[l]++;
      }
   }
//???{ printf("pe = %3d, %30s %23s, At exit\n", my_pe, __func__, __FILE__); fflush(stdout); }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

#undef  f
#undef  s
#undef  sp
#undef  fp
#undef  i
#undef  j
#undef  l
#undef  rb
#undef  lev
#undef  block_size
#undef  type
#undef  type1
#undef  par
#undef  start
#undef  t1
#undef  t2
#undef  t3
#undef  t4
#undef  status
}

// Sort by binning the dots.  Bin in the desired direction, find the bin
// (or bin) that divides, bin in another direction, and repeat that
// for the third direction.  Then with the three bins (if we need all three
// to get a good binning), we can divide the dots.
// Communicate the dots at the end of each stage.  At the end, use the dots
// to determine where to send the blocks to.
void sort(Globals_t * const glbl, int const div, int const fact, int const dir)
{
   OBTAIN_ACCESS_TO_dots

   typedef struct {
      Frame_Header_t myFrame;
      int i, j, sum, total_dots, part, dir1, point1, extra1,
          bin1[fact], point[fact], extra[fact];
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__sort_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define sum               (lcl->sum)
#define total_dots        (lcl->total_dots)
#define part              (lcl->part)
#define dir1              (lcl->dir1)
#define point1            (lcl->point1)
#define extra1            (lcl->extra1)
#define bin1              (lcl->bin1)
#define point             (lcl->point)
#define extra             (lcl->extra)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__sort_t)

   //TRACErcb
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, At entry\n", my_pe, __func__, __FILE__); fflush(stdout); }

//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Allreduce 1, div=%d, me[div]=%d, np[div]=%d\n", my_pe, __func__, __FILE__, div, me[div], np[div]); fflush(stdout); }
   CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
   gasket__mpi_Allreduce (glbl, &num_dots, &total_dots, 1, MPI_INTEGER, MPI_SUM, comms[div], div, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
//???{ printf("pe = %3d, %30s %23s, rtn frm Allreduce 1, div=%d, me[div]=%d, np[div]=%d\n", my_pe, __func__, __FILE__, div, me[div], np[div]); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, Allreduce of num_dots to total_dots: %d to %d,  fact=%d, part=%d\n", my_pe, __func__, __FILE__, num_dots, total_dots, fact, ((total_dots+fact-1)/fact)); fflush(stdout); }

   for (i = 0; i < mesh_size[dir]; i++)
      bin[i] = 0;

   for (i = 0; i < max_active_dot; i++)
      if (dots[i].number >= 0) {
//>>>{ printf("pe = %3d, %30s %23s line %7d, i = %d, dir = %d, max_active_dot = %d, dots[i].cen[dir] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, i, dir, max_active_dot, dots[i].cen[dir]); fflush(stdout); }
         bin[dots[i].cen[dir]]++;
      }

//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Allreduce 2, div=%d, me[div]=%d, np[div]=%d\n", my_pe, __func__, __FILE__, div, me[div], np[div]); fflush(stdout); }
   CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
   gasket__mpi_Allreduce (glbl, bin, gbin, mesh_size[dir], MPI_INTEGER, MPI_SUM, comms[div], div, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, rtn frm Allreduce 2, div=%d, me[div]=%d, np[div]=%d\n", my_pe, __func__, __FILE__, div, me[div], np[div]); fflush(stdout); }

   part = (total_dots+fact-1)/fact;
   for (sum = j = i = 0; i < mesh_size[dir] && j < (fact-1); i++) {
//???{ printf("pe = %3d, %30s %23s, dir = %d, mesh_size[dir] = %d, fact = %d, i = %d, gbin[i] = %d, sum = %d -> %d, part = %d\n", my_pe, __func__, __FILE__, dir, mesh_size[dir], fact, i, gbin[i], sum, sum+gbin[i], part); fflush(stdout); }
      sum += gbin[i];
      if (sum >= (j+1)*part) {
         bin1[j] = gbin[i];
         extra[j] = sum - (j+1)*part;
//???{ printf("pe = %3d, %30s %23s, extra[%d] got set to %d\n", my_pe, __func__, __FILE__, j, extra[j]); fflush(stdout); }
         point[j++] = i;
      } else {
//???{ printf("pe = %3d, %30s %23s, extra[%d] got LEFT   %d\n", my_pe, __func__, __FILE__, j, extra[j]); fflush(stdout); }
      }
   }

   for (i = 0; i < max_active_dot; i++)
      if (dots[i].number >= 0) {
         for (j = 0; j < (fact-1); j++)
            if (dots[i].cen[dir] <  point[j]) {
               dots[i].new_proc = j;
               break;
            } else if (dots[i].cen[dir] == point[j]) {
               if (extra[j])
                  dots[i].new_proc = -1 - j;
               else
                  dots[i].new_proc = j;
               break;
            }
         if (j == (fact-1))
            dots[i].new_proc = j;
      }


//???{ printf("pe = %3d, %30s %23s, fact = %d\n", my_pe, __func__, __FILE__, fact); fflush(stdout); }
   for (j = 0; j < (fact-1); j++) {
//???{ printf("pe = %3d, %30s %23s, fact = %d, j = %d, extra[j] = %d\n", my_pe, __func__, __FILE__, fact, j, extra[j]); fflush(stdout); }
      if (extra[j]) {
         dir1 = (dir+1)%3;
//???{ printf("pe = %3d, %30s %23s, fact = %d, j = %d, extra[j] = %d, dir1 = %d, mesh_size[dir1] = %d\n", my_pe, __func__, __FILE__, fact, j, extra[j], dir1, mesh_size[dir1]); fflush(stdout); }
         for (i = 0; i < mesh_size[dir1]; i++)
            bin[i] = 0;
//???{ printf("pe = %3d, %30s %23s, fact = %d, j = %d, extra[j] = %d, dir1 = %d, mesh_size[dir1] = %d, max_active_dot = %d\n", my_pe, __func__, __FILE__, fact, j, extra[j], dir1, mesh_size[dir1], max_active_dot); fflush(stdout); }
         for (i = 0; i < max_active_dot; i++)
            if (dots[i].number >= 0 && dots[i].new_proc == (-1-j))
               bin[dots[i].cen[dir1]]++;
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Allreduce 3\n", my_pe, __func__, __FILE__); fflush(stdout); }
         CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         gasket__mpi_Allreduce (glbl, bin, gbin, mesh_size[dir1], MPI_INTEGER, MPI_SUM,
                       comms[div], div, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
         part = bin1[j] - extra[j];
         for (sum = i = 0; i < mesh_size[dir1]; i++) {
            sum += gbin[i];
            if (sum >= part) {
               extra1 = sum - part;
               point1 = i;
               bin1[j] = gbin[i];
               break;
            }
         }
         for (i = 0; i < max_active_dot; i++)
            if (dots[i].number >= 0)
               if (dots[i].new_proc == (-1-j))
                  if (dots[i].cen[dir1] < point1)
                     dots[i].new_proc = j;
                  else if (dots[i].cen[dir1] == point1) {
                     if (!extra1)
                        dots[i].new_proc = j;
                     // else dots[i].new_proc = (-1-j) - no change
                  } else
                     dots[i].new_proc = j + 1;
         if (extra1) {
            dir1 = (dir+2)%3;
            for (i = 0; i < mesh_size[dir1]; i++)
               bin[i] = 0;
            for (i = 0; i < max_active_dot; i++)
               if (dots[i].number >= 0 && dots[i].new_proc == (-1-j))
                  bin[dots[i].cen[dir1]]++;
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Allreduce 4\n", my_pe, __func__, __FILE__); fflush(stdout); }
            CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
            gasket__mpi_Allreduce (glbl, bin, gbin, mesh_size[dir1], MPI_INTEGER, MPI_SUM,
                          comms[div], div, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
            DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
            part = bin1[j] - extra1;
            for (sum = i = 0; i < mesh_size[dir1]; i++) {
               sum += gbin[i];
               if (sum >= part) {
                  point1 = i;
                  break;
               }
            }
            for (i = 0; i < max_active_dot; i++)
               if (dots[i].number >= 0)
                  if (dots[i].new_proc == (-1-j))
                     if (dots[i].cen[dir1] <= point1)
                        dots[i].new_proc = j;
                     else
                        dots[i].new_proc = j+1;
         }
      }
   }
//???{ printf("pe = %3d, %30s %23s, At exit\n", my_pe, __func__, __FILE__); fflush(stdout); }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  sum
#undef  total_dots
#undef  part
#undef  dir1
#undef  point1
#undef  extra1
#undef  bin1
#undef  point
#undef  extra

}

int factor(Globals_t * const glbl, int *fac)
{
   OBTAIN_ACCESS_TO_dots
   int nfac = 0, mfac = 2, done = 0;
   int number_of_pes = num_pes;
   //TRACErcb
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, At entry\n", my_pe, __func__, __FILE__); fflush(stdout); }

   while (!done)
      if (number_of_pes == (number_of_pes/mfac)*mfac) {
         fac[nfac++] = mfac;
         number_of_pes /= mfac;
         if (number_of_pes == 1)
            done = 1;
      } else {
         mfac++;
         if (mfac*mfac > number_of_pes) {
            fac[nfac++] = number_of_pes;
            done = 1;
         }
      }

//???{ printf("pe = %3d, %30s %23s, At exit nfac = %d\n", my_pe, __func__, __FILE__, nfac); fflush(stdout); }
   return nfac;
}

int find_dir(Globals_t * const glbl, int fact, int npx1, int npy1, int npz1)
{
   OBTAIN_ACCESS_TO_dots
   /* Find direction with largest number of processors left
    * that is divisible by the factor.
    */
   int dir;
   //TRACErcb
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, At entry\n", my_pe, __func__, __FILE__); fflush(stdout); }

   if (reorder) {
      if (fact > 2)
         if ((npx1/fact)*fact == npx1)
            if ((npy1/fact)*fact == npy1)
               if ((npz1/fact)*fact == npz1)
                  if (npx1 >= npy1)
                     if (npx1 >= npz1)
                        dir = 0;
                     else
                        dir = 2;
                  else
                     if (npy1 >= npz1)
                        dir = 1;
                     else
                        dir = 2;
               else
                  if (npx1 >= npy1)
                     dir = 0;
                  else
                     dir = 1;
            else
               if (((npz1/fact)*fact) == npz1)
                  if (npx1 >= npz1)
                     dir = 0;
                  else
                     dir = 2;
               else
                  dir = 0;
         else
            if ((npy1/fact)*fact == npy1)
               if (((npz1/fact)*fact) == npz1)
                  if (npy1 >= npz1)
                     dir = 1;
                  else
                     dir = 2;
               else
                  dir = 1;
            else
               dir = 2;
      else /* factor is 2 and np[xyz]1 are either 1 or a factor of 2 */
         if (npx1 >= npy1)
            if (npx1 >= npz1)
               dir = 0;
            else
               dir = 2;
         else
            if (npy1 >= npz1)
               dir = 1;
            else
               dir = 2;
   } else {
      /* if not reorder, divide z fist, y second, and x last */
      if (fact > 2)
         if ((npz1/fact)*fact == npz1)
            dir = 2;
         else if ((npy1/fact)*fact == npy1)
            dir = 1;
         else
            dir = 0;
      else
         if (npz1 > 1)
            dir = 2;
         else if (npy1 > 1)
            dir = 1;
         else
            dir = 0;
   }

//???{ printf("pe = %3d, %30s %23s, At exit dir = %d\n", my_pe, __func__, __FILE__, dir); fflush(stdout); }
   return dir;
}

void move_dots(Globals_t * const glbl, int const div, int const fact)
{
   OBTAIN_ACCESS_TO_dots

   typedef struct {
      Frame_Header_t myFrame;
      int i, j, d, sg, mg, partner, type, off[fact+1], which, err, nr;
      MPI_Status status;
      struct {
         int *send_int;
         int *recv_int;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__move_dots_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define d                 (lcl->d)
#define sg                (lcl->sg)
#define mg                (lcl->mg)
#define partner           (lcl->partner)
#define type              (lcl->type)
#define off               (lcl->off)
#define which             (lcl->which)
#define err               (lcl->err)
#define nr                (lcl->nr)
#define send_int          (lcl->pointers.send_int)
#define recv_int          (lcl->pointers.recv_int)
#define status            (lcl->status)


   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__move_dots_t)
   //TRACErcb
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, At entry\n", my_pe, __func__, __FILE__); fflush(stdout); }

   send_int = (int *) send_buff;
   recv_int = (int *) recv_buff;

   sg = np[div]/fact;
   mg = me[div]/sg;

   for (i = 0; i < fact; i++)
      bin[i] = 0;

   // determine which proc to send dots to
   for (d = 0; d < max_active_dot; d++)
      if (dots[d].number >= 0)
         bin[dots[d].new_proc]++;

   type = 30;
   for (i = 0; i < fact; i++)
      if (i != mg) {
         partner = me[div]%sg + i*sg;
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Irecv, expecting data from partner=%d (communicator div=%d, me[div]=%d, np[div]=%d)\n", my_pe, __func__, __FILE__, partner, div, me[div], np[div]); fflush(stdout); }
         gasket__mpi_Irecv(glbl, &gbin[i], 1, MPI_INTEGER, partner, type, comms[div], div,
                   &request[i], __FILE__, __LINE__);
//???{ printf("pe = %3d, %30s %23s, rtn frm Irecv, expecting data from partner=%d (communicator div=%d, me[div]=%d, np[div]=%d)\n", my_pe, __func__, __FILE__, partner, div, me[div], np[div]); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
      }

//???{ printf("pe = %3d, %30s %23s, About to do Sends, fact = %d\n", my_pe, __func__, __FILE__, fact); fflush(stdout); }
   for (i = 0; i < fact; i++)
      if (i != mg) {
         partner = me[div]%sg + i*sg;
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Send 1, sending to partner=%d, (communicator div=%d, me[div]=%d, np[div]=%d)\n", my_pe, __func__, __FILE__, partner, div, me[div], np[div]); fflush(stdout); }
         CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         gasket__mpi_Send(glbl, &bin[i], 1, MPI_INTEGER, partner, type, comms[div], div, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
      }
//???{ printf("pe = %3d, %30s %23s, Sends done; about do do waits, fact = %d\n", my_pe, __func__, __FILE__, fact); fflush(stdout); }

   type = 31;
   off[0] = 0;
   for (nr = i = 0; i < fact; i++) {
      if (i != mg) {
         partner = me[div]%sg + i*sg;
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Wait 1, waiting for partner=%d, (communicator div=%d, me[div]=%d, np[div]=%d)\n", my_pe, __func__, __FILE__, partner, div, me[div], np[div]); fflush(stdout); }
         CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         err = gasket__mpi_Wait__for_Irecv(glbl, &gbin[i], 1, MPI_INTEGER, partner, comms[div], div, &request[i], &status, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
         if (gbin[i] > 0) {
            partner = me[div]%sg + i*sg;
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Irecv\n", my_pe, __func__, __FILE__); fflush(stdout); }
            gasket__mpi_Irecv(glbl, &recv_int[off[i]], 6*gbin[i], MPI_INTEGER, partner,
                      type, comms[div], div, &request[i], __FILE__, __LINE__);
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
            off[i+1] = off[i] + 6*gbin[i];
            nr++;
         } else {
            off[i+1] = off[i];
            request[i] = MPI_REQUEST_NULL;
         }
      } else {
         off[i+1] = off[i];
         request[i] = MPI_REQUEST_NULL;
      }
   }
//???{ printf("pe = %3d, %30s %23s, Waits done\n", my_pe, __func__, __FILE__); fflush(stdout); }

//???{ printf("pe = %3d, %30s %23s, At point 2, num_dots = %d\n", my_pe, __func__, __FILE__, num_dots); fflush(stdout); }
   for (i = 0; i < fact; i++)
      if (i != mg && bin[i] > 0) {
         for (j = d = 0; d < max_active_dot; d++)
            if (dots[d].number >= 0 && dots[d].new_proc == i) {
               send_int[j++] = dots[d].cen[0];
               send_int[j++] = dots[d].cen[1];
               send_int[j++] = dots[d].cen[2];
               send_int[j++] = dots[d].number;
               send_int[j++] = dots[d].n;
               send_int[j++] = dots[d].proc;
               dots[d].number = -1;
               num_dots--;
            }

         partner = me[div]%sg + i*sg;
//if (my_pe == 134) { printf ("pe = %3d, send_int = 0x%p, j = %d, max_active_dot = %d\n", my_pe, send_int, j, max_active_dot);fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Send 2\n", my_pe, __func__, __FILE__); fflush(stdout); }
         CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         gasket__mpi_Send(glbl, send_int, 6*bin[i], MPI_INTEGER, partner, type, comms[div], div, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
      }
//???{ printf("pe = %3d, %30s %23s, At point 3, num_dots = %d, nr = %d\n", my_pe, __func__, __FILE__, num_dots, nr); fflush(stdout); }

#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this.  It is suitable only for reference version, because it does NOT necessarily Waitany for the ENTIRE list of requests.  See #else.
   for (d = i = 0; i < nr; i++) {
//???{ printf("pe = %3d, %30s %23s, calling Wait 2\n", my_pe, __func__, __FILE__); fflush(stdout); }
      err = MPI_Waitany (fact, request, &which, &status);
//???{ printf("pe = %3d, %30s %23s, At point 4, num_dots = %d, i = %d, which=%d, off[which] = %d, off[which+1] = %d\n", my_pe, __func__, __FILE__, num_dots, which, which, off[which], off[which+1]); fflush(stdout); }
      for (j = off[which]; j < off[which+1]; ) {
         for ( ; d < max_num_dots; d++)
            if (dots[d].number < 0)
               break;
         if (d == max_num_dots) {
            printf("%d ERROR: need more dots in move_dots %d %d\n",
                   my_pe, max_num_dots, num_dots);
            exit(-1);
         }
         dots[d].cen[0] = recv_int[j++];
         dots[d].cen[1] = recv_int[j++];
         dots[d].cen[2] = recv_int[j++];
         dots[d].number = recv_int[j++];
         dots[d].n = recv_int[j++];
         dots[d].proc = recv_int[j++];
         num_dots++;
         if ((d+1) > max_active_dot)
            max_active_dot = d+1;
      }
//???{ printf("pe = %3d, %30s %23s, At point 5, num_dots = %d\n", my_pe, __func__, __FILE__, num_dots); fflush(stdout); }
   }
#else  // Since not necessarily ALL elements request[0:fact-1] have a message pending to be received, we have to control the loop more carefully than the reference code above.
   d = 0;
   for (i = 0; i < fact; i++) {
      if (i != mg && gbin[i] > 0) {
         partner = me[div]%sg + i*sg;
//if (my_pe == 331) { printf ("pe = %3d, recv_int = 0x%p, j = %d, max_active_dot = %d\n", my_pe, recv_int, j, max_active_dot); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Wait 2\n", my_pe, __func__, __FILE__); fflush(stdout); }
         CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         err = gasket__mpi_Wait__for_Irecv (glbl, &recv_int[off[i]], 6*gbin[i], MPI_INTEGER, partner, comms[div], div, &request[i], &status, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, i=%d, off[i]=%d, off[i+1]=%d, max_num_dots=%d\n", my_pe, __func__, __FILE__, __LINE__, i, off[i], off[i+1], max_num_dots); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, At point 4, num_dots = %d, i = %d, which=%d, off[which] = %d, off[which+1] = %d\n", my_pe, __func__, __FILE__, num_dots, i, i, off[i], off[i+1]); fflush(stdout); }
         for (j = off[i]; j < off[i+1]; ) {
            for ( ; d < max_num_dots; d++) {
//>>>{ printf("pe = %3d, %30s %23s line %7d, d=%d, dots[d].number=%d\n", my_pe, __func__, __FILE__, __LINE__, d, dots[d].number); fflush(stdout); }
               if (dots[d].number < 0)
                  break;
            }
            if (d == max_num_dots) {
               printf("%d ERROR: need more dots in move_dots %d %d\n",
                      my_pe, max_num_dots, num_dots);
               exit(-1);
            }
//>>>{ printf("pe = %3d, %30s %23s line %7d, d=%d, j=%d, recv_int=%d %d %d %d %d %d at %p, num_dots++=%d\n", my_pe, __func__, __FILE__, __LINE__, d, j, recv_int[j], recv_int[j+1], recv_int[j+2], recv_int[j+3], recv_int[j+4], recv_int[j+5], recv_int, num_dots+1); fflush(stdout); }
            dots[d].cen[0] = recv_int[j++];
            dots[d].cen[1] = recv_int[j++];
            dots[d].cen[2] = recv_int[j++];
            dots[d].number = recv_int[j++];
            dots[d].n = recv_int[j++];
            dots[d].proc = recv_int[j++];
            num_dots++;
            if ((d+1) > max_active_dot)
               max_active_dot = d+1;
         }
//???{ printf("pe = %3d, %30s %23s, At point 5, num_dots = %d\n", my_pe, __func__, __FILE__, num_dots); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
      }
   }
#endif
//???{ printf("pe = %3d, %30s %23s, At exit\n", my_pe, __func__, __FILE__); fflush(stdout); }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  d
#undef  sg
#undef  mg
#undef  partner
#undef  type
#undef  off
#undef  which
#undef  err
#undef  nr
#undef  send_int
#undef  recv_int
#undef  status
}

void move_dots_back(Globals_t * const glbl)
{
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_to
   OBTAIN_ACCESS_TO_from
   OBTAIN_ACCESS_TO_dots

   typedef struct {
      Frame_Header_t myFrame;
      int i, j, d, nr, err, which;
      MPI_Status status;
      struct {
         int *send_int;
         int *recv_int;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__move_dots_back_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define d                 (lcl->d)
#define nr                (lcl->nr)
#define err               (lcl->err)
#define which             (lcl->which)
#define send_int          (lcl->pointers.send_int)
#define recv_int          (lcl->pointers.recv_int)
#define status            (lcl->status)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__move_dots_back_t)
   //TRACErcb
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, At entry\n", my_pe, __func__, __FILE__); fflush(stdout); }

   send_int = (int *) send_buff;
   recv_int = (int *) recv_buff;

   gbin[0] = 0;
   for (nr = i = 0; i < num_pes; i++)
      if (from[i] > 0) {
         gbin[i+1] = gbin[i] + 2*from[i];
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Irecv\n", my_pe, __func__, __FILE__); fflush(stdout); }
         gasket__mpi_Irecv(glbl, &recv_int[gbin[i]], 2*from[i], MPI_INTEGER, i, 50,
                   MPI_COMM_WORLD, -9999, &request[i], __FILE__, __LINE__);
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
         nr++;
      } else {
         gbin[i+1] = gbin[i];
         request[i] = MPI_REQUEST_NULL;
      }

   for (i = 0; i < num_pes; i++)
      if (to[i] > 0) {
         for (j = d = 0; d < max_active_dot; d++)
            if (dots[d].number >= 0 && dots[d].proc == i) {
               send_int[j++] = dots[d].n;
               send_int[j++] = my_pe;
            }
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Send\n", my_pe, __func__, __FILE__); fflush(stdout); }
         CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         gasket__mpi_Send(glbl, send_int, 2*to[i], MPI_INTEGER, i, 50, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
      }

#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this.  It is suitable only for reference version, because it does NOT necessarily Waitany for the ENTIRE list of requests.  See #else.
   for (i = 0; i < nr; i++) {
//???{ printf("pe = %3d, %30s %23s, calling Wait\n", my_pe, __func__, __FILE__); fflush(stdout); }
      err = MPI_Waitany (num_pes, request, &which, &status);
      for (j = 0; j < from[which]; j++)
         blocks[recv_int[gbin[which]+2*j]].new_proc =
               recv_int[gbin[which]+2*j+1];
   }
#else  // Since not necessarily ALL elements request[0:num_pes-1] have a message pending to be received, we have to control the loop more carefully than the reference code above.
   for (i = 0; i < num_pes; i++) {
      if (from[i] > 0) {
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Wait\n", my_pe, __func__, __FILE__); fflush(stdout); }
         CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         err = gasket__mpi_Wait__for_Irecv(glbl, &recv_int[gbin[i]], 2*from[i], MPI_INTEGER, i, MPI_COMM_WORLD, -9999, &request[i], &status, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
      }
      for (j = 0; j < from[i]; j++)
         blocks[recv_int[gbin[i]+2*j]].new_proc =
               recv_int[gbin[i]+2*j+1];
   }
#endif
//???{ printf("pe = %3d, %30s %23s, At exit\n", my_pe, __func__, __FILE__); fflush(stdout); }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  d
#undef  nr
#undef  err
#undef  which
#undef  send_int
#undef  recv_int
#undef  status
}

void move_blocks(Globals_t * const glbl, double * const tp, double * const tm, double * const tu)
{
   static int mul[3][3] = { {1, 2, 0}, {0, 2, 1}, {0, 1, 2} };

   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_parents
   OBTAIN_ACCESS_TO_from
   OBTAIN_ACCESS_TO_sorted_list
   OBTAIN_ACCESS_TO_dots

   typedef struct {
      Frame_Header_t myFrame;
      int n, n1, nl, p, c, c1, dir, i, j, k, i1, j1, k1, in,
          offset, off[3], f, fcase, pos[3], proc, number;
      struct {
         Block_t *bp, *bp1;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__move_blocks_t;

// #define n                 (lcl->n)  // Overloaded name ("n" appears as a member of a struct elsewhere.  Just use the fully qualified name, lcl->n).
#define n1                (lcl->n1)
#define nl                (lcl->nl)
#define p                 (lcl->p)
#define c                 (lcl->c)
#define c1                (lcl->c1)
#define dir               (lcl->dir)
#define i                 (lcl->i)
#define j                 (lcl->j)
#define k                 (lcl->k)
#define i1                (lcl->i1)
#define j1                (lcl->j1)
#define k1                (lcl->k1)
#define in                (lcl->in)
#define offset            (lcl->offset)
#define off               (lcl->off)
#define f                 (lcl->f)
#define fcase             (lcl->fcase)
#define pos               (lcl->pos)
#define proc              (lcl->proc)
// #define number            (lcl->number)  // Overloaded name ("number" appears as a member of a struct elsewhere.  Just use the fully qualified name, lcl->number).
#define bp                (lcl->pointers.bp)
#define bp1               (lcl->pointers.bp1)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__move_blocks_t)
   //TRACErcb
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, At entry\n", my_pe, __func__, __FILE__); fflush(stdout); }

   if (stencil == 7)  // add to face case when diags are needed
      f = 0;
   else
      f = 1;

   CALL_SUSPENDABLE_CALLEE(1)
   comm_proc(glbl);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
   CALL_SUSPENDABLE_CALLEE(1)
   comm_parent_proc(glbl);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
   CALL_SUSPENDABLE_CALLEE(1)
   update_comm_list(glbl);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)

   // go through blocks being moved and reset their nei[] list
   // (partially done above with comm_proc) and the lists of their neighbors
   for (in = 0; in < sorted_index[num_refine+1]; in++) {
      lcl->n = sorted_list[in].n;
      if ((bp = &blocks[lcl->n])->number >= 0 && bp->new_proc != my_pe) {
         for (c = 0; c < 6; c++) {
            c1 = (c/2)*2 + (c+1)%2;
            dir = c/2;
            fcase = (c1%2)*10;
            if (bp->nei_level[c] == (bp->level-1)) {
               if (bp->nei[c][0][0] >= 0)
                  for (k = fcase+6, i = 0; i < 2; i++)
                     for (j = 0; j < 2; j++, k++)
                        if (blocks[bp->nei[c][0][0]].nei[c1][i][j] == lcl->n) {
                           bp1 = &blocks[bp->nei[c][0][0]];
                           offset = p2[num_refine - bp1->level - 1];
                           bp1->nei[c1][i][j] = -1 - bp->new_proc;
                           bp1->nei_refine[c1] = bp->refine;
                           if (bp1->new_proc == my_pe) {
                              CALL_SUSPENDABLE_CALLEE(1)
                              add_comm_list(glbl, dir, bp->nei[c][0][0], bp->new_proc,
                                            k, ((bp1->cen[mul[dir][1]]+(2*i-1)*
                                                offset)*mesh_size[mul[dir][0]]
                                                + bp1->cen[mul[dir][0]]+(2*j-1)*offset),
                                            (bp1->cen[mul[dir][2]] +
                                             (2*(c1%2)-1)*p2[num_refine - bp1->level]));
                              DEBRIEF_SUSPENDABLE_FUNCTION(;)
                           }
                           bp->nei_refine[c] = bp1->refine;
                           bp->nei[c][0][0] = -1 - bp1->new_proc;
                           goto done;
                        }
               done: ;
            } else if (bp->nei_level[c] == bp->level) {
               if (bp->nei[c][0][0] >= 0) {
                  bp1 = &blocks[bp->nei[c][0][0]];
                  bp1->nei[c1][0][0] = -1 - bp->new_proc;
                  bp1->nei_refine[c1] = bp->refine;
                  if (bp1->new_proc == my_pe) {
                     CALL_SUSPENDABLE_CALLEE(1)
                     add_comm_list(glbl, dir, bp->nei[c][0][0], bp->new_proc, fcase+f,
                                   (bp1->cen[mul[dir][1]]*mesh_size[mul[dir][0]] +
                                    bp1->cen[mul[dir][0]]),
                                   (bp1->cen[mul[dir][2]] +
                                    (2*(c1%2)-1)*p2[num_refine - bp1->level]));
                     DEBRIEF_SUSPENDABLE_FUNCTION(;)
                  }
                  bp->nei_refine[c] = bp1->refine;
                  bp->nei[c][0][0] = -1 - bp1->new_proc;
               }
            } else if (bp->nei_level[c] == (bp->level+1)) {
               for (k = fcase+2, i = 0; i < 2; i++)
                  for (j = 0; j < 2; j++, k++)
                     if (bp->nei[c][i][j] >= 0) {
                        bp1 = &blocks[bp->nei[c][i][j]];
                        bp1->nei[c1][0][0] = -1 - bp->new_proc;
                        bp1->nei_refine[c1] = bp->refine;
                        if (bp1->new_proc == my_pe) {
                           CALL_SUSPENDABLE_CALLEE(1)
                           add_comm_list(glbl, dir, bp->nei[c][i][j], bp->new_proc, k,
                                         (bp1->cen[mul[dir][1]]*mesh_size[mul[dir][0]] +
                                          bp1->cen[mul[dir][0]]),
                                         (bp1->cen[mul[dir][2]] +
                                          (2*(c1%2)-1)*p2[num_refine- bp1->level]));
                           DEBRIEF_SUSPENDABLE_FUNCTION(;)
                        }
                        bp->nei_refine[c] = bp1->refine;
                        bp->nei[c][i][j] = -1 - bp1->new_proc;
                     }
            }
         }
         // move parent connection in blocks being moved
         if (bp->parent != -1)
            if (bp->parent_node == my_pe) {
               parents[bp->parent].child[bp->child_number] = bp->number;
               parents[bp->parent].child_node[bp->child_number] = bp->new_proc;
               CALL_SUSPENDABLE_CALLEE(1)
               add_par_list(glbl, &par_p, bp->parent, bp->number, bp->child_number,
                            bp->new_proc, 1);
               DEBRIEF_SUSPENDABLE_FUNCTION(;)
               bp->parent = parents[bp->parent].number;
            } else
               del_par_list(glbl, &par_b, (-2-bp->parent), lcl->n, bp->child_number,
                            bp->parent_node);
      }
   }

   /* swap blocks - if space is tight, may take multiple passes */
   j = 0;
   do {
      CALL_SUSPENDABLE_CALLEE(1)
      exchange(glbl, tp, tm, tu);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      for (n1 = i = 0; i < num_pes; i++)
         n1 += from[i];
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
//???{ printf("pe = %3d, %30s %23s, calling Allreduce\n", my_pe, __func__, __FILE__); fflush(stdout); }
      CALL_SUSPENDABLE_CALLEE(1)
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
      gasket__mpi_Allreduce(glbl, &n1, &lcl->n, 1, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
//{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
//>>>{ printf("pe = %3d, %30s %23s line %7d\n", my_pe, __func__, __FILE__, __LINE__); fflush(stdout); }
//>>>{ printf("pe = %3d, %30s %23s line %7d, max_active_dot = %d, dots is at %p, dots[12].cen[2] = 0x%x\n", my_pe, __func__, __FILE__, __LINE__, max_active_dot, dots, dots[12].cen[2]); fflush(stdout); }
      j++;
   } while (lcl->n && j < 10);

   // reestablish on-core and off-core comm lists
   for (in = 0; in < sorted_index[num_refine+1]; in++) {
      lcl->n = sorted_list[in].n;
      if ((bp = &blocks[lcl->n])->number >= 0 && bp->new_proc == -1) {
         nl = bp->number - block_start[bp->level];
         pos[2] = nl/((p2[bp->level]*npx*init_block_x)*
                      (p2[bp->level]*npy*init_block_y));
         pos[1] = (nl%((p2[bp->level]*npx*init_block_x)*
                       (p2[bp->level]*npy*init_block_y)))/
                  (p2[bp->level]*npx*init_block_x);
         pos[0] = nl%(p2[bp->level]*npx*init_block_x);
         for (c = 0; c < 6; c++) {
            dir = c/2;
            i1 = j1 = k1 = 0;
            if      (c == 0) i1 = -1;
            else if (c == 1) i1 =  1;
            else if (c == 2) j1 = -1;
            else if (c == 3) j1 =  1;
            else if (c == 4) k1 = -1;
            else if (c == 5) k1 =  1;
            c1 = (c/2)*2 + (c+1)%2;
            fcase = (c%2)*10;
            if (bp->nei_level[c] == (bp->level-1)) {
               if (bp->nei[c][0][0] < 0) {
                  proc = -1 - bp->nei[c][0][0];
                  i = pos[mul[dir][1]]%2;
                  j = pos[mul[dir][0]]%2;
                  if (proc == my_pe) {
                     lcl->number = (((pos[2]/2+k1)*p2[bp->level-1]*npy*init_block_y)+
                                (pos[1]/2+j1))*p2[bp->level-1]*npx*init_block_x+
                              pos[0]/2 + i1 + block_start[bp->level-1];
                     n1 = find_sorted_list(glbl, lcl->number, (bp->level-1));
                     bp->nei[c][0][0] = n1;
                     blocks[n1].nei[c1][i][j] = lcl->n;
                  } else {
                     CALL_SUSPENDABLE_CALLEE(1)
                     add_comm_list(glbl, dir, lcl->n, proc, fcase+2+2*i+j,
                                   (bp->cen[mul[dir][1]]*mesh_size[mul[dir][0]] +
                                    bp->cen[mul[dir][0]]),
                                   (bp->cen[mul[dir][2]] +
                                    (2*(c%2)-1)*p2[num_refine- bp->level]));
                     DEBRIEF_SUSPENDABLE_FUNCTION(;)
                  }
               }
            } else if (bp->nei_level[c] == bp->level) {
               if (bp->nei[c][0][0] < 0) {
                  proc = -1 - bp->nei[c][0][0];
                  if (proc == my_pe) {
                     lcl->number = (((pos[2]+k1)*p2[bp->level]*npy*init_block_y) +
                                (pos[1]+j1))*p2[bp->level]*npx*init_block_x +
                              pos[0] + i1 + block_start[bp->level];
                     n1 = find_sorted_list(glbl, lcl->number, bp->level);
                     bp->nei[c][0][0] = n1;
                     blocks[n1].nei[c1][0][0] = lcl->n;
                  } else {
                     CALL_SUSPENDABLE_CALLEE(1)
                     add_comm_list(glbl, dir, lcl->n, proc, fcase+f,
                                   (bp->cen[mul[dir][1]]*mesh_size[mul[dir][0]] +
                                    bp->cen[mul[dir][0]]),
                                   (bp->cen[mul[dir][2]] +
                                    (2*(c%2)-1)*p2[num_refine- bp->level]));
                     DEBRIEF_SUSPENDABLE_FUNCTION(;)
                  }
               }
            } else if (bp->nei_level[c] == (bp->level+1)) {
               offset = p2[num_refine - bp->level - 1];
               off[0] = off[1] = off[2] = 0;
               for (k = fcase+6, i = 0; i < 2; i++)
                  for (j = 0; j < 2; j++, k++)
                     if (bp->nei[c][i][j] < 0) {
                        off[mul[dir][0]] = j;
                        off[mul[dir][1]] = i;
                        proc = -1 - bp->nei[c][i][j];
                        if (proc == my_pe) {
                           lcl->number = (((2*(pos[2]+k1)-(k1-1)/2+off[2])*
                                          p2[bp->level+1]*npy*init_block_y) +
                                      (2*(pos[1]+j1)-(j1-1)/2+off[1]))*
                                          p2[bp->level+1]*npx*init_block_x +
                                    2*(pos[0]+i1)-(i1-1)/2 + off[0] +
                                    block_start[bp->level+1];
                           n1 = find_sorted_list(glbl, lcl->number, (bp->level+1));
                           bp->nei[c][i][j] = n1;
                           blocks[n1].nei[c1][0][0] = lcl->n;
                        } else {
                           CALL_SUSPENDABLE_CALLEE(1)
                           add_comm_list(glbl, dir, lcl->n, proc, k,
                                         ((bp->cen[mul[dir][1]]+(2*i-1)*offset)*mesh_size[mul[dir][0]]
                                          + bp->cen[mul[dir][0]]+(2*j-1)*offset),
                                         (bp->cen[mul[dir][2]] +
                                          (2*(c%2)-1)*p2[num_refine- bp->level]));
                           DEBRIEF_SUSPENDABLE_FUNCTION(;)
                        }
                     }
            }
         }
         // connect to parent if moved
         if (bp->parent != -1)
            if (bp->parent_node == my_pe) {
               for (p = 0; p < max_active_parent; p++)
                  if (parents[p].number == -2 - bp->parent) {
                     bp->parent = p;
                     parents[p].child[bp->child_number] = lcl->n;
                     parents[p].child_node[bp->child_number] = my_pe;
                     break;
                  }
            } else {
               CALL_SUSPENDABLE_CALLEE(1)
               add_par_list(glbl, &par_b, (-2-bp->parent), lcl->n, bp->child_number,
                            bp->parent_node, 0);
               DEBRIEF_SUSPENDABLE_FUNCTION(;)
            }
      }
   }
//???{ printf("pe = %3d, %30s %23s, At exit\n", my_pe, __func__, __FILE__); fflush(stdout); }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
// #undef  n
#undef  n1
#undef  nl
#undef  p
#undef  c
#undef  c1
#undef  dir
#undef  i
#undef  j
#undef  k
#undef  i1
#undef  j1
#undef  k1
#undef  in
#undef  offset
#undef  off
#undef  f
#undef  fcase
#undef  pos
#undef  proc
// #undef  number
#undef  bp
#undef  bp1
}
