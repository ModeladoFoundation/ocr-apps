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

/* comm partner variables */

#ifndef __COMM_H__
#define __COMM_H__

#include "block.h"

// for comm_parent - this is a non-symetric communication

typedef struct {
   int *                    comm_part;            // core to communicate with
   Dbmeta_t         dbmeta__comm_part;
   int *                    par_comm_num;         // number to communicate to each core
   Dbmeta_t         dbmeta__par_comm_num;
   int *                    index;                // offset into next two arrays
   Dbmeta_t         dbmeta__index;
   int *                    comm_b;               // block number to communicate from
   Dbmeta_t         dbmeta__comm_b;
   int *                    comm_p;               // parent number of block (for sorting)
   Dbmeta_t         dbmeta__comm_p;
   int *                    comm_c;               // child number of block
   Dbmeta_t         dbmeta__comm_c;
   int                      num_comm_part;        // number of other cores to communicate with
   int                      max_part;             // max communication partners
   int                      par_num_cases;        // number to communicate
   int                      max_cases;            // max number to communicate
} Par_Comm_t;


typedef struct {

   double *                                             comm__send_buff;            // use in comm and for balancing blocks
   #define   send_buff               glbl->global__comm.comm__send_buff
   Dbmeta_t                                             dbmeta__comm__send_buff;
   #define   dbmeta__send_buff       glbl->global__comm.dbmeta__comm__send_buff

   double *                                             comm__recv_buff;            // use in comm and for balancing blocks
   #define   recv_buff               glbl->global__comm.comm__recv_buff
   Dbmeta_t                                             dbmeta__comm__recv_buff;
   #define   dbmeta__recv_buff       glbl->global__comm.dbmeta__comm__recv_buff

   int *                                                comm__comm_partner[3];      // list of comm partners in each direction
   #define   comm_partner            glbl->global__comm.comm__comm_partner
   Dbmeta_t                                             dbmeta__comm__comm_partner[3];
   #define   dbmeta__comm_partner    glbl->global__comm.dbmeta__comm__comm_partner

   int *                                                comm__send_size[3];         // send sizes for each comm partner
   #define   send_size               glbl->global__comm.comm__send_size
   Dbmeta_t                                             dbmeta__comm__send_size[3];
   #define   dbmeta__send_size       glbl->global__comm.dbmeta__comm__send_size

   int *                                                comm__recv_size[3];         // send sizes for each comm partner
   #define   recv_size               glbl->global__comm.comm__recv_size
   Dbmeta_t                                             dbmeta__comm__recv_size[3];
   #define   dbmeta__recv_size       glbl->global__comm.dbmeta__comm__recv_size

   int *                                                comm__comm_index[3];        // index into comm_block, _face_case, and offsets
   #define   comm_index              glbl->global__comm.comm__comm_index
   Dbmeta_t                                             dbmeta__comm__comm_index[3];
   #define   dbmeta__comm_index      glbl->global__comm.dbmeta__comm__comm_index

   int *                                                comm__comm_num[3];          // number of blocks for each comm partner
   #define   comm_num                glbl->global__comm.comm__comm_num
   Dbmeta_t                                             dbmeta__comm__comm_num[3];
   #define   dbmeta__comm_num        glbl->global__comm.dbmeta__comm__comm_num

   int *                                                comm__comm_block[3];        // array containing local block number for comm
   #define   comm_block              glbl->global__comm.comm__comm_block
   Dbmeta_t                                             dbmeta__comm__comm_block[3];
   #define   dbmeta__comm_block      glbl->global__comm.dbmeta__comm__comm_block

   int *                                                comm__comm_face_case[3];    // array containing face cases for comm
   #define   comm_face_case          glbl->global__comm.comm__comm_face_case
   Dbmeta_t                                             dbmeta__comm__comm_face_case[3];
   #define   dbmeta__comm_face_case  glbl->global__comm.dbmeta__comm__comm_face_case

   int *                                                comm__comm_pos[3];          // position for center of sending face
   #define   comm_pos                glbl->global__comm.comm__comm_pos
   Dbmeta_t                                             dbmeta__comm__comm_pos[3];
   #define   dbmeta__comm_pos        glbl->global__comm.dbmeta__comm__comm_pos

   int *                                                comm__comm_pos1[3];         // perpendicular position of sending face
   #define   comm_pos1               glbl->global__comm.comm__comm_pos1
   Dbmeta_t                                             dbmeta__comm__comm_pos1[3];
   #define   dbmeta__comm_pos1       glbl->global__comm.dbmeta__comm__comm_pos1

   int *                                                comm__comm_send_off[3];     // offset into send buffer (global, convert to local)
   #define   comm_send_off           glbl->global__comm.comm__comm_send_off
   Dbmeta_t                                             dbmeta__comm__comm_send_off[3];
   #define   dbmeta__comm_send_off   glbl->global__comm.dbmeta__comm__comm_send_off

   int *                                                comm__comm_recv_off[3];     // offset into recv buffer
   #define   comm_recv_off           glbl->global__comm.comm__comm_recv_off
   Dbmeta_t                                             dbmeta__comm__comm_recv_off[3];
   #define   dbmeta__comm_recv_off   glbl->global__comm.dbmeta__comm__comm_recv_off

   MPI_Request *                                        comm__request;
   #define   request                 glbl->global__comm.comm__request
   Dbmeta_t                                             dbmeta__comm__request;
   #define   dbmeta__request         glbl->global__comm.dbmeta__comm__request

   MPI_Request *                                        comm__s_req;
   #define   s_req                   glbl->global__comm.comm__s_req
   Dbmeta_t                                             dbmeta__comm__s_req;
   #define   dbmeta__s_req           glbl->global__comm.dbmeta__comm__s_req

   int *                                                comm__bin;
   #define   bin                     glbl->global__comm.comm__bin
   Dbmeta_t                                             dbmeta__comm__bin;
   #define   dbmeta__bin             glbl->global__comm.dbmeta__comm__bin

   int *                                                comm__gbin;
   #define   gbin                    glbl->global__comm.comm__gbin
   Dbmeta_t                                             dbmeta__comm__gbin;
   #define   dbmeta__gbin            glbl->global__comm.dbmeta__comm__gbin

   MPI_Comm *                                           comm__comms;
   #define   comms                   glbl->global__comm.comm__comms
   Dbmeta_t                                             dbmeta__comm__comms;
   #define   dbmeta__comms           glbl->global__comm.dbmeta__comm__comms

   int *                                                comm__me;
   #define   me                      glbl->global__comm.comm__me
   Dbmeta_t                                             dbmeta__comm__me;
   #define   dbmeta__me              glbl->global__comm.dbmeta__comm__me

   int *                                                comm__np;
   #define   np                      glbl->global__comm.comm__np
   Dbmeta_t                                             dbmeta__comm__np;
   #define   dbmeta__np              glbl->global__comm.dbmeta__comm__np

   Par_Comm_t                                    comm__par_b;
#define par_b                 glbl->global__comm.comm__par_b
   Par_Comm_t                                    comm__par_p;
#define par_p                 glbl->global__comm.comm__par_p
   Par_Comm_t                                    comm__par_p1;
#define par_p1                glbl->global__comm.comm__par_p1
   int                                           comm__s_buf_size;
#define s_buf_size            glbl->global__comm.comm__s_buf_size
   int                                           comm__r_buf_size;
#define r_buf_size            glbl->global__comm.comm__r_buf_size
   int                                           comm__num_comm_partners[3]; // number of comm partners in each direction
#define num_comm_partners     glbl->global__comm.comm__num_comm_partners
   int                                           comm__max_comm_part[3];     // lengths of comm partners arrays
#define max_comm_part         glbl->global__comm.comm__max_comm_part
   int                                           comm__num_cases[3];         // amount used in above six arrays
#define num_cases             glbl->global__comm.comm__num_cases
   int                                           comm__max_num_cases[3];     // length of above six arrays
#define max_num_cases         glbl->global__comm.comm__max_num_cases
   int                                           comm__s_buf_num[3];         // total amount being sent in each direction
#define s_buf_num             glbl->global__comm.comm__s_buf_num
   int                                           comm__r_buf_num[3];         // total amount being received in each direction
#define r_buf_num             glbl->global__comm.comm__r_buf_num
   int                                           comm__max_num_req;
#define max_num_req           glbl->global__comm.comm__max_num_req
   int                                           comm__pad;                  // Not used.  Here to increase struct size to multiple of 8 (on typical platform).

} Comm_t;

#endif
