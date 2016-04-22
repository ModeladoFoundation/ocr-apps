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

-- fails-with:
-- vectorize_loops4.rg:30: vectorization failed: loop body has a corner case statement not supported for the moment
--    r[i] = i
--    ^

import "regent"

local i1d = index_type(int, "i1d")

task toplevel()
  var n = 8
  var is = ispace(i1d, n)
  var r = region(is, int)
  __demand(__vectorize)
  for i in is do
    r[i] = i
  end
end

regentlib.start(toplevel)
