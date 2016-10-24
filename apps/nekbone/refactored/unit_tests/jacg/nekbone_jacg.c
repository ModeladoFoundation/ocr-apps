#ifndef UNIT_TEST_NEKBONE_JACG_C
#define UNIT_TEST_NEKBONE_JACG_C

#include <stdio.h>
#include "blas1.h"
#include "polybasis.h"

#define Err_t int
#define IFEB if(err) break

int main(int argc, char * argv[])
{
    FILE * fout = 0;

    Err_t err=0;
    while(!err){
        fout = fopen("./ccode.out", "w");
        if(!fout) {err=__LINE__; IFEB;}

        const unsigned int N = 84; //Largest value allowed in Nekbone's speclib.f::ZWGJ
        double x[N], v;
        double alpha, beta;

        unsigned int podegree, firstdegree, lastdegree, k;

        alpha = 0; // This is how it is use throughout NEKbone.
        beta  = 0; // This is how it is use throughout NEKbone.

        // If any of these values changes, make also the change in nekbone_jacg.f90
        firstdegree = 2;
        lastdegree  = 25;

        if(lastdegree >= N){
            printf("ERROR: test_jacg: lastdegree must be smaller than 100.\n");
            err=__LINE__; IFEB;
        }

        nbb_rvector_t vx = {0};
        vx.v = x;
        vx.length = N;

        for(podegree=firstdegree; podegree<=lastdegree; ++podegree){
            {
                vx.length = N;
                for(k=0; k<N; ++k) *nbb_atrv(vx,k) = 0;
            }

            vx.length = podegree;  //To adjust for what we are going to use.

            for(k=0; k<podegree; ++k){
                v = k;
                *nbb_atrv(vx,k) = v /(v+1);
            }

            err = nbb_JACG(vx, podegree, alpha, beta); IFEB;

            fprintf(fout,"%10u\n", podegree);
            for(k=0; k<vx.length; ++k){
                fprintf(fout,"%23.14E\n", nbb_getrv(vx,k));
            }
        }
        IFEB;

        if(fout){
            fclose(fout); fout=0;
        }

        break; //while(!err){
    }

    if(fout){
        fclose(fout); fout=0;
    }

    if(err){
        printf("ERROR: %lu\n", (unsigned long)err );
        return 1;
    }
    return 0;
}

#endif // UNIT_TEST_NEKBONE_JACG_C
