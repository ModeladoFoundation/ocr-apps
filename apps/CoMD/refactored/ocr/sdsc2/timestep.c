#include <math.h>
#include <ocr.h>

#include "extensions/ocr-affinity.h"

#include "comd.h"
#include "command.h"
#include "timers.h"
#include "potentials.h"
#include "simulation.h"
#include "reductions.h"

static ocrGuid_t position_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
static ocrGuid_t exchange_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

//parms: endedt
//depv: timer, sim, mass, DBK_linkCellGuidsH, DBK_atomDataGuidsH
ocrGuid_t period_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  //ocrPrintf("%s\n",__func__);
  PRM_period_edt_t* PTR_PRM_period_edt = (PRM_period_edt_t*) paramv;

  ocrGuid_t timer_g = depv[0].guid;
  ocrGuid_t simH_g = depv[1].guid;
  ocrGuid_t mass_g = depv[2].guid;
  ocrGuid_t DBK_linkCellGuidsH = depv[3].guid;
  ocrGuid_t DBK_atomDataGuidsH = depv[4].guid;

  mdtimer_t* timer_p = (mdtimer_t*)depv[0].ptr;
  simulationH_t* simH_p = (simulationH_t*)depv[1].ptr;
  mass_t* mass_p = depv[2].ptr;
  ocrGuid_t* PTR_linkCellGuids = (ocrGuid_t*)depv[3].ptr;
  ocrGuid_t* PTR_atomDataGuids = (ocrGuid_t*)depv[4].ptr;

  u32* grid = simH_p->boxDataStH.grid;

  profile_stop(total_timer, timer_p);
  profile_start(total_timer, timer_p);
  print_status(simH_p, mass_p, get_elapsed_time(total_timer, timer_p));

  u64 ds = simH_p->step+simH_p->period<=simH_p->steps?simH_p->period:simH_p->steps-simH_p->step;
  simH_p->step+=ds;

  ocrGuid_t EDT_finalize = PTR_PRM_period_edt->EDT_finalize;

  ocrGuid_t* leaves_p; ocrGuid_t leaves_g;
  ocrDbCreate(&leaves_g, (void**)&leaves_p, sizeof(ocrGuid_t)*5*simH_p->boxDataStH.b_num, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC);

  PRM_red_t PRM_red;
  PRM_red.n = 0; //Update later
  PRM_red.epsilon = simH_p->pot.potential&LJ ? simH_p->pot.lj.epsilon : 1;
  PRM_red.guid = NULL_GUID;

  ocrGuid_t OEVT_reduction = build_reduction(simH_g, simH_p->reductionH_g, simH_p->boxDataStH.b_num, leaves_p,
                                             sizeof(PRM_ured_edt_t)/sizeof(u64), &PRM_red, ured_edt, grid, 0 );

  if(simH_p->step!=simH_p->steps) {
    ocrGuid_t tmp, next_period_edt;
    ocrEdtTemplateCreate(&tmp, period_edt, sizeof(PRM_period_edt_t)/sizeof(u64), 6);
    ocrEdtCreate(&next_period_edt, tmp, EDT_PARAM_DEF, (u64*)PTR_PRM_period_edt, 6, NULL, EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL);
    ocrEdtTemplateDestroy(tmp);

    ocrDbRelease(timer_g);
    ocrAddDependence(timer_g,next_period_edt,0,DB_MODE_RW);
    ocrAddDependence(simH_g,next_period_edt,1,DB_MODE_RW);
    ocrAddDependence(mass_g,next_period_edt,2,DB_MODE_RO);
    ocrAddDependence(DBK_linkCellGuidsH,next_period_edt,3,DB_MODE_RO);
    ocrAddDependence(DBK_atomDataGuidsH,next_period_edt,4,DB_MODE_RO);
    ocrAddDependence(OEVT_reduction,next_period_edt,5,DB_MODE_RO);
  }
  else
    ocrAddDependence(OEVT_reduction,EDT_finalize,5,DB_MODE_RO);

  PRM_position_edt_t PRM_position_edt;
  PRM_position_edt.ds = ds;
  PRM_position_edt.dt = simH_p->dt;
  PRM_position_edt.EDT_exchange = NULL_GUID;
  PRM_position_edt.DBK_linkCellH = NULL_GUID;

  PRM_forcevel_edt_t PRM_forcevel_edt;
  PRM_forcevel_edt.ds = ds;
  PRM_forcevel_edt.leaves_g = NULL_GUID;
  PRM_forcevel_edt.sigma = simH_p->pot.potential&LJ ? simH_p->pot.lj.sigma:1;
  PRM_forcevel_edt.epsilon = simH_p->pot.potential&LJ ? simH_p->pot.lj.epsilon:1;
  PRM_forcevel_edt.cutoff = simH_p->pot.cutoff;
  PRM_forcevel_edt.domain[0] = simH_p->boxDataStH.domain[0];
  PRM_forcevel_edt.domain[1] = simH_p->boxDataStH.domain[1];
  PRM_forcevel_edt.domain[2] = simH_p->boxDataStH.domain[2];

  ocrGuid_t tmpp,tmpf,tmpx;
  ocrEdtTemplateCreate(&tmpp, position_edt, sizeof(PRM_position_edt_t)/sizeof(u64), 29);
  ocrEdtTemplateCreate(&tmpf, simH_p->pot.forcevel_edt, sizeof(PRM_forcevel_edt_t)/sizeof(u64), 56);
  ocrEdtTemplateCreate(&tmpx, exchange_edt, 1, 2);

  ocrGuid_t* pe = leaves_p+simH_p->boxDataStH.b_num;
  ocrGuid_t* fe = pe+simH_p->boxDataStH.b_num;
  ocrGuid_t* pf = fe+simH_p->boxDataStH.b_num;
  ocrGuid_t* ff = pf+simH_p->boxDataStH.b_num;

  //TODO: affinity hints here
    ocrGuid_t PDaffinityGuid = NULL_GUID;

    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );
    ocrHint_t HNT_db;
    ocrHintInit( &HNT_db, OCR_HINT_DB_T );

  //TODO: Use affinity hints here to distribute the datablocks across policy domains
  u32 b;
  for( b=0; b<simH_p->boxDataStH.b_num; ++b )
  {
#ifdef ENABLE_EXTENSION_AFFINITY
        u64 count = 1;
        ocrAffinityQuery(PTR_linkCellGuids[b], &count, &PDaffinityGuid);
        ocrSetHintValue( &HNT_db, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
        ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif
    ocrGuid_t EDT_exchange;
    ocrEdtCreate(&EDT_exchange, tmpx, 1, (u64*) &ds, 2, NULL, EDT_PROP_FINISH, PICK_1_1(&HNT_edt,PDaffinityGuid), pf+b);
    PRM_position_edt.DBK_linkCellH = PTR_linkCellGuids[b];
    PRM_position_edt.EDT_exchange = EDT_exchange;

    ocrEdtCreate(pe+b, tmpp, EDT_PARAM_DEF, (u64*)&PRM_position_edt, 29, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), NULL);
    ocrAddDependence(PTR_atomDataGuids[b], pe[b], 0, DB_MODE_RW);
    ocrAddDependence(mass_g, pe[b], 1, DB_MODE_RO);

    ocrGuid_t* ffp;
    ocrDbCreate(ff+b, (void**)&ffp, sizeof(ocrGuid_t), DB_PROP_NONE, PICK_1_1(&HNT_db,PDaffinityGuid), NO_ALLOC);

    PRM_forcevel_edt.leaves_g = leaves_p[b];

    ocrEdtCreate(fe+b, tmpf, EDT_PARAM_DEF, (u64*)&PRM_forcevel_edt, 56, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), ffp);
    ocrDbRelease(ff[b]);

    ocrAddDependence(PTR_atomDataGuids[b], fe[b], 0, DB_MODE_RW);
    ocrAddDependence(simH_g, fe[b], 27, DB_MODE_RO);
    ocrAddDependence(ff[b], fe[b], 28, DB_MODE_RW);
    ocrAddDependence(pf[b], fe[b], 29, DB_MODE_NULL);
  }

  b = 0;
  u32 ijk[3] = {0,0,0};
  for(ijk[0]=0; ijk[0]<simH_p->boxDataStH.grid[0]; ++ijk[0])
  for(ijk[1]=0; ijk[1]<simH_p->boxDataStH.grid[1]; ++ijk[1])
  for(ijk[2]=0; ijk[2]<simH_p->boxDataStH.grid[2]; ++ijk[2]) {
    u8 n;
    for(n=0; n<26; ++n) {
      u32 i = neighbor_id(n,ijk,simH_p->boxDataStH.grid);
      ocrAddDependence(PTR_atomDataGuids[i], fe[b], 1+n, DB_MODE_RO);
      ocrAddDependence(pf[i], fe[b], 30+n, DB_MODE_NULL);
      ocrAddDependence(ff[i], pe[b], 3+n, DB_MODE_RO);
    }
    ++b;
  }

  for(b = 0; b < simH_p->boxDataStH.b_num; ++b)
    ocrAddDependence(ff[b], pe[b], 2, DB_MODE_RO);

  ocrDbDestroy(leaves_g);
  ocrEdtTemplateDestroy(tmpp);
  ocrEdtTemplateDestroy(tmpf);
  ocrEdtTemplateDestroy(tmpx);

  return NULL_GUID;
}

void velocity(atomData_t* PTR_atomData, real_t dt)
{
  u8 a;
  for(a=0; a<PTR_atomData->atoms; ++a) {
    PTR_atomData->p[a][0] += dt*PTR_atomData->f[a][0];
    PTR_atomData->p[a][1] += dt*PTR_atomData->f[a][1];
    PTR_atomData->p[a][2] += dt*PTR_atomData->f[a][2];
  }
}

//params: count, dt, exchange, DBK_linkCellH
//depv: DBK_atomDataH0, mass, ff0, .., ff26
ocrGuid_t position_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t PDaffinityGuid = NULL_GUID;
    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&PDaffinityGuid);
    ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif

  PRM_position_edt_t* PTR_PRM_position_edt = (PRM_position_edt_t*) paramv;
  real_t ds = PTR_PRM_position_edt->ds;
  real_t dt = PTR_PRM_position_edt->dt;
  ocrGuid_t EDT_exchange = PTR_PRM_position_edt->EDT_exchange;
  ocrGuid_t DBK_linkCellH = PTR_PRM_position_edt->DBK_linkCellH;

  ocrGuid_t DBK_atomDataH = depv[0].guid;
  ocrGuid_t mass_g = depv[1].guid;

  atomData_t* PTR_atomData = (atomData_t*)depv[0].ptr;
  real_t* invmass = ((mass_t*)depv[1].ptr)->inv_mass;
  u8 a;
  for(a=0; a<PTR_atomData->atoms; ++a) {
    real_t im = invmass[PTR_atomData->s[a]];
    PTR_atomData->r[a][0] += dt*PTR_atomData->p[a][0]*im;
    PTR_atomData->r[a][1] += dt*PTR_atomData->p[a][1]*im;
    PTR_atomData->r[a][2] += dt*PTR_atomData->p[a][2]*im;
  }

  ocrGuid_t tmp,tmpg,exc;
  exc = EDT_exchange;
  if(--ds) {
    PTR_PRM_position_edt->ds = ds;
    ocrEdtTemplateCreate(&tmp, exchange_edt, 1, 2);
    ocrEdtCreate(&PTR_PRM_position_edt->EDT_exchange, tmp, 1, (u64*)&ds, 2, NULL, EDT_PROP_FINISH, PICK_1_1(&HNT_edt,PDaffinityGuid), &PTR_atomData->nextpf);
    ocrEdtTemplateDestroy(tmp);
    ocrEdtTemplateCreate(&tmp, position_edt, sizeof(PRM_position_edt_t)/sizeof(u64), 29);
    ocrEdtCreate(&tmpg, tmp, EDT_PARAM_DEF, (u64*)PTR_PRM_position_edt, 29, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), NULL);
    ocrEdtTemplateDestroy(tmp);
    ocrAddDependence(DBK_atomDataH, tmpg, 0, DB_MODE_RW);
    ocrAddDependence(mass_g, tmpg, 1, DB_MODE_RO);
    u8 n;
    for(n=2; n < 29; ++n)
      ocrAddDependence(*(ocrGuid_t*)depv[n].ptr, tmpg, n, DB_MODE_RO);
  }

  ocrDbRelease(DBK_atomDataH);
  ocrAddDependence(DBK_atomDataH, exc, 0, DB_MODE_RW);
  ocrAddDependence(DBK_linkCellH, exc, 1, DB_MODE_RW);

  return NULL_GUID;
}

static inline void copy_atom(u8 s, u8 d, atomData_t* sPTR_atomData, atomData_t* dPTR_atomData, linkCellH_t* sPTR_linkCell, linkCellH_t* dPTR_linkCell)
{
  memcpy(dPTR_atomData->r[d],sPTR_atomData->r[s],sizeof(real3_t));
  memcpy(dPTR_atomData->p[d],sPTR_atomData->p[s],sizeof(real3_t));
  memcpy(dPTR_atomData->f[d],sPTR_atomData->f[s],sizeof(real3_t));
  dPTR_linkCell->gid[d]=sPTR_linkCell->gid[s]; dPTR_atomData->s[d]=sPTR_atomData->s[s];
}

static inline void move_atom(u8 s, atomData_t* sPTR_atomData, atomData_t* dPTR_atomData, linkCellH_t* sPTR_linkCell, linkCellH_t* dPTR_linkCell)
{
  copy_atom(s,dPTR_atomData->atoms,sPTR_atomData,dPTR_atomData,sPTR_linkCell,dPTR_linkCell);
  if(--sPTR_atomData->atoms && s!=sPTR_atomData->atoms)
    copy_atom(sPTR_atomData->atoms,s,sPTR_atomData,sPTR_atomData,sPTR_linkCell,sPTR_linkCell);
  if(++dPTR_atomData->atoms>dPTR_linkCell->max_occupancy)
    dPTR_linkCell->max_occupancy = dPTR_atomData->atoms;
  ocrAssert(dPTR_atomData->atoms<=MAXATOMS);
}

//depv: DBK_atomDataH0, DBK_linkCellH0, DBK_atomDataH1, DBK_linkCellH1
ocrGuid_t move_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  atomData_t* PTR_atomData0 = (atomData_t*)depv[0].ptr;
  linkCellH_t* PTR_linkCell0 = (linkCellH_t*)depv[1].ptr;
  atomData_t* PTR_atomData1 = (atomData_t*)depv[2].ptr;
  linkCellH_t* PTR_linkCell1 = (linkCellH_t*)depv[3].ptr;
  u8 a;
  for(a=0; a<PTR_atomData0->atoms; ++a)
    if(PTR_atomData0->r[a][0] <  PTR_linkCell1->max[0] &&
       PTR_atomData0->r[a][0] >= PTR_linkCell1->min[0] &&
       PTR_atomData0->r[a][1] <  PTR_linkCell1->max[1] &&
       PTR_atomData0->r[a][1] >= PTR_linkCell1->min[1] &&
       PTR_atomData0->r[a][2] <  PTR_linkCell1->max[2] &&
       PTR_atomData0->r[a][2] >= PTR_linkCell1->min[2])
      move_atom(a, PTR_atomData0, PTR_atomData1, PTR_linkCell0, PTR_linkCell1);

  return NULL_GUID;
}

//depv: DBK_atomDataH0, DBK_linkCellH0
ocrGuid_t exchange_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t PDaffinityGuid = NULL_GUID;
    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&PDaffinityGuid);
    ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif

  ocrGuid_t DBK_atomDataH = depv[0].guid;
  ocrGuid_t DBK_linkCellH = depv[1].guid;

  atomData_t* PTR_atomData = (atomData_t*)depv[0].ptr;
  linkCellH_t* PTR_linkCell = (linkCellH_t*)depv[1].ptr;
  u32 x=0;
  u8 a,f;
  for(a=0; a<PTR_atomData->atoms; ++a) {
    f=0;
    if(PTR_atomData->r[a][2] >= PTR_linkCell->max[2])
      f += 18;
    else if(PTR_atomData->r[a][2] >= PTR_linkCell->min[2])
      f += 9;
    if(PTR_atomData->r[a][1] >= PTR_linkCell->max[1])
      f += 6;
    else if(PTR_atomData->r[a][1] >= PTR_linkCell->min[1])
      f += 3;
    if(PTR_atomData->r[a][0] >= PTR_linkCell->max[0])
      f += 2;
    else if(PTR_atomData->r[a][0] >= PTR_linkCell->min[0])
      f += 1;
    if(f!=13) {
      if(f>13) --f;
      x |= (1<<f);
    }
  }

  ocrGuid_t* nbr_atomDataGuids = PTR_linkCell->nbr_atomDataGuids;
  ocrGuid_t* nbr_linkCellGuids = PTR_linkCell->nbr_linkCellGuids;

  ocrGuid_t tmp, edtg;
  ocrEdtTemplateCreate(&tmp, move_edt, 0, 4);
  for(f=0; f<26; ++f)
    if((1<<f)&x) {
      ocrEdtCreate(&edtg, tmp, 0, NULL, 4, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), NULL);
      ocrAddDependence(DBK_atomDataH, edtg, 0, DB_MODE_EW);
      ocrAddDependence(DBK_linkCellH, edtg, 1, DB_MODE_EW);
      ocrAddDependence(nbr_atomDataGuids[f], edtg, 2, DB_MODE_EW);
      ocrAddDependence(nbr_linkCellGuids[f], edtg, 3, DB_MODE_EW);
    }
  ocrEdtTemplateDestroy(tmp);

  return NULL_GUID;
}
