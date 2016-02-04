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

#include "realm_hello.h"

enum {
  TOP_LEVEL_TASK = Processor::TASK_ID_FIRST_AVAILABLE+0,
};

void top_level_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{ 
  printf("HELLO WORLD from Processor " IDFMT "\n len = %ld, val = %d\n", p.id, arglen, *(int*)args);
  Runtime rt = Runtime::get_runtime();
  rt.shutdown();
}

int main(int argc, char **argv)
{
  Runtime rt;

  int ret = rt.init(&argc, &argv);
  printf("init return %d\n", ret);

  rt.register_task(TOP_LEVEL_TASK, top_level_task);
 
  rt.run(TOP_LEVEL_TASK, Runtime::ONE_TASK_ONLY, &argc, sizeof(int));
  return 0;
}
