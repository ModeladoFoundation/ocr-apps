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
#include <math.h>

#include "control.h"
#include "meta.h"
#include "block.h"
#include "proto.h"

void init (BlockMeta_t * meta) {
   typedef struct {
      Frame_Header_t myFrame;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__init_t;

   Control_t    * control  = meta->controlDb.base;
   double (* pCells) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
           (double(*)/*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (meta->blockDb.base->cells);

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__init_t)
printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
   int seed = meta->xPos + (meta->yPos * control->npx) + (meta->zPos * control->npx * control->npy);
   int var, i, j, k, l;
   l = 0;
   for (var = 0; var < control->num_vars; var++) {
      for (i = 0; i <= control->x_block_size+1; i++) {
         for (j = 0; j <= control->y_block_size+1; j++) {
            for (k = 0; k <= control->z_block_size+1; k++) {
               pCells[var][i][j][k] = 999900 + l++;
            }
         }
      }
      for (i = 1; i <= control->x_block_size; i++) {
         for (j = 1; j <= control->y_block_size; j++) {
            for (k = 1; k <= control->z_block_size; k++) {
               pCells[var][i][j][k] = ((double) rand_r(&seed))/((double) RAND_MAX);
               //pCells[var][i][j][k] = ((double) ((meta->xPos * 2 + i) * 100 + (meta->yPos * 2 + j) * 10 + (meta->zPos * 2 + k)));
            }
         }
      }
   }
   // Generate the golden checksum.
   CALL_SUSPENDABLE_CALLEE
   transmitBlockContributionForChecksum(meta, 0);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
} // init


// TODO: FIXME: clean-up
#if 0
//#include <mpi.h>

//#include "block.h"
//#include "comm.h"
//#include "proto.h"
// Initialize the problem and setup initial blocks.
void init(Globals_t * const glbl)
{
   OBTAIN_ACCESS_TO_grid_sum
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_cells

   typedef struct {
      Frame_Header_t myFrame;
      int n, var, i, j, k, l, m, o, size, dir, i1, i2, j1, j2, k1, k2, ib, jb, kb;
      int start[num_pes], pos[3][num_pes], pos1[npx][npy][npz], set,
          num, npx1, npy1, npz1, pes, fact, fac[25], nfac, f;
      int max_mesh_size;
      struct {
         Block_t *bp;
         Cell_t  *cp;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__init_t;

#define n                 (lcl->n)
#define var               (lcl->var)
#define i                 (lcl->i)
#define j                 (lcl->j)
#define k                 (lcl->k)
#define l                 (lcl->l)
#define m                 (lcl->m)
#define o                 (lcl->o)
#define size              (lcl->size)
#define dir               (lcl->dir)
#define i1                (lcl->i1)
#define i2                (lcl->i2)
#define j1                (lcl->j1)
#define j2                (lcl->j2)
#define k1                (lcl->k1)
#define k2                (lcl->k2)
#define ib                (lcl->ib)
#define jb                (lcl->jb)
#define kb                (lcl->kb)
#define start             (lcl->start)
#define pos               (lcl->pos)
#define pos1              (lcl->pos1)
#define set               (lcl->set)
#define num               (lcl->num)
#define npx1              (lcl->npx1)
#define npy1              (lcl->npy1)
#define npz1              (lcl->npz1)
#define pes               (lcl->pes)
#define fact              (lcl->fact)
#define fac               (lcl->fac)
#define nfac              (lcl->nfac)
#define f                 (lcl->f)
#define max_mesh_size     (lcl->max_mesh_size)
#define bp                (lcl->pointers.bp)
#define cp                (lcl->pointers.cp)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__init_t)
   TRACE

   tol = pow(10.0, ((double) -error_tol));

   p2[0] = /* p8 not used.  commented out!  p8[0] = */ 1;
   for (i = 0; i < (num_refine+1); i++) {
      // Not used:  p8[i+1] = p8[i]*8;
      p2[i+1] = p2[i]*2;
      sorted_index[i] = 0;
   }
   sorted_index[num_refine+1] = 0;
   block_start[0] = 0;
   global_max_b =  init_block_x*init_block_y*init_block_z;
   num = num_pes*global_max_b;
   for (i = 1; i <= num_refine; i++) {
      block_start[i] = block_start[i-1] + num;
      num *= 8;
      num_blocks[i] = 0;
      local_num_blocks[i] = 0;
   }

   /* initialize for communication arrays, which are initialized below */
   zero_comm_list(glbl);

   x_block_half = x_block_size/2;
   y_block_half = y_block_size/2;
   z_block_half = z_block_size/2;

   if (!code) {
      /* for E/W (X dir) messages:
         0: whole -> whole (7), 1: whole -> whole (27),
         2: whole -> quarter, 3: quarter -> whole */
      msg_len[0][0] = msg_len[0][1] = y_block_size*z_block_size;
      msg_len[0][2] = msg_len[0][3] = y_block_half*z_block_half;
      /* for N/S (Y dir) messages */
      msg_len[1][0] = x_block_size*z_block_size;
      msg_len[1][1] = (x_block_size+2)*z_block_size;
      msg_len[1][2] = msg_len[1][3] = x_block_half*z_block_half;
      /* for U/D (Z dir) messages */
      msg_len[2][0] = x_block_size*y_block_size;
      msg_len[2][1] = (x_block_size+2)*(y_block_size+2);
      msg_len[2][2] = msg_len[2][3] = x_block_half*y_block_half;
   } else if (code == 1) {
      /* for E/W (X dir) messages */
      msg_len[0][0] = msg_len[0][1] = (y_block_size+2)*(z_block_size+2);
      msg_len[0][2] = (y_block_half+1)*(z_block_half+1);
      msg_len[0][3] = (y_block_half+2)*(z_block_half+2);
      /* for N/S (Y dir) messages */
      msg_len[1][0] = msg_len[1][1] = (x_block_size+2)*(z_block_size+2);
      msg_len[1][2] = (x_block_half+1)*(z_block_half+1);
      msg_len[1][3] = (x_block_half+2)*(z_block_half+2);
      /* for U/D (Z dir) messages */
      msg_len[2][0] = msg_len[2][1] = (x_block_size+2)*(y_block_size+2);
      msg_len[2][2] = (x_block_half+1)*(y_block_half+1);
      msg_len[2][3] = (x_block_half+2)*(y_block_half+2);
   } else {
      /* for E/W (X dir) messages */
      msg_len[0][0] = msg_len[0][1] = (y_block_size+2)*(z_block_size+2);
      msg_len[0][2] = (y_block_half+1)*(z_block_half+1);
      msg_len[0][3] = (y_block_size+2)*(z_block_size+2);
      /* for N/S (Y dir) messages */
      msg_len[1][0] = msg_len[1][1] = (x_block_size+2)*(z_block_size+2);
      msg_len[1][2] = (x_block_half+1)*(z_block_half+1);
      msg_len[1][3] = (x_block_size+2)*(z_block_size+2);
      /* for U/D (Z dir) messages */
      msg_len[2][0] = msg_len[2][1] = (x_block_size+2)*(y_block_size+2);
      msg_len[2][2] = (x_block_half+1)*(y_block_half+1);
      msg_len[2][3] = (x_block_size+2)*(y_block_size+2);
   }

   /* Determine position of each core in initial mesh */
   npx1 = npx;
   npy1 = npy;
   npz1 = npz;
   for (i = 0; i < 3; i++)
      for (j = 0; j < num_pes; j++)
         pos[i][j] = 0;
   nfac = factor(glbl, fac);
   max_num_req = num_pes;
   gasket__ma_malloc(glbl, &dbmeta__request, (void *) &request, max_num_req*sizeof(MPI_Request), __FILE__, __LINE__);
   if (nonblocking) {
      gasket__ma_malloc(glbl, &dbmeta__s_req, (void *) &s_req, max_num_req*sizeof(MPI_Request), __FILE__, __LINE__);
   }
   pes = 1;
   start[0] = 0;
   num = num_pes;
   gasket__ma_malloc(glbl, &dbmeta__comms, (void *) &comms, (nfac+1)*sizeof(MPI_Comm), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &dbmeta__me, (void *) &me, (nfac+1)*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &dbmeta__np, (void *) &np, (nfac+1)*sizeof(int), __FILE__, __LINE__);
   SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.
   comms[0] = MPI_COMM_WORLD;
   me[0] = my_pe;
   np[0] = num_pes;
   // initialize
   for (n = 0, i = nfac; i > 0; i--, n++) {
      fact = fac[i-1];
      dir = find_dir(glbl, fact, npx1, npy1, npz1);
      if (dir == 0)
         npx1 /= fact;
      else
         if (dir == 1)
            npy1 /= fact;
         else
            npz1 /= fact;
      num /= fact;
      set = me[n]/num;
#ifdef BUILD_OCR_VERSION
      // It would be nice to be able to utilize the MPI communicators as a way to know who to
      // talk to.  But this stuff doesn't work unless we do an MPI_Init, and that doesn't seem
      // to work unless we start the program with a command-line invocation of mpirun, which starts
      // up a whole bunch of ranks.  In OCR, we want OCR to create the tasks, so we don't want to
      // start up any more than a single thread, so we don't want to use mpirun. Soooo....
      //
      // ... The places where this is needed are hacked to get just the required information in an alternate fashion.
      np[n+1] = num;
      me[n+1] = my_pe % num;
#else
      MPI_Comm_split(comms[n], set, me[n], &comms[n+1]);
      MPI_Comm_rank(comms[n+1], &me[n+1]);
      MPI_Comm_size(comms[n+1], &np[n+1]);
#endif
      for (j = pes-1; j >= 0; j--)
         for (k = 0; k < fact; k++) {
            m = j*fact + k;
            if (!k)
               start[m] = start[j];
            else
               start[m] = start[m-1] + num;
            for (l = start[m], o = 0; o < num; l++, o++)
               pos[dir][l] = pos[dir][l]*fact + k;
         }
      pes *= fact;
   }
   for (i = 0; i < num_pes; i++)
      pos1[pos[0][i]][pos[1][i]][pos[2][i]] = i;

   max_active_block = init_block_x*init_block_y*init_block_z;
   num_active = max_active_block;
   global_active = num_active*num_pes;
   num_parents = max_active_parent = 0;
   size = p2[num_refine+1];  /* block size is p2[num_refine+1-level]
                              * smallest block is size p2[1], so can find
                              * its center */
   mesh_size[0] = npx*init_block_x*size;
   max_mesh_size = mesh_size[0];
   mesh_size[1] = npy*init_block_y*size;
   if (mesh_size[1] > max_mesh_size)
      max_mesh_size = mesh_size[1];
   mesh_size[2] = npz*init_block_z*size;
   if (mesh_size[2] > max_mesh_size)
      max_mesh_size = mesh_size[2];
   if ((num_pes+1) > max_mesh_size)
      max_mesh_size = num_pes + 1;
   gasket__ma_malloc(glbl, &dbmeta__bin , (void *) &bin , max_mesh_size*sizeof(int), __FILE__, __LINE__);
   gasket__ma_malloc(glbl, &dbmeta__gbin, (void *) &gbin, max_mesh_size*sizeof(int), __FILE__, __LINE__);
   SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.
   if (stencil == 7)
      f = 0;
   else
      f = 1;
   for (o = n = k1 = k = 0; k < npz; k++)
      for (k2 = 0; k2 < init_block_z; k1++, k2++)
         for (j1 = j = 0; j < npy; j++)
            for (j2 = 0; j2 < init_block_y; j1++, j2++)
               for (i1 = i = 0; i < npx; i++)
                  for (i2 = 0; i2 < init_block_x; i1++, i2++, n++) {
                     m = pos1[i][j][k];
                     if (m == my_pe) {
                        bp = &blocks[o];
                        cp = &cells[0];
                        bp->level = 0;
                        bp->number = n;
                        bp->parent = -1;
                        bp->cen[0] = i1*size + size/2;
                        bp->cen[1] = j1*size + size/2;
                        bp->cen[2] = k1*size + size/2;
                        add_sorted_list(glbl, o, n, 0);
                        for (var = 0; var < num_vars; var++)
                           for (ib = 1; ib <= x_block_size; ib++)
                              for (jb = 1; jb <= y_block_size; jb++)
                                 for (kb = 1; kb <= z_block_size; kb++)
                                    (*cp)[var][ib][jb][kb] =
                                       ((double) rand())/((double) RAND_MAX);
                        if (i2 == 0)
                           if (i == 0) { /* 0 boundary */
                              bp->nei_level[0] = -2;
                              bp->nei[0][0][0] = 0;
                           } else {      /* boundary with neighbor core */
                              bp->nei_level[0] = 0;
                              bp->nei[0][0][0] = -1 - pos1[i-1][j][k];
                              CALL_SUSPENDABLE_CALLEE(1)
                              add_comm_list(glbl, 0, o, pos1[i-1][j][k], 0+f,
                                            bp->cen[2]*mesh_size[1]+bp->cen[1],
                                            bp->cen[0] - size/2);
                              DEBRIEF_SUSPENDABLE_FUNCTION(;)
                           }
                        else {          /* neighbor on core */
                           bp->nei_level[0] = 0;
                           bp->nei[0][0][0] = o - 1;
                        }
                        bp->nei_refine[0] = 0;
                        if (i2 == (init_block_x - 1))
                           if (i == (npx - 1)) { /* 1 boundary */
                              bp->nei_level[1] = -2;
                              bp->nei[1][0][0] = 0;
                           } else {      /* boundary with neighbor core */
                              bp->nei_level[1] = 0;
                              bp->nei[1][0][0] = -1 - pos1[i+1][j][k];
                              CALL_SUSPENDABLE_CALLEE(1)
                              add_comm_list(glbl, 0, o, pos1[i+1][j][k], 10+f,
                                            bp->cen[2]*mesh_size[1]+bp->cen[1],
                                            bp->cen[0] + size/2);
                              DEBRIEF_SUSPENDABLE_FUNCTION(;)
                           }
                        else {          /* neighbor on core */
                           bp->nei_level[1] = 0;
                           bp->nei[1][0][0] = o + 1;
                        }
                        bp->nei_refine[1] = 0;
                        if (j2 == 0)
                           if (j == 0) { /* 0 boundary */
                              bp->nei_level[2] = -2;
                              bp->nei[2][0][0] = 0;
                           } else {      /* boundary with neighbor core */
                              bp->nei_level[2] = 0;
                              bp->nei[2][0][0] = -1 - pos1[i][j-1][k];
                              CALL_SUSPENDABLE_CALLEE(1)
                              add_comm_list(glbl, 1, o, pos1[i][j-1][k], 0+f,
                                            bp->cen[2]*mesh_size[0]+bp->cen[0],
                                            bp->cen[1] - size/2);
                              DEBRIEF_SUSPENDABLE_FUNCTION(;)
                           }
                        else {          /* neighbor on core */
                           bp->nei_level[2] = 0;
                           bp->nei[2][0][0] = o - init_block_x;
                        }
                        bp->nei_refine[2] = 0;
                        if (j2 == (init_block_y - 1))
                           if (j == (npy - 1)) { /* 1 boundary */
                              bp->nei_level[3] = -2;
                              bp->nei[3][0][0] = 0;
                           } else {      /* boundary with neighbor core */
                              bp->nei_level[3] = 0;
                              bp->nei[3][0][0] = -1 - pos1[i][j+1][k];
                              CALL_SUSPENDABLE_CALLEE(1)
                              add_comm_list(glbl, 1, o, pos1[i][j+1][k], 10+f,
                                            bp->cen[2]*mesh_size[0]+bp->cen[0],
                                            bp->cen[1] + size/2);
                              DEBRIEF_SUSPENDABLE_FUNCTION(;)
                           }
                        else {          /* neighbor on core */
                           bp->nei_level[3] = 0;
                           bp->nei[3][0][0] = o + init_block_x;
                        }
                        bp->nei_refine[3] = 0;
                        if (k2 == 0)
                           if (k == 0) { /* 0 boundary */
                              bp->nei_level[4] = -2;
                              bp->nei[4][0][0] = 0;
                           } else {      /* boundary with neighbor core */
                              bp->nei_level[4] = 0;
                              bp->nei[4][0][0] = -1 - pos1[i][j][k-1];
                              CALL_SUSPENDABLE_CALLEE(1)
                              add_comm_list(glbl, 2, o, pos1[i][j][k-1], 0+f,
                                            bp->cen[1]*mesh_size[0]+bp->cen[0],
                                            bp->cen[2] - size/2);
                              DEBRIEF_SUSPENDABLE_FUNCTION(;)
                           }
                        else {          /* neighbor on core */
                           bp->nei_level[4] = 0;
                           bp->nei[4][0][0] = o - init_block_x*init_block_y;
                        }
                        bp->nei_refine[4] = 0;
                        if (k2 == (init_block_z - 1))
                           if (k == (npz - 1)) { /* 1 boundary */
                              bp->nei_level[5] = -2;
                              bp->nei[5][0][0] = 0;
                           } else {      /* boundary with neighbor core */
                              bp->nei_level[5] = 0;
                              bp->nei[5][0][0] = -1 - pos1[i][j][k+1];
                              CALL_SUSPENDABLE_CALLEE(1)
                              add_comm_list(glbl, 2, o, pos1[i][j][k+1], 10+f,
                                            bp->cen[1]*mesh_size[0]+bp->cen[0],
                                            bp->cen[2] + size/2);
                              DEBRIEF_SUSPENDABLE_FUNCTION(;)
                           }
                        else {          /* neighbor on core */
                           bp->nei_level[5] = 0;
                           bp->nei[5][0][0] = o + init_block_x*init_block_y;
                        }
                        bp->nei_refine[5] = 0;
                        o++;
                     }
                  }

   CALL_SUSPENDABLE_CALLEE(1)
   check_buff_size(glbl);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)

   for (var = 0; var < num_vars; var++) {
      CALL_SUSPENDABLE_CALLEE(1)
      grid_sum[var] = check_sum(glbl, var);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
   }

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

#undef  n
#undef  var
#undef  i
#undef  j
#undef  k
#undef  l
#undef  m
#undef  o
#undef  size
#undef  dir
#undef  i1
#undef  i2
#undef  j1
#undef  j2
#undef  k1
#undef  k2
#undef  ib
#undef  jb
#undef  kb
#undef  start
#undef  pos
#undef  pos1
#undef  set
#undef  num
#undef  npx1
#undef  npy1
#undef  npz1
#undef  pes
#undef  fact
#undef  fac
#undef  nfac
#undef  f
#undef  bp
#undef  max_mesh_size
#undef  cp
}
#endif
