#ifndef UNIT_TEST_NEKBONE_BLAS1_C
#define UNIT_TEST_NEKBONE_BLAS1_C

#include <stdio.h>
#include <string.h> //memset
#include "blas1.h"

#define Err_t int
#define IFEB if(err) break

#define REAL_TOLERANCE 1E-14
#define INT_TOLERANCE 0

Err_t check_nbb_rzero()
{
    Err_t err=0;
    while(!err){
        const BLAS_UINT_TYPE N=5;
        BLAS_REAL_TYPE x[5], ref[5];

        unsigned long i;
        for(i=0; i<N; ++i){
            x[i]=i;
            ref[i]=x[i];
        }

        memset(ref, 0, N*sizeof(BLAS_REAL_TYPE));

        nbb_rvector_t vx;
        vx.v = x;
        vx.length = N;

        err=nbb_rzero(vx); IFEB;

        double total_diff=0;
        for(i=0; i<N; ++i){
            double diff = nbb_getrv(vx,i) - ref[i];
            if(diff<0) diff=-diff;
            total_diff += diff;
        }

        if(total_diff>REAL_TOLERANCE){
                printf("%g\n", total_diff);
            err=__LINE__;
            break;
        }

        break; //while(!err){
    }
    return err;
}

Err_t check_nbb_rzeroi()
{
    Err_t err=0;
    while(!err){
        const BLAS_UINT_TYPE N=5;
        BLAS_UINT_TYPE x[5], ref[5];

        const unsigned long first=2;
        const unsigned long last =3;

        unsigned long i;
        for(i=0; i<N; ++i){
            x[i]=i;
            ref[i]=x[i];
        }

        for(i=first; i<=last; ++i){
            ref[i]=0;
        }

        nbb_ivector_t vx;
        vx.v = x;
        vx.length = N;

        err=nbb_rzeroi(vx, first,last); IFEB;

        unsigned long total_diff=0;
        for(i=0; i<N; ++i){
            unsigned long diff = nbb_getiv(vx,i) - ref[i];
            if(diff<0) diff=-diff;
            total_diff += diff;
        }

        if(total_diff>INT_TOLERANCE){
            err=__LINE__;
            break;
        }

        break; //while(!err){
    }
    return err;
}

Err_t check_nbb_rone()
{
    Err_t err=0;
    while(!err){
        const BLAS_UINT_TYPE N=5;
        BLAS_REAL_TYPE x[5], ref[5];

        nbb_rvector_t vx;
        vx.v = x;
        vx.length = N;

        err=nbb_rzero(vx); IFEB;

        unsigned long i;
        for(i=0; i<N; ++i){
            ref[i]=1;
        }

        err=nbb_rone(vx); IFEB;

        double total_diff=0;
        for(i=0; i<N; ++i){
            double diff = nbb_getrv(vx,i) - ref[i];
            if(diff<0) diff=-diff;
            total_diff += diff;
        }

        if(total_diff>REAL_TOLERANCE){
            err=__LINE__;
            break;
        }

        break; //while(!err){
    }
    return err;
}

Err_t check_nbb_copy()
{
    Err_t err=0;
    while(!err){
        const BLAS_UINT_TYPE N=5;
        BLAS_REAL_TYPE x[5], y[5], ref[5];

        unsigned long i;
        for(i=0; i<N; ++i){
            ref[i]=i;
            y[i]=ref[i];
        }

        nbb_rvector_t vx, vy;
        vx.v=x;
        vx.length=N;
        err=nbb_rzero(vx); IFEB;

        vy.v=y;
        vy.length = N;

        err=nbb_copy(vx, vy); IFEB;

        double total_diff=0;
        for(i=0; i<N; ++i){
            double diff = nbb_getrv(vx,i) - ref[i];
            if(diff<0) diff=-diff;
            total_diff += diff;
        }

        if(total_diff>REAL_TOLERANCE){
            err=__LINE__;
            break;
        }

        break; //while(!err){
    }
    return err;
}

Err_t check_nbb_copyi()
{
    Err_t err=0;
    while(!err){
        const BLAS_UINT_TYPE N=5;
        BLAS_REAL_TYPE x[5], y[5], ref[5];

        const BLAS_UINT_TYPE first = 2;
        const BLAS_UINT_TYPE last  = 3;

        nbb_rvector_t vx, vy;
        vx.v=x;
        vx.length=N;
        err=nbb_rzero(vx); IFEB;

        vy.v=y;
        vy.length = N;

        unsigned long i;
        for(i=0; i<N; ++i){
            ref[i]=0;
            y[i]=0;
            if( first <=i && i <=last){
                ref[i]=i;
                y[i]=ref[i];
            }
        }

        err=nbb_copyi(vx, vy, first, last); IFEB;

        double total_diff=0;
        for(i=0; i<N; ++i){
            double diff = nbb_getrv(vx,i) - ref[i];
            if(diff<0) diff=-diff;
            total_diff += diff;
        }

        if(total_diff>REAL_TOLERANCE){
            err=__LINE__;
            break;
        }

        break; //while(!err){
    }
    return err;
}

Err_t check_nbb_add2()
{
    Err_t err=0;
    while(!err){
        const BLAS_UINT_TYPE N=5;
        BLAS_REAL_TYPE x[5], y[5], ref[5];

        unsigned long i;
        for(i=0; i<N; ++i){
            ref[i]=i;
            y[i]=ref[i];
            x[i]=ref[i];
            ref[i]+=ref[i];
        }

        nbb_rvector_t vx, vy;
        vx.v=x;
        vx.length=N;
        vy.v=y;
        vy.length = N;

        err=nbb_add2(vx,vy); IFEB;

        double total_diff=0;
        for(i=0; i<N; ++i){
            double diff = nbb_getrv(vx,i) - ref[i];
            if(diff<0) diff=-diff;
            total_diff += diff;
        }

        if(total_diff>REAL_TOLERANCE){
            err=__LINE__;
            break;
        }

        break; //while(!err){
    }
    return err;
}

Err_t check_nbb_col2()
{
    Err_t err=0;
    while(!err){
        const BLAS_UINT_TYPE N=5;
        BLAS_REAL_TYPE x[5], y[5], ref[5];

        unsigned long i;
        for(i=0; i<N; ++i){
            ref[i]=i;
            y[i]=ref[i];
            x[i]=ref[i];
            ref[i]*=ref[i];
        }

        nbb_rvector_t vx, vy;
        vx.v=x;
        vx.length=N;
        vy.v=y;
        vy.length = N;

        err=nbb_col2(vx,vy); IFEB;

        double total_diff=0;
        for(i=0; i<N; ++i){
            double diff = nbb_getrv(vx,i) - ref[i];
            if(diff<0) diff=-diff;
            total_diff += diff;
        }

        if(total_diff>REAL_TOLERANCE){
            err=__LINE__;
            break;
        }

        break; //while(!err){
    }
    return err;
}

Err_t check_nbb_add2s1i()
{
    Err_t err=0;
    while(!err){
        const BLAS_UINT_TYPE N=5;
        BLAS_REAL_TYPE x[5], y[5], ref[5];
        BLAS_REAL_TYPE c=3;

        const BLAS_UINT_TYPE first = 2;
        const BLAS_UINT_TYPE last  = 3;

        unsigned long i;

        for(i=0; i<N; ++i){
            x[i]=0;
            y[i]=0;
            ref[i]=0;
        }

        for(i=0; i<N; ++i){
            if( first<=i && i <=last){
                ref[i]=i;
                y[i]=ref[i];
                x[i]=ref[i];
                ref[i] = c*ref[i] + ref[i];
            }
        }

        nbb_rvector_t vx, vy;
        vx.v=x;
        vx.length=N;
        vy.v=y;
        vy.length = N;

        err=nbb_add2s1i(vx,vy,c, first,last); IFEB;

        double total_diff=0;
        for(i=0; i<N; ++i){
            double diff = nbb_getrv(vx,i) - ref[i];
            if(diff<0) diff=-diff;
            total_diff += diff;
        }

        if(total_diff>REAL_TOLERANCE){
            err=__LINE__;
            break;
        }

        break; //while(!err){
    }
    return err;
}

int main(int argc, char * argv[])
{
    Err_t err=0;
    while(!err){

        err = check_nbb_rzero(); IFEB;
        err = check_nbb_rzeroi(); IFEB;
        err = check_nbb_rone(); IFEB;
        err = check_nbb_copy(); IFEB;
        err = check_nbb_copyi(); IFEB;
        err = check_nbb_add2(); IFEB;
        err = check_nbb_col2(); IFEB;
        err = check_nbb_add2s1i(); IFEB;

        break; //while(!err){
    }

    if(err){
        printf("ERROR: %lu\n", (unsigned long)err );
        return 1;
    }
    return 0;
}

#endif // UNIT_TEST_NEKBONE_BLAS1_C
