#ifndef NEKBONE_INIT_MESH_H
#define NEKBONE_INIT_MESH_H

//It returns zero upon success.
int nbb_init_mesh(int in_ifbrick, //Set to 1 for a brick; otherwise zero.
              unsigned int * o_npx, unsigned int * o_npy, unsigned int * o_npz,
              unsigned int * o_mx, unsigned int * o_my, unsigned int * o_mz,
              unsigned int in_nelt, //The number of element per ranks
              unsigned int in_np, //The total number of ranks in use
              unsigned int in_nid, //The ID for the current rank: 0< nid < np
              unsigned int * o_nelx, unsigned int * o_nely, unsigned int * o_nelz,
              unsigned int * o_lglel);

#endif // NEKBONE_INIT_MESH_H
