// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You
// may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef FAINT_TEXT_EXPRESSION_CMDS_HH
#define FAINT_TEXT_EXPRESSION_CMDS_HH

#include <map>
#include <memory>
#include <vector>

namespace faint{

class TextExpression{
public:
  TextExpression(){}
  virtual ~TextExpression() = default;
  virtual TextExpression* Clone() const = 0;
  virtual utf8_string Evaluate(const ExpressionContext&) const = 0;
  TextExpression& operator=(const TextExpression&) = delete;
  TextExpression(const TextExpression&) = delete;
};

using expr_ptr = std::unique_ptr<TextExpression>;
using expr_list = std::vector<expr_ptr>;

// Returns an expression for the passed in name with specified
// arg-list, or nullptr if the name is not a command.
//
// Throws ExpressionParseError if the argument count is incorrect for
// the command.
TextExpression* create_command_expr(const utf8_string& name, expr_list& args);

// Returns the command expression names supported by
// create_command_expr.
std::vector<utf8_string> command_expr_names();

// Returns the map of names to constant text values.
const std::map<utf8_string, utf8_string>& constant_exprs();

} // namespace

#endif
