// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "text/auto-complete.hh"
#include "text/char-constants.hh"

void test_auto_complete(){
  using namespace faint;

  const auto euroStr = utf8_string(1, euro_sign);
  const auto snowmanStr = utf8_string(1, snowman);
  const auto sqSstr = utf8_string(1, superscript_two);

  AutoComplete ac({euroStr,
        "a",
        "ab",
        "aaa",
        "abc",
        "abb",
        "ab1",
        "ab2",
        "qwerty",
        snowmanStr + snowmanStr,
        snowmanStr + sqSstr + euroStr});

  {
    // Test "AutoComplete"
    Words w = ac.match("a");
    ASSERT_EQUAL(w.size(), 7);
    ASSERT_EQUAL(w.size(), 7);
    NOT(w.empty());
    EQUAL(w.get(0), "a");
    EQUAL(w.get(1), "aaa");
    EQUAL(w.get(2), "ab");
    EQUAL(w.get(3), "ab1");
    EQUAL(w.get(4), "ab2");
    EQUAL(w.get(5), "abb");
    EQUAL(w.get(6), "abc");

    w = ac.match("b");
    ASSERT_EQUAL(w.size(), 0);
    VERIFY(w.empty());

    w = ac.match("ab");
    ASSERT_EQUAL(w.size(), 5);
    EQUAL(w.get(0), "ab");
    EQUAL(w.get(1), "ab1");
    EQUAL(w.get(2), "ab2");
    EQUAL(w.get(3), "abb");
    EQUAL(w.get(4), "abc");

    w = ac.match("ab1");
    ASSERT_EQUAL(w.size(), 1);
    EQUAL(w.get(0), "ab1");

    w = ac.match("abc");
    ASSERT_EQUAL(w.size(), 1);
    EQUAL(w.get(0), "abc");

    w = ac.match("abcd");
    VERIFY(w.empty());

    w = ac.match("q");
    ASSERT_EQUAL(w.size(), 1);
    EQUAL(w.get(0), "qwerty");

    w = ac.match(snowmanStr);
    ASSERT_EQUAL(w.size(), 2);
    EQUAL(w.get(0), snowmanStr + sqSstr + euroStr);
    EQUAL(w.get(1), snowmanStr + snowmanStr);

    w = ac.match(euroStr);
    ASSERT_EQUAL(w.size(), 1);
    EQUAL(w.get(0), euroStr);
  }

  {
    // Test "AutoCompleteState"
    AutoCompleteState acs(ac);
    EQUAL(acs.Complete("a"), "a");
    ASSERT(!acs.Empty());
    EQUAL(acs.Next(), "aaa");
    EQUAL(acs.Next(), "ab");
    EQUAL(acs.Next(), "ab1");
    EQUAL(acs.Next(), "ab2");
    EQUAL(acs.Next(), "abb");
    EQUAL(acs.Next(), "abc");
    EQUAL(acs.Next(), "a");
    EQUAL(acs.Prev(), "abc");
    EQUAL(acs.Prev(), "abb");

    EQUAL(acs.Complete("ab1"), "ab1");
    ASSERT(!acs.Empty());
    EQUAL(acs.Next(), "ab1");
    EQUAL(acs.Next(), "ab1");
    EQUAL(acs.Prev(), "ab1");
    EQUAL(acs.Prev(), "ab1");

    acs.Forget();
    VERIFY(acs.Empty());

    EQUAL(acs.Complete("q"), "qwerty");
    ASSERT(!acs.Empty());
    EQUAL(acs.Next(), "qwerty");

    EQUAL(acs.Complete(snowmanStr), snowmanStr + sqSstr + euroStr);
    ASSERT(!acs.Empty());
    EQUAL(acs.Next(), snowmanStr + snowmanStr);
    EQUAL(acs.Next(), snowmanStr + sqSstr + euroStr);
  }
}
