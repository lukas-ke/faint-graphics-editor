// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/int-size.hh"
#include "geo/size.hh"
#include "geo/tri.hh"
#include "text/formatting.hh"
#include "text/text-buffer.hh"
#include "text/text-line.hh"
#include "util/text-geo.hh"

namespace{
using namespace faint;

class Test_TextInfo : public TextInfo{
public:
  coord GetWidth(const utf8_string& str) const override{
    return static_cast<coord>(str.size()) * 10.0;
  }

  coord ComputeRowHeight() const override{
    return 12.0;
  }

  Size TextSize(const faint::utf8_string& str) const override{
    return Size(GetWidth(str), ComputeRowHeight());
  }

  text_lines_t RowYourBoat(){
    return {{
      TextLine::SoftBreak(0.0, "Row, row, row your boat,"),
      TextLine::SoftBreak(0.0, "Gently down the stream."),
      TextLine::HardBreak(0.0, "Merrily, merrily, merrily, merrily,"),
      TextLine::HardBreak(0.0, "Life is but a dream.")}};
  }

  void Test_text_extents(){
    EQUAL(rounded(text_extents(*this, RowYourBoat())),
      IntSize(35 * 10, 4 * 12));
  }

  void Test_text_line_regions(){
    auto tris = text_line_regions(*this,
      Tri(Point(10,12), Point(300,12), Point(10,300)),
      RowYourBoat(),
      Align(HorizontalAlign::LEFT, VerticalAlign::TOP));

    ASSERT(tris.size() == 4);
    EQUAL(tris[0].P0(), Point(10,12));
    EQUAL(rounded(tris[1].P0()), rounded(Point(10,24)));
    EQUAL(rounded(tris[2].P0()), IntPoint(10,36));
    EQUAL(rounded(tris[3].P0()), IntPoint(10,48));
  }

  void Test_text_selection_region(){
    auto tris = text_selection_region(*this,
      Tri(Point(10,12), Point(300,12), Point(10,300)),
      RowYourBoat(),
      CaretRange(1,3),
      Align(HorizontalAlign::LEFT, VerticalAlign::TOP));

    ASSERT(tris.size() == 1);
    EQUAL(rounded(tris[0].P0()), rounded(Point(20, 12)));
  }
};

} // namespace


void test_text_geo(){
  using namespace faint;

  { // Test "caret_from_extents".

    std::vector<int> e = {0, 10, 17, 23};

    // Far outside to the left
    EQUAL(caret_from_extents(e, Point(-100,0), 0), 0);

    // Close outside to the left
    EQUAL(caret_from_extents(e, Point(-1,0), 0), 0);

    // Various distances inside
    EQUAL(caret_from_extents(e, Point(0,0), 0), 0);
    EQUAL(caret_from_extents(e, Point(4,0), 0), 0);
    EQUAL(caret_from_extents(e, Point(6,0), 0), 1);
    EQUAL(caret_from_extents(e, Point(10,0), 0), 1);
    EQUAL(caret_from_extents(e, Point(15,0), 0), 2);
    EQUAL(caret_from_extents(e, Point(18,0), 0), 2);
    EQUAL(caret_from_extents(e, Point(22,0), 0), 3);

    // Close outside to the right
    EQUAL(caret_from_extents(e, Point(24,0), 0), 3);

    // Far outside to the right
    EQUAL(caret_from_extents(e, Point(100,0), 0), 3);

    // Test with an offset of 100 both for the extents and each point,
    // should give the same results as the previous tests
    EQUAL(caret_from_extents(e, Point(0,0), 100), 0);
    EQUAL(caret_from_extents(e, Point(99,0), 100), 0);
    EQUAL(caret_from_extents(e, Point(100,0), 100), 0);
    EQUAL(caret_from_extents(e, Point(104,0), 100), 0);
    EQUAL(caret_from_extents(e, Point(106,0), 100), 1);
    EQUAL(caret_from_extents(e, Point(110,0), 100), 1);
    EQUAL(caret_from_extents(e, Point(115,0), 100), 2);
    EQUAL(caret_from_extents(e, Point(118,0), 100), 2);
    EQUAL(caret_from_extents(e, Point(122,0), 100), 3);
    EQUAL(caret_from_extents(e, Point(124,0), 100), 3);
    EQUAL(caret_from_extents(e, Point(200,0), 100), 3);

    // Extra test for the offset: extra far left
    EQUAL(caret_from_extents(e, Point(-100,0), 100), 0);
  }

  { // Test "char_from_extents".

    std::vector<int> e = {0, 10, 17, 23};
    EQUAL(char_from_extents(e, Point(-10,0), 0), 0);
    EQUAL(char_from_extents(e, Point(0,0), 0), 0);
    EQUAL(char_from_extents(e, Point(7,0), 0), 0);
    EQUAL(char_from_extents(e, Point(9,0), 0), 0);
    EQUAL(char_from_extents(e, Point(11,0), 0), 1);
    EQUAL(char_from_extents(e, Point(15,0), 0), 1);
    EQUAL(char_from_extents(e, Point(16,0), 0), 1);
    EQUAL(char_from_extents(e, Point(18,0), 0), 2);
    EQUAL(char_from_extents(e, Point(22,0), 0), 2);
    EQUAL(char_from_extents(e, Point(25,0), 0), 2);
    EQUAL(char_from_extents(e, Point(35,0), 0), 2);

    EQUAL(char_from_extents(e, Point(-10,0), 10), 0);
    EQUAL(char_from_extents(e, Point(0,0), 10), 0);
    EQUAL(char_from_extents(e, Point(10,0), 10), 0);
    EQUAL(char_from_extents(e, Point(17,0), 10), 0);
    EQUAL(char_from_extents(e, Point(19,0), 10), 0);
    EQUAL(char_from_extents(e, Point(21,0), 10), 1);
    EQUAL(char_from_extents(e, Point(25,0), 10), 1);
    EQUAL(char_from_extents(e, Point(26,0), 10), 1);
    EQUAL(char_from_extents(e, Point(28,0), 10), 2);
    EQUAL(char_from_extents(e, Point(32,0), 10), 2);
    EQUAL(char_from_extents(e, Point(35,0), 10), 2);
    EQUAL(char_from_extents(e, Point(45,0), 10), 2);
  }

  { // Test "index_to_row_column".

    text_lines_t lines = {
      TextLine::SoftBreak(0.0, "Alouette,"),
      TextLine::SoftBreak(0.0, "gentille alouette"),
      TextLine::HardBreak(0.0, "Alouette, je te plumerai"),
      TextLine::SoftBreak(0.0, "Je te plumerai")};

    auto fmt = [](const TextPos& p){
      return bracketed(comma_sep(str_uint(p.row), str_uint(p.col)));
    };

    using Tp = TextPos;
    EQUALF(index_to_row_column(lines, 0), Tp(0,0), fmt);
    EQUALF(index_to_row_column(lines, 1), Tp(0,1), fmt);
    EQUALF(index_to_row_column(lines, 8), Tp(0,8), fmt);
    EQUALF(index_to_row_column(lines, 9), Tp(1,0), fmt);
    EQUALF(index_to_row_column(lines, 10), Tp(1,1), fmt);
    EQUALF(index_to_row_column(lines, 25), Tp(1,16), fmt);
    EQUALF(index_to_row_column(lines, 26), Tp(2,0), fmt);
    EQUALF(index_to_row_column(lines, 49), Tp(2,23), fmt);
    EQUALF(index_to_row_column(lines, 50), Tp(3,0), fmt);
    EQUALF(index_to_row_column(lines, 62), Tp(3,12), fmt);
    EQUALF(index_to_row_column(lines, 63), Tp(3,13), fmt);

    // Out of bounds clamping
    EQUALF(index_to_row_column(lines, 64), Tp(3,13), fmt);
  }

  { // Test "text_extents", "text_line_regions", "text_selection_region"

    Test_TextInfo ti;
    ti.Test_text_extents();
    ti.Test_text_line_regions();
    ti.Test_text_selection_region();
  }
}
