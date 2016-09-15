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
        fout = fopen("./ccode.out", "w");
        if(!fout) {err=__LINE__; IFEB;}

        double x,y;

        const int N = 15;
        double samples[15];
        int i;

        samples[ 0] =  0;
        samples[ 1] = -0.5;
        samples[ 2] =  0.5;
        samples[ 3] =  1;
        samples[ 4] =  2;
        samples[ 5] =  1.5;
        samples[ 6] =  2.5;
        samples[ 7] =  3;
        samples[ 8] =  3.5;
        samples[ 9] =  4;
        samples[10] =  5;
        samples[11] =  6;
        samples[12] =  1.25;
        samples[13] =  3.25;
        samples[14] =  5.25;

        for(i=0; i<N; ++i){
            x = samples[i];
            y = nbb_GAMMAF(x);
            fprintf(fout, "%10u%23.14E%23.14E\n", i+1, x,y);
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
