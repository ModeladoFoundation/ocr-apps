#ifndef NEKBONE_SET_VERT_BOX_H
#include "set_vert_box.h"
#endif

int nbb_mod1(int i, int n, int * o_mod1)
{
    *o_mod1 = 0;

    if(i == 0) return 0;
    if(n == 0) return __LINE__; //'WARNING:  Attempt to take MOD(I,0) in function mod1.'

    i += n - 1;

    *o_mod1 = (i % n) + 1;

    return 0;
}

int nbb_get_exyz(unsigned int in_nelx, unsigned int in_nely, unsigned int in_nelz,
                  unsigned int in_eg,
                  unsigned int * o_ex, unsigned int * o_ey, unsigned int * o_ez)
{
    const unsigned int nelxy = in_nelx * in_nely;

    *o_ez = 1 + (in_eg - 1) / nelxy;

    int err = nbb_mod1(in_eg, nelxy, o_ey);
    if(err) return err;

    *o_ey = 1 + ((*o_ey) - 1)/ in_nelx;

    err = nbb_mod1(in_eg, in_nelx, o_ex);

    return err;
}

int nbb_set_vert_box(unsigned int * in_lglel,
                     unsigned int in_nx1, unsigned int in_ny1, unsigned int in_nz1,
                     unsigned int in_nelx, unsigned int in_nely, unsigned int in_nelz,
                     unsigned int in_nelt,
                     unsigned long * o_glo_num
                     )
{
    // Set up global numbering for elements in a box
    unsigned long ii, kg, jg, ig;  //The latter 3 for proper promotion

    unsigned int nn = in_nx1 -1; // nn := polynomial order

    o_glo_num[0] = 0;

    int err = 0;

    unsigned int e;
    for(e=1; e<=in_nelt; ++e){
        unsigned int eg = in_lglel[e];
        unsigned int ex=0, ey=0, ez=0;
        err = nbb_get_exyz(in_nelx,in_nely,in_nelz, eg, &ex, &ey, &ez);
        if(err) return err;
        unsigned int k;
        for(k=0; k<=nn; ++k){
            unsigned int j;
            for(j=0; j<=nn; ++j){
                unsigned int i;
                for(i=0; i<=nn; ++i){
                    kg = nn * (ez -1) +k;
                    jg = nn * (ey -1) +j;
                    ig = nn * (ex -1) +i;

                    ii = 1 + ig + jg*(nn*in_nelx+1) + kg *(nn*in_nelx+1) * (nn*in_nely+1);

                    unsigned long ll;
                    ll = 1 + i + in_nx1*j + in_nx1*in_ny1*k + in_nx1*in_ny1*in_nz1*(e-1);

                    o_glo_num[ll] = ii;
                }//for(i
            }//for(j
        }//for(k
    }//for(e

    return 0;
}
