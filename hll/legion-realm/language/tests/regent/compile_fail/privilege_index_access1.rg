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
-- privilege_index_access1.rg:25: invalid privilege reads($r) for dereference of i1d(int32, $r)
--   return r[p]
--          ^

import "regent"

local i1d = index_type(int, "i1d")

task f(is : ispace(i1d), r : region(is, int), p : i1d) : int
  return r[p]
end
f:compile()
