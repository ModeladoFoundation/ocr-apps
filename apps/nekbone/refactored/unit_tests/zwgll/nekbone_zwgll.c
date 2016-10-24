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

        const unsigned int N = 84;  //Largest value allowed in Nekbone's speclib.f::ZWGJ
        double work_z[N], work_w[N];
        NBN_REAL z[N], w[N];

        unsigned int pDOF, firstdegree, lastdegree, k;

        // If any of these values changes, make also the change in nekbone_jacg.f90
        firstdegree = 2;
        lastdegree  = 25;

        if(lastdegree >= N){
            printf("ERROR: test_jacg: lastdegree must be smaller than 100.\n");
            err=__LINE__; IFEB;
        }

        const double one = 1;

        for(pDOF=firstdegree; pDOF<=lastdegree; ++pDOF){
            for(k=0; k<N; ++k){
                work_z[k]=0;
                work_w[k]=0;
            }
            for(k=0; k < pDOF; ++k){
                const double u = pDOF;
                const double v = k;
                work_z[k] = v /(v + one);
                work_w[k] = v / (u + one);
            }

            err = nbb_ZWGLL(work_z, work_w, pDOF, z, w); IFEB;

            fprintf(fout,"%10u\n", pDOF);
            for(k=0; k < pDOF; ++k){
                fprintf(fout,"%u  %23.14E  %23.14E\n", k +1, z[k], w[k]); //k+1 in order to make the same as the Fortran side.
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
