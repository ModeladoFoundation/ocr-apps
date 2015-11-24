#include <math.h>
#include <ocr.h>

#include "comd.h"
#include "reductions.h"

ocrGuid_t build_reduction(ocrGuid_t sim, ocrGuid_t reduction, u32 leaves, ocrGuid_t* leaves_p,
                          u32 paramc, u64* paramv, ocrGuid_t (*red_edt)(u32, u64*, u32, ocrEdtDep_t*))
{
  ocrGuid_t root,tmp[FANIN];
  u32 f;
  for(f=0;f<FANIN;++f)
    ocrEdtTemplateCreate(tmp+f,red_edt,1,f+1);

  u32 ff = f = leaves;
  while(f>FANIN) {
    ff = f;
    f=f/FANIN+(f&(FANIN-1)?1:0);
  }

  ocrGuid_t rtmp;
  ocrEdtTemplateCreate(&rtmp,red_edt,paramc,f+2);
  paramv[0]=f;
  ocrEdtCreate(leaves_p, rtmp, paramc, paramv, f+2, NULL, EDT_PROP_NONE, NULL_GUID, &root);
  ocrAddDependence(sim,*leaves_p,f,DB_MODE_RW);
  ocrAddDependence(reduction,*leaves_p,f+1,DB_MODE_RW);
  ocrEdtTemplateDestroy(rtmp);

  u32 l;
  u32 gap = FANIN;
  while(gap<leaves) gap *= FANIN;

  while(f<leaves) {
    u32 rem = f&(FANIN-1);
    l=f/FANIN+(rem?1:0);
    u32 newgap = gap/FANIN;

    u32 ll,fff = 0;
    for(ll=0; ll<l; ++ll) {
      ocrGuid_t edt = leaves_p[ll*gap];
      u32 dd = (ll==l-1 && rem)?rem:FANIN;
      u32 d;
      for(d=0; d<dd; ++d) {
        ocrGuid_t event;
        fff += FANIN;
        u32 ddd = fff>ff ? ff+FANIN-fff : FANIN;
        paramv[0] = ddd;
        ocrEdtCreate(leaves_p+ll*gap+d*newgap, tmp[ddd-1], 1, paramv, ddd, NULL, EDT_PROP_NONE, NULL_GUID, &event);
        ocrAddDependence(event,edt,d,d?DB_MODE_RO:DB_MODE_RW);
      }
    }
    gap = newgap;
    l = f; f = ff;
    ff = leaves;
    while(ff>f*FANIN)
      ff=ff/FANIN+(ff&(FANIN-1)?1:0);
  }
  for(l=0;l<leaves;l+=FANIN) {
    u32 ll = l+FANIN>leaves?leaves-l:FANIN;
    while(ll) {
      --ll;
      ocrGuid_t event;
      ocrEventCreate(&event,OCR_EVENT_ONCE_T,1);
      ocrAddDependence(event,leaves_p[l],ll,ll?DB_MODE_RO:DB_MODE_RW);
      leaves_p[l+ll] = event;
    }
  }

  for(f=0;f<FANIN;++f)
    ocrEdtTemplateDestroy(tmp[f]);
  return root;
}

//params: n, tred
//depv: vcm0..vcmn-1, [sim,reduction]
ocrGuid_t vred_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  real_t* vcm0 = (real_t*)depv[0].ptr;
  u32 d; u64 n = *(u64*)paramv;
  for(d=1; d<n; ++d) {
    real_t* vcm1 = (real_t*)depv[d].ptr;
    vcm0[0] += vcm1[0];
    vcm0[1] += vcm1[1];
    vcm0[2] += vcm1[2];
    vcm0[3] += vcm1[3];
    ((u64*)vcm0)[4] += ((u64*)vcm1)[4];
    ocrDbDestroy(depv[d].guid);
  }
  if(paramc>1) {
    simulation_t* sim = (simulation_t*)depv[n].ptr;
    sim->atoms0 = sim->atoms = ((u64*)vcm0)[4];
    reduction_t* reduction = (reduction_t*)depv[depc-1].ptr;
    reduction->value[0] = -vcm0[0]/vcm0[3];
    reduction->value[1] = -vcm0[1]/vcm0[3];
    reduction->value[2] = -vcm0[2]/vcm0[3];
    ocrDbDestroy(depv[0].guid);
    reduction->reduction = *(ocrGuid_t*)(paramv+1);
    return depv[depc-1].guid;
  }
  return depv[0].guid;
}

//params: n, temperature, ured
//depv: ek0..ekn-1, [sim,reduction]
ocrGuid_t tred_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  real_t* ek = (real_t*)depv[0].ptr;
  u32 d; u64 n = *(u64*)paramv;
  for(d=1; d<n; ++d) {
    *ek += *(real_t*)depv[d].ptr;
    ocrDbDestroy(depv[d].guid);
  }
  if(paramc>1) {
    simulation_t* sim = depv[n].ptr;
    *ek /= (sim->atoms*kB_eV_1_5);
    reduction_t* reduction = (reduction_t*)depv[depc-1].ptr;
    reduction->value[0] = sqrt((*(double*)(paramv+1))/ *ek);
    ocrDbDestroy(depv[0].guid);
    reduction->reduction = *(ocrGuid_t*)(paramv+2);
    return depv[depc-1].guid;
  }
  return depv[0].guid;
}

//params: n, epsilon
//depv: uk0..ukn-1, [sim, reduction]
ocrGuid_t ured_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  real_t* u = (real_t*)depv[0].ptr;
  u32 d; u64 n = *(u64*)paramv;
  for(d=1; d<n; ++d) {
    u[0] += ((real_t*)depv[d].ptr)[0];
    u[1] += ((real_t*)depv[d].ptr)[1];
    ocrDbDestroy(depv[d].guid);
  }
  if(paramc>1) {
    simulation_t* sim = depv[n].ptr;
    sim->e_potential = u[0]**(double*)(paramv+1);
    sim->e_kinetic = u[1];
    ocrDbDestroy(depv[0].guid);
    reduction_t* reduction = (reduction_t*)depv[depc-1].ptr;
    reduction->reduction = NULL_GUID;
    return depv[n].guid;
  }
  return depv[0].guid;
}
