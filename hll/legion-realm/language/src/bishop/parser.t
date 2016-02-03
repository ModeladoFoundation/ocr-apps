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

-- Bishop Parser

local parsing = require("parsing")
local ast = require("bishop/ast")

local parser = {}

function parser.value(p)
  local value
  if p:matches(p.number) then
    local token = p:next(p.number)
    local pos = ast.save(p)
    value = ast.unspecialized.expr.Constant {
      value = token.value,
      position = pos,
    }
  elseif p:nextif("$") then
    local token = p:next(p.name)
    local pos = ast.save(p)
    value = ast.unspecialized.expr.Variable {
      value = token.value,
      position = pos,
    }
  elseif p:matches(p.name) then
    local token = p:next(p.name)
    local pos = ast.save(p)
    value = ast.unspecialized.expr.Keyword {
      value = token.value,
      position = pos,
    }
  elseif p:nextif("(") then
    value = p:expr()
    p:expect(")")
  else
    p:error("unexpected type of value")
  end
  return value
end

function parser.expr_complex(p)
  local expr = p:value()
  while true do
    if p:nextif("[") then
      if p:matches(p.name) and p:lookahead("=") then
        local constraints = p:constraints()
        local pos = ast.save(p)
        expr = ast.unspecialized.expr.Filter {
          value = expr,
          constraints = constraints,
          position = pos,
        }
      else
        local value = p:expr()
        local pos = ast.save(p)
        expr = ast.unspecialized.expr.Index {
          value = expr,
          index = value,
          position = pos,
        }
      end
      p:expect("]")
    elseif p:nextif(".") then
      local token = p:next(p.name)
      local pos = ast.save(p)
      expr = ast.unspecialized.expr.Field {
        value = expr,
        field = token.value,
        position = pos,
      }
    else
      break
    end
  end
  return expr
end

function parser.expr_unary(precedence)
  return function(p)
    local position = ast.save(p)
    local op = p:next().type
    local rhs = p:expr(precedence)
    return ast.unspecialized.expr.Unary {
      op = op,
      rhs = rhs,
      position = position,
    }
  end
end

function parser.expr_binary_left(p, lhs)
  local position = lhs.position
  local op = p:next().type
  local rhs = p:expr(op)
  return ast.unspecialized.expr.Binary {
    op = op,
    lhs = lhs,
    rhs = rhs,
    position = position,
  }
end

parser.expr = parsing.Pratt()
  :prefix("-", parser.expr_unary(50))
  :infix("*", 40, parser.expr_binary_left)
  :infix("/", 40, parser.expr_binary_left)
  :infix("%", 40, parser.expr_binary_left)
  :infix("+", 30, parser.expr_binary_left)
  :infix("-", 30, parser.expr_binary_left)
  :prefix(parsing.default, parser.expr_complex)


function parser.property(p)
  local field = p:next(p.name)
  local pos = ast.save(p)
  p:expect(":")
  local value = p:expr()
  p:expect(";")
  return ast.unspecialized.Property {
    field = field.value,
    value = value,
    position = pos,
  }
end

function parser.properties(p)
  local properties = terralib.newlist()
  while not p:matches("}") do
    properties:insert(p:property())
  end
  return properties
end

function parser.constraint(p)
  local pos = ast.save(p)
  local field = p:next(p.name).value
  p:expect("=")
  local value = p:expr()
  if value:is(ast.unspecialized.expr.Variable) then
    return ast.unspecialized.PatternMatch {
      field = field,
      binder = value.value,
      position = pos,
    }
  else
    return ast.unspecialized.Constraint {
      field = field,
      value = value,
      position = pos,
    }
  end
end

function parser.constraints(p)
  local constraints = terralib.newlist()
  if not p:matches("]") then
    constraints:insert(p:constraint())
    while p:matches("and") do
      p:expect("and")
      constraints:insert(p:constraint())
    end
  end
  return constraints
end

function parser.element(p)
  local ctor
  local tbl = {}
  if p:nextif("task") then
    ctor = ast.unspecialized.element.Task
  elseif p:nextif("region") then
    ctor = ast.unspecialized.element.Region
  --elseif p:nextif("for") then
  --  ctor = ast.unspecialized.element.BlockElement
  --  tbl = { type = "for" }
  --elseif p:nextif("while") then
  --  ctor = ast.unspecialized.element.BlockElement
  --  tbl = { type = "while" }
  --elseif p:nextif("do") then
  --  ctor = ast.unspecialized.element.BlockElement
  --  tbl = { type = "do" }
  else
    p:error("unexpected element name")
  end
  local pos = ast.save(p)

  local name = terralib.newlist()
  local classes = terralib.newlist()
  local constraints = terralib.newlist()

  while p:matches("#") or p:matches(".") or p:matches("[") do
    if p:nextif("#") then
      if #name > 0 then
        p:error("selector cannot have multiple names")
      end
      name:insert(p:next(p.name).value)
    elseif p:nextif(".") then
      classes:insert(p:next(p.name).value)
    elseif p:nextif("[") then
      constraints:insertall(p:constraints())
      p:expect("]")
    else
      p:error("unexpected token")
    end
  end

  tbl.name = name
  tbl.classes = classes
  tbl.constraints = terralib.newlist()
  tbl.patterns = terralib.newlist()
  constraints:map(function(c)
    if c:is(ast.unspecialized.Constraint) then tbl.constraints:insert(c)
    else assert(c:is(ast.unspecialized.PatternMatch)) tbl.patterns:insert(c) end
  end)
  tbl.position = pos

  return ctor(tbl)
end

function parser.selector(p)
  local elements = terralib.newlist()
  repeat
    elements:insert(p:element())
  until p:matches(",") or p:matches("{")
  return ast.unspecialized.Selector {
    elements = elements,
    position = elements[1].position,
  }
end

function parser.selectors(p)
  local selectors = terralib.newlist()
  -- every rule should have at least one selector
  selectors:insert(p:selector())
  while p:matches(",") do
    p:expect(",")
    selectors:insert(p:selector())
    if p:matches("{") then break end
  end
  return selectors
end

function parser.rule(p)
  local selectors = p:selectors()
  p:expect("{")
  local properties = p:properties()
  p:expect("}")
  return ast.unspecialized.Rule {
    selectors = selectors,
    properties = properties,
    position = selectors[1].position,
  }
end

function parser.rules(p)
  local rules = terralib.newlist()
  while not p:matches("end") do
    rules:insert(p:rule())
  end
  return rules
end

function parser.top(p)
  if not p:matches("mapper") then
    p:error("unexpected token in top-level statement")
  end
  p:expect("mapper")
  local pos = ast.save(p)

  local rules = p:rules()

  if not p:matches("end") then
    p:error("unexpected token in top-level statement")
  end
  p:expect("end")

  return ast.unspecialized.Rules {
    rules = rules,
    position = pos,
  }
end

function parser:parse(lex)
  return parsing.Parse(self, lex, "top")
end

return parser
