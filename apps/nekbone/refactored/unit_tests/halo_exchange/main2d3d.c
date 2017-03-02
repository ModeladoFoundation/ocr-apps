#include <stdio.h>
#include <stdlib.h> //malloc,free

#include "app_ocr_err_util.h"
#include "nekos_triplet.h"
#include "neko_halo_util.h"

int test1_2D()
{
    NBN_REAL * V = 0;
    rankIndexedValue_t * riV = 0;

    int err=0;
    while(!err){
        const Triplet R = {3,3,1};  //The rank partition
        const Triplet E = {2,2,1};  //The element per rank partition
        const Triplet D = {3,3,1};  //The DOF partition

        const Idz rank_id = 1;

        if( rank_id < 0 || abcT(R) <= rank_id ){
            err=__LINE__;
            break;
        }

        Idz sz_rankIndexedValue = calculate_length_rankIndexedValue(D.a, E);
        printf("TEST1_2D> sz_rankIndexedValue= %ld\n", sz_rankIndexedValue);

        riV = (rankIndexedValue_t*) malloc( sz_rankIndexedValue *sizeof(rankIndexedValue_t) );
        if(!riV) { err=__LINE__; break; }

        const Idz endE = abcT(E);
        const Idz endD = abcT(D);

        Idz sz_V = endD * endE;
        V = (NBN_REAL*) malloc( sz_V * sizeof(NBN_REAL));
        if(!V) { err=__LINE__; break;}

        Idz i;
        for(i=0; i<sz_V; ++i) V[i]=1;

        Idz sz_haloValues = 0;
        err = halo_exchanges2(R, E, D, rank_id, sz_V, V, &sz_haloValues, riV); IFEB;

        for(i=0; i<sz_V; ++i){
            Idz e_id = i/abcT(D);
            Idz d_id = i - e_id * abcT(D);

            Triplet r = index_to_coords(rank_id, R);
            Triplet e = {0}; //The element coordinates
            Triplet d = {0}; //The pDOF coordinates

            e = index_to_coords(e_id, E);
            d = index_to_coords(d_id, D);

            Triplet re = build_coordinateRE(E, e, r);

            char bufd[256], bufre[256];
            printf("TEST1_2D> rankID=%ld de=%ld V=%21.14e d=%s re=%s\n",
                   rank_id, i, V[i],
                   triplet2text(bufd,d), triplet2text(bufre,re)
                   );
        }IFEB;

        for(i=0; i<sz_haloValues; ++i){
            printf("TEST1_2D> HALO> i=%ld FromR=%ld val=%14.14e ToR=%ld AT de=%ld\n",
                   i, rank_id, riV[i].value, riV[i].rankID, riV[i].eleDofID
                  );
        }

        break;
    }

    if(V){
        free(V);
        V = 0;
    }
    if(riV){
        free(riV);
        riV = 0;
    }

    return err;
}

int test1_3D()
{
    NBN_REAL * V = 0;
    rankIndexedValue_t * riV = 0;

    int err=0;
    while(!err){
        const Triplet R = {3,3,3};  //The rank partition
        const Triplet E = {2,2,2};  //The element per rank partition
        const Triplet D = {3,3,3};  //The DOF partition

        const Idz rank_id = 13;

        if( rank_id < 0 || abcT(R) <= rank_id ){
            err=__LINE__;
            break;
        }

        Idz sz_rankIndexedValue = calculate_length_rankIndexedValue(D.a, E);
        printf("TEST1_3D> sz_rankIndexedValue= %ld\n", sz_rankIndexedValue);

        riV = (rankIndexedValue_t*) malloc( sz_rankIndexedValue *sizeof(rankIndexedValue_t) );
        if(!riV) { err=__LINE__; break; }

        const Idz endE = abcT(E);
        const Idz endD = abcT(D);

        Idz sz_V = endD * endE;
        V = (NBN_REAL*) malloc( sz_V * sizeof(NBN_REAL));
        if(!V) { err=__LINE__; break;}

        Idz i;
        for(i=0; i<sz_V; ++i) V[i]=1;

        Idz sz_haloValues = 0;
        err = halo_exchanges2(R, E, D, rank_id, sz_V, V, &sz_haloValues, riV); IFEB;

        for(i=0; i<sz_V; ++i){
            Idz e_id = i/abcT(D);
            Idz d_id = i - e_id * abcT(D);

            Triplet r = index_to_coords(rank_id, R);
            Triplet e = {0}; //The element coordinates
            Triplet d = {0}; //The pDOF coordinates

            e = index_to_coords(e_id, E);
            d = index_to_coords(d_id, D);

            Triplet re = build_coordinateRE(E, e, r);

            char bufd[256], bufre[256];
            printf("TEST1_3D> rankID=%ld de=%ld V=%21.14e d=%s re=%s\n",
                   rank_id, i, V[i],
                   triplet2text(bufd,d), triplet2text(bufre,re)
                   );
        }IFEB;

        for(i=0; i<sz_haloValues; ++i){
            printf("TEST1_3D> HALO> i=%ld FromR=%ld val=%14.14e ToR=%ld AT de=%ld\n",
                   i, rank_id, riV[i].value, riV[i].rankID, riV[i].eleDofID
                  );
        }

        break;
    }

    if(V){
        free(V);
        V = 0;
    }
    if(riV){
        free(riV);
        riV = 0;
    }

    return err;
}

int main(int argc, char * argv[])
{
    int err=0;
    while(!err){
        err = test1_2D(); IFEB;
        err = test1_3D(); IFEB;
        break;
    }

    if(err){
        printf("ERROR: %d\n", err);
        return 1;
    }
    return err;
}
