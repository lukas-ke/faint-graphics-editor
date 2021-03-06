// -*- coding: us-ascii-unix -*-
#include <algorithm>
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "tests/test-util/threw-exception.hh"

#include "text/char-constants.hh"
#include "text/string-util.hh"

void test_utf8_string(){
  using namespace faint;

  {
    // Construction
    EQUAL(utf8_string(10, utf8_char("A")).size(), 10);
    EQUAL(utf8_string(10, utf8_char("A")).bytes(), 10);
    VERIFY(is_ascii(utf8_string(10, utf8_char("A"))));
  }

  // Helper to check that utf8_string throws std::out_range when it
  // should.
  auto threw_out_of_range = [](auto f){
    return threw_exception<std::out_of_range>(f);
  };

  { // utf8_string::at

    EQUAL(utf8_string("abc").at(0), utf8_char("a"));
    EQUAL(utf8_string("abc").at(2), utf8_char("c"));

    // at throws std::out_of_range on invalid index
    VERIFY(threw_out_of_range([](){utf8_string("abc").at(3);}));
  }

  { // utf8_string::empty
    VERIFY(utf8_string().empty());
    VERIFY(utf8_string("").empty());
    NOT(utf8_string("a").empty());
  }

  // utf8_string::back
  EQUAL(utf8_string("hello").back(), utf8_char("o"));

  // utf8_string::front
  EQUAL(utf8_string("hello").front(), utf8_char("h"));


  { // utf8_string::substr

    // substr out-of-bounds end is clamped
    EQUAL(utf8_string("abcd").substr(0, 100), "abcd");

    // substr throws std::out_of_range on initial index out-of-bounds
    VERIFY(threw_out_of_range([](){
        utf8_string("abc").substr(3, 1);
      }));
  }

  { // utf8_string::erase

    { // No span
      utf8_string s = "Hello";
      s.erase();
      VERIFY(s.empty());
    }

    { // From index to end
      utf8_string s = "hello";
      s.erase(1);
      EQUAL(s, "h");
    }

    { // Interval
      utf8_string s = "hello";
      s.erase(0, 1);
      EQUAL(s, "ello");
    }

    { // Offset interval
      utf8_string s = "hello";
      s.erase(1, 2);
      EQUAL(s, "hlo");
    }

    { // Overstated end is clamped
      utf8_string s = "hello";
      s.erase(1, 1000);
      EQUAL(s, "h");
    }

    { // erase throws std::out_of_range when initial index is invalid
      utf8_string s = "hello";
      VERIFY(threw_out_of_range([&](){s.erase(5);}));
      EQUAL(s, "hello");
    }
  }

  { // utf8_string::insert(size_t, const utf8_string&)
    {
      utf8_string s("abc");
      s.insert(0, "hello");
      EQUAL(s, "helloabc");
    }

    {
      utf8_string s("abc");
      s.insert(2, "hello");
      EQUAL(s, "abhelloc");
    }

    {
      utf8_string s("abc");
      s.insert(3, "hello");
      EQUAL(s, "abchello");
    }

    {
      utf8_string s("abc");
      VERIFY(threw_out_of_range(
        [&s](){
          s.insert(4, "hello");
        }));
      EQUAL(s, "abc"); // unmodified
    }
  }

  { // utf8_string::insert(size_t, size_t, const utf8_char&)
    {
      utf8_string s("abc");
      s.insert(0, 4, utf8_char("x"));
      EQUAL(s, "xxxxabc");
    }

    {
      utf8_string s("abc");
      s.insert(2, 4, utf8_char("x"));
      EQUAL(s, "abxxxxc");
    }

    {
      utf8_string s("abc");
      s.insert(3, 4, utf8_char("x"));
      EQUAL(s, "abcxxxx");
    }

    {
      utf8_string s("abc");
      VERIFY(threw_out_of_range(
        [&s](){
          s.insert(4, 4, utf8_char("x"));
        }));
      EQUAL(s, "abc"); // unmodified
    }
  }

  { // misc

    // A string with all ascii code points
    utf8_string s;
    for (unsigned int i = 0; i != 128; i++){
      s += utf8_char(i);
    }
    VERIFY(is_ascii(s));
    EQUAL(s.size(), 128);
    EQUAL(s.bytes(), s.size());
    EQUAL(s[0], utf8_char(0));
    EQUAL(s[127], utf8_char(127));
    EQUAL(s.find(chars::snowman), utf8_string::npos);

    size_t preBytes = s.bytes();
    EQUAL(chars::snowman.bytes(), 3);
    s += chars::snowman;
    EQUAL(s.bytes(), preBytes + 3);
    EQUAL(s[128], chars::snowman);

    EQUAL(s.find(chars::snowman, 0), 128);
    EQUAL(s.rfind(chars::snowman), 128);

    NOT(is_ascii(s));
    auto s2 = s + s;
    EQUAL(s2.size(), s.size() * 2);
    EQUAL(s2.bytes(), s.bytes() * 2);
    EQUAL(s2.substr(0, s.size()), s);
    s2 += s;

    utf8_string s3(s2);
    EQUAL(s2, s3);

    s2.clear();
    VERIFY(s2.empty());
    EQUAL(s2.size(), 0);
    EQUAL(s2.bytes(), 0);

    utf8_string expr("Hello\\perimeter(rect1, mm)");
    EQUAL(expr.find(utf8_char("\\")), 5);
    EQUAL(expr.find(chars::backslash), 5);

    // Ascii
    EQUAL(utf8_char("A").codepoint(), 0x41);
    EQUAL(utf8_char("B").codepoint(), 0x42);
    EQUAL(utf8_char("a").codepoint(), 0x61);
    EQUAL(utf8_char("b").codepoint(), 0x62);

    // One-byte, first code point
    EQUAL(utf8_char(0x0000).codepoint(), 0x0000);
    EQUAL(utf8_char(0x0000).bytes(), 1);

    // One-byte, last code point
    EQUAL(utf8_char(0x7f).codepoint(), 0x7f);
    EQUAL(utf8_char(0x7f).bytes(), 1);

    // Two-byte, first code point
    EQUAL(utf8_char(0x80).bytes(), 2);
    EQUAL(utf8_char(0x80).codepoint(), 0x80);

    // Two-byte, last code point
    EQUAL(utf8_char(0x7ff).bytes(), 2);
    EQUAL(utf8_char(0x7ff).codepoint(), 0x7ff);

    // Three-byte, first code point
    EQUAL(utf8_char(0x800).bytes(), 3);
    EQUAL(utf8_char(0x800).codepoint(), 0x800);

    // Three-byte, last code point
    EQUAL(utf8_char(0xffff).bytes(), 3);
    EQUAL(utf8_char(0xffff).codepoint(), 0xffff);

    // Four-byte, first code point
    EQUAL(utf8_char(0x10000).bytes(), 4);
    EQUAL(utf8_char(0x10000).codepoint(), 0x10000);

    // Four-byte, last code point
    EQUAL(utf8_char(0x1fffff).bytes(), 4);
    EQUAL(utf8_char(0x1fffff).codepoint(), 0x1fffff);

    // Old favorite
    EQUAL(chars::snowman.codepoint(), 0x2603);

    EQUAL(utf8_char("0").bytes(), 1);
    EQUAL(utf8_char("0").codepoint(), 0x30);
    EQUAL(utf8_char("1").bytes(), 1);
    EQUAL(utf8_char("1").codepoint(), 0x31);

    EQUAL(utf8_char("9").bytes(), 1);
    EQUAL(utf8_char("9").codepoint(), 0x39);
    VERIFY(ends("hello world", with("world")));
    VERIFY(!ends("hello morld", with("world")));
    VERIFY(!ends("orld", with("world")));
    VERIFY(ends("world", with("world")));
  }

  {
    // Iteration
    utf8_string alpha("abcdefghijklmnopqrstuvwxyz");
    VERIFY(std::is_sorted(begin(alpha), end(alpha)));
    VERIFY(std::all_of(begin(alpha), end(alpha), faint::isalpha));
    VERIFY(std::none_of(begin(alpha), end(alpha), faint::isdigit));

    {
      auto it = std::find(begin(alpha), end(alpha), utf8_char("a"));
      VERIFY(it == begin(alpha));
      EQUAL(*it, utf8_char("a"));
    }

    VERIFY(std::find(begin(alpha), end(alpha), utf8_char("1")) == end(alpha));

    {
      std::vector<utf8_char> v;
      std::for_each(begin(alpha), end(alpha), [&](auto c){v.push_back(c);});
      ASSERT(v.size() == 26);
      EQUAL(v[0], utf8_char("a"));
      EQUAL(v[25], utf8_char("z"));
    }

    {
      auto it = std::max_element(begin(alpha), end(alpha));
      ASSERT(it != end(alpha));
      EQUAL(*it, utf8_char("z"));
    }

    {
      auto it = std::min_element(begin(alpha), end(alpha));
      ASSERT(it != end(alpha));
      ASSERT(*it == utf8_char("a"));
    }

    EQUAL(std::count_if(begin(alpha), end(alpha), faint::isalpha), 26);
    EQUAL(std::count(begin(alpha), end(alpha), faint::utf8_char("a")), 1);
  }

  {
    utf8_string digits("0123456789");
    VERIFY(std::all_of(begin(digits), end(digits), faint::isdigit));
    VERIFY(std::none_of(begin(digits), end(digits), faint::isalpha));
  }

  {
    utf8_string mix("0123456789abcdefghijklmnopqrstuvwxyz");
    VERIFY(!std::all_of(begin(mix), end(mix), faint::isdigit));
    VERIFY(!std::all_of(begin(mix), end(mix), faint::isalpha));
    VERIFY(!std::none_of(begin(mix), end(mix), faint::isalpha));
    VERIFY(!std::none_of(begin(mix), end(mix), faint::isdigit));
    VERIFY(std::any_of(begin(mix), end(mix), faint::isdigit));
    VERIFY(std::any_of(begin(mix), end(mix), faint::isalpha));
  }
}
