#include <ocr.h>

#include "comd.h"
#include "constants.h"
#include "potentials.h"

//depv: rpf0, .., rpf25
ocrGuid_t eamforce_edt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
  return NULL_GUID;
}

u8 init_eam(const char* pot_dir, const char* pot_name, const char* pot_type, potential_t* p, real_t dt)
{
  u8 insane = 0;
  //if(strcmp(pot_type, "setfl" ) == 0)
  //   insane = eam_init_setfl(p, pot_dir, pot_name);
  //else if(strcmp(pot_type,"funcfl") == 0)
  //   insane = eam_init_funcfl(p, pot_dir, pot_name);
  //else {
  //  printf("EAM Type not supported\n");
  //  return 1;
  //}
  //if(!insane) {
  //  p->mass *= amu2internal_mass;
  //  p->inv_mass_2 = 0.5/p->mass;
  //  p->inv_mass_dt = dt/p->mass;
  //}
  //p->force_edt = eamforce_edt;
  return insane;
}
