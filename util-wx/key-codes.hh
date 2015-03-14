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

#ifndef FAINT_KEY_CODES_HH
#define FAINT_KEY_CODES_HH
#include "util/key-press.hh"

namespace faint{namespace key{

extern const Key one;
extern const Key alt;
extern const Key asterisk;
extern const Key A;
extern const Key B;
extern const Key C;
extern const Key H;
extern const Key V;
extern const Key E;
extern const Key D;
extern const Key I;
extern const Key J;
extern const Key N;
extern const Key O;
extern const Key P;
extern const Key Q;
extern const Key T;
extern const Key W;
extern const Key back;
extern const Key ctrl;
extern const Key del;
extern const Key down;
extern const Key end;
extern const Key enter;
extern const Key esc;
extern const Key home;
extern const Key left;
extern const Key minus;
extern const Key nine;
extern const Key num_minus;
extern const Key paragraph;
extern const Key num_plus;
extern const Key plus;
extern const Key pgdn;
extern const Key pgup;
extern const Key right;
extern const Key space;
extern const Key shift;
extern const Key tab;
extern const Key up;
extern const Key zero;
extern const Key F1;
extern const Key F2;
extern const Key F3;
extern const Key F4;
extern const Key F5;
extern const Key F6;
extern const Key F7;
extern const Key F8;
extern const Key F9;
extern const Key F10;
extern const Key F11;
extern const Key F12;
bool modifier(int key);

}} // namespace

#endif
