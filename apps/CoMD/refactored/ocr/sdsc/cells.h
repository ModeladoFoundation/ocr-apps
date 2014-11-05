#ifndef CELLS_H
#define CELLS_H

#include <stdlib.h>
#include <math.h>
#include <ocr.h>

#include "simulation.h"
#include "command.h"

box** init_lattice(simulation* s, command* cmd, real_t lattice_constant, ocrGuid_t** list, ocrGuid_t* ptrs);

void set_temperature(simulation* sim, box** bxs_ptr, real_t temperature);

void random_displacement(box** bxs_ptr, u32 boxes_num, real_t delta);

void create_fcc_lattice(int nx, int ny, int nz, real_t lat, boxes* bxs, box** bxs_ptr);

void init_atoms(ocrGuid_t* list, u32 boxes_num, box** bxs_ptr);

void fork_redistribute(ocrGuid_t sim, ocrGuid_t cont, u32 depc, ocrGuid_t* list, u32 boxes_num);

static inline u32 coordinates2box(u32 grid[3], real3 inv_box_size, real3 r)
{
  return ((u32)(r[0]*inv_box_size[0])) +
         ((u32)(r[1]*inv_box_size[1])*grid[0]) +
         ((u32)(r[2]*inv_box_size[2])*grid[0]*grid[1]);
};

static inline void box2grid(u32 box, u32 bg[3], u32 row, u32 plane)
{
  bg[0] = box%row; bg[1] = (box%plane)/row; bg[2] = box/plane;
};

static inline u32 grid2box(u32 bg[3], u32 row, u32 plane)
{
  return bg[0]+bg[1]*row+bg[2]*plane;
};

static inline u32 faces(u32 bg[3], u32 bbg[3], u32 g[3])
{
  return (abs(bg[0]-bbg[0])<g[0]-1 ? 1 : (bg[0]==0 ? 0 : 2)) +
         (abs(bg[1]-bbg[1])<g[1]-1 ? 3 : (bg[1]==0 ? 0 : 6)) +
         (abs(bg[2]-bbg[2])<g[2]-1 ? 9 : (bg[2]==0 ? 0 : 18));
};

#endif
