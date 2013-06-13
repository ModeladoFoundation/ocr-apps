/* RAG -- moved initialization code here */

/* This first implementation allows for runnable code */
/* and is not meant to be optimal. Final implementation */
/* should separate declaration and allocation phases */
/* so that allocation can be scheduled in a cache conscious */
/* manner. */

   /**************/
   /* Allocation */
   /**************/

static INLINE
void domain_AllocateNodalPersistent(size_t hcSize) {
TRACE1("Allocate EP entry");
#if defined(FSIM)
  xe_printf("rag: domain              %16.16lx\n",domain);
  xe_printf("rag: domain->m_x         %16.16lx\n",domain->m_x);
  xe_printf("rag: domain->m_y         %16.16lx\n",domain->m_y);
  xe_printf("rag: domain->m_z         %16.16lx\n",domain->m_z);
  xe_printf("rag: domain->m_xd        %16.16lx\n",domain->m_xd);
  xe_printf("rag: domain->m_yd        %16.16lx\n",domain->m_yd);
  xe_printf("rag: domain->m_zd        %16.16lx\n",domain->m_zd);
  xe_printf("rag: domain->m_xdd       %16.16lx\n",domain->m_xdd);
  xe_printf("rag: domain->m_ydd       %16.16lx\n",domain->m_ydd);
  xe_printf("rag: domain->m_zdd       %16.16lx\n",domain->m_zdd);
  xe_printf("rag: domain->m_fx        %16.16lx\n",domain->m_fx);
  xe_printf("rag: domain->m_fy        %16.16lx\n",domain->m_fy);
  xe_printf("rag: domain->m_fz        %16.16lx\n",domain->m_fz);
  xe_printf("rag: domain->m_nodalMass %16.16lx\n",domain->m_nodalMass);
#endif // FSIM or OCR
TRACE1("Allocate NP m_(x|y|z)");
   if(domain->m_x != NULL)DRAM_FREE(domain->m_x); domain->m_x = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
   if(domain->m_y != NULL)DRAM_FREE(domain->m_y); domain->m_y = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
   if(domain->m_z != NULL)DRAM_FREE(domain->m_z); domain->m_z = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
TRACE1("Allocate NP m_(x|y|z)d");
   if(domain->m_xd != NULL)DRAM_FREE(domain->m_xd); domain->m_xd = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t));
   if(domain->m_yd != NULL)DRAM_FREE(domain->m_yd); domain->m_yd = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t));
   if(domain->m_zd != NULL)DRAM_FREE(domain->m_zd); domain->m_zd = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t));
TRACE1("Allocate NP m_(x|y|z)dd");
   if(domain->m_xdd != NULL)DRAM_FREE(domain->m_xdd); domain->m_xdd = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t));
   if(domain->m_ydd != NULL)DRAM_FREE(domain->m_ydd); domain->m_ydd = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t));
   if(domain->m_zdd != NULL)DRAM_FREE(domain->m_zdd); domain->m_zdd = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t));
TRACE1("Allocate NP m_f(x|y|z)");
   if(domain->m_fx != NULL)DRAM_FREE(domain->m_fx); domain->m_fx = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
   if(domain->m_fy != NULL)DRAM_FREE(domain->m_fy); domain->m_fy = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
   if(domain->m_fz != NULL)DRAM_FREE(domain->m_fz); domain->m_fz = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
TRACE1("Allocate NP m_nodalMass(x|y|z)");
   if(domain->m_nodalMass != NULL)DRAM_FREE(domain->m_nodalMass); domain->m_nodalMass = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t));
TRACE1("Allocate NP zero m_(x|y|z)d, m_(x|y|z)dd and m_nodalMass(x|y|z)");
#if defined(FSIM)
  xe_printf("rag: domain %16.16lx\n",domain);
  xe_printf("rag: domain->m_xd %16.16lx\n",domain->m_xd);
  xe_printf("rag: domain->m_xy %16.16lx\n",domain->m_yd);
  xe_printf("rag: domain->m_xz %16.16lx\n",domain->m_zd);
  xe_printf("rag: domain->m_xd %16.16lx\n",domain->m_xdd);
  xe_printf("rag: domain->m_xy %16.16lx\n",domain->m_ydd);
  xe_printf("rag: domain->m_xz %16.16lx\n",domain->m_zdd);
  xe_printf("rag: domain->m_nodalMassv %16.16lx\n",domain->m_nodalMass);
  xe_printf("rag: loop (0;<%16.16lx;1)\n",hcSize);
#endif // FSIM or OCR
  FINISH
    EDT_PAR_FOR_0xNx1(i,hcSize,domain_AllocateNodalPersistent_edt_1,domain);
  END_FINISH
#if defined(FSIM) 
  xe_printf("rag: domain              %16.16lx\n",domain);
  xe_printf("rag: domain->m_x         %16.16lx\n",domain->m_x);
  xe_printf("rag: domain->m_y         %16.16lx\n",domain->m_y);
  xe_printf("rag: domain->m_z         %16.16lx\n",domain->m_z);
  xe_printf("rag: domain->m_xd        %16.16lx\n",domain->m_xd);
  xe_printf("rag: domain->m_yd        %16.16lx\n",domain->m_yd);
  xe_printf("rag: domain->m_zd        %16.16lx\n",domain->m_zd);
  xe_printf("rag: domain->m_xdd       %16.16lx\n",domain->m_xdd);
  xe_printf("rag: domain->m_ydd       %16.16lx\n",domain->m_ydd);
  xe_printf("rag: domain->m_zdd       %16.16lx\n",domain->m_zdd);
  xe_printf("rag: domain->m_fx        %16.16lx\n",domain->m_fx);
  xe_printf("rag: domain->m_fy        %16.16lx\n",domain->m_fy);
  xe_printf("rag: domain->m_fz        %16.16lx\n",domain->m_fz);
  xe_printf("rag: domain->m_nodalMass %16.16lx\n",domain->m_nodalMass);
#endif // FSIM or OCR
TRACE1("Allocate NP returns");
} // domain_AllocateNodalPersistent()

static INLINE
void domain_AllocateElemPersistent(size_t hcSize) {
TRACE1("Allocate EP entry");
#if defined(FSIM) 
  xe_printf("rag: domain                %16.16lx\n",domain);
  xe_printf("rag: domain->m_matElemlist %16.16lx\n",domain->m_matElemlist);
  xe_printf("rag: domain->m_nodelist    %16.16lx\n",domain->m_nodelist);
  xe_printf("rag: domain->m_lxim        %16.16lx\n",domain->m_lxim);
  xe_printf("rag: domain->m_lxip        %16.16lx\n",domain->m_lxip);
  xe_printf("rag: domain->m_letam       %16.16lx\n",domain->m_letam);
  xe_printf("rag: domain->m_letap       %16.16lx\n",domain->m_letap);
  xe_printf("rag: domain->m_lzetam      %16.16lx\n",domain->m_lzetam);
  xe_printf("rag: domain->m_lzetap      %16.16lx\n",domain->m_lzetap);
  xe_printf("rag: domain->m_elemBC      %16.16lx\n",domain->m_elemBC);
  xe_printf("rag: domain->m_e           %16.16lx\n",domain->m_e);
  xe_printf("rag: domain->m_p           %16.16lx\n",domain->m_p);
  xe_printf("rag: domain->m_q           %16.16lx\n",domain->m_q);
  xe_printf("rag: domain->m_ql          %16.16lx\n",domain->m_ql);
  xe_printf("rag: domain->m_qq          %16.16lx\n",domain->m_qq);
  xe_printf("rag: domain->m_v           %16.16lx\n",domain->m_v);
  xe_printf("rag: domain->m_volo        %16.16lx\n",domain->m_volo);
  xe_printf("rag: domain->m_delv        %16.16lx\n",domain->m_delv);
  xe_printf("rag: domain->m_vdov        %16.16lx\n",domain->m_vdov);
  xe_printf("rag: domain->m_arealg      %16.16lx\n",domain->m_arealg);
  xe_printf("rag: domain->m_ss          %16.16lx\n",domain->m_ss);
  xe_printf("rag: domain->m_elemMass    %16.16lx\n",domain->m_elemMass);
#endif // FSIM or OCR
TRACE1("Allocate EP m_matElemlist and m_nodelist");
   if(domain->m_matElemlist != NULL)DRAM_FREE(domain->m_matElemlist); domain->m_matElemlist = (SHARED Index_t *)DRAM_MALLOC(hcSize,sizeof(Index_t)) ;
   if(domain->m_nodelist != NULL)DRAM_FREE(domain->m_nodelist); domain->m_nodelist= (SHARED Index_t *)DRAM_MALLOC(hcSize,EIGHT*sizeof(Index_t)) ;
TRACE1("Allocate EP m_lixm,m_lixp,m_letam,m_letap,m_lzetam,m_lzetap");
   if(domain->m_lxim != NULL)DRAM_FREE(domain->m_lxim); domain->m_lxim = (SHARED Index_t *)DRAM_MALLOC(hcSize,sizeof(Index_t)) ;
   if(domain->m_lxip != NULL)DRAM_FREE(domain->m_lxip); domain->m_lxip = (SHARED Index_t *)DRAM_MALLOC(hcSize,sizeof(Index_t)) ;
   if(domain->m_letam != NULL)DRAM_FREE(domain->m_letam); domain->m_letam = (SHARED Index_t *)DRAM_MALLOC(hcSize,sizeof(Index_t)) ;
   if(domain->m_letap != NULL)DRAM_FREE(domain->m_letap); domain->m_letap = (SHARED Index_t *)DRAM_MALLOC(hcSize,sizeof(Index_t)) ;
   if(domain->m_lzetam != NULL)DRAM_FREE(domain->m_lzetam); domain->m_lzetam = (SHARED Index_t *)DRAM_MALLOC(hcSize,sizeof(Index_t)) ;
   if(domain->m_lzetap != NULL)DRAM_FREE(domain->m_lzetap); domain->m_lzetap = (SHARED Index_t *)DRAM_MALLOC(hcSize,sizeof(Index_t)) ;
TRACE1("Allocate EP m_elemBC");
   if(domain->m_elemBC != NULL)DRAM_FREE(domain->m_elemBC); domain->m_elemBC = (SHARED Int_t *)DRAM_MALLOC(hcSize,sizeof(Int_t)) ;
TRACE1("Allocate EP m_e,m_p,m_q,m_ql,m_qq");
   if(domain->m_e != NULL)DRAM_FREE(domain->m_e); domain->m_e = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t));
   if(domain->m_p != NULL)DRAM_FREE(domain->m_p); domain->m_p = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t));
   if(domain->m_q != NULL)DRAM_FREE(domain->m_q); domain->m_q = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
   if(domain->m_ql != NULL)DRAM_FREE(domain->m_ql); domain->m_ql = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
   if(domain->m_qq != NULL)DRAM_FREE(domain->m_qq); domain->m_qq = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
TRACE1("Allocate EP m_v,m_volo,m_delv,m_vdov");
   if(domain->m_v != NULL)DRAM_FREE(domain->m_v); domain->m_v = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t));
   if(domain->m_volo != NULL)DRAM_FREE(domain->m_volo); domain->m_volo = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
   if(domain->m_delv != NULL)DRAM_FREE(domain->m_delv); domain->m_delv = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
   if(domain->m_vdov != NULL)DRAM_FREE(domain->m_vdov); domain->m_vdov = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
TRACE1("Allocate EP m_arealg,m_ss,m_elemMass");
   if(domain->m_arealg != NULL)DRAM_FREE(domain->m_arealg); domain->m_arealg = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
   if(domain->m_ss != NULL)DRAM_FREE(domain->m_ss); domain->m_ss = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
   if(domain->m_elemMass != NULL)DRAM_FREE(domain->m_elemMass); domain->m_elemMass = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
TRACE1("Allocate EP zero m_(e|p|v)");
#if defined(FSIM) 
  xe_printf("rag: domain %16.16lx\n",domain);
  xe_printf("rag: domain->m_e %16.16lx\n",domain->m_e);
  xe_printf("rag: domain->m_p %16.16lx\n",domain->m_p);
  xe_printf("rag: domain->m_v %16.16lx\n",domain->m_v);
  xe_printf("rag: loop (0;<%16.16lx;1)\n",hcSize);
#endif // FSIM or OCR
   FINISH 
     EDT_PAR_FOR_0xNx1(i,hcSize,domain_AllocateElemPersistent_edt_1,domain);
   END_FINISH
#if defined(FSIM) 
  xe_printf("rag: domain                %16.16lx\n",domain);
  xe_printf("rag: domain->m_matElemlist %16.16lx\n",domain->m_matElemlist);
  xe_printf("rag: domain->m_nodelist    %16.16lx\n",domain->m_nodelist);
  xe_printf("rag: domain->m_lxim        %16.16lx\n",domain->m_lxim);
  xe_printf("rag: domain->m_lxip        %16.16lx\n",domain->m_lxip);
  xe_printf("rag: domain->m_letam       %16.16lx\n",domain->m_letam);
  xe_printf("rag: domain->m_letap       %16.16lx\n",domain->m_letap);
  xe_printf("rag: domain->m_lzetam      %16.16lx\n",domain->m_lzetam);
  xe_printf("rag: domain->m_lzetap      %16.16lx\n",domain->m_lzetap);
  xe_printf("rag: domain->m_elemBC      %16.16lx\n",domain->m_elemBC);
  xe_printf("rag: domain->m_e           %16.16lx\n",domain->m_e);
  xe_printf("rag: domain->m_p           %16.16lx\n",domain->m_p);
  xe_printf("rag: domain->m_q           %16.16lx\n",domain->m_q);
  xe_printf("rag: domain->m_ql          %16.16lx\n",domain->m_ql);
  xe_printf("rag: domain->m_qq          %16.16lx\n",domain->m_qq);
  xe_printf("rag: domain->m_v           %16.16lx\n",domain->m_v);
  xe_printf("rag: domain->m_volo        %16.16lx\n",domain->m_volo);
  xe_printf("rag: domain->m_delv        %16.16lx\n",domain->m_delv);
  xe_printf("rag: domain->m_vdov        %16.16lx\n",domain->m_vdov);
  xe_printf("rag: domain->m_arealg      %16.16lx\n",domain->m_arealg);
  xe_printf("rag: domain->m_ss          %16.16lx\n",domain->m_ss);
  xe_printf("rag: domain->m_elemMass    %16.16lx\n",domain->m_elemMass);
#endif // FSIM or OCR
TRACE1("Allocate EP returns");
} // domain_AllocateElemPersistent()

   /* Temporaries should not be initialized in bulk but */
   /* this is a runnable placeholder for now */
static INLINE
void domain_AllocateElemTemporary(size_t hcSize) {
TRACE1("Allocate ET entry");
#if defined(FSIM) 
  xe_printf("rag: domain              %16.16lx\n",domain);
  xe_printf("rag: domain->m_dxx       %16.16lx\n",domain->m_dxx);
  xe_printf("rag: domain->m_dyy       %16.16lx\n",domain->m_dyy);
  xe_printf("rag: domain->m_dzz       %16.16lx\n",domain->m_dzz);
  xe_printf("rag: domain->m_delv_xi   %16.16lx\n",domain->m_delv_xi);
  xe_printf("rag: domain->m_delv_eta  %16.16lx\n",domain->m_delv_eta);
  xe_printf("rag: domain->m_delv_zeta %16.16lx\n",domain->m_delv_zeta);
  xe_printf("rag: domain->m_delx_xi   %16.16lx\n",domain->m_delx_xi);
  xe_printf("rag: domain->m_delx_eta  %16.16lx\n",domain->m_delx_eta);
  xe_printf("rag: domain->m_delx_zeta %16.16lx\n",domain->m_delx_zeta);
  xe_printf("rag: domain->m_vnew      %16.16lx\n",domain->m_vnew);
#endif // FSIM or OCR
TRACE1("Allocate ET m_d(xx|yy|zz)");
  if(domain->m_dxx != NULL)DRAM_FREE(domain->m_dxx); domain->m_dxx = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
  if(domain->m_dyy != NULL)DRAM_FREE(domain->m_dyy); domain->m_dyy = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
  if(domain->m_dzz != NULL)DRAM_FREE(domain->m_dzz); domain->m_dzz = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
TRACE1("Allocate ET m_delv_zi,m_delv_eta,m_delv_zeta");
  if(domain->m_delv_xi != NULL)DRAM_FREE(domain->m_delv_xi); domain->m_delv_xi = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
  if(domain->m_delv_eta != NULL)DRAM_FREE(domain->m_delv_eta); domain->m_delv_eta = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
  if(domain->m_delv_zeta != NULL)DRAM_FREE(domain->m_delv_zeta); domain->m_delv_zeta = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
TRACE1("Allocate ET m_delx_zi,m_delx_eta,m_delx_zeta");
  if(domain->m_delx_xi != NULL)DRAM_FREE(domain->m_delx_xi); domain->m_delx_xi = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
  if(domain->m_delx_eta != NULL)DRAM_FREE(domain->m_delx_eta); domain->m_delx_eta = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
  if(domain->m_delx_zeta != NULL)DRAM_FREE(domain->m_delx_zeta); domain->m_delx_zeta = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
TRACE1("Allocate ET m_vnew");
  if(domain->m_vnew != NULL)DRAM_FREE(domain->m_vnew); domain->m_vnew = (SHARED Real_t *)DRAM_MALLOC(hcSize,sizeof(Real_t)) ;
#if defined(FSIM) 
  xe_printf("rag: domain              %16.16lx\n",domain);
  xe_printf("rag: domain->m_dxx       %16.16lx\n",domain->m_dxx);
  xe_printf("rag: domain->m_dyy       %16.16lx\n",domain->m_dyy);
  xe_printf("rag: domain->m_dzz       %16.16lx\n",domain->m_dzz);
  xe_printf("rag: domain->m_delv_xi   %16.16lx\n",domain->m_delv_xi);
  xe_printf("rag: domain->m_delv_eta  %16.16lx\n",domain->m_delv_eta);
  xe_printf("rag: domain->m_delv_zeta %16.16lx\n",domain->m_delv_zeta);
  xe_printf("rag: domain->m_delx_xi   %16.16lx\n",domain->m_delx_xi);
  xe_printf("rag: domain->m_delx_eta  %16.16lx\n",domain->m_delx_eta);
  xe_printf("rag: domain->m_delx_zeta %16.16lx\n",domain->m_delx_zeta);
  xe_printf("rag: domain->m_vnew      %16.16lx\n",domain->m_vnew);
#endif // FSIM or OCR
TRACE1("Allocate ET returns");
} // domain_AllocateElemTemporary()

static INLINE
void domain_AllocateNodesets(size_t hcSize) {
TRACE1("Allocate NS entry");
#if defined(FSIM) 
  xe_printf("rag: domain              %16.16lx\n",domain);
  xe_printf("rag: domain->m_symmX     %16.16lx\n",domain->m_symmX);
  xe_printf("rag: domain->m_symmY     %16.16lx\n",domain->m_symmY);
  xe_printf("rag: domain->m_symmZ     %16.16lx\n",domain->m_symmZ);
#endif // FSIM or OCR
TRACE1("Allocate NS m_symm(X|Y|Z)");
   if(domain->m_symmX != NULL)DRAM_FREE(domain->m_symmX); domain->m_symmX = (SHARED Index_t *)DRAM_MALLOC(hcSize,sizeof(Index_t)) ;
   if(domain->m_symmY != NULL)DRAM_FREE(domain->m_symmY); domain->m_symmY = (SHARED Index_t *)DRAM_MALLOC(hcSize,sizeof(Index_t)) ;
   if(domain->m_symmZ != NULL)DRAM_FREE(domain->m_symmZ); domain->m_symmZ = (SHARED Index_t *)DRAM_MALLOC(hcSize,sizeof(Index_t)) ;
#if defined(FSIM) 
  xe_printf("rag: domain              %16.16lx\n",domain);
  xe_printf("rag: domain->m_symmX     %16.16lx\n",domain->m_symmX);
  xe_printf("rag: domain->m_symmY     %16.16lx\n",domain->m_symmY);
  xe_printf("rag: domain->m_symmZ     %16.16lx\n",domain->m_symmZ);
#endif // FSIM or OCR
TRACE1("Allocate NS returns");
} // domain_AllocateNodesets()

#if !defined(FSIM) && !defined(OCR)
   /****************/
   /* Deallocation */
   /****************/
static INLINE
void domain_DeallocateNodalPersistent(void) {
TRACE1("Deallocate NP m_(x|y|z)");
   if(domain->m_x != NULL)DRAM_FREE(domain->m_x); domain->m_x = NULL;
   if(domain->m_y != NULL)DRAM_FREE(domain->m_y); domain->m_y = NULL;
   if(domain->m_z != NULL)DRAM_FREE(domain->m_z); domain->m_z = NULL;
TRACE1("Deallocate NP m_(x|y|z)d");
   if(domain->m_xd != NULL)DRAM_FREE(domain->m_xd); domain->m_xd = NULL;
   if(domain->m_yd != NULL)DRAM_FREE(domain->m_yd); domain->m_yd = NULL;
   if(domain->m_zd != NULL)DRAM_FREE(domain->m_zd); domain->m_zd = NULL;
TRACE1("Deallocate NP m_(x|y|z)dd");
   if(domain->m_xdd != NULL)DRAM_FREE(domain->m_xdd); domain->m_xdd = NULL;
   if(domain->m_ydd != NULL)DRAM_FREE(domain->m_ydd); domain->m_ydd = NULL;
   if(domain->m_zdd != NULL)DRAM_FREE(domain->m_zdd); domain->m_zdd = NULL;
TRACE1("Deallocate NP m_f(x|y|z)");
   if(domain->m_fx != NULL)DRAM_FREE(domain->m_fx); domain->m_fx = NULL;
   if(domain->m_fy != NULL)DRAM_FREE(domain->m_fy); domain->m_fy = NULL;
   if(domain->m_fz != NULL)DRAM_FREE(domain->m_fz); domain->m_fz = NULL;
TRACE1("Deallocate NP m_nodalMass(x|y|z)");
   if(domain->m_nodalMass != NULL)DRAM_FREE(domain->m_nodalMass); domain->m_nodalMass = NULL;
} // domain_DeallocateNodalPersistent()

static INLINE
void domain_DeallocateElemPersistent(void) {
TRACE1("Deallocate EP m_matElemlist and m_nodelist");
   if(domain->m_matElemlist != NULL)DRAM_FREE(domain->m_matElemlist); domain->m_matElemlist = NULL;
   if(domain->m_nodelist != NULL)DRAM_FREE(domain->m_nodelist); domain->m_nodelist= NULL;
TRACE1("Deallocate EP m_lixm,m_lixp,m_letam,m_letap,m_lzetam,m_lzetap");
   if(domain->m_lxim != NULL)DRAM_FREE(domain->m_lxim); domain->m_lxim = NULL;
   if(domain->m_lxip != NULL)DRAM_FREE(domain->m_lxip); domain->m_lxip = NULL;
   if(domain->m_letam != NULL)DRAM_FREE(domain->m_letam); domain->m_letam = NULL;
   if(domain->m_letap != NULL)DRAM_FREE(domain->m_letap); domain->m_letap = NULL;
   if(domain->m_lzetam != NULL)DRAM_FREE(domain->m_lzetam); domain->m_lzetam = NULL;
   if(domain->m_lzetap != NULL)DRAM_FREE(domain->m_lzetap); domain->m_lzetap = NULL;
TRACE1("Deallocate EP m_elemBC");
   if(domain->m_elemBC != NULL)DRAM_FREE(domain->m_elemBC); domain->m_elemBC = NULL;
TRACE1("Deallocate EP m_e,m_p,m_q,m_ql,m_qq");
   if(domain->m_e != NULL)DRAM_FREE(domain->m_e); domain->m_e = NULL;
   if(domain->m_p != NULL)DRAM_FREE(domain->m_p); domain->m_p = NULL;
   if(domain->m_q != NULL)DRAM_FREE(domain->m_q); domain->m_q = NULL;
   if(domain->m_ql != NULL)DRAM_FREE(domain->m_ql); domain->m_ql = NULL;
   if(domain->m_qq != NULL)DRAM_FREE(domain->m_qq); domain->m_qq = NULL;
TRACE1("Deallocate EP m_v,m_volo,m_delv,m_vdov");
   if(domain->m_v != NULL)DRAM_FREE(domain->m_v); domain->m_v = NULL;
   if(domain->m_volo != NULL)DRAM_FREE(domain->m_volo); domain->m_volo = NULL;
   if(domain->m_delv != NULL)DRAM_FREE(domain->m_delv); domain->m_delv = NULL;
   if(domain->m_vdov != NULL)DRAM_FREE(domain->m_vdov); domain->m_vdov = NULL;
TRACE1("Deallocate EP m_arealg,m_ss,m_elemMass");
   if(domain->m_arealg != NULL)DRAM_FREE(domain->m_arealg); domain->m_arealg = NULL;
   if(domain->m_ss != NULL)DRAM_FREE(domain->m_ss); domain->m_ss = NULL;
   if(domain->m_elemMass != NULL)DRAM_FREE(domain->m_elemMass); domain->m_elemMass = NULL;
} // domain_DeallocateElemPersistent()

   /* Temporaries should not be initialized in bulk but */
   /* this is a runnable placeholder for now */
static INLINE
void domain_DeallocateElemTemporary(void) {
TRACE1("Deallocate ET m_d(xx|yy|zz)");
  if(domain->m_dxx != NULL)DRAM_FREE(domain->m_dxx); domain->m_dxx = NULL;
  if(domain->m_dyy != NULL)DRAM_FREE(domain->m_dyy); domain->m_dyy = NULL;
  if(domain->m_dzz != NULL)DRAM_FREE(domain->m_dzz); domain->m_dzz = NULL;
TRACE1("Deallocate ET m_delv_zi,m_delv_eta,m_delv_zeta");
  if(domain->m_delv_xi != NULL)DRAM_FREE(domain->m_delv_xi); domain->m_delv_xi = NULL;
  if(domain->m_delv_eta != NULL)DRAM_FREE(domain->m_delv_eta); domain->m_delv_eta = NULL;
  if(domain->m_delv_zeta != NULL)DRAM_FREE(domain->m_delv_zeta); domain->m_delv_zeta = NULL;
TRACE1("Deallocate ET m_delx_zi,m_delx_eta,m_delx_zeta");
  if(domain->m_delx_xi != NULL)DRAM_FREE(domain->m_delx_xi); domain->m_delx_xi = NULL;
  if(domain->m_delx_eta != NULL)DRAM_FREE(domain->m_delx_eta); domain->m_delx_eta = NULL;
  if(domain->m_delx_zeta != NULL)DRAM_FREE(domain->m_delx_zeta); domain->m_delx_zeta = NULL;
TRACE1("Deallocate ET m_vnew");
  if(domain->m_vnew != NULL)DRAM_FREE(domain->m_vnew); domain->m_vnew = NULL;
} // domain_DeallocateElemTemporary()

static INLINE
void domain_DeallocateNodesets(void) {
TRACE1("Deallocate NS m_symm(X|Y|Z)");
   if(domain->m_symmX != NULL)DRAM_FREE(domain->m_symmX); domain->m_symmX = NULL;
   if(domain->m_symmY != NULL)DRAM_FREE(domain->m_symmY); domain->m_symmY = NULL;
   if(domain->m_symmZ != NULL)DRAM_FREE(domain->m_symmZ); domain->m_symmZ = NULL;
} // domain_DeallocateNodesets()
#endif //NOT FSIM and NOT OCR

static INLINE
void InitializeProblem( SHARED struct Domain_t *domain , Index_t edgeElems , Index_t edgeNodes ) {

TRACE0("/* get run options to measure various metrics */");

  /****************************/
  /*   Initialize Sedov Mesh  */
  /****************************/

TRACE0("/* construct a uniform box for this processor */");

  domain->m_sizeX   = edgeElems ;
  domain->m_sizeY   = edgeElems ;
  domain->m_sizeZ   = edgeElems ;
  domain->m_numElem = edgeElems*edgeElems*edgeElems ;
  domain->m_numNode = edgeNodes*edgeNodes*edgeNodes ;

  Index_t domElems = domain->m_numElem ;

TRACE0("/* allocate field memory */");

  domain_AllocateElemPersistent(domain->m_numElem) ;
  domain_AllocateElemTemporary (domain->m_numElem) ;

  domain_AllocateNodalPersistent(domain->m_numNode) ;
  domain_AllocateNodesets(edgeNodes*edgeNodes) ;

TRACE0("/* initialize nodal coordinates */");

  FINISH
    Real_t sf = cast_Real_t(1.125)/cast_Real_t(edgeElems);
    Index_t dimN = edgeNodes, dimNdimN = edgeNodes*edgeNodes;

    for(Index_t pln = 0 ; pln < edgeNodes ; ++pln ) {
      Real_t tz = cast_Real_t(pln)*sf;
      Index_t pln_nidx = pln*dimNdimN;
      for(Index_t row = 0 ; row < edgeNodes ; ++row ) {
        Real_t ty = cast_Real_t(row)*sf;
        Index_t pln_row_nidx = pln_nidx + row*dimN;
        PAR_FOR_0xNx1(col,edgeNodes,pln,row,tz,ty,pln_row_nidx,domain,sf,dimN,dimNdimN)
          Real_t tx = cast_Real_t(col)*sf;
          Index_t nidx = pln_row_nidx+col;
          domain->m_x[nidx] = tx;
//DEBUG if(nidx==1)fprintf(stdout,"m_x[1] = %e\n",domain->m_x[1]);
          domain->m_y[nidx] = ty;
//DEBUG if(nidx==1)fprintf(stdout,"m_y[1] = %e\n",domain->m_y[1]);
          domain->m_z[nidx] = tz;
//DEBUG if(nidx==1)fprintf(stdout,"m_z[1] = %e\n",domain->m_z[1]);
        END_PAR_FOR(col)
      } // for row
    } // for pln
  END_FINISH

TRACE0("/* embed hexehedral elements in nodal point lattice */");

  FINISH
    Index_t dimE = edgeElems, dimEdimE = edgeElems*edgeElems;
    Index_t dimN = edgeNodes, dimNdimN = edgeNodes*edgeNodes;

    for(Index_t pln = 0 ; pln < edgeElems ; ++pln ) {
      Index_t pln_nidx = pln*dimNdimN;
      Index_t pln_zidx = pln*dimEdimE;
      for(Index_t row = 0 ; row < edgeElems ; ++row ) {
        Index_t pln_row_nidx = pln_nidx + row*dimN;
        Index_t pln_row_zidx = pln_zidx + row*dimE;
        PAR_FOR_0xNx1(col,edgeElems,pln,row,pln_row_nidx,pln_row_zidx,domain,dimE,dimEdimE,dimN,dimNdimN)
          Index_t nidx = pln_row_nidx+col;
          Index_t zidx = pln_row_zidx+col;
          SHARED Index_t *localNode = (SHARED Index_t *)&domain->m_nodelist[EIGHT*zidx] ;
          localNode[0] = nidx                       ;
          localNode[1] = nidx                   + 1 ;
          localNode[2] = nidx            + dimN + 1 ;
          localNode[3] = nidx            + dimN     ;
          localNode[4] = nidx + dimNdimN            ;
          localNode[5] = nidx + dimNdimN        + 1 ;
          localNode[6] = nidx + dimNdimN + dimN + 1 ;
          localNode[7] = nidx + dimNdimN + dimN     ;
        END_PAR_FOR(col)
      } // for row
    } // for pln
  END_FINISH

TRACE0("/* Create a material IndexSet (entire domain same material for now) */");

  FINISH
    PAR_FOR_0xNx1(i,domElems,domain)
      domain->m_matElemlist[i] = i ;
    END_PAR_FOR(i)
  END_FINISH
   
TRACE0("/* initialize material parameters */");

  domain->m_dtfixed            = cast_Real_t(-1.0e-7) ;
  domain->m_deltatime          = cast_Real_t(1.0e-7) ;
  domain->m_deltatimemultlb    = cast_Real_t(1.1) ;
  domain->m_deltatimemultub    = cast_Real_t(1.2) ;
  domain->m_stoptime           = cast_Real_t(1.0e-2) ;
  domain->m_dtcourant          = cast_Real_t(1.0e+20) ;
  domain->m_dthydro            = cast_Real_t(1.0e+20) ;
  domain->m_dtmax              = cast_Real_t(1.0e-2) ;
  domain->m_time               = cast_Real_t(0.) ;
  domain->m_cycle              = 0 ;

  domain->m_e_cut              = cast_Real_t(1.0e-7) ;
  domain->m_p_cut              = cast_Real_t(1.0e-7) ;
  domain->m_q_cut              = cast_Real_t(1.0e-7) ;
  domain->m_u_cut              = cast_Real_t(1.0e-7) ;
  domain->m_v_cut              = cast_Real_t(1.0e-10) ;

  domain->m_hgcoef             = cast_Real_t(3.0) ;

  domain->m_qstop              = cast_Real_t(1.0e+12) ;
  domain->m_monoq_max_slope    = cast_Real_t(1.0) ;
  domain->m_monoq_limiter_mult = cast_Real_t(2.0) ;
  domain->m_qlc_monoq          = cast_Real_t(0.5) ;
  domain->m_qqc_monoq          = cast_Real_t(2.0)/cast_Real_t(3.0) ;
  domain->m_qqc                = cast_Real_t(2.0) ;

  domain->m_pmin               = cast_Real_t(0.) ;
  domain->m_emin               = cast_Real_t(-1.0e+15) ;

  domain->m_dvovmax            = cast_Real_t(0.1) ;

  domain->m_eosvmax            = cast_Real_t(1.0e+9) ;
  domain->m_eosvmin            = cast_Real_t(1.0e-9) ;

  domain->m_refdens            = cast_Real_t(1.0) ;

  FINISH
    PAR_FOR_0xNx1(i,domElems,domain)
#if  defined(HAB_C)
      Real_t *x_local = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
      Real_t *y_local = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
      Real_t *z_local = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
#else // NOT HAB_C
      Real_t x_local[EIGHT], y_local[EIGHT], z_local[EIGHT] ;
#endif //    HAB_C
      SHARED Index_t *elemToNode = (SHARED Index_t *)&domain->m_nodelist[EIGHT*i] ;
      for( Index_t lnode=0 ; lnode < EIGHT ; ++lnode ) {
        Index_t gnode = elemToNode[lnode];
        x_local[lnode] = domain->m_x[gnode];
        y_local[lnode] = domain->m_y[gnode];
        z_local[lnode] = domain->m_z[gnode];
      } // for lnode

      // volume calculations
      Real_t volume = CalcElemVolume(x_local, y_local, z_local );
      domain->m_volo[i] = volume ;
      domain->m_elemMass[i] = volume ;

// RAG ///////////////////////////////////////////////////////// RAG //
// RAG  Atomic Memory Floating-point Addition Scatter operation  RAG //
// RAG ///////////////////////////////////////////////////////// RAG //

      for( Index_t j=0 ; j<EIGHT ; ++j ) {
        Index_t idx = elemToNode[j] ;
        Real_t value = volume / cast_Real_t(8.0);
        AMO__sync_addition_double(&domain->m_nodalMass[idx], value);
      } // for j

#if  defined(HAB_C)
      free(z_local) ;
      free(y_local) ;
      free(x_local) ;
#endif //    HAB_C
    END_PAR_FOR(i)
  END_FINISH

TRACE0("/* deposit energy */");

  domain->m_e[0] = cast_Real_t(3.948746e+7) ;

TRACE0("/* set up symmetry nodesets */");
//DEBUG fprintf(stdout,"e(0)=%e\n",domain->m_e[0]);

  FINISH
    Index_t dimN = edgeNodes, dimNdimN = dimN*dimN;
    
    for ( Index_t i = 0; i < edgeNodes ; ++i ) {
      Index_t planeInc = i*dimNdimN ;
      Index_t rowInc   = i*dimN ;
      PAR_FOR_0xNx1(j,edgeNodes,i,planeInc,rowInc,domain,dimN,dimNdimN)
        Index_t nidx = rowInc + j;
        domain->m_symmX[nidx] = planeInc + j*dimN ;
        domain->m_symmY[nidx] = planeInc + j ;
        domain->m_symmZ[nidx] = rowInc   + j ;
      END_PAR_FOR(j)
    } // for i
  END_FINISH

TRACE0("/* set up elemement connectivity information */");
//DEBUG fprintf(stdout,"e(0)=%e\n",domain->m_e[0]);

  FINISH
    domain->m_lxim[0] = 0 ;
    PAR_FOR_0xNx1(i,domElems-1,domain,domElems)
          domain->m_lxim[i+1] = i ;
          domain->m_lxip[i  ] = i+1 ;
    END_PAR_FOR(i)
    domain->m_lxip[domElems-1] = domElems-1 ;
  END_FINISH

  FINISH
    PAR_FOR_0xNx1(i,edgeElems,domain,domElems,edgeElems)
      domain->m_letam[i] = i ; 
      domain->m_letap[domElems-edgeElems+i] = domElems-edgeElems+i ;
    END_PAR_FOR(i)
  END_FINISH

  FINISH
    PAR_FOR_0xNx1(i,(domElems-edgeElems),domain,edgeElems)
      domain->m_letam[i+edgeElems] = i ;
      domain->m_letap[i          ] = i+edgeElems ;
    END_PAR_FOR(i)
  END_FINISH

  FINISH
    PAR_FOR_0xNx1(i,(edgeElems*edgeElems),domain,domElems,edgeElems)
      domain->m_lzetam[i] = i ;
      domain->m_lzetap[domElems-edgeElems*edgeElems+i] = domElems-edgeElems*edgeElems+i ;
    END_PAR_FOR(i)
  END_FINISH

  FINISH
    Index_t dimE = edgeElems, dimEdimE = dimE*dimE;
    PAR_FOR_0xNx1(i,(domElems-dimEdimE),domain,dimEdimE,domElems)
      domain->m_lzetam[i+dimEdimE] = i ;
      domain->m_lzetap[i]          = i+dimEdimE ;
    END_PAR_FOR(i)
  END_FINISH

TRACE0("/* set up boundary condition information */");
//DEBUG fprintf(stdout,"e(0)=%e\n",domain->m_e[0]);

  FINISH
    PAR_FOR_0xNx1(i,domElems,domain,domElems)
      domain->m_elemBC[i] = 0 ;  /* clear BCs by default */
    END_PAR_FOR(i)
  END_FINISH

TRACE0("/* faces on \"external\" boundaries will be */");
TRACE0("/* symmetry plane or free surface BCs     */");
//DEBUG fprintf(stdout,"e(0)=%e\n",domain->m_e[0]);

  FINISH
    Index_t dimE = edgeElems, dimEdimE = dimE*dimE;
    for ( Index_t i = 0 ; i < edgeElems ; ++i ) {
      Index_t planeInc = i*dimEdimE ;
      Index_t rowInc   = i*dimE ;
      PAR_FOR_0xNx1(j,edgeElems,i,domain,planeInc,rowInc,domElems,dimE,dimEdimE)
        domain->m_elemBC[planeInc+j*dimE           ] |= XI_M_SYMM ;
        domain->m_elemBC[planeInc+j*dimE+1*dimE-1  ] |= XI_P_FREE ;
        domain->m_elemBC[planeInc+j                ] |= ETA_M_SYMM ;
        domain->m_elemBC[planeInc+j+dimEdimE-dimE  ] |= ETA_P_FREE ;
        domain->m_elemBC[rowInc+j                  ] |= ZETA_M_SYMM ;
        domain->m_elemBC[rowInc+j+domElems-dimEdimE] |= ZETA_P_FREE ;
      END_PAR_FOR(j)
    } // for i
  END_FINISH
} // InitializeProblem()
