static INLINE
void TimeIncrement( SHARED struct Domain_t *domain) {
  Real_t targetdt = domain->m_stoptime - domain->m_time ;

  if ((domain->m_dtfixed <= cast_Real_t(0.0)) && (domain->m_cycle != cast_Int_t(0))) {
    Real_t ratio ;
    Real_t olddt = domain->m_deltatime ;

TRACE1("/* This will require a reduction in parallel */");

    Real_t newdt = cast_Real_t(1.0e+20) ;
    if (domain->m_dtcourant < newdt) {
      newdt = domain->m_dtcourant / cast_Real_t(2.0) ;
    }
    if (domain->m_dthydro < newdt) {
      newdt = domain->m_dthydro * cast_Real_t(2.0) / cast_Real_t(3.0) ;
    }

    ratio = newdt / olddt ;
    if (ratio >= cast_Real_t(1.0)) {
      if (ratio < domain->m_deltatimemultlb) {
        newdt = olddt ;
      } else if (ratio > domain->m_deltatimemultub) {
        newdt = olddt*domain->m_deltatimemultub ;
      }
    }

    if (newdt > domain->m_dtmax) {
      newdt = domain->m_dtmax ;
    }
    domain->m_deltatime = newdt ;
   }

TRACE1("/* TRY TO PREVENT VERY SMALL SCALING ON THE NEXT CYCLE */");

   if ((targetdt > domain->m_deltatime) &&
       (targetdt < (cast_Real_t(4.0) * domain->m_deltatime / cast_Real_t(3.0))) ) {
      targetdt = cast_Real_t(2.0) * domain->m_deltatime / cast_Real_t(3.0) ;
   }

   if (targetdt < domain->m_deltatime) {
      domain->m_deltatime = targetdt ;
   }

   domain->m_time += domain->m_deltatime ;

   ++domain->m_cycle ;
} // TimeIncrement()

static INLINE
void CalcElemShapeFunctionDerivatives( const Real_t *const x,   /* IN */
                                       const Real_t *const y,   /* IN */
                                       const Real_t *const z,   /* IN */
                                       Real_t *b,               /* OUT b[][EIGHT] */
                                       Real_t *const volume ) { /* OUT */
  const Real_t x0 = x[0] ;   const Real_t x1 = x[1] ;
  const Real_t x2 = x[2] ;   const Real_t x3 = x[3] ;
  const Real_t x4 = x[4] ;   const Real_t x5 = x[5] ;
  const Real_t x6 = x[6] ;   const Real_t x7 = x[7] ;

  const Real_t y0 = y[0] ;   const Real_t y1 = y[1] ;
  const Real_t y2 = y[2] ;   const Real_t y3 = y[3] ;
  const Real_t y4 = y[4] ;   const Real_t y5 = y[5] ;
  const Real_t y6 = y[6] ;   const Real_t y7 = y[7] ;

  const Real_t z0 = z[0] ;   const Real_t z1 = z[1] ;
  const Real_t z2 = z[2] ;   const Real_t z3 = z[3] ;
  const Real_t z4 = z[4] ;   const Real_t z5 = z[5] ;
  const Real_t z6 = z[6] ;   const Real_t z7 = z[7] ;

  Real_t fjxxi, fjxet, fjxze;
  Real_t fjyxi, fjyet, fjyze;
  Real_t fjzxi, fjzet, fjzze;
  Real_t cjxxi, cjxet, cjxze;
  Real_t cjyxi, cjyet, cjyze;
  Real_t cjzxi, cjzet, cjzze;

  fjxxi = .125 * ( (x6-x0) + (x5-x3) - (x7-x1) - (x4-x2) );
  fjxet = .125 * ( (x6-x0) - (x5-x3) + (x7-x1) - (x4-x2) );
  fjxze = .125 * ( (x6-x0) + (x5-x3) + (x7-x1) + (x4-x2) );

  fjyxi = .125 * ( (y6-y0) + (y5-y3) - (y7-y1) - (y4-y2) );
  fjyet = .125 * ( (y6-y0) - (y5-y3) + (y7-y1) - (y4-y2) );
  fjyze = .125 * ( (y6-y0) + (y5-y3) + (y7-y1) + (y4-y2) );

  fjzxi = .125 * ( (z6-z0) + (z5-z3) - (z7-z1) - (z4-z2) );
  fjzet = .125 * ( (z6-z0) - (z5-z3) + (z7-z1) - (z4-z2) );
  fjzze = .125 * ( (z6-z0) + (z5-z3) + (z7-z1) + (z4-z2) );

  /* compute cofactors */
  cjxxi =    (fjyet * fjzze) - (fjzet * fjyze);
  cjxet =  - (fjyxi * fjzze) + (fjzxi * fjyze);
  cjxze =    (fjyxi * fjzet) - (fjzxi * fjyet);

  cjyxi =  - (fjxet * fjzze) + (fjzet * fjxze);
  cjyet =    (fjxxi * fjzze) - (fjzxi * fjxze);
  cjyze =  - (fjxxi * fjzet) + (fjzxi * fjxet);

  cjzxi =    (fjxet * fjyze) - (fjyet * fjxze);
  cjzet =  - (fjxxi * fjyze) + (fjyxi * fjxze);
  cjzze =    (fjxxi * fjyet) - (fjyxi * fjxet);

  /* calculate partials :
     this need only be done for l = 0,1,2,3   since , by symmetry ,
     (6,7,4,5) = - (0,1,2,3) .
  */
  b[0*EIGHT+0] = -  cjxxi  -  cjxet  -  cjxze;
  b[0*EIGHT+1] =    cjxxi  -  cjxet  -  cjxze;
  b[0*EIGHT+2] =    cjxxi  +  cjxet  -  cjxze;
  b[0*EIGHT+3] = -  cjxxi  +  cjxet  -  cjxze;
  b[0*EIGHT+4] = -b[0*EIGHT+2];
  b[0*EIGHT+5] = -b[0*EIGHT+3];
  b[0*EIGHT+6] = -b[0*EIGHT+0];
  b[0*EIGHT+7] = -b[0*EIGHT+1];

  b[1*EIGHT+0] = -  cjyxi  -  cjyet  -  cjyze;
  b[1*EIGHT+1] =    cjyxi  -  cjyet  -  cjyze;
  b[1*EIGHT+2] =    cjyxi  +  cjyet  -  cjyze;
  b[1*EIGHT+3] = -  cjyxi  +  cjyet  -  cjyze;
  b[1*EIGHT+4] = -b[1*EIGHT+2];
  b[1*EIGHT+5] = -b[1*EIGHT+3];
  b[1*EIGHT+6] = -b[1*EIGHT+0];
  b[1*EIGHT+7] = -b[1*EIGHT+1];

  b[2*EIGHT+0] = -  cjzxi  -  cjzet  -  cjzze;
  b[2*EIGHT+1] =    cjzxi  -  cjzet  -  cjzze;
  b[2*EIGHT+2] =    cjzxi  +  cjzet  -  cjzze;
  b[2*EIGHT+3] = -  cjzxi  +  cjzet  -  cjzze;
  b[2*EIGHT+4] = -b[2*EIGHT+2];
  b[2*EIGHT+5] = -b[2*EIGHT+3];
  b[2*EIGHT+6] = -b[2*EIGHT+0];
  b[2*EIGHT+7] = -b[2*EIGHT+1];

  /* calculate jacobian determinant (volume) */
  *volume = cast_Real_t(8.) * ( fjxet * cjxet + fjyet * cjyet + fjzet * cjzet);
} // CalcElemShapeFunctionDerivatives()

static INLINE
void SumElemFaceNormal(Real_t *normalX0, Real_t *normalY0, Real_t *normalZ0,
                       Real_t *normalX1, Real_t *normalY1, Real_t *normalZ1,
                       Real_t *normalX2, Real_t *normalY2, Real_t *normalZ2,
                       Real_t *normalX3, Real_t *normalY3, Real_t *normalZ3,
                       const Real_t x0, const Real_t y0, const Real_t z0,
                       const Real_t x1, const Real_t y1, const Real_t z1,
                       const Real_t x2, const Real_t y2, const Real_t z2,
                       const Real_t x3, const Real_t y3, const Real_t z3) {
   Real_t bisectX0 = cast_Real_t(0.5) * (x3 + x2 - x1 - x0);
   Real_t bisectY0 = cast_Real_t(0.5) * (y3 + y2 - y1 - y0);
   Real_t bisectZ0 = cast_Real_t(0.5) * (z3 + z2 - z1 - z0);
   Real_t bisectX1 = cast_Real_t(0.5) * (x2 + x1 - x3 - x0);
   Real_t bisectY1 = cast_Real_t(0.5) * (y2 + y1 - y3 - y0);
   Real_t bisectZ1 = cast_Real_t(0.5) * (z2 + z1 - z3 - z0);
   Real_t areaX = cast_Real_t(0.25) * (bisectY0 * bisectZ1 - bisectZ0 * bisectY1);
   Real_t areaY = cast_Real_t(0.25) * (bisectZ0 * bisectX1 - bisectX0 * bisectZ1);
   Real_t areaZ = cast_Real_t(0.25) * (bisectX0 * bisectY1 - bisectY0 * bisectX1);

   *normalX0 += areaX;
   *normalX1 += areaX;
   *normalX2 += areaX;
   *normalX3 += areaX;

   *normalY0 += areaY;
   *normalY1 += areaY;
   *normalY2 += areaY;
   *normalY3 += areaY;

   *normalZ0 += areaZ;
   *normalZ1 += areaZ;
   *normalZ2 += areaZ;
   *normalZ3 += areaZ;
} // SumElemFaceNormal()

static INLINE
void CalcElemNodeNormals(Real_t pfx[EIGHT],       /* OUT */
                         Real_t pfy[EIGHT],       /* OUT */
                         Real_t pfz[EIGHT],       /* OUT */
                         const Real_t x[EIGHT],   /* IN */
                         const Real_t y[EIGHT],   /* IN */
                         const Real_t z[EIGHT]) { /* IN */
  for( Index_t i = 0 ; i < EIGHT ; ++i ) {
    pfx[i] = cast_Real_t(0.0);
    pfy[i] = cast_Real_t(0.0);
    pfz[i] = cast_Real_t(0.0);
  }
  /* evaluate face one: nodes 0, 1, 2, 3 */
  SumElemFaceNormal(&pfx[0], &pfy[0], &pfz[0],
                    &pfx[1], &pfy[1], &pfz[1],
                    &pfx[2], &pfy[2], &pfz[2],
                    &pfx[3], &pfy[3], &pfz[3],
                    x[0], y[0], z[0], x[1], y[1], z[1],
                    x[2], y[2], z[2], x[3], y[3], z[3]);
  /* evaluate face two: nodes 0, 4, 5, 1 */
  SumElemFaceNormal(&pfx[0], &pfy[0], &pfz[0],
                    &pfx[4], &pfy[4], &pfz[4],
                    &pfx[5], &pfy[5], &pfz[5],
                    &pfx[1], &pfy[1], &pfz[1],
                    x[0], y[0], z[0], x[4], y[4], z[4],
                    x[5], y[5], z[5], x[1], y[1], z[1]);
  /* evaluate face three: nodes 1, 5, 6, 2 */
  SumElemFaceNormal(&pfx[1], &pfy[1], &pfz[1],
                    &pfx[5], &pfy[5], &pfz[5],
                    &pfx[6], &pfy[6], &pfz[6],
                    &pfx[2], &pfy[2], &pfz[2],
                    x[1], y[1], z[1], x[5], y[5], z[5],
                    x[6], y[6], z[6], x[2], y[2], z[2]);
  /* evaluate face four: nodes 2, 6, 7, 3 */
  SumElemFaceNormal(&pfx[2], &pfy[2], &pfz[2],
                    &pfx[6], &pfy[6], &pfz[6],
                    &pfx[7], &pfy[7], &pfz[7],
                    &pfx[3], &pfy[3], &pfz[3],
                    x[2], y[2], z[2], x[6], y[6], z[6],
                    x[7], y[7], z[7], x[3], y[3], z[3]);
  /* evaluate face five: nodes 3, 7, 4, 0 */
  SumElemFaceNormal(&pfx[3], &pfy[3], &pfz[3],
                    &pfx[7], &pfy[7], &pfz[7],
                    &pfx[4], &pfy[4], &pfz[4],
                    &pfx[0], &pfy[0], &pfz[0],
                    x[3], y[3], z[3], x[7], y[7], z[7],
                    x[4], y[4], z[4], x[0], y[0], z[0]);
  /* evaluate face six: nodes 4, 7, 6, 5 */
  SumElemFaceNormal(&pfx[4], &pfy[4], &pfz[4],
                    &pfx[7], &pfy[7], &pfz[7],
                    &pfx[6], &pfy[6], &pfz[6],
                    &pfx[5], &pfy[5], &pfz[5],
                    x[4], y[4], z[4], x[7], y[7], z[7],
                    x[6], y[6], z[6], x[5], y[5], z[5]);
} // CalcElemNodeNormals()

static INLINE
void SumElemStressesToNodeForces( HC_UPC_CONST Real_t *B,  /* IN   B[][EIGHT] */
                                  const Real_t stress_xx,  /* IN */
                                  const Real_t stress_yy,  /* IN */
                                  const Real_t stress_zz,  /* IN */
                                  Real_t* const fx,        /* OUT */
                                  Real_t* const fy,        /* OUT */
                                  Real_t* const fz ) {     /* OUT */
  Real_t pfx0 = B[0*EIGHT+0] ;   Real_t pfx1 = B[0*EIGHT+1] ;
  Real_t pfx2 = B[0*EIGHT+2] ;   Real_t pfx3 = B[0*EIGHT+3] ;
  Real_t pfx4 = B[0*EIGHT+4] ;   Real_t pfx5 = B[0*EIGHT+5] ;
  Real_t pfx6 = B[0*EIGHT+6] ;   Real_t pfx7 = B[0*EIGHT+7] ;

  Real_t pfy0 = B[1*EIGHT+0] ;   Real_t pfy1 = B[1*EIGHT+1] ;
  Real_t pfy2 = B[1*EIGHT+2] ;   Real_t pfy3 = B[1*EIGHT+3] ;
  Real_t pfy4 = B[1*EIGHT+4] ;   Real_t pfy5 = B[1*EIGHT+5] ;
  Real_t pfy6 = B[1*EIGHT+6] ;   Real_t pfy7 = B[1*EIGHT+7] ;

  Real_t pfz0 = B[2*EIGHT+0] ;   Real_t pfz1 = B[2*EIGHT+1] ;
  Real_t pfz2 = B[2*EIGHT+2] ;   Real_t pfz3 = B[2*EIGHT+3] ;
  Real_t pfz4 = B[2*EIGHT+4] ;   Real_t pfz5 = B[2*EIGHT+5] ;
  Real_t pfz6 = B[2*EIGHT+6] ;   Real_t pfz7 = B[2*EIGHT+7] ;

  fx[0] = -( stress_xx * pfx0 );
  fx[1] = -( stress_xx * pfx1 );
  fx[2] = -( stress_xx * pfx2 );
  fx[3] = -( stress_xx * pfx3 );
  fx[4] = -( stress_xx * pfx4 );
  fx[5] = -( stress_xx * pfx5 );
  fx[6] = -( stress_xx * pfx6 );
  fx[7] = -( stress_xx * pfx7 );

  fy[0] = -( stress_yy * pfy0  );
  fy[1] = -( stress_yy * pfy1  );
  fy[2] = -( stress_yy * pfy2  );
  fy[3] = -( stress_yy * pfy3  );
  fy[4] = -( stress_yy * pfy4  );
  fy[5] = -( stress_yy * pfy5  );
  fy[6] = -( stress_yy * pfy6  );
  fy[7] = -( stress_yy * pfy7  );

  fz[0] = -( stress_zz * pfz0 );
  fz[1] = -( stress_zz * pfz1 );
  fz[2] = -( stress_zz * pfz2 );
  fz[3] = -( stress_zz * pfz3 );
  fz[4] = -( stress_zz * pfz4 );
  fz[5] = -( stress_zz * pfz5 );
  fz[6] = -( stress_zz * pfz6 );
  fz[7] = -( stress_zz * pfz7 );
} // SumElemStressesToNodeForces()

static INLINE
void CollectDomainNodesToElemNodes(SHARED const Index_t* elemToNode,
                                   Real_t elemX[EIGHT],
                                   Real_t elemY[EIGHT],
                                   Real_t elemZ[EIGHT],
                                   SHARED struct Domain_t *domain) {
// RAG -- nd(0..7)i = elemToNode[0..7]
  Index_t nd0i = elemToNode[0] ;
  Index_t nd1i = elemToNode[1] ;
  Index_t nd2i = elemToNode[2] ;
  Index_t nd3i = elemToNode[3] ;
  Index_t nd4i = elemToNode[4] ;
  Index_t nd5i = elemToNode[5] ;
  Index_t nd6i = elemToNode[6] ;
  Index_t nd7i = elemToNode[7] ;
// RAG -- GATHER elemX[0..7] = domain->m_x[elemToNode[0..7]]
  elemX[0] = domain->m_x[nd0i];
  elemX[1] = domain->m_x[nd1i];
  elemX[2] = domain->m_x[nd2i];
  elemX[3] = domain->m_x[nd3i];
  elemX[4] = domain->m_x[nd4i];
  elemX[5] = domain->m_x[nd5i];
  elemX[6] = domain->m_x[nd6i];
  elemX[7] = domain->m_x[nd7i];
// RAG -- GATHER elemY[0..7] = domain->m_y[elemToNode[0..7]]
  elemY[0] = domain->m_y[nd0i];
  elemY[1] = domain->m_y[nd1i];
  elemY[2] = domain->m_y[nd2i];
  elemY[3] = domain->m_y[nd3i];
  elemY[4] = domain->m_y[nd4i];
  elemY[5] = domain->m_y[nd5i];
  elemY[6] = domain->m_y[nd6i];
  elemY[7] = domain->m_y[nd7i];
// RAG -- GATHER elemZ[0..7] = domain->m_z[elemToNode[0..7]]
  elemZ[0] = domain->m_z[nd0i];
  elemZ[1] = domain->m_z[nd1i];
  elemZ[2] = domain->m_z[nd2i];
  elemZ[3] = domain->m_z[nd3i];
  elemZ[4] = domain->m_z[nd4i];
  elemZ[5] = domain->m_z[nd5i];
  elemZ[6] = domain->m_z[nd6i];
  elemZ[7] = domain->m_z[nd7i];
} // CollectDomainNodesToElemNodes()

static INLINE
void VoluDer(const Real_t x0, const Real_t x1, const Real_t x2, /* IN */
             const Real_t x3, const Real_t x4, const Real_t x5, /* IN */
             const Real_t y0, const Real_t y1, const Real_t y2, /* IN */
             const Real_t y3, const Real_t y4, const Real_t y5, /* IN */
             const Real_t z0, const Real_t z1, const Real_t z2, /* IN */
             const Real_t z3, const Real_t z4, const Real_t z5, /* IN */
             Real_t* dvdx, Real_t* dvdy, Real_t* dvdz) {        /* OUT */
  const Real_t twelfth = cast_Real_t(1.0) / cast_Real_t(12.0) ;

  *dvdx = + (y1 + y2) * (z0 + z1) - (y0 + y1) * (z1 + z2)
          + (y0 + y4) * (z3 + z4) - (y3 + y4) * (z0 + z4)
          - (y2 + y5) * (z3 + z5) + (y3 + y5) * (z2 + z5);

  *dvdy = - (x1 + x2) * (z0 + z1) + (x0 + x1) * (z1 + z2)
          - (x0 + x4) * (z3 + z4) + (x3 + x4) * (z0 + z4)
          + (x2 + x5) * (z3 + z5) - (x3 + x5) * (z2 + z5);

  *dvdz = - (y1 + y2) * (x0 + x1) + (y0 + y1) * (x1 + x2)
          - (y0 + y4) * (x3 + x4) + (y3 + y4) * (x0 + x4)
          + (y2 + y5) * (x3 + x5) - (y3 + y5) * (x2 + x5);

  *dvdx *= twelfth;
  *dvdy *= twelfth;
  *dvdz *= twelfth;
} // VoluDer()

static INLINE
void CalcElemVolumeDerivative(Real_t dvdx[EIGHT],      /* OUT */
                              Real_t dvdy[EIGHT],      /* OUT */
                              Real_t dvdz[EIGHT],      /* OUT */
                              const Real_t x[EIGHT],   /* IN */
                              const Real_t y[EIGHT],   /* IN */
                              const Real_t z[EIGHT]) { /* IN */
   VoluDer(x[1], x[2], x[3], x[4], x[5], x[7],
           y[1], y[2], y[3], y[4], y[5], y[7],
           z[1], z[2], z[3], z[4], z[5], z[7],
           &dvdx[0], &dvdy[0], &dvdz[0]);
   VoluDer(x[0], x[1], x[2], x[7], x[4], x[6],
           y[0], y[1], y[2], y[7], y[4], y[6],
           z[0], z[1], z[2], z[7], z[4], z[6],
           &dvdx[3], &dvdy[3], &dvdz[3]);
   VoluDer(x[3], x[0], x[1], x[6], x[7], x[5],
           y[3], y[0], y[1], y[6], y[7], y[5],
           z[3], z[0], z[1], z[6], z[7], z[5],
           &dvdx[2], &dvdy[2], &dvdz[2]);
   VoluDer(x[2], x[3], x[0], x[5], x[6], x[4],
           y[2], y[3], y[0], y[5], y[6], y[4],
           z[2], z[3], z[0], z[5], z[6], z[4],
           &dvdx[1], &dvdy[1], &dvdz[1]);
   VoluDer(x[7], x[6], x[5], x[0], x[3], x[1],
           y[7], y[6], y[5], y[0], y[3], y[1],
           z[7], z[6], z[5], z[0], z[3], z[1],
           &dvdx[4], &dvdy[4], &dvdz[4]);
   VoluDer(x[4], x[7], x[6], x[1], x[0], x[2],
           y[4], y[7], y[6], y[1], y[0], y[2],
           z[4], z[7], z[6], z[1], z[0], z[2],
           &dvdx[5], &dvdy[5], &dvdz[5]);
   VoluDer(x[5], x[4], x[7], x[2], x[1], x[3],
           y[5], y[4], y[7], y[2], y[1], y[3],
           z[5], z[4], z[7], z[2], z[1], z[3],
           &dvdx[6], &dvdy[6], &dvdz[6]);
   VoluDer(x[6], x[5], x[4], x[3], x[2], x[0],
           y[6], y[5], y[4], y[3], y[2], y[0],
           z[6], z[5], z[4], z[3], z[2], z[0],
           &dvdx[7], &dvdy[7], &dvdz[7]);
} // CalcElemVolumeDerivative()

static INLINE
void CalcElemFBHourglassForce(Real_t *xd, Real_t *yd, Real_t *zd,          /* IN */
                              Real_t *hourgam0, Real_t *hourgam1,          /* IN */
                              Real_t *hourgam2, Real_t *hourgam3,          /* IN */
                              Real_t *hourgam4, Real_t *hourgam5,          /* IN */
                              Real_t *hourgam6, Real_t *hourgam7,          /* IN */
                              Real_t coefficient,                          /* IN */
                              Real_t *hgfx, Real_t *hgfy, Real_t *hgfz ) { /* OUT */
// RAG --   h(x|y|z)[0][0..3] =        MatMul( d(x|y|z)[0][0..7] , hourgam [0..7][0..3] )
// RAG -- hgf(x|y|x)[0][0..7] = coef * MatMul( h(x|y|z)[0][0..3] , hourgamT[0..7][0..3] )
  Index_t i00=0;
  Index_t i01=1;
  Index_t i02=2;
  Index_t i03=3;
  Real_t h00 =
      hourgam0[i00] * xd[0] + hourgam1[i00] * xd[1] +
      hourgam2[i00] * xd[2] + hourgam3[i00] * xd[3] +
      hourgam4[i00] * xd[4] + hourgam5[i00] * xd[5] +
      hourgam6[i00] * xd[6] + hourgam7[i00] * xd[7];

  Real_t h01 =
      hourgam0[i01] * xd[0] + hourgam1[i01] * xd[1] +
      hourgam2[i01] * xd[2] + hourgam3[i01] * xd[3] +
      hourgam4[i01] * xd[4] + hourgam5[i01] * xd[5] +
      hourgam6[i01] * xd[6] + hourgam7[i01] * xd[7];

  Real_t h02 =
      hourgam0[i02] * xd[0] + hourgam1[i02] * xd[1]+
      hourgam2[i02] * xd[2] + hourgam3[i02] * xd[3]+
      hourgam4[i02] * xd[4] + hourgam5[i02] * xd[5]+
      hourgam6[i02] * xd[6] + hourgam7[i02] * xd[7];

  Real_t h03 =
      hourgam0[i03] * xd[0] + hourgam1[i03] * xd[1] +
      hourgam2[i03] * xd[2] + hourgam3[i03] * xd[3] +
      hourgam4[i03] * xd[4] + hourgam5[i03] * xd[5] +
      hourgam6[i03] * xd[6] + hourgam7[i03] * xd[7];

  hgfx[0] = coefficient *
      (hourgam0[i00] * h00 + hourgam0[i01] * h01 +
       hourgam0[i02] * h02 + hourgam0[i03] * h03);

  hgfx[1] = coefficient *
      (hourgam1[i00] * h00 + hourgam1[i01] * h01 +
       hourgam1[i02] * h02 + hourgam1[i03] * h03);

  hgfx[2] = coefficient *
      (hourgam2[i00] * h00 + hourgam2[i01] * h01 +
       hourgam2[i02] * h02 + hourgam2[i03] * h03);

  hgfx[3] = coefficient *
      (hourgam3[i00] * h00 + hourgam3[i01] * h01 +
       hourgam3[i02] * h02 + hourgam3[i03] * h03);

  hgfx[4] = coefficient *
      (hourgam4[i00] * h00 + hourgam4[i01] * h01 +
       hourgam4[i02] * h02 + hourgam4[i03] * h03);

  hgfx[5] = coefficient *
      (hourgam5[i00] * h00 + hourgam5[i01] * h01 +
       hourgam5[i02] * h02 + hourgam5[i03] * h03);

  hgfx[6] = coefficient *
      (hourgam6[i00] * h00 + hourgam6[i01] * h01 +
       hourgam6[i02] * h02 + hourgam6[i03] * h03);

  hgfx[7] = coefficient *
      (hourgam7[i00] * h00 + hourgam7[i01] * h01 +
       hourgam7[i02] * h02 + hourgam7[i03] * h03);

  h00 =
      hourgam0[i00] * yd[0] + hourgam1[i00] * yd[1] +
      hourgam2[i00] * yd[2] + hourgam3[i00] * yd[3] +
      hourgam4[i00] * yd[4] + hourgam5[i00] * yd[5] +
      hourgam6[i00] * yd[6] + hourgam7[i00] * yd[7];

  h01 =
      hourgam0[i01] * yd[0] + hourgam1[i01] * yd[1] +
      hourgam2[i01] * yd[2] + hourgam3[i01] * yd[3] +
      hourgam4[i01] * yd[4] + hourgam5[i01] * yd[5] +
      hourgam6[i01] * yd[6] + hourgam7[i01] * yd[7];

  h02 =
      hourgam0[i02] * yd[0] + hourgam1[i02] * yd[1]+
      hourgam2[i02] * yd[2] + hourgam3[i02] * yd[3]+
      hourgam4[i02] * yd[4] + hourgam5[i02] * yd[5]+
      hourgam6[i02] * yd[6] + hourgam7[i02] * yd[7];

  h03 =
      hourgam0[i03] * yd[0] + hourgam1[i03] * yd[1] +
      hourgam2[i03] * yd[2] + hourgam3[i03] * yd[3] +
      hourgam4[i03] * yd[4] + hourgam5[i03] * yd[5] +
      hourgam6[i03] * yd[6] + hourgam7[i03] * yd[7];


  hgfy[0] = coefficient *
      (hourgam0[i00] * h00 + hourgam0[i01] * h01 +
       hourgam0[i02] * h02 + hourgam0[i03] * h03);

  hgfy[1] = coefficient *
      (hourgam1[i00] * h00 + hourgam1[i01] * h01 +
       hourgam1[i02] * h02 + hourgam1[i03] * h03);

  hgfy[2] = coefficient *
      (hourgam2[i00] * h00 + hourgam2[i01] * h01 +
       hourgam2[i02] * h02 + hourgam2[i03] * h03);

  hgfy[3] = coefficient *
      (hourgam3[i00] * h00 + hourgam3[i01] * h01 +
       hourgam3[i02] * h02 + hourgam3[i03] * h03);

  hgfy[4] = coefficient *
      (hourgam4[i00] * h00 + hourgam4[i01] * h01 +
       hourgam4[i02] * h02 + hourgam4[i03] * h03);

  hgfy[5] = coefficient *
      (hourgam5[i00] * h00 + hourgam5[i01] * h01 +
       hourgam5[i02] * h02 + hourgam5[i03] * h03);

  hgfy[6] = coefficient *
      (hourgam6[i00] * h00 + hourgam6[i01] * h01 +
       hourgam6[i02] * h02 + hourgam6[i03] * h03);

  hgfy[7] = coefficient *
      (hourgam7[i00] * h00 + hourgam7[i01] * h01 +
       hourgam7[i02] * h02 + hourgam7[i03] * h03);

  h00 =
      hourgam0[i00] * zd[0] + hourgam1[i00] * zd[1] +
      hourgam2[i00] * zd[2] + hourgam3[i00] * zd[3] +
      hourgam4[i00] * zd[4] + hourgam5[i00] * zd[5] +
      hourgam6[i00] * zd[6] + hourgam7[i00] * zd[7];

  h01 =
      hourgam0[i01] * zd[0] + hourgam1[i01] * zd[1] +
      hourgam2[i01] * zd[2] + hourgam3[i01] * zd[3] +
      hourgam4[i01] * zd[4] + hourgam5[i01] * zd[5] +
      hourgam6[i01] * zd[6] + hourgam7[i01] * zd[7];

  h02 =
      hourgam0[i02] * zd[0] + hourgam1[i02] * zd[1]+
      hourgam2[i02] * zd[2] + hourgam3[i02] * zd[3]+
      hourgam4[i02] * zd[4] + hourgam5[i02] * zd[5]+
      hourgam6[i02] * zd[6] + hourgam7[i02] * zd[7];

  h03 =
      hourgam0[i03] * zd[0] + hourgam1[i03] * zd[1] +
      hourgam2[i03] * zd[2] + hourgam3[i03] * zd[3] +
      hourgam4[i03] * zd[4] + hourgam5[i03] * zd[5] +
      hourgam6[i03] * zd[6] + hourgam7[i03] * zd[7];


  hgfz[0] = coefficient *
      (hourgam0[i00] * h00 + hourgam0[i01] * h01 +
       hourgam0[i02] * h02 + hourgam0[i03] * h03);

  hgfz[1] = coefficient *
      (hourgam1[i00] * h00 + hourgam1[i01] * h01 +
       hourgam1[i02] * h02 + hourgam1[i03] * h03);

  hgfz[2] = coefficient *
      (hourgam2[i00] * h00 + hourgam2[i01] * h01 +
       hourgam2[i02] * h02 + hourgam2[i03] * h03);

  hgfz[3] = coefficient *
      (hourgam3[i00] * h00 + hourgam3[i01] * h01 +
       hourgam3[i02] * h02 + hourgam3[i03] * h03);

  hgfz[4] = coefficient *
      (hourgam4[i00] * h00 + hourgam4[i01] * h01 +
       hourgam4[i02] * h02 + hourgam4[i03] * h03);

  hgfz[5] = coefficient *
      (hourgam5[i00] * h00 + hourgam5[i01] * h01 +
       hourgam5[i02] * h02 + hourgam5[i03] * h03);

  hgfz[6] = coefficient *
      (hourgam6[i00] * h00 + hourgam6[i01] * h01 +
       hourgam6[i02] * h02 + hourgam6[i03] * h03);

  hgfz[7] = coefficient *
      (hourgam7[i00] * h00 + hourgam7[i01] * h01 +
       hourgam7[i02] * h02 + hourgam7[i03] * h03);
} // CalcElemFBHourglassForce()

static INLINE
Real_t CalcElemVolume_scalars( const Real_t x0, const Real_t x1,
               const Real_t x2, const Real_t x3,
               const Real_t x4, const Real_t x5,
               const Real_t x6, const Real_t x7,
               const Real_t y0, const Real_t y1,
               const Real_t y2, const Real_t y3,
               const Real_t y4, const Real_t y5,
               const Real_t y6, const Real_t y7,
               const Real_t z0, const Real_t z1,
               const Real_t z2, const Real_t z3,
               const Real_t z4, const Real_t z5,
               const Real_t z6, const Real_t z7 ) {
  Real_t twelveth = cast_Real_t(1.0)/cast_Real_t(12.0);

  Real_t dx61 = x6 - x1;
  Real_t dy61 = y6 - y1;
  Real_t dz61 = z6 - z1;

  Real_t dx70 = x7 - x0;
  Real_t dy70 = y7 - y0;
  Real_t dz70 = z7 - z0;

  Real_t dx63 = x6 - x3;
  Real_t dy63 = y6 - y3;
  Real_t dz63 = z6 - z3;

  Real_t dx20 = x2 - x0;
  Real_t dy20 = y2 - y0;
  Real_t dz20 = z2 - z0;

  Real_t dx50 = x5 - x0;
  Real_t dy50 = y5 - y0;
  Real_t dz50 = z5 - z0;

  Real_t dx64 = x6 - x4;
  Real_t dy64 = y6 - y4;
  Real_t dz64 = z6 - z4;

  Real_t dx31 = x3 - x1;
  Real_t dy31 = y3 - y1;
  Real_t dz31 = z3 - z1;

  Real_t dx72 = x7 - x2;
  Real_t dy72 = y7 - y2;
  Real_t dz72 = z7 - z2;

  Real_t dx43 = x4 - x3;
  Real_t dy43 = y4 - y3;
  Real_t dz43 = z4 - z3;

  Real_t dx57 = x5 - x7;
  Real_t dy57 = y5 - y7;
  Real_t dz57 = z5 - z7;

  Real_t dx14 = x1 - x4;
  Real_t dy14 = y1 - y4;
  Real_t dz14 = z1 - z4;

  Real_t dx25 = x2 - x5;
  Real_t dy25 = y2 - y5;
  Real_t dz25 = z2 - z5;

#define TRIPLE_PRODUCT(x1, y1, z1, x2, y2, z2, x3, y3, z3) \
   ((x1)*((y2)*(z3) - (z2)*(y3)) + (x2)*((z1)*(y3) - (y1)*(z3)) + (x3)*((y1)*(z2) - (z1)*(y2)))

  Real_t volume =
    TRIPLE_PRODUCT(dx31 + dx72, dx63, dx20,
       dy31 + dy72, dy63, dy20,
       dz31 + dz72, dz63, dz20) +
    TRIPLE_PRODUCT(dx43 + dx57, dx64, dx70,
       dy43 + dy57, dy64, dy70,
       dz43 + dz57, dz64, dz70) +
    TRIPLE_PRODUCT(dx14 + dx25, dx61, dx50,
       dy14 + dy25, dy61, dy50,
       dz14 + dz25, dz61, dz50);

#undef TRIPLE_PRODUCT

  volume *= twelveth;

  return volume ;
} // CalcElemVolume_scalars()

static INLINE
Real_t CalcElemVolume( const Real_t x[EIGHT],
                       const Real_t y[EIGHT],
                       const Real_t z[EIGHT] ) {
  return CalcElemVolume_scalars( x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7],
                                 y[0], y[1], y[2], y[3], y[4], y[5], y[6], y[7],
                                 z[0], z[1], z[2], z[3], z[4], z[5], z[6], z[7]);
} // CalcElemVolume()

static INLINE
Real_t AreaFace( const Real_t x0, const Real_t x1,
                 const Real_t x2, const Real_t x3,
                 const Real_t y0, const Real_t y1,
                 const Real_t y2, const Real_t y3,
                 const Real_t z0, const Real_t z1,
                 const Real_t z2, const Real_t z3) {
  Real_t fx = (x2 - x0) - (x3 - x1);
  Real_t fy = (y2 - y0) - (y3 - y1);
  Real_t fz = (z2 - z0) - (z3 - z1);
  Real_t gx = (x2 - x0) + (x3 - x1);
  Real_t gy = (y2 - y0) + (y3 - y1);
  Real_t gz = (z2 - z0) + (z3 - z1);
  Real_t area = (fx * fx + fy * fy + fz * fz)
              * (gx * gx + gy * gy + gz * gz)
              - (fx * gx + fy * gy + fz * gz)
              * (fx * gx + fy * gy + fz * gz) ;
  return area ;
} // AreaFace

static INLINE
Real_t CalcElemCharacteristicLength( const Real_t x[EIGHT],
                                     const Real_t y[EIGHT],
                                     const Real_t z[EIGHT],
                                     const Real_t volume) {
  Real_t a, charLength = cast_Real_t(0.0);

  a = AreaFace(x[0],x[1],x[2],x[3],
               y[0],y[1],y[2],y[3],
               z[0],z[1],z[2],z[3]) ;
  charLength = FMAX(a,charLength) ;

  a = AreaFace(x[4],x[5],x[6],x[7],
               y[4],y[5],y[6],y[7],
               z[4],z[5],z[6],z[7]) ;
  charLength = FMAX(a,charLength) ;

  a = AreaFace(x[0],x[1],x[5],x[4],
               y[0],y[1],y[5],y[4],
               z[0],z[1],z[5],z[4]) ;
  charLength = FMAX(a,charLength) ;

  a = AreaFace(x[1],x[2],x[6],x[5],
               y[1],y[2],y[6],y[5],
               z[1],z[2],z[6],z[5]) ;
  charLength = FMAX(a,charLength) ;

  a = AreaFace(x[2],x[3],x[7],x[6],
               y[2],y[3],y[7],y[6],
               z[2],z[3],z[7],z[6]) ;
  charLength = FMAX(a,charLength) ;

  a = AreaFace(x[3],x[0],x[4],x[7],
               y[3],y[0],y[4],y[7],
               z[3],z[0],z[4],z[7]) ;
  charLength = FMAX(a,charLength) ;

  charLength = cast_Real_t(4.0) * volume / SQRT(charLength);

  return charLength;
} // CalcElemCharacteristicLength()

static INLINE
void CalcElemVelocityGrandient( const Real_t* const xvel,
                                const Real_t* const yvel,
                                const Real_t* const zvel,
                                HC_UPC_CONST Real_t *b, // RAG -- b[][EIGHT]
                                const Real_t detJ,
                                Real_t* const d ) {
  const Real_t inv_detJ = cast_Real_t(1.0) / detJ ;
  Real_t dyddx, dxddy, dzddx, dxddz, dzddy, dyddz;
  const Real_t* const pfx = &b[0*EIGHT];
  const Real_t* const pfy = &b[1*EIGHT];
  const Real_t* const pfz = &b[2*EIGHT];

  d[0] = inv_detJ * ( pfx[0] * (xvel[0]-xvel[6])
                    + pfx[1] * (xvel[1]-xvel[7])
                    + pfx[2] * (xvel[2]-xvel[4])
                    + pfx[3] * (xvel[3]-xvel[5]) );

  d[1] = inv_detJ * ( pfy[0] * (yvel[0]-yvel[6])
                    + pfy[1] * (yvel[1]-yvel[7])
                    + pfy[2] * (yvel[2]-yvel[4])
                    + pfy[3] * (yvel[3]-yvel[5]) );

  d[2] = inv_detJ * ( pfz[0] * (zvel[0]-zvel[6])
                    + pfz[1] * (zvel[1]-zvel[7])
                    + pfz[2] * (zvel[2]-zvel[4])
                    + pfz[3] * (zvel[3]-zvel[5]) );

  dyddx  = inv_detJ * ( pfx[0] * (yvel[0]-yvel[6])
                      + pfx[1] * (yvel[1]-yvel[7])
                      + pfx[2] * (yvel[2]-yvel[4])
                      + pfx[3] * (yvel[3]-yvel[5]) );

  dxddy  = inv_detJ * ( pfy[0] * (xvel[0]-xvel[6])
                      + pfy[1] * (xvel[1]-xvel[7])
                      + pfy[2] * (xvel[2]-xvel[4])
                      + pfy[3] * (xvel[3]-xvel[5]) );

  dzddx  = inv_detJ * ( pfx[0] * (zvel[0]-zvel[6])
                      + pfx[1] * (zvel[1]-zvel[7])
                      + pfx[2] * (zvel[2]-zvel[4])
                      + pfx[3] * (zvel[3]-zvel[5]) );

  dxddz  = inv_detJ * ( pfz[0] * (xvel[0]-xvel[6])
                      + pfz[1] * (xvel[1]-xvel[7])
                      + pfz[2] * (xvel[2]-xvel[4])
                      + pfz[3] * (xvel[3]-xvel[5]) );

  dzddy  = inv_detJ * ( pfy[0] * (zvel[0]-zvel[6])
                      + pfy[1] * (zvel[1]-zvel[7])
                      + pfy[2] * (zvel[2]-zvel[4])
                      + pfy[3] * (zvel[3]-zvel[5]) );

  dyddz  = inv_detJ * ( pfz[0] * (yvel[0]-yvel[6])
                      + pfz[1] * (yvel[1]-yvel[7])
                      + pfz[2] * (yvel[2]-yvel[4])
                      + pfz[3] * (yvel[3]-yvel[5]) );
  d[5]  = cast_Real_t( .5) * ( dxddy + dyddx );
  d[4]  = cast_Real_t( .5) * ( dxddz + dzddx );
  d[3]  = cast_Real_t( .5) * ( dzddy + dyddz );
} // CalcElemVelocityGrandient()
