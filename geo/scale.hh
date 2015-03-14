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

#ifndef FAINT_SCALE_HH
#define FAINT_SCALE_HH
#include "geo/size.hh"
#include "util/distinct.hh"

namespace faint{

using NewSize = Order<Size>::New;

class Scale{
public:
  constexpr Scale(coord x, coord y) : x(x), y(y) {}
  constexpr Scale(coord xy) : x(xy), y(xy) {}
  Scale(const NewSize&, const Size&);
  Scale(int, int) = delete;
  coord x;
  coord y;
};

Scale invert_x_scale();
Scale invert_y_scale();
Scale abs(const Scale&);
Size operator*(const Scale&, const Size&);
Size operator*(const Size&, const Scale&);
Scale inverse(const Scale&);

} // namespace

#endif
