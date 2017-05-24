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

#include "block.h"
#include "proto.h"

// Routines to add and delete entries from the communication list that is
// used to exchange values for ghost cells.
int return_buf_size(rankH_t* PTR_rankH, int dir, int fcase, int sendOrRecvFlag)
{
   int i, j, s_len, r_len, *tmp;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    block *bp = &PTR_rankH->blockH;

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
      case 0: s_len = r_len = PTR_cmd->comm_vars*PTR_rankH->msg_len[dir][0];
              break;
      case 1: s_len = r_len = PTR_cmd->comm_vars*PTR_rankH->msg_len[dir][1];
              break;
      case 2:
      case 3:
      case 4:
      case 5: s_len = PTR_cmd->comm_vars*PTR_rankH->msg_len[dir][2];
              r_len = PTR_cmd->comm_vars*PTR_rankH->msg_len[dir][3];
              break;
      case 6:
      case 7:
      case 8:
      case 9: s_len = PTR_cmd->comm_vars*PTR_rankH->msg_len[dir][3];
              r_len = PTR_cmd->comm_vars*PTR_rankH->msg_len[dir][2];
              break;
   }

   return (sendOrRecvFlag)?s_len:r_len;
}
