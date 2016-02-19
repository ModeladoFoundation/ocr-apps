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
#include "proto.h"
#include "timer.h"

// This routine splits blocks that are being refined into 8 new blocks,
// copies the data over to the new blocks, and then disconnects the
// original block from the mesh and connects the new blocks to the
// mesh.  The information in old block is also transferred to a parent
// block, which also contains information to identify its children.
void split_blocks(Globals_t * const glbl)
{
   static int side[6][2][2] = { { {0, 2}, {4, 6} }, { {1, 3}, {5, 7} },
                                { {0, 1}, {4, 5} }, { {2, 3}, {6, 7} },
                                { {0, 1}, {2, 3} }, { {4, 5}, {6, 7} } };
   static int off[6] = {1, -1, 2, -2, 4, -4};
   static int mul[3][3] = { {1, 2, 0}, {0, 2, 1}, {0, 1, 2} };

   OBTAIN_ACCESS_TO_parents
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_cells

   typedef struct {
      Frame_Header_t myFrame;
      int i, j, k, m, n, o, v, nl, xp, yp, zp, c, c1, other,
          i1, i2, j1, j2, k1, k2, dir, fcase, pe, f, p,
          level, sib[8], offset, d, half_size;
      struct {
         Parent_t *pp;
         Block_t *bp, *bp1;
         Cell_t  *cp, *cp1;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__split_blocks_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define k                 (lcl->k)
#define m                 (lcl->m)
#define n                 (lcl->n)
#define o                 (lcl->o)
#define v                 (lcl->v)
#define nl                (lcl->nl)
#define xp                (lcl->xp)
#define yp                (lcl->yp)
#define zp                (lcl->zp)
#define c                 (lcl->c)
#define c1                (lcl->c1)
#define other             (lcl->other)
#define i1                (lcl->i1)
#define i2                (lcl->i2)
#define j1                (lcl->j1)
#define j2                (lcl->j2)
#define k1                (lcl->k1)
#define k2                (lcl->k2)
#define dir               (lcl->dir)
#define fcase             (lcl->fcase)
#define pe                (lcl->pe)
#define f                 (lcl->f)
#define p                 (lcl->p)
// #define level             (lcl->level)  // Name overload.  ("level" is used as an element of a struct elsewhere.)  Just use fully-qualified name in code below.
#define sib               (lcl->sib)
#define offset            (lcl->offset)
#define d                 (lcl->d)
#define half_size         (lcl->half_size)
#define pp                (lcl->pointers.pp)
#define bp                (lcl->pointers.bp)
#define bp1               (lcl->pointers.bp1)
#define cp                (lcl->pointers.cp)
#define cp1               (lcl->pointers.cp1)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__split_blocks_t)
   TRACE

   if (stencil == 7)  // add to face case when diags are needed
      f = 0;
   else
      f = 1;
   // have to do this by level, else could have a block at a level split and
   // its offspring try to connect to a block at a lower (that will be split)
   for (m = lcl->level = 0; lcl->level <= cur_max_level; lcl->level++) {
      // can not use sorted list here since this routine changes the list
      for (n = 0; n < max_active_block; n++)
         if (blocks[n].number >= 0 && blocks[n].level == lcl->level) {
            bp = &blocks[n];
            cp = &cells[n];
            if (bp->refine == 1) {
               nl = bp->number - block_start[lcl->level];
               zp = nl/((p2[lcl->level]*npx*init_block_x)*
                        (p2[lcl->level]*npy*init_block_y));
               yp = (nl%((p2[lcl->level]*npx*init_block_x)*
                         (p2[lcl->level]*npy*init_block_y)))/
                    (p2[lcl->level]*npx*init_block_x);
               xp = nl%(p2[lcl->level]*npx*init_block_x);
               if ((num_active + 8) > max_num_blocks) {
                  printf("ERROR: Need more blocks %d %d on %d\n", num_active, max_num_blocks);
                  exit(-1);
               }
//printf ("In %s at line %d on PE %d, calling del_sorted_list with number=%d level=%d\n", __FILE__, __LINE__, my_pe, bp->number, lcl->level);
               del_sorted_list(glbl, bp->number, lcl->level);
//printf ("PE %d back successfully from del_sorted_list\n", my_pe);
               num_active += 7;
               local_num_blocks[lcl->level]--;
               local_num_blocks[lcl->level+1] += 8;
               for (p = 0; p < max_active_parent; p++)
                  if (parents[p].number < 0)
                     break;
               if (p == max_num_parents) {
                  printf("ERROR: Need more parents\n");
                  exit(-1);
               }
               if (p == max_active_parent)
                  max_active_parent++;
               num_parents++;
               num_refined++;
               pp = &parents[p];
               pp->number = bp->number;
               pp->level = bp->level;
               pp->parent = bp->parent;
               pp->parent_node = bp->parent_node;
               pp->child_number = bp->child_number;
               if (pp->parent_node == my_pe) {
                  if (pp->parent >= 0)
                     parents[pp->parent].child[pp->child_number] = -1 - p;
               } else if (pp->parent < -1) {
                  del_par_list(glbl, &par_b, (-2-bp->parent), n, bp->child_number,
                               bp->parent_node);
                  CALL_SUSPENDABLE_CALLEE(1)
                  add_par_list(glbl, &par_b, (-2-bp->parent), (-1-p),
                               bp->child_number, bp->parent_node, 0);
                  DEBRIEF_SUSPENDABLE_FUNCTION(;)
               }
               pp->refine = 0;
               pp->cen[0] = bp->cen[0];
               pp->cen[1] = bp->cen[1];
               pp->cen[2] = bp->cen[2];

               // Define the 8 children
               for (o = 0; o < 8; o++) {
                  for ( ; m < max_num_blocks; m++)
                     if (blocks[m].number < 0)
                        break;
                  if (m == max_num_blocks) {
                     printf("Error: No inactive blocks available %d %d %d\n", m, num_active, max_num_blocks);
                     exit(-1);
                  }
                  if ((m+1) > max_active_block)
                     max_active_block = m+1;
                  bp1 = &blocks[m];
                  cp1 = &cells[m];
                  sib[o] = m;
                  pp->child[o] = m;
                  pp->child_node[o] = my_pe;
                  bp1->refine = 0;
                  bp1->level = lcl->level + 1;
                  bp1->parent = p;
                  bp1->parent_node = my_pe;
                  bp1->child_number = o;
                  i1 = (o%2);
                  j1 = ((o/2)%2);
                  k1 = (o/4);
                  bp1->number = ((2*zp+k1)*(p2[lcl->level+1]*npy*init_block_y) +
                                 (2*yp+j1))*(p2[lcl->level+1]*npx*init_block_x) +
                                2*xp + i1 + block_start[lcl->level+1];
                  add_sorted_list(glbl, m, bp1->number, (lcl->level+1));
                  bp1->cen[0] = bp->cen[0] +
                                (2*i1 - 1)*p2[num_refine - lcl->level - 1];
                  bp1->cen[1] = bp->cen[1] +
                                (2*j1 - 1)*p2[num_refine - lcl->level - 1];
                  bp1->cen[2] = bp->cen[2] +
                                (2*k1 - 1)*p2[num_refine - lcl->level - 1];
                  half_size = p2[num_refine - lcl->level - 1];
                  i1 *= x_block_half;
                  j1 *= y_block_half;
                  k1 *= z_block_half;
                  for (v = 0; v < num_vars; v++)
                     for (i2 = i = 1; i <= x_block_half; i++, i2+=2)
                        for (j2 = j = 1; j <= y_block_half; j++, j2+=2)
                           for (k2 = k = 1; k <= z_block_half; k++, k2+=2)
                              (*cp1)[v][i2  ][j2  ][k2  ] =
                              (*cp1)[v][i2+1][j2  ][k2  ] =
                              (*cp1)[v][i2  ][j2+1][k2  ] =
                              (*cp1)[v][i2+1][j2+1][k2  ] =
                              (*cp1)[v][i2  ][j2  ][k2+1] =
                              (*cp1)[v][i2+1][j2  ][k2+1] =
                              (*cp1)[v][i2  ][j2+1][k2+1] =
                              (*cp1)[v][i2+1][j2+1][k2+1] =
                                    (*cp)[v][i+i1][j+j1][k+k1]/8.0;
               }

               // children all defined - connect children & disconnect parent
               for (c = 0; c < 6; c++) {
                  // deal with internal connections amoung 8 siblings
                  for (i = 0; i < 2; i++)
                     for (j = 0; j < 2; j++) {
                        blocks[sib[side[c][i][j]+off[c]]].nei_level[c] =
                              lcl->level + 1;
                        blocks[sib[side[c][i][j]+off[c]]].nei[c][0][0] =
                              sib[side[c][i][j]];
                     }
                  // deal with external connections
                  if (bp->nei_level[c] == -2)  // external boundary
                     for (i = 0; i < 2; i++)
                        for (j = 0; j < 2; j++) {
                           blocks[sib[side[c][i][j]]].nei_level[c] = -2;
                           blocks[sib[side[c][i][j]]].nei[c][0][0] = 0;
                        }
                  else if (bp->nei_level[c] == lcl->level-1) // level less parent
                     if (bp->nei[c][0][0] >= 0) { // error
                        printf("%d ERROR: internal misconnect block %d c %d\n",
                               my_pe, bp->number, c);
                        exit(-1);
                     } else {
                        if (bp->nei_refine[c] == 1) {
                           dir = c/2;
                           fcase = (c%2)*10;
                           pe = -1 - bp->nei[c][0][0];
                           d = 2*(c%2) - 1;
                           for (k = fcase+2, i = 0; i < 2; i++)
                              for (j = 0; j < 2; j++, k++) {
                                 bp1 = &blocks[sib[side[c][i][j]]];
                                 CALL_SUSPENDABLE_CALLEE(1)
                                 add_comm_list(glbl, dir, sib[side[c][i][j]], pe, k,
                                (bp1->cen[mul[dir][1]]*mesh_size[mul[dir][0]] +
                                  bp1->cen[mul[dir][0]]),
                                (bp1->cen[mul[dir][2]] + d*half_size));
                                 DEBRIEF_SUSPENDABLE_FUNCTION(;)
                                 bp1->nei_level[c] = lcl->level;
                                 bp1->nei[c][0][0] = bp->nei[c][0][0];
                                 bp1->nei_refine[c] = 0;
                              }
                           k = -1 - fcase;
                           del_comm_list(glbl, dir, n, pe, k);
                        } else {
                           printf("%d ERROR: connected block unrefined %d dir %d\n",
                                  my_pe, bp->number, c);
                           exit(-1);
                        }
                     }
                  else if (bp->nei_level[c] == lcl->level) // same level as parent
                     if (bp->nei[c][0][0] >= 0) {
                        other = bp->nei[c][0][0];
                        c1 = (c/2)*2 + (c+1)%2;
                        blocks[other].nei_level[c1] = lcl->level + 1;
                        for (i = 0; i < 2; i++)
                           for (j = 0; j < 2; j++) {
                              bp1 = &blocks[sib[side[c][i][j]]];
                              bp1->nei_level[c] = lcl->level;
                              bp1->nei[c][0][0] = other;
                              blocks[other].nei[c1][i][j] = sib[side[c][i][j]];
                           }
                     } else {
                        dir = c/2;
                        fcase = (c%2)*10;
                        pe = -1 - bp->nei[c][0][0];
                        d = 2*(c%2) - 1;
                        if (bp->nei_refine[c] == 1) {
                           for (i = 0; i < 2; i++)
                              for (j = 0; j < 2; j++) {
                                 bp1 = &blocks[sib[side[c][i][j]]];
                                 CALL_SUSPENDABLE_CALLEE(1)
                                 add_comm_list(glbl, dir, sib[side[c][i][j]], pe,
                                               fcase+f,
                                (bp1->cen[mul[dir][1]]*mesh_size[mul[dir][0]] +
                                  bp1->cen[mul[dir][0]]),
                                (bp1->cen[mul[dir][2]] + d*half_size));
                                 DEBRIEF_SUSPENDABLE_FUNCTION(;)
                                 bp1->nei_level[c] = lcl->level + 1;
                                 bp1->nei[c][0][0] = bp->nei[c][0][0];
                                 bp1->nei_refine[c] = 0;
                              }
                           k = fcase+f;
                           del_comm_list(glbl, dir, n, pe, k);
                        } else {
                           for (k = fcase+2, i = 0; i < 2; i++)
                              for (j = 0; j < 2; j++, k++) {
                                 bp1 = &blocks[sib[side[c][i][j]]];
                                 CALL_SUSPENDABLE_CALLEE(1)
                                 add_comm_list(glbl, dir, sib[side[c][i][j]], pe, k,
                                (bp1->cen[mul[dir][1]]*mesh_size[mul[dir][0]] +
                                  bp1->cen[mul[dir][0]]),
                                (bp1->cen[mul[dir][2]] + d*half_size));
                                 DEBRIEF_SUSPENDABLE_FUNCTION(;)
                                 bp1->nei_level[c] = lcl->level;
                                 bp1->nei[c][0][0] = bp->nei[c][0][0];
                                 bp1->nei_refine[c] = 0;
                              }
                           del_comm_list(glbl, dir, n, pe, fcase+f);
                        }
                     }
                  else if (bp->nei_level[c] == lcl->level+1) { // same level as child
                     dir = c/2;
                     fcase = (c%2)*10;
                     c1 = (c/2)*2 + (c+1)%2;
                     d = 2*(c%2) - 1;
                     for (k = fcase+6, i = 0; i < 2; i++)
                        for (j = 0; j < 2; j++, k++)
                           if (bp->nei[c][i][j] >= 0) {
                              other = bp->nei[c][i][j];
                              bp1 = &blocks[sib[side[c][i][j]]];
                              bp1->nei_level[c] = lcl->level+1;
                              bp1->nei[c][0][0] = other;
                              blocks[other].nei_level[c1] = lcl->level + 1;
                              blocks[other].nei[c1][0][0] = sib[side[c][i][j]];
                           } else {
                              pe = -1 - bp->nei[c][i][j];
                              bp1 = &blocks[sib[side[c][i][j]]];
                              CALL_SUSPENDABLE_CALLEE(1)
                              add_comm_list(glbl, dir, sib[side[c][i][j]], pe,
                                               fcase+f,
                                (bp1->cen[mul[dir][1]]*mesh_size[mul[dir][0]] +
                                  bp1->cen[mul[dir][0]]),
                                    (bp1->cen[mul[dir][2]] + d*half_size));
                              DEBRIEF_SUSPENDABLE_FUNCTION(;)
                              bp1->nei_level[c] = lcl->level + 1;
                              bp1->nei[c][0][0] = bp->nei[c][i][j];
                              bp1->nei_refine[c] = 0;
                              del_comm_list(glbl, dir, n, pe, k);
                           }
                  } else {
                     printf("%d ERROR: misconnected b %d %d l %d nei[%d] %d\n",
                            my_pe, n, bp->number, lcl->level, c, bp->nei_level[c]);
                     exit(-1);
                  }
               }
               /* children all defined and connected - inactivate block */
               bp->number = -1;
               if (n < m)
                  m = n;
            } else { /* block not being refined - check off node connections */
               for (c = 0; c < 6; c++)
                  // different core and being refined
                  if (bp->nei[c][0][0] < 0 && bp->nei_refine[c] == 1) {
                     dir = c/2;
                     fcase = (c%2)*10;
                     pe = -1 - bp->nei[c][0][0];
                     d = 2*(c%2) - 1;
                     if (bp->nei_level[c] == lcl->level) {  // same level
                        offset = p2[num_refine - lcl->level - 1];
                        for (k = fcase+6, i = 0; i < 2; i++)
                           for (j = 0; j < 2; j++, k++) {
                              bp->nei[c][i][j] = bp->nei[c][0][0];
                              CALL_SUSPENDABLE_CALLEE(1)
                              add_comm_list(glbl, dir, n, pe, k,
               ((bp->cen[mul[dir][1]]+(2*i-1)*offset)*mesh_size[mul[dir][0]] +
                bp->cen[mul[dir][0]]+(2*j-1)*offset),
                              (bp->cen[mul[dir][2]] + d*p2[num_refine-lcl->level]));
                              DEBRIEF_SUSPENDABLE_FUNCTION(;)
                           }
                        bp->nei_level[c] = lcl->level + 1;
                        del_comm_list(glbl, dir, n, pe, fcase+f);
                     } else if (bp->nei_level[c] == lcl->level-1) {
                        CALL_SUSPENDABLE_CALLEE(1)
                        add_comm_list(glbl, dir, n, pe, fcase+f,
                                (bp->cen[mul[dir][1]]*mesh_size[mul[dir][0]] +
                                  bp->cen[mul[dir][0]]),
                              (bp->cen[mul[dir][2]] + d*p2[num_refine-lcl->level]));
                        DEBRIEF_SUSPENDABLE_FUNCTION(;)
                        bp->nei_level[c] = lcl->level;
                        k = -1 - fcase;
                        del_comm_list(glbl, dir, n, pe, k);
                     }
                     bp->nei_refine[c] = 0;
                  }
            }
         }
      CALL_SUSPENDABLE_CALLEE(1)
      comm_refine(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
   }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  k
#undef  m
#undef  n
#undef  o
#undef  v
#undef  nl
#undef  xp
#undef  yp
#undef  zp
#undef  c
#undef  c1
#undef  other
#undef  i1
#undef  i2
#undef  j1
#undef  j2
#undef  k1
#undef  k2
#undef  dir
#undef  fcase
#undef  pe
#undef  f
#undef  p
#undef  level
#undef  sib
#undef  offset
#undef  d
#undef  half_size
#undef  pp
#undef  bp
#undef  bp1
#undef  cp
#undef  cp1
}

// This routine takes blocks that are to be coarsened and recombines them.
// Before this routine can be called, all of the child blocks need to be on
// the same processor as the parent.  A new block is created and the parent
// and child blocks are inactivated.
void consolidate_blocks(Globals_t * const glbl)
{
   static int side[6][2][2] = { { {0, 2}, {4, 6} }, { {1, 3}, {5, 7} },
                                { {0, 1}, {4, 5} }, { {2, 3}, {6, 7} },
                                { {0, 1}, {2, 3} }, { {4, 5}, {6, 7} } };
   static int mul[3][3] = { {1, 2, 0}, {0, 2, 1}, {0, 1, 2} };

   OBTAIN_ACCESS_TO_parents
   OBTAIN_ACCESS_TO_grid_sum
   OBTAIN_ACCESS_TO_sorted_list
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_cells

   typedef struct {
      Frame_Header_t myFrame;
      int n, p, i, j, k, i1, j1, k1, i2, j2, k2, level, o, v, f, c, offset,
          other, c1, dir, fcase, pe, nl, pos[3], d, in;
      struct {
         Parent_t *pp;
         Block_t *bp, *bp1;
         Cell_t  *cp, *cp1;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__consolidate_blocks_t;

// #define n                 (lcl->n)      // Name overload.  ("n" is used as an element of a struct elsewhere.)  Just use fully-qualified name in code below.
#define p                 (lcl->p)
#define i                 (lcl->i)
#define j                 (lcl->j)
#define k                 (lcl->k)
#define i1                (lcl->i1)
#define j1                (lcl->j1)
#define k1                (lcl->k1)
#define i2                (lcl->i2)
#define j2                (lcl->j2)
#define k2                (lcl->k2)
// #define level             (lcl->level)  // Name overload.  ("level" is used as an element of a struct elsewhere.)  Just use fully-qualified name in code below.
#define o                 (lcl->o)
#define v                 (lcl->v)
#define f                 (lcl->f)
#define c                 (lcl->c)
#define offset            (lcl->offset)
#define other             (lcl->other)
#define c1                (lcl->c1)
#define dir               (lcl->dir)
#define fcase             (lcl->fcase)
#define pe                (lcl->pe)
#define nl                (lcl->nl)
#define pos               (lcl->pos)
#define d                 (lcl->d)
#define in                (lcl->in)
#define pp                (lcl->pointers.pp)
#define bp                (lcl->pointers.bp)
#define bp1               (lcl->pointers.bp1)
#define cp                (lcl->pointers.cp)
#define cp1               (lcl->pointers.cp1)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__consolidate_blocks_t)
   TRACE

   if (stencil == 7)  // add to face case when diags are needed
      f = 0;
   else
      f = 1;
   // assume that blocks were moved back to node with parent
   for (lcl->level = cur_max_level; lcl->level >= 0; lcl->level--)
      for (p = 0; p < max_active_parent; p++)
         if ((pp = &parents[p])->number >= 0 && pp->level == lcl->level &&
             pp->refine == -1) {
            for (lcl->n = 0; lcl->n < max_num_blocks; lcl->n++)
               if (blocks[lcl->n].number < 0)  // found inactive block
                  break;
            if (lcl->n == max_num_blocks) {
               printf("Out of free blocks in consolidate_blocks %d\n", my_pe);
               exit(-1);
            }
            bp = &blocks[lcl->n];
            cp = &cells[lcl->n];
            if ((lcl->n+1) > max_active_block)
               max_active_block = lcl->n+1;
            num_active -= 7;
            num_reformed++;
            local_num_blocks[lcl->level]++;
            local_num_blocks[lcl->level+1] -= 8;
            bp->number = pp->number;
            pp->number = -1;
            bp->level = pp->level;
            bp->parent = pp->parent;
            bp->parent_node = pp->parent_node;
            bp->child_number = pp->child_number;
            if (bp->level)
               if (bp->parent_node == my_pe) {
if (bp->parent < 0) printf ("At %s:%d PE=%d Wayward write!  bp->parent = %d  Effectively worked around if merely -1, though algorithmic ramifications are unknown\n",
__FILE__, __LINE__, my_pe, bp->parent);
                  parents[bp->parent].child[bp->child_number] = lcl->n;
               } else if (pp->parent < -1) { // else communicate this change later
                  del_par_list(glbl, &par_b, (-2-bp->parent), (-1-p), bp->child_number,
                               bp->parent_node);
                  CALL_SUSPENDABLE_CALLEE(1)
                  add_par_list(glbl, &par_b, (-2-bp->parent), lcl->n, bp->child_number,
                               bp->parent_node, 0);
                  DEBRIEF_SUSPENDABLE_FUNCTION(;)
               }
            add_sorted_list(glbl, lcl->n, bp->number, lcl->level);
            bp->refine = 0;
            bp->cen[0] = pp->cen[0];
            bp->cen[1] = pp->cen[1];
            bp->cen[2] = pp->cen[2];
            // Copy child arrays back to new block.
            for (o = 0; o < 8; o++) {
               bp1 = &blocks[pp->child[o]];
               cp1 = &cells[pp->child[o]];
//printf ("In %s at line %d on PE %d, calling del_sorted_list with number=%d level=%d\n", __FILE__, __LINE__, my_pe, bp1->number, lcl->level+1);
               del_sorted_list(glbl, bp1->number, (lcl->level+1));
//printf ("PE %d back successfully from del_sorted_list\n", my_pe);
               bp1->number = -1;
               i1 = (o%2)*x_block_half;
               j1 = ((o/2)%2)*y_block_half;
               k1 = (o/4)*z_block_half;
               for (v = 0; v < num_vars; v++)
                  for (i2 = i = 1; i <= x_block_half; i++, i2+=2)
                     for (j2 = j = 1; j <= y_block_half; j++, j2+=2)
                        for (k2 = k = 1; k <= z_block_half; k++, k2+=2)
                           (*cp)[v][i+i1][j+j1][k+k1] =
                                 (*cp1)[v][i2  ][j2  ][k2  ] +
                                 (*cp1)[v][i2+1][j2  ][k2  ] +
                                 (*cp1)[v][i2  ][j2+1][k2  ] +
                                 (*cp1)[v][i2+1][j2+1][k2  ] +
                                 (*cp1)[v][i2  ][j2  ][k2+1] +
                                 (*cp1)[v][i2+1][j2  ][k2+1] +
                                 (*cp1)[v][i2  ][j2+1][k2+1] +
                                 (*cp1)[v][i2+1][j2+1][k2+1];
            }
            // now figure out communication
            for (c = 0; c < 6; c++) {
               other = pp->child[side[c][0][0]]; // first child on this side
               // four options - boundary, level of parent, level of children,
               // and level of children + 1 (that are offnode and unrefining)
               if (blocks[other].nei_level[c] == -2) {
                  // external boundary (only need to check one child)
                  bp->nei_level[c] = -2;
                  bp->nei_refine[c] = 0;
               } else if (blocks[other].nei_level[c] == lcl->level)
                  // same level as parent
                  if (blocks[other].nei[c][0][0] >= 0) {
                     // on node - if it gets consolidated later, it will fix
                     // the connections at that point
                     c1 = (c/2)*2 + (c+1)%2;
                     bp->nei[c][0][0] = blocks[other].nei[c][0][0];
                     bp->nei_level[c] = lcl->level;
                     bp->nei_refine[c] = 0;
                     blocks[blocks[other].nei[c][0][0]].nei[c1][0][0] = lcl->n;
                     blocks[blocks[other].nei[c][0][0]].nei_level[c1] = lcl->level;
                     blocks[blocks[other].nei[c][0][0]].nei_refine[c1] = 0;
                  } else {
                     // off node - either not unrefining or
                     //            unrefining, but will all be on same node
                     dir = c/2;
                     fcase = (c%2)*10;
                     pe = -1 - blocks[other].nei[c][0][0];
                     for (k = fcase+2, i = 0; i < 2; i++)
                        for (j = 0; j < 2; j++, k++)
                           del_comm_list(glbl, dir, pp->child[side[c][i][j]], pe, k);
                     bp->nei[c][0][0] = -1 - pe;
                     bp->nei_refine[c] = 0;
                     d = 2*(c%2) - 1;
                     if (blocks[other].nei_refine[c] == 0) {
                        bp->nei_level[c] = lcl->level;
                        k = fcase + f;
                        CALL_SUSPENDABLE_CALLEE(1)
                        add_comm_list(glbl, dir, lcl->n, pe, k, (bp->cen[mul[dir][1]]*
                              mesh_size[mul[dir][0]] + bp->cen[mul[dir][0]]),
                              (bp->cen[mul[dir][2]] + d*p2[num_refine-lcl->level]));
                        DEBRIEF_SUSPENDABLE_FUNCTION(;)
                     } else {
                        bp->nei_level[c] = lcl->level - 1;
                        nl = bp->number - block_start[lcl->level];
                        pos[2] = nl/((p2[lcl->level]*npx*init_block_x)*
                                     (p2[lcl->level]*npy*init_block_y));
                        pos[1] = (nl%((p2[lcl->level]*npx*init_block_x)*
                                      (p2[lcl->level]*npy*init_block_y)))/
                                 (p2[lcl->level]*npx*init_block_x);
                        pos[0] = nl%(p2[lcl->level]*npx*init_block_x);
                        k = fcase + 2 + pos[mul[dir][1]]%2 +
                                     2*(pos[mul[dir][0]]%2);
                        CALL_SUSPENDABLE_CALLEE(0)
                        add_comm_list(glbl, dir, lcl->n, pe, k, (bp->cen[mul[dir][1]]*
                              mesh_size[mul[dir][0]] + bp->cen[mul[dir][0]]),
                              (bp->cen[mul[dir][2]] + d*p2[num_refine-lcl->level]));
                        DEBRIEF_SUSPENDABLE_FUNCTION(;)
                     }
                  }
               else {
                  dir = c/2;
                  fcase = (c%2)*10;
                  offset = p2[num_refine - lcl->level - 1];
                  for (k = fcase+6, i = 0; i < 2; i++)
                     for (j = 0; j < 2; j++, k++) {
                        other = pp->child[side[c][i][j]];
                        if (blocks[other].nei[c][0][0] >= 0) {
                           if (blocks[other].nei_level[c] == lcl->level+2) {
                              printf("%d ERROR: %d con %d block %d c %d wrong level %d\n",
                                     my_pe, p, lcl->n, other, c, lcl->level);
                              exit(-1);
                           }
                           c1 = (c/2)*2 + (c+1)%2;
                           bp->nei[c][i][j] = blocks[other].nei[c][0][0];
                           bp->nei_level[c] = lcl->level + 1;
                           bp->nei_refine[c] = 0;
                           blocks[blocks[other].nei[c][0][0]].nei[c1][0][0] =
                                 lcl->n;
                           blocks[blocks[other].nei[c][0][0]].nei_level[c1] =
                                 lcl->level;
                           blocks[blocks[other].nei[c][0][0]].nei_refine[c1] =
                                 0;
                        } else {
                           if (blocks[other].nei_level[c] == lcl->level+2 &&
                                blocks[other].nei_refine[c] == -1) {
                              // being refined to level+1 on one proc
                              pe = -1 - blocks[other].nei[c][0][0];
                              for (k1 = fcase+6, i1 = 0; i1 < 2; i1++)
                                 for (j1 = 0; j1 < 2; j1++)
                                    del_comm_list(glbl, dir, other, pe, k1);
                              bp->nei[c][i][j] = -1 - pe;
                              bp->nei_refine[c] = 0;
                              bp->nei_level[c] = lcl->level + 1;
                              d = 2*(c%2) - 1;
                              CALL_SUSPENDABLE_CALLEE(0)
                              add_comm_list(glbl, dir, lcl->n, pe, k,
                                ((bp->cen[mul[dir][1]]+(2*i-1)*offset)*mesh_size[mul[dir][0]]
                                + bp->cen[mul[dir][0]]+(2*j-1)*offset),
                                (bp->cen[mul[dir][2]] + d*p2[num_refine-lcl->level]));
                              DEBRIEF_SUSPENDABLE_FUNCTION(;)
                           } else if (blocks[other].nei_level[c] == lcl->level+1) {
                              if (blocks[other].nei_refine[c] == 0) {
                                 pe = -1 - blocks[other].nei[c][0][0];
                                 del_comm_list(glbl, dir, other, pe, fcase+f);
                                 bp->nei[c][i][j] = -1 - pe;
                                 bp->nei_refine[c] = 0;
                                 bp->nei_level[c] = lcl->level + 1;
                                 d = 2*(c%2) - 1;
                                 CALL_SUSPENDABLE_CALLEE(1)
                                 add_comm_list(glbl, dir, lcl->n, pe, k,
                                   ((bp->cen[mul[dir][1]]+(2*i-1)*offset)*mesh_size[mul[dir][0]]
                                   + bp->cen[mul[dir][0]]+(2*j-1)*offset),
                                   (bp->cen[mul[dir][2]] + d*p2[num_refine-lcl->level]));
                                 DEBRIEF_SUSPENDABLE_FUNCTION(;)
                              } else {
                                 pe = -1 - blocks[other].nei[c][0][0];
                                 del_comm_list(glbl, dir, pp->child[side[c][i][j]],
                                               pe, fcase+f);
                                 bp->nei[c][0][0] = -1 - pe;
                                 bp->nei_refine[c] = 0;
                                 bp->nei_level[c] = lcl->level;
                                 d = 2*(c%2) - 1;
                                 if (i == 0 && j == 0) {
                                    CALL_SUSPENDABLE_CALLEE(1)
                                    add_comm_list(glbl, dir, lcl->n, pe, fcase+f,
                                                  (bp->cen[mul[dir][1]]*
                                                  mesh_size[mul[dir][0]] +
                                                  bp->cen[mul[dir][0]]),
                                                  (bp->cen[mul[dir][2]] +
                                                  d*p2[num_refine-lcl->level]));
                                    DEBRIEF_SUSPENDABLE_FUNCTION(;)
                                 }
                              }
                           } else {
                              printf("%d ERROR: misconnected con b %d %d l %d nei[%d] %d other %d %d ol %d\n",
                                     my_pe, lcl->n, bp->number, lcl->level, c,
                                     bp->nei_level[c], other,
                                     blocks[other].number,
                                     blocks[other].nei_level[c]);
                              exit(-1);
                           }
                        }
                     }
               }
            }
         }

   // Go through blocks and check off node connections that are unrefining
   for (in = 0; in < sorted_index[num_refine+1]; in++) {
      lcl->n = sorted_list[in].n;
      if ((bp = &blocks[lcl->n])->number >= 0)
         for (c = 0; c < 6; c++)
            if (bp->nei[c][0][0] < 0 && bp->nei_refine[c] == -1) {
               lcl->level = bp->level;
               dir = c/2;
               fcase = (c%2)*10;
               pe = -1 - bp->nei[c][0][0];
               if (bp->nei_level[c] == lcl->level) {
                  // go from "whole to whole" to "whole to quarter"
                  bp->nei_level[c] = lcl->level - 1;
                  del_comm_list(glbl, dir, lcl->n, pe, fcase+f);
                  nl = bp->number - block_start[lcl->level];
                  pos[2] = nl/((p2[lcl->level]*npx*init_block_x)*
                               (p2[lcl->level]*npy*init_block_y));
                  pos[1] = (nl%((p2[lcl->level]*npx*init_block_x)*
                                (p2[lcl->level]*npy*init_block_y)))/
                           (p2[lcl->level]*npx*init_block_x);
                  pos[0] = nl%(p2[lcl->level]*npx*init_block_x);
                  k = fcase + 2 + pos[mul[dir][0]]%2 + 2*(pos[mul[dir][1]]%2);
                  d = 2*(c%2) - 1;
                  CALL_SUSPENDABLE_CALLEE(1)
                  add_comm_list(glbl, dir, lcl->n, pe, k, (bp->cen[mul[dir][1]]*
                        mesh_size[mul[dir][0]] + bp->cen[mul[dir][0]]),
                        (bp->cen[mul[dir][2]] + d*p2[num_refine-lcl->level]));
                  DEBRIEF_SUSPENDABLE_FUNCTION(;)
               } else if (bp->nei_level[c] == lcl->level+1) {
                  // go from "quarter to whole" to "whole to whole"
                  for (k = fcase+6, i = 0; i < 2; i++)
                     for (j = 0; j < 2; j++, k++)
                        del_comm_list(glbl, dir, lcl->n, pe, k);
                  d = 2*(c%2) - 1;
                  CALL_SUSPENDABLE_CALLEE(1)
                  add_comm_list(glbl, dir, lcl->n, pe, fcase+f, (bp->cen[mul[dir][1]]*
                        mesh_size[mul[dir][0]] + bp->cen[mul[dir][0]]),
                        (bp->cen[mul[dir][2]] + d*p2[num_refine-lcl->level]));
                  DEBRIEF_SUSPENDABLE_FUNCTION(;)
                  bp->nei_level[c] = lcl->level;
               } else {
                  printf("%d ERROR: con nei block %d pe %d bad b %d %d l %d %d\n",
                         my_pe, c, pe, lcl->n, bp->number, lcl->level, bp->nei_level[c]);
                  exit(-1);
               }
               bp->nei_refine[c] = 0;
            }
   }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  n
#undef  p
#undef  i
#undef  j
#undef  k
#undef  i1
#undef  j1
#undef  k1
#undef  i2
#undef  j2
#undef  k2
#undef  level
#undef  o
#undef  v
#undef  f
#undef  c
#undef  offset
#undef  other
#undef  c1
#undef  dir
#undef  fcase
#undef  pe
#undef  nl
#undef  pos
#undef  d
#undef  in
#undef  pp
#undef  bp
#undef  bp1
#undef  cp
#undef  cp1
}

void add_sorted_list(Globals_t * const glbl, int n, int number, int level)
{
   int i, j;
   OBTAIN_ACCESS_TO_sorted_list
   TRACE

   for (i = sorted_index[level]; i < sorted_index[level+1]; i++)
      if (number > sorted_list[i].number)
         break;
   for (j = sorted_index[num_refine+1]; j > i; j--) {
      sorted_list[j].number = sorted_list[j-1].number;
      sorted_list[j].n      = sorted_list[j-1].n;
   }
   sorted_list[i].number = number;
   sorted_list[i].n      = n;
   for (i = level+1; i <= (num_refine+1); i++)
      sorted_index[i]++;
}

void del_sorted_list(Globals_t * const glbl, int number, int level)
{
   int i, j;
   OBTAIN_ACCESS_TO_sorted_list
   TRACE

   for (i = sorted_index[level]; i < sorted_index[level+1]; i++)
      if (number == sorted_list[i].number)
         break;
   if (number != sorted_list[i].number) {
      printf("ERROR: del_sorted_list on %d - number %d not found\n",
             my_pe, number);
      exit(-1);
   }
   for (j = level+1; j <= (num_refine+1); j++)
      sorted_index[j]--;
   for (j = i; j < sorted_index[num_refine+1]; j++) {
      sorted_list[j].number = sorted_list[j+1].number;
      sorted_list[j].n      = sorted_list[j+1].n;
   }
}

int find_sorted_list(Globals_t * const glbl, int number, int level)
{
   int i;
   OBTAIN_ACCESS_TO_sorted_list

   for (i = sorted_index[level]; i < sorted_index[level+1]; i++)
      if (number == sorted_list[i].number)
         return sorted_list[i].n;
   printf("ERROR: find_sorted_list on %d - number %d not found\n",
          my_pe, number);
   exit(-1);
}
