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

#include <algorithm> // std::find
#include "text/char-constants.hh"
#include "text/formatting.hh"
#include "text/text-expression.hh"
#include "text/text-expression-cmds.hh"

namespace faint{

ExpressionEvalError::ExpressionEvalError(const utf8_string& description)
  : description(description)
{}

class Text : public TextExpression{
public:
  Text(const utf8_string& text)
    : m_text(text)
  {}

  virtual TextExpression* Clone() const override{
    return new Text(m_text);
  }

  utf8_string Evaluate(const ExpressionContext&) const override{
    return m_text;
  }

  Text(const Text&) = delete;
  Text& operator=(const Text&) = delete;
private:
  utf8_string m_text;
};

class ExpressionTree::ExpressionTreeImpl{
public:
  ExpressionTreeImpl()
  {}
  std::vector<std::unique_ptr<TextExpression>> expressions;
private:
  ExpressionTreeImpl(const ExpressionTreeImpl&);
  ExpressionTreeImpl& operator=(const ExpressionTreeImpl&);
};

ExpressionTree::ExpressionTree(){
  m_impl = new ExpressionTreeImpl();
}

ExpressionTree::ExpressionTree(const ExpressionTree& other){
  m_impl = new ExpressionTreeImpl();
  auto& expressions = m_impl->expressions;
  const auto& otherExprs = other.m_impl->expressions;
  for (const auto& expr : otherExprs){
    expressions.emplace_back(expr->Clone());
  }
}

ExpressionTree::~ExpressionTree(){
  delete m_impl;
  m_impl = nullptr;
}

void ExpressionTree::Append(TextExpression* expr){
  assert(m_impl != nullptr);
  m_impl->expressions.emplace_back(std::unique_ptr<TextExpression>(expr));
}

eval_result_t ExpressionTree::Evaluate(const ExpressionContext& ctx) const{
  assert(m_impl != nullptr);
  try{
    utf8_string result = "";
    const auto& expressions(m_impl->expressions);
    for (const auto& expr : expressions){
      result += expr->Evaluate(ctx);
    }
    return {result};
  }
  catch (const ExpressionEvalError& e){
    return {e};
  }
}

ExpressionParseError::ExpressionParseError(size_t pos,
  const utf8_string& description)
  : pos(pos),
    description(description)
{}

class ParseState{
public:
  ParseState(const utf8_string& str, ExpressionTree& tree)
    : pos(0),
      storePos(0),
      str(str),
      tree(tree)
  {}

  void Append(TextExpression* token){
    tree.Append(token);
  }

  size_t Find(const utf8_char& ch){
    return str.find(ch, pos);
  }

  utf8_string ConsumeTo(size_t p2){
    assert(p2 >= storePos);
    if (p2 == utf8_string::npos){
      if (str.size() == 0){
        return "";
      }
      p2 = str.size();
    }
    return ConsumeCount(p2 - pos);
  }

  utf8_string ConsumeCount(size_t num){
    size_t oldPos = storePos;
    pos = pos + num;
    storePos = pos;
    return str.substr(oldPos, num);
  }

  void StoreTo(size_t p2){
    storePos = pos;
    pos = p2;
  }

  void EatWhitespace(){
    while (pos < str.size() && str[pos] == space){
      pos++;
    }
    storePos = pos;
  }

  bool Eat(const utf8_char& ch){
    if (On(ch)){
      pos++;
      storePos = pos;
      return true;
    }
    return false;
  }

  bool On(const utf8_char& ch){
    return str[pos] == ch;
  }

  // Index of the next character in str.
  size_t pos;

  // Characters should be appended from here on Consume<...>.
  // storePos is always <= pos.
  size_t storePos;

  const utf8_string& str;
  ExpressionTree& tree;

  ParseState& operator=(const ParseState&) = delete;
  ParseState(const ParseState&) = delete;
};


static bool identifier_char(const utf8_char& ch){
  unsigned int cp = ch.codepoint();
  return
    (0x30 <= cp && cp <= 0x39) || // 0-9
    (0x41 <= cp && cp <= 0x5a) || // A-Z
    (0x61 <= cp && cp <= 0x7a) || // a-z
    cp == 0x5f; // underscore
}

static utf8_string read_identifier(ParseState& st){
  size_t pos = st.pos;
  while (pos < st.str.size() && identifier_char(st.str[pos])){
    pos++;
  }
  return st.ConsumeTo(pos);
}

static bool create_constant(ParseState& st, const utf8_string& name){
  const auto& constants = constant_exprs();
  auto it = constants.find(name);
  if (it != constants.end()){
    st.Append(new Text(it->second));
    return true;
  }
  return false;
}

static void create_command(ParseState& st, const utf8_string& commandName,
  expr_list& args)
{
  if (TextExpression* expr = create_command_expr(commandName, args)){
    st.tree.Append(expr);
  }
  else if (args.empty()){
    if (!create_constant(st, commandName)){
      throw ExpressionParseError(0, // Fixme: Use real pos
        space_sep("Unknown command:", commandName));
    }
  }
  else{
    throw ExpressionParseError(0, // Fixme: Use real pos
      space_sep("Unknown command:", commandName));
  }
}

static void parse_args(ParseState& st, const utf8_string& commandName){
  expr_list args;

  st.EatWhitespace();
  if (st.Eat(right_parenthesis)){
    // No arguments
    create_command(st, commandName, args);
  }
  else{
    while (st.pos < st.str.size()){
      utf8_string identifier = read_identifier(st);
      if (identifier.empty()){
        throw ExpressionParseError(st.pos, "Expected identifier");
      }
      args.emplace_back(new Text(identifier));
      st.EatWhitespace();
      if (st.Eat(comma)){
        st.EatWhitespace();
      }
      else if (st.Eat(right_parenthesis)){
        create_command(st, commandName, args);
        return;
      }
      else{
        throw ExpressionParseError(st.pos, "Expected closing parenthesis.");
      }
    }
    throw ExpressionParseError(st.pos, "Expected closing parenthesis.");
  }
}

static void read_command(ParseState& st){
  size_t commandPos = st.pos;
  assert(st.Eat(backslash));
  utf8_string name = read_identifier(st);
  if (st.Eat(left_parenthesis)){
    parse_args(st, name);
  }
  else if (!create_constant(st, name)){
    auto names(command_expr_names());
    if (std::find(names.begin(), names.end(), name) != names.end()){
      throw ExpressionParseError(commandPos,
        space_sep(name, "missing parameter list."));
    }
    else{
      throw ExpressionParseError(commandPos, space_sep("Unknown constant:", name));
    }
  }
}

static void read_any(ParseState& st){
  while (st.pos != st.str.size()){
    size_t newPos = st.Find(utf8_char(backslash));
    if (newPos == utf8_string::npos){
      // No command found. Append the rest of the string
      st.Append(new Text(st.ConsumeTo(newPos)));
    }
    else if (newPos == st.str.size() - 1){
      // Command marker at end
      throw ExpressionParseError(st.str.size(),
        "\\ at end of text, expected command name.");
    }
    else if (st.str[newPos + 1] == backslash){
      // Escaped backslash
      st.StoreTo(newPos);
      continue;
    }
    else{
      // Command
      utf8_string text = st.ConsumeTo(newPos);
      if (!text.empty()){
        st.Append(new Text(text));
      }
      read_command(st);
    }
  }
}

parse_result_t parse_text_expression(const utf8_string& s){
  try{
    ExpressionTree tree;
    ParseState state(s, tree);
    read_any(state);
    return tree;
  }
  catch (const ExpressionParseError& error){
    return error;
  }
}

std::vector<utf8_string> expression_names(){
  std::vector<utf8_string> names;
  const auto& constants = constant_exprs();
  for (const auto& item : constants){
    names.emplace_back(backslash + item.first);
  }

  const auto& commands = command_expr_names();
  for (const auto& name : commands){
    names.emplace_back(backslash + name);
  }
  return names;
}

} // namespace
