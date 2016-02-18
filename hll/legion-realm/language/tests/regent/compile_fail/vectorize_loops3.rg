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

-- fails-with:
-- vectorize_loops3.rg:40: vectorization failed: loop body has aliasing references of path region(fs2()).v
--     e.p1.v = e.p2.v
--     ^

import "regent"

fspace fs2
{
  v : float,
}

fspace fs1(r : region(fs2))
{
  p1 : ptr(fs2, r),
  p2 : ptr(fs2, r),
}

task f(r2 : region(fs2), r : region(fs1(r2)))
where
  reads(r2.v, r.p1, r.p2),
  writes(r2.v, r.p1, r.p2)
do
  __demand(__vectorize)
  for e in r do
    e.p1.v = e.p2.v
  end
end

task toplevel()
  var n = 8
  var r2 = region(ispace(ptr, n), fs2)
  var r1 = region(ispace(ptr, n), fs1(r2))
  for i = 0, n / 2 do
    var ptr2_a = new(ptr(fs2, r2))
    var ptr2_b = new(ptr(fs2, r2))
    ptr2_a.v = 1.0
    ptr2_b.v = 2.0

    var ptr1_a = new(ptr(fs1(r2), r1))
    var ptr1_b = new(ptr(fs1(r2), r1))
    ptr1_a.p1 = ptr2_a
    ptr1_a.p2 = ptr2_b

    ptr1_b.p1 = ptr2_b
    ptr1_b.p2 = ptr2_a
  end
  f(r2, r1)
end

regentlib.start(toplevel)
