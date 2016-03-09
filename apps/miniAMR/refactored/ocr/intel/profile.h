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

#ifndef __PROFILE_H__
#define __PROFILE_H__

#include "dbcommheader.h"

typedef struct {
   DbCommHeader_t  dbCommHeader;
   union {
      double       allDoubles[0];
      double       doubleHeader;
      struct {
#if 0
         double    XXXaverage[128];
         double    XXXstddev[128];
         double    XXXminimum[128];
         double    XXXmaximum[128];
         double    XXXtimer_all;
         double    XXXtimer_comm_all;
         double    XXXtimer_comm_dir[3];
         double    XXXtimer_comm_recv[3];
         double    XXXtimer_comm_pack[3];
         double    XXXtimer_comm_send[3];
         double    XXXtimer_comm_same[3];
         double    XXXtimer_comm_diff[3];
         double    XXXtimer_comm_bc[3];
         double    XXXtimer_comm_wait[3];
         double    XXXtimer_comm_unpack[3];
         double    XXXtimer_calc_all;
         double    XXXtimer_cs_all;
         double    XXXtimer_cs_red;
         double    XXXtimer_cs_calc;
#endif
         double    timer_refine_all;
#if 0
         double    XXXtimer_refine_co;
         double    XXXtimer_refine_mr;
         double    XXXtimer_refine_cc;
         double    XXXtimer_refine_sb;
         double    XXXtimer_refine_c1;
         double    XXXtimer_refine_c2;
         double    XXXtimer_refine_sy;
         double    XXXtimer_cb_all;
         double    XXXtimer_cb_cb;
         double    XXXtimer_cb_pa;
         double    XXXtimer_cb_mv;
         double    XXXtimer_cb_un;
         double    XXXtimer_target_all;
         double    XXXtimer_target_rb;
         double    XXXtimer_target_dc;
         double    XXXtimer_target_pa;
         double    XXXtimer_target_mv;
         double    XXXtimer_target_un;
         double    XXXtimer_target_cb;
         double    XXXtimer_target_ab;
         double    XXXtimer_target_da;
         double    XXXtimer_target_sb;
         double    XXXtimer_lb_all;
         double    XXXtimer_lb_sort;
         double    XXXtimer_lb_pa;
         double    XXXtimer_lb_mv;
         double    XXXtimer_lb_un;
         double    XXXtimer_lb_misc;
         double    XXXtimer_lb_mb;
         double    XXXtimer_lb_ma;
         double    XXXtimer_rs_all;
         double    XXXtimer_rs_ca;
         double    XXXtimer_rs_pa;
         double    XXXtimer_rs_mv;
         double    XXXtimer_rs_un;
         double    XXXtimer_plot;
         double    XXXsize_mesg_recv[3];
         double    XXXsize_mesg_send[3];
         double    XXXsize_malloc;
         double    XXXsize_malloc_init;
#endif
         union {
//          double XXXtotal_blocks;
            double doubleTailer;
         };
      };
   };
   union {
      int          allInts[0];
      int          intHeader;
      struct {
#if 0
         int       XXXnb_min;
         int       XXXnb_max;
         int       XXXnrrs;
         int       XXXnrs;
         int       XXXnps;
         int       XXXnlbs;
         int       XXXnum_refined;
         int       XXXnum_reformed;
         int       XXXnum_moved_all;
         int       XXXnum_moved_lb;
         int       XXXnum_moved_rs;
         int       XXXnum_moved_reduce;
         int       XXXnum_moved_coarsen;
         int       XXXcounter_halo_recv[3];
         int       XXXcounter_halo_send[3];
         int       XXXcounter_face_recv[3];
         int       XXXcounter_face_send[3];
         int       XXXcounter_bc[3];
         int       XXXcounter_same[3];
         int       XXXcounter_diff[3];
         int       XXXcounter_malloc;
         int       XXXcounter_malloc_init;
#endif
         union {
//          int    XXXtotal_red;
            int    intTailer;
         };
      };
   };
} Profile_t;
#define sizeof_Profile_t (sizeof(Profile_t))
#define NUM_PROFILE_DOUBLES (((offsetof(Profile_t, doubleTailer)-offsetof(Profile_t, doubleHeader)) / sizeof(double)) + 1)
#define NUM_PROFILE_INTS    (((offsetof(Profile_t,    intTailer)-offsetof(Profile_t,    intHeader)) / sizeof(   int)) + 1)

#endif
