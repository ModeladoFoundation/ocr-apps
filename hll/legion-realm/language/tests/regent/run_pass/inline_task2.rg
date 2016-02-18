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

task inc1(x : int) : int
  return x + 1
end

__demand(__inline)
task dec1(x : int) : int
  return x - 1
end

__demand(__inline)
task f(x : int) : int
  return dec1(inc1(x + 5))
end

task g(x : int) : int
  return x + 5
end

task main()
  for i = 0, 10 do
    var a = f(i)
    var b = g(i)
    regentlib.assert(a == b, "test failed")
  end
end

regentlib.start(main)
