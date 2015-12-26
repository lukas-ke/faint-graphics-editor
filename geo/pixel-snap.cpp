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
#include "util/make-vector.hh"
#include "util/type-util.hh"

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

std::vector<PathPt> pixel_snap(const std::vector<PathPt>& pts, coord lineWidth){
  if (pts.size() <= 1){
    return pts;
  }

  const auto snap = [lineWidth](const auto& p){
    // Fixme: Too simplistic. Needs to account for the previous point
    // to avoid getting blended line end-points.
    return pixel_snap(p, lineWidth);
  };

  return make_vector(pts, [lineWidth, snap](const PathPt& pt){
    return pt.Visit(
      convert_f<ArcTo, PathPt>,
      convert_f<Close, PathPt>,
      convert_f<CubicBezier, PathPt>,
      snap,
      snap);
    });
}

Points pixel_snap(const Points& src, coord lineWidth){
  return Points(pixel_snap(src.GetPoints(), lineWidth));
}

} // namespace
