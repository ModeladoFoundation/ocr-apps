#ifdef CILK
extern "C" {
#endif

void              domain_AllocateNodalPersistent_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ) ;

void               domain_AllocateElemPersistent_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ) ;

void                     InitStressTermsForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                        SHARED Real_t *sigxx , SHARED Real_t *sigyy , SHARED Real_t *sigzz ) ;

void                     IntegrateStressForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                        SHARED Real_t *sigxx , SHARED Real_t *sigyy , SHARED Real_t *sigzz ,
                                                        SHARED Real_t *determ ) ;

void                CalcFBHourglassForceForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                        SHARED Real_t *determ ,
                                                        SHARED Real_t *x8n  , SHARED Real_t *y8n  , SHARED Real_t *z8n  ,
                                                        SHARED Real_t *dvdx , SHARED Real_t *dvdy , SHARED Real_t *dvdz ,
                                                        Real_t hourg ) ;

void                CalcHourglassControlForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                        SHARED Real_t *determ ,
                                                        SHARED Real_t *x8n  , SHARED Real_t *y8n  , SHARED Real_t *z8n  ,
                                                        SHARED Real_t *dvdx , SHARED Real_t *dvdy , SHARED Real_t *dvdz ) ;

void                     CalcVolumeForceForElems_edt_1( Index_t i_out , Index_t i_end ,
                                                        SHARED Real_t *determ) ;

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
                                                         SHARED Real_t* p_new , SHARED Real_t* bvc ,         /* OUT */
                                                         SHARED Real_t* pbvc ,                               /* OUT */
                                                         SHARED Real_t* e_old , SHARED Real_t* compression , /* IN */
                                                         SHARED Real_t *vnewc ,                              /* IN */
                                                         Real_t pmin , Real_t p_cut , Real_t eosvmax ) ;     /* IN */

void                           CalcEnergyForElems_edt_1( Index_t i_out , Index_t i_end ,
                                                         SHARED Real_t *e_new ,                         /* OUT */
                                                         SHARED Real_t *e_old , SHARED Real_t *delvc ,  /* IN */
                                                         SHARED Real_t *p_old , SHARED Real_t *q_old ,  /* IN */
                                                         SHARED Real_t *work ,                          /* IN */
                                                         Real_t emin ) ;                                /* IN */

void                           CalcEnergyForElems_edt_2( Index_t i_out , Index_t i_end ,
                                                         SHARED Real_t *q_new ,                               /* OUT */
                                                         SHARED Real_t *e_new ,                               /* IN/OUT */
                                                         SHARED Real_t *compHalfStep , SHARED Real_t *qq ,    /* IN */
                                                         SHARED Real_t *ql , SHARED Real_t *pbvc ,            /* IN */
                                                         SHARED Real_t *bvc , SHARED Real_t *pHalfStep ,      /* IN */
                                                         SHARED Real_t *delvc , SHARED Real_t *p_old ,        /* IN */
                                                         SHARED Real_t *q_old , SHARED Real_t *work ,         /* IN */
                                                         Real_t rho0 , Real_t e_cut , Real_t emin ) ;         /* IN */

void                           CalcEnergyForElems_edt_3( Index_t i_out , Index_t i_end ,
                                                         SHARED Real_t *e_new ,                            /* IN/OUT */
                                                         SHARED Real_t *delvc , SHARED Real_t *pbvc ,      /* IN */
                                                         SHARED Real_t *vnewc , SHARED Real_t *bvc ,       /* IN */
                                                         SHARED Real_t *p_new , SHARED Real_t *ql ,        /* IN */
                                                         SHARED Real_t *qq , SHARED Real_t *p_old ,        /* IN */
                                                         SHARED Real_t *q_old , SHARED Real_t *pHalfStep , /* IN */
                                                         SHARED Real_t *q_new ,                            /* IN */
                                                         Real_t rho0 , Real_t e_cut , Real_t emin ) ;      /* IN */

void                           CalcEnergyForElems_edt_4( Index_t i_out , Index_t i_end ,
                                                         SHARED Real_t *q_new ,                         /* OUT */
                                                         SHARED Real_t *delvc , SHARED Real_t *pbvc ,   /* IN */
                                                         SHARED Real_t *e_new , SHARED Real_t *vnewc ,  /* IN */
                                                         SHARED Real_t *bvc , SHARED Real_t *p_new ,    /* IN */
                                                         SHARED Real_t *ql , SHARED Real_t *qq ,        /* IN */
                                                         Real_t rho0 , Real_t q_cut ) ;                 /* IN */

void                       CalcSoundSpeedForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         SHARED Real_t *vnewc, SHARED Real_t *enewc,
                                                         SHARED Real_t *pnewc, SHARED Real_t *pbvc,
                                                         SHARED Real_t *bvc, 
                                                         Real_t rho0 ) ;

void                              EvalEOSForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         SHARED Real_t *delvc , SHARED Real_t *e_old ,
                                                         SHARED Real_t *p_old , SHARED Real_t *q_old ,
                                                         SHARED Real_t *qq , SHARED Real_t *ql ) ; // RAG GATHERS

void                              EvalEOSForElems_edt_2( Index_t i_out , Index_t i_end ,
                                                         SHARED Real_t *compression , SHARED Real_t *vnewc ,
                                                         SHARED Real_t *delvc , SHARED Real_t *compHalfStep ,
                                                         SHARED Real_t *work , SHARED Real_t *p_old ,
                                                         Real_t eosvmin , Real_t eosvmax ) ; // RAG STRIDE ONE

void                              EvalEOSForElems_edt_3( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         SHARED Real_t *p_new , SHARED Real_t *e_new ,
                                                         SHARED Real_t *q_new ) ; // RAG SCATTERS


void              ApplyMaterialPropertiesForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         SHARED Real_t *vnewc ) ;

void              ApplyMaterialPropertiesForElems_edt_2( Index_t i_out , Index_t i_end , 
                                                         SHARED Real_t *vnewc ,
                                                         Real_t eosvmin , Real_t eosvmax ) ;

void              ApplyMaterialPropertiesForElems_edt_3( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         Real_t eosvmin , Real_t eosvmax ) ;

void                        UpdateVolumesForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         Real_t v_cut ) ;

void                  CalcHydroConstraintForElems_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ,
                                                         SHARED Real_t *pDtHydro , SHARED Index_t *pHydro_elem ,
                                                         SHARED uint64_t *pidamin_lock ,
                                                         Real_t dvovmax ) ;
#ifdef CILK
} // extern "C"
#endif
