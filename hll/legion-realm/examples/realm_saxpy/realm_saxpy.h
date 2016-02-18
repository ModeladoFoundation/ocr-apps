/* Copyright 2016 Stanford University, NVIDIA Corporation
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

#ifndef __REALM_SAXPY__
#define __REALM_SAXPY__

#include "common.h"
#include "utilities.h"
#include "accessor.h"
#include "arrays.h"
#include "realm.h"

using namespace Realm;
using namespace LegionRuntime::Arrays;
using namespace LegionRuntime::Accessor;

struct SaxpyArgs {
public:
  RegionInstance x_inst, y_inst, z_inst;
  float alpha;
  Rect<1> bounds;
};

#endif
