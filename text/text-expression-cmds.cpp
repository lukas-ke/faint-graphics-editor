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

#include <functional>
#include <map>
#include <sstream>
#include "geo/tri.hh"
#include "objects/object.hh"
#include "text/char-constants.hh"
#include "text/formatting.hh"
#include "text/string-util.hh"
#include "text/text-expression.hh"
#include "text/text-expression-cmds.hh"
#include "text/text-expression-conversions.hh"
#include "util/object-util.hh"
#include "util/optional.hh"
#include "generated/text-expression-constants.hh"

namespace faint{

constexpr Precision EXPRESSION_FLOAT_PRECISION(2);

template<Optional<coord>(*func)(const ExpressionContext&)>
static coord or_throw(const utf8_string& unit, const ExpressionContext& c){
  return func(c).Visit(
    [](coord factor){
      return factor;
    },
    [&]() -> coord{
      throw ExpressionEvalError(space_sep("Coordinate system not calibrated,",
          unit, "not available."));
    });
}

static const Object* get_object_or_throw(const ExpressionContext& ctx,
  const utf8_string& name)
{
  const Object* obj = ctx.GetObject(name);
  if (obj == nullptr){
    throw ExpressionEvalError(space_sep("No object named",
      quoted(name) + "."));
  }
  return obj;

}

static void throw_if_wrong_arg_count(const utf8_string& command,
  size_t expected,
  const expr_list& args)
{
  size_t size = args.size();
  if (size != expected){
    size_t pos = 0; // Fixme: Use real command position
    throw ExpressionParseError(pos,
      space_sep(command, "expects", str_uint(expected), "arguments"));
  }
}

using MeasureFunc = const std::function<coord(const Object*,
  const ExpressionContext&)>;

template<typename T>
class MeasureExpression : public  TextExpression{
public:
  // Base for measurement expressions. Derived classes must specialize
  // some private virtuals.
  //
  // Templated on the derived type, to allow cloning.
  MeasureExpression(const utf8_string& name,
    const MeasureFunc& func,
    expr_list& args)
  : m_func(func),
    m_name(name)
  {
    throw_if_wrong_arg_count(name,
      2, args);
    m_object = std::move(args[0]);
    m_unit = std::move(args[1]);
  }

  TextExpression* Clone() const override{
    expr_list args;
    args.emplace_back(m_object->Clone());
    args.emplace_back(m_unit->Clone());
    return new T(m_name, m_func, args);
  }

  utf8_string Evaluate(const ExpressionContext& ctx) const override{
    const Object* object = get_object_or_throw(ctx, m_object->Evaluate(ctx));
    utf8_string wantedUnit = m_unit->Evaluate(ctx);
    coord pixels = m_func(object, ctx);

    if (wantedUnit.empty()){
      throw ExpressionEvalError(space_sep("No unit specified"));
    }
    const auto& conversions = length_conversions();
    CheckUnit(wantedUnit, conversions);

    const utf8_string intrinsicUnit(IntrinsicUnit(wantedUnit));

    const faint::coord value = intrinsicUnit == "px" ?
      pixels :
      ctx.GetCalibration().Visit(
        [&](const Calibration& c) -> coord{
          coord calibratedMeasure = pixels / AdjustConversion(c.Scale());

          auto it = conversions.find(intrinsicUnit);
          if (it == end(conversions)){
            throw ExpressionEvalError(space_sep("Unknown unit:",
              wantedUnit + "."));
          }
          else{
            return it->second[c.unit].Visit(
              [&](coord calibratedToWanted){
                return calibratedMeasure * calibratedToWanted;
              },
              [&]() -> coord{
                throw ExpressionEvalError(space_sep("No known conversion",
                    "from", wantedUnit, "to", c.unit));
              });
          }
        },

        [&]() -> coord{
          throw ExpressionEvalError(space_sep("Coordinate system not",
              "calibrated,", wantedUnit,
              "not available."));
        });

    return str(value, EXPRESSION_FLOAT_PRECISION);
  }

  MeasureExpression() = delete;

private:
  // Check that the unit is acceptable for the derived type (T).
  // Should throw ExpressionEvalError otherwise.
  virtual void CheckUnit(const utf8_string&,
    const conversions_map_t&) const = 0;

  // Returns the "intrinsic" unit that makes up unit, e.g. mm for mm2
  virtual utf8_string IntrinsicUnit(const utf8_string& unit) const = 0;

  // Adjust the conversion from pixels to calibrated measure,
  // allows squaring the measure for area measurements.
  virtual coord AdjustConversion(coord conversion) const = 0;

private:
  MeasureFunc m_func;
  const utf8_string& m_name;
  expr_ptr m_object;
  expr_ptr m_unit;
};

class AreaExpression : public MeasureExpression<AreaExpression>{
public:
  using MeasureExpression::MeasureExpression;

private:
  void CheckUnit(const utf8_string& unit,
    const conversions_map_t& c) const override
  {
    if (!ends(unit, with("2"))){
      if (unit != "px" && c.find(unit) == end(c)){
        throw ExpressionEvalError(space_sep("Unknown unit:", unit + "."));
      }
      else{
        throw ExpressionEvalError(space_sep("Length unit", unit,
            "specified for area. Use", no_sep(unit, "2"), "instead."));
      }
    }
  }

  utf8_string IntrinsicUnit(const utf8_string& unit) const override{
    // Strip "2" (for square)., e.g. mm2->mm
    return unit.substr(0, unit.size() - 1);
  }

  coord AdjustConversion(coord conversion) const override{
    // Use conversion^2 for area conversions
    return conversion * conversion;
  }
};

class LengthExpression : public MeasureExpression<LengthExpression>{
public:
  using MeasureExpression::MeasureExpression;
private:
  void CheckUnit(const utf8_string& unit,
    const conversions_map_t& c) const override
  {
    if (ends(unit, with("2"))){
      auto desquared(unit.substr(0, unit.size() - 1));
      if (desquared != "px" && c.find(desquared) == end(c)){
        throw ExpressionEvalError(space_sep("Unknown unit:", unit + "."));
      }
      else{
        throw ExpressionEvalError(space_sep("Area unit", unit,
            "specified for length. Use", desquared, "instead."));
      }
    }
  }

  utf8_string IntrinsicUnit(const utf8_string& unit) const override{
    // Unit used as is for looking up measurement conversions.
    return unit;
  }

  coord AdjustConversion(coord conversion) const override{
    // Used as is for lengths.
    return conversion;
  }
};

static unsigned int from_hex_str(const utf8_string& s){
  if (!is_ascii(s)){
    throw ExpressionEvalError(space_sep(s, "is not a hexadecimal value."));
  }
  std::istringstream ss(s.str());
  ss >> std::hex;
  unsigned int value = 0;
  if (!(ss >> value)){
    throw ExpressionEvalError(space_sep(s, "is not a hexadecimal value."));
  }
  return value;
}

class UnicodeExpression : public TextExpression{
public:
  UnicodeExpression(expr_list& args){
    throw_if_wrong_arg_count("u", // Fixme: Duplication
      1, args);
    m_arg = std::move(args[0]);
  }

  utf8_string Evaluate(const ExpressionContext& ctx) const override{
    utf8_string text = m_arg->Evaluate(ctx);
    unsigned int cp = from_hex_str(text);
    if (surrogate_codepoint(cp)){
      throw ExpressionEvalError(space_sep("Invalid code point:",
          text, "(UTF-16 surrogate pair)."));
    }
    else if (cp > 0x10ffff){
      throw ExpressionEvalError(space_sep("Invalid code point:",
          text, " > 10FFFF"));
    }
    return utf8_string(1, utf8_char(cp));
  }

  TextExpression* Clone() const override{
    expr_list args;
    args.emplace_back(m_arg->Clone());
    return new UnicodeExpression(args);
  }

private:
  expr_ptr m_arg;
};

class CommandExpressions{
  // Factory for commands (e.g. AreaExpression).
public:
  using creation_func = std::function<TextExpression*(expr_list& args)>;
  void Add(const utf8_string& s, const creation_func& f){
    m_creators[s] = f;
  }

  TextExpression* Create(const utf8_string& name, expr_list& args) const{
    auto it = m_creators.find(name);
    if (it == m_creators.end()){
      return nullptr;
    }
    return it->second(args);
  }

  std::vector<utf8_string> GetNames() const{
    std::vector<utf8_string> v;
    for (const auto& item : m_creators){
      v.emplace_back(item.first);
    }
    return v;
  }

private:
  std::map<utf8_string, creation_func> m_creators;
};

static CommandExpressions init_command_expressions(){
  CommandExpressions c;
  c.Add("perimeter",
    [](expr_list& args){
      return new LengthExpression("perimeter",
        [](const Object* obj, const ExpressionContext& ctx){
          if (is_text_object(obj)){
            // \ref(err1): Checking perimeter of self could lead to infinite
            // recursion, as ObjText::GetPath evaluates its
            // expression. Referring to other text objects should be
            // OK but I have no way to test this.
            return 0.0;
          }
          return perimeter(obj, ctx);
        }, args);
    });

  c.Add("width",
    [](expr_list& args){
      return new LengthExpression("width",
        [](const Object* obj, const ExpressionContext&){
          return obj->GetTri().Width();
        }, args);});


  c.Add("height",
    [](expr_list& args){
      return new LengthExpression("height",
        [](const Object* obj, const ExpressionContext&){
          return obj->GetTri().Height();
        }, args);});

  c.Add("area",
    [](expr_list& args){
      return new AreaExpression("area",
        [](const Object* obj, const ExpressionContext&){
          return object_area(obj);
        }, args);});


  c.Add("u",
    [](expr_list& args){
      return new UnicodeExpression(args);
    });
  return c;
}

const CommandExpressions& command_expressions(){
  static CommandExpressions c(init_command_expressions());
  return c;
}

std::vector<utf8_string> command_expr_names(){
  return command_expressions().GetNames();
}

TextExpression* create_command_expr(const utf8_string& name, expr_list& args){
  return command_expressions().Create(name, args);
}

} // namespace
