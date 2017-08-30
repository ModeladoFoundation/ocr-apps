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
#include <math.h>

#include "block.h"
#include "proto.h"
#include "timer.h"

void init_profile(rankH_t* PTR_rankH)
{
   int i;

   timerH_t* PTR_timerH = &(PTR_rankH->timerH);

   PTR_timerH->timer_all = 0.0;

   PTR_timerH->timer_comm_all = 0.0;
   for (i = 0; i < 3; i++) {
      PTR_timerH->timer_comm_dir[i] = 0.0;
      PTR_timerH->timer_comm_recv[i] = 0.0;
      PTR_timerH->timer_comm_pack[i] = 0.0;
      PTR_timerH->timer_comm_send[i] = 0.0;
      PTR_timerH->timer_comm_same[i] = 0.0;
      PTR_timerH->timer_comm_diff[i] = 0.0;
      PTR_timerH->timer_comm_bc[i] = 0.0;
      PTR_timerH->timer_comm_wait[i] = 0.0;
      PTR_timerH->timer_comm_unpack[i] = 0.0;
   }

   PTR_timerH->timer_calc_all = 0.0;

   PTR_timerH->timer_cs_all = 0.0;
   PTR_timerH->timer_cs_red = 0.0;
   PTR_timerH->timer_cs_calc = 0.0;

   PTR_timerH->timer_refine_all = 0.0;
   PTR_timerH->timer_refine_co = 0.0;
   PTR_timerH->timer_refine_mr = 0.0;
   PTR_timerH->timer_refine_cc = 0.0;
   PTR_timerH->timer_refine_sb = 0.0;
   PTR_timerH->timer_refine_c1 = 0.0;
   PTR_timerH->timer_refine_c2 = 0.0;
   PTR_timerH->timer_refine_sy = 0.0;
   PTR_timerH->timer_cb_all = 0.0;
   PTR_timerH->timer_cb_cb = 0.0;
   PTR_timerH->timer_cb_pa = 0.0;
   PTR_timerH->timer_cb_mv = 0.0;
   PTR_timerH->timer_cb_un = 0.0;
   PTR_timerH->timer_target_all = 0.0;
   PTR_timerH->timer_target_rb = 0.0;
   PTR_timerH->timer_target_dc = 0.0;
   PTR_timerH->timer_target_pa = 0.0;
   PTR_timerH->timer_target_mv = 0.0;
   PTR_timerH->timer_target_un = 0.0;
   PTR_timerH->timer_target_cb = 0.0;
   PTR_timerH->timer_target_ab = 0.0;
   PTR_timerH->timer_target_da = 0.0;
   PTR_timerH->timer_target_sb = 0.0;
   PTR_timerH->timer_lb_all = 0.0;
   PTR_timerH->timer_lb_sort = 0.0;
   PTR_timerH->timer_lb_pa = 0.0;
   PTR_timerH->timer_lb_mv = 0.0;
   PTR_timerH->timer_lb_un = 0.0;
   PTR_timerH->timer_lb_misc = 0.0;
   PTR_timerH->timer_lb_mb = 0.0;
   PTR_timerH->timer_lb_ma = 0.0;
   PTR_timerH->timer_rs_all = 0.0;
   PTR_timerH->timer_rs_ca = 0.0;
   PTR_timerH->timer_rs_pa = 0.0;
   PTR_timerH->timer_rs_mv = 0.0;
   PTR_timerH->timer_rs_un = 0.0;

   PTR_timerH->timer_plot = 0.0;

   PTR_timerH->total_blocks = 0;
   PTR_timerH->nrrs = 0;
   PTR_timerH->nrs = 0;
   PTR_timerH->nps = 0;
   PTR_timerH->nlbs = 0;
   PTR_timerH->num_refined = 0;
   PTR_timerH->num_reformed = 0;
   PTR_timerH->num_moved_all = 0;
   PTR_timerH->num_moved_lb = 0;
   PTR_timerH->num_moved_rs = 0;
   PTR_timerH->num_moved_reduce = 0;
   PTR_timerH->num_moved_coarsen = 0;
   for (i = 0; i < 3; i++) {
      PTR_timerH->counter_halo_recv[i] = 0;
      PTR_timerH->counter_halo_send[i] = 0;
      PTR_timerH->size_mesg_recv[i] = 0.0;
      PTR_timerH->size_mesg_send[i] = 0.0;
      PTR_timerH->counter_face_recv[i] = 0;
      PTR_timerH->counter_face_send[i] = 0;
      PTR_timerH->counter_bc[i] = 0;
      PTR_timerH->counter_same[i] = 0;
      PTR_timerH->counter_diff[i] = 0;
   }
   PTR_timerH->total_red = 0;
}
