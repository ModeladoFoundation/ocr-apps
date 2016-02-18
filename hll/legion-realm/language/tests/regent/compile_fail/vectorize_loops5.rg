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
-- vectorize_loops5.rg:39: vectorization failed: loop body has an assignment between expressions that have inadmissible types
--    e.v1 = e.v2
--    ^

import "regent"

struct vec2
{
  x : float,
  y : float,
}

fspace fs
{
  v1 : vec2[1],
  v2 : vec2[1],
}

task toplevel()
  var n = 8
  var r = region(ispace(ptr, n), fs)
  __demand(__vectorize)
  for e in r do
    e.v1 = e.v2
  end
end

regentlib.start(toplevel)
