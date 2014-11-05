#include <math.h>
#include <ocr.h>

#include "simulation.h"
#include "cells.h"

#define DOUBLE_VAL (1.0/UINT64_C(2305843009213693951))

static inline double lcg61(u64* seed)
{
  *seed *= UINT64_C(437799614237992725);
  *seed %= UINT64_C(2305843009213693951);
  return *seed*DOUBLE_VAL;
}

static inline real_t gasdev(u64* seed)
{
   real_t rsq,v2;
   do {
      rsq = 2.0*lcg61(seed)-1.0;
      v2 = 2.0*lcg61(seed)-1.0;
      rsq = rsq*rsq+v2*v2;
   } while(rsq >= 1.0 || rsq == 0.0);

   return v2 * sqrt(-2.0*log(rsq)/rsq);
}

static inline u64 mk_seed(u32 id, u32 call_site)
{
   u32 s1 = id * UINT32_C(2654435761);
   u32 s2 = (id+call_site) * UINT32_C(2654435761);

   u64 seed = (UINT64_C(0x100000000) * s1) + s2;
   lcg61(&seed); lcg61(&seed);
   lcg61(&seed); lcg61(&seed);
   lcg61(&seed); lcg61(&seed);
   lcg61(&seed); lcg61(&seed);
   lcg61(&seed); lcg61(&seed);

   return seed;
}

void init_atoms(ocrGuid_t* list, u32 boxes_num, box** bxs_ptr)
{
  for(u32 b = 0; b<boxes_num; ++b) {
    ocrDbCreate(list+b, (void**)bxs_ptr+b, sizeof(box), 0, NULL_GUID, NO_ALLOC);
    bxs_ptr[b]->atoms = 0;
  }
}

void create_fcc_lattice(int nx, int ny, int nz, real_t lat, boxes* bxs, box** bxs_ptr)
{
  int nb = 4; // number of atoms in the basis
  real3 basis[4] = { {0.25, 0.25, 0.25},
                     {0.25, 0.75, 0.75},
                     {0.75, 0.25, 0.75},
                     {0.75, 0.75, 0.25} };

  bxs->atoms = 0;
  real3 r;
  u32 gid = 0;
  for(u32 ix=0; ix<nx; ++ix)
    for(u32 iy=0; iy<ny; ++iy)
      for(u32 iz=0; iz<nz; ++iz)
        for(u32 ib=0; ib<nb; ++ib) {
          r[0] = (ix+basis[ib][0]) * lat;
          r[1] = (iy+basis[ib][1]) * lat;
          r[2] = (iz+basis[ib][2]) * lat;
          box* box_ptr = bxs_ptr[coordinates2box(bxs->grid, bxs->inv_box_size, r)];
          box_ptr->gid[box_ptr->atoms] = gid;
          box_ptr->r[box_ptr->atoms][0] = r[0];
          box_ptr->r[box_ptr->atoms][1] = r[1];
          box_ptr->r[box_ptr->atoms][2] = r[2];
          box_ptr->p[box_ptr->atoms][0] = box_ptr->p[box_ptr->atoms][1] = box_ptr->p[box_ptr->atoms][2] =
          box_ptr->f[box_ptr->atoms][0] = box_ptr->f[box_ptr->atoms][1] = box_ptr->f[box_ptr->atoms][2] =
          box_ptr->U[box_ptr->atoms] = box_ptr->a[box_ptr->atoms] = 0;
          ++box_ptr->atoms;
          ++bxs->atoms; ++gid;
        }
}

void random_displacement(box** bxs_ptr, u32 boxes_num, real_t delta)
{
  for(u32 b = 0; b<boxes_num; ++b)
    for(u32 a = 0; a < bxs_ptr[b]->atoms; ++a) {
      u64 seed = mk_seed(bxs_ptr[b]->gid[a], 457);
      bxs_ptr[b]->r[a][0] += (2.0*lcg61(&seed)-1.0) * delta;
      bxs_ptr[b]->r[a][1] += (2.0*lcg61(&seed)-1.0) * delta;
      bxs_ptr[b]->r[a][2] += (2.0*lcg61(&seed)-1.0) * delta;
    }
}

static void zero_vcm(box** bxs_ptr, u32 boxes_num, u32 atoms)
{
  real3 vcm;
  vcm[0] = vcm[1] = vcm[2] = 0;

  for(u32 b = 0; b<boxes_num; ++b)
    for(u32 a = 0; a < bxs_ptr[b]->atoms; ++a) {
      vcm[0] += bxs_ptr[b]->p[a][0];
      vcm[1] += bxs_ptr[b]->p[a][1];
      vcm[2] += bxs_ptr[b]->p[a][2];
    }
  vcm[0]/=atoms;
  vcm[1]/=atoms;
  vcm[2]/=atoms;

  for(u32 b = 0; b<boxes_num; ++b)
    for(u32 a = 0; a < bxs_ptr[b]->atoms; ++a) {
      bxs_ptr[b]->p[a][0] -= vcm[0];
      bxs_ptr[b]->p[a][1] -= vcm[1];
      bxs_ptr[b]->p[a][2] -= vcm[2];
    }
}

void set_temperature(simulation* sim, box** bxs_ptr, real_t temperature)
{
  real_t mass = sim->pot.mass;
  for(u32 b = 0; b<sim->bxs.boxes_num; ++b)
    for(u32 a = 0; a < bxs_ptr[b]->atoms; ++a) {
      real_t sigma = sqrt(kB_eV * temperature/mass);
      u64 seed = mk_seed(bxs_ptr[b]->gid[a], 123);
      bxs_ptr[b]->p[a][0] = mass*sigma*gasdev(&seed);
      bxs_ptr[b]->p[a][1] = mass*sigma*gasdev(&seed);
      bxs_ptr[b]->p[a][2] = mass*sigma*gasdev(&seed);
    }

  if (temperature == 0.0) return;
  zero_vcm(bxs_ptr, sim->bxs.boxes_num, sim->bxs.atoms);
  kinetic_energy(sim, bxs_ptr, sim->bxs.boxes_num);
  real_t temp = (sim->e_kinetic/sim->bxs.atoms)*kB_eV_1_5;
  real_t scale_factor = sqrt(temperature/temp);
  for(u32 b = 0; b<sim->bxs.boxes_num; ++b)
    for(u32 a = 0; a < bxs_ptr[b]->atoms; ++a) {
      bxs_ptr[b]->p[a][0] *= scale_factor;
      bxs_ptr[b]->p[a][1] *= scale_factor;
      bxs_ptr[b]->p[a][2] *= scale_factor;
    }
}
