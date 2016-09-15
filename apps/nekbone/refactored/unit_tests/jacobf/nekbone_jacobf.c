#ifndef UNIT_TEST_NEKBONE_JACOBF_C
#define UNIT_TEST_NEKBONE_JACOBF_C

#include <stdio.h>
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

        double POLY,PDER, POLYM1,PDERM1, POLYM2,PDERM2;

        double alpha, beta;
        double x,sz;

        unsigned int podegree, firstdegree, lastdegree;
        unsigned int sampleCount,s;

        alpha = 0; // This is how it is use throughout NEKbone.
        beta  = 0; // This is how it is use throughout NEKbone.

        // If any of these values changes, make also the change in nekbone_jacobf.f90
        firstdegree = 2;
        lastdegree  = 16;
        sampleCount = 10;

        sz = sampleCount;
        for(podegree=firstdegree; podegree<=lastdegree; ++podegree){
            for(s=1; s<=sampleCount; ++s){
                x = 1;
                x = x /(sz +1);

                err = nbb_JACOBF(podegree, alpha,beta, x,
                        &POLY, &PDER, &POLYM1,&PDERM1, &POLYM2,&PDERM2); IFEB;

                fprintf(fout, "%10u%10u%23.14E%23.14E%23.14E%23.14E%23.14E%23.14E\n",
                        podegree,s,  POLY,PDER, POLYM1,PDERM1, POLYM2,PDERM2
                       );
            }
            IFEB;
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

#endif // UNIT_TEST_NEKBONE_JACOBF_C
