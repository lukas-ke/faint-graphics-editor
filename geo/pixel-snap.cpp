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

#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/pixel-snap.hh"
#include "geo/points.hh"
#include "geo/pathpt-iter.hh"

namespace faint{

Point pixel_snap_xy(const Point& p, coord){ // Fixme: Use lineWidth
  return floated(truncated(p)) + Point::Both(0.5);
}

Point pixel_snap_xy_middle(const Point& p){
  return floated(rounded(p));
}

Tri pixel_snap(const Tri& t, coord lineWidth){ // Fixme: Use lineWidth
  return {
    pixel_snap_xy(t.P0(), lineWidth),
    pixel_snap_xy(t.P1(), lineWidth),
    pixel_snap_xy(t.P2(), lineWidth)};
}

Tri pixel_snap_middle(const Tri& t){
  return {
    pixel_snap_xy_middle(t.P0()),
    pixel_snap_xy_middle(t.P1()),
    pixel_snap_xy_middle(t.P2())};
}

static MoveTo pixel_snap(const MoveTo& m, coord lineWidth){ // Fixme: Use lineWidth
  return MoveTo(pixel_snap_xy(m.p, lineWidth));
}

static LineTo pixel_snap(const LineTo& l, coord lineWidth){ // Fixme: Use lineWidth
  return LineTo(pixel_snap_xy(l.p, lineWidth));
}

Points pixel_snap(const Points& src, coord lineWidth){
  if (src.Size() <= 1){
    return src;
  }
  std::vector<PathPt> pts(src.GetPoints());
  std::vector<PathPt> out;
  out.reserve(pts.size());

  for_each_pt(pts,
    [&](const ArcTo& a){out.push_back(a);},
    [&](const Close& c){out.push_back(c);},
    [&](const CubicBezier& b){out.push_back(b);},
    [&](const LineTo& l){
      // Fixme: Too simplistic. Needs to account for the previous point
      // to avoid getting blended line end-points.
      out.push_back(pixel_snap(l, lineWidth));
    },
    [&](const MoveTo& m){
      // Fixme: See line-to note
      out.push_back(pixel_snap(m, lineWidth));
    });
  return Points(out);
}

} // namespace
