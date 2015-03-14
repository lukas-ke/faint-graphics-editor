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

#include <cassert>
#include <cmath>
#include "geo/angle.hh"
#include "geo/arc.hh"
#include "geo/tri.hh"

namespace faint{

AngleSpan::AngleSpan()
  : start(Angle::Zero()),
    stop(Angle::Zero())
{}

AngleSpan::AngleSpan(const Angle& start, const Angle& stop)
  : start(start),
    stop(stop)
{}

int required_curve_count(const AngleSpan& angles){
  Angle span = angles.start - angles.stop;
  if (span <= pi / 2){
    return 2;
  }
  else if (span <= pi) {
    return 4;
  }
  else if (span <= 3 * pi / 2){
    return 6;
  }
  return 8;
}

ArcEndPoints::ArcEndPoints(const Tri& tri, const AngleSpan& angles){
  coord rx = tri.Width() / 2;
  coord ry = tri.Height() / 2;
  coord aCosEta1 = rx * cos(angles.start);
  coord bSinEta1 = ry * sin(angles.start);
  Point c(center_point(tri));
  Angle mainAngle(tri.GetAngle());
  coord x1 = c.x + aCosEta1 * cos(mainAngle) - bSinEta1 * sin(mainAngle);
  coord y1 = c.y + aCosEta1 * sin(mainAngle) + bSinEta1 * cos(mainAngle);

  // end point
  double aCosEta2 = rx * cos(angles.stop);
  double bSinEta2 = ry * sin(angles.stop);
  coord x2 = c.x + aCosEta2 * cos(mainAngle) - bSinEta2 * sin(mainAngle);
  coord y2 = c.y + aCosEta2 * sin(mainAngle) + bSinEta2 * cos(mainAngle);

  m_p0 = Point(x1, y1);
  m_p1 = Point(x2, y2);
}

Point ArcEndPoints::operator[](size_t i) const{
  assert(i <= 1);
  return i == 0 ? m_p0 : m_p1;
}

std::vector<Point> ArcEndPoints::GetVector() const{
  return { m_p0, m_p1 };
}

std::vector<Point> arc_as_path(const Tri& tri, const AngleSpan& angles){
  // This algorithm is based on documentation and code from
  // SpaceRoots.org, by L. Maisonobe "Drawing an elliptical arc using
  // polylines, quadratic or cubic Bezier curves" and
  // "EllipticalArc.java" Available here:
  // http://www.spaceroots.org/downloads.html

  const int numCurves = required_curve_count(angles);
  const double rx = tri.Width() / 2.0;
  const double ry = tri.Height() / 2.0;
  const Angle mainAngle(tri.GetAngle());
  const Point c(center_point(tri));

  Angle curveSpan = (angles.stop - angles.start) / numCurves;
  Angle currentAngle = angles.start;
  coord cosCurrentAngle = cos(currentAngle);
  coord sinCurrentAngle = sin(currentAngle);
  coord rxCosCurrentAngle = rx * cosCurrentAngle;
  coord rySinCurrentAngle = ry * sinCurrentAngle;
  coord rxSinCurrentAngle = rx * sinCurrentAngle;
  coord ryCosCurrentAngle = ry * cosCurrentAngle;
  coord sinMainAngle = sin(mainAngle);
  coord cosMainAngle = cos(mainAngle);

  coord x = c.x + rxCosCurrentAngle * cosMainAngle -
    rySinCurrentAngle * sinMainAngle;
  coord y = c.y + rxCosCurrentAngle * sinMainAngle +
    rySinCurrentAngle * cosMainAngle;
  coord xDot = -rxSinCurrentAngle * cosMainAngle -
    ryCosCurrentAngle * sinMainAngle;
  coord yDot = -rxSinCurrentAngle * sinMainAngle +
    ryCosCurrentAngle * cosMainAngle;

  // Fixme: Using Point requires knowledge that one should move_to or
  // line_to v[0], and then use curve_to with the remaining points (in
  // sets of three), use PathPt (or similar) instead.
  std::vector<Point> v;
  // Arc start point
  v.emplace_back(x, y);
  coord t = tan(0.5 * curveSpan);
  coord alpha = sin(curveSpan) * (std::sqrt(4 + 3 * t * t) - 1) / 3.0;

  for (int i = 0; i != numCurves; i++){
    coord prevX = x;
    coord prevY = y;
    double prevXDot = xDot;
    double prevYDot = yDot;
    currentAngle += curveSpan;
    cosCurrentAngle = cos(currentAngle);
    sinCurrentAngle = sin(currentAngle);
    rxCosCurrentAngle = rx * cosCurrentAngle;
    rySinCurrentAngle = ry * sinCurrentAngle;
    rxSinCurrentAngle = rx * sinCurrentAngle;
    ryCosCurrentAngle = ry * cosCurrentAngle;
    x = c.x + rxCosCurrentAngle * cosMainAngle - rySinCurrentAngle * sinMainAngle;
    y = c.y + rxCosCurrentAngle * sinMainAngle + rySinCurrentAngle * cosMainAngle;
    xDot = -rxSinCurrentAngle * cosMainAngle - ryCosCurrentAngle * sinMainAngle;
    yDot = -rxSinCurrentAngle * sinMainAngle + ryCosCurrentAngle * cosMainAngle;
    v.emplace_back(prevX + alpha * prevXDot, prevY + alpha * prevYDot);
    v.emplace_back(x - alpha * xDot, y - alpha * yDot);
    v.emplace_back(x, y);
  }
  return v;
}

} // namespace
