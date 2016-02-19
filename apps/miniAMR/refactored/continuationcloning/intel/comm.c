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

// The routines in this file are used in the communication of ghost values
// between blocks, both on processor and off processor.

// Main communication routine that sends and recieves ghost values between
// blocks on different processors and directos blocks on the same processor
// to exchange their ghost values.
void comm(Globals_t * const glbl, int const start, int const num_comm, int const stage)
{
   OBTAIN_ACCESS_TO_sorted_list
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_cells

   int permutations[6][3] = { {0, 1, 2}, {1, 2, 0}, {2, 0, 1},
                              {0, 2, 1}, {1, 0, 2}, {2, 1, 0} };

   typedef struct {
      Frame_Header_t myFrame;
      int i, j, k, l, m, n, dir, o, in, which, offset, type;
      double t1, t2, t3, t4;
      MPI_Status status;
      struct {
         Block_t *bp;
         Cell_t  *cp;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__comm_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define k                 (lcl->k)
#define l                 (lcl->l)
#define m                 (lcl->m)
// #define n                 (lcl->n)  // Overloaded.  Use fully-qualified symbol at point of reference.
#define dir               (lcl->dir)
#define o                 (lcl->o)
#define in                (lcl->in)
#define which             (lcl->which)
#define offset            (lcl->offset)
#define type              (lcl->type)
#define t1                (lcl->t1)
#define t2                (lcl->t2)
#define t3                (lcl->t3)
#define t4                (lcl->t4)
#define status            (lcl->status)
#define bp                (lcl->pointers.bp)
#define cp                (lcl->pointers.cp)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__comm_t)
   TRACE

   for (o = 0; o < 3; o++) {
      if (permute)
         dir = permutations[stage%6][o];
      else
         dir = o;
      type = dir;
      t1 = timer();
      for (i = 0; i < num_comm_partners[dir]; i++) {
         gasket__mpi_Irecv(glbl, &recv_buff[comm_recv_off[dir][comm_index[dir][i]]],
                   recv_size[dir][i], MPI_DOUBLE,
                   comm_partner[dir][i], type, MPI_COMM_WORLD, -9999, &request[i], __FILE__, __LINE__);
         counter_halo_recv[dir]++;
         size_mesg_recv[dir] += (double) recv_size[dir][i]*sizeof(double);
      }
      timer_comm_recv[dir] += timer() - t1;

/**** the send and recv list can be same if kept ordered (length can be diff)
        would need to expand face case so that:     **** done ****
        0   whole -> whole
        1   whole -> whole   w/ 27 point stencil (send corners)
        2-5 whole -> quarter w/ number indicating which quarter (matters recv)
        6-9 quarter -> whole w/ number indicating which quarter (matters send)
        + 10 for E, N, U
**** one large send buffer -- can pack and send for a neighbor and reuse */
      for (i = 0; i < num_comm_partners[dir]; i++) {
         t2 = timer();
         for (lcl->n = 0; lcl->n < comm_num[dir][i]; lcl->n++) {
            offset = comm_send_off[dir][comm_index[dir][i]+lcl->n];
            if (!nonblocking)
               offset -= comm_send_off[dir][comm_index[dir][i]];
            pack_face(glbl, &send_buff[offset], comm_block[dir][comm_index[dir][i]+lcl->n],
                      comm_face_case[dir][comm_index[dir][i]+lcl->n], dir,
                      start, num_comm);
         }
         counter_face_send[dir] += comm_num[dir][i];
         t3 = timer();
         if (nonblocking) {
            CALL_SUSPENDABLE_CALLEE(1)
            gasket__mpi_Isend(glbl, &send_buff[comm_send_off[dir][comm_index[dir][i]]],
                      send_size[dir][i], MPI_DOUBLE, comm_partner[dir][i],
                      type, MPI_COMM_WORLD, -9999, &s_req[i], __FILE__, __LINE__);
            DEBRIEF_SUSPENDABLE_FUNCTION(;)
         } else {
            CALL_SUSPENDABLE_CALLEE(0)
            gasket__mpi_Send(glbl, send_buff, send_size[dir][i], MPI_DOUBLE,
                     comm_partner[dir][i], type, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
            DEBRIEF_SUSPENDABLE_FUNCTION(;)
         }
         counter_halo_send[dir]++;
         size_mesg_send[dir] += (double) send_size[dir][i]*sizeof(double);
         t4 = timer();
         timer_comm_pack[dir] += t3 - t2;
         timer_comm_send[dir] += t4 - t3;
      }

      // While values are being sent over the mesh, go through and direct
      // blocks to exchange ghost values with other blocks that are on
      // processor.  Also apply boundary conditions for boundary of domain.
      for (in = 0; in < sorted_index[num_refine+1]; in++) {
         lcl->n = sorted_list[in].n;
         bp = &blocks[lcl->n];
         cp = &cells[lcl->n];
         if (bp->number >= 0)
            for (l = dir*2; l < (dir*2 + 2); l++) {
               if (bp->nei_level[l] == bp->level) {
                  t2 = timer();
                  if ((m = bp->nei[l][0][0]) > lcl->n) {
                     on_proc_comm(glbl, lcl->n, m, l, start, num_comm);
                     counter_same[dir] += 2;
                  }
                  timer_comm_same[dir] += timer() - t2;
               } else if (bp->nei_level[l] == (bp->level+1)) {
                  t2 = timer();
                  for (i = 0; i < 2; i++)
                     for (j = 0; j < 2; j++)
                        if ((m = bp->nei[l][i][j]) > lcl->n) {
                           on_proc_comm_diff(glbl, lcl->n, m, l, i, j, start, num_comm);
                           counter_diff[dir] += 2;
                        }
                  timer_comm_diff[dir] += timer() - t2;
               } else if (bp->nei_level[l] == (bp->level-1)) {
                  t2 = timer();
                  if ((m = bp->nei[l][0][0]) > lcl->n) {
                     k = dir*2 + 1 - l%2;
                     for (i = 0; i < 2; i++)
                        for (j = 0; j < 2; j++)
                           if (blocks[m].nei[k][i][j] == lcl->n) {
                              on_proc_comm_diff(glbl, m, lcl->n, k, i, j, start, num_comm);
                              counter_diff[dir] += 2;
                           }
                  }
                  timer_comm_diff[dir] += timer() - t2;
               } else if (bp->nei_level[l] == -2) {
                  t2 = timer();
                  apply_bc(glbl, l, cp, start, num_comm);
                  counter_bc[dir]++;
                  timer_comm_bc[dir] += timer() - t2;
               } else {
                  printf("ERROR: misconnected block\n");
                  exit(-1);
               }
            }
      }

      for (i = 0; i < num_comm_partners[dir]; i++) {
         t2 = timer();
#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this:
         MPI_Waitany(num_comm_partners[dir], request, &which, &status);
#else   // OCR doesn't (presently) support the concept of an EDT triggering when ANY of a list of dependences is satisfied.  Rework
        // to wait specifically for each IRecv.  Also provide stuff OCR version needs to be able to receive the result buffer.
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Wait__for_Irecv(glbl, &recv_buff[comm_recv_off[dir][comm_index[dir][i]]], recv_size[dir][i], MPI_DOUBLE, comm_partner[dir][i], MPI_COMM_WORLD, -9999, &request[i], &status, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         which = i;
#endif
         t3 = timer();
         for (lcl->n = 0; lcl->n < comm_num[dir][which]; lcl->n++)
          unpack_face(glbl, &recv_buff[comm_recv_off[dir][comm_index[dir][which]+lcl->n]],
                      comm_block[dir][comm_index[dir][which]+lcl->n],
                      comm_face_case[dir][comm_index[dir][which]+lcl->n],
                      dir, start, num_comm);
         counter_face_recv[dir] += comm_num[dir][which];
         t4 = timer();
         timer_comm_wait[dir] += t3 - t2;
         timer_comm_unpack[dir] += t4 - t3;
      }
      timer_comm_dir[dir] += timer() - t1;

      if (nonblocking) {
         t2 = timer();
         for (i = 0; i < num_comm_partners[dir]; i++) {
#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this:
            MPI_Waitany(num_comm_partners[dir], s_req, &which, &status);
#else   // OCR doesn't (presently) support the concept of an EDT triggering when ANY of a list of dependences is satisfied.  Rework
        // to wait specifically for each ISend.
            gasket__mpi_Wait__for_Isend(glbl, &s_req[i], &status, __FILE__, __LINE__);
#endif
         }
         t3 = timer();
         timer_comm_wait[dir] += t3 - t2;
      }
   }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  k
#undef  l
#undef  m
//#undef  n
#undef  dir
#undef  o
#undef  in
#undef  which
#undef  offset
#undef  type
#undef  t1
#undef  t2
#undef  t3
#undef  t4
#undef  status
#undef  bp
#undef  cp
}

// Pack face to send - note different cases for different directions.
void pack_face(Globals_t * const glbl, double *send_buf, int block_num, int face_case, int dir,
               int start, int num_comm)
{
   int i, j, k, n, m;
   int is, ie, js, je, ks, ke;
   OBTAIN_ACCESS_TO_cells
   Cell_t  *cp;
   TRACE

   cp = &cells[block_num];

   if (!code) {

      if (dir == 0) {        /* X - East, West */

         /* X directions (East and West) sent first, so just send
            the real values and no ghosts
         */
         if (face_case >= 10) { /* +X - East */
            i = x_block_size;
            face_case = face_case - 10;
         } else                 /* -X - West */
            i = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 1; j <= y_block_size; j++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 1; j < y_block_size; j += 2)
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k  ] +
                                   (*cp)[m][i][j  ][k+1] +
                                   (*cp)[m][i][j+1][k  ] +
                                   (*cp)[m][i][j+1][k+1];
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k]/4.0;
         }

      } else if (dir == 1) { /* Y - North, South */

         /* Y directions (North and South) sent second, so send the real values
         */
         if (face_case >= 10) { /* +Y - North */
            j = y_block_size;
            face_case = face_case - 10;
         } else                 /* -Y - South */
            j = 1;
         if (face_case == 0) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i <= x_block_size; i++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case == 1) {
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i < x_block_size; i += 2)
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i  ][j][k  ] +
                                   (*cp)[m][i  ][j][k+1] +
                                   (*cp)[m][i+1][j][k  ] +
                                   (*cp)[m][i+1][j][k+1];
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k]/4.0;
         }

      } else {               /* Z - Up, Down */

         /* Z directions (Up and Down) sent last
         */
         if (face_case >= 10) { /* +Z - Up */
            k = z_block_size;
            face_case = face_case - 10;
         } else                 /* -Z - Down */
            k = 1;
         if (face_case == 0) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i <= x_block_size; i++)
                  for (j = 1; j <= y_block_size; j++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case == 1) {
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i < x_block_size; i += 2)
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i  ][j  ][k] +
                                   (*cp)[m][i  ][j+1][k] +
                                   (*cp)[m][i+1][j  ][k] +
                                   (*cp)[m][i+1][j+1][k];
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     send_buf[n] = (*cp)[m][i][j][k]/4.0;
         }
      }

   } else if (code == 1) { /* send all ghosts */

      if (dir == 0) {        /* X - East, West */

         if (face_case >= 10) { /* +X - East */
            i = x_block_size;
            face_case = face_case - 10;
         } else                 /* -X - West */
            i = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 0; j <= y_block_size+1; j++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  j = 0;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
               for (j = 1; j < y_block_size; j += 2) {
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j  ][k] +
                                     (*cp)[m][i][j+1][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k  ] +
                                   (*cp)[m][i][j  ][k+1] +
                                   (*cp)[m][i][j+1][k  ] +
                                   (*cp)[m][i][j+1][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j  ][k] +
                                     (*cp)[m][i][j+1][k];
                  }
               }
               if (face_case%2 == 1) {
                  j = y_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               js = 0;
               je = y_block_half + 1;
            } else {
               js = y_block_half;
               je = y_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half + 1;
            } else {
               ks = z_block_half;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k]/4.0;
         }

      } else if (dir == 1) { /* Y - North, South */

         if (face_case >= 10) { /* +Y - North */
            j = y_block_size;
            face_case = face_case - 10;
         } else                 /* -Y - South */
            j = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  i = 0;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
               for (i = 1; i < x_block_size; i += 2) {
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i  ][j][k  ] +
                                   (*cp)[m][i  ][j][k+1] +
                                   (*cp)[m][i+1][j][k  ] +
                                   (*cp)[m][i+1][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
               }
               if (face_case%2 == 1) {
                  i = x_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half + 1;
            } else {
               is = x_block_half;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half + 1;
            } else {
               ks = z_block_half;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k]/4.0;
         }

      } else {               /* Z - Up, Down */

         /* Z directions (Up and Down) sent last
         */
         if (face_case >= 10) { /* +Z - Up */
            k = z_block_size;
            face_case = face_case - 10;
         } else                 /* -Z - Down */
            k = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  i = 0;
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k] +
                                   (*cp)[m][i][j+1][k];
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
               for (i = 1; i < x_block_size; i += 2) {
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i  ][j  ][k] +
                                   (*cp)[m][i  ][j+1][k] +
                                   (*cp)[m][i+1][j  ][k] +
                                   (*cp)[m][i+1][j+1][k];
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
               }
               if (face_case%2 == 1) {
                  i = x_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k] +
                                   (*cp)[m][i][j+1][k];
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half + 1;
            } else {
               is = x_block_half;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               js = 0;
               je = y_block_half + 1;
            } else {
               js = y_block_half;
               je = y_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     send_buf[n] = (*cp)[m][i][j][k]/4.0;
         }
      }

   } else { /* code == 2 send all ghosts and do all processing on send side */

      if (dir == 0) {        /* X - East, West */

         if (face_case >= 10) { /* +X - East */
            i = x_block_size;
            face_case = face_case - 10;
         } else                 /* -X - West */
            i = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 0; j <= y_block_size+1; j++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  j = 0;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
               for (j = 1; j < y_block_size; j += 2) {
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j  ][k] +
                                     (*cp)[m][i][j+1][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k  ] +
                                   (*cp)[m][i][j  ][k+1] +
                                   (*cp)[m][i][j+1][k  ] +
                                   (*cp)[m][i][j+1][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j  ][k] +
                                     (*cp)[m][i][j+1][k];
                  }
               }
               if (face_case%2 == 1) {
                  j = y_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++) {
               j = js - 1;
               k = ks - 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (k = ks; k <= ke; k++, n+=2)
                  send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
               k = ke + 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (j = js; j <= je; j++) {
                  k = ks - 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  for (k = ks; k <= ke; k++, n+=2)
                     send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
                  k = ke + 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  k = ks - 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  for (k = ks; k <= ke; k++, n+=2)
                     send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
                  k = ke + 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               }
               j = je + 1;
               k = ks - 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (k = ks; k <= ke; k++, n+=2)
                  send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
               k = ke + 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
            }
         }

      } else if (dir == 1) { /* Y - North, South */

         if (face_case >= 10) { /* +Y - North */
            j = y_block_size;
            face_case = face_case - 10;
         } else                 /* -Y - South */
            j = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  i = 0;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
               for (i = 1; i < x_block_size; i += 2) {
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i  ][j][k  ] +
                                   (*cp)[m][i  ][j][k+1] +
                                   (*cp)[m][i+1][j][k  ] +
                                   (*cp)[m][i+1][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
               }
               if (face_case%2 == 1) {
                  i = x_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++) {
               i = is - 1;
               k = ks - 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (k = ks; k <= ke; k++, n+=2)
                  send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
               k = ke + 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (i = is; i <= ie; i++) {
                  k = ks - 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  for (k = ks; k <= ke; k++, n+=2)
                     send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
                  k = ke + 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  k = ks - 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  for (k = ks; k <= ke; k++, n+=2)
                     send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
                  k = ke + 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               }
               i = ie + 1;
               k = ks - 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (k = ks; k <= ke; k++, n+=2)
                  send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
               k = ke + 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
            }
         }

      } else {               /* Z - Up, Down */

         /* Z directions (Up and Down) sent last
         */
         if (face_case >= 10) { /* +Z - Up */
            k = z_block_size;
            face_case = face_case - 10;
         } else                 /* -Z - Down */
            k = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  i = 0;
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k] +
                                   (*cp)[m][i][j+1][k];
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
               for (i = 1; i < x_block_size; i += 2) {
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i  ][j  ][k] +
                                   (*cp)[m][i  ][j+1][k] +
                                   (*cp)[m][i+1][j  ][k] +
                                   (*cp)[m][i+1][j+1][k];
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
               }
               if (face_case%2 == 1) {
                  i = x_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k] +
                                   (*cp)[m][i][j+1][k];
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++) {
               i = is - 1;
               j = js - 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (j = js; j <= je; j++, n+=2)
                  send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
               j = je + 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (i = is; i <= ie; i++) {
                  j = js - 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  for (j = js; j <= je; j++, n+=2)
                     send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
                  j = je + 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  j = js - 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  for (j = js; j <= je; j++, n+=2)
                     send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
                  j = je + 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               }
               i = ie + 1;
               j = js - 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (j = js; j <= je; j++, n+=2)
                  send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
               j = je + 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
            }
         }
      }
   }
}

// Unpack ghost values that have been recieved.
// The sense of the face case is reversed since we are receiving what was sent
void unpack_face(Globals_t * const glbl, double *recv_buf, int block_num, int face_case, int dir,
                 int start, int num_comm)
{
   int i, j, k, n, m;
   int is, ie, js, je, ks, ke;
   OBTAIN_ACCESS_TO_cells
   Cell_t  *cp;
   TRACE

   cp = &cells[block_num];

   if (!code) {

      if (dir == 0) {        /* X - East, West */

         /* X directions (East and West)
            just recv the real values and no ghosts
            face_case based on send - so reverse
         */
         if (face_case >= 10) { /* +X - from East */
            i = x_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -X - from West */
            i = 0;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 1; j <= y_block_size; j++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - one case */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 1; j < y_block_size; j += 2)
                  for (k = 1; k < z_block_size; k += 2, n++)
                     (*cp)[m][i][j  ][k  ] =
                     (*cp)[m][i][j  ][k+1] =
                     (*cp)[m][i][j+1][k  ] =
                     (*cp)[m][i][j+1][k+1] = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to recv */
            if (face_case%2 == 0) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }

      } else if (dir == 1) { /* Y - North, South */

         /* Y directions (North and South) sent second, so recv the real values
         */
         if (face_case >= 10) { /* +Y - from North */
            j = y_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Y - from South */
            j = 0;
         if (face_case == 0) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i <= x_block_size; i++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case == 1) {
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* one case - recv into 4 cells per cell sent */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i < x_block_size; i += 2)
                  for (k = 1; k < z_block_size; k += 2, n++)
                     (*cp)[m][i  ][j][k  ] =
                     (*cp)[m][i  ][j][k+1] =
                     (*cp)[m][i+1][j][k  ] =
                     (*cp)[m][i+1][j][k+1] = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }

      } else {               /* Z - Up, Down */

         /* Z directions (Up and Down) sent last
         */
         if (face_case >= 10) { /* +Z - from Up */
            k = z_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Z - from Down */
            k = 0;
         if (face_case == 0) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i <= x_block_size; i++)
                  for (j = 1; j <= y_block_size; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case == 1) {
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* one case - receive into 4 cells */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i < x_block_size; i += 2)
                  for (j = 1; j < y_block_size; j += 2, n++)
                     (*cp)[m][i  ][j  ][k] =
                     (*cp)[m][i  ][j+1][k] =
                     (*cp)[m][i+1][j  ][k] =
                     (*cp)[m][i+1][j+1][k] = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }
      }

   } else if (code == 1) {  /* send ghosts */

      if (dir == 0) {        /* X - East, West */

         if (face_case >= 10) { /* +X - from East */
            i = x_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -X - from West */
            i = 0;
         if (face_case < 2) {        /* whole face -> whole */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 0; j <= y_block_size+1; j++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               j = 0;
               k = 0;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (k = 1; k < z_block_size; k += 2, n++)
                  (*cp)[m][i][j][k  ] =
                  (*cp)[m][i][j][k+1] = recv_buf[n];
               k = z_block_size + 1;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (j = 1; j < y_block_size; j += 2) {
                  k = 0;
                  (*cp)[m][i][j  ][k] =
                  (*cp)[m][i][j+1][k] = recv_buf[n++];
                  for (k = 1; k < z_block_size; k += 2, n++)
                     (*cp)[m][i][j  ][k  ] =
                     (*cp)[m][i][j  ][k+1] =
                     (*cp)[m][i][j+1][k  ] =
                     (*cp)[m][i][j+1][k+1] = recv_buf[n];
                  k = z_block_size + 1;
                  (*cp)[m][i][j  ][k] =
                  (*cp)[m][i][j+1][k] = recv_buf[n++];
               }
               j = y_block_size + 1;
               k = 0;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (k = 1; k < z_block_size; k += 2, n++)
                  (*cp)[m][i][j][k  ] =
                  (*cp)[m][i][j][k+1] = recv_buf[n];
               k = z_block_size + 1;
               (*cp)[m][i][j][k] = recv_buf[n++];
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to recv */
            if (face_case%2 == 0) {
               js = 0;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }

      } else if (dir == 1) { /* Y - North, South */

         if (face_case >= 10) { /* +Y - from North */
            j = y_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Y - from South */
            j = 0;
         if (face_case < 2) {        /* whole face -> whole */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               i = 0;
               k = 0;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (k = 1; k < z_block_size; k += 2, n++)
                  (*cp)[m][i][j][k  ] =
                  (*cp)[m][i][j][k+1] = recv_buf[n];
               k = z_block_size + 1;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (i = 1; i < x_block_size; i += 2) {
                  k = 0;
                  (*cp)[m][i  ][j][k] =
                  (*cp)[m][i+1][j][k] = recv_buf[n++];
                  for (k = 1; k < z_block_size; k += 2, n++)
                     (*cp)[m][i  ][j][k  ] =
                     (*cp)[m][i  ][j][k+1] =
                     (*cp)[m][i+1][j][k  ] =
                     (*cp)[m][i+1][j][k+1] = recv_buf[n];
                  k = z_block_size + 1;
                  (*cp)[m][i  ][j][k] =
                  (*cp)[m][i+1][j][k] = recv_buf[n++];
               }
               i = x_block_size + 1;
               k = 0;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (k = 1; k < z_block_size; k += 2, n++)
                  (*cp)[m][i][j][k  ] =
                  (*cp)[m][i][j][k+1] = recv_buf[n];
               k = z_block_size + 1;
               (*cp)[m][i][j][k] = recv_buf[n++];
            }
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }

      } else {               /* Z - Up, Down */

         if (face_case >= 10) { /* +Z - from Up */
            k = z_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Z - from Down */
            k = 0;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               i = 0;
               j = 0;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (j = 1; j < y_block_size; j += 2, n++)
                  (*cp)[m][i][j  ][k] =
                  (*cp)[m][i][j+1][k] = recv_buf[n];
               j = y_block_size + 1;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (i = 1; i < x_block_size; i += 2) {
                  j = 0;
                  (*cp)[m][i  ][j][k] =
                  (*cp)[m][i+1][j][k] = recv_buf[n++];
                  for (j = 1; j < y_block_size; j += 2, n++)
                     (*cp)[m][i  ][j  ][k] =
                     (*cp)[m][i  ][j+1][k] =
                     (*cp)[m][i+1][j  ][k] =
                     (*cp)[m][i+1][j+1][k] = recv_buf[n];
                  j = y_block_size + 1;
                  (*cp)[m][i  ][j][k] =
                  (*cp)[m][i+1][j][k] = recv_buf[n++];
               }
               i = x_block_size + 1;
               j = 0;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (j = 1; j < y_block_size; j += 2, n++)
                  (*cp)[m][i][j  ][k] =
                  (*cp)[m][i][j+1][k] = recv_buf[n];
               j = y_block_size + 1;
               (*cp)[m][i][j][k] = recv_buf[n++];
            }
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               js = 0;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }
      }

   } else {  /* code == 2 send ghosts and process on send */

      if (dir == 0) {        /* X - East, West */

         if (face_case >= 10) { /* +X - from East */
            i = x_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -X - from West */
            i = 0;
         if (face_case <= 5) {        /* whole face -> whole or quarter face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 0; j <= y_block_size+1; j++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to recv */
            if (face_case%2 == 0) {
               js = 0;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }

      } else if (dir == 1) { /* Y - North, South */

         if (face_case >= 10) { /* +Y - from North */
            j = y_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Y - from South */
            j = 0;
         if (face_case <= 5) {        /* whole face -> whole or quarter face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }

      } else {               /* Z - Up, Down */

         if (face_case >= 10) { /* +Z - from Up */
            k = z_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Z - from Down */
            k = 0;
         if (face_case <= 5) {        /* whole face -> whole or quarter face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               js = 0;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }
      }
   }
}

// Routine that does on processor communication between two blocks that
// are at the same level of refinement.
void on_proc_comm(Globals_t * const glbl, int n, int n1, int l, int start, int num_comm)
{
   int i, j, k, m;
   int is, ie, js, je;
   OBTAIN_ACCESS_TO_cells
   Cell_t  *cp, *cp1;
   TRACE

   /* Determine direction and then exchange data across the face
   */
   if (!code) {
      if ((l/2) == 0) {         /* West, East */
         if ((l%2) == 0) {      /* West */
            cp = &cells[n];
            cp1 = &cells[n1];
         } else {               /* East */
            cp1 = &cells[n];
            cp = &cells[n1];
         }
         for (m = start; m < start+num_comm; m++)
            for (j = 1; j <= y_block_size; j++)
               for (k = 1; k <= z_block_size; k++) {
                  (*cp1)[m][x_block_size+1][j][k] = (*cp)[m][1][j][k];
                  (*cp)[m][0][j][k] = (*cp1)[m][x_block_size][j][k];
               }
      } else if ((l/2) == 1) {  /* South, North */
         if ((l%2) == 0) {      /* South */
            cp = &cells[n];
            cp1 = &cells[n1];
         } else {               /* North */
            cp1 = &cells[n];
            cp = &cells[n1];
         }
         if (stencil == 7) {
           is = 1;
           ie = x_block_size;
         } else {
           is = 0;
           ie = x_block_size + 1;
         }
         for (m = start; m < start+num_comm; m++)
            for (i = is; i <= ie; i++)
               for (k = 1; k <= z_block_size; k++) {
                  (*cp1)[m][i][y_block_size+1][k] = (*cp)[m][i][1][k];
                  (*cp)[m][i][0][k] = (*cp1)[m][i][y_block_size][k];
               }
      } else if ((l/2) == 2) {  /* Down, Up */
         if ((l%2) == 0) {      /* Down */
            cp = &cells[n];
            cp1 = &cells[n1];
         } else {               /* Up */
            cp1 = &cells[n];
            cp = &cells[n1];
         }
         if (stencil == 7) {
           is = 1;
           ie = x_block_size;
           js = 1;
           je = y_block_size;
         } else {
           is = 0;
           ie = x_block_size + 1;
           js = 0;
           je = y_block_size + 1;
         }
         for (m = start; m < start+num_comm; m++)
            for (i = is; i <= ie; i++)
               for (j = js; j <= je; j++) {
                  (*cp1)[m][i][j][z_block_size+1] = (*cp)[m][i][j][1];
                  (*cp)[m][i][j][0] = (*cp1)[m][i][j][z_block_size];
               }
      }
   } else {  /* set all ghosts */
      if ((l/2) == 0) {         /* West, East */
         if ((l%2) == 0) {      /* West */
            cp = &cells[n];
            cp1 = &cells[n1];
         } else {               /* East */
            cp1 = &cells[n];
            cp = &cells[n1];
         }
         for (m = start; m < start+num_comm; m++)
            for (j = 0; j <= y_block_size+1; j++)
               for (k = 0; k <= z_block_size+1; k++) {
                  (*cp1)[m][x_block_size+1][j][k] = (*cp)[m][1][j][k];
                  (*cp)[m][0][j][k] = (*cp1)[m][x_block_size][j][k];
               }
      } else if ((l/2) == 1) {  /* South, North */
         if ((l%2) == 0) {      /* South */
            cp = &cells[n];
            cp1 = &cells[n1];
         } else {               /* North */
            cp1 = &cells[n];
            cp = &cells[n1];
         }
         for (m = start; m < start+num_comm; m++)
            for (i = 0; i <= x_block_size+1; i++)
               for (k = 0; k <= z_block_size+1; k++) {
                  (*cp1)[m][i][y_block_size+1][k] = (*cp)[m][i][1][k];
                  (*cp)[m][i][0][k] = (*cp1)[m][i][y_block_size][k];
               }
      } else if ((l/2) == 2) {  /* Down, Up */
         if ((l%2) == 0) {      /* Down */
            cp = &cells[n];
            cp1 = &cells[n1];
         } else {               /* Up */
            cp1 = &cells[n];
            cp = &cells[n1];
         }
         for (m = start; m < start+num_comm; m++)
            for (i = 0; i <= x_block_size+1; i++)
               for (j = 0; j <= y_block_size+1; j++) {
                  (*cp1)[m][i][j][z_block_size+1] = (*cp)[m][i][j][1];
                  (*cp)[m][i][j][0] = (*cp1)[m][i][j][z_block_size];
               }
      }
   }
}

// Routine that does on processor communication between two blocks that are
// at different levels of refinement.  The order of the blocks that are
// being input determine which block is at a higher level of refinement.
void on_proc_comm_diff(Globals_t * const glbl, int n, int n1, int l, int iq, int jq,
                       int start, int num_comm)
{
   int i, j, k, m;
   int i0, i1, i2, i3, j0, j1, j2, j3, k0, k1, k2, k3;
   OBTAIN_ACCESS_TO_cells
   Cell_t  *cp, *cp1;
   TRACE

   cp = &cells[n];
   cp1 = &cells[n1];

   /* (iq, jq) quarter face on block n to whole face on block n1
   */
   if (!code) {
      /* only have to communicate ghost values - bp is level, bp1 is level+1 -
       * in 2 to 1 case get 0..block_half from one proc and
       *                block_half+1..block_size+1 from another
       * in 1 to 2 case get 0..block_size+1 from 0..block_half+1 or
       *                block_half..block_size+1 with averages
       */
      if ((l/2) == 0) {
         if (l == 0) {             /* West */
            i0 = 0;
            i1 = 1;
            i2 = x_block_size + 1;
            i3 = x_block_size;
         } else {                  /* East */
            i0 = x_block_size + 1;
            i1 = x_block_size;
            i2 = 0;
            i3 = 1;
         }
         j1 = jq*y_block_half;
         k1 = iq*z_block_half;
         for (m = start; m < start+num_comm; m++)
            for (j = 1; j <= y_block_half; j++)
               for (k = 1; k <= z_block_half; k++) {
                  (*cp1)[m][i2][2*j-1][2*k-1] =
                  (*cp1)[m][i2][2*j-1][2*k  ] =
                  (*cp1)[m][i2][2*j  ][2*k-1] =
                  (*cp1)[m][i2][2*j  ][2*k  ] =
                                             (*cp)[m][i1][j+j1][k+k1]/4.0;
                  (*cp)[m][i0][j+j1][k+k1] =
                                             (*cp1)[m][i3][2*j-1][2*k-1] +
                                             (*cp1)[m][i3][2*j-1][2*k  ] +
                                             (*cp1)[m][i3][2*j  ][2*k-1] +
                                             (*cp1)[m][i3][2*j  ][2*k  ];
               }
      } else if ((l/2) == 1) {
         if (l == 2) {             /* South */
            j0 = 0;
            j1 = 1;
            j2 = y_block_size + 1;
            j3 = y_block_size;
         } else {                  /* North */
            j0 = y_block_size + 1;
            j1 = y_block_size;
            j2 = 0;
            j3 = 1;
         }
         i1 = jq*x_block_half;
         k1 = iq*z_block_half;
         for (m = start; m < start+num_comm; m++)
            for (i = 1; i <= x_block_half; i++)
               for (k = 1; k <= z_block_half; k++) {
                  (*cp1)[m][2*i-1][j2][2*k-1] =
                  (*cp1)[m][2*i-1][j2][2*k  ] =
                  (*cp1)[m][2*i  ][j2][2*k-1] =
                  (*cp1)[m][2*i  ][j2][2*k  ] =
                                             (*cp)[m][i+i1][j1][k+k1]/4.0;
                  (*cp)[m][i+i1][j0][k+k1] =
                                             (*cp1)[m][2*i-1][j3][2*k-1] +
                                             (*cp1)[m][2*i-1][j3][2*k  ] +
                                             (*cp1)[m][2*i  ][j3][2*k-1] +
                                             (*cp1)[m][2*i  ][j3][2*k  ];
               }
      } else if ((l/2) == 2) {
         if (l == 4) {             /* Down */
            k0 = 0;
            k1 = 1;
            k2 = z_block_size + 1;
            k3 = z_block_size;
         } else {                  /* Up */
            k0 = z_block_size + 1;
            k1 = z_block_size;
            k2 = 0;
            k3 = 1;
         }
         i1 = jq*x_block_half;
         j1 = iq*y_block_half;
         for (m = start; m < start+num_comm; m++)
            for (i = 1; i <= x_block_half; i++)
               for (j = 1; j <= y_block_half; j++) {
                  (*cp1)[m][2*i-1][2*j-1][k2] =
                  (*cp1)[m][2*i-1][2*j  ][k2] =
                  (*cp1)[m][2*i  ][2*j-1][k2] =
                  (*cp1)[m][2*i  ][2*j  ][k2] =
                                              (*cp)[m][i+i1][j+j1][k1]/4.0;
                  (*cp)[m][i+i1][j+j1][k0] =
                                              (*cp1)[m][2*i-1][2*j-1][k3] +
                                              (*cp1)[m][2*i-1][2*j  ][k3] +
                                              (*cp1)[m][2*i  ][2*j-1][k3] +
                                              (*cp1)[m][2*i  ][2*j  ][k3];
               }
      }
   } else {  /* transfer ghosts */
      if ((l/2) == 0) {
         if (l == 0) {             /* West */
            i0 = 0;
            i1 = 1;
            i2 = x_block_size + 1;
            i3 = x_block_size;
         } else {                  /* East */
            i0 = x_block_size + 1;
            i1 = x_block_size;
            i2 = 0;
            i3 = 1;
         }
         j1 = jq*y_block_half;
         k1 = iq*z_block_half;
         j2 = y_block_size + 1;
         j3 = y_block_half + 1;
         k2 = z_block_size + 1;
         k3 = z_block_half + 1;
         for (m = start; m < start+num_comm; m++) {
            (*cp1)[m][i2][0][0] = (*cp)[m][i1][j1][k1]/4.0;
            for (k = 1; k <= z_block_half; k++)
               (*cp1)[m][i2][0][2*k-1] =
               (*cp1)[m][i2][0][2*k  ] = (*cp)[m][i1][j1][k+k1]/4.0;
            (*cp1)[m][i2][0][k2] = (*cp)[m][i1][j1][k3+k1]/4.0;
            if (jq == 0) {
               if (iq == 0)
                  (*cp)[m][i0][0][0 ] = (*cp1)[m][i3][0][0 ];
               else
                  (*cp)[m][i0][0][k2] = (*cp1)[m][i3][0][k2];
               for (k = 1; k <= z_block_half; k++)
                  (*cp)[m][i0][0][k+k1] = ((*cp1)[m][i3][0][2*k-1] +
                                               (*cp1)[m][i3][0][2*k  ]);
            }
            for (j = 1; j <= y_block_half; j++) {
               (*cp1)[m][i2][2*j-1][0] =
               (*cp1)[m][i2][2*j  ][0] = (*cp)[m][i1][j+j1][k1]/4.0;
               if (iq == 0)
                  (*cp)[m][i0][j+j1][0 ] = ((*cp1)[m][i3][2*j-1][0 ] +
                                                (*cp1)[m][i3][2*j  ][0 ]);
               else
                  (*cp)[m][i0][j+j1][k2] = ((*cp1)[m][i3][2*j-1][k2] +
                                                (*cp1)[m][i3][2*j  ][k2]);
               for (k = 1; k <= z_block_half; k++) {
                  (*cp1)[m][i2][2*j-1][2*k-1] =
                  (*cp1)[m][i2][2*j-1][2*k  ] =
                  (*cp1)[m][i2][2*j  ][2*k-1] =
                  (*cp1)[m][i2][2*j  ][2*k  ] =
                                             (*cp)[m][i1][j+j1][k+k1]/4.0;
                  (*cp)[m][i0][j+j1][k+k1] =
                                             (*cp1)[m][i3][2*j-1][2*k-1] +
                                             (*cp1)[m][i3][2*j-1][2*k  ] +
                                             (*cp1)[m][i3][2*j  ][2*k-1] +
                                             (*cp1)[m][i3][2*j  ][2*k  ];
               }
               (*cp1)[m][i2][2*j-1][k2] =
               (*cp1)[m][i2][2*j  ][k2] = (*cp)[m][i1][j+j1][k3+k1]/4.0;
            }
            (*cp1)[m][i2][j2][0] = (*cp)[m][i1][j3+j1][k1]/4.0;
            for (k = 1; k <= z_block_half; k++)
               (*cp1)[m][i2][j2][2*k-1] =
               (*cp1)[m][i2][j2][2*k  ] = (*cp)[m][i1][j3+j1][k+k1]/4.0;
            (*cp1)[m][i2][j2][k2] = (*cp)[m][i1][j3+j1][k3+k1]/4.0;
            if (jq == 1) {
               if (iq == 0)
                  (*cp)[m][i0][j2][0 ] = (*cp1)[m][i3][j2][0 ];
               else
                  (*cp)[m][i0][j2][k2] = (*cp1)[m][i3][j2][k2];
               for (k = 1; k <= z_block_half; k++)
                  (*cp)[m][i0][j2][k+k1] = ((*cp1)[m][i3][j2][2*k-1] +
                                                (*cp1)[m][i3][j2][2*k  ]);
            }
         }
      } else if ((l/2) == 1) {
         if (l == 2) {             /* South */
            j0 = 0;
            j1 = 1;
            j2 = y_block_size + 1;
            j3 = y_block_size;
         } else {                  /* North */
            j0 = y_block_size + 1;
            j1 = y_block_size;
            j2 = 0;
            j3 = 1;
         }
         i1 = jq*x_block_half;
         k1 = iq*z_block_half;
         i2 = x_block_size + 1;
         i3 = x_block_half + 1;
         k2 = z_block_size + 1;
         k3 = z_block_half + 1;
         for (m = start; m < start+num_comm; m++) {
            (*cp1)[m][0][j2][0 ] = (*cp)[m][i1][j1][k1]/4.0;
            for (k = 1; k <= z_block_half; k++)
               (*cp1)[m][0][j2][2*k-1] =
               (*cp1)[m][0][j2][2*k  ] = (*cp)[m][i1][j1][k+k1]/4.0;
            (*cp1)[m][0][j2][k2] = (*cp)[m][i1][j1][k3+k1]/4.0;
            if (jq == 0) {
               if (iq == 0)
                  (*cp)[m][0][j0][0 ] = (*cp1)[m][0][j3][0 ];
               else
                  (*cp)[m][0][j0][k2] = (*cp1)[m][0][j3][k2];
               for (k = 1; k <= z_block_half; k++)
                  (*cp)[m][0][j0][k+k1] = ((*cp1)[m][0][j3][2*k-1] +
                                               (*cp1)[m][0][j3][2*k  ]);
            }
            for (i = 1; i <= x_block_half; i++) {
               (*cp1)[m][2*i-1][j2][0] =
               (*cp1)[m][2*i  ][j2][0] = (*cp)[m][i+i1][j1][k1]/4.0;
               if (iq == 0)
                  (*cp)[m][i+i1][j0][0 ] = ((*cp1)[m][2*i-1][j3][0 ] +
                                                (*cp1)[m][2*i  ][j3][0 ]);
               else
                  (*cp)[m][i+i1][j0][k2] = ((*cp1)[m][2*i-1][j3][k2] +
                                                (*cp1)[m][2*i  ][j3][k2]);
               for (k = 1; k <= z_block_half; k++) {
                  (*cp1)[m][2*i-1][j2][2*k-1] =
                  (*cp1)[m][2*i-1][j2][2*k  ] =
                  (*cp1)[m][2*i  ][j2][2*k-1] =
                  (*cp1)[m][2*i  ][j2][2*k  ] =
                                             (*cp)[m][i+i1][j1][k+k1]/4.0;
                  (*cp)[m][i+i1][j0][k+k1] =
                                             (*cp1)[m][2*i-1][j3][2*k-1] +
                                             (*cp1)[m][2*i-1][j3][2*k  ] +
                                             (*cp1)[m][2*i  ][j3][2*k-1] +
                                             (*cp1)[m][2*i  ][j3][2*k  ];
               }
               (*cp1)[m][2*i-1][j2][k2] =
               (*cp1)[m][2*i  ][j2][k2] = (*cp)[m][i+i1][j1][k3+k1]/4.0;
            }
            (*cp1)[m][i2][j2][0 ] = (*cp)[m][i3+i1][j1][k1]/4.0;
            for (k = 1; k <= z_block_half; k++)
               (*cp1)[m][i2][j2][2*k-1] =
               (*cp1)[m][i2][j2][2*k  ] = (*cp)[m][i3+i1][j1][k+k1]/4.0;
            (*cp1)[m][i2][j2][k2] = (*cp)[m][i3+i1][j1][k3+k1]/4.0;
            if (jq == 1) {
               if (iq == 0)
                  (*cp)[m][i2][j0][0 ] = (*cp1)[m][i2][j3][0 ];
               else
                  (*cp)[m][i2][j0][k2] = (*cp1)[m][i2][j3][k2];
               for (k = 1; k <= z_block_half; k++)
                  (*cp)[m][i2][j0][k+k1] = ((*cp1)[m][i2][j3][2*k-1] +
                                                (*cp1)[m][i2][j3][2*k  ]);
            }
         }
      } else if ((l/2) == 2) {
         if (l == 4) {             /* Down */
            k0 = 0;
            k1 = 1;
            k2 = z_block_size + 1;
            k3 = z_block_size;
         } else {                  /* Up */
            k0 = z_block_size + 1;
            k1 = z_block_size;
            k2 = 0;
            k3 = 1;
         }
         i1 = jq*x_block_half;
         j1 = iq*y_block_half;
         i2 = x_block_size + 1;
         i3 = x_block_half + 1;
         j2 = y_block_size + 1;
         j3 = y_block_half + 1;
         for (m = start; m < start+num_comm; m++) {
            (*cp1)[m][0][0 ][k2] = (*cp)[m][i1][j1][k1]/4.0;
            for (j = 1; j <= y_block_half; j++)
               (*cp1)[m][0][2*j-1][k2] =
               (*cp1)[m][0][2*j  ][k2] = (*cp)[m][i1][j+j1][k1]/4.0;
            (*cp1)[m][0][j2][k2] = (*cp)[m][i1][j3+j1][k1]/4.0;
            if (jq == 0) {
               if (iq == 0)
                  (*cp)[m][0][0 ][k0] = (*cp1)[m][0][0 ][k3];
               else
                  (*cp)[m][0][j2][k0] = (*cp1)[m][0][j2][k3];
               for (j = 1; j <= y_block_half; j++)
                  (*cp)[m][0][j+j1][k0] = ((*cp1)[m][0][2*j-1][k3] +
                                               (*cp1)[m][0][2*j  ][k3]);
            }
            for (i = 1; i <= x_block_half; i++) {
               (*cp1)[m][2*i-1][0][k2] =
               (*cp1)[m][2*i  ][0][k2] = (*cp)[m][i+i1][j1][k1]/4.0;
               if (iq == 0)
                  (*cp)[m][i+i1][0][k0] = ((*cp1)[m][2*i-1][0][k3] +
                                               (*cp1)[m][2*i  ][0][k3]);
               else
                  (*cp)[m][i+i1][j2][k0] = ((*cp1)[m][2*i-1][j2][k3] +
                                                (*cp1)[m][2*i  ][j2][k3]);
               for (j = 1; j <= y_block_half; j++) {
                  (*cp1)[m][2*i-1][2*j-1][k2] =
                  (*cp1)[m][2*i-1][2*j  ][k2] =
                  (*cp1)[m][2*i  ][2*j-1][k2] =
                  (*cp1)[m][2*i  ][2*j  ][k2] =
                                              (*cp)[m][i+i1][j+j1][k1]/4.0;
                  (*cp)[m][i+i1][j+j1][k0] =
                                              (*cp1)[m][2*i-1][2*j-1][k3] +
                                              (*cp1)[m][2*i-1][2*j  ][k3] +
                                              (*cp1)[m][2*i  ][2*j-1][k3] +
                                              (*cp1)[m][2*i  ][2*j  ][k3];
               }
               (*cp1)[m][2*i-1][j2][k2] =
               (*cp1)[m][2*i  ][j2][k2] = (*cp)[m][i+i1][j3+j1][k1]/4.0;
            }
            (*cp1)[m][i2][0 ][k2] = (*cp)[m][i3+i1][j1][k1]/4.0;
            for (j = 1; j <= y_block_half; j++)
               (*cp1)[m][i2][2*j-1][k2] =
               (*cp1)[m][i2][2*j  ][k2] = (*cp)[m][i3+i1][j+j1][k1]/4.0;
            (*cp1)[m][i2][j2][k2] = (*cp)[m][i3+i1][j3+j1][k1]/4.0;
            if (jq == 1) {
               if (iq == 0)
                  (*cp)[m][i2][0 ][k0] = (*cp1)[m][i2][0 ][k3];
               else
                  (*cp)[m][i2][j2][k0] = (*cp1)[m][i2][j2][k3];
               for (j = 1; j <= y_block_half; j++)
                  (*cp)[m][i2][j+j1][k0] = ((*cp1)[m][i2][2*j-1][k3] +
                                                (*cp1)[m][i2][2*j  ][k3]);
            }
         }
      }
   }
}

// Apply reflective boundary conditions to a face of a block.
void apply_bc(Globals_t * const glbl, int l, void *cp_arg, int start, int num_comm)
{
   int var, i, j, k, f, t;

   OBTAIN_ACCESS_TO_cells
   Cell_t  *cp = (Cell_t *) cp_arg;
   TRACE

   t = 0;
   f = 1;
   if (!code && stencil == 7)
      switch (l) {
         case 1: t = x_block_size + 1;
                 f = x_block_size;
         case 0: for (var = start; var < start+num_comm; var++)
                    for (j = 1; j <= y_block_size; j++)
                       for (k = 1; k <= z_block_size; k++)
                          (*cp)[var][t][j][k] = (*cp)[var][f][j][k];
                 break;
         case 3: t = y_block_size + 1;
                 f = y_block_size;
         case 2: for (var = start; var < start+num_comm; var++)
                    for (i = 1; i <= x_block_size; i++)
                       for (k = 1; k <= z_block_size; k++)
                          (*cp)[var][i][t][k] = (*cp)[var][i][f][k];
                 break;
         case 5: t = z_block_size + 1;
                 f = z_block_size;
         case 4: for (var = start; var < start+num_comm; var++)
                    for (i = 1; i <= x_block_size; i++)
                       for (j = 1; j <= y_block_size; j++)
                          (*cp)[var][i][j][t] = (*cp)[var][i][j][f];
                 break;
      }
   else
      switch (l) {
         case 1: t = x_block_size + 1;
                 f = x_block_size;
         case 0: for (var = start; var < start+num_comm; var++)
                    for (j = 0; j <= y_block_size+1; j++)
                       for (k = 0; k <= z_block_size+1; k++)
                          (*cp)[var][t][j][k] = (*cp)[var][f][j][k];
                 break;
         case 3: t = y_block_size + 1;
                 f = y_block_size;
         case 2: for (var = start; var < start+num_comm; var++)
                    for (i = 0; i <= x_block_size+1; i++)
                       for (k = 0; k <= z_block_size+1; k++)
                          (*cp)[var][i][t][k] = (*cp)[var][i][f][k];
                 break;
         case 5: t = z_block_size + 1;
                 f = z_block_size;
         case 4: for (var = start; var < start+num_comm; var++)
                    for (i = 0; i <= x_block_size+1; i++)
                       for (j = 0; j <= y_block_size+1; j++)
                          (*cp)[var][i][j][t] = (*cp)[var][i][j][f];
                 break;
      }
}
