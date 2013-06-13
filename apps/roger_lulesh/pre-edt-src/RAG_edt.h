#ifdef CILK
extern "C" {
#endif

void              domain_AllocateNodalPersistent_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ) ;

void               domain_AllocateElemPersistent_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ) ;

void                     InitStressTermsForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                        Real_t *sigxx , Real_t *sigyy , Real_t *sigzz ) ;

void                     IntegrateStressForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                        Real_t *sigxx , Real_t *sigyy , Real_t *sigzz , Real_t *determ ) ;

void                CalcFBHourglassForceForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                        Real_t hourg ,
                                                        Real_t *determ ,
                                                        Real_t *x8n  , Real_t *y8n  , Real_t *z8n  ,
                                                        Real_t *dvdx , Real_t *dvdy , Real_t *dvdz ) ;

void                CalcHourglassControlForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                        Real_t *determ ,
                                                        Real_t *x8n  , Real_t *y8n  , Real_t *z8n  ,
                                                        Real_t *dvdx , Real_t *dvdy , Real_t *dvdz ) ;

void                     CalcVolumeForceForElems_edt_1( Index_t i_out , Index_t i_end ,
                                                        Real_t *determ) ;

void                           CalcForceForNodes_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ) ;

void                    CalcAccelerationForNodes_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ) ;

void ApplyAccelerationBoundaryConditionsForNodes_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ) ;

void                        CalcVelocityForNodes_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                        Real_t dt, Real_t u_cut ) ;

void                        CalcPositionForNodes_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                        Real_t dt ) ;

void                      CalcKinematicsForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                        Real_t dt ) ;

void                        CalcLagrangeElements_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ) ;

void             CalcMonotonicQGradientsForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ) ;

void                CalcMonotonicQRegionForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                        Real_t qlc_monoq , Real_t qqc_monoq ,
                                                        Real_t monoq_limiter_mult , Real_t monoq_max_slope ,
                                                        Real_t ptiny ) ;

void                                CalcQForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         Real_t qstop, SHARED uint64_t *pIndex_AMO ) ;

void                         CalcPressureForElems_edt_1( Index_t i_out , Index_t i_end ,
                                                         Real_t* p_new , Real_t* bvc , Real_t* pbvc ,          /* OUT */
                                                         Real_t* e_old , Real_t* compression , Real_t *vnewc , /* IN */
                                                         Real_t pmin , Real_t p_cut , Real_t eosvmax ) ;       /* IN */

void                           CalcEnergyForElems_edt_1( Index_t i_out , Index_t i_end ,
                                                         Real_t *e_new ,                  /* OUT */
                                                         Real_t *e_old , Real_t *delvc ,  /* IN */
                                                         Real_t *p_old , Real_t *q_old ,  /* IN */
                                                         Real_t *work ,  Real_t emin ) ;  /* IN */

void                           CalcEnergyForElems_edt_2( Index_t i_out , Index_t i_end ,
                                                         Real_t *q_new ,                               /* OUT */
                                                         Real_t *e_new ,                               /* IN/OUT */
                                                         Real_t *compHalfStep , Real_t *qq ,           /* IN */
                                                         Real_t *ql , Real_t *pbvc ,                   /* IN */
                                                         Real_t *bvc , Real_t *pHalfStep ,             /* IN */
                                                         Real_t *delvc , Real_t *p_old ,               /* IN */
                                                         Real_t *q_old , Real_t *work ,                /* IN */
                                                         Real_t rho0 , Real_t e_cut , Real_t emin ) ;  /* IN */

void                           CalcEnergyForElems_edt_3( Index_t i_out , Index_t i_end ,
                                                         Real_t *e_new ,                               /* IN/OUT */
                                                         Real_t *delvc , Real_t *pbvc ,                /* IN */
                                                         Real_t *vnewc , Real_t *bvc , Real_t *p_new , /* IN */
                                                         Real_t *ql , Real_t *qq ,                     /* IN */
                                                         Real_t *p_old , Real_t *q_old ,               /* IN */
                                                         Real_t *pHalfStep , Real_t *q_new ,           /* IN */
                                                         Real_t rho0 , Real_t e_cut , Real_t emin ) ;  /* IN */

void                           CalcEnergyForElems_edt_4( Index_t i_out , Index_t i_end ,
                                                         Real_t *q_new ,                  /* OUT */
                                                         Real_t *delvc , Real_t *pbvc ,   /* IN */
                                                         Real_t *e_new , Real_t *vnewc ,  /* IN */
                                                         Real_t *bvc , Real_t *p_new ,    /* IN */
                                                         Real_t *ql , Real_t *qq ,        /* IN */
                                                         Real_t rho0 , Real_t q_cut ) ;   /* IN */

void                       CalcSoundSpeedForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         Real_t *vnewc, Real_t *enewc,
                                                         Real_t *pnewc, Real_t *pbvc,
                                                         Real_t *bvc, 
                                                         Real_t rho0 ) ;

void                              EvalEOSForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         Real_t *delvc , Real_t *e_old ,
                                                         Real_t *p_old , Real_t *q_old ,
                                                         Real_t *qq , Real_t *ql ) ; // RAG GATHERS

void                              EvalEOSForElems_edt_2( Index_t i_out , Index_t i_end ,
                                                         Real_t *compression , Real_t *vnewc ,
                                                         Real_t *delvc , Real_t *compHalfStep ,
                                                         Real_t *work , Real_t *p_old ,
                                                         Real_t eosvmin , Real_t eosvmax ) ; // RAG STRIDE ONE

void                              EvalEOSForElems_edt_3( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         Real_t *p_new , Real_t *e_new ,
                                                         Real_t *q_new ) ; // RAG SCATTERS


void              ApplyMaterialPropertiesForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         Real_t *vnewc ) ;

void              ApplyMaterialPropertiesForElems_edt_2( Index_t i_out , Index_t i_end , 
                                                         Real_t *vnewc ,
                                                         Real_t eosvmin , Real_t eosvmax ) ;

void              ApplyMaterialPropertiesForElems_edt_3( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         Real_t eosvmin , Real_t eosvmax ) ;

void                        UpdateVolumesForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         Real_t v_cut ) ;

void                  CalcHydroConstraintForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         Real_t *pDtHydro , Index_t *pHydro_elem ,
                                                         uint64_t *pidamin_lock ,
                                                         Real_t dvovmax ) ;
#ifdef CILK
} // extern "C"
#endif
