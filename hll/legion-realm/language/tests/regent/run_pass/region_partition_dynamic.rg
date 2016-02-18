-- Copyright 2016 Stanford University
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

local c = regentlib.c

task f()
  var r = region(ispace(ptr, 5), int)
  var x0 = new(ptr(int, r))
  var x1 = new(ptr(int, r))
  var x2 = new(ptr(int, r))

  var rc = c.legion_coloring_create()
  c.legion_coloring_add_point(rc, 0, __raw(x0))
  c.legion_coloring_add_point(rc, 1, __raw(x1))
  c.legion_coloring_add_point(rc, 2, __raw(x2))
  var p = partition(disjoint, r, rc)
  c.legion_coloring_destroy(rc)

  for i = 0, 3 do
    var ri = p[i]
    for x in ri do
      @x = i
    end
  end

  var s = 0
  for i = 0, 3 do
    var ri = p[i]
    for x in ri do
      s += @x
    end
  end

  return s
end

task main()
  regentlib.assert(f() == 3, "test failed")
end
regentlib.start(main)
