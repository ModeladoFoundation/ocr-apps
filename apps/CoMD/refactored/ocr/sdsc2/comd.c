#ifndef TG_ARCH
#include <time.h>
#endif
#include <string.h>
#include <ocr.h>

#include "comd.h"
#include "constants.h"
#include "timers.h"
#include "command.h"
#include "simulation.h"
#include "potentials.h"

static inline void timestamp(const char* msg)
{
#ifndef TG_ARCH
  time_t t = time(NULL);
  char* time_string = ctime(&t);
  time_string[24] = '\0';
  PRINTF("%s: ", time_string);
#endif
  PRINTF("%s\n", msg);
}

ocrGuid_t period_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
ocrGuid_t end_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  u64 argc = getArgc(depv[0].ptr);
  ocrGuid_t argv_g;
  char** argv;
  ocrDbCreate(&argv_g, (void**)&argv, sizeof(char*)*argc, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
  for(u32 a = 0; a < argc; ++a)
    argv[a] = getArgv(depv[0].ptr,a);

  ocrGuid_t timer_g;
  mdtimer_t* timers_p;
  ocrDbCreate(&timer_g, (void**)&timers_p, sizeof(mdtimer_t)*number_of_timers, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
#ifndef TG_ARCH
  memset(timers_p, 0, sizeof(mdtimer_t)*number_of_timers);
#else
  for(u32 a = 0; a < sizeof(mdtimer_t)*number_of_timers; ++a) ((char*)timers_p)[a]='\0';
#endif
  timestamp("Starting Initialization\n");
  profile_start(total_timer, timers_p);

  ocrGuid_t cmd_g;
  command_t* cmd_p;
  ocrDbCreate(&cmd_g, (void**)&cmd_p, sizeof(command_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
  parse_command(argc, argv, cmd_p);
  ocrDbDestroy(argv_g);

  ocrGuid_t sim_g;
  simulation_t* sim_p;
  ocrDbCreate(&sim_g, (void**)&sim_p, sizeof(simulation_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
  u8 insane = init_simulation(cmd_p, sim_p, timers_p);
  ocrDbDestroy(depv[0].guid);

  ocrGuid_t tmp_g,edt_g;
  if(insane) {
    ocrDbDestroy(timer_g);
    ocrDbDestroy(sim_g);
    timestamp("CoMD ending with initialization error\n");
    ocrShutdown();
  }
  else {
    ocrEdtTemplateCreate(&tmp_g, end_edt, 0, 6);
    ocrEdtCreate(&edt_g, tmp_g, 0, NULL, 6, NULL, EDT_PROP_NONE, NULL_GUID, NULL);
    ocrEdtTemplateDestroy(tmp_g);
    ocrAddDependence(timer_g,edt_g,0,DB_MODE_RW);
    ocrAddDependence(sim_g,edt_g,1,DB_MODE_RO);
    ocrAddDependence(sim_p->boxes.box,edt_g,2,DB_MODE_RO);
    ocrAddDependence(sim_p->boxes.rpf,edt_g,3,DB_MODE_RO);
    ocrAddDependence(sim_p->pot.mass,edt_g,4,DB_MODE_RO);

    ocrGuid_t fin, fin_g;
    if(sim_p->steps) {
      fin_g = edt_g;
      ocrEdtTemplateCreate(&tmp_g, period_edt, 1, 6);
      ocrEdtCreate(&edt_g, tmp_g, 1, (u64*)&fin_g, 6, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
      ocrEdtTemplateDestroy(tmp_g);
      ocrAddDependence(timer_g,edt_g,0,DB_MODE_RW);
      ocrAddDependence(sim_g,edt_g,1,DB_MODE_RW);
      ocrAddDependence(sim_p->pot.mass,edt_g,2,DB_MODE_RO);
      ocrAddDependence(sim_p->boxes.box,edt_g,3,DB_MODE_RO);
      ocrAddDependence(sim_p->boxes.rpf,edt_g,4,DB_MODE_RO);
    }

    ocrEdtTemplateCreate(&tmp_g, fork_init_edt, 5, 3);
    u64 pv[5] = {0,0,0,0,0};
    ((u32*)pv)[0] = sim_p->boxes.grid[0];
    ((u32*)pv)[1] = sim_p->boxes.grid[1];
    ((u32*)pv)[2] = sim_p->boxes.grid[2];
    ((u32*)pv)[3] = cmd_p->nx;
    ((u32*)pv)[4] = cmd_p->ny;
    ((u32*)pv)[5] = cmd_p->nz;
    *((real_t*)pv+3) = cmd_p->delta;
    *((real_t*)pv+4) = cmd_p->temperature;
    ocrEdtCreate(&fin_g, tmp_g, 5, pv, 3, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
    ocrEdtTemplateDestroy(tmp_g);
    ocrAddDependence(fin,edt_g,5,DB_MODE_NULL);
    ocrAddDependence(sim_g,fin_g,0,DB_MODE_RO);
    ocrAddDependence(sim_p->boxes.box,fin_g,1,DB_MODE_RO);
    ocrAddDependence(sim_p->boxes.rpf,fin_g,2,DB_MODE_RO);
  }
  ocrDbDestroy(cmd_g);

  return NULL_GUID;
}

void print_status(simulation_t* sim_p, mass_t* mass, double time)
{
  real_t energy = (sim_p->e_potential + sim_p->e_kinetic) / sim_p->atoms;
  if(sim_p->step==0) {
    sim_p->energy0 = energy;
    PRINTF("\nInitial energy : %f, atom count : %u\n\n", energy, sim_p->atoms0);
    timestamp("Starting simulation\n");
    PRINTF("#                                                                                         Performance\n");
    PRINTF("#  Loop   Time(fs)       Total Energy   Potential Energy     Kinetic Energy  Temperature   (us/atom)     # Atoms\n");
  }

  real_t e_k = sim_p->e_kinetic / sim_p->atoms;
  real_t e_u = sim_p->e_potential / sim_p->atoms;
  real_t temp = e_k / kB_eV_1_5;

  double time_per_atom = 1.0e6*time/(double)((sim_p->period > sim_p->step ? sim_p->period : 1)*sim_p->atoms);
  PRINTF("%7d %10.2f %18.12f %18.12f %18.12f %12.4f %10.4f %12d\n",
         sim_p->step, sim_p->step*sim_p->dt, energy, e_u, e_k, temp, time_per_atom, sim_p->atoms);
}

//depv: timer, sim, box, rpf, mass
ocrGuid_t end_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation_t* sim_p = (simulation_t*)depv[1].ptr;
  mdtimer_t* timer_p = (mdtimer_t*)depv[0].ptr;

  profile_stop(total_timer, timer_p);
  print_status(sim_p, (mass_t*)depv[4].ptr, get_total_time(total_timer, timer_p));
  timestamp("Ending simulation\n");

  real_t energy = (sim_p->e_potential + sim_p->e_kinetic) / sim_p->atoms;
  PRINTF("\nSimulation Validation:\n");
  PRINTF("  Initial energy  : %14.12f\n", sim_p->energy0);
  PRINTF("  Final energy    : %14.12f\n", energy);
  PRINTF("  eFinal/eInitial : %f\n", energy/sim_p->energy0);

  int atoms_diff = sim_p->atoms - sim_p->atoms0;
  if(atoms_diff == 0)
    PRINTF("  Final atom count : %d, no atoms lost\n", sim_p->atoms);
  else {
    PRINTF("#############################\n");
    PRINTF("# WARNING: %6d atoms lost #\n", atoms_diff);
    PRINTF("#############################\n");
  }

  print_timers(timer_p, sim_p->atoms);

  ocrGuid_t* box = (ocrGuid_t*) depv[2].ptr;
  ocrGuid_t* rpf = (ocrGuid_t*) depv[3].ptr;
  u32 n = sim_p->boxes.boxes_num;
  while(n) {
    --n;
    ocrDbDestroy(box[n]);
    ocrDbDestroy(rpf[n]);
  }
  ocrDbDestroy(depv[2].guid);
  ocrDbDestroy(depv[3].guid);
  ocrDbDestroy(sim_p->reduction);
  ocrDbDestroy(sim_p->pot.mass);

  if(sim_p->pot.potential&EAM) {
    ocrDbDestroy(sim_p->pot.eam.phi.values);
    ocrDbDestroy(sim_p->pot.eam.rho.values);
    ocrDbDestroy(sim_p->pot.eam.f.values);
  }
  ocrDbDestroy(depv[0].guid);
  ocrDbDestroy(depv[1].guid);

  timestamp("CoMD Ending\n");
  ocrShutdown();

  return NULL_GUID;
}
