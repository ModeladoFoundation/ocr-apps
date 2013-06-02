// use this instead of the naked 3 and 8 constants

#define EIGHT ((int)8)
#define THREE ((int)3)

/************************************************************/
/* Allow for flexible data layout experiments by separating */
/* array interface from underlying implementation.          */
/************************************************************/

struct Domain {

   /******************/
   /* Implementation */
   /******************/

   /* Node-centered */

   shared double  *m_x ;          /* coordinates */
   shared double  *m_y ; 
   shared double  *m_z ;

   shared double  *m_xd ;         /* velocities */
   shared double  *m_yd ; 
   shared double  *m_zd ; 

   shared double  *m_xdd ;        /* accelerations */
   shared double  *m_ydd ;
   shared double  *m_zdd ;

   shared double  *m_fx ;         /* forces */
   shared double  *m_fy ;
   shared double  *m_fz ;

   shared double  *m_nodalMass ;  /* mass */

   shared int *m_symmX ;      /* symmetry plane nodesets */
   shared int *m_symmY ;
   shared int *m_symmZ ;

   /* Element-centered */

   shared int *m_matElemlist ;  /* material indexset */
   shared int *m_nodelist ;     /* elemToNode connectivity */

   shared int *m_lxim ;         /* element connectivity across each face */
   shared int *m_lxip ;
   shared int *m_letam ;
   shared int *m_letap ;
   shared int *m_lzetam ;
   shared int *m_lzetap ;

   shared int   *m_elemBC ;       /* symmetry/free-surface flags for each elem face */

   shared double  *m_dxx ;          /* principal strains -- temporary */
   shared double  *m_dyy ;
   shared double  *m_dzz ;

   shared double  *m_delv_xi ;      /* velocity gradient -- temporary */
   shared double  *m_delv_eta ;
   shared double  *m_delv_zeta ;

   shared double  *m_delx_xi ;      /* coordinate gradient -- temporary */
   shared double  *m_delx_eta ;
   shared double  *m_delx_zeta ;
   
   shared double  *m_e ;            /* energy */

   shared double  *m_p ;            /* pressure */
   shared double  *m_q ;            /* q */
   shared double  *m_ql ;           /* linear term for q */
   shared double  *m_qq ;           /* quadratic term for q */

   shared double  *m_v ;            /* relative volume */
   shared double  *m_volo ;         /* reference volume */
   shared double  *m_vnew ;         /* new relative volume -- temporary */
   shared double  *m_delv ;         /* m_vnew - m_v */
   shared double  *m_vdov ;         /* volume derivative over volume */

   shared double  *m_arealg ;       /* characteristic length of an element */
   
   shared double  *m_ss ;           /* "sound speed" */

   shared double  *m_elemMass ;     /* mass */

   /* Parameters */

   double  m_dtfixed ;           /* fixed time increment */
   double  m_time ;              /* current time */
   double  m_deltatime ;         /* variable time increment */
   double  m_deltatimemultlb ;
   double  m_deltatimemultub ;
   double  m_stoptime ;          /* end time for simulation */

   double  m_u_cut ;             /* velocity tolerance */
   double  m_hgcoef ;            /* hourglass control */
   double  m_qstop ;             /* excessive q indicator */
   double  m_monoq_max_slope ;
   double  m_monoq_limiter_mult ;
   double  m_e_cut ;             /* energy tolerance */
   double  m_p_cut ;             /* pressure tolerance */
   double  m_ss4o3 ;
   double  m_q_cut ;             /* q tolerance */
   double  m_v_cut ;             /* relative volume tolerance */
   double  m_qlc_monoq ;         /* linear term coef for q */
   double  m_qqc_monoq ;         /* quadratic term coef for q */
   double  m_qqc ;
   double  m_eosvmax ;
   double  m_eosvmin ;
   double  m_pmin ;              /* pressure floor */
   double  m_emin ;              /* energy floor */
   double  m_dvovmax ;           /* maximum allowable volume change */
   double  m_refdens ;           /* reference density */

   double  m_dtcourant ;         /* courant constraint */
   double  m_dthydro ;           /* volume change constraint */
   double  m_dtmax ;             /* maximum allowable time increment */

   int   m_cycle ;             /* iteration count for simulation */

   int m_sizeX ;           /* X,Y,Z extent of this block */
   int m_sizeY ;
   int m_sizeZ ;

   int m_numElem ;         /* Elements/Nodes in this domain */
   int m_numNode ;
} /* domain */ ;

shared struct Domain domain;

/* This first implementation allows for runnable code */
/* and is not meant to be optimal. Final implementation */
/* should separate declaration and allocation phases */
/* so that allocation can be scheduled in a cache conscious */
/* manner. */

   /**************/
   /* Allocation */
   /**************/

   void domain_AllocateNodalPersistent(size_t hcSize)
   {
    if(MYTHREAD==0) {
      if(domain.m_x != NULL)upc_free(domain.m_x); domain.m_x = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_y != NULL)upc_free(domain.m_y); domain.m_y = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_z != NULL)upc_free(domain.m_z); domain.m_z = upc_global_alloc(hcSize,sizeof(double)) ;

      if(domain.m_xd != NULL)upc_free(domain.m_xd); domain.m_xd = upc_global_alloc(hcSize,sizeof(double));for(int i=0; i<hcSize; ++i) domain.m_xd[i] = (double)(0.0);
      if(domain.m_yd != NULL)upc_free(domain.m_yd); domain.m_yd = upc_global_alloc(hcSize,sizeof(double));for(int i=0; i<hcSize; ++i) domain.m_yd[i] = (double)(0.0);
      if(domain.m_zd != NULL)upc_free(domain.m_zd); domain.m_zd = upc_global_alloc(hcSize,sizeof(double));for(int i=0; i<hcSize; ++i) domain.m_zd[i] = (double)(0.0);

      if(domain.m_xdd != NULL)upc_free(domain.m_xdd); domain.m_xdd = upc_global_alloc(hcSize,sizeof(double));for(int i=0; i<hcSize; ++i) domain.m_xdd[i] = (double)(0.0);
      if(domain.m_ydd != NULL)upc_free(domain.m_ydd); domain.m_ydd = upc_global_alloc(hcSize,sizeof(double));for(int i=0; i<hcSize; ++i) domain.m_ydd[i] = (double)(0.0);
      if(domain.m_zdd != NULL)upc_free(domain.m_zdd); domain.m_zdd = upc_global_alloc(hcSize,sizeof(double));for(int i=0; i<hcSize; ++i) domain.m_zdd[i] = (double)(0.0) ;

      if(domain.m_fx != NULL)upc_free(domain.m_fx); domain.m_fx = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_fy != NULL)upc_free(domain.m_fy); domain.m_fy = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_fz != NULL)upc_free(domain.m_fz); domain.m_fz = upc_global_alloc(hcSize,sizeof(double)) ;

      if(domain.m_nodalMass != NULL)upc_free(domain.m_nodalMass); domain.m_nodalMass = upc_global_alloc(hcSize,sizeof(double));for(int i=0; i<hcSize; ++i) domain.m_nodalMass[i] = (double)(0.0) ;
    }
    upc_barrier;
   }

   void domain_AllocateElemPersistent(size_t hcSize)
   {
    if(MYTHREAD==0) {
      if(domain.m_matElemlist != NULL)upc_free(domain.m_matElemlist); domain.m_matElemlist = upc_global_alloc(hcSize,sizeof(int)) ;
      if(domain.m_nodelist != NULL)upc_free(domain.m_nodelist); domain.m_nodelist= upc_global_alloc(hcSize,EIGHT*sizeof(int)) ;

      if(domain.m_lxim != NULL)upc_free(domain.m_lxim); domain.m_lxim = upc_global_alloc(hcSize,sizeof(int)) ;
      if(domain.m_lxip != NULL)upc_free(domain.m_lxip); domain.m_lxip = upc_global_alloc(hcSize,sizeof(int)) ;
      if(domain.m_letam != NULL)upc_free(domain.m_letam); domain.m_letam = upc_global_alloc(hcSize,sizeof(int)) ;
      if(domain.m_letap != NULL)upc_free(domain.m_letap); domain.m_letap = upc_global_alloc(hcSize,sizeof(int)) ;
      if(domain.m_lzetam != NULL)upc_free(domain.m_lzetam); domain.m_lzetam = upc_global_alloc(hcSize,sizeof(int)) ;
      if(domain.m_lzetap != NULL)upc_free(domain.m_lzetap); domain.m_lzetap = upc_global_alloc(hcSize,sizeof(int)) ;

      if(domain.m_elemBC != NULL)upc_free(domain.m_elemBC); domain.m_elemBC = upc_global_alloc(hcSize,sizeof(int)) ;

      if(domain.m_e != NULL)upc_free(domain.m_e); domain.m_e = upc_global_alloc(hcSize,sizeof(double));for(int i=0; i<hcSize; ++i) domain.m_e[i] = (double)(0.0);

      if(domain.m_p != NULL)upc_free(domain.m_p); domain.m_p = upc_global_alloc(hcSize,sizeof(double));for(int i=0; i<hcSize; ++i) domain.m_p[i] = (double)(0.0);
      if(domain.m_q != NULL)upc_free(domain.m_q); domain.m_q = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_ql != NULL)upc_free(domain.m_ql); domain.m_ql = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_qq != NULL)upc_free(domain.m_qq); domain.m_qq = upc_global_alloc(hcSize,sizeof(double)) ;

      if(domain.m_v != NULL)upc_free(domain.m_v); domain.m_v = upc_global_alloc(hcSize,sizeof(double));for(int i=0; i<hcSize; ++i) domain.m_v[i] = (double)(1.0);
      if(domain.m_volo != NULL)upc_free(domain.m_volo); domain.m_volo = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_delv != NULL)upc_free(domain.m_delv); domain.m_delv = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_vdov != NULL)upc_free(domain.m_vdov); domain.m_vdov = upc_global_alloc(hcSize,sizeof(double)) ;

      if(domain.m_arealg != NULL)upc_free(domain.m_arealg); domain.m_arealg = upc_global_alloc(hcSize,sizeof(double)) ;
   
      if(domain.m_ss != NULL)upc_free(domain.m_ss); domain.m_ss = upc_global_alloc(hcSize,sizeof(double)) ;

      if(domain.m_elemMass != NULL)upc_free(domain.m_elemMass); domain.m_elemMass = upc_global_alloc(hcSize,sizeof(double)) ;
    }
    upc_barrier;
   }

   /* Temporaries should not be initialized in bulk but */
   /* this is a runnable placeholder for now */
   void domain_AllocateElemTemporary(size_t hcSize)
   {
    if(MYTHREAD==0) {
      if(domain.m_dxx != NULL)upc_free(domain.m_dxx); domain.m_dxx = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_dyy != NULL)upc_free(domain.m_dyy); domain.m_dyy = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_dzz != NULL)upc_free(domain.m_dzz); domain.m_dzz = upc_global_alloc(hcSize,sizeof(double)) ;

      if(domain.m_delv_xi != NULL)upc_free(domain.m_delv_xi); domain.m_delv_xi = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_delv_eta != NULL)upc_free(domain.m_delv_eta); domain.m_delv_eta = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_delv_zeta != NULL)upc_free(domain.m_delv_zeta); domain.m_delv_zeta = upc_global_alloc(hcSize,sizeof(double)) ;

      if(domain.m_delx_xi != NULL)upc_free(domain.m_delx_xi); domain.m_delx_xi = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_delx_eta != NULL)upc_free(domain.m_delx_eta); domain.m_delx_eta = upc_global_alloc(hcSize,sizeof(double)) ;
      if(domain.m_delx_zeta != NULL)upc_free(domain.m_delx_zeta); domain.m_delx_zeta = upc_global_alloc(hcSize,sizeof(double)) ;

      if(domain.m_vnew != NULL)upc_free(domain.m_vnew); domain.m_vnew = upc_global_alloc(hcSize,sizeof(double)) ;
    }
    upc_barrier;
   }

   void domain_AllocateNodesets(size_t hcSize)
   {
    if(MYTHREAD==0) {
      if(domain.m_symmX != NULL)upc_free(domain.m_symmX); domain.m_symmX = upc_global_alloc(hcSize,sizeof(int)) ;
      if(domain.m_symmY != NULL)upc_free(domain.m_symmY); domain.m_symmY = upc_global_alloc(hcSize,sizeof(int)) ;
      if(domain.m_symmZ != NULL)upc_free(domain.m_symmZ); domain.m_symmZ = upc_global_alloc(hcSize,sizeof(int)) ;
    }
    upc_barrier;
   }
   
   /**********/
   /* Access */
   /**********/

   /* Node-centered */

#define domain_x(idx)           domain.m_x[(int)(idx)]
#define domain_y(idx)           domain.m_y[(int)(idx)]
#define domain_z(idx)           domain.m_z[(int)(idx)]

#define domain_xd(idx)          domain.m_xd[(int)(idx)]
#define domain_yd(idx)          domain.m_yd[(int)(idx)]
#define domain_zd(idx)          domain.m_zd[(int)(idx)]

#define domain_xdd(idx)         domain.m_xdd[(int)(idx)]
#define domain_ydd(idx)         domain.m_ydd[(int)(idx)]
#define domain_zdd(idx)         domain.m_zdd[(int)(idx)]

#define domain_fx(idx)          domain.m_fx[(int)(idx)]
#define domain_fy(idx)          domain.m_fy[(int)(idx)]
#define domain_fz(idx)          domain.m_fz[(int)(idx)]

#define domain_nodalMass(idx)   domain.m_nodalMass[(int)(idx)]

#define domain_symmX(idx)       domain.m_symmX[(int)(idx)]
#define domain_symmY(idx)       domain.m_symmY[(int)(idx)]
#define domain_symmZ(idx)       domain.m_symmZ[(int)(idx)]

   /* Element-centered */

#define domain_matElemlist(idx) domain.m_matElemlist[(int)(idx)]
#define domain_nodelist(idx)    domain.m_nodelist[EIGHT*(int)(idx)]
#define domain_nodelist_ref(idx) ((int *)&domain.m_nodelist[EIGHT*(int)(idx)])

#define domain_lxim(idx)        domain.m_lxim[(int)(idx)]
#define domain_lxip(idx)        domain.m_lxip[(int)(idx)]
#define domain_letam(idx)       domain.m_letam[(int)(idx)]
#define domain_letap(idx)       domain.m_letap[(int)(idx)]
#define domain_lzetam(idx)      domain.m_lzetam[(int)(idx)]
#define domain_lzetap(idx)      domain.m_lzetap[(int)(idx)]
 
#define domain_elemBC(idx)      domain.m_elemBC[(int)(idx)]

#define domain_dxx(idx)         domain.m_dxx[idx]
#define domain_dyy(idx)         domain.m_dyy[idx]
#define domain_dzz(idx)         domain.m_dzz[idx]

#define domain_delv_xi(idx)     domain.m_delv_xi[(int)(idx)]
#define domain_delv_eta(idx)    domain.m_delv_eta[(int)(idx)]
#define domain_delv_zeta(idx)   domain.m_delv_zeta[(int)(idx)]

#define domain_delx_xi(idx)     domain.m_delx_xi[(int)(idx)]
#define domain_delx_eta(idx)    domain.m_delx_eta[(int)(idx)]
#define domain_delx_zeta(idx)   domain.m_delx_zeta[(int)(idx)]

#define domain_e(idx)           domain.m_e[(int)(idx)]

#define domain_p(idx)           domain.m_p[(int)(idx)]
#define domain_q(idx)           domain.m_q[(int)(idx)]
#define domain_ql(idx)          domain.m_ql[(int)(idx)]
#define domain_qq(idx)          domain.m_qq[(int)(idx)]

#define domain_v(idx)           domain.m_v[(int)(idx)]
#define domain_volo(idx)        domain.m_volo[(int)(idx)]
#define domain_vnew(idx)        domain.m_vnew[(int)(idx)]
#define domain_delv(idx)        domain.m_delv[(int)(idx)]
#define domain_vdov(idx)        domain.m_vdov[(int)(idx)]

#define domain_arealg(idx)      domain.m_arealg[(int)(idx)]

#define domain_ss(idx)          domain.m_ss[(int)(idx)]

#define domain_elemMass(idx)    domain.m_elemMass[(int)(idx)]

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

#if 0
static inline   Int_t   *Allocate_Int_t(size_t hcSize) { return   (Int_t *)malloc(hcSize*  sizeof(Int_t)); }
#endif
static inline  double  *Allocate_double(size_t hcSize) { return  (double *)malloc(hcSize* sizeof(double)); }
static inline int *Allocate_int(size_t hcSize) { return (int *)malloc(hcSize*sizeof(int)); }

#if 0
void   Release_Ind_t(  Ind_t *ptr) { if(ptr != NULL) { free(ptr); } }
#endif
void  Release_double( double *ptr) { if(ptr != NULL) { free(ptr); } }
void Release_int(int *ptr) { if(ptr != NULL) { free(ptr); } }

#if 0
#define   cast_Ind_t(arg) (  (Ind_t)(arg))
#endif
#define  cast_double(arg) ( (double)(arg))
#define cast_int(arg) ((int)(arg))
