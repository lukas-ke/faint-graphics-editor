// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "text/slice.hh"

void test_slice(){
  using namespace faint;

  {
    // slice
    EQUAL(slice("asdf", 0, 0), "");
    EQUAL(slice("asdf", 0, 1), "a");
    EQUAL(slice("asdf", 1, 1), "");
    EQUAL(slice("asdf", 1, 2), "s");
    EQUAL(slice("asdf", 1, 4), "sdf");
    EQUAL(slice("asdf", 1, 5), "sdf");
    EQUAL(slice("asdf", 0, 5), "asdf");

    EQUAL(slice("asdf", 4, 2), "");

    EQUAL(slice("asdf", 0, -1), "asd");
    EQUAL(slice("asdf", 0, -3), "a");
    EQUAL(slice("asdf", 0, -4), "");
    EQUAL(slice("asdf", 0, -5), "");

    EQUAL(slice("asdf", -5, 2), "as");
    EQUAL(slice("asdf", -3, -2), "s");
    EQUAL(slice("asdf", -2, -1), "d");

    EQUAL(slice("asdf", 10, 20), "");
    EQUAL(slice("", 0, 1), "");
  }

  {
    // slice_from
    EQUAL(slice_from("asdf", 0), "asdf");
    EQUAL(slice_from("asdf", 1), "sdf");
    EQUAL(slice_from("asdf", 3), "f");
    EQUAL(slice_from("asdf", 4), "");
    EQUAL(slice_from("asdf", 5), "");

    EQUAL(slice_from("asdf", -1), "f");
    EQUAL(slice_from("asdf", -2), "df");
    EQUAL(slice_from("asdf", -4), "asdf");
    EQUAL(slice_from("asdf", -5), "asdf");

    EQUAL(slice_from("", 0), "");
    EQUAL(slice_from("", 1), "");
  }

  {
    // slice_up_to
    EQUAL(slice_up_to("asdf", 0), "");
    EQUAL(slice_up_to("asdf", 1), "a");
    EQUAL(slice_up_to("asdf", 4), "asdf");
    EQUAL(slice_up_to("asdf", 5), "asdf");

    EQUAL(slice_up_to("asdf", -0), "");
    EQUAL(slice_up_to("asdf", -1), "asd");
    EQUAL(slice_up_to("asdf", -2), "as");
    EQUAL(slice_up_to("asdf", -4), "");
    EQUAL(slice_up_to("asdf", -5), "");
  }

  {
    // char_at
    EQUAL(char_at("asdf", 0), "a");
    EQUAL(char_at("asdf", 3), "f");
    EQUAL(char_at("asdf", 4), "");
    EQUAL(char_at("asdf", 5), "");
    EQUAL(char_at("asdf", -1), "f");
    EQUAL(char_at("asdf", -2), "d");
    EQUAL(char_at("asdf", -4), "a");
    EQUAL(char_at("asdf", -6), "");
  }
}
