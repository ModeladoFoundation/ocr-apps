#ifndef SIMULATION_H
#define SIMULATION_H

#include "comd.h"
#include "command.h"
#include "timers.h"

u8 init_simulation(command_t* cmd_p, simulationH_t* simH_p, mdtimer_t* timer_p);

ocrGuid_t EDT_init_fork(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

void destroy_simulation(simulationH_t* simH_p);

void print_status(simulationH_t* simH_p, mass_t* mass, double time);

ocrGuid_t timestep_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

void velocity(atomData_t* PTR_atomData, real_t dt);

ocrGuid_t veluk_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

#endif
