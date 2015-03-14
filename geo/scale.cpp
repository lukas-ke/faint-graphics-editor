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

#include <cmath>
#include "geo/scale.hh"

namespace faint{

Scale::Scale(const NewSize& inNew, const Size& oldSz){
  const Size& newSz(inNew.Get());
  x = newSz.w / oldSz.w;
  y = newSz.h / oldSz.h;
}

Scale invert_x_scale(){
  return Scale(-1.0, 1.0);
}

Scale invert_y_scale(){
  return Scale(1.0, -1.0);
}

Scale abs(const Scale& sc){
  return Scale(std::fabs(sc.x), std::fabs(sc.y));
}

Size operator*(const Scale& sc, const Size& sz){
  return Size(sc.x * sz.w, sc.y * sz.h);
}

Size operator*(const Size& sz, const Scale& sc){
  return Size(sc.x * sz.w, sc.y * sz.h);
}

Scale inverse(const Scale& sc){
  if (sc.x == 0 || sc.y == 0){
    return sc;
  }
  return Scale(1.0 / sc.x, 1.0 / sc.y);
}

} // namespace
