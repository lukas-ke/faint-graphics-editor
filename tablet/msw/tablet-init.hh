// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#ifndef FAINT_TABLET_INIT_HH
#define FAINT_TABLET_INIT_HH
#include "tablet/msw/tablet-error.hh"

#ifndef _WIN32
#error tablet-init.hh included on non-VC compiler
#endif

namespace faint{namespace tablet{

InitResult initialize(HINSTANCE, HWND);
void uninitialize();
bool is_wt_packet(unsigned int);
void msg_activate(WPARAM, LPARAM);
void activate(bool);
struct WTP{
  int x;
  int y;
  int pressure;
  int cursor;
};

WTP get_wt_packet(WPARAM wParam, LPARAM lParam);

}} // namespace

#endif
