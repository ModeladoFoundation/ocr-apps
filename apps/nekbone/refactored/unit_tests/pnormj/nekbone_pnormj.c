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
        fout = fopen("./z_ccode.out", "w");
        if(!fout) {err=__LINE__; IFEB;}

        double alpha, beta;
        unsigned int pDOF, firstdegree, lastdegree;

        alpha = 0; // This is how it is use throughout NEKbone.
        beta  = 0; // This is how it is use throughout NEKbone.

        // If any of these values changes, make also the change in nekbone_pnormj.f90
        firstdegree = 2;
        lastdegree  = 25;

        const unsigned int N = 84; //Largest value allowed in Nekbone's speclib.f::ZWGJ
        if(lastdegree >= N){
            printf("ERROR: test_jacg: lastdegree must be smaller than 100.\n");
            err=__LINE__; IFEB;
        }

        for(pDOF=firstdegree; pDOF<=lastdegree; ++pDOF){
            double nj = nbb_pnormj(pDOF, alpha, beta);
            fprintf(fout,"%10u %23.14E\n", pDOF, nj);
        }

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
