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

extern struct Domain_t *domain;

#include "RAG_edt.h"

#include "kernels.h"

void domain_AllocateNodalPersistent_edt_1(Index_t i_out, Index_t i_end, SHARED struct Domain_t *domain) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
      domain->m_xd[i] = (Real_t)(0.0);
      domain->m_yd[i] = (Real_t)(0.0);
      domain->m_zd[i] = (Real_t)(0.0);

      domain->m_xdd[i] = (Real_t)(0.0);
      domain->m_ydd[i] = (Real_t)(0.0);
      domain->m_zdd[i] = (Real_t)(0.0) ;

      domain->m_nodalMass[i] = (Real_t)(0.0) ;
  } // for i
} // domain_AllocateNodalPersistent_edt_1()

void domain_AllocateElemPersistent_edt_1(Index_t i_out, Index_t i_end, SHARED struct Domain_t *domain) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
            domain->m_e[i] = (Real_t)(0.0);
            domain->m_p[i] = (Real_t)(0.0);
            domain->m_v[i] = (Real_t)(1.0);
  } // for i
} // domain_AllocateElemPersistent_edt_1()
 
void InitStressTermsForElems_edt_1(Index_t i_out, Index_t i_end, SHARED struct Domain_t *domain,
                                   Real_t *sigxx, Real_t *sigyy, Real_t *sigzz  ) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
    sigxx[i] = sigyy[i] = sigzz[i] =  - domain->m_p[i] - domain->m_q[i] ;
  } // for i
} // InitStressTermsForElems_edt_1()

void IntegrateStressForElems_edt_1(Index_t i_out, Index_t i_end, struct Domain_t *domain,
                                   Real_t *sigxx, Real_t *sigyy, Real_t *sigzz, Real_t *determ ) {
#if  defined(HAB_C)
  Real_t *B        = (Real_t *)malloc(THREE*EIGHT*sizeof(Real_t)) ;
  Real_t *x_local  = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
  Real_t *y_local  = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
  Real_t *z_local  = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
  Real_t *fx_local = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
  Real_t *fy_local = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
  Real_t *fz_local = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
#else // NOT HAB_C
  Real_t B[THREE*EIGHT]; // -- shape function derivatives B[THREE][EIGHT]
  Real_t  x_local[EIGHT],  y_local[EIGHT],  z_local[EIGHT];
  Real_t fx_local[EIGHT], fy_local[EIGHT], fz_local[EIGHT];
#endif //    HAB_C
  for(Index_t i = i_out ; i < i_end ; ++i ) {
    SHARED HAB_CONST Index_t* HAB_CONST elemNodes = (SHARED Index_t *)&domain->m_nodelist[EIGHT*i];
    // get nodal coordinates from global arrays and copy into local arrays.
    for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode ) {
      Index_t gnode = elemNodes[lnode];
      x_local[lnode] = domain->m_x[gnode];
      y_local[lnode] = domain->m_y[gnode];
      z_local[lnode] = domain->m_z[gnode];
    } // for lnode
    /* Volume calculation involves extra work for numerical consistency. */
    CalcElemShapeFunctionDerivatives(x_local, y_local, z_local,
                                     B, &determ[i]);
    CalcElemNodeNormals( &B[0*EIGHT], &B[1*EIGHT], &B[2*EIGHT],
                         x_local, y_local, z_local );
    SumElemStressesToNodeForces( B, sigxx[i], sigyy[i], sigzz[i],
                                 fx_local, fy_local, fz_local ) ;
// RAG ///////////////////////////////////////////////////////// RAG //
// RAG  Atomic Memory Floating-point Addition Scatter operation  RAG //
// RAG ///////////////////////////////////////////////////////// RAG //
    // accumulate nodal force contributions to global force arrray.
    for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode ) {
      Index_t gnode = elemNodes[lnode];
      AMO__sync_addition_double(&domain->m_fx[gnode], fx_local[lnode]);
      AMO__sync_addition_double(&domain->m_fy[gnode], fy_local[lnode]);
      AMO__sync_addition_double(&domain->m_fz[gnode], fz_local[lnode]);
    } // for lnode
#if  defined(HAB_C)
    free(fz_local) ; free(fy_local) ; free(fx_local) ;
    free(z_local)  ; free(y_local)  ; free(x_local) ;
    free(B) ;
#endif //    HAB_C
  } // for i
} // IntegrateStressForElems_edt_1()

// RAG -- GAMMA now compile time constants in rodata
static HAB_CONST Real_t GAMMA[FOUR*EIGHT] = {
( 1.), ( 1.), (-1.), (-1.), (-1.), (-1.), ( 1.), ( 1.),
( 1.), (-1.), (-1.), ( 1.), (-1.), ( 1.), ( 1.), (-1.),
( 1.), (-1.), ( 1.), (-1.), ( 1.), (-1.), ( 1.), (-1.),
(-1.), ( 1.), (-1.), ( 1.), ( 1.), (-1.), ( 1.), (-1.),
};

void CalcFBHourglassForceForElems_edt_1( Index_t i2_out, Index_t i2_end, SHARED struct Domain_t *domain,
                                         Real_t hourg,
                                         Real_t *determ ,
                                         Real_t *x8n  , Real_t *y8n  , Real_t *z8n  ,
                                         Real_t *dvdx , Real_t *dvdy , Real_t *dvdz ) {
  for(Index_t i2 = i2_out ; i2 < i2_end ; ++i2 ) {
#if  defined(HAB_C)
    Real_t *hgfx = (Real_t *)malloc(EIGHT*sizeof(Real_t));
    Real_t *hgfy = (Real_t *)malloc(EIGHT*sizeof(Real_t));
    Real_t *hgfz = (Real_t *)malloc(EIGHT*sizeof(Real_t));

    Real_t *hourgam0 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam1 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam2 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam3 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam4 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam5 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam6 = (Real_t *)malloc(FOUR*sizeof(Real_t));
    Real_t *hourgam7 = (Real_t *)malloc(FOUR*sizeof(Real_t));

    Real_t *xd1 = (Real_t *)malloc(EIGHT*sizeof(Real_t));
    Real_t *yd1 = (Real_t *)malloc(EIGHT*sizeof(Real_t));
    Real_t *zd1 = (Real_t *)malloc(EIGHT*sizeof(Real_t));

    Index_t *elemToNode  = (Index_t *)malloc(EIGHT*sizeof(Index_t)) ;
#else // NOT HAB_C
    Real_t hgfx[EIGHT], hgfy[EIGHT], hgfz[EIGHT];

    Real_t hourgam0[FOUR], hourgam1[FOUR], hourgam2[FOUR], hourgam3[FOUR];
    Real_t hourgam4[FOUR], hourgam5[FOUR], hourgam6[FOUR], hourgam7[FOUR];

    Real_t  xd1[EIGHT],  yd1[EIGHT],  zd1[EIGHT];

    Index_t elemToNode[EIGHT];
#endif //    HAB_C


    Real_t coefficient;

    Index_t i3=EIGHT*i2;

    Real_t volinv=cast_Real_t(1.0)/determ[i2];
    Real_t ss1, mass1, volume13 ;

// RAG -- GATHER/SCATTER Index Values

    for( Index_t i=0 ; i<EIGHT ; ++i ) {
      elemToNode[i] = *(Index_t *)&domain->m_nodelist[EIGHT*i2+i];
    } // for i

    for( Index_t i1=0 ; i1<FOUR ; ++i1 ){
      Real_t hourmodx =
          x8n[i3+0] * GAMMA[i1*EIGHT+0] + x8n[i3+1] * GAMMA[i1*EIGHT+1] +
          x8n[i3+2] * GAMMA[i1*EIGHT+2] + x8n[i3+3] * GAMMA[i1*EIGHT+3] +
          x8n[i3+4] * GAMMA[i1*EIGHT+4] + x8n[i3+5] * GAMMA[i1*EIGHT+5] +
          x8n[i3+6] * GAMMA[i1*EIGHT+6] + x8n[i3+7] * GAMMA[i1*EIGHT+7];

      Real_t hourmody =
          y8n[i3+0] * GAMMA[i1*EIGHT+0] + y8n[i3+1] * GAMMA[i1*EIGHT+1] +
          y8n[i3+2] * GAMMA[i1*EIGHT+2] + y8n[i3+3] * GAMMA[i1*EIGHT+3] +
          y8n[i3+4] * GAMMA[i1*EIGHT+4] + y8n[i3+5] * GAMMA[i1*EIGHT+5] +
          y8n[i3+6] * GAMMA[i1*EIGHT+6] + y8n[i3+7] * GAMMA[i1*EIGHT+7];

      Real_t hourmodz =
          z8n[i3+0] * GAMMA[i1*EIGHT+0] + z8n[i3+1] * GAMMA[i1*EIGHT+1] +
          z8n[i3+2] * GAMMA[i1*EIGHT+2] + z8n[i3+3] * GAMMA[i1*EIGHT+3] +
          z8n[i3+4] * GAMMA[i1*EIGHT+4] + z8n[i3+5] * GAMMA[i1*EIGHT+5] +
          z8n[i3+6] * GAMMA[i1*EIGHT+6] + z8n[i3+7] * GAMMA[i1*EIGHT+7];

      hourgam0[i1] = GAMMA[i1*EIGHT+0] -  volinv*(dvdx[i3  ] * hourmodx +
                                                  dvdy[i3  ] * hourmody +
                                                  dvdz[i3  ] * hourmodz );

      hourgam1[i1] = GAMMA[i1*EIGHT+1] -  volinv*(dvdx[i3+1] * hourmodx +
                                                  dvdy[i3+1] * hourmody +
                                                  dvdz[i3+1] * hourmodz );

      hourgam2[i1] = GAMMA[i1*EIGHT+2] -  volinv*(dvdx[i3+2] * hourmodx +
                                                  dvdy[i3+2] * hourmody +
                                                  dvdz[i3+2] * hourmodz );

      hourgam3[i1] = GAMMA[i1*EIGHT+3] -  volinv*(dvdx[i3+3] * hourmodx +
                                                  dvdy[i3+3] * hourmody +
                                                  dvdz[i3+3] * hourmodz );

      hourgam4[i1] = GAMMA[i1*EIGHT+4] -  volinv*(dvdx[i3+4] * hourmodx +
                                                  dvdy[i3+4] * hourmody +
                                                  dvdz[i3+4] * hourmodz );

      hourgam5[i1] = GAMMA[i1*EIGHT+5] -  volinv*(dvdx[i3+5] * hourmodx +
                                                  dvdy[i3+5] * hourmody +
                                                  dvdz[i3+5] * hourmodz );

      hourgam6[i1] = GAMMA[i1*EIGHT+6] -  volinv*(dvdx[i3+6] * hourmodx +
                                                  dvdy[i3+6] * hourmody +
                                                  dvdz[i3+6] * hourmodz );

      hourgam7[i1] = GAMMA[i1*EIGHT+7] -  volinv*(dvdx[i3+7] * hourmodx +
                                                  dvdy[i3+7] * hourmody +
                                                  dvdz[i3+7] * hourmodz );

    } // for i1

    /* compute forces */
    /* store forces into h arrays (force arrays) */

    ss1=domain->m_ss[i2];
    mass1=domain->m_elemMass[i2];
    volume13=CBRT(determ[i2]);

// RAG ///////////////////////////////////////////////////////////// RAG //
// RAG  GATHER (x|y|z)d1[0,,7] = domain->m_(x|y|z)[elemToNode[0..7]]  RAG //
// RAG ///////////////////////////////////////////////////////////// RAG //

    for( Index_t i=0 ; i<EIGHT ; ++i ) {
      Index_t gnode = elemToNode[i];
      xd1[i] = domain->m_xd[gnode];
      yd1[i] = domain->m_yd[gnode];
      zd1[i] = domain->m_zd[gnode];
    } // for i

    coefficient = - hourg * cast_Real_t(0.01) * ss1 * mass1 / volume13;

    CalcElemFBHourglassForce(xd1,yd1,zd1,
                             hourgam0,hourgam1,hourgam2,hourgam3,
                             hourgam4,hourgam5,hourgam6,hourgam7,
                             coefficient, hgfx, hgfy, hgfz);

// RAG ///////////////////////////////////////////////////////// RAG //
// RAG  Atomic Memory Floating-point Addition Scatter operation  RAG //
// RAG ///////////////////////////////////////////////////////// RAG //

    for( Index_t i=0 ; i<EIGHT ; ++i ) {
      Index_t gnode = elemToNode[i];
      AMO__sync_addition_double(&domain->m_fx[gnode], hgfx[i]);
      AMO__sync_addition_double(&domain->m_fy[gnode], hgfy[i]);
      AMO__sync_addition_double(&domain->m_fz[gnode], hgfz[i]);
    } // for i

#if  defined(HAB_C)
    free(elemToNode);
    free(hgfz); free(hgfy); free(hgfx);
    free(hourgam7); free(hourgam6); free(hourgam5); free(hourgam4);
    free(hourgam3); free(hourgam2); free(hourgam1); free(hourgam0);
    free(zd1); free(yd1); free(xd1);
#endif //    HAB_C
  } // for i2
} // CalcFBHourglassForceForElems()

void CalcHourglassControlForElems_edt_1( Index_t i_out, Index_t i_end, SHARED struct Domain_t *domain,
                                         Real_t *determ ,
                                         Real_t *x8n  , Real_t *y8n  , Real_t *z8n  ,
                                         Real_t *dvdx , Real_t *dvdy , Real_t *dvdz ) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
#if  defined(HAB_C)
     Real_t  *x1  = (Real_t *)malloc(EIGHT*sizeof(Real_t));
     Real_t  *y1  = (Real_t *)malloc(EIGHT*sizeof(Real_t));
     Real_t  *z1  = (Real_t *)malloc(EIGHT*sizeof(Real_t));
     Real_t  *pfx = (Real_t *)malloc(EIGHT*sizeof(Real_t));
     Real_t  *pfy = (Real_t *)malloc(EIGHT*sizeof(Real_t));
     Real_t  *pfz = (Real_t *)malloc(EIGHT*sizeof(Real_t));
#else // NOT HAB_C
     Real_t  x1[EIGHT],  y1[EIGHT],  z1[EIGHT] ;
     Real_t pfx[EIGHT], pfy[EIGHT], pfz[EIGHT] ;
#endif //    HAB_C

     SHARED Index_t* elemToNode = (SHARED Index_t *) &domain->m_nodelist[EIGHT*i];
// RAG -- GATHER (x|y|z)1[0..7] = domain->m_(x|y|z)[domain->m_nodelist[0..7]]

     CollectDomainNodesToElemNodes(elemToNode, x1, y1, z1, domain);

     CalcElemVolumeDerivative(pfx, pfy, pfz, x1, y1, z1);

//TRACE6("/* load into temporary storage for FB Hour Glass control */");
     for( Index_t ii=0 ; ii<EIGHT ; ++ii ){
       Index_t jj=EIGHT*i+ii;

       dvdx[jj] = pfx[ii];
       dvdy[jj] = pfy[ii];
       dvdz[jj] = pfz[ii];
       x8n[jj]  = x1[ii];
       y8n[jj]  = y1[ii];
       z8n[jj]  = z1[ii];
     } // for ii

     determ[i] = domain->m_volo[i] * domain->m_v[i];

//TRACE6("/* Do a check for negative volumes */");
     if ( domain->m_v[i] <= cast_Real_t(0.0) ) {
       EXIT(VolumeError) ;
     }
#if  defined(HAB_C)
     free(pfz); free(pfy); free(pfx);
     free(z1) ; free(y1) ; free(x1);
#endif //    HAB_C
  } // for i
} // CalcHourglassControlForElems_edt_1()


void CalcVolumeForceForElems_edt_1( Index_t i_out , Index_t i_end ,
                                    Real_t *determ ) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
     if ( determ[i] <= cast_Real_t(0.0) ) {
       EXIT(VolumeError) ;
     }
  } // for i
} // CalcVolueForceForElems_edt_1()

void CalcForceForNodes_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
    domain->m_fx[i] = cast_Real_t(0.0) ;
    domain->m_fy[i] = cast_Real_t(0.0) ;
    domain->m_fz[i] = cast_Real_t(0.0) ;
  } // for i
} // CalcForceForNodes_edt_1()


void CalcAccelerationForNodes_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
    domain->m_xdd[i] = domain->m_fx[i] / domain->m_nodalMass[i];
//DEBUG if(i==1)fprintf(stdout,"CAFN: m_xdd %e\n",domain->m_xdd[1]);
//DEBUG if(i==1)fprintf(stdout,"CAFN: m_fx  %e\n",domain->m_fx[1]);
//DEBUG if(i==1)fprintf(stdout,"CAFN: m_nodalMass %e\n",domain->m_nodalMass[1]);
    domain->m_ydd[i] = domain->m_fy[i] / domain->m_nodalMass[i];
//DEBUG if(i==1)fprintf(stdout,"CAFN: m_ydd %e\n",domain->m_ydd[1]);
    domain->m_zdd[i] = domain->m_fz[i] / domain->m_nodalMass[i];
//DEBUG if(i==1)fprintf(stdout,"CAFN: m_zdd %e\n",domain->m_zdd[1]);
  } // for i
} // CalcAccelerationForNodes_edt_1()

void ApplyAccelerationBoundaryConditionsForNodes_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain ) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
// RAG -- SCATTER domain->m_(x|y|z)dd[domain->m_symm(X|Y|Z)[i]] = 0.0
    domain->m_xdd[domain->m_symmX[i]] = cast_Real_t(0.0) ;
    domain->m_ydd[domain->m_symmY[i]] = cast_Real_t(0.0) ;
    domain->m_zdd[domain->m_symmZ[i]] = cast_Real_t(0.0) ;
  } // for i
} // ApplyAccelerationBoundaryConditionsForNodes_edt_1()


void CalcVelocityForNodes_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain,
                                 Real_t dt, Real_t u_cut ) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
    Real_t xdtmp, ydtmp, zdtmp ;
// RAG -- DAXPY       -- (x|y|x)d += dt * (x|y|z)dd
// RAG -- DFLUSH .i.e -- -u_cut < (x|y|z)d <u_cut to 0.0
    xdtmp = domain->m_xd[i] + domain->m_xdd[i] * dt ;
    if( FABS(xdtmp) < u_cut ) xdtmp = cast_Real_t(0.0);
    domain->m_xd[i] = xdtmp ;
//DEBUG if(i==1)fprintf(stdout,"CVFN:m_xd[1]= %e\n",domain->m_xd[1]);
    ydtmp = domain->m_yd[i] + domain->m_ydd[i] * dt ;
    if( FABS(ydtmp) < u_cut ) ydtmp = cast_Real_t(0.0);
    domain->m_yd[i] = ydtmp ;
//DEBUG if(i==1)fprintf(stdout,"CVFN:m_yd[1]= %e\n",domain->m_yd[1]);
    zdtmp = domain->m_zd[i] + domain->m_zdd[i] * dt ;
    if( FABS(zdtmp) < u_cut ) zdtmp = cast_Real_t(0.0);
    domain->m_zd[i] = zdtmp ;
//DEBUG if(i==1)fprintf(stdout,"CVFN:m_zd[1]= %e\n",domain->m_zd[1]);
  } // for i
} // CalcVelocityForNodes_edt_1()

void CalcPositionForNodes_edt_1( Index_t i_out , Index_t i_end , SHARED struct Domain_t *domain,
                                 Real_t dt ) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
// RAG -- DAXPY       -- (x|y|x) += dt * (x|y|z)d
      domain->m_x[i] += domain->m_xd[i] * dt ;
//DEBUG if(i==1)fprintf(stdout,"CPFN:m_x[1]= %e\n",domain->m_x[1]);
      domain->m_y[i] += domain->m_yd[i] * dt ;
//DEBUG if(i==1)fprintf(stdout,"CPFN:m_y[1]= %e\n",domain->m_y[1]);
      domain->m_z[i] += domain->m_zd[i] * dt ;
//DEBUG if(i==1)fprintf(stdout,"CPFN:m_z[1]= %e\n",domain->m_z[1]);
  } // for i
} // CalcPositionForNodes_edt_1()

void CalcKinematicsForElems_edt_1( Index_t i_out, Index_t i_end, SHARED struct Domain_t *domain,
                                   Real_t dt ) {
  for(Index_t k = i_out ; k < i_end ; ++k ) {
#if  defined(HAB_C)
    Real_t *B        = (Real_t *)malloc(THREE*EIGHT*sizeof(Real_t)) ;
    Real_t *D        = (Real_t *)malloc(THREE*EIGHT*sizeof(Real_t)) ;
    Real_t *x_local  = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
    Real_t *y_local  = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
    Real_t *z_local  = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
    Real_t *xd_local = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
    Real_t *yd_local = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
    Real_t *zd_local = (Real_t *)malloc(EIGHT*sizeof(Real_t)) ;
    Real_t *detJ     = (Real_t *)malloc(sizeof(Real_t)) ;
    *detJ     = cast_Real_t(0.0) ;
#else // NOT HAB_C
    Real_t B[THREE*EIGHT] ; /** shape function derivatives */ /* RAG -- B[THREE][EIGHT] */
    Real_t D[SIX] ;
    Real_t x_local[EIGHT] ;
    Real_t y_local[EIGHT] ;
    Real_t z_local[EIGHT] ;
    Real_t xd_local[EIGHT] ;
    Real_t yd_local[EIGHT] ;
    Real_t zd_local[EIGHT] ;
    Real_t detJ = cast_Real_t(0.0) ;
#endif //    HAB_C
    Real_t volume ;
    Real_t relativeVolume ;
    SHARED HAB_CONST Index_t* HAB_CONST elemToNode = (SHARED Index_t *)&domain->m_nodelist[EIGHT*k] ;

    // get nodal coordinates from global arrays and copy into local arrays.
    for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode ) {
      Index_t gnode = elemToNode[lnode];
//DEBUG if(k==0)fprintf(stdout,"gnode%d= %d\n",lnode,gnode);
      x_local[lnode] = domain->m_x[gnode];
//DEBUG if(k==0)fprintf(stdout,"x_local%d= %e\n",lnode,x_local[lnode]);
      y_local[lnode] = domain->m_y[gnode];
//DEBUG if(k==0)fprintf(stdout,"y_local%d= %e\n",lnode,y_local[lnode]);
      z_local[lnode] = domain->m_z[gnode];
//DEBUG if(k==0)fprintf(stdout,"z_local%d= %e\n",lnode,z_local[lnode]);
    } // for lnode

    // volume calculations
    volume = CalcElemVolume(x_local, y_local, z_local );
//DEBUG if(k==0)fprintf(stdout,"volume= %e\n",volume);
    relativeVolume = volume / domain->m_volo[k] ;
//DEBUG if(k==0)fprintf(stdout,"relVol= %e\n",relativeVolume);
    domain->m_vnew[k] = relativeVolume ;
//DEBUG if(k==0)fprintf(stdout,"m_v   = %e\n",domain->m_v[0]);
    domain->m_delv[k] = relativeVolume - domain->m_v[k] ;
//DEBUG if(k==0)fprintf(stdout,"m_delv= %e\n",domain->m_delv[0]);

    // set characteristic length
    domain->m_arealg[k] = CalcElemCharacteristicLength(x_local,
                                                      y_local,
                                                      z_local,
                                                      volume);

    // get nodal velocities from global array and copy into local arrays.
    for( Index_t lnode=0 ; lnode<EIGHT ; ++lnode ) {
      Index_t gnode = elemToNode[lnode];
      xd_local[lnode] = domain->m_xd[gnode];
      yd_local[lnode] = domain->m_yd[gnode];
      zd_local[lnode] = domain->m_zd[gnode];
    } // for lnode

    Real_t dt2 = cast_Real_t(0.5) * dt;
    for( Index_t j=0 ; j<EIGHT ; ++j ) {
       x_local[j] -= dt2 * xd_local[j];
       y_local[j] -= dt2 * yd_local[j];
       z_local[j] -= dt2 * zd_local[j];
    } // for j

#if  defined(HAB_C)
    CalcElemShapeFunctionDerivatives( x_local,
                                      y_local,
                                      z_local,
                                      B, detJ );

    CalcElemVelocityGrandient( xd_local,
                               yd_local,
                               zd_local,
                               B, *detJ, D );
#else // NOT HAB_C
    CalcElemShapeFunctionDerivatives( x_local,
                                      y_local,
                                      z_local,
                                      B, &detJ );

    CalcElemVelocityGrandient( xd_local,
                               yd_local,
                               zd_local,
                               B, detJ, D );
#endif //    HAB_C

    // put velocity gradient quantities into their global arrays.
    domain->m_dxx[k] = D[0];
    domain->m_dyy[k] = D[1];
    domain->m_dzz[k] = D[2];
#if  defined(HAB_C)
    free(zd_local); free(yd_local); free(xd_local);
    free(z_local); free(y_local); free(x_local);
    free(D); free(B);
#endif //    HAB_C
  } // for i
} // CalcKinematicsForElems_edt_1()


void CalcLagrangeElements_edt_1( Index_t i_out, Index_t i_end, SHARED struct Domain_t *domain ) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
    // calc strain rate and apply as constraint (only done in FB element)
    Real_t vdov = domain->m_dxx[i] + domain->m_dyy[i] + domain->m_dzz[i] ;
    Real_t vdovthird = vdov/cast_Real_t(3.0) ;
        
    // make the rate of deformation tensor deviatoric
    domain->m_vdov[i] = vdov ;
    domain->m_dxx[i] -= vdovthird ;
    domain->m_dyy[i] -= vdovthird ;
    domain->m_dzz[i] -= vdovthird ;

    // See if any volumes are negative, and take appropriate action.
    if (domain->m_vnew[i] <= cast_Real_t(0.0)) {
      EXIT(VolumeError) ;
    } // if domain->m_vnew
  } // for i
} // CalcLagrangeElements_edt_1()


#define SUM4(a,b,c,d) (a + b + c + d)
void CalcMonotonicQGradientsForElems_edt_1( Index_t i_out, Index_t i_end, SHARED struct Domain_t *domain ) {
  for(Index_t i = i_out ; i < i_end ; ++i ) {
    HAB_CONST Real_t ptiny = cast_Real_t(1.e-36) ;
    Real_t ax,ay,az ;
    Real_t dxv,dyv,dzv ;

    SHARED const Index_t *elemToNode = (SHARED Index_t *)&domain->m_nodelist[EIGHT*i];
    Index_t n0 = elemToNode[0] ;
    Index_t n1 = elemToNode[1] ;
    Index_t n2 = elemToNode[2] ;
    Index_t n3 = elemToNode[3] ;
    Index_t n4 = elemToNode[4] ;
    Index_t n5 = elemToNode[5] ;
    Index_t n6 = elemToNode[6] ;
    Index_t n7 = elemToNode[7] ;

    Real_t x0 = domain->m_x[n0] ;
    Real_t x1 = domain->m_x[n1] ;
    Real_t x2 = domain->m_x[n2] ;
    Real_t x3 = domain->m_x[n3] ;
    Real_t x4 = domain->m_x[n4] ;
    Real_t x5 = domain->m_x[n5] ;
    Real_t x6 = domain->m_x[n6] ;
    Real_t x7 = domain->m_x[n7] ;

    Real_t y0 = domain->m_y[n0] ;
    Real_t y1 = domain->m_y[n1] ;
    Real_t y2 = domain->m_y[n2] ;
    Real_t y3 = domain->m_y[n3] ;
    Real_t y4 = domain->m_y[n4] ;
    Real_t y5 = domain->m_y[n5] ;
    Real_t y6 = domain->m_y[n6] ;
    Real_t y7 = domain->m_y[n7] ;

    Real_t z0 = domain->m_z[n0] ;
    Real_t z1 = domain->m_z[n1] ;
    Real_t z2 = domain->m_z[n2] ;
    Real_t z3 = domain->m_z[n3] ;
    Real_t z4 = domain->m_z[n4] ;
    Real_t z5 = domain->m_z[n5] ;
    Real_t z6 = domain->m_z[n6] ;
    Real_t z7 = domain->m_z[n7] ;

    Real_t xv0 = domain->m_xd[n0] ;
    Real_t xv1 = domain->m_xd[n1] ;
    Real_t xv2 = domain->m_xd[n2] ;
    Real_t xv3 = domain->m_xd[n3] ;
    Real_t xv4 = domain->m_xd[n4] ;
    Real_t xv5 = domain->m_xd[n5] ;
    Real_t xv6 = domain->m_xd[n6] ;
    Real_t xv7 = domain->m_xd[n7] ;

    Real_t yv0 = domain->m_yd[n0] ;
    Real_t yv1 = domain->m_yd[n1] ;
    Real_t yv2 = domain->m_yd[n2] ;
    Real_t yv3 = domain->m_yd[n3] ;
    Real_t yv4 = domain->m_yd[n4] ;
    Real_t yv5 = domain->m_yd[n5] ;
    Real_t yv6 = domain->m_yd[n6] ;
    Real_t yv7 = domain->m_yd[n7] ;

    Real_t zv0 = domain->m_zd[n0] ;
    Real_t zv1 = domain->m_zd[n1] ;
    Real_t zv2 = domain->m_zd[n2] ;
    Real_t zv3 = domain->m_zd[n3] ;
    Real_t zv4 = domain->m_zd[n4] ;
    Real_t zv5 = domain->m_zd[n5] ;
    Real_t zv6 = domain->m_zd[n6] ;
    Real_t zv7 = domain->m_zd[n7] ;

    Real_t vol = domain->m_volo[i]*domain->m_vnew[i] ;
    Real_t norm = cast_Real_t(1.0) / ( vol + ptiny ) ;

    Real_t dxj = cast_Real_t(-0.25)*(SUM4(x0,x1,x5,x4) - SUM4(x3,x2,x6,x7)) ;
    Real_t dyj = cast_Real_t(-0.25)*(SUM4(y0,y1,y5,y4) - SUM4(y3,y2,y6,y7)) ;
    Real_t dzj = cast_Real_t(-0.25)*(SUM4(z0,z1,z5,z4) - SUM4(z3,z2,z6,z7)) ;

    Real_t dxi = cast_Real_t( 0.25)*(SUM4(x1,x2,x6,x5) - SUM4(x0,x3,x7,x4)) ;
    Real_t dyi = cast_Real_t( 0.25)*(SUM4(y1,y2,y6,y5) - SUM4(y0,y3,y7,y4)) ;
    Real_t dzi = cast_Real_t( 0.25)*(SUM4(z1,z2,z6,z5) - SUM4(z0,z3,z7,z4)) ;

    Real_t dxk = cast_Real_t( 0.25)*(SUM4(x4,x5,x6,x7) - SUM4(x0,x1,x2,x3)) ;
    Real_t dyk = cast_Real_t( 0.25)*(SUM4(y4,y5,y6,y7) - SUM4(y0,y1,y2,y3)) ;
    Real_t dzk = cast_Real_t( 0.25)*(SUM4(z4,z5,z6,z7) - SUM4(z0,z1,z2,z3)) ;

    /* find delvk and delxk ( i cross j ) */

    ax = dyi*dzj - dzi*dyj ;
    ay = dzi*dxj - dxi*dzj ;
    az = dxi*dyj - dyi*dxj ;

    domain->m_delx_zeta[i] = vol / SQRT(ax*ax + ay*ay + az*az + ptiny) ;

    ax *= norm ;
    ay *= norm ;
    az *= norm ;

    dxv = cast_Real_t(0.25)*(SUM4(xv4,xv5,xv6,xv7) - SUM4(xv0,xv1,xv2,xv3)) ;
    dyv = cast_Real_t(0.25)*(SUM4(yv4,yv5,yv6,yv7) - SUM4(yv0,yv1,yv2,yv3)) ;
    dzv = cast_Real_t(0.25)*(SUM4(zv4,zv5,zv6,zv7) - SUM4(zv0,zv1,zv2,zv3)) ;

    domain->m_delv_zeta[i] = ax*dxv + ay*dyv + az*dzv ;

    /* find delxi and delvi ( j cross k ) */

    ax = dyj*dzk - dzj*dyk ;
    ay = dzj*dxk - dxj*dzk ;
    az = dxj*dyk - dyj*dxk ;

    domain->m_delx_xi[i] = vol / SQRT(ax*ax + ay*ay + az*az + ptiny) ;

    ax *= norm ;
    ay *= norm ;
    az *= norm ;

    dxv = cast_Real_t(0.25)*(SUM4(xv1,xv2,xv6,xv5) - SUM4(xv0,xv3,xv7,xv4)) ;
    dyv = cast_Real_t(0.25)*(SUM4(yv1,yv2,yv6,yv5) - SUM4(yv0,yv3,yv7,yv4)) ;
    dzv = cast_Real_t(0.25)*(SUM4(zv1,zv2,zv6,zv5) - SUM4(zv0,zv3,zv7,zv4)) ;

    domain->m_delv_xi[i] = ax*dxv + ay*dyv + az*dzv ;

    /* find delxj and delvj ( k cross i ) */

    ax = dyk*dzi - dzk*dyi ;
    ay = dzk*dxi - dxk*dzi ;
    az = dxk*dyi - dyk*dxi ;

    domain->m_delx_eta[i] = vol / SQRT(ax*ax + ay*ay + az*az + ptiny) ;

    ax *= norm ;
    ay *= norm ;
    az *= norm ;

    dxv = cast_Real_t(-0.25)*(SUM4(xv0,xv1,xv5,xv4) - SUM4(xv3,xv2,xv6,xv7)) ;
    dyv = cast_Real_t(-0.25)*(SUM4(yv0,yv1,yv5,yv4) - SUM4(yv3,yv2,yv6,yv7)) ;
    dzv = cast_Real_t(-0.25)*(SUM4(zv0,zv1,zv5,zv4) - SUM4(zv3,zv2,zv6,zv7)) ;

    domain->m_delv_eta[i] = ax*dxv + ay*dyv + az*dzv ;
  } // for i
} // CalcMonotonicQGradientsForElems_edt_1()
#undef SUM4

void CalcMonotonicQRegionForElems_edt_1( Index_t ielem_out, Index_t ielem_end, SHARED struct Domain_t *domain,
                          Real_t qlc_monoq, Real_t qqc_monoq, // parameters
                          Real_t monoq_limiter_mult, Real_t monoq_max_slope,
                          Real_t ptiny ) {   // the elementset length
  for( Index_t ielem = ielem_out ; ielem < ielem_end ; ++ielem ) {
    Real_t qlin, qquad ;
    Real_t phixi, phieta, phizeta ;
    Index_t i = domain->m_matElemlist[ielem];
    Int_t bcMask = domain->m_elemBC[i] ;
    Real_t delvm, delvp ;

    /*  phixi     */
    Real_t norm = cast_Real_t(1.) / ( domain->m_delv_xi[i] + ptiny ) ;

    switch (bcMask & XI_M) {
      case 0:         delvm = domain->m_delv_xi[domain->m_lxim[i]] ; break ;
      case XI_M_SYMM: delvm = domain->m_delv_xi[i] ;                break ;
      case XI_M_FREE: delvm = cast_Real_t(0.0) ;                   break ;
      default:        /* ERROR */ ;                                break ;
    } // switch XI_M

    switch (bcMask & XI_P) {
      case 0:         delvp = domain->m_delv_xi[domain->m_lxip[i]] ; break ;
      case XI_P_SYMM: delvp = domain->m_delv_xi[i] ;                break ;
      case XI_P_FREE: delvp = cast_Real_t(0.0) ;                   break ;
      default:        /* ERROR */ ;                                break ;
    } // switch XI_P

    delvm = delvm * norm ;
    delvp = delvp * norm ;

    phixi = cast_Real_t(.5) * ( delvm + delvp ) ;

    delvm *= monoq_limiter_mult ;
    delvp *= monoq_limiter_mult ;

    if ( delvm < phixi ) phixi = delvm ;
    if ( delvp < phixi ) phixi = delvp ;
    if ( phixi < cast_Real_t(0.)) phixi = cast_Real_t(0.) ;
    if ( phixi > monoq_max_slope) phixi = monoq_max_slope;


    /*  phieta     */
    norm = cast_Real_t(1.) / ( domain->m_delv_eta[i] + ptiny ) ;

    switch (bcMask & ETA_M) {
      case 0:          delvm = domain->m_delv_eta[domain->m_letam[i]] ; break ;
      case ETA_M_SYMM: delvm = domain->m_delv_eta[i] ;                 break ;
      case ETA_M_FREE: delvm = cast_Real_t(0.0) ;                     break ;
      default:         /* ERROR */ ;                                  break ;
    } // switch ETA_M

    switch (bcMask & ETA_P) {
      case 0:          delvp = domain->m_delv_eta[domain->m_letap[i]] ; break ;
      case ETA_P_SYMM: delvp = domain->m_delv_eta[i] ;                 break ;
      case ETA_P_FREE: delvp = cast_Real_t(0.0) ;                     break ;
      default:         /* ERROR */ ;                                  break ;
    } // switch ETA_P

    delvm = delvm * norm ;
    delvp = delvp * norm ;

    phieta = cast_Real_t(.5) * ( delvm + delvp ) ;

    delvm *= monoq_limiter_mult ;
    delvp *= monoq_limiter_mult ;

    if ( delvm  < phieta ) phieta = delvm ;
    if ( delvp  < phieta ) phieta = delvp ;
    if ( phieta < cast_Real_t(0.)) phieta = cast_Real_t(0.) ;
    if ( phieta > monoq_max_slope)  phieta = monoq_max_slope;

    /*  phizeta     */
    norm = cast_Real_t(1.) / ( domain->m_delv_zeta[i] + ptiny ) ;

    switch (bcMask & ZETA_M) {
      case 0:           delvm = domain->m_delv_zeta[domain->m_lzetam[i]] ; break ;
      case ZETA_M_SYMM: delvm = domain->m_delv_zeta[i] ;                  break ;
      case ZETA_M_FREE: delvm = cast_Real_t(0.0) ;                       break ;
      default:          /* ERROR */ ;                                    break ;
    } // switch ZETA_M

    switch (bcMask & ZETA_P) {
      case 0:           delvp = domain->m_delv_zeta[domain->m_lzetap[i]] ; break ;
      case ZETA_P_SYMM: delvp = domain->m_delv_zeta[i] ;                  break ;
      case ZETA_P_FREE: delvp = cast_Real_t(0.0) ;                       break ;
      default:          /* ERROR */ ;                                    break ;
    } // switch ZETA_P

    delvm = delvm * norm ;
    delvp = delvp * norm ;

    phizeta = cast_Real_t(.5) * ( delvm + delvp ) ;

    delvm *= monoq_limiter_mult ;
    delvp *= monoq_limiter_mult ;

    if ( delvm   < phizeta ) phizeta = delvm ;
    if ( delvp   < phizeta ) phizeta = delvp ;
    if ( phizeta < cast_Real_t(0.)) phizeta = cast_Real_t(0.);
    if ( phizeta > monoq_max_slope  ) phizeta = monoq_max_slope;

    /* Remove length scale */

    if ( domain->m_vdov[i] > cast_Real_t(0.) )  {
      qlin  = cast_Real_t(0.) ;
      qquad = cast_Real_t(0.) ;
    } else {
      Real_t delvxxi   = domain->m_delv_xi[i]   * domain->m_delx_xi[i]   ;
      Real_t delvxeta  = domain->m_delv_eta[i]  * domain->m_delx_eta[i]  ;
      Real_t delvxzeta = domain->m_delv_zeta[i] * domain->m_delx_zeta[i] ;

      if ( delvxxi   > cast_Real_t(0.) ) delvxxi   = cast_Real_t(0.) ;
      if ( delvxeta  > cast_Real_t(0.) ) delvxeta  = cast_Real_t(0.) ;
      if ( delvxzeta > cast_Real_t(0.) ) delvxzeta = cast_Real_t(0.) ;

      Real_t rho = domain->m_elemMass[i] / (domain->m_volo[i] * domain->m_vnew[i]) ;

      qlin = -qlc_monoq * rho *
           ( delvxxi   * (cast_Real_t(1.) - phixi  ) +
             delvxeta  * (cast_Real_t(1.) - phieta ) +
             delvxzeta * (cast_Real_t(1.) - phizeta) ) ;

      qquad = qqc_monoq * rho *
            (  delvxxi*delvxxi     * (cast_Real_t(1.) - phixi*phixi    ) +
               delvxeta*delvxeta   * (cast_Real_t(1.) - phieta*phieta  ) +
               delvxzeta*delvxzeta * (cast_Real_t(1.) - phizeta*phizeta) ) ;
    } // if domain->m_vdov

    domain->m_qq[i] = qquad ;
    domain->m_ql[i] = qlin  ;
  } // for ielem
} // CalcMonotonicQRegionForElems_edt_1()

void CalcQForElems_edt_1( Index_t i_out, Index_t i_end, SHARED struct Domain_t *domain,
                                         Real_t qstop, SHARED uint64_t *pIndex_AMO ) {
  for( Index_t i = i_out ; i < i_end ; ++i ) {
    if ( domain->m_q[i] > qstop ) {
      AMO__sync_fetch_and_add_uint64_t(pIndex_AMO,(uint64_t)1);
#if   !defined(CILK) && !defined(HAB_C)
        break ;
#endif // CILK
     } // if domain->m_q
  } // for i
} // CalcQForElems_edt_1()

void CalcPressureForElems_edt_1( Index_t i_out, Index_t i_end,
                          Real_t* p_new, Real_t* bvc ,
                          Real_t* pbvc, Real_t* e_old ,
                          Real_t* compression, Real_t *vnewc,
                          Real_t pmin, Real_t p_cut, Real_t eosvmax ) {
  for( Index_t i = i_out ; i < i_end ; ++i ) {
      Real_t c1s = cast_Real_t(2.0)/cast_Real_t(3.0) ;
      bvc[i]  = c1s * (compression[i] + cast_Real_t(1.));
      pbvc[i] = c1s;
//} // for i
//for( Index_t i = i_out ; i < i_end ; ++i ) {
      p_new[i] = bvc[i] * e_old[i] ;

      if    (FABS(p_new[i]) <  p_cut   ) 
         p_new[i] = cast_Real_t(0.0) ;

      if    ( vnewc[i] >= eosvmax ) /* impossible condition here? */
         p_new[i] = cast_Real_t(0.0) ;

      if    (p_new[i]       <  pmin)
         p_new[i]   = pmin ;
  } // for i
} // CalcPressureForElems_edt_1()

void CalcEnergyForElems_edt_1( Index_t i_out, Index_t i_end,
                               Real_t *e_new, Real_t *e_old,
                               Real_t *delvc, Real_t *p_old,
                               Real_t *q_old, Real_t *work,
                               Real_t emin ) {
  for( Index_t i = i_out ; i < i_end ; ++i ) {
    e_new[i] = e_old[i] - cast_Real_t(0.5) * delvc[i] * (p_old[i] + q_old[i])
             + cast_Real_t(0.5) * work[i];
    if (e_new[i]  < emin ) {
      e_new[i] = emin ;
    } // if e_new
  } // for i
} // CalcEnergyForElems_edt_1()

void CalcEnergyForElems_edt_2( Index_t i_out, Index_t i_end,
                               Real_t *compHalfStep, Real_t *q_new,
                               Real_t *qq, Real_t *ql,
                               Real_t *pbvc, Real_t *e_new,
                               Real_t *bvc, Real_t *pHalfStep,
                               Real_t *delvc, Real_t *p_old,
                               Real_t *q_old, Real_t *work,
                               Real_t rho0, Real_t e_cut, Real_t emin) {
  for( Index_t i = i_out ; i < i_end ; ++i ) {
      Real_t vhalf = cast_Real_t(1.) / (cast_Real_t(1.) + compHalfStep[i]) ;

      if ( delvc[i] > cast_Real_t(0.) ) {
        q_new[i] /* = qq[i] = ql[i] */ = cast_Real_t(0.) ;
      } else {
        Real_t ssc = ( pbvc[i] * e_new[i]
                   + vhalf * vhalf * bvc[i] * pHalfStep[i] ) / rho0 ;

        if ( ssc <= cast_Real_t(0.) ) {
          ssc =cast_Real_t(.333333e-36) ;
        } else {
          ssc = SQRT(ssc) ;
        } // if ssc

        q_new[i] = (ssc*ql[i] + qq[i]) ;
      } // if delvc

      e_new[i] +=  cast_Real_t(0.5) * delvc[i]
               * ( cast_Real_t(3.0) * (p_old[i]     + q_old[i])
                 - cast_Real_t(4.0) * (pHalfStep[i] + q_new[i])) ;

      e_new[i] += cast_Real_t(0.5) * work[i];

      if (FABS(e_new[i]) < e_cut) {
        e_new[i] = cast_Real_t(0.)  ;
      } // e_cut
      if (     e_new[i]  < emin ) {
        e_new[i] = emin ;
      } // if emin
  } // for i
} // CalcEnergyForElems_edt_2()

void CalcEnergyForElems_edt_3( Index_t i_out, Index_t i_end,
                               Real_t *delvc, Real_t *pbvc,
                               Real_t *e_new, Real_t *vnewc,
                               Real_t *bvc, Real_t *p_new,
                               Real_t *ql, Real_t *qq,
                               Real_t *p_old, Real_t *q_old,
                               Real_t *pHalfStep, Real_t *q_new,
                               Real_t rho0, Real_t e_cut, Real_t emin) {
  for( Index_t i = i_out ; i < i_end ; ++i ) {
    HAB_CONST Real_t sixth = cast_Real_t(1.0) / cast_Real_t(6.0) ;
    Real_t q_tilde ;

    if (delvc[i] > cast_Real_t(0.)) {
      q_tilde = cast_Real_t(0.) ;
    } else {
      Real_t ssc = ( pbvc[i] * e_new[i]
                 + vnewc[i] * vnewc[i] * bvc[i] * p_new[i] ) / rho0 ;

      if ( ssc <= cast_Real_t(0.) ) {
        ssc = cast_Real_t(.333333e-36) ;
      } else {
        ssc = SQRT(ssc) ;
      } // if ssc

      q_tilde = (ssc*ql[i] + qq[i]) ;
    } // if delvc

    e_new[i] += - ( cast_Real_t(7.0)*(p_old[i]     + q_old[i])
                  - cast_Real_t(8.0)*(pHalfStep[i] + q_new[i])
                  + (p_new[i] + q_tilde)) * delvc[i]*sixth ;

    if (FABS(e_new[i]) < e_cut) {
      e_new[i] = cast_Real_t(0.)  ;
    } // if e_cut
    if (     e_new[i]  < emin ) {
      e_new[i] = emin ;
    } // if emin
  } // for i
} // CalcEnergyForElems_edt_3()

void CalcEnergyForElems_edt_4( Index_t i_out, Index_t i_end,
                               Real_t *delvc, Real_t *pbvc,
                               Real_t *e_new, Real_t *vnewc,
                               Real_t *bvc, Real_t *p_new,
                               Real_t *ql, Real_t *qq,
                               Real_t *q_new,
                               Real_t rho0, Real_t q_cut) {
  for( Index_t i = i_out ; i < i_end ; ++i ) {
    if ( delvc[i] <= cast_Real_t(0.) ) {
      Real_t ssc = (  pbvc[i] * e_new[i]
                   + vnewc[i] * vnewc[i] * bvc[i] * p_new[i] ) / rho0 ;

      if ( ssc <= cast_Real_t(0.) ) {
        ssc = cast_Real_t(.333333e-36) ;
      } else {
        ssc = SQRT(ssc) ;
      } // if ssc

      q_new[i] = (ssc*ql[i] + qq[i]) ;

      if (FABS(q_new[i]) < q_cut) q_new[i] = cast_Real_t(0.) ;
    } // if delvc
  } // for i
} // CalcEnergyForElems_edt_4()

void CalcSoundSpeedForElems_edt_1( Index_t i_out, Index_t i_end, SHARED struct Domain_t *domain,
                                   Real_t *vnewc, Real_t *enewc,
                                   Real_t *pnewc, Real_t *pbvc,
                                   Real_t *bvc, 
                                   Real_t rho0 ) {
  for( Index_t i = i_out ; i < i_end ; ++i ) {
    Index_t iz = domain->m_matElemlist[i];
    Real_t ssTmp = (pbvc[i] * enewc[i] + vnewc[i] * vnewc[i] *
                     bvc[i] * pnewc[i]) / rho0;
    if (ssTmp <= cast_Real_t(1.111111e-36)) {
      ssTmp = cast_Real_t(1.111111e-36);
    } // if ssTmp
    domain->m_ss[iz] = SQRT(ssTmp); // RAG -- SCATTER OPERATION
  } // for i
} // CalcSoundSpeedForElems_edt_1()

void EvalEOSForElems_edt_1( Index_t i_out, Index_t i_end, SHARED struct Domain_t *domain,
                            Real_t *delvc, Real_t *e_old,
                            Real_t *p_old, Real_t *q_old,
                            Real_t *qq, Real_t *ql) {
  for( Index_t i = i_out ; i < i_end ; ++i ) { // RAG GATHERS
    Index_t zidx = domain->m_matElemlist[i] ;
    e_old[i] = domain->m_e[zidx] ;
    delvc[i] = domain->m_delv[zidx] ;
    p_old[i] = domain->m_p[zidx] ;
    q_old[i] = domain->m_q[zidx] ;
    qq[i]    = domain->m_qq[zidx] ;
    ql[i]    = domain->m_ql[zidx] ;
  } // for i
} // EvalEOSForElems_edt_1()

void EvalEOSForElems_edt_2( Index_t i_out, Index_t i_end,
                            Real_t *compression, Real_t *vnewc,
                            Real_t *delvc, Real_t *compHalfStep,
                            Real_t *work, Real_t *p_old,
                            Real_t eosvmin, Real_t eosvmax ) {
  for( Index_t i = i_out ; i < i_end ; ++i ) { // RAG STRIDE ONE
//  PAR_FOR_0xNx1(i,length,compression,vnewc,delvc,compHalfStep,work,p_old,eosvmin,eosvmax)
    Real_t vchalf ;
    compression[i] = cast_Real_t(1.) / vnewc[i] - cast_Real_t(1.);
    vchalf = vnewc[i] - delvc[i] * cast_Real_t(.5);
    compHalfStep[i] = cast_Real_t(1.) / vchalf - cast_Real_t(1.);
    work[i] = cast_Real_t(0.) ; 
//  END_PAR_FOR(i)

    if ( eosvmin != cast_Real_t(0.) ) {
//  PAR_FOR_0xNx1(i,length,compression,vnewc,delvc,compHalfStep,work,p_old,eosvmin,eosvmax)
      if (vnewc[i] <= eosvmin) { /* impossible due to calling func? */
        compHalfStep[i] = compression[i] ;
      } // if vnewc
//  END_PAR_FOR(i)
    } // if eosvmin

    if ( eosvmax != cast_Real_t(0.) ) {
//  PAR_FOR_0xNx1(i,length,compression,vnewc,delvc,compHalfStep,work,p_old,eosvmin,eosvmax)
      if (vnewc[i] >= eosvmax) { /* impossible due to calling func? */
        p_old[i]        = cast_Real_t(0.) ;
        compression[i]  = cast_Real_t(0.) ;
        compHalfStep[i] = cast_Real_t(0.) ;
      } // if vnewc
//  END_PAR_FOR(i)
     } // if eosvmax
  } // for i
} // EvalEOSForElems_edt_2()

void EvalEOSForElems_edt_3( Index_t i_out, Index_t i_end, SHARED struct Domain_t *domain,
                            Real_t *p_new, Real_t *e_new,
                            Real_t *q_new ) {
  for( Index_t i = i_out ; i < i_end ; ++i ) { // RAG SCATTERS
    Index_t zidx = domain->m_matElemlist[i] ;
    domain->m_p[zidx] = p_new[i] ;
    domain->m_e[zidx] = e_new[i] ;
    domain->m_q[zidx] = q_new[i] ;
  } // for i
} // EvalEOSForElems_edt_3()
