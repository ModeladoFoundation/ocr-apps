#ifndef NEKBONE_POLYBASIS_H
#include "polybasis.h"
#endif

#include "blas.h"

#include <math.h> //atan, cos, pow

#include <stdio.h> //printf for debugging

#define IFEB if(err) break

int nbb_JACOBF(unsigned int in_polyOrder,
               double in_alpha, double in_beta,
               double in_X,    // in_X is the sample in -1<X<1
               double * o_POLY,
               double * o_PDER,
               double * o_POLYM1,
               double * o_PDERM1,
               double * o_POLYM2,
               double * o_PDERM2
               )
{
#   ifdef NEK_CHECKS_POLYBASIS
    if(!o_POLY || !o_PDER || !o_POLYM1 || !o_PDERM1 || !o_POLYM2 || !o_PDERM2) return __LINE__;
    if( in_X < (double)(-1) || (double)(1) > in_X ) return __LINE__;
    if(in_polyOrder > 84) return __LINE__; //84 is called NMAX in nekbone::speclib.f
    if(1 > in_polyOrder) return __LINE__;
    if(in_alpha < (double)(-1) || in_beta < (double)(-1)) return __LINE__;
#   endif

    double apb = in_alpha + in_beta;
    *o_POLY = 1;
    *o_PDER = 0;

    if(in_polyOrder == 0) return 0;

    double polyL = *o_POLY;
    double pderL = *o_PDER;

    *o_POLY = (in_alpha - in_beta +(apb+2)*in_X)/2;
    *o_PDER = (apb + 2)/2;

    if(in_polyOrder == 1) return 0;

    double PSAVE  = 0;
    double PDSAVE = 0;

    unsigned int k;
    for(k=2; k<=in_polyOrder; ++k){
        const double dk = k;
        const double a1 = dk * 2 *(dk+apb) * (dk*2+apb-2);
        const double a2 = (dk*2+apb-1)*(in_alpha*in_alpha - in_beta*in_beta);
        const double b3 = (dk*2+apb-2);
        const double a3 = b3*(b3+1)*(b3+2);
        const double a4 = (dk+in_alpha-1)*(dk+in_beta-1)*(dk*2+apb)*2;

        const double polyn = ((a2+a3*in_X)*(*o_POLY) - a4*polyL)/a1;
        const double pdern = ((a2+a3*in_X)*(*o_PDER) - a4*pderL + a3*(*o_POLY))/a1;

        PSAVE  = polyL;
        PDSAVE = pderL;

        polyL = *o_POLY;
        *o_POLY = polyn;

        pderL = *o_PDER;
        *o_PDER = pdern;
    }

    *o_POLYM1 = polyL;
    *o_PDERM1 = pderL;
    *o_POLYM2 = PSAVE;
    *o_PDERM2 = PDSAVE;

    return 0;
}

int nbb_GAMMAF_compare_equal(double x, double y)
{
#   define USE_STRAIGHT_GAMMAF_compare_equal
#   ifdef USE_STRAIGHT_GAMMAF_compare_equal
        return (x == y);
#   else
        const double eps=1e-14;
        double diff = x-y;
        if(diff<0) diff =-diff;
        return (diff < eps);
#   endif
}

double nbb_GAMMAF(double X)
{
    const double half = ((double)1)/2;
    const double three_halves = ((double)3)/2;
    const double five_halves = ((double)5)/2;
    const double seven_halves = ((double)7)/2;
    const double one  = ((double)1);
    const double two  = ((double)2);
    const double three = ((double)3);
    const double four = ((double)4);
    const double five = ((double)5);
    const double six = ((double)6);
    const double sqrtpi = 1.772453850905516027; //sqrt(pi);

    double g = one;

    if(nbb_GAMMAF_compare_equal(X, -half)) g = -two * sqrtpi;
    if(nbb_GAMMAF_compare_equal(X,  half)) g = sqrtpi;
    if(nbb_GAMMAF_compare_equal(X,   one)) g = one;
    if(nbb_GAMMAF_compare_equal(X,   two)) g = one;
    if(nbb_GAMMAF_compare_equal(X, three_halves)) g = sqrtpi/2;
    if(nbb_GAMMAF_compare_equal(X,  five_halves)) g = sqrtpi * 0.75;
    if(nbb_GAMMAF_compare_equal(X, seven_halves)) g = (sqrtpi*1.5)*1.25;
    if(nbb_GAMMAF_compare_equal(X, three)) g = two;
    if(nbb_GAMMAF_compare_equal(X, four)) g = six;
    if(nbb_GAMMAF_compare_equal(X, five)) g = 24;
    if(nbb_GAMMAF_compare_equal(X, six)) g = 120;

    return g;
}

int nbb_JACG(nbb_rvector_t o_xjac, unsigned int in_polyOrder,
             double in_alpha, double in_beta)
{
#   ifdef NEK_CHECKS_POLYBASIS
    if(!o_xjac.v || o_xjac.length ==0) return __LINE__;
    if(1 > in_polyOrder) return __LINE__;
    if(in_alpha < (double)(-1) || in_beta < (double)(-1)) return __LINE__;
#   endif

    int err=0;

    const unsigned int kstop = 10;
    const double eps = 1e-12;

    const double pi = 3.1415926535897932384626;
    const double N = in_polyOrder -1;
    const double dth = pi/(N*2+2);

    double xlast=0;
    unsigned int j;
    for(j=0; j<in_polyOrder; ++j){
        double x,x1,x2;

        if(j==0){
            x = cos( (2*j+1)*dth  );
        }else{
            x1 = cos( (2*j+1)*dth  );
            x2 = xlast;
            x = (x1+x2)/2;
        }

        unsigned int k;
        for(k=0; k<kstop; ++k){
            double p,pd, pm1,pdm1, pm2,pdm2;
            err = nbb_JACOBF(in_polyOrder, in_alpha, in_beta, x,
                             &p,&pd, &pm1,&pdm1, &pm2,&pdm2); IFEB;
            double recsum = 0;
            const long jm = ((long)j) - 1;
            long i;
            for(i=0; i<=jm; ++i){
                recsum += 1 / (x - nbb_getrv(o_xjac,N-i));
            }
            const double delx = -p/(pd - recsum*p);
            x += delx;
            if( -eps < delx && delx < eps ){
                break;
            }
        }//for(k
        IFEB;

        *nbb_atrv(o_xjac, in_polyOrder-j-1) = x;
        xlast = x;
    }//for(j
    if(err) return err;

    unsigned int ii,jj;
    for(ii=0; ii<in_polyOrder; ++ii){
        double xmin = 1e30;
        unsigned int jmin = 0;
        for(jj=ii; jj<in_polyOrder; ++jj){
            if(nbb_getrv(o_xjac,jj) < xmin){
                xmin = nbb_getrv(o_xjac,jj);
                jmin = jj;
            }
        }//for(jj

        if(jmin != ii){
            double swap = nbb_getrv(o_xjac,ii);
            *nbb_atrv(o_xjac,ii) = nbb_getrv(o_xjac,jmin);
            *nbb_atrv(o_xjac,jmin) = swap;
        }
    }//for(ii

    return 0;
}

double nbb_endw1(unsigned int in_n, double in_alpha, double in_beta)
{
    const double one   = 1;
    const double two   = 2;
    const double three = 3;
    const double four  = 4;

    const double apb   = in_alpha + in_beta;

    double endw1 = 0;
    if( in_n == 0 ){
        return endw1;
    }

    double f1 = nbb_GAMMAF(in_alpha+two) * nbb_GAMMAF(in_beta+one) / nbb_GAMMAF(apb+three);
    f1 = f1 * (apb+two) * pow(two, apb+two) / two;

    if( in_n == 1 ){
        endw1 = f1;
        return endw1;
    }

    double fint1 = nbb_GAMMAF(in_alpha+two) * nbb_GAMMAF(in_beta+one) / nbb_GAMMAF(apb+three);
    fint1 = fint1 * pow(two, apb+two);

    double fint2 = nbb_GAMMAF(in_alpha+two) * nbb_GAMMAF(in_beta+two) / nbb_GAMMAF(apb+four);
    fint2 = fint2 * pow(two, apb+three);

    double f2 = (-two*(in_beta+two)*fint1 + (apb+four)*fint2) * (apb+three) / four;

    if( in_n == 2 ){
        endw1 = f2;
        return endw1;
    }

    double f3;
    unsigned int i;
    for(i=3; i<=in_n; ++i){
        const double di = (i-1);
        const double abn = in_alpha + in_beta + di;
        const double abnn = abn + di;
        const double a1 = -(two*(di+in_alpha)*(di+in_beta))/(abn*abnn*(abnn+one));
        const double a2 = (two*(in_alpha-in_beta))/(abnn*(abnn+two));
        const double a3 = (two*(abn+one))/((abnn+two)*(abnn+one));
        f3 = -(a2*f2 + a1*f1)/a3;
        f1 = f2;
        f2 = f3;
    }

    endw1 = f3;
    return endw1;
}

double nbb_endw2(unsigned int in_n, double in_alpha, double in_beta)
{
    const double one   = 1;
    const double two   = 2;
    const double three = 3;
    const double four  = 4;

    const double apb   = in_alpha + in_beta;

    double endw2 = 0;
    if( in_n == 0 ){
        return endw2;
    }

    double f1 = nbb_GAMMAF(in_alpha+one) * nbb_GAMMAF(in_beta+two) / nbb_GAMMAF(apb+three);
    f1 = f1 * (apb+two) * pow(two, apb+two) / two;

    if( in_n == 1 ){
        endw2 = f1;
        return endw2;
    }
    double fint1 = nbb_GAMMAF(in_alpha+one) * nbb_GAMMAF(in_beta+two) / nbb_GAMMAF(apb+three);
    fint1 = fint1 * pow(two, apb+two);

    double fint2 = nbb_GAMMAF(in_alpha+two) * nbb_GAMMAF(in_beta+two) / nbb_GAMMAF(apb+four);
    fint2 = fint2 * pow(two, apb+three);

    double f2 = (two*(in_alpha+two)*fint1 - (apb+four)*fint2) * (apb+three) / four;

    if( in_n == 2 ){
        endw2 = f2;
        return endw2;
    }

    double f3;
    unsigned int i;
    for(i=3; i<=in_n; ++i){
        const double di = (i-1);
        const double abn = in_alpha + in_beta + di;
        const double abnn = abn + di;
        const double a1 = -(two*(di+in_alpha)*(di+in_beta))/(abn*abnn*(abnn+one));
        const double a2 = (two*(in_alpha-in_beta))/(abnn*(abnn+two));
        const double a3 = (two*(abn+one))/((abnn+two)*(abnn+one));
        f3 = -(a2*f2 + a1*f1)/a3;
        f1 = f2;
        f2 = f3;
    }

    endw2 = f3;
    return endw2;
}

void nbb_fd_weights_full(double in_xx, double * in_x, unsigned int in_n,
                     unsigned int in_m, nbb_matrix2_t * io_c)
{
      double c1 = 1;
      double c4 = in_x[0] - in_xx;

      unsigned int i,j,k;

      for(k=0; k <= in_m; ++k){
        for(j=0; j <= in_n; ++j){
            *nbb_at2(*io_c, j,k) = 0;
        }
      }

      *nbb_at2(*io_c, 0,0) = 1;

      for(i=1; i <= in_n; ++i){
         unsigned int mn = i;
         if(i>in_m) mn = in_m;

         double c2 = 1;
         double c5 = c4;
         c4 = in_x[i] - in_xx;
         for(j=0; j <= i-1; ++j){
            double c3 = in_x[i] - in_x[j];
            c2 = c2*c3;
            for(k=mn; k >= 1; --k){
                BLAS_REAL_TYPE u = nbb_get2(*io_c,i-1,k-1);
                BLAS_REAL_TYPE v = nbb_get2(*io_c,i-1,k);

                *nbb_at2(*io_c, i,k) = c1 * (k*u - c5*v) /c2;
            }
            BLAS_REAL_TYPE w = nbb_get2(*io_c,i-1,0);
            *nbb_at2(*io_c, i,0) = -c1 * c5 * w / c2;
            for(k=mn; k >= 1; --k){
                BLAS_REAL_TYPE uu = nbb_get2(*io_c,j,k);
                BLAS_REAL_TYPE vv = nbb_get2(*io_c,j,k-1);
                *nbb_at2(*io_c, j,k) = (c4*uu - k*vv) /c3;
            }
            BLAS_REAL_TYPE ww = nbb_get2(*io_c,j,0);
            *nbb_at2(*io_c, j,0) = c4*ww/c3;
         }
         c1 = c2;
      }
}

double nbb_pnormj(unsigned int in_P, double in_alpha, double in_beta)
{
    double X = 0;

    const double one = 1;
    const double two = 2;
    const double dn = in_P;

    const double consta = in_alpha + in_beta + one;
    double prod;
    if(in_P <= 1){
        prod = nbb_GAMMAF(dn + in_alpha) * nbb_GAMMAF(dn + in_beta);
        prod = prod /(nbb_GAMMAF(dn) * nbb_GAMMAF(dn+in_alpha+in_beta));
        X = prod * pow(two, consta)/(two*dn + consta);
        return X;
    }

    prod = nbb_GAMMAF(in_alpha + one) * nbb_GAMMAF(in_beta + one);
    prod = prod / ( two*(one + consta) * nbb_GAMMAF( consta + one));
    prod = prod * (one + in_alpha) * (two + in_alpha);
    prod = prod * (one + in_beta) * (two + in_beta);

    unsigned int i;
    for(i=3; i<=in_P; ++i){
        double dindx = i;
        double frac = (dindx + in_alpha);
        frac *= (dindx + in_beta)/(dindx*(dindx + in_alpha + in_beta));
        prod = prod * frac;
    }

    X = prod * pow(two, consta) / (two*dn + consta);
    return X;
}

int nbb_ZWGJD(double * io_z, double * io_w, unsigned int in_pDOF, double in_alpha, double in_beta)
{
    int err=0;
    while(!err){
        const unsigned int N = in_pDOF - 1;
        const double one = 1;
        const double two = 2;
        const double APB = in_alpha + in_beta;

        //DBG> unsigned int k;

        if(!io_z || !io_w) { err=__LINE__; break; }
        //Minimum number of Gauss points is 1.
        if(in_pDOF==0) { err=__LINE__; break; }
        if(in_alpha <= -1 || in_beta <= -1) { err=__LINE__; break; }

        if(in_pDOF == 1){
            io_z[0] = (in_beta - in_alpha)/(APB + two);
            io_w[0] = nbb_GAMMAF(in_alpha + one) * nbb_GAMMAF(in_beta + one)/nbb_GAMMAF(APB + two) * pow(two, APB+one);
            return err;
        }

        nbb_rvector_t Z;
        Z.length = in_pDOF;
        Z.v = io_z;

        err = nbb_JACG(Z, in_pDOF, in_alpha, in_beta); IFEB;

        //DBG> for(k=0; k!=Z.length; ++k){
        //DBG>     printf("DBG>C> JACG> %u %23.14E\n", k, io_z[k] );
        //DBG> }

        const unsigned int NP1 = N + one;
        const unsigned int NP2 = N + two;
        const double DNP1 = NP1;
        const double DNP2 = NP2;
        const double FAC1 = DNP1 + in_alpha + in_beta + one;
        const double FAC2 = FAC1 + DNP1;
        const double FAC3 = FAC2 + one;
        const double FNORM = nbb_pnormj(NP1, in_alpha, in_beta);
        const double RCOEF = (FNORM * FAC2 * FAC3) / (two * FAC1 * DNP2);

        //DBG> printf("DBG>C> const> %u  %u  %23.14E  %23.14E  %23.14E  %23.14E  %23.14E  \n", in_pDOF,NP1, FAC1, FAC2, FAC3, FNORM, RCOEF   );

        unsigned int i;
        for(i=0; i < in_pDOF; ++i){
            double P=0, PD=0, PM1=0, PDM1=0, PM2=0, PDM2=0;
            double Zi = nbb_getrv(Z,i);
            err = nbb_JACOBF(NP2, in_alpha, in_beta, Zi, &P, &PD, &PM1, &PDM1, &PM2, &PDM2); IFEB;
            io_w[i] = -RCOEF / ( P * PDM1);
        } IFEB;

        break;
    }
    return err;
}

int nbb_ZWGLJD(double * io_z, double * io_w, unsigned int in_pDOF, double in_alpha, double in_beta)
{
    int err=0;
    while(!err){
        const unsigned int N = in_pDOF - 1;
        const unsigned int NM1 = N - 1;
        const double one = 1;
        const double two = 2;

        if(!io_z || !io_w) { err=__LINE__; break; }
        //Minimum number of Gauss-Lobatto points is 2.
        if(in_pDOF<=1) { err=__LINE__; break; }
        if(in_alpha <= -1 || in_beta <= -1) { err=__LINE__; break; }

        if(NM1 > 0){
            const double ALPG = in_alpha + one;
            const double BETG = in_beta + one;
            //Original call --> CALL ZWGJD (Z(2),W(2),NM1,ALPG,BETG)
            err = nbb_ZWGJD(io_z+1, io_w+1, NM1, ALPG, BETG); IFEB;
        }

        io_z[0] = -one;
        io_z[in_pDOF-1] = one;

        unsigned int i;
        for(i=1; i < N; ++i){
            const double square = io_z[i] *  io_z[i];
            io_w[i] = io_w[i] / (one - square);
        }

        double P=0, PD=0, PM1=0, PDM1=0, PM2=0, PDM2=0;

        err = nbb_JACOBF(N, in_alpha, in_beta, io_z[0], &P, &PD, &PM1, &PDM1, &PM2, &PDM2); IFEB;
        io_w[0] = nbb_endw1(N, in_alpha, in_beta) / (two * PD);

        err = nbb_JACOBF(N, in_alpha, in_beta, io_z[N], &P, &PD, &PM1, &PDM1, &PM2, &PDM2); IFEB;
        io_w[N] = nbb_endw2(N, in_alpha, in_beta) / (two * PD);


        break;
    }
    return err;
}

int nbb_ZWGLJ(double * in_work_z, double * in_work_w,
              unsigned int in_pDOF, double in_alpha, double in_beta,
              NBN_REAL * io_z, NBN_REAL * io_w
              )
{
    int err=0;
    while(!err){
        const unsigned int NMAX = 84; //Largest polynomial degree, in ZWGLJ, allowed.

        if(in_pDOF > NMAX) { err = __LINE__; IFEB; }

        err = nbb_ZWGLJD(in_work_z, in_work_w, in_pDOF, in_alpha, in_beta); IFEB;

        //DBG> printf("sizeof(NBN_REAL) = %d\n", (int) sizeof(NBN_REAL));

        unsigned int i;
        for(i=0; i<in_pDOF; ++i){
            const double tz = in_work_z[i];
            const double tw = in_work_w[i];
            io_z[i] = tz;
            io_w[i] = tw;
        }

        break;
    }
    return err;
}

int nbb_ZWGLL(double * in_work_z, double * in_work_w, unsigned int in_pDOF, NBN_REAL * io_z, NBN_REAL * io_w)
{
    int err=0;
    while(!err){
        const double alpha = 0;
        const double beta = 0;
        err = nbb_ZWGLJ(in_work_z, in_work_w, in_pDOF, alpha, beta, io_z, io_w); IFEB;
        break;
    }
    return err;
}

int nbb_semhat(nbb_matrix2_t io_a, NBN_REAL * io_b, nbb_matrix2_t io_c,
               nbb_matrix2_t io_d, NBN_REAL * io_z, NBN_REAL * io_w,
               unsigned int in_Porder, double * io_work_w, double * io_work_z)
{
    int err=0;
    while(!err){
        const unsigned int pDOF = in_Porder + 1;

        err = nbb_ZWGLL(io_work_z, io_work_w, pDOF, io_z, io_b);

//        if( "debug"){
//            unsigned int t;
//            for(t=0; t<pDOF; ++t){
//                printf("DBG-ZB> %10u   %23.14E   %23.14E\n", t, io_z[t], io_b[t]);
//            }
//        }

        unsigned int i,j;
        for(i=0; i<pDOF; ++i){
            nbb_matrix2_t temp_w;
            temp_w.m = io_work_w;
            temp_w.sz_rows = pDOF;
            temp_w.sz_cols = 2;

            nbb_fd_weights_full(io_z[i], io_z, in_Porder, 1, &temp_w);

            for(j=0; j<pDOF; ++j){
                *nbb_at2(io_d, i,j) = nbb_get2(temp_w,j,1); // Derivative matrix
            }
        }

//        if( "debug"){
//            for(i=0; i<pDOF; ++i){
//                for(j=0; j<pDOF; ++j){
//                    printf("DBG-D> %10u  %10u   %23.14E\n", i,j, nbb_get2(io_d,i,j) );
//                }
//            }
//        }

        for(i=0; i < (io_a.sz_cols * io_a.sz_rows); ++i){
            io_a.m[i] = 0;
        }

        for(j=0; j < pDOF; ++j){
            for(i=0; i < pDOF; ++i){
                unsigned int k;
                for(k=0; k < pDOF; ++k){
                    double dki = nbb_get2(io_d,k,i);
                    double dkj = nbb_get2(io_d,k,j);
                    *nbb_at2(io_a,i,j) = nbb_get2(io_a,i,j) + dki * io_b[k] * dkj;
                }

                *nbb_at2(io_c, i,j) = io_b[i] * nbb_get2(io_d,i,j);
            }
        }

//        if( "debug"){
//            for(i=0; i<pDOF; ++i){
//                for(j=0; j<pDOF; ++j){
//                    printf("DBG-AC> %10u  %10u   %23.14E   %23.14E\n", i,j, nbb_get2(io_a,i,j), nbb_get2(io_c,i,j) );
//                }
//            }
//        }

        break;
    }
    return err;
}

//2016OCt23: Typically the weight matrix g is fully used.  In the case of Nekbone,
//           only its main diagonal elements are used.
//           So they are the only one calculated.
//TODO: Add the zeroed off diagonals components of the Gauss weights.
void nbb_setup_g(unsigned int in_pDOF, NBN_REAL * in_wxm1,
                NBN_REAL * o_g1, NBN_REAL * o_g4, NBN_REAL * o_g6)
{
    //2016Oct21: Noting that all g(1:6,i,j,k,e) are all the same for any value of i,j,k,e,
    //           the simplification of calculating only once g for e=1
    //           was decided.  This is equivalent to what would happen if a proper
    //           memoization mechanism was used, as is the case in many actual
    //           application.
    //TODO: Remove memoization of Gaussian weights g.

    unsigned i,j,k;

    const unsigned int nx1 = in_pDOF;
    const unsigned int ny1 = in_pDOF;
    const unsigned int nz1 = in_pDOF;

    for(k=0; k < nz1; ++k){
        for(j=0; j < ny1; ++j){
            for(i=0; i < nx1; ++i){
                unsigned long h = hash_columnMajor3(0, nx1,ny1,nz1, i,j,k);

                double x = in_wxm1[i];
                double y = in_wxm1[j];
                double z = in_wxm1[k];

                double prod = x*y*z;

                //DBG> printf("i,j,k= %u,%u,%u  h = %lu  x,y,z=%g,%g,%g  prod=%g\n", i,j,k, h, x,y,z, prod);
                o_g1[h] = prod;
                o_g4[h] = prod;
                o_g6[h] = prod;
            }
        }
    }
}

int nbb_proxy_setup(nbb_matrix2_t io_a, NBN_REAL * io_b, nbb_matrix2_t  io_c,
                    nbb_matrix2_t io_d, NBN_REAL * io_z, NBN_REAL * io_w,
                    unsigned int in_pDOF, double * io_work_w, double * io_work_z,
                    NBN_REAL * o_g1, NBN_REAL * o_g4, NBN_REAL * o_g6,
                    nbb_matrix2_t o_dxm1, nbb_matrix2_t o_dxtm1)
{
    int err=0;
    while(!err){

        err = nbb_semhat(io_a,io_b,io_c,io_d,io_z,io_w,
                         in_pDOF-1, io_work_w, io_work_z); IFEB;

        const unsigned int n2 = in_pDOF * in_pDOF;

        unsigned i;

        for(i=0; i < n2; ++i){
            o_dxm1.m[i] = io_d.m[i];
        }

        err = nbb_transpose2(&o_dxtm1, o_dxm1); IFEB;

        //2016OCt21: Although important in normal usage, the Gll points zgm1 are
        //           not used anywhere in the baseline NEkbone code.
        //           So they will be omitted for now.
        //TODO: add zgm1, the GLL points, even if they are still not used.
        //Not in use--> call copy(zgm1,z,nx1)   ! GLL points

        //The following is skipped as wxm1 is only used in nbb_setup_g and nowhere else.
        //Furthermore io_b is just an intermediate vectpr that will not be used again
        //after this function is done.
        //call copy(wxm1,b,nx1)   ! GLL weights

        nbb_setup_g(in_pDOF, io_b, o_g1, o_g4, o_g6);

        break;
    }
    return err;
}


