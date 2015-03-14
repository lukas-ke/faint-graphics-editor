// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#include <vector>
#include <stack>
#include <sstream>
#include "geo/primitive.hh"
#include "util/parse-math-string.hh"

namespace faint{

using std::string;

static const string operators("%+-*/ ");

static bool is_operator(const string& token){
  return operators.find(token) != string::npos;
}

static bool is_operand(const string& token){
  return !is_operator(token);
}

static string::size_type find_operator(const string& s, string::size_type from){
  return s.find_first_of(operators, from);
}

static int precedence(const string& c){
  if (c == "-" || c == "+"){
    return 1;
  }
  if (c == "*" || c == "/"){
    return 2;
  }
  return 0;
}

// Split the string into operators and operands
static std::vector<string> tokenize(const string& s){
  std::vector<string> tokens;
  string::size_type prevPos = 0;

  while (prevPos != string::npos && prevPos < s.size()){
    string::size_type pos = find_operator(s, prevPos);
    if (pos == string::npos){
      // No operator left before the end of the string.
      // Push the remainder of the string as a single token
      tokens.push_back(s.substr(prevPos, s.size() - prevPos));
    }
    else {
      // Push the operand preceding the found operator, if any
      if (pos - prevPos > 0){
        tokens.push_back(s.substr(prevPos, pos - prevPos));
      }
      if (s[pos] != ' '){
        // Push the operator
        tokens.push_back(s.substr(pos, 1));
      }
    }
    prevPos = pos;
    if (prevPos != string::npos){
      prevPos += 1;
    }
  }
  return tokens;
}

static std::vector<string> infix_to_postfix(const std::vector<string>& infix){
  std::vector<string> postfix;
  std::stack<string> stack;

  for (const string& token : infix){
    if (is_operand(token)){
      postfix.push_back(token);
    }
    else if (stack.empty() || precedence(stack.top()) < precedence(token)){
      stack.push(token);
    }
    else {
      postfix.push_back(stack.top());
      stack.pop();
      stack.push(token);
    }
  }

  while (!stack.empty()){
    postfix.push_back(stack.top());
    stack.pop();
  }
  return postfix;
}

static double my_stof(const string& s){
  std::stringstream ss(s);
  double d;
  ss >> d;
  return d;
}

// Compute the value of a postfix expression
static double compute(std::vector<string> postfix){
  std::stack<double> operands;
  for (const string& token : postfix){
    if (is_operand(token)){
      operands.push(my_stof(token));
    }
    else {
      if (operands.size() < 2){
        // Error - crap stack
        return -1;
      }
      double rhs = operands.top();
      operands.pop();
      double lhs = operands.top();
      operands.pop();
      if (token == "-"){
        operands.push(lhs - rhs);
      }
      if (token == "+"){
        operands.push(lhs + rhs);
      }
      if (token == "/"){
        operands.push(lhs / rhs);
      }
      if (token == "*"){
        operands.push(lhs * rhs);
      }
    }
  }
  if (!operands.empty()){
    return operands.top();
  }
  return 0;
}

coord parse_math_string(const string& s, coord originalValue){
  std::vector<string> tokens = tokenize(s);
  bool previousWasOperator = false;
  for (size_t i = 0; i != tokens.size(); i++){
    if (tokens[i] == "%" && i != tokens.size() - 1){
      // Error: % within expression
      return -1;
    }
    else if (is_operator(tokens[i])){
      if (previousWasOperator){
        // Error: Two operators in a row
        return -1;
      }
      previousWasOperator = true;
    }
    else {
      previousWasOperator = false;
    }
  }

  bool percentage = !tokens.empty() && tokens.back() == "%";
  if (percentage){
    tokens.pop_back();
  }

  std::vector<string> postfix = infix_to_postfix(tokens);
  double result = compute(postfix);
  if (percentage){
    return rounded((originalValue * result) / 100);
  }
  return result;
}

} // namespace
