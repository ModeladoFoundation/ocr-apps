/* Copyright 2016 Rice University, Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//reimplementing the realm_saxpy_cpu using OCR
//gcc -L${OCR_INSTALL}/lib -I${OCR_INSTALL}/include -locr_${OCR_TYPE} ocr_saxpy.c

#include<assert.h>
#include<stdlib.h>

#include"ocr.h"

struct param_type_t
{
  u64 count;
  double val;
};

typedef struct param_type_t param_type;

//shutdown  task
ocrGuid_t shutdown_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[])
{
  assert(argc == 0 && depc == 1);
  ocrShutdown();
  return NULL_GUID;
}

//check-result task that verifies saxpy(X,Y) == Z
ocrGuid_t check_result_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[])
{
  assert(argc == 2 && depc == 3);
  float *x_ptr = (float*)depv[0].ptr, *y_ptr = (float*)depv[1].ptr, *z_ptr = (float*)depv[2].ptr;
  param_type params =*(param_type *)argv;
  int count = params.count, success = 1, i;
  float alpha = params.val;

  for(i=0; i<count; i++){
    float expected = alpha * x_ptr[i] + y_ptr[i];
    float actual = z_ptr[i];
    // FMAs are too acurate
    float diff = (actual >= expected) ? actual - expected : expected - actual;
    float relative = diff / expected;
    if (relative > 1e-6)
    {
      PRINTF("Expected: %.8g Actual: %.8g\n", expected, actual);
      success = 0;
      break;
    }
  }
  if (success)
    PRINTF("SUCCESS!\n\n");
  else
    PRINTF("FAILURE!\n\n");

  ocrDbDestroy(depv[0].guid);
  ocrDbDestroy(depv[1].guid);
  ocrDbDestroy(depv[2].guid);

  return NULL_GUID;
}

//calculate-saxpy task that computes Z=saxpy(X,Y)
ocrGuid_t cpu_saxpy_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[])
{
  assert(argc == 2 && depc == 2);
  float *inst_z, *x_ptr = (float*)depv[0].ptr, *y_ptr = (float*)depv[1].ptr;
  param_type params =*(param_type *)argv;
  int count = params.count, i;
  float alpha = params.val;
  ocrGuid_t z_db_guid;
  ocrDbCreate(&z_db_guid, (void **)(&inst_z), count*sizeof(float), DB_PROP_NONE, NULL_GUID, NO_ALLOC);

  for(i=0; i<count; i++)
    inst_z[i] = alpha * x_ptr[i] + y_ptr[i];
  return z_db_guid;
}

//fill task that initializes all elements of the array with initial value
ocrGuid_t fill_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[])
{
  assert(argc == 2 && depc ==1);
  param_type params =*(param_type *)argv;
  int count = params.count, i;
  float init_val = params.val;
  float *arr = (float*)(depv[0].ptr);
  for(i=0; i<count; i++)
    arr[i] = init_val;
  return depv[0].guid;
}

//top-level task that computes saxpy and verifies the result
ocrGuid_t top_level_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[])
{
  float *inst_x, *inst_y;
  ocrGuid_t x_db_guid, y_db_guid;
  int count = 16383;
  //create data blocks for X and Y
  ocrDbCreate(&x_db_guid, (void **)(&inst_x), count*sizeof(float), DB_PROP_NO_ACQUIRE, NULL_GUID, NO_ALLOC);
  ocrDbCreate(&y_db_guid, (void **)(&inst_y), count*sizeof(float), DB_PROP_NO_ACQUIRE, NULL_GUID, NO_ALLOC);
  PRINTF("Created data blocks - X:0x%lx Y:0x%lx\n", x_db_guid, y_db_guid);

  float init_x_value = drand48();
  float init_y_value = drand48();

  //invoke fill task to fill X and Y with inital values
  ocrGuid_t fill_edt0, fill_edt1, fill_edt_t, out_evt[3];
  ocrEdtTemplateCreate(&fill_edt_t, fill_func, 2, 1);
  //fill X
  param_type params0 = {count, init_x_value};
  ocrEdtCreate(&fill_edt0, fill_edt_t, EDT_PARAM_DEF, (u64*)&params0, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &out_evt[0]);
  //fill Y
  param_type params1 = {count, init_y_value};
  ocrEdtCreate(&fill_edt1, fill_edt_t, EDT_PARAM_DEF, (u64*)&params1, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, NULL_GUID, &out_evt[1]);

  out_evt[2] = UNINITIALIZED_GUID;

  //calculate saxpy and return Z
  param_type params = {count, drand48()};
  ocrGuid_t cpu_edt_t, cpu_edt, cpu_out_evt;
  ocrEdtTemplateCreate(&cpu_edt_t, cpu_saxpy_func, 2, 2);
  ocrEdtCreate(&cpu_edt, cpu_edt_t, EDT_PARAM_DEF, (u64*)&params, EDT_PARAM_DEF, out_evt, EDT_PROP_NONE, NULL_GUID, &cpu_out_evt);

  //check result and verify saxpy(X,Y) == Z
  ocrGuid_t check_edt_t, check_edt, check_out_edt;
  ocrEdtTemplateCreate(&check_edt_t, check_result_func, 2, 3);
  ocrEdtCreate(&check_edt, check_edt_t, EDT_PARAM_DEF, (u64*)&params, EDT_PARAM_DEF, out_evt, EDT_PROP_NONE, NULL_GUID, &check_out_edt);

  //Enable the EDTs
  ocrAddDependence(cpu_out_evt, check_edt, 2, DB_MODE_RO);
  ocrAddDependence(x_db_guid, fill_edt0, 0, DB_MODE_RW);
  ocrAddDependence(y_db_guid, fill_edt1, 0, DB_MODE_RW);

  return NULL_GUID;
}

ocrGuid_t mainEdt(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[])
{
  PRINTF("starting mainEdt\n");

  //top level task
  ocrGuid_t top_edt_t, top_edt, top_out_evt;
  ocrEdtTemplateCreate(&top_edt_t, top_level_func, 0, 1);
  ocrEdtCreate(&top_edt, top_edt_t, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_GUID, &top_out_evt);

  //shutdown task
  ocrGuid_t sd_edt_t, sd_edt;
  ocrEdtTemplateCreate(&sd_edt_t, shutdown_func, 0, 1);
  ocrEdtCreate(&sd_edt, sd_edt_t, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, &top_out_evt, EDT_PROP_NONE, NULL_GUID, NULL);

  //enable the top level task
  ocrAddDependence(NULL_GUID, top_edt, 0, DB_MODE_RO);

  return NULL_GUID;
}

