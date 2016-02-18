-- Copyright 2016 Stanford University, NVIDIA Corporation
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

-- runs-with:
-- [["-ll:rsize", "10"]]

import "regent"
import "bishop"

local c = bishoplib.c

mapper

$HAS_REGMEM = memories[kind=regmem].size > 0

task#foo[target=$proc] region#r1 {
  target : $HAS_REGMEM ? $proc.memories[kind=regmem] :
                         $proc.memories[kind=l1cache];
}

task[target=$proc] region {
  target : $proc.memories[kind=sysmem];
}

end

fspace Vec2
{
  x : float,
  y : float,
}

task foo(r1 : region(Vec2), r2 : region(int))
where reads(r1.x, r2), writes(r1.y)
do
  var memory_r1x = c.bishop_physical_region_get_memory(__physical(r1.x)[0])
  var kind_r1x = c.legion_memory_kind(memory_r1x)
  var memory_r1y = c.bishop_physical_region_get_memory(__physical(r1.y)[0])
  var kind_r1y = c.legion_memory_kind(memory_r1y)
  var memory_r2 = c.bishop_physical_region_get_memory(__physical(r2)[0])
  var kind_r2 = c.legion_memory_kind(memory_r2)
  regentlib.assert(kind_r1x == c.REGDMA_MEM or kind_r1x == c.LEVEL1_CACHE,
    "test failed")
  regentlib.assert(kind_r1y == c.REGDMA_MEM or kind_r1y == c.LEVEL1_CACHE,
    "test failed")
  regentlib.assert(kind_r2 == c.SYSTEM_MEM, "test failed")
end

task toplevel()
  var r1 = region(ispace(ptr, 10), Vec2)
  var r2 = region(ispace(ptr, 10), int)
  new(ptr(Vec2, r1), 10)
  new(ptr(int, r2), 10)
  foo(r1, r2)
end

bishoplib.register_bishop_mappers()
regentlib.start(toplevel)
