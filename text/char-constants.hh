// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You
// may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef FAINT_CHAR_CONSTANTS_HH
#define FAINT_CHAR_CONSTANTS_HH
#include "text/utf8-char.hh"

namespace faint{

constexpr utf8_char utf8_null(0x00);
constexpr utf8_char backslash(0x5c); // aka "reverse solidus"
constexpr utf8_char bullet(0x2022);
constexpr utf8_char comma(0x2c);
constexpr utf8_char degree_sign(0xb0);
constexpr utf8_char double_prime(0x2033);
constexpr utf8_char downwards_arrow_with_tip_leftwards(0x21b2);
constexpr utf8_char eol(0x0a);
constexpr utf8_char euro_sign(0x20ac);
constexpr utf8_char exclamation_mark(0x21);
constexpr utf8_char full_stop(0x2e);
constexpr utf8_char greek_capital_letter_delta(0x394);
constexpr utf8_char greek_small_letter_pi(0x3c0);
constexpr utf8_char hyphen(0x2D);
constexpr utf8_char left_parenthesis(0x28);
constexpr utf8_char middle_dot(0x00b7);
constexpr utf8_char no_break_space(0xc2a0);
constexpr utf8_char pilcrow_sign(0x00b6);
constexpr utf8_char prime(0x2032);
constexpr utf8_char question_mark(0x3f);
constexpr utf8_char replacement_character(0xfffd);
constexpr utf8_char right_parenthesis(0x29);
constexpr utf8_char snowman(0x2603);
constexpr utf8_char space(0x20);
constexpr utf8_char superscript_two(0xb2);
constexpr utf8_char tab(0x9);
constexpr utf8_char triple_prime(0x2034);

inline bool is_punctuation(const utf8_char& ch){
  return ch == comma ||
    ch == exclamation_mark ||
    ch == full_stop ||
    ch == question_mark;
}

} // namespace

#endif
