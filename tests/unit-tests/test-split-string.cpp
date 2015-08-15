// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "geo/int-size.hh"
#include "text/char-constants.hh"
#include "text/split-string.hh"
#include "util/optional.hh"
#include "util/distinct.hh"

namespace{

using namespace faint;

class category_test_split_string;
using px_per_char = Distinct<int, category_test_split_string, 0>;

class TextInfo_split_string : public TextInfo{
public:
  TextInfo_split_string(px_per_char pixelsPerChar)
    : m_pixelsPerChar(pixelsPerChar.Get())
  {}

  int GetWidth(const utf8_string& str) const override{
    // For easier testing, simply make each character
    // m_pixelsPerChar-wide, rather than requiring a font and dc.
    return resigned(str.size()) * m_pixelsPerChar;
  }

  int ComputeRowHeight() const override{
    FAIL();
  }

  IntSize TextSize(const faint::utf8_string&) const override{
    FAIL();
  }

private:
  int m_pixelsPerChar;
};

} // namespace

void test_split_string(){
  using namespace faint;

  {
    // No splitting
    TextInfo_split_string ti(px_per_char(10));
    auto lines = split_string(ti,
      "This is sort of a sentence.",
      max_width_t(280.0));

    ASSERT(lines.size() == 1);
    EQUAL(lines[0].text, "This is sort of a sentence.");
    NOT(lines[0].hardBreak);
  }

  {
    // Split at space
    TextInfo_split_string ti(px_per_char(10));
    auto lines = split_string(ti, "Hello world", max_width_t(60.0));
    ASSERT(lines.size() == 2);
    EQUAL(lines[0].text, "Hello ");
    NOT(lines[0].hardBreak);

    EQUAL(lines[1].text, "world");
    NOT(lines[1].hardBreak);
  }

  {
    // Split at hard line break
    TextInfo_split_string ti(px_per_char(10));
    auto lines = split_string(ti, "Hello\nworld", max_width_t(60.0));
    ASSERT(lines.size() == 2);

    EQUAL(lines[0].text, "Hello "); // Space stands in for \n
    VERIFY(lines[0].hardBreak);

    EQUAL(lines[1].text, "world");
    NOT(lines[1].hardBreak);
  }

  {
    // Split within word
    TextInfo_split_string ti(px_per_char(10));
    auto lines = split_string(ti, "Machiavellianism", max_width_t(80.0));
    ASSERT(lines.size() == 2);
    EQUAL(lines[0].text, "Machiave");
    NOT(lines[0].hardBreak);
    EQUAL(lines[1].text, "llianism");
    NOT(lines[1].hardBreak);
  }

  {
    // Split within word
    TextInfo_split_string ti(px_per_char(10));
    auto lines = split_string(ti,
      "Machiavellianism the employment of cunning and duplicity\nin statecraft or in general conduct", max_width_t(80.0));
    ASSERT(lines.size() == 15);
    EQUAL(lines[0].text, "Machiave");
    NOT(lines[0].hardBreak);
    EQUAL(lines[1].text, "llianism ");
    NOT(lines[1].hardBreak);
    EQUAL(lines[2].text, "the ");
    EQUAL(lines[3].text, "emplo");
    EQUAL(lines[4].text, "yment of ");
    EQUAL(lines[5].text, "cunning ");
    EQUAL(lines[6].text, "and ");
    EQUAL(lines[7].text, "dupl");
    EQUAL(lines[8].text, "icity ");
    EQUAL(lines[9].text, "in ");
    EQUAL(lines[10].text, "state");
    EQUAL(lines[11].text, "craft or ");
    EQUAL(lines[12].text, "in ");
    EQUAL(lines[13].text, "general ");
    EQUAL(lines[14].text, "conduct");
  }

  {
    TextInfo_split_string ti(px_per_char(10));
    // Unicode
    auto lines = split_string(ti, utf8_string(5, chars::snowman) + " " +
      utf8_string(5, chars::greek_capital_letter_delta), max_width_t(60.0));
    EQUAL(lines[0].text, utf8_string(5, chars::snowman) + " ");
    EQUAL(lines[1].text, utf8_string(5, chars::greek_capital_letter_delta));
  }
}
