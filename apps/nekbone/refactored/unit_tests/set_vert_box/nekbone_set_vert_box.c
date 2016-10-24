#ifndef UNIT_TEST_NEKBONE_INIT_MESH_C
#define UNIT_TEST_NEKBONE_INIT_MESH_C

#include <stdio.h>
#include <stdlib.h> //malloc & free
#include "cubic.h"
#include "init_mesh.h"
#include "set_vert_box.h"
#include "setup.h"

#define Err_t int
#define IFEB if(err) break

int main(int argc, char * argv[])
{
    FILE * fout = 0;

    unsigned int * lglel = NULL;
    unsigned long * glo_num = NULL;

    Err_t err=0;
    while(!err){
        fout = fopen("./z_ccode.out", "w");
        if(!fout) {err=__LINE__; IFEB;}

        fprintf(fout, "#if_brick,nelt,np,nid, npx,npy,npz, mx,my,mz, nelx,nely,nelz\n");

        unsigned int npx,npy,npz;
        npx = 1;
        npy = 1;
        npz = 1;
        unsigned int mx, my, mz;
        mx = 1;
        my = 1;
        mz = 1;

        lglel = NULL;
        lglel = (unsigned int *) malloc( sizeof(unsigned int) * (G_IELN+1) ); //+1 in order to be safe from the changes
                                                                              //from one-offset Fortran to zero-offset C.
        if(!lglel){
            err = __LINE__; break;
        }

        glo_num = NULL;
        glo_num = (unsigned long *) malloc( sizeof(unsigned long) * (G_NXN * G_NXN * G_NXN * G_IELN +1) ); //+1 in order to be safe from the changes
                                                                                                           //from one-offset Fortran to zero-offset C.
        if(!glo_num){
            err = __LINE__; break;
        }

        unsigned int nx1, ny1, nz1;
        for(nx1=G_NX0; nx1 <=G_NXN; nx1 += G_NXD){
            ny1 = nx1;
            nz1 = nx1;
            unsigned int nelt;
            for(nelt=G_IEL0; nelt <=G_IELN; nelt +=G_IELD){
                unsigned int nid;
                for(nid=G_MPI_RANK_ID_MIN; nid<=G_MPI_RANK_ID_MAX; nid+=G_MPI_RANK_ID_D){
                    unsigned int np = G_MPI_RANK_ID_MAX + 1;
                    unsigned int if_brick;
                    for(if_brick=G_IFBRICK_MIN; if_brick<=G_IFBRICK_MAX; if_brick+=G_IFBRICK_D){
                        int ifbrick;
                        if(if_brick == 0) ifbrick = 0; //False
                        else              ifbrick = 1; //True

                        unsigned int nelx, nely, nelz;

                        npx=1; npy=1; npz=1; //Initialized to invalid values
                        mx=1; my=1; mz=1; //Initialized to invalid values
                        nelx=1; nely=1; nelz=1; //Initialized to invalid values


                        err = nbb_init_mesh(ifbrick, &npx,&npy,&npz, &mx,&my,&mz, nelt,np,nid, &nelx,&nely,&nelz, lglel); IFEB;

                        fprintf(fout, "%u  %u  %u  %u  ", if_brick, nelt, np, nid);
                        fprintf(fout,"%u  %u  %u  ", npx, npy, npz);
                        fprintf(fout,"%u  %u  %u  ", mx, my, mz);
                        fprintf(fout,"%u  %u  %u\n", nelx, nely, nelz);

                        err = nbb_set_vert_box(lglel, nx1,ny1,nz1, nelx,nely,nelz, nelt, glo_num); IFEB;

                        unsigned int i;
                        for(i=0; i<=(G_NXN * G_NXN * G_NXN * G_IELN); ++i){
                            fprintf(fout, "%12u  %12lu\n", i, glo_num[i]);
                        }

                    }//for(if_brick
                    IFEB;
                }//for(nid
                IFEB;
            }//for(nelt
            IFEB;
        } // for(nx1
        IFEB;

        fclose(fout); fout=0;

        break; //while(!err){
    }

    if(fout){
        fclose(fout); fout=0;
    }

    if(lglel){
        free(lglel);
        lglel=NULL;
    }

    if(glo_num){
        free(glo_num);
        glo_num=NULL;
    }

    if(err){
        printf("ERROR: %lu\n", (unsigned long)err );
        return 1;
    }
    return 0;
}

#endif // UNIT_TEST_NEKBONE_INIT_MESH_C
