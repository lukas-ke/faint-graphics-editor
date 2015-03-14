// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include <vector>
#include "text/utf8-string.hh"
#include "util/iter.hh"

namespace{

enum class IterableTestEnum{
  BEGIN = 0,
  FIRST_VALUE = 0,
  SECOND_VALUE = 1,
  THIRD_VALUE = 2,
  END
};

}

void test_iter(){
  using namespace faint;

  std::vector<utf8_string> strings = {"Can't",
                                      "think",
                                      "of",
                                      "any",
                                      "interesting",
                                      "example text."};

  std::vector<char> chars = {'A', 'B', 'C', 'D', 'E', 'F'};
  {
    // Test "enumerate"
    int i = 0;
    for (auto item : enumerate(strings)){
      ASSERT(i < 6);
      ASSERT(item.num == i);
      EQUAL(*item, strings[to_size_t(item.num)]);
      i++;
    }
  }

  {
    // Test "but_first"
    int i = 0;
    for (const auto& item : but_first(strings)){
      ASSERT(i < 5);
      ASSERT(item == strings[to_size_t(i + 1)]);
      i++;
    }
  }

  {
    // Test "but_last"
    size_t i = 0;
    for (const auto& item : but_last(strings)){
      ASSERT(i < 5);
      EQUAL(item, strings[i]);
      i++;
    }
  }

  {
    // Test "reversed"
    int i = 0;
    for (const auto& item : reversed(strings)){
      ASSERT(i < 6);
      EQUAL(item, strings[to_size_t(5 - i)]);
      i++;
    }
  }

  {
    // Mega-test
    int i = 0;
    for (auto it : enumerate(reversed(but_last(strings)))){
      ASSERT(i < 5);
      ASSERT(i == it.num);
      const size_t index = to_size_t(4 - i);
      EQUAL(*it, strings[index]);
      EQUAL(it->str()[0], strings[index].str()[0]);
      i++;
    }
  }

  {
    // Test "zip"
    int i = 0;
    for (auto item : zip(strings, chars)){
      ASSERT(i < 6);
      EQUAL(item.first, strings[to_size_t(i)]);
      EQUAL(item.second, chars[to_size_t(i)]);
      i++;
    }
  }

  {
    // Test "iterable"
    int i = 0;
    for (IterableTestEnum value : iterable<IterableTestEnum>()){
      ASSERT(value < IterableTestEnum::END);
      VERIFY(static_cast<int>(value) == i);
      i++;
    }
  }
}
