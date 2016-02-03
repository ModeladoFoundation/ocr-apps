-- Copyright 2015 Stanford University, NVIDIA Corporation
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

-- This tests the way in which default partitions in placed into the
-- environment, particularly with respect to the order that regions
-- are accessed in.

task f(s : region(int), t : partition(disjoint, s), n : int) : int
where reads(s) do
  var w = 0
  for i = 0, n do
    var si = t[i]
    for y in si do
      w += @y
    end
  end
  return w
end

task h1(a : region(int),
        b : partition(disjoint, a), c : partition(disjoint, a),
        d : cross_product(b, c), n : int) : int
where reads(a) do
  -- Access d[0].partition first
  var x = d[0].partition
  return f(d[0], x, n)
end

task h2(a : region(int),
        b : partition(disjoint, a), c : partition(disjoint, a),
        d : cross_product(b, c), n : int) : int
where reads(a) do
  -- Access d[0] first
  var x = d[0]
  return f(x, d[0].partition, n)
end

task g() : int
  var r = region(ispace(ptr, 5), int)
  var x0 = new(ptr(int, r))
  var x1 = new(ptr(int, r))
  var x2 = new(ptr(int, r))

  var rc0 = c.legion_coloring_create()
  c.legion_coloring_add_point(rc0, 0, __raw(x0))
  c.legion_coloring_add_point(rc0, 0, __raw(x1))
  c.legion_coloring_add_point(rc0, 0, __raw(x2))
  var p0 = partition(disjoint, r, rc0)
  c.legion_coloring_destroy(rc0)

  var rc1 = c.legion_coloring_create()
  var n = 5
  for i = 0, n do
    c.legion_coloring_ensure_color(rc1, i)
  end
  c.legion_coloring_add_point(rc1, 0, __raw(x0))
  c.legion_coloring_add_point(rc1, 1, __raw(x1))
  c.legion_coloring_add_point(rc1, 2, __raw(x2))
  var p1 = partition(disjoint, r, rc1)
  c.legion_coloring_destroy(rc1)

  var cp = cross_product(p0, p1)

  var r0 = cp[0].partition[0]
  @x0 = 100

  var r1 = cp[0].partition[1]
  @x1 = 20

  var r2 = cp[0].partition[2]
  @x2 = 3

  return h1(r, p0, p1, cp, n) + h2(r, p0, p1, cp, n)
end

task main()
  regentlib.assert(g() == 246, "test failed")
end
regentlib.start(main)
