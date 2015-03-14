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

#ifndef FAINT_TEXT_EXPRESSION_HH
#define FAINT_TEXT_EXPRESSION_HH
#include <vector>
#include "text/text-expression-context.hh"
#include "util/either.hh"

namespace faint{

class TextExpression;

class ExpressionEvalError{
  // Error type returned when evaluation of an ExpressionTree fails.
public:
  ExpressionEvalError(const utf8_string&);
  utf8_string description;
};

using eval_result_t = Either<utf8_string, ExpressionEvalError>;

class ExpressionTree{
  // An expression tree which can be evaluated with an ExpressionContext to
  // create an utf8_string.
public:
  class ExpressionTreeImpl;
  ExpressionTree();
  ExpressionTree(const ExpressionTree&);
  void Append(TextExpression*);
  ~ExpressionTree();

  eval_result_t Evaluate(const ExpressionContext&) const;

  ExpressionTree& operator=(const ExpressionTree&) = delete;
private:
  ExpressionTreeImpl* m_impl;
};

class ExpressionParseError{
  // Error returned when parse_text_expression fails.
public:
  ExpressionParseError(size_t, const utf8_string&);
  size_t pos;
  utf8_string description;
};

using parse_result_t = Either<ExpressionTree, ExpressionParseError>;

// Parse the text for commands, expressed as:
//
//  \<command>(<arg1, arg2...>)
//
// Returns either an ExpressionTree, which can be evaluated to
// create the parameterized string or an ExpressionParseError.
parse_result_t parse_text_expression(const utf8_string&);

// Returns the names of all expandable commands
std::vector<utf8_string> expression_names();

} // namespace

#endif
