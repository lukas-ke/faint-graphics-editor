// -*- coding: us-ascii-unix -*-
#include <vector>
#include <algorithm>
#include "test-sys/test.hh"
#include "text/utf8-string.hh"
#include "util/iter.hh"

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
    // Test "but_first" with single element
    std::vector<int> v{0};
    ASSERT(v.size() == 1);
    auto gen(but_first(v));
    VERIFY(distance(begin(gen), end(gen)) == 0);
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
    for (const auto& [str, ch] : zip(strings, chars)){
      ASSERT(i < 6);
      EQUAL(str, strings[to_size_t(i)]);
      EQUAL(ch, chars[to_size_t(i)]);
      i++;
    }
  }

  {
    // Test "iterable"
    enum class IterableTestEnum{
      BEGIN = 0,
      FIRST_VALUE = 0,
      SECOND_VALUE = 1,
      THIRD_VALUE = 2,
      END
    };

    int i = 0;
    for (IterableTestEnum value : iterable<IterableTestEnum>()){
      ASSERT(value < IterableTestEnum::END);
      VERIFY(static_cast<int>(value) == i);
      i++;
    }
  }
}
