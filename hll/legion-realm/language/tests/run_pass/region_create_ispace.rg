-- Copyright 2015 Stanford University
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

import "regent"

local i1d = index_type(int, "i1d")
struct i2 { x : int, y : int }
local i2d = index_type(i2, "i2d")

task f0(r : region(int))
where reads(r) do
  var s = 0
  for x in r do
    s += 1
  end
  return s
end

task f1(is : ispace(i1d), r : region(is, int))
where reads(r) do
  var s = 0
  for x in r do
    s += @x
  end
  return s
end

task f2(is : ispace(i2d), r : region(is, int))
where reads(r) do
  var s = 0
  for x in r do
    s += @x
  end
  return s
end

task main()
  var is0 = ispace(ptr, 5)
  var r0 = region(is0, int)

  var is1 = ispace(i1d, 7, 2)
  var r1 = region(is1, int)

  var is2 = ispace(i2d, { x = 2, y = 2 }, { x = 3, y = 4 })
  var r2 = region(is2, int)

  regentlib.assert(f0(r0) == 0, "test failed")

  for i in is1 do
    r1[i] = i
  end
  regentlib.assert(f1(is1, r1) == 35, "test failed")

  for i in is2 do
    r2[i] = 1
  end

  regentlib.assert(f2(is2, r2) == 4, "test failed")
end
regentlib.start(main)
