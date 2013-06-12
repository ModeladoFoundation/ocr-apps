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

#ifdef CILK
} // extern "C"
#endif
