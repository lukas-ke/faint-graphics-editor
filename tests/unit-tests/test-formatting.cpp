// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"

#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/gradient.hh"
#include "bitmap/pattern.hh"
#include "geo/angle.hh"
#include "geo/axis.hh"
#include "geo/line.hh"
#include "geo/range.hh"
#include "geo/scale.hh"
#include "text/char-constants.hh"
#include "text/formatting.hh"
#include "util/index.hh"
#include "util/pos-info.hh"

void test_formatting(){
  using namespace faint;
  EQUAL(bracketed("hello world"), "(hello world)");
  EQUAL(capitalized("hello"), "Hello");
  EQUAL(lbl("Label", "Value"), "Label: Value");
  EQUAL(lbl("Label", 10), "Label: 10");
  EQUAL(quoted("hello world"), "\"hello world\"");
  EQUAL(str(IntPoint(1,2)), "1,2");
  EQUAL(str(IntSize(1,2)), "1,2");
  EQUAL(str(Point(1.1, 2.3)), "1,2");

  EQUAL(str_user(0_idx), "1");
  EQUAL(str_coder(0_idx), "0");

  // Paint
  EQUAL(str(Paint(Color(1,2,3,255))), "(RGB: 1,2,3), #010203");
  EQUAL(str(Paint(Color(1,2,3,10))), "(RGBA: 1,2,3,10), #010203");
  EQUAL(str(Paint(Color(1,2,3,0))), "(RGBA: 1,2,3,0), #010203");
  EQUAL(str(Paint(Pattern(Bitmap(IntSize(10,10))))), "Pattern");
  LinearGradient lg(Angle::Deg(90), {ColorStop({255,255,255},0.0),
    ColorStop({255,255,255},1.0)});
  RadialGradient rg(Point(10,10), Radii(10.0,20.0),
  {ColorStop({255,255,255},0.0), ColorStop({255,255,255},1.0)});
  EQUAL(str(Paint(Gradient(lg))), "Gradient");
  EQUAL(str(Paint(Gradient(rg))), "Gradient");

  // Scale
  EQUAL(str(Scale(1.0, 1.0)), "100%");
  EQUAL(str(Scale(1.0, 0.5)), "100%, 50%");
  EQUAL(str(Scale(0.5, 1.0)), "50%, 100%");

  // Axis
  EQUAL(str_axis_adverb(Axis::HORIZONTAL), "Horizontally");
  EQUAL(str_axis_adverb(Axis::VERTICAL), "Vertically");

  VERIFY(str_degrees_symbol(Angle::Deg(90)) == utf8_string("90.0") +
   degree_sign);
  VERIFY(str_degrees_symbol(90_deg) == utf8_string("90.0") +
    degree_sign);

  EQUAL(str_degrees(Angle::Deg(90)), "90.0");
  EQUAL(str_degrees(Angle::Deg(270)), "270.0");
  VERIFY(str_degrees_int_symbol(90) == utf8_string("90") + degree_sign);

  EQUAL(str_center_radius(Point(1.0,2.0), Radii(10.0,12.0)),
    "c: 1.0,2.0 r: 10.0,12.0");

  EQUAL(str_center_radius(Point(1.0,2.0), 10.0),
    "c: 1.0,2.0 r: 10.0");

  EQUAL(str_from_to(IntPoint(0,1), IntPoint(2,3)), "0,1->2,3");
  EQUAL(str_from_to(Point(0,1), Point(2,3)), "0,1->2,3");

  EQUAL(str_hex(Color(0,128,255)), "#0080FF");

  EQUAL(str_interval(Interval(min_t(10), max_t(20))), "10->20");
  EQUAL(str_interval(Interval(min_t(-20), max_t(-10))), "-20->-10");
  EQUAL(str_int_length(10), "10");
  EQUAL(str_length(10.5), "10.5");
  EQUAL(str_length(10.0), "10.0");
  VERIFY(str_line_status(IntLineSegment(IntPoint(1,0),IntPoint(3,0))) ==
    no_sep("1,0->3,0, length: 3, angle: 0", degree_sign));

  VERIFY(str_line_status_subpixel(LineSegment(Point(1,0),Point(3.1,0))) ==
    no_sep("1,0->3,0, length: 2.1, angle: 0", degree_sign));

  EQUAL(str_percentage(1,10), "10%");

  EQUAL(str_range(ClosedIntRange(min_t(10),max_t(20))), "10->20");

  EQUAL(str_rgb(Color(1,2,3,4)), "1,2,3");
  EQUAL(str_rgba(Color(1,2,3,4)), "1,2,3,4");
  EQUAL(str_rgba(Color(1,2,3,255)), "1,2,3,255");

  EQUAL(str_smart_rgba(Color(1,2,3,4)), "1,2,3,4");
  EQUAL(str_smart_rgba(Color(1,2,3,255)), "1,2,3");

  EQUAL(str_smart_rgba(Color(1,2,3,4), rgb_prefix(true)), "RGBA: 1,2,3,4");
  EQUAL(str_smart_rgba(Color(1,2,3,255), rgb_prefix(true)), "RGB: 1,2,3");

  EQUAL(str_int(-1), "-1");
  EQUAL(str_int(0), "0");
  EQUAL(str_int(1), "1");

  EQUAL(StrBtn(MouseButton::LEFT).This(false), "left");
  EQUAL(StrBtn(MouseButton::LEFT).This(true), "Left");
  EQUAL(StrBtn(MouseButton::LEFT).Other(false), "right");
  EQUAL(StrBtn(MouseButton::LEFT).Other(true), "Right");
  EQUAL(StrBtn(MouseButton::RIGHT).This(false), "right");
  EQUAL(StrBtn(MouseButton::RIGHT).This(true), "Right");
  EQUAL(StrBtn(MouseButton::RIGHT).Other(false), "left");
  EQUAL(StrBtn(MouseButton::RIGHT).Other(true), "Left");

  // Variadic template join
  EQUAL(join("SEP"), ""); // Separator only
  EQUAL(join("SEP", "hello"), "hello");
  EQUAL(join("SEP", "hello","hello"), "helloSEPhello");
  EQUAL(comma_sep("hello"), "hello");
  EQUAL(comma_sep("hello", "world"), "hello, world");
  EQUAL(comma_sep("hello", "round", "world"), "hello, round, world");
  EQUAL(no_sep("hello", "world"), "helloworld");

  // Vector join
  using str_vec = std::vector<utf8_string>;
  EQUAL(no_sep(str_vec()), "");
  EQUAL(no_sep(str_vec({"hello"})), "hello");
  EQUAL(no_sep({"hello", "world"}), "helloworld");
  EQUAL(comma_sep({"hello", "world"}), "hello, world");
  EQUAL(endline_sep({"hello", "world"}), "hello\nworld");
  EQUAL(endline_sep({"hello", "world", "meh"}), "hello\nworld\nmeh");
}
