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

  PRM_force_edt_t* PTR_PRM_force_edt = (PRM_force_edt_t*) paramv;
  real_t sigma = PTR_PRM_force_edt->lj.sigma;
  real_t epsilon = PTR_PRM_force_edt->lj.epsilon;
  real_t cut = PTR_PRM_force_edt->cutoff;
  real_t* shift = PTR_PRM_force_edt->domain;

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

  PRM_forcevel_edt_t* PTR_PRM_forcevel_edt = (PRM_forcevel_edt_t*) paramv;
  real_t ds = PTR_PRM_forcevel_edt->ds;
  real_t sigma = PTR_PRM_forcevel_edt->sigma;
  real_t epsilon = PTR_PRM_forcevel_edt->epsilon;
  real_t cut = PTR_PRM_forcevel_edt->cutoff;
  real_t* shift = PTR_PRM_forcevel_edt->domain;

  rpf_t* nrpf[26];
  u8 n;
  for(n = 0; n < 26; ++n)
    nrpf[n] = (rpf_t*)depv[n+1].ptr;

  ljforce(rpf, nrpf, sigma, epsilon, cut, shift);

  simulation_t* sim = (simulation_t*)depv[27].ptr;

  ocrGuid_t tmp, tmpg;
  if(--ds) {
    PTR_PRM_forcevel_edt->ds = ds;
    velocity(rpf, sim->dt);
    ocrEdtTemplateCreate(&tmp, ljforcevel_edt, sizeof(PRM_forcevel_edt_t)/sizeof(u64), 56);
    ocrEdtCreate(&tmpg, tmp, EDT_PARAM_DEF, (u64*)PTR_PRM_forcevel_edt, 56, NULL, EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), (ocrGuid_t*)depv[28].ptr);
    ocrEdtTemplateDestroy(tmp);
    ocrAddDependence(depv[0].guid, tmpg, 0, DB_MODE_RW);
    for(n=1; n < 28; ++n)
      ocrAddDependence(depv[n].guid, tmpg, n, DB_MODE_RO);
    ocrAddDependence(depv[n].guid, tmpg, n, DB_MODE_RW);
    ocrAddDependence(rpf->nextpf, tmpg, 29, DB_MODE_NULL);
    for(n=30; n < 56; ++n)
      ocrAddDependence(nrpf[n-30]->nextpf, tmpg, n, DB_MODE_NULL);
    return depv[28].guid;
  }
  else {
    PRM_veluk_edt_t PRM_veluk_edt;
    PRM_veluk_edt.dt = 0.5*sim->dt;
    PRM_veluk_edt.leaves_g = PTR_PRM_forcevel_edt->leaves_g;
    ocrEdtTemplateCreate(&tmp, veluk_edt, sizeof(PRM_veluk_edt_t)/sizeof(u64), 2);
    ocrEdtCreate(&tmpg, tmp, EDT_PARAM_DEF, (u64*)&PRM_veluk_edt, 2, NULL, EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL);
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
  PRINTF("  Mass             : %.3f amu\n", p->massFloat / amu2internal_mass);
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
  ocrDbCreate(&p->mass, (void**)&mass_p, sizeof(mass_t), DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC);
  mass_p->mass[0] = 63.55 * amu2internal_mass;
  mass_p->inv_mass[0] = 1.0/mass_p->mass[0];
  mass_p->inv_mass_2[0] = 0.5/mass_p->mass[0];
  mass_p->inv_mass_dt[0] = dt/mass_p->mass[0];

  p->massFloat = mass_p->mass[0];
}
