#define LULESH_SHOW_PROGRESS 1

enum { VolumeError = -1, QStopError = -2 } ;

#if defined(OCR)
//RAG HACK FOR EMULATION OF FSIM ON linux with OCR
#define xe_printf(...) printf(__VA_ARGS__)
#endif
#if defined(FSIM)
#if 1
#define TRACE0(str) xe_printf("RAG:: %s\n",str);
#define TRACE1(str) xe_printf("RAG:::: %s\n",str);
#define TRACE2(str) xe_printf("RAG:::::: %s\n",str);
#define TRACE3(str) xe_printf("RAG:::::::: %s\n",str);
#define TRACE4(str) xe_printf("RAG:::::::::: %s\n",str);
#define TRACE5(str) xe_printf("RAG:::::::::::: %s\n",str);
#define TRACE6(str) xe_printf("RAG:::::::::::::: %s\n",str);
#else // 0 or 1
#define TRACE0(str) xe_printf("RAG:: %s\n",str);
#define TRACE1(str)
#define TRACE2(str)
#define TRACE3(str)
#define TRACE4(str)
#define TRACE5(str)
#define TRACE6(str)
#endif // 0 or 1
#elif defined(OCR)
#if       0
#define TRACE0(str)    printf("RAG:: %s\n",str);fflush(stdout);
#define TRACE1(str)    printf("RAG:::: %s\n",str);fflush(stdout);
#define TRACE2(str)    printf("RAG:::::: %s\n",str);fflush(stdout);
#define TRACE3(str)    printf("RAG:::::::: %s\n",str);fflush(stdout);
#define TRACE4(str)    printf("RAG:::::::::: %s\n",str);fflush(stdout);
#define TRACE5(str)    printf("RAG:::::::::::: %s\n",str);fflush(stdout);
#define TRACE6(str)    printf("RAG:::::::::::::: %s\n",str);fflush(stdout);
#else  // 0 or 1
#define TRACE0(str)    //printf("RAG:: %s\n",str);fflush(stdout);
#define TRACE1(str)
#define TRACE2(str)
#define TRACE3(str)
#define TRACE4(str)
#define TRACE5(str)
#define TRACE6(str)
#endif // 0 or 1
#else // NOT FSIM or OCR
#define TRACE0(str)
#define TRACE1(str)
#define TRACE2(str)
#define TRACE3(str)
#define TRACE4(str)
#define TRACE5(str)
#define TRACE6(str)
#endif // FSIM or OCR
////////////////////////////////////////////////////////////////////////
// RAG some generic macros to address language restrictions
////////////////////////////////////////////////////////////////////////
#ifndef OCR_SPAD_WORKAROUND
#define OCR_SPAD_WORKAROUND 0
#endif

#if  defined(HAB_C)

#define INLINE
#define HC_UPC_CONST
#define HAB_CONST
#define SHARED
#define EXIT(code) exit(code)
#include "hc.h"

#elif defined(CILK)

#define INLINE inline
#define HC_UPC_CONST const
#define HAB_CONST const
#define SHARED
#define EXIT(code) exit(code)

#include <cilk/cilk.h>

#elif defined(UPC)

#define INLINE inline
#define HC_UPC_CONST
#define HAB_CONST const
#define SHARED shared
#define EXIT(code) exit(code)

#include <upc_strict.h>
#include <upc_collective.h>

#elif defined(FSIM)

#define INLINE inline
#define HC_UPC_CONST const
#define HAB_CONST const
// RAG RAG workaround llvm compiler bug when passing shared point to guid to some functions
#if 0
#define SHARED rmdglobal
#else
#define SHARED
#endif
#define EXIT(code) { xe_printf("RAG: exit(%d)\n",code); ocrShutdown(); }

#include "xe-edt.h"
#include "xe-memory.h"
#include "xe-console.h"
#include "xe-global.h"

struct DomainObject_t {
  ocrGuid_t guid;
  uint64_t *base;
  size_t    offset;
  size_t    limit;
  size_t    edgeElems;
  size_t    edgeNodes;
};

#elif defined(OCR)

#define INLINE inline
#define HC_UPC_CONST const
#define HAB_CONST const
#define SHARED
#define EXIT(code) { /*printf("RAG: exit(%d)\n",code);*/ ocrShutdown(); exit(code); }

#include "ocr.h"

struct DomainObject_t {
  ocrGuid_t guid;
  void     *base;
  size_t    offset;
  size_t    limit;
  size_t    edgeElems;
  size_t    edgeNodes;
};

#else // DEFAULT is C99

#define INLINE inline
#define HC_UPC_CONST const
#define HAB_CONST const
#define SHARED
#define EXIT(code) exit(code)

#endif // HAB_C, CILK, UPC, FSIM or C99

/****************************************************/
/* Allow flexibility for arithmetic representations */
/****************************************************/

/* Could also support fixed point and interval arithmetic types */

typedef int64_t  Index_t ; /* array subscript and loop index */
typedef double   Real_t ;  /* floating point representation */
typedef uint64_t Int_t ;   /* integer representation */

#define   cast_Int_t(arg) (  (Int_t)(arg))
#define  cast_Real_t(arg) ( (Real_t)(arg))
#define cast_Index_t(arg) ((Index_t)(arg))

/************************************************************/
/* Allow for flexible data layout experiments by separating */
/* array interface from underlying implementation.          */
/************************************************************/

#define ONE ((Index_t)1)
// The NumberOfDimensions
#define THREE ((Index_t)3)
// Either 2^3/2 or 2^2 not sure which
#define FOUR ((Index_t)4)
// Either 6 or 2*3
#define SIX ((Index_t)6)
// 2^NumberOfDimensions
#define EIGHT ((Index_t)8)

/* Stuff needed for boundary conditions */
/* 2 BCs on each of 6 hexahedral faces (12 bits) */
#
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

/************************************************************/
/* Allow for flexible data layout experiments by separating */
/* array interface from underlying implementation.          */
/************************************************************/

struct Domain_t {

   /******************/
   /* Implementation */
   /******************/

   /* Node-centered */

   SHARED Real_t  *m_x ;          /* coordinates */
   SHARED Real_t  *m_y ;
   SHARED Real_t  *m_z ;

   SHARED Real_t  *m_xd ;         /* velocities */
   SHARED Real_t  *m_yd ;
   SHARED Real_t  *m_zd ;

   SHARED Real_t  *m_xdd ;        /* accelerations */
   SHARED Real_t  *m_ydd ;
   SHARED Real_t  *m_zdd ;

   SHARED Real_t  *m_fx ;         /* forces */
   SHARED Real_t  *m_fy ;
   SHARED Real_t  *m_fz ;

   SHARED Real_t  *m_nodalMass ;  /* mass */

   SHARED Index_t *m_symmX ;      /* symmetry plane nodesets */
   SHARED Index_t *m_symmY ;
   SHARED Index_t *m_symmZ ;

   /* Element-centered */

   SHARED Index_t *m_matElemlist ;  /* material indexset */
   SHARED Index_t *m_nodelist ;     /* elemToNode connectivity */

   SHARED Index_t *m_lxim ;         /* element connectivity across each face */
   SHARED Index_t *m_lxip ;
   SHARED Index_t *m_letam ;
   SHARED Index_t *m_letap ;
   SHARED Index_t *m_lzetam ;
   SHARED Index_t *m_lzetap ;

   SHARED Int_t   *m_elemBC ;       /* symmetry/free-surface flags for each elem face */

   SHARED Real_t  *m_dxx ;          /* principal strains -- temporary */
   SHARED Real_t  *m_dyy ;
   SHARED Real_t  *m_dzz ;

   SHARED Real_t  *m_delv_xi ;      /* velocity gradient -- temporary */
   SHARED Real_t  *m_delv_eta ;
   SHARED Real_t  *m_delv_zeta ;

   SHARED Real_t  *m_delx_xi ;      /* coordinate gradient -- temporary */
   SHARED Real_t  *m_delx_eta ;
   SHARED Real_t  *m_delx_zeta ;

   SHARED Real_t  *m_e ;            /* energy */

   SHARED Real_t  *m_p ;            /* pressure */
   SHARED Real_t  *m_q ;            /* q */
   SHARED Real_t  *m_ql ;           /* linear term for q */
   SHARED Real_t  *m_qq ;           /* quadratic term for q */

   SHARED Real_t  *m_v ;            /* relative volume */
   SHARED Real_t  *m_volo ;         /* reference volume */
   SHARED Real_t  *m_vnew ;         /* new relative volume -- temporary */
   SHARED Real_t  *m_delv ;         /* m_vnew - m_v */
   SHARED Real_t  *m_vdov ;         /* volume derivative over volume */

   SHARED Real_t  *m_arealg ;       /* characteristic length of an element */

   SHARED Real_t  *m_ss ;           /* "sound speed" */

   SHARED Real_t  *m_elemMass ;     /* mass */

   /* Parameters */

   Real_t  m_dtfixed ;           /* fixed time increment */
   Real_t  m_time ;              /* current time */
   Real_t  m_deltatime ;         /* variable time increment */
   Real_t  m_deltatimemultlb ;
   Real_t  m_deltatimemultub ;
   Real_t  m_stoptime ;          /* end time for simulation */

   Real_t  m_u_cut ;             /* velocity tolerance */
   Real_t  m_hgcoef ;            /* hourglass control */
   Real_t  m_qstop ;             /* excessive q indicator */
   Real_t  m_monoq_max_slope ;
   Real_t  m_monoq_limiter_mult ;
   Real_t  m_e_cut ;             /* energy tolerance */
   Real_t  m_p_cut ;             /* pressure tolerance */
   Real_t  m_q_cut ;             /* q tolerance */
   Real_t  m_v_cut ;             /* relative volume tolerance */
   Real_t  m_qlc_monoq ;         /* linear term coef for q */
   Real_t  m_qqc_monoq ;         /* quadratic term coef for q */
   Real_t  m_qqc ;
   Real_t  m_eosvmax ;
   Real_t  m_eosvmin ;
   Real_t  m_pmin ;              /* pressure floor */
   Real_t  m_emin ;              /* energy floor */
   Real_t  m_dvovmax ;           /* maximum allowable volume change */
   Real_t  m_refdens ;           /* reference density */

   Real_t  m_dtcourant ;         /* courant constraint */
   Real_t  m_dthydro ;           /* volume change constraint */
   Real_t  m_dtmax ;             /* maximum allowable time increment */

   Int_t   m_cycle ;             /* iteration count for simulation */

   Index_t m_sizeX ;           /* X,Y,Z extent of this block */
   Index_t m_sizeY ;
   Index_t m_sizeZ ;

   Index_t m_numElem ;         /* Elements/Nodes in this domain */
   Index_t m_numNode ;
};

/* This first implementation allows for runnable code */
/* and is not meant to be optimal. Final implementation */
/* should separate declaration and allocation phases */
/* so that allocation can be scheduled in a cache conscious */
/* manner. */

#if 0
   /**********/
   /* Access */
   /**********/

   /* Node-centered */

#define domain_x(idx)           domain->m_x[(Index_t)(idx)]
#define domain_y(idx)           domain->m_y[(Index_t)(idx)]
#define domain_z(idx)           domain->m_z[(Index_t)(idx)]

#define domain_xd(idx)          domain->m_xd[(Index_t)(idx)]
#define domain_yd(idx)          domain->m_yd[(Index_t)(idx)]
#define domain_zd(idx)          domain->m_zd[(Index_t)(idx)]

#define domain_xdd(idx)         domain->m_xdd[(Index_t)(idx)]
#define domain_ydd(idx)         domain->m_ydd[(Index_t)(idx)]
#define domain_zdd(idx)         domain->m_zdd[(Index_t)(idx)]

#define domain_fx(idx)          domain->m_fx[(Index_t)(idx)]
#define domain_fy(idx)          domain->m_fy[(Index_t)(idx)]
#define domain_fz(idx)          domain->m_fz[(Index_t)(idx)]

#define domain_nodalMass(idx)   domain->m_nodalMass[(Index_t)(idx)]

#define domain_symmX(idx)       domain->m_symmX[(Index_t)(idx)]
#define domain_symmY(idx)       domain->m_symmY[(Index_t)(idx)]
#define domain_symmZ(idx)       domain->m_symmZ[(Index_t)(idx)]

   /* Element-centered */

#define domain_matElemlist(idx) domain->m_matElemlist[(Index_t)(idx)]
#define domain_nodelist(idx)    domain->m_nodelist[EIGHT*(Index_t)(idx)]
#define domain_nodelist_ref(idx) ((SHARED Index_t *)&domain->m_nodelist[EIGHT*(Index_t)(idx)])

#define domain_lxim(idx)        domain->m_lxim[(Index_t)(idx)]
#define domain_lxip(idx)        domain->m_lxip[(Index_t)(idx)]
#define domain_letam(idx)       domain->m_letam[(Index_t)(idx)]
#define domain_letap(idx)       domain->m_letap[(Index_t)(idx)]
#define domain_lzetam(idx)      domain->m_lzetam[(Index_t)(idx)]
#define domain_lzetap(idx)      domain->m_lzetap[(Index_t)(idx)]

#define domain_elemBC(idx)      domain->m_elemBC[(Index_t)(idx)]

#define domain_dxx(idx)         domain->m_dxx[idx]
#define domain_dyy(idx)         domain->m_dyy[idx]
#define domain_dzz(idx)         domain->m_dzz[idx]

#define domain_delv_xi(idx)     domain->m_delv_xi[(Index_t)(idx)]
#define domain_delv_eta(idx)    domain->m_delv_eta[(Index_t)(idx)]
#define domain_delv_zeta(idx)   domain->m_delv_zeta[(Index_t)(idx)]

#define domain_delx_xi(idx)     domain->m_delx_xi[(Index_t)(idx)]
#define domain_delx_eta(idx)    domain->m_delx_eta[(Index_t)(idx)]
#define domain_delx_zeta(idx)   domain->m_delx_zeta[(Index_t)(idx)]

#define domain_e(idx)           domain->m_e[(Index_t)(idx)]

#define domain_p(idx)           domain->m_p[(Index_t)(idx)]
#define domain_q(idx)           domain->m_q[(Index_t)(idx)]
#define domain_ql(idx)          domain->m_ql[(Index_t)(idx)]
#define domain_qq(idx)          domain->m_qq[(Index_t)(idx)]

#define domain_v(idx)           domain->m_v[(Index_t)(idx)]
#define domain_volo(idx)        domain->m_volo[(Index_t)(idx)]
#define domain_vnew(idx)        domain->m_vnew[(Index_t)(idx)]
#define domain_delv(idx)        domain->m_delv[(Index_t)(idx)]
#define domain_vdov(idx)        domain->m_vdov[(Index_t)(idx)]

#define domain_arealg(idx)      domain->m_arealg[(Index_t)(idx)]

#define domain_ss(idx)          domain->m_ss[(Index_t)(idx)]

#define domain_elemMass(idx)    domain->m_elemMass[(Index_t)(idx)]

   /* Params */

#define domain_dtfixed()              domain->m_dtfixed
#define domain_time()                 domain->m_time
#define domain_deltatime()            domain->m_deltatime
#define domain_deltatimemultlb()      domain->m_deltatimemultlb
#define domain_deltatimemultub()      domain->m_deltatimemultub
#define domain_stoptime()             domain->m_stoptime

#define domain_u_cut()                domain->m_u_cut
#define domain_hgcoef()               domain->m_hgcoef
#define domain_qstop()                domain->m_qstop
#define domain_monoq_max_slope()      domain->m_monoq_max_slope
#define domain_monoq_limiter_mult()   domain->m_monoq_limiter_mult
#define domain_e_cut()                domain->m_e_cut
#define domain_p_cut()                domain->m_p_cut
#define domain_q_cut()                domain->m_q_cut
#define domain_v_cut()                domain->m_v_cut
#define domain_qlc_monoq()            domain->m_qlc_monoq
#define domain_qqc_monoq()            domain->m_qqc_monoq
#define domain_qqc()                  domain->m_qqc
#define domain_eosvmax()              domain->m_eosvmax
#define domain_eosvmin()              domain->m_eosvmin
#define domain_pmin()                 domain->m_pmin
#define domain_emin()                 domain->m_emin
#define domain_dvovmax()              domain->m_dvovmax
#define domain_refdens()              domain->m_refdens

#define domain_dtcourant()            domain->m_dtcourant
#define domain_dthydro()              domain->m_dthydro
#define domain_dtmax()                domain->m_dtmax

#define domain_cycle()                domain->m_cycle

#define domain_sizeX()                domain->m_sizeX
#define domain_sizeY()                domain->m_sizeY
#define domain_sizeZ()                domain->m_sizeZ
#define domain_numElem()              domain->m_numElem
#define domain_numNode()              domain->m_numNode
#endif

////////////////////////////////////////////////////////////////////////
// RAG some generic macros to address parallelization
// RAG and stripmapping and tiling in a general manner
////////////////////////////////////////////////////////////////////////

#if   defined(HAB_C)

// RAG good choice might be a small multiple of a cacheline size
// otherwise i think there is to much false sharing or runtime overhead.
#define HAB_C_BLK_SIZE (16)
// RAG -- to test forasync set #if 1 below to #if 0
#if 1 // DO NOT USE forasync

#define EDT_PAR_FOR_0xNx1(index,len,edt_name, ... ) \
  { Index_t index ## _len = (len) , index ## _blk = (HAB_C_BLK_SIZE) ; \
    for ( Index_t index ## _out=0 ; index ## _out < index ## _len ; index ## _out  += index ## _blk ) { \
      Index_t index ## _end = ( index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len; \
      async IN(index ## _out , index ## _end , __VA_ARGS__ ) { \
      edt_name(index ## _out , index ## _end , __VA_ARGS__ ); \
  } } } // for async IN(...) for (index,len)

#define PAR_FOR_0xNx1(index,len, ... ) \
  Index_t index ## _len = (len); \
  Index_t index ## _blk = (HAB_C_BLK_SIZE); \
  for ( Index_t index ## _out=0; index ## _out < index ## _len; index ## _out  += index ## _blk ) { \
    Index_t index ## _end = ( index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len; \
    async IN (index ## _out, index ## _end, __VA_ARGS__ ) { \
      for ( Index_t index = index ## _out ; index < index ## _end; ++index ) {

#define END_PAR_FOR(index) \
  } } } // for async IN(...) for (index,len)

#define FINISH \
  finish {

#define END_FINISH \
  } // finish

#else  // DO USE forasync

#define EDT_PAR_FOR_0xNx1(index,len,edt_name, ... ) \
  { Index_t index ## _len = (len) , index ## _blk = (HAB_C_BLK_SIZE) ; \
    for ( Index_t index ## _out=0 ; index ## _out < index ## _len ; index ## _out  += index ## _blk ) { \
      Index_t index ## _end = ( index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len; \
      async IN(index ## _out , index ## _end , __VA_ARGS__ ) { \
      edt_name(index ## _out , index ## _end , __VA_ARGS__ ); \
  } } } // for async IN(...) for (index,len)

#define PAR_FOR_0xNx1(index,len, ... ) \
  { Index_t index = 0, index ## _blk = HAB_C_BLK_SIZE ; \
  forasync in(__VA_ARGS__) point(index) size(len) seq(index ## _blk) {

#define END_PAR_FOR(index) \
  } } // forasync(index,len) in(...)

#define FINISH \
  finish {

#define END_FINISH \
  } // finish
#endif // DO OR DO NOT USE forasync

#elif defined(CILK)

#define CILK_BLK_SIZE (16)

#define EDT_PAR_FOR_0xNx1(index,len,edt_name, ... ) \
  { Index_t index ## _len = (len) , index ## _blk = (CILK_BLK_SIZE) ; \
    cilk_for ( Index_t index ## _out=0 ; index ## _out < index ## _len ; index ## _out  += index ## _blk ) { \
      Index_t index ## _end = ( index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len; \
      edt_name(index ## _out , index ## _end , __VA_ARGS__ ); \
  } } // cilk_for (index)

#define BLK_PAR_FOR_0xNx1(index,len, ... ) \
  { Index_t index ## _len = (len) , index ## _blk = (CILK_BLK_SIZE) ; \
    cilk_for ( Index_t index ## _out=0 ; index ## _out < index ## _len ; index ## _out  += index ## _blk ) { \
      Index_t index ## _end = ( index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len; \
      for(Index_t index = index ## _out ; index < index ## _end ; ++index ) {

#define END_BLK_PAR_FOR(index) \
  } } } // for inner and cilk_for outer ( index ) \

#define PAR_FOR_0xNx1(index,len, ... ) \
  cilk_for ( Index_t index = 0 ; index < len ; ++index ) {

#define END_PAR_FOR(index) \
  } // cilk_for (index=0, index<len, ++index)

#define FINISH {
#define END_FINISH cilk_sync; }

#elif defined(UPC)

#define PAR_FOR_0xNx1(index,len, ... ) \
  for ( Index_t index = 0 ; index < len ; ++index /*; index */ ) {

#define END_PAR_FOR(index) \
  } // upc_forall( index=0 ; index < len ; ++index, index )

#define FINISH {
#define END_FINISH upc_barrier;}

#elif defined(FSIM)

#define FSIM_BLK_SIZE (16)

#define EDT_PAR_FOR_0xNx1(index,len,edt_name, ... ) \
  for ( Index_t index ## _out=0 , index ## _len = (len) , index ## _blk = (FSIM_BLK_SIZE) \
      ; index ## _out < index ## _len \
      ; index ## _out  += index ## _blk ) { \
    Index_t index ## _end = ( index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len; \
    edt_name(index ## _out , index ## _end , __VA_ARGS__ ); \
  }

#define BLK_PAR_FOR_0xNx1(index,len, ... ) \
  for ( Index_t index ## _out=0, index ## _len = (len) , index ## _blk = (FSIM_BLK_SIZE) \
      ; index ## _out < index ## _len \
      ; index ## _out  += index ## _blk ) { \
    Index_t index ## _end = ( index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len; \
    for(Index_t index = index ## _out ; index < index ## _end ; ++index ) {

#define END_BLK_PAR_FOR(index) \
  } } // for inner and for outer ( index ) \

#define PAR_FOR_0xNx1(index,len, ... ) \
  for ( Index_t index = 0 ; index < len ; ++index ) {

#define END_PAR_FOR(index) \
  } // for( index=0 ; index < len ; ++index )

#define FINISH {
#define END_FINISH }

#elif defined(OCR)

#define OCR_BLK_SIZE (16)

#define EDT_PAR_FOR_0xNx1(index,len,edt_name, ... ) \
  for ( Index_t index ## _out=0 , index ## _len = (len) , index ## _blk = (OCR_BLK_SIZE) \
      ; index ## _out < index ## _len \
      ; index ## _out  += index ## _blk ) { \
    Index_t index ## _end = ( index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len; \
    edt_name(index ## _out , index ## _end , __VA_ARGS__ ); \
  } // for (index)

#define BLK_PAR_FOR_0xNx1(index,len, ... ) \
  for ( Index_t index ## _out=0, index ## _len = (len) , index ## _blk = (OCR_BLK_SIZE) \
      ; index ## _out < index ## _len \
      ; index ## _out  += index ## _blk ) { \
    Index_t index ## _end = ( index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len; \
    for(Index_t index = index ## _out ; index < index ## _end ; ++index ) {

#define END_BLK_PAR_FOR(index) \
  } } // for inner and for outer ( index ) \

#define PAR_FOR_0xNx1(index,len, ... ) \
  for ( Index_t index = 0 ; index < len ; ++index ) {

#define END_PAR_FOR(index) \
  } // for( index=0 ; index < len ; ++index )

#define FINISH {
#define END_FINISH }

#else // DEFAULT is C99

#define C99_BLK_SIZE (16)

#define EDT_PAR_FOR_0xNx1(index,len, edt_name, ... ) \
  for ( Index_t index ## _out=0 , index ## _len = (len) , index ## _blk = (C99_BLK_SIZE) \
      ; index ## _out < index ## _len \
      ; index ## _out  += index ## _blk ) { \
    Index_t index ## _end = ( index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len; \
    edt_name(index ## _out , index ## _end , __VA_ARGS__ ); \
  } // for (index)

#define BLK_PAR_FOR_0xNx1(index,len, ... ) \
  for ( Index_t index ## _out=0, index ## _len = (len) , index ## _blk = (C99_BLK_SIZE) \
      ; index ## _out < index ## _len \
      ; index ## _out  += index ## _blk ) { \
    Index_t index ## _end = ( index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len; \
    for(Index_t index = index ## _out ; index < index ## _end ; ++index ) {

#define END_BLK_PAR_FOR(index) \
  } } // for inner and for outer ( index ) \

#define PAR_FOR_0xNx1(index,len, ... ) \
  for ( Index_t index = 0 ; index < len ; ++index ) {

#define END_PAR_FOR(index) \
  } // for( index=0 ; index < len ; ++index )

#define FINISH {
#define END_FINISH }

#endif // HAB_C, CILK, UPC, FSIM, ORC or C99
