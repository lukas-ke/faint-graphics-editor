// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#ifndef FWD_DC_WX_HH
#define FWD_DC_WX_HH
#include <vector>

class IntPoint;
class wxDC;

namespace faint{

class Color;

void set_transparent_brush(wxDC&);
void set_brush(wxDC&, const Color&);
void set_pen(wxDC&, const Color&, int w=1);
void set_dotted_pen(wxDC&, const Color&, int w=1);
void draw_polygon(wxDC&, const std::vector<IntPoint>&);
void draw_line(wxDC&, const IntPoint&, const IntPoint&);

} // namespace

#endif
