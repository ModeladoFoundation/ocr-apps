#ifndef NEKBONE_INIT_MESH_H
#include "init_mesh.h"
#endif

#ifndef NEKBONE_CUBIC_H
#include "cubic.h"
#endif

//DBG> #include "ocr.h"

int nbb_init_mesh(int in_ifbrick, //Set to 1 for a brick; otherwise zero.
              unsigned int * o_npx, unsigned int * o_npy, unsigned int * o_npz,
              unsigned int * o_mx, unsigned int * o_my, unsigned int * o_mz,
              unsigned int in_nelt, //The number of element per ranks
              unsigned int in_np, //The total number of ranks in use
              unsigned int in_nid, //The ID for the current rank: 0< nid < np
              unsigned int * o_nelx, unsigned int * o_nely, unsigned int * o_nelz,
              unsigned int * o_lglel)
{
    int err = 0;
    while(!err){
        if(!o_lglel) {err=__LINE__; break;}
        *o_nelx = 1;
        *o_nely = 1;
        *o_nelz = 1;

        o_lglel[0]=0;

        if(in_ifbrick == 0 ){
            // A 1-D array of elements of length P*lelt
            *o_nelx = in_nelt*in_np;
            *o_nely = 1;
            *o_nelz = 1;

            unsigned int e;
            for(e=1; e<=in_nelt; ++e){
                unsigned int eg;
                eg = e + in_nid * in_nelt;
                o_lglel[e] = eg;
            }
        } else {
            //! A 3-D block of elements
            if ((*o_npx) * (*o_npy) * (*o_npz) != in_np){
                unsigned long nx, ny, nz;
                nx=0; ny=0; nz=0;
                err = nek_cubic(&nx, &ny, &nz, in_np); if(err) break; // xyz distribution of total proc
                *o_npx = nx;
                *o_npy = ny;
                *o_npz = nz;
            }
            if ((*o_mx) * (*o_my) * (*o_mz) != in_nelt){
                unsigned long mx, my, mz;
                mx=0; my=0; mz=0;
                err = nek_cubic(&mx,&my,&mz,in_nelt); if(err) break; //xyz distribution of elements per proc
                *o_mx = mx;
                *o_my = my;
                *o_mz = mz;
            }

            *o_nelx = (*o_mx) * (*o_npx);
            *o_nely = (*o_my) * (*o_npy);
            *o_nelz = (*o_mz) * (*o_npz);

            unsigned int e = 1;

            unsigned int offs;
            //offs = (mod(in_nid,npx)*mx) + npx*(my*mx)*(mod(in_nid/npx,npy)) + (npx*npy)*(mx*my*mz)*(in_nid/(npx*npy))
            unsigned int mxmy = (*o_my) * (*o_mx);
            unsigned int mxmymz = mxmy * (*o_mz);
            unsigned int npxnpy = (*o_npx) * (*o_npy);
            offs = (in_nid % (*o_npx)) * (*o_mx);
            offs += (*o_npx) * mxmy * ( (in_nid/(*o_npx)) % (*o_npy) );
            offs += npxnpy * mxmymz * (in_nid/npxnpy);

            unsigned int i, j, k;
            for(k=0; k < *o_mz; ++k){
                for(j=0; j < *o_my; ++j){
                    for(i=0; i < *o_mx; ++i){
                       unsigned int eg;
                       eg = offs + i + (j * (*o_nelx)) + (k*(*o_nelx)*(*o_nely)) + 1;
                       o_lglel[e] = eg;
                       //DBG> PRINTF("DBG lglel> Rid i,j,k,e,offs o_lglel  Rid=%u %u,%u,%u e=%u offs=%u  lglel[e]=%u\n", in_nid, i,j,k,e, offs,  eg);
                       ++e;
                    }
                }
            }
        }

        break;
    }//while(!err)
    return err;
}
