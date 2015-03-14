// -*- coding: us-ascii-unix -*-
#include <map>
#include "geo/rect.hh"
#include "test-sys/test.hh"
#include "text/char-constants.hh"
#include "text/text-expression.hh"
#include "objects/object.hh"
#include "objects/objrectangle.hh"
#include "objects/objtext.hh"
#include "util/default-settings.hh"
#include "util/object-util.hh"

using namespace faint;

class MockExpressionContext : public ExpressionContext{
public:
  ~MockExpressionContext(){
    for (auto& p : m_objects){
      delete p.second;
    }
  }

  Optional<Calibration> GetCalibration() const override{
    return m_calibration;
  }

  virtual const Object* GetObject(const utf8_string& name) const override{
    auto it = m_objects.find(name);
    if (it == m_objects.end()){
      return nullptr;
    }
    return it->second;
  }

  void Add(Object* object, const utf8_string& name){
    m_objects[name] = object;
  }

  void SetCalibration(const Optional<Calibration>& calibration){
    m_calibration = calibration;
  }

private:
  Optional<Calibration> m_calibration;
  std::map<utf8_string, Object*> m_objects;
};

class EqualText{
public:
  EqualText(const utf8_string& str)
    : m_str(str)
  {}
  void operator()(const utf8_string& str) const{
    EQUAL(m_str, str)
  }
private:
  utf8_string m_str;
};

static void print_parse_error(const ExpressionParseError& error){
  std::stringstream ss;
  ss << error.pos << " " << error.description.str() << std::endl;
  MESSAGE(ss.str());
}

static void print_eval_error(const ExpressionEvalError& error){
  MESSAGE(error.description.str());
}

static void print_result(const utf8_string& result){
  MESSAGE(result.str());
}

class EqualExpr{
public:
  EqualExpr(const utf8_string& str, const ExpressionContext& c)
    : m_context(c),
      m_str(str)
  {}

  void operator()(const ExpressionTree& tree) const{
    tree.Evaluate(m_context).Visit(
      FAIL_UNLESS_CALLED_FWD(EqualText(m_str)),
      FAIL_IF_CALLED_FWD(print_eval_error));
  }

  bool operator=(const EqualExpr&) = delete;
private:
  const ExpressionContext& m_context;
  utf8_string m_str;
};

class EqualEvalError{
public:
  EqualEvalError(const utf8_string& d)
    : m_description(d)
  {}

  void operator()(const ExpressionEvalError& error) const{
    EQUAL(error.description, m_description);
  }
  utf8_string m_description;
};

class EvalErr{
public:
  EvalErr(const utf8_string& str, const ExpressionContext& c)
    : m_context(c),
      m_str(str)
  {}

  void operator()(const ExpressionTree& tree) const{
    tree.Evaluate(m_context).Visit(
      FAIL_IF_CALLED_FWD(print_result),
      FAIL_UNLESS_CALLED_FWD(EqualEvalError(m_str)));
  }

  bool operator=(const EvalErr&) = delete;
private:
  const ExpressionContext& m_context;
  utf8_string m_str;
};

class EqualParseError{
public:
  EqualParseError(const size_t pos, const utf8_string& description)
    : m_description(description),
      m_pos(pos)
  {}

  void operator()(const ExpressionParseError& error) const{
    EQUAL(error.pos, m_pos);
    EQUAL(error.description, m_description);
  }
  utf8_string m_description;
  size_t m_pos;
};

#define VALID_EXPRESSION(EXPR,RESULT)\
  parse_text_expression((EXPR)).Visit( \
    FAIL_UNLESS_CALLED_FWD(EqualExpr((RESULT), c)), \
    FAIL_IF_CALLED())

#define PARSE_ERROR(EXPR, COL, TXT)\
  parse_text_expression((EXPR)).Visit(\
    FAIL_IF_CALLED(),\
    FAIL_UNLESS_CALLED_FWD(\
      EqualParseError((COL), (TXT))));\

#define EVAL_ERROR(EXPR, TXT) \
  parse_text_expression((EXPR)).Visit( \
    FAIL_UNLESS_CALLED_FWD(EvalErr(TXT, c)),\
    FAIL_IF_CALLED_FWD(print_parse_error));

static utf8_string get_valid(const utf8_string& expr, ExpressionContext& c){
  return parse_text_expression(expr).Visit(
    [&](const ExpressionTree& tree){
      return tree.Evaluate(c).Visit(
        [](const utf8_string& result) -> utf8_string{
          return result;
        },
        [](const ExpressionEvalError&) -> utf8_string{
          FAIL();
        });},
    [](const ExpressionParseError&) -> utf8_string{
      FAIL();
  });
}

void test_text_expression(){
  using namespace faint;

  MockExpressionContext c;
  c.Add(create_rectangle_object(Tri(Point(0,0),Point(10,0),Point(0,10)),
    default_rectangle_settings()), "rect1");
  c.Add(create_rectangle_object(Tri(Point(0,0),Point(20,0),Point(0,20)),
    default_rectangle_settings()), "rect2");
  c.Add(new ObjText(Tri(Point(0,0), Point(100,0),Point(0,20)), "Hello world",
    default_text_settings()), "text1");

  VALID_EXPRESSION("", "");
  VALID_EXPRESSION("Hello", "Hello");

  PARSE_ERROR("Hello\\perimeter", 5, "perimeter missing parameter list.");

  PARSE_ERROR("Hello\\perimeter(", 16, "Expected closing parenthesis.");

  EVAL_ERROR("Hello\\perimeter(rect1,mm)",
    "Coordinate system not calibrated, mm not available.");

  VALID_EXPRESSION("Hello\\perimeter(rect1,px)", "Hello40.00");

  c.SetCalibration(Calibration(LineSegment(Point(0,0), Point(0,1)),
    10, "mm"));

  VALID_EXPRESSION("Hello\\perimeter(rect1,mm)", "Hello400.00");
  VALID_EXPRESSION("Hello\\perimeter(rect1,px)", "Hello40.00");

  VALID_EXPRESSION("Hello\\perimeter( rect1 , mm )  world", "Hello400.00  world");
  VALID_EXPRESSION("Hello\\perimeter(  rect1  ,  mm  ) world",
    "Hello400.00 world");

  VALID_EXPRESSION("Hello\\perimeter(rect2,mm)", "Hello800.00");

  PARSE_ERROR("\\perimeter(,mm)", 11, "Expected identifier");

  VALID_EXPRESSION("Hello\\perimeter(rect1,mm)\\perimeter(rect2,mm)world",
    "Hello400.00800.00world");

  EVAL_ERROR("Hello\\perimeter(not_real,mm)", "No object named \"not_real\".");

  VALID_EXPRESSION("\\pi", utf8_string(1, greek_small_letter_pi));

  VALID_EXPRESSION("\\pi\\pi", utf8_string(2, greek_small_letter_pi));

  VALID_EXPRESSION("\\pi\\deg", utf8_string(1, greek_small_letter_pi) +
    degree_sign);

  VALID_EXPRESSION("\\pi()", utf8_string(1, greek_small_letter_pi));

  KNOWN_ERROR(get_valid("\\perimeter(text1,px)", c) != "0.00"); // \def(err1)
  VALID_EXPRESSION("\\width(text1,px)", "100.00");
  VALID_EXPRESSION("\\height(text1,px)", "20.00");

  VALID_EXPRESSION("\\area(rect1,mm2)", "10000.00");

  EVAL_ERROR("Hello\\area(rect1,kg)",
    "Unknown unit: kg.");

  EVAL_ERROR("Hello\\area(rect1,kg2)",
    "Unknown unit: kg2.");

  EVAL_ERROR("Hello\\area(rect1,mm)",
    "Length unit mm specified for area. Use mm2 instead.");

  EVAL_ERROR("Hello\\area(rect1,px)",
    "Length unit px specified for area. Use px2 instead.");

  PARSE_ERROR("\\nonsense", 0,
    "Unknown constant: nonsense");

  PARSE_ERROR("\\nonsense()", 0,
    "Unknown command: nonsense");
}
