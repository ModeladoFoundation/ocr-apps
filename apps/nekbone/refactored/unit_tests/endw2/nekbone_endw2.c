#ifndef UNIT_TEST_NEKBONE_GAMMAF_C
#define UNIT_TEST_NEKBONE_GAMMAF_C

#include <stdio.h>
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

        const unsigned int dof = 25; //This has to match what is in the corresponding .f90 file.

        unsigned int ab;
        for(ab=0; ab <=1; ++ab){
            double alpha, beta;
            if(ab==0){
                alpha = 0;
                beta  = 0;
            } else {
                alpha = -0.5;
                beta  = -0.5;
            }
            unsigned int i;
            for(i=0; i<=dof; ++i){
                double f = nbb_endw2(i, alpha, beta);
                fprintf(fout,"%10u %10u %23.14E\n", i, ab, f);
            }
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

#endif // UNIT_TEST_NEKBONE_GAMMAF_C
