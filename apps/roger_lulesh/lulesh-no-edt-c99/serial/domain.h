/************************************************************/
/* Allow for flexible data layout experiments by separating */
/* array interface from underlying implementation.          */
/************************************************************/

struct Domain {

   /******************/
   /* Implementation */
   /******************/

   /* Node-centered */

   Real_t  *m_x ;          /* coordinates */
   Real_t  *m_y ; 
   Real_t  *m_z ;

   Real_t  *m_xd ;         /* velocities */
   Real_t  *m_yd ; 
   Real_t  *m_zd ; 

   Real_t  *m_xdd ;        /* accelerations */
   Real_t  *m_ydd ;
   Real_t  *m_zdd ;

   Real_t  *m_fx ;         /* forces */
   Real_t  *m_fy ;
   Real_t  *m_fz ;

   Real_t  *m_nodalMass ;  /* mass */

   Index_t *m_symmX ;      /* symmetry plane nodesets */
   Index_t *m_symmY ;
   Index_t *m_symmZ ;

   /* Element-centered */

   Index_t *m_matElemlist ;  /* material indexset */
   Index_t *m_nodelist ;     /* elemToNode connectivity */

   Index_t *m_lxim ;         /* element connectivity across each face */
   Index_t *m_lxip ;
   Index_t *m_letam ;
   Index_t *m_letap ;
   Index_t *m_lzetam ;
   Index_t *m_lzetap ;

   Int_t   *m_elemBC ;       /* symmetry/free-surface flags for each elem face */

   Real_t  *m_dxx ;          /* principal strains -- temporary */
   Real_t  *m_dyy ;
   Real_t  *m_dzz ;

   Real_t  *m_delv_xi ;      /* velocity gradient -- temporary */
   Real_t  *m_delv_eta ;
   Real_t  *m_delv_zeta ;

   Real_t  *m_delx_xi ;      /* coordinate gradient -- temporary */
   Real_t  *m_delx_eta ;
   Real_t  *m_delx_zeta ;
   
   Real_t  *m_e ;            /* energy */

   Real_t  *m_p ;            /* pressure */
   Real_t  *m_q ;            /* q */
   Real_t  *m_ql ;           /* linear term for q */
   Real_t  *m_qq ;           /* quadratic term for q */

   Real_t  *m_v ;            /* relative volume */
   Real_t  *m_volo ;         /* reference volume */
   Real_t  *m_vnew ;         /* new relative volume -- temporary */
   Real_t  *m_delv ;         /* m_vnew - m_v */
   Real_t  *m_vdov ;         /* volume derivative over volume */

   Real_t  *m_arealg ;       /* characteristic length of an element */
   
   Real_t  *m_ss ;           /* "sound speed" */

   Real_t  *m_elemMass ;     /* mass */

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
   Real_t  m_ss4o3 ;
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
} domain ;

/* This first implementation allows for runnable code */
/* and is not meant to be optimal. Final implementation */
/* should separate declaration and allocation phases */
/* so that allocation can be scheduled in a cache conscious */
/* manner. */

   /**************/
   /* Allocation */
   /**************/

   void domain_AllocateNodalPersistent(size_t size)
   {
      if(domain.m_x != NULL)free(domain.m_x); domain.m_x = malloc(size*sizeof(Real_t)) ;
      if(domain.m_y != NULL)free(domain.m_y); domain.m_y = malloc(size*sizeof(Real_t)) ;
      if(domain.m_z != NULL)free(domain.m_z); domain.m_z = malloc(size*sizeof(Real_t)) ;

      if(domain.m_xd != NULL)free(domain.m_xd); domain.m_xd = malloc(size*sizeof(Real_t));for(Index_t i=0; i<size; ++i) domain.m_xd[i] = (Real_t)(0.0);
      if(domain.m_yd != NULL)free(domain.m_yd); domain.m_yd = malloc(size*sizeof(Real_t));for(Index_t i=0; i<size; ++i) domain.m_yd[i] = (Real_t)(0.0);
      if(domain.m_zd != NULL)free(domain.m_zd); domain.m_zd = malloc(size*sizeof(Real_t));for(Index_t i=0; i<size; ++i) domain.m_zd[i] = (Real_t)(0.0);

      if(domain.m_xdd != NULL)free(domain.m_xdd); domain.m_xdd = malloc(size*sizeof(Real_t));for(Index_t i=0; i<size; ++i) domain.m_xdd[i] = (Real_t)(0.0);
      if(domain.m_ydd != NULL)free(domain.m_ydd); domain.m_ydd = malloc(size*sizeof(Real_t));for(Index_t i=0; i<size; ++i) domain.m_ydd[i] = (Real_t)(0.0);
      if(domain.m_zdd != NULL)free(domain.m_zdd); domain.m_zdd = malloc(size*sizeof(Real_t));for(Index_t i=0; i<size; ++i) domain.m_zdd[i] = (Real_t)(0.0) ;

      if(domain.m_fx != NULL)free(domain.m_fx); domain.m_fx = malloc(size*sizeof(Real_t)) ;
      if(domain.m_fy != NULL)free(domain.m_fy); domain.m_fy = malloc(size*sizeof(Real_t)) ;
      if(domain.m_fz != NULL)free(domain.m_fz); domain.m_fz = malloc(size*sizeof(Real_t)) ;

      if(domain.m_nodalMass != NULL)free(domain.m_nodalMass); domain.m_nodalMass = malloc(size*sizeof(Real_t));for(Index_t i=0; i<size; ++i) domain.m_nodalMass[i] = (Real_t)(0.0) ;
   }

   void domain_AllocateElemPersistent(size_t size)
   {
      if(domain.m_matElemlist != NULL)free(domain.m_matElemlist); domain.m_matElemlist = malloc(size*sizeof(Index_t)) ;
      if(domain.m_nodelist != NULL)free(domain.m_nodelist); domain.m_nodelist= malloc((Index_t)(8)*size*sizeof(Index_t)) ;

      if(domain.m_lxim != NULL)free(domain.m_lxim); domain.m_lxim = malloc(size*sizeof(Index_t)) ;
      if(domain.m_lxip != NULL)free(domain.m_lxip); domain.m_lxip = malloc(size*sizeof(Index_t)) ;
      if(domain.m_letam != NULL)free(domain.m_letam); domain.m_letam = malloc(size*sizeof(Index_t)) ;
      if(domain.m_letap != NULL)free(domain.m_letap); domain.m_letap = malloc(size*sizeof(Index_t)) ;
      if(domain.m_lzetam != NULL)free(domain.m_lzetam); domain.m_lzetam = malloc(size*sizeof(Index_t)) ;
      if(domain.m_lzetap != NULL)free(domain.m_lzetap); domain.m_lzetap = malloc(size*sizeof(Index_t)) ;

      if(domain.m_elemBC != NULL)free(domain.m_elemBC); domain.m_elemBC = malloc(size*sizeof(Int_t)) ;

      if(domain.m_e != NULL)free(domain.m_e); domain.m_e = malloc(size*sizeof(Real_t));for(Index_t i=0; i<size; ++i) domain.m_e[i] = (Real_t)(0.0);

      if(domain.m_p != NULL)free(domain.m_p); domain.m_p = malloc(size*sizeof(Real_t));for(Index_t i=0; i<size; ++i) domain.m_p[i] = (Real_t)(0.0);
      if(domain.m_q != NULL)free(domain.m_q); domain.m_q = malloc(size*sizeof(Real_t)) ;
      if(domain.m_ql != NULL)free(domain.m_ql); domain.m_ql = malloc(size*sizeof(Real_t)) ;
      if(domain.m_qq != NULL)free(domain.m_qq); domain.m_qq = malloc(size*sizeof(Real_t)) ;

      if(domain.m_v != NULL)free(domain.m_v); domain.m_v = malloc(size*sizeof(Real_t));for(Index_t i=0; i<size; ++i) domain.m_v[i] = (Real_t)(1.0);
      if(domain.m_volo != NULL)free(domain.m_volo); domain.m_volo = malloc(size*sizeof(Real_t)) ;
      if(domain.m_delv != NULL)free(domain.m_delv); domain.m_delv = malloc(size*sizeof(Real_t)) ;
      if(domain.m_vdov != NULL)free(domain.m_vdov); domain.m_vdov = malloc(size*sizeof(Real_t)) ;

      if(domain.m_arealg != NULL)free(domain.m_arealg); domain.m_arealg = malloc(size*sizeof(Real_t)) ;
   
      if(domain.m_ss != NULL)free(domain.m_ss); domain.m_ss = malloc(size*sizeof(Real_t)) ;

      if(domain.m_elemMass != NULL)free(domain.m_elemMass); domain.m_elemMass = malloc(size*sizeof(Real_t)) ;
   }

   /* Temporaries should not be initialized in bulk but */
   /* this is a runnable placeholder for now */
   void domain_AllocateElemTemporary(size_t size)
   {
      if(domain.m_dxx != NULL)free(domain.m_dxx); domain.m_dxx = malloc(size*sizeof(Real_t)) ;
      if(domain.m_dyy != NULL)free(domain.m_dyy); domain.m_dyy = malloc(size*sizeof(Real_t)) ;
      if(domain.m_dzz != NULL)free(domain.m_dzz); domain.m_dzz = malloc(size*sizeof(Real_t)) ;

      if(domain.m_delv_xi != NULL)free(domain.m_delv_xi); domain.m_delv_xi = malloc(size*sizeof(Real_t)) ;
      if(domain.m_delv_eta != NULL)free(domain.m_delv_eta); domain.m_delv_eta = malloc(size*sizeof(Real_t)) ;
      if(domain.m_delv_zeta != NULL)free(domain.m_delv_zeta); domain.m_delv_zeta = malloc(size*sizeof(Real_t)) ;

      if(domain.m_delx_xi != NULL)free(domain.m_delx_xi); domain.m_delx_xi = malloc(size*sizeof(Real_t)) ;
      if(domain.m_delx_eta != NULL)free(domain.m_delx_eta); domain.m_delx_eta = malloc(size*sizeof(Real_t)) ;
      if(domain.m_delx_zeta != NULL)free(domain.m_delx_zeta); domain.m_delx_zeta = malloc(size*sizeof(Real_t)) ;

      if(domain.m_vnew != NULL)free(domain.m_vnew); domain.m_vnew = malloc(size*sizeof(Real_t)) ;
   }

   void domain_AllocateNodesets(size_t size)
   {
      if(domain.m_symmX != NULL)free(domain.m_symmX); domain.m_symmX = malloc(size*sizeof(Index_t)) ;
      if(domain.m_symmY != NULL)free(domain.m_symmY); domain.m_symmY = malloc(size*sizeof(Index_t)) ;
      if(domain.m_symmZ != NULL)free(domain.m_symmZ); domain.m_symmZ = malloc(size*sizeof(Index_t)) ;
   }
   
   /**********/
   /* Access */
   /**********/

   /* Node-centered */

#define domain_x(idx)           domain.m_x[(Index_t)(idx)]
#define domain_y(idx)           domain.m_y[(Index_t)(idx)]
#define domain_z(idx)           domain.m_z[(Index_t)(idx)]

#define domain_xd(idx)          domain.m_xd[(Index_t)(idx)]
#define domain_yd(idx)          domain.m_yd[(Index_t)(idx)]
#define domain_zd(idx)          domain.m_zd[(Index_t)(idx)]

#define domain_xdd(idx)         domain.m_xdd[(Index_t)(idx)]
#define domain_ydd(idx)         domain.m_ydd[(Index_t)(idx)]
#define domain_zdd(idx)         domain.m_zdd[(Index_t)(idx)]

#define domain_fx(idx)          domain.m_fx[(Index_t)(idx)]
#define domain_fy(idx)          domain.m_fy[(Index_t)(idx)]
#define domain_fz(idx)          domain.m_fz[(Index_t)(idx)]

#define domain_nodalMass(idx)   domain.m_nodalMass[(Index_t)(idx)]

#define domain_symmX(idx)       domain.m_symmX[(Index_t)(idx)]
#define domain_symmY(idx)       domain.m_symmY[(Index_t)(idx)]
#define domain_symmZ(idx)       domain.m_symmZ[(Index_t)(idx)]

   /* Element-centered */

#define domain_matElemlist(idx) domain.m_matElemlist[(Index_t)(idx)]
#define domain_nodelist(idx)    domain.m_nodelist[(Index_t)(8)*(Index_t)(idx)]
#define domain_nodelist_ref(idx) ((Index_t *)&domain.m_nodelist[(Index_t)(8)*(Index_t)(idx)])

#define domain_lxim(idx)        domain.m_lxim[(Index_t)(idx)]
#define domain_lxip(idx)        domain.m_lxip[(Index_t)(idx)]
#define domain_letam(idx)       domain.m_letam[(Index_t)(idx)]
#define domain_letap(idx)       domain.m_letap[(Index_t)(idx)]
#define domain_lzetam(idx)      domain.m_lzetam[(Index_t)(idx)]
#define domain_lzetap(idx)      domain.m_lzetap[(Index_t)(idx)]
 
#define domain_elemBC(idx)      domain.m_elemBC[(Index_t)(idx)]

#define domain_dxx(idx)         domain.m_dxx[idx]
#define domain_dyy(idx)         domain.m_dyy[idx]
#define domain_dzz(idx)         domain.m_dzz[idx]

#define domain_delv_xi(idx)     domain.m_delv_xi[(Index_t)(idx)]
#define domain_delv_eta(idx)    domain.m_delv_eta[(Index_t)(idx)]
#define domain_delv_zeta(idx)   domain.m_delv_zeta[(Index_t)(idx)]

#define domain_delx_xi(idx)     domain.m_delx_xi[(Index_t)(idx)]
#define domain_delx_eta(idx)    domain.m_delx_eta[(Index_t)(idx)]
#define domain_delx_zeta(idx)   domain.m_delx_zeta[(Index_t)(idx)]

#define domain_e(idx)           domain.m_e[(Index_t)(idx)]

#define domain_p(idx)           domain.m_p[(Index_t)(idx)]
#define domain_q(idx)           domain.m_q[(Index_t)(idx)]
#define domain_ql(idx)          domain.m_ql[(Index_t)(idx)]
#define domain_qq(idx)          domain.m_qq[(Index_t)(idx)]

#define domain_v(idx)           domain.m_v[(Index_t)(idx)]
#define domain_volo(idx)        domain.m_volo[(Index_t)(idx)]
#define domain_vnew(idx)        domain.m_vnew[(Index_t)(idx)]
#define domain_delv(idx)        domain.m_delv[(Index_t)(idx)]
#define domain_vdov(idx)        domain.m_vdov[(Index_t)(idx)]

#define domain_arealg(idx)      domain.m_arealg[(Index_t)(idx)]

#define domain_ss(idx)          domain.m_ss[(Index_t)(idx)]

#define domain_elemMass(idx)    domain.m_elemMass[(Index_t)(idx)]

   /* Params */

#define domain_dtfixed()              domain.m_dtfixed
#define domain_time()                 domain.m_time
#define domain_deltatime()            domain.m_deltatime
#define domain_deltatimemultlb()      domain.m_deltatimemultlb
#define domain_deltatimemultub()      domain.m_deltatimemultub
#define domain_stoptime()             domain.m_stoptime

#define domain_u_cut()                domain.m_u_cut
#define domain_hgcoef()               domain.m_hgcoef
#define domain_qstop()                domain.m_qstop
#define domain_monoq_max_slope()      domain.m_monoq_max_slope
#define domain_monoq_limiter_mult()   domain.m_monoq_limiter_mult
#define domain_e_cut()                domain.m_e_cut
#define domain_p_cut()                domain.m_p_cut
#define domain_ss4o3()                domain.m_ss4o3
#define domain_q_cut()                domain.m_q_cut
#define domain_v_cut()                domain.m_v_cut
#define domain_qlc_monoq()            domain.m_qlc_monoq
#define domain_qqc_monoq()            domain.m_qqc_monoq
#define domain_qqc()                  domain.m_qqc
#define domain_eosvmax()              domain.m_eosvmax
#define domain_eosvmin()              domain.m_eosvmin
#define domain_pmin()                 domain.m_pmin
#define domain_emin()                 domain.m_emin
#define domain_dvovmax()              domain.m_dvovmax
#define domain_refdens()              domain.m_refdens

#define domain_dtcourant()            domain.m_dtcourant
#define domain_dthydro()              domain.m_dthydro
#define domain_dtmax()                domain.m_dtmax

#define domain_cycle()                domain.m_cycle

#define domain_sizeX()                domain.m_sizeX
#define domain_sizeY()                domain.m_sizeY
#define domain_sizeZ()                domain.m_sizeZ
#define domain_numElem()              domain.m_numElem
#define domain_numNode()              domain.m_numNode

static inline   Int_t   *Allocate_Int_t(size_t size) { return   (Int_t *)malloc(size*  sizeof(Int_t)); }
static inline  Real_t  *Allocate_Real_t(size_t size) { return  (Real_t *)malloc(size* sizeof(Real_t)); }
static inline Index_t *Allocate_Index_t(size_t size) { return (Index_t *)malloc(size*sizeof(Index_t)); }

void   Release_Int_t(  Int_t **ptr) { if(*ptr != NULL) { free(*ptr); *ptr = NULL; } }
void  Release_Real_t( Real_t **ptr) { if(*ptr != NULL) { free(*ptr); *ptr = NULL; } }
void Release_Index_t(Index_t **ptr) { if(*ptr != NULL) { free(*ptr); *ptr = NULL; } }

#define   cast_Int_t(arg) (  (Int_t)(arg))
#define  cast_Real_t(arg) ( (Real_t)(arg))
#define cast_Index_t(arg) ((Index_t)(arg))
