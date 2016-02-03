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

local ast = require("regent/ast")
local symbol_table = require("regent/symbol_table")

local traverse_symbols = {}
local codegen

function traverse_symbols.block(defined, undefined, node)
  local defined_local = defined:new_local_scope()
  node.stats:map(function(stat)
    traverse_symbols.stat(defined_local, undefined, stat)
  end)
end

function traverse_symbols.stat(defined, undefined, node)
  if node:is(ast.typed.stat.If) then
    traverse_symbols.expr(defined, undefined, node.cond)
    traverse_symbols.block(defined, undefined, node.then_block)
    node.elseif_blocks:map(function(elseif_block)
      traverse_symbols.stat(defined, undefined, elseif_block)
    end)
    traverse_symbols.block(defined, undefined, node.else_block)

  elseif node:is(ast.typed.stat.Elseif) or
         node:is(ast.typed.stat.While) then
    traverse_symbols.expr(defined, undefined, node.cond)
    traverse_symbols.block(defined, undefined, node.block)

  elseif node:is(ast.typed.stat.ForNum) then
    node.values:map(function(value)
      traverse_symbols.expr(defined, undefined, value)
    end)
    local defined_local = defined:new_local_scope()
    defined_local:insert(node, node.symbol, true)
    traverse_symbols.block(defined_local, undefined, node.block)

  elseif node:is(ast.typed.stat.ForList) then
    traverse_symbols.expr(defined, undefined, node.value)
    local defined_local = defined:new_local_scope()
    defined_local:insert(node, node.symbol, true)
    traverse_symbols.block(defined_local, undefined, node.block)

  elseif node:is(ast.typed.stat.ForListVectorized) then
    traverse_symbols.expr(defined, undefined, node.value)
    local defined_local = defined:new_local_scope()
    defined_local:insert(node, node.symbol, true)
    traverse_symbols.block(defined_local, undefined, node.orig_block)

  elseif node:is(ast.typed.stat.Repeat) then
    traverse_symbols.block(defined, undefined, node.block)
    traverse_symbols.expr(defined, undefined, node.until_cond)

  elseif node:is(ast.typed.stat.Block) then
    traverse_symbols.block(defined, undefined, node.block)

  elseif node:is(ast.typed.stat.Var) then
    node.values:map(function(value)
      traverse_symbols.expr(defined, undefined, value)
    end)
    node.symbols:map(function(symbol)
      defined:insert(node, symbol, true)
    end)

  elseif node:is(ast.typed.stat.VarUnpack) then
    traverse_symbols.expr(defined, undefined, node.value)
    node.symbols:map(function(symbol)
      defined:insert(node, symbol, true)
    end)

  elseif node:is(ast.typed.stat.Return) then
    traverse_symbols.expr(defined, undefined, node.value)

  elseif node:is(ast.typed.stat.Break) then

  elseif node:is(ast.typed.stat.Assignment) or
         node:is(ast.typed.stat.Reduce) then
    node.lhs:map(function(lh)
      traverse_symbols.expr(defined, undefined, lh)
    end)
    node.rhs:map(function(rh)
      traverse_symbols.expr(defined, undefined, rh)
    end)

  elseif node:is(ast.typed.stat.Expr) then
    traverse_symbols.expr(defined, undefined, node.expr)

  else
    assert(false, "unexpected node type " .. tostring(node:type()))
  end
end

function traverse_symbols.expr(defined, undefined, node)
  if node:is(ast.typed.expr.ID) then
    if not defined:safe_lookup(node.value) then
      undefined[node.value] = true
    end

  elseif node:is(ast.typed.expr.FieldAccess) then
    -- we have to extract the symbols from the generated Terra code
    local defined_local = defined:new_local_scope()
    local code = codegen(node)
    local actions = code.actions
    traverse_symbols.terra_stat(defined_local, undefined, actions.tree)

  elseif node:is(ast.typed.expr.IndexAccess) then
    local defined_local = defined:new_local_scope()
    local code = codegen(node)
    local actions = code.actions
    traverse_symbols.terra_stat(defined_local, undefined, actions.tree)
    traverse_symbols.expr(defined, undefined, node.value)
    traverse_symbols.expr(defined, undefined, node.index)

  elseif node:is(ast.typed.expr.Unary) then
    traverse_symbols.expr(defined, undefined, node.rhs)

  elseif node:is(ast.typed.expr.Binary) then
    traverse_symbols.expr(defined, undefined, node.lhs)
    traverse_symbols.expr(defined, undefined, node.rhs)

  elseif node:is(ast.typed.expr.Ctor) then
    node.fields:map(function(field)
      traverse_symbols.expr(defined, undefined, field)
    end)

  elseif node:is(ast.typed.expr.CtorRecField) or
         node:is(ast.typed.expr.CtorListField) then
    traverse_symbols.expr(defined, undefined, node.value)

  elseif node:is(ast.typed.expr.Constant) then

  elseif node:is(ast.typed.expr.Call) then
    node.args:map(function(arg)
      traverse_symbols.expr(defined, undefined, arg)
    end)

  elseif node:is(ast.typed.expr.Cast) then
    traverse_symbols.expr(defined, undefined, node.arg)

  elseif node:is(ast.typed.expr.StaticCast) then
    traverse_symbols.expr(defined, undefined, node.value)

  elseif node:is(ast.typed.expr.Deref) then
    local defined_local = defined:new_local_scope()
    local code = codegen(node)
    local actions = code.actions
    traverse_symbols.terra_stat(defined_local, undefined, actions.tree)
    traverse_symbols.expr(defined, undefined, node.value)

  else
    assert(false, "unexpected node type " .. tostring(node:type()))
  end
end

function traverse_symbols.find_undefined_symbols(expr_codegen, symbol, block)
  local defined = symbol_table:new_global_scope()
  defined:insert(block, symbol, true)
  local undefined = {}
  codegen = expr_codegen
  traverse_symbols.block(defined, undefined, block)
  return undefined
end

function traverse_symbols.terra_stat(defined, undefined, stat)
  if stat:is "treelist" then
    local statements = stat.statements or stat.trees
    statements:map(function(statement)
      traverse_symbols.terra_stat(defined, undefined, statement)
    end)

  elseif stat:is "defvar" then
    stat.variables:map(function(variable)
      defined:insert(nil, variable.symbol, true)
    end)

  elseif stat:is "assignment" then
    stat.lhs:map(function(lh)
      traverse_symbols.terra_expr(defined, undefined, lh)
    end)
    stat.rhs:map(function(rh)
      traverse_symbols.terra_expr(defined, undefined, rh)
    end)

  elseif stat:is "apply" then

  elseif stat:is "if" then
    stat.branches:map(function(branch)
      traverse_symbols.terra_stat(defined, undefined, branch)
    end)
    traverse_symbols.terra_stat(defined, undefined, stat.orelse)

  elseif stat:is "ifbranch" then
    traverse_symbols.terra_stat(defined, undefined, stat.body)
    traverse_symbols.terra_expr(defined, undefined, stat.condition)

  elseif stat:is "block" then
    traverse_symbols.terra_stat(defined, undefined, stat.body)

  else
    assert(false, "unsupported terra statement")
  end
end

function traverse_symbols.terra_expr(defined, undefined, exp)
  if exp:is "var" then
    if not defined:safe_lookup(exp.value) then
      undefined[exp.value] = true
    end

  elseif exp:is "index" or exp:is "select" then
    traverse_symbols.terra_expr(defined, undefined, exp.value)

  elseif exp:is "operator" then
    exp.operands:map(function(operand)
      traverse_symbols.terra_expr(defined, undefined, operand)
    end)

  elseif exp:is "apply" then
    exp.arguments:map(function(argument)
      traverse_symbols.terra_expr(defined, undefined, argument)
    end)

  elseif exp:is "constant" then

  else
    assert(false, "unsupported terra expression")
  end
end

return traverse_symbols
