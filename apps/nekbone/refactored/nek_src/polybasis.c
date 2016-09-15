#ifndef NEKBONE_POLYBASIS_H
#include "polybasis.h"
#endif

#include "blas.h"

#include <math.h> //atan, cos

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
    const double zero = 0;
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
    const double pi = 3.1415926535897932384626;
    const double sqrtpi = sqrt(pi);

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

