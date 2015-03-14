// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "util/enum-util.hh"

namespace {

enum class E : int {
  MIN_VALUE = 0,

  FIRST_ENTRY = MIN_VALUE,
  SECOND_ENTRY,
  THIRD_ENTRY,

  MAX_VALUE = THIRD_ENTRY,
  BAD_VALUE = 100
};

}

void test_enum_util(){
  using namespace faint;
  NOT(within_enum<E>(-1));
  VERIFY(within_enum<E>(0));
  VERIFY(within_enum<E>(1));
  VERIFY(within_enum<E>(2));
  NOT(within_enum<E>(3));

  VERIFY(to_enum(-1, default_to(E::BAD_VALUE)) == E::BAD_VALUE);
  VERIFY(to_enum(0, default_to(E::BAD_VALUE)) == E::FIRST_ENTRY);
  VERIFY(to_enum(1, default_to(E::BAD_VALUE)) == E::SECOND_ENTRY);
  VERIFY(to_enum(2, default_to(E::BAD_VALUE)) == E::THIRD_ENTRY);
  VERIFY(to_enum(3, default_to(E::BAD_VALUE)) == E::BAD_VALUE);
}
