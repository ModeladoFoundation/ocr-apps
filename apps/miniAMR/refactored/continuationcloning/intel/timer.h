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

#ifndef __TIMER_H__
#define __TIMER_H__

typedef struct {
   double                                        timer__average[128];
#define average              glbl->global__timer.timer__average
   double                                        timer__stddev[128];
#define stddev               glbl->global__timer.timer__stddev
   double                                        timer__minimum[128];
#define minimum              glbl->global__timer.timer__minimum
   double                                        timer__maximum[128];
#define maximum              glbl->global__timer.timer__maximum
   double                                        timer__timer_all;
#define timer_all            glbl->global__timer.timer__timer_all
   double                                        timer__timer_comm_all;
#define timer_comm_all       glbl->global__timer.timer__timer_comm_all
   double                                        timer__timer_comm_dir[3];
#define timer_comm_dir       glbl->global__timer.timer__timer_comm_dir
   double                                        timer__timer_comm_recv[3];
#define timer_comm_recv      glbl->global__timer.timer__timer_comm_recv
   double                                        timer__timer_comm_pack[3];
#define timer_comm_pack      glbl->global__timer.timer__timer_comm_pack
   double                                        timer__timer_comm_send[3];
#define timer_comm_send      glbl->global__timer.timer__timer_comm_send
   double                                        timer__timer_comm_same[3];
#define timer_comm_same      glbl->global__timer.timer__timer_comm_same
   double                                        timer__timer_comm_diff[3];
#define timer_comm_diff      glbl->global__timer.timer__timer_comm_diff
   double                                        timer__timer_comm_bc[3];
#define timer_comm_bc        glbl->global__timer.timer__timer_comm_bc
   double                                        timer__timer_comm_wait[3];
#define timer_comm_wait      glbl->global__timer.timer__timer_comm_wait
   double                                        timer__timer_comm_unpack[3];
#define timer_comm_unpack    glbl->global__timer.timer__timer_comm_unpack
   double                                        timer__timer_calc_all;
#define timer_calc_all       glbl->global__timer.timer__timer_calc_all
   double                                        timer__timer_cs_all;
#define timer_cs_all         glbl->global__timer.timer__timer_cs_all
   double                                        timer__timer_cs_red;
#define timer_cs_red         glbl->global__timer.timer__timer_cs_red
   double                                        timer__timer_cs_calc;
#define timer_cs_calc        glbl->global__timer.timer__timer_cs_calc
   double                                        timer__timer_refine_all;
#define timer_refine_all     glbl->global__timer.timer__timer_refine_all
   double                                        timer__timer_refine_co;
#define timer_refine_co      glbl->global__timer.timer__timer_refine_co
   double                                        timer__timer_refine_mr;
#define timer_refine_mr      glbl->global__timer.timer__timer_refine_mr
   double                                        timer__timer_refine_cc;
#define timer_refine_cc      glbl->global__timer.timer__timer_refine_cc
   double                                        timer__timer_refine_sb;
#define timer_refine_sb      glbl->global__timer.timer__timer_refine_sb
   double                                        timer__timer_refine_c1;
#define timer_refine_c1      glbl->global__timer.timer__timer_refine_c1
   double                                        timer__timer_refine_c2;
#define timer_refine_c2      glbl->global__timer.timer__timer_refine_c2
   double                                        timer__timer_refine_sy;
#define timer_refine_sy      glbl->global__timer.timer__timer_refine_sy
   double                                        timer__timer_cb_all;
#define timer_cb_all         glbl->global__timer.timer__timer_cb_all
   double                                        timer__timer_cb_cb;
#define timer_cb_cb          glbl->global__timer.timer__timer_cb_cb
   double                                        timer__timer_cb_pa;
#define timer_cb_pa          glbl->global__timer.timer__timer_cb_pa
   double                                        timer__timer_cb_mv;
#define timer_cb_mv          glbl->global__timer.timer__timer_cb_mv
   double                                        timer__timer_cb_un;
#define timer_cb_un          glbl->global__timer.timer__timer_cb_un
   double                                        timer__timer_target_all;
#define timer_target_all     glbl->global__timer.timer__timer_target_all
   double                                        timer__timer_target_rb;
#define timer_target_rb      glbl->global__timer.timer__timer_target_rb
   double                                        timer__timer_target_dc;
#define timer_target_dc      glbl->global__timer.timer__timer_target_dc
   double                                        timer__timer_target_pa;
#define timer_target_pa      glbl->global__timer.timer__timer_target_pa
   double                                        timer__timer_target_mv;
#define timer_target_mv      glbl->global__timer.timer__timer_target_mv
   double                                        timer__timer_target_un;
#define timer_target_un      glbl->global__timer.timer__timer_target_un
   double                                        timer__timer_target_cb;
#define timer_target_cb      glbl->global__timer.timer__timer_target_cb
   double                                        timer__timer_target_ab;
#define timer_target_ab      glbl->global__timer.timer__timer_target_ab
   double                                        timer__timer_target_da;
#define timer_target_da      glbl->global__timer.timer__timer_target_da
   double                                        timer__timer_target_sb;
#define timer_target_sb      glbl->global__timer.timer__timer_target_sb
   double                                        timer__timer_lb_all;
#define timer_lb_all         glbl->global__timer.timer__timer_lb_all
   double                                        timer__timer_lb_sort;
#define timer_lb_sort        glbl->global__timer.timer__timer_lb_sort
   double                                        timer__timer_lb_pa;
#define timer_lb_pa          glbl->global__timer.timer__timer_lb_pa
   double                                        timer__timer_lb_mv;
#define timer_lb_mv          glbl->global__timer.timer__timer_lb_mv
   double                                        timer__timer_lb_un;
#define timer_lb_un          glbl->global__timer.timer__timer_lb_un
   double                                        timer__timer_lb_misc;
#define timer_lb_misc        glbl->global__timer.timer__timer_lb_misc
   double                                        timer__timer_lb_mb;
#define timer_lb_mb          glbl->global__timer.timer__timer_lb_mb
   double                                        timer__timer_lb_ma;
#define timer_lb_ma          glbl->global__timer.timer__timer_lb_ma
   double                                        timer__timer_rs_all;
#define timer_rs_all         glbl->global__timer.timer__timer_rs_all
   double                                        timer__timer_rs_ca;
#define timer_rs_ca          glbl->global__timer.timer__timer_rs_ca
   double                                        timer__timer_rs_pa;
#define timer_rs_pa          glbl->global__timer.timer__timer_rs_pa
   double                                        timer__timer_rs_mv;
#define timer_rs_mv          glbl->global__timer.timer__timer_rs_mv
   double                                        timer__timer_rs_un;
#define timer_rs_un          glbl->global__timer.timer__timer_rs_un
   double                                        timer__timer_plot;
#define timer_plot           glbl->global__timer.timer__timer_plot
   double                                        timer__size_mesg_recv[3];
#define size_mesg_recv       glbl->global__timer.timer__size_mesg_recv
   double                                        timer__size_mesg_send[3];
#define size_mesg_send       glbl->global__timer.timer__size_mesg_send
   double                                        timer__size_malloc;
#define size_malloc          glbl->global__timer.timer__size_malloc
   double                                        timer__size_malloc_init;
#define size_malloc_init     glbl->global__timer.timer__size_malloc_init
   long                                          timer__total_blocks;
#define total_blocks         glbl->global__timer.timer__total_blocks
   int                                           timer__nb_min;
#define nb_min               glbl->global__timer.timer__nb_min
   int                                           timer__nb_max;
#define nb_max               glbl->global__timer.timer__nb_max
   int                                           timer__nrrs;
#define nrrs                 glbl->global__timer.timer__nrrs
   int                                           timer__nrs;
#define nrs                  glbl->global__timer.timer__nrs
   int                                           timer__nps;
#define nps                  glbl->global__timer.timer__nps
   int                                           timer__nlbs;
#define nlbs                 glbl->global__timer.timer__nlbs
   int                                           timer__num_refined;
#define num_refined          glbl->global__timer.timer__num_refined
   int                                           timer__num_reformed;
#define num_reformed         glbl->global__timer.timer__num_reformed
   int                                           timer__num_moved_all;
#define num_moved_all        glbl->global__timer.timer__num_moved_all
   int                                           timer__num_moved_lb;
#define num_moved_lb         glbl->global__timer.timer__num_moved_lb
   int                                           timer__num_moved_rs;
#define num_moved_rs         glbl->global__timer.timer__num_moved_rs
   int                                           timer__num_moved_reduce;
#define num_moved_reduce     glbl->global__timer.timer__num_moved_reduce
   int                                           timer__num_moved_coarsen;
#define num_moved_coarsen    glbl->global__timer.timer__num_moved_coarsen
   int                                           timer__counter_halo_recv[3];
#define counter_halo_recv    glbl->global__timer.timer__counter_halo_recv
   int                                           timer__counter_halo_send[3];
#define counter_halo_send    glbl->global__timer.timer__counter_halo_send
   int                                           timer__counter_face_recv[3];
#define counter_face_recv    glbl->global__timer.timer__counter_face_recv
   int                                           timer__counter_face_send[3];
#define counter_face_send    glbl->global__timer.timer__counter_face_send
   int                                           timer__counter_bc[3];
#define counter_bc           glbl->global__timer.timer__counter_bc
   int                                           timer__counter_same[3];
#define counter_same         glbl->global__timer.timer__counter_same
   int                                           timer__counter_diff[3];
#define counter_diff         glbl->global__timer.timer__counter_diff
   int                                           timer__counter_malloc;
#define counter_malloc       glbl->global__timer.timer__counter_malloc
   int                                           timer__counter_malloc_init;
#define counter_malloc_init  glbl->global__timer.timer__counter_malloc_init
   int                                           timer__total_red;
#define total_red            glbl->global__timer.timer__total_red
} Timer_t;




#endif
