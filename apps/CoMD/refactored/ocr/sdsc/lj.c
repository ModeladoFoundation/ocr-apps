#include <stdio.h>
#include <string.h>

#include "simulation.h"
#include "cells.h"

#define POT_SHIFT 1.0

void fork_lj_force(ocrGuid_t sim, simulation* sim_ptr, ocrGuid_t cont, ocrGuid_t* list, u32 grid[3]);

void lj_print(potential* p)
{
  PRINTF("  Potential type   : Lennard-Jones\n");
  PRINTF("  Species name     : %s\n", p->name);
  PRINTF("  Atomic number    : %d\n", p->atomic_no);
  PRINTF("  Mass             : "FMT1" amu\n", p->mass / amu2internal_mass); // print in amu
  PRINTF("  Lattice Type     : %s\n", p->lattice_type);
  PRINTF("  Lattice spacing  : %f Angstroms\n", p->lat);
  PRINTF("  Cutoff           : %f Angstroms\n", p->cutoff);
  PRINTF("  Epsilon          : %f eV\n", p->lj.epsilon);
  PRINTF("  Sigma            : %f Angstroms\n", p->lj.sigma);
}

void init_lj(potential* p, real_t dt)
{
  p->cutoff = 2.5*2.315;
  p->mass = 63.55 * amu2internal_mass;
  p->inv_mass_2 = 1/(2*p->mass);
  p->inv_mass_dt = dt/p->mass;
  p->lat = 3.615;
  strcpy(p->lattice_type, "FCC");
  strcpy(p->name, "Cu");
  p->atomic_no = 29;
  p->print = lj_print;
  p->fork_force_computation = fork_lj_force;
  p->lj.epsilon = 0.167;
  p->lj.sigma = 2.315;
  p->eam.f.values = NULL_GUID;
}

static real_t lj_force_box(box* a, box* b, real3 fshift, real_t sigma, real_t epsilon, real_t cut)
{
  real_t e_pot = 0;
  real_t cut2 = cut*cut;
  real_t s6 = sigma*sigma*sigma*sigma*sigma*sigma;
  real_t cut6 = s6/(cut2*cut2*cut2);
  real_t shift = POT_SHIFT*cut6*(cut6-1);

  for(u32 aa = 0; aa < a->atoms; ++aa)
    for(u32 bb = 0; bb < b->atoms; ++bb) {

      real_t r2; real3 dr;
      dr[0] = a->r[aa][0]-(b->r[bb][0]+fshift[0]);
      dr[1] = a->r[aa][1]-(b->r[bb][1]+fshift[1]);
      dr[2] = a->r[aa][2]-(b->r[bb][2]+fshift[2]);
      r2=dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2];

      if ( r2 > cut2) continue;
      r2 = 1.0/r2;

      real_t r6 = s6 * (r2*r2*r2);
      real_t energy = (r6*(r6-1.0)-shift)*0.5;
      a->U[aa] += energy;
      e_pot += energy;

      real_t fr = -4.0*epsilon*r6*r2*(12.0*r6-6.0);
      a->f[aa][0] -= dr[0]*fr;
      a->f[aa][1] -= dr[1]*fr;
      a->f[aa][2] -= dr[2]*fr;
    }

  return e_pot*4.0*epsilon;
}

static real_t lj_force_box_same(box* a, real_t sigma, real_t epsilon, real_t cut)
{
  real_t e_pot = 0;
  real_t s6 = sigma*sigma*sigma*sigma*sigma*sigma;
  real_t cut2 = cut*cut;
  real_t cut6 = s6/(cut2*cut2*cut2);
  real_t shift = POT_SHIFT*cut6*(cut6-1);

  for(u32 aa = 0; aa < a->atoms; ++aa)
    for(u32 bb = aa+1; bb < a->atoms; ++bb) {

      real_t r2; real3 dr;
      dr[0] = a->r[aa][0]-a->r[bb][0];
      dr[1] = a->r[aa][1]-a->r[bb][1];
      dr[2] = a->r[aa][2]-a->r[bb][2];
      r2=dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2];

      if(r2 > cut2) continue;
      r2 = 1.0/r2;

      real_t r6 = s6 * (r2*r2*r2);
      real_t energy = r6*(r6-1.0)-shift;
      a->U[aa] += energy*0.5;
      a->U[bb] += energy*0.5;
      e_pot += energy;

      real_t fr = -4.0*epsilon*r6*r2*(12.0*r6-6.0);
      a->f[aa][0] -= dr[0]*fr;
      a->f[aa][1] -= dr[1]*fr;
      a->f[aa][2] -= dr[2]*fr;
      a->f[bb][0] += dr[0]*fr;
      a->f[bb][1] += dr[1]*fr;
      a->f[bb][2] += dr[2]*fr;
    }

  return e_pot*4.0*epsilon;
}

static ocrGuid_t lj_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation* sim = (simulation*)depv[0].ptr;
  ocrGuid_t res; real_t* res_ptr;
  ocrDbCreate(&res, (void**)&res_ptr, sizeof(real_t), 0, NULL_GUID, NO_ALLOC);
  *res_ptr = lj_force_box_same((box*)depv[1].ptr, sim->pot.lj.sigma, sim->pot.lj.epsilon, sim->pot.cutoff);
  u32 dep = 1;
  u32 plane = sim->bxs.grid[0]*sim->bxs.grid[1];
  u32 bg[3]; box2grid(paramv[1], bg, sim->bxs.grid[0], plane);
  u32 bbg[3];
  // Previous plane, previous row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? sim->bxs.grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2]==0 ? sim->bxs.grid[2]-1 : bg[2]-1;
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += lj_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.lj.sigma, sim->pot.lj.epsilon, sim->pot.cutoff);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Previous plane, same rsim->bxs.ow
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += lj_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.lj.sigma, sim->pot.lj.epsilon, sim->pot.cutoff);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Previous plane, next row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==sim->bxs.grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += lj_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.lj.sigma, sim->pot.lj.epsilon, sim->pot.cutoff);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Same plane, previous row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? sim->bxs.grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2];
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += lj_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.lj.sigma, sim->pot.lj.epsilon, sim->pot.cutoff);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Same plane, same row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  *res_ptr += lj_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.lj.sigma, sim->pot.lj.epsilon, sim->pot.cutoff);
  ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  *res_ptr += lj_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.lj.sigma, sim->pot.lj.epsilon, sim->pot.cutoff);
  // Same plane, next row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==sim->bxs.grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += lj_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.lj.sigma, sim->pot.lj.epsilon, sim->pot.cutoff);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Next plane, previous row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? sim->bxs.grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2]==sim->bxs.grid[2]-1 ? 0 : bg[2]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += lj_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.lj.sigma, sim->pot.lj.epsilon, sim->pot.cutoff);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Next plane, same row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += lj_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.lj.sigma, sim->pot.lj.epsilon, sim->pot.cutoff);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Next plane, next row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==sim->bxs.grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += lj_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.lj.sigma, sim->pot.lj.epsilon, sim->pot.cutoff);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  ocrAddDependence(res, paramv[0], 1+paramv[1], DB_MODE_CONST);

  return NULL_GUID;
}

static ocrGuid_t lj_red_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation* sim = (simulation*)depv[0].ptr;
  sim->e_potential = 0;
  for(u32 b = 1; b < depc; ++b) {
    sim->e_potential += *(real_t*)depv[b].ptr;
    ocrDbDestroy(depv[b].guid);
  }
  ocrEventSatisfy(paramv[0], NULL_GUID);

  return NULL_GUID;
}

static inline void spawn_pair(ocrGuid_t sim, ocrGuid_t tmp, ocrGuid_t red, ocrGuid_t* list, u32 b, u32 grid[3], u32 plane)
{
  u64 paramv[2];
  paramv[0] = red; paramv[1] = b;
  ocrGuid_t edt;
  ocrEdtCreate(&edt, tmp, 2, paramv, 28, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(sim, edt, 0, DB_MODE_RW);
  ocrAddDependence(list[b], edt, 1, DB_MODE_CONST);

  u32 dep = 1;
  u32 bg[3]; box2grid(b, bg, grid[0], plane);
  u32 bbg[3];
  // Previous plane, previous row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2]==0 ? grid[2]-1 : bg[2]-1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Previous plane, same row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Previous plane, next row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Same plane, previous row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2];
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Same plane, same row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
  ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
  // Same plane, next row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Next plane, previous row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2]==grid[2]-1 ? 0 : bg[2]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Next plane, same row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Next plane, next row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
}

void fork_lj_force(ocrGuid_t sim, simulation* sim_ptr, ocrGuid_t cont, ocrGuid_t* list, u32 grid[3])
{
  ocrGuid_t f,fdb,*fdb_ptr;
  ocrEventCreate(&f, OCR_EVENT_ONCE_T, false);
  ocrAddDependence(f, cont, CONT_DEPC, DB_MODE_CONST);

  u32 pairs = sim_ptr->bxs.boxes_num;
  ocrGuid_t tmp,red;
  ocrEdtTemplateCreate(&tmp, lj_red_edt, 1, pairs+1);
  ocrEdtCreate(&red, tmp, 1, &f, pairs+1, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(sim, red, 0, DB_MODE_RW);
  ocrEdtTemplateDestroy(tmp);

  u32 plane = grid[0]*grid[1];
  ocrEdtTemplateCreate(&tmp, lj_edt, 2, 28);
  for(u32 b = 0; b < sim_ptr->bxs.boxes_num; ++b)
    spawn_pair(sim, tmp, red, list, b, grid, plane);
  ocrEdtTemplateDestroy(tmp);
}
