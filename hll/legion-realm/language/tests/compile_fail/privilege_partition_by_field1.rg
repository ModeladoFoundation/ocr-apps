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
-- privilege_partition_by_field1.rg:24: invalid privileges in argument 1: reads($r)
--   var p = partition(r, c)
--                   ^

import "regent"

task f(r : region(int))
  var c = ispace(ptr, 5)
  var p = partition(r, c)
end
f:compile()
