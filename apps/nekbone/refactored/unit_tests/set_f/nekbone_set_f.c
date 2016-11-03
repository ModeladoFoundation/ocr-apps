#ifndef UNIT_TEST_NEKBONE_SET_F_C
#define UNIT_TEST_NEKBONE_SET_F_C

#include <stdio.h>
#include <stdlib.h> //malloc, free

#include "set_f.h"

#define Err_t int
#define IFEB if(err) break

int main(int argc, char * argv[])
{
    FILE * fout = 0;

    NBN_REAL *f=0;

    Err_t err=0;
    while(!err){
        fout = fopen("./z_ccode.out", "w");
        if(!fout) {err=__LINE__; IFEB;}

        unsigned int pDOF, elementCount, N;

        unsigned int i;

        //Assuming that pDOF can never be >= 25
        //Assuming that elementCount can never be >= 1000
        f = (NBN_REAL*) malloc((25*25*25*1000)*sizeof(NBN_REAL)); if(!f){err=__LINE__; IFEB;}

        // If any of these values changes, make also the change in nekbone_set_f.c
        pDOF = 25; //Must be <=25
        elementCount = 10; //Must be <= 1000

        N = pDOF * pDOF * pDOF * elementCount;

        err = nbb_set_f(N, f); IFEB;

        for(i=0; i<N; ++i){
            fprintf(fout,"%10u  %10u  %10u  %23.14E\n", pDOF, elementCount, i+1, f[i] );
        }

        if(fout){
            fclose(fout); fout=0;
        }

        break; //while(!err){
    }

    if(f){
        free(f); f=0;
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

#endif // UNIT_TEST_NEKBONE_SET_F_C
