#include <stdio.h>
#include <string.h>
#include <math.h>

#include "simulation.h"
#include "cells.h"

void fork_eam_force(ocrGuid_t sim, simulation* sim_ptr, ocrGuid_t cont, ocrGuid_t* list, u32 grid[3]);

void eam_print(potential* p)
{
  PRINTF("  Potential type   : EAM\n");
  PRINTF("  Species name     : %s\n", p->name);
  PRINTF("  Atomic number    : %d\n", p->atomic_no);
  PRINTF("  Mass             : %f amu\n", p->mass / amu2internal_mass); // print in amu
  PRINTF("  Lattice Type     : %s\n", p->lattice_type);
  PRINTF("  Lattice spacing  : %f Angstroms\n", p->lat);
  PRINTF("  Cutoff           : %f Angstroms\n", p->cutoff);
}

static void init_interpolator(interpolator* i, u32 n, real_t x0, real_t dx, ocrGuid_t buffer, real_t* buffer_ptr)
{
  i->n = n;
  i->x0 = x0;
  i->xn = dx;
  i->inv_dx = 1/dx;
  i->values = buffer;
  for(u32 i=n; i; --i)
    buffer_ptr[i] = buffer_ptr[i-1];
  buffer_ptr[n+2] = buffer_ptr[n+1] = buffer_ptr[n];
}

static u8 eam_init_setfl(potential* pot, const char* pot_dir, char* pot_name)
{
  char tmp[4096];
#ifdef TG_ARCH
  strcpy(tmp, pot_dir);
  tmp[strlen(pot_dir)]='/';
  strcpy(tmp+strlen(pot_dir)+1, pot_name);
#else
  sprintf(tmp, "%s/%s", pot_dir, pot_name);
#endif

  FILE* file = fopen(tmp, "r");
  if(file == NULL) {
    PRINTF("potential file %s not found\n", tmp);
    return 3;
  }

  fgets(tmp, sizeof(tmp), file);
#ifdef TG_ARCH
  {
    u32 i = 0;
    while(tmp[i]==' ') ++i;
    while(tmp[i]!=' ') {
      pot->name[i] = tmp[i];
      ++i;
    }
    pot_name[i] = '\0';
  }
#else
  sscanf(tmp, "%s", pot->name);
#endif
  fgets(tmp, sizeof(tmp), file);
  fgets(tmp, sizeof(tmp), file);
  fgets(tmp, sizeof(tmp), file);

  u32 elements;
#ifdef TG_ARCH
  {
    u32 i = 0;
    while(tmp[i]==' ') ++i;
    elements = atoi(tmp+i);
  }
#else
  sscanf(tmp, "%u", &elements);
#endif
  if(elements != 1) {
    PRINTF("found %u species, alloy currently not supported\n", elements);
    return 3;
  }

  u32 n_rho, nr;
  real_t d_rho, dr;
  fgets(tmp, sizeof(tmp), file);
#ifdef TG_ARCH
  {
    u32 i = 0;
    while(tmp[i]==' ') ++i;
    n_rho = atoi(tmp+i);
    while(tmp[i]!=' ') ++i;
    while(tmp[i]==' ') ++i;
    d_rho = atof(tmp+i);
    while(tmp[i]!=' ') ++i;
    while(tmp[i]==' ') ++i;
    nr = atoi(tmp+i);
    while(tmp[i]!=' ') ++i;
    while(tmp[i]==' ') ++i;
    dr = atof(tmp+i);
    while(tmp[i]!=' ') ++i;
    while(tmp[i]==' ') ++i;
    pot->cutoff = atof(tmp+i);
  }
#else
  sscanf(tmp, "%u "EMT1" %u "EMT1" "EMT1, &n_rho, &d_rho, &nr, &dr, &pot->cutoff);
#endif

  fgets(tmp, sizeof(tmp), file);
#ifdef TG_ARCH
  {
    u32 i = 0;
    while(tmp[i]==' ') ++i;
    pot->atomic_no = atoi(tmp+i);
    while(tmp[i]!=' ') ++i;
    while(tmp[i]==' ') ++i;
    pot->mass = atof(tmp+i);
    while(tmp[i]!=' ') ++i;
    while(tmp[i]==' ') ++i;
    pot->lat = atof(tmp+i);
    while(tmp[i]!=' ') ++i;
    strncpy(pot->lattice_type, tmp+i, 3);
    pot->lattice_type[4] = '\0';
  }
#else
  sscanf(tmp, "%u "EMT1" "EMT1" %s", &pot->atomic_no, &pot->mass, &pot->lat, pot->lattice_type);
#endif

  ocrGuid_t buffer;
  real_t* buffer_ptr;

  ocrDbCreate(&buffer, (void**)&buffer_ptr, sizeof(real_t)*(n_rho+3), 0, NULL_GUID, NO_ALLOC);
  for(u32 i=0; i<n_rho; ++i)
#ifdef TG_ARCH
  {
    fgets(tmp, sizeof(tmp), file);
    u32 j = 0;
    while(tmp[j]!=' ') ++j;
    buffer_ptr[i] = atof(tmp+j);
  }
#else
    fscanf(file, FMT1, buffer_ptr+i);
#endif
  init_interpolator(&pot->eam.f, n_rho, 0.0, d_rho, buffer, buffer_ptr);

  ocrDbCreate(&buffer, (void**)&buffer_ptr, sizeof(real_t)*(nr+3), 0, NULL_GUID, NO_ALLOC);
  for(u32 i=0; i<nr; ++i)
#ifdef TG_ARCH
  {
    fgets(tmp, sizeof(tmp), file);
    u32 j = 0;
    while(tmp[j]!=' ') ++j;
    buffer_ptr[i] = atof(tmp+j);
  }
#else
    fscanf(file, FMT1, buffer_ptr+i);
#endif
  init_interpolator(&pot->eam.rho, nr, 0.0, dr, buffer, buffer_ptr);

  ocrDbCreate(&buffer, (void**)&buffer_ptr, sizeof(real_t)*(nr+3), 0, NULL_GUID, NO_ALLOC);
  for(u32 i=0; i<nr; ++i)
#ifdef TG_ARCH
  {
    fgets(tmp, sizeof(tmp), file);
    u32 j = 0;
    while(tmp[j]!=' ') ++j;
    buffer_ptr[i] = atof(tmp+j);
  }
#else
    fscanf(file, FMT1, buffer_ptr+i);
#endif
  for(u32 i=1; i<nr; ++i)
    buffer_ptr[i] /= i*dr;
  buffer_ptr[0] = buffer_ptr[1] + (buffer_ptr[1] - buffer_ptr[2]);
  init_interpolator(&pot->eam.phi, nr, 0.0, dr, buffer, buffer_ptr);

  fclose(file);
  return 0;
}

static u8 eam_init_funcfl(potential* pot, const char* pot_dir, char* pot_name)
{
  char tmp[4096];
#ifdef TG_ARCH
  strcpy(tmp, pot_dir);
  tmp[strlen(pot_dir)]='/';
  strcpy(tmp+strlen(pot_dir)+1, pot_name);
#else
  sprintf(tmp, "%s/%s", pot_dir, pot_name);
#endif

  FILE* file = fopen(tmp, "r");
  if(file == NULL) {
    PRINTF("potential file %s not found\n", tmp);
    return 3;
  }

  fgets(tmp, sizeof(tmp), file);
#ifdef TG_ARCH
  {
    u32 i = 0;
    while(tmp[i]!=' ') {
      pot->name[i] = tmp[i];
      ++i;
    }
    pot_name[i] = '\0';
  }
#else
  sscanf(tmp, "%s", pot->name);
#endif

  fgets(tmp,sizeof(tmp),file);
#ifdef TG_ARCH
  {
    u32 i = 0;
    while(tmp[i]==' ') ++i;
    pot->atomic_no = atoi(tmp+i);
    while(tmp[i]!=' ') ++i;
    while(tmp[i]==' ') ++i;
    pot->mass = atof(tmp+i);
    while(tmp[i]!=' ') ++i;
    while(tmp[i]==' ') ++i;
    pot->lat = atof(tmp+i);
    while(tmp[i]!=' ') ++i;
    strncpy(pot->lattice_type, tmp+i, 3);
    pot->lattice_type[4] = '\0';
  }
#else
  sscanf(tmp, "%u "EMT1" "EMT1" %s", &pot->atomic_no, &pot->mass, &pot->lat, pot->lattice_type);
#endif

  u32 n_rho, nr;
  real_t d_rho, dr;
  fgets(tmp, sizeof(tmp), file);
#ifdef TG_ARCH
  {
    u32 i = 0;
    while(tmp[i]==' ') ++i;
    n_rho = atoi(tmp+i);
    while(tmp[i]!=' ') ++i;
    while(tmp[i]==' ') ++i;
    d_rho = atof(tmp+i);
    while(tmp[i]!=' ') ++i;
    while(tmp[i]==' ') ++i;
    nr = atoi(tmp+i);
    while(tmp[i]!=' ') ++i;
    while(tmp[i]==' ') ++i;
    dr = atof(tmp+i);
  }
#else
  sscanf(tmp, "%u "EMT1" %u "EMT1" "EMT1, &n_rho, &d_rho, &nr, &dr, &pot->cutoff);
#endif

  ocrGuid_t buffer;
  real_t* buffer_ptr;

  ocrDbCreate(&buffer, (void**)&buffer_ptr, sizeof(real_t)*(n_rho+3), 0, NULL_GUID, NO_ALLOC);
#ifdef TG_ARCH
  for(u32 i=0; i<n_rho; i+=5)
  {
    fgets(tmp, sizeof(tmp), file);
    u32 j = 0; u32 k;
    for(k=0; k<5; ++k) {
      while(tmp[j]==' ') ++j;
      buffer_ptr[i+k] = atof(tmp+j);
    }
  }
#else
  for(u32 i=0; i<n_rho; ++i)
    fscanf(file, FMT1, buffer_ptr+i);
#endif
  init_interpolator(&pot->eam.f, n_rho, 0.0, d_rho, buffer, buffer_ptr);

  ocrDbCreate(&buffer, (void**)&buffer_ptr, sizeof(real_t)*(nr+3), 0, NULL_GUID, NO_ALLOC);
#ifdef TG_ARCH
  for(u32 i=0; i<nr; i+=5)
  {
    fgets(tmp, sizeof(tmp), file);
    u32 j = 0; u32 k;
    for(k=0; k<5; ++k) {
      while(tmp[j]==' ') ++j;
      buffer_ptr[i+k] = atof(tmp+j);
    }
  }
#else
  for(u32 i=0; i<nr; ++i)
    fscanf(file, FMT1, buffer_ptr+i);
#endif
  for(u32 i=1; i<nr; ++i) {
    buffer_ptr[i] *= buffer_ptr[i]/(i*dr);
    buffer_ptr[i] *= hartree2Ev*bohr2Angs;
  }
  buffer_ptr[0] = buffer_ptr[1] + (buffer_ptr[1] - buffer_ptr[2]);
  init_interpolator(&pot->eam.phi, nr, 0.0, dr, buffer, buffer_ptr);

  ocrDbCreate(&buffer, (void**)&buffer_ptr, sizeof(real_t)*(nr+3), 0, NULL_GUID, NO_ALLOC);
#ifdef TG_ARCH
  for(u32 i=0; i<nr; i+=5)
  {
    fgets(tmp, sizeof(tmp), file);
    u32 j = 0; u32 k;
    for(k=0; k<5; ++k) {
      while(tmp[j]==' ') ++j;
      buffer_ptr[i+k] = atof(tmp+j);
    }
  }
#else
  for(u32 i=0; i<nr; ++i)
    fscanf(file, FMT1, buffer_ptr+i);
#endif
  init_interpolator(&pot->eam.rho, nr, 0.0, dr, buffer, buffer_ptr);

  fclose(file);
  return 0;
}

u8 init_eam(const char* pot_dir, char* pot_name, const char* pot_type, potential* p, real_t dt)
{
  u8 insane = 0;
  p->print = eam_print;
  p->fork_force_computation = fork_eam_force;
  if(strcmp(pot_type, "setfl" ) == 0)
     insane = eam_init_setfl(p, pot_dir, pot_name);
  else if(strcmp(pot_type,"funcfl") == 0)
     insane = eam_init_funcfl(p, pot_dir, pot_name);
  else {
    PRINTF("EAM Type not supported\n");
    return 1;
  }
  if(!insane) {
    p->mass *= amu2internal_mass;
    p->inv_mass_2 = 1/(2*p->mass);
    p->inv_mass_dt = dt/p->mass;
  }
  return insane;
}

static void interpolate(interpolator* in, real_t* values, real_t r, real_t* f, real_t* df)
{
  if(r<in->x0) r = in->x0;
  r = (r-in->x0)*in->inv_dx;

  u32 i = (u32)floor(r);
  if(i > in->n) {
     i = in->n;
     r = in->xn;
  }
  ++i;
  r = r - floor(r);
  real_t g1 = values[i+1] - values[i-1];
  real_t g2 = values[i+2] - values[i];

  *f = values[i] + 0.5*r*(g1 + r*(values[i+1] + values[i-1] - 2.0*values[i]) );
  *df = 0.5*(g1 + r*(g2-g1))*in->inv_dx;
}

static real_t eam_force_box(box* a, box* b, real3 fshift, real_t cut, interpolator* iphi, real_t* phi, interpolator* irho, real_t* rho)
{
  real_t e_pot = 0;
  real_t cut2 = cut*cut;

  for(u32 aa = 0; aa < a->atoms; ++aa)
    for(u32 bb = 0; bb < b->atoms; ++bb) {

      real_t r; real3 dr;
      dr[0] = a->r[aa][0]-(b->r[bb][0]+fshift[0]);
      dr[1] = a->r[aa][1]-(b->r[bb][1]+fshift[1]);
      dr[2] = a->r[aa][2]-(b->r[bb][2]+fshift[2]);
      r=dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2];

      if( r > cut2) continue;
      r=sqrt(r);
      real_t inv_r = 1/r;

      real_t d_phi, phi_tmp, d_rho, rho_tmp;
      interpolate(iphi, phi, r, &phi_tmp, &d_phi);
      interpolate(irho, rho, r, &rho_tmp, &d_rho);
      a->f[aa][0] -= dr[0]*d_phi*inv_r;
      a->f[aa][1] -= dr[1]*d_phi*inv_r;
      a->f[aa][2] -= dr[2]*d_phi*inv_r;
      a->U[aa] += 0.5*phi_tmp;
      e_pot += 0.5*phi_tmp;

      a->a[aa] += rho_tmp;
    }

  return e_pot;
}

static real_t eam_force_box_same(box* a, real_t cut, interpolator* iphi, real_t* phi, interpolator* irho, real_t* rho)
{
  real_t e_pot = 0;
  real_t cut2 = cut*cut;

  for(u32 aa = 0; aa < a->atoms; ++aa)
    for(u32 bb = aa+1; bb < a->atoms; ++bb) {

      real_t r; real3 dr;
      dr[0] = a->r[aa][0]-a->r[bb][0];
      dr[1] = a->r[aa][1]-a->r[bb][1];
      dr[2] = a->r[aa][2]-a->r[bb][2];
      r=dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2];

      if( r > cut2) continue;
      r=sqrt(r);
      real_t inv_r = 1/r;

      real_t d_phi, phi_tmp, d_rho, rho_tmp;
      interpolate(iphi, phi, r, &phi_tmp, &d_phi);
      interpolate(irho, rho, r, &rho_tmp, &d_rho);
      a->f[aa][0] -= dr[0]*d_phi*inv_r;
      a->f[aa][1] -= dr[1]*d_phi*inv_r;
      a->f[aa][2] -= dr[2]*d_phi*inv_r;
      a->f[bb][0] += dr[0]*d_phi*inv_r;
      a->f[bb][1] += dr[1]*d_phi*inv_r;
      a->f[bb][2] += dr[2]*d_phi*inv_r;
      a->U[aa] += 0.5*phi_tmp;
      a->U[bb] += 0.5*phi_tmp;
      e_pot += phi_tmp;

      a->a[aa] += rho_tmp;
      a->a[bb] += rho_tmp;
    }

  return e_pot;
}

// sim box box cont depc phi rho
ocrGuid_t eam_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation* sim = (simulation*)depv[0].ptr;
  ocrGuid_t res; real_t* res_ptr;
  ocrDbCreate(&res, (void**)&res_ptr, sizeof(real_t), 0, NULL_GUID, NO_ALLOC);
  *res_ptr = eam_force_box_same((box*)depv[1].ptr, sim->pot.cutoff, &sim->pot.eam.phi, (real_t*)depv[depc-2].ptr, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);

  u32 dep=1;
  u32 plane = sim->bxs.grid[0]*sim->bxs.grid[1];
  u32 bg[3]; box2grid(paramv[1], bg, sim->bxs.grid[0], plane);
  u32 bbg[3];

  // Previous plane, previous row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? sim->bxs.grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2]==0 ? sim->bxs.grid[2]-1 : bg[2]-1;
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += eam_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.phi, (real_t*)depv[depc-2].ptr, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Previous plane, same row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += eam_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.phi, (real_t*)depv[depc-2].ptr, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Previous plane, next row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==sim->bxs.grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += eam_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.phi, (real_t*)depv[depc-2].ptr, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Same plane, previous row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? sim->bxs.grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2];
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += eam_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.phi, (real_t*)depv[depc-2].ptr, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Same plane, same row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  *res_ptr += eam_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.phi, (real_t*)depv[depc-2].ptr, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
  ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  *res_ptr += eam_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.phi, (real_t*)depv[depc-2].ptr, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
  // Same plane, next row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==sim->bxs.grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += eam_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.phi, (real_t*)depv[depc-2].ptr, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Next plane, previous row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? sim->bxs.grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2]==sim->bxs.grid[2]-1 ? 0 : bg[2]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += eam_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.phi, (real_t*)depv[depc-2].ptr, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Next plane, same row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += eam_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.phi, (real_t*)depv[depc-2].ptr, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Next plane, next row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==sim->bxs.grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    *res_ptr += eam_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.phi, (real_t*)depv[depc-2].ptr, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }

  ocrAddDependence(res, paramv[0], 1+paramv[1], DB_MODE_CONST);

  return NULL_GUID;
}

// sim box cont depc f
ocrGuid_t rho_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation* sim = (simulation*)depv[0].ptr;
  box* a = (box*)depv[1].ptr;
  ocrGuid_t res; real_t* res_ptr;
  ocrDbCreate(&res, (void**)&res_ptr, sizeof(real_t), 0, NULL_GUID, NO_ALLOC);
  *res_ptr = 0;
  for(u32 aa = 0; aa < a->atoms; ++aa) {
    real_t fembed;
    interpolate(&sim->pot.eam.f, (real_t*)depv[depc-1].ptr, a->a[aa], &fembed, &a->a[aa]);
    a->U[aa] += fembed;
    *res_ptr += fembed;
  }
  ocrAddDependence(res, paramv[0], paramv[1], DB_MODE_CONST);

  return NULL_GUID;
}

static void rho_force_box(box* a, box* b, real3 fshift, real_t cut, interpolator* irho, real_t* rho)
{
  real_t cut2 = cut*cut;

  for(u32 aa = 0; aa < a->atoms; ++aa)
    for(u32 bb = 0; bb < b->atoms; ++bb) {

      real_t r; real3 dr;
      dr[0] = a->r[aa][0]-(b->r[bb][0]+fshift[0]);
      dr[1] = a->r[aa][1]-(b->r[bb][1]+fshift[1]);
      dr[2] = a->r[aa][2]-(b->r[bb][2]+fshift[2]);
      r=dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2];

      if( r > cut2) continue;
      r=sqrt(r);
      real_t inv_r = 1/r;

      real_t d_rho, rho_tmp;
      interpolate(irho, rho, r, &rho_tmp, &d_rho);
      a->f[aa][0] -= (a->a[aa]+b->a[bb])*d_rho*dr[0]*inv_r;
      a->f[aa][1] -= (a->a[aa]+b->a[bb])*d_rho*dr[1]*inv_r;
      a->f[aa][2] -= (a->a[aa]+b->a[bb])*d_rho*dr[2]*inv_r;
    }
}

static void rho_force_box_same(box* a, real_t cut, interpolator* irho, real_t* rho)
{
  real_t cut2 = cut*cut;

  for(u32 aa = 0; aa < a->atoms; ++aa)
    for(u32 bb = aa+1; bb < a->atoms; ++bb) {

      real_t r; real3 dr;
      dr[0] = a->r[aa][0]-a->r[bb][0];
      dr[1] = a->r[aa][1]-a->r[bb][1];
      dr[2] = a->r[aa][2]-a->r[bb][2];
      r=dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2];

      if( r > cut2) continue;
      r=sqrt(r);
      real_t inv_r = 1/r;

      real_t d_rho, rho_tmp;
      interpolate(irho, rho, r, &rho_tmp, &d_rho);
      a->f[aa][0] -= (a->a[aa]+a->a[bb])*d_rho*dr[0]*inv_r;
      a->f[aa][1] -= (a->a[aa]+a->a[bb])*d_rho*dr[1]*inv_r;
      a->f[aa][2] -= (a->a[aa]+a->a[bb])*d_rho*dr[2]*inv_r;
      a->f[bb][0] += (a->a[aa]+a->a[bb])*d_rho*dr[0]*inv_r;
      a->f[bb][1] += (a->a[aa]+a->a[bb])*d_rho*dr[1]*inv_r;
      a->f[bb][2] += (a->a[aa]+a->a[bb])*d_rho*dr[2]*inv_r;
    }
}

// sim box box face rho
ocrGuid_t rho_force_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation* sim = (simulation*)depv[0].ptr;
  rho_force_box_same((box*)depv[1].ptr, sim->pot.cutoff, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
  u32 dep=1;
  u32 plane = sim->bxs.grid[0]*sim->bxs.grid[1];
  u32 bg[3]; box2grid(paramv[0], bg, sim->bxs.grid[0], plane);
  u32 bbg[3];
  // Previous plane, previous row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? sim->bxs.grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2]==0 ? sim->bxs.grid[2]-1 : bg[2]-1;
  for(u32 bb = 0; bb < 3; ++bb) {
    rho_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Previous plane, same row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  for(u32 bb = 0; bb < 3; ++bb) {
    rho_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Previous plane, next row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==sim->bxs.grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    rho_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Same plane, previous row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? sim->bxs.grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2];
  for(u32 bb = 0; bb < 3; ++bb) {
    rho_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Same plane, same row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  rho_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
  ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  rho_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
  // Same plane, next row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==sim->bxs.grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    rho_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Next plane, previous row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? sim->bxs.grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2]==sim->bxs.grid[2]-1 ? 0 : bg[2]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    rho_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Next plane, same row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  for(u32 bb = 0; bb < 3; ++bb) {
    rho_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }
  // Next plane, next row
  bbg[0] = bg[0]==0 ? sim->bxs.grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==sim->bxs.grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    rho_force_box((box*)depv[1].ptr, (box*)depv[++dep].ptr, sim->bxs.shift[faces(bg,bbg,sim->bxs.grid)], sim->pot.cutoff, &sim->pot.eam.rho, (real_t*)depv[depc-1].ptr);
    ++bbg[0]; if(bbg[0] == sim->bxs.grid[0]) bbg[0] = 0;
  }

  return NULL_GUID;
}

// sim box box face rho
static inline void spawn_pair_rho(ocrGuid_t sim, ocrGuid_t values, ocrGuid_t tmp, ocrGuid_t* list, u32 b, u32 grid[3], u32 plane)
{
  u64 bb = b;
  ocrGuid_t edt;
  ocrEdtCreate(&edt, tmp, 1, &bb, 29, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(sim, edt, 0, DB_MODE_CONST);
  ocrAddDependence(list[b], edt, 1, DB_MODE_RW);

  u32 dep=1;
  u32 bg[3]; box2grid(b, bg, grid[0], plane);
  u32 bbg[3];

  // Previous plane, previous row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2]==0 ? grid[2]-1 : bg[2]-1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Previous plane, same row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Previous plane, next row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Same plane, previous row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2];
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Same plane, same row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
  ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
  // Same plane, next row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Next plane, previous row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2]==grid[2]-1 ? 0 : bg[2]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Next plane, same row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Next plane, next row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }

  ocrAddDependence(values, edt, ++dep, DB_MODE_CONST);
}

// sim pot0 .. potn list
static ocrGuid_t rho_red_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation* sim = (simulation*)depv[0].ptr;
  for(u32 b = 1; b < depc-1; ++b) {
    sim->e_potential += *(real_t*)depv[b].ptr;
    ocrDbDestroy(depv[b].guid);
  }

  ocrGuid_t* list = (ocrGuid_t*)depv[depc-1].ptr;
  ocrGuid_t tmp; u32* grid = sim->bxs.grid;
  u32 plane = grid[0]*grid[1];
  ocrEdtTemplateCreate(&tmp, rho_force_edt, 1, 29);
  for(u32 b = 0; b < sim->bxs.boxes_num; ++b)
    spawn_pair_rho(depv[0].guid, sim->pot.eam.rho.values, tmp, list, b, grid, plane);

  ocrEdtTemplateDestroy(tmp);
  return NULL_GUID;
}

// sim pot0 .. potn list
static ocrGuid_t eam_red_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  simulation* sim = (simulation*)depv[0].ptr;
  sim->e_potential = 0;
  for(u32 b = 1; b < depc-1; ++b) {
    sim->e_potential += *(real_t*)depv[b].ptr;
    ocrDbDestroy(depv[b].guid);
  }

  ocrGuid_t tmp, red;
  ocrEdtTemplateCreate(&tmp, rho_red_edt, 0, sim->bxs.boxes_num+2);
  ocrEdtCreate(&red, tmp, 0, NULL, sim->bxs.boxes_num+2, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(depv[0].guid, red, 0, DB_MODE_RW);
  ocrAddDependence(sim->bxs.list, red, sim->bxs.boxes_num+1, DB_MODE_RW);
  ocrEdtTemplateDestroy(tmp);

  ocrGuid_t* list = (ocrGuid_t*)depv[depc-1].ptr;
  ocrGuid_t edt;
  ocrEdtTemplateCreate(&tmp, rho_edt, 2, 3);
  for(u32 b = 0; b < sim->bxs.boxes_num; ++b) {
    u64 paramv[2]; paramv[0]=red; paramv[1]=b+1;
    ocrEdtCreate(&edt, tmp, 2, paramv, 3, NULL, 0, NULL_GUID, NULL);
    ocrAddDependence(depv[0].guid, edt, 0, DB_MODE_RW);
    ocrAddDependence(list[b], edt, 1, DB_MODE_RW);
    ocrAddDependence(sim->pot.eam.f.values, edt, 2, DB_MODE_RW);
  }
  ocrEdtTemplateDestroy(tmp);

  return NULL_GUID;
}

static inline void spawn_pair(ocrGuid_t sim, eam_potential* eam, ocrGuid_t tmp, ocrGuid_t red, ocrGuid_t* list, u32 b, u32 grid[3], u32 plane)
{
  u64 paramv[2]; paramv[0]=red; paramv[1]=b;
  ocrGuid_t edt;
  ocrEdtCreate(&edt, tmp, 2, paramv, 30, NULL, 0, NULL_GUID, NULL);
  ocrAddDependence(sim, edt, 0, DB_MODE_RW);
  ocrAddDependence(list[b], edt, 1, DB_MODE_RW);

  u32 dep=1;
  u32 bg[3]; box2grid(b, bg, grid[0], plane);
  u32 bbg[3];

  // Previous plane, previous row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2]==0 ? grid[2]-1 : bg[2]-1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Previous plane, same row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Previous plane, next row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Same plane, previous row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2];
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Same plane, same row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
  ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
  // Same plane, next row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Next plane, previous row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==0 ? grid[1]-1 : bg[1]-1;
  bbg[2] = bg[2]==grid[2]-1 ? 0 : bg[2]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Next plane, same row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1];
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }
  // Next plane, next row
  bbg[0] = bg[0]==0 ? grid[0]-1 : bg[0]-1;
  bbg[1] = bg[1]==grid[1]-1 ? 0 : bg[1]+1;
  for(u32 bb = 0; bb < 3; ++bb) {
    ocrAddDependence(list[grid2box(bbg,grid[0],plane)], edt, ++dep, DB_MODE_CONST);
    ++bbg[0]; if(bbg[0] == grid[0]) bbg[0] = 0;
  }

  ocrAddDependence(eam->phi.values, edt, ++dep, DB_MODE_CONST);
  ocrAddDependence(eam->rho.values, edt, ++dep, DB_MODE_CONST);
}

void fork_eam_force(ocrGuid_t sim, simulation* sim_ptr, ocrGuid_t cont, ocrGuid_t* list, u32 grid[3])
{
  u32 pairs = sim_ptr->bxs.boxes_num;

  ocrGuid_t tmp,red,fin;
  ocrEdtTemplateCreate(&tmp, eam_red_edt, 0, pairs+2);
  ocrEdtCreate(&red, tmp, 0, NULL, pairs+2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(fin, cont, CONT_DEPC, DB_MODE_CONST);
  ocrAddDependence(sim, red, 0, DB_MODE_RW);
  ocrAddDependence(sim_ptr->bxs.list, red, pairs+1, DB_MODE_RW);
  ocrEdtTemplateDestroy(tmp);

  pairs = 0; u32 plane = grid[0]*grid[1];
  ocrEdtTemplateCreate(&tmp, eam_edt, 2, 30);
  for(u32 b = 0; b < sim_ptr->bxs.boxes_num; ++b)
    spawn_pair(sim, &sim_ptr->pot.eam, tmp, red, list, b, grid, plane);
  ocrEdtTemplateDestroy(tmp);
}
