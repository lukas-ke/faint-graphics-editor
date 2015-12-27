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

#include <sstream>
#include "util/points-to-svg-path-string.hh"
#include "geo/pathpt-iter.hh"
#include "text/slice.hh"

namespace faint{

utf8_string points_to_svg_path_string(const std::vector<PathPt>& points){
  std::stringstream ss;

  for_each_pt(points,
    [&ss](const ArcTo& a){
      ss << "A" << " " <<
        a.r.x << " " <<
        a.r.y << " " <<
        a.axisRotation.Deg() << " " <<
        a.largeArcFlag << " " <<
        a.sweepFlag << " " <<
        a.p.x << " " <<
        a.p.y << " ";
    },
    [&ss](const Close&){
      ss << "z ";
    },
    [&ss](const CubicBezier& b){
      ss << "C " <<
        b.c.x << " " << b.c.y << " " <<
        b.d.x << " " << b.d.y << " " <<
        b.p.x << " " << b.p.y << " ";
    },
    [&ss](const LineTo& l){
      ss << "L " <<
        l.p.x << " " << l.p.y << " ";
    },
    [&ss](const MoveTo& m){
      ss << "M " <<
        m.p.x << " " << m.p.y << " ";
    });

  return utf8_string(slice_up_to(ss.str(), -1));
}

} // namespace
