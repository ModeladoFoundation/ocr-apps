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
#include <mpi.h>

#include "block.h"
#include "comm.h"
#include "proto.h"

// Routines to add and delete entries from the communication list that is
// used to exchange values for ghost cells.
void add_comm_list(Globals_t * const glbl, int const dir, int const block_f, int const pe, int const fcase, int const pos, int const pos1)
{
   typedef struct {
      Frame_Header_t myFrame;
      int i, j, s_len, r_len;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__add_comm_list_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define s_len             (lcl->s_len)
#define r_len             (lcl->r_len)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__add_comm_list_t)
   TRACE

   /* set indexes for send and recieve to determine length of message:
    * for example, if we send a whole face to a quarter face, we will
    * recieve a message sent from a quarter face to a whole face and
    * use 2 as index for the send and 3 for the recv.
    * We can use same index except for offset */
   if (fcase >= 10)    /* +- direction encoded in fcase */
      i = fcase - 10;
   else
      i = fcase;
   switch (i) {
      case 0: s_len = r_len = comm_vars*msg_len[dir][0];
              break;
      case 1: s_len = r_len = comm_vars*msg_len[dir][1];
              break;
      case 2:
      case 3:
      case 4:
      case 5: s_len = comm_vars*msg_len[dir][2];
              r_len = comm_vars*msg_len[dir][3];
              break;
      case 6:
      case 7:
      case 8:
      case 9: s_len = comm_vars*msg_len[dir][3];
              r_len = comm_vars*msg_len[dir][2];
              break;
   }

   for (i = 0; i < num_comm_partners[dir]; i++)
      if (comm_partner[dir][i] >= pe)
         break;

   /* i is being used below as an index where information about this
    * block should go */
   if (i < num_comm_partners[dir] && comm_partner[dir][i] == pe) {
      send_size[dir][i] += s_len;
      recv_size[dir][i] += r_len;
      for (j = num_comm_partners[dir]-1; j > i; j--)
         comm_index[dir][j]++;
      comm_num[dir][i]++;
   } else {
      // make sure arrays are long enough
      // move stuff i and above up one
      if (num_comm_partners[dir] == max_comm_part[dir]) {
         max_comm_part[dir] = (int)(2.0*((double) (num_comm_partners[dir]+1)));

         gasket__ma_malloc(glbl, &dbmeta__tmpDb1, (void *) &tmpDb1, max_comm_part[dir]*sizeof(int), __FILE__, __LINE__);
         gasket__ma_malloc(glbl, &dbmeta__tmpDb2, (void *) &tmpDb2, max_comm_part[dir]*sizeof(int), __FILE__, __LINE__);
         gasket__ma_malloc(glbl, &dbmeta__tmpDb3, (void *) &tmpDb3, max_comm_part[dir]*sizeof(int), __FILE__, __LINE__);
         gasket__ma_malloc(glbl, &dbmeta__tmpDb4, (void *) &tmpDb4, max_comm_part[dir]*sizeof(int), __FILE__, __LINE__);
         gasket__ma_malloc(glbl, &dbmeta__tmpDb5, (void *) &tmpDb5, max_comm_part[dir]*sizeof(int), __FILE__, __LINE__);
         SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.

         for (j = 0; j < i; j++)
            tmpDb1[j] = comm_partner[dir][j];
         for (j = i; j < num_comm_partners[dir]; j++)
            tmpDb1[j+1] = comm_partner[dir][j];
         for (j = 0; j < i; j++)
            tmpDb2[j] = send_size[dir][j];
         for (j = i; j < num_comm_partners[dir]; j++)
            tmpDb2[j+1] = send_size[dir][j];
         for (j = 0; j < i; j++)
            tmpDb3[j] = recv_size[dir][j];
         for (j = i; j < num_comm_partners[dir]; j++)
            tmpDb3[j+1] = recv_size[dir][j];
         for (j = 0; j <= i; j++)   // Note that this one is different
            tmpDb4[j] = comm_index[dir][j];
         for (j = i; j < num_comm_partners[dir]; j++)
            tmpDb4[j+1] = comm_index[dir][j] + 1;
         for (j = 0; j < i; j++)
            tmpDb5[j] = comm_num[dir][j];
         for (j = i; j < num_comm_partners[dir]; j++)
            tmpDb5[j+1] = comm_num[dir][j];

         gasket__free(&dbmeta__comm_partner[dir], (void **) &comm_partner[dir], __FILE__, __LINE__);
         gasket__free(&dbmeta__send_size[dir], (void **) &send_size[dir], __FILE__, __LINE__);
         gasket__free(&dbmeta__recv_size[dir], (void **) &recv_size[dir], __FILE__, __LINE__);
         gasket__free(&dbmeta__comm_index[dir], (void **) &comm_index[dir], __FILE__, __LINE__);
         gasket__free(&dbmeta__comm_num[dir], (void **) &comm_num[dir], __FILE__, __LINE__);

         comm_partner[dir]  = tmpDb1;
         dbmeta__comm_partner[dir] = dbmeta__tmpDb1;
         tmpDb1 = NULL;
         dbmeta__tmpDb1.guid = NULL_GUID;
         dbmeta__tmpDb1.size = 0;

         send_size[dir] = tmpDb2;
         dbmeta__send_size[dir] = dbmeta__tmpDb2;
         tmpDb2 = NULL;
         dbmeta__tmpDb2.guid = NULL_GUID;
         dbmeta__tmpDb2.size = 0;

         recv_size[dir] = tmpDb3;
         dbmeta__recv_size[dir] = dbmeta__tmpDb3;
         tmpDb3 = NULL;
         dbmeta__tmpDb3.guid = NULL_GUID;
         dbmeta__tmpDb3.size = 0;

         comm_index[dir] = tmpDb4;
         dbmeta__comm_index[dir] = dbmeta__tmpDb4;
         tmpDb4 = NULL;
         dbmeta__tmpDb4.guid = NULL_GUID;
         dbmeta__tmpDb4.size = 0;

         comm_num[dir] = tmpDb5;
         dbmeta__comm_num[dir] = dbmeta__tmpDb5;
         tmpDb5 = NULL;
         dbmeta__tmpDb5.guid = NULL_GUID;
         dbmeta__tmpDb5.size = 0;
      } else {
         for (j = num_comm_partners[dir]; j > i; j--) {
            comm_partner[dir][j] = comm_partner[dir][j-1];
            send_size[dir][j] = send_size[dir][j-1];
            recv_size[dir][j] = recv_size[dir][j-1];
            comm_index[dir][j] = comm_index[dir][j-1] + 1;
            comm_num[dir][j] = comm_num[dir][j-1];
         }
      }

      if (i == num_comm_partners[dir])
         if (i == 0)
            comm_index[dir][i] = 0;
         else
            comm_index[dir][i] = comm_index[dir][i-1] + comm_num[dir][i-1];
      num_comm_partners[dir]++;
      comm_partner[dir][i] = pe;
      send_size[dir][i] = s_len;
      recv_size[dir][i] = r_len;
      comm_num[dir][i] = 1;  // still have to put info into arrays
   }

   if ((num_cases[dir]+1) > max_num_cases[dir]) {
      max_num_cases[dir] = (int)(2.0*((double) (num_cases[dir]+1)));

      gasket__ma_malloc(glbl, &dbmeta__tmpDb1, (void *) &tmpDb1, max_num_cases[dir]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__tmpDb2, (void *) &tmpDb2, max_num_cases[dir]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__tmpDb3, (void *) &tmpDb3, max_num_cases[dir]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__tmpDb4, (void *) &tmpDb4, max_num_cases[dir]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__tmpDb5, (void *) &tmpDb5, max_num_cases[dir]*sizeof(int), __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__tmpDb6, (void *) &tmpDb6, max_num_cases[dir]*sizeof(int), __FILE__, __LINE__);
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.

      for (j = 0; j < num_cases[dir]; j++)
         tmpDb1[j] = comm_block[dir][j];
      for (j = 0; j < num_cases[dir]; j++)
         tmpDb2[j] = comm_face_case[dir][j];
      for (j = 0; j < num_cases[dir]; j++)
         tmpDb3[j] = comm_pos[dir][j];
      for (j = 0; j < num_cases[dir]; j++)
         tmpDb4[j] = comm_pos1[dir][j];
      for (j = 0; j < num_cases[dir]; j++)
         tmpDb5[j] = comm_send_off[dir][j];
      for (j = 0; j < num_cases[dir]; j++)
         tmpDb6[j] = comm_recv_off[dir][j];

      gasket__free(&dbmeta__comm_block[dir], (void **) &comm_block[dir], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_face_case[dir], (void **) &comm_face_case[dir], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_pos[dir], (void **) &comm_pos[dir], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_pos1[dir], (void **) &comm_pos1[dir], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_send_off[dir], (void **) &comm_send_off[dir], __FILE__, __LINE__);
      gasket__free(&dbmeta__comm_recv_off[dir], (void **) &comm_recv_off[dir], __FILE__, __LINE__);

      comm_block[dir] = tmpDb1;
      dbmeta__comm_block[dir] = dbmeta__tmpDb1;
      tmpDb1 = NULL;
      dbmeta__tmpDb1.guid = NULL_GUID;
      dbmeta__tmpDb1.size = 0;

      comm_face_case[dir] = tmpDb2;
      dbmeta__comm_face_case[dir] = dbmeta__tmpDb2;
      tmpDb2 = NULL;
      dbmeta__tmpDb2.guid = NULL_GUID;
      dbmeta__tmpDb2.size = 0;

      comm_pos[dir] = tmpDb3;
      dbmeta__comm_pos[dir] = dbmeta__tmpDb3;
      tmpDb3 = NULL;
      dbmeta__tmpDb3.guid = NULL_GUID;
      dbmeta__tmpDb3.size = 0;

      comm_pos1[dir] = tmpDb4;
      dbmeta__comm_pos1[dir] = dbmeta__tmpDb4;
      tmpDb4 = NULL;
      dbmeta__tmpDb4.guid = NULL_GUID;
      dbmeta__tmpDb4.size = 0;

      comm_send_off[dir] = tmpDb5;
      dbmeta__comm_send_off[dir] = dbmeta__tmpDb5;
      tmpDb5 = NULL;
      dbmeta__tmpDb5.guid = NULL_GUID;
      dbmeta__tmpDb5.size = 0;

      comm_recv_off[dir] = tmpDb6;
      dbmeta__comm_recv_off[dir] = dbmeta__tmpDb6;
      tmpDb6 = NULL;
      dbmeta__tmpDb6.guid = NULL_GUID;
      dbmeta__tmpDb6.size = 0;
   }
   if (comm_index[dir][i] == num_cases[dir]) {
      // at end
      comm_block[dir][num_cases[dir]] = block_f;
      comm_face_case[dir][num_cases[dir]] = fcase;
      comm_pos[dir][num_cases[dir]] = pos;
      comm_pos1[dir][num_cases[dir]] = pos1;
      comm_send_off[dir][num_cases[dir]] = s_buf_num[dir];
      comm_recv_off[dir][num_cases[dir]] = r_buf_num[dir];
   } else {
      for (j = num_cases[dir]; j > comm_index[dir][i]+comm_num[dir][i]-1; j--){
         comm_block[dir][j] = comm_block[dir][j-1];
         comm_face_case[dir][j] = comm_face_case[dir][j-1];
         comm_pos[dir][j] = comm_pos[dir][j-1];
         comm_pos1[dir][j] = comm_pos1[dir][j-1];
         comm_send_off[dir][j] = comm_send_off[dir][j-1] + s_len;
         comm_recv_off[dir][j] = comm_recv_off[dir][j-1] + r_len;
      }
      for (j = comm_index[dir][i]+comm_num[dir][i]-1;
           j >= comm_index[dir][i]; j--)
         if (j == comm_index[dir][i] || comm_pos[dir][j-1] < pos ||
             (comm_pos[dir][j-1] == pos && comm_pos1[dir][j-1] < pos1)) {
            comm_block[dir][j] = block_f;
            comm_face_case[dir][j] = fcase;
            comm_pos[dir][j] = pos;
            comm_pos1[dir][j] = pos1;
            if (j == num_cases[dir]) {
               comm_send_off[dir][j] = s_buf_num[dir];
               comm_recv_off[dir][j] = r_buf_num[dir];
            }
            // else comm_[send,recv]_off[j] values are correct
            break;
         } else {
            comm_block[dir][j] = comm_block[dir][j-1];
            comm_face_case[dir][j] = comm_face_case[dir][j-1];
            comm_pos[dir][j] = comm_pos[dir][j-1];
            comm_pos1[dir][j] = comm_pos1[dir][j-1];
            comm_send_off[dir][j] = comm_send_off[dir][j-1] + s_len;
            comm_recv_off[dir][j] = comm_recv_off[dir][j-1] + r_len;
         }
   }
   num_cases[dir]++;
   s_buf_num[dir] += s_len;
   r_buf_num[dir] += r_len;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  s_len
#undef  r_len
}

void del_comm_list(Globals_t * const glbl, int dir, int block_f, int pe, int fcase)
{
   int i, j, k, s_len, r_len;
   TRACE

   if (fcase >= 10)    /* +- direction encoded in fcase */
      i = fcase - 10;
   else if (fcase >= 0)
      i = fcase;
   else              /* special case to delete the one in a direction when
                        we don't know which quarter we were sending to */
      i = 2;
   switch (i) {
      case 0: s_len = r_len = comm_vars*msg_len[dir][0];
              break;
      case 1: s_len = r_len = comm_vars*msg_len[dir][1];
              break;
      case 2:
      case 3:
      case 4:
      case 5: s_len = comm_vars*msg_len[dir][2];
              r_len = comm_vars*msg_len[dir][3];
              break;
      case 6:
      case 7:
      case 8:
      case 9: s_len = comm_vars*msg_len[dir][3];
              r_len = comm_vars*msg_len[dir][2];
              break;
   }

   for (i = 0; i < num_comm_partners[dir]; i++)
      if (comm_partner[dir][i] == pe)
         break;

   /* i is being used below as an index where information about this
    * block is located */
   num_cases[dir]--;
   for (j = comm_index[dir][i]; j < comm_index[dir][i] + comm_num[dir][i]; j++)
      if (comm_block[dir][j] == block_f && (comm_face_case[dir][j] == fcase ||
          (fcase==  -1 && comm_face_case[dir][j] >= 2 &&
                          comm_face_case[dir][j] <= 5) ||
          (fcase== -11 && comm_face_case[dir][j] >=12 &&
                          comm_face_case[dir][j] <=15))) {
         for (k = j; k < num_cases[dir]; k++) {
            comm_block[dir][k] = comm_block[dir][k+1];
            comm_face_case[dir][k] = comm_face_case[dir][k+1];
            comm_pos[dir][k] = comm_pos[dir][k+1];
            comm_pos1[dir][k] = comm_pos1[dir][k+1];
            comm_send_off[dir][k] = comm_send_off[dir][k+1] - s_len;
            comm_recv_off[dir][k] = comm_recv_off[dir][k+1] - r_len;
         }
         break;
      }
   comm_num[dir][i]--;
   if (comm_num[dir][i]) {
      send_size[dir][i] -= s_len;
      recv_size[dir][i] -= r_len;
      for (j = i+1; j < num_comm_partners[dir]; j++)
         comm_index[dir][j]--;
   } else {
      num_comm_partners[dir]--;
      for (j = i; j < num_comm_partners[dir]; j++) {
         comm_partner[dir][j] = comm_partner[dir][j+1];
         send_size[dir][j] = send_size[dir][j+1];
         recv_size[dir][j] = recv_size[dir][j+1];
         comm_num[dir][j] = comm_num[dir][j+1];
         comm_index[dir][j] = comm_index[dir][j+1] - 1;
      }
   }

   s_buf_num[dir] -= s_len;
   r_buf_num[dir] -= r_len;
}

void zero_comm_list(Globals_t * const glbl)
{
   int i;
   TRACE

   for (i = 0; i < 3; i++) {
      num_comm_partners[i] = 0;
      s_buf_num[i] = r_buf_num[i] = 0;
      comm_index[i][0] = 0;
      comm_send_off[i][0] = comm_recv_off[i][0] = 0;
   }
}

// check sizes of send and recv buffers and adjust, if necessary
void check_buff_size(Globals_t * const glbl)
{
   typedef struct {
      Frame_Header_t myFrame;
      int i, j, max_send, max_comm, max_recv, season;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__check_buff_size_t;

#define i                 (lcl->i)
#define j                 (lcl->j)
#define max_send          (lcl->max_send)
#define max_comm          (lcl->max_comm)
#define max_recv          (lcl->max_recv)
#define season            (lcl->season)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__check_buff_size_t)
   TRACE

   for (max_send = max_comm = max_recv = i = 0; i < 3; i++) {
      if (nonblocking) {
         if (s_buf_num[i] > max_send)
            max_send = s_buf_num[i];
      } else
         for (j = 0; j < num_comm_partners[i]; j++)
            if (send_size[i][j] > max_send)
               max_send = send_size[i][j];
      if (num_comm_partners[i] > max_comm)
         max_comm = num_comm_partners[i];
      if (r_buf_num[i] > max_recv)
         max_recv = r_buf_num[i];
   }

   season = 0;

   if (max_send > s_buf_size) {
      season = 1;
      s_buf_size = (int) (2.0*((double) max_send));
      gasket__free(&dbmeta__send_buff, (void **) &send_buff, __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__send_buff, (void *) &send_buff, s_buf_size*sizeof(double), __FILE__, __LINE__);
   }

   if (max_recv > r_buf_size) {
      season = 1;
      r_buf_size = (int) (2.0*((double) max_recv));
      gasket__free(&dbmeta__recv_buff, (void **) &recv_buff, __FILE__, __LINE__);
      gasket__ma_malloc(glbl, &dbmeta__recv_buff, (void *) &recv_buff, r_buf_size*sizeof(double), __FILE__, __LINE__);
   }

   if (max_comm > max_num_req) {
      season = 1;
      gasket__free(&dbmeta__request, (void **) &request, __FILE__, __LINE__);
      max_num_req = (int) (2.0*((double) max_comm));
      gasket__ma_malloc(glbl, &dbmeta__request, (void *) &request, max_num_req*sizeof(MPI_Request), __FILE__, __LINE__);
      if (nonblocking) {
         gasket__free(&dbmeta__s_req, (void **) &s_req, __FILE__, __LINE__);
         gasket__ma_malloc(glbl, &dbmeta__s_req, (void *) &s_req, max_num_req*sizeof(MPI_Request), __FILE__, __LINE__);
      }
   }
   if (season) {
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.
   }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  max_send
#undef  max_comm
#undef  max_recv
#undef  season
}

void update_comm_list(Globals_t * const glbl)
{
   OBTAIN_ACCESS_TO_blocks

   typedef struct {
      Frame_Header_t myFrame;
      int dir, mcp, ncp, mnc, nc, i, j, n, c, f, i1, j1;
      struct {
         int *cpe, *cn, *cb, *cf, *cpos, *cpos1;
         int *space;
         Block_t *bp;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__update_comm_list_t;

#define dir               (lcl->dir)
#define mcp               (lcl->mcp)
#define ncp               (lcl->ncp)
#define mnc               (lcl->mnc)
#define nc                (lcl->nc)
#define i                 (lcl->i)
#define j                 (lcl->j)
#define n                 (lcl->n)
#define c                 (lcl->c)
#define f                 (lcl->f)
#define i1                (lcl->i1)
#define j1                (lcl->j1)
#define cpe               (lcl->pointers.cpe)
#define cn                (lcl->pointers.cn)
#define cb                (lcl->pointers.cb)
#define cf                (lcl->pointers.cf)
#define cpos              (lcl->pointers.cpos)
#define cpos1             (lcl->pointers.cpos1)
#define space             (lcl->pointers.space)
#define bp                (lcl->pointers.bp)

   SUSPENDABLE_FUNCTION_PROLOGUE(Frame__update_comm_list_t)
   TRACE

   space = (int *) recv_buff;

   mcp = num_comm_partners[0];
   if (num_comm_partners[1] > mcp)
      mcp = num_comm_partners[1];
   if (num_comm_partners[2] > mcp)
      mcp = num_comm_partners[2];
   mnc = num_cases[0];
   if (num_cases[1] > mnc)
      mnc = num_cases[1];
   if (num_cases[2] > mnc)
      mnc = num_cases[2];

   cpe = space;
   cn = &cpe[mcp];
   cb = &cn[mnc];
   cf = &cb[mnc];
   cpos = &cf[mnc];
   cpos1 = &cpos[mnc];

   for (dir = 0; dir < 3; dir++) {
      // make copies since original is going to be changed
      ncp = num_comm_partners[dir];
      for (i = 0; i < ncp; i++) {
         cpe[i] = comm_partner[dir][i];
         cn[i] = comm_num[dir][i];
      }
      nc = num_cases[dir];
      for (j = 0; j < nc; j++) {
         cb[j] = comm_block[dir][j];
         cf[j] = comm_face_case[dir][j];
         cpos[j] = comm_pos[dir][j];
         cpos1[j] = comm_pos1[dir][j];
      }

      // Go through communication lists and delete those that are being
      // sent from blocks being moved and change those where the the block
      // being communicated with is being moved (delete if moving here).
      for (n = i = 0; i < ncp; i++)
         for (j = 0; j < cn[i]; j++, n++) {
            bp = &blocks[cb[n]];
            if (bp->new_proc != my_pe)  // block being moved
               del_comm_list(glbl, dir, cb[n], cpe[i], cf[n]);
            else {
               if (cf[n] >= 10) {
                  f = cf[n] - 10;
                  c = 2*dir + 1;
               } else {
                  f = cf[n];
                  c = 2*dir;
               }
               if (f <= 5) {
                  if (bp->nei[c][0][0] != (-1 - cpe[i])) {
                     del_comm_list(glbl, dir, cb[n], cpe[i], cf[n]);
                     if ((-1 - bp->nei[c][0][0]) != my_pe) {
                        CALL_SUSPENDABLE_CALLEE(1)
                        add_comm_list(glbl, dir, cb[n], (-1 - bp->nei[c][0][0]),
                                      cf[n], cpos[n], cpos1[n]);
                        DEBRIEF_SUSPENDABLE_FUNCTION(;)
                     }
                  }
               } else {
                  i1 = (f - 6)/2;
                  j1 = f%2;
                  if (bp->nei[c][i1][j1] != (-1 - cpe[i])) {
                     del_comm_list(glbl, dir, cb[n], cpe[i], cf[n]);
                     if ((-1 - bp->nei[c][i1][j1]) != my_pe) {
                        CALL_SUSPENDABLE_CALLEE(1)
                        add_comm_list(glbl, dir, cb[n], (-1 - bp->nei[c][i1][j1]),
                                      cf[n], cpos[n], cpos1[n]);
                        DEBRIEF_SUSPENDABLE_FUNCTION(;)
                     }
                  }
               }
            }
         }
   }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  dir
#undef  mcp
#undef  ncp
#undef  mnc
#undef  nc
#undef  i
#undef  j
#undef  n
#undef  c
#undef  f
#undef  i1
#undef  j1
#undef  cpe
#undef  cn
#undef  cb
#undef  cf
#undef  cpos
#undef  cpos1
#undef  space
#undef  bp
}
