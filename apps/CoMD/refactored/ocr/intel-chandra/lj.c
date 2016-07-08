#include <string.h>
#include <ocr.h>

#include "extensions/ocr-affinity.h"

#include "comd.h"
#include "constants.h"
#include "potentials.h"

ocrGuid_t ljforcevel_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    return NULL_GUID;
}

//extern ocrGuid_t ljforce_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

#if 1
void ljforce(atomData_t* PTR_atomData, atomData_t** nPTR_atomData, real_t sigma, real_t epsilon, real_t cut, real_t* shift)
{
  real_t pot = 0;
  real_t cut2 = cut*cut;
  real_t s6 = sigma*sigma*sigma*sigma*sigma*sigma;
  real_t cut6 = s6 /(cut2*cut2*cut2);
  real_t pshift = POT_SHIFT*cut6*(cut6-1.0);

  u8 aa,bb;
  for(aa = 0; aa < PTR_atomData->nAtoms; ++aa)
    for(bb = aa+1; bb < PTR_atomData->nAtoms; ++bb) {
      real_t r2; real3_t dr;
      dr[0] = PTR_atomData->r[aa][0]-PTR_atomData->r[bb][0];
      dr[1] = PTR_atomData->r[aa][1]-PTR_atomData->r[bb][1];
      dr[2] = PTR_atomData->r[aa][2]-PTR_atomData->r[bb][2];
      r2=dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2];

      if(r2 > cut2) continue;
      r2 = 1.0/r2;

      real_t r6 = s6 * (r2*r2*r2);
      real_t energy = (r6*(r6-1.0)-pshift)*0.5;
      PTR_atomData->u[aa]+=energy;
      PTR_atomData->u[bb]+=energy;

      real_t fr = -epsilon*r6*r2*(12.0*r6-6.0);
      PTR_atomData->f[aa][0]-=dr[0]*fr;
      PTR_atomData->f[aa][1]-=dr[1]*fr;
      PTR_atomData->f[aa][2]-=dr[2]*fr;
      PTR_atomData->f[bb][0]+=dr[0]*fr;
      PTR_atomData->f[bb][1]+=dr[1]*fr;
      PTR_atomData->f[bb][2]+=dr[2]*fr;
    }

  u8 n;
  for(n = 0; n <= 26; ++n) {
  if(n==13) continue; //computed above
    atomData_t* bPTR_atomData = nPTR_atomData[n];
    for(aa = 0; aa < PTR_atomData->nAtoms; ++aa)
      for(bb = 0; bb < bPTR_atomData->nAtoms; ++bb) {
        real_t r2; real3_t dr;
        dr[0] = PTR_atomData->r[aa][0]-(bPTR_atomData->r[bb][0]+FSHIFT_X(PTR_atomData->nmask,n));
        dr[1] = PTR_atomData->r[aa][1]-(bPTR_atomData->r[bb][1]+FSHIFT_Y(PTR_atomData->nmask,n));
        dr[2] = PTR_atomData->r[aa][2]-(bPTR_atomData->r[bb][2]+FSHIFT_Z(PTR_atomData->nmask,n));
        r2=dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2];
        //PRINTF("gid %d-%d %18.12f %18.12f %18.12f %8.6f %8.6f %8.6f %8.6f %8.6f %8.6f s: %18.12f %18.12f %18.12f\n", PTR_atomData->gid[aa], bPTR_atomData->gid[bb], dr[0], dr[1], dr[2], PTR_atomData->r[aa][0], PTR_atomData->r[aa][1], PTR_atomData->r[aa][2],
        //bPTR_atomData->r[bb][0], bPTR_atomData->r[bb][1], bPTR_atomData->r[bb][2],
        //FSHIFT_X(PTR_atomData->nmask,n),
        //FSHIFT_Y(PTR_atomData->nmask,n),
        //FSHIFT_Z(PTR_atomData->nmask,n)
        //);

        if(r2 > cut2) continue;
        r2 = 1.0/r2;

        real_t r6 = s6 * (r2*r2*r2);
        real_t energy = (r6*(r6-1.0)-pshift)*0.5;
        PTR_atomData->u[aa]+=energy;

        real_t fr = -epsilon*r6*r2*(12.0*r6-6.0);
        PTR_atomData->f[aa][0]-=dr[0]*fr;
        PTR_atomData->f[aa][1]-=dr[1]*fr;
        PTR_atomData->f[aa][2]-=dr[2]*fr;
      }
  }

}

//params: sigma, epsilon, cutoff, shiftx, shifty, shiftz
//depv: DBK_atomDataH0, .., DBK_atomDataH26
ocrGuid_t ljforce_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  //PRINTF("ljforce_edt\n");
  atomData_t* PTR_atomData = (atomData_t*)depv[13].ptr;
#ifndef TG_ARCH
  memset(PTR_atomData->f,0,sizeof(real3_t)*PTR_atomData->nAtoms);
  memset(PTR_atomData->u,0,sizeof(real_t)*PTR_atomData->nAtoms);
#else
  u32 m;
  for(m=0; m<sizeof(real3_t)*PTR_atomData->nAtoms; ++m) ((char*)PTR_atomData->f)[m]='\0';
  for(m=0; m<sizeof(real_t)*PTR_atomData->nAtoms; ++m) ((char*)PTR_atomData->u)[m]='\0';
#endif

  computeForceEdtParamv_t* PTR_PRM_force_edt = (computeForceEdtParamv_t*) paramv;
  real_t sigma = PTR_PRM_force_edt->lj.sigma;
  real_t epsilon = PTR_PRM_force_edt->lj.epsilon;
  real_t cut = PTR_PRM_force_edt->cutoff;
  real_t* shift = PTR_PRM_force_edt->domain;

  atomData_t* nPTR_atomData[27];
  u8 n, k=0;
  for(n = 0; n <= 26; ++n)
  {
    //if(n==13) continue;
    nPTR_atomData[k++] = (atomData_t*)depv[n].ptr;
    //PRINTF("n %d %p\n", n, nPTR_atomData[k-1]);
  }

  ljforce(PTR_atomData, nPTR_atomData, sigma, epsilon, cut, shift);

  return NULL_GUID; //depv[0].guid;
}

#endif

#if 0

//params: count, leaf, sigma, epsilon, cutoff, shiftx, shifty, shiftz
//depv: DBK_atomDataH0, .., DBK_atomDataH26, sim, nextff, signals0, .., signals26
ocrGuid_t ljforcevel_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t PDaffinityGuid = NULL_GUID;
    ocrHint_t HNT_edt;
    ocrHintInit( &HNT_edt, OCR_HINT_EDT_T );
#ifdef ENABLE_EXTENSION_AFFINITY
    ocrAffinityGetCurrent(&PDaffinityGuid);
    ocrSetHintValue( &HNT_edt, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(PDaffinityGuid) );
#endif

  ocrGuid_t DBK_atomDataH = depv[0].guid;

  atomData_t* PTR_atomData = (atomData_t*)depv[0].ptr;
#ifndef TG_ARCH
  memset(PTR_atomData->f,0,sizeof(real3_t)*PTR_atomData->nAtoms);
  memset(PTR_atomData->u,0,sizeof(real_t)*PTR_atomData->nAtoms);
#else
  u32 m;
  for(m=0; m<sizeof(real3_t)*PTR_atomData->nAtoms; ++m) ((char*)PTR_atomData->f)[m]='\0';
  for(m=0; m<sizeof(real_t)*PTR_atomData->nAtoms; ++m) ((char*)PTR_atomData->u)[m]='\0';
#endif

  PRM_forcevel_edt_t* PTR_PRM_forcevel_edt = (PRM_forcevel_edt_t*) paramv;
  real_t ds = PTR_PRM_forcevel_edt->ds;
  real_t sigma = PTR_PRM_forcevel_edt->sigma;
  real_t epsilon = PTR_PRM_forcevel_edt->epsilon;
  real_t cut = PTR_PRM_forcevel_edt->cutoff;
  real_t* shift = PTR_PRM_forcevel_edt->domain;

  atomData_t* nPTR_atomData[26];
  u8 n;
  for(n = 0; n < 26; ++n)
    nPTR_atomData[n] = (atomData_t*)depv[n+1].ptr;

  ljforce(PTR_atomData, nPTR_atomData, sigma, epsilon, cut, shift);

  simulationH_t* sim = (simulationH_t*)depv[27].ptr;

  ocrGuid_t tmp, tmpg;
  if(--ds) {
    PTR_PRM_forcevel_edt->ds = ds;
    velocity(PTR_atomData, sim->dt);
    ocrEdtTemplateCreate(&tmp, ljforcevel_edt, sizeof(PRM_forcevel_edt_t)/sizeof(u64), 56);
    ocrEdtCreate(&tmpg, tmp, EDT_PARAM_DEF, (u64*)PTR_PRM_forcevel_edt, 56, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), (ocrGuid_t*)depv[28].ptr);
    ocrDbRelease(depv[28].guid);
    ocrEdtTemplateDestroy(tmp);
    ocrAddDependence(PTR_atomData->nextpf, tmpg, 29, DB_MODE_NULL);
    ocrDbRelease(DBK_atomDataH);
    ocrAddDependence(DBK_atomDataH, tmpg, 0, DB_MODE_RW);
    for(n=1; n < 28; ++n)
      ocrAddDependence(depv[n].guid, tmpg, n, DB_MODE_RO);
    ocrAddDependence(depv[n].guid, tmpg, n, DB_MODE_RW);
    for(n=30; n < 56; ++n)
      ocrAddDependence(nPTR_atomData[n-30]->nextpf, tmpg, n, DB_MODE_NULL);
    return depv[28].guid;
  }
  else {
    PRM_veluk_edt_t PRM_veluk_edt;
    PRM_veluk_edt.dt = 0.5*sim->dt;
    PRM_veluk_edt.leaves_g = PTR_PRM_forcevel_edt->leaves_g;
    ocrEdtTemplateCreate(&tmp, veluk_edt, sizeof(PRM_veluk_edt_t)/sizeof(u64), 2);
    ocrEdtCreate(&tmpg, tmp, EDT_PARAM_DEF, (u64*)&PRM_veluk_edt, 2, NULL, EDT_PROP_NONE, PICK_1_1(&HNT_edt,PDaffinityGuid), NULL);
    ocrEdtTemplateDestroy(tmp);
    ocrDbRelease(DBK_atomDataH);
    ocrAddDependence(DBK_atomDataH, tmpg, 0, DB_MODE_RW);
    ocrAddDependence(sim->pot.mass, tmpg, 1, DB_MODE_RO);
    ocrDbDestroy(depv[28].guid);
  }

  return NULL_GUID;
}

#endif

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

  ocrDbRelease(p->mass);

}
