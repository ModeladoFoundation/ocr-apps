#include "hc.h" 
/*
                 Copyright (c) 2010.
      Lawrence Livermore National Security, LLC.
Produced at the Lawrence Livermore National Laboratory.
                  LLNL-CODE-461231
                All rights reserved.
This file is part of LULESH, Version 1.0.0
Please also read this link -- http://www.opensource.org/licenses/index.php
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the disclaimer below.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the disclaimer (as noted below)
     in the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the LLNS/LLNL nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY, LLC,
THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
Additional BSD Notice
1. This notice is required to be provided under our contract with the U.S.
   Department of Energy (DOE). This work was produced at Lawrence Livermore
   National Laboratory under Contract No. DE-AC52-07NA27344 with the DOE.
2. Neither the United States Government nor Lawrence Livermore National
   Security, LLC nor any of their employees, makes any warranty, express
   or implied, or assumes any liability or responsibility for the accuracy,
   completeness, or usefulness of any information, apparatus, product, or
   process disclosed, or represents that its use would not infringe
   privately-owned rights.
3. Also, reference herein to any specific commercial products, process, or
   services by trade name, trademark, manufacturer or otherwise does not
   necessarily constitute or imply its endorsement, recommendation, or
   favoring by the United States Government or Lawrence Livermore National
   Security, LLC. The views and opinions of authors expressed herein do not
   necessarily state or reflect those of the United States Government or
   Lawrence Livermore National Security, LLC, and shall not be used for
   advertising or product endorsement purposes.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if !defined(FSIM) && !defined(OCR)
#include <string.h>
#endif
#include "RAG.h"
#include "AMO.h"
#include "MEM.h"
#include "FP.h"
#define LULESH_SHOW_PROGRESS 1
enum __unnamed_enum___F0_L79_C1_VolumeError__COMMA__QStopError {VolumeError=-1,QStopError=-2};
#if defined(FSIM)
#elif defined(OCR)
#endif // FSIM or OCR
struct Domain_t *domain = (struct Domain_t *)((void *)0);

static Real_t *Allocate_Real_t(size_t hcSize)
{
  return (Real_t *)(malloc((hcSize * sizeof(Real_t ))));
}

static void Release_Real_t(Real_t *ptr)
{
  if (ptr != ((Real_t *)((void *)0))) {
    free(ptr);
  }
}
/* RAG -- prototypes for the edt functions */
#include "RAG_edt.h"
/* RAG -- moved serial/scalar functions to here */
#include "kernels.h"
/* This first implementation allows for runnable code */
/* and is not meant to be optimal. Final implementation */
/* should separate declaration and allocation phases */
/* so that allocation can be scheduled in a cache conscious */
/* manner. */
/**************/
/* Allocation */
/**************/

struct __hc_domain_AllocateNodalPersistent__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
  unsigned long hcSize;
}
;
void __hc_domain_AllocateNodalPersistent__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_domain_AllocateNodalPersistent__(unsigned long hcSize,struct hc_workerState *ws)
{
  struct __hc_domain_AllocateNodalPersistent__frame_t__ *__hc_new_frame__ = (struct __hc_domain_AllocateNodalPersistent__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_domain_AllocateNodalPersistent__frame_t__ ),__hc_domain_AllocateNodalPersistent__));
  __hc_new_frame__ -> hcSize = hcSize;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_157_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
}
;
void _async_157_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_157_5(long i_out,long i_end,struct Domain_t *domain,struct hc_workerState *ws)
{
  struct _async_157_5frame_t__ *__hc_new_frame__ = (struct _async_157_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_157_5frame_t__ ),_async_157_5));
  ((struct _async_157_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_157_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_157_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_domain_AllocateNodalPersistent__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  unsigned long hcSize;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  hcSize = ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> hcSize;;
#ifdef FSIM
#endif // FSIM
;
  if ((domain -> m_x) != ((Real_t *)((void *)0))) 
    free((domain -> m_x));
  domain -> m_x = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_y) != ((Real_t *)((void *)0))) 
    free((domain -> m_y));
  domain -> m_y = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_z) != ((Real_t *)((void *)0))) 
    free((domain -> m_z));
  domain -> m_z = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));;
  if ((domain -> m_xd) != ((Real_t *)((void *)0))) 
    free((domain -> m_xd));
  domain -> m_xd = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_yd) != ((Real_t *)((void *)0))) 
    free((domain -> m_yd));
  domain -> m_yd = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_zd) != ((Real_t *)((void *)0))) 
    free((domain -> m_zd));
  domain -> m_zd = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));;
  if ((domain -> m_xdd) != ((Real_t *)((void *)0))) 
    free((domain -> m_xdd));
  domain -> m_xdd = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_ydd) != ((Real_t *)((void *)0))) 
    free((domain -> m_ydd));
  domain -> m_ydd = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_zdd) != ((Real_t *)((void *)0))) 
    free((domain -> m_zdd));
  domain -> m_zdd = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));;
  if ((domain -> m_fx) != ((Real_t *)((void *)0))) 
    free((domain -> m_fx));
  domain -> m_fx = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_fy) != ((Real_t *)((void *)0))) 
    free((domain -> m_fy));
  domain -> m_fy = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_fz) != ((Real_t *)((void *)0))) 
    free((domain -> m_fz));
  domain -> m_fz = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));;
  if ((domain -> m_nodalMass) != ((Real_t *)((void *)0))) 
    free((domain -> m_nodalMass));
  domain -> m_nodalMass = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));;
#ifdef FSIM
#endif
  hc_startFinish(ws);
{
{
      Index_t i_len = hcSize;
      Index_t i_blk = 16;
      for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
        Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
        __hc_frame__ -> pc = 2;
        ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> i_end = i_end;
        ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> i_out = i_out;
        ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> i_len = i_len;
        ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
        ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> hcSize = hcSize;
        struct hc_frameHeader *_async_157_5frame__2 = buildFrame__async_157_5(i_out,i_end,domain,ws);
        _async_157_5frame__2 -> sched_policy = hc_sched_policy(ws);
        if (_async_157_5frame__2 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_157_5frame__2);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_157_5(ws,((struct hc_frameHeader *)_async_157_5frame__2),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async2:;
          i_end = ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> i_end;
          i_out = ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> i_out;
          i_len = ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> i_len;
          i_blk = ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> i_blk;
          hcSize = ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> hcSize;
        }
      }
    };
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> hcSize = hcSize;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    hcSize = ((struct __hc_domain_AllocateNodalPersistent__frame_t__ *)__hc_frame__) -> hcSize;
  }
#ifdef FSIM
#endif // FSIM
;
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_157_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_157_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_157_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_157_5frame_t__ *)__hc_frame__) -> domain;
  domain_AllocateNodalPersistent_edt_1(i_out,i_end,domain);
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_domain_AllocateElemPersistent__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
  unsigned long hcSize;
}
;
void __hc_domain_AllocateElemPersistent__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_domain_AllocateElemPersistent__(unsigned long hcSize,struct hc_workerState *ws)
{
  struct __hc_domain_AllocateElemPersistent__frame_t__ *__hc_new_frame__ = (struct __hc_domain_AllocateElemPersistent__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_domain_AllocateElemPersistent__frame_t__ ),__hc_domain_AllocateElemPersistent__));
  ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_new_frame__) -> hcSize = hcSize;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_241_6frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
}
;
void _async_241_6(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_241_6(long i_out,long i_end,struct Domain_t *domain,struct hc_workerState *ws)
{
  struct _async_241_6frame_t__ *__hc_new_frame__ = (struct _async_241_6frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_241_6frame_t__ ),_async_241_6));
  ((struct _async_241_6frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_241_6frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_241_6frame_t__ *)__hc_new_frame__) -> domain = domain;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_domain_AllocateElemPersistent__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  unsigned long hcSize;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  hcSize = ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> hcSize;;
#if defined(FSIM)
#endif // FSIM
;
  if ((domain -> m_matElemlist) != ((Index_t *)((void *)0))) 
    free((domain -> m_matElemlist));
  domain -> m_matElemlist = ((Index_t *)(malloc((hcSize * sizeof(Index_t )))));
  if ((domain -> m_nodelist) != ((Index_t *)((void *)0))) 
    free((domain -> m_nodelist));
  domain -> m_nodelist = ((Index_t *)(malloc((hcSize * (((Index_t )8) * sizeof(Index_t ))))));;
  if ((domain -> m_lxim) != ((Index_t *)((void *)0))) 
    free((domain -> m_lxim));
  domain -> m_lxim = ((Index_t *)(malloc((hcSize * sizeof(Index_t )))));
  if ((domain -> m_lxip) != ((Index_t *)((void *)0))) 
    free((domain -> m_lxip));
  domain -> m_lxip = ((Index_t *)(malloc((hcSize * sizeof(Index_t )))));
  if ((domain -> m_letam) != ((Index_t *)((void *)0))) 
    free((domain -> m_letam));
  domain -> m_letam = ((Index_t *)(malloc((hcSize * sizeof(Index_t )))));
  if ((domain -> m_letap) != ((Index_t *)((void *)0))) 
    free((domain -> m_letap));
  domain -> m_letap = ((Index_t *)(malloc((hcSize * sizeof(Index_t )))));
  if ((domain -> m_lzetam) != ((Index_t *)((void *)0))) 
    free((domain -> m_lzetam));
  domain -> m_lzetam = ((Index_t *)(malloc((hcSize * sizeof(Index_t )))));
  if ((domain -> m_lzetap) != ((Index_t *)((void *)0))) 
    free((domain -> m_lzetap));
  domain -> m_lzetap = ((Index_t *)(malloc((hcSize * sizeof(Index_t )))));;
  if ((domain -> m_elemBC) != ((Int_t *)((void *)0))) 
    free((domain -> m_elemBC));
  domain -> m_elemBC = ((Int_t *)(malloc((hcSize * sizeof(Int_t )))));;
  if ((domain -> m_e) != ((Real_t *)((void *)0))) 
    free((domain -> m_e));
  domain -> m_e = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_p) != ((Real_t *)((void *)0))) 
    free((domain -> m_p));
  domain -> m_p = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_q) != ((Real_t *)((void *)0))) 
    free((domain -> m_q));
  domain -> m_q = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_ql) != ((Real_t *)((void *)0))) 
    free((domain -> m_ql));
  domain -> m_ql = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_qq) != ((Real_t *)((void *)0))) 
    free((domain -> m_qq));
  domain -> m_qq = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));;
  if ((domain -> m_v) != ((Real_t *)((void *)0))) 
    free((domain -> m_v));
  domain -> m_v = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_volo) != ((Real_t *)((void *)0))) 
    free((domain -> m_volo));
  domain -> m_volo = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_delv) != ((Real_t *)((void *)0))) 
    free((domain -> m_delv));
  domain -> m_delv = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_vdov) != ((Real_t *)((void *)0))) 
    free((domain -> m_vdov));
  domain -> m_vdov = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));;
  if ((domain -> m_arealg) != ((Real_t *)((void *)0))) 
    free((domain -> m_arealg));
  domain -> m_arealg = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_ss) != ((Real_t *)((void *)0))) 
    free((domain -> m_ss));
  domain -> m_ss = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_elemMass) != ((Real_t *)((void *)0))) 
    free((domain -> m_elemMass));
  domain -> m_elemMass = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));;
#if defined(FSIM)
#endif // FSIM
  hc_startFinish(ws);
{
{
      Index_t i_len = hcSize;
      Index_t i_blk = 16;
      for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
        Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
        __hc_frame__ -> pc = 2;
        ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> i_end = i_end;
        ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> i_out = i_out;
        ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> i_len = i_len;
        ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
        ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> hcSize = hcSize;
        struct hc_frameHeader *_async_241_6frame__2 = buildFrame__async_241_6(i_out,i_end,domain,ws);
        _async_241_6frame__2 -> sched_policy = hc_sched_policy(ws);
        if (_async_241_6frame__2 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_241_6frame__2);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_241_6(ws,((struct hc_frameHeader *)_async_241_6frame__2),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async2:;
          i_end = ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> i_end;
          i_out = ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> i_out;
          i_len = ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> i_len;
          i_blk = ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> i_blk;
          hcSize = ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> hcSize;
        }
      }
    };
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> hcSize = hcSize;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    hcSize = ((struct __hc_domain_AllocateElemPersistent__frame_t__ *)__hc_frame__) -> hcSize;
  }
#if defined(FSIM)
#endif // FSIM
;
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_241_6(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_241_6frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_241_6frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_241_6frame_t__ *)__hc_frame__) -> domain;
  domain_AllocateElemPersistent_edt_1(i_out,i_end,domain);
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}
/* Temporaries should not be initialized in bulk but */
/* this is a runnable placeholder for now */

void domain_AllocateElemTemporary(size_t hcSize)
{;
#if defined(FSIM)
#endif // FSIM
;
  if ((domain -> m_dxx) != ((Real_t *)((void *)0))) 
    free((domain -> m_dxx));
  domain -> m_dxx = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_dyy) != ((Real_t *)((void *)0))) 
    free((domain -> m_dyy));
  domain -> m_dyy = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_dzz) != ((Real_t *)((void *)0))) 
    free((domain -> m_dzz));
  domain -> m_dzz = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));;
  if ((domain -> m_delv_xi) != ((Real_t *)((void *)0))) 
    free((domain -> m_delv_xi));
  domain -> m_delv_xi = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_delv_eta) != ((Real_t *)((void *)0))) 
    free((domain -> m_delv_eta));
  domain -> m_delv_eta = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_delv_zeta) != ((Real_t *)((void *)0))) 
    free((domain -> m_delv_zeta));
  domain -> m_delv_zeta = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));;
  if ((domain -> m_delx_xi) != ((Real_t *)((void *)0))) 
    free((domain -> m_delx_xi));
  domain -> m_delx_xi = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_delx_eta) != ((Real_t *)((void *)0))) 
    free((domain -> m_delx_eta));
  domain -> m_delx_eta = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
  if ((domain -> m_delx_zeta) != ((Real_t *)((void *)0))) 
    free((domain -> m_delx_zeta));
  domain -> m_delx_zeta = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));;
  if ((domain -> m_vnew) != ((Real_t *)((void *)0))) 
    free((domain -> m_vnew));
  domain -> m_vnew = ((Real_t *)(malloc((hcSize * sizeof(Real_t )))));
#if defined(FSIM)
#endif // FSIM
;
}

void domain_AllocateNodesets(size_t hcSize)
{;
#if defined(FSIM)
#endif // FSIM
;
  if ((domain -> m_symmX) != ((Index_t *)((void *)0))) 
    free((domain -> m_symmX));
  domain -> m_symmX = ((Index_t *)(malloc((hcSize * sizeof(Index_t )))));
  if ((domain -> m_symmY) != ((Index_t *)((void *)0))) 
    free((domain -> m_symmY));
  domain -> m_symmY = ((Index_t *)(malloc((hcSize * sizeof(Index_t )))));
  if ((domain -> m_symmZ) != ((Index_t *)((void *)0))) 
    free((domain -> m_symmZ));
  domain -> m_symmZ = ((Index_t *)(malloc((hcSize * sizeof(Index_t )))));
#if defined(FSIM)
#endif // FSIM
;
}
#if !defined(FSIM) && !defined(OCR)
/****************/
/* Deallocation */
/****************/

void domain_DeallocateNodalPersistent()
{;
  if ((domain -> m_x) != ((Real_t *)((void *)0))) 
    free((domain -> m_x));
  domain -> m_x = ((Real_t *)((void *)0));
  if ((domain -> m_y) != ((Real_t *)((void *)0))) 
    free((domain -> m_y));
  domain -> m_y = ((Real_t *)((void *)0));
  if ((domain -> m_z) != ((Real_t *)((void *)0))) 
    free((domain -> m_z));
  domain -> m_z = ((Real_t *)((void *)0));;
  if ((domain -> m_xd) != ((Real_t *)((void *)0))) 
    free((domain -> m_xd));
  domain -> m_xd = ((Real_t *)((void *)0));
  if ((domain -> m_yd) != ((Real_t *)((void *)0))) 
    free((domain -> m_yd));
  domain -> m_yd = ((Real_t *)((void *)0));
  if ((domain -> m_zd) != ((Real_t *)((void *)0))) 
    free((domain -> m_zd));
  domain -> m_zd = ((Real_t *)((void *)0));;
  if ((domain -> m_xdd) != ((Real_t *)((void *)0))) 
    free((domain -> m_xdd));
  domain -> m_xdd = ((Real_t *)((void *)0));
  if ((domain -> m_ydd) != ((Real_t *)((void *)0))) 
    free((domain -> m_ydd));
  domain -> m_ydd = ((Real_t *)((void *)0));
  if ((domain -> m_zdd) != ((Real_t *)((void *)0))) 
    free((domain -> m_zdd));
  domain -> m_zdd = ((Real_t *)((void *)0));;
  if ((domain -> m_fx) != ((Real_t *)((void *)0))) 
    free((domain -> m_fx));
  domain -> m_fx = ((Real_t *)((void *)0));
  if ((domain -> m_fy) != ((Real_t *)((void *)0))) 
    free((domain -> m_fy));
  domain -> m_fy = ((Real_t *)((void *)0));
  if ((domain -> m_fz) != ((Real_t *)((void *)0))) 
    free((domain -> m_fz));
  domain -> m_fz = ((Real_t *)((void *)0));;
  if ((domain -> m_nodalMass) != ((Real_t *)((void *)0))) 
    free((domain -> m_nodalMass));
  domain -> m_nodalMass = ((Real_t *)((void *)0));
}

void domain_DeallocateElemPersistent()
{;
  if ((domain -> m_matElemlist) != ((Index_t *)((void *)0))) 
    free((domain -> m_matElemlist));
  domain -> m_matElemlist = ((Index_t *)((void *)0));
  if ((domain -> m_nodelist) != ((Index_t *)((void *)0))) 
    free((domain -> m_nodelist));
  domain -> m_nodelist = ((Index_t *)((void *)0));;
  if ((domain -> m_lxim) != ((Index_t *)((void *)0))) 
    free((domain -> m_lxim));
  domain -> m_lxim = ((Index_t *)((void *)0));
  if ((domain -> m_lxip) != ((Index_t *)((void *)0))) 
    free((domain -> m_lxip));
  domain -> m_lxip = ((Index_t *)((void *)0));
  if ((domain -> m_letam) != ((Index_t *)((void *)0))) 
    free((domain -> m_letam));
  domain -> m_letam = ((Index_t *)((void *)0));
  if ((domain -> m_letap) != ((Index_t *)((void *)0))) 
    free((domain -> m_letap));
  domain -> m_letap = ((Index_t *)((void *)0));
  if ((domain -> m_lzetam) != ((Index_t *)((void *)0))) 
    free((domain -> m_lzetam));
  domain -> m_lzetam = ((Index_t *)((void *)0));
  if ((domain -> m_lzetap) != ((Index_t *)((void *)0))) 
    free((domain -> m_lzetap));
  domain -> m_lzetap = ((Index_t *)((void *)0));;
  if ((domain -> m_elemBC) != ((Int_t *)((void *)0))) 
    free((domain -> m_elemBC));
  domain -> m_elemBC = ((Int_t *)((void *)0));;
  if ((domain -> m_e) != ((Real_t *)((void *)0))) 
    free((domain -> m_e));
  domain -> m_e = ((Real_t *)((void *)0));
  if ((domain -> m_p) != ((Real_t *)((void *)0))) 
    free((domain -> m_p));
  domain -> m_p = ((Real_t *)((void *)0));
  if ((domain -> m_q) != ((Real_t *)((void *)0))) 
    free((domain -> m_q));
  domain -> m_q = ((Real_t *)((void *)0));
  if ((domain -> m_ql) != ((Real_t *)((void *)0))) 
    free((domain -> m_ql));
  domain -> m_ql = ((Real_t *)((void *)0));
  if ((domain -> m_qq) != ((Real_t *)((void *)0))) 
    free((domain -> m_qq));
  domain -> m_qq = ((Real_t *)((void *)0));;
  if ((domain -> m_v) != ((Real_t *)((void *)0))) 
    free((domain -> m_v));
  domain -> m_v = ((Real_t *)((void *)0));
  if ((domain -> m_volo) != ((Real_t *)((void *)0))) 
    free((domain -> m_volo));
  domain -> m_volo = ((Real_t *)((void *)0));
  if ((domain -> m_delv) != ((Real_t *)((void *)0))) 
    free((domain -> m_delv));
  domain -> m_delv = ((Real_t *)((void *)0));
  if ((domain -> m_vdov) != ((Real_t *)((void *)0))) 
    free((domain -> m_vdov));
  domain -> m_vdov = ((Real_t *)((void *)0));;
  if ((domain -> m_arealg) != ((Real_t *)((void *)0))) 
    free((domain -> m_arealg));
  domain -> m_arealg = ((Real_t *)((void *)0));
  if ((domain -> m_ss) != ((Real_t *)((void *)0))) 
    free((domain -> m_ss));
  domain -> m_ss = ((Real_t *)((void *)0));
  if ((domain -> m_elemMass) != ((Real_t *)((void *)0))) 
    free((domain -> m_elemMass));
  domain -> m_elemMass = ((Real_t *)((void *)0));
}
/* Temporaries should not be initialized in bulk but */
/* this is a runnable placeholder for now */

void domain_DeallocateElemTemporary()
{;
  if ((domain -> m_dxx) != ((Real_t *)((void *)0))) 
    free((domain -> m_dxx));
  domain -> m_dxx = ((Real_t *)((void *)0));
  if ((domain -> m_dyy) != ((Real_t *)((void *)0))) 
    free((domain -> m_dyy));
  domain -> m_dyy = ((Real_t *)((void *)0));
  if ((domain -> m_dzz) != ((Real_t *)((void *)0))) 
    free((domain -> m_dzz));
  domain -> m_dzz = ((Real_t *)((void *)0));;
  if ((domain -> m_delv_xi) != ((Real_t *)((void *)0))) 
    free((domain -> m_delv_xi));
  domain -> m_delv_xi = ((Real_t *)((void *)0));
  if ((domain -> m_delv_eta) != ((Real_t *)((void *)0))) 
    free((domain -> m_delv_eta));
  domain -> m_delv_eta = ((Real_t *)((void *)0));
  if ((domain -> m_delv_zeta) != ((Real_t *)((void *)0))) 
    free((domain -> m_delv_zeta));
  domain -> m_delv_zeta = ((Real_t *)((void *)0));;
  if ((domain -> m_delx_xi) != ((Real_t *)((void *)0))) 
    free((domain -> m_delx_xi));
  domain -> m_delx_xi = ((Real_t *)((void *)0));
  if ((domain -> m_delx_eta) != ((Real_t *)((void *)0))) 
    free((domain -> m_delx_eta));
  domain -> m_delx_eta = ((Real_t *)((void *)0));
  if ((domain -> m_delx_zeta) != ((Real_t *)((void *)0))) 
    free((domain -> m_delx_zeta));
  domain -> m_delx_zeta = ((Real_t *)((void *)0));;
  if ((domain -> m_vnew) != ((Real_t *)((void *)0))) 
    free((domain -> m_vnew));
  domain -> m_vnew = ((Real_t *)((void *)0));
}

void domain_DeallocateNodesets()
{;
  if ((domain -> m_symmX) != ((Index_t *)((void *)0))) 
    free((domain -> m_symmX));
  domain -> m_symmX = ((Index_t *)((void *)0));
  if ((domain -> m_symmY) != ((Index_t *)((void *)0))) 
    free((domain -> m_symmY));
  domain -> m_symmY = ((Index_t *)((void *)0));
  if ((domain -> m_symmZ) != ((Index_t *)((void *)0))) 
    free((domain -> m_symmZ));
  domain -> m_symmZ = ((Index_t *)((void *)0));
}
#endif //NOT FSIM and NOT OCR
/* Stuff needed for boundary conditions */
/* 2 BCs on each of 6 hexahedral faces (12 bits) */
#define XI_M        0x003
#define XI_M_SYMM   0x001
#define XI_M_FREE   0x002
#define XI_P        0x00c
#define XI_P_SYMM   0x004
#define XI_P_FREE   0x008
#define ETA_M       0x030
#define ETA_M_SYMM  0x010
#define ETA_M_FREE  0x020
#define ETA_P       0x0c0
#define ETA_P_SYMM  0x040
#define ETA_P_FREE  0x080
#define ZETA_M      0x300
#define ZETA_M_SYMM 0x100
#define ZETA_M_FREE 0x200
#define ZETA_P      0xc00
#define ZETA_P_SYMM 0x400
#define ZETA_P_FREE 0x800

struct __hc_InitStressTermsForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
  long numElem;
  Real_t *sigxx;
  Real_t *sigyy;
  Real_t *sigzz;
}
;
void __hc_InitStressTermsForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_InitStressTermsForElems__(long numElem,Real_t *sigxx,Real_t *sigyy,Real_t *sigzz,struct hc_workerState *ws)
{
  struct __hc_InitStressTermsForElems__frame_t__ *__hc_new_frame__ = (struct __hc_InitStressTermsForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_InitStressTermsForElems__frame_t__ ),__hc_InitStressTermsForElems__));
  ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_new_frame__) -> numElem = numElem;
  ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_new_frame__) -> sigxx = sigxx;
  ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_new_frame__) -> sigyy = sigyy;
  ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_new_frame__) -> sigzz = sigzz;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_458_6frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  Real_t *sigxx;
  Real_t *sigyy;
  Real_t *sigzz;
  struct Domain_t *domain;
}
;
void _async_458_6(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_458_6(long i_out,long i_end,Real_t *sigxx,Real_t *sigyy,Real_t *sigzz,struct Domain_t *domain,struct hc_workerState *ws)
{
  struct _async_458_6frame_t__ *__hc_new_frame__ = (struct _async_458_6frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_458_6frame_t__ ),_async_458_6));
  ((struct _async_458_6frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_458_6frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_458_6frame_t__ *)__hc_new_frame__) -> sigxx = sigxx;
  ((struct _async_458_6frame_t__ *)__hc_new_frame__) -> sigyy = sigyy;
  ((struct _async_458_6frame_t__ *)__hc_new_frame__) -> sigzz = sigzz;
  ((struct _async_458_6frame_t__ *)__hc_new_frame__) -> domain = domain;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_InitStressTermsForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long numElem;
  Real_t *sigxx;
  Real_t *sigyy;
  Real_t *sigzz;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  numElem = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> numElem;
  sigxx = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigxx;
  sigyy = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigyy;
  sigzz = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigzz;
//
// pull in the stresses appropriate to the hydro integration
//
  hc_startFinish(ws);
{
{
      Index_t i_len = numElem;
      Index_t i_blk = 16;
      for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
        Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
        __hc_frame__ -> pc = 2;
        ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
        ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
        ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
        ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
        ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
        ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigxx = sigxx;
        ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigyy = sigyy;
        ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigzz = sigzz;
        struct hc_frameHeader *_async_458_6frame__2 = buildFrame__async_458_6(i_out,i_end,sigxx,sigyy,sigzz,domain,ws);
        _async_458_6frame__2 -> sched_policy = hc_sched_policy(ws);
        if (_async_458_6frame__2 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_458_6frame__2);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_458_6(ws,((struct hc_frameHeader *)_async_458_6frame__2),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async2:;
          i_end = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> i_end;
          i_out = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> i_out;
          i_len = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> i_len;
          i_blk = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> i_blk;
          numElem = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> numElem;
          sigxx = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigxx;
          sigyy = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigyy;
          sigzz = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigzz;
        }
      }
    };
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
  ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigxx = sigxx;
  ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigyy = sigyy;
  ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigzz = sigzz;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    numElem = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> numElem;
    sigxx = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigxx;
    sigyy = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigyy;
    sigzz = ((struct __hc_InitStressTermsForElems__frame_t__ *)__hc_frame__) -> sigzz;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_458_6(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  Real_t *sigxx;
  Real_t *sigyy;
  Real_t *sigzz;
  struct Domain_t *domain;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_458_6frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_458_6frame_t__ *)__hc_frame__) -> i_end;
  sigxx = ((struct _async_458_6frame_t__ *)__hc_frame__) -> sigxx;
  sigyy = ((struct _async_458_6frame_t__ *)__hc_frame__) -> sigyy;
  sigzz = ((struct _async_458_6frame_t__ *)__hc_frame__) -> sigzz;
  domain = ((struct _async_458_6frame_t__ *)__hc_frame__) -> domain;
  InitStressTermsForElems_edt_1(i_out,i_end,sigxx,sigyy,sigzz,domain);
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_IntegrateStressForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long k_end;
  long k_out;
  long k_len;
  long k_blk;
  long numElem;
  Real_t *sigxx;
  Real_t *sigyy;
  Real_t *sigzz;
  Real_t *determ;
}
;
void __hc_IntegrateStressForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_IntegrateStressForElems__(long numElem,Real_t *sigxx,Real_t *sigyy,Real_t *sigzz,Real_t *determ,struct hc_workerState *ws)
{
  struct __hc_IntegrateStressForElems__frame_t__ *__hc_new_frame__ = (struct __hc_IntegrateStressForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_IntegrateStressForElems__frame_t__ ),__hc_IntegrateStressForElems__));
  ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_new_frame__) -> numElem = numElem;
  ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_new_frame__) -> sigxx = sigxx;
  ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_new_frame__) -> sigyy = sigyy;
  ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_new_frame__) -> sigzz = sigzz;
  ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_new_frame__) -> determ = determ;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_468_5frame_t__ 
{
  struct hc_frameHeader header;
  long k_out;
  long k_end;
  Real_t *sigxx;
  Real_t *sigyy;
  Real_t *sigzz;
  Real_t *determ;
  struct Domain_t *domain;
}
;
void _async_468_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_468_5(long k_out,long k_end,Real_t *sigxx,Real_t *sigyy,Real_t *sigzz,Real_t *determ,struct Domain_t *domain,struct hc_workerState *ws)
{
  struct _async_468_5frame_t__ *__hc_new_frame__ = (struct _async_468_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_468_5frame_t__ ),_async_468_5));
  ((struct _async_468_5frame_t__ *)__hc_new_frame__) -> k_out = k_out;
  ((struct _async_468_5frame_t__ *)__hc_new_frame__) -> k_end = k_end;
  ((struct _async_468_5frame_t__ *)__hc_new_frame__) -> sigxx = sigxx;
  ((struct _async_468_5frame_t__ *)__hc_new_frame__) -> sigyy = sigyy;
  ((struct _async_468_5frame_t__ *)__hc_new_frame__) -> sigzz = sigzz;
  ((struct _async_468_5frame_t__ *)__hc_new_frame__) -> determ = determ;
  ((struct _async_468_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_IntegrateStressForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long numElem;
  Real_t *sigxx;
  Real_t *sigyy;
  Real_t *sigzz;
  Real_t *determ;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  numElem = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> numElem;
  sigxx = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigxx;
  sigyy = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigyy;
  sigzz = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigzz;
  determ = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> determ;
// loop over all elements
  hc_startFinish(ws);
{
{
      Index_t k_len = numElem;
      Index_t k_blk = 16;
      for (Index_t k_out = 0; k_out < k_len; k_out += k_blk) {
        Index_t k_end = ((k_out + k_blk) < k_len)?(k_out + k_blk) : k_len;
        __hc_frame__ -> pc = 2;
        ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> k_end = k_end;
        ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> k_out = k_out;
        ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> k_len = k_len;
        ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> k_blk = k_blk;
        ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
        ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigxx = sigxx;
        ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigyy = sigyy;
        ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigzz = sigzz;
        ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> determ = determ;
        struct hc_frameHeader *_async_468_5frame__2 = buildFrame__async_468_5(k_out,k_end,sigxx,sigyy,sigzz,determ,domain,ws);
        _async_468_5frame__2 -> sched_policy = hc_sched_policy(ws);
        if (_async_468_5frame__2 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_468_5frame__2);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_468_5(ws,((struct hc_frameHeader *)_async_468_5frame__2),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async2:;
          k_end = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> k_end;
          k_out = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> k_out;
          k_len = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> k_len;
          k_blk = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> k_blk;
          numElem = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> numElem;
          sigxx = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigxx;
          sigyy = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigyy;
          sigzz = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigzz;
          determ = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> determ;
        }
      }
    };
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
  ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigxx = sigxx;
  ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigyy = sigyy;
  ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigzz = sigzz;
  ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> determ = determ;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    numElem = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> numElem;
    sigxx = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigxx;
    sigyy = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigyy;
    sigzz = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> sigzz;
    determ = ((struct __hc_IntegrateStressForElems__frame_t__ *)__hc_frame__) -> determ;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_468_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long k_out;
  long k_end;
  Real_t *sigxx;
  Real_t *sigyy;
  Real_t *sigzz;
  Real_t *determ;
  struct Domain_t *domain;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  k_out = ((struct _async_468_5frame_t__ *)__hc_frame__) -> k_out;
  k_end = ((struct _async_468_5frame_t__ *)__hc_frame__) -> k_end;
  sigxx = ((struct _async_468_5frame_t__ *)__hc_frame__) -> sigxx;
  sigyy = ((struct _async_468_5frame_t__ *)__hc_frame__) -> sigyy;
  sigzz = ((struct _async_468_5frame_t__ *)__hc_frame__) -> sigzz;
  determ = ((struct _async_468_5frame_t__ *)__hc_frame__) -> determ;
  domain = ((struct _async_468_5frame_t__ *)__hc_frame__) -> domain;
  IntegrateStressForElems_edt_1(k_out,k_end,sigxx,sigyy,sigzz,determ,domain);
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}
// RAG -- GAMMA now compile time constants in rodata
static const Real_t GAMMA[((Index_t )4) * ((Index_t )8)] = {(1.0), (1.0), ((-1.0)), ((-1.0)), ((-1.0)), ((-1.0)), (1.0), (1.0), (1.0), ((-1.0)), ((-1.0)), (1.0), ((-1.0)), (1.0), (1.0), ((-1.0)), (1.0), ((-1.0)), (1.0), ((-1.0)), (1.0), ((-1.0)), (1.0), ((-1.0)), ((-1.0)), (1.0), ((-1.0)), (1.0), (1.0), ((-1.0)), (1.0), ((-1.0))};

struct __hc_CalcFBHourglassForceForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long numElem;
  long i2_end;
  long i2_out;
  long i2_len;
  long i2_blk;
  Real_t *determ;
  Real_t *x8n;
  Real_t *y8n;
  Real_t *z8n;
  Real_t *dvdx;
  Real_t *dvdy;
  Real_t *dvdz;
  double hourg;
}
;
void __hc_CalcFBHourglassForceForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcFBHourglassForceForElems__(Real_t *determ,Real_t *x8n,Real_t *y8n,Real_t *z8n,Real_t *dvdx,Real_t *dvdy,Real_t *dvdz,double hourg,struct hc_workerState *ws)
{
  struct __hc_CalcFBHourglassForceForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcFBHourglassForceForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcFBHourglassForceForElems__frame_t__ ),__hc_CalcFBHourglassForceForElems__));
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_new_frame__) -> determ = determ;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_new_frame__) -> x8n = x8n;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_new_frame__) -> y8n = y8n;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_new_frame__) -> z8n = z8n;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_new_frame__) -> dvdx = dvdx;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_new_frame__) -> dvdy = dvdy;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_new_frame__) -> dvdz = dvdz;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_new_frame__) -> hourg = hourg;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_499_5frame_t__ 
{
  struct hc_frameHeader header;
  long i2_out;
  long i2_end;
  struct Domain_t *domain;
  double *GAMMA;
  Real_t *determ;
  Real_t *x8n;
  Real_t *y8n;
  Real_t *z8n;
  Real_t *dvdx;
  Real_t *dvdy;
  Real_t *dvdz;
  double hourg;
}
;
void _async_499_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_499_5(long i2_out,long i2_end,struct Domain_t *domain,double *GAMMA,Real_t *determ,Real_t *x8n,Real_t *y8n,Real_t *z8n,Real_t *dvdx,Real_t *dvdy,Real_t *dvdz,double hourg,struct hc_workerState *ws)
{
  struct _async_499_5frame_t__ *__hc_new_frame__ = (struct _async_499_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_499_5frame_t__ ),_async_499_5));
  ((struct _async_499_5frame_t__ *)__hc_new_frame__) -> i2_out = i2_out;
  ((struct _async_499_5frame_t__ *)__hc_new_frame__) -> i2_end = i2_end;
  ((struct _async_499_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_499_5frame_t__ *)__hc_new_frame__) -> GAMMA = GAMMA;
  ((struct _async_499_5frame_t__ *)__hc_new_frame__) -> determ = determ;
  ((struct _async_499_5frame_t__ *)__hc_new_frame__) -> x8n = x8n;
  ((struct _async_499_5frame_t__ *)__hc_new_frame__) -> y8n = y8n;
  ((struct _async_499_5frame_t__ *)__hc_new_frame__) -> z8n = z8n;
  ((struct _async_499_5frame_t__ *)__hc_new_frame__) -> dvdx = dvdx;
  ((struct _async_499_5frame_t__ *)__hc_new_frame__) -> dvdy = dvdy;
  ((struct _async_499_5frame_t__ *)__hc_new_frame__) -> dvdz = dvdz;
  ((struct _async_499_5frame_t__ *)__hc_new_frame__) -> hourg = hourg;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcFBHourglassForceForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  Real_t *determ;
  Real_t *x8n;
  Real_t *y8n;
  Real_t *z8n;
  Real_t *dvdx;
  Real_t *dvdy;
  Real_t *dvdz;
  double hourg;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  determ = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> determ;
  x8n = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> x8n;
  y8n = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> y8n;
  z8n = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> z8n;
  dvdx = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdx;
  dvdy = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdy;
  dvdz = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdz;
  hourg = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> hourg;
/*************************************************
    *
    *     FUNCTION: Calculates the Flanagan-Belytschko anti-hourglass
    *               force.
    *
    *************************************************/
  Index_t numElem = (domain -> m_numElem);
/*************************************************/
/*    compute the hourglass modes */
  hc_startFinish(ws);
{
    Index_t i2_len = numElem;
    Index_t i2_blk = 16;
    for (Index_t i2_out = 0; i2_out < i2_len; i2_out += i2_blk) {
      Index_t i2_end = ((i2_out + i2_blk) < i2_len)?(i2_out + i2_blk) : i2_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> i2_end = i2_end;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> i2_out = i2_out;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> i2_len = i2_len;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> i2_blk = i2_blk;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> determ = determ;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> x8n = x8n;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> y8n = y8n;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> z8n = z8n;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdx = dvdx;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdy = dvdy;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdz = dvdz;
      ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> hourg = hourg;
      struct hc_frameHeader *_async_499_5frame__2 = buildFrame__async_499_5(i2_out,i2_end,domain,GAMMA,determ,x8n,y8n,z8n,dvdx,dvdy,dvdz,hourg,ws);
      _async_499_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_499_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_499_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_499_5(ws,((struct hc_frameHeader *)_async_499_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i2_end = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> i2_end;
        i2_out = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> i2_out;
        i2_len = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> i2_len;
        i2_blk = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> i2_blk;
        numElem = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> numElem;
        determ = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> determ;
        x8n = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> x8n;
        y8n = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> y8n;
        z8n = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> z8n;
        dvdx = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdx;
        dvdy = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdy;
        dvdz = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdz;
        hourg = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> hourg;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> determ = determ;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> x8n = x8n;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> y8n = y8n;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> z8n = z8n;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdx = dvdx;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdy = dvdy;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdz = dvdz;
  ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> hourg = hourg;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    numElem = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> numElem;
    determ = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> determ;
    x8n = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> x8n;
    y8n = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> y8n;
    z8n = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> z8n;
    dvdx = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdx;
    dvdy = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdy;
    dvdz = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> dvdz;
    hourg = ((struct __hc_CalcFBHourglassForceForElems__frame_t__ *)__hc_frame__) -> hourg;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_499_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i2_out;
  long i2_end;
  struct Domain_t *domain;
  double *GAMMA;
  Real_t *determ;
  Real_t *x8n;
  Real_t *y8n;
  Real_t *z8n;
  Real_t *dvdx;
  Real_t *dvdy;
  Real_t *dvdz;
  double hourg;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i2_out = ((struct _async_499_5frame_t__ *)__hc_frame__) -> i2_out;
  i2_end = ((struct _async_499_5frame_t__ *)__hc_frame__) -> i2_end;
  domain = ((struct _async_499_5frame_t__ *)__hc_frame__) -> domain;
  GAMMA = ((struct _async_499_5frame_t__ *)__hc_frame__) -> GAMMA;
  determ = ((struct _async_499_5frame_t__ *)__hc_frame__) -> determ;
  x8n = ((struct _async_499_5frame_t__ *)__hc_frame__) -> x8n;
  y8n = ((struct _async_499_5frame_t__ *)__hc_frame__) -> y8n;
  z8n = ((struct _async_499_5frame_t__ *)__hc_frame__) -> z8n;
  dvdx = ((struct _async_499_5frame_t__ *)__hc_frame__) -> dvdx;
  dvdy = ((struct _async_499_5frame_t__ *)__hc_frame__) -> dvdy;
  dvdz = ((struct _async_499_5frame_t__ *)__hc_frame__) -> dvdz;
  hourg = ((struct _async_499_5frame_t__ *)__hc_frame__) -> hourg;
  for (Index_t i2 = i2_out; i2 < i2_end; ++i2) {
#if  defined(HAB_C)
    Real_t *hgfx = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *hgfy = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *hgfz = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *hourgam0 = (Real_t *)(malloc((((Index_t )4) * sizeof(Real_t ))));
    Real_t *hourgam1 = (Real_t *)(malloc((((Index_t )4) * sizeof(Real_t ))));
    Real_t *hourgam2 = (Real_t *)(malloc((((Index_t )4) * sizeof(Real_t ))));
    Real_t *hourgam3 = (Real_t *)(malloc((((Index_t )4) * sizeof(Real_t ))));
    Real_t *hourgam4 = (Real_t *)(malloc((((Index_t )4) * sizeof(Real_t ))));
    Real_t *hourgam5 = (Real_t *)(malloc((((Index_t )4) * sizeof(Real_t ))));
    Real_t *hourgam6 = (Real_t *)(malloc((((Index_t )4) * sizeof(Real_t ))));
    Real_t *hourgam7 = (Real_t *)(malloc((((Index_t )4) * sizeof(Real_t ))));
    Real_t *xd1 = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *yd1 = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *zd1 = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Index_t *elemToNode = (Index_t *)(malloc((((Index_t )8) * sizeof(Index_t ))));
#else // NOT HAB_C
#endif //    HAB_C
    Real_t coefficient;
    Index_t i3 = (((Index_t )8) * i2);
    Real_t volinv = (((Real_t )1.0) / determ[i2]);
    Real_t ss1;
    Real_t mass1;
    Real_t volume13;
// RAG -- GATHER/SCATTER Index Values
    for (Index_t i = 0; i < ((Index_t )8); ++i) {
      elemToNode[i] = (domain -> m_nodelist)[(((Index_t )8) * i2) + i];
// for i
    }
    for (Index_t i1 = 0; i1 < ((Index_t )4); ++i1) {
      Real_t hourmodx = ((((((((x8n[i3 + 0] * GAMMA[(i1 * ((Index_t )8)) + 0]) + (x8n[i3 + 1] * GAMMA[(i1 * ((Index_t )8)) + 1])) + (x8n[i3 + 2] * GAMMA[(i1 * ((Index_t )8)) + 2])) + (x8n[i3 + 3] * GAMMA[(i1 * ((Index_t )8)) + 3])) + (x8n[i3 + 4] * GAMMA[(i1 * ((Index_t )8)) + 4])) + (x8n[i3 + 5] * GAMMA[(i1 * ((Index_t )8)) + 5])) + (x8n[i3 + 6] * GAMMA[(i1 * ((Index_t )8)) + 6])) + (x8n[i3 + 7] * GAMMA[(i1 * ((Index_t )8)) + 7]));
      Real_t hourmody = ((((((((y8n[i3 + 0] * GAMMA[(i1 * ((Index_t )8)) + 0]) + (y8n[i3 + 1] * GAMMA[(i1 * ((Index_t )8)) + 1])) + (y8n[i3 + 2] * GAMMA[(i1 * ((Index_t )8)) + 2])) + (y8n[i3 + 3] * GAMMA[(i1 * ((Index_t )8)) + 3])) + (y8n[i3 + 4] * GAMMA[(i1 * ((Index_t )8)) + 4])) + (y8n[i3 + 5] * GAMMA[(i1 * ((Index_t )8)) + 5])) + (y8n[i3 + 6] * GAMMA[(i1 * ((Index_t )8)) + 6])) + (y8n[i3 + 7] * GAMMA[(i1 * ((Index_t )8)) + 7]));
      Real_t hourmodz = ((((((((z8n[i3 + 0] * GAMMA[(i1 * ((Index_t )8)) + 0]) + (z8n[i3 + 1] * GAMMA[(i1 * ((Index_t )8)) + 1])) + (z8n[i3 + 2] * GAMMA[(i1 * ((Index_t )8)) + 2])) + (z8n[i3 + 3] * GAMMA[(i1 * ((Index_t )8)) + 3])) + (z8n[i3 + 4] * GAMMA[(i1 * ((Index_t )8)) + 4])) + (z8n[i3 + 5] * GAMMA[(i1 * ((Index_t )8)) + 5])) + (z8n[i3 + 6] * GAMMA[(i1 * ((Index_t )8)) + 6])) + (z8n[i3 + 7] * GAMMA[(i1 * ((Index_t )8)) + 7]));
      hourgam0[i1] = (GAMMA[(i1 * ((Index_t )8)) + 0] - (volinv * (((dvdx[i3] * hourmodx) + (dvdy[i3] * hourmody)) + (dvdz[i3] * hourmodz))));
      hourgam1[i1] = (GAMMA[(i1 * ((Index_t )8)) + 1] - (volinv * (((dvdx[i3 + 1] * hourmodx) + (dvdy[i3 + 1] * hourmody)) + (dvdz[i3 + 1] * hourmodz))));
      hourgam2[i1] = (GAMMA[(i1 * ((Index_t )8)) + 2] - (volinv * (((dvdx[i3 + 2] * hourmodx) + (dvdy[i3 + 2] * hourmody)) + (dvdz[i3 + 2] * hourmodz))));
      hourgam3[i1] = (GAMMA[(i1 * ((Index_t )8)) + 3] - (volinv * (((dvdx[i3 + 3] * hourmodx) + (dvdy[i3 + 3] * hourmody)) + (dvdz[i3 + 3] * hourmodz))));
      hourgam4[i1] = (GAMMA[(i1 * ((Index_t )8)) + 4] - (volinv * (((dvdx[i3 + 4] * hourmodx) + (dvdy[i3 + 4] * hourmody)) + (dvdz[i3 + 4] * hourmodz))));
      hourgam5[i1] = (GAMMA[(i1 * ((Index_t )8)) + 5] - (volinv * (((dvdx[i3 + 5] * hourmodx) + (dvdy[i3 + 5] * hourmody)) + (dvdz[i3 + 5] * hourmodz))));
      hourgam6[i1] = (GAMMA[(i1 * ((Index_t )8)) + 6] - (volinv * (((dvdx[i3 + 6] * hourmodx) + (dvdy[i3 + 6] * hourmody)) + (dvdz[i3 + 6] * hourmodz))));
      hourgam7[i1] = (GAMMA[(i1 * ((Index_t )8)) + 7] - (volinv * (((dvdx[i3 + 7] * hourmodx) + (dvdy[i3 + 7] * hourmody)) + (dvdz[i3 + 7] * hourmodz))));
// for i1
    }
/* compute forces */
/* store forces into h arrays (force arrays) */
    ss1 = (domain -> m_ss)[i2];
    mass1 = (domain -> m_elemMass)[i2];
    volume13 = cbrt(determ[i2]);
// RAG ///////////////////////////////////////////////////////////// RAG //
// RAG  GATHER (x|y|z)d1[0,,7] = domain->m_(x|y|z)[elemToNode[0..7]]  RAG //
// RAG ///////////////////////////////////////////////////////////// RAG //
    for (Index_t i = 0; i < ((Index_t )8); ++i) {
      Index_t gnode = elemToNode[i];
      xd1[i] = (domain -> m_xd)[gnode];
      yd1[i] = (domain -> m_yd)[gnode];
      zd1[i] = (domain -> m_zd)[gnode];
// for i
    }
    coefficient = ((((-hourg * ((Real_t )0.01)) * ss1) * mass1) / volume13);
    CalcElemFBHourglassForce(xd1,yd1,zd1,hourgam0,hourgam1,hourgam2,hourgam3,hourgam4,hourgam5,hourgam6,hourgam7,coefficient,hgfx,hgfy,hgfz);
// RAG ///////////////////////////////////////////////////////// RAG //
// RAG  Atomic Memory Floating-point Addition Scatter operation  RAG //
// RAG ///////////////////////////////////////////////////////// RAG //
    for (Index_t i = 0; i < ((Index_t )8); ++i) {
      Index_t gnode = elemToNode[i];
      AMO__sync_addition_double(((domain -> m_fx) + gnode),hgfx[i]);
      AMO__sync_addition_double(((domain -> m_fy) + gnode),hgfy[i]);
      AMO__sync_addition_double(((domain -> m_fz) + gnode),hgfz[i]);
// for i
    }
#if  defined(HAB_C)
    free(elemToNode);
    free(hgfz);
    free(hgfy);
    free(hgfx);
    free(hourgam7);
    free(hourgam6);
    free(hourgam5);
    free(hourgam4);
    free(hourgam3);
    free(hourgam2);
    free(hourgam1);
    free(hourgam0);
    free(zd1);
    free(yd1);
    free(xd1);
#endif //    HAB_C
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcHourglassControlForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long numElem;
  long numElem8;
  Real_t *dvdx;
  Real_t *dvdy;
  Real_t *dvdz;
  Real_t *x8n;
  Real_t *y8n;
  Real_t *z8n;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
  double *determ;
  double hgcoef;
}
;
void __hc_CalcHourglassControlForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcHourglassControlForElems__(double *determ,double hgcoef,struct hc_workerState *ws)
{
  struct __hc_CalcHourglassControlForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcHourglassControlForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcHourglassControlForElems__frame_t__ ),__hc_CalcHourglassControlForElems__));
  ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_new_frame__) -> determ = determ;
  ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_new_frame__) -> hgcoef = hgcoef;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_659_6frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double *determ;
  Real_t *dvdx;
  Real_t *dvdy;
  Real_t *dvdz;
  Real_t *x8n;
  Real_t *y8n;
  Real_t *z8n;
}
;
void _async_659_6(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_659_6(long i_out,long i_end,struct Domain_t *domain,double *determ,Real_t *dvdx,Real_t *dvdy,Real_t *dvdz,Real_t *x8n,Real_t *y8n,Real_t *z8n,struct hc_workerState *ws)
{
  struct _async_659_6frame_t__ *__hc_new_frame__ = (struct _async_659_6frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_659_6frame_t__ ),_async_659_6));
  ((struct _async_659_6frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_659_6frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_659_6frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_659_6frame_t__ *)__hc_new_frame__) -> determ = determ;
  ((struct _async_659_6frame_t__ *)__hc_new_frame__) -> dvdx = dvdx;
  ((struct _async_659_6frame_t__ *)__hc_new_frame__) -> dvdy = dvdy;
  ((struct _async_659_6frame_t__ *)__hc_new_frame__) -> dvdz = dvdz;
  ((struct _async_659_6frame_t__ *)__hc_new_frame__) -> x8n = x8n;
  ((struct _async_659_6frame_t__ *)__hc_new_frame__) -> y8n = y8n;
  ((struct _async_659_6frame_t__ *)__hc_new_frame__) -> z8n = z8n;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcHourglassControlForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  double *determ;
  double hgcoef;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
    case 3:
    goto __post_func3;
  }
  determ = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> determ;
  hgcoef = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> hgcoef;;
  Index_t numElem = (domain -> m_numElem);
  Index_t numElem8 = (numElem * ((Index_t )8));
  Real_t *dvdx = Allocate_Real_t(numElem8);
  Real_t *dvdy = Allocate_Real_t(numElem8);
  Real_t *dvdz = Allocate_Real_t(numElem8);
  Real_t *x8n = Allocate_Real_t(numElem8);
  Real_t *y8n = Allocate_Real_t(numElem8);
  Real_t *z8n = Allocate_Real_t(numElem8);;;
  hc_startFinish(ws);
{
    Index_t i_len = numElem;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> numElem8 = numElem8;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdx = dvdx;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdy = dvdy;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdz = dvdz;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> x8n = x8n;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> y8n = y8n;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> z8n = z8n;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> determ = determ;
      ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> hgcoef = hgcoef;
      struct hc_frameHeader *_async_659_6frame__2 = buildFrame__async_659_6(i_out,i_end,domain,determ,dvdx,dvdy,dvdz,x8n,y8n,z8n,ws);
      _async_659_6frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_659_6frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_659_6frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_659_6(ws,((struct hc_frameHeader *)_async_659_6frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> i_blk;
        numElem = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> numElem;
        numElem8 = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> numElem8;
        dvdx = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdx;
        dvdy = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdy;
        dvdz = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdz;
        x8n = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> x8n;
        y8n = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> y8n;
        z8n = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> z8n;
        determ = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> determ;
        hgcoef = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> hgcoef;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
  ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> numElem8 = numElem8;
  ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdx = dvdx;
  ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdy = dvdy;
  ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdz = dvdz;
  ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> x8n = x8n;
  ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> y8n = y8n;
  ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> z8n = z8n;
  ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> determ = determ;
  ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> hgcoef = hgcoef;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    numElem = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> numElem;
    numElem8 = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> numElem8;
    dvdx = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdx;
    dvdy = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdy;
    dvdz = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdz;
    x8n = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> x8n;
    y8n = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> y8n;
    z8n = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> z8n;
    determ = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> determ;
    hgcoef = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> hgcoef;
  }
  if (hgcoef > ((Real_t )0.0)) {;
    __hc_frame__ -> pc = 3;
    ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
    ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> numElem8 = numElem8;
    ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdx = dvdx;
    ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdy = dvdy;
    ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdz = dvdz;
    ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> x8n = x8n;
    ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> y8n = y8n;
    ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> z8n = z8n;
    ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> determ = determ;
    ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> hgcoef = hgcoef;
    struct hc_frameHeader *__hc_CalcFBHourglassForceForElems__frame__3 = buildFrame___hc_CalcFBHourglassForceForElems__(determ,x8n,y8n,z8n,dvdx,dvdy,dvdz,hgcoef,ws);
    __hc_CalcFBHourglassForceForElems__(ws,((struct hc_frameHeader *)__hc_CalcFBHourglassForceForElems__frame__3),0);
    if (ws -> costatus == HC_FUNC_COMPLETE) {
      hc_free_unwind_frame(ws);
    }
    else {
      return ;
    }
    if (0) {
      __post_func3:;
      __hc_frame__ = hc_free_unwind_frame(ws);
      numElem = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> numElem;
      numElem8 = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> numElem8;
      dvdx = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdx;
      dvdy = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdy;
      dvdz = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> dvdz;
      x8n = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> x8n;
      y8n = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> y8n;
      z8n = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> z8n;
      determ = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> determ;
      hgcoef = ((struct __hc_CalcHourglassControlForElems__frame_t__ *)__hc_frame__) -> hgcoef;
    }
  };
  Release_Real_t(z8n);
  Release_Real_t(y8n);
  Release_Real_t(x8n);
  Release_Real_t(dvdz);
  Release_Real_t(dvdy);
  Release_Real_t(dvdx);;
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_659_6(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double *determ;
  Real_t *dvdx;
  Real_t *dvdy;
  Real_t *dvdz;
  Real_t *x8n;
  Real_t *y8n;
  Real_t *z8n;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_659_6frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_659_6frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_659_6frame_t__ *)__hc_frame__) -> domain;
  determ = ((struct _async_659_6frame_t__ *)__hc_frame__) -> determ;
  dvdx = ((struct _async_659_6frame_t__ *)__hc_frame__) -> dvdx;
  dvdy = ((struct _async_659_6frame_t__ *)__hc_frame__) -> dvdy;
  dvdz = ((struct _async_659_6frame_t__ *)__hc_frame__) -> dvdz;
  x8n = ((struct _async_659_6frame_t__ *)__hc_frame__) -> x8n;
  y8n = ((struct _async_659_6frame_t__ *)__hc_frame__) -> y8n;
  z8n = ((struct _async_659_6frame_t__ *)__hc_frame__) -> z8n;
  for (Index_t i = i_out; i < i_end; ++i) {
#if  defined(HAB_C)
    Real_t *x1 = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *y1 = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *z1 = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *pfx = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *pfy = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *pfz = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
#else // NOT HAB_C
#endif //    HAB_C
    Index_t *elemToNode = ((domain -> m_nodelist) + (((Index_t )8) * i));
// RAG -- GATHER (x|y|z)1[0..7] = domain->m_(x|y|z)[domain->m_nodelist[0..7]]
    CollectDomainNodesToElemNodes(elemToNode,x1,y1,z1);
    CalcElemVolumeDerivative(pfx,pfy,pfz,x1,y1,z1);
//TRACE6("/* load into temporary storage for FB Hour Glass control */");
    for (Index_t ii = 0; ii < ((Index_t )8); ++ii) {
      Index_t jj = ((((Index_t )8) * i) + ii);
      dvdx[jj] = pfx[ii];
      dvdy[jj] = pfy[ii];
      dvdz[jj] = pfz[ii];
      x8n[jj] = x1[ii];
      y8n[jj] = y1[ii];
      z8n[jj] = z1[ii];
// for ii
    }
    determ[i] = ((domain -> m_volo)[i] * (domain -> m_v)[i]);
//TRACE6("/* Do a check for negative volumes */");
    if ((domain -> m_v)[i] <= ((Real_t )0.0)) {
      exit(VolumeError);
    }
#if  defined(HAB_C)
    free(pfz);
    free(pfy);
    free(pfx);
    free(z1);
    free(y1);
    free(x1);
#endif //    HAB_C
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcVolumeForceForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  double hgcoef;
  Real_t *sigxx;
  Real_t *sigyy;
  Real_t *sigzz;
  Real_t *determ;
  long numElem;
  long k_end;
  long k_out;
  long k_len;
  long k_blk;
}
;
void __hc_CalcVolumeForceForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcVolumeForceForElems__(struct hc_workerState *ws)
{
  struct __hc_CalcVolumeForceForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcVolumeForceForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcVolumeForceForElems__frame_t__ ),__hc_CalcVolumeForceForElems__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_744_7frame_t__ 
{
  struct hc_frameHeader header;
  long k_out;
  long k_end;
  Real_t *determ;
}
;
void _async_744_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_744_7(long k_out,long k_end,Real_t *determ,struct hc_workerState *ws)
{
  struct _async_744_7frame_t__ *__hc_new_frame__ = (struct _async_744_7frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_744_7frame_t__ ),_async_744_7));
  ((struct _async_744_7frame_t__ *)__hc_new_frame__) -> k_out = k_out;
  ((struct _async_744_7frame_t__ *)__hc_new_frame__) -> k_end = k_end;
  ((struct _async_744_7frame_t__ *)__hc_new_frame__) -> determ = determ;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcVolumeForceForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
    case 2:
    goto __post_func2;
    case 3:
    goto __post_finish3;
    case 4:
    goto __post_async4;
    case 5:
    goto __post_func5;
  };
  Index_t numElem = (domain -> m_numElem);
  if (numElem != 0) {
    Real_t hgcoef = (domain -> m_hgcoef);
    Real_t *sigxx = Allocate_Real_t(numElem);
    Real_t *sigyy = Allocate_Real_t(numElem);
    Real_t *sigzz = Allocate_Real_t(numElem);
    Real_t *determ = Allocate_Real_t(numElem);;;
    __hc_frame__ -> pc = 1;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> hgcoef = hgcoef;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigxx = sigxx;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigyy = sigyy;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigzz = sigzz;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> determ = determ;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
    struct hc_frameHeader *__hc_InitStressTermsForElems__frame__1 = buildFrame___hc_InitStressTermsForElems__(numElem,sigxx,sigyy,sigzz,ws);
    __hc_InitStressTermsForElems__(ws,((struct hc_frameHeader *)__hc_InitStressTermsForElems__frame__1),0);
    if (ws -> costatus == HC_FUNC_COMPLETE) {
      hc_free_unwind_frame(ws);
    }
    else {
      return ;
    }
    if (0) {
      __post_func1:;
      __hc_frame__ = hc_free_unwind_frame(ws);
      hgcoef = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> hgcoef;
      sigxx = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigxx;
      sigyy = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigyy;
      sigzz = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigzz;
      determ = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> determ;
      numElem = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> numElem;
    };;
    __hc_frame__ -> pc = 2;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> hgcoef = hgcoef;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigxx = sigxx;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigyy = sigyy;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigzz = sigzz;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> determ = determ;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
    struct hc_frameHeader *__hc_IntegrateStressForElems__frame__2 = buildFrame___hc_IntegrateStressForElems__(numElem,sigxx,sigyy,sigzz,determ,ws);
    __hc_IntegrateStressForElems__(ws,((struct hc_frameHeader *)__hc_IntegrateStressForElems__frame__2),0);
    if (ws -> costatus == HC_FUNC_COMPLETE) {
      hc_free_unwind_frame(ws);
    }
    else {
      return ;
    }
    if (0) {
      __post_func2:;
      __hc_frame__ = hc_free_unwind_frame(ws);
      hgcoef = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> hgcoef;
      sigxx = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigxx;
      sigyy = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigyy;
      sigzz = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigzz;
      determ = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> determ;
      numElem = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> numElem;
    };
    hc_startFinish(ws);
{
      Index_t k_len = numElem;
      Index_t k_blk = 16;
      for (Index_t k_out = 0; k_out < k_len; k_out += k_blk) {
        Index_t k_end = ((k_out + k_blk) < k_len)?(k_out + k_blk) : k_len;
        __hc_frame__ -> pc = 4;
        ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> k_end = k_end;
        ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> k_out = k_out;
        ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> k_len = k_len;
        ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> k_blk = k_blk;
        ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> hgcoef = hgcoef;
        ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigxx = sigxx;
        ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigyy = sigyy;
        ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigzz = sigzz;
        ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> determ = determ;
        ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
        struct hc_frameHeader *_async_744_7frame__4 = buildFrame__async_744_7(k_out,k_end,determ,ws);
        _async_744_7frame__4 -> sched_policy = hc_sched_policy(ws);
        if (_async_744_7frame__4 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_744_7frame__4);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_744_7(ws,((struct hc_frameHeader *)_async_744_7frame__4),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async4:;
          k_end = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> k_end;
          k_out = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> k_out;
          k_len = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> k_len;
          k_blk = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> k_blk;
          hgcoef = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> hgcoef;
          sigxx = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigxx;
          sigyy = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigyy;
          sigzz = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigzz;
          determ = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> determ;
          numElem = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> numElem;
        }
      }
    }
    __hc_frame__ -> pc = 3;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> hgcoef = hgcoef;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigxx = sigxx;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigyy = sigyy;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigzz = sigzz;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> determ = determ;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
    if (hc_stopFinish(ws)) 
      return ;
    if (0) {
      __post_finish3:;
      hgcoef = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> hgcoef;
      sigxx = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigxx;
      sigyy = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigyy;
      sigzz = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigzz;
      determ = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> determ;
      numElem = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> numElem;
    };
    __hc_frame__ -> pc = 5;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> hgcoef = hgcoef;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigxx = sigxx;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigyy = sigyy;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigzz = sigzz;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> determ = determ;
    ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
    struct hc_frameHeader *__hc_CalcHourglassControlForElems__frame__5 = buildFrame___hc_CalcHourglassControlForElems__(determ,hgcoef,ws);
    __hc_CalcHourglassControlForElems__(ws,((struct hc_frameHeader *)__hc_CalcHourglassControlForElems__frame__5),0);
    if (ws -> costatus == HC_FUNC_COMPLETE) {
      hc_free_unwind_frame(ws);
    }
    else {
      return ;
    }
    if (0) {
      __post_func5:;
      __hc_frame__ = hc_free_unwind_frame(ws);
      hgcoef = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> hgcoef;
      sigxx = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigxx;
      sigyy = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigyy;
      sigzz = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> sigzz;
      determ = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> determ;
      numElem = ((struct __hc_CalcVolumeForceForElems__frame_t__ *)__hc_frame__) -> numElem;
    };
    Release_Real_t(determ);
    Release_Real_t(sigzz);
    Release_Real_t(sigyy);
    Release_Real_t(sigxx);
// if numElem
  };
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_744_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long k_out;
  long k_end;
  Real_t *determ;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  k_out = ((struct _async_744_7frame_t__ *)__hc_frame__) -> k_out;
  k_end = ((struct _async_744_7frame_t__ *)__hc_frame__) -> k_end;
  determ = ((struct _async_744_7frame_t__ *)__hc_frame__) -> determ;
  for (Index_t k = k_out; k < k_end; ++k) {
    if (determ[k] <= ((Real_t )0.0)) {
      exit(VolumeError);
    }
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcForceForNodes__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long numNode;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
}
;
void __hc_CalcForceForNodes__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcForceForNodes__(struct hc_workerState *ws)
{
  struct __hc_CalcForceForNodes__frame_t__ *__hc_new_frame__ = (struct __hc_CalcForceForNodes__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcForceForNodes__frame_t__ ),__hc_CalcForceForNodes__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_767_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
}
;
void _async_767_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_767_5(long i_out,long i_end,struct Domain_t *domain,struct hc_workerState *ws)
{
  struct _async_767_5frame_t__ *__hc_new_frame__ = (struct _async_767_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_767_5frame_t__ ),_async_767_5));
  ((struct _async_767_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_767_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_767_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcForceForNodes__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
    case 3:
    goto __post_func3;
  }
  Index_t numNode = (domain -> m_numNode);
  hc_startFinish(ws);
{
    Index_t i_len = numNode;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> numNode = numNode;
      struct hc_frameHeader *_async_767_5frame__2 = buildFrame__async_767_5(i_out,i_end,domain,ws);
      _async_767_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_767_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_767_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_767_5(ws,((struct hc_frameHeader *)_async_767_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> i_blk;
        numNode = ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> numNode;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> numNode = numNode;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    numNode = ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> numNode;
  };
  __hc_frame__ -> pc = 3;
  ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> numNode = numNode;
  struct hc_frameHeader *__hc_CalcVolumeForceForElems__frame__3 = buildFrame___hc_CalcVolumeForceForElems__(ws);
  __hc_CalcVolumeForceForElems__(ws,((struct hc_frameHeader *)__hc_CalcVolumeForceForElems__frame__3),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func3:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    numNode = ((struct __hc_CalcForceForNodes__frame_t__ *)__hc_frame__) -> numNode;
  };;
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_767_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_767_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_767_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_767_5frame_t__ *)__hc_frame__) -> domain;
  for (Index_t i = i_out; i < i_end; ++i) {
    (domain -> m_fx)[i] = ((Real_t )0.0);
    (domain -> m_fy)[i] = ((Real_t )0.0);
    (domain -> m_fz)[i] = ((Real_t )0.0);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcAccelerationForNodes__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long numNode;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
}
;
void __hc_CalcAccelerationForNodes__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcAccelerationForNodes__(struct hc_workerState *ws)
{
  struct __hc_CalcAccelerationForNodes__frame_t__ *__hc_new_frame__ = (struct __hc_CalcAccelerationForNodes__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcAccelerationForNodes__frame_t__ ),__hc_CalcAccelerationForNodes__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_787_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
}
;
void _async_787_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_787_5(long i_out,long i_end,struct Domain_t *domain,struct hc_workerState *ws)
{
  struct _async_787_5frame_t__ *__hc_new_frame__ = (struct _async_787_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_787_5frame_t__ ),_async_787_5));
  ((struct _async_787_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_787_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_787_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcAccelerationForNodes__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  Index_t numNode = (domain -> m_numNode);
  hc_startFinish(ws);
{
    Index_t i_len = numNode;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcAccelerationForNodes__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcAccelerationForNodes__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcAccelerationForNodes__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcAccelerationForNodes__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcAccelerationForNodes__frame_t__ *)__hc_frame__) -> numNode = numNode;
      struct hc_frameHeader *_async_787_5frame__2 = buildFrame__async_787_5(i_out,i_end,domain,ws);
      _async_787_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_787_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_787_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_787_5(ws,((struct hc_frameHeader *)_async_787_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_CalcAccelerationForNodes__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcAccelerationForNodes__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcAccelerationForNodes__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcAccelerationForNodes__frame_t__ *)__hc_frame__) -> i_blk;
        numNode = ((struct __hc_CalcAccelerationForNodes__frame_t__ *)__hc_frame__) -> numNode;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcAccelerationForNodes__frame_t__ *)__hc_frame__) -> numNode = numNode;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    numNode = ((struct __hc_CalcAccelerationForNodes__frame_t__ *)__hc_frame__) -> numNode;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_787_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_787_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_787_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_787_5frame_t__ *)__hc_frame__) -> domain;
  for (Index_t i = i_out; i < i_end; ++i) {
    (domain -> m_xdd)[i] = ((domain -> m_fx)[i] / (domain -> m_nodalMass)[i]);
//DEBUG if(i==1)fprintf(stdout,"CAFN: m_xdd %e\n",domain->m_xdd[1]);
//DEBUG if(i==1)fprintf(stdout,"CAFN: m_fx  %e\n",domain->m_fx[1]);
//DEBUG if(i==1)fprintf(stdout,"CAFN: m_nodalMass %e\n",domain->m_nodalMass[1]);
    (domain -> m_ydd)[i] = ((domain -> m_fy)[i] / (domain -> m_nodalMass)[i]);
//DEBUG if(i==1)fprintf(stdout,"CAFN: m_ydd %e\n",domain->m_ydd[1]);
    (domain -> m_zdd)[i] = ((domain -> m_fz)[i] / (domain -> m_nodalMass)[i]);
//DEBUG if(i==1)fprintf(stdout,"CAFN: m_zdd %e\n",domain->m_zdd[1]);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long numNodeBC;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
}
;
void __hc_ApplyAccelerationBoundaryConditionsForNodes__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_ApplyAccelerationBoundaryConditionsForNodes__(struct hc_workerState *ws)
{
  struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *__hc_new_frame__ = (struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ ),__hc_ApplyAccelerationBoundaryConditionsForNodes__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_804_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
}
;
void _async_804_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_804_5(long i_out,long i_end,struct Domain_t *domain,struct hc_workerState *ws)
{
  struct _async_804_5frame_t__ *__hc_new_frame__ = (struct _async_804_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_804_5frame_t__ ),_async_804_5));
  ((struct _async_804_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_804_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_804_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_ApplyAccelerationBoundaryConditionsForNodes__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  Index_t numNodeBC = (((domain -> m_sizeX) + 1) * ((domain -> m_sizeX) + 1));
  hc_startFinish(ws);
{
    Index_t i_len = numNodeBC;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)__hc_frame__) -> numNodeBC = numNodeBC;
      struct hc_frameHeader *_async_804_5frame__2 = buildFrame__async_804_5(i_out,i_end,domain,ws);
      _async_804_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_804_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_804_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_804_5(ws,((struct hc_frameHeader *)_async_804_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)__hc_frame__) -> i_blk;
        numNodeBC = ((struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)__hc_frame__) -> numNodeBC;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)__hc_frame__) -> numNodeBC = numNodeBC;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    numNodeBC = ((struct __hc_ApplyAccelerationBoundaryConditionsForNodes__frame_t__ *)__hc_frame__) -> numNodeBC;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_804_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_804_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_804_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_804_5frame_t__ *)__hc_frame__) -> domain;
  for (Index_t i = i_out; i < i_end; ++i) {
// RAG -- SCATTER domain->m_(x|y|z)dd[domain->m_symm(X|Y|Z)[i]] = 0.0
    (domain -> m_xdd)[(domain -> m_symmX)[i]] = ((Real_t )0.0);
    (domain -> m_ydd)[(domain -> m_symmY)[i]] = ((Real_t )0.0);
    (domain -> m_zdd)[(domain -> m_symmZ)[i]] = ((Real_t )0.0);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcVelocityForNodes__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long numNode;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
  double dt;
  double u_cut;
}
;
void __hc_CalcVelocityForNodes__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcVelocityForNodes__(double dt,double u_cut,struct hc_workerState *ws)
{
  struct __hc_CalcVelocityForNodes__frame_t__ *__hc_new_frame__ = (struct __hc_CalcVelocityForNodes__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcVelocityForNodes__frame_t__ ),__hc_CalcVelocityForNodes__));
  ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_new_frame__) -> dt = dt;
  ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_new_frame__) -> u_cut = u_cut;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_818_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double dt;
  double u_cut;
}
;
void _async_818_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_818_5(long i_out,long i_end,struct Domain_t *domain,double dt,double u_cut,struct hc_workerState *ws)
{
  struct _async_818_5frame_t__ *__hc_new_frame__ = (struct _async_818_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_818_5frame_t__ ),_async_818_5));
  ((struct _async_818_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_818_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_818_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_818_5frame_t__ *)__hc_new_frame__) -> dt = dt;
  ((struct _async_818_5frame_t__ *)__hc_new_frame__) -> u_cut = u_cut;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcVelocityForNodes__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  double dt;
  double u_cut;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  dt = ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> dt;
  u_cut = ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> u_cut;
  Index_t numNode = (domain -> m_numNode);
//DEBUG fprintf(stdout,"CVFN:dt= %e\n",dt);
  hc_startFinish(ws);
{
    Index_t i_len = numNode;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> numNode = numNode;
      ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> dt = dt;
      ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> u_cut = u_cut;
      struct hc_frameHeader *_async_818_5frame__2 = buildFrame__async_818_5(i_out,i_end,domain,dt,u_cut,ws);
      _async_818_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_818_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_818_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_818_5(ws,((struct hc_frameHeader *)_async_818_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> i_blk;
        numNode = ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> numNode;
        dt = ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> dt;
        u_cut = ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> u_cut;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> numNode = numNode;
  ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> dt = dt;
  ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> u_cut = u_cut;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    numNode = ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> numNode;
    dt = ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> dt;
    u_cut = ((struct __hc_CalcVelocityForNodes__frame_t__ *)__hc_frame__) -> u_cut;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_818_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double dt;
  double u_cut;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_818_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_818_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_818_5frame_t__ *)__hc_frame__) -> domain;
  dt = ((struct _async_818_5frame_t__ *)__hc_frame__) -> dt;
  u_cut = ((struct _async_818_5frame_t__ *)__hc_frame__) -> u_cut;
  for (Index_t i = i_out; i < i_end; ++i) {
    Real_t xdtmp;
    Real_t ydtmp;
    Real_t zdtmp;
// RAG -- DAXPY       -- (x|y|x)d += dt * (x|y|z)dd
// RAG -- DFLUSH .i.e -- -u_cut < (x|y|z)d <u_cut to 0.0
    xdtmp = ((domain -> m_xd)[i] + ((domain -> m_xdd)[i] * dt));
    if (fabs(xdtmp) < u_cut) 
      xdtmp = ((Real_t )0.0);
    (domain -> m_xd)[i] = xdtmp;
//DEBUG if(i==1)fprintf(stdout,"CVFN:m_xd[1]= %e\n",domain->m_xd[1]);
    ydtmp = ((domain -> m_yd)[i] + ((domain -> m_ydd)[i] * dt));
    if (fabs(ydtmp) < u_cut) 
      ydtmp = ((Real_t )0.0);
    (domain -> m_yd)[i] = ydtmp;
//DEBUG if(i==1)fprintf(stdout,"CVFN:m_yd[1]= %e\n",domain->m_yd[1]);
    zdtmp = ((domain -> m_zd)[i] + ((domain -> m_zdd)[i] * dt));
    if (fabs(zdtmp) < u_cut) 
      zdtmp = ((Real_t )0.0);
    (domain -> m_zd)[i] = zdtmp;
//DEBUG if(i==1)fprintf(stdout,"CVFN:m_zd[1]= %e\n",domain->m_zd[1]);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcPositionForNodes__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long numNode;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
  double dt;
}
;
void __hc_CalcPositionForNodes__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcPositionForNodes__(double dt,struct hc_workerState *ws)
{
  struct __hc_CalcPositionForNodes__frame_t__ *__hc_new_frame__ = (struct __hc_CalcPositionForNodes__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcPositionForNodes__frame_t__ ),__hc_CalcPositionForNodes__));
  ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_new_frame__) -> dt = dt;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_847_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double dt;
}
;
void _async_847_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_847_5(long i_out,long i_end,struct Domain_t *domain,double dt,struct hc_workerState *ws)
{
  struct _async_847_5frame_t__ *__hc_new_frame__ = (struct _async_847_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_847_5frame_t__ ),_async_847_5));
  ((struct _async_847_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_847_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_847_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_847_5frame_t__ *)__hc_new_frame__) -> dt = dt;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcPositionForNodes__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  double dt;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  dt = ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> dt;
  Index_t numNode = (domain -> m_numNode);
//DEBUG fprintf(stdout,"CPFN:dt= %e\n",dt);
  hc_startFinish(ws);
{
    Index_t i_len = numNode;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> numNode = numNode;
      ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> dt = dt;
      struct hc_frameHeader *_async_847_5frame__2 = buildFrame__async_847_5(i_out,i_end,domain,dt,ws);
      _async_847_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_847_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_847_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_847_5(ws,((struct hc_frameHeader *)_async_847_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> i_blk;
        numNode = ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> numNode;
        dt = ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> dt;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> numNode = numNode;
  ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> dt = dt;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    numNode = ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> numNode;
    dt = ((struct __hc_CalcPositionForNodes__frame_t__ *)__hc_frame__) -> dt;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_847_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double dt;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_847_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_847_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_847_5frame_t__ *)__hc_frame__) -> domain;
  dt = ((struct _async_847_5frame_t__ *)__hc_frame__) -> dt;
  for (Index_t i = i_out; i < i_end; ++i) {
// RAG -- DAXPY       -- (x|y|x) += dt * (x|y|z)d
    (domain -> m_x)[i] += ((domain -> m_xd)[i] * dt);
//DEBUG if(i==1)fprintf(stdout,"CPFN:m_x[1]= %e\n",domain->m_x[1]);
    (domain -> m_y)[i] += ((domain -> m_yd)[i] * dt);
//DEBUG if(i==1)fprintf(stdout,"CPFN:m_y[1]= %e\n",domain->m_y[1]);
    (domain -> m_z)[i] += ((domain -> m_zd)[i] * dt);
//DEBUG if(i==1)fprintf(stdout,"CPFN:m_z[1]= %e\n",domain->m_z[1]);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_LagrangeNodal__frame_t__ 
{
  struct hc_frameHeader header;
  double delt;
  double u_cut;
}
;
void __hc_LagrangeNodal__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_LagrangeNodal__(struct hc_workerState *ws)
{
  struct __hc_LagrangeNodal__frame_t__ *__hc_new_frame__ = (struct __hc_LagrangeNodal__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_LagrangeNodal__frame_t__ ),__hc_LagrangeNodal__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_LagrangeNodal__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
    case 2:
    goto __post_func2;
    case 3:
    goto __post_func3;
    case 4:
    goto __post_func4;
    case 5:
    goto __post_func5;
  }
  Real_t delt = (domain -> m_deltatime);
  Real_t u_cut = (domain -> m_u_cut);;;;
  __hc_frame__ -> pc = 1;
  ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> delt = delt;
  ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> u_cut = u_cut;
  struct hc_frameHeader *__hc_CalcForceForNodes__frame__1 = buildFrame___hc_CalcForceForNodes__(ws);
  __hc_CalcForceForNodes__(ws,((struct hc_frameHeader *)__hc_CalcForceForNodes__frame__1),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func1:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    delt = ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> delt;
    u_cut = ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> u_cut;
  };
  __hc_frame__ -> pc = 2;
  ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> delt = delt;
  ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> u_cut = u_cut;
  struct hc_frameHeader *__hc_CalcAccelerationForNodes__frame__2 = buildFrame___hc_CalcAccelerationForNodes__(ws);
  __hc_CalcAccelerationForNodes__(ws,((struct hc_frameHeader *)__hc_CalcAccelerationForNodes__frame__2),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func2:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    delt = ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> delt;
    u_cut = ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> u_cut;
  };
  __hc_frame__ -> pc = 3;
  ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> delt = delt;
  ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> u_cut = u_cut;
  struct hc_frameHeader *__hc_ApplyAccelerationBoundaryConditionsForNodes__frame__3 = buildFrame___hc_ApplyAccelerationBoundaryConditionsForNodes__(ws);
  __hc_ApplyAccelerationBoundaryConditionsForNodes__(ws,((struct hc_frameHeader *)__hc_ApplyAccelerationBoundaryConditionsForNodes__frame__3),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func3:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    delt = ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> delt;
    u_cut = ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> u_cut;
  };
  __hc_frame__ -> pc = 4;
  ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> delt = delt;
  ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> u_cut = u_cut;
  struct hc_frameHeader *__hc_CalcVelocityForNodes__frame__4 = buildFrame___hc_CalcVelocityForNodes__(delt,u_cut,ws);
  __hc_CalcVelocityForNodes__(ws,((struct hc_frameHeader *)__hc_CalcVelocityForNodes__frame__4),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func4:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    delt = ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> delt;
    u_cut = ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> u_cut;
  };
  __hc_frame__ -> pc = 5;
  ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> delt = delt;
  ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> u_cut = u_cut;
  struct hc_frameHeader *__hc_CalcPositionForNodes__frame__5 = buildFrame___hc_CalcPositionForNodes__(delt,ws);
  __hc_CalcPositionForNodes__(ws,((struct hc_frameHeader *)__hc_CalcPositionForNodes__frame__5),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func5:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    delt = ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> delt;
    u_cut = ((struct __hc_LagrangeNodal__frame_t__ *)__hc_frame__) -> u_cut;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

static Real_t CalcElemVolume_scalars(const Real_t x0,const Real_t x1,const Real_t x2,const Real_t x3,const Real_t x4,const Real_t x5,const Real_t x6,const Real_t x7,const Real_t y0,const Real_t y1,const Real_t y2,const Real_t y3,const Real_t y4,const Real_t y5,const Real_t y6,const Real_t y7,const Real_t z0,const Real_t z1,const Real_t z2,const Real_t z3,const Real_t z4,const Real_t z5,const Real_t z6,const Real_t z7)
{
  Real_t twelveth = (((Real_t )1.0) / ((Real_t )12.0));
  Real_t dx61 = (x6 - x1);
  Real_t dy61 = (y6 - y1);
  Real_t dz61 = (z6 - z1);
  Real_t dx70 = (x7 - x0);
  Real_t dy70 = (y7 - y0);
  Real_t dz70 = (z7 - z0);
  Real_t dx63 = (x6 - x3);
  Real_t dy63 = (y6 - y3);
  Real_t dz63 = (z6 - z3);
  Real_t dx20 = (x2 - x0);
  Real_t dy20 = (y2 - y0);
  Real_t dz20 = (z2 - z0);
  Real_t dx50 = (x5 - x0);
  Real_t dy50 = (y5 - y0);
  Real_t dz50 = (z5 - z0);
  Real_t dx64 = (x6 - x4);
  Real_t dy64 = (y6 - y4);
  Real_t dz64 = (z6 - z4);
  Real_t dx31 = (x3 - x1);
  Real_t dy31 = (y3 - y1);
  Real_t dz31 = (z3 - z1);
  Real_t dx72 = (x7 - x2);
  Real_t dy72 = (y7 - y2);
  Real_t dz72 = (z7 - z2);
  Real_t dx43 = (x4 - x3);
  Real_t dy43 = (y4 - y3);
  Real_t dz43 = (z4 - z3);
  Real_t dx57 = (x5 - x7);
  Real_t dy57 = (y5 - y7);
  Real_t dz57 = (z5 - z7);
  Real_t dx14 = (x1 - x4);
  Real_t dy14 = (y1 - y4);
  Real_t dz14 = (z1 - z4);
  Real_t dx25 = (x2 - x5);
  Real_t dy25 = (y2 - y5);
  Real_t dz25 = (z2 - z5);
#define TRIPLE_PRODUCT(x1, y1, z1, x2, y2, z2, x3, y3, z3) \
   ((x1)*((y2)*(z3) - (z2)*(y3)) + (x2)*((z1)*(y3) - (y1)*(z3)) + (x3)*((y1)*(z2) - (z1)*(y2)))
  Real_t volume = ((((((dx31 + dx72) * ((dy63 * dz20) - (dy20 * dz63))) + ((dy31 + dy72) * ((dx20 * dz63) - (dx63 * dz20)))) + ((dz31 + dz72) * ((dx63 * dy20) - (dx20 * dy63)))) + ((((dx43 + dx57) * ((dy64 * dz70) - (dy70 * dz64))) + ((dy43 + dy57) * ((dx70 * dz64) - (dx64 * dz70)))) + ((dz43 + dz57) * ((dx64 * dy70) - (dx70 * dy64))))) + ((((dx14 + dx25) * ((dy61 * dz50) - (dy50 * dz61))) + ((dy14 + dy25) * ((dx50 * dz61) - (dx61 * dz50)))) + ((dz14 + dz25) * ((dx61 * dy50) - (dx50 * dy61)))));
#undef TRIPLE_PRODUCT
  volume *= twelveth;
  return volume;
// CalcElemVolume_scalars()
}

static Real_t CalcElemVolume(const Real_t x[(Index_t )8],const Real_t y[(Index_t )8],const Real_t z[(Index_t )8])
{
  return CalcElemVolume_scalars(x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7],y[0],y[1],y[2],y[3],y[4],y[5],y[6],y[7],z[0],z[1],z[2],z[3],z[4],z[5],z[6],z[7]);
// CalcElemVolume()
}

static Real_t AreaFace(const Real_t x0,const Real_t x1,const Real_t x2,const Real_t x3,const Real_t y0,const Real_t y1,const Real_t y2,const Real_t y3,const Real_t z0,const Real_t z1,const Real_t z2,const Real_t z3)
{
  Real_t fx = ((x2 - x0) - (x3 - x1));
  Real_t fy = ((y2 - y0) - (y3 - y1));
  Real_t fz = ((z2 - z0) - (z3 - z1));
  Real_t gx = ((x2 - x0) + (x3 - x1));
  Real_t gy = ((y2 - y0) + (y3 - y1));
  Real_t gz = ((z2 - z0) + (z3 - z1));
  Real_t area = (((((fx * fx) + (fy * fy)) + (fz * fz)) * (((gx * gx) + (gy * gy)) + (gz * gz))) - ((((fx * gx) + (fy * gy)) + (fz * gz)) * (((fx * gx) + (fy * gy)) + (fz * gz))));
  return area;
// AreaFace
}

static Real_t CalcElemCharacteristicLength(const Real_t x[(Index_t )8],const Real_t y[(Index_t )8],const Real_t z[(Index_t )8],const Real_t volume)
{
  Real_t a;
  Real_t charLength = (Real_t )0.0;
  a = AreaFace(x[0],x[1],x[2],x[3],y[0],y[1],y[2],y[3],z[0],z[1],z[2],z[3]);
  charLength = fmax(a,charLength);
  a = AreaFace(x[4],x[5],x[6],x[7],y[4],y[5],y[6],y[7],z[4],z[5],z[6],z[7]);
  charLength = fmax(a,charLength);
  a = AreaFace(x[0],x[1],x[5],x[4],y[0],y[1],y[5],y[4],z[0],z[1],z[5],z[4]);
  charLength = fmax(a,charLength);
  a = AreaFace(x[1],x[2],x[6],x[5],y[1],y[2],y[6],y[5],z[1],z[2],z[6],z[5]);
  charLength = fmax(a,charLength);
  a = AreaFace(x[2],x[3],x[7],x[6],y[2],y[3],y[7],y[6],z[2],z[3],z[7],z[6]);
  charLength = fmax(a,charLength);
  a = AreaFace(x[3],x[0],x[4],x[7],y[3],y[0],y[4],y[7],z[3],z[0],z[4],z[7]);
  charLength = fmax(a,charLength);
  charLength = ((((Real_t )4.0) * volume) / sqrt(charLength));
  return charLength;
// CalcElemCharacteristicLength()
}

static void CalcElemVelocityGrandient(const Real_t *const xvel,const Real_t *const yvel,const Real_t *const zvel,
// RAG -- b[][EIGHT]
Real_t *b,const Real_t detJ,Real_t *const d)
{
  const Real_t inv_detJ = (((Real_t )1.0) / detJ);
  Real_t dyddx;
  Real_t dxddy;
  Real_t dzddx;
  Real_t dxddz;
  Real_t dzddy;
  Real_t dyddz;
  const Real_t *const pfx = (b + 0 * ((Index_t )8));
  const Real_t *const pfy = (b + 1 * ((Index_t )8));
  const Real_t *const pfz = (b + 2 * ((Index_t )8));
  d[0] = (inv_detJ * ((((pfx[0] * (xvel[0] - xvel[6])) + (pfx[1] * (xvel[1] - xvel[7]))) + (pfx[2] * (xvel[2] - xvel[4]))) + (pfx[3] * (xvel[3] - xvel[5]))));
  d[1] = (inv_detJ * ((((pfy[0] * (yvel[0] - yvel[6])) + (pfy[1] * (yvel[1] - yvel[7]))) + (pfy[2] * (yvel[2] - yvel[4]))) + (pfy[3] * (yvel[3] - yvel[5]))));
  d[2] = (inv_detJ * ((((pfz[0] * (zvel[0] - zvel[6])) + (pfz[1] * (zvel[1] - zvel[7]))) + (pfz[2] * (zvel[2] - zvel[4]))) + (pfz[3] * (zvel[3] - zvel[5]))));
  dyddx = (inv_detJ * ((((pfx[0] * (yvel[0] - yvel[6])) + (pfx[1] * (yvel[1] - yvel[7]))) + (pfx[2] * (yvel[2] - yvel[4]))) + (pfx[3] * (yvel[3] - yvel[5]))));
  dxddy = (inv_detJ * ((((pfy[0] * (xvel[0] - xvel[6])) + (pfy[1] * (xvel[1] - xvel[7]))) + (pfy[2] * (xvel[2] - xvel[4]))) + (pfy[3] * (xvel[3] - xvel[5]))));
  dzddx = (inv_detJ * ((((pfx[0] * (zvel[0] - zvel[6])) + (pfx[1] * (zvel[1] - zvel[7]))) + (pfx[2] * (zvel[2] - zvel[4]))) + (pfx[3] * (zvel[3] - zvel[5]))));
  dxddz = (inv_detJ * ((((pfz[0] * (xvel[0] - xvel[6])) + (pfz[1] * (xvel[1] - xvel[7]))) + (pfz[2] * (xvel[2] - xvel[4]))) + (pfz[3] * (xvel[3] - xvel[5]))));
  dzddy = (inv_detJ * ((((pfy[0] * (zvel[0] - zvel[6])) + (pfy[1] * (zvel[1] - zvel[7]))) + (pfy[2] * (zvel[2] - zvel[4]))) + (pfy[3] * (zvel[3] - zvel[5]))));
  dyddz = (inv_detJ * ((((pfz[0] * (yvel[0] - yvel[6])) + (pfz[1] * (yvel[1] - yvel[7]))) + (pfz[2] * (yvel[2] - yvel[4]))) + (pfz[3] * (yvel[3] - yvel[5]))));
  d[5] = (((Real_t )0.5) * (dxddy + dyddx));
  d[4] = (((Real_t )0.5) * (dxddz + dzddx));
  d[3] = (((Real_t )0.5) * (dzddy + dyddz));
// CalcElemVelocityGrandient()
}

struct __hc_CalcKinematicsForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long k_end;
  long k_out;
  long k_len;
  long k_blk;
  long numElem;
  double dt;
}
;
void __hc_CalcKinematicsForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcKinematicsForElems__(long numElem,double dt,struct hc_workerState *ws)
{
  struct __hc_CalcKinematicsForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcKinematicsForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcKinematicsForElems__frame_t__ ),__hc_CalcKinematicsForElems__));
  ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_new_frame__) -> numElem = numElem;
  ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_new_frame__) -> dt = dt;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1111_5frame_t__ 
{
  struct hc_frameHeader header;
  long k_out;
  long k_end;
  struct Domain_t *domain;
  double dt;
}
;
void _async_1111_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1111_5(long k_out,long k_end,struct Domain_t *domain,double dt,struct hc_workerState *ws)
{
  struct _async_1111_5frame_t__ *__hc_new_frame__ = (struct _async_1111_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1111_5frame_t__ ),_async_1111_5));
  ((struct _async_1111_5frame_t__ *)__hc_new_frame__) -> k_out = k_out;
  ((struct _async_1111_5frame_t__ *)__hc_new_frame__) -> k_end = k_end;
  ((struct _async_1111_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_1111_5frame_t__ *)__hc_new_frame__) -> dt = dt;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcKinematicsForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long numElem;
  double dt;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  numElem = ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> numElem;
  dt = ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> dt;
  hc_startFinish(ws);
{
// loop over all elements
    Index_t k_len = numElem;
    Index_t k_blk = 16;
    for (Index_t k_out = 0; k_out < k_len; k_out += k_blk) {
      Index_t k_end = ((k_out + k_blk) < k_len)?(k_out + k_blk) : k_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> k_end = k_end;
      ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> k_out = k_out;
      ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> k_len = k_len;
      ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> k_blk = k_blk;
      ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
      ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> dt = dt;
      struct hc_frameHeader *_async_1111_5frame__2 = buildFrame__async_1111_5(k_out,k_end,domain,dt,ws);
      _async_1111_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_1111_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1111_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1111_5(ws,((struct hc_frameHeader *)_async_1111_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        k_end = ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> k_end;
        k_out = ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> k_out;
        k_len = ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> k_len;
        k_blk = ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> k_blk;
        numElem = ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> numElem;
        dt = ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> dt;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
  ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> dt = dt;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    numElem = ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> numElem;
    dt = ((struct __hc_CalcKinematicsForElems__frame_t__ *)__hc_frame__) -> dt;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_1111_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long k_out;
  long k_end;
  struct Domain_t *domain;
  double dt;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  k_out = ((struct _async_1111_5frame_t__ *)__hc_frame__) -> k_out;
  k_end = ((struct _async_1111_5frame_t__ *)__hc_frame__) -> k_end;
  domain = ((struct _async_1111_5frame_t__ *)__hc_frame__) -> domain;
  dt = ((struct _async_1111_5frame_t__ *)__hc_frame__) -> dt;
  for (Index_t k = k_out; k < k_end; ++k) {
#if  defined(HAB_C)
    Real_t *B = (Real_t *)(malloc(((((Index_t )3) * ((Index_t )8)) * sizeof(Real_t ))));
    Real_t *D = (Real_t *)(malloc(((((Index_t )3) * ((Index_t )8)) * sizeof(Real_t ))));
    Real_t *x_local = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *y_local = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *z_local = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *xd_local = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *yd_local = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *zd_local = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *detJ = (Real_t *)(malloc((sizeof(Real_t ))));
     *detJ = ((Real_t )0.0);
#else // NOT HAB_C
/** shape function derivatives */
/* RAG -- B[THREE][EIGHT] */
#endif //    HAB_C
    Real_t volume;
    Real_t relativeVolume;
    Index_t *elemToNode = ((domain -> m_nodelist) + (((Index_t )8) * k));
// get nodal coordinates from global arrays and copy into local arrays.
    for (Index_t lnode = 0; lnode < ((Index_t )8); ++lnode) {
      Index_t gnode = elemToNode[lnode];
//DEBUG if(k==0)fprintf(stdout,"gnode%d= %d\n",lnode,gnode);
      x_local[lnode] = (domain -> m_x)[gnode];
//DEBUG if(k==0)fprintf(stdout,"x_local%d= %e\n",lnode,x_local[lnode]);
      y_local[lnode] = (domain -> m_y)[gnode];
//DEBUG if(k==0)fprintf(stdout,"y_local%d= %e\n",lnode,y_local[lnode]);
      z_local[lnode] = (domain -> m_z)[gnode];
//DEBUG if(k==0)fprintf(stdout,"z_local%d= %e\n",lnode,z_local[lnode]);
// for lnode
    }
// volume calculations
    volume = CalcElemVolume(x_local,y_local,z_local);
//DEBUG if(k==0)fprintf(stdout,"volume= %e\n",volume);
    relativeVolume = (volume / (domain -> m_volo)[k]);
//DEBUG if(k==0)fprintf(stdout,"relVol= %e\n",relativeVolume);
    (domain -> m_vnew)[k] = relativeVolume;
//DEBUG if(k==0)fprintf(stdout,"m_v   = %e\n",domain->m_v[0]);
    (domain -> m_delv)[k] = (relativeVolume - (domain -> m_v)[k]);
//DEBUG if(k==0)fprintf(stdout,"m_delv= %e\n",domain->m_delv[0]);
// set characteristic length
    (domain -> m_arealg)[k] = CalcElemCharacteristicLength(x_local,y_local,z_local,volume);
// get nodal velocities from global array and copy into local arrays.
    for (Index_t lnode = 0; lnode < ((Index_t )8); ++lnode) {
      Index_t gnode = elemToNode[lnode];
      xd_local[lnode] = (domain -> m_xd)[gnode];
      yd_local[lnode] = (domain -> m_yd)[gnode];
      zd_local[lnode] = (domain -> m_zd)[gnode];
// for lnode
    }
    Real_t dt2 = (((Real_t )0.5) * dt);
    for (Index_t j = 0; j < ((Index_t )8); ++j) {
      x_local[j] -= (dt2 * xd_local[j]);
      y_local[j] -= (dt2 * yd_local[j]);
      z_local[j] -= (dt2 * zd_local[j]);
// for j
    }
#if  defined(HAB_C)
    CalcElemShapeFunctionDerivatives(x_local,y_local,z_local,B,detJ);
    CalcElemVelocityGrandient(xd_local,yd_local,zd_local,B, *detJ,D);
#else // NOT HAB_C
#endif //    HAB_C
// put velocity gradient quantities into their global arrays.
    (domain -> m_dxx)[k] = D[0];
    (domain -> m_dyy)[k] = D[1];
    (domain -> m_dzz)[k] = D[2];
#if  defined(HAB_C)
    free(zd_local);
    free(yd_local);
    free(xd_local);
    free(z_local);
    free(y_local);
    free(x_local);
    free(D);
    free(B);
#endif //    HAB_C
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcLagrangeElements__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long numElem;
  long k_end;
  long k_out;
  long k_len;
  long k_blk;
  double deltatime;
}
;
void __hc_CalcLagrangeElements__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcLagrangeElements__(double deltatime,struct hc_workerState *ws)
{
  struct __hc_CalcLagrangeElements__frame_t__ *__hc_new_frame__ = (struct __hc_CalcLagrangeElements__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcLagrangeElements__frame_t__ ),__hc_CalcLagrangeElements__));
  ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_new_frame__) -> deltatime = deltatime;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1224_9frame_t__ 
{
  struct hc_frameHeader header;
  long k_out;
  long k_end;
  struct Domain_t *domain;
}
;
void _async_1224_9(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1224_9(long k_out,long k_end,struct Domain_t *domain,struct hc_workerState *ws)
{
  struct _async_1224_9frame_t__ *__hc_new_frame__ = (struct _async_1224_9frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1224_9frame_t__ ),_async_1224_9));
  ((struct _async_1224_9frame_t__ *)__hc_new_frame__) -> k_out = k_out;
  ((struct _async_1224_9frame_t__ *)__hc_new_frame__) -> k_end = k_end;
  ((struct _async_1224_9frame_t__ *)__hc_new_frame__) -> domain = domain;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcLagrangeElements__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  double deltatime;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
    case 2:
    goto __post_finish2;
    case 3:
    goto __post_async3;
  }
  deltatime = ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> deltatime;
  Index_t numElem = (domain -> m_numElem);
  if (numElem > 0) {
    __hc_frame__ -> pc = 1;
    ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> numElem = numElem;
    ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> deltatime = deltatime;
    struct hc_frameHeader *__hc_CalcKinematicsForElems__frame__1 = buildFrame___hc_CalcKinematicsForElems__(numElem,deltatime,ws);
    __hc_CalcKinematicsForElems__(ws,((struct hc_frameHeader *)__hc_CalcKinematicsForElems__frame__1),0);
    if (ws -> costatus == HC_FUNC_COMPLETE) {
      hc_free_unwind_frame(ws);
    }
    else {
      return ;
    }
    if (0) {
      __post_func1:;
      __hc_frame__ = hc_free_unwind_frame(ws);
      numElem = ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> numElem;
      deltatime = ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> deltatime;
    }
// element loop to do some stuff not included in the elemlib function.
    hc_startFinish(ws);
{
      Index_t k_len = numElem;
      Index_t k_blk = 16;
      for (Index_t k_out = 0; k_out < k_len; k_out += k_blk) {
        Index_t k_end = ((k_out + k_blk) < k_len)?(k_out + k_blk) : k_len;
        __hc_frame__ -> pc = 3;
        ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> k_end = k_end;
        ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> k_out = k_out;
        ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> k_len = k_len;
        ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> k_blk = k_blk;
        ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> numElem = numElem;
        ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> deltatime = deltatime;
        struct hc_frameHeader *_async_1224_9frame__3 = buildFrame__async_1224_9(k_out,k_end,domain,ws);
        _async_1224_9frame__3 -> sched_policy = hc_sched_policy(ws);
        if (_async_1224_9frame__3 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_1224_9frame__3);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_1224_9(ws,((struct hc_frameHeader *)_async_1224_9frame__3),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async3:;
          k_end = ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> k_end;
          k_out = ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> k_out;
          k_len = ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> k_len;
          k_blk = ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> k_blk;
          numElem = ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> numElem;
          deltatime = ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> deltatime;
        }
      }
    }
    __hc_frame__ -> pc = 2;
    ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> numElem = numElem;
    ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> deltatime = deltatime;
    if (hc_stopFinish(ws)) 
      return ;
    if (0) {
      __post_finish2:;
      numElem = ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> numElem;
      deltatime = ((struct __hc_CalcLagrangeElements__frame_t__ *)__hc_frame__) -> deltatime;
    }
// if numElem
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_1224_9(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long k_out;
  long k_end;
  struct Domain_t *domain;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  k_out = ((struct _async_1224_9frame_t__ *)__hc_frame__) -> k_out;
  k_end = ((struct _async_1224_9frame_t__ *)__hc_frame__) -> k_end;
  domain = ((struct _async_1224_9frame_t__ *)__hc_frame__) -> domain;
  for (Index_t k = k_out; k < k_end; ++k) {
// calc strain rate and apply as constraint (only done in FB element)
    Real_t vdov = (((domain -> m_dxx)[k] + (domain -> m_dyy)[k]) + (domain -> m_dzz)[k]);
    Real_t vdovthird = (vdov / ((Real_t )3.0));
// make the rate of deformation tensor deviatoric
    (domain -> m_vdov)[k] = vdov;
    (domain -> m_dxx)[k] -= vdovthird;
    (domain -> m_dyy)[k] -= vdovthird;
    (domain -> m_dzz)[k] -= vdovthird;
// See if any volumes are negative, and take appropriate action.
    if ((domain -> m_vnew)[k] <= ((Real_t )0.0)) {
      exit(VolumeError);
// if domain->m_vnew
    }
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcMonotonicQGradientsForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long numElem;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
}
;
void __hc_CalcMonotonicQGradientsForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcMonotonicQGradientsForElems__(struct hc_workerState *ws)
{
  struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcMonotonicQGradientsForElems__frame_t__ ),__hc_CalcMonotonicQGradientsForElems__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1249_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
}
;
void _async_1249_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1249_5(long i_out,long i_end,struct Domain_t *domain,struct hc_workerState *ws)
{
  struct _async_1249_5frame_t__ *__hc_new_frame__ = (struct _async_1249_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1249_5frame_t__ ),_async_1249_5));
  ((struct _async_1249_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1249_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1249_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcMonotonicQGradientsForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
#define SUM4(a,b,c,d) (a + b + c + d)
  Index_t numElem = (domain -> m_numElem);
  hc_startFinish(ws);
{
    Index_t i_len = numElem;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
      struct hc_frameHeader *_async_1249_5frame__2 = buildFrame__async_1249_5(i_out,i_end,domain,ws);
      _async_1249_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_1249_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1249_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1249_5(ws,((struct hc_frameHeader *)_async_1249_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)__hc_frame__) -> i_blk;
        numElem = ((struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)__hc_frame__) -> numElem;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    numElem = ((struct __hc_CalcMonotonicQGradientsForElems__frame_t__ *)__hc_frame__) -> numElem;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
#undef SUM4
}

void _async_1249_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1249_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1249_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_1249_5frame_t__ *)__hc_frame__) -> domain;
  for (Index_t i = i_out; i < i_end; ++i) {
    Real_t ptiny = (Real_t )1.e-36;
    Real_t ax;
    Real_t ay;
    Real_t az;
    Real_t dxv;
    Real_t dyv;
    Real_t dzv;
    const Index_t *elemToNode = ((domain -> m_nodelist) + (((Index_t )8) * i));
    Index_t n0 = elemToNode[0];
    Index_t n1 = elemToNode[1];
    Index_t n2 = elemToNode[2];
    Index_t n3 = elemToNode[3];
    Index_t n4 = elemToNode[4];
    Index_t n5 = elemToNode[5];
    Index_t n6 = elemToNode[6];
    Index_t n7 = elemToNode[7];
    Real_t x0 = (domain -> m_x)[n0];
    Real_t x1 = (domain -> m_x)[n1];
    Real_t x2 = (domain -> m_x)[n2];
    Real_t x3 = (domain -> m_x)[n3];
    Real_t x4 = (domain -> m_x)[n4];
    Real_t x5 = (domain -> m_x)[n5];
    Real_t x6 = (domain -> m_x)[n6];
    Real_t x7 = (domain -> m_x)[n7];
    Real_t y0 = (domain -> m_y)[n0];
    Real_t y1 = (domain -> m_y)[n1];
    Real_t y2 = (domain -> m_y)[n2];
    Real_t y3 = (domain -> m_y)[n3];
    Real_t y4 = (domain -> m_y)[n4];
    Real_t y5 = (domain -> m_y)[n5];
    Real_t y6 = (domain -> m_y)[n6];
    Real_t y7 = (domain -> m_y)[n7];
    Real_t z0 = (domain -> m_z)[n0];
    Real_t z1 = (domain -> m_z)[n1];
    Real_t z2 = (domain -> m_z)[n2];
    Real_t z3 = (domain -> m_z)[n3];
    Real_t z4 = (domain -> m_z)[n4];
    Real_t z5 = (domain -> m_z)[n5];
    Real_t z6 = (domain -> m_z)[n6];
    Real_t z7 = (domain -> m_z)[n7];
    Real_t xv0 = (domain -> m_xd)[n0];
    Real_t xv1 = (domain -> m_xd)[n1];
    Real_t xv2 = (domain -> m_xd)[n2];
    Real_t xv3 = (domain -> m_xd)[n3];
    Real_t xv4 = (domain -> m_xd)[n4];
    Real_t xv5 = (domain -> m_xd)[n5];
    Real_t xv6 = (domain -> m_xd)[n6];
    Real_t xv7 = (domain -> m_xd)[n7];
    Real_t yv0 = (domain -> m_yd)[n0];
    Real_t yv1 = (domain -> m_yd)[n1];
    Real_t yv2 = (domain -> m_yd)[n2];
    Real_t yv3 = (domain -> m_yd)[n3];
    Real_t yv4 = (domain -> m_yd)[n4];
    Real_t yv5 = (domain -> m_yd)[n5];
    Real_t yv6 = (domain -> m_yd)[n6];
    Real_t yv7 = (domain -> m_yd)[n7];
    Real_t zv0 = (domain -> m_zd)[n0];
    Real_t zv1 = (domain -> m_zd)[n1];
    Real_t zv2 = (domain -> m_zd)[n2];
    Real_t zv3 = (domain -> m_zd)[n3];
    Real_t zv4 = (domain -> m_zd)[n4];
    Real_t zv5 = (domain -> m_zd)[n5];
    Real_t zv6 = (domain -> m_zd)[n6];
    Real_t zv7 = (domain -> m_zd)[n7];
    Real_t vol = ((domain -> m_volo)[i] * (domain -> m_vnew)[i]);
    Real_t norm = (((Real_t )1.0) / (vol + ptiny));
    Real_t dxj = (((Real_t )(-0.25)) * ((((x0 + x1) + x5) + x4) - (((x3 + x2) + x6) + x7)));
    Real_t dyj = (((Real_t )(-0.25)) * ((((y0 + y1) + y5) + y4) - (((y3 + y2) + y6) + y7)));
    Real_t dzj = (((Real_t )(-0.25)) * ((((z0 + z1) + z5) + z4) - (((z3 + z2) + z6) + z7)));
    Real_t dxi = (((Real_t )0.25) * ((((x1 + x2) + x6) + x5) - (((x0 + x3) + x7) + x4)));
    Real_t dyi = (((Real_t )0.25) * ((((y1 + y2) + y6) + y5) - (((y0 + y3) + y7) + y4)));
    Real_t dzi = (((Real_t )0.25) * ((((z1 + z2) + z6) + z5) - (((z0 + z3) + z7) + z4)));
    Real_t dxk = (((Real_t )0.25) * ((((x4 + x5) + x6) + x7) - (((x0 + x1) + x2) + x3)));
    Real_t dyk = (((Real_t )0.25) * ((((y4 + y5) + y6) + y7) - (((y0 + y1) + y2) + y3)));
    Real_t dzk = (((Real_t )0.25) * ((((z4 + z5) + z6) + z7) - (((z0 + z1) + z2) + z3)));
/* find delvk and delxk ( i cross j ) */
    ax = ((dyi * dzj) - (dzi * dyj));
    ay = ((dzi * dxj) - (dxi * dzj));
    az = ((dxi * dyj) - (dyi * dxj));
    (domain -> m_delx_zeta)[i] = (vol / sqrt(((((ax * ax) + (ay * ay)) + (az * az)) + ptiny)));
    ax *= norm;
    ay *= norm;
    az *= norm;
    dxv = (((Real_t )0.25) * ((((xv4 + xv5) + xv6) + xv7) - (((xv0 + xv1) + xv2) + xv3)));
    dyv = (((Real_t )0.25) * ((((yv4 + yv5) + yv6) + yv7) - (((yv0 + yv1) + yv2) + yv3)));
    dzv = (((Real_t )0.25) * ((((zv4 + zv5) + zv6) + zv7) - (((zv0 + zv1) + zv2) + zv3)));
    (domain -> m_delv_zeta)[i] = (((ax * dxv) + (ay * dyv)) + (az * dzv));
/* find delxi and delvi ( j cross k ) */
    ax = ((dyj * dzk) - (dzj * dyk));
    ay = ((dzj * dxk) - (dxj * dzk));
    az = ((dxj * dyk) - (dyj * dxk));
    (domain -> m_delx_xi)[i] = (vol / sqrt(((((ax * ax) + (ay * ay)) + (az * az)) + ptiny)));
    ax *= norm;
    ay *= norm;
    az *= norm;
    dxv = (((Real_t )0.25) * ((((xv1 + xv2) + xv6) + xv5) - (((xv0 + xv3) + xv7) + xv4)));
    dyv = (((Real_t )0.25) * ((((yv1 + yv2) + yv6) + yv5) - (((yv0 + yv3) + yv7) + yv4)));
    dzv = (((Real_t )0.25) * ((((zv1 + zv2) + zv6) + zv5) - (((zv0 + zv3) + zv7) + zv4)));
    (domain -> m_delv_xi)[i] = (((ax * dxv) + (ay * dyv)) + (az * dzv));
/* find delxj and delvj ( k cross i ) */
    ax = ((dyk * dzi) - (dzk * dyi));
    ay = ((dzk * dxi) - (dxk * dzi));
    az = ((dxk * dyi) - (dyk * dxi));
    (domain -> m_delx_eta)[i] = (vol / sqrt(((((ax * ax) + (ay * ay)) + (az * az)) + ptiny)));
    ax *= norm;
    ay *= norm;
    az *= norm;
    dxv = (((Real_t )(-0.25)) * ((((xv0 + xv1) + xv5) + xv4) - (((xv3 + xv2) + xv6) + xv7)));
    dyv = (((Real_t )(-0.25)) * ((((yv0 + yv1) + yv5) + yv4) - (((yv3 + yv2) + yv6) + yv7)));
    dzv = (((Real_t )(-0.25)) * ((((zv0 + zv1) + zv5) + zv4) - (((zv3 + zv2) + zv6) + zv7)));
    (domain -> m_delv_eta)[i] = (((ax * dxv) + (ay * dyv)) + (az * dzv));
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcMonotonicQRegionForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long ielem_end;
  long ielem_out;
  long ielem_len;
  long ielem_blk;
  double qlc_monoq;
  double qqc_monoq;
  double monoq_limiter_mult;
  double monoq_max_slope;
  double ptiny;
  long elength;
}
;
void __hc_CalcMonotonicQRegionForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcMonotonicQRegionForElems__(double qlc_monoq,double qqc_monoq,double monoq_limiter_mult,double monoq_max_slope,double ptiny,long elength,struct hc_workerState *ws)
{
  struct __hc_CalcMonotonicQRegionForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcMonotonicQRegionForElems__frame_t__ ),__hc_CalcMonotonicQRegionForElems__));
  ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_new_frame__) -> qlc_monoq = qlc_monoq;
  ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_new_frame__) -> qqc_monoq = qqc_monoq;
  ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_new_frame__) -> monoq_limiter_mult = monoq_limiter_mult;
  ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_new_frame__) -> monoq_max_slope = monoq_max_slope;
  ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_new_frame__) -> ptiny = ptiny;
  ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_new_frame__) -> elength = elength;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1403_5frame_t__ 
{
  struct hc_frameHeader header;
  long ielem_out;
  long ielem_end;
  struct Domain_t *domain;
  double qlc_monoq;
  double qqc_monoq;
  double monoq_limiter_mult;
  double monoq_max_slope;
  double ptiny;
}
;
void _async_1403_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1403_5(long ielem_out,long ielem_end,struct Domain_t *domain,double qlc_monoq,double qqc_monoq,double monoq_limiter_mult,double monoq_max_slope,double ptiny,struct hc_workerState *ws)
{
  struct _async_1403_5frame_t__ *__hc_new_frame__ = (struct _async_1403_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1403_5frame_t__ ),_async_1403_5));
  ((struct _async_1403_5frame_t__ *)__hc_new_frame__) -> ielem_out = ielem_out;
  ((struct _async_1403_5frame_t__ *)__hc_new_frame__) -> ielem_end = ielem_end;
  ((struct _async_1403_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_1403_5frame_t__ *)__hc_new_frame__) -> qlc_monoq = qlc_monoq;
  ((struct _async_1403_5frame_t__ *)__hc_new_frame__) -> qqc_monoq = qqc_monoq;
  ((struct _async_1403_5frame_t__ *)__hc_new_frame__) -> monoq_limiter_mult = monoq_limiter_mult;
  ((struct _async_1403_5frame_t__ *)__hc_new_frame__) -> monoq_max_slope = monoq_max_slope;
  ((struct _async_1403_5frame_t__ *)__hc_new_frame__) -> ptiny = ptiny;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcMonotonicQRegionForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  double qlc_monoq;
  double qqc_monoq;
  double monoq_limiter_mult;
  double monoq_max_slope;
  double ptiny;
  long elength;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  qlc_monoq = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> qlc_monoq;
  qqc_monoq = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> qqc_monoq;
  monoq_limiter_mult = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> monoq_limiter_mult;
  monoq_max_slope = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> monoq_max_slope;
  ptiny = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ptiny;
  elength = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> elength;
  hc_startFinish(ws);
{
    Index_t ielem_len = elength;
    Index_t ielem_blk = 16;
    for (Index_t ielem_out = 0; ielem_out < ielem_len; ielem_out += ielem_blk) {
      Index_t ielem_end = ((ielem_out + ielem_blk) < ielem_len)?(ielem_out + ielem_blk) : ielem_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ielem_end = ielem_end;
      ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ielem_out = ielem_out;
      ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ielem_len = ielem_len;
      ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ielem_blk = ielem_blk;
      ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> qlc_monoq = qlc_monoq;
      ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> qqc_monoq = qqc_monoq;
      ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> monoq_limiter_mult = monoq_limiter_mult;
      ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> monoq_max_slope = monoq_max_slope;
      ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ptiny = ptiny;
      ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> elength = elength;
      struct hc_frameHeader *_async_1403_5frame__2 = buildFrame__async_1403_5(ielem_out,ielem_end,domain,qlc_monoq,qqc_monoq,monoq_limiter_mult,monoq_max_slope,ptiny,ws);
      _async_1403_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_1403_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1403_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1403_5(ws,((struct hc_frameHeader *)_async_1403_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        ielem_end = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ielem_end;
        ielem_out = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ielem_out;
        ielem_len = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ielem_len;
        ielem_blk = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ielem_blk;
        qlc_monoq = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> qlc_monoq;
        qqc_monoq = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> qqc_monoq;
        monoq_limiter_mult = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> monoq_limiter_mult;
        monoq_max_slope = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> monoq_max_slope;
        ptiny = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ptiny;
        elength = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> elength;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> qlc_monoq = qlc_monoq;
  ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> qqc_monoq = qqc_monoq;
  ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> monoq_limiter_mult = monoq_limiter_mult;
  ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> monoq_max_slope = monoq_max_slope;
  ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ptiny = ptiny;
  ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> elength = elength;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    qlc_monoq = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> qlc_monoq;
    qqc_monoq = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> qqc_monoq;
    monoq_limiter_mult = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> monoq_limiter_mult;
    monoq_max_slope = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> monoq_max_slope;
    ptiny = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> ptiny;
    elength = ((struct __hc_CalcMonotonicQRegionForElems__frame_t__ *)__hc_frame__) -> elength;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_1403_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long ielem_out;
  long ielem_end;
  struct Domain_t *domain;
  double qlc_monoq;
  double qqc_monoq;
  double monoq_limiter_mult;
  double monoq_max_slope;
  double ptiny;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  ielem_out = ((struct _async_1403_5frame_t__ *)__hc_frame__) -> ielem_out;
  ielem_end = ((struct _async_1403_5frame_t__ *)__hc_frame__) -> ielem_end;
  domain = ((struct _async_1403_5frame_t__ *)__hc_frame__) -> domain;
  qlc_monoq = ((struct _async_1403_5frame_t__ *)__hc_frame__) -> qlc_monoq;
  qqc_monoq = ((struct _async_1403_5frame_t__ *)__hc_frame__) -> qqc_monoq;
  monoq_limiter_mult = ((struct _async_1403_5frame_t__ *)__hc_frame__) -> monoq_limiter_mult;
  monoq_max_slope = ((struct _async_1403_5frame_t__ *)__hc_frame__) -> monoq_max_slope;
  ptiny = ((struct _async_1403_5frame_t__ *)__hc_frame__) -> ptiny;
  for (Index_t ielem = ielem_out; ielem < ielem_end; ++ielem) {
    Real_t qlin;
    Real_t qquad;
    Real_t phixi;
    Real_t phieta;
    Real_t phizeta;
    Index_t i = (domain -> m_matElemlist)[ielem];
    Int_t bcMask = (domain -> m_elemBC)[i];
    Real_t delvm;
    Real_t delvp;
/*  phixi     */
    Real_t norm = (((Real_t )1.0) / ((domain -> m_delv_xi)[i] + ptiny));
    switch(bcMask & 3){
      case 0:
{
        delvm = (domain -> m_delv_xi)[(domain -> m_lxim)[i]];
        break; 
      }
      case 1:
{
        delvm = (domain -> m_delv_xi)[i];
        break; 
      }
      case 2:
{
        delvm = ((Real_t )0.0);
        break; 
      }
/* ERROR */
      default:
{;
        break; 
      }
// switch XI_M
    }
    switch(bcMask & 0x00c){
      case 0:
{
        delvp = (domain -> m_delv_xi)[(domain -> m_lxip)[i]];
        break; 
      }
      case 4:
{
        delvp = (domain -> m_delv_xi)[i];
        break; 
      }
      case 8:
{
        delvp = ((Real_t )0.0);
        break; 
      }
/* ERROR */
      default:
{;
        break; 
      }
// switch XI_P
    }
    delvm = (delvm * norm);
    delvp = (delvp * norm);
    phixi = (((Real_t )0.5) * (delvm + delvp));
    delvm *= monoq_limiter_mult;
    delvp *= monoq_limiter_mult;
    if (delvm < phixi) 
      phixi = delvm;
    if (delvp < phixi) 
      phixi = delvp;
    if (phixi < ((Real_t )0.0)) 
      phixi = ((Real_t )0.0);
    if (phixi > monoq_max_slope) 
      phixi = monoq_max_slope;
/*  phieta     */
    norm = (((Real_t )1.0) / ((domain -> m_delv_eta)[i] + ptiny));
    switch(bcMask & 48){
      case 0:
{
        delvm = (domain -> m_delv_eta)[(domain -> m_letam)[i]];
        break; 
      }
      case 16:
{
        delvm = (domain -> m_delv_eta)[i];
        break; 
      }
      case 32:
{
        delvm = ((Real_t )0.0);
        break; 
      }
/* ERROR */
      default:
{;
        break; 
      }
// switch ETA_M
    }
    switch(bcMask & 0x0c0){
      case 0:
{
        delvp = (domain -> m_delv_eta)[(domain -> m_letap)[i]];
        break; 
      }
      case 64:
{
        delvp = (domain -> m_delv_eta)[i];
        break; 
      }
      case 128:
{
        delvp = ((Real_t )0.0);
        break; 
      }
/* ERROR */
      default:
{;
        break; 
      }
// switch ETA_P
    }
    delvm = (delvm * norm);
    delvp = (delvp * norm);
    phieta = (((Real_t )0.5) * (delvm + delvp));
    delvm *= monoq_limiter_mult;
    delvp *= monoq_limiter_mult;
    if (delvm < phieta) 
      phieta = delvm;
    if (delvp < phieta) 
      phieta = delvp;
    if (phieta < ((Real_t )0.0)) 
      phieta = ((Real_t )0.0);
    if (phieta > monoq_max_slope) 
      phieta = monoq_max_slope;
/*  phizeta     */
    norm = (((Real_t )1.0) / ((domain -> m_delv_zeta)[i] + ptiny));
    switch(bcMask & 0x300){
      case 0:
{
        delvm = (domain -> m_delv_zeta)[(domain -> m_lzetam)[i]];
        break; 
      }
      case 256:
{
        delvm = (domain -> m_delv_zeta)[i];
        break; 
      }
      case 512:
{
        delvm = ((Real_t )0.0);
        break; 
      }
/* ERROR */
      default:
{;
        break; 
      }
// switch ZETA_M
    }
    switch(bcMask & 0xc00){
      case 0:
{
        delvp = (domain -> m_delv_zeta)[(domain -> m_lzetap)[i]];
        break; 
      }
      case 1024:
{
        delvp = (domain -> m_delv_zeta)[i];
        break; 
      }
      case 2048:
{
        delvp = ((Real_t )0.0);
        break; 
      }
/* ERROR */
      default:
{;
        break; 
      }
// switch ZETA_P
    }
    delvm = (delvm * norm);
    delvp = (delvp * norm);
    phizeta = (((Real_t )0.5) * (delvm + delvp));
    delvm *= monoq_limiter_mult;
    delvp *= monoq_limiter_mult;
    if (delvm < phizeta) 
      phizeta = delvm;
    if (delvp < phizeta) 
      phizeta = delvp;
    if (phizeta < ((Real_t )0.0)) 
      phizeta = ((Real_t )0.0);
    if (phizeta > monoq_max_slope) 
      phizeta = monoq_max_slope;
/* Remove length scale */
    if ((domain -> m_vdov)[i] > ((Real_t )0.0)) {
      qlin = ((Real_t )0.0);
      qquad = ((Real_t )0.0);
    }
    else {
      Real_t delvxxi = ((domain -> m_delv_xi)[i] * (domain -> m_delx_xi)[i]);
      Real_t delvxeta = ((domain -> m_delv_eta)[i] * (domain -> m_delx_eta)[i]);
      Real_t delvxzeta = ((domain -> m_delv_zeta)[i] * (domain -> m_delx_zeta)[i]);
      if (delvxxi > ((Real_t )0.0)) 
        delvxxi = ((Real_t )0.0);
      if (delvxeta > ((Real_t )0.0)) 
        delvxeta = ((Real_t )0.0);
      if (delvxzeta > ((Real_t )0.0)) 
        delvxzeta = ((Real_t )0.0);
      Real_t rho = ((domain -> m_elemMass)[i] / ((domain -> m_volo)[i] * (domain -> m_vnew)[i]));
      qlin = ((-qlc_monoq * rho) * (((delvxxi * (((Real_t )1.0) - phixi)) + (delvxeta * (((Real_t )1.0) - phieta))) + (delvxzeta * (((Real_t )1.0) - phizeta))));
      qquad = ((qqc_monoq * rho) * ((((delvxxi * delvxxi) * (((Real_t )1.0) - (phixi * phixi))) + ((delvxeta * delvxeta) * (((Real_t )1.0) - (phieta * phieta)))) + ((delvxzeta * delvxzeta) * (((Real_t )1.0) - (phizeta * phizeta)))));
// if domain->m_vdov
    }
    (domain -> m_qq)[i] = qquad;
    (domain -> m_ql)[i] = qlin;
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcMonotonicQForElems__frame_t__ 
{
  struct hc_frameHeader header;
  double qlc_monoq;
  double qqc_monoq;
  double ptiny;
  double monoq_max_slope;
  double monoq_limiter_mult;
  long elength;
}
;
void __hc_CalcMonotonicQForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcMonotonicQForElems__(struct hc_workerState *ws)
{
  struct __hc_CalcMonotonicQForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcMonotonicQForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcMonotonicQForElems__frame_t__ ),__hc_CalcMonotonicQForElems__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcMonotonicQForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
  }
//
// initialize parameters
// 
  Real_t ptiny = (Real_t )1.e-36;
  Real_t monoq_max_slope = (domain -> m_monoq_max_slope);
  Real_t monoq_limiter_mult = (domain -> m_monoq_limiter_mult);
//
// calculate the monotonic q for pure regions
//
  Index_t elength = (domain -> m_numElem);
  if (elength > 0) {
    Real_t qlc_monoq = (domain -> m_qlc_monoq);
    Real_t qqc_monoq = (domain -> m_qqc_monoq);
// parameters
    __hc_frame__ -> pc = 1;
    ((struct __hc_CalcMonotonicQForElems__frame_t__ *)__hc_frame__) -> qlc_monoq = qlc_monoq;
    ((struct __hc_CalcMonotonicQForElems__frame_t__ *)__hc_frame__) -> qqc_monoq = qqc_monoq;
    ((struct __hc_CalcMonotonicQForElems__frame_t__ *)__hc_frame__) -> ptiny = ptiny;
    ((struct __hc_CalcMonotonicQForElems__frame_t__ *)__hc_frame__) -> monoq_max_slope = monoq_max_slope;
    ((struct __hc_CalcMonotonicQForElems__frame_t__ *)__hc_frame__) -> monoq_limiter_mult = monoq_limiter_mult;
    ((struct __hc_CalcMonotonicQForElems__frame_t__ *)__hc_frame__) -> elength = elength;
    struct hc_frameHeader *__hc_CalcMonotonicQRegionForElems__frame__1 = buildFrame___hc_CalcMonotonicQRegionForElems__(qlc_monoq,qqc_monoq,monoq_limiter_mult,monoq_max_slope,ptiny,elength,ws);
    __hc_CalcMonotonicQRegionForElems__(ws,((struct hc_frameHeader *)__hc_CalcMonotonicQRegionForElems__frame__1),0);
    if (ws -> costatus == HC_FUNC_COMPLETE) {
      hc_free_unwind_frame(ws);
    }
    else {
      return ;
    }
    if (0) {
      __post_func1:;
      __hc_frame__ = hc_free_unwind_frame(ws);
      qlc_monoq = ((struct __hc_CalcMonotonicQForElems__frame_t__ *)__hc_frame__) -> qlc_monoq;
      qqc_monoq = ((struct __hc_CalcMonotonicQForElems__frame_t__ *)__hc_frame__) -> qqc_monoq;
      ptiny = ((struct __hc_CalcMonotonicQForElems__frame_t__ *)__hc_frame__) -> ptiny;
      monoq_max_slope = ((struct __hc_CalcMonotonicQForElems__frame_t__ *)__hc_frame__) -> monoq_max_slope;
      monoq_limiter_mult = ((struct __hc_CalcMonotonicQForElems__frame_t__ *)__hc_frame__) -> monoq_limiter_mult;
      elength = ((struct __hc_CalcMonotonicQForElems__frame_t__ *)__hc_frame__) -> elength;
    }
// the elemset length
// if elength
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}
static uint64_t index_AMO;
static uint64_t *pIndex_AMO = &index_AMO;

struct __hc_CalcQForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  double qstop;
  long numElem;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
}
;
void __hc_CalcQForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcQForElems__(struct hc_workerState *ws)
{
  struct __hc_CalcQForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcQForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcQForElems__frame_t__ ),__hc_CalcQForElems__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1588_9frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double qstop;
  uint64_t *pIndex_AMO;
}
;
void _async_1588_9(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1588_9(long i_out,long i_end,struct Domain_t *domain,double qstop,uint64_t *pIndex_AMO,struct hc_workerState *ws)
{
  struct _async_1588_9frame_t__ *__hc_new_frame__ = (struct _async_1588_9frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1588_9frame_t__ ),_async_1588_9));
  ((struct _async_1588_9frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1588_9frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1588_9frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_1588_9frame_t__ *)__hc_new_frame__) -> qstop = qstop;
  ((struct _async_1588_9frame_t__ *)__hc_new_frame__) -> pIndex_AMO = pIndex_AMO;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcQForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
    case 2:
    goto __post_func2;
    case 3:
    goto __post_finish3;
    case 4:
    goto __post_async4;
  }
  Real_t qstop = (domain -> m_qstop);
  Index_t numElem = (domain -> m_numElem);
//
// MONOTONIC Q option
//
/* Calculate velocity gradients */
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> qstop = qstop;
  ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
  struct hc_frameHeader *__hc_CalcMonotonicQGradientsForElems__frame__1 = buildFrame___hc_CalcMonotonicQGradientsForElems__(ws);
  __hc_CalcMonotonicQGradientsForElems__(ws,((struct hc_frameHeader *)__hc_CalcMonotonicQGradientsForElems__frame__1),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func1:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    qstop = ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> qstop;
    numElem = ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> numElem;
  }
/* Transfer veloctiy gradients in the first order elements */
/* problem->commElements->Transfer(CommElements::monoQ) ; */
  __hc_frame__ -> pc = 2;
  ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> qstop = qstop;
  ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
  struct hc_frameHeader *__hc_CalcMonotonicQForElems__frame__2 = buildFrame___hc_CalcMonotonicQForElems__(ws);
  __hc_CalcMonotonicQForElems__(ws,((struct hc_frameHeader *)__hc_CalcMonotonicQForElems__frame__2),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func2:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    qstop = ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> qstop;
    numElem = ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> numElem;
  }
/* Don't allow excessive artificial viscosity */
  if (numElem != 0) {
#ifdef    UPC
#else  // UPC
     *pIndex_AMO = 0;
#endif // UPC
    hc_startFinish(ws);
{
      Index_t i_len = numElem;
      Index_t i_blk = 16;
      for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
        Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
        __hc_frame__ -> pc = 4;
        ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
        ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
        ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
        ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
        ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> qstop = qstop;
        ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
        struct hc_frameHeader *_async_1588_9frame__4 = buildFrame__async_1588_9(i_out,i_end,domain,qstop,pIndex_AMO,ws);
        _async_1588_9frame__4 -> sched_policy = hc_sched_policy(ws);
        if (_async_1588_9frame__4 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_1588_9frame__4);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_1588_9(ws,((struct hc_frameHeader *)_async_1588_9frame__4),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async4:;
          i_end = ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> i_end;
          i_out = ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> i_out;
          i_len = ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> i_len;
          i_blk = ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> i_blk;
          qstop = ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> qstop;
          numElem = ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> numElem;
        }
      }
    }
    __hc_frame__ -> pc = 3;
    ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> qstop = qstop;
    ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
    if (hc_stopFinish(ws)) 
      return ;
    if (0) {
      __post_finish3:;
      qstop = ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> qstop;
      numElem = ((struct __hc_CalcQForElems__frame_t__ *)__hc_frame__) -> numElem;
    }
#ifdef    UPC
// if pIndex_AMO
#else  // UPC
    if ( *pIndex_AMO > 0) {
      exit(QStopError);
// if pIndex_AMO
    }
#endif // UPC
// if numElem
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_1588_9(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double qstop;
  uint64_t *pIndex_AMO;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1588_9frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1588_9frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_1588_9frame_t__ *)__hc_frame__) -> domain;
  qstop = ((struct _async_1588_9frame_t__ *)__hc_frame__) -> qstop;
  pIndex_AMO = ((struct _async_1588_9frame_t__ *)__hc_frame__) -> pIndex_AMO;
  for (Index_t i = i_out; i < i_end; ++i) {
    if ((domain -> m_q)[i] > qstop) {
      AMO__sync_fetch_and_add_uint64_t(pIndex_AMO,1);
#if   !defined(CILK) && !defined(HAB_C)
#endif // CILK
// if domain->m_q
    }
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcPressureForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
  Real_t *p_new;
  Real_t *bvc;
  Real_t *pbvc;
  Real_t *e_old;
  Real_t *compression;
  Real_t *vnewc;
  double pmin;
  double p_cut;
  double eosvmax;
  long length;
}
;
void __hc_CalcPressureForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcPressureForElems__(Real_t *p_new,Real_t *bvc,Real_t *pbvc,Real_t *e_old,Real_t *compression,Real_t *vnewc,double pmin,double p_cut,double eosvmax,long length,struct hc_workerState *ws)
{
  struct __hc_CalcPressureForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcPressureForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcPressureForElems__frame_t__ ),__hc_CalcPressureForElems__));
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_new_frame__) -> p_new = p_new;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_new_frame__) -> bvc = bvc;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_new_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_new_frame__) -> e_old = e_old;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_new_frame__) -> compression = compression;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_new_frame__) -> pmin = pmin;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_new_frame__) -> p_cut = p_cut;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_new_frame__) -> eosvmax = eosvmax;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_new_frame__) -> length = length;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1619_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  Real_t *bvc;
  Real_t *pbvc;
  Real_t *compression;
}
;
void _async_1619_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1619_5(long i_out,long i_end,Real_t *bvc,Real_t *pbvc,Real_t *compression,struct hc_workerState *ws)
{
  struct _async_1619_5frame_t__ *__hc_new_frame__ = (struct _async_1619_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1619_5frame_t__ ),_async_1619_5));
  ((struct _async_1619_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1619_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1619_5frame_t__ *)__hc_new_frame__) -> bvc = bvc;
  ((struct _async_1619_5frame_t__ *)__hc_new_frame__) -> pbvc = pbvc;
  ((struct _async_1619_5frame_t__ *)__hc_new_frame__) -> compression = compression;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1627_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  Real_t *p_new;
  Real_t *bvc;
  Real_t *e_old;
  Real_t *vnewc;
  double pmin;
  double p_cut;
  double eosvmax;
}
;
void _async_1627_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1627_5(long i_out,long i_end,Real_t *p_new,Real_t *bvc,Real_t *e_old,Real_t *vnewc,double pmin,double p_cut,double eosvmax,struct hc_workerState *ws)
{
  struct _async_1627_5frame_t__ *__hc_new_frame__ = (struct _async_1627_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1627_5frame_t__ ),_async_1627_5));
  ((struct _async_1627_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1627_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1627_5frame_t__ *)__hc_new_frame__) -> p_new = p_new;
  ((struct _async_1627_5frame_t__ *)__hc_new_frame__) -> bvc = bvc;
  ((struct _async_1627_5frame_t__ *)__hc_new_frame__) -> e_old = e_old;
  ((struct _async_1627_5frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct _async_1627_5frame_t__ *)__hc_new_frame__) -> pmin = pmin;
  ((struct _async_1627_5frame_t__ *)__hc_new_frame__) -> p_cut = p_cut;
  ((struct _async_1627_5frame_t__ *)__hc_new_frame__) -> eosvmax = eosvmax;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcPressureForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  Real_t *p_new;
  Real_t *bvc;
  Real_t *pbvc;
  Real_t *e_old;
  Real_t *compression;
  Real_t *vnewc;
  double pmin;
  double p_cut;
  double eosvmax;
  long length;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
    case 3:
    goto __post_finish3;
    case 4:
    goto __post_async4;
  }
  p_new = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_new;
  bvc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> bvc;
  pbvc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pbvc;
  e_old = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> e_old;
  compression = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> compression;
  vnewc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> vnewc;
  pmin = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pmin;
  p_cut = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_cut;
  eosvmax = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> eosvmax;
  length = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> length;
  hc_startFinish(ws);
{
    Index_t i_len = length;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> compression = compression;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> length = length;
      struct hc_frameHeader *_async_1619_5frame__2 = buildFrame__async_1619_5(i_out,i_end,bvc,pbvc,compression,ws);
      _async_1619_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_1619_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1619_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1619_5(ws,((struct hc_frameHeader *)_async_1619_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_blk;
        p_new = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_new;
        bvc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> bvc;
        pbvc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pbvc;
        e_old = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> e_old;
        compression = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> compression;
        vnewc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> vnewc;
        pmin = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pmin;
        p_cut = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_cut;
        eosvmax = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> eosvmax;
        length = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> length;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> length = length;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    p_new = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_new;
    bvc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pbvc;
    e_old = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> e_old;
    compression = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> compression;
    vnewc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> vnewc;
    pmin = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pmin;
    p_cut = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_cut;
    eosvmax = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    length = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> length;
  }
  hc_startFinish(ws);
{
    Index_t i_len = length;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 4;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> compression = compression;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
      ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> length = length;
      struct hc_frameHeader *_async_1627_5frame__4 = buildFrame__async_1627_5(i_out,i_end,p_new,bvc,e_old,vnewc,pmin,p_cut,eosvmax,ws);
      _async_1627_5frame__4 -> sched_policy = hc_sched_policy(ws);
      if (_async_1627_5frame__4 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1627_5frame__4);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1627_5(ws,((struct hc_frameHeader *)_async_1627_5frame__4),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async4:;
        i_end = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> i_blk;
        p_new = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_new;
        bvc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> bvc;
        pbvc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pbvc;
        e_old = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> e_old;
        compression = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> compression;
        vnewc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> vnewc;
        pmin = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pmin;
        p_cut = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_cut;
        eosvmax = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> eosvmax;
        length = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> length;
      }
    }
  }
  __hc_frame__ -> pc = 3;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> length = length;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish3:;
    p_new = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_new;
    bvc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pbvc;
    e_old = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> e_old;
    compression = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> compression;
    vnewc = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> vnewc;
    pmin = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> pmin;
    p_cut = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> p_cut;
    eosvmax = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    length = ((struct __hc_CalcPressureForElems__frame_t__ *)__hc_frame__) -> length;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_1627_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  Real_t *p_new;
  Real_t *bvc;
  Real_t *e_old;
  Real_t *vnewc;
  double pmin;
  double p_cut;
  double eosvmax;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1627_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1627_5frame_t__ *)__hc_frame__) -> i_end;
  p_new = ((struct _async_1627_5frame_t__ *)__hc_frame__) -> p_new;
  bvc = ((struct _async_1627_5frame_t__ *)__hc_frame__) -> bvc;
  e_old = ((struct _async_1627_5frame_t__ *)__hc_frame__) -> e_old;
  vnewc = ((struct _async_1627_5frame_t__ *)__hc_frame__) -> vnewc;
  pmin = ((struct _async_1627_5frame_t__ *)__hc_frame__) -> pmin;
  p_cut = ((struct _async_1627_5frame_t__ *)__hc_frame__) -> p_cut;
  eosvmax = ((struct _async_1627_5frame_t__ *)__hc_frame__) -> eosvmax;
  for (Index_t i = i_out; i < i_end; ++i) {
    p_new[i] = (bvc[i] * e_old[i]);
    if (fabs(p_new[i]) < p_cut) 
      p_new[i] = ((Real_t )0.0);
/* impossible condition here? */
    if (vnewc[i] >= eosvmax) 
      p_new[i] = ((Real_t )0.0);
    if (p_new[i] < pmin) 
      p_new[i] = pmin;
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_1619_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  Real_t *bvc;
  Real_t *pbvc;
  Real_t *compression;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1619_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1619_5frame_t__ *)__hc_frame__) -> i_end;
  bvc = ((struct _async_1619_5frame_t__ *)__hc_frame__) -> bvc;
  pbvc = ((struct _async_1619_5frame_t__ *)__hc_frame__) -> pbvc;
  compression = ((struct _async_1619_5frame_t__ *)__hc_frame__) -> compression;
  for (Index_t i = i_out; i < i_end; ++i) {
    Real_t c1s = (((Real_t )2.0) / ((Real_t )3.0));
    bvc[i] = (c1s * (compression[i] + ((Real_t )1.0)));
    pbvc[i] = c1s;
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcEnergyForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  Real_t *pHalfStep;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
  Real_t *p_new;
  Real_t *e_new;
  Real_t *q_new;
  Real_t *bvc;
  Real_t *pbvc;
  Real_t *p_old;
  Real_t *e_old;
  Real_t *q_old;
  Real_t *compression;
  Real_t *compHalfStep;
  Real_t *vnewc;
  Real_t *work;
  Real_t *delvc;
  double pmin;
  double p_cut;
  double e_cut;
  double q_cut;
  double emin;
  Real_t *qq;
  Real_t *ql;
  double rho0;
  double eosvmax;
  long length;
}
;
void __hc_CalcEnergyForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcEnergyForElems__(Real_t *p_new,Real_t *e_new,Real_t *q_new,Real_t *bvc,Real_t *pbvc,Real_t *p_old,Real_t *e_old,Real_t *q_old,Real_t *compression,Real_t *compHalfStep,Real_t *vnewc,Real_t *work,Real_t *delvc,double pmin,double p_cut,double e_cut,double q_cut,double emin,Real_t *qq,Real_t *ql,double rho0,double eosvmax,long length,struct hc_workerState *ws)
{
  struct __hc_CalcEnergyForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcEnergyForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcEnergyForElems__frame_t__ ),__hc_CalcEnergyForElems__));
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> p_new = p_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> e_new = e_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> q_new = q_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> bvc = bvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> p_old = p_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> e_old = e_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> q_old = q_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> compression = compression;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> work = work;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> delvc = delvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> pmin = pmin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> p_cut = p_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> e_cut = e_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> q_cut = q_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> emin = emin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> qq = qq;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> ql = ql;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> rho0 = rho0;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> eosvmax = eosvmax;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_new_frame__) -> length = length;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1657_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  Real_t *e_new;
  Real_t *e_old;
  Real_t *delvc;
  Real_t *p_old;
  Real_t *q_old;
  Real_t *work;
  double emin;
}
;
void _async_1657_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1657_5(long i_out,long i_end,Real_t *e_new,Real_t *e_old,Real_t *delvc,Real_t *p_old,Real_t *q_old,Real_t *work,double emin,struct hc_workerState *ws)
{
  struct _async_1657_5frame_t__ *__hc_new_frame__ = (struct _async_1657_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1657_5frame_t__ ),_async_1657_5));
  ((struct _async_1657_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1657_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1657_5frame_t__ *)__hc_new_frame__) -> e_new = e_new;
  ((struct _async_1657_5frame_t__ *)__hc_new_frame__) -> e_old = e_old;
  ((struct _async_1657_5frame_t__ *)__hc_new_frame__) -> delvc = delvc;
  ((struct _async_1657_5frame_t__ *)__hc_new_frame__) -> p_old = p_old;
  ((struct _async_1657_5frame_t__ *)__hc_new_frame__) -> q_old = q_old;
  ((struct _async_1657_5frame_t__ *)__hc_new_frame__) -> work = work;
  ((struct _async_1657_5frame_t__ *)__hc_new_frame__) -> emin = emin;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1676_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  Real_t *compHalfStep;
  Real_t *q_new;
  Real_t *qq;
  Real_t *ql;
  Real_t *pbvc;
  Real_t *e_new;
  Real_t *bvc;
  Real_t *pHalfStep;
  double rho0;
  Real_t *delvc;
  Real_t *p_old;
  Real_t *q_old;
  Real_t *work;
  double e_cut;
  double emin;
}
;
void _async_1676_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1676_5(long i_out,long i_end,Real_t *compHalfStep,Real_t *q_new,Real_t *qq,Real_t *ql,Real_t *pbvc,Real_t *e_new,Real_t *bvc,Real_t *pHalfStep,double rho0,Real_t *delvc,Real_t *p_old,Real_t *q_old,Real_t *work,double e_cut,double emin,struct hc_workerState *ws)
{
  struct _async_1676_5frame_t__ *__hc_new_frame__ = (struct _async_1676_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1676_5frame_t__ ),_async_1676_5));
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> compHalfStep = compHalfStep;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> q_new = q_new;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> qq = qq;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> ql = ql;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> pbvc = pbvc;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> e_new = e_new;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> bvc = bvc;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> pHalfStep = pHalfStep;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> rho0 = rho0;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> delvc = delvc;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> p_old = p_old;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> q_old = q_old;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> work = work;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> e_cut = e_cut;
  ((struct _async_1676_5frame_t__ *)__hc_new_frame__) -> emin = emin;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1716_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  Real_t *delvc;
  Real_t *pbvc;
  Real_t *e_new;
  Real_t *vnewc;
  Real_t *bvc;
  Real_t *p_new;
  double rho0;
  Real_t *ql;
  Real_t *qq;
  Real_t *p_old;
  Real_t *q_old;
  Real_t *pHalfStep;
  Real_t *q_new;
  double e_cut;
  double emin;
}
;
void _async_1716_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1716_5(long i_out,long i_end,Real_t *delvc,Real_t *pbvc,Real_t *e_new,Real_t *vnewc,Real_t *bvc,Real_t *p_new,double rho0,Real_t *ql,Real_t *qq,Real_t *p_old,Real_t *q_old,Real_t *pHalfStep,Real_t *q_new,double e_cut,double emin,struct hc_workerState *ws)
{
  struct _async_1716_5frame_t__ *__hc_new_frame__ = (struct _async_1716_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1716_5frame_t__ ),_async_1716_5));
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> delvc = delvc;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> pbvc = pbvc;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> e_new = e_new;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> bvc = bvc;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> p_new = p_new;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> rho0 = rho0;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> ql = ql;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> qq = qq;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> p_old = p_old;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> q_old = q_old;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> pHalfStep = pHalfStep;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> q_new = q_new;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> e_cut = e_cut;
  ((struct _async_1716_5frame_t__ *)__hc_new_frame__) -> emin = emin;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1757_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  Real_t *delvc;
  Real_t *pbvc;
  Real_t *e_new;
  Real_t *vnewc;
  Real_t *bvc;
  Real_t *p_new;
  double rho0;
  Real_t *ql;
  Real_t *qq;
  Real_t *q_new;
  double q_cut;
}
;
void _async_1757_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1757_5(long i_out,long i_end,Real_t *delvc,Real_t *pbvc,Real_t *e_new,Real_t *vnewc,Real_t *bvc,Real_t *p_new,double rho0,Real_t *ql,Real_t *qq,Real_t *q_new,double q_cut,struct hc_workerState *ws)
{
  struct _async_1757_5frame_t__ *__hc_new_frame__ = (struct _async_1757_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1757_5frame_t__ ),_async_1757_5));
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> delvc = delvc;
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> pbvc = pbvc;
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> e_new = e_new;
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> bvc = bvc;
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> p_new = p_new;
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> rho0 = rho0;
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> ql = ql;
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> qq = qq;
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> q_new = q_new;
  ((struct _async_1757_5frame_t__ *)__hc_new_frame__) -> q_cut = q_cut;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcEnergyForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  Real_t *p_new;
  Real_t *e_new;
  Real_t *q_new;
  Real_t *bvc;
  Real_t *pbvc;
  Real_t *p_old;
  Real_t *e_old;
  Real_t *q_old;
  Real_t *compression;
  Real_t *compHalfStep;
  Real_t *vnewc;
  Real_t *work;
  Real_t *delvc;
  double pmin;
  double p_cut;
  double e_cut;
  double q_cut;
  double emin;
  Real_t *qq;
  Real_t *ql;
  double rho0;
  double eosvmax;
  long length;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
    case 3:
    goto __post_func3;
    case 4:
    goto __post_finish4;
    case 5:
    goto __post_async5;
    case 6:
    goto __post_func6;
    case 7:
    goto __post_finish7;
    case 8:
    goto __post_async8;
    case 9:
    goto __post_func9;
    case 10:
    goto __post_finish10;
    case 11:
    goto __post_async11;
  }
  p_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new;
  e_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new;
  q_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new;
  bvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc;
  pbvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc;
  p_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old;
  e_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old;
  q_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old;
  compression = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression;
  compHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
  vnewc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc;
  work = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work;
  delvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc;
  pmin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin;
  p_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut;
  e_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut;
  q_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut;
  emin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin;
  qq = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq;
  ql = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql;
  rho0 = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0;
  eosvmax = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax;
  length = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length;
  Real_t *pHalfStep = Allocate_Real_t(length);
  hc_startFinish(ws);
{
    Index_t i_len = length;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep = pHalfStep;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression = compression;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work = work;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin = emin;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq = qq;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql = ql;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length = length;
      struct hc_frameHeader *_async_1657_5frame__2 = buildFrame__async_1657_5(i_out,i_end,e_new,e_old,delvc,p_old,q_old,work,emin,ws);
      _async_1657_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_1657_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1657_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1657_5(ws,((struct hc_frameHeader *)_async_1657_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_blk;
        pHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep;
        p_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new;
        e_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new;
        q_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new;
        bvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc;
        pbvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc;
        p_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old;
        e_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old;
        q_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old;
        compression = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression;
        compHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
        vnewc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc;
        work = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work;
        delvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc;
        pmin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin;
        p_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut;
        e_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut;
        q_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut;
        emin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin;
        qq = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq;
        ql = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql;
        rho0 = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0;
        eosvmax = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax;
        length = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep = pHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work = work;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin = emin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq = qq;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql = ql;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length = length;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    pHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep;
    p_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new;
    e_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new;
    q_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new;
    bvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc;
    p_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old;
    e_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old;
    q_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old;
    compression = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression;
    compHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
    vnewc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc;
    work = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work;
    delvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc;
    pmin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin;
    p_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut;
    e_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut;
    q_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut;
    emin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin;
    qq = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq;
    ql = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql;
    rho0 = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0;
    eosvmax = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    length = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length;
  }
  __hc_frame__ -> pc = 3;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep = pHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work = work;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin = emin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq = qq;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql = ql;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length = length;
  struct hc_frameHeader *__hc_CalcPressureForElems__frame__3 = buildFrame___hc_CalcPressureForElems__(pHalfStep,bvc,pbvc,e_new,compHalfStep,vnewc,pmin,p_cut,eosvmax,length,ws);
  __hc_CalcPressureForElems__(ws,((struct hc_frameHeader *)__hc_CalcPressureForElems__frame__3),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func3:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    pHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep;
    p_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new;
    e_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new;
    q_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new;
    bvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc;
    p_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old;
    e_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old;
    q_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old;
    compression = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression;
    compHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
    vnewc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc;
    work = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work;
    delvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc;
    pmin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin;
    p_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut;
    e_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut;
    q_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut;
    emin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin;
    qq = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq;
    ql = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql;
    rho0 = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0;
    eosvmax = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    length = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length;
  }
  hc_startFinish(ws);
{
    Index_t i_len = length;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 5;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep = pHalfStep;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression = compression;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work = work;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin = emin;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq = qq;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql = ql;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length = length;
      struct hc_frameHeader *_async_1676_5frame__5 = buildFrame__async_1676_5(i_out,i_end,compHalfStep,q_new,qq,ql,pbvc,e_new,bvc,pHalfStep,rho0,delvc,p_old,q_old,work,e_cut,emin,ws);
      _async_1676_5frame__5 -> sched_policy = hc_sched_policy(ws);
      if (_async_1676_5frame__5 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1676_5frame__5);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1676_5(ws,((struct hc_frameHeader *)_async_1676_5frame__5),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async5:;
        i_end = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_blk;
        pHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep;
        p_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new;
        e_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new;
        q_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new;
        bvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc;
        pbvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc;
        p_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old;
        e_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old;
        q_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old;
        compression = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression;
        compHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
        vnewc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc;
        work = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work;
        delvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc;
        pmin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin;
        p_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut;
        e_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut;
        q_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut;
        emin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin;
        qq = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq;
        ql = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql;
        rho0 = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0;
        eosvmax = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax;
        length = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length;
      }
    }
  }
  __hc_frame__ -> pc = 4;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep = pHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work = work;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin = emin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq = qq;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql = ql;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length = length;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish4:;
    pHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep;
    p_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new;
    e_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new;
    q_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new;
    bvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc;
    p_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old;
    e_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old;
    q_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old;
    compression = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression;
    compHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
    vnewc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc;
    work = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work;
    delvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc;
    pmin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin;
    p_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut;
    e_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut;
    q_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut;
    emin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin;
    qq = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq;
    ql = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql;
    rho0 = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0;
    eosvmax = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    length = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length;
  }
  __hc_frame__ -> pc = 6;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep = pHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work = work;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin = emin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq = qq;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql = ql;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length = length;
  struct hc_frameHeader *__hc_CalcPressureForElems__frame__6 = buildFrame___hc_CalcPressureForElems__(p_new,bvc,pbvc,e_new,compression,vnewc,pmin,p_cut,eosvmax,length,ws);
  __hc_CalcPressureForElems__(ws,((struct hc_frameHeader *)__hc_CalcPressureForElems__frame__6),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func6:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    pHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep;
    p_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new;
    e_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new;
    q_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new;
    bvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc;
    p_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old;
    e_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old;
    q_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old;
    compression = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression;
    compHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
    vnewc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc;
    work = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work;
    delvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc;
    pmin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin;
    p_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut;
    e_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut;
    q_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut;
    emin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin;
    qq = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq;
    ql = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql;
    rho0 = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0;
    eosvmax = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    length = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length;
  }
  hc_startFinish(ws);
{
    Index_t i_len = length;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 8;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep = pHalfStep;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression = compression;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work = work;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin = emin;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq = qq;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql = ql;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length = length;
      struct hc_frameHeader *_async_1716_5frame__8 = buildFrame__async_1716_5(i_out,i_end,delvc,pbvc,e_new,vnewc,bvc,p_new,rho0,ql,qq,p_old,q_old,pHalfStep,q_new,e_cut,emin,ws);
      _async_1716_5frame__8 -> sched_policy = hc_sched_policy(ws);
      if (_async_1716_5frame__8 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1716_5frame__8);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1716_5(ws,((struct hc_frameHeader *)_async_1716_5frame__8),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async8:;
        i_end = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_blk;
        pHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep;
        p_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new;
        e_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new;
        q_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new;
        bvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc;
        pbvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc;
        p_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old;
        e_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old;
        q_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old;
        compression = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression;
        compHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
        vnewc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc;
        work = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work;
        delvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc;
        pmin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin;
        p_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut;
        e_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut;
        q_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut;
        emin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin;
        qq = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq;
        ql = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql;
        rho0 = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0;
        eosvmax = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax;
        length = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length;
      }
    }
  }
  __hc_frame__ -> pc = 7;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep = pHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work = work;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin = emin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq = qq;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql = ql;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length = length;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish7:;
    pHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep;
    p_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new;
    e_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new;
    q_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new;
    bvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc;
    p_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old;
    e_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old;
    q_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old;
    compression = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression;
    compHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
    vnewc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc;
    work = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work;
    delvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc;
    pmin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin;
    p_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut;
    e_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut;
    q_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut;
    emin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin;
    qq = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq;
    ql = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql;
    rho0 = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0;
    eosvmax = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    length = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length;
  }
  __hc_frame__ -> pc = 9;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep = pHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work = work;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin = emin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq = qq;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql = ql;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length = length;
  struct hc_frameHeader *__hc_CalcPressureForElems__frame__9 = buildFrame___hc_CalcPressureForElems__(p_new,bvc,pbvc,e_new,compression,vnewc,pmin,p_cut,eosvmax,length,ws);
  __hc_CalcPressureForElems__(ws,((struct hc_frameHeader *)__hc_CalcPressureForElems__frame__9),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func9:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    pHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep;
    p_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new;
    e_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new;
    q_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new;
    bvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc;
    p_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old;
    e_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old;
    q_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old;
    compression = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression;
    compHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
    vnewc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc;
    work = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work;
    delvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc;
    pmin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin;
    p_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut;
    e_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut;
    q_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut;
    emin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin;
    qq = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq;
    ql = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql;
    rho0 = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0;
    eosvmax = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    length = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length;
  }
//DEBUG fprintf(stdout," e_new6= %e\n",e_new[0]);
  hc_startFinish(ws);
{
    Index_t i_len = length;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 11;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep = pHalfStep;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression = compression;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work = work;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin = emin;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq = qq;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql = ql;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
      ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length = length;
      struct hc_frameHeader *_async_1757_5frame__11 = buildFrame__async_1757_5(i_out,i_end,delvc,pbvc,e_new,vnewc,bvc,p_new,rho0,ql,qq,q_new,q_cut,ws);
      _async_1757_5frame__11 -> sched_policy = hc_sched_policy(ws);
      if (_async_1757_5frame__11 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1757_5frame__11);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1757_5(ws,((struct hc_frameHeader *)_async_1757_5frame__11),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async11:;
        i_end = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> i_blk;
        pHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep;
        p_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new;
        e_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new;
        q_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new;
        bvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc;
        pbvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc;
        p_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old;
        e_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old;
        q_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old;
        compression = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression;
        compHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
        vnewc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc;
        work = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work;
        delvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc;
        pmin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin;
        p_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut;
        e_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut;
        q_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut;
        emin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin;
        qq = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq;
        ql = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql;
        rho0 = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0;
        eosvmax = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax;
        length = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length;
      }
    }
  }
  __hc_frame__ -> pc = 10;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep = pHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work = work;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin = emin;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq = qq;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql = ql;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length = length;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish10:;
    pHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pHalfStep;
    p_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_new;
    e_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_new;
    q_new = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_new;
    bvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pbvc;
    p_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_old;
    e_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_old;
    q_old = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_old;
    compression = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compression;
    compHalfStep = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
    vnewc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> vnewc;
    work = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> work;
    delvc = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> delvc;
    pmin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> pmin;
    p_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> p_cut;
    e_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> e_cut;
    q_cut = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> q_cut;
    emin = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> emin;
    qq = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> qq;
    ql = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> ql;
    rho0 = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> rho0;
    eosvmax = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    length = ((struct __hc_CalcEnergyForElems__frame_t__ *)__hc_frame__) -> length;
  }
  Release_Real_t(pHalfStep);
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_1757_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  Real_t *delvc;
  Real_t *pbvc;
  Real_t *e_new;
  Real_t *vnewc;
  Real_t *bvc;
  Real_t *p_new;
  double rho0;
  Real_t *ql;
  Real_t *qq;
  Real_t *q_new;
  double q_cut;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> i_end;
  delvc = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> delvc;
  pbvc = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> pbvc;
  e_new = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> e_new;
  vnewc = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> vnewc;
  bvc = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> bvc;
  p_new = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> p_new;
  rho0 = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> rho0;
  ql = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> ql;
  qq = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> qq;
  q_new = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> q_new;
  q_cut = ((struct _async_1757_5frame_t__ *)__hc_frame__) -> q_cut;
  for (Index_t i = i_out; i < i_end; ++i) {
    if (delvc[i] <= ((Real_t )0.0)) {
      Real_t ssc = (((pbvc[i] * e_new[i]) + (((vnewc[i] * vnewc[i]) * bvc[i]) * p_new[i])) / rho0);
      if (ssc <= ((Real_t )0.0)) {
        ssc = ((Real_t ).333333e-36);
      }
      else {
        ssc = sqrt(ssc);
// if ssc
      }
      q_new[i] = ((ssc * ql[i]) + qq[i]);
      if (fabs(q_new[i]) < q_cut) 
        q_new[i] = ((Real_t )0.0);
// if delvc
    }
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_1716_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  Real_t *delvc;
  Real_t *pbvc;
  Real_t *e_new;
  Real_t *vnewc;
  Real_t *bvc;
  Real_t *p_new;
  double rho0;
  Real_t *ql;
  Real_t *qq;
  Real_t *p_old;
  Real_t *q_old;
  Real_t *pHalfStep;
  Real_t *q_new;
  double e_cut;
  double emin;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> i_end;
  delvc = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> delvc;
  pbvc = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> pbvc;
  e_new = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> e_new;
  vnewc = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> vnewc;
  bvc = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> bvc;
  p_new = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> p_new;
  rho0 = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> rho0;
  ql = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> ql;
  qq = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> qq;
  p_old = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> p_old;
  q_old = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> q_old;
  pHalfStep = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> pHalfStep;
  q_new = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> q_new;
  e_cut = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> e_cut;
  emin = ((struct _async_1716_5frame_t__ *)__hc_frame__) -> emin;
  for (Index_t i = i_out; i < i_end; ++i) {
    Real_t sixth = (((Real_t )1.0) / ((Real_t )6.0));
    Real_t q_tilde;
    if (delvc[i] > ((Real_t )0.0)) {
      q_tilde = ((Real_t )0.0);
    }
    else {
      Real_t ssc = (((pbvc[i] * e_new[i]) + (((vnewc[i] * vnewc[i]) * bvc[i]) * p_new[i])) / rho0);
      if (ssc <= ((Real_t )0.0)) {
        ssc = ((Real_t ).333333e-36);
      }
      else {
        ssc = sqrt(ssc);
// if ssc
      }
      q_tilde = ((ssc * ql[i]) + qq[i]);
// if delvc
    }
    e_new[i] += ((-(((((Real_t )7.0) * (p_old[i] + q_old[i])) - (((Real_t )8.) * (pHalfStep[i] + q_new[i]))) + (p_new[i] + q_tilde)) * delvc[i]) * sixth);
//DEBUG if(i==0)fprintf(stdout," e_new4= %e\n",e_new[0]);
    if (fabs(e_new[i]) < e_cut) {
      e_new[i] = ((Real_t )0.0);
// if e_cut
    }
    if (e_new[i] < emin) {
      e_new[i] = emin;
// if emin
    }
//DEBUG if(i==0)fprintf(stdout," e_new5= %e\n",e_new[0]);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_1676_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  Real_t *compHalfStep;
  Real_t *q_new;
  Real_t *qq;
  Real_t *ql;
  Real_t *pbvc;
  Real_t *e_new;
  Real_t *bvc;
  Real_t *pHalfStep;
  double rho0;
  Real_t *delvc;
  Real_t *p_old;
  Real_t *q_old;
  Real_t *work;
  double e_cut;
  double emin;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> i_end;
  compHalfStep = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> compHalfStep;
  q_new = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> q_new;
  qq = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> qq;
  ql = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> ql;
  pbvc = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> pbvc;
  e_new = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> e_new;
  bvc = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> bvc;
  pHalfStep = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> pHalfStep;
  rho0 = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> rho0;
  delvc = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> delvc;
  p_old = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> p_old;
  q_old = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> q_old;
  work = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> work;
  e_cut = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> e_cut;
  emin = ((struct _async_1676_5frame_t__ *)__hc_frame__) -> emin;
  for (Index_t i = i_out; i < i_end; ++i) {
    Real_t vhalf = (((Real_t )1.0) / (((Real_t )1.0) + compHalfStep[i]));
    if (delvc[i] > ((Real_t )0.0)) {
/* = qq[i] = ql[i] */
      q_new[i] = ((Real_t )0.0);
    }
    else {
      Real_t ssc = (((pbvc[i] * e_new[i]) + (((vhalf * vhalf) * bvc[i]) * pHalfStep[i])) / rho0);
      if (ssc <= ((Real_t )0.0)) {
        ssc = ((Real_t ).333333e-36);
      }
      else {
        ssc = sqrt(ssc);
// if ssc
      }
      q_new[i] = ((ssc * ql[i]) + qq[i]);
// if delvc
    }
    e_new[i] += ((((Real_t )0.5) * delvc[i]) * ((((Real_t )3.0) * (p_old[i] + q_old[i])) - (((Real_t )4.0) * (pHalfStep[i] + q_new[i]))));
//DEBUG if(i==0)fprintf(stdout," e_new1= %e\n",e_new[0]);
    e_new[i] += (((Real_t )0.5) * work[i]);
//DEBUG if(i==0)fprintf(stdout," e_new2= %e\n",e_new[0]);
    if (fabs(e_new[i]) < e_cut) {
      e_new[i] = ((Real_t )0.0);
// e_cut
    }
    if (e_new[i] < emin) {
      e_new[i] = emin;
// if emin
    }
//DEBUG if(i==0)fprintf(stdout," e_new3= %e\n",e_new[0]);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_1657_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  Real_t *e_new;
  Real_t *e_old;
  Real_t *delvc;
  Real_t *p_old;
  Real_t *q_old;
  Real_t *work;
  double emin;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1657_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1657_5frame_t__ *)__hc_frame__) -> i_end;
  e_new = ((struct _async_1657_5frame_t__ *)__hc_frame__) -> e_new;
  e_old = ((struct _async_1657_5frame_t__ *)__hc_frame__) -> e_old;
  delvc = ((struct _async_1657_5frame_t__ *)__hc_frame__) -> delvc;
  p_old = ((struct _async_1657_5frame_t__ *)__hc_frame__) -> p_old;
  q_old = ((struct _async_1657_5frame_t__ *)__hc_frame__) -> q_old;
  work = ((struct _async_1657_5frame_t__ *)__hc_frame__) -> work;
  emin = ((struct _async_1657_5frame_t__ *)__hc_frame__) -> emin;
  for (Index_t i = i_out; i < i_end; ++i) {
//DEBUG if(i==0)fprintf(stdout," e_old = %e\n",e_old[i]);
//DEBUG if(i==0)fprintf(stdout," p_old = %e\n",p_old[i])
//DEBUG if(i==0)fprintf(stdout," q_old = %e\n",q_old[i]);
//DEBUG if(i==0)fprintf(stdout," delvc = %e\n",delvc[i]);
//DEBUG if(i==0)fprintf(stdout," work  = %e\n",work[i] );
    e_new[i] = ((e_old[i] - ((((Real_t )0.5) * delvc[i]) * (p_old[i] + q_old[i]))) + (((Real_t )0.5) * work[i]));
//DEBUG if(i==0)fprintf(stdout," e_new0= %e\n",e_new[i]);
    if (e_new[i] < emin) {
      e_new[i] = emin;
    }
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcSoundSpeedForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
  Real_t *vnewc;
  double rho0;
  Real_t *enewc;
  Real_t *pnewc;
  Real_t *pbvc;
  Real_t *bvc;
  double ss4o3;
  long nz;
}
;
void __hc_CalcSoundSpeedForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcSoundSpeedForElems__(Real_t *vnewc,double rho0,Real_t *enewc,Real_t *pnewc,Real_t *pbvc,Real_t *bvc,double ss4o3,long nz,struct hc_workerState *ws)
{
  struct __hc_CalcSoundSpeedForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcSoundSpeedForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcSoundSpeedForElems__frame_t__ ),__hc_CalcSoundSpeedForElems__));
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_new_frame__) -> rho0 = rho0;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_new_frame__) -> enewc = enewc;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_new_frame__) -> pnewc = pnewc;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_new_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_new_frame__) -> bvc = bvc;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_new_frame__) -> ss4o3 = ss4o3;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_new_frame__) -> nz = nz;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1785_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  Real_t *pbvc;
  Real_t *enewc;
  Real_t *vnewc;
  Real_t *bvc;
  Real_t *pnewc;
  double rho0;
}
;
void _async_1785_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1785_5(long i_out,long i_end,struct Domain_t *domain,Real_t *pbvc,Real_t *enewc,Real_t *vnewc,Real_t *bvc,Real_t *pnewc,double rho0,struct hc_workerState *ws)
{
  struct _async_1785_5frame_t__ *__hc_new_frame__ = (struct _async_1785_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1785_5frame_t__ ),_async_1785_5));
  ((struct _async_1785_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1785_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1785_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_1785_5frame_t__ *)__hc_new_frame__) -> pbvc = pbvc;
  ((struct _async_1785_5frame_t__ *)__hc_new_frame__) -> enewc = enewc;
  ((struct _async_1785_5frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct _async_1785_5frame_t__ *)__hc_new_frame__) -> bvc = bvc;
  ((struct _async_1785_5frame_t__ *)__hc_new_frame__) -> pnewc = pnewc;
  ((struct _async_1785_5frame_t__ *)__hc_new_frame__) -> rho0 = rho0;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcSoundSpeedForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  Real_t *vnewc;
  double rho0;
  Real_t *enewc;
  Real_t *pnewc;
  Real_t *pbvc;
  Real_t *bvc;
  double ss4o3;
  long nz;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  vnewc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> vnewc;
  rho0 = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> rho0;
  enewc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> enewc;
  pnewc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> pnewc;
  pbvc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> pbvc;
  bvc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> bvc;
  ss4o3 = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> ss4o3;
  nz = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> nz;
  hc_startFinish(ws);
{
    Index_t i_len = nz;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
      ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
      ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> enewc = enewc;
      ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> pnewc = pnewc;
      ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
      ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
      ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> ss4o3 = ss4o3;
      ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> nz = nz;
      struct hc_frameHeader *_async_1785_5frame__2 = buildFrame__async_1785_5(i_out,i_end,domain,pbvc,enewc,vnewc,bvc,pnewc,rho0,ws);
      _async_1785_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_1785_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1785_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1785_5(ws,((struct hc_frameHeader *)_async_1785_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> i_blk;
        vnewc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> vnewc;
        rho0 = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> rho0;
        enewc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> enewc;
        pnewc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> pnewc;
        pbvc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> pbvc;
        bvc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> bvc;
        ss4o3 = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> ss4o3;
        nz = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> nz;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> enewc = enewc;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> pnewc = pnewc;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> ss4o3 = ss4o3;
  ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> nz = nz;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    vnewc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> vnewc;
    rho0 = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> rho0;
    enewc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> enewc;
    pnewc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> pnewc;
    pbvc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> pbvc;
    bvc = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> bvc;
    ss4o3 = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> ss4o3;
    nz = ((struct __hc_CalcSoundSpeedForElems__frame_t__ *)__hc_frame__) -> nz;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_1785_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  Real_t *pbvc;
  Real_t *enewc;
  Real_t *vnewc;
  Real_t *bvc;
  Real_t *pnewc;
  double rho0;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1785_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1785_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_1785_5frame_t__ *)__hc_frame__) -> domain;
  pbvc = ((struct _async_1785_5frame_t__ *)__hc_frame__) -> pbvc;
  enewc = ((struct _async_1785_5frame_t__ *)__hc_frame__) -> enewc;
  vnewc = ((struct _async_1785_5frame_t__ *)__hc_frame__) -> vnewc;
  bvc = ((struct _async_1785_5frame_t__ *)__hc_frame__) -> bvc;
  pnewc = ((struct _async_1785_5frame_t__ *)__hc_frame__) -> pnewc;
  rho0 = ((struct _async_1785_5frame_t__ *)__hc_frame__) -> rho0;
  for (Index_t i = i_out; i < i_end; ++i) {
    Index_t iz = (domain -> m_matElemlist)[i];
    Real_t ssTmp = (((pbvc[i] * enewc[i]) + (((vnewc[i] * vnewc[i]) * bvc[i]) * pnewc[i])) / rho0);
    if (ssTmp <= ((Real_t )1.111111e-36)) {
      ssTmp = ((Real_t )1.111111e-36);
// if ssTmp
    }
    (domain -> m_ss)[iz] = sqrt(ssTmp);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_EvalEOSForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  double e_cut;
  double p_cut;
  double ss4o3;
  double q_cut;
  double eosvmax;
  double eosvmin;
  double pmin;
  double emin;
  double rho0;
  Real_t *e_old;
  Real_t *delvc;
  Real_t *p_old;
  Real_t *q_old;
  Real_t *compression;
  Real_t *compHalfStep;
  Real_t *qq;
  Real_t *ql;
  Real_t *work;
  Real_t *p_new;
  Real_t *e_new;
  Real_t *q_new;
  Real_t *bvc;
  Real_t *pbvc;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
  long i_len_s0;
  long i_blk_s1;
  long i_len_s2;
  long i_blk_s3;
  Real_t *vnewc;
  long length;
}
;
void __hc_EvalEOSForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_EvalEOSForElems__(Real_t *vnewc,long length,struct hc_workerState *ws)
{
  struct __hc_EvalEOSForElems__frame_t__ *__hc_new_frame__ = (struct __hc_EvalEOSForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_EvalEOSForElems__frame_t__ ),__hc_EvalEOSForElems__));
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_new_frame__) -> length = length;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1830_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  Real_t *delvc;
  Real_t *e_old;
  Real_t *p_old;
  Real_t *q_old;
  Real_t *qq;
  Real_t *ql;
}
;
void _async_1830_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1830_5(long i_out,long i_end,struct Domain_t *domain,Real_t *delvc,Real_t *e_old,Real_t *p_old,Real_t *q_old,Real_t *qq,Real_t *ql,struct hc_workerState *ws)
{
  struct _async_1830_5frame_t__ *__hc_new_frame__ = (struct _async_1830_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1830_5frame_t__ ),_async_1830_5));
  ((struct _async_1830_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1830_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1830_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_1830_5frame_t__ *)__hc_new_frame__) -> delvc = delvc;
  ((struct _async_1830_5frame_t__ *)__hc_new_frame__) -> e_old = e_old;
  ((struct _async_1830_5frame_t__ *)__hc_new_frame__) -> p_old = p_old;
  ((struct _async_1830_5frame_t__ *)__hc_new_frame__) -> q_old = q_old;
  ((struct _async_1830_5frame_t__ *)__hc_new_frame__) -> qq = qq;
  ((struct _async_1830_5frame_t__ *)__hc_new_frame__) -> ql = ql;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1846_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  Real_t *compression;
  Real_t *vnewc;
  Real_t *delvc;
  Real_t *compHalfStep;
  Real_t *work;
  double eosvmin;
  double eosvmax;
  Real_t *p_old;
}
;
void _async_1846_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1846_5(long i_out,long i_end,Real_t *compression,Real_t *vnewc,Real_t *delvc,Real_t *compHalfStep,Real_t *work,double eosvmin,double eosvmax,Real_t *p_old,struct hc_workerState *ws)
{
  struct _async_1846_5frame_t__ *__hc_new_frame__ = (struct _async_1846_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1846_5frame_t__ ),_async_1846_5));
  ((struct _async_1846_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1846_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1846_5frame_t__ *)__hc_new_frame__) -> compression = compression;
  ((struct _async_1846_5frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct _async_1846_5frame_t__ *)__hc_new_frame__) -> delvc = delvc;
  ((struct _async_1846_5frame_t__ *)__hc_new_frame__) -> compHalfStep = compHalfStep;
  ((struct _async_1846_5frame_t__ *)__hc_new_frame__) -> work = work;
  ((struct _async_1846_5frame_t__ *)__hc_new_frame__) -> eosvmin = eosvmin;
  ((struct _async_1846_5frame_t__ *)__hc_new_frame__) -> eosvmax = eosvmax;
  ((struct _async_1846_5frame_t__ *)__hc_new_frame__) -> p_old = p_old;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1857_7frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  Real_t *compression;
  Real_t *vnewc;
  Real_t *delvc;
  Real_t *compHalfStep;
  Real_t *work;
  double eosvmin;
  double eosvmax;
  Real_t *p_old;
}
;
void _async_1857_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1857_7(long i_out,long i_end,Real_t *compression,Real_t *vnewc,Real_t *delvc,Real_t *compHalfStep,Real_t *work,double eosvmin,double eosvmax,Real_t *p_old,struct hc_workerState *ws)
{
  struct _async_1857_7frame_t__ *__hc_new_frame__ = (struct _async_1857_7frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1857_7frame_t__ ),_async_1857_7));
  ((struct _async_1857_7frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1857_7frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1857_7frame_t__ *)__hc_new_frame__) -> compression = compression;
  ((struct _async_1857_7frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct _async_1857_7frame_t__ *)__hc_new_frame__) -> delvc = delvc;
  ((struct _async_1857_7frame_t__ *)__hc_new_frame__) -> compHalfStep = compHalfStep;
  ((struct _async_1857_7frame_t__ *)__hc_new_frame__) -> work = work;
  ((struct _async_1857_7frame_t__ *)__hc_new_frame__) -> eosvmin = eosvmin;
  ((struct _async_1857_7frame_t__ *)__hc_new_frame__) -> eosvmax = eosvmax;
  ((struct _async_1857_7frame_t__ *)__hc_new_frame__) -> p_old = p_old;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1865_7frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  Real_t *compression;
  Real_t *vnewc;
  Real_t *delvc;
  Real_t *compHalfStep;
  Real_t *work;
  double eosvmin;
  double eosvmax;
  Real_t *p_old;
}
;
void _async_1865_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1865_7(long i_out,long i_end,Real_t *compression,Real_t *vnewc,Real_t *delvc,Real_t *compHalfStep,Real_t *work,double eosvmin,double eosvmax,Real_t *p_old,struct hc_workerState *ws)
{
  struct _async_1865_7frame_t__ *__hc_new_frame__ = (struct _async_1865_7frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1865_7frame_t__ ),_async_1865_7));
  ((struct _async_1865_7frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1865_7frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1865_7frame_t__ *)__hc_new_frame__) -> compression = compression;
  ((struct _async_1865_7frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct _async_1865_7frame_t__ *)__hc_new_frame__) -> delvc = delvc;
  ((struct _async_1865_7frame_t__ *)__hc_new_frame__) -> compHalfStep = compHalfStep;
  ((struct _async_1865_7frame_t__ *)__hc_new_frame__) -> work = work;
  ((struct _async_1865_7frame_t__ *)__hc_new_frame__) -> eosvmin = eosvmin;
  ((struct _async_1865_7frame_t__ *)__hc_new_frame__) -> eosvmax = eosvmax;
  ((struct _async_1865_7frame_t__ *)__hc_new_frame__) -> p_old = p_old;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1885_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  Real_t *p_new;
  Real_t *e_new;
  Real_t *q_new;
}
;
void _async_1885_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1885_5(long i_out,long i_end,struct Domain_t *domain,Real_t *p_new,Real_t *e_new,Real_t *q_new,struct hc_workerState *ws)
{
  struct _async_1885_5frame_t__ *__hc_new_frame__ = (struct _async_1885_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1885_5frame_t__ ),_async_1885_5));
  ((struct _async_1885_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1885_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1885_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_1885_5frame_t__ *)__hc_new_frame__) -> p_new = p_new;
  ((struct _async_1885_5frame_t__ *)__hc_new_frame__) -> e_new = e_new;
  ((struct _async_1885_5frame_t__ *)__hc_new_frame__) -> q_new = q_new;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_EvalEOSForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  Real_t *vnewc;
  long length;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
    case 3:
    goto __post_finish3;
    case 4:
    goto __post_async4;
    case 5:
    goto __post_async5;
    case 6:
    goto __post_async6;
    case 7:
    goto __post_func7;
    case 8:
    goto __post_finish8;
    case 9:
    goto __post_async9;
    case 10:
    goto __post_func10;
  }
  vnewc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc;
  length = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length;
  Real_t e_cut = (domain -> m_e_cut);
  Real_t p_cut = (domain -> m_p_cut);
  Real_t ss4o3 = (domain -> m_ss4o3);
  Real_t q_cut = (domain -> m_q_cut);
  Real_t eosvmax = (domain -> m_eosvmax);
  Real_t eosvmin = (domain -> m_eosvmin);
  Real_t pmin = (domain -> m_pmin);
  Real_t emin = (domain -> m_emin);
  Real_t rho0 = (domain -> m_refdens);
  Real_t *e_old = Allocate_Real_t(length);
  Real_t *delvc = Allocate_Real_t(length);
  Real_t *p_old = Allocate_Real_t(length);
  Real_t *q_old = Allocate_Real_t(length);
  Real_t *compression = Allocate_Real_t(length);
  Real_t *compHalfStep = Allocate_Real_t(length);
  Real_t *qq = Allocate_Real_t(length);
  Real_t *ql = Allocate_Real_t(length);
  Real_t *work = Allocate_Real_t(length);
  Real_t *p_new = Allocate_Real_t(length);
  Real_t *e_new = Allocate_Real_t(length);
  Real_t *q_new = Allocate_Real_t(length);
  Real_t *bvc = Allocate_Real_t(length);
  Real_t *pbvc = Allocate_Real_t(length);;
// RAG GATHERS
  hc_startFinish(ws);
{
    Index_t i_len = length;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3 = ss4o3;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin = emin;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression = compression;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq = qq;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql = ql;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work = work;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length = length;
      struct hc_frameHeader *_async_1830_5frame__2 = buildFrame__async_1830_5(i_out,i_end,domain,delvc,e_old,p_old,q_old,qq,ql,ws);
      _async_1830_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_1830_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1830_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1830_5(ws,((struct hc_frameHeader *)_async_1830_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk;
        e_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut;
        p_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut;
        ss4o3 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3;
        q_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut;
        eosvmax = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax;
        eosvmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin;
        pmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin;
        emin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin;
        rho0 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0;
        e_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old;
        delvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc;
        p_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old;
        q_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old;
        compression = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression;
        compHalfStep = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
        qq = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq;
        ql = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql;
        work = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work;
        p_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new;
        e_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new;
        q_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new;
        bvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc;
        pbvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc;
        vnewc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc;
        length = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3 = ss4o3;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin = emin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq = qq;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql = ql;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work = work;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length = length;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    e_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut;
    p_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut;
    ss4o3 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3;
    q_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut;
    eosvmax = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    eosvmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin;
    pmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin;
    emin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin;
    rho0 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0;
    e_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old;
    delvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc;
    p_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old;
    q_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old;
    compression = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression;
    compHalfStep = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
    qq = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq;
    ql = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql;
    work = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work;
    p_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new;
    e_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new;
    q_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new;
    bvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc;
    vnewc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc;
    length = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length;
  }
// RAG STRIDE ONE
  hc_startFinish(ws);
{
    Index_t i_len = length;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 4;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3 = ss4o3;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin = emin;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression = compression;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq = qq;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql = ql;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work = work;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length = length;
      struct hc_frameHeader *_async_1846_5frame__4 = buildFrame__async_1846_5(i_out,i_end,compression,vnewc,delvc,compHalfStep,work,eosvmin,eosvmax,p_old,ws);
      _async_1846_5frame__4 -> sched_policy = hc_sched_policy(ws);
      if (_async_1846_5frame__4 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1846_5frame__4);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1846_5(ws,((struct hc_frameHeader *)_async_1846_5frame__4),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async4:;
        i_end = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk;
        e_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut;
        p_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut;
        ss4o3 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3;
        q_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut;
        eosvmax = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax;
        eosvmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin;
        pmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin;
        emin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin;
        rho0 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0;
        e_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old;
        delvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc;
        p_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old;
        q_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old;
        compression = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression;
        compHalfStep = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
        qq = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq;
        ql = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql;
        work = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work;
        p_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new;
        e_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new;
        q_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new;
        bvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc;
        pbvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc;
        vnewc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc;
        length = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length;
      }
    };
    if (eosvmin != ((Real_t )0.0)) {
      Index_t i_len_s0 = length;
      Index_t i_blk_s1 = 16;
      for (Index_t i_out = 0; i_out < i_len_s0; i_out += i_blk_s1) {
        Index_t i_end = ((i_out + i_blk_s1) < i_len_s0)?(i_out + i_blk_s1) : i_len_s0;
        __hc_frame__ -> pc = 5;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len_s0 = i_len_s0;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk_s1 = i_blk_s1;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3 = ss4o3;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin = emin;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression = compression;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq = qq;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql = ql;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work = work;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length = length;
        struct hc_frameHeader *_async_1857_7frame__5 = buildFrame__async_1857_7(i_out,i_end,compression,vnewc,delvc,compHalfStep,work,eosvmin,eosvmax,p_old,ws);
        _async_1857_7frame__5 -> sched_policy = hc_sched_policy(ws);
        if (_async_1857_7frame__5 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_1857_7frame__5);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_1857_7(ws,((struct hc_frameHeader *)_async_1857_7frame__5),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async5:;
          i_end = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_end;
          i_out = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_out;
          i_len_s0 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len_s0;
          i_blk_s1 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk_s1;
          i_len = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len;
          i_blk = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk;
          e_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut;
          p_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut;
          ss4o3 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3;
          q_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut;
          eosvmax = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax;
          eosvmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin;
          pmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin;
          emin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin;
          rho0 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0;
          e_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old;
          delvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc;
          p_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old;
          q_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old;
          compression = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression;
          compHalfStep = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
          qq = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq;
          ql = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql;
          work = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work;
          p_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new;
          e_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new;
          q_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new;
          bvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc;
          pbvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc;
          vnewc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc;
          length = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length;
        }
      }
// if eosvmin
    }
    if (eosvmax != ((Real_t )0.0)) {
      Index_t i_len_s2 = length;
      Index_t i_blk_s3 = 16;
      for (Index_t i_out = 0; i_out < i_len_s2; i_out += i_blk_s3) {
        Index_t i_end = ((i_out + i_blk_s3) < i_len_s2)?(i_out + i_blk_s3) : i_len_s2;
        __hc_frame__ -> pc = 6;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len_s2 = i_len_s2;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk_s3 = i_blk_s3;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3 = ss4o3;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin = emin;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression = compression;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq = qq;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql = ql;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work = work;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
        ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length = length;
        struct hc_frameHeader *_async_1865_7frame__6 = buildFrame__async_1865_7(i_out,i_end,compression,vnewc,delvc,compHalfStep,work,eosvmin,eosvmax,p_old,ws);
        _async_1865_7frame__6 -> sched_policy = hc_sched_policy(ws);
        if (_async_1865_7frame__6 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_1865_7frame__6);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_1865_7(ws,((struct hc_frameHeader *)_async_1865_7frame__6),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async6:;
          i_end = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_end;
          i_out = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_out;
          i_len_s2 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len_s2;
          i_blk_s3 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk_s3;
          i_len = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len;
          i_blk = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk;
          e_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut;
          p_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut;
          ss4o3 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3;
          q_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut;
          eosvmax = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax;
          eosvmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin;
          pmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin;
          emin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin;
          rho0 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0;
          e_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old;
          delvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc;
          p_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old;
          q_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old;
          compression = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression;
          compHalfStep = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
          qq = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq;
          ql = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql;
          work = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work;
          p_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new;
          e_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new;
          q_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new;
          bvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc;
          pbvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc;
          vnewc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc;
          length = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length;
        }
      }
// if eosvmax
    }
  }
  __hc_frame__ -> pc = 3;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3 = ss4o3;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin = emin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq = qq;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql = ql;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work = work;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length = length;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish3:;
    e_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut;
    p_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut;
    ss4o3 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3;
    q_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut;
    eosvmax = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    eosvmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin;
    pmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin;
    emin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin;
    rho0 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0;
    e_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old;
    delvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc;
    p_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old;
    q_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old;
    compression = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression;
    compHalfStep = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
    qq = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq;
    ql = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql;
    work = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work;
    p_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new;
    e_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new;
    q_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new;
    bvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc;
    vnewc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc;
    length = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length;
  }
  __hc_frame__ -> pc = 7;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3 = ss4o3;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin = emin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq = qq;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql = ql;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work = work;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length = length;
  struct hc_frameHeader *__hc_CalcEnergyForElems__frame__7 = buildFrame___hc_CalcEnergyForElems__(p_new,e_new,q_new,bvc,pbvc,p_old,e_old,q_old,compression,compHalfStep,vnewc,work,delvc,pmin,p_cut,e_cut,q_cut,emin,qq,ql,rho0,eosvmax,length,ws);
  __hc_CalcEnergyForElems__(ws,((struct hc_frameHeader *)__hc_CalcEnergyForElems__frame__7),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func7:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    e_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut;
    p_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut;
    ss4o3 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3;
    q_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut;
    eosvmax = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    eosvmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin;
    pmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin;
    emin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin;
    rho0 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0;
    e_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old;
    delvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc;
    p_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old;
    q_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old;
    compression = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression;
    compHalfStep = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
    qq = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq;
    ql = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql;
    work = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work;
    p_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new;
    e_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new;
    q_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new;
    bvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc;
    vnewc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc;
    length = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length;
  }
// RAG SCATTERS
  hc_startFinish(ws);
{
    Index_t i_len = length;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 9;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3 = ss4o3;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin = emin;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression = compression;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq = qq;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql = ql;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work = work;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
      ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length = length;
      struct hc_frameHeader *_async_1885_5frame__9 = buildFrame__async_1885_5(i_out,i_end,domain,p_new,e_new,q_new,ws);
      _async_1885_5frame__9 -> sched_policy = hc_sched_policy(ws);
      if (_async_1885_5frame__9 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_1885_5frame__9);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_1885_5(ws,((struct hc_frameHeader *)_async_1885_5frame__9),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async9:;
        i_end = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> i_blk;
        e_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut;
        p_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut;
        ss4o3 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3;
        q_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut;
        eosvmax = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax;
        eosvmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin;
        pmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin;
        emin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin;
        rho0 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0;
        e_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old;
        delvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc;
        p_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old;
        q_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old;
        compression = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression;
        compHalfStep = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
        qq = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq;
        ql = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql;
        work = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work;
        p_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new;
        e_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new;
        q_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new;
        bvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc;
        pbvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc;
        vnewc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc;
        length = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length;
      }
    }
  }
  __hc_frame__ -> pc = 8;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3 = ss4o3;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin = emin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq = qq;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql = ql;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work = work;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length = length;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish8:;
    e_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut;
    p_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut;
    ss4o3 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3;
    q_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut;
    eosvmax = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    eosvmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin;
    pmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin;
    emin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin;
    rho0 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0;
    e_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old;
    delvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc;
    p_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old;
    q_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old;
    compression = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression;
    compHalfStep = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
    qq = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq;
    ql = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql;
    work = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work;
    p_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new;
    e_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new;
    q_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new;
    bvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc;
    vnewc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc;
    length = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length;
  }
  __hc_frame__ -> pc = 10;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut = e_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut = p_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3 = ss4o3;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut = q_cut;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin = pmin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin = emin;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0 = rho0;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old = e_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc = delvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old = p_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old = q_old;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression = compression;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep = compHalfStep;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq = qq;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql = ql;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work = work;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new = p_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new = e_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new = q_new;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc = bvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc = pbvc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
  ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length = length;
  struct hc_frameHeader *__hc_CalcSoundSpeedForElems__frame__10 = buildFrame___hc_CalcSoundSpeedForElems__(vnewc,rho0,e_new,p_new,pbvc,bvc,ss4o3,length,ws);
  __hc_CalcSoundSpeedForElems__(ws,((struct hc_frameHeader *)__hc_CalcSoundSpeedForElems__frame__10),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func10:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    e_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_cut;
    p_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_cut;
    ss4o3 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ss4o3;
    q_cut = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_cut;
    eosvmax = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmax;
    eosvmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> eosvmin;
    pmin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pmin;
    emin = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> emin;
    rho0 = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> rho0;
    e_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_old;
    delvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> delvc;
    p_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_old;
    q_old = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_old;
    compression = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compression;
    compHalfStep = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> compHalfStep;
    qq = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> qq;
    ql = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> ql;
    work = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> work;
    p_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> p_new;
    e_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> e_new;
    q_new = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> q_new;
    bvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> bvc;
    pbvc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> pbvc;
    vnewc = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> vnewc;
    length = ((struct __hc_EvalEOSForElems__frame_t__ *)__hc_frame__) -> length;
  }
  Release_Real_t(pbvc);
  Release_Real_t(bvc);
  Release_Real_t(q_new);
  Release_Real_t(e_new);
  Release_Real_t(p_new);
  Release_Real_t(work);
  Release_Real_t(ql);
  Release_Real_t(qq);
  Release_Real_t(compHalfStep);
  Release_Real_t(compression);
  Release_Real_t(q_old);
  Release_Real_t(p_old);
  Release_Real_t(delvc);
  Release_Real_t(e_old);
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_1885_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  Real_t *p_new;
  Real_t *e_new;
  Real_t *q_new;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1885_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1885_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_1885_5frame_t__ *)__hc_frame__) -> domain;
  p_new = ((struct _async_1885_5frame_t__ *)__hc_frame__) -> p_new;
  e_new = ((struct _async_1885_5frame_t__ *)__hc_frame__) -> e_new;
  q_new = ((struct _async_1885_5frame_t__ *)__hc_frame__) -> q_new;
  for (Index_t i = i_out; i < i_end; ++i) {
    Index_t zidx = (domain -> m_matElemlist)[i];
    (domain -> m_p)[zidx] = p_new[i];
    (domain -> m_e)[zidx] = e_new[i];
//DEBUG if(i==0)fprintf(stdout,"e_new = %e\n",e_new[i]);
    (domain -> m_q)[zidx] = q_new[i];
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_1865_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  Real_t *compression;
  Real_t *vnewc;
  Real_t *delvc;
  Real_t *compHalfStep;
  Real_t *work;
  double eosvmin;
  double eosvmax;
  Real_t *p_old;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1865_7frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1865_7frame_t__ *)__hc_frame__) -> i_end;
  compression = ((struct _async_1865_7frame_t__ *)__hc_frame__) -> compression;
  vnewc = ((struct _async_1865_7frame_t__ *)__hc_frame__) -> vnewc;
  delvc = ((struct _async_1865_7frame_t__ *)__hc_frame__) -> delvc;
  compHalfStep = ((struct _async_1865_7frame_t__ *)__hc_frame__) -> compHalfStep;
  work = ((struct _async_1865_7frame_t__ *)__hc_frame__) -> work;
  eosvmin = ((struct _async_1865_7frame_t__ *)__hc_frame__) -> eosvmin;
  eosvmax = ((struct _async_1865_7frame_t__ *)__hc_frame__) -> eosvmax;
  p_old = ((struct _async_1865_7frame_t__ *)__hc_frame__) -> p_old;
  for (Index_t i = i_out; i < i_end; ++i) {
/* impossible due to calling func? */
    if (vnewc[i] >= eosvmax) {
      p_old[i] = ((Real_t )0.0);
      compression[i] = ((Real_t )0.0);
      compHalfStep[i] = ((Real_t )0.0);
// if vnewc
    }
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_1857_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  Real_t *compression;
  Real_t *vnewc;
  Real_t *delvc;
  Real_t *compHalfStep;
  Real_t *work;
  double eosvmin;
  double eosvmax;
  Real_t *p_old;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1857_7frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1857_7frame_t__ *)__hc_frame__) -> i_end;
  compression = ((struct _async_1857_7frame_t__ *)__hc_frame__) -> compression;
  vnewc = ((struct _async_1857_7frame_t__ *)__hc_frame__) -> vnewc;
  delvc = ((struct _async_1857_7frame_t__ *)__hc_frame__) -> delvc;
  compHalfStep = ((struct _async_1857_7frame_t__ *)__hc_frame__) -> compHalfStep;
  work = ((struct _async_1857_7frame_t__ *)__hc_frame__) -> work;
  eosvmin = ((struct _async_1857_7frame_t__ *)__hc_frame__) -> eosvmin;
  eosvmax = ((struct _async_1857_7frame_t__ *)__hc_frame__) -> eosvmax;
  p_old = ((struct _async_1857_7frame_t__ *)__hc_frame__) -> p_old;
  for (Index_t i = i_out; i < i_end; ++i) {
/* impossible due to calling func? */
    if (vnewc[i] <= eosvmin) {
      compHalfStep[i] = compression[i];
// if vnewc
    }
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_1846_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  Real_t *compression;
  Real_t *vnewc;
  Real_t *delvc;
  Real_t *compHalfStep;
  Real_t *work;
  double eosvmin;
  double eosvmax;
  Real_t *p_old;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1846_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1846_5frame_t__ *)__hc_frame__) -> i_end;
  compression = ((struct _async_1846_5frame_t__ *)__hc_frame__) -> compression;
  vnewc = ((struct _async_1846_5frame_t__ *)__hc_frame__) -> vnewc;
  delvc = ((struct _async_1846_5frame_t__ *)__hc_frame__) -> delvc;
  compHalfStep = ((struct _async_1846_5frame_t__ *)__hc_frame__) -> compHalfStep;
  work = ((struct _async_1846_5frame_t__ *)__hc_frame__) -> work;
  eosvmin = ((struct _async_1846_5frame_t__ *)__hc_frame__) -> eosvmin;
  eosvmax = ((struct _async_1846_5frame_t__ *)__hc_frame__) -> eosvmax;
  p_old = ((struct _async_1846_5frame_t__ *)__hc_frame__) -> p_old;
  for (Index_t i = i_out; i < i_end; ++i) {
    Real_t vchalf;
    compression[i] = ((((Real_t )1.0) / vnewc[i]) - ((Real_t )1.0));
    vchalf = (vnewc[i] - (delvc[i] * ((Real_t )0.5)));
    compHalfStep[i] = ((((Real_t )1.0) / vchalf) - ((Real_t )1.0));
    work[i] = ((Real_t )0.0);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_1830_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  Real_t *delvc;
  Real_t *e_old;
  Real_t *p_old;
  Real_t *q_old;
  Real_t *qq;
  Real_t *ql;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1830_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1830_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_1830_5frame_t__ *)__hc_frame__) -> domain;
  delvc = ((struct _async_1830_5frame_t__ *)__hc_frame__) -> delvc;
  e_old = ((struct _async_1830_5frame_t__ *)__hc_frame__) -> e_old;
  p_old = ((struct _async_1830_5frame_t__ *)__hc_frame__) -> p_old;
  q_old = ((struct _async_1830_5frame_t__ *)__hc_frame__) -> q_old;
  qq = ((struct _async_1830_5frame_t__ *)__hc_frame__) -> qq;
  ql = ((struct _async_1830_5frame_t__ *)__hc_frame__) -> ql;
  for (Index_t i = i_out; i < i_end; ++i) {
    Index_t zidx = (domain -> m_matElemlist)[i];
    e_old[i] = (domain -> m_e)[zidx];
//DEBUG if(i==0)fprintf(stdout,"e_old = %e\n",e_old[i]);
    delvc[i] = (domain -> m_delv)[zidx];
//DEBUG if(i==0)fprintf(stdout,"delvc = %e\n",delvc[i]);
    p_old[i] = (domain -> m_p)[zidx];
    q_old[i] = (domain -> m_q)[zidx];
    qq[i] = (domain -> m_qq)[zidx];
    ql[i] = (domain -> m_ql)[zidx];
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_ApplyMaterialPropertiesForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  double eosvmin;
  double eosvmax;
  Real_t *vnewc;
  long length;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
}
;
void __hc_ApplyMaterialPropertiesForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_ApplyMaterialPropertiesForElems__(struct hc_workerState *ws)
{
  struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *__hc_new_frame__ = (struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_ApplyMaterialPropertiesForElems__frame_t__ ),__hc_ApplyMaterialPropertiesForElems__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1926_7frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  Real_t *vnewc;
}
;
void _async_1926_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1926_7(long i_out,long i_end,struct Domain_t *domain,Real_t *vnewc,struct hc_workerState *ws)
{
  struct _async_1926_7frame_t__ *__hc_new_frame__ = (struct _async_1926_7frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1926_7frame_t__ ),_async_1926_7));
  ((struct _async_1926_7frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1926_7frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1926_7frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_1926_7frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1937_9frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  Real_t *vnewc;
  double eosvmin;
}
;
void _async_1937_9(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1937_9(long i_out,long i_end,Real_t *vnewc,double eosvmin,struct hc_workerState *ws)
{
  struct _async_1937_9frame_t__ *__hc_new_frame__ = (struct _async_1937_9frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1937_9frame_t__ ),_async_1937_9));
  ((struct _async_1937_9frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1937_9frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1937_9frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct _async_1937_9frame_t__ *)__hc_new_frame__) -> eosvmin = eosvmin;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1945_9frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  Real_t *vnewc;
  double eosvmax;
}
;
void _async_1945_9(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1945_9(long i_out,long i_end,Real_t *vnewc,double eosvmax,struct hc_workerState *ws)
{
  struct _async_1945_9frame_t__ *__hc_new_frame__ = (struct _async_1945_9frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1945_9frame_t__ ),_async_1945_9));
  ((struct _async_1945_9frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1945_9frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1945_9frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct _async_1945_9frame_t__ *)__hc_new_frame__) -> eosvmax = eosvmax;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1957_7frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  Real_t *vnewc;
  double eosvmin;
  double eosvmax;
}
;
void _async_1957_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1957_7(long i_out,long i_end,struct Domain_t *domain,Real_t *vnewc,double eosvmin,double eosvmax,struct hc_workerState *ws)
{
  struct _async_1957_7frame_t__ *__hc_new_frame__ = (struct _async_1957_7frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1957_7frame_t__ ),_async_1957_7));
  ((struct _async_1957_7frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1957_7frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1957_7frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_1957_7frame_t__ *)__hc_new_frame__) -> vnewc = vnewc;
  ((struct _async_1957_7frame_t__ *)__hc_new_frame__) -> eosvmin = eosvmin;
  ((struct _async_1957_7frame_t__ *)__hc_new_frame__) -> eosvmax = eosvmax;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_ApplyMaterialPropertiesForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
    case 3:
    goto __post_finish3;
    case 4:
    goto __post_async4;
    case 5:
    goto __post_async5;
    case 6:
    goto __post_finish6;
    case 7:
    goto __post_async7;
    case 8:
    goto __post_func8;
  }
  Index_t length = (domain -> m_numElem);
  if (length != 0) {;
    Real_t eosvmin = (domain -> m_eosvmin);
    Real_t eosvmax = (domain -> m_eosvmax);
    Real_t *vnewc = Allocate_Real_t(length);
// RAG GATHERS
    hc_startFinish(ws);
{
      Index_t i_len = length;
      Index_t i_blk = 16;
      for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
        Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
        __hc_frame__ -> pc = 2;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length = length;
        struct hc_frameHeader *_async_1926_7frame__2 = buildFrame__async_1926_7(i_out,i_end,domain,vnewc,ws);
        _async_1926_7frame__2 -> sched_policy = hc_sched_policy(ws);
        if (_async_1926_7frame__2 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_1926_7frame__2);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_1926_7(ws,((struct hc_frameHeader *)_async_1926_7frame__2),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async2:;
          i_end = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_end;
          i_out = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_out;
          i_len = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_len;
          i_blk = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_blk;
          eosvmin = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin;
          eosvmax = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax;
          vnewc = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc;
          length = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length;
        }
      }
    }
    __hc_frame__ -> pc = 1;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length = length;
    if (hc_stopFinish(ws)) 
      return ;
    if (0) {
      __post_finish1:;
      eosvmin = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin;
      eosvmax = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax;
      vnewc = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc;
      length = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length;
    }
// RAG STIDE ONES
    hc_startFinish(ws);
{
      if (eosvmin != ((Real_t )0.0)) {
        Index_t i_len = length;
        Index_t i_blk = 16;
        for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
          Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
          __hc_frame__ -> pc = 4;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length = length;
          struct hc_frameHeader *_async_1937_9frame__4 = buildFrame__async_1937_9(i_out,i_end,vnewc,eosvmin,ws);
          _async_1937_9frame__4 -> sched_policy = hc_sched_policy(ws);
          if (_async_1937_9frame__4 -> sched_policy == HELP_FIRST) {
            deque_push_default(_async_1937_9frame__4);
          }
          else {
            deque_push_default(__hc_frame__);
            _async_1937_9(ws,((struct hc_frameHeader *)_async_1937_9frame__4),0);
            if (ws -> costatus != HC_ASYNC_COMPLETE) {
              return ;
            }
            if (hc_check_continuation_stolen(ws,__hc_frame__)) {
              return ;
            }
          }
          if (0) {
            __post_async4:;
            i_end = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_end;
            i_out = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_out;
            i_len = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_len;
            i_blk = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_blk;
            eosvmin = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin;
            eosvmax = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax;
            vnewc = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc;
            length = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length;
          }
        }
// if eosvmin
      }
      if (eosvmax != ((Real_t )0.0)) {
        Index_t i_len = length;
        Index_t i_blk = 16;
        for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
          Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
          __hc_frame__ -> pc = 5;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
          ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length = length;
          struct hc_frameHeader *_async_1945_9frame__5 = buildFrame__async_1945_9(i_out,i_end,vnewc,eosvmax,ws);
          _async_1945_9frame__5 -> sched_policy = hc_sched_policy(ws);
          if (_async_1945_9frame__5 -> sched_policy == HELP_FIRST) {
            deque_push_default(_async_1945_9frame__5);
          }
          else {
            deque_push_default(__hc_frame__);
            _async_1945_9(ws,((struct hc_frameHeader *)_async_1945_9frame__5),0);
            if (ws -> costatus != HC_ASYNC_COMPLETE) {
              return ;
            }
            if (hc_check_continuation_stolen(ws,__hc_frame__)) {
              return ;
            }
          }
          if (0) {
            __post_async5:;
            i_end = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_end;
            i_out = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_out;
            i_len = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_len;
            i_blk = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_blk;
            eosvmin = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin;
            eosvmax = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax;
            vnewc = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc;
            length = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length;
          }
        }
// if eosvmax
      }
    }
    __hc_frame__ -> pc = 3;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length = length;
    if (hc_stopFinish(ws)) 
      return ;
    if (0) {
      __post_finish3:;
      eosvmin = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin;
      eosvmax = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax;
      vnewc = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc;
      length = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length;
    }
// RAG GATHER ERROR CHECK
    hc_startFinish(ws);
{
      Index_t i_len = length;
      Index_t i_blk = 16;
      for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
        Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
        __hc_frame__ -> pc = 7;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
        ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length = length;
        struct hc_frameHeader *_async_1957_7frame__7 = buildFrame__async_1957_7(i_out,i_end,domain,vnewc,eosvmin,eosvmax,ws);
        _async_1957_7frame__7 -> sched_policy = hc_sched_policy(ws);
        if (_async_1957_7frame__7 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_1957_7frame__7);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_1957_7(ws,((struct hc_frameHeader *)_async_1957_7frame__7),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async7:;
          i_end = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_end;
          i_out = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_out;
          i_len = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_len;
          i_blk = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> i_blk;
          eosvmin = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin;
          eosvmax = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax;
          vnewc = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc;
          length = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length;
        }
      }
    }
    __hc_frame__ -> pc = 6;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length = length;
    if (hc_stopFinish(ws)) 
      return ;
    if (0) {
      __post_finish6:;
      eosvmin = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin;
      eosvmax = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax;
      vnewc = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc;
      length = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length;
    }
    __hc_frame__ -> pc = 8;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin = eosvmin;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax = eosvmax;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc = vnewc;
    ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length = length;
    struct hc_frameHeader *__hc_EvalEOSForElems__frame__8 = buildFrame___hc_EvalEOSForElems__(vnewc,length,ws);
    __hc_EvalEOSForElems__(ws,((struct hc_frameHeader *)__hc_EvalEOSForElems__frame__8),0);
    if (ws -> costatus == HC_FUNC_COMPLETE) {
      hc_free_unwind_frame(ws);
    }
    else {
      return ;
    }
    if (0) {
      __post_func8:;
      __hc_frame__ = hc_free_unwind_frame(ws);
      eosvmin = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmin;
      eosvmax = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> eosvmax;
      vnewc = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> vnewc;
      length = ((struct __hc_ApplyMaterialPropertiesForElems__frame_t__ *)__hc_frame__) -> length;
    }
    Release_Real_t(vnewc);
// if length
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_1957_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  Real_t *vnewc;
  double eosvmin;
  double eosvmax;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1957_7frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1957_7frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_1957_7frame_t__ *)__hc_frame__) -> domain;
  vnewc = ((struct _async_1957_7frame_t__ *)__hc_frame__) -> vnewc;
  eosvmin = ((struct _async_1957_7frame_t__ *)__hc_frame__) -> eosvmin;
  eosvmax = ((struct _async_1957_7frame_t__ *)__hc_frame__) -> eosvmax;
  for (Index_t i = i_out; i < i_end; ++i) {
    Index_t zn = (domain -> m_matElemlist)[i];
    Real_t vc = (domain -> m_v)[zn];
    if (eosvmin != ((Real_t )0.0)) {
      if (vc < eosvmin) {
        vc = eosvmin;
// if domain->m_v
      }
// if eosvmin
    }
    if (eosvmax != ((Real_t )0.0)) {
      if (vc > eosvmax) {
        vc = eosvmax;
// if domain->m_v
      }
// if eosvmax
    }
    if (vc <= 0.0) {
      exit(VolumeError);
// if domain->m_v
    }
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_1945_9(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  Real_t *vnewc;
  double eosvmax;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1945_9frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1945_9frame_t__ *)__hc_frame__) -> i_end;
  vnewc = ((struct _async_1945_9frame_t__ *)__hc_frame__) -> vnewc;
  eosvmax = ((struct _async_1945_9frame_t__ *)__hc_frame__) -> eosvmax;
  for (Index_t i = i_out; i < i_end; ++i) {
    if (vnewc[i] > eosvmax) {
      vnewc[i] = eosvmax;
// if vnewc
    }
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_1937_9(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  Real_t *vnewc;
  double eosvmin;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1937_9frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1937_9frame_t__ *)__hc_frame__) -> i_end;
  vnewc = ((struct _async_1937_9frame_t__ *)__hc_frame__) -> vnewc;
  eosvmin = ((struct _async_1937_9frame_t__ *)__hc_frame__) -> eosvmin;
  for (Index_t i = i_out; i < i_end; ++i) {
    if (vnewc[i] < eosvmin) {
      vnewc[i] = eosvmin;
// if vnewc
    }
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_1926_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  Real_t *vnewc;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1926_7frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1926_7frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_1926_7frame_t__ *)__hc_frame__) -> domain;
  vnewc = ((struct _async_1926_7frame_t__ *)__hc_frame__) -> vnewc;
  for (Index_t i = i_out; i < i_end; ++i) {
    Index_t zn = (domain -> m_matElemlist)[i];
    vnewc[i] = (domain -> m_vnew)[zn];
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_UpdateVolumesForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  double v_cut;
  long numElem;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
}
;
void __hc_UpdateVolumesForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_UpdateVolumesForElems__(struct hc_workerState *ws)
{
  struct __hc_UpdateVolumesForElems__frame_t__ *__hc_new_frame__ = (struct __hc_UpdateVolumesForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_UpdateVolumesForElems__frame_t__ ),__hc_UpdateVolumesForElems__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_1990_7frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double v_cut;
}
;
void _async_1990_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_1990_7(long i_out,long i_end,struct Domain_t *domain,double v_cut,struct hc_workerState *ws)
{
  struct _async_1990_7frame_t__ *__hc_new_frame__ = (struct _async_1990_7frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_1990_7frame_t__ ),_async_1990_7));
  ((struct _async_1990_7frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_1990_7frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_1990_7frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_1990_7frame_t__ *)__hc_new_frame__) -> v_cut = v_cut;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_UpdateVolumesForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  Index_t numElem = (domain -> m_numElem);
  if (numElem != 0) {
    Real_t v_cut = (domain -> m_v_cut);
    hc_startFinish(ws);
{
      Index_t i_len = numElem;
      Index_t i_blk = 16;
      for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
        Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
        __hc_frame__ -> pc = 2;
        ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
        ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
        ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
        ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
        ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> v_cut = v_cut;
        ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
        struct hc_frameHeader *_async_1990_7frame__2 = buildFrame__async_1990_7(i_out,i_end,domain,v_cut,ws);
        _async_1990_7frame__2 -> sched_policy = hc_sched_policy(ws);
        if (_async_1990_7frame__2 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_1990_7frame__2);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_1990_7(ws,((struct hc_frameHeader *)_async_1990_7frame__2),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async2:;
          i_end = ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> i_end;
          i_out = ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> i_out;
          i_len = ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> i_len;
          i_blk = ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> i_blk;
          v_cut = ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> v_cut;
          numElem = ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> numElem;
        }
      }
    }
    __hc_frame__ -> pc = 1;
    ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> v_cut = v_cut;
    ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> numElem = numElem;
    if (hc_stopFinish(ws)) 
      return ;
    if (0) {
      __post_finish1:;
      v_cut = ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> v_cut;
      numElem = ((struct __hc_UpdateVolumesForElems__frame_t__ *)__hc_frame__) -> numElem;
    }
// if numElem
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_1990_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double v_cut;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_1990_7frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_1990_7frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_1990_7frame_t__ *)__hc_frame__) -> domain;
  v_cut = ((struct _async_1990_7frame_t__ *)__hc_frame__) -> v_cut;
  for (Index_t i = i_out; i < i_end; ++i) {
    Real_t tmpV;
    tmpV = (domain -> m_vnew)[i];
    if (fabs((tmpV - ((Real_t )1.0))) < v_cut) {
      tmpV = ((Real_t )1.0);
// tmpV
    }
    (domain -> m_v)[i] = tmpV;
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_LagrangeElements__frame_t__ 
{
  struct hc_frameHeader header;
  double deltatime;
}
;
void __hc_LagrangeElements__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_LagrangeElements__(struct hc_workerState *ws)
{
  struct __hc_LagrangeElements__frame_t__ *__hc_new_frame__ = (struct __hc_LagrangeElements__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_LagrangeElements__frame_t__ ),__hc_LagrangeElements__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_LagrangeElements__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
    case 2:
    goto __post_func2;
    case 3:
    goto __post_func3;
    case 4:
    goto __post_func4;
  }
  Real_t deltatime = (domain -> m_deltatime);;
  __hc_frame__ -> pc = 1;
  ((struct __hc_LagrangeElements__frame_t__ *)__hc_frame__) -> deltatime = deltatime;
  struct hc_frameHeader *__hc_CalcLagrangeElements__frame__1 = buildFrame___hc_CalcLagrangeElements__(deltatime,ws);
  __hc_CalcLagrangeElements__(ws,((struct hc_frameHeader *)__hc_CalcLagrangeElements__frame__1),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func1:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    deltatime = ((struct __hc_LagrangeElements__frame_t__ *)__hc_frame__) -> deltatime;
  };
  __hc_frame__ -> pc = 2;
  ((struct __hc_LagrangeElements__frame_t__ *)__hc_frame__) -> deltatime = deltatime;
  struct hc_frameHeader *__hc_CalcQForElems__frame__2 = buildFrame___hc_CalcQForElems__(ws);
  __hc_CalcQForElems__(ws,((struct hc_frameHeader *)__hc_CalcQForElems__frame__2),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func2:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    deltatime = ((struct __hc_LagrangeElements__frame_t__ *)__hc_frame__) -> deltatime;
  };
  __hc_frame__ -> pc = 3;
  ((struct __hc_LagrangeElements__frame_t__ *)__hc_frame__) -> deltatime = deltatime;
  struct hc_frameHeader *__hc_ApplyMaterialPropertiesForElems__frame__3 = buildFrame___hc_ApplyMaterialPropertiesForElems__(ws);
  __hc_ApplyMaterialPropertiesForElems__(ws,((struct hc_frameHeader *)__hc_ApplyMaterialPropertiesForElems__frame__3),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func3:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    deltatime = ((struct __hc_LagrangeElements__frame_t__ *)__hc_frame__) -> deltatime;
  };
  __hc_frame__ -> pc = 4;
  ((struct __hc_LagrangeElements__frame_t__ *)__hc_frame__) -> deltatime = deltatime;
  struct hc_frameHeader *__hc_UpdateVolumesForElems__frame__4 = buildFrame___hc_UpdateVolumesForElems__(ws);
  __hc_UpdateVolumesForElems__(ws,((struct hc_frameHeader *)__hc_UpdateVolumesForElems__frame__4),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func4:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    deltatime = ((struct __hc_LagrangeElements__frame_t__ *)__hc_frame__) -> deltatime;
  }
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

struct __hc_CalcCourantConstraintForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  Real_t *pDtCourant;
  Index_t *pCourant_elem;
  long i_end;
  long i_out;
  double qqc;
  double qqc2;
  long length;
  long i_len;
  long i_blk;
}
;
void __hc_CalcCourantConstraintForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcCourantConstraintForElems__(struct hc_workerState *ws)
{
  struct __hc_CalcCourantConstraintForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcCourantConstraintForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcCourantConstraintForElems__frame_t__ ),__hc_CalcCourantConstraintForElems__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2037_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double qqc2;
  Real_t *pDtCourant;
  Index_t *pCourant_elem;
  uint64_t *pidamin_lock;
}
;
void _async_2037_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2037_5(long i_out,long i_end,struct Domain_t *domain,double qqc2,Real_t *pDtCourant,Index_t *pCourant_elem,uint64_t *pidamin_lock,struct hc_workerState *ws)
{
  struct _async_2037_5frame_t__ *__hc_new_frame__ = (struct _async_2037_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2037_5frame_t__ ),_async_2037_5));
  ((struct _async_2037_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_2037_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_2037_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_2037_5frame_t__ *)__hc_new_frame__) -> qqc2 = qqc2;
  ((struct _async_2037_5frame_t__ *)__hc_new_frame__) -> pDtCourant = pDtCourant;
  ((struct _async_2037_5frame_t__ *)__hc_new_frame__) -> pCourant_elem = pCourant_elem;
  ((struct _async_2037_5frame_t__ *)__hc_new_frame__) -> pidamin_lock = pidamin_lock;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcCourantConstraintForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  Real_t *pDtCourant = (Real_t *)(malloc((((Index_t )1) * sizeof(Real_t ))));
  Index_t *pCourant_elem = (Index_t *)(malloc((((Index_t )1) * sizeof(Index_t ))));
   *pDtCourant = ((Real_t )1.0e+20);
   *pCourant_elem = (-1);
  hc_startFinish(ws);
{
    Real_t qqc = (domain -> m_qqc);
    Real_t qqc2 = ((((Real_t )64.0) * qqc) * qqc);
    Index_t length = (domain -> m_numElem);
    Index_t i_len = length;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> qqc = qqc;
      ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> qqc2 = qqc2;
      ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> length = length;
      ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> pDtCourant = pDtCourant;
      ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> pCourant_elem = pCourant_elem;
      struct hc_frameHeader *_async_2037_5frame__2 = buildFrame__async_2037_5(i_out,i_end,domain,qqc2,pDtCourant,pCourant_elem,pidamin_lock,ws);
      _async_2037_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_2037_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_2037_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_2037_5(ws,((struct hc_frameHeader *)_async_2037_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> i_out;
        qqc = ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> qqc;
        qqc2 = ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> qqc2;
        length = ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> length;
        i_len = ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> i_blk;
        pDtCourant = ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> pDtCourant;
        pCourant_elem = ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> pCourant_elem;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> pDtCourant = pDtCourant;
  ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> pCourant_elem = pCourant_elem;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    pDtCourant = ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> pDtCourant;
    pCourant_elem = ((struct __hc_CalcCourantConstraintForElems__frame_t__ *)__hc_frame__) -> pCourant_elem;
  }
/* Don't try to register a time constraint if none of the elements
   * were active */
  if ( *pCourant_elem != (-1)) {
//DEBUG fprintf(stdout,"dtcourant %e\n",domain->m_dtcourant);
    domain -> m_dtcourant =  *pDtCourant;
// if *pCourant_elem
  }
  free(pCourant_elem);
  free(pDtCourant);
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_2037_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double qqc2;
  Real_t *pDtCourant;
  Index_t *pCourant_elem;
  uint64_t *pidamin_lock;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_2037_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_2037_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_2037_5frame_t__ *)__hc_frame__) -> domain;
  qqc2 = ((struct _async_2037_5frame_t__ *)__hc_frame__) -> qqc2;
  pDtCourant = ((struct _async_2037_5frame_t__ *)__hc_frame__) -> pDtCourant;
  pCourant_elem = ((struct _async_2037_5frame_t__ *)__hc_frame__) -> pCourant_elem;
  pidamin_lock = ((struct _async_2037_5frame_t__ *)__hc_frame__) -> pidamin_lock;
  for (Index_t i = i_out; i < i_end; ++i) {
    Index_t indx = (domain -> m_matElemlist)[i];
    Real_t dtf = ((domain -> m_ss)[indx] * (domain -> m_ss)[indx]);
    if ((domain -> m_vdov)[indx] < ((Real_t )0.0)) {
      dtf = (dtf + ((((qqc2 * (domain -> m_arealg)[indx]) * (domain -> m_arealg)[indx]) * (domain -> m_vdov)[indx]) * (domain -> m_vdov)[indx]));
// if domain->m_vdov
    }
    dtf = sqrt(dtf);
    dtf = ((domain -> m_arealg)[indx] / dtf);
/* determine minimum timestep with its corresponding elem */
    if ((domain -> m_vdov)[indx] != ((Real_t )0.0)) {
      if (dtf <  *pDtCourant) {
// LOCK
        AMO__lock_uint64_t(pidamin_lock);
         *pDtCourant = dtf;
         *pCourant_elem = indx;
// UNLOCK
        AMO__unlock_uint64_t(pidamin_lock);
// if *pDtCourant
      }
// if domain->m_vdov
    }
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcHydroConstraintForElems__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  Real_t *pDtHydro;
  Index_t *pHydro_elem;
  long i_end;
  long i_out;
  double dvovmax;
  long length;
  long i_len;
  long i_blk;
}
;
void __hc_CalcHydroConstraintForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcHydroConstraintForElems__(struct hc_workerState *ws)
{
  struct __hc_CalcHydroConstraintForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcHydroConstraintForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcHydroConstraintForElems__frame_t__ ),__hc_CalcHydroConstraintForElems__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2088_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double dvovmax;
  uint64_t *pidamin_lock;
  Real_t *pDtHydro;
  Index_t *pHydro_elem;
}
;
void _async_2088_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2088_5(long i_out,long i_end,struct Domain_t *domain,double dvovmax,uint64_t *pidamin_lock,Real_t *pDtHydro,Index_t *pHydro_elem,struct hc_workerState *ws)
{
  struct _async_2088_5frame_t__ *__hc_new_frame__ = (struct _async_2088_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2088_5frame_t__ ),_async_2088_5));
  ((struct _async_2088_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_2088_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_2088_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_2088_5frame_t__ *)__hc_new_frame__) -> dvovmax = dvovmax;
  ((struct _async_2088_5frame_t__ *)__hc_new_frame__) -> pidamin_lock = pidamin_lock;
  ((struct _async_2088_5frame_t__ *)__hc_new_frame__) -> pDtHydro = pDtHydro;
  ((struct _async_2088_5frame_t__ *)__hc_new_frame__) -> pHydro_elem = pHydro_elem;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcHydroConstraintForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_finish1;
    case 2:
    goto __post_async2;
  }
  Real_t *pDtHydro = (Real_t *)(malloc((((Index_t )1) * sizeof(Real_t ))));
  Index_t *pHydro_elem = (Index_t *)(malloc((((Index_t )1) * sizeof(Index_t ))));
   *pDtHydro = ((Real_t )1.0e+20);
   *pHydro_elem = (-1);
  hc_startFinish(ws);
{
    Real_t dvovmax = (domain -> m_dvovmax);
    Index_t length = (domain -> m_numElem);
    Index_t i_len = length;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 2;
      ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> dvovmax = dvovmax;
      ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> length = length;
      ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> pDtHydro = pDtHydro;
      ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> pHydro_elem = pHydro_elem;
      struct hc_frameHeader *_async_2088_5frame__2 = buildFrame__async_2088_5(i_out,i_end,domain,dvovmax,pidamin_lock,pDtHydro,pHydro_elem,ws);
      _async_2088_5frame__2 -> sched_policy = hc_sched_policy(ws);
      if (_async_2088_5frame__2 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_2088_5frame__2);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_2088_5(ws,((struct hc_frameHeader *)_async_2088_5frame__2),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async2:;
        i_end = ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> i_out;
        dvovmax = ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> dvovmax;
        length = ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> length;
        i_len = ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> i_blk;
        pDtHydro = ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> pDtHydro;
        pHydro_elem = ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> pHydro_elem;
      }
    }
  }
  __hc_frame__ -> pc = 1;
  ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> pDtHydro = pDtHydro;
  ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> pHydro_elem = pHydro_elem;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish1:;
    pDtHydro = ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> pDtHydro;
    pHydro_elem = ((struct __hc_CalcHydroConstraintForElems__frame_t__ *)__hc_frame__) -> pHydro_elem;
  }
  if ( *pHydro_elem != (-1)) {
    domain -> m_dthydro =  *pDtHydro;
//DEBUG fprintf(stdout,"dthydro %e\n",domain->m_dthydro);
// if *pHydro_elem
  }
  free(pHydro_elem);
  free(pDtHydro);
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

void _async_2088_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  double dvovmax;
  uint64_t *pidamin_lock;
  Real_t *pDtHydro;
  Index_t *pHydro_elem;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_2088_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_2088_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_2088_5frame_t__ *)__hc_frame__) -> domain;
  dvovmax = ((struct _async_2088_5frame_t__ *)__hc_frame__) -> dvovmax;
  pidamin_lock = ((struct _async_2088_5frame_t__ *)__hc_frame__) -> pidamin_lock;
  pDtHydro = ((struct _async_2088_5frame_t__ *)__hc_frame__) -> pDtHydro;
  pHydro_elem = ((struct _async_2088_5frame_t__ *)__hc_frame__) -> pHydro_elem;
  for (Index_t i = i_out; i < i_end; ++i) {
    Index_t indx = (domain -> m_matElemlist)[i];
    if ((domain -> m_vdov)[indx] != ((Real_t )0.0)) {
      Real_t dtdvov = (dvovmax / (fabs((domain -> m_vdov)[indx]) + ((Real_t )1.e-20)));
      if ( *pDtHydro > dtdvov) {
// LOCK
        AMO__lock_uint64_t(pidamin_lock);
         *pDtHydro = dtdvov;
         *pHydro_elem = indx;
// UNLOCK
        AMO__unlock_uint64_t(pidamin_lock);
// if *pDtHydro
      }
// if domain->m_vdov
    }
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

struct __hc_CalcTimeConstraintsForElems__frame_t__ 
{
  struct hc_frameHeader header;
}
;
void __hc_CalcTimeConstraintsForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_CalcTimeConstraintsForElems__(struct hc_workerState *ws)
{
  struct __hc_CalcTimeConstraintsForElems__frame_t__ *__hc_new_frame__ = (struct __hc_CalcTimeConstraintsForElems__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_CalcTimeConstraintsForElems__frame_t__ ),__hc_CalcTimeConstraintsForElems__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_CalcTimeConstraintsForElems__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
    case 2:
    goto __post_func2;
  };;
  __hc_frame__ -> pc = 1;
  struct hc_frameHeader *__hc_CalcCourantConstraintForElems__frame__1 = buildFrame___hc_CalcCourantConstraintForElems__(ws);
  __hc_CalcCourantConstraintForElems__(ws,((struct hc_frameHeader *)__hc_CalcCourantConstraintForElems__frame__1),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func1:;
    __hc_frame__ = hc_free_unwind_frame(ws);
  };
  __hc_frame__ -> pc = 2;
  struct hc_frameHeader *__hc_CalcHydroConstraintForElems__frame__2 = buildFrame___hc_CalcHydroConstraintForElems__(ws);
  __hc_CalcHydroConstraintForElems__(ws,((struct hc_frameHeader *)__hc_CalcHydroConstraintForElems__frame__2),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func2:;
    __hc_frame__ = hc_free_unwind_frame(ws);
  };
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}

struct __hc_LagrangeLeapFrog__frame_t__ 
{
  struct hc_frameHeader header;
}
;
void __hc_LagrangeLeapFrog__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_LagrangeLeapFrog__(struct hc_workerState *ws)
{
  struct __hc_LagrangeLeapFrog__frame_t__ *__hc_new_frame__ = (struct __hc_LagrangeLeapFrog__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_LagrangeLeapFrog__frame_t__ ),__hc_LagrangeLeapFrog__));
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_LagrangeLeapFrog__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
    case 2:
    goto __post_func2;
    case 3:
    goto __post_func3;
  };;;
  __hc_frame__ -> pc = 1;
  struct hc_frameHeader *__hc_LagrangeNodal__frame__1 = buildFrame___hc_LagrangeNodal__(ws);
  __hc_LagrangeNodal__(ws,((struct hc_frameHeader *)__hc_LagrangeNodal__frame__1),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func1:;
    __hc_frame__ = hc_free_unwind_frame(ws);
  };;
  __hc_frame__ -> pc = 2;
  struct hc_frameHeader *__hc_LagrangeElements__frame__2 = buildFrame___hc_LagrangeElements__(ws);
  __hc_LagrangeElements__(ws,((struct hc_frameHeader *)__hc_LagrangeElements__frame__2),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func2:;
    __hc_frame__ = hc_free_unwind_frame(ws);
  };
  __hc_frame__ -> pc = 3;
  struct hc_frameHeader *__hc_CalcTimeConstraintsForElems__frame__3 = buildFrame___hc_CalcTimeConstraintsForElems__(ws);
  __hc_CalcTimeConstraintsForElems__(ws,((struct hc_frameHeader *)__hc_CalcTimeConstraintsForElems__frame__3),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func3:;
    __hc_frame__ = hc_free_unwind_frame(ws);
  };
  ws -> costatus = HC_FUNC_COMPLETE;
  return ;
}
#if    defined(OCR)
/* (ocrGuid_t *)guid,ocrEdt_ funcPtr,u32 paramc, u64 *params, void *paramv[], u16 properties, u32 depc, ocrGuid_t *depv, ocrGuid_t *outputEvent */
#elif defined(FSIM)
#else // DEFAULT, cilk, h-c, c99, and upc

struct __hc_main__frame_t__ 
{
  struct hc_frameHeader header;
  struct hc_finishState fscache[1];
  long edgeElems;
  long edgeNodes;
  long domElems;
  long col_end;
  long col_out;
  double ty;
  long pln_row_nidx;
  long col_len;
  long col_blk;
  long row;
  double tz;
  long pln_nidx;
  long pln;
  double sf;
  long dimN;
  long dimNdimN;
  long pln_row_zidx;
  long pln_zidx;
  long dimE;
  long dimEdimE;
  long i_end;
  long i_out;
  long i_len;
  long i_blk;
  long j_end;
  long j_out;
  long planeInc;
  long rowInc;
  long j_len;
  long j_blk;
  long i;
  int argc;
  char **argv;
  int __rtval__;
}
;
void __hc_main__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame___hc_main__(int argc,char **argv,struct hc_workerState *ws)
{
  struct __hc_main__frame_t__ *__hc_new_frame__ = (struct __hc_main__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_main__frame_t__ ),__hc_main__));
  ((struct __hc_main__frame_t__ *)__hc_new_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_new_frame__) -> argv = argv;
  ((struct hc_frameHeader *)__hc_new_frame__) -> rtvalp = &((struct __hc_main__frame_t__ *)__hc_new_frame__) -> __rtval__;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2239_9frame_t__ 
{
  struct hc_frameHeader header;
  long col_out;
  long col_end;
  long pln;
  long row;
  double tz;
  double ty;
  long pln_row_nidx;
  struct Domain_t *domain;
  double sf;
  long dimN;
  long dimNdimN;
}
;
void _async_2239_9(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2239_9(long col_out,long col_end,long pln,long row,double tz,double ty,long pln_row_nidx,struct Domain_t *domain,double sf,long dimN,long dimNdimN,struct hc_workerState *ws)
{
  struct _async_2239_9frame_t__ *__hc_new_frame__ = (struct _async_2239_9frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2239_9frame_t__ ),_async_2239_9));
  ((struct _async_2239_9frame_t__ *)__hc_new_frame__) -> col_out = col_out;
  ((struct _async_2239_9frame_t__ *)__hc_new_frame__) -> col_end = col_end;
  ((struct _async_2239_9frame_t__ *)__hc_new_frame__) -> pln = pln;
  ((struct _async_2239_9frame_t__ *)__hc_new_frame__) -> row = row;
  ((struct _async_2239_9frame_t__ *)__hc_new_frame__) -> tz = tz;
  ((struct _async_2239_9frame_t__ *)__hc_new_frame__) -> ty = ty;
  ((struct _async_2239_9frame_t__ *)__hc_new_frame__) -> pln_row_nidx = pln_row_nidx;
  ((struct _async_2239_9frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_2239_9frame_t__ *)__hc_new_frame__) -> sf = sf;
  ((struct _async_2239_9frame_t__ *)__hc_new_frame__) -> dimN = dimN;
  ((struct _async_2239_9frame_t__ *)__hc_new_frame__) -> dimNdimN = dimNdimN;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2265_9frame_t__ 
{
  struct hc_frameHeader header;
  long col_out;
  long col_end;
  long pln;
  long row;
  long pln_row_nidx;
  long pln_row_zidx;
  struct Domain_t *domain;
  long dimE;
  long dimEdimE;
  long dimN;
  long dimNdimN;
}
;
void _async_2265_9(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2265_9(long col_out,long col_end,long pln,long row,long pln_row_nidx,long pln_row_zidx,struct Domain_t *domain,long dimE,long dimEdimE,long dimN,long dimNdimN,struct hc_workerState *ws)
{
  struct _async_2265_9frame_t__ *__hc_new_frame__ = (struct _async_2265_9frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2265_9frame_t__ ),_async_2265_9));
  ((struct _async_2265_9frame_t__ *)__hc_new_frame__) -> col_out = col_out;
  ((struct _async_2265_9frame_t__ *)__hc_new_frame__) -> col_end = col_end;
  ((struct _async_2265_9frame_t__ *)__hc_new_frame__) -> pln = pln;
  ((struct _async_2265_9frame_t__ *)__hc_new_frame__) -> row = row;
  ((struct _async_2265_9frame_t__ *)__hc_new_frame__) -> pln_row_nidx = pln_row_nidx;
  ((struct _async_2265_9frame_t__ *)__hc_new_frame__) -> pln_row_zidx = pln_row_zidx;
  ((struct _async_2265_9frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_2265_9frame_t__ *)__hc_new_frame__) -> dimE = dimE;
  ((struct _async_2265_9frame_t__ *)__hc_new_frame__) -> dimEdimE = dimEdimE;
  ((struct _async_2265_9frame_t__ *)__hc_new_frame__) -> dimN = dimN;
  ((struct _async_2265_9frame_t__ *)__hc_new_frame__) -> dimNdimN = dimNdimN;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2285_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
}
;
void _async_2285_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2285_5(long i_out,long i_end,struct Domain_t *domain,struct hc_workerState *ws)
{
  struct _async_2285_5frame_t__ *__hc_new_frame__ = (struct _async_2285_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2285_5frame_t__ ),_async_2285_5));
  ((struct _async_2285_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_2285_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_2285_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2330_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
}
;
void _async_2330_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2330_5(long i_out,long i_end,struct Domain_t *domain,struct hc_workerState *ws)
{
  struct _async_2330_5frame_t__ *__hc_new_frame__ = (struct _async_2330_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2330_5frame_t__ ),_async_2330_5));
  ((struct _async_2330_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_2330_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_2330_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2382_7frame_t__ 
{
  struct hc_frameHeader header;
  long j_out;
  long j_end;
  long i;
  long planeInc;
  long rowInc;
  struct Domain_t *domain;
  long dimN;
  long dimNdimN;
}
;
void _async_2382_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2382_7(long j_out,long j_end,long i,long planeInc,long rowInc,struct Domain_t *domain,long dimN,long dimNdimN,struct hc_workerState *ws)
{
  struct _async_2382_7frame_t__ *__hc_new_frame__ = (struct _async_2382_7frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2382_7frame_t__ ),_async_2382_7));
  ((struct _async_2382_7frame_t__ *)__hc_new_frame__) -> j_out = j_out;
  ((struct _async_2382_7frame_t__ *)__hc_new_frame__) -> j_end = j_end;
  ((struct _async_2382_7frame_t__ *)__hc_new_frame__) -> i = i;
  ((struct _async_2382_7frame_t__ *)__hc_new_frame__) -> planeInc = planeInc;
  ((struct _async_2382_7frame_t__ *)__hc_new_frame__) -> rowInc = rowInc;
  ((struct _async_2382_7frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_2382_7frame_t__ *)__hc_new_frame__) -> dimN = dimN;
  ((struct _async_2382_7frame_t__ *)__hc_new_frame__) -> dimNdimN = dimNdimN;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2396_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  long domElems;
}
;
void _async_2396_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2396_5(long i_out,long i_end,struct Domain_t *domain,long domElems,struct hc_workerState *ws)
{
  struct _async_2396_5frame_t__ *__hc_new_frame__ = (struct _async_2396_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2396_5frame_t__ ),_async_2396_5));
  ((struct _async_2396_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_2396_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_2396_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_2396_5frame_t__ *)__hc_new_frame__) -> domElems = domElems;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2404_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  long domElems;
  long edgeElems;
}
;
void _async_2404_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2404_5(long i_out,long i_end,struct Domain_t *domain,long domElems,long edgeElems,struct hc_workerState *ws)
{
  struct _async_2404_5frame_t__ *__hc_new_frame__ = (struct _async_2404_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2404_5frame_t__ ),_async_2404_5));
  ((struct _async_2404_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_2404_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_2404_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_2404_5frame_t__ *)__hc_new_frame__) -> domElems = domElems;
  ((struct _async_2404_5frame_t__ *)__hc_new_frame__) -> edgeElems = edgeElems;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2411_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  long edgeElems;
}
;
void _async_2411_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2411_5(long i_out,long i_end,struct Domain_t *domain,long edgeElems,struct hc_workerState *ws)
{
  struct _async_2411_5frame_t__ *__hc_new_frame__ = (struct _async_2411_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2411_5frame_t__ ),_async_2411_5));
  ((struct _async_2411_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_2411_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_2411_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_2411_5frame_t__ *)__hc_new_frame__) -> edgeElems = edgeElems;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2418_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  long domElems;
  long edgeElems;
}
;
void _async_2418_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2418_5(long i_out,long i_end,struct Domain_t *domain,long domElems,long edgeElems,struct hc_workerState *ws)
{
  struct _async_2418_5frame_t__ *__hc_new_frame__ = (struct _async_2418_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2418_5frame_t__ ),_async_2418_5));
  ((struct _async_2418_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_2418_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_2418_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_2418_5frame_t__ *)__hc_new_frame__) -> domElems = domElems;
  ((struct _async_2418_5frame_t__ *)__hc_new_frame__) -> edgeElems = edgeElems;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2426_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  long dimEdimE;
  long domElems;
}
;
void _async_2426_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2426_5(long i_out,long i_end,struct Domain_t *domain,long dimEdimE,long domElems,struct hc_workerState *ws)
{
  struct _async_2426_5frame_t__ *__hc_new_frame__ = (struct _async_2426_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2426_5frame_t__ ),_async_2426_5));
  ((struct _async_2426_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_2426_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_2426_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_2426_5frame_t__ *)__hc_new_frame__) -> dimEdimE = dimEdimE;
  ((struct _async_2426_5frame_t__ *)__hc_new_frame__) -> domElems = domElems;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2436_5frame_t__ 
{
  struct hc_frameHeader header;
  long i_out;
  long i_end;
  struct Domain_t *domain;
  long domElems;
}
;
void _async_2436_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2436_5(long i_out,long i_end,struct Domain_t *domain,long domElems,struct hc_workerState *ws)
{
  struct _async_2436_5frame_t__ *__hc_new_frame__ = (struct _async_2436_5frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2436_5frame_t__ ),_async_2436_5));
  ((struct _async_2436_5frame_t__ *)__hc_new_frame__) -> i_out = i_out;
  ((struct _async_2436_5frame_t__ *)__hc_new_frame__) -> i_end = i_end;
  ((struct _async_2436_5frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_2436_5frame_t__ *)__hc_new_frame__) -> domElems = domElems;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

struct _async_2450_7frame_t__ 
{
  struct hc_frameHeader header;
  long j_out;
  long j_end;
  long i;
  struct Domain_t *domain;
  long planeInc;
  long rowInc;
  long domElems;
  long dimE;
  long dimEdimE;
}
;
void _async_2450_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__);

inline struct hc_frameHeader *buildFrame__async_2450_7(long j_out,long j_end,long i,struct Domain_t *domain,long planeInc,long rowInc,long domElems,long dimE,long dimEdimE,struct hc_workerState *ws)
{
  struct _async_2450_7frame_t__ *__hc_new_frame__ = (struct _async_2450_7frame_t__ *)(hc_init_async_frame_ws(ws,sizeof(struct _async_2450_7frame_t__ ),_async_2450_7));
  ((struct _async_2450_7frame_t__ *)__hc_new_frame__) -> j_out = j_out;
  ((struct _async_2450_7frame_t__ *)__hc_new_frame__) -> j_end = j_end;
  ((struct _async_2450_7frame_t__ *)__hc_new_frame__) -> i = i;
  ((struct _async_2450_7frame_t__ *)__hc_new_frame__) -> domain = domain;
  ((struct _async_2450_7frame_t__ *)__hc_new_frame__) -> planeInc = planeInc;
  ((struct _async_2450_7frame_t__ *)__hc_new_frame__) -> rowInc = rowInc;
  ((struct _async_2450_7frame_t__ *)__hc_new_frame__) -> domElems = domElems;
  ((struct _async_2450_7frame_t__ *)__hc_new_frame__) -> dimE = dimE;
  ((struct _async_2450_7frame_t__ *)__hc_new_frame__) -> dimEdimE = dimEdimE;
  return (struct hc_frameHeader *)__hc_new_frame__;
}

void __hc_main__(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int argc;
  char **argv;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
    case 2:
    goto __post_func2;
    case 3:
    goto __post_finish3;
    case 4:
    goto __post_async4;
    case 5:
    goto __post_finish5;
    case 6:
    goto __post_async6;
    case 7:
    goto __post_finish7;
    case 8:
    goto __post_async8;
    case 9:
    goto __post_finish9;
    case 10:
    goto __post_async10;
    case 11:
    goto __post_finish11;
    case 12:
    goto __post_async12;
    case 13:
    goto __post_finish13;
    case 14:
    goto __post_async14;
    case 15:
    goto __post_finish15;
    case 16:
    goto __post_async16;
    case 17:
    goto __post_finish17;
    case 18:
    goto __post_async18;
    case 19:
    goto __post_finish19;
    case 20:
    goto __post_async20;
    case 21:
    goto __post_finish21;
    case 22:
    goto __post_async22;
    case 23:
    goto __post_finish23;
    case 24:
    goto __post_async24;
    case 25:
    goto __post_finish25;
    case 26:
    goto __post_async26;
    case 27:
    goto __post_func27;
  }
  argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
  argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
#endif // OCR or FSIM
#if     defined(FSIM)
// tiny problem size 
// ran to completion quickly with 5, many cycles for 10, 15, 30 and 45
#else   // FSIM
// standard problem size
  Index_t edgeElems = 45;
#endif // FSIM
  Index_t edgeNodes = (edgeElems + 1);
  Index_t domElems;;
#if defined(FSIM) || defined(OCR)
#endif // FSIM or OCR
  domain = ((struct Domain_t *)(malloc((((Index_t )1) * sizeof(struct Domain_t )))));
#if defined(FSIM)
#endif
#if defined(FSIM) || defined(OCR) || defined(UPC)
// for i
#else // DEFAULT all the others
  memset(domain,0,(sizeof((((Index_t )1) * sizeof(struct Domain_t )))));
#endif // FSIM or OCR
;
/****************************/
/*   Initialize Sedov Mesh  */
/****************************/
;
  domain -> m_sizeX = edgeElems;
  domain -> m_sizeY = edgeElems;
  domain -> m_sizeZ = edgeElems;
  domain -> m_numElem = ((edgeElems * edgeElems) * edgeElems);
  domain -> m_numNode = ((edgeNodes * edgeNodes) * edgeNodes);
  domElems = (domain -> m_numElem);;
  __hc_frame__ -> pc = 1;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  struct hc_frameHeader *__hc_domain_AllocateElemPersistent__frame__1 = buildFrame___hc_domain_AllocateElemPersistent__(((size_t )(domain -> m_numElem)),ws);
  __hc_domain_AllocateElemPersistent__(ws,((struct hc_frameHeader *)__hc_domain_AllocateElemPersistent__frame__1),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func1:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  }
  domain_AllocateElemTemporary((domain -> m_numElem));
  __hc_frame__ -> pc = 2;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  struct hc_frameHeader *__hc_domain_AllocateNodalPersistent__frame__2 = buildFrame___hc_domain_AllocateNodalPersistent__(((size_t )(domain -> m_numNode)),ws);
  __hc_domain_AllocateNodalPersistent__(ws,((struct hc_frameHeader *)__hc_domain_AllocateNodalPersistent__frame__2),0);
  if (ws -> costatus == HC_FUNC_COMPLETE) {
    hc_free_unwind_frame(ws);
  }
  else {
    return ;
  }
  if (0) {
    __post_func2:;
    __hc_frame__ = hc_free_unwind_frame(ws);
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  }
  domain_AllocateNodesets((edgeNodes * edgeNodes));;
  hc_startFinish(ws);
{
    Real_t sf = (((Real_t )1.125) / ((Real_t )edgeElems));
    Index_t dimN = edgeNodes;
    Index_t dimNdimN = (edgeNodes * edgeNodes);
    for (Index_t pln = 0; pln < edgeNodes; ++pln) {
      Real_t tz = (((Real_t )pln) * sf);
      Index_t pln_nidx = (pln * dimNdimN);
      for (Index_t row = 0; row < edgeNodes; ++row) {
        Real_t ty = (((Real_t )row) * sf);
        Index_t pln_row_nidx = (pln_nidx + (row * dimN));
        Index_t col_len = edgeNodes;
        Index_t col_blk = 16;
        for (Index_t col_out = 0; col_out < col_len; col_out += col_blk) {
          Index_t col_end = ((col_out + col_blk) < col_len)?(col_out + col_blk) : col_len;
          __hc_frame__ -> pc = 4;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_end = col_end;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_out = col_out;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> ty = ty;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln_row_nidx = pln_row_nidx;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_len = col_len;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_blk = col_blk;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> row = row;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> tz = tz;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln_nidx = pln_nidx;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln = pln;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> sf = sf;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimN = dimN;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimNdimN = dimNdimN;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
          struct hc_frameHeader *_async_2239_9frame__4 = buildFrame__async_2239_9(col_out,col_end,pln,row,tz,ty,pln_row_nidx,domain,sf,dimN,dimNdimN,ws);
          _async_2239_9frame__4 -> sched_policy = hc_sched_policy(ws);
          if (_async_2239_9frame__4 -> sched_policy == HELP_FIRST) {
            deque_push_default(_async_2239_9frame__4);
          }
          else {
            deque_push_default(__hc_frame__);
            _async_2239_9(ws,((struct hc_frameHeader *)_async_2239_9frame__4),0);
            if (ws -> costatus != HC_ASYNC_COMPLETE) {
              return ;
            }
            if (hc_check_continuation_stolen(ws,__hc_frame__)) {
              return ;
            }
          }
          if (0) {
            __post_async4:;
            col_end = ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_end;
            col_out = ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_out;
            ty = ((struct __hc_main__frame_t__ *)__hc_frame__) -> ty;
            pln_row_nidx = ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln_row_nidx;
            col_len = ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_len;
            col_blk = ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_blk;
            row = ((struct __hc_main__frame_t__ *)__hc_frame__) -> row;
            tz = ((struct __hc_main__frame_t__ *)__hc_frame__) -> tz;
            pln_nidx = ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln_nidx;
            pln = ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln;
            sf = ((struct __hc_main__frame_t__ *)__hc_frame__) -> sf;
            dimN = ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimN;
            dimNdimN = ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimNdimN;
            edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
            edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
            domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
            argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
            argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
          }
        }
// for row
      }
// for pln
    }
  }
  __hc_frame__ -> pc = 3;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish3:;
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  };
  hc_startFinish(ws);
{
    Index_t dimE = edgeElems;
    Index_t dimEdimE = (edgeElems * edgeElems);
    Index_t dimN = edgeNodes;
    Index_t dimNdimN = (edgeNodes * edgeNodes);
    for (Index_t pln = 0; pln < edgeElems; ++pln) {
      Index_t pln_nidx = (pln * dimNdimN);
      Index_t pln_zidx = (pln * dimEdimE);
      for (Index_t row = 0; row < edgeElems; ++row) {
        Index_t pln_row_nidx = (pln_nidx + (row * dimN));
        Index_t pln_row_zidx = (pln_zidx + (row * dimE));
        Index_t col_len = edgeElems;
        Index_t col_blk = 16;
        for (Index_t col_out = 0; col_out < col_len; col_out += col_blk) {
          Index_t col_end = ((col_out + col_blk) < col_len)?(col_out + col_blk) : col_len;
          __hc_frame__ -> pc = 6;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_end = col_end;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_out = col_out;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln_row_nidx = pln_row_nidx;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln_row_zidx = pln_row_zidx;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_len = col_len;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_blk = col_blk;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> row = row;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln_nidx = pln_nidx;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln_zidx = pln_zidx;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln = pln;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimE = dimE;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimEdimE = dimEdimE;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimN = dimN;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimNdimN = dimNdimN;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
          ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
          struct hc_frameHeader *_async_2265_9frame__6 = buildFrame__async_2265_9(col_out,col_end,pln,row,pln_row_nidx,pln_row_zidx,domain,dimE,dimEdimE,dimN,dimNdimN,ws);
          _async_2265_9frame__6 -> sched_policy = hc_sched_policy(ws);
          if (_async_2265_9frame__6 -> sched_policy == HELP_FIRST) {
            deque_push_default(_async_2265_9frame__6);
          }
          else {
            deque_push_default(__hc_frame__);
            _async_2265_9(ws,((struct hc_frameHeader *)_async_2265_9frame__6),0);
            if (ws -> costatus != HC_ASYNC_COMPLETE) {
              return ;
            }
            if (hc_check_continuation_stolen(ws,__hc_frame__)) {
              return ;
            }
          }
          if (0) {
            __post_async6:;
            col_end = ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_end;
            col_out = ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_out;
            pln_row_nidx = ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln_row_nidx;
            pln_row_zidx = ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln_row_zidx;
            col_len = ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_len;
            col_blk = ((struct __hc_main__frame_t__ *)__hc_frame__) -> col_blk;
            row = ((struct __hc_main__frame_t__ *)__hc_frame__) -> row;
            pln_nidx = ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln_nidx;
            pln_zidx = ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln_zidx;
            pln = ((struct __hc_main__frame_t__ *)__hc_frame__) -> pln;
            dimE = ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimE;
            dimEdimE = ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimEdimE;
            dimN = ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimN;
            dimNdimN = ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimNdimN;
            edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
            edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
            domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
            argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
            argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
          }
        }
// for row
      }
// for pln
    }
  }
  __hc_frame__ -> pc = 5;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish5:;
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  };
  hc_startFinish(ws);
{
    Index_t i_len = domElems;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 8;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
      struct hc_frameHeader *_async_2285_5frame__8 = buildFrame__async_2285_5(i_out,i_end,domain,ws);
      _async_2285_5frame__8 -> sched_policy = hc_sched_policy(ws);
      if (_async_2285_5frame__8 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_2285_5frame__8);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_2285_5(ws,((struct hc_frameHeader *)_async_2285_5frame__8),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async8:;
        i_end = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk;
        edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
        edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
        domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
        argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
        argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
      }
    }
  }
  __hc_frame__ -> pc = 7;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish7:;
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  };
  domain -> m_dtfixed = ((Real_t )(-1.0e-7));
  domain -> m_deltatime = ((Real_t )1.0e-7);
  domain -> m_deltatimemultlb = ((Real_t )1.1);
  domain -> m_deltatimemultub = ((Real_t )1.2);
  domain -> m_stoptime = ((Real_t )0.01);
  domain -> m_dtcourant = ((Real_t )1.0e+20);
  domain -> m_dthydro = ((Real_t )1.0e+20);
  domain -> m_dtmax = ((Real_t )0.01);
  domain -> m_time = ((Real_t )0.0);
  domain -> m_cycle = 0;
  domain -> m_e_cut = ((Real_t )1.0e-7);
  domain -> m_p_cut = ((Real_t )1.0e-7);
  domain -> m_q_cut = ((Real_t )1.0e-7);
  domain -> m_u_cut = ((Real_t )1.0e-7);
  domain -> m_v_cut = ((Real_t )1.0e-10);
  domain -> m_hgcoef = ((Real_t )3.0);
  domain -> m_ss4o3 = (((Real_t )4.0) / ((Real_t )3.0));
  domain -> m_qstop = ((Real_t )1.0e+12);
  domain -> m_monoq_max_slope = ((Real_t )1.0);
  domain -> m_monoq_limiter_mult = ((Real_t )2.0);
  domain -> m_qlc_monoq = ((Real_t )0.5);
  domain -> m_qqc_monoq = (((Real_t )2.0) / ((Real_t )3.0));
  domain -> m_qqc = ((Real_t )2.0);
  domain -> m_pmin = ((Real_t )0.0);
  domain -> m_emin = ((Real_t )(-1.0e+15));
  domain -> m_dvovmax = ((Real_t )0.1);
  domain -> m_eosvmax = ((Real_t )1.0e+9);
  domain -> m_eosvmin = ((Real_t )1.0e-9);
  domain -> m_refdens = ((Real_t )1.0);
  hc_startFinish(ws);
{
    Index_t i_len = domElems;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 10;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
      struct hc_frameHeader *_async_2330_5frame__10 = buildFrame__async_2330_5(i_out,i_end,domain,ws);
      _async_2330_5frame__10 -> sched_policy = hc_sched_policy(ws);
      if (_async_2330_5frame__10 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_2330_5frame__10);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_2330_5(ws,((struct hc_frameHeader *)_async_2330_5frame__10),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async10:;
        i_end = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk;
        edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
        edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
        domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
        argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
        argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
      }
    }
  }
  __hc_frame__ -> pc = 9;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish9:;
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  };
  (domain -> m_e)[0] = ((Real_t )3.948746e+7);;
//DEBUG fprintf(stdout,"e(0)=%e\n",domain->m_e[0]);
  hc_startFinish(ws);
{
    Index_t dimN = edgeNodes;
    Index_t dimNdimN = (dimN * dimN);
    for (Index_t i = 0; i < edgeNodes; ++i) {
      Index_t planeInc = (i * dimNdimN);
      Index_t rowInc = (i * dimN);
      Index_t j_len = edgeNodes;
      Index_t j_blk = 16;
      for (Index_t j_out = 0; j_out < j_len; j_out += j_blk) {
        Index_t j_end = ((j_out + j_blk) < j_len)?(j_out + j_blk) : j_len;
        __hc_frame__ -> pc = 12;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_end = j_end;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_out = j_out;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> planeInc = planeInc;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> rowInc = rowInc;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_len = j_len;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_blk = j_blk;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> i = i;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimN = dimN;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimNdimN = dimNdimN;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
        struct hc_frameHeader *_async_2382_7frame__12 = buildFrame__async_2382_7(j_out,j_end,i,planeInc,rowInc,domain,dimN,dimNdimN,ws);
        _async_2382_7frame__12 -> sched_policy = hc_sched_policy(ws);
        if (_async_2382_7frame__12 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_2382_7frame__12);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_2382_7(ws,((struct hc_frameHeader *)_async_2382_7frame__12),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async12:;
          j_end = ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_end;
          j_out = ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_out;
          planeInc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> planeInc;
          rowInc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> rowInc;
          j_len = ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_len;
          j_blk = ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_blk;
          i = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i;
          dimN = ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimN;
          dimNdimN = ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimNdimN;
          edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
          edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
          domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
          argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
          argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
        }
      }
// for i
    }
  }
  __hc_frame__ -> pc = 11;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish11:;
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  };
//DEBUG fprintf(stdout,"e(0)=%e\n",domain->m_e[0]);
  hc_startFinish(ws);
{
    (domain -> m_lxim)[0] = 0;
    Index_t i_len = (domElems - 1);
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 14;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
      struct hc_frameHeader *_async_2396_5frame__14 = buildFrame__async_2396_5(i_out,i_end,domain,domElems,ws);
      _async_2396_5frame__14 -> sched_policy = hc_sched_policy(ws);
      if (_async_2396_5frame__14 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_2396_5frame__14);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_2396_5(ws,((struct hc_frameHeader *)_async_2396_5frame__14),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async14:;
        i_end = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk;
        edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
        edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
        domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
        argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
        argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
      }
    }
    (domain -> m_lxip)[domElems - 1] = (domElems - 1);
  }
  __hc_frame__ -> pc = 13;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish13:;
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  }
  hc_startFinish(ws);
{
    Index_t i_len = edgeElems;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 16;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
      struct hc_frameHeader *_async_2404_5frame__16 = buildFrame__async_2404_5(i_out,i_end,domain,domElems,edgeElems,ws);
      _async_2404_5frame__16 -> sched_policy = hc_sched_policy(ws);
      if (_async_2404_5frame__16 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_2404_5frame__16);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_2404_5(ws,((struct hc_frameHeader *)_async_2404_5frame__16),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async16:;
        i_end = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk;
        edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
        edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
        domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
        argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
        argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
      }
    }
  }
  __hc_frame__ -> pc = 15;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish15:;
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  }
  hc_startFinish(ws);
{
    Index_t i_len = (domElems - edgeElems);
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 18;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
      struct hc_frameHeader *_async_2411_5frame__18 = buildFrame__async_2411_5(i_out,i_end,domain,edgeElems,ws);
      _async_2411_5frame__18 -> sched_policy = hc_sched_policy(ws);
      if (_async_2411_5frame__18 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_2411_5frame__18);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_2411_5(ws,((struct hc_frameHeader *)_async_2411_5frame__18),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async18:;
        i_end = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk;
        edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
        edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
        domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
        argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
        argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
      }
    }
  }
  __hc_frame__ -> pc = 17;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish17:;
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  }
  hc_startFinish(ws);
{
    Index_t i_len = (edgeElems * edgeElems);
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 20;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
      struct hc_frameHeader *_async_2418_5frame__20 = buildFrame__async_2418_5(i_out,i_end,domain,domElems,edgeElems,ws);
      _async_2418_5frame__20 -> sched_policy = hc_sched_policy(ws);
      if (_async_2418_5frame__20 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_2418_5frame__20);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_2418_5(ws,((struct hc_frameHeader *)_async_2418_5frame__20),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async20:;
        i_end = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk;
        edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
        edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
        domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
        argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
        argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
      }
    }
  }
  __hc_frame__ -> pc = 19;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish19:;
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  }
  hc_startFinish(ws);
{
    Index_t dimE = edgeElems;
    Index_t dimEdimE = (dimE * dimE);
    Index_t i_len = (domElems - dimEdimE);
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 22;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimE = dimE;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimEdimE = dimEdimE;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
      struct hc_frameHeader *_async_2426_5frame__22 = buildFrame__async_2426_5(i_out,i_end,domain,dimEdimE,domElems,ws);
      _async_2426_5frame__22 -> sched_policy = hc_sched_policy(ws);
      if (_async_2426_5frame__22 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_2426_5frame__22);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_2426_5(ws,((struct hc_frameHeader *)_async_2426_5frame__22),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async22:;
        i_end = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out;
        dimE = ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimE;
        dimEdimE = ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimEdimE;
        i_len = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk;
        edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
        edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
        domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
        argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
        argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
      }
    }
  }
  __hc_frame__ -> pc = 21;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish21:;
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  };
//DEBUG fprintf(stdout,"e(0)=%e\n",domain->m_e[0]);
  hc_startFinish(ws);
{
    Index_t i_len = domElems;
    Index_t i_blk = 16;
    for (Index_t i_out = 0; i_out < i_len; i_out += i_blk) {
      Index_t i_end = ((i_out + i_blk) < i_len)?(i_out + i_blk) : i_len;
      __hc_frame__ -> pc = 24;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end = i_end;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out = i_out;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len = i_len;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk = i_blk;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
      ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
      struct hc_frameHeader *_async_2436_5frame__24 = buildFrame__async_2436_5(i_out,i_end,domain,domElems,ws);
      _async_2436_5frame__24 -> sched_policy = hc_sched_policy(ws);
      if (_async_2436_5frame__24 -> sched_policy == HELP_FIRST) {
        deque_push_default(_async_2436_5frame__24);
      }
      else {
        deque_push_default(__hc_frame__);
        _async_2436_5(ws,((struct hc_frameHeader *)_async_2436_5frame__24),0);
        if (ws -> costatus != HC_ASYNC_COMPLETE) {
          return ;
        }
        if (hc_check_continuation_stolen(ws,__hc_frame__)) {
          return ;
        }
      }
      if (0) {
        __post_async24:;
        i_end = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_end;
        i_out = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_out;
        i_len = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_len;
        i_blk = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i_blk;
        edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
        edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
        domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
        argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
        argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
      }
    }
  }
  __hc_frame__ -> pc = 23;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish23:;
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  };;
//DEBUG fprintf(stdout,"e(0)=%e\n",domain->m_e[0]);
  hc_startFinish(ws);
{
    Index_t dimE = edgeElems;
    Index_t dimEdimE = (dimE * dimE);
    for (Index_t i = 0; i < edgeElems; ++i) {
      Index_t planeInc = (i * dimEdimE);
      Index_t rowInc = (i * dimE);
      Index_t j_len = edgeElems;
      Index_t j_blk = 16;
      for (Index_t j_out = 0; j_out < j_len; j_out += j_blk) {
        Index_t j_end = ((j_out + j_blk) < j_len)?(j_out + j_blk) : j_len;
        __hc_frame__ -> pc = 26;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_end = j_end;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_out = j_out;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> planeInc = planeInc;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> rowInc = rowInc;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_len = j_len;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_blk = j_blk;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> i = i;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimE = dimE;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimEdimE = dimEdimE;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
        ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
        struct hc_frameHeader *_async_2450_7frame__26 = buildFrame__async_2450_7(j_out,j_end,i,domain,planeInc,rowInc,domElems,dimE,dimEdimE,ws);
        _async_2450_7frame__26 -> sched_policy = hc_sched_policy(ws);
        if (_async_2450_7frame__26 -> sched_policy == HELP_FIRST) {
          deque_push_default(_async_2450_7frame__26);
        }
        else {
          deque_push_default(__hc_frame__);
          _async_2450_7(ws,((struct hc_frameHeader *)_async_2450_7frame__26),0);
          if (ws -> costatus != HC_ASYNC_COMPLETE) {
            return ;
          }
          if (hc_check_continuation_stolen(ws,__hc_frame__)) {
            return ;
          }
        }
        if (0) {
          __post_async26:;
          j_end = ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_end;
          j_out = ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_out;
          planeInc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> planeInc;
          rowInc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> rowInc;
          j_len = ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_len;
          j_blk = ((struct __hc_main__frame_t__ *)__hc_frame__) -> j_blk;
          i = ((struct __hc_main__frame_t__ *)__hc_frame__) -> i;
          dimE = ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimE;
          dimEdimE = ((struct __hc_main__frame_t__ *)__hc_frame__) -> dimEdimE;
          edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
          edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
          domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
          argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
          argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
        }
      }
// for i
    }
  }
  __hc_frame__ -> pc = 25;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish25:;
    edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
    edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
    domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
    argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
    argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
  };
#ifdef FSIM
#else // NOT FSIM
#ifdef    HEX
#else  // NOT HEX
  printf("rag: e(0)=%e\n",(domain -> m_e)[0]);
#endif // HEX
#endif // FSIM
  while((domain -> m_time) < (domain -> m_stoptime)){;
    TimeIncrement();;
    __hc_frame__ -> pc = 27;
    ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems = edgeElems;
    ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes = edgeNodes;
    ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems = domElems;
    ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc = argc;
    ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv = argv;
    struct hc_frameHeader *__hc_LagrangeLeapFrog__frame__27 = buildFrame___hc_LagrangeLeapFrog__(ws);
    __hc_LagrangeLeapFrog__(ws,((struct hc_frameHeader *)__hc_LagrangeLeapFrog__frame__27),0);
    if (ws -> costatus == HC_FUNC_COMPLETE) {
      hc_free_unwind_frame(ws);
    }
    else {
      return ;
    }
    if (0) {
      __post_func27:;
      __hc_frame__ = hc_free_unwind_frame(ws);
      edgeElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeElems;
      edgeNodes = ((struct __hc_main__frame_t__ *)__hc_frame__) -> edgeNodes;
      domElems = ((struct __hc_main__frame_t__ *)__hc_frame__) -> domElems;
      argc = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argc;
      argv = ((struct __hc_main__frame_t__ *)__hc_frame__) -> argv;
    }
#if       LULESH_SHOW_PROGRESS
#ifdef      FSIM
#else    // NOT FSIM
#ifdef        HEX
#else      // NOT HEX
    printf("time = %e, dt=%e, e(0)=%e\n",((double )(domain -> m_time)),((double )(domain -> m_deltatime)),((double )(domain -> m_e)[0]));
#endif     // HEX
    fflush(stdout);
#endif   // FSIM
#endif // LULESH_SHOW_PROGRESS
// while time
  }
#ifdef    FSIM
#else //  NOT FSIM
#ifdef      HEX
#else    // NOT HEX
  printf("   Final Origin Energy = %12.6e \n",((double )(domain -> m_e)[0]));
#endif //   HEX
  fflush(stdout);
#endif // FSIM
#if defined(FSIM) || defined(OCR)
#else
;
  domain_DeallocateNodesets();
  domain_DeallocateNodalPersistent();
  domain_DeallocateElemTemporary();
  domain_DeallocateElemPersistent();
  free(domain);
#endif // FSIM or OCR
;
  exit(0);
// IMPOSSIBLE
  ((struct __hc_main__frame_t__ *)__hc_frame__) -> __rtval__ = 0;
  ws -> costatus = HC_FUNC_COMPLETE;
  hc_setDone(ws);
  return ;
}

void _async_2450_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long j_out;
  long j_end;
  long i;
  struct Domain_t *domain;
  long planeInc;
  long rowInc;
  long domElems;
  long dimE;
  long dimEdimE;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  j_out = ((struct _async_2450_7frame_t__ *)__hc_frame__) -> j_out;
  j_end = ((struct _async_2450_7frame_t__ *)__hc_frame__) -> j_end;
  i = ((struct _async_2450_7frame_t__ *)__hc_frame__) -> i;
  domain = ((struct _async_2450_7frame_t__ *)__hc_frame__) -> domain;
  planeInc = ((struct _async_2450_7frame_t__ *)__hc_frame__) -> planeInc;
  rowInc = ((struct _async_2450_7frame_t__ *)__hc_frame__) -> rowInc;
  domElems = ((struct _async_2450_7frame_t__ *)__hc_frame__) -> domElems;
  dimE = ((struct _async_2450_7frame_t__ *)__hc_frame__) -> dimE;
  dimEdimE = ((struct _async_2450_7frame_t__ *)__hc_frame__) -> dimEdimE;
  for (Index_t j = j_out; j < j_end; ++j) {
    (domain -> m_elemBC)[planeInc + (j * dimE)] |= 1;
    (domain -> m_elemBC)[((planeInc + (j * dimE)) + (1 * dimE)) - 1] |= 8;
    (domain -> m_elemBC)[planeInc + j] |= 16;
    (domain -> m_elemBC)[((planeInc + j) + dimEdimE) - dimE] |= 128;
    (domain -> m_elemBC)[rowInc + j] |= 256;
    (domain -> m_elemBC)[((rowInc + j) + domElems) - dimEdimE] |= 2048;
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_2436_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  long domElems;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_2436_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_2436_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_2436_5frame_t__ *)__hc_frame__) -> domain;
  domElems = ((struct _async_2436_5frame_t__ *)__hc_frame__) -> domElems;
  for (Index_t i = i_out; i < i_end; ++i) {
/* clear BCs by default */
    (domain -> m_elemBC)[i] = 0;
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_2426_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  long dimEdimE;
  long domElems;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_2426_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_2426_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_2426_5frame_t__ *)__hc_frame__) -> domain;
  dimEdimE = ((struct _async_2426_5frame_t__ *)__hc_frame__) -> dimEdimE;
  domElems = ((struct _async_2426_5frame_t__ *)__hc_frame__) -> domElems;
  for (Index_t i = i_out; i < i_end; ++i) {
    (domain -> m_lzetam)[i + dimEdimE] = i;
    (domain -> m_lzetap)[i] = (i + dimEdimE);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_2418_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  long domElems;
  long edgeElems;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_2418_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_2418_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_2418_5frame_t__ *)__hc_frame__) -> domain;
  domElems = ((struct _async_2418_5frame_t__ *)__hc_frame__) -> domElems;
  edgeElems = ((struct _async_2418_5frame_t__ *)__hc_frame__) -> edgeElems;
  for (Index_t i = i_out; i < i_end; ++i) {
    (domain -> m_lzetam)[i] = i;
    (domain -> m_lzetap)[(domElems - (edgeElems * edgeElems)) + i] = ((domElems - (edgeElems * edgeElems)) + i);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_2411_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  long edgeElems;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_2411_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_2411_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_2411_5frame_t__ *)__hc_frame__) -> domain;
  edgeElems = ((struct _async_2411_5frame_t__ *)__hc_frame__) -> edgeElems;
  for (Index_t i = i_out; i < i_end; ++i) {
    (domain -> m_letam)[i + edgeElems] = i;
    (domain -> m_letap)[i] = (i + edgeElems);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_2404_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  long domElems;
  long edgeElems;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_2404_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_2404_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_2404_5frame_t__ *)__hc_frame__) -> domain;
  domElems = ((struct _async_2404_5frame_t__ *)__hc_frame__) -> domElems;
  edgeElems = ((struct _async_2404_5frame_t__ *)__hc_frame__) -> edgeElems;
  for (Index_t i = i_out; i < i_end; ++i) {
    (domain -> m_letam)[i] = i;
    (domain -> m_letap)[(domElems - edgeElems) + i] = ((domElems - edgeElems) + i);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_2396_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  long domElems;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_2396_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_2396_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_2396_5frame_t__ *)__hc_frame__) -> domain;
  domElems = ((struct _async_2396_5frame_t__ *)__hc_frame__) -> domElems;
  for (Index_t i = i_out; i < i_end; ++i) {
    (domain -> m_lxim)[i + 1] = i;
    (domain -> m_lxip)[i] = (i + 1);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_2382_7(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long j_out;
  long j_end;
  long i;
  long planeInc;
  long rowInc;
  struct Domain_t *domain;
  long dimN;
  long dimNdimN;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  j_out = ((struct _async_2382_7frame_t__ *)__hc_frame__) -> j_out;
  j_end = ((struct _async_2382_7frame_t__ *)__hc_frame__) -> j_end;
  i = ((struct _async_2382_7frame_t__ *)__hc_frame__) -> i;
  planeInc = ((struct _async_2382_7frame_t__ *)__hc_frame__) -> planeInc;
  rowInc = ((struct _async_2382_7frame_t__ *)__hc_frame__) -> rowInc;
  domain = ((struct _async_2382_7frame_t__ *)__hc_frame__) -> domain;
  dimN = ((struct _async_2382_7frame_t__ *)__hc_frame__) -> dimN;
  dimNdimN = ((struct _async_2382_7frame_t__ *)__hc_frame__) -> dimNdimN;
  for (Index_t j = j_out; j < j_end; ++j) {
    Index_t nidx = (rowInc + j);
    (domain -> m_symmX)[nidx] = (planeInc + (j * dimN));
    (domain -> m_symmY)[nidx] = (planeInc + j);
    (domain -> m_symmZ)[nidx] = (rowInc + j);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_2330_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_2330_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_2330_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_2330_5frame_t__ *)__hc_frame__) -> domain;
  for (Index_t i = i_out; i < i_end; ++i) {
#if  defined(HAB_C)
    Real_t *x_local = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *y_local = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
    Real_t *z_local = (Real_t *)(malloc((((Index_t )8) * sizeof(Real_t ))));
#else // NOT HAB_C
#endif //    HAB_C
    Index_t *elemToNode = ((domain -> m_nodelist) + (((Index_t )8) * i));
    for (Index_t lnode = 0; lnode < ((Index_t )8); ++lnode) {
      Index_t gnode = elemToNode[lnode];
      x_local[lnode] = (domain -> m_x)[gnode];
      y_local[lnode] = (domain -> m_y)[gnode];
      z_local[lnode] = (domain -> m_z)[gnode];
// for lnode
    }
// volume calculations
    Real_t volume = CalcElemVolume(x_local,y_local,z_local);
    (domain -> m_volo)[i] = volume;
    (domain -> m_elemMass)[i] = volume;
// RAG ///////////////////////////////////////////////////////// RAG //
// RAG  Atomic Memory Floating-point Addition Scatter operation  RAG //
// RAG ///////////////////////////////////////////////////////// RAG //
    for (Index_t j = 0; j < ((Index_t )8); ++j) {
      Index_t idx = elemToNode[j];
      Real_t value = (volume / ((Real_t )8.));
      AMO__sync_addition_double(((domain -> m_nodalMass) + idx),value);
// for j
    }
#if  defined(HAB_C)
    free(z_local);
    free(y_local);
    free(x_local);
#endif //    HAB_C
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_2285_5(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long i_out;
  long i_end;
  struct Domain_t *domain;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i_out = ((struct _async_2285_5frame_t__ *)__hc_frame__) -> i_out;
  i_end = ((struct _async_2285_5frame_t__ *)__hc_frame__) -> i_end;
  domain = ((struct _async_2285_5frame_t__ *)__hc_frame__) -> domain;
  for (Index_t i = i_out; i < i_end; ++i) {
    (domain -> m_matElemlist)[i] = i;
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_2265_9(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long col_out;
  long col_end;
  long pln;
  long row;
  long pln_row_nidx;
  long pln_row_zidx;
  struct Domain_t *domain;
  long dimE;
  long dimEdimE;
  long dimN;
  long dimNdimN;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  col_out = ((struct _async_2265_9frame_t__ *)__hc_frame__) -> col_out;
  col_end = ((struct _async_2265_9frame_t__ *)__hc_frame__) -> col_end;
  pln = ((struct _async_2265_9frame_t__ *)__hc_frame__) -> pln;
  row = ((struct _async_2265_9frame_t__ *)__hc_frame__) -> row;
  pln_row_nidx = ((struct _async_2265_9frame_t__ *)__hc_frame__) -> pln_row_nidx;
  pln_row_zidx = ((struct _async_2265_9frame_t__ *)__hc_frame__) -> pln_row_zidx;
  domain = ((struct _async_2265_9frame_t__ *)__hc_frame__) -> domain;
  dimE = ((struct _async_2265_9frame_t__ *)__hc_frame__) -> dimE;
  dimEdimE = ((struct _async_2265_9frame_t__ *)__hc_frame__) -> dimEdimE;
  dimN = ((struct _async_2265_9frame_t__ *)__hc_frame__) -> dimN;
  dimNdimN = ((struct _async_2265_9frame_t__ *)__hc_frame__) -> dimNdimN;
  for (Index_t col = col_out; col < col_end; ++col) {
    Index_t nidx = (pln_row_nidx + col);
    Index_t zidx = (pln_row_zidx + col);
    Index_t *localNode = ((domain -> m_nodelist) + (((Index_t )8) * zidx));
    localNode[0] = nidx;
    localNode[1] = (nidx + 1);
    localNode[2] = ((nidx + dimN) + 1);
    localNode[3] = (nidx + dimN);
    localNode[4] = (nidx + dimNdimN);
    localNode[5] = ((nidx + dimNdimN) + 1);
    localNode[6] = (((nidx + dimNdimN) + dimN) + 1);
    localNode[7] = ((nidx + dimNdimN) + dimN);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

void _async_2239_9(struct hc_workerState *ws,struct hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  long col_out;
  long col_end;
  long pln;
  long row;
  double tz;
  double ty;
  long pln_row_nidx;
  struct Domain_t *domain;
  double sf;
  long dimN;
  long dimNdimN;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  col_out = ((struct _async_2239_9frame_t__ *)__hc_frame__) -> col_out;
  col_end = ((struct _async_2239_9frame_t__ *)__hc_frame__) -> col_end;
  pln = ((struct _async_2239_9frame_t__ *)__hc_frame__) -> pln;
  row = ((struct _async_2239_9frame_t__ *)__hc_frame__) -> row;
  tz = ((struct _async_2239_9frame_t__ *)__hc_frame__) -> tz;
  ty = ((struct _async_2239_9frame_t__ *)__hc_frame__) -> ty;
  pln_row_nidx = ((struct _async_2239_9frame_t__ *)__hc_frame__) -> pln_row_nidx;
  domain = ((struct _async_2239_9frame_t__ *)__hc_frame__) -> domain;
  sf = ((struct _async_2239_9frame_t__ *)__hc_frame__) -> sf;
  dimN = ((struct _async_2239_9frame_t__ *)__hc_frame__) -> dimN;
  dimNdimN = ((struct _async_2239_9frame_t__ *)__hc_frame__) -> dimNdimN;
  for (Index_t col = col_out; col < col_end; ++col) {
    Real_t tx = (((Real_t )col) * sf);
    Index_t nidx = (pln_row_nidx + col);
    (domain -> m_x)[nidx] = tx;
//DEBUG if(nidx==1)fprintf(stdout,"m_x[1] = %e\n",domain->m_x[1]);
    (domain -> m_y)[nidx] = ty;
//DEBUG if(nidx==1)fprintf(stdout,"m_y[1] = %e\n",domain->m_y[1]);
    (domain -> m_z)[nidx] = tz;
//DEBUG if(nidx==1)fprintf(stdout,"m_z[1] = %e\n",domain->m_z[1]);
  }
  ws -> costatus = HC_ASYNC_COMPLETE;
  return ;
}

int main(int argc,char **argv)
{
  struct hc_options options;
  struct hc_context ctxt;
  hc_comm_init(&argc,&argv,&ctxt);
  read_options(&options,&argc,argv);
  hc_rt0(&options,&ctxt,"main");
  struct hc_workerState *ws = current_ws();
  ws -> costatus = HC_FUNC_COMPLETE;
  struct __hc_main__frame_t__ *__hc_frame__ = (struct __hc_main__frame_t__ *)(hc_init_susp_mth_frame_ws(ws,sizeof(struct __hc_main__frame_t__ ),__hc_main__));
  __hc_frame__ -> argc = argc;
  __hc_frame__ -> argv = argv;
  ((struct hc_frameHeader *)__hc_frame__) -> rtvalp = &__hc_frame__ -> __rtval__;
  hc_rt_roll(&ctxt,((struct hc_frameHeader *)__hc_frame__));
  int __rt_value__ = __hc_frame__ -> __rtval__;
  HC_FREE(ws,__hc_frame__);
  hc_rt_cleanup(&ctxt);
  hc_comm_finalize();
  return __rt_value__;
// main()
}
