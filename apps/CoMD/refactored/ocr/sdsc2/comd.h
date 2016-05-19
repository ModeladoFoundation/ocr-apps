#ifndef COMD_H
#define COMD_H

#include <string.h>
#include <math.h>
#include <ocr.h>

#include "constants.h"

void getPartitionID(s64 i, s64 lb_g, s64 ub_g, s64 R, s64* id);
void splitDimension(s64 Num_procs, s64* Num_procsx, s64* Num_procsy, s64* Num_procsz);

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
  real_t massFloat;
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
  ocrGuid_t OEVT_reduction;
  ocrGuid_t sim;
} reductionH_t;

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

  ocrGuid_t reductionH_g;

} simulation_t;

//Param structs
typedef struct
{
    ocrGuid_t EDT_finalize;
} PRM_period_edt_t;

typedef struct
{
    u32 grid[3];
    u32 lattice[3];
    real_t delta, temperature;
} PRM_fork_init_edt_t;

typedef struct
{
    u32 coords[3];
    u32 lattice[3];
    real_t delta, temperature;
    ocrGuid_t vleaf, tleaf, uleaf;
} PRM_FNC_init_t;

typedef struct
{
    ocrGuid_t tleaf;
    ocrGuid_t EDT_sched;
} PRM_vcm_edt_t;

typedef struct
{
    real_t dt;
    ocrGuid_t uleaf;
} PRM_ukvel_edt_t;

typedef struct
{
    lj_t lj;
    real_t cutoff;
    real_t domain[3];
} PRM_force_edt_t;

typedef struct
{
    u32 n;
    real_t temperature, epsilon;
    ocrGuid_t guid;
} PRM_red_t;

typedef struct
{
    u32 n;
    real_t epsilon;
} PRM_ured_edt_t;

typedef struct
{
    u32 n;
    real_t temperature;
    ocrGuid_t guid;
} PRM_tred_edt_t;

typedef struct
{
    u32 n;
    ocrGuid_t guid;
} PRM_vred_edt_t;

typedef struct
{
    real_t ds, dt;
    ocrGuid_t EDT_exchange, boxes_g;
} PRM_position_edt_t;

typedef struct
{
    real_t ds, sigma, epsilon, cutoff;
    real_t domain[3];
    ocrGuid_t leaves_g;
} PRM_forcevel_edt_t;

typedef struct
{
    real_t dt;
    ocrGuid_t leaves_g;
} PRM_veluk_edt_t;


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

static inline int globalRankFromCoords( int id_x, int id_y, int id_z, int NR_X, int NR_Y, int NR_Z )
{
    return NR_X*NR_Y*id_z + NR_X*id_y + id_x;
}

static inline int getPoliyDomainID( int b, u32* grid, int PD_X, int PD_Y, int PD_Z )
{
    int id_x = (b/grid[2])/grid[1];
    int id_y = (b/grid[2])%grid[1];
    int id_z = b%grid[2];

    s64 pd_x; getPartitionID(id_x, 0, grid[0]-1, PD_X, &pd_x);
    s64 pd_y; getPartitionID(id_y, 0, grid[1]-1, PD_Y, &pd_y);
    s64 pd_z; getPartitionID(id_z, 0, grid[2]-1, PD_Z, &pd_z);

    //Each linkcell, with id=b, is mapped to a PD. The mapping is similar to how the link cells map to
    //MPI ranks. In other words, all the PDs are arranged as a 3-D grid.
    //And, a 3-D subgrid of linkcells is mapped to a PD preserving "locality" within a PD.
    //
    int pd = globalRankFromCoords(pd_x, pd_y, pd_z, PD_X, PD_Y, PD_Z);
    //PRINTF("%d box %d %d %d, policy domain %d: %d %d %d\n", b, id_x, id_y, id_z, pd, PD_X, PD_Y, PD_Z);

    return pd;
}

#endif
