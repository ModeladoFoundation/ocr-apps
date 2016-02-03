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

task f(r : region(int),
       p0 : partition(disjoint, r), p1 : partition(disjoint, r),
       p2 : partition(disjoint, r),
       cp : cross_product(p0, p1, p2))
where reads(r) do

  var r000 = cp[0].product[0].partition[0]
  var r001 = cp[0].product[0].partition[1]
  var r010 = cp[0].product[1].partition[0]
  var r011 = cp[0].product[1].partition[1]
  var r100 = cp[1].product[0].partition[0]
  var r101 = cp[1].product[0].partition[1]
  var r110 = cp[1].product[1].partition[0]
  var r111 = cp[1].product[1].partition[1]

  var s = 0
  for x in r000 do s += @x*1 end
  for x in r001 do s += @x*10 end
  for x in r010 do s += @x*100 end
  for x in r011 do s += @x*1000 end
  for x in r100 do s += @x*10000 end
  for x in r101 do s += @x*100000 end
  for x in r110 do s += @x*1000000 end
  for x in r111 do s += @x*10000000 end
  return s
end

task g(t : region(int),
       r : region(int),
       p0 : partition(disjoint, r), p1 : partition(disjoint, t),
       cp : cross_product(p0, p1))
where reads(r) do

  var r00 = cp[0].partition[0]
  var r01 = cp[0].partition[0]
  var r10 = cp[0].partition[1]
  var r11 = cp[0].partition[1]

  var s = 0
  for x in r00 do s += @x*1 end
  for x in r01 do s += @x*10 end
  for x in r10 do s += @x*100 end
  for x in r11 do s += @x*1000 end
  return s
end

task main()
  var r = region(ispace(ptr, 8), int)
  var x0 = new(ptr(int, r))
  var x1 = new(ptr(int, r))
  var x2 = new(ptr(int, r))
  var x3 = new(ptr(int, r))
  var x4 = new(ptr(int, r))
  var x5 = new(ptr(int, r))
  var x6 = new(ptr(int, r))
  var x7 = new(ptr(int, r))

  var colors0 = c.legion_coloring_create()
  c.legion_coloring_add_point(colors0, 0, __raw(x0))
  c.legion_coloring_add_point(colors0, 0, __raw(x1))
  c.legion_coloring_add_point(colors0, 0, __raw(x2))
  c.legion_coloring_add_point(colors0, 0, __raw(x3))
  c.legion_coloring_add_point(colors0, 1, __raw(x4))
  c.legion_coloring_add_point(colors0, 1, __raw(x5))
  c.legion_coloring_add_point(colors0, 1, __raw(x6))
  c.legion_coloring_add_point(colors0, 1, __raw(x7))
  var part0 = partition(disjoint, r, colors0)
  c.legion_coloring_destroy(colors0)

  var colors1 = c.legion_coloring_create()
  c.legion_coloring_add_point(colors1, 0, __raw(x0))
  c.legion_coloring_add_point(colors1, 0, __raw(x1))
  c.legion_coloring_add_point(colors1, 1, __raw(x2))
  c.legion_coloring_add_point(colors1, 1, __raw(x3))
  c.legion_coloring_add_point(colors1, 0, __raw(x4))
  c.legion_coloring_add_point(colors1, 0, __raw(x5))
  c.legion_coloring_add_point(colors1, 1, __raw(x6))
  c.legion_coloring_add_point(colors1, 1, __raw(x7))
  var part1 = partition(disjoint, r, colors1)
  c.legion_coloring_destroy(colors1)

  var colors2 = c.legion_coloring_create()
  c.legion_coloring_add_point(colors2, 0, __raw(x0))
  c.legion_coloring_add_point(colors2, 1, __raw(x1))
  c.legion_coloring_add_point(colors2, 0, __raw(x2))
  c.legion_coloring_add_point(colors2, 1, __raw(x3))
  c.legion_coloring_add_point(colors2, 0, __raw(x4))
  c.legion_coloring_add_point(colors2, 1, __raw(x5))
  c.legion_coloring_add_point(colors2, 0, __raw(x6))
  c.legion_coloring_add_point(colors2, 1, __raw(x7))
  var part2 = partition(disjoint, r, colors2)
  c.legion_coloring_destroy(colors2)

  var prod = cross_product(part0, part1, part2)
  for x in r do @x = 1 end

  regentlib.assert(f(r, part0, part1, part2, prod) == 11111111, "test failed")
  regentlib.assert(
    g(r, prod[0], prod[0].partition, part2, prod[0].product) == 1111,
    "test failed")
end
regentlib.start(main)
