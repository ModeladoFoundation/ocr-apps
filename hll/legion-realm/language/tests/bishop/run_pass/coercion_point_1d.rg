-- Copyright 2016 Stanford University, NVIDIA Corporation
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

-- runs-with:
-- [["-ll:cpu", "4"]]

import "regent"
import "bishop"

local c = bishoplib.c

mapper

task#foo[index=$p] {
  target : processors[isa=x86][(($p + $p + 4) / 2) % processors[isa=x86].size];
}

task#bar {
  target : processors[isa=x86][1];
}

task {
  target : processors[isa=x86][2];
}

end

task bar()
  var proc =
    c.legion_runtime_get_executing_processor(__runtime(), __context())
  var procs = c.bishop_all_processors()
  regentlib.assert(procs.list[1].id == proc.id, "assertion failed")
end

task foo(x : int)
  var proc =
    c.legion_runtime_get_executing_processor(__runtime(), __context())
  var procs = c.bishop_all_processors()
  regentlib.assert(procs.list[(x + 2) % procs.size].id == proc.id,
    "assertion failed")
end

task toplevel()
  var proc =
    c.legion_runtime_get_executing_processor(__runtime(), __context())
  var procs = c.bishop_all_processors()
  bar()
  __demand(__parallel)
  for i = 0, 4 do
    foo(i)
  end
  regentlib.assert(procs.list[2].id == proc.id, "assertion failed")
end

bishoplib.register_bishop_mappers()
regentlib.start(toplevel)
