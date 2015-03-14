// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#ifndef FAINT_ARC_HH
#define FAINT_ARC_HH
#include <vector>
#include "geo/angle.hh"
#include "geo/geo-fwd.hh"
#include "geo/point.hh"

namespace faint{

class AngleSpan{
public:
  AngleSpan();
  AngleSpan(const Angle& start, const Angle& stop);
  static AngleSpan Rad(coord start, coord stop){
    return AngleSpan(Angle::Rad(start), Angle::Rad(stop));
  }
  Angle start;
  Angle stop;
};

std::vector<Point> arc_as_path(const Tri&, const AngleSpan&);

class ArcEndPoints{
  // The end points for the arc with a specific angle span on an
  // ellipse specified by a tri.
public:
  ArcEndPoints(const Tri&, const AngleSpan&);
  Point operator[](size_t) const;
  std::vector<Point> GetVector() const;
  static const int num_points = 2;
private:
  Point m_p0;
  Point m_p1;
};

} // namespace

#endif
