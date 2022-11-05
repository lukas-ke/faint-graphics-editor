// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "bitmap/color.hh"
#include "text/formatting.hh"

void test_parse_hex_color(){
  using namespace faint;
  const auto f = [](const Optional<ColRGB>& c){
    return c.Visit(
      [](const ColRGB& c){
        return str(c);
      },
      [](){
        return "no color";
      });
  };

  const Optional<ColRGB> no_color{no_option()};
  EQUALF(parse_hex_color(""), no_color, f);
  EQUALF(parse_hex_color("-1"), no_color, f);
  EQUALF(parse_hex_color("#-1"), no_color, f);
  EQUALF(parse_hex_color("#ff00qq"), no_color, f);
  EQUALF(parse_hex_color("#fffffff"), no_color, f);
  EQUALF(parse_hex_color("#000000"), option(ColRGB(0, 0, 0)), f);
  EQUALF(parse_hex_color("#123456"), option(ColRGB(18, 52, 86)), f);
  EQUALF(parse_hex_color("#ffffff"), option(ColRGB(255, 255, 255)), f);
}
