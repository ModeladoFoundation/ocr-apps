#include <string.h>
#include <ocr.h>

#include "comd.h"
#include "constants.h"
#include "potentials.h"
#include "simulation.h"

void ljforce(rpf_t* rpf, rpf_t** nrpf, real_t sigma, real_t epsilon, real_t cut, real_t* shift)
{
  real_t pot = 0;
  real_t cut2 = cut*cut;
  real_t s6 = sigma*sigma*sigma*sigma*sigma*sigma;
  real_t cut6 = s6 /(cut2*cut2*cut2);
  real_t pshift = POT_SHIFT*cut6*(cut6-1.0);

  u8 aa,bb;
  for(aa = 0; aa < rpf->atoms; ++aa)
    for(bb = aa+1; bb < rpf->atoms; ++bb) {
      real_t r2; real3_t dr;
      dr[0] = rpf->r[aa][0]-rpf->r[bb][0];
      dr[1] = rpf->r[aa][1]-rpf->r[bb][1];
      dr[2] = rpf->r[aa][2]-rpf->r[bb][2];
      r2=dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2];

      if(r2 > cut2) continue;
      r2 = 1.0/r2;

      real_t r6 = s6 * (r2*r2*r2);
      real_t energy = (r6*(r6-1.0)-pshift)*0.5;
      rpf->u[aa]+=energy;
      rpf->u[bb]+=energy;

      real_t fr = -epsilon*r6*r2*(12.0*r6-6.0);
      rpf->f[aa][0]-=dr[0]*fr;
      rpf->f[aa][1]-=dr[1]*fr;
      rpf->f[aa][2]-=dr[2]*fr;
      rpf->f[bb][0]+=dr[0]*fr;
      rpf->f[bb][1]+=dr[1]*fr;
      rpf->f[bb][2]+=dr[2]*fr;
    }

  u8 n;
  for(n = 0; n < 26; ++n) {
    rpf_t* brpf = nrpf[n];
    for(aa = 0; aa < rpf->atoms; ++aa)
      for(bb = 0; bb < brpf->atoms; ++bb) {
        real_t r2; real3_t dr;
        dr[0] = rpf->r[aa][0]-(brpf->r[bb][0]+FSHIFT_X(rpf->nmask,n));
        dr[1] = rpf->r[aa][1]-(brpf->r[bb][1]+FSHIFT_Y(rpf->nmask,n));
        dr[2] = rpf->r[aa][2]-(brpf->r[bb][2]+FSHIFT_Z(rpf->nmask,n));
        r2=dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2];

        if(r2 > cut2) continue;
        r2 = 1.0/r2;

        real_t r6 = s6 * (r2*r2*r2);
        real_t energy = (r6*(r6-1.0)-pshift)*0.5;
        rpf->u[aa]+=energy;

        real_t fr = -epsilon*r6*r2*(12.0*r6-6.0);
        rpf->f[aa][0]-=dr[0]*fr;
        rpf->f[aa][1]-=dr[1]*fr;
        rpf->f[aa][2]-=dr[2]*fr;
      }
  }

}

//params: sigma, epsilon, cutoff, shiftx, shifty, shiftz
//depv: rpf0, .., rpf26
ocrGuid_t ljforce_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  rpf_t* rpf = (rpf_t*)depv[0].ptr;
#ifndef TG_ARCH
  memset(rpf->f,0,sizeof(real3_t)*rpf->atoms);
  memset(rpf->u,0,sizeof(real_t)*rpf->atoms);
#else
  u32 m;
  for(m=0; m<sizeof(real3_t)*rpf->atoms; ++m) ((char*)rpf->f)[m]='\0';
  for(m=0; m<sizeof(real_t)*rpf->atoms; ++m) ((char*)rpf->u)[m]='\0';
#endif

  real_t sigma = *(real_t*)paramv;
  real_t epsilon = *(real_t*)(paramv+1);
  real_t cut = *(real_t*)(paramv+2);
  real_t* shift = (real_t*)paramv+3;

  rpf_t* nrpf[26];
  u8 n;
  for(n = 0; n < 26; ++n)
    nrpf[n] = (rpf_t*)depv[n+1].ptr;

  ljforce(rpf, nrpf, sigma, epsilon, cut, shift);

  return depv[0].guid;
}

//params: count, leaf, sigma, epsilon, cutoff, shiftx, shifty, shiftz
//depv: rpf0, .., rpf26, sim, nextff, signals0, .., signals26
ocrGuid_t ljforcevel_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  rpf_t* rpf = (rpf_t*)depv[0].ptr;
#ifndef TG_ARCH
  memset(rpf->f,0,sizeof(real3_t)*rpf->atoms);
  memset(rpf->u,0,sizeof(real_t)*rpf->atoms);
#else
  u32 m;
  for(m=0; m<sizeof(real3_t)*rpf->atoms; ++m) ((char*)rpf->f)[m]='\0';
  for(m=0; m<sizeof(real_t)*rpf->atoms; ++m) ((char*)rpf->u)[m]='\0';
#endif

  real_t sigma = *(real_t*)(paramv+2);
  real_t epsilon = *(real_t*)(paramv+3);
  real_t cut = *(real_t*)(paramv+4);
  real_t* shift = (real_t*)paramv+5;

  rpf_t* nrpf[26];
  u8 n;
  for(n = 0; n < 26; ++n)
    nrpf[n] = (rpf_t*)depv[n+1].ptr;

  ljforce(rpf, nrpf, sigma, epsilon, cut, shift);

  simulation_t* sim = (simulation_t*)depv[27].ptr;

  ocrGuid_t tmp, tmpg;
  if(--paramv[0]) {
    velocity(rpf, sim->dt);
    ocrEdtTemplateCreate(&tmp, ljforcevel_edt, 8, 56);
    ocrEdtCreate(&tmpg, tmp, 8, paramv, 56, NULL, EDT_PROP_NONE, NULL_GUID, (ocrGuid_t*)depv[28].ptr);
    ocrEdtTemplateDestroy(tmp);
    ocrAddDependence(depv[0].guid, tmpg, 0, DB_MODE_RW);
    for(n=1; n < 29; ++n)
      ocrAddDependence(depv[n].guid, tmpg, n, DB_MODE_RO);
    ocrAddDependence(rpf->nextpf, tmpg, 29, DB_MODE_NULL);
    for(n=30; n < 56; ++n)
      ocrAddDependence(nrpf[n-30]->nextpf, tmpg, n, DB_MODE_NULL);
    return depv[28].guid;
  }
  else {
    u64 pv[2]; *((real_t*)pv) = 0.5*sim->dt; pv[1]=paramv[1];
    ocrEdtTemplateCreate(&tmp, veluk_edt, 2, 2);
    ocrEdtCreate(&tmpg, tmp, 2, pv, 2, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrEdtTemplateDestroy(tmp);
    ocrAddDependence(depv[0].guid, tmpg, 0, DB_MODE_RW);
    ocrAddDependence(sim->pot.mass, tmpg, 1, DB_MODE_RO);
    ocrDbDestroy(depv[28].guid);
  }

  return NULL_GUID;
}

void print_lj(void* po)
{
  potential_t* p = (potential_t*)po;
  PRINTF("  Potential type   : Lennard-Jones\n");
  PRINTF("  Species name     : Cu\n");
  PRINTF("  Atomic number    : %u\n", p->atomic_no);
  PRINTF("  Mass             : %.3f amu\n", p->mass / amu2internal_mass);
  PRINTF("  Lattice Type     : FCC\n");
  PRINTF("  Lattice spacing  : %.3f Angstroms\n", p->lat);
  PRINTF("  Cutoff           : %.3f Angstroms\n", p->cutoff);
  PRINTF("  Epsilon          : %.3f eV\n", p->lj.epsilon*.25);
  PRINTF("  Sigma            : %.3f Angstroms\n", p->lj.sigma);
}

void init_lj(potential_t* p, real_t dt)
{
  p->potential = LJ+Cu;
  p->cutoff = 2.5*2.315;
  p->lat = 3.615;
  p->lattice_type=FCC;
  p->atomic_no = 29;
  p->lj.epsilon = 0.167*4;
  p->lj.sigma = 2.315;
  p->force_edt = ljforce_edt;
  p->forcevel_edt = ljforcevel_edt;
  p->print = print_lj;

  mass_t* mass_p;
  ocrDbCreate(&p->mass, (void**)&mass_p, sizeof(mass_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
  mass_p->mass[0] = 63.55 * amu2internal_mass;
  mass_p->inv_mass[0] = 1.0/mass_p->mass[0];
  mass_p->inv_mass_2[0] = 0.5/mass_p->mass[0];
  mass_p->inv_mass_dt[0] = dt/mass_p->mass[0];
}
