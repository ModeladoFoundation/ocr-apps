#ifdef CILK
extern "C" {
#endif

void domain_AllocateNodalPersistent_edt_1( Index_t i_out , Index_t i_end ,
                                           SHARED struct Domain_t *domain ) ;

void  domain_AllocateElemPersistent_edt_1( Index_t i_out , Index_t i_end ,
                                           SHARED struct Domain_t *domain ) ;

void        InitStressTermsForElems_edt_1( Index_t i_out , Index_t i_end ,
                                           Real_t *sigxx , Real_t *sigyy , Real_t *sigzz ,
                                           SHARED struct Domain_t *domain ) ;

void        IntegrateStressForElems_edt_1( Index_t i_out , Index_t i_end ,
                                           Real_t *sigxx , Real_t *sigyy , Real_t *sigzz , Real_t *determ ,
                                           SHARED struct Domain_t *domain ) ;

void   CalcFBHourglassForceForElems_edt_1( Index_t i2_out , Index_t i2_end ,
                                           Real_t hourg , 
                                           HAB_CONST Real_t *GAMMA , Real_t *determ ,
                                           Real_t *x8n  , Real_t *y8n  , Real_t *z8n  ,
                                           Real_t *dvdx , Real_t *dvdy , Real_t *dvdz ,
                                           SHARED struct Domain_t *domain) ;

void   CalcHourglassControlForElems_edt_1( Index_t i_out , Index_t i_end ,
                                           Real_t *determ ,
                                           Real_t *x8n  , Real_t *y8n  , Real_t *z8n  ,
                                           Real_t *dvdx , Real_t *dvdy , Real_t *dvdz ,
                                           SHARED struct Domain_t *domain) ;

void        CalcVolumeForceForElems_edt_1( Index_t i_out , Index_t i_end ,
                                           Real_t *determ) ;

void              CalcForceForNodes_edt_1( Index_t i_out , Index_t i_end ,
                                           SHARED struct Domain_t *domain) ;

#ifdef CILK
} // extern "C"
#endif
