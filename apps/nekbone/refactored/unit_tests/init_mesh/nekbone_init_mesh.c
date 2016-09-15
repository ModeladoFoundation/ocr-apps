#ifndef UNIT_TEST_NEKBONE_INIT_MESH_C
#define UNIT_TEST_NEKBONE_INIT_MESH_C

#include <stdio.h>
#include "cubic.h"

#define Err_t int
#define IFEB if(err) break

int main(int argc, char * argv[])
{
    FILE * fin = 0;
    FILE * fout = 0;

    Err_t err=0;
    while(!err){
        fin = fopen("inputparams.dat", "r");
        if(!fin) {err=__LINE__; IFEB;}
        unsigned int maxValue = 0;
        fscanf(fin, "%u", &maxValue);
        fclose(fin); fin=0;
        //printf("maxValue = %u\n", maxValue);

        fout = fopen("./ccode.out", "w");
        if(!fout) {err=__LINE__; IFEB;}

        unsigned long k;
        for(k=1; k<=maxValue; ++k){
            unsigned long x,y,z;
            err = nek_cubic(&x,&y, &z, k);
            fprintf(fout,"%10lu %10lu %10lu %10lu\n", k, x,y,z);
        }
        IFEB;
        fclose(fout); fout=0;

        break; //while(!err){
    }

    if(fin){
        fclose(fin); fin=0;
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

#endif // UNIT_TEST_NEKBONE_INIT_MESH_C
