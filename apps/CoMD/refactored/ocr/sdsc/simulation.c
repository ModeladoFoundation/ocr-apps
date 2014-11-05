#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ocr.h>

#include "command.h"
#include "timers.h"
#include "cells.h"
#include "simulation.h"

static u8 sanity_checks(command* cmd, double cutoff, double lattice_const, char lattice_type[8]);

u8 init_simulation(command* cmd, simulation* sim, timer* t, ocrGuid_t** list)
{
  sim->step = 0;
  sim->steps = cmd->steps;
  sim->period = cmd->period;
  sim->dt = cmd->dt;
  sim->e_potential = 0.0;
  sim->e_kinetic = 0.0;

  u8 insane = 0;
  if(cmd->doeam)
    insane = init_eam(cmd->pot_dir, cmd->pot_name, cmd->pot_type, &sim->pot, sim->dt);
  else
    init_lj(&sim->pot, sim->dt);
  if(insane) return insane;

  real_t lattice_const = cmd->lat;
  if(cmd->lat < 0.0)
    lattice_const = sim->pot.lat;
  insane = sanity_checks(cmd, sim->pot.cutoff, lattice_const, sim->pot.lattice_type);
  if(insane) return insane;

  ocrGuid_t box_tmp;
  box** box_ptr = init_lattice(sim, cmd, lattice_const, list, &box_tmp);

  profile_start(redistribute_timer,t);
  redistribute_atoms(sim, box_ptr, sim->bxs.boxes_num);
  profile_stop(redistribute_timer,t);

  ocrDbDestroy(box_tmp);

  return 0;
}

void validate_result(simulation* sim)
{
  real_t energy = (sim->e_potential + sim->e_kinetic) / sim->bxs.atoms;

  PRINTF("\n\n");
  PRINTF("Simulation Validation:\n");
  PRINTF("  Initial energy  : %14.12f\n", sim->energy0);
  PRINTF("  Final energy    : %14.12f\n", energy);
  PRINTF("  eFinal/eInitial : %f\n", energy/sim->energy0);

  int atoms_diff = sim->bxs.atoms - sim->atoms0;
  if(atoms_diff == 0)
    PRINTF("  Final atom count : %d, no atoms lost\n", sim->bxs.atoms);
  else {
    PRINTF("#############################\n");
    PRINTF("# WARNING: %6d atoms lost #\n", atoms_diff);
    PRINTF("#############################\n");
  }
}

void print_status_header()
{
  PRINTF(
  "#                                                                                         Performance\n"
  "#  Loop   Time(fs)       Total Energy   Potential Energy     Kinetic Energy  Temperature   (us/atom)     # Atoms\n");
}

void print_status(simulation* sim, double time)
{
   u32 step = sim->step; u32 atoms = sim->bxs.atoms;
   real_t energy = (sim->e_potential+sim->e_kinetic) / atoms;
   real_t e_k = sim->e_kinetic / atoms;
   real_t e_u = sim->e_potential / atoms;
   real_t temp = e_k * kB_eV_1_5;

   double time_per_atom = 1.0e6*time/(double)((sim->period > step ? sim->period : 1)*atoms);
   PRINTF(" %6d %10.2f %18.12f %18.12f %18.12f %12.4f %10.4f %12d\n",
          step, step*sim->dt, energy, e_u, e_k, temp, time_per_atom, atoms);
}

void print_simulation(simulation* sim)
{
   PRINTF("Simulation data: \n");
   PRINTF("  Total atoms        : %d\n", sim->bxs.atoms);
   PRINTF("  Min bounds  : [ %14.10f, %14.10f, %14.10f ]\n",
          sim->bxs.local_min[0], sim->bxs.local_min[1], sim->bxs.local_min[2]);
   PRINTF("  Max bounds  : [ %14.10f, %14.10f, %14.10f ]\n\n",
          sim->bxs.local_max[0], sim->bxs.local_max[1], sim->bxs.local_max[2]);
   PRINTF("  Boxes        : %6d,%6d,%6d = %8d\n",
          sim->bxs.grid[0], sim->bxs.grid[1], sim->bxs.grid[2], sim->bxs.boxes_num);
   PRINTF("  Box size           : [ %14.10f, %14.10f, %14.10f ]\n",
          sim->bxs.box_size[0], sim->bxs.box_size[1], sim->bxs.box_size[2]);
   PRINTF("  Box factor         : [ %14.10f, %14.10f, %14.10f ] \n",
          sim->bxs.box_size[0]/sim->pot.cutoff,
          sim->bxs.box_size[1]/sim->pot.cutoff,
          sim->bxs.box_size[2]/sim->pot.cutoff);
   PRINTF("  Max Link Cell Occupancy: %d of %d\n\n", sim->bxs.max_occupancy, MAXATOMS);
   PRINTF("Potential data: \n");
   sim->pot.print(&sim->pot);
}

u8 sanity_checks(command* cmd, double cutoff, double lattice_const, char lattice_type[8])
{
  double minx = 2*cutoff;
  double miny = 2*cutoff;
  double minz = 2*cutoff;
  double sizex = cmd->nx*lattice_const;
  double sizey = cmd->ny*lattice_const;
  double sizez = cmd->nz*lattice_const;
  u8 failure = 0;
  if(sizex < minx || sizey < miny || sizez < minz) {
    failure |= 2;
    PRINTF("\nSimulation too small.\n"
           "  Increase the number of unit cells to make the simulation\n"
           "  at least (%3.2f, %3.2f. %3.2f) Ansgstroms in size\n", minx, miny, minz);
  }
  if (strcasecmp(lattice_type, "FCC") != 0) {
     failure |= 4;
     PRINTF("\nOnly FCC Lattice type supported, not %s. Fatal Error.\n", lattice_type);
  }
  return failure;
}

void destroy_simulation(void* s, void* l)
{
  simulation* sim = (simulation*)s;
  ocrGuid_t* list = (ocrGuid_t*)l;
  u32 b;
  for(b = 0; b < sim->bxs.boxes_num; ++b)
    ocrDbDestroy(list[b]);
  ocrDbDestroy(sim->bxs.list);
  if(sim->pot.eam.f.values != NULL_GUID) {
    ocrDbDestroy(sim->pot.eam.phi.values);
    ocrDbDestroy(sim->pot.eam.rho.values);
    ocrDbDestroy(sim->pot.eam.f.values);
  }
}
