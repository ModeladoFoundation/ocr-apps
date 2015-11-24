#ifndef SIMULATION_H
#define SIMULATION_H

#include "comd.h"
#include "command.h"
#include "timers.h"

u8 init_simulation(command_t* cmd_p, simulation_t* sim_p, mdtimer_t* timer_p);

ocrGuid_t fork_init_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

void destroy_simulation(simulation_t* sim_p);

void print_status(simulation_t* sim_p, mass_t* mass, double time);

ocrGuid_t timestep_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

void velocity(rpf_t* rpf, real_t dt);

ocrGuid_t veluk_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

#endif
