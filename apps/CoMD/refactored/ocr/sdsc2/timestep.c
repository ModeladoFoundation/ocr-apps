#include <math.h>
#include <ocr.h>

#include "comd.h"
#include "command.h"
#include "timers.h"
#include "potentials.h"
#include "simulation.h"
#include "reductions.h"

static ocrGuid_t position_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
static ocrGuid_t exchange_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

//parms: endedt
//depv: timer, sim, mass, boxs, rpfs
ocrGuid_t period_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation_t* sim_p = (simulation_t*)depv[1].ptr;
  mdtimer_t* timer_p = (mdtimer_t*)depv[0].ptr;

  profile_stop(total_timer, timer_p);
  profile_start(total_timer, timer_p);
  print_status(sim_p, (mass_t*)depv[2].ptr, get_elapsed_time(total_timer, timer_p));
  u64 ds = sim_p->step+sim_p->period<=sim_p->steps?sim_p->period:sim_p->steps-sim_p->step;
  sim_p->step+=ds;

  ocrGuid_t tmp_g = (ocrGuid_t)paramv[0];
  if(sim_p->step!=sim_p->steps) {
    ocrGuid_t tmp;
    ocrEdtTemplateCreate(&tmp, period_edt, 1, 6);
    ocrEdtCreate(&tmp_g, tmp, 1, paramv, 6, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrEdtTemplateDestroy(tmp);
    ocrAddDependence(depv[0].guid,tmp_g,0,DB_MODE_RW);
    ocrAddDependence(depv[1].guid,tmp_g,1,DB_MODE_RO);
    ocrAddDependence(depv[2].guid,tmp_g,2,DB_MODE_RO);
    ocrAddDependence(depv[3].guid,tmp_g,3,DB_MODE_RO);
    ocrAddDependence(depv[4].guid,tmp_g,4,DB_MODE_RO);
  }

  u64 pv[12] = {0,sim_p->pot.potential&LJ ? *(u64*)&sim_p->pot.lj.epsilon:1,
                 sim_p->pot.potential&LJ ? *(u64*)&sim_p->pot.lj.sigma:1,
                 sim_p->pot.potential&LJ ? *(u64*)&sim_p->pot.lj.epsilon:1,
                 *(u64*)&sim_p->pot.cutoff,*(u64*)&sim_p->boxes.domain[0],
                 *(u64*)&sim_p->boxes.domain[1],*(u64*)&sim_p->boxes.domain[2],
                 ds, sim_p->dt, 0, 0};
  ocrGuid_t* leaves_p; ocrGuid_t leaves_g;
  ocrDbCreate(&leaves_g, (void**)&leaves_p, sizeof(ocrGuid_t)*5*sim_p->boxes.boxes_num, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
  ocrGuid_t reduction = build_reduction(depv[1].guid, sim_p->reduction, sim_p->boxes.boxes_num, leaves_p, 2, pv, ured_edt);
  ocrAddDependence(reduction,tmp_g,5,DB_MODE_RO);

  ocrGuid_t* rpfs = (ocrGuid_t*)depv[4].ptr;
  ocrGuid_t* boxes = (ocrGuid_t*)depv[3].ptr;

  pv[0] = ds;
  ocrGuid_t tmpp,tmpf,tmpx;
  ocrEdtTemplateCreate(&tmpp, position_edt, 4, 29);
  ocrEdtTemplateCreate(&tmpf, sim_p->pot.forcevel_edt, 8, 56);
  ocrEdtTemplateCreate(&tmpx, exchange_edt, 0, 2);

  u32 b;
  ocrGuid_t* pe = leaves_p+sim_p->boxes.boxes_num;
  ocrGuid_t* fe = pe+sim_p->boxes.boxes_num;
  ocrGuid_t* pf = fe+sim_p->boxes.boxes_num;
  ocrGuid_t* ff = pf+sim_p->boxes.boxes_num;
  for(b = 0; b < sim_p->boxes.boxes_num; ++b) {
    pv[1] = sim_p->dt; pv[11] = boxes[b];
    ocrEdtCreate((ocrGuid_t*)(pv+10), tmpx, 0, NULL, 2, NULL, EDT_PROP_FINISH, NULL_GUID, pf+b);
    ocrEdtCreate(pe+b, tmpp, 4, pv+8, 29, NULL, EDT_PROP_NONE, NULL_GUID, NULL);
    ocrAddDependence(rpfs[b], pe[b], 0, DB_MODE_RW);
    ocrAddDependence(depv[2].guid, pe[b], 1, DB_MODE_RO);
    pv[1] = leaves_p[b];
    ocrGuid_t* ffp;
    ocrDbCreate(ff+b, (void**)&ffp, sizeof(ocrGuid_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    ocrEdtCreate(fe+b, tmpf, 8, pv, 56, NULL, EDT_PROP_NONE, NULL_GUID, ffp);
    ocrAddDependence(rpfs[b], fe[b], 0, DB_MODE_RW);
    ocrAddDependence(depv[1].guid, fe[b], 27, DB_MODE_RO);
    ocrAddDependence(ff[b], fe[b], 28, DB_MODE_RW);
    ocrAddDependence(pf[b], fe[b], 29, DB_MODE_NULL);
  }

  b = 0;
  u32 ijk[3] = {0,0,0};
  for(ijk[0]=0; ijk[0]<sim_p->boxes.grid[0]; ++ijk[0])
  for(ijk[1]=0; ijk[1]<sim_p->boxes.grid[1]; ++ijk[1])
  for(ijk[2]=0; ijk[2]<sim_p->boxes.grid[2]; ++ijk[2]) {
    u8 n;
    for(n=0; n<26; ++n) {
      u32 i = neighbor_id(n,ijk,sim_p->boxes.grid);
      ocrAddDependence(rpfs[i], fe[b], 1+n, DB_MODE_RO);
      ocrAddDependence(pf[i], fe[b], 30+n, DB_MODE_NULL);
      ocrAddDependence(ff[i], pe[b], 3+n, DB_MODE_RO);
    }
    ++b;
  }
  for(b = 0; b < sim_p->boxes.boxes_num; ++b)
    ocrAddDependence(ff[b], pe[b], 2, DB_MODE_RO);

  ocrDbDestroy(leaves_g);
  ocrEdtTemplateDestroy(tmpp);
  ocrEdtTemplateDestroy(tmpf);
  ocrEdtTemplateDestroy(tmpx);

  return NULL_GUID;
}

void velocity(rpf_t* rpf, real_t dt)
{
  u8 a;
  for(a=0; a<rpf->atoms; ++a) {
    rpf->p[a][0] += dt*rpf->f[a][0];
    rpf->p[a][1] += dt*rpf->f[a][1];
    rpf->p[a][2] += dt*rpf->f[a][2];
  }
}

//params: count, dt, exchange, box0
//depv: rpf0, mass, ff0, .., ff26
ocrGuid_t position_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  rpf_t* rpf = (rpf_t*)depv[0].ptr;
  real_t* invmass = ((mass_t*)depv[1].ptr)->inv_mass;
  real_t dt = *(real_t*)paramv+1;
  u8 a;
  for(a=0; a<rpf->atoms; ++a) {
    real_t im = invmass[rpf->s[a]];
    rpf->r[a][0] += dt*rpf->p[a][0]*im;
    rpf->r[a][1] += dt*rpf->p[a][1]*im;
    rpf->r[a][2] += dt*rpf->p[a][2]*im;
  }

  ocrGuid_t tmp,tmpg,exc;
  exc = *(ocrGuid_t*)(paramv+2);
  if(--paramv[0]) {
    ocrEdtTemplateCreate(&tmp, exchange_edt, 0, 2);
    ocrEdtCreate((ocrGuid_t*)(paramv+2), tmp, 0, NULL, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &rpf->nextpf);
    ocrEdtTemplateDestroy(tmp);
    ocrEdtTemplateCreate(&tmp, position_edt, 4, 29);
    ocrEdtCreate(&tmpg, tmp, 4, paramv, 29, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrEdtTemplateDestroy(tmp);
    ocrAddDependence(depv[0].guid, tmpg, 0, DB_MODE_RW);
    ocrAddDependence(depv[1].guid, tmpg, 1, DB_MODE_RO);
    u8 n;
    for(n=2; n < 29; ++n)
      ocrAddDependence(*(ocrGuid_t*)depv[n].ptr, tmpg, n, DB_MODE_RO);
  }
  ocrAddDependence(depv[0].guid, exc, 0, DB_MODE_RO);
  ocrAddDependence(*(ocrGuid_t*)(paramv+3), exc, 1, DB_MODE_RO);

  return NULL_GUID;
}

static inline void copy_atom(u8 s, u8 d, rpf_t* srpf, rpf_t* drpf, box_t* sbox, box_t* dbox)
{
  memcpy(drpf->r[d],srpf->r[s],sizeof(real3_t));
  memcpy(drpf->p[d],srpf->p[s],sizeof(real3_t));
  memcpy(drpf->f[d],srpf->f[s],sizeof(real3_t));
  dbox->gid[d]=sbox->gid[s]; drpf->s[d]=srpf->s[s];
}

static inline void move_atom(u8 s, rpf_t* srpf, rpf_t* drpf, box_t* sbox, box_t* dbox)
{
  copy_atom(s,drpf->atoms,srpf,drpf,sbox,dbox);
  if(--srpf->atoms && s!=srpf->atoms)
    copy_atom(srpf->atoms,s,srpf,srpf,sbox,sbox);
  if(++drpf->atoms>dbox->max_occupancy)
    dbox->max_occupancy = drpf->atoms;
  ASSERT(drpf->atoms<=MAXATOMS);
}

//depv: rpf0, box0, rpf1, box1
ocrGuid_t move_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  rpf_t* rpf0 = (rpf_t*)depv[0].ptr;
  box_t* box0 = (box_t*)depv[1].ptr;
  rpf_t* rpf1 = (rpf_t*)depv[2].ptr;
  box_t* box1 = (box_t*)depv[3].ptr;
  u8 a;
  for(a=0; a<rpf0->atoms; ++a)
    if(rpf0->r[a][0] <  box1->max[0] &&
       rpf0->r[a][0] >= box1->min[0] &&
       rpf0->r[a][1] <  box1->max[1] &&
       rpf0->r[a][1] >= box1->min[1] &&
       rpf0->r[a][2] <  box1->max[2] &&
       rpf0->r[a][2] >= box1->min[2])
      move_atom(a, rpf0, rpf1, box0, box1);

  return NULL_GUID;
}

//depv: rpf0, box0
ocrGuid_t exchange_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  rpf_t* rpf = (rpf_t*)depv[0].ptr;
  box_t* box = (box_t*)depv[1].ptr;
  u32 x=0;
  u8 a,f;
  for(a=0; a<rpf->atoms; ++a) {
    f=0;
    if(rpf->r[a][2] >= box->max[2])
      f += 18;
    else if(rpf->r[a][2] >= box->min[2])
      f += 9;
    if(rpf->r[a][1] >= box->max[1])
      f += 6;
    else if(rpf->r[a][1] >= box->min[1])
      f += 3;
    if(rpf->r[a][0] >= box->max[0])
      f += 2;
    else if(rpf->r[a][0] >= box->min[0])
      f += 1;
    if(f!=13) {
      if(f>13) --f;
      x |= (1<<f);
    }
  }

  ocrGuid_t* rpfs = box->rneighbors;
  ocrGuid_t* boxes = box->bneighbors;

  ocrGuid_t tmp, edtg;
  ocrEdtTemplateCreate(&tmp, move_edt, 0, 4);
  for(f=0; f<26; ++f)
    if((1<<f)&x) {
      ocrEdtCreate(&edtg, tmp, 0, NULL, 4, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
      ocrAddDependence(depv[0].guid, edtg, 0, DB_MODE_EW);
      ocrAddDependence(depv[1].guid, edtg, 1, DB_MODE_EW);
      ocrAddDependence(rpfs[f], edtg, 2, DB_MODE_EW);
      ocrAddDependence(boxes[f], edtg, 3, DB_MODE_EW);
    }
  ocrEdtTemplateDestroy(tmp);

  return NULL_GUID;
}
