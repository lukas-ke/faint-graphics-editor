// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "util/at-most.hh"

void test_at_most(){
  using faint::AtMost2;

  {
    // No elements - the first function should be called, (with no
    // arguments)
    AtMost2<int> zero;
    zero.Visit(
      FAIL_UNLESS_CALLED(),
      FAIL_IF_CALLED(),
      FAIL_IF_CALLED());
  }

  {
    // One element - the second function should be called with one matching
    // integer
    AtMost2<int> one(10);
    auto eq_10 = [](int v){
      EQUAL(v, 10);
    };
    one.Visit(
      FAIL_IF_CALLED(),
      FAIL_UNLESS_CALLED_FWD(eq_10),
      FAIL_IF_CALLED());
  }

  {
    // Two elements - the third function should be called with two
    // matching integers.
    AtMost2<int> two(10,20);
    auto eq_10_20 = [](int v0, int v1){
      EQUAL(v0, 10);
      EQUAL(v1, 20);
    };
    two.Visit(
      FAIL_IF_CALLED(),
      FAIL_IF_CALLED(),
      FAIL_UNLESS_CALLED_FWD(eq_10_20));
  }
}
