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

#include "wx/defs.h"
#include "util-wx/key-codes.hh"

namespace faint{namespace key{

const Key alt(WXK_ALT);
const Key asterisk(39);
const Key back(WXK_BACK);
const Key ctrl(WXK_CONTROL);
const Key A('A');
const Key B('B');
const Key C('C');
const Key D('D');
const Key E('E');
const Key H('H');
const Key I('I');
const Key J('J');
const Key N('N');
const Key O('O');
const Key P('P');
const Key Q('Q');
const Key T('T');
const Key V('V');
const Key W('W');
const Key del(127);
const Key down(WXK_DOWN);
const Key end(WXK_END);
const Key enter(WXK_RETURN);
const Key esc(WXK_ESCAPE);
const Key F1(WXK_F1);
const Key F2(WXK_F2);
const Key F3(WXK_F3);
const Key F4(WXK_F4);
const Key F5(WXK_F5);
const Key F6(WXK_F6);
const Key F7(WXK_F7);
const Key F8(WXK_F8);
const Key F9(WXK_F9);
const Key F10(WXK_F10);
const Key F11(WXK_F11);
const Key F12(WXK_F12);
const Key home(WXK_HOME);
const Key left(WXK_LEFT);
const Key minus(45);
const Key nine(57);
const Key num_plus(388);
const Key num_minus(390);
const Key paragraph(167);
const Key pgdn(WXK_PAGEDOWN);
const Key pgup(WXK_PAGEUP);
const Key plus(43);
const Key right(WXK_RIGHT);
const Key shift(WXK_SHIFT);
const Key tab(WXK_TAB);
const Key space(32);
const Key up(WXK_UP);
const Key zero(48);

bool modifier(int key){
  return key == shift || key == ctrl || key == alt;
}

}} // namespace
