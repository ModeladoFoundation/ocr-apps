#include <ocr.h>

#include "hpgmg.h"

ocrGuid_t do_solves(ocrGuid_t start, mg_type* mg_ptr, int num, int warmup)
{

 ocrGuid_t c = start;
  int n,l;
for(n = 0; n < num; ++n) {
  if (!warmup)
   c = time_all(mg_ptr,c);
    PRINTF("FMGSolve...\n");
    c = init_ur(mg_ptr, c,0);
    c = init_ur(mg_ptr, c,1);
    c = restrict_all(mg_ptr, c);
    c = solve(mg_ptr, c, NULL_GUID);
    for(l = mg_ptr->max_levels-1; l; --l) {
      c = interpolate(l, mg_ptr, c, FMG_INTERPOLATE);
      c = vcycle(l-1, mg_ptr, c);
    }
    c = scaled_residual_norm(mg_ptr, c);
   if (!warmup)
    c = time_all(mg_ptr,c);
 }

  return c;
}

ocrGuid_t time_all(mg_type* mg_ptr, ocrGuid_t start) {
  ocrGuid_t i,i_t,fin;
  ocrEdtTemplateCreate(&i_t, time_edt, 1, 2);

  u64 p = 4;
  ocrEdtCreate(&i, i_t, 1, &p, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[0], i, 0, DB_MODE_RW);
  ocrAddDependence(start, i, 1, DB_MODE_CONST);

  ocrEdtTemplateDestroy(i_t);
  return fin;
}

ocrGuid_t init_ur(mg_type* mg_ptr, ocrGuid_t start, int flag)
{
  ocrGuid_t i,i_t,fin;
  ocrEdtTemplateCreate(&i_t, init_ur_level_edt, 1, 2);
  u64 max_levels = mg_ptr->max_levels;

  ocrEdtCreate(&i, i_t, 1, &max_levels, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  if (!flag)
    ocrAddDependence(mg_ptr->levels[0], i, 0, DB_MODE_CONST);
  else
    ocrAddDependence(mg_ptr->levels[mg_ptr->max_levels-1], i, 0, DB_MODE_CONST);
  ocrAddDependence(start, i, 1, DB_MODE_CONST);

  ocrEdtTemplateDestroy(i_t);
  return fin;
}

ocrGuid_t restrict_all(mg_type* mg_ptr, ocrGuid_t start)
{
  ocrGuid_t r,r_t,fin;
  ocrEdtTemplateCreate(&r_t, restrict_level_edt, 1, 3);
  ocrGuid_t box = NULL_GUID;

  int l;
  for(l=0; l < mg_ptr->max_levels-2; ++l) {
    ocrEdtCreate(&r, r_t, 1, &box, 3, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
    ocrAddDependence(mg_ptr->levels[l], r, 0, DB_MODE_RW);
    ocrAddDependence(mg_ptr->levels[l+1], r, 1, DB_MODE_CONST);
    ocrAddDependence(start, r, 2, DB_MODE_CONST);
    start = fin;
  }
    ocrEventCreate(&box, OCR_EVENT_ONCE_T, 1);
    ocrEdtCreate(&r, r_t, 1, &box, 3, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
    ocrAddDependence(mg_ptr->levels[l], r, 0, DB_MODE_RW);
    ocrAddDependence(mg_ptr->levels[l+1], r, 1, DB_MODE_CONST);
    ocrAddDependence(start, r, 2, DB_MODE_CONST);

  ocrEdtTemplateDestroy(r_t);
  return box;
}

ocrGuid_t restrict_level(int l, mg_type* mg_ptr, ocrGuid_t start)
{
  ocrGuid_t r,r_t,fin;
  ocrEdtTemplateCreate(&r_t, restrict_level_edt, 2, 3);

  ocrGuid_t box = NULL_GUID;
  if(l == mg_ptr->max_levels-2)
    ocrEventCreate(&box, OCR_EVENT_ONCE_T, 1);

  u64 pv[2] = {box,1};
  ocrEdtCreate(&r, r_t, 2, pv, 3, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[l], r, 0, DB_MODE_RW);
  ocrAddDependence(mg_ptr->levels[l+1], r, 1, DB_MODE_CONST);
  ocrAddDependence(start, r, 2, DB_MODE_CONST);


  /////// time restrict ///////
  start = fin;
  ocrGuid_t i,i_t;
  ocrEdtTemplateCreate(&i_t, time_edt, 1, 2);
  u64 p = 2;
  ocrEdtCreate(&i, i_t, 1, &p, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[l], i, 0, DB_MODE_RW);
  ocrAddDependence(start, i, 1, DB_MODE_CONST);
  ocrEdtTemplateDestroy(i_t);
  /////////////////////////////


  ocrEdtTemplateDestroy(r_t);
  return box != NULL_GUID ? box : fin;
}

ocrGuid_t solve(mg_type* mg_ptr, ocrGuid_t start1, ocrGuid_t start2)
{
  ocrGuid_t s,s_t,fin;
  if (start2 == NULL_GUID) {
    ocrEdtTemplateCreate(&s_t, solve_edt, 0, 2);
    ocrEdtCreate(&s, s_t, 0, NULL, 2, NULL, EDT_PROP_NONE, NULL_GUID, &fin);
    ocrAddDependence(mg_ptr->levels[mg_ptr->max_levels-1], s, 0, DB_MODE_CONST);
    ocrAddDependence(start1, s, 1, DB_MODE_CONST);
  } else {
    ocrEdtTemplateCreate(&s_t, solve_edt, 0, 3);
    ocrEdtCreate(&s, s_t, 0, NULL, 3, NULL, EDT_PROP_NONE, NULL_GUID, &fin);
    ocrAddDependence(mg_ptr->levels[mg_ptr->max_levels-1], s, 0, DB_MODE_CONST);
    ocrAddDependence(start1, s, 1, DB_MODE_CONST);
    ocrAddDependence(start2, s, 2, DB_MODE_CONST);

  }

  ocrEdtTemplateDestroy(s_t);
  return fin;
}

ocrGuid_t interpolate(int l, mg_type* mg_ptr, ocrGuid_t start, u64 type)
{
  ocrGuid_t i,i_t,x,x_t,fin;
  ocrEdtTemplateCreate(&i_t, interpolate_level_edt, 1, 3);
  ocrEdtTemplateCreate(&x_t, exchange_level_edt, 2, 2);

  if (type == FMG_INTERPOLATE) {
    u64 iter[2] = {0,1};
    ocrEdtCreate(&x, x_t, 2, iter, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
    ocrAddDependence(mg_ptr->levels[l], x, 0, DB_MODE_RW);
    ocrAddDependence(start, x, 1, DB_MODE_CONST);
    start = fin;
  }

  ocrEdtCreate(&i, i_t, 1, &type, 3, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[l-1], i, 0, DB_MODE_CONST);
  ocrAddDependence(mg_ptr->levels[l], i, 1, DB_MODE_RW);
  ocrAddDependence(start, i, 2, DB_MODE_CONST);


  //////// time interpolate ////////
  start = fin;
  ocrGuid_t tm,i_tm;
  ocrEdtTemplateCreate(&i_tm, time_edt, 1, 2);
  u64 p = 3;
  ocrEdtCreate(&tm, i_tm, 1, &p, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[l], tm, 0, DB_MODE_RW);
  ocrAddDependence(start, tm, 1, DB_MODE_CONST);
  ocrEdtTemplateDestroy(i_tm);
  /////////////////////////////


  ocrEdtTemplateDestroy(x_t);
  ocrEdtTemplateDestroy(i_t);
  return fin;
}

ocrGuid_t vcycle(int ln, mg_type* mg_ptr, ocrGuid_t start)
{
  ocrGuid_t c = start, c1 = start;
  int l;
  for(l = ln; l < mg_ptr->max_levels-1; ++l) {
    c = smooth(l, mg_ptr, c1);
    c = residual(l, mg_ptr, c);
    c = restrict_level(l, mg_ptr, c);
    c1 = zero_vector(l+1, mg_ptr, 0, c);
  }
  c = solve(mg_ptr, c,c1);
  for(l = mg_ptr->max_levels-1; l>ln; --l) {
    c = interpolate(l, mg_ptr, c, VC_INTERPOLATE);
    c = smooth(l-1, mg_ptr, c);
  }

  return c;
}

ocrGuid_t zero_vector(int l, mg_type* mg_ptr, u64 type, ocrGuid_t start)
{
  ocrGuid_t i,i_t,fin;
  ocrEdtTemplateCreate(&i_t, zero_vector_level_edt, 1, 2);

  ocrEdtCreate(&i, i_t, 1, &type, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[l], i, 0, DB_MODE_CONST);
  ocrAddDependence(start, i, 1, DB_MODE_CONST);

  ocrEdtTemplateDestroy(i_t);
  return fin;
}


ocrGuid_t smooth(int l, mg_type* mg_ptr, ocrGuid_t start)
{
  ocrGuid_t s,s_t,x,x_t,fin;
  ocrEdtTemplateCreate(&s_t, smooth_level_edt, 1, 2);
  ocrEdtTemplateCreate(&x_t, exchange_level_edt, 1, 2);

  int iter;
  for(iter = 0; iter < NUM_SMOOTHS*CHEBYSHEV_DEGREE; ++iter) {
    u64 iter_num = iter;
    ocrEdtCreate(&x, x_t, 1, &iter_num, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
    ocrAddDependence(mg_ptr->levels[l], x, 0, DB_MODE_RW);
    ocrAddDependence(start, x, 1, DB_MODE_CONST);
    start = fin;
    ocrEdtCreate(&s, s_t, 1, &iter_num, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
    ocrAddDependence(mg_ptr->levels[l], s, 0, DB_MODE_RW);
    ocrAddDependence(start, s, 1, DB_MODE_CONST);
    start = fin;
  }

  //////// time smooth ////////
  ocrGuid_t i,i_t;
  ocrEdtTemplateCreate(&i_t, time_edt, 1, 2);
  u64 p = 0;
  ocrEdtCreate(&i, i_t, 1, &p, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[l], i, 0, DB_MODE_RW);
  ocrAddDependence(start, i, 1, DB_MODE_CONST);
  ocrEdtTemplateDestroy(i_t);
  /////////////////////////////

  ocrEdtTemplateDestroy(x_t);
  ocrEdtTemplateDestroy(s_t);
  return fin;
}

ocrGuid_t residual(int l, mg_type* mg_ptr, ocrGuid_t start)
{
  ocrGuid_t i,i_t,x,x_t,fin;
  ocrEdtTemplateCreate(&i_t, residual_level_edt, 0, 2);
  ocrEdtTemplateCreate(&x_t, exchange_level_edt, 1, 2);

  u64 iter = 0;
  ocrEdtCreate(&x, x_t, 1, &iter, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[l], x, 0, DB_MODE_RW);
  ocrAddDependence(start, x, 1, DB_MODE_CONST);
  start = fin;

  ocrEdtCreate(&i, i_t, 0, NULL, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[l], i, 0, DB_MODE_RW);
  ocrAddDependence(start, i, 1, DB_MODE_CONST);


  //////// time residual ////////
  start = fin;
  ocrGuid_t tm,i_tm;
  ocrEdtTemplateCreate(&i_tm, time_edt, 1, 2);
  u64 p = 1;
  ocrEdtCreate(&tm, i_tm, 1, &p, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[l], tm, 0, DB_MODE_RW);
  ocrAddDependence(start, tm, 1, DB_MODE_CONST);
  ocrEdtTemplateDestroy(i_tm);
  /////////////////////////////


  ocrEdtTemplateDestroy(x_t);
  ocrEdtTemplateDestroy(i_t);
  return fin;
}

ocrGuid_t scaled_residual_norm(mg_type* mg_ptr, ocrGuid_t start)
{
  ocrGuid_t i,i_t,x,x_t,fin;
  ocrEdtTemplateCreate(&i_t, residual_level_edt, 1, 2);
  ocrEdtTemplateCreate(&x_t, exchange_level_edt, 1, 2);

  u64 iter = 0;
  ocrEdtCreate(&x, x_t, 1, &iter, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[0], x, 0, DB_MODE_RW);
  ocrAddDependence(start, x, 1, DB_MODE_CONST);
  start = fin;

  u64 p = 1;
  ocrEdtCreate(&i, i_t, 1, &p, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[0], i, 0, DB_MODE_RW);
  ocrAddDependence(start, i, 1, DB_MODE_CONST);

  ocrEdtTemplateDestroy(i_t);

  //////// time residual ////////
  start = fin;
  ocrGuid_t tm,i_tm;
  ocrEdtTemplateCreate(&i_tm, time_edt, 1, 2);
  u64 op = 1;
  ocrEdtCreate(&tm, i_tm, 1, &op, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[0], tm, 0, DB_MODE_RW);
  ocrAddDependence(start, tm, 1, DB_MODE_CONST);
  ocrEdtTemplateDestroy(i_tm);
  /////////////////////////////


  start = fin;

  ocrEdtTemplateCreate(&i_t, mulv_level_edt, 0, 2);

  ocrEdtCreate(&i, i_t, 0, NULL, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[0], i, 0, DB_MODE_CONST);
  ocrAddDependence(start, i, 1, DB_MODE_CONST);

  ocrEdtTemplateDestroy(i_t);
  start = fin;

  ocrEdtTemplateCreate(&i_t, norm_level_edt, 0, 2);

  ocrEdtCreate(&i, i_t, 0, NULL, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[0], i, 0, DB_MODE_RW);
  ocrAddDependence(start, i, 1, DB_MODE_CONST);


  ocrEdtTemplateDestroy(i_t);
  start = fin;

  ocrEdtTemplateCreate(&i_t, norm_final_edt, 0, 2);

  ocrEdtCreate(&i, i_t, 0, NULL, 2, NULL, EDT_PROP_FINISH, NULL_GUID, &fin);
  ocrAddDependence(mg_ptr->levels[0], i, 0, DB_MODE_CONST);
  ocrAddDependence(start, i, 1, DB_MODE_CONST);

  ocrEdtTemplateDestroy(i_t);
  start = fin;

  return fin;
}
