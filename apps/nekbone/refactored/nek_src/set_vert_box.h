#ifndef NEKBONE_SET_VERT_BOX_H
#define NEKBONE_SET_VERT_BOX_H

//Returns zero upon success.
int nbb_mod1(int i, int n, int * o_mod1);

//Returns zero upon success.
int nbb_get_exyz(unsigned int in_nelx, unsigned int in_nely, unsigned int in_nelz,
                 unsigned int in_eg,
                 unsigned int * o_ex, unsigned int * o_ey, unsigned int * o_ez);

//Returns zero upon success.
int nbb_set_vert_box(unsigned int * in_lglel,
                     unsigned int in_nx1, unsigned int in_ny1, unsigned int in_nz1,
                     unsigned int in_nelx, unsigned int in_nely, unsigned int in_nelz,
                     unsigned int in_nelt,
                     unsigned long * o_glo_num
                     );

#endif // NEKBONE_SET_VERT_BOX_H
