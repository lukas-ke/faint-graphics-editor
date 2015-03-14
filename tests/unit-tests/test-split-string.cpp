// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "geo/size.hh"
#include "text/char-constants.hh"
#include "text/split-string.hh"
#include "util/optional.hh"

namespace{
using namespace faint;

class TextInfo_split_string : public TextInfo{
public:
  coord GetWidth(const utf8_string& str) const override{
    // Use ten pixels for each character
    return static_cast<coord>(str.size()) * 10.0;
  }

  coord ComputeRowHeight() const override{
    FAIL();
  }

  Size TextSize(const faint::utf8_string&) const override{
    FAIL();
  }
};

}

void test_split_string(){
  using namespace faint;
  TextInfo_split_string ti;

  {
    // No splitting
    auto lines = split_string(ti,
      "This is sort of a sentence.",
      max_width_t(280.0));

    ASSERT(lines.size() == 1);
    // Fixme: Figure out why I add an extra space, and get rid of it
    // probably
    EQUAL(lines[0].text, "This is sort of a sentence. ");
    NOT(lines[0].hardBreak);
  }

  {
    // Split at space
    auto lines = split_string(ti, "Hello world", max_width_t(60.0));
    ASSERT(lines.size() == 2);
    EQUAL(lines[0].text, "Hello ");
    NOT(lines[0].hardBreak);

    EQUAL(lines[1].text, "world ");
    NOT(lines[1].hardBreak);
  }

  {
    // Split at hard line break
    auto lines = split_string(ti, "Hello\nworld", max_width_t(60.0));
    ASSERT(lines.size() == 2);

    EQUAL(lines[0].text, "Hello "); // Fixme: extra space?
    VERIFY(lines[0].hardBreak);

    EQUAL(lines[1].text, "world ");
    NOT(lines[1].hardBreak);
  }

  {
    // Split within word
    auto lines = split_string(ti, "Machiavellianism", max_width_t(80.0));
    ASSERT(lines.size() == 2);
    EQUAL(lines[0].text, "Machiave ");
    NOT(lines[0].hardBreak);
    EQUAL(lines[1].text, "llianism ");
    NOT(lines[1].hardBreak);
  }

  {
    // Split within word
    auto lines = split_string(ti,
      "Machiavellianism the employment of cunning and duplicity\nin statecraft or in general conduct", max_width_t(80.0));
    ASSERT(lines.size() == 15);
    EQUAL(lines[0].text, "Machiave ");
    NOT(lines[0].hardBreak);
    EQUAL(lines[1].text, "llianism ");
    NOT(lines[1].hardBreak);
    EQUAL(lines[2].text, "the ");
    EQUAL(lines[3].text, "emplo ");
    EQUAL(lines[4].text, "yment of ");
    EQUAL(lines[5].text, "cunning ");
    EQUAL(lines[6].text, "and ");
    EQUAL(lines[7].text, "dupl ");
    EQUAL(lines[8].text, "icity ");
    EQUAL(lines[9].text, "in ");
    EQUAL(lines[10].text, "state ");
    EQUAL(lines[11].text, "craft or ");
    EQUAL(lines[12].text, "in ");
    EQUAL(lines[13].text, "general ");
    EQUAL(lines[14].text, "conduct ");
  }

  {
    // Unicode
    auto lines = split_string(ti, utf8_string(5,snowman) + " " +
      utf8_string(5, greek_capital_letter_delta), max_width_t(60.0));
    EQUAL(lines[0].text, utf8_string(5, snowman) + " ");
    EQUAL(lines[1].text, utf8_string(5, greek_capital_letter_delta) + " ");
  }
}
