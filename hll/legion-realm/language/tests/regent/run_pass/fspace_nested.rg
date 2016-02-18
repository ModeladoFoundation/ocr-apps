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

fspace inner {
  a : int,
  b : int,
  c : int,
}

fspace middle {
  d : inner,
  e : inner,
  f : inner,
}

fspace outer {
  g : middle,
  h : middle,
  i : middle,
}

task f(x : outer)
  var y = x
  y.g.d = { a = 1, b = 2, c = 3 }
  y.h.{d, e, f}.c = 12
  y.i = y.g
  return y
end

task main()
  var z : outer
  z.{g, h, i}.{d, e, f}.{a, b, c} = 0
  var w = f(z)

  regentlib.assert(w.g.d.a == 1 and w.g.d.b == 2 and w.g.d.c == 3, "test failed")
  regentlib.assert(w.h.d.c == 12 and w.h.e.c == 12 and w.h.f.c == 12, "test failed")
  regentlib.assert(w.i.d.a == 1 and w.i.d.b == 2 and w.i.d.c == 3, "test failed")
end
regentlib.start(main)
