#include <ocr.h>

#include "simulation.h"
#include "cells.h"

static ocrGuid_t timestep_position(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
static ocrGuid_t timestep_force(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
static ocrGuid_t timestep_velocity(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
static ocrGuid_t timestep_end(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
static ocrGuid_t timestep_redistribute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

void timestep(ocrGuid_t timer, void* timer_ptr, ocrGuid_t sim, ocrGuid_t cont, ocrGuid_t list, ocrGuid_t* list_ptr, u32 boxes_num)
{
  profile_start(velocity_timer, timer_ptr);

  ocrGuid_t tmp,edt;
  ocrEdtTemplateCreate(&tmp, timestep_position, 1, 4);
  ocrEdtCreate(&edt, tmp, 1, &cont, 4, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(timer, edt, 0, DB_MODE_RW);
  ocrAddDependence(sim, edt, 1, DB_MODE_CONST);
  ocrAddDependence(list, edt, 2, DB_MODE_CONST);
  fork_advance_velocity(sim, edt, 3, list_ptr, boxes_num, 0.5);
  ocrEdtTemplateDestroy(tmp);
}


ocrGuid_t timestep_position(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  profile_stop(velocity_timer, depv[0].ptr);
  profile_start(position_timer, depv[0].ptr);
  ocrGuid_t tmp,edt;
  ocrEdtTemplateCreate(&tmp, timestep_redistribute, 1, 4);
  ocrEdtCreate(&edt, tmp, 1, paramv, 4, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(depv[0].guid, edt, 0, DB_MODE_RW);
  ocrAddDependence(depv[1].guid, edt, 1, DB_MODE_CONST);
  ocrAddDependence(depv[2].guid, edt, 2, DB_MODE_CONST);
  simulation* sim = (simulation*)depv[1].ptr;
  fork_advance_position(depv[1].guid, edt, 3, (ocrGuid_t*)depv[2].ptr, sim->bxs.boxes_num);
  ocrEdtTemplateDestroy(tmp);
  return NULL_GUID;
}

ocrGuid_t timestep_redistribute(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  profile_stop(position_timer, depv[0].ptr);
  profile_start(redistribute_timer, depv[0].ptr);
  ocrGuid_t tmp,edt;
  ocrEdtTemplateCreate(&tmp, timestep_force, 1, 4);
  ocrEdtCreate(&edt, tmp, 1, paramv, 4, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(depv[0].guid, edt, 0, DB_MODE_RW);
  ocrAddDependence(depv[1].guid, edt, 1, DB_MODE_CONST);
  ocrAddDependence(depv[2].guid, edt, 2, DB_MODE_CONST);
  simulation* sim = (simulation*)depv[1].ptr;
  fork_redistribute(depv[1].guid, edt, 3, (ocrGuid_t*)depv[2].ptr, sim->bxs.boxes_num);
  ocrEdtTemplateDestroy(tmp);
  return NULL_GUID;
}

ocrGuid_t timestep_force(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  profile_stop(redistribute_timer, depv[0].ptr);

  profile_start(compute_force_timer, depv[0].ptr);
  ocrGuid_t tmp,edt;
  ocrEdtTemplateCreate(&tmp, timestep_velocity, 1, 4);
  ocrEdtCreate(&edt, tmp, 1, paramv, 4, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(depv[0].guid, edt, 0, DB_MODE_RW);
  ocrAddDependence(depv[1].guid, edt, 1, DB_MODE_RW);
  ocrAddDependence(depv[2].guid, edt, 2, DB_MODE_CONST);
  simulation* sim = (simulation*)depv[1].ptr;
  sim->pot.fork_force_computation(depv[1].guid, sim, edt, (ocrGuid_t*)depv[2].ptr, sim->bxs.grid);
  ocrEdtTemplateDestroy(tmp);
  return NULL_GUID;
}

ocrGuid_t timestep_velocity(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  profile_stop(compute_force_timer, depv[0].ptr);
  profile_start(velocity_timer, depv[0].ptr);
  simulation* sim = (simulation*)depv[1].ptr;
  ocrGuid_t tmp,edt;
  real_t dt = 1;
  if((++sim->step)%sim->period)
    ocrEdtTemplateCreate(&tmp, timestep_position, 1, 4);
  else {
    ocrEdtTemplateCreate(&tmp, timestep_end, 1, 4);
    dt = 0.5;
  }
  ocrEdtCreate(&edt, tmp, 1, paramv, 4, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(depv[0].guid, edt, 0, DB_MODE_RW);
  ocrAddDependence(depv[1].guid, edt, 1, DB_MODE_CONST);
  ocrAddDependence(depv[2].guid, edt, 2, DB_MODE_CONST);
  fork_advance_velocity(depv[1].guid, edt, 3, (ocrGuid_t*)depv[2].ptr, sim->bxs.boxes_num, dt);
  ocrEdtTemplateDestroy(tmp);
  return NULL_GUID;
}

ocrGuid_t timestep_end(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation* sim = (simulation*)depv[1].ptr;
  fork_kinetic_energy(depv[1].guid, paramv[0], 2, (ocrGuid_t*)depv[2].ptr, sim->bxs.boxes_num);
  return NULL_GUID;
}

static inline real_t kinetic_energy_box(box* bxs_ptr, real_t inv_mass_2)
{
  real_t e_kinetic = 0;
  for(u32 a = 0; a < bxs_ptr->atoms; ++a) {
    e_kinetic += inv_mass_2*(
                 bxs_ptr->p[a][0]*bxs_ptr->p[a][0]+
                 bxs_ptr->p[a][1]*bxs_ptr->p[a][1]+
                 bxs_ptr->p[a][2]*bxs_ptr->p[a][2]);
  }
  return e_kinetic;
}

void kinetic_energy(simulation* sim, box** bxs_ptr, u32 boxes_num)
{
  sim->e_kinetic = 0;
  for(u32 b = 0; b<boxes_num; ++b)
    sim->e_kinetic += kinetic_energy_box(bxs_ptr[b],sim->pot.inv_mass_2);
}

static ocrGuid_t ke_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation* sim = (simulation*)depv[0].ptr;
  ocrGuid_t res; real_t* res_ptr;
  ocrDbCreate(&res, (void**)&res_ptr, sizeof(real_t), 0, NULL_GUID, NO_ALLOC);
  *res_ptr = kinetic_energy_box((box*)depv[1].ptr, sim->pot.inv_mass_2);
  ocrAddDependence(res, paramv[0], paramv[1], DB_MODE_CONST);

  return NULL_GUID;
}

static ocrGuid_t ke_red_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation* sim = (simulation*)depv[0].ptr;
  sim->e_kinetic = 0;
  for(u32 b = 1; b < depc; ++b) {
    sim->e_kinetic += *(real_t*)depv[b].ptr;
    ocrDbDestroy(depv[b].guid);
  }
  ocrEventSatisfy(paramv[0], NULL_GUID);

  return NULL_GUID;
}

void fork_kinetic_energy(ocrGuid_t sim, ocrGuid_t cont, u32 depc, ocrGuid_t* list, u32 boxes_num)
{
  ocrGuid_t f,fdb,*fdb_ptr;
  ocrEventCreate(&f, OCR_EVENT_ONCE_T, false);
  ocrAddDependence(f, cont, depc, DB_MODE_CONST);

  ocrGuid_t tmp,red;
  ocrEdtTemplateCreate(&tmp, ke_red_edt, 1, boxes_num+1);
  ocrEdtCreate(&red, tmp, 1, &f, boxes_num+1, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(sim, red, 0, DB_MODE_RW);
  ocrEdtTemplateDestroy(tmp);

  ocrEdtTemplateCreate(&tmp, ke_edt, 2, 2);
  for(u32 b = 0; b < boxes_num; ++b) {
    u64 paramv[2]; paramv[0] = red; paramv[1] = b+1;
    ocrGuid_t edt;
    ocrEdtCreate(&edt, tmp, 2, paramv, 2, NULL, 0, NULL_GUID, NULL);
    ocrAddDependence(sim, edt, 0, DB_MODE_RW);
    ocrAddDependence(list[b], edt, 1, DB_MODE_CONST);
  }
  ocrEdtTemplateDestroy(tmp);
}

static inline void advance_velocity_box(box* bxs_ptr, real_t dt)
{
  for(u32 a = 0; a < bxs_ptr->atoms; ++a) {
    bxs_ptr->p[a][0] += dt*bxs_ptr->f[a][0];
    bxs_ptr->p[a][1] += dt*bxs_ptr->f[a][1];
    bxs_ptr->p[a][2] += dt*bxs_ptr->f[a][2];
  }
}

void advance_velocity(box** bxs_ptr, u32 boxes_num, real_t dt)
{
  for(u32 b = 0; b<boxes_num; ++b)
    advance_velocity_box(bxs_ptr[b], dt);
}

static ocrGuid_t av_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation* sim = (simulation*)depv[0].ptr;
  advance_velocity_box((box*)depv[1].ptr, *(real_t*)depv[2].ptr);
  return NULL_GUID;
}

static ocrGuid_t av_red_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  ocrDbDestroy(depv[depc-1].guid);
  return NULL_GUID;
}

void fork_advance_velocity(ocrGuid_t sim, ocrGuid_t cont, u32 depc, ocrGuid_t* list, u32 boxes_num, real_t step)
{
  ocrGuid_t dt; real_t* dt_ptr;
  ocrDbCreate(&dt, (void**)&dt_ptr, sizeof(real_t), 0, NULL_GUID, NO_ALLOC);
  *dt_ptr = step;
  ocrGuid_t tmp,red,red_event;
  ocrEdtTemplateCreate(&tmp, av_red_edt, 0, boxes_num+1);
  ocrEdtCreate(&red, tmp, 0, NULL, boxes_num+1, NULL, 0, NULL_GUID, &red_event);
  ocrAddDependence(dt, red, boxes_num, DB_MODE_CONST);
  ocrAddDependence(red_event, cont, depc, DB_MODE_CONST);
  ocrEdtTemplateDestroy(tmp);

  ocrEdtTemplateCreate(&tmp, av_edt, 0, 3);
  for(u32 b = 0; b < boxes_num; ++b) {
    ocrGuid_t edt;
    ocrEdtCreate(&edt, tmp, 0, NULL, 3, NULL, 0, NULL_GUID, &red_event);
    ocrAddDependence(red_event, red, b, DB_MODE_CONST);
    ocrAddDependence(sim, edt, 0, DB_MODE_CONST);
    ocrAddDependence(list[b], edt, 1, DB_MODE_RW);
    ocrAddDependence(dt, edt, 2, DB_MODE_RW);
  }
  ocrEdtTemplateDestroy(tmp);
}

static inline void advance_position_box(box* bxs_ptr, real_t dt_mass)
{
  for(u32 a = 0; a < bxs_ptr->atoms; ++a) {
    bxs_ptr->r[a][0] += dt_mass*bxs_ptr->p[a][0];
    bxs_ptr->r[a][1] += dt_mass*bxs_ptr->p[a][1];
    bxs_ptr->r[a][2] += dt_mass*bxs_ptr->p[a][2];
    bxs_ptr->U[a] = bxs_ptr->a[a] = 0;
    bxs_ptr->f[a][0] = bxs_ptr->f[a][1] = bxs_ptr->f[a][2] = 0;
  }
}

void advance_position(box** bxs_ptr, u32 boxes_num, real_t dt_mass)
{
  for(u32 b = 0; b<boxes_num; ++b)
    advance_position_box(bxs_ptr[b], dt_mass);
}

ocrGuid_t ap_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation* sim = (simulation*)depv[0].ptr;
  advance_position_box((box*)depv[1].ptr, sim->pot.inv_mass_dt);
  return NULL_GUID;
}

ocrGuid_t ap_red_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  return NULL_GUID;
}

void fork_advance_position(ocrGuid_t sim, ocrGuid_t cont, u32 depc, ocrGuid_t* list, u32 boxes_num)
{
  ocrGuid_t tmp,red,red_event;
  ocrEdtTemplateCreate(&tmp, ap_red_edt, 0, boxes_num);
  ocrEdtCreate(&red, tmp, 0, NULL, boxes_num, NULL, 0, NULL_GUID, &red_event);
  ocrAddDependence(red_event, cont, depc, DB_MODE_CONST);
  ocrEdtTemplateDestroy(tmp);

  ocrEdtTemplateCreate(&tmp, ap_edt, 0, 2);
  for(u32 b = 0; b < boxes_num; ++b) {
    ocrGuid_t edt;
    ocrEdtCreate(&edt, tmp, 0, NULL, 2, NULL, 0, NULL_GUID, &red_event);
    ocrAddDependence(red_event, red, b, DB_MODE_CONST);
    ocrAddDependence(sim, edt, 0, DB_MODE_CONST);
    ocrAddDependence(list[b], edt, 1, DB_MODE_RW);
  }
  ocrEdtTemplateDestroy(tmp);
}
