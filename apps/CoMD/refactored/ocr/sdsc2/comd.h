#ifndef COMD_H
#define COMD_H

#include <string.h>
#include <ocr.h>

#include "constants.h"

typedef double real_t;
typedef real_t real3_t[3];

typedef struct
{
  real_t sigma;
  real_t epsilon;
} lj_t;

typedef struct
{
  u32 n;
  real_t x0;
  real_t xn;
  real_t inv_dx;
  ocrGuid_t values;
} interpolator_t;

typedef struct
{
  interpolator_t phi;
  interpolator_t rho;
  interpolator_t f;
} eam_t;

typedef struct
{
  real_t mass[MAXSPECIES];
  real_t inv_mass[MAXSPECIES];
  real_t inv_mass_2[MAXSPECIES];
  real_t inv_mass_dt[MAXSPECIES];
} mass_t;

typedef struct
{
  u8 lattice_type;
  u8 potential;

  real_t cutoff;
  ocrGuid_t mass;
  real_t lat;
  u32 atomic_no;

  void (*print)(void* p);

  ocrGuid_t (*force_edt)(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
  ocrGuid_t (*forcevel_edt)(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

  union {
    lj_t lj;
    eam_t eam;
  };
} potential_t;

typedef struct
{
  u32 atoms;
  u8 nmask;
  real3_t r[MAXATOMS];
  real3_t p[MAXATOMS];
  real3_t f[MAXATOMS];
  real_t u[MAXATOMS];
  u8 s[MAXATOMS];
  ocrGuid_t box;
  ocrGuid_t nextpf;
} rpf_t;

typedef struct
{
  u32 coordinates[3];
  real3_t min;
  real3_t max;
  u32 max_occupancy;
  u32 gid[MAXATOMS];
  ocrGuid_t rneighbors[26];
  ocrGuid_t bneighbors[26];
} box_t;

typedef struct
{
  real3_t domain;
  u32 grid[3];
  u32 boxes_num;
  real3_t box_size;
  real3_t inv_box_size;
  ocrGuid_t box;
  ocrGuid_t rpf;
} boxes_t;

typedef struct
{
  real3_t value;
  ocrGuid_t reduction;
  ocrGuid_t sim;
} reduction_t;

typedef struct
{
  u32 step;
  u32 steps;
  u32 period;
  real_t dt;

  potential_t pot;

  boxes_t boxes;

  real_t e_potential;
  real_t e_kinetic;
  u32 atoms;

  double energy0;
  u32 atoms0;

  ocrGuid_t reduction;

} simulation_t;

static inline u32 neighbor_id(u8 n, u32* coordinates, u32* grid)
{
  u32 c[3] = {coordinates[0], coordinates[1], coordinates[2]};

  if(n>12)
    ++n;

  if(n<9) {
    if(c[2]) --c[2];
    else c[2] = grid[2]-1;
  } else if(n>17) {
    if(c[2]+1==grid[2]) c[2]=0;
    else ++c[2];
  }
  n%=9;
  if(n<3) {
    if(c[1]) --c[1];
    else c[1] = grid[1]-1;
  } else if(n>5) {
    if(c[1]+1==grid[1]) c[1]=0;
    else ++c[1];
  }
  n%=3;
  c[0] = (c[0]+grid[0]-1+n)%grid[0];

  return (c[0]*grid[1]+c[1])*grid[2]+c[2];
}

#endif
